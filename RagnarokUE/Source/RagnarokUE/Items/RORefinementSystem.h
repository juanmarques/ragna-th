// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "RORefinementSystem.generated.h"

class UROInventoryComponent;
class UROWeaponData;

/**
 * URORefinementSystem
 * Static utility class implementing the Ragnarok Online refinement (upgrade) system.
 *
 * Refinement rules:
 * - Weapons and armor can be refined from +0 to +10 (pre-renewal max).
 * - Success rate decreases at higher refine levels.
 * - Safe refine limits: Lv1 weapon=+7, Lv2=+6, Lv3=+5, Lv4=+4, Armor=+4.
 * - Below safe limit, success is 100%.
 * - Failed refinement destroys the equipment (standard RO behavior).
 * - Each weapon level requires a specific ore item.
 */
UCLASS()
class RAGNAROKUE_API URORefinementSystem : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Get the safe refine limit for a weapon level.
	 * Refinement at or below this level has 100% success rate.
	 * @param WeaponLevel 0 for armor, 1-4 for weapons.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Refinement")
	static int32 GetSafeRefineLimit(int32 WeaponLevel);

	/**
	 * Get the success rate for refining from CurrentRefine to CurrentRefine+1.
	 * @param CurrentRefine The current refine level.
	 * @param WeaponLevel 0 for armor, 1-4 for weapons.
	 * @return Success rate as a percentage (0.0 - 100.0).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Refinement")
	static float GetRefineSuccessRate(int32 CurrentRefine, int32 WeaponLevel);

	/**
	 * Get the ATK/DEF bonus for a given refine level and weapon level.
	 * @param RefineLevel The refine level (+0 to +10).
	 * @param WeaponLevel 0 for armor (DEF bonus), 1-4 for weapons (ATK bonus).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Refinement")
	static int32 GetRefineBonus(int32 RefineLevel, int32 WeaponLevel);

	/**
	 * Get the required ore item ID for refining a weapon of the given level.
	 * @param WeaponLevel 0 for armor, 1-4 for weapons.
	 * @return Item ID of the required ore. Standard RO ore IDs:
	 *         984 = Oridecon (Lv3-4 weapons), 985 = Elunium (armor),
	 *         1010 = Phracon (Lv1 weapons), 1011 = Emveretarcon (Lv2 weapons).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Refinement")
	static int32 GetRequiredOre(int32 WeaponLevel);

	/**
	 * Get the Zeny cost for refining at the given refine level.
	 * @param CurrentRefine The current refine level before the attempt.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Refinement")
	static int32 GetRefineCost(int32 CurrentRefine);

	/**
	 * Attempt to refine an item. Checks ore availability, zeny cost,
	 * calculates success/failure, and applies the result.
	 * On success: increments refine level.
	 * On failure: destroys the equipment (sets ItemID to 0).
	 * @param Inventory The player's inventory component (for ore/zeny checks and item access).
	 * @param ItemSlotIndex The inventory slot index of the item to refine.
	 * @param WeaponLevel The weapon level (0 for armor, 1-4 for weapons).
	 * @return True if refinement succeeded, false if it failed or couldn't be attempted.
	 */
	UFUNCTION(BlueprintCallable, Category = "Refinement")
	static bool AttemptRefine(UROInventoryComponent* Inventory, int32 ItemSlotIndex, int32 WeaponLevel);

private:
	/** Maximum standard refine level. */
	static constexpr int32 MAX_REFINE = 10;

	/**
	 * Internal success rate tables.
	 * Index = refine level being attempted (the level we're going TO).
	 * Values are percentages.
	 */
	static float GetArmorRefineRate(int32 TargetRefine);
	static float GetWeaponRefineRate(int32 TargetRefine, int32 WeaponLevel);
};
