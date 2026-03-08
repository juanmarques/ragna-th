// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "ROJobComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJobChanged, EROJobClass, OldJob, EROJobClass, NewJob);

/**
 * UROJobComponent
 * Manages the RO job/class system including job changes along the class tree.
 * Validates job change prerequisites and manages the job change paths.
 */
UCLASS(ClassGroup=(RagnarokUE), meta=(BlueprintSpawnableComponent))
class RAGNAROKUE_API UROJobComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UROJobComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ---- Replicated Properties ----

	UPROPERTY(ReplicatedUsing=OnRep_CurrentJobClass, EditAnywhere, BlueprintReadOnly, Category="Job")
	EROJobClass CurrentJobClass;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category="Job")
	int32 AvailableSkillPoints;

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category="Job")
	FOnJobChanged OnJobChanged;

	// ---- Functions ----

	/** Request a job change on the server. Validates prerequisites. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestJobChange(EROJobClass NewJob);

	/** Check if the character can change from CurrentJobClass to TargetJob. */
	UFUNCTION(BlueprintCallable, Category="Job")
	bool CanChangeToJob(EROJobClass TargetJob) const;

	/** Get available job change options for a given job class. */
	UFUNCTION(BlueprintCallable, Category="Job")
	static TArray<EROJobClass> GetAvailableJobChanges(EROJobClass Current);

	/** Get the tier of a given job class. */
	UFUNCTION(BlueprintCallable, Category="Job")
	static EROJobTier GetJobTier(EROJobClass Job);

	/** Get the max job level for the current job tier. */
	UFUNCTION(BlueprintCallable, Category="Job")
	int32 GetMaxJobLevel() const;

	/** Get the max job level for a specific tier. */
	UFUNCTION(BlueprintCallable, Category="Job")
	static int32 GetMaxJobLevelForTier(EROJobTier Tier);

	/** Get the base job for the current class (for determining class tree). */
	UFUNCTION(BlueprintCallable, Category="Job")
	static EROJobClass GetBaseJobForClass(EROJobClass Job);

	/** Add skill points (called on job level up). */
	void AddSkillPoints(int32 Amount);

protected:
	UFUNCTION()
	void OnRep_CurrentJobClass();

private:
	/** Internal job change logic (server only). */
	void ExecuteJobChange(EROJobClass NewJob);

	/** Get the minimum job level required to change to the next tier. */
	static int32 GetRequiredJobLevelForChange(EROJobTier TargetTier);
};
