// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

UROAttributeSet::UROAttributeSet()
{
	// Default values matching a level 1 Novice
	InitHP(40.0f);
	InitMaxHP(40.0f);
	InitSP(10.0f);
	InitMaxSP(10.0f);
	InitATK(5.0f);
	InitMATK(5.0f);
	InitDEF(1.0f);
	InitMDEF(1.0f);
	InitHIT(1.0f);
	InitFLEE(1.0f);
	InitASPD(150.0f);
	InitCritRate(1.0f);
	InitPerfectDodge(1.0f);
	InitMoveSpeed(150.0f);
	InitHPRegen(1.0f);
	InitSPRegen(1.0f);
	InitIncomingDamage(0.0f);
	InitIncomingHealing(0.0f);
}

void UROAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp HP to [0, MaxHP]
	if (Attribute == GetHPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHP());
	}
	// Clamp SP to [0, MaxSP]
	else if (Attribute == GetSPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxSP());
	}
	// When MaxHP changes, scale current HP proportionally
	else if (Attribute == GetMaxHPAttribute())
	{
		NewValue = FMath::Max(1.0f, NewValue);
	}
	// When MaxSP changes, ensure minimum of 0
	else if (Attribute == GetMaxSPAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
	// Clamp defensive stats to reasonable values
	else if (Attribute == GetDEFAttribute() || Attribute == GetMDEFAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
	// Move speed cannot go below 0
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
}

void UROAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Handle incoming damage meta attribute
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float LocalDamage = GetIncomingDamage();
		SetIncomingDamage(0.0f);

		if (LocalDamage > 0.0f)
		{
			const float NewHP = FMath::Max(0.0f, GetHP() - LocalDamage);
			SetHP(NewHP);

			// Death check: if HP reaches zero, broadcast a gameplay event
			if (NewHP <= 0.0f)
			{
				if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
				{
					FGameplayEventData EventData;
					EventData.EventMagnitude = LocalDamage;
					if (Data.EffectSpec.GetContext().GetEffectCauser())
					{
						EventData.Instigator = Data.EffectSpec.GetContext().GetEffectCauser();
					}
					EventData.Target = ASC->GetAvatarActor();

					// Send death event tag
					FGameplayTag DeathTag = FGameplayTag::RequestGameplayTag(FName("Status.Dead"), false);
					if (DeathTag.IsValid())
					{
						ASC->HandleGameplayEvent(DeathTag, &EventData);
					}
				}
			}
		}
	}
	// Handle incoming healing meta attribute
	else if (Data.EvaluatedData.Attribute == GetIncomingHealingAttribute())
	{
		const float LocalHealing = GetIncomingHealing();
		SetIncomingHealing(0.0f);

		if (LocalHealing > 0.0f)
		{
			const float NewHP = FMath::Min(GetMaxHP(), GetHP() + LocalHealing);
			SetHP(NewHP);
		}
	}
	// If HP was modified directly, clamp it
	else if (Data.EvaluatedData.Attribute == GetHPAttribute())
	{
		SetHP(FMath::Clamp(GetHP(), 0.0f, GetMaxHP()));
	}
	// If SP was modified directly, clamp it
	else if (Data.EvaluatedData.Attribute == GetSPAttribute())
	{
		SetSP(FMath::Clamp(GetSP(), 0.0f, GetMaxSP()));
	}
}

void UROAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UROAttributeSet, HP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UROAttributeSet, MaxHP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UROAttributeSet, SP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UROAttributeSet, MaxSP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UROAttributeSet, ATK, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UROAttributeSet, MATK, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UROAttributeSet, DEF, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UROAttributeSet, MDEF, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UROAttributeSet, HIT, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UROAttributeSet, FLEE, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UROAttributeSet, ASPD, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UROAttributeSet, CritRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UROAttributeSet, PerfectDodge, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UROAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UROAttributeSet, HPRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UROAttributeSet, SPRegen, COND_None, REPNOTIFY_Always);
}

// --- RepNotify Implementations ---

void UROAttributeSet::OnRep_HP(const FGameplayAttributeData& OldHP)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UROAttributeSet, HP, OldHP);
}

void UROAttributeSet::OnRep_MaxHP(const FGameplayAttributeData& OldMaxHP)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UROAttributeSet, MaxHP, OldMaxHP);
}

void UROAttributeSet::OnRep_SP(const FGameplayAttributeData& OldSP)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UROAttributeSet, SP, OldSP);
}

void UROAttributeSet::OnRep_MaxSP(const FGameplayAttributeData& OldMaxSP)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UROAttributeSet, MaxSP, OldMaxSP);
}

void UROAttributeSet::OnRep_ATK(const FGameplayAttributeData& OldATK)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UROAttributeSet, ATK, OldATK);
}

void UROAttributeSet::OnRep_MATK(const FGameplayAttributeData& OldMATK)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UROAttributeSet, MATK, OldMATK);
}

void UROAttributeSet::OnRep_DEF(const FGameplayAttributeData& OldDEF)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UROAttributeSet, DEF, OldDEF);
}

void UROAttributeSet::OnRep_MDEF(const FGameplayAttributeData& OldMDEF)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UROAttributeSet, MDEF, OldMDEF);
}

void UROAttributeSet::OnRep_HIT(const FGameplayAttributeData& OldHIT)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UROAttributeSet, HIT, OldHIT);
}

void UROAttributeSet::OnRep_FLEE(const FGameplayAttributeData& OldFLEE)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UROAttributeSet, FLEE, OldFLEE);
}

void UROAttributeSet::OnRep_ASPD(const FGameplayAttributeData& OldASPD)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UROAttributeSet, ASPD, OldASPD);
}

void UROAttributeSet::OnRep_CritRate(const FGameplayAttributeData& OldCritRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UROAttributeSet, CritRate, OldCritRate);
}

void UROAttributeSet::OnRep_PerfectDodge(const FGameplayAttributeData& OldPerfectDodge)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UROAttributeSet, PerfectDodge, OldPerfectDodge);
}

void UROAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UROAttributeSet, MoveSpeed, OldMoveSpeed);
}

void UROAttributeSet::OnRep_HPRegen(const FGameplayAttributeData& OldHPRegen)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UROAttributeSet, HPRegen, OldHPRegen);
}

void UROAttributeSet::OnRep_SPRegen(const FGameplayAttributeData& OldSPRegen)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UROAttributeSet, SPRegen, OldSPRegen);
}

void UROAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float MinValue, float MaxValue, float& NewValue) const
{
	NewValue = FMath::Clamp(NewValue, MinValue, MaxValue);
}
