// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "ROBT_PassiveMonster.generated.h"

class AROMonsterBase;
class AROMonsterAIController;

/**
 * UROBTTask_PassiveIdle
 * Passive monster idle behavior: stand still or slow roam near spawn.
 * When damaged, switches to fight-back mode via blackboard.
 */
UCLASS(meta = (DisplayName = "RO Passive: Idle/Roam"))
class RAGNAROKUE_API UROBTTask_PassiveIdle : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UROBTTask_PassiveIdle();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

	/** Radius around home location for roaming. */
	UPROPERTY(EditAnywhere, Category = "Passive")
	float RoamRadius = 500.0f;

	/** Minimum idle wait time in seconds. */
	UPROPERTY(EditAnywhere, Category = "Passive")
	float MinIdleTime = 3.0f;

	/** Maximum idle wait time in seconds. */
	UPROPERTY(EditAnywhere, Category = "Passive")
	float MaxIdleTime = 10.0f;
};

/**
 * UROBTTask_PassiveFightBack
 * When a passive monster is damaged, it fights back against the attacker.
 * Chases and attacks until attacker is dead or out of chase range.
 */
UCLASS(meta = (DisplayName = "RO Passive: Fight Back"))
class RAGNAROKUE_API UROBTTask_PassiveFightBack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UROBTTask_PassiveFightBack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Check if the monster should disengage and return home. */
	bool ShouldReturnHome(const AROMonsterBase* Monster) const;
};

/**
 * UROBTTask_PassiveReturnHome
 * If the attacker leaves chase range, return home and heal to full.
 */
UCLASS(meta = (DisplayName = "RO Passive: Return Home & Heal"))
class RAGNAROKUE_API UROBTTask_PassiveReturnHome : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UROBTTask_PassiveReturnHome();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
