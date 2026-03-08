// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROGameplayAbility.h"
#include "ROAttributeSet.h"
#include "RagnarokUE/Combat/ROCastingComponent.h"
#include "RagnarokUE/Character/ROStatsComponent.h"
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

	// Pre-renewal RO formula: CastTime * (1 - DEX/150)
	// At 150 DEX, cast time is fully reduced to 0 (instant cast)
	const float Reduction = FMath::Clamp(static_cast<float>(DEX) / 150.0f, 0.0f, 1.0f);
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
	if (!Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		return false;
	}

	// SP deduction happens after cast completes, not on commit
	// This allows cast interruption to not waste SP
	return true;
}

void UROGameplayAbility::OnCastComplete()
{
	// Deduct SP cost now that cast completed
	if (CachedActorInfo)
	{
		DeductSP(CachedActorInfo, GetSPCost());

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
	if (CachedActorInfo)
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

	// No SP deducted on interrupt
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, true);
}

void UROGameplayAbility::DeductSP(const FGameplayAbilityActorInfo* ActorInfo, float Amount)
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	const UROAttributeSet* AttrSet = ASC->GetSet<UROAttributeSet>();
	if (AttrSet)
	{
		const float CurrentSP = AttrSet->GetSP();
		const float NewSP = FMath::Max(0.0f, CurrentSP - Amount);

		// Apply SP change via a direct modifier
		ASC->ApplyModToAttribute(UROAttributeSet::GetSPAttribute(), EGameplayModOp::Override, NewSP);
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
