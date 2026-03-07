// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWeaponData.h"

UROWeaponData::UROWeaponData()
	: WeaponType(EROWeaponType::Sword)
	, ATK(0)
	, WeaponLevel(1)
	, CardSlots(0)
	, Element(EROElement::Neutral)
	, Range(1)
	, ASPD_Penalty(0)
{
	ItemType = EROItemType::Weapon;
	bStackable = false;
	MaxStack = 1;
}

int32 UROWeaponData::GetRefineBonusPerLevel() const
{
	switch (WeaponLevel)
	{
	case 1: return 2;
	case 2: return 3;
	case 3: return 5;
	case 4: return 7;
	default: return 2;
	}
}

int32 UROWeaponData::GetTotalRefineBonus(int32 RefineLevel) const
{
	return RefineLevel * GetRefineBonusPerLevel();
}
