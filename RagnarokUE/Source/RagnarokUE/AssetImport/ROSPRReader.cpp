// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROSPRReader.h"

namespace
{
	template<typename T>
	bool SafeRead(const uint8*& Cursor, const uint8* End, T& OutValue)
	{
		if (Cursor + sizeof(T) > End)
		{
			return false;
		}
		OutValue = *(reinterpret_cast<const T*>(Cursor));
		Cursor += sizeof(T);
		return true;
	}
}

bool FROSPRReader::Parse(const TArray<uint8>& Data)
{
	Frames.Empty();
	Palette.Empty();
	PaletteFrameCount = 0;
	RGBAFrameCount = 0;

	if (Data.Num() < 4)
	{
		UE_LOG(LogTemp, Error, TEXT("ROSPRReader: Data too small for SPR header"));
		return false;
	}

	const uint8* Cursor = Data.GetData();
	const uint8* End = Cursor + Data.Num();

	// Validate magic: "SP"
	if (Cursor[0] != 'S' || Cursor[1] != 'P')
	{
		UE_LOG(LogTemp, Error, TEXT("ROSPRReader: Invalid SPR magic"));
		return false;
	}
	Cursor += 2;

	// Read version
	if (!SafeRead(Cursor, End, Version))
	{
		return false;
	}

	uint16 RawPaletteCount = 0;
	if (!SafeRead(Cursor, End, RawPaletteCount))
	{
		return false;
	}
	PaletteFrameCount = RawPaletteCount;

	// RGBA frames only exist in v2.0+
	uint16 RawRGBACount = 0;
	if (Version >= 0x0200)
	{
		if (!SafeRead(Cursor, End, RawRGBACount))
		{
			return false;
		}
	}
	RGBAFrameCount = RawRGBACount;

	// Read palette from end of file (256 colors x 4 bytes)
	if (Data.Num() >= 1024)
	{
		const uint8* PaletteData = Data.GetData() + Data.Num() - 1024;
		Palette.SetNum(256);
		for (int32 i = 0; i < 256; ++i)
		{
			// Palette stored as RGBA in the file
			Palette[i] = FColor(
				PaletteData[i * 4 + 0],  // R
				PaletteData[i * 4 + 1],  // G
				PaletteData[i * 4 + 2],  // B
				(i == 0) ? 0 : 255       // Index 0 is transparent
			);
		}
	}

	// Decode palette frames
	bool bUseRLE = (Version >= 0x0201);
	for (int32 i = 0; i < PaletteFrameCount; ++i)
	{
		if (bUseRLE)
		{
			if (!DecodeRLEPaletteFrame(Cursor, End))
			{
				UE_LOG(LogTemp, Error, TEXT("ROSPRReader: Failed to decode RLE palette frame %d"), i);
				return false;
			}
		}
		else
		{
			if (!DecodePaletteFrame(Cursor, End))
			{
				UE_LOG(LogTemp, Error, TEXT("ROSPRReader: Failed to decode palette frame %d"), i);
				return false;
			}
		}
	}

	// Decode RGBA frames
	for (int32 i = 0; i < RGBAFrameCount; ++i)
	{
		if (!DecodeRGBAFrame(Cursor, End))
		{
			UE_LOG(LogTemp, Error, TEXT("ROSPRReader: Failed to decode RGBA frame %d"), i);
			return false;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ROSPRReader: Parsed %d palette frames + %d RGBA frames (v%d.%d)"),
		PaletteFrameCount, RGBAFrameCount, Version >> 8, Version & 0xFF);
	return true;
}

const FROSPRFrame* FROSPRReader::GetFrame(int32 Index) const
{
	if (Index >= 0 && Index < Frames.Num())
	{
		return &Frames[Index];
	}
	return nullptr;
}

bool FROSPRReader::DecodePaletteFrame(const uint8*& Cursor, const uint8* End)
{
	uint16 Width = 0, Height = 0;
	if (!SafeRead(Cursor, End, Width) || !SafeRead(Cursor, End, Height))
	{
		return false;
	}

	int32 PixelCount = Width * Height;
	if (Cursor + PixelCount > End)
	{
		return false;
	}

	// Read palette indices
	TArray<uint8> IndexedPixels;
	IndexedPixels.SetNum(PixelCount);
	FMemory::Memcpy(IndexedPixels.GetData(), Cursor, PixelCount);
	Cursor += PixelCount;

	// Convert to RGBA
	FROSPRFrame Frame;
	Frame.Width = Width;
	Frame.Height = Height;
	ConvertPaletteToRGBA(Frame, IndexedPixels);
	Frames.Add(MoveTemp(Frame));

	return true;
}

bool FROSPRReader::DecodeRLEPaletteFrame(const uint8*& Cursor, const uint8* End)
{
	uint16 Width = 0, Height = 0;
	if (!SafeRead(Cursor, End, Width) || !SafeRead(Cursor, End, Height))
	{
		return false;
	}

	// RLE encoded data size
	uint16 EncodedSize = 0;
	if (!SafeRead(Cursor, End, EncodedSize))
	{
		return false;
	}

	if (Cursor + EncodedSize > End)
	{
		return false;
	}

	const uint8* RLEEnd = Cursor + EncodedSize;
	int32 PixelCount = Width * Height;

	// Decode RLE: byte 0x00 followed by count = transparent run; otherwise literal palette index
	TArray<uint8> IndexedPixels;
	IndexedPixels.SetNumZeroed(PixelCount);
	int32 WritePos = 0;

	while (Cursor < RLEEnd && WritePos < PixelCount)
	{
		uint8 Byte = *Cursor++;
		if (Byte == 0)
		{
			if (Cursor >= RLEEnd)
			{
				break;
			}
			uint8 RunLength = *Cursor++;
			// Index 0 = transparent, already zeroed
			WritePos += RunLength;
		}
		else
		{
			IndexedPixels[WritePos++] = Byte;
		}
	}

	// Ensure cursor is at the end of the encoded block
	Cursor = RLEEnd;

	FROSPRFrame Frame;
	Frame.Width = Width;
	Frame.Height = Height;
	ConvertPaletteToRGBA(Frame, IndexedPixels);
	Frames.Add(MoveTemp(Frame));

	return true;
}

bool FROSPRReader::DecodeRGBAFrame(const uint8*& Cursor, const uint8* End)
{
	uint16 Width = 0, Height = 0;
	if (!SafeRead(Cursor, End, Width) || !SafeRead(Cursor, End, Height))
	{
		return false;
	}

	int32 DataSize = Width * Height * 4;
	if (Cursor + DataSize > End)
	{
		return false;
	}

	FROSPRFrame Frame;
	Frame.Width = Width;
	Frame.Height = Height;
	Frame.PixelData.SetNum(DataSize);

	// SPR RGBA is stored as ABGR, convert to RGBA
	for (int32 i = 0; i < Width * Height; ++i)
	{
		Frame.PixelData[i * 4 + 0] = Cursor[i * 4 + 3]; // R (from A position)
		Frame.PixelData[i * 4 + 1] = Cursor[i * 4 + 2]; // G (from B position)
		Frame.PixelData[i * 4 + 2] = Cursor[i * 4 + 1]; // B (from G position)
		Frame.PixelData[i * 4 + 3] = Cursor[i * 4 + 0]; // A (from R position)
	}
	Cursor += DataSize;

	Frames.Add(MoveTemp(Frame));
	return true;
}

void FROSPRReader::ConvertPaletteToRGBA(FROSPRFrame& Frame, const TArray<uint8>& IndexedPixels) const
{
	int32 PixelCount = Frame.Width * Frame.Height;
	Frame.PixelData.SetNum(PixelCount * 4);

	for (int32 i = 0; i < PixelCount; ++i)
	{
		uint8 PaletteIndex = IndexedPixels[i];
		if (PaletteIndex < Palette.Num())
		{
			const FColor& Color = Palette[PaletteIndex];
			Frame.PixelData[i * 4 + 0] = Color.R;
			Frame.PixelData[i * 4 + 1] = Color.G;
			Frame.PixelData[i * 4 + 2] = Color.B;
			Frame.PixelData[i * 4 + 3] = Color.A;
		}
		else
		{
			// Transparent fallback
			Frame.PixelData[i * 4 + 0] = 0;
			Frame.PixelData[i * 4 + 1] = 0;
			Frame.PixelData[i * 4 + 2] = 0;
			Frame.PixelData[i * 4 + 3] = 0;
		}
	}
}
