// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROMVPManager.generated.h"

class AROMonsterBase;

/**
 * FROMVPTombstoneInfo
 * Data for the tombstone actor spawned at MVP death location.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROMVPTombstoneInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "MVP")
	int32 MonsterID = 0;

	UPROPERTY(BlueprintReadOnly, Category = "MVP")
	FString MonsterName;

	UPROPERTY(BlueprintReadOnly, Category = "MVP")
	FString KillerName;

	UPROPERTY(BlueprintReadOnly, Category = "MVP")
	FVector DeathLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "MVP")
	FDateTime KillTime;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMVPKilled, int32, MonsterID, const FString&, KillerName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMVPSpawned, int32, MonsterID, FVector, Location);

/**
 * UROMVPManager
 * Server-wide MVP tracking subsystem.
 * Tracks kill times, respawn timers, and broadcasts MVP events.
 */
UCLASS()
class RAGNAROKUE_API UROMVPManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- MVP Kill/Respawn Tracking ----

	/** Map of MonsterID -> last time this MVP was killed. */
	UPROPERTY()
	TMap<int32, FDateTime> LastMVPKillTime;

	/** Map of MonsterID -> respawn time in seconds. */
	UPROPERTY()
	TMap<int32, float> MVPRespawnTimers;

	/** Active tombstones. */
	UPROPERTY()
	TArray<FROMVPTombstoneInfo> ActiveTombstones;

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category = "MVP|Events")
	FOnMVPKilled OnMVPKilled;

	UPROPERTY(BlueprintAssignable, Category = "MVP|Events")
	FOnMVPSpawned OnMVPSpawned;

	// ---- Functions ----

	/**
	 * Called when an MVP is killed.
	 * Records kill time, broadcasts announcement, schedules respawn.
	 */
	UFUNCTION(BlueprintCallable, Category = "MVP")
	void NotifyMVPKilled(int32 MonsterID, const FString& KillerName, FVector DeathLocation);

	/**
	 * Called when an MVP spawns.
	 * Broadcasts to server.
	 */
	UFUNCTION(BlueprintCallable, Category = "MVP")
	void NotifyMVPSpawned(int32 MonsterID, FVector Location);

	/**
	 * Get the next expected spawn time for an MVP.
	 * Returns FDateTime::MinValue() if no kill recorded.
	 */
	UFUNCTION(BlueprintCallable, Category = "MVP")
	FDateTime GetNextMVPSpawnTime(int32 MonsterID) const;

	/**
	 * Set the respawn timer for a specific MVP.
	 * Call during initialization to configure per-MVP respawn.
	 */
	UFUNCTION(BlueprintCallable, Category = "MVP")
	void SetMVPRespawnTimer(int32 MonsterID, float RespawnSeconds);

	/**
	 * Get all active tombstones.
	 */
	UFUNCTION(BlueprintCallable, Category = "MVP")
	const TArray<FROMVPTombstoneInfo>& GetActiveTombstones() const { return ActiveTombstones; }

	/**
	 * Check if an MVP is currently due to respawn.
	 */
	UFUNCTION(BlueprintCallable, Category = "MVP")
	bool IsMVPReadyToRespawn(int32 MonsterID) const;

protected:
	/** Initialize default MVP respawn timers. */
	void InitializeDefaultTimers();
};
