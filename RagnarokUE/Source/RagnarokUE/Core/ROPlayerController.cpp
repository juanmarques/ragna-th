// Copyright Ragna-TH Project. All Rights Reserved.

#include "Core/ROPlayerController.h"
#include "RagnarokUE.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "RagnarokUE/Skills/ROSkillTreeComponent.h"
#include "AbilitySystemComponent.h"

// ---------------------------------------------------------------
// Construction
// ---------------------------------------------------------------

AROPlayerController::AROPlayerController()
	: bIsMovingToDestination(false)
	, MoveDestination(FVector::ZeroVector)
	, MoveAcceptanceRadius(50.0f)
	, CameraZoomDistance(800.0f)
	, CameraZoomMin(300.0f)
	, CameraZoomMax(2000.0f)
	, CameraZoomStep(100.0f)
	, CameraYawOffset(0.0f)
	, CameraRotationSpeed(2.0f)
	, SelectedTarget(nullptr)
	, bIsRotatingCamera(false)
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

// ---------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------

void AROPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogRagnarokUE, Log, TEXT("ROPlayerController::BeginPlay – Controller initialised for %s."),
		*GetName());
}

void AROPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!InputComponent)
	{
		return;
	}

	// Mouse clicks
	InputComponent->BindAction("LeftClick", IE_Pressed, this, &AROPlayerController::OnLeftMouseClick);
	InputComponent->BindAction("RightClick", IE_Pressed, this, &AROPlayerController::OnRightMouseClick);

	// Mouse wheel for zoom
	InputComponent->BindAxis("MouseWheelAxis", this, &AROPlayerController::OnMouseWheelAxis);

	// Middle mouse for camera rotation
	InputComponent->BindAction("MiddleMouseButton", IE_Pressed, this, &AROPlayerController::OnMiddleMousePressed);
	InputComponent->BindAction("MiddleMouseButton", IE_Released, this, &AROPlayerController::OnMiddleMouseReleased);

	// UI toggles – standard Ragnarok keybinds
	InputComponent->BindAction("ToggleInventory", IE_Pressed, this, &AROPlayerController::ToggleInventory);
	InputComponent->BindAction("ToggleEquipment", IE_Pressed, this, &AROPlayerController::ToggleEquipment);
	InputComponent->BindAction("ToggleSkillWindow", IE_Pressed, this, &AROPlayerController::ToggleSkillWindow);
	InputComponent->BindAction("ToggleStatsWindow", IE_Pressed, this, &AROPlayerController::ToggleStatsWindow);
	InputComponent->BindAction("ToggleQuestLog", IE_Pressed, this, &AROPlayerController::ToggleQuestLog);
	InputComponent->BindAction("ToggleMap", IE_Pressed, this, &AROPlayerController::ToggleMap);
	InputComponent->BindAction("TogglePartyWindow", IE_Pressed, this, &AROPlayerController::TogglePartyWindow);
	InputComponent->BindAction("ToggleGuildWindow", IE_Pressed, this, &AROPlayerController::ToggleGuildWindow);
	InputComponent->BindAction("ToggleFriendsList", IE_Pressed, this, &AROPlayerController::ToggleFriendsList);
	InputComponent->BindAction("ToggleOptionsMenu", IE_Pressed, this, &AROPlayerController::ToggleOptionsMenu);
	InputComponent->BindAction("ToggleChatWindow", IE_Pressed, this, &AROPlayerController::ToggleChatWindow);
}

void AROPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// Handle click-to-move
	if (bIsMovingToDestination)
	{
		TickClickToMove(DeltaTime);
	}

	// Handle camera rotation while middle mouse is held
	if (bIsRotatingCamera)
	{
		UpdateCameraRotation();
	}
}

// ---------------------------------------------------------------
// Replication
// ---------------------------------------------------------------

void AROPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AROPlayerController, SelectedTarget);
}

// ---------------------------------------------------------------
// Click-to-Move
// ---------------------------------------------------------------

void AROPlayerController::SetDestination(const FVector& Destination)
{
	MoveDestination = Destination;
	bIsMovingToDestination = true;

	// Use the navigation system for pathfinding
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, MoveDestination);
	}

	// Notify the server
	if (!HasAuthority())
	{
		ServerSetDestination(Destination);
	}

	UE_LOG(LogRagnarokUE, Verbose, TEXT("SetDestination – Moving to (%.0f, %.0f, %.0f)."),
		Destination.X, Destination.Y, Destination.Z);
}

void AROPlayerController::StopMovement()
{
	bIsMovingToDestination = false;
	MoveDestination = FVector::ZeroVector;

	// Stop any active pathfinding movement
	if (GetPathFollowingComponent())
	{
		GetPathFollowingComponent()->AbortMove(*this, FPathFollowingResultFlags::UserAbort);
	}

	Super::StopMovement();
}

void AROPlayerController::TickClickToMove(float DeltaTime)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		bIsMovingToDestination = false;
		return;
	}

	const float DistanceToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), MoveDestination);
	if (DistanceToTarget <= MoveAcceptanceRadius)
	{
		bIsMovingToDestination = false;

		UE_LOG(LogRagnarokUE, Verbose, TEXT("TickClickToMove – Arrived at destination."));
	}
}

// ---------------------------------------------------------------
// Camera control
// ---------------------------------------------------------------

void AROPlayerController::ZoomCamera(float Delta)
{
	CameraZoomDistance = FMath::Clamp(
		CameraZoomDistance - (Delta * CameraZoomStep),
		CameraZoomMin,
		CameraZoomMax
	);

	// Apply to the pawn's spring arm if present
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		USpringArmComponent* SpringArm = ControlledPawn->FindComponentByClass<USpringArmComponent>();
		if (SpringArm)
		{
			SpringArm->TargetArmLength = CameraZoomDistance;
		}
	}
}

void AROPlayerController::RotateCamera(float YawDelta)
{
	CameraYawOffset += YawDelta;

	// Wrap to 0-360 range
	CameraYawOffset = FMath::Fmod(CameraYawOffset + 360.0f, 360.0f);

	// Apply to the pawn's spring arm if present
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		USpringArmComponent* SpringArm = ControlledPawn->FindComponentByClass<USpringArmComponent>();
		if (SpringArm)
		{
			FRotator CurrentRotation = SpringArm->GetRelativeRotation();
			CurrentRotation.Yaw = CameraYawOffset;
			SpringArm->SetRelativeRotation(CurrentRotation);
		}
	}
}

// ---------------------------------------------------------------
// Target selection
// ---------------------------------------------------------------

void AROPlayerController::SelectTarget(AActor* NewTarget)
{
	AActor* OldTarget = SelectedTarget;

	if (OldTarget != NewTarget)
	{
		// Only the server should modify the replicated property directly
		if (HasAuthority())
		{
			SelectedTarget = NewTarget;
		}
		else
		{
			// On the client, send to server and let replication update SelectedTarget
			ServerSelectTarget(NewTarget);
		}

		OnTargetChanged.Broadcast(NewTarget);

		if (NewTarget)
		{
			UE_LOG(LogRagnarokUE, Log, TEXT("SelectTarget – Selected %s."), *NewTarget->GetName());
		}
		else
		{
			UE_LOG(LogRagnarokUE, Log, TEXT("SelectTarget – Target cleared."));
		}
	}
}

void AROPlayerController::ClearTarget()
{
	SelectTarget(nullptr);
}

// ---------------------------------------------------------------
// UI toggles
// ---------------------------------------------------------------

void AROPlayerController::ToggleInventory()
{
	OnUIWindowToggled.Broadcast(FName("Inventory"));
	UE_LOG(LogRagnarokUE, Verbose, TEXT("UI Toggle: Inventory"));
}

void AROPlayerController::ToggleEquipment()
{
	OnUIWindowToggled.Broadcast(FName("Equipment"));
	UE_LOG(LogRagnarokUE, Verbose, TEXT("UI Toggle: Equipment"));
}

void AROPlayerController::ToggleSkillWindow()
{
	OnUIWindowToggled.Broadcast(FName("Skills"));
	UE_LOG(LogRagnarokUE, Verbose, TEXT("UI Toggle: Skills"));
}

void AROPlayerController::ToggleStatsWindow()
{
	OnUIWindowToggled.Broadcast(FName("Stats"));
	UE_LOG(LogRagnarokUE, Verbose, TEXT("UI Toggle: Stats"));
}

void AROPlayerController::ToggleQuestLog()
{
	OnUIWindowToggled.Broadcast(FName("QuestLog"));
	UE_LOG(LogRagnarokUE, Verbose, TEXT("UI Toggle: QuestLog"));
}

void AROPlayerController::ToggleMap()
{
	OnUIWindowToggled.Broadcast(FName("Map"));
	UE_LOG(LogRagnarokUE, Verbose, TEXT("UI Toggle: Map"));
}

void AROPlayerController::TogglePartyWindow()
{
	OnUIWindowToggled.Broadcast(FName("Party"));
	UE_LOG(LogRagnarokUE, Verbose, TEXT("UI Toggle: Party"));
}

void AROPlayerController::ToggleGuildWindow()
{
	OnUIWindowToggled.Broadcast(FName("Guild"));
	UE_LOG(LogRagnarokUE, Verbose, TEXT("UI Toggle: Guild"));
}

void AROPlayerController::ToggleFriendsList()
{
	OnUIWindowToggled.Broadcast(FName("Friends"));
	UE_LOG(LogRagnarokUE, Verbose, TEXT("UI Toggle: Friends"));
}

void AROPlayerController::ToggleOptionsMenu()
{
	OnUIWindowToggled.Broadcast(FName("Options"));
	UE_LOG(LogRagnarokUE, Verbose, TEXT("UI Toggle: Options"));
}

void AROPlayerController::ToggleChatWindow()
{
	OnUIWindowToggled.Broadcast(FName("Chat"));
	UE_LOG(LogRagnarokUE, Verbose, TEXT("UI Toggle: Chat"));
}

// ---------------------------------------------------------------
// Server RPCs – Implementation
// ---------------------------------------------------------------

void AROPlayerController::ServerSetDestination_Implementation(const FVector& Destination)
{
	// On the server: move the authoritative pawn
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		MoveDestination = Destination;
		bIsMovingToDestination = true;

		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Destination);

		UE_LOG(LogRagnarokUE, Verbose,
			TEXT("ServerSetDestination – %s moving to (%.0f, %.0f, %.0f)."),
			*GetName(), Destination.X, Destination.Y, Destination.Z);
	}
}

bool AROPlayerController::ServerSetDestination_Validate(const FVector& Destination)
{
	// Basic sanity: reject obviously invalid coordinates
	// (extreme values that could indicate cheating or corruption)
	const float MaxWorldExtent = 500000.0f;
	return Destination.GetAbsMax() < MaxWorldExtent;
}

void AROPlayerController::ServerSelectTarget_Implementation(AActor* NewTarget)
{
	// Validate that the target is a legitimate actor in the world
	if (NewTarget && !NewTarget->IsPendingKillPending())
	{
		SelectedTarget = NewTarget;
		UE_LOG(LogRagnarokUE, Verbose,
			TEXT("ServerSelectTarget – %s targeting %s."),
			*GetName(), *NewTarget->GetName());
	}
	else
	{
		SelectedTarget = nullptr;
	}
}

bool AROPlayerController::ServerSelectTarget_Validate(AActor* NewTarget)
{
	// nullptr is valid (deselect). Otherwise the actor should exist.
	return true;
}

void AROPlayerController::ServerUseSkill_Implementation(int32 SkillID, int32 SkillLevel, AActor* Target)
{
	UE_LOG(LogRagnarokUE, Log,
		TEXT("ServerUseSkill – %s using skill %d (Lv%d) on %s."),
		*GetName(),
		SkillID,
		SkillLevel,
		Target ? *Target->GetName() : TEXT("Self/Ground"));

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	// Validate the player has learned this skill at the requested level
	UROSkillTreeComponent* SkillTree = ControlledPawn->FindComponentByClass<UROSkillTreeComponent>();
	if (!SkillTree)
	{
		UE_LOG(LogRagnarokUE, Warning, TEXT("ServerUseSkill – No SkillTreeComponent on pawn."));
		return;
	}

	const int32 LearnedLevel = SkillTree->GetSkillLevel(SkillID);
	if (LearnedLevel <= 0)
	{
		UE_LOG(LogRagnarokUE, Warning, TEXT("ServerUseSkill – Player has not learned skill %d."), SkillID);
		return;
	}

	// Clamp requested level to what the player has actually learned
	const int32 ActualLevel = FMath::Min(SkillLevel, LearnedLevel);

	// Activate the skill via the Ability System Component
	UAbilitySystemComponent* ASC = ControlledPawn->FindComponentByClass<UAbilitySystemComponent>();
	if (ASC)
	{
		// Store the target for the ability to pick up
		SelectedTarget = Target;

		// Find the ability spec by skill ID tag
		FGameplayTag SkillTag = FGameplayTag::RequestGameplayTag(
			FName(*FString::Printf(TEXT("Skill.ID.%d"), SkillID)), false);
		if (SkillTag.IsValid())
		{
			FGameplayTagContainer TagContainer;
			TagContainer.AddTag(SkillTag);
			ASC->TryActivateAbilitiesByTag(TagContainer);
		}
	}
}

bool AROPlayerController::ServerUseSkill_Validate(int32 SkillID, int32 SkillLevel, AActor* Target)
{
	// Basic validation: skill IDs are positive, levels are within a sane range
	if (SkillID <= 0 || SkillID > 10000)
	{
		return false;
	}
	if (SkillLevel < 1 || SkillLevel > 10)
	{
		return false;
	}

	// Validate target is not pending kill (null is valid for self/ground skills)
	if (Target && Target->IsPendingKillPending())
	{
		return false;
	}

	return true;
}

// ---------------------------------------------------------------
// Input handlers
// ---------------------------------------------------------------

void AROPlayerController::OnLeftMouseClick()
{
	FHitResult HitResult;
	if (!TraceUnderCursor(HitResult))
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();

	// Check if we clicked on a targetable actor (anything that isn't
	// the landscape / static world geometry). In practice this would
	// check for an interface or component tag; for now we check if
	// the actor has a non-default owner or is a Pawn.
	APawn* HitPawn = Cast<APawn>(HitActor);
	if (HitPawn && HitPawn != GetPawn())
	{
		// Clicked on another pawn (monster, NPC proxy, other player)
		SelectTarget(HitPawn);

		// Also move toward the target
		SetDestination(HitPawn->GetActorLocation());
		return;
	}

	// Clicked on ground – move to that position
	ClearTarget();
	SetDestination(HitResult.Location);
}

void AROPlayerController::OnRightMouseClick()
{
	// Right-click: context action on current target, or cancel move
	if (SelectedTarget)
	{
		// If we have a target, attempt to interact (talk to NPC, attack monster, etc.)
		// This will be routed through the appropriate system when implemented.
		UE_LOG(LogRagnarokUE, Verbose, TEXT("OnRightMouseClick – Context action on %s."),
			*SelectedTarget->GetName());
	}
	else
	{
		// No target – stop moving
		StopMovement();
	}
}

void AROPlayerController::OnMouseWheelAxis(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		ZoomCamera(Value);
	}
}

void AROPlayerController::OnMiddleMousePressed()
{
	bIsRotatingCamera = true;
}

void AROPlayerController::OnMiddleMouseReleased()
{
	bIsRotatingCamera = false;
}

void AROPlayerController::UpdateCameraRotation()
{
	float MouseX = 0.0f;
	float MouseY = 0.0f;
	GetInputMouseDelta(MouseX, MouseY);

	if (!FMath::IsNearlyZero(MouseX))
	{
		RotateCamera(MouseX * CameraRotationSpeed);
	}
}

bool AROPlayerController::TraceUnderCursor(FHitResult& OutHit) const
{
	// Get cursor position in screen space
	float LocationX = 0.0f;
	float LocationY = 0.0f;
	if (!GetMousePosition(LocationX, LocationY))
	{
		return false;
	}

	// Deproject from screen to world
	FVector WorldLocation;
	FVector WorldDirection;
	if (!DeprojectScreenPositionToWorld(LocationX, LocationY, WorldLocation, WorldDirection))
	{
		return false;
	}

	// Perform a line trace
	const FVector TraceStart = WorldLocation;
	const FVector TraceEnd = WorldLocation + (WorldDirection * 100000.0f);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetPawn());
	QueryParams.bTraceComplex = false;

	return GetWorld()->LineTraceSingleByChannel(
		OutHit,
		TraceStart,
		TraceEnd,
		ECollisionChannel::ECC_Visibility,
		QueryParams
	);
}
