// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROSpawnPoint.generated.h"

class AROCharacterBase;

/**
 * AROSpawnPoint
 * Placed in the world to define save/respawn locations for players.
 * Players can save their spawn point at Kafra NPCs or by other means.
 * When a player dies, they respawn at their saved spawn point.
 */
UCLASS(ClassGroup=(RagnarokUE), Blueprintable)
class RAGNAROKUE_API AROSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AROSpawnPoint();

	// ---- Spawn Point Configuration ----

	/** Map ID this spawn point belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	FName MapID;

	/** Human-readable name for this spawn point (e.g., "Prontera Center", "Izlude Dock"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	FString SpawnName;

	/** Whether this is the default spawn point for the map (used for new characters or fallback). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	bool bIsDefaultSpawn = false;

	// ---- Functions ----

	/**
	 * Set this spawn point as a player's respawn location.
	 * Stores the spawn point reference on the player for use on death.
	 * @param Player The player character to assign this spawn to.
	 */
	UFUNCTION(BlueprintCallable, Category = "SpawnPoint")
	void SetPlayerSpawnPoint(AActor* Player);

	/** Get the transform for spawning a player at this point. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SpawnPoint")
	FTransform GetSpawnTransform() const;

	/** Get the spawn location. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SpawnPoint")
	FVector GetSpawnLocation() const;

	/** Get the spawn rotation. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SpawnPoint")
	FRotator GetSpawnRotation() const;

protected:
	/** Root scene component for placement. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoint")
	TObjectPtr<USceneComponent> SceneRoot;

#if WITH_EDITORONLY_DATA
	/** Billboard for visibility in the editor. */
	UPROPERTY(VisibleAnywhere, Category = "SpawnPoint")
	TObjectPtr<UBillboardComponent> EditorSprite;
#endif
};
