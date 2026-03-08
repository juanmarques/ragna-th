// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROGameplayAbility.h"
#include "ROAttributeSet.h"
#include "RagnarokUE/Combat/ROCastingComponent.h"
#include "RagnarokUE/Character/ROStatsComponent.h"
#include "RagnarokUE/Character/ROCharacterBase.h"
#include "RagnarokUE/Core/ROPlayerController.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

UROGameplayAbility::UROGameplayAbility()
{
	SkillLevel = 1;
	MaxSkillLevel = 10;
	SPCostBase = 0.0f;
	SPCostPerLevel = 0.0f;
	VariableCastTimeBase = 0.0f;
	FixedCastTime = 0.0f;
	CooldownDuration = 0.0f;
	SkillElement = EROElement::Neutral;
	SkillID = 0;
	SkillName = NAME_None;
	CachedActorInfo = nullptr;
	bRequiresTarget = false;

	// Default instancing policy: one instance per actor
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

float UROGameplayAbility::GetSPCost() const
{
	return SPCostBase + SPCostPerLevel * static_cast<float>(SkillLevel);
}

float UROGameplayAbility::GetVariableCastTime(int32 DEX, int32 INT_Stat) const
{
	if (VariableCastTimeBase <= 0.0f)
	{
		return 0.0f;
	}

	// Renewal RO formula: CastTime * (1 - sqrt((DEX*2 + INT) / 530))
	const float StatSum = static_cast<float>(DEX * 2 + INT_Stat);
	const float Reduction = FMath::Clamp(FMath::Sqrt(StatSum / 530.0f), 0.0f, 1.0f);
	const float Result = VariableCastTimeBase * (1.0f - Reduction);
	return FMath::Max(0.0f, Result);
}

float UROGameplayAbility::GetCooldown() const
{
	return CooldownDuration;
}

bool UROGameplayAbility::CanActivateAbility(
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

	// Check SP cost
	const UROAttributeSet* AttrSet = GetROAttributeSet(ActorInfo);
	if (AttrSet)
	{
		const float CurrentSP = AttrSet->GetSP();
		const float Cost = GetSPCost();
		if (CurrentSP < Cost)
		{
			return false;
		}
	}

	// Check not silenced
	if (HasStatusEffectTag(ActorInfo, FName("Status.Silence")))
	{
		return false;
	}

	// Check not stunned
	if (HasStatusEffectTag(ActorInfo, FName("Status.Stun")))
	{
		return false;
	}

	// Check not frozen
	if (HasStatusEffectTag(ActorInfo, FName("Status.Freeze")))
	{
		return false;
	}

	// Check not sleeping
	if (HasStatusEffectTag(ActorInfo, FName("Status.Sleep")))
	{
		return false;
	}

	// Check not petrified (hard stone Phase 2 only — Phase 1 allows abilities except attacks)
	if (HasStatusEffectTag(ActorInfo, FName("Status.Stone.Phase2")))
	{
		return false;
	}

	// Validate target exists for skills that require one (prevents wasting SP/cooldown)
	if (bRequiresTarget && ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		APawn* Pawn = Cast<APawn>(ActorInfo->AvatarActor.Get());
		AROPlayerController* PC = Pawn ? Cast<AROPlayerController>(Pawn->GetController()) : nullptr;
		if (!PC || !PC->SelectedTarget)
		{
			return false;
		}
	}

	return true;
}

void UROGameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Cache activation data for delegate callbacks
	CachedHandle = Handle;
	CachedActorInfo = ActorInfo;
	CachedActivationInfo = ActivationInfo;

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Get cast times - read actual DEX and INT from the stats component
	int32 DEX = 1;
	int32 INT_Stat = 1;
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		UROStatsComponent* StatsComp = ActorInfo->AvatarActor->FindComponentByClass<UROStatsComponent>();
		if (StatsComp)
		{
			DEX = StatsComp->GetTotalStat(EROStat::DEX);
			INT_Stat = StatsComp->GetTotalStat(EROStat::INT_STAT);
		}
	}

	const float VarCast = GetVariableCastTime(DEX, INT_Stat);
	const float TotalCast = VarCast + FixedCastTime;

	if (TotalCast > 0.0f && ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		// Start casting via the casting component
		UROCastingComponent* CastComp = ActorInfo->AvatarActor->FindComponentByClass<UROCastingComponent>();
		if (CastComp)
		{
			CastComp->OnCastComplete.RemoveDynamic(this, &UROGameplayAbility::OnCastComplete);
			CastComp->OnCastInterrupted.RemoveDynamic(this, &UROGameplayAbility::OnCastInterrupted);
			CastComp->OnCastComplete.AddDynamic(this, &UROGameplayAbility::OnCastComplete);
			CastComp->OnCastInterrupted.AddDynamic(this, &UROGameplayAbility::OnCastInterrupted);
			CastComp->StartCasting(VarCast, FixedCastTime, SkillID);
			return; // Wait for cast to complete
		}
	}

	// No cast time or no casting component - execute immediately
	OnCastComplete();
}

bool UROGameplayAbility::CommitAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	FGameplayTagContainer* OptionalRelevantTags)
{
	// Only commit cooldown here, NOT cost.
	// SP deduction happens in OnCastComplete so that interrupted casts don't waste SP.
	// Calling Super::CommitAbility would apply cost GE, causing double SP deduction.
	if (!CommitAbilityCooldown(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UROGameplayAbility::OnCastComplete()
{
	// Deduct SP cost now that cast completed.
	// Validate CachedActorInfo and its weak pointers before use to guard against
	// dangling pointer access if the actor was destroyed during cast.
	if (CachedActorInfo && CachedActorInfo->AbilitySystemComponent.IsValid())
	{
		// Re-check SP before deducting: another skill may have consumed SP during cast,
		// preventing a double-spend that leaves SP negative.
		UAbilitySystemComponent* ASC = CachedActorInfo->AbilitySystemComponent.Get();
		const float SPCost = GetSPCost();
		const float CurrentSP = ASC->GetNumericAttribute(UROAttributeSet::GetSPAttribute());
		if (CurrentSP < SPCost)
		{
			// Not enough SP anymore - cancel the ability effect
			// Unbind from casting component first
			if (CachedActorInfo->AvatarActor.IsValid())
			{
				UROCastingComponent* CastComp = CachedActorInfo->AvatarActor->FindComponentByClass<UROCastingComponent>();
				if (CastComp)
				{
					CastComp->OnCastComplete.RemoveDynamic(this, &UROGameplayAbility::OnCastComplete);
					CastComp->OnCastInterrupted.RemoveDynamic(this, &UROGameplayAbility::OnCastInterrupted);
				}
			}
			EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, true);
			return;
		}

		DeductSP(CachedActorInfo, SPCost);

		// Unbind from casting component
		if (CachedActorInfo->AvatarActor.IsValid())
		{
			UROCastingComponent* CastComp = CachedActorInfo->AvatarActor->FindComponentByClass<UROCastingComponent>();
			if (CastComp)
			{
				CastComp->OnCastComplete.RemoveDynamic(this, &UROGameplayAbility::OnCastComplete);
				CastComp->OnCastInterrupted.RemoveDynamic(this, &UROGameplayAbility::OnCastInterrupted);
			}
		}
	}

	// Subclasses override OnCastComplete to provide specific skill effects
}

void UROGameplayAbility::OnCastInterrupted()
{
	if (CachedActorInfo && CachedActorInfo->AbilitySystemComponent.IsValid())
	{
		// Unbind from casting component
		if (CachedActorInfo->AvatarActor.IsValid())
		{
			UROCastingComponent* CastComp = CachedActorInfo->AvatarActor->FindComponentByClass<UROCastingComponent>();
			if (CastComp)
			{
				CastComp->OnCastComplete.RemoveDynamic(this, &UROGameplayAbility::OnCastComplete);
				CastComp->OnCastInterrupted.RemoveDynamic(this, &UROGameplayAbility::OnCastInterrupted);
			}
		}
	}

	// No SP deducted on interrupt.
	// Validate CachedActorInfo before passing to EndAbility. If the actor info is
	// no longer valid (e.g., actor destroyed during cast), use CurrentActorInfo
	// from the ability instance instead.
	const FGameplayAbilityActorInfo* ActorInfoToUse = CachedActorInfo;
	if (!ActorInfoToUse || !ActorInfoToUse->AbilitySystemComponent.IsValid())
	{
		ActorInfoToUse = GetCurrentActorInfo();
	}
	EndAbility(CachedHandle, ActorInfoToUse, CachedActivationInfo, true, true);
}

void UROGameplayAbility::DeductSP(const FGameplayAbilityActorInfo* ActorInfo, float Amount)
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid() || Amount <= 0.0f)
	{
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	const UROAttributeSet* AttrSet = ASC->GetSet<UROAttributeSet>();
	if (AttrSet)
	{
		const float CurrentSP = AttrSet->GetSP();
		const float NewSP = FMath::Max(0.0f, CurrentSP - Amount);

		// Set the base value directly so the change persists, replicates, and triggers
		// PreAttributeChange clamping. ApplyModToAttribute with Override only adds an
		// aggregator modifier which doesn't reliably persist across recalculations.
		ASC->SetNumericAttributeBase(UROAttributeSet::GetSPAttribute(), NewSP);

		// Sync the GAS attribute back to the character's replicated CurrentSP property.
		// PostGameplayEffectExecute handles this for GE-driven changes, but
		// SetNumericAttributeBase bypasses that path.
		if (AActor* AvatarActor = ASC->GetAvatarActor())
		{
			if (AROCharacterBase* Character = Cast<AROCharacterBase>(AvatarActor))
			{
				Character->CurrentSP = FMath::RoundToInt32(AttrSet->GetSP());
			}
		}
	}
}

bool UROGameplayAbility::HasStatusEffectTag(const FGameplayAbilityActorInfo* ActorInfo, FName TagName) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, false);
	if (Tag.IsValid())
	{
		return ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(Tag);
	}
	return false;
}

const UROAttributeSet* UROGameplayAbility::GetROAttributeSet(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return nullptr;
	}

	return ActorInfo->AbilitySystemComponent->GetSet<UROAttributeSet>();
}
