// Copyright Ragna-TH Project. All Rights Reserved.

#include "RODamageGameplayEffect.h"
#include "RODamageExecution.h"

URODamageGameplayEffect::URODamageGameplayEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Add the RO damage execution calculation so the full damage formula runs
	FGameplayEffectExecutionDefinition ExecDef;
	ExecDef.CalculationClass = URODamageExecution::StaticClass();
	Executions.Add(ExecDef);
}
