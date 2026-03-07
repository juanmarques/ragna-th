// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "ROBTTask_RoamRandom.generated.h"

/**
 * UROBTTask_RoamRandom
 * BT Task: Pick a random navigable point within roam radius from home location,
 * move there, wait a random duration, then finish.
 */
UCLASS(meta = (DisplayName = "RO: Roam Random"))
class RAGNAROKUE_API UROBTTask_RoamRandom : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UROBTTask_RoamRandom();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;

	/** Radius around home location to pick random points. */
	UPROPERTY(EditAnywhere, Category = "Roam", meta = (ClampMin = "100.0"))
	float RoamRadius = 500.0f;

	/** Minimum wait time at destination (seconds). */
	UPROPERTY(EditAnywhere, Category = "Roam", meta = (ClampMin = "0.5"))
	float MinWaitTime = 3.0f;

	/** Maximum wait time at destination (seconds). */
	UPROPERTY(EditAnywhere, Category = "Roam", meta = (ClampMin = "1.0"))
	float MaxWaitTime = 10.0f;

	/** Acceptance radius for reaching destination. */
	UPROPERTY(EditAnywhere, Category = "Roam", meta = (ClampMin = "10.0"))
	float AcceptanceRadius = 50.0f;

protected:
	/** Task execution phases. */
	enum class ERoamPhase : uint8
	{
		Moving,
		Waiting,
		Done
	};

	/** Per-instance memory for this task node. */
	struct FRoamTaskMemory
	{
		ERoamPhase Phase = ERoamPhase::Moving;
		float WaitTimeRemaining = 0.0f;
		FVector TargetLocation = FVector::ZeroVector;
	};
};
