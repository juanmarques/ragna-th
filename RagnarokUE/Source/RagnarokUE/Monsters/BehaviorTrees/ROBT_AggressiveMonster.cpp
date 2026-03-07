// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROBT_AggressiveMonster.h"
#include "RagnarokUE/Monsters/ROMonsterBase.h"
#include "RagnarokUE/Monsters/ROMonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

// ============================================================================
// UROBTTask_AggressiveScan
// ============================================================================

UROBTTask_AggressiveScan::UROBTTask_AggressiveScan()
{
	NodeName = "RO Aggressive: Scan For Target";
}

EBTNodeResult::Type UROBTTask_AggressiveScan::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AROMonsterAIController* MonsterAI = Cast<AROMonsterAIController>(OwnerComp.GetAIOwner());
	if (!MonsterAI)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}

	// If already in combat with a valid target, don't rescan
	if (BB->GetValueAsBool(AROMonsterAIController::BB_IsInCombat))
	{
		AActor* ExistingTarget = Cast<AActor>(BB->GetValueAsObject(AROMonsterAIController::BB_TargetActor));
		if (ExistingTarget && IsValid(ExistingTarget))
		{
			return EBTNodeResult::Succeeded;
		}
		// Target invalid, exit combat and rescan
		MonsterAI->ExitCombat();
	}

	// Scan for nearest player within aggro range
	AActor* Target = MonsterAI->FindTarget();
	if (Target)
	{
		MonsterAI->EnterCombat(Target);
		return EBTNodeResult::Succeeded;
	}

	// No target found - that's ok for aggressive scan, tree will loop
	return EBTNodeResult::Failed;
}

FString UROBTTask_AggressiveScan::GetStaticDescription() const
{
	return FString::Printf(TEXT("Scan for nearest player every %.1f sec"), ScanInterval);
}

// ============================================================================
// UROBTTask_AggressiveChaseAttack
// ============================================================================

UROBTTask_AggressiveChaseAttack::UROBTTask_AggressiveChaseAttack()
{
	NodeName = "RO Aggressive: Chase & Attack";
	bNotifyTick = true;
	bNotifyAbort = true;
}

EBTNodeResult::Type UROBTTask_AggressiveChaseAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
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

	// Begin chasing the target
	AIController->MoveToActor(Target, Monster->AttackRange * 0.9f);

	return EBTNodeResult::InProgress;
}

void UROBTTask_AggressiveChaseAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

	AROMonsterAIController* MonsterAI = Cast<AROMonsterAIController>(AIController);

	// Check if we're too far from home
	const float DistFromHome = FVector::Dist(Monster->GetActorLocation(), Monster->SpawnLocation);
	if (DistFromHome > Monster->ChaseRange)
	{
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
		// Target gone, exit combat
		if (MonsterAI)
		{
			MonsterAI->ExitCombat();
		}
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// Check if target switch is needed
	if (bCanSwitchTarget && ShouldSwitchTarget(Monster, Target))
	{
		AActor* NewTarget = Monster->GetHighestThreatTarget();
		if (NewTarget && NewTarget != Target)
		{
			if (MonsterAI)
			{
				MonsterAI->SetTargetActor(NewTarget);
			}
			Target = NewTarget;
		}
	}

	// Check distance to target
	const float DistToTarget = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());

	if (DistToTarget <= Monster->AttackRange)
	{
		// In range - attack
		if (Monster->CanAttack())
		{
			// Face target
			const FVector Direction = (Target->GetActorLocation() - Monster->GetActorLocation()).GetSafeNormal();
			Monster->SetActorRotation(Direction.Rotation());

			// Deal damage
			FDamageEvent DamageEvent;
			Target->TakeDamage(static_cast<float>(Monster->ATK), DamageEvent, AIController, Monster);
			Monster->MarkAttackPerformed();
		}
	}
	else
	{
		// Continue chasing
		AIController->MoveToActor(Target, Monster->AttackRange * 0.9f);
	}
}

EBTNodeResult::Type UROBTTask_AggressiveChaseAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AIController->StopMovement();
	}
	return EBTNodeResult::Aborted;
}

bool UROBTTask_AggressiveChaseAttack::ShouldSwitchTarget(const AROMonsterBase* Monster, AActor* CurrentTarget) const
{
	if (!Monster || !CurrentTarget)
	{
		return false;
	}

	// Check if highest threat target is different and closer
	AActor* HighestThreat = Monster->GetHighestThreatTarget();
	if (!HighestThreat || HighestThreat == CurrentTarget)
	{
		return false;
	}

	const FVector MonsterLoc = Monster->GetActorLocation();
	const float DistToCurrent = FVector::DistSquared(MonsterLoc, CurrentTarget->GetActorLocation());
	const float DistToNew = FVector::DistSquared(MonsterLoc, HighestThreat->GetActorLocation());

	// Switch if new target is closer
	return DistToNew < DistToCurrent;
}

FString UROBTTask_AggressiveChaseAttack::GetStaticDescription() const
{
	return FString::Printf(TEXT("Chase & attack target. Switch target: %s"),
		bCanSwitchTarget ? TEXT("Yes") : TEXT("No"));
}
