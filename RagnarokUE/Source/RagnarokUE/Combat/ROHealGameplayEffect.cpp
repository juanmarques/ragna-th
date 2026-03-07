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

	// Use SetByCaller so the ability can set the heal amount at runtime
	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = FGameplayTag::RequestGameplayTag(FName("Data.HealAmount"));
	HealModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);

	Modifiers.Add(HealModifier);
}
