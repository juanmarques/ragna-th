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

	// OPT1 mutual exclusion: remove any existing OPT1 effect before applying a new one
	if (IsOPT1Effect(Effect))
	{
		for (int32 i = ActiveEffectsArray.Num() - 1; i >= 0; --i)
		{
			if (IsOPT1Effect(ActiveEffectsArray[i].Effect) && ActiveEffectsArray[i].Effect != Effect)
			{
				EROStatusEffect OldEffect = ActiveEffectsArray[i].Effect;
				RemoveStatusTag(OldEffect);
				ActiveEffectsArray.RemoveAt(i);
				OnStatusEffectChanged.Broadcast(OldEffect, false);
			}
		}
	}

	// Check if already active - refresh duration if so
	int32 ExistingIdx = FindEffectIndex(Effect);
	if (ExistingIdx != INDEX_NONE)
	{
		// Refresh: reset to new full duration
		ActiveEffectsArray[ExistingIdx].RemainingDuration = Duration;
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
	// Cannot act while stunned, frozen, sleeping, or stone cursed (Phase 2 only)
	if (HasStatusEffect(EROStatusEffect::Stun)
		|| HasStatusEffect(EROStatusEffect::Freeze)
		|| HasStatusEffect(EROStatusEffect::Sleep))
	{
		return false;
	}

	// Stone only blocks actions in Phase 2 (hardened)
	int32 StoneIdx = FindEffectIndex(EROStatusEffect::Stone);
	if (StoneIdx != INDEX_NONE && ActiveEffectsArray[StoneIdx].bStoneCurseHardened)
	{
		return false;
	}

	return true;
}

bool UROStatusEffectComponent::CanAttack() const
{
	// Cannot attack while stunned, frozen, sleeping, or stone cursed (both phases)
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
	return CanAct() && !HasStatusEffect(EROStatusEffect::Fear);
}

TArray<FROActiveStatusEffect> UROStatusEffectComponent::GetAllActiveEffects() const
{
	return ActiveEffectsArray;
}

bool UROStatusEffectComponent::GetElementOverride(EROElement& OutElement, EROElementLevel& OutLevel) const
{
	// Stone Curse Phase 2: element becomes Earth Lv1
	int32 StoneIdx = FindEffectIndex(EROStatusEffect::Stone);
	if (StoneIdx != INDEX_NONE && ActiveEffectsArray[StoneIdx].bStoneCurseHardened)
	{
		OutElement = EROElement::Earth;
		OutLevel = EROElementLevel::Level1;
		return true;
	}

	// Freeze: element becomes Water Lv1
	if (HasStatusEffect(EROStatusEffect::Freeze))
	{
		OutElement = EROElement::Water;
		OutLevel = EROElementLevel::Level1;
		return true;
	}

	return false;
}

void UROStatusEffectComponent::ProcessPeriodicEffects(float DeltaTime)
{
	for (int32 i = 0; i < ActiveEffectsArray.Num(); ++i)
	{
		FROActiveStatusEffect& ActiveEffect = ActiveEffectsArray[i];
		ActiveEffect.TickAccumulator += DeltaTime;

		while (ActiveEffect.TickAccumulator >= StatusTickInterval)
		{
			ActiveEffect.TickAccumulator -= StatusTickInterval;

			AActor* Owner = GetOwner();
			if (!Owner)
			{
				break;
			}

			UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();

			switch (ActiveEffect.Effect)
			{
			case EROStatusEffect::Stone:
			{
				// Stone Curse two-phase mechanic
				ActiveEffect.StonePhaseTimer += StatusTickInterval;
				if (!ActiveEffect.bStoneCurseHardened)
				{
					// Phase 1 (soft stone): lasts 3 seconds, can move but can't attack/cast
					if (ActiveEffect.StonePhaseTimer >= 3.0f)
					{
						ActiveEffect.bStoneCurseHardened = true;
						ActiveEffect.StonePhaseTimer = 0.0f;
						// Element overridden to Earth Lv1 via GetElementOverride()
					}
				}
				else
				{
					// Phase 2 (hard stone): drain 1% HP every 5 seconds
					if (ActiveEffect.StonePhaseTimer >= 5.0f)
					{
						ActiveEffect.StonePhaseTimer -= 5.0f;
						if (ASC)
						{
							const UROAttributeSet* AttrSet = ASC->GetSet<UROAttributeSet>();
							if (AttrSet)
							{
								const float StoneDamage = AttrSet->GetMaxHP() * 0.01f;
								const float NewHP = FMath::Max(1.0f, AttrSet->GetHP() - StoneDamage);
								ASC->ApplyModToAttribute(UROAttributeSet::GetHPAttribute(), EGameplayModOp::Override, NewHP);
							}
						}
					}
				}
				break;
			}
			case EROStatusEffect::Poison:
			{
				// Pre-renewal Poison: MaxHP * 1.5% + 2 per tick, cannot drop below 25% MaxHP
				if (ASC)
				{
					const UROAttributeSet* AttrSet = ASC->GetSet<UROAttributeSet>();
					if (AttrSet)
					{
						const float PoisonDamage = AttrSet->GetMaxHP() * 0.015f + 2.0f;
						const float MinHP = AttrSet->GetMaxHP() * 0.25f;
						const float NewHP = FMath::Max(MinHP, AttrSet->GetHP() - PoisonDamage);
						ASC->ApplyModToAttribute(UROAttributeSet::GetHPAttribute(), EGameplayModOp::Override, NewHP);
					}
				}
				break;
			}
			case EROStatusEffect::DeadlyPoison:
			{
				// Pre-renewal Deadly Poison: MaxHP * 4% per tick, CAN kill
				if (ASC)
				{
					const UROAttributeSet* AttrSet = ASC->GetSet<UROAttributeSet>();
					if (AttrSet)
					{
						const float PoisonDamage = AttrSet->GetMaxHP() * 0.04f;
						const float NewHP = FMath::Max(0.0f, AttrSet->GetHP() - PoisonDamage);
						ASC->ApplyModToAttribute(UROAttributeSet::GetHPAttribute(), EGameplayModOp::Override, NewHP);
					}
				}
				break;
			}
			case EROStatusEffect::Bleeding:
			{
				// Bleeding: HP drain + blocks HP/SP natural regen (regen blocked via Status.Bleeding tag)
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
			case EROStatusEffect::Curse:
			{
				// Curse: LUK→0 and movement speed -10% (applied once on application, handled via tag)
				// Periodic: no tick damage, but we enforce effects
				break;
			}
			case EROStatusEffect::Confusion:
			{
				// Confusion: randomize movement direction (handled by movement component checking tag)
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

bool UROStatusEffectComponent::IsOPT1Effect(EROStatusEffect Effect)
{
	return Effect == EROStatusEffect::Stun
		|| Effect == EROStatusEffect::Freeze
		|| Effect == EROStatusEffect::Stone
		|| Effect == EROStatusEffect::Sleep;
}

bool UROStatusEffectComponent::ApplyStatusEffectWithResist(EROStatusEffect Effect, float Duration, int32 Level,
	float BaseChance, int32 TargetVIT, int32 TargetINT, int32 TargetLUK, int32 TargetBaseLevel)
{
	float Resistance = 0.0f;
	switch (Effect)
	{
	case EROStatusEffect::Stun:
		Resistance = TargetVIT * 1.0f; // VIT reduces stun chance
		break;
	case EROStatusEffect::Freeze:
	case EROStatusEffect::Stone:
		Resistance = TargetINT * 0.5f + TargetVIT * 0.3f; // MDEF-like resistance
		break;
	case EROStatusEffect::Sleep:
		Resistance = TargetINT * 1.0f;
		break;
	case EROStatusEffect::Blind:
		Resistance = TargetINT * 0.5f + TargetVIT * 0.3f;
		break;
	case EROStatusEffect::Silence:
		Resistance = TargetVIT * 0.5f + TargetINT * 0.3f;
		break;
	case EROStatusEffect::Curse:
		Resistance = TargetLUK * 1.0f;
		break;
	case EROStatusEffect::Poison:
	case EROStatusEffect::DeadlyPoison:
		Resistance = TargetVIT * 0.5f + TargetLUK * 0.3f;
		break;
	case EROStatusEffect::Bleeding:
		Resistance = TargetVIT * 0.5f + TargetLUK * 0.5f;
		break;
	case EROStatusEffect::Confusion:
		Resistance = TargetINT * 0.5f + TargetLUK * 0.5f;
		break;
	default:
		Resistance = 0.0f;
		break;
	}
	float AdjustedChance = FMath::Max(5.0f, BaseChance - Resistance);
	return ApplyStatusEffect(Effect, Duration, Level, AdjustedChance);
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
