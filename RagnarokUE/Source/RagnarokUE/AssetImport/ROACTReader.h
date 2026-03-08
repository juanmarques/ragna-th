// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * A single sprite layer within an animation frame.
 */
struct FROACTLayer
{
	int32 OffsetX = 0;
	int32 OffsetY = 0;

	/** Index into the SPR file's frame list. */
	int32 SpriteIndex = 0;

	/** Bitfield: 0x01 = horizontal mirror. */
	uint32 Flags = 0;

	/** Tint color (RGBA). */
	FColor Color = FColor::White;

	/** Scale factors. */
	float ScaleX = 1.0f;
	float ScaleY = 1.0f;

	/** Rotation in degrees. */
	int32 Rotation = 0;

	/** Sprite type: 0 = palette frame, 1 = RGBA frame. */
	int32 SpriteType = 0;

	bool IsMirrored() const { return (Flags & 0x01) != 0; }
};

/**
 * A single animation frame containing one or more layers.
 */
struct FROACTFrame
{
	/** Unused range data (kept for compatibility). */
	FVector2D Range1Min = FVector2D::ZeroVector;
	FVector2D Range1Max = FVector2D::ZeroVector;
	FVector2D Range2Min = FVector2D::ZeroVector;
	FVector2D Range2Max = FVector2D::ZeroVector;

	/** All layers in this frame, drawn in order. */
	TArray<FROACTLayer> Layers;

	/** Event ID (-1 = none). Used for sound triggers etc. */
	int32 EventID = -1;

	/** Anchor points for attachment positioning. */
	TArray<FVector2D> AnchorPoints;
};

/**
 * An action (animation sequence) containing multiple frames.
 */
struct FROACTAction
{
	TArray<FROACTFrame> Frames;
};

/**
 * FROACTReader
 * Reads Ragnarok Online ACT animation files.
 * ACT files define how SPR sprite frames are composited and animated.
 * Supports versions 2.0 through 2.5.
 */
class RAGNAROKUE_API FROACTReader
{
public:
	/**
	 * Parse an ACT file from raw byte data.
	 * @param Data Raw ACT file bytes.
	 * @return true if parsing succeeded.
	 */
	bool Parse(const TArray<uint8>& Data);

	/** Get the number of actions (animation sequences). */
	int32 GetActionCount() const { return Actions.Num(); }

	/**
	 * Get an action by index.
	 * @param Index Action index (0-based).
	 * @return Pointer to the action, or nullptr if invalid.
	 */
	const FROACTAction* GetAction(int32 Index) const;

	/** Get the ACT format version. */
	uint16 GetVersion() const { return Version; }

	/** Get all event sound file references. */
	const TArray<FString>& GetSoundFiles() const { return SoundFiles; }

	/** Animation interval in milliseconds (v2.2+, default 150ms). */
	float GetAnimationInterval() const { return AnimationInterval; }

private:
	uint16 Version = 0;
	TArray<FROACTAction> Actions;
	TArray<FString> SoundFiles;
	float AnimationInterval = 150.0f;
};
