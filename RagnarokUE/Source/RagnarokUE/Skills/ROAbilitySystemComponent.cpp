// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROAbilitySystemComponent.h"
#include "ROGameplayAbility.h"
#include "Abilities/ROAbility_Bash.h"
#include "Abilities/ROAbility_MagnumBreak.h"
#include "Abilities/ROAbility_FireBolt.h"
#include "Abilities/ROAbility_ColdBolt.h"
#include "Abilities/ROAbility_LightningBolt.h"
#include "Abilities/ROAbility_Heal.h"
#include "Abilities/ROAbility_DoubleStrafe.h"
#include "Abilities/ROAbility_Hiding.h"
#include "Abilities/ROAbility_Discount.h"

UROAbilitySystemComponent::UROAbilitySystemComponent()
{
	// Enable replication by default for multiplayer
	SetIsReplicatedByDefault(true);
}

void UROAbilitySystemComponent::GrantAbilitiesForJob(EROJobClass Job)
{
	// Remove any existing abilities for this job first to avoid duplicates
	RemoveAbilitiesForJob(Job);

	TArray<TSubclassOf<UROGameplayAbility>> AbilityClasses = GetAbilityClassesForJob(Job);
	TArray<FGameplayAbilitySpecHandle> Handles;

	for (const TSubclassOf<UROGameplayAbility>& AbilityClass : AbilityClasses)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, GetOwner());
			FGameplayAbilitySpecHandle Handle = GiveAbility(Spec);
			if (Handle.IsValid())
			{
				Handles.Add(Handle);
			}
		}
	}

	if (Handles.Num() > 0)
	{
		GrantedAbilityHandles.Add(Job, Handles);
	}
}

void UROAbilitySystemComponent::RemoveAbilitiesForJob(EROJobClass Job)
{
	TArray<FGameplayAbilitySpecHandle>* Handles = GrantedAbilityHandles.Find(Job);
	if (Handles)
	{
		for (const FGameplayAbilitySpecHandle& Handle : *Handles)
		{
			if (Handle.IsValid())
			{
				ClearAbility(Handle);
			}
		}
		GrantedAbilityHandles.Remove(Job);
	}
}

FGameplayAbilitySpecHandle UROAbilitySystemComponent::GrantAbilityOfClass(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level)
{
	if (!AbilityClass)
	{
		return FGameplayAbilitySpecHandle();
	}

	FGameplayAbilitySpec Spec(AbilityClass, Level, INDEX_NONE, GetOwner());
	return GiveAbility(Spec);
}

TArray<FGameplayAbilitySpecHandle> UROAbilitySystemComponent::GetGrantedAbilitiesForJob(EROJobClass Job) const
{
	const TArray<FGameplayAbilitySpecHandle>* Handles = GrantedAbilityHandles.Find(Job);
	if (Handles)
	{
		return *Handles;
	}
	return TArray<FGameplayAbilitySpecHandle>();
}

TArray<TSubclassOf<UROGameplayAbility>> UROAbilitySystemComponent::GetAbilityClassesForJob(EROJobClass Job) const
{
	TArray<TSubclassOf<UROGameplayAbility>> Abilities;

	switch (Job)
	{
	case EROJobClass::Swordsman:
	case EROJobClass::HighSwordsman:
		Abilities.Add(UROAbility_Bash::StaticClass());
		Abilities.Add(UROAbility_MagnumBreak::StaticClass());
		break;

	case EROJobClass::Magician:
	case EROJobClass::HighMagician:
		Abilities.Add(UROAbility_FireBolt::StaticClass());
		Abilities.Add(UROAbility_ColdBolt::StaticClass());
		Abilities.Add(UROAbility_LightningBolt::StaticClass());
		break;

	case EROJobClass::Acolyte:
	case EROJobClass::HighAcolyte:
		Abilities.Add(UROAbility_Heal::StaticClass());
		break;

	case EROJobClass::Archer:
	case EROJobClass::HighArcher:
		Abilities.Add(UROAbility_DoubleStrafe::StaticClass());
		break;

	case EROJobClass::Thief:
	case EROJobClass::HighThief:
		Abilities.Add(UROAbility_Hiding::StaticClass());
		break;

	case EROJobClass::Merchant:
	case EROJobClass::HighMerchant:
		Abilities.Add(UROAbility_Discount::StaticClass());
		break;

	// 2nd classes inherit their 1st class skills plus new ones
	case EROJobClass::Knight:
	case EROJobClass::LordKnight:
		Abilities.Add(UROAbility_Bash::StaticClass());
		Abilities.Add(UROAbility_MagnumBreak::StaticClass());
		break;

	case EROJobClass::Crusader:
	case EROJobClass::Paladin:
		Abilities.Add(UROAbility_Bash::StaticClass());
		Abilities.Add(UROAbility_MagnumBreak::StaticClass());
		break;

	case EROJobClass::Wizard:
	case EROJobClass::HighWizard:
		Abilities.Add(UROAbility_FireBolt::StaticClass());
		Abilities.Add(UROAbility_ColdBolt::StaticClass());
		Abilities.Add(UROAbility_LightningBolt::StaticClass());
		break;

	case EROJobClass::Sage:
	case EROJobClass::Professor:
		Abilities.Add(UROAbility_FireBolt::StaticClass());
		Abilities.Add(UROAbility_ColdBolt::StaticClass());
		Abilities.Add(UROAbility_LightningBolt::StaticClass());
		break;

	case EROJobClass::Hunter:
	case EROJobClass::Sniper:
		Abilities.Add(UROAbility_DoubleStrafe::StaticClass());
		break;

	case EROJobClass::Priest:
	case EROJobClass::HighPriest:
		Abilities.Add(UROAbility_Heal::StaticClass());
		break;

	case EROJobClass::Monk:
	case EROJobClass::Champion:
		Abilities.Add(UROAbility_Heal::StaticClass());
		break;

	case EROJobClass::Assassin:
	case EROJobClass::AssassinCross:
		Abilities.Add(UROAbility_Hiding::StaticClass());
		break;

	case EROJobClass::Rogue:
	case EROJobClass::Stalker:
		Abilities.Add(UROAbility_Hiding::StaticClass());
		break;

	case EROJobClass::Blacksmith:
	case EROJobClass::Whitesmith:
		Abilities.Add(UROAbility_Discount::StaticClass());
		break;

	case EROJobClass::Alchemist:
	case EROJobClass::Creator:
		Abilities.Add(UROAbility_Discount::StaticClass());
		break;

	default:
		// Novice and other classes get no special abilities by default
		break;
	}

	return Abilities;
}
