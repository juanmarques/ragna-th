// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROCardData.h"

UROCardData::UROCardData()
	: TargetSlotType(EROEquipSlot::Weapon)
	, ElementGrant(EROElement::Neutral)
	, RaceBonusTarget(EROMonsterRace::Formless)
	, RaceBonusPercent(0.0f)
{
	ItemType = EROItemType::Card;
	bStackable = true;
	MaxStack = 100;
	Weight = 1.0f;
}

FString UROCardData::GetCompoundNamePart() const
{
	if (!CompoundPrefix.IsEmpty())
	{
		return CompoundPrefix;
	}
	return CompoundSuffix;
}
