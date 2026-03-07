// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "ROBTTask_ChaseTarget.generated.h"

/**
 * UROBTTask_ChaseTarget
 * BT Task: Chase the target actor from blackboard.
 * Finishes successfully when within attack range.
 * Aborts and returns home if target goes out of chase range.
 */
UCLASS(meta = (DisplayName = "RO: Chase Target"))
class RAGNAROKUE_API UROBTTask_ChaseTarget : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UROBTTask_ChaseTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

	/** How close to get before stopping (multiplied by monster's AttackRange). */
	UPROPERTY(EditAnywhere, Category = "Chase", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float AttackRangeMultiplier = 0.9f;

	/** How often to re-issue MoveToActor (to update path if target moves). Seconds. */
	UPROPERTY(EditAnywhere, Category = "Chase", meta = (ClampMin = "0.1"))
	float PathUpdateInterval = 0.5f;

protected:
	/** Per-instance memory. */
	struct FChaseMemory
	{
		float TimeSinceLastPathUpdate = 0.0f;
	};

	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FChaseMemory); }
};
