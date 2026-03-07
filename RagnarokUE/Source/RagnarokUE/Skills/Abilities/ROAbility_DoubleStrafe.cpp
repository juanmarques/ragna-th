// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROAbility_DoubleStrafe.h"
#include "AbilitySystemComponent.h"
#include "RagnarokUE/Skills/ROAttributeSet.h"

UROAbility_DoubleStrafe::UROAbility_DoubleStrafe()
{
	SkillID = 46;
	SkillName = FName("AC_DOUBLE");
	MaxSkillLevel = 10;
	SkillLevel = 1;
	SkillElement = EROElement::Neutral;

	// SP cost: 12 flat
	SPCostBase = 12.0f;
	SPCostPerLevel = 0.0f;

	// Instant cast
	VariableCastTimeBase = 0.0f;
	FixedCastTime = 0.0f;
	CooldownDuration = 0.3f;
}

bool UROAbility_DoubleStrafe::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Check bow equipped
	if (!IsBowEquipped(ActorInfo))
	{
		return false;
	}

	return true;
}

void UROAbility_DoubleStrafe::OnCastComplete()
{
	Super::OnCastComplete();

	if (!CachedActorInfo || !CachedActorInfo->AbilitySystemComponent.IsValid())
	{
		EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
		return;
	}

	// 2-hit attack: each hit does ATK * (100% + 10% * Level)
	const float PerHitMod = GetPerHitDamageModifier();
	const float TotalDamageMod = PerHitMod * 2.0f; // 2 hits

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
			DamageSpec.Data->SetSetByCallerMagnitude(DamageTypeTag, 0.0f); // Physical
		}

		FGameplayTag ElementModTag = FGameplayTag::RequestGameplayTag(FName("Data.ElementMod"), false);
		if (ElementModTag.IsValid())
		{
			DamageSpec.Data->SetSetByCallerMagnitude(ElementModTag, 1.0f);
		}

		FGameplayTag SizeModTag = FGameplayTag::RequestGameplayTag(FName("Data.SizeMod"), false);
		if (SizeModTag.IsValid())
		{
			DamageSpec.Data->SetSetByCallerMagnitude(SizeModTag, 1.0f);
		}
	}

	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

bool UROAbility_DoubleStrafe::IsBowEquipped(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	// Check for bow equipment tag on the owning actor
	FGameplayTag BowTag = FGameplayTag::RequestGameplayTag(FName("Equipment.Weapon.Bow"), false);
	if (BowTag.IsValid())
	{
		return ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(BowTag);
	}

	// Fallback: in a full implementation, check the equipment component
	// For now, allow activation if we can't verify (development convenience)
	return true;
}

float UROAbility_DoubleStrafe::GetPerHitDamageModifier() const
{
	// Damage per hit: ATK * (100% + 10% * SkillLevel)
	return 1.0f + 0.1f * static_cast<float>(SkillLevel);
}
