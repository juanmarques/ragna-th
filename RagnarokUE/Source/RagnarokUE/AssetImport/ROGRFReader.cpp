// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROGRFReader.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Compression.h"

FROGRFReader::FROGRFReader()
{
}

FROGRFReader::~FROGRFReader()
{
	Close();
}

bool FROGRFReader::Open(const FString& FilePath)
{
	Close();

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FileHandle = PlatformFile.OpenRead(*FilePath);
	if (!FileHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("ROGRFReader: Failed to open file: %s"), *FilePath);
		return false;
	}

	OpenFilePath = FilePath;

	// Read and validate header
	uint8 HeaderBuf[GRF_HEADER_SIZE];
	if (!FileHandle->Read(HeaderBuf, GRF_HEADER_SIZE))
	{
		UE_LOG(LogTemp, Error, TEXT("ROGRFReader: Failed to read header from: %s"), *FilePath);
		Close();
		return false;
	}

	// Validate signature: "Master of Magic\0" (16 bytes)
	if (FMemory::Memcmp(HeaderBuf, GRF_SIGNATURE, 15) != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("ROGRFReader: Invalid GRF signature in: %s"), *FilePath);
		Close();
		return false;
	}

	// Parse header fields (after 30-byte signature+key block)
	const uint8* H = HeaderBuf + 30;
	uint32 FileTableOffset = *(reinterpret_cast<const uint32*>(H));
	uint32 Seed = *(reinterpret_cast<const uint32*>(H + 4));
	uint32 RawFileCount = *(reinterpret_cast<const uint32*>(H + 8));
	Version = *(reinterpret_cast<const uint32*>(H + 12));

	if (Version != 0x0200)
	{
		UE_LOG(LogTemp, Warning, TEXT("ROGRFReader: GRF version 0x%04X may not be fully supported (expected 0x0200)"), Version);
	}

	// For v0x0200: actual file count = RawFileCount - Seed - 7
	int32 ActualFileCount = static_cast<int32>(RawFileCount) - static_cast<int32>(Seed) - 7;
	if (ActualFileCount < 0)
	{
		ActualFileCount = 0;
	}

	UE_LOG(LogTemp, Log, TEXT("ROGRFReader: Opened %s - Version: 0x%04X, Files: %d"), *FilePath, Version, ActualFileCount);

	if (!ParseFileTable())
	{
		UE_LOG(LogTemp, Error, TEXT("ROGRFReader: Failed to parse file table"));
		Close();
		return false;
	}

	return true;
}

void FROGRFReader::Close()
{
	if (FileHandle)
	{
		delete FileHandle;
		FileHandle = nullptr;
	}
	FileEntries.Empty();
	OpenFilePath.Empty();
	Version = 0;
}

TArray<FString> FROGRFReader::GetFileList() const
{
	TArray<FString> Result;
	FileEntries.GetKeys(Result);
	return Result;
}

TArray<FString> FROGRFReader::FindFiles(const FString& Pattern) const
{
	TArray<FString> Result;
	for (const auto& Pair : FileEntries)
	{
		if (Pair.Key.MatchesWildcard(Pattern))
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}

bool FROGRFReader::FileExists(const FString& InternalPath) const
{
	return FileEntries.Contains(InternalPath.ToLower());
}

bool FROGRFReader::ExtractFile(const FString& InternalPath, TArray<uint8>& OutData) const
{
	const FROGRFEntry* Entry = GetEntry(InternalPath);
	if (!Entry)
	{
		UE_LOG(LogTemp, Warning, TEXT("ROGRFReader: File not found in archive: %s"), *InternalPath);
		return false;
	}

	if (!FileHandle)
	{
		return false;
	}

	// Seek to the file data (offset is relative to end of header)
	int64 DataOffset = static_cast<int64>(Entry->Offset) + GRF_HEADER_SIZE;
	if (!FileHandle->Seek(DataOffset))
	{
		UE_LOG(LogTemp, Error, TEXT("ROGRFReader: Failed to seek to offset %lld for: %s"), DataOffset, *InternalPath);
		return false;
	}

	// Read compressed data
	TArray<uint8> CompressedData;
	CompressedData.SetNum(Entry->CompressedSizeAligned);
	if (!FileHandle->Read(CompressedData.GetData(), Entry->CompressedSizeAligned))
	{
		UE_LOG(LogTemp, Error, TEXT("ROGRFReader: Failed to read compressed data for: %s"), *InternalPath);
		return false;
	}

	// If uncompressed size matches compressed size, data is stored uncompressed
	if (Entry->UncompressedSize == Entry->CompressedSize)
	{
		OutData = MoveTemp(CompressedData);
		OutData.SetNum(Entry->UncompressedSize);
		return true;
	}

	// Decompress - pass only CompressedSize bytes to zlib (ignore alignment padding)
	CompressedData.SetNum(Entry->CompressedSize);
	return DecompressZlib(CompressedData, OutData, Entry->UncompressedSize);
}

const FROGRFEntry* FROGRFReader::GetEntry(const FString& InternalPath) const
{
	return FileEntries.Find(InternalPath.ToLower());
}

bool FROGRFReader::ParseFileTable()
{
	if (!FileHandle)
	{
		return false;
	}

	// Read header again to get file table offset
	uint8 HeaderBuf[GRF_HEADER_SIZE];
	FileHandle->Seek(0);
	FileHandle->Read(HeaderBuf, GRF_HEADER_SIZE);

	const uint8* H = HeaderBuf + 30;
	uint32 FileTableOffset = *(reinterpret_cast<const uint32*>(H));

	// Seek to file table
	int64 TablePos = static_cast<int64>(FileTableOffset) + GRF_HEADER_SIZE;
	if (!FileHandle->Seek(TablePos))
	{
		return false;
	}

	// Read compressed and uncompressed sizes of the file table
	uint32 CompressedTableSize = 0;
	uint32 UncompressedTableSize = 0;
	FileHandle->Read(reinterpret_cast<uint8*>(&CompressedTableSize), 4);
	FileHandle->Read(reinterpret_cast<uint8*>(&UncompressedTableSize), 4);

	// Read compressed file table
	TArray<uint8> CompressedTable;
	CompressedTable.SetNum(CompressedTableSize);
	if (!FileHandle->Read(CompressedTable.GetData(), CompressedTableSize))
	{
		return false;
	}

	// Decompress file table
	TArray<uint8> TableData;
	if (!DecompressZlib(CompressedTable, TableData, UncompressedTableSize))
	{
		UE_LOG(LogTemp, Error, TEXT("ROGRFReader: Failed to decompress file table"));
		return false;
	}

	// Parse entries from the decompressed table
	const uint8* Cursor = TableData.GetData();
	const uint8* End = Cursor + TableData.Num();

	while (Cursor < End)
	{
		// Read null-terminated filename
		const char* NameStart = reinterpret_cast<const char*>(Cursor);
		int32 NameLen = 0;
		while (Cursor < End && *Cursor != 0)
		{
			++Cursor;
			++NameLen;
		}
		if (Cursor >= End)
		{
			break;
		}
		++Cursor; // Skip null terminator

		// Need at least 17 bytes for the entry fields
		if (Cursor + 17 > End)
		{
			break;
		}

		FROGRFEntry Entry;
		Entry.FileName = FString(NameLen, NameStart);

		Entry.CompressedSize = *(reinterpret_cast<const uint32*>(Cursor));
		Cursor += 4;
		Entry.CompressedSizeAligned = *(reinterpret_cast<const uint32*>(Cursor));
		Cursor += 4;
		Entry.UncompressedSize = *(reinterpret_cast<const uint32*>(Cursor));
		Cursor += 4;
		Entry.Flags = *Cursor;
		Cursor += 1;
		Entry.Offset = *(reinterpret_cast<const uint32*>(Cursor));
		Cursor += 4;

		if (Entry.IsFile() && Entry.UncompressedSize > 0)
		{
			FileEntries.Add(Entry.FileName.ToLower(), Entry);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ROGRFReader: Parsed %d file entries"), FileEntries.Num());
	return true;
}

bool FROGRFReader::DecompressZlib(const TArray<uint8>& CompressedData, TArray<uint8>& OutData, uint32 UncompressedSize)
{
	OutData.SetNum(UncompressedSize);
	int32 ActualUncompressedSize = static_cast<int32>(UncompressedSize);

	if (!FCompression::UncompressMemory(
		NAME_Zlib,
		OutData.GetData(),
		ActualUncompressedSize,
		CompressedData.GetData(),
		CompressedData.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("ROGRFReader: Zlib decompression failed"));
		return false;
	}

	return true;
}
