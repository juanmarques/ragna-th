// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ROHealGameplayEffect.generated.h"

/**
 * UROHealGameplayEffect
 * C++-configured instant GameplayEffect for healing.
 * Applies a SetByCaller magnitude to IncomingHealing, which is then
 * processed by UROAttributeSet::PostGameplayEffectExecute to add HP.
 */
UCLASS()
class RAGNAROKUE_API UROHealGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UROHealGameplayEffect();
};
