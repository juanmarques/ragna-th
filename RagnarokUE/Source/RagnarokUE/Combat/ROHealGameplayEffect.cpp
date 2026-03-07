// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROHealGameplayEffect.h"
#include "RagnarokUE/Skills/ROAttributeSet.h"
#include "GameplayTagContainer.h"

UROHealGameplayEffect::UROHealGameplayEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Add a modifier that applies IncomingHealing via SetByCaller
	FGameplayModifierInfo HealModifier;
	HealModifier.Attribute = UROAttributeSet::GetIncomingHealingAttribute();
	HealModifier.ModifierOp = EGameplayModOp::Additive;

	// Use SetByCaller magnitude so the ability can set the heal amount at runtime
	// The tag "Data.HealAmount" is used to look up the magnitude from the spec
	FGameplayTag HealAmountTag = FGameplayTag::RequestGameplayTag(FName("Data.HealAmount"));
	HealModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(HealAmountTag);

	Modifiers.Add(HealModifier);
}
