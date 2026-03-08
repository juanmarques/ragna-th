// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROAbility_ColdBolt.h"
#include "AbilitySystemComponent.h"
#include "RagnarokUE/Skills/ROAttributeSet.h"
#include "RagnarokUE/Combat/ROElementalSystem.h"
#include "RagnarokUE/Combat/RODamageGameplayEffect.h"
#include "RagnarokUE/Core/ROPlayerController.h"

UROAbility_ColdBolt::UROAbility_ColdBolt()
{
	SkillID = 20;
	SkillName = FName("MG_COLDBOLT");
	MaxSkillLevel = 10;
	SkillLevel = 1;
	SkillElement = EROElement::Water;

	// SP cost: 10 + 2 * Level (Lv1=12, Lv2=14, ... Lv10=30 per RO tables)
	SPCostBase = 10.0f;
	SPCostPerLevel = 2.0f;

	VariableCastTimeBase = 0.7f;
	FixedCastTime = 0.0f; // Pre-renewal has no fixed cast time
	CooldownDuration = 0.3f;

	BoltInterval = 0.15f;
	bRequiresTarget = true;
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

	const int32 NumBolts = GetBoltCount();
	const float PerBoltMod = GetPerBoltDamageModifier();
	const float TotalDamageMod = PerBoltMod * static_cast<float>(NumBolts);

	FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(
		URODamageGameplayEffect::StaticClass(), SkillLevel);

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

		FGameplayTag AttackElementTag = FGameplayTag::RequestGameplayTag(FName("Data.AttackElement"), false);
		if (AttackElementTag.IsValid())
		{
			DamageSpec.Data->SetSetByCallerMagnitude(AttackElementTag, static_cast<float>(SkillElement));
		}

		// Apply the damage effect to the target
		SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);
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
