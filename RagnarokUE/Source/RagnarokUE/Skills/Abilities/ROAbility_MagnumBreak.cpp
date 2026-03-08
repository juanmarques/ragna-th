// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROAbility_MagnumBreak.h"
#include "AbilitySystemComponent.h"
#include "RagnarokUE/Skills/ROAttributeSet.h"
#include "RagnarokUE/Combat/ROStatusEffectComponent.h"
#include "RagnarokUE/Combat/RODamageGameplayEffect.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

UROAbility_MagnumBreak::UROAbility_MagnumBreak()
{
	SkillID = 7;
	SkillName = FName("SM_MAGNUM");
	MaxSkillLevel = 10;
	SkillLevel = 1;
	SkillElement = EROElement::Fire;

	// SP cost: 30 flat
	SPCostBase = 30.0f;
	SPCostPerLevel = 0.0f;

	// Short animation delay, no cast bar
	VariableCastTimeBase = 0.0f;
	FixedCastTime = 0.0f;
	CooldownDuration = 2.0f;

	AoERadius = 500.0f;
	PushbackForce = 800.0f;
	FireEndowDuration = 10.0f;
}

void UROAbility_MagnumBreak::OnCastComplete()
{
	Super::OnCastComplete();

	if (!CachedActorInfo || !CachedActorInfo->AvatarActor.IsValid())
	{
		EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
		return;
	}

	if (!CachedActorInfo->AbilitySystemComponent.IsValid())
	{
		EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
		return;
	}

	UAbilitySystemComponent* SourceASC = CachedActorInfo->AbilitySystemComponent.Get();
	AActor* AvatarActor = CachedActorInfo->AvatarActor.Get();
	const FVector Origin = AvatarActor->GetActorLocation();

	// Find all enemies within AoE radius
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarActor);

	if (GetWorld())
	{
		GetWorld()->OverlapMultiByChannel(
			OverlapResults,
			Origin,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(AoERadius),
			QueryParams
		);
	}

	const float DamageMod = GetDamageModifier();

	// Apply damage and pushback to each hit target
	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* HitActor = Result.GetActor();
		if (!HitActor || HitActor == AvatarActor)
		{
			continue;
		}

		// Apply pushback
		const FVector PushDirection = (HitActor->GetActorLocation() - Origin).GetSafeNormal();
		ACharacter* HitCharacter = Cast<ACharacter>(HitActor);
		if (HitCharacter)
		{
			HitCharacter->LaunchCharacter(PushDirection * PushbackForce, true, true);
		}

		// Apply fire damage via GAS
		UAbilitySystemComponent* TargetASC = HitActor->FindComponentByClass<UAbilitySystemComponent>();
		if (TargetASC)
		{
			FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(
				URODamageGameplayEffect::StaticClass(), SkillLevel);

			if (DamageSpec.IsValid())
			{
				FGameplayTag SkillModTag = FGameplayTag::RequestGameplayTag(FName("Data.SkillMod"), false);
				if (SkillModTag.IsValid())
				{
					DamageSpec.Data->SetSetByCallerMagnitude(SkillModTag, DamageMod);
				}

				FGameplayTag DamageTypeTag = FGameplayTag::RequestGameplayTag(FName("Data.DamageType"), false);
				if (DamageTypeTag.IsValid())
				{
					DamageSpec.Data->SetSetByCallerMagnitude(DamageTypeTag, 0.0f); // Physical with fire element
				}

				FGameplayTag ElementModTag = FGameplayTag::RequestGameplayTag(FName("Data.ElementMod"), false);
				if (ElementModTag.IsValid())
				{
					// Fire element attack - modifier depends on target's element
					DamageSpec.Data->SetSetByCallerMagnitude(ElementModTag, 1.0f); // Default; actual calc via execution
				}

				// Apply the damage effect to the target
				SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);
			}
		}
	}

	// Apply fire element endow to self
	ApplyFireEndow();

	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

float UROAbility_MagnumBreak::GetDamageModifier() const
{
	// Magnum Break damage: ATK * (100% + 20% * SkillLevel) + fire bonus
	return 1.0f + 0.2f * static_cast<float>(SkillLevel);
}

void UROAbility_MagnumBreak::ApplyFireEndow() const
{
	if (!CachedActorInfo || !CachedActorInfo->AbilitySystemComponent.IsValid())
	{
		return;
	}

	// Apply fire endow gameplay tag for 10 seconds
	UAbilitySystemComponent* ASC = CachedActorInfo->AbilitySystemComponent.Get();
	FGameplayTag FireEndowTag = FGameplayTag::RequestGameplayTag(FName("Buff.Element.Fire"), false);
	if (FireEndowTag.IsValid())
	{
		ASC->AddLooseGameplayTag(FireEndowTag);

		// Set a timer to remove the tag after duration
		// Use weak pointer to avoid dangling reference if ASC is destroyed
		if (CachedActorInfo->AvatarActor.IsValid())
		{
			FTimerHandle TimerHandle;
			FTimerDelegate TimerDelegate;
			TWeakObjectPtr<UAbilitySystemComponent> WeakASC(ASC);
			TimerDelegate.BindLambda([WeakASC, FireEndowTag]()
			{
				if (WeakASC.IsValid())
				{
					WeakASC->RemoveLooseGameplayTag(FireEndowTag);
				}
			});

			CachedActorInfo->AvatarActor->GetWorldTimerManager().SetTimer(
				TimerHandle, TimerDelegate, FireEndowDuration, false);
		}
	}
}
