// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROCardSystem.h"
#include "ROItemBase.h"
#include "ROWeaponData.h"
#include "ROArmorData.h"
#include "ROCardData.h"
#include "ROItemDatabase.h"

bool UROCardSystem::InsertCard(FROItemInstance& Equipment, int32 CardID, int32 SlotIndex, UROItemDatabase* ItemDatabase)
{
	if (CardID <= 0 || SlotIndex < 0)
	{
		return false;
	}

	// Ensure the card slot array is large enough
	if (SlotIndex >= Equipment.CardSlots.Num())
	{
		return false;
	}

	// Check if the slot is already occupied
	if (Equipment.CardSlots[SlotIndex] != 0)
	{
		return false;
	}

	// Validate card type matches equipment type (weapon card in weapon, armor card in matching slot)
	if (ItemDatabase)
	{
		const UROItemBase* EquipData = ItemDatabase->GetItemData(Equipment.ItemID);
		const UROCardData* CardData = ItemDatabase->GetCardData(CardID);
		if (EquipData && CardData && !ValidateCardSlot(EquipData, CardData))
		{
			return false;
		}
	}

	Equipment.CardSlots[SlotIndex] = CardID;
	return true;
}

int32 UROCardSystem::RemoveCard(FROItemInstance& Equipment, int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= Equipment.CardSlots.Num())
	{
		return 0;
	}

	int32 RemovedCardID = Equipment.CardSlots[SlotIndex];
	Equipment.CardSlots[SlotIndex] = 0;
	return RemovedCardID;
}

TArray<UROCardData*> UROCardSystem::GetCardEffects(const FROItemInstance& Equipment, UROItemDatabase* ItemDatabase)
{
	TArray<UROCardData*> Result;

	if (!ItemDatabase)
	{
		return Result;
	}

	for (int32 CardID : Equipment.CardSlots)
	{
		if (CardID > 0)
		{
			UROCardData* CardData = ItemDatabase->GetCardData(CardID);
			if (CardData)
			{
				Result.Add(CardData);
			}
		}
	}

	return Result;
}

bool UROCardSystem::ValidateCardSlot(const UROItemBase* Equipment, const UROCardData* Card)
{
	if (!Equipment || !Card)
	{
		return false;
	}

	// Weapon cards go in weapons
	if (Card->TargetSlotType == EROEquipSlot::Weapon)
	{
		return Equipment->ItemType == EROItemType::Weapon;
	}

	// For armor cards, check the specific slot type
	if (Equipment->ItemType == EROItemType::Armor)
	{
		const UROArmorData* ArmorData = Cast<UROArmorData>(Equipment);
		if (ArmorData)
		{
			// Accessory cards can go in either accessory slot
			if (Card->TargetSlotType == EROEquipSlot::AccessoryL ||
				Card->TargetSlotType == EROEquipSlot::AccessoryR)
			{
				return ArmorData->EquipSlot == EROEquipSlot::AccessoryL ||
					   ArmorData->EquipSlot == EROEquipSlot::AccessoryR;
			}
			return ArmorData->EquipSlot == Card->TargetSlotType;
		}
	}

	return false;
}

FString UROCardSystem::GetCompoundName(const UROItemBase* BaseItem, const TArray<UROCardData*>& Cards)
{
	if (!BaseItem)
	{
		return TEXT("");
	}

	FString BaseName = BaseItem->DisplayName.ToString();

	if (Cards.Num() == 0)
	{
		return BaseName;
	}

	// Count occurrences of each card for multiplier prefixes
	TMap<int32, int32> CardCounts; // CardID -> Count
	TMap<int32, const UROCardData*> CardDataMap;

	for (const UROCardData* Card : Cards)
	{
		if (Card)
		{
			int32& Count = CardCounts.FindOrAdd(Card->ItemID);
			Count++;
			CardDataMap.Add(Card->ItemID, Card);
		}
	}

	// Build prefix and suffix strings
	FString Prefixes;
	FString Suffixes;

	for (const auto& Pair : CardCounts)
	{
		const UROCardData* Card = CardDataMap[Pair.Key];
		if (!Card)
		{
			continue;
		}

		FString Multiplier = GetMultiplierPrefix(Pair.Value);

		if (!Card->CompoundPrefix.IsEmpty())
		{
			if (!Prefixes.IsEmpty())
			{
				Prefixes += TEXT(" ");
			}
			Prefixes += Multiplier + Card->CompoundPrefix;
		}
		else if (!Card->CompoundSuffix.IsEmpty())
		{
			if (!Suffixes.IsEmpty())
			{
				Suffixes += TEXT(" ");
			}
			Suffixes += Multiplier + Card->CompoundSuffix;
		}
	}

	// Combine: "Prefix BaseItem Suffix"
	FString CompoundName;
	if (!Prefixes.IsEmpty())
	{
		CompoundName = Prefixes + TEXT(" ");
	}
	CompoundName += BaseName;
	if (!Suffixes.IsEmpty())
	{
		CompoundName += TEXT(" ") + Suffixes;
	}

	return CompoundName;
}

FString UROCardSystem::GetMultiplierPrefix(int32 Count)
{
	// RO uses these prefixes for multiple of the same card
	switch (Count)
	{
	case 1: return TEXT("");
	case 2: return TEXT("Double ");
	case 3: return TEXT("Triple ");
	case 4: return TEXT("Quadruple ");
	default: return TEXT("");
	}
}
