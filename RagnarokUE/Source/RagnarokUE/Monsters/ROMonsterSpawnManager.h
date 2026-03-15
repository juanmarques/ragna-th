// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROMonsterSpawnManager.generated.h"

class AROMonsterBase;

/**
 * FRORespawnEntry
 * Tracks a pending respawn: which spawn definition and when to respawn.
 */
USTRUCT()
struct FRORespawnEntry
{
	GENERATED_BODY()

	UPROPERTY()
	int32 SpawnDefIndex = -1;

	UPROPERTY()
	float RespawnTime = 0.0f;

	FRORespawnEntry() {}
	FRORespawnEntry(int32 InIndex, float InTime) : SpawnDefIndex(InIndex), RespawnTime(InTime) {}
};

/**
 * AROMonsterSpawnManager
 * Placed per map zone. Manages spawning and respawning of monsters
 * based on configurable spawn definitions.
 */
UCLASS(Blueprintable)
class RAGNAROKUE_API AROMonsterSpawnManager : public AActor
{
	GENERATED_BODY()

public:
	AROMonsterSpawnManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ---- Spawn Definitions ----

	/** Spawn definitions for this zone. Configure per-monster spawn rules. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Manager")
	TArray<FROMonsterSpawnInfo> SpawnDefinitions;

	/** Currently active (alive) monsters managed by this spawner. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Manager")
	TArray<TObjectPtr<AROMonsterBase>> ActiveMonsters;

	/** Monster class to spawn (should be AROMonsterBase or a BP subclass). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Manager")
	TSubclassOf<AROMonsterBase> MonsterClass;

	// ---- Functions ----

	/** Spawn a monster based on a spawn definition. */
	UFUNCTION(BlueprintCallable, Category = "Spawn Manager")
	AROMonsterBase* SpawnMonster(const FROMonsterSpawnInfo& Info);

	/** Helper: spawn a single monster by ID at a specific location. Pass InSpawnDefIndex for managed definitions. */
	UFUNCTION(BlueprintCallable, Category = "Spawn Manager")
	AROMonsterBase* SpawnSingleMonster(int32 MonsterID, FVector Location, int32 InSpawnDefIndex = -1);

	/** Called when a managed monster dies. Starts respawn timer. */
	UFUNCTION()
	void OnMonsterDied(AROMonsterBase* Monster, AActor* Killer);

protected:
	/** Pending respawns queue. */
	UPROPERTY()
	TArray<FRORespawnEntry> RespawnQueue;

	/** Count of currently alive monsters per spawn definition index. */
	UPROPERTY()
	TMap<int32, int32> AliveCountPerDef;

	/** Do the initial spawn for all definitions at BeginPlay. */
	void PerformInitialSpawn();

	/** Process respawn queue, spawn when timers expire. */
	void ProcessRespawnQueue(float CurrentTime);

	/** Get a random spawn location within the spawn definition's radius. */
	FVector GetRandomSpawnLocation(const FROMonsterSpawnInfo& Info) const;

	/** Find the spawn definition index for a given monster. */
	int32 FindSpawnDefIndex(const AROMonsterBase* Monster) const;

	/** Returns true when the current world time is considered nighttime. */
	bool IsCurrentlyNightTime() const;

	/** Returns true when a spawn definition is allowed to spawn at current time. */
	bool IsSpawnDefinitionActiveAtCurrentTime(const FROMonsterSpawnInfo& Info) const;
};
