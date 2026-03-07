// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ROItemBase.h"
#include "ROArmorData.generated.h"

/**
 * UROArmorData
 * Data asset for armor/equipment items (non-weapon equippables).
 */
UCLASS(BlueprintType)
class RAGNAROKUE_API UROArmorData : public UROItemBase
{
	GENERATED_BODY()

public:
	UROArmorData();

	/** Which equipment slot this armor occupies. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armor")
	EROEquipSlot EquipSlot;

	/** Physical defense bonus. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armor")
	int32 DEF;

	/** Number of card slots (0-1 for most armor, accessories can have 1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armor", meta = (ClampMin = "0", ClampMax = "1"))
	int32 CardSlots;

	/** Magical defense bonus. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armor")
	int32 MDEF;
};
