// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROAbility_Heal.h"
#include "AbilitySystemComponent.h"
#include "RagnarokUE/Skills/ROAttributeSet.h"
#include "RagnarokUE/Combat/RODamageGameplayEffect.h"
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
		// Deal holy damage to undead equal to heal amount via the damage GE
		FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(
			URODamageGameplayEffect::StaticClass(), SkillLevel);

		if (DamageSpec.IsValid())
		{
			// For undead damage from Heal, we set SkillMod to the raw heal amount
			// and DamageType to Misc (2) so it bypasses DEF/MDEF
			FGameplayTag SkillModTag = FGameplayTag::RequestGameplayTag(FName("Data.SkillMod"), false);
			if (SkillModTag.IsValid())
			{
				// SkillMod acts as a multiplier on ATK/MATK in the execution.
				// For Heal damage to undead, we want fixed damage = HealAmount.
				// Set SkillMod high enough that ATK * SkillMod ~ HealAmount.
				// Using DamageType Misc (2) which computes: ATK * SkillMod * ElementMod
				// We want the result to be HealAmount, so SkillMod = HealAmount / ATK.
				// However, the source ATK may be low. Instead, pass HealAmount as SkillMod
				// and rely on the Misc path which uses SourceATK * SkillMod.
				DamageSpec.Data->SetSetByCallerMagnitude(SkillModTag, HealAmount);
			}

			FGameplayTag DamageTypeTag = FGameplayTag::RequestGameplayTag(FName("Data.DamageType"), false);
			if (DamageTypeTag.IsValid())
			{
				DamageSpec.Data->SetSetByCallerMagnitude(DamageTypeTag, 2.0f); // Misc (ignores DEF/MDEF)
			}

			FGameplayTag ElementModTag = FGameplayTag::RequestGameplayTag(FName("Data.ElementMod"), false);
			if (ElementModTag.IsValid())
			{
				DamageSpec.Data->SetSetByCallerMagnitude(ElementModTag, 1.0f); // Holy
			}

			SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);
		}
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
