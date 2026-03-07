// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROConsumableData.h"

UROConsumableData::UROConsumableData()
	: HPRestore(0)
	, SPRestore(0)
	, HPRestorePercent(0.0f)
	, SPRestorePercent(0.0f)
	, BuffDuration(0.0f)
{
	ItemType = EROItemType::Consumable;
	bStackable = true;
	MaxStack = 30000;
}
