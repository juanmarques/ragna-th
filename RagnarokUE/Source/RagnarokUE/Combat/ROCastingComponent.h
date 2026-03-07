// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ROCastingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCastComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCastInterrupted);

/**
 * UROCastingComponent
 * Manages skill cast bars with variable and fixed cast time support.
 * Replicates casting state for other clients to display cast bars.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RAGNAROKUE_API UROCastingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UROCastingComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Properties ---

	/** Whether the character is currently casting a skill. */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Casting")
	bool bIsCasting;

	/** Elapsed cast time in seconds. */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Casting")
	float CurrentCastTime;

	/** Total cast time required (variable + fixed). */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Casting")
	float TotalCastTime;

	/** The variable portion of cast time (can be interrupted by movement/damage). */
	UPROPERTY(BlueprintReadOnly, Category = "Casting")
	float VariableCastTime;

	/** The fixed portion of cast time (cannot be reduced by stats). */
	UPROPERTY(BlueprintReadOnly, Category = "Casting")
	float FixedCastTime;

	/** Skill ID currently being cast. */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Casting")
	int32 CastingSkillID;

	// --- Delegates ---

	/** Fired when casting completes successfully. */
	UPROPERTY(BlueprintAssignable, Category = "Casting|Events")
	FOnCastComplete OnCastComplete;

	/** Fired when casting is interrupted. */
	UPROPERTY(BlueprintAssignable, Category = "Casting|Events")
	FOnCastInterrupted OnCastInterrupted;

	// --- Functions ---

	/**
	 * Begin casting a skill with the given cast times.
	 * @param InVariableCastTime - Cast time that can be interrupted
	 * @param InFixedCastTime - Cast time that cannot be reduced
	 * @param InSkillID - The skill being cast
	 */
	UFUNCTION(BlueprintCallable, Category = "Casting")
	void StartCasting(float InVariableCastTime, float InFixedCastTime, int32 InSkillID);

	/**
	 * Interrupt the current cast (called when hit or moved during variable cast phase).
	 */
	UFUNCTION(BlueprintCallable, Category = "Casting")
	void InterruptCasting();

	/**
	 * Get the casting progress as a 0-1 ratio.
	 */
	UFUNCTION(BlueprintCallable, Category = "Casting")
	float GetCastProgress() const;

	/**
	 * Check if currently in the variable (interruptible) phase of casting.
	 */
	UFUNCTION(BlueprintCallable, Category = "Casting")
	bool IsInVariableCastPhase() const;

protected:
	virtual void BeginPlay() override;

private:
	/** Complete the cast and broadcast delegate. */
	void CompleteCast();

	/** Reset all casting state. */
	void ResetCastState();
};
