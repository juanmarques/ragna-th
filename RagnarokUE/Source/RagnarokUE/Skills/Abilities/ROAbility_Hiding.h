// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/Skills/ROGameplayAbility.h"
#include "ROAbility_Hiding.generated.h"

/**
 * UROAbility_Hiding
 * Thief skill - Toggle stealth. 10 levels.
 * SP drain over time, higher levels = less SP drain.
 * SP drain per tick: 10 - SkillLevel
 * Can be detected by Detector-type monsters.
 */
UCLASS()
class RAGNAROKUE_API UROAbility_Hiding : public UROGameplayAbility
{
	GENERATED_BODY()

public:
	UROAbility_Hiding();

	/** Tick interval for SP drain in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill|Hiding")
	float SPDrainTickInterval;

protected:
	virtual void OnCastComplete() override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/** Called to end hiding (toggle off or SP depleted). */
	UFUNCTION()
	void EndHiding();

private:
	/** Get SP drain per tick at current level. */
	float GetSPDrainPerTick() const;

	/** Timer handle for SP drain ticks. */
	FTimerHandle SPDrainTimerHandle;

	/** Process a single SP drain tick. */
	void OnSPDrainTick();

	/** Whether hiding is currently active. */
	bool bIsHiding;
};
