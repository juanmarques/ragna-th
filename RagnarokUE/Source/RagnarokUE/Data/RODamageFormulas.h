// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RODamageFormulas.generated.h"

/**
 * URODamageFormulas
 * Static function library implementing Ragnarok Online pre-renewal damage formulas.
 * All formulas aim to match the original RO server calculations.
 */
UCLASS()
class RAGNAROKUE_API URODamageFormulas : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ============================
	// Offensive Stats
	// ============================

	/**
	 * Calculate base ATK from stats (weapon ATK is added separately).
	 * Formula: STR + floor(STR/10)^2 + DEX/5 + LUK/5
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Damage")
	static int32 CalculateBaseATK(int32 STR, int32 DEX, int32 LUK);

	/**
	 * Calculate base MATK range from INT.
	 * Returns FVector2D where X = min MATK, Y = max MATK.
	 * Min: INT + floor(INT/7)^2
	 * Max: INT + floor(INT/5)^2
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Damage")
	static FVector2D CalculateBaseMATK(int32 INT);

	// ============================
	// Defensive Stats
	// ============================

	/**
	 * Calculate hard DEF (equipment DEF) damage reduction.
	 * In pre-renewal: HardDEF directly reduces damage as a flat amount.
	 * Returns the DEF value to subtract from incoming damage.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Damage")
	static int32 CalculateHardDEF(int32 EquipDEF);

	/**
	 * Calculate soft DEF (VIT-based) flat damage reduction.
	 * Formula: VIT + floor(VIT/5)^2
	 * In pre-renewal, this is subtracted from damage after hard DEF reduction.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Damage")
	static int32 CalculateSoftDEF(int32 VIT);

	/**
	 * Calculate soft MDEF (INT-based) damage reduction.
	 * Formula: INT + floor(INT/5)^2 + DEX/5 + VIT/5
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Damage")
	static int32 CalculateSoftMDEF(int32 INT, int32 VIT, int32 DEX);

	// ============================
	// Accuracy & Evasion
	// ============================

	/**
	 * Calculate total HIT rate.
	 * Formula: 175 + BaseLevel + DEX + floor(LUK/3)
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Damage")
	static int32 CalculateHitRate(int32 DEX, int32 LUK, int32 BaseLevel);

	/**
	 * Calculate total FLEE rate.
	 * Formula: 100 + BaseLevel + AGI + floor(LUK/5)
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Damage")
	static int32 CalculateFleeRate(int32 AGI, int32 LUK, int32 BaseLevel);

	/**
	 * Calculate critical hit rate.
	 * Formula: LUK * 0.3 + 1
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Damage")
	static float CalculateCritRate(int32 LUK);

	// ============================
	// Attack Speed
	// ============================

	/**
	 * Get the base ASPD value for a given job class.
	 * This is the weapon-type delay used in ASPD calculations.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Damage")
	static int32 GetBaseASPDForJob(EROJobClass Job);

	/**
	 * Calculate ASPD (attacks per second scaled to 200).
	 * Formula: 200 - (BaseASPD - floor((AGI * 4 + DEX) / 5))
	 * Clamped between 0 and 190.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Damage")
	static float CalculateASPD(int32 BaseASPD, int32 AGI, int32 DEX);

	// ============================
	// HP / SP
	// ============================

	/**
	 * Calculate maximum HP based on level, VIT, and job class.
	 * Uses job-specific HP modifier tables.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Damage")
	static int32 CalculateMaxHP(int32 BaseLevel, int32 VIT, EROJobClass Job);

	/**
	 * Calculate maximum SP based on level, INT, and job class.
	 * Uses job-specific SP modifier tables.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Damage")
	static int32 CalculateMaxSP(int32 BaseLevel, int32 INT, EROJobClass Job);

	// ============================
	// Elemental System
	// ============================

	/**
	 * Get the damage multiplier from the 10x10x4 elemental effectiveness table.
	 * Returns a multiplier (e.g., 2.0 for double damage, 0.0 for immune, -0.25 for absorb 25%).
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Damage")
	static float GetElementalModifier(EROElement AtkElement, EROElement DefElement, EROElementLevel DefLevel);

	// ============================
	// Stat Point Cost
	// ============================

	/**
	 * Calculate stat point cost to raise a stat by 1 from CurrentStatValue.
	 * Formula: floor((CurrentStatValue - 1) / 10) + 2
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Damage")
	static int32 CalculateStatPointCost(int32 CurrentStatValue);

	// ============================
	// Full Damage Calculations
	// ============================

	/**
	 * Calculate final physical damage.
	 * @param BaseATK          Attacker's base ATK (stat-based)
	 * @param WeaponATK        Attacker's weapon ATK
	 * @param SkillModifier    Skill damage % modifier (100 = normal attack, 200 = double, etc.)
	 * @param TargetHardDEF    Target's equipment DEF
	 * @param TargetSoftDEF    Target's VIT-based soft DEF
	 * @param AtkElement       Element of the attack
	 * @param DefElement       Element of the defender
	 * @param DefElementLevel  Defender's element level
	 * @param SizeModifier     Size penalty modifier (1.0 = no penalty)
	 * @param RaceModifier     Racial bonus modifier (1.0 = no bonus)
	 * @param bIsCritical      Whether this is a critical hit (ignores FLEE, ignores DEF)
	 * @return Final damage value (minimum 1)
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Damage")
	static int32 CalculatePhysicalDamage(
		int32 BaseATK,
		int32 WeaponATK,
		float SkillModifier,
		int32 TargetHardDEF,
		int32 TargetSoftDEF,
		EROElement AtkElement,
		EROElement DefElement,
		EROElementLevel DefElementLevel,
		float SizeModifier,
		float RaceModifier,
		bool bIsCritical);

	/**
	 * Calculate final magical damage.
	 * @param MATK             Attacker's MATK (random between min and max)
	 * @param SkillModifier    Skill damage % modifier
	 * @param TargetMDEF_Hard  Target's equipment MDEF
	 * @param TargetMDEF_Soft  Target's INT-based soft MDEF
	 * @param AtkElement       Element of the spell
	 * @param DefElement       Element of the defender
	 * @param DefElementLevel  Defender's element level
	 * @param RaceModifier     Racial bonus modifier
	 * @return Final damage value (minimum 1)
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Damage")
	static int32 CalculateMagicalDamage(
		int32 MATK,
		float SkillModifier,
		int32 TargetMDEF_Hard,
		int32 TargetMDEF_Soft,
		EROElement AtkElement,
		EROElement DefElement,
		EROElementLevel DefElementLevel,
		float RaceModifier);

private:
	/** Get job-specific HP modifier. Higher for tanky jobs like Knight. */
	static float GetJobHPModifier(EROJobClass Job);

	/** Get job-specific SP modifier. Higher for caster jobs like Wizard. */
	static float GetJobSPModifier(EROJobClass Job);

	/** Internal: Initialize and return the elemental table. */
	static const TArray<TArray<TArray<float>>>& GetElementalTable();
};
