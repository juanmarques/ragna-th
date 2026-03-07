// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROAbility_Heal.h"
#include "AbilitySystemComponent.h"
#include "RagnarokUE/Skills/ROAttributeSet.h"

UROAbility_Heal::UROAbility_Heal()
{
	SkillID = 28;
	SkillName = FName("AL_HEAL");
	MaxSkillLevel = 10;
	SkillLevel = 1;
	SkillElement = EROElement::Holy;

	// SP cost: 13 + 2 * Level (base=11, perLevel=2 so at Lv1 = 11+2=13)
	SPCostBase = 11.0f;
	SPCostPerLevel = 2.0f;

	// Short variable cast time
	VariableCastTimeBase = 1.0f;
	FixedCastTime = 0.0f;
	CooldownDuration = 0.3f;

	bDamageUndead = true;
}

void UROAbility_Heal::OnCastComplete()
{
	Super::OnCastComplete();

	if (!CachedActorInfo || !CachedActorInfo->AbilitySystemComponent.IsValid())
	{
		EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
		return;
	}

	UAbilitySystemComponent* SourceASC = CachedActorInfo->AbilitySystemComponent.Get();

	// Get caster stats for heal formula
	// BaseLevel would come from the character component; using a default for now
	int32 BaseLevel = 1;
	int32 INT_Stat = 1;

	const UROAttributeSet* AttrSet = SourceASC->GetSet<UROAttributeSet>();
	if (AttrSet)
	{
		// MATK serves as a proxy for INT-based magic power
		INT_Stat = FMath::RoundToInt32(AttrSet->GetMATK());
	}

	// In a full implementation, BaseLevel comes from the character's level
	// For now, use MATK as a proxy for INT contribution
	const float HealAmount = CalculateHealAmount(BaseLevel, INT_Stat);

	// Determine if target is undead (check for Undead race tag)
	AActor* TargetActor = nullptr; // Would come from targeting system
	bool bTargetIsUndead = false;

	if (TargetActor)
	{
		UAbilitySystemComponent* TargetASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();
		if (TargetASC)
		{
			FGameplayTag UndeadTag = FGameplayTag::RequestGameplayTag(FName("Race.Undead"), false);
			if (UndeadTag.IsValid())
			{
				bTargetIsUndead = TargetASC->HasMatchingGameplayTag(UndeadTag);
			}
		}
	}

	if (bDamageUndead && bTargetIsUndead)
	{
		// Deal holy damage to undead equal to heal amount
		if (TargetActor)
		{
			UAbilitySystemComponent* TargetASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();
			if (TargetASC)
			{
				TargetASC->ApplyModToAttribute(
					UROAttributeSet::GetIncomingDamageAttribute(),
					EGameplayModOp::Additive,
					HealAmount);
			}
		}
	}
	else
	{
		// Heal the target (or self if no target)
		UAbilitySystemComponent* HealTarget = SourceASC;
		if (TargetActor)
		{
			UAbilitySystemComponent* TargetASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();
			if (TargetASC)
			{
				HealTarget = TargetASC;
			}
		}

		// Apply healing via the IncomingHealing meta attribute
		HealTarget->ApplyModToAttribute(
			UROAttributeSet::GetIncomingHealingAttribute(),
			EGameplayModOp::Additive,
			HealAmount);
	}

	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

float UROAbility_Heal::CalculateHealAmount(int32 BaseLevel, int32 INT_Stat) const
{
	// RO Heal formula: (BaseLevel + INT) / 8 * (4 + 8 * SkillLevel)
	const float StatComponent = static_cast<float>(BaseLevel + INT_Stat) / 8.0f;
	const float LevelComponent = 4.0f + 8.0f * static_cast<float>(SkillLevel);
	return FMath::Max(1.0f, StatComponent * LevelComponent);
}
