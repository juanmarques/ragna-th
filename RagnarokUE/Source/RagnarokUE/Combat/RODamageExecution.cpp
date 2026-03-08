// Copyright Ragna-TH Project. All Rights Reserved.

#include "RODamageExecution.h"
#include "RagnarokUE/Skills/ROAttributeSet.h"
#include "ROElementalSystem.h"
#include "ROStatusEffectComponent.h"
#include "RagnarokUE/Monsters/ROMonsterBase.h"
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

	// ElementMod: compute from attack element vs target's defense element
	float ElementMod = 1.0f;
	FGameplayTag AttackElementTag = FGameplayTag::RequestGameplayTag(FName("Data.AttackElement"), false);
	if (AttackElementTag.IsValid())
	{
		const float AttackElementFloat = Spec.GetSetByCallerMagnitude(AttackElementTag, false, -1.0f);
		if (AttackElementFloat >= 0.0f)
		{
			const EROElement AttackElement = static_cast<EROElement>(FMath::RoundToInt32(AttackElementFloat));

			// Determine target's defense element and level
			EROElement DefElement = EROElement::Neutral;
			EROElementLevel DefLevel = EROElementLevel::Level1;

			UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
			if (TargetASC)
			{
				AActor* TargetActor = TargetASC->GetOwnerActor();
				if (TargetActor)
				{
					// Check status effect overrides first (Freeze -> Water, Stone -> Earth)
					UROStatusEffectComponent* StatusComp = TargetActor->FindComponentByClass<UROStatusEffectComponent>();
					if (StatusComp && StatusComp->GetElementOverride(DefElement, DefLevel))
					{
						// Element overridden by status effect
					}
					else if (AROMonsterBase* Monster = Cast<AROMonsterBase>(TargetActor))
					{
						// Monster has explicit element properties
						DefElement = Monster->Element;
						DefLevel = Monster->ElementLevel;
					}
					// else: player characters default to Neutral Lv1
				}
			}

			ElementMod = UROElementalSystem::GetElementalModifier(AttackElement, DefElement, DefLevel);
		}
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

		// Pre-renewal RO: DEF is flat subtraction.
		// The GAS DEF attribute stores total DEF (soft DEF from VIT + equipment hard DEF).
		// Both are subtracted from damage as flat reduction. Critical hits ignore DEF.
		FinalDamage = CalculatePhysicalDamage(SourceATK, SkillMod, ElementMod, SizeMod, TargetDEF, bIsCritical);
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
	float TotalDEF, bool bIsCritical)
{
	// Pre-renewal RO Physical Damage Formula:
	// FinalDamage = max(1, (ATK * SkillMod * ElementMod * SizeMod) - DEF)
	// Critical: always uses max weapon ATK roll, ignores FLEE and DEF

	float RawDamage = ATK * SkillMod * ElementMod * SizeMod;

	if (!bIsCritical)
	{
		// Flat DEF subtraction (pre-renewal behavior)
		RawDamage -= TotalDEF;
	}
	// Critical hits: DEF is ignored. No multiplicative bonus in pre-renewal;
	// the damage increase comes from always using max weapon ATK variance.

	return FMath::Max(1.0f, RawDamage);
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
