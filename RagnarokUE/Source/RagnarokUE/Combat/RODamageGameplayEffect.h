// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "RODamageGameplayEffect.generated.h"

/**
 * URODamageGameplayEffect
 * C++-configured GameplayEffect that runs URODamageExecution.
 * Used by all damage-dealing abilities so the RO damage formula executes
 * when the GE is applied to a target.
 *
 * DurationPolicy: Instant
 * Executions: URODamageExecution
 */
UCLASS()
class RAGNAROKUE_API URODamageGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	URODamageGameplayEffect();
};
