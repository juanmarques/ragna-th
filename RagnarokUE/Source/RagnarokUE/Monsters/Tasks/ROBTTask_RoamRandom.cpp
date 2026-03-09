// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROBTTask_RoamRandom.h"
#include "RagnarokUE/Monsters/ROMonsterBase.h"
#include "RagnarokUE/Monsters/ROMonsterAIController.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

UROBTTask_RoamRandom::UROBTTask_RoamRandom()
{
	NodeName = "RO: Roam Random";
	bNotifyTick = true;
	bNotifyTaskFinished = true;

	// Default BlackboardKey is HomeLocation
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UROBTTask_RoamRandom, BlackboardKey));
}

uint16 UROBTTask_RoamRandom::GetInstanceMemorySize() const
{
	return sizeof(FRoamTaskMemory);
}

EBTNodeResult::Type UROBTTask_RoamRandom::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FRoamTaskMemory* Memory = reinterpret_cast<FRoamTaskMemory*>(NodeMemory);
	if (!Memory)
	{
		return EBTNodeResult::Failed;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	// Get home location from blackboard
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	FVector HomeLocation = FVector::ZeroVector;
	if (BB)
	{
		HomeLocation = BB->GetValueAsVector(AROMonsterAIController::BB_HomeLocation);
	}
	else
	{
		HomeLocation = Pawn->GetActorLocation();
	}

	// Find a random navigable point within roam radius
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Pawn->GetWorld());
	if (!NavSys)
	{
		return EBTNodeResult::Failed;
	}

	FNavLocation NavResult;
	const bool bFound = NavSys->GetRandomReachablePointInRadius(HomeLocation, RoamRadius, NavResult);
	if (!bFound)
	{
		// No navigable point found, just wait in place
		Memory->Phase = ERoamPhase::Waiting;
		Memory->WaitTimeRemaining = FMath::FRandRange(MinWaitTime, MaxWaitTime);
		Memory->TargetLocation = Pawn->GetActorLocation();
		return EBTNodeResult::InProgress;
	}

	Memory->TargetLocation = NavResult.Location;
	Memory->Phase = ERoamPhase::Moving;
	Memory->WaitTimeRemaining = FMath::FRandRange(MinWaitTime, MaxWaitTime);
	Memory->MoveTimer = 0.0f;

	// Start moving
	AIController->MoveToLocation(NavResult.Location, AcceptanceRadius);

	return EBTNodeResult::InProgress;
}

void UROBTTask_RoamRandom::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FRoamTaskMemory* Memory = reinterpret_cast<FRoamTaskMemory*>(NodeMemory);
	if (!Memory)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Check if combat started (should abort roaming)
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (BB && BB->GetValueAsBool(AROMonsterAIController::BB_IsInCombat))
	{
		AIController->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	switch (Memory->Phase)
	{
	case ERoamPhase::Moving:
	{
		// Timeout check: if movement takes too long, force transition to waiting
		Memory->MoveTimer += DeltaSeconds;
		if (Memory->MoveTimer >= MaxMoveTime)
		{
			Memory->Phase = ERoamPhase::Waiting;
			Memory->MoveTimer = 0.0f;
			AIController->StopMovement();
			break;
		}

		// Check if we've arrived at destination
		const float DistToTarget = FVector::Dist(
			AIController->GetPawn()->GetActorLocation(),
			Memory->TargetLocation
		);

		if (DistToTarget <= AcceptanceRadius)
		{
			// Arrived, start waiting
			Memory->Phase = ERoamPhase::Waiting;
			AIController->StopMovement();
		}

		// Also check if pathfollowing is idle (movement complete or failed)
		UPathFollowingComponent* PathComp = AIController->GetPathFollowingComponent();
		if (PathComp && PathComp->GetStatus() == EPathFollowingStatus::Idle)
		{
			Memory->Phase = ERoamPhase::Waiting;
		}
		break;
	}

	case ERoamPhase::Waiting:
	{
		Memory->WaitTimeRemaining -= DeltaSeconds;
		if (Memory->WaitTimeRemaining <= 0.0f)
		{
			Memory->Phase = ERoamPhase::Done;
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		break;
	}

	case ERoamPhase::Done:
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		break;
	}
	}
}

EBTNodeResult::Type UROBTTask_RoamRandom::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AIController->StopMovement();
	}
	return EBTNodeResult::Aborted;
}

FString UROBTTask_RoamRandom::GetStaticDescription() const
{
	return FString::Printf(TEXT("Roam randomly within %.0f units.\nWait %.1f-%.1f sec at destination."),
		RoamRadius, MinWaitTime, MaxWaitTime);
}
