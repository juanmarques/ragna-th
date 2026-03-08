// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RODamageExecution.generated.h"

/**
 * URODamageExecution
 * Gameplay Effect Execution Calculation implementing the full Ragnarok Online damage formula.
 *
 * Physical damage (pre-renewal):
 *   FinalDamage = max(1, (ATK * SkillMod * ElementMod * SizeMod) - DEF)
 *   Hit check: HitRate - FleeRate >= random(0,100)
 *   Critical: CritRate > random(0,100) -> ignore flee and DEF (no bonus damage in pre-renewal)
 *
 * Magical damage:
 *   FinalDamage = max(1, MATK * SkillMod * ElementMod - MDEF)
 */
UCLASS()
class RAGNAROKUE_API URODamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	URODamageExecution();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:
	/** Attribute captures for the damage formula. */
	FGameplayEffectAttributeCaptureDefinition ATKDef;
	FGameplayEffectAttributeCaptureDefinition MATKDef;
	FGameplayEffectAttributeCaptureDefinition HITDef;
	FGameplayEffectAttributeCaptureDefinition CritRateDef;
	FGameplayEffectAttributeCaptureDefinition DEFDef;
	FGameplayEffectAttributeCaptureDefinition MDEFDef;
	FGameplayEffectAttributeCaptureDefinition FLEEDef;
	FGameplayEffectAttributeCaptureDefinition PerfectDodgeDef;

	/**
	 * Calculate physical damage using the RO formula.
	 */
	static float CalculatePhysicalDamage(
		float ATK, float SkillMod, float ElementMod, float SizeMod,
		float TotalDEF, bool bIsCritical);

	/**
	 * Calculate magical damage using the RO formula.
	 */
	static float CalculateMagicalDamage(
		float MATK, float SkillMod, float ElementMod, float MDEF);

	/**
	 * Perform hit/miss check.
	 * @return True if the attack hits
	 */
	static bool PerformHitCheck(float HIT, float FLEE);

	/**
	 * Perform critical hit check.
	 * @return True if critical
	 */
	static bool PerformCriticalCheck(float CritRate, float PerfectDodge);
};
