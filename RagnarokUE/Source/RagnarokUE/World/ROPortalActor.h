// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROPortalActor.generated.h"

class UBoxComponent;
class UParticleSystemComponent;
class UNiagaraComponent;
class UROMapManager;

/**
 * AROPortalActor
 * Placed in the world as a map transition trigger.
 * When a player overlaps the trigger volume and meets requirements,
 * they are teleported to the destination map and location.
 */
UCLASS(ClassGroup=(RagnarokUE), Blueprintable)
class RAGNAROKUE_API AROPortalActor : public AActor
{
	GENERATED_BODY()

public:
	AROPortalActor();

	// ---- Portal Configuration ----

	/** The map ID this portal is on (set to enable cross-map detection). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FName SourceMapID;

	/** The map ID to travel to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FName DestinationMapID;

	/** The world location to place the player at in the destination map. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FVector DestinationLocation = FVector::ZeroVector;

	/** The rotation (yaw) to face the player at after teleporting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	float DestinationRotation = 0.0f;

	/** Required base level to use this portal. 0 means no requirement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	int32 RequiredBaseLevel = 0;

	/** Optional display name shown when approaching the portal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FString PortalDisplayName;

protected:
	virtual void BeginPlay() override;

	/** Trigger volume for detecting player overlap. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<UBoxComponent> TriggerVolume;

	/** Root scene component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Called when an actor enters the portal trigger. */
	UFUNCTION()
	void OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	/**
	 * Check if the player meets the level requirement.
	 * @param PlayerCharacter The player character to check.
	 * @return True if the player meets the requirement.
	 */
	bool MeetsLevelRequirement(AActor* PlayerCharacter) const;

	/** Execute the map travel for a player. */
	void TeleportPlayer(AActor* PlayerCharacter);
};
