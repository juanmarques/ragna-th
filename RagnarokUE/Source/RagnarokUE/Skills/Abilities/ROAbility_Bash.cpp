// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROAbility_Bash.h"
#include "AbilitySystemComponent.h"
#include "RagnarokUE/Skills/ROAttributeSet.h"
#include "RagnarokUE/Combat/ROStatusEffectComponent.h"
#include "RagnarokUE/Combat/ROElementalSystem.h"
#include "RagnarokUE/Combat/RODamageGameplayEffect.h"
#include "RagnarokUE/Core/ROPlayerController.h"
#include "RagnarokUE/Character/ROStatsComponent.h"
#include "RagnarokUE/Character/ROLevelingComponent.h"

UROAbility_Bash::UROAbility_Bash()
{
	SkillID = 5;
	SkillName = FName("SM_BASH");
	MaxSkillLevel = 10;
	SkillLevel = 1;
	SkillElement = EROElement::Neutral;

	// SP cost: 6 + 2 * Level (Lv1=8, Lv5=16, Lv10=26)
	SPCostBase = 6.0f;
	SPCostPerLevel = 2.0f;

	// Instant cast
	VariableCastTimeBase = 0.0f;
	FixedCastTime = 0.0f;
	CooldownDuration = 0.5f;

	StunChanceBase = 5.0f;
	StunDuration = 5.0f;
}

void UROAbility_Bash::OnCastComplete()
{
	Super::OnCastComplete();

	if (!CachedActorInfo || !CachedActorInfo->AbilitySystemComponent.IsValid())
	{
		EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
		return;
	}

	UAbilitySystemComponent* SourceASC = CachedActorInfo->AbilitySystemComponent.Get();

	// Resolve target from the player controller's selected target
	AActor* TargetActor = nullptr;
	if (CachedActorInfo->AvatarActor.IsValid())
	{
		APawn* OwnerPawn = Cast<APawn>(CachedActorInfo->AvatarActor.Get());
		if (OwnerPawn)
		{
			AROPlayerController* PC = Cast<AROPlayerController>(OwnerPawn->GetController());
			if (PC)
			{
				TargetActor = PC->SelectedTarget;
			}
		}
	}

	if (!TargetActor)
	{
		EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
		return;
	}

	UAbilitySystemComponent* TargetASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();
	if (!TargetASC)
	{
		EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
		return;
	}

	// Get damage modifier: ATK * (100% + 30% * Level)
	const float DamageMod = GetDamageModifier();

	// Create and apply damage gameplay effect
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(
		URODamageGameplayEffect::StaticClass(), SkillLevel);

	if (DamageSpecHandle.IsValid())
	{
		// Set damage parameters via SetByCaller
		FGameplayTag SkillModTag = FGameplayTag::RequestGameplayTag(FName("Data.SkillMod"), false);
		if (SkillModTag.IsValid())
		{
			DamageSpecHandle.Data->SetSetByCallerMagnitude(SkillModTag, DamageMod);
		}

		FGameplayTag DamageTypeTag = FGameplayTag::RequestGameplayTag(FName("Data.DamageType"), false);
		if (DamageTypeTag.IsValid())
		{
			DamageSpecHandle.Data->SetSetByCallerMagnitude(DamageTypeTag, 0.0f); // Physical
		}

		FGameplayTag AttackElementTag = FGameplayTag::RequestGameplayTag(FName("Data.AttackElement"), false);
		if (AttackElementTag.IsValid())
		{
			DamageSpecHandle.Data->SetSetByCallerMagnitude(AttackElementTag, static_cast<float>(SkillElement));
		}

		FGameplayTag SizeModTag = FGameplayTag::RequestGameplayTag(FName("Data.SizeMod"), false);
		if (SizeModTag.IsValid())
		{
			DamageSpecHandle.Data->SetSetByCallerMagnitude(SizeModTag, 1.0f);
		}

		// Apply the damage effect to the target
		SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC);
	}

	// Apply stun at high levels (with VIT resistance)
	const float StunChance = GetStunChance();
	if (StunChance > 0.0f && TargetActor)
	{
		UROStatusEffectComponent* StatusComp = TargetActor->FindComponentByClass<UROStatusEffectComponent>();
		if (StatusComp)
		{
			// Read target stats for resistance calculation
			int32 TargetVIT = 0;
			int32 TargetINT = 0;
			int32 TargetLUK = 0;
			int32 TargetBaseLevel = 1;
			UROStatsComponent* TargetStats = TargetActor->FindComponentByClass<UROStatsComponent>();
			if (TargetStats)
			{
				TargetVIT = TargetStats->GetTotalStat(EROStat::VIT);
				TargetINT = TargetStats->GetTotalStat(EROStat::INT_STAT);
				TargetLUK = TargetStats->GetTotalStat(EROStat::LUK);
			}
			UROLevelingComponent* TargetLevel = TargetActor->FindComponentByClass<UROLevelingComponent>();
			if (TargetLevel)
			{
				TargetBaseLevel = TargetLevel->BaseLevel;
			}
			StatusComp->ApplyStatusEffectWithResist(
				EROStatusEffect::Stun, StunDuration, 1, StunChance,
				TargetVIT, TargetINT, TargetLUK, TargetBaseLevel);
		}
	}

	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

float UROAbility_Bash::GetDamageModifier() const
{
	// Damage: ATK * (100% + 30% * SkillLevel)
	return 1.0f + 0.3f * static_cast<float>(SkillLevel);
}

float UROAbility_Bash::GetStunChance() const
{
	// Stun chance starts at level 6: 5% per level above 5
	if (SkillLevel >= 6)
	{
		return StunChanceBase * static_cast<float>(SkillLevel - 5);
	}
	return 0.0f;
}
