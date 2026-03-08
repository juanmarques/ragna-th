// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROAbility_Heal.h"
#include "AbilitySystemComponent.h"
#include "RagnarokUE/Skills/ROAttributeSet.h"
#include "RagnarokUE/Combat/ROHealGameplayEffect.h"
#include "RagnarokUE/Character/ROStatsComponent.h"
#include "RagnarokUE/Character/ROLevelingComponent.h"
#include "RagnarokUE/Core/ROPlayerController.h"

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
	int32 BaseLevel = 1;
	int32 INT_Stat = 1;

	// Get actual base level from the leveling component
	if (CachedActorInfo->AvatarActor.IsValid())
	{
		AActor* AvatarActor = CachedActorInfo->AvatarActor.Get();

		UROLevelingComponent* LevelComp = AvatarActor->FindComponentByClass<UROLevelingComponent>();
		if (LevelComp)
		{
			BaseLevel = LevelComp->BaseLevel;
		}

		// Get actual INT from the stats component
		UROStatsComponent* StatsComp = AvatarActor->FindComponentByClass<UROStatsComponent>();
		if (StatsComp)
		{
			INT_Stat = StatsComp->GetTotalStat(EROStat::INT_STAT);
		}
	}

	// Fallback: use MATK from attribute set if stats component not available
	if (INT_Stat <= 1)
	{
		const UROAttributeSet* AttrSet = SourceASC->GetSet<UROAttributeSet>();
		if (AttrSet)
		{
			INT_Stat = FMath::RoundToInt32(AttrSet->GetMATK());
		}
	}

	const float HealAmount = CalculateHealAmount(BaseLevel, INT_Stat);

	// Resolve target: try player controller's selected target, fall back to self
	AActor* TargetActor = nullptr;
	if (CachedActorInfo->AvatarActor.IsValid())
	{
		APawn* OwnerPawn = Cast<APawn>(CachedActorInfo->AvatarActor.Get());
		if (OwnerPawn)
		{
			AROPlayerController* PC = Cast<AROPlayerController>(OwnerPawn->GetController());
			if (PC && PC->SelectedTarget)
			{
				TargetActor = PC->SelectedTarget;
			}
		}
	}

	// Self-heal fallback if no target selected
	if (!TargetActor && CachedActorInfo->AvatarActor.IsValid())
	{
		TargetActor = CachedActorInfo->AvatarActor.Get();
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

	// Determine if target is undead (check for Undead race tag)
	bool bTargetIsUndead = false;
	FGameplayTag UndeadTag = FGameplayTag::RequestGameplayTag(FName("Race.Undead"), false);
	if (UndeadTag.IsValid())
	{
		bTargetIsUndead = TargetASC->HasMatchingGameplayTag(UndeadTag);
	}

	if (bDamageUndead && bTargetIsUndead)
	{
		// In RO, Heal deals fixed Holy damage to Undead equal to the heal amount.
		// This bypasses ATK/DEF entirely. Apply directly as IncomingDamage.
		TargetASC->ApplyModToAttribute(
			UROAttributeSet::GetIncomingDamageAttribute(),
			EGameplayModOp::Additive,
			HealAmount);
	}
	else
	{
		// Heal the target via the heal GE
		FGameplayEffectSpecHandle HealSpec = MakeOutgoingGameplayEffectSpec(
			UROHealGameplayEffect::StaticClass(), SkillLevel);

		if (HealSpec.IsValid())
		{
			FGameplayTag HealAmountTag = FGameplayTag::RequestGameplayTag(FName("Data.HealAmount"), false);
			if (HealAmountTag.IsValid())
			{
				HealSpec.Data->SetSetByCallerMagnitude(HealAmountTag, HealAmount);
			}

			SourceASC->ApplyGameplayEffectSpecToTarget(*HealSpec.Data.Get(), TargetASC);
		}
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
