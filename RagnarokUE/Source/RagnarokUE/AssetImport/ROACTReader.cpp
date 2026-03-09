// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROACTReader.h"

namespace ACTReaderInternal
{
	template<typename T>
	static bool SafeRead(const uint8*& Cursor, const uint8* End, T& OutValue)
	{
		if (Cursor + sizeof(T) > End)
		{
			return false;
		}
		OutValue = *(reinterpret_cast<const T*>(Cursor));
		Cursor += sizeof(T);
		return true;
	}

	bool SkipBytes(const uint8*& Cursor, const uint8* End, int32 Count)
	{
		if (Cursor + Count > End)
		{
			return false;
		}
		Cursor += Count;
		return true;
	}
}
using namespace ACTReaderInternal;

bool FROACTReader::Parse(const TArray<uint8>& Data)
{
	Actions.Empty();
	SoundFiles.Empty();
	AnimationInterval = 150.0f;

	if (Data.Num() < 4)
	{
		UE_LOG(LogTemp, Error, TEXT("ROACTReader: Data too small for ACT header"));
		return false;
	}

	const uint8* Cursor = Data.GetData();
	const uint8* End = Cursor + Data.Num();

	// Validate magic: "AC"
	if (Cursor[0] != 'A' || Cursor[1] != 'C')
	{
		UE_LOG(LogTemp, Error, TEXT("ROACTReader: Invalid ACT magic"));
		return false;
	}
	Cursor += 2;

	if (!SafeRead(Cursor, End, Version))
	{
		return false;
	}

	uint16 ActionCount = 0;
	if (!SafeRead(Cursor, End, ActionCount))
	{
		return false;
	}

	// v2.0+: 10 bytes reserved
	if (Version >= 0x0200)
	{
		if (!SkipBytes(Cursor, End, 10))
		{
			return false;
		}
	}

	// Parse each action
	Actions.SetNum(ActionCount);
	for (int32 a = 0; a < ActionCount; ++a)
	{
		uint32 FrameCount = 0;
		if (!SafeRead(Cursor, End, FrameCount))
		{
			return false;
		}

		Actions[a].Frames.SetNum(FrameCount);

		for (uint32 f = 0; f < FrameCount; ++f)
		{
			FROACTFrame& Frame = Actions[a].Frames[f];

			// Range data (unused but must be read)
			// v2.0+: Range1 = 4 int32s, Range2 doesn't exist in all versions
			if (Version < 0x0200)
			{
				// v1.x: no range data
			}
			else
			{
				int32 RangeX1, RangeY1, RangeX2, RangeY2;
				// Skip 32 bytes of range/unused data
				if (!SkipBytes(Cursor, End, 32))
				{
					return false;
				}
			}

			// Layer count
			uint32 LayerCount = 0;
			if (!SafeRead(Cursor, End, LayerCount))
			{
				return false;
			}

			Frame.Layers.SetNum(LayerCount);

			for (uint32 l = 0; l < LayerCount; ++l)
			{
				FROACTLayer& Layer = Frame.Layers[l];

				int32 X, Y;
				if (!SafeRead(Cursor, End, X) || !SafeRead(Cursor, End, Y))
				{
					return false;
				}
				Layer.OffsetX = X;
				Layer.OffsetY = Y;

				int32 SprIdx;
				if (!SafeRead(Cursor, End, SprIdx))
				{
					return false;
				}
				Layer.SpriteIndex = SprIdx;

				uint32 Flags;
				if (!SafeRead(Cursor, End, Flags))
				{
					return false;
				}
				Layer.Flags = Flags;

				// v2.0+: color
				if (Version >= 0x0200)
				{
					uint8 R, G, B, A;
					if (!SafeRead(Cursor, End, R) || !SafeRead(Cursor, End, G) ||
						!SafeRead(Cursor, End, B) || !SafeRead(Cursor, End, A))
					{
						return false;
					}
					Layer.Color = FColor(R, G, B, A);
				}

				// v2.0+: scale
				if (Version >= 0x0200)
				{
					float ScaleX = 1.0f;
					if (Version <= 0x0203)
					{
						// Older versions: single float scale
						if (!SafeRead(Cursor, End, ScaleX))
						{
							return false;
						}
						Layer.ScaleX = ScaleX;
						Layer.ScaleY = ScaleX;
					}
					else
					{
						// v2.4+: separate X/Y scale
						float ScaleY = 1.0f;
						if (!SafeRead(Cursor, End, ScaleX) || !SafeRead(Cursor, End, ScaleY))
						{
							return false;
						}
						Layer.ScaleX = ScaleX;
						Layer.ScaleY = ScaleY;
					}
				}

				// v2.0+: rotation
				if (Version >= 0x0200)
				{
					int32 Rot;
					if (!SafeRead(Cursor, End, Rot))
					{
						return false;
					}
					Layer.Rotation = Rot;
				}

				// v2.0+: sprite type
				if (Version >= 0x0200)
				{
					int32 SprType;
					if (!SafeRead(Cursor, End, SprType))
					{
						return false;
					}
					Layer.SpriteType = SprType;
				}

				// v2.5+: additional layer data (width/height override)
				if (Version >= 0x0205)
				{
					int32 W, H;
					if (!SafeRead(Cursor, End, W) || !SafeRead(Cursor, End, H))
					{
						return false;
					}
				}
			}

			// v2.0+: event ID
			if (Version >= 0x0200)
			{
				int32 EventID;
				if (!SafeRead(Cursor, End, EventID))
				{
					return false;
				}
				Frame.EventID = EventID;
			}

			// v2.3+: anchor points
			if (Version >= 0x0203)
			{
				uint32 AnchorCount = 0;
				if (!SafeRead(Cursor, End, AnchorCount))
				{
					return false;
				}

				Frame.AnchorPoints.SetNum(AnchorCount);
				for (uint32 ap = 0; ap < AnchorCount; ++ap)
				{
					// Skip unknown 4 bytes, then read X, Y, skip attr (4 bytes)
					if (!SkipBytes(Cursor, End, 4))
					{
						return false;
					}
					int32 AX, AY;
					if (!SafeRead(Cursor, End, AX) || !SafeRead(Cursor, End, AY))
					{
						return false;
					}
					Frame.AnchorPoints[ap] = FVector2D(AX, AY);
					if (!SkipBytes(Cursor, End, 4))
					{
						return false;
					}
				}
			}
		}
	}

	// v2.1+: sound file table
	if (Version >= 0x0201 && Cursor < End)
	{
		uint32 SoundCount = 0;
		if (SafeRead(Cursor, End, SoundCount))
		{
			SoundFiles.SetNum(SoundCount);
			for (uint32 s = 0; s < SoundCount; ++s)
			{
				// Each sound file name is 40 bytes, null-padded
				if (Cursor + 40 > End)
				{
					break;
				}
				char NameBuf[41];
				FMemory::Memcpy(NameBuf, Cursor, 40);
				NameBuf[40] = '\0';
				Cursor += 40;
				SoundFiles[s] = FString(ANSI_TO_TCHAR(NameBuf));
			}
		}
	}

	// v2.2+: animation interval
	if (Version >= 0x0202 && Cursor + 4 <= End)
	{
		float Interval;
		if (SafeRead(Cursor, End, Interval))
		{
			AnimationInterval = (Interval > 0.0f) ? Interval : 150.0f;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ROACTReader: Parsed %d actions (v%d.%d), %d sounds"),
		Actions.Num(), static_cast<int32>(Version >> 8), static_cast<int32>(Version & 0xFF), SoundFiles.Num());
	return true;
}

const FROACTAction* FROACTReader::GetAction(int32 Index) const
{
	if (Index >= 0 && Index < Actions.Num())
	{
		return &Actions[Index];
	}
	return nullptr;
}
