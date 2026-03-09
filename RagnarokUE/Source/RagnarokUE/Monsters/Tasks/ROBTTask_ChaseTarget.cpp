// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROBTTask_ChaseTarget.h"
#include "RagnarokUE/Monsters/ROMonsterBase.h"
#include "RagnarokUE/Monsters/ROMonsterAIController.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UROBTTask_ChaseTarget::UROBTTask_ChaseTarget()
{
	NodeName = "RO: Chase Target";
	bNotifyTick = true;
	bNotifyTaskFinished = true;

	// Default BlackboardKey filters to Object (for TargetActor)
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UROBTTask_ChaseTarget, BlackboardKey), AActor::StaticClass());
}

EBTNodeResult::Type UROBTTask_ChaseTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FChaseMemory* Memory = reinterpret_cast<FChaseMemory*>(NodeMemory);
	if (Memory)
	{
		Memory->TimeSinceLastPathUpdate = PathUpdateInterval; // Force immediate path update
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AROMonsterBase* Monster = Cast<AROMonsterBase>(AIController->GetPawn());
	if (!Monster || Monster->bIsDead)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(AROMonsterAIController::BB_TargetActor));
	if (!Target || !IsValid(Target))
	{
		return EBTNodeResult::Failed;
	}

	// Check if already in attack range
	const float DistToTarget = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());
	if (DistToTarget <= Monster->AttackRange * AttackRangeMultiplier)
	{
		return EBTNodeResult::Succeeded;
	}

	// Start chasing
	AIController->MoveToActor(Target, Monster->AttackRange * AttackRangeMultiplier);

	return EBTNodeResult::InProgress;
}

void UROBTTask_ChaseTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FChaseMemory* Memory = reinterpret_cast<FChaseMemory*>(NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AROMonsterBase* Monster = Cast<AROMonsterBase>(AIController->GetPawn());
	if (!Monster || Monster->bIsDead)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AActor* Target = BB ? Cast<AActor>(BB->GetValueAsObject(AROMonsterAIController::BB_TargetActor)) : nullptr;

	// Target validation
	if (!Target || !IsValid(Target))
	{
		AIController->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Check if target is out of chase range from home
	const float DistFromHome = FVector::Dist(Monster->GetActorLocation(), Monster->SpawnLocation);
	if (DistFromHome > Monster->ChaseRange)
	{
		// Target too far from home, abort chase and return
		AIController->StopMovement();

		AROMonsterAIController* MonsterAI = Cast<AROMonsterAIController>(AIController);
		if (MonsterAI)
		{
			MonsterAI->ReturnToHome();
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Check if within attack range
	const float DistToTarget = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());
	if (DistToTarget <= Monster->AttackRange * AttackRangeMultiplier)
	{
		AIController->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// Periodically update path to follow moving target
	if (Memory)
	{
		Memory->TimeSinceLastPathUpdate += DeltaSeconds;
		if (Memory->TimeSinceLastPathUpdate >= PathUpdateInterval)
		{
			Memory->TimeSinceLastPathUpdate = 0.0f;
			AIController->MoveToActor(Target, Monster->AttackRange * AttackRangeMultiplier);
		}
	}
}

EBTNodeResult::Type UROBTTask_ChaseTarget::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AIController->StopMovement();
	}
	return EBTNodeResult::Aborted;
}

FString UROBTTask_ChaseTarget::GetStaticDescription() const
{
	return FString::Printf(TEXT("Chase target until within attack range (x%.1f).\nPath update every %.1f sec.\nAbort if out of chase range."),
		AttackRangeMultiplier, PathUpdateInterval);
}
