// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/HitResult.h"
#include "GameFramework/PlayerController.h"
#include "ROPlayerController.generated.h"

/**
 * Delegate broadcast when the player selects a new target (monster, NPC, or player).
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetChanged, AActor*, NewTarget);

/**
 * Delegate broadcast when a UI window toggle is requested.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIWindowToggled, FName, WindowName);

/**
 * AROPlayerController
 *
 * Ragnarok Online-style player controller implementing:
 *   - Click-to-move navigation
 *   - Isometric camera control (zoom via scroll, rotation via middle-mouse)
 *   - Target selection (click on actors)
 *   - UI window toggles (inventory, equipment, skills, etc.)
 *   - Server RPCs for authoritative movement, targeting, and skill use
 */
UCLASS(Blueprintable)
class RAGNAROKUE_API AROPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AROPlayerController();

	//~ APlayerController interface
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;
	//~ End APlayerController interface

	// ---------------------------------------------------------------
	// Click-to-move
	// ---------------------------------------------------------------

	/** Whether the player is currently moving toward a click destination. */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsMovingToDestination;

	/** World-space destination the player is moving toward. */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector MoveDestination;

	/** Acceptance radius – stop moving when this close to the destination. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveAcceptanceRadius;

	/**
	 * Issue a move command to a world location. On the owning client this
	 * sets local state and sends a server RPC.
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetDestination(const FVector& Destination);

	/** Stop any active click-to-move navigation. */
	virtual void StopMovement() override;

	// ---------------------------------------------------------------
	// Camera control
	// ---------------------------------------------------------------

	/** Current camera zoom distance (spring arm length). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraZoomDistance;

	/** Minimum zoom distance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraZoomMin;

	/** Maximum zoom distance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraZoomMax;

	/** Zoom increment per scroll tick. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraZoomStep;

	/** Current camera yaw offset from the default isometric angle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraYawOffset;

	/** Camera rotation speed when dragging with middle mouse. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraRotationSpeed;

	/** Zoom the camera in or out. Positive = zoom in, negative = zoom out. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ZoomCamera(float Delta);

	/** Rotate the camera by a yaw delta (degrees). */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void RotateCamera(float YawDelta);

	// ---------------------------------------------------------------
	// Target selection
	// ---------------------------------------------------------------

	/** The actor the player currently has selected (monster, NPC, player). */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_SelectedTarget, Category = "Target")
	TObjectPtr<AActor> SelectedTarget;

	/** Select a new target actor. nullptr to deselect. */
	UFUNCTION(BlueprintCallable, Category = "Target")
	void SelectTarget(AActor* NewTarget);

	/** Clear the current target selection. */
	UFUNCTION(BlueprintCallable, Category = "Target")
	void ClearTarget();

	/** Broadcast whenever the selected target changes. */
	UPROPERTY(BlueprintAssignable, Category = "Target")
	FOnTargetChanged OnTargetChanged;

	// ---------------------------------------------------------------
	// UI window toggles
	// ---------------------------------------------------------------

	/** Broadcast when any UI window toggle is requested. */
	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnUIWindowToggled OnUIWindowToggled;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleEquipment();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleSkillWindow();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleStatsWindow();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleQuestLog();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleMap();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePartyWindow();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleGuildWindow();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleFriendsList();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleOptionsMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleChatWindow();

	// ---------------------------------------------------------------
	// Server RPCs
	// ---------------------------------------------------------------

	/** Tell the server where we want to move. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetDestination(const FVector& Destination);

	/** Tell the server which actor we selected as target. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSelectTarget(AActor* NewTarget);

	/** Request the server to execute a skill on the current target. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUseSkill(int32 SkillID, int32 SkillLevel, AActor* Target);

	// ---------------------------------------------------------------
	// Replication
	// ---------------------------------------------------------------

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called on client when SelectedTarget is replicated from server. */
	UFUNCTION()
	void OnRep_SelectedTarget();

private:
	// ---------------------------------------------------------------
	// Input handlers
	// ---------------------------------------------------------------

	/** Called on left mouse button press – performs a line trace to set
	 *  destination or select a target depending on what was hit. */
	void OnLeftMouseClick();

	/** Called on right mouse button press. */
	void OnRightMouseClick();

	/** Called on mouse wheel axis input. */
	void OnMouseWheelAxis(float Value);

	/** Called on middle mouse button press / release. */
	void OnMiddleMousePressed();
	void OnMiddleMouseReleased();

	/** Called every frame while middle mouse is held – rotates camera. */
	void UpdateCameraRotation();

	/** Whether the middle mouse button is currently held (for camera rotation). */
	bool bIsRotatingCamera;

	// ---------------------------------------------------------------
	// Movement helpers
	// ---------------------------------------------------------------

	/** Tick-based movement toward MoveDestination using simple move-to. */
	void TickClickToMove(float DeltaTime);

	/**
	 * Perform a line trace from the cursor into the world.
	 * @param OutHit  Filled with the hit result.
	 * @return true if something was hit.
	 */
	bool TraceUnderCursor(FHitResult& OutHit) const;
};
