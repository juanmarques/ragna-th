// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * A single frame (image) within an SPR sprite file.
 */
struct FROSPRFrame
{
	uint16 Width = 0;
	uint16 Height = 0;

	/** RGBA pixel data (Width * Height * 4 bytes). Always converted to RGBA regardless of source format. */
	TArray<uint8> PixelData;

	bool IsValid() const { return Width > 0 && Height > 0 && PixelData.Num() == Width * Height * 4; }
};

/**
 * FROSPRReader
 * Reads Ragnarok Online SPR sprite files.
 * Supports versions 1.0, 1.1, 2.0, and 2.1.
 *
 * SPR files contain palette-indexed frames (v1.0+) and optional RGBA frames (v2.0+).
 * v2.1 adds RLE compression for palette frames.
 * A 256-color palette is stored at the end of the file.
 */
class RAGNAROKUE_API FROSPRReader
{
public:
	/**
	 * Parse an SPR file from raw byte data.
	 * @param Data Raw SPR file bytes (e.g., extracted from a GRF archive).
	 * @return true if parsing succeeded.
	 */
	bool Parse(const TArray<uint8>& Data);

	/** Get the total number of frames (palette + RGBA). */
	int32 GetFrameCount() const { return Frames.Num(); }

	/** Get the number of palette-indexed frames. */
	int32 GetPaletteFrameCount() const { return PaletteFrameCount; }

	/** Get the number of RGBA frames. */
	int32 GetRGBAFrameCount() const { return RGBAFrameCount; }

	/**
	 * Get a frame by index. Palette frames come first, then RGBA frames.
	 * @param Index Frame index (0-based).
	 * @return Pointer to the frame, or nullptr if index is invalid.
	 */
	const FROSPRFrame* GetFrame(int32 Index) const;

	/** Get the 256-color palette (256 entries x 4 bytes RGBA). */
	const TArray<FColor>& GetPalette() const { return Palette; }

	/** Get the SPR format version (e.g., 0x0201 for v2.1). */
	uint16 GetVersion() const { return Version; }

private:
	/** Decode a palette-indexed frame (raw pixel indices). */
	bool DecodePaletteFrame(const uint8*& Cursor, const uint8* End);

	/** Decode an RLE-compressed palette frame (v2.1+). */
	bool DecodeRLEPaletteFrame(const uint8*& Cursor, const uint8* End);

	/** Decode an RGBA frame. */
	bool DecodeRGBAFrame(const uint8*& Cursor, const uint8* End);

	/** Convert a palette-indexed frame to RGBA using the loaded palette. */
	void ConvertPaletteToRGBA(FROSPRFrame& Frame, const TArray<uint8>& IndexedPixels) const;

	uint16 Version = 0;
	int32 PaletteFrameCount = 0;
	int32 RGBAFrameCount = 0;
	TArray<FROSPRFrame> Frames;
	TArray<FColor> Palette;
};
