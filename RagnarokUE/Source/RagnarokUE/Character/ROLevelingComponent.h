// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "ROLevelingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBaseLevelUp, int32, NewBaseLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJobLevelUp, int32, NewJobLevel);

/**
 * UROLevelingComponent
 * Manages the dual leveling system (Base Level + Job Level) from Ragnarok Online.
 * Handles experience gain, level-up logic, and stat/skill point distribution.
 */
UCLASS(ClassGroup=(RagnarokUE), meta=(BlueprintSpawnableComponent))
class RAGNAROKUE_API UROLevelingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UROLevelingComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	// ---- Replicated Properties ----

	UPROPERTY(ReplicatedUsing=OnRep_BaseLevel, EditAnywhere, BlueprintReadOnly, Category="Leveling")
	int32 BaseLevel;

	UPROPERTY(ReplicatedUsing=OnRep_JobLevel, EditAnywhere, BlueprintReadOnly, Category="Leveling")
	int32 JobLevel;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category="Leveling")
	int64 CurrentBaseExp;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category="Leveling")
	int64 CurrentJobExp;

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category="Leveling")
	FOnBaseLevelUp OnBaseLevelUp;

	UPROPERTY(BlueprintAssignable, Category="Leveling")
	FOnJobLevelUp OnJobLevelUp;

	// ---- Functions ----

	/** Add base experience. Handles multi-level ups. Server authoritative. */
	UFUNCTION(BlueprintCallable, Category="Leveling")
	void AddBaseExp(int64 Amount);

	/** Add job experience. Handles multi-level ups. Server authoritative. */
	UFUNCTION(BlueprintCallable, Category="Leveling")
	void AddJobExp(int64 Amount);

	/** Get required base exp for next level (0 if at max). */
	UFUNCTION(BlueprintCallable, Category="Leveling")
	int64 GetRequiredBaseExp() const;

	/** Get required job exp for next level (0 if at max). */
	UFUNCTION(BlueprintCallable, Category="Leveling")
	int64 GetRequiredJobExp() const;

	/** Get base exp percentage (0.0 - 1.0) for UI display. */
	UFUNCTION(BlueprintCallable, Category="Leveling")
	float GetBaseExpPercentage() const;

	/** Get job exp percentage (0.0 - 1.0) for UI display. */
	UFUNCTION(BlueprintCallable, Category="Leveling")
	float GetJobExpPercentage() const;

	/** Reset job level to 1 with 0 exp (called on job change). */
	void ResetJobLevel();

	/** Get the max base level (99 for normal, 99 for trans). */
	UFUNCTION(BlueprintCallable, Category="Leveling")
	int32 GetMaxBaseLevel() const;

	/** Get the max job level for the current job. */
	UFUNCTION(BlueprintCallable, Category="Leveling")
	int32 GetMaxJobLevel() const;

protected:
	UFUNCTION()
	void OnRep_BaseLevel();

	UFUNCTION()
	void OnRep_JobLevel();

private:
	/** Process base level up: grant stat points, notify components. */
	void ProcessBaseLevelUp();

	/** Process job level up: grant skill points, notify components. */
	void ProcessJobLevelUp();

	/** Get stat points awarded for reaching a given base level. */
	static int32 GetStatPointsForLevel(int32 Level);
};
