// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "ROBT_AggressiveMonster.generated.h"

class AROMonsterBase;
class AROMonsterAIController;

/**
 * UROBTTask_AggressiveScan
 * Scan for the nearest player within aggro range.
 * On finding a target, sets it in the blackboard and enters combat.
 */
UCLASS(meta = (DisplayName = "RO Aggressive: Scan For Target"))
class RAGNAROKUE_API UROBTTask_AggressiveScan : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UROBTTask_AggressiveScan();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

	/** How often to rescan (scan is done per-execute, but this documents intent). */
	UPROPERTY(EditAnywhere, Category = "Aggressive")
	float ScanInterval = 0.5f;
};

/**
 * UROBTTask_AggressiveChaseAttack
 * Chase the current target and attack until dead or out of range.
 * Can switch targets if hit by a closer player (depends on behavior flags).
 */
UCLASS(meta = (DisplayName = "RO Aggressive: Chase & Attack"))
class RAGNAROKUE_API UROBTTask_AggressiveChaseAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UROBTTask_AggressiveChaseAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

	/** Whether this monster can switch targets when hit by a closer player. */
	UPROPERTY(EditAnywhere, Category = "Aggressive")
	bool bCanSwitchTarget = false;

protected:
	/** Check if a new attacker should cause target switch. */
	bool ShouldSwitchTarget(const AROMonsterBase* Monster, AActor* CurrentTarget) const;
};
