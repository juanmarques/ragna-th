// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROMonsterAIController.h"
#include "ROMonsterBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

// Static blackboard key names
const FName AROMonsterAIController::BB_TargetActor = FName("TargetActor");
const FName AROMonsterAIController::BB_HomeLocation = FName("HomeLocation");
const FName AROMonsterAIController::BB_AggroRange = FName("AggroRange");
const FName AROMonsterAIController::BB_AttackRange = FName("AttackRange");
const FName AROMonsterAIController::BB_ChaseRange = FName("ChaseRange");
const FName AROMonsterAIController::BB_IsInCombat = FName("IsInCombat");
const FName AROMonsterAIController::BB_MonsterBehavior = FName("MonsterBehavior");

AROMonsterAIController::AROMonsterAIController()
{
	BT_Passive = nullptr;
	BT_Aggressive = nullptr;
	BT_Assist = nullptr;
	BT_CastSensor = nullptr;
	MonsterBlackboardData = nullptr;
	OwnerMonster = nullptr;
}

void AROMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	OwnerMonster = Cast<AROMonsterBase>(InPawn);
	if (!OwnerMonster)
	{
		UE_LOG(LogTemp, Warning, TEXT("ROMonsterAIController possessed a non-ROMonsterBase pawn!"));
		return;
	}

	// Select and run behavior tree
	UBehaviorTree* SelectedBT = SelectBehaviorTree(OwnerMonster->Behavior);
	if (!SelectedBT)
	{
		UE_LOG(LogTemp, Warning, TEXT("No behavior tree found for monster %s with behavior %d"),
			*OwnerMonster->MonsterName.ToString(), static_cast<int32>(OwnerMonster->Behavior));
		return;
	}

	// Use the behavior tree's blackboard asset
	if (SelectedBT->BlackboardAsset)
	{
		if (UseBlackboard(SelectedBT->BlackboardAsset, Blackboard))
		{
			InitializeBlackboard();
		}
	}

	RunBehaviorTree(SelectedBT);
}

void AROMonsterAIController::OnUnPossess()
{
	// Stop behavior tree
	UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BTComp)
	{
		BTComp->StopTree();
	}

	OwnerMonster = nullptr;
	Super::OnUnPossess();
}

UBehaviorTree* AROMonsterAIController::SelectBehaviorTree(EROMonsterBehavior InBehavior) const
{
	switch (InBehavior)
	{
	case EROMonsterBehavior::Passive:
		return BT_Passive;

	case EROMonsterBehavior::Aggressive:
	case EROMonsterBehavior::Detector:
	case EROMonsterBehavior::ChangeTarget:
		return BT_Aggressive;

	case EROMonsterBehavior::Assist:
		return BT_Assist;

	case EROMonsterBehavior::CastSensor:
		return BT_CastSensor ? BT_CastSensor : BT_Aggressive;

	case EROMonsterBehavior::Looter:
		// Looters use passive with additional loot-seeking logic
		return BT_Passive;

	default:
		return BT_Passive;
	}
}

AActor* AROMonsterAIController::FindTarget() const
{
	if (!OwnerMonster)
	{
		return nullptr;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector MonsterLocation = OwnerMonster->GetActorLocation();
	const float AggroRangeSq = FMath::Square(OwnerMonster->AggroRange);

	AActor* ClosestTarget = nullptr;
	float ClosestDistSq = AggroRangeSq;

	// Find all player characters within aggro range
	TArray<AActor*> PlayerCharacters;
	UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), PlayerCharacters);

	for (AActor* Actor : PlayerCharacters)
	{
		// Skip self and other monsters
		if (Actor == OwnerMonster || Actor->IsA(AROMonsterBase::StaticClass()))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(MonsterLocation, Actor->GetActorLocation());
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestTarget = Actor;
		}
	}

	return ClosestTarget;
}

void AROMonsterAIController::HandleDamageReceived(AActor* Attacker, float DamageAmount)
{
	if (!OwnerMonster || !Attacker)
	{
		return;
	}

	// Threat is already updated in AROMonsterBase::TakeDamage via AddThreat
	// Here we handle the AI response

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	// If not currently in combat, enter combat with the attacker
	const bool bInCombat = BB->GetValueAsBool(BB_IsInCombat);
	if (!bInCombat)
	{
		EnterCombat(Attacker);
		return;
	}

	// If already in combat, consider switching targets based on threat
	AActor* HighestThreat = OwnerMonster->GetHighestThreatTarget();
	if (HighestThreat && HighestThreat != BB->GetValueAsObject(BB_TargetActor))
	{
		// For ChangeTarget behavior, always switch to highest threat
		if (OwnerMonster->Behavior == EROMonsterBehavior::ChangeTarget)
		{
			SetTargetActor(HighestThreat);
		}
	}
}

void AROMonsterAIController::ReturnToHome()
{
	if (!OwnerMonster)
	{
		return;
	}

	ExitCombat();

	// Move back to spawn location
	MoveToLocation(OwnerMonster->SpawnLocation, 50.0f);

	// Reset monster state (heal to full, clear threat)
	OwnerMonster->ResetToIdle();
}

bool AROMonsterAIController::IsTargetValid() const
{
	if (!OwnerMonster)
	{
		return false;
	}

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return false;
	}

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(BB_TargetActor));
	if (!Target)
	{
		return false;
	}

	// Check if target is still alive (not pending destroy)
	if (!IsValid(Target))
	{
		return false;
	}

	// Check if target is within chase range from spawn location
	const float TargetDistFromHome = FVector::Dist(Target->GetActorLocation(), OwnerMonster->SpawnLocation);
	if (TargetDistFromHome > OwnerMonster->ChaseRange)
	{
		return false;
	}

	return true;
}

void AROMonsterAIController::SetTargetActor(AActor* NewTarget)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsObject(BB_TargetActor, NewTarget);
	}

	if (OwnerMonster)
	{
		OwnerMonster->CurrentTarget = NewTarget;
	}
}

void AROMonsterAIController::EnterCombat(AActor* Target)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	BB->SetValueAsBool(BB_IsInCombat, true);
	SetTargetActor(Target);
}

void AROMonsterAIController::ExitCombat()
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	BB->SetValueAsBool(BB_IsInCombat, false);
	BB->ClearValue(BB_TargetActor);

	if (OwnerMonster)
	{
		OwnerMonster->CurrentTarget = nullptr;
	}
}

void AROMonsterAIController::InitializeBlackboard()
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB || !OwnerMonster)
	{
		return;
	}

	BB->SetValueAsVector(BB_HomeLocation, OwnerMonster->SpawnLocation);
	BB->SetValueAsFloat(BB_AggroRange, OwnerMonster->AggroRange);
	BB->SetValueAsFloat(BB_AttackRange, OwnerMonster->AttackRange);
	BB->SetValueAsFloat(BB_ChaseRange, OwnerMonster->ChaseRange);
	BB->SetValueAsBool(BB_IsInCombat, false);
}
