// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROItemBase.generated.h"

/**
 * UROItemBase
 * Base data asset for all Ragnarok Online items.
 * Stores static item data loaded from data tables.
 */
UCLASS(BlueprintType)
class RAGNAROKUE_API UROItemBase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UROItemBase();

	/** Unique numeric item identifier matching the RO item database. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	int32 ItemID;

	/** Internal name used for lookups. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName ItemName;

	/** Localized display name shown to the player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	/** Localized item description. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (MultiLine = true))
	FText Description;

	/** Icon texture for inventory UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Weight of a single unit (RO weight units, 1 = 0.1 actual weight). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	float Weight;

	/** Primary item type category. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EROItemType ItemType;

	/** NPC buy price in Zeny. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	int32 BuyPrice;

	/** NPC sell price in Zeny (typically BuyPrice / 2). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	int32 SellPrice;

	/** Whether this item can stack in inventory. Equipment cannot stack. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bStackable;

	/** Maximum stack size if stackable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (EditCondition = "bStackable", ClampMin = "1"))
	int32 MaxStack;

	/** Minimum base level required to use/equip this item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	int32 RequiredLevel;

	// --- UPrimaryDataAsset Interface ---
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
