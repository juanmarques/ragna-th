// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "ROAbilitySystemComponent.generated.h"

class UROGameplayAbility;

/**
 * UROAbilitySystemComponent
 * Custom Ability System Component for Ragnarok Online characters.
 * Manages granting and revoking abilities based on job class.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RAGNAROKUE_API UROAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UROAbilitySystemComponent();

	/**
	 * Grant all abilities associated with a specific job class.
	 * Stores ability handles for later removal.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Abilities")
	void GrantAbilitiesForJob(EROJobClass Job);

	/**
	 * Remove all abilities previously granted for a specific job class.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Abilities")
	void RemoveAbilitiesForJob(EROJobClass Job);

	/**
	 * Grant a single ability at a given level. Returns the handle.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Abilities")
	FGameplayAbilitySpecHandle GrantAbilityOfClass(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level = 1);

	/**
	 * Get all granted ability handles for a job class.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Abilities")
	TArray<FGameplayAbilitySpecHandle> GetGrantedAbilitiesForJob(EROJobClass Job) const;

protected:
	/**
	 * Returns the list of ability classes associated with a given job.
	 * Override in Blueprint or extend in C++ to add more job skill mappings.
	 */
	virtual TArray<TSubclassOf<UROGameplayAbility>> GetAbilityClassesForJob(EROJobClass Job) const;

private:
	/** Map of job class -> array of granted ability handles */
	TMap<EROJobClass, TArray<FGameplayAbilitySpecHandle>> GrantedAbilityHandles;
};
