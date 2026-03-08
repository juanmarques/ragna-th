// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROBT_PassiveMonster.h"
#include "RagnarokUE/Monsters/ROMonsterBase.h"
#include "RagnarokUE/Monsters/ROMonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

// ============================================================================
// UROBTTask_PassiveIdle
// ============================================================================

UROBTTask_PassiveIdle::UROBTTask_PassiveIdle()
{
	NodeName = "RO Passive: Idle/Roam";
}

EBTNodeResult::Type UROBTTask_PassiveIdle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AROMonsterBase* Monster = Cast<AROMonsterBase>(AIController->GetPawn());
	if (!Monster)
	{
		return EBTNodeResult::Failed;
	}

	// If in combat (from being hit), abort idle and let fight-back handle it
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (BB && BB->GetValueAsBool(AROMonsterAIController::BB_IsInCombat))
	{
		return EBTNodeResult::Failed;
	}

	// Pick a random point within roam radius of home location
	const FVector HomeLocation = Monster->SpawnLocation;
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Monster->GetWorld());
	if (!NavSys)
	{
		return EBTNodeResult::Failed;
	}

	FNavLocation NavResult;
	const bool bFound = NavSys->GetRandomReachablePointInRadius(HomeLocation, RoamRadius, NavResult);
	if (!bFound)
	{
		// If no nav point found, just stay put - succeed anyway
		return EBTNodeResult::Succeeded;
	}

	// Move to the random point
	AIController->MoveToLocation(NavResult.Location, 50.0f);

	return EBTNodeResult::Succeeded;
}

FString UROBTTask_PassiveIdle::GetStaticDescription() const
{
	return FString::Printf(TEXT("Passive idle/roam within %.0f units, wait %.1f-%.1f sec"),
		RoamRadius, MinIdleTime, MaxIdleTime);
}

// ============================================================================
// UROBTTask_PassiveFightBack
// ============================================================================

UROBTTask_PassiveFightBack::UROBTTask_PassiveFightBack()
{
	NodeName = "RO Passive: Fight Back";
	bNotifyTick = true;
	bNotifyAbort = true;
}

EBTNodeResult::Type UROBTTask_PassiveFightBack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AROMonsterBase* Monster = Cast<AROMonsterBase>(AIController->GetPawn());
	if (!Monster)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}

	// Check if we have a target to fight
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(AROMonsterAIController::BB_TargetActor));
	if (!Target || !IsValid(Target))
	{
		return EBTNodeResult::Failed;
	}

	// Begin chasing
	AIController->MoveToActor(Target, Monster->AttackRange * 0.9f);

	return EBTNodeResult::InProgress;
}

void UROBTTask_PassiveFightBack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
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

	// Check if should return home (target too far)
	if (ShouldReturnHome(Monster))
	{
		AROMonsterAIController* MonsterAI = Cast<AROMonsterAIController>(AIController);
		if (MonsterAI)
		{
			MonsterAI->ReturnToHome();
		}
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AActor* Target = BB ? Cast<AActor>(BB->GetValueAsObject(AROMonsterAIController::BB_TargetActor)) : nullptr;

	if (!Target || !IsValid(Target))
	{
		// Target gone, return home
		AROMonsterAIController* MonsterAI = Cast<AROMonsterAIController>(AIController);
		if (MonsterAI)
		{
			MonsterAI->ReturnToHome();
		}
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// Check if in attack range
	const float DistToTarget = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());
	if (DistToTarget <= Monster->AttackRange)
	{
		// In range - attempt attack
		if (Monster->CanAttack())
		{
			// Face the target
			const FVector Direction = (Target->GetActorLocation() - Monster->GetActorLocation()).GetSafeNormal();
			Monster->SetActorRotation(Direction.Rotation());

			// Apply damage to target
			FDamageEvent DamageEvent;
			Target->TakeDamage(static_cast<float>(Monster->GetAttackDamage()), DamageEvent, AIController, Monster);
			Monster->MarkAttackPerformed();
		}
	}
	else
	{
		// Continue chasing
		AIController->MoveToActor(Target, Monster->AttackRange * 0.9f);
	}
}

EBTNodeResult::Type UROBTTask_PassiveFightBack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AIController->StopMovement();
	}
	return EBTNodeResult::Aborted;
}

bool UROBTTask_PassiveFightBack::ShouldReturnHome(const AROMonsterBase* Monster) const
{
	if (!Monster)
	{
		return true;
	}

	const float DistFromHome = FVector::Dist(Monster->GetActorLocation(), Monster->SpawnLocation);
	return DistFromHome > Monster->ChaseRange;
}

FString UROBTTask_PassiveFightBack::GetStaticDescription() const
{
	return TEXT("Fight back against attacker until dead or out of chase range");
}

// ============================================================================
// UROBTTask_PassiveReturnHome
// ============================================================================

UROBTTask_PassiveReturnHome::UROBTTask_PassiveReturnHome()
{
	NodeName = "RO Passive: Return Home & Heal";
}

EBTNodeResult::Type UROBTTask_PassiveReturnHome::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AROMonsterBase* Monster = Cast<AROMonsterBase>(AIController->GetPawn());
	if (!Monster)
	{
		return EBTNodeResult::Failed;
	}

	AROMonsterAIController* MonsterAI = Cast<AROMonsterAIController>(AIController);
	if (MonsterAI)
	{
		MonsterAI->ReturnToHome();
	}
	else
	{
		// Fallback: directly move and reset
		AIController->MoveToLocation(Monster->SpawnLocation, 50.0f);
		Monster->ResetToIdle();
	}

	return EBTNodeResult::Succeeded;
}

FString UROBTTask_PassiveReturnHome::GetStaticDescription() const
{
	return TEXT("Return to spawn location and heal to full HP");
}
