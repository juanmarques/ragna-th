// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "ROBT_AssistMonster.generated.h"

class AROMonsterBase;
class AROMonsterAIController;

/**
 * UROBTTask_AssistScanAllies
 * Passive until a nearby same-type monster is attacked.
 * Scans nearby monsters of the same MonsterID for combat state.
 * When one is found in combat, aggro onto that monster's attacker.
 */
UCLASS(meta = (DisplayName = "RO Assist: Scan Allies"))
class RAGNAROKUE_API UROBTTask_AssistScanAllies : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UROBTTask_AssistScanAllies();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

	/** Range to scan for allied monsters in combat. */
	UPROPERTY(EditAnywhere, Category = "Assist")
	float AssistRange = 1500.0f;
};

/**
 * UROBTTask_AssistAttack
 * After assist trigger, chase and attack the target that attacked our ally.
 * Stacking behavior: multiple assist monsters will all aggro the same attacker.
 */
UCLASS(meta = (DisplayName = "RO Assist: Stack Aggro Attack"))
class RAGNAROKUE_API UROBTTask_AssistAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UROBTTask_AssistAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
