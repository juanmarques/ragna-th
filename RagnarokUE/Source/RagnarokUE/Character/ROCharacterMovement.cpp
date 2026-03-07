// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROCharacterMovement.h"
#include "ROStatsComponent.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"

UROCharacterMovement::UROCharacterMovement()
{
	PrimaryComponentTick.bCanEverTick = true;

	bIsMovingToDestination = false;
	MoveDestination = FVector::ZeroVector;
	MoveAcceptanceRadius = 50.0f;
	BaseMovementSpeed = 400.0f; // UE units per second (roughly matching RO walk speed)
	MovementSpeedBonusPercent = 0.0f;

	// RO-style defaults
	MaxWalkSpeed = BaseMovementSpeed;
	bOrientRotationToMovement = true;
	RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	bConstrainToPlane = true;
	bSnapToPlaneAtStart = true;

	// Use NavMesh walking
	bUseRVOAvoidance = true;
	AvoidanceConsiderationRadius = 150.0f;
}

void UROCharacterMovement::BeginPlay()
{
	Super::BeginPlay();

	// Cache the stats component reference
	if (AActor* Owner = GetOwner())
	{
		CachedStatsComponent = Owner->FindComponentByClass<UROStatsComponent>();
	}
}

void UROCharacterMovement::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsMovingToDestination)
	{
		return;
	}

	// Check if we've reached the destination
	if (AActor* Owner = GetOwner())
	{
		const float DistanceToTarget = FVector::Dist2D(Owner->GetActorLocation(), MoveDestination);

		if (DistanceToTarget <= MoveAcceptanceRadius)
		{
			// Arrived at destination
			bIsMovingToDestination = false;
			StopMovementImmediately();
			return;
		}

		// If not using AI controller pathfinding, do simple direct movement
		ACharacter* CharOwner = Cast<ACharacter>(Owner);
		if (CharOwner && !CharOwner->GetController())
		{
			// Fallback: direct movement without AI controller
			FVector Direction = (MoveDestination - Owner->GetActorLocation()).GetSafeNormal2D();
			AddInputVector(Direction);
		}
	}
}

void UROCharacterMovement::MoveToLocation(FVector Destination)
{
	MoveDestination = Destination;
	bIsMovingToDestination = true;

	RequestNavMeshMove();
}

void UROCharacterMovement::StopMovementCommand()
{
	bIsMovingToDestination = false;
	StopMovementImmediately();

	// Also stop AI move if active
	if (ACharacter* CharOwner = Cast<ACharacter>(GetOwner()))
	{
		if (AAIController* AIController = Cast<AAIController>(CharOwner->GetController()))
		{
			AIController->StopMovement();
		}
	}
}

bool UROCharacterMovement::IsMoving() const
{
	return bIsMovingToDestination && Velocity.SizeSquared() > KINDA_SMALL_NUMBER;
}

float UROCharacterMovement::GetMaxSpeed() const
{
	float Speed = BaseMovementSpeed;

	// Apply AGI bonus: each point of AGI above 1 gives a small speed increase
	// In RO, AGI primarily affects ASPD and FLEE, but movement speed items/buffs exist
	// We add a subtle AGI bonus for feel: +0.5% per AGI point above 1
	if (CachedStatsComponent.IsValid())
	{
		const int32 TotalAGI = CachedStatsComponent->TotalAGI;
		const float AGIBonus = FMath::Max(0.0f, (TotalAGI - 1) * 0.005f);
		Speed *= (1.0f + AGIBonus);
	}

	// Apply buff-based movement speed bonus (e.g., Increase AGI, Peco riding)
	Speed *= (1.0f + MovementSpeedBonusPercent);

	// Clamp to reasonable bounds
	// In RO, max movement speed is roughly 2x base with Peco/Cart Revolution etc.
	Speed = FMath::Clamp(Speed, 0.0f, BaseMovementSpeed * 2.5f);

	return Speed;
}

void UROCharacterMovement::RequestNavMeshMove()
{
	ACharacter* CharOwner = Cast<ACharacter>(GetOwner());
	if (!CharOwner)
	{
		return;
	}

	// Try to use AI controller for NavMesh pathfinding
	AAIController* AIController = Cast<AAIController>(CharOwner->GetController());
	if (AIController)
	{
		AIController->MoveToLocation(
			MoveDestination,
			MoveAcceptanceRadius,
			true,  // bStopOnOverlap
			true,  // bUsePathfinding
			false, // bProjectDestinationToNavigation
			true,  // bCanStrafe
			nullptr, // FilterClass
			true   // bAllowPartialPath
		);
		return;
	}

	// Fallback: if no AI controller, use navigation system directly for path query
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys)
	{
		return;
	}

	// Simple direct movement will be handled in TickComponent
	// The tick will use AddInputVector for basic movement towards destination
}
