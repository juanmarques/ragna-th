// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * GRF (Game Resource File) archive entry metadata.
 */
struct FROGRFEntry
{
	FString FileName;
	uint32 CompressedSize = 0;
	uint32 CompressedSizeAligned = 0;
	uint32 UncompressedSize = 0;
	uint8 Flags = 0;
	uint32 Offset = 0;

	bool IsFile() const { return (Flags & 0x01) != 0; }
};

/**
 * FROGRFReader
 * Reads Ragnarok Online GRF archive files and extracts individual resources.
 * Supports GRF version 0x0200 (the standard format used by most RO clients).
 */
class RAGNAROKUE_API FROGRFReader
{
public:
	FROGRFReader();
	~FROGRFReader();

	/**
	 * Open a GRF archive file.
	 * @param FilePath Absolute path to the .grf file.
	 * @return true if the archive was opened and parsed successfully.
	 */
	bool Open(const FString& FilePath);

	/** Close the archive and free resources. */
	void Close();

	/** Whether an archive is currently open. */
	bool IsOpen() const { return FileHandle != nullptr; }

	/** Get the number of file entries in the archive. */
	int32 GetFileCount() const { return FileEntries.Num(); }

	/** Get all file entry names. */
	TArray<FString> GetFileList() const;

	/**
	 * Get file entries matching a path pattern (e.g., "data\\sprite\\*").
	 * @param Pattern Wildcard pattern to match against file paths.
	 */
	TArray<FString> FindFiles(const FString& Pattern) const;

	/**
	 * Check if a file exists in the archive.
	 * @param InternalPath The internal GRF path (e.g., "data\\sprite\\npc\\merchant.spr").
	 */
	bool FileExists(const FString& InternalPath) const;

	/**
	 * Extract a file's raw bytes from the archive.
	 * @param InternalPath The internal GRF path.
	 * @param OutData The decompressed file data.
	 * @return true if extraction succeeded.
	 */
	bool ExtractFile(const FString& InternalPath, TArray<uint8>& OutData) const;

	/**
	 * Get the entry metadata for a file.
	 * @param InternalPath The internal GRF path.
	 * @return Pointer to the entry, or nullptr if not found.
	 */
	const FROGRFEntry* GetEntry(const FString& InternalPath) const;

private:
	static constexpr int32 GRF_HEADER_SIZE = 46;
	static constexpr const char* GRF_SIGNATURE = "Master of Magic";

	/** Parse the file table from the GRF header. */
	bool ParseFileTable();

	/** Decompress zlib-compressed data. */
	static bool DecompressZlib(const TArray<uint8>& CompressedData, TArray<uint8>& OutData, uint32 UncompressedSize);

	/** File handle to the open GRF archive. */
	IFileHandle* FileHandle = nullptr;

	/** Path to the currently open GRF file. */
	FString OpenFilePath;

	/** Version of the GRF archive. */
	uint32 Version = 0;

	/** All file entries indexed by internal path. */
	TMap<FString, FROGRFEntry> FileEntries;
};
