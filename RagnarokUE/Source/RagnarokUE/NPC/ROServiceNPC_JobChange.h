// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/NPC/RONPCBase.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROServiceNPC_JobChange.generated.h"

class AROCharacterBase;
class UROJobComponent;
class UROLevelingComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnJobChangeResult, bool, bSuccess, EROJobClass, OldJob, EROJobClass, NewJob);

/**
 * AROServiceNPC_JobChange
 * Job change NPC that transitions players from one class to another.
 * Each NPC instance handles one specific class transition
 * (e.g., Swordsman NPC in Izlude handles Novice -> Swordsman).
 */
UCLASS(ClassGroup=(RagnarokUE), Blueprintable)
class RAGNAROKUE_API AROServiceNPC_JobChange : public ARONPCBase
{
	GENERATED_BODY()

public:
	AROServiceNPC_JobChange();

	// ---- Configuration ----

	/** The job class this NPC will change the player to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JobChange")
	EROJobClass TargetJobClass;

	/** The prerequisite job class the player must currently have. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JobChange")
	EROJobClass RequiredCurrentJob;

	/** Minimum job level required for job change. Standard: 40 (50 for optimal). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JobChange", meta = (ClampMin = "1", ClampMax = "50"))
	int32 MinJobLevel;

	/** Optimal job level that grants bonus skill points. Standard: 50. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JobChange", meta = (ClampMin = "1", ClampMax = "50"))
	int32 OptimalJobLevel;

	/** Number of initial skill points granted on job change. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JobChange", meta = (ClampMin = "0"))
	int32 InitialSkillPoints;

	// ---- Delegates ----

	/** Broadcast when a job change attempt completes. */
	UPROPERTY(BlueprintAssignable, Category = "JobChange|Events")
	FOnJobChangeResult OnJobChangeResult;

	// ---- Interaction ----

	/** Override: Check requirements and offer job change. */
	virtual void OnInteract_Implementation(AROCharacterBase* Interactor) override;

	// ---- Server RPCs ----

	/** Attempt to change the player's job class. Server-authoritative. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAttemptJobChange();

	// ---- Queries ----

	/**
	 * Check if a player meets all job change requirements.
	 * @param Player The player to check.
	 * @return True if the player can change to TargetJobClass.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "JobChange")
	bool CanPlayerChangeJob(AROCharacterBase* Player) const;

	/**
	 * Get a description of why the player cannot change jobs.
	 * Returns empty text if the player meets all requirements.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "JobChange")
	FText GetRequirementFailureReason(AROCharacterBase* Player) const;

protected:
	/** The player currently interacting with this job change NPC. */
	UPROPERTY()
	TObjectPtr<AROCharacterBase> CurrentInteractor;

	/** Execute the job change on the server. */
	void ExecuteJobChange(AROCharacterBase* Player);
};
