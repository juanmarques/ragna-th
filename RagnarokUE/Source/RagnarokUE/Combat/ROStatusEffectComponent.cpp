// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROStatusEffectComponent.h"
#include "AbilitySystemComponent.h"
#include "RagnarokUE/Skills/ROAttributeSet.h"
#include "Net/UnrealNetwork.h"

UROStatusEffectComponent::UROStatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetIsReplicatedByDefault(true);
}

void UROStatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UROStatusEffectComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UROStatusEffectComponent, ActiveEffectsArray);
}

void UROStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Only process on server
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	// Decrement durations and remove expired effects
	for (int32 i = ActiveEffectsArray.Num() - 1; i >= 0; --i)
	{
		ActiveEffectsArray[i].RemainingDuration -= DeltaTime;

		if (ActiveEffectsArray[i].RemainingDuration <= 0.0f)
		{
			EROStatusEffect ExpiredEffect = ActiveEffectsArray[i].Effect;
			RemoveStatusTag(ExpiredEffect);
			ActiveEffectsArray.RemoveAt(i);
			OnStatusEffectChanged.Broadcast(ExpiredEffect, false);
		}
	}

	// Process periodic effects
	ProcessPeriodicEffects(DeltaTime);
}

bool UROStatusEffectComponent::ApplyStatusEffect(EROStatusEffect Effect, float Duration, int32 Level, float Chance)
{
	// Roll chance
	if (Chance < 100.0f)
	{
		const float Roll = FMath::FRandRange(0.0f, 100.0f);
		if (Roll > Chance)
		{
			return false;
		}
	}

	// Check if already active - refresh duration if so
	int32 ExistingIdx = FindEffectIndex(Effect);
	if (ExistingIdx != INDEX_NONE)
	{
		// Refresh: take the longer remaining duration
		ActiveEffectsArray[ExistingIdx].RemainingDuration = FMath::Max(ActiveEffectsArray[ExistingIdx].RemainingDuration, Duration);
		ActiveEffectsArray[ExistingIdx].TotalDuration = Duration;
		ActiveEffectsArray[ExistingIdx].Level = FMath::Max(ActiveEffectsArray[ExistingIdx].Level, Level);
		return true;
	}

	// Apply new effect
	FROActiveStatusEffect NewEffect;
	NewEffect.Effect = Effect;
	NewEffect.RemainingDuration = Duration;
	NewEffect.TotalDuration = Duration;
	NewEffect.Level = Level;
	NewEffect.TickAccumulator = 0.0f;
	ActiveEffectsArray.Add(NewEffect);

	ApplyStatusTag(Effect);
	OnStatusEffectChanged.Broadcast(Effect, true);

	return true;
}

void UROStatusEffectComponent::RemoveStatusEffect(EROStatusEffect Effect)
{
	int32 Idx = FindEffectIndex(Effect);
	if (Idx != INDEX_NONE)
	{
		RemoveStatusTag(Effect);
		ActiveEffectsArray.RemoveAt(Idx);
		OnStatusEffectChanged.Broadcast(Effect, false);
	}
}

void UROStatusEffectComponent::RemoveAllStatusEffects()
{
	for (const FROActiveStatusEffect& ActiveEffect : ActiveEffectsArray)
	{
		RemoveStatusTag(ActiveEffect.Effect);
		OnStatusEffectChanged.Broadcast(ActiveEffect.Effect, false);
	}
	ActiveEffectsArray.Empty();
}

bool UROStatusEffectComponent::HasStatusEffect(EROStatusEffect Effect) const
{
	return FindEffectIndex(Effect) != INDEX_NONE;
}

float UROStatusEffectComponent::GetRemainingDuration(EROStatusEffect Effect) const
{
	int32 Idx = FindEffectIndex(Effect);
	if (Idx != INDEX_NONE)
	{
		return ActiveEffectsArray[Idx].RemainingDuration;
	}
	return 0.0f;
}

int32 UROStatusEffectComponent::GetEffectLevel(EROStatusEffect Effect) const
{
	int32 Idx = FindEffectIndex(Effect);
	if (Idx != INDEX_NONE)
	{
		return ActiveEffectsArray[Idx].Level;
	}
	return 0;
}

bool UROStatusEffectComponent::CanAct() const
{
	// Cannot act while stunned, frozen, or stone cursed
	return !HasStatusEffect(EROStatusEffect::Stun)
		&& !HasStatusEffect(EROStatusEffect::Freeze)
		&& !HasStatusEffect(EROStatusEffect::Stone)
		&& !HasStatusEffect(EROStatusEffect::Sleep);
}

bool UROStatusEffectComponent::CanCast() const
{
	// Cannot cast while silenced or unable to act
	return CanAct() && !HasStatusEffect(EROStatusEffect::Silence);
}

bool UROStatusEffectComponent::CanMove() const
{
	// Cannot move while unable to act (stun/freeze/stone/sleep)
	return CanAct();
}

TArray<FROActiveStatusEffect> UROStatusEffectComponent::GetAllActiveEffects() const
{
	return ActiveEffectsArray;
}

void UROStatusEffectComponent::ProcessPeriodicEffects(float DeltaTime)
{
	for (int32 i = 0; i < ActiveEffectsArray.Num(); ++i)
	{
		FROActiveStatusEffect& ActiveEffect = ActiveEffectsArray[i];
		ActiveEffect.TickAccumulator += DeltaTime;

		if (ActiveEffect.TickAccumulator >= StatusTickInterval)
		{
			ActiveEffect.TickAccumulator -= StatusTickInterval;

			AActor* Owner = GetOwner();
			if (!Owner)
			{
				continue;
			}

			UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();

			switch (ActiveEffect.Effect)
			{
			case EROStatusEffect::Poison:
			{
				// Poison: drain HP over time (base damage = MaxHP * 1% per tick)
				if (ASC)
				{
					const UROAttributeSet* AttrSet = ASC->GetSet<UROAttributeSet>();
					if (AttrSet)
					{
						const float PoisonDamage = AttrSet->GetMaxHP() * 0.01f * static_cast<float>(ActiveEffect.Level);
						const float NewHP = FMath::Max(1.0f, AttrSet->GetHP() - PoisonDamage); // Poison cannot kill
						ASC->ApplyModToAttribute(UROAttributeSet::GetHPAttribute(), EGameplayModOp::Override, NewHP);
					}
				}
				break;
			}
			case EROStatusEffect::DeadlyPoison:
			{
				// Deadly Poison: stronger HP drain (MaxHP * 2% per tick, CAN kill)
				if (ASC)
				{
					const UROAttributeSet* AttrSet = ASC->GetSet<UROAttributeSet>();
					if (AttrSet)
					{
						const float PoisonDamage = AttrSet->GetMaxHP() * 0.02f * static_cast<float>(ActiveEffect.Level);
						const float NewHP = FMath::Max(0.0f, AttrSet->GetHP() - PoisonDamage);
						ASC->ApplyModToAttribute(UROAttributeSet::GetHPAttribute(), EGameplayModOp::Override, NewHP);
					}
				}
				break;
			}
			case EROStatusEffect::Bleeding:
			{
				// Bleeding: prevents natural HP regen and drains small amount
				if (ASC)
				{
					const UROAttributeSet* AttrSet = ASC->GetSet<UROAttributeSet>();
					if (AttrSet)
					{
						const float BleedDamage = AttrSet->GetMaxHP() * 0.005f;
						const float NewHP = FMath::Max(1.0f, AttrSet->GetHP() - BleedDamage);
						ASC->ApplyModToAttribute(UROAttributeSet::GetHPAttribute(), EGameplayModOp::Override, NewHP);
					}
				}
				break;
			}
			default:
				// Other status effects don't have periodic damage
				break;
			}
		}
	}
}

void UROStatusEffectComponent::ApplyStatusTag(EROStatusEffect Effect)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
	if (ASC)
	{
		FName TagName = GetTagNameForEffect(Effect);
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, false);
		if (Tag.IsValid())
		{
			ASC->AddLooseGameplayTag(Tag);
		}
	}
}

void UROStatusEffectComponent::RemoveStatusTag(EROStatusEffect Effect)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
	if (ASC)
	{
		FName TagName = GetTagNameForEffect(Effect);
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, false);
		if (Tag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(Tag);
		}
	}
}

FName UROStatusEffectComponent::GetTagNameForEffect(EROStatusEffect Effect)
{
	switch (Effect)
	{
	case EROStatusEffect::Stun:         return FName("Status.Stun");
	case EROStatusEffect::Poison:       return FName("Status.Poison");
	case EROStatusEffect::Freeze:       return FName("Status.Freeze");
	case EROStatusEffect::Stone:        return FName("Status.Stone");
	case EROStatusEffect::Sleep:        return FName("Status.Sleep");
	case EROStatusEffect::Blind:        return FName("Status.Blind");
	case EROStatusEffect::Silence:      return FName("Status.Silence");
	case EROStatusEffect::Curse:        return FName("Status.Curse");
	case EROStatusEffect::Bleeding:     return FName("Status.Bleeding");
	case EROStatusEffect::Confusion:    return FName("Status.Confusion");
	case EROStatusEffect::DeadlyPoison: return FName("Status.DeadlyPoison");
	case EROStatusEffect::Fear:         return FName("Status.Fear");
	case EROStatusEffect::Hallucination:return FName("Status.Hallucination");
	default:                            return FName("Status.Unknown");
	}
}

int32 UROStatusEffectComponent::FindEffectIndex(EROStatusEffect Effect) const
{
	for (int32 i = 0; i < ActiveEffectsArray.Num(); ++i)
	{
		if (ActiveEffectsArray[i].Effect == Effect)
		{
			return i;
		}
	}
	return INDEX_NONE;
}
