// Copyright Ragna-TH Project. All Rights Reserved.

#include "RODamageExecution.h"
#include "RagnarokUE/Skills/ROAttributeSet.h"
#include "ROElementalSystem.h"
#include "AbilitySystemComponent.h"

// Struct to hold captured attributes at execution time
struct FRODamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(ATK);
	DECLARE_ATTRIBUTE_CAPTUREDEF(MATK);
	DECLARE_ATTRIBUTE_CAPTUREDEF(HIT);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritRate);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DEF);
	DECLARE_ATTRIBUTE_CAPTUREDEF(MDEF);
	DECLARE_ATTRIBUTE_CAPTUREDEF(FLEE);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PerfectDodge);

	FRODamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UROAttributeSet, ATK, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UROAttributeSet, MATK, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UROAttributeSet, HIT, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UROAttributeSet, CritRate, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UROAttributeSet, DEF, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UROAttributeSet, MDEF, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UROAttributeSet, FLEE, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UROAttributeSet, PerfectDodge, Target, false);
	}
};

static const FRODamageStatics& GetDamageStatics()
{
	static FRODamageStatics DamageStatics;
	return DamageStatics;
}

URODamageExecution::URODamageExecution()
{
	const FRODamageStatics& Statics = GetDamageStatics();

	// Capture source attributes
	ATKDef = Statics.ATKDef;
	MATKDef = Statics.MATKDef;
	HITDef = Statics.HITDef;
	CritRateDef = Statics.CritRateDef;

	// Capture target attributes
	DEFDef = Statics.DEFDef;
	MDEFDef = Statics.MDEFDef;
	FLEEDef = Statics.FLEEDef;
	PerfectDodgeDef = Statics.PerfectDodgeDef;

	RelevantAttributesToCapture.Add(ATKDef);
	RelevantAttributesToCapture.Add(MATKDef);
	RelevantAttributesToCapture.Add(HITDef);
	RelevantAttributesToCapture.Add(CritRateDef);
	RelevantAttributesToCapture.Add(DEFDef);
	RelevantAttributesToCapture.Add(MDEFDef);
	RelevantAttributesToCapture.Add(FLEEDef);
	RelevantAttributesToCapture.Add(PerfectDodgeDef);
}

void URODamageExecution::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = SourceTags;
	EvalParams.TargetTags = TargetTags;

	// Capture source attributes
	float SourceATK = 0.0f;
	float SourceMATK = 0.0f;
	float SourceHIT = 0.0f;
	float SourceCritRate = 0.0f;

	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(ATKDef, EvalParams, SourceATK);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MATKDef, EvalParams, SourceMATK);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(HITDef, EvalParams, SourceHIT);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CritRateDef, EvalParams, SourceCritRate);

	// Capture target attributes
	float TargetDEF = 0.0f;
	float TargetMDEF = 0.0f;
	float TargetFLEE = 0.0f;
	float TargetPerfectDodge = 0.0f;

	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DEFDef, EvalParams, TargetDEF);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MDEFDef, EvalParams, TargetMDEF);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(FLEEDef, EvalParams, TargetFLEE);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(PerfectDodgeDef, EvalParams, TargetPerfectDodge);

	// Read damage parameters from set-by-caller magnitudes on the GE spec
	// SkillMod: total skill damage modifier (e.g. 1.3 for 130% damage)
	float SkillMod = 1.0f;
	FGameplayTag SkillModTag = FGameplayTag::RequestGameplayTag(FName("Data.SkillMod"), false);
	if (SkillModTag.IsValid())
	{
		SkillMod = Spec.GetSetByCallerMagnitude(SkillModTag, false, 1.0f);
	}

	// DamageType: 0=Physical, 1=Magical, 2=Misc
	float DamageTypeFloat = 0.0f;
	FGameplayTag DamageTypeTag = FGameplayTag::RequestGameplayTag(FName("Data.DamageType"), false);
	if (DamageTypeTag.IsValid())
	{
		DamageTypeFloat = Spec.GetSetByCallerMagnitude(DamageTypeTag, false, 0.0f);
	}
	const ERODamageType DamageType = static_cast<ERODamageType>(FMath::RoundToInt32(DamageTypeFloat));

	// ElementMod: elemental damage multiplier (pre-calculated by the ability)
	float ElementMod = 1.0f;
	FGameplayTag ElementModTag = FGameplayTag::RequestGameplayTag(FName("Data.ElementMod"), false);
	if (ElementModTag.IsValid())
	{
		ElementMod = Spec.GetSetByCallerMagnitude(ElementModTag, false, 1.0f);
	}

	// SizeMod: size-based damage multiplier
	float SizeMod = 1.0f;
	FGameplayTag SizeModTag = FGameplayTag::RequestGameplayTag(FName("Data.SizeMod"), false);
	if (SizeModTag.IsValid())
	{
		SizeMod = Spec.GetSetByCallerMagnitude(SizeModTag, false, 1.0f);
	}

	float FinalDamage = 0.0f;

	if (DamageType == ERODamageType::Physical)
	{
		// Critical check first - crits bypass flee
		bool bIsCritical = PerformCriticalCheck(SourceCritRate, TargetPerfectDodge);

		if (!bIsCritical)
		{
			// Hit/miss check for non-critical physical attacks
			if (!PerformHitCheck(SourceHIT, TargetFLEE))
			{
				// Miss - no damage
				return;
			}
		}

		// In RO, DEF has two components:
		// HardDEF = equipment DEF (flat reduction)
		// SoftDEF = VIT-based DEF (percentage reduction)
		// For simplicity, we split the DEF attribute:
		// HardDEF = DEF (the main value)
		// SoftDEF = DEF / 2 (approximation; in full implementation this comes from VIT)
		const float HardDEF = TargetDEF;
		const float SoftDEF = FMath::Clamp(TargetDEF * 0.5f, 0.0f, 99.0f);

		FinalDamage = CalculatePhysicalDamage(SourceATK, SkillMod, ElementMod, SizeMod, HardDEF, SoftDEF, bIsCritical);
	}
	else if (DamageType == ERODamageType::Magical)
	{
		// Magical attacks always hit (no flee check)
		FinalDamage = CalculateMagicalDamage(SourceMATK, SkillMod, ElementMod, TargetMDEF);
	}
	else
	{
		// Misc damage - ignores DEF/MDEF
		FinalDamage = FMath::Max(1.0f, SourceATK * SkillMod * ElementMod);
	}

	// Apply final damage to the target's IncomingDamage meta attribute
	if (FinalDamage > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				UROAttributeSet::GetIncomingDamageAttribute(),
				EGameplayModOp::Additive,
				FinalDamage
			)
		);
	}
}

float URODamageExecution::CalculatePhysicalDamage(
	float ATK, float SkillMod, float ElementMod, float SizeMod,
	float HardDEF, float SoftDEF, bool bIsCritical)
{
	// RO Physical Damage Formula:
	// FinalDamage = max(1, (ATK * SkillMod * ElementMod * SizeMod) - (HardDEF * (1 - SoftDEF/100)))
	// Critical: +40% damage, ignores FLEE (handled before calling this)

	float RawDamage = ATK * SkillMod * ElementMod * SizeMod;

	// Apply critical bonus
	if (bIsCritical)
	{
		RawDamage *= 1.4f;
	}

	// Defense reduction
	const float DefReduction = HardDEF * (1.0f - SoftDEF / 100.0f);
	const float FinalDamage = RawDamage - DefReduction;

	return FMath::Max(1.0f, FinalDamage);
}

float URODamageExecution::CalculateMagicalDamage(
	float MATK, float SkillMod, float ElementMod, float MDEF)
{
	// RO Magical Damage Formula:
	// FinalDamage = max(1, MATK * SkillMod * ElementMod - MDEF)

	const float RawDamage = MATK * SkillMod * ElementMod;
	const float FinalDamage = RawDamage - MDEF;

	return FMath::Max(1.0f, FinalDamage);
}

bool URODamageExecution::PerformHitCheck(float HIT, float FLEE)
{
	// RO Hit Formula: HitRate - FleeRate >= random(0, 100)
	// HitRate = attacker's HIT, FleeRate = target's FLEE
	// Minimum 5% hit rate, maximum 95% hit rate
	const float HitChance = FMath::Clamp(HIT - FLEE + 80.0f, 5.0f, 95.0f);
	const float Roll = FMath::FRandRange(0.0f, 100.0f);
	return Roll <= HitChance;
}

bool URODamageExecution::PerformCriticalCheck(float CritRate, float PerfectDodge)
{
	// RO Critical Formula: CritRate - PerfectDodge > random(0, 100)
	const float EffectiveCrit = FMath::Max(0.0f, CritRate - PerfectDodge);
	const float Roll = FMath::FRandRange(0.0f, 100.0f);
	return Roll < EffectiveCrit;
}
