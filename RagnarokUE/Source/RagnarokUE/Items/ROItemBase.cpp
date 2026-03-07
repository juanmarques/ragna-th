// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROItemBase.h"

UROItemBase::UROItemBase()
	: ItemID(0)
	, ItemName(NAME_None)
	, DisplayName(FText::GetEmpty())
	, Description(FText::GetEmpty())
	, Weight(0.0f)
	, ItemType(EROItemType::EtcItem)
	, BuyPrice(0)
	, SellPrice(0)
	, bStackable(true)
	, MaxStack(30000)
	, RequiredLevel(0)
{
}

FPrimaryAssetId UROItemBase::GetPrimaryAssetId() const
{
	// Use the item type name as the asset type and ItemName as the asset name
	FString TypeString = StaticEnum<EROItemType>()->GetNameStringByValue(static_cast<int64>(ItemType));
	return FPrimaryAssetId(FPrimaryAssetType(*TypeString), GetFName());
}
