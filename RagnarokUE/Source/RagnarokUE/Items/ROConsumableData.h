// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ROItemBase.h"
#include "GameplayEffect.h"
#include "ROConsumableData.generated.h"

/**
 * UROConsumableData
 * Data asset for consumable items (potions, food, scrolls, etc.).
 *
 * Classic RO potion values:
 *   Red Potion    (501): HP +45
 *   Orange Potion (502): HP +105
 *   Yellow Potion (503): HP +175
 *   White Potion  (504): HP +325
 *   Blue Potion   (505): SP +60
 */
UCLASS(BlueprintType)
class RAGNAROKUE_API UROConsumableData : public UROItemBase
{
	GENERATED_BODY()

public:
	UROConsumableData();

	/** Flat HP restored on use. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
	int32 HPRestore;

	/** Flat SP restored on use. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
	int32 SPRestore;

	/** Percentage of MaxHP restored on use (0.0 - 1.0). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HPRestorePercent;

	/** Percentage of MaxSP restored on use (0.0 - 1.0). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SPRestorePercent;

	/** Duration of the buff effect in seconds (0 = instant). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (ClampMin = "0.0"))
	float BuffDuration;

	/** Optional GAS GameplayEffect applied on consumption. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
	TSubclassOf<UGameplayEffect> ConsumableEffect;
};
