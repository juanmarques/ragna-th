// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ROItemBase.h"
#include "ROWeaponData.generated.h"

/**
 * UROWeaponData
 * Data asset for weapon items. Extends UROItemBase with weapon-specific stats.
 */
UCLASS(BlueprintType)
class RAGNAROKUE_API UROWeaponData : public UROItemBase
{
	GENERATED_BODY()

public:
	UROWeaponData();

	/** Weapon sub-type (Sword, Dagger, Bow, etc.). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EROWeaponType WeaponType;

	/** Base ATK value of the weapon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	int32 ATK;

	/** Weapon level (1-4). Higher level weapons get more refine bonus but break more easily. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "1", ClampMax = "4"))
	int32 WeaponLevel;

	/** Number of card slots (0-4). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "0", ClampMax = "4"))
	int32 CardSlots;

	/** Innate element of the weapon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EROElement Element;

	/** Attack range in cells. Melee weapons typically have 1. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "1"))
	int32 Range;

	/** ASPD penalty applied when equipping this weapon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	int32 ASPD_Penalty;

	/**
	 * Get the ATK bonus per refine level based on weapon level.
	 * Lv1: +2, Lv2: +3, Lv3: +5, Lv4: +7 per refine.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
	int32 GetRefineBonusPerLevel() const;

	/**
	 * Get the total refine ATK bonus for a given refine level.
	 * @param RefineLevel The current refine level of the weapon.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
	int32 GetTotalRefineBonus(int32 RefineLevel) const;
};
