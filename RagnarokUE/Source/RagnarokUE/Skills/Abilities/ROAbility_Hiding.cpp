// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROAbility_Hiding.h"
#include "AbilitySystemComponent.h"
#include "RagnarokUE/Skills/ROAttributeSet.h"
#include "RagnarokUE/Character/ROCharacterBase.h"

UROAbility_Hiding::UROAbility_Hiding()
{
	SkillID = 51;
	SkillName = FName("TF_HIDING");
	MaxSkillLevel = 10;
	SkillLevel = 1;
	SkillElement = EROElement::Neutral;

	// 10 SP to activate, plus SP is drained over time
	SPCostBase = 10.0f;
	SPCostPerLevel = 0.0f;

	// Instant cast
	VariableCastTimeBase = 0.0f;
	FixedCastTime = 0.0f;
	CooldownDuration = 0.5f;

	SPDrainTickInterval = 1.0f;
	bIsHiding = false;
}

void UROAbility_Hiding::OnCastComplete()
{
	Super::OnCastComplete();

	if (!CachedActorInfo || !CachedActorInfo->AbilitySystemComponent.IsValid())
	{
		EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
		return;
	}

	UAbilitySystemComponent* ASC = CachedActorInfo->AbilitySystemComponent.Get();

	// Toggle behavior: if already hiding, stop hiding
	FGameplayTag HidingTag = FGameplayTag::RequestGameplayTag(FName("Status.Hiding"), false);
	if (HidingTag.IsValid() && ASC->HasMatchingGameplayTag(HidingTag))
	{
		EndHiding();
		return;
	}

	// Start hiding
	bIsHiding = true;

	// Apply hiding tag
	if (HidingTag.IsValid())
	{
		ASC->AddLooseGameplayTag(HidingTag);
	}

	// Start SP drain timer (use weak pointer to avoid crash if ability is destroyed while timer pending)
	if (CachedActorInfo->AvatarActor.IsValid())
	{
		TWeakObjectPtr<UROAbility_Hiding> WeakThis(this);
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->OnSPDrainTick();
			}
		});

		CachedActorInfo->AvatarActor->GetWorldTimerManager().SetTimer(
			SPDrainTimerHandle, TimerDelegate, SPDrainTickInterval, true);
	}

	// Note: ability stays active (does not EndAbility) while hiding is toggled on
}

void UROAbility_Hiding::EndHiding()
{
	if (!bIsHiding)
	{
		return;
	}

	bIsHiding = false;

	if (CachedActorInfo && CachedActorInfo->AbilitySystemComponent.IsValid())
	{
		UAbilitySystemComponent* ASC = CachedActorInfo->AbilitySystemComponent.Get();

		// Remove hiding tag
		FGameplayTag HidingTag = FGameplayTag::RequestGameplayTag(FName("Status.Hiding"), false);
		if (HidingTag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(HidingTag);
		}
	}

	// Clear SP drain timer
	if (CachedActorInfo && CachedActorInfo->AvatarActor.IsValid())
	{
		CachedActorInfo->AvatarActor->GetWorldTimerManager().ClearTimer(SPDrainTimerHandle);
	}

	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

void UROAbility_Hiding::OnSPDrainTick()
{
	if (!bIsHiding || !CachedActorInfo || !CachedActorInfo->AbilitySystemComponent.IsValid())
	{
		EndHiding();
		return;
	}

	UAbilitySystemComponent* ASC = CachedActorInfo->AbilitySystemComponent.Get();
	const UROAttributeSet* AttrSet = ASC->GetSet<UROAttributeSet>();

	if (!AttrSet)
	{
		EndHiding();
		return;
	}

	const float CurrentSP = AttrSet->GetSP();
	const float DrainAmount = GetSPDrainPerTick();

	if (CurrentSP < DrainAmount)
	{
		// Not enough SP - end hiding
		EndHiding();
		return;
	}

	// Deduct SP using SetNumericAttributeBase to persist across recalculations
	const float NewSP = CurrentSP - DrainAmount;
	ASC->SetNumericAttributeBase(UROAttributeSet::GetSPAttribute(), NewSP);

	// Sync back to replicated property since SetNumericAttributeBase bypasses PostGameplayEffectExecute
	if (AROCharacterBase* Character = Cast<AROCharacterBase>(CachedActorInfo->AvatarActor.Get()))
	{
		Character->CurrentSP = FMath::RoundToInt32(NewSP);
	}
}

void UROAbility_Hiding::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	// Ensure timer is cleaned up when ability is removed (e.g. character destruction, disconnect)
	if (bIsHiding)
	{
		EndHiding();
	}

	Super::OnRemoveAbility(ActorInfo, Spec);
}

float UROAbility_Hiding::GetSPDrainPerTick() const
{
	// SP drain per tick: 10 - SkillLevel
	return FMath::Max(1.0f, 10.0f - static_cast<float>(SkillLevel));
}
