// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROAbility_ColdBolt.h"
#include "AbilitySystemComponent.h"
#include "RagnarokUE/Skills/ROAttributeSet.h"
#include "RagnarokUE/Combat/ROElementalSystem.h"

UROAbility_ColdBolt::UROAbility_ColdBolt()
{
	SkillID = 20;
	SkillName = FName("MG_COLDBOLT");
	MaxSkillLevel = 10;
	SkillLevel = 1;
	SkillElement = EROElement::Water;

	// SP cost: 12 + 3 * Level
	SPCostBase = 9.0f;
	SPCostPerLevel = 3.0f;

	VariableCastTimeBase = 0.7f;
	FixedCastTime = 0.3f;
	CooldownDuration = 0.3f;

	BoltInterval = 0.15f;
}

void UROAbility_ColdBolt::OnCastComplete()
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
			DamageSpec.Data->SetSetByCallerMagnitude(ElementModTag, 1.0f); // Water element
		}
	}

	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

int32 UROAbility_ColdBolt::GetBoltCount() const
{
	return SkillLevel;
}

float UROAbility_ColdBolt::GetPerBoltDamageModifier() const
{
	return 1.0f;
}
