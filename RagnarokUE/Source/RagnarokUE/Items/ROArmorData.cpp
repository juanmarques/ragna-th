// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROArmorData.h"

UROArmorData::UROArmorData()
	: EquipSlot(EROEquipSlot::Armor)
	, DEF(0)
	, CardSlots(0)
	, MDEF(0)
{
	ItemType = EROItemType::Armor;
	bStackable = false;
	MaxStack = 1;
}
