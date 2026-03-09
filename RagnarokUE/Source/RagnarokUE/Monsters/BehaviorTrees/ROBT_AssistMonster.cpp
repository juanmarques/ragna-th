// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROBT_AssistMonster.h"
#include "RagnarokUE/Monsters/ROMonsterBase.h"
#include "RagnarokUE/Monsters/ROMonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================================
// UROBTTask_AssistScanAllies
// ============================================================================

UROBTTask_AssistScanAllies::UROBTTask_AssistScanAllies()
{
	NodeName = "RO Assist: Scan Allies";
}

EBTNodeResult::Type UROBTTask_AssistScanAllies::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AROMonsterAIController* MonsterAI = Cast<AROMonsterAIController>(OwnerComp.GetAIOwner());
	if (!MonsterAI)
	{
		return EBTNodeResult::Failed;
	}

	AROMonsterBase* Monster = Cast<AROMonsterBase>(MonsterAI->GetPawn());
	if (!Monster || Monster->bIsDead)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}

	// If already in combat, continue fighting
	if (BB->GetValueAsBool(AROMonsterAIController::BB_IsInCombat))
	{
		AActor* ExistingTarget = Cast<AActor>(BB->GetValueAsObject(AROMonsterAIController::BB_TargetActor));
		if (ExistingTarget && IsValid(ExistingTarget))
		{
			return EBTNodeResult::Succeeded;
		}
		MonsterAI->ExitCombat();
	}

	// Scan for nearby same-type monsters that are in combat
	const FVector MonsterLocation = Monster->GetActorLocation();
	const float AssistRangeSq = FMath::Square(AssistRange);

	TArray<AActor*> AllMonsters;
	UGameplayStatics::GetAllActorsOfClass(Monster->GetWorld(), AROMonsterBase::StaticClass(), AllMonsters);

	for (AActor* Actor : AllMonsters)
	{
		AROMonsterBase* Ally = Cast<AROMonsterBase>(Actor);
		if (!Ally || Ally == Monster || Ally->bIsDead)
		{
			continue;
		}

		// Must be the same monster type (same MonsterID)
		if (Ally->MonsterID != Monster->MonsterID)
		{
			continue;
		}

		// Must be within assist range
		const float DistSq = FVector::DistSquared(MonsterLocation, Ally->GetActorLocation());
		if (DistSq > AssistRangeSq)
		{
			continue;
		}

		// Check if ally is in combat (has a current target)
		if (Ally->CurrentTarget && IsValid(Ally->CurrentTarget.Get()))
		{
			// Assist! Aggro onto the ally's attacker
			MonsterAI->EnterCombat(Ally->CurrentTarget.Get());
			return EBTNodeResult::Succeeded;
		}
	}

	// No allies in combat, stay passive
	return EBTNodeResult::Failed;
}

FString UROBTTask_AssistScanAllies::GetStaticDescription() const
{
	return FString::Printf(TEXT("Scan for same-type allies in combat within %.0f units"), AssistRange);
}

// ============================================================================
// UROBTTask_AssistAttack
// ============================================================================

UROBTTask_AssistAttack::UROBTTask_AssistAttack()
{
	NodeName = "RO Assist: Stack Aggro Attack";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UROBTTask_AssistAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	// Begin chasing the target (ally's attacker)
	AIController->MoveToActor(Target, Monster->AttackRange * 0.9f);

	return EBTNodeResult::InProgress;
}

void UROBTTask_AssistAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

	// Check chase range from home
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
		if (MonsterAI)
		{
			MonsterAI->ExitCombat();
		}
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// Check distance and attack
	const float DistToTarget = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());
	if (DistToTarget <= Monster->AttackRange)
	{
		if (Monster->CanAttack())
		{
			const FVector Direction = (Target->GetActorLocation() - Monster->GetActorLocation()).GetSafeNormal();
			Monster->SetActorRotation(Direction.Rotation());

			FDamageEvent DamageEvent;
			Target->TakeDamage(static_cast<float>(Monster->GetAttackDamage()), DamageEvent, AIController, Monster);
			Monster->MarkAttackPerformed();
		}
	}
	else
	{
		AIController->MoveToActor(Target, Monster->AttackRange * 0.9f);
	}
}

EBTNodeResult::Type UROBTTask_AssistAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AIController->StopMovement();
	}
	return EBTNodeResult::Aborted;
}

FString UROBTTask_AssistAttack::GetStaticDescription() const
{
	return TEXT("Assist attack: chase and attack ally's attacker (stacking aggro)");
}
