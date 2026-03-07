// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROAbility_FireBolt.h"
#include "AbilitySystemComponent.h"
#include "RagnarokUE/Skills/ROAttributeSet.h"
#include "RagnarokUE/Combat/ROElementalSystem.h"

UROAbility_FireBolt::UROAbility_FireBolt()
{
	SkillID = 19;
	SkillName = FName("MG_FIREBOLT");
	MaxSkillLevel = 10;
	SkillLevel = 1;
	SkillElement = EROElement::Fire;

	// SP cost: 12 + 3 * Level (base=9, perLevel=3 so at Lv1 = 9+3=12)
	SPCostBase = 9.0f;
	SPCostPerLevel = 3.0f;

	// Variable cast time scales with level
	VariableCastTimeBase = 0.7f; // Base variable cast per bolt
	FixedCastTime = 0.3f;
	CooldownDuration = 0.3f;

	BoltInterval = 0.15f;
}

void UROAbility_FireBolt::OnCastComplete()
{
	Super::OnCastComplete();

	if (!CachedActorInfo || !CachedActorInfo->AbilitySystemComponent.IsValid())
	{
		EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
		return;
	}

	UAbilitySystemComponent* SourceASC = CachedActorInfo->AbilitySystemComponent.Get();
	const int32 NumBolts = GetBoltCount();
	const float PerBoltMod = GetPerBoltDamageModifier();

	// In a full implementation, each bolt would be fired on a timer for visual effect.
	// Here we calculate the total damage for all bolts and apply at once.
	// A production version would use a looping timer with BoltInterval.

	const float TotalDamageMod = PerBoltMod * static_cast<float>(NumBolts);

	FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(
		UGameplayEffect::StaticClass(), SkillLevel);

	if (DamageSpec.IsValid())
	{
		FGameplayTag SkillModTag = FGameplayTag::RequestGameplayTag(FName("Data.SkillMod"), false);
		if (SkillModTag.IsValid())
		{
			DamageSpec.Data->SetSetByCallerMagnitude(SkillModTag, TotalDamageMod);
		}

		FGameplayTag DamageTypeTag = FGameplayTag::RequestGameplayTag(FName("Data.DamageType"), false);
		if (DamageTypeTag.IsValid())
		{
			DamageSpec.Data->SetSetByCallerMagnitude(DamageTypeTag, 1.0f); // Magical
		}

		FGameplayTag ElementModTag = FGameplayTag::RequestGameplayTag(FName("Data.ElementMod"), false);
		if (ElementModTag.IsValid())
		{
			// Fire element modifier will be looked up against target's element in the execution
			DamageSpec.Data->SetSetByCallerMagnitude(ElementModTag, 1.0f);
		}
	}

	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

int32 UROAbility_FireBolt::GetBoltCount() const
{
	// Number of bolts = skill level
	return SkillLevel;
}

float UROAbility_FireBolt::GetPerBoltDamageModifier() const
{
	// Each bolt does MATK * 100% = 1.0 modifier per bolt
	return 1.0f;
}
