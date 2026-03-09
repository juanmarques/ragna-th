// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ROWorldSubsystem.generated.h"

/** Per-map player tracking data. */
USTRUCT(BlueprintType)
struct FROMapPlayerInfo
{
	GENERATED_BODY()

	/** Map identifier. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World")
	FName MapID;

	/** Player network IDs currently on this map. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World")
	TArray<FString> PlayerNetIDs;

	/** Number of players on this map. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World")
	int32 PlayerCount = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerEnteredMap, FName, MapID, const FString&, PlayerNetID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLeftMap, FName, MapID, const FString&, PlayerNetID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldPlayerDisconnected, const FString&, PlayerNetID);

/**
 * UROWorldSubsystem
 * Server-side world management. Tracks player-per-map distribution,
 * provides broadcast functions, and manages player count per map.
 */
UCLASS()
class RAGNAROKUESERVER_API UROWorldSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- Player Tracking ----

	/**
	 * Register a player entering a map.
	 * @param MapID       The map the player entered.
	 * @param PlayerNetID The player's network identifier.
	 */
	UFUNCTION(BlueprintCallable, Category = "World")
	void PlayerEnteredMap(FName MapID, const FString& PlayerNetID);

	/**
	 * Register a player leaving a map.
	 * @param MapID       The map the player left.
	 * @param PlayerNetID The player's network identifier.
	 */
	UFUNCTION(BlueprintCallable, Category = "World")
	void PlayerLeftMap(FName MapID, const FString& PlayerNetID);

	/**
	 * Move a player between maps (convenience: removes from old, adds to new).
	 * @param PlayerNetID The player's network identifier.
	 * @param OldMapID    The map the player is leaving.
	 * @param NewMapID    The map the player is entering.
	 */
	UFUNCTION(BlueprintCallable, Category = "World")
	void PlayerChangedMap(const FString& PlayerNetID, FName OldMapID, FName NewMapID);

	// ---- Queries ----

	/** Get the number of players on a specific map. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World")
	int32 GetPlayerCountOnMap(FName MapID) const;

	/** Get total players across all maps. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World")
	int32 GetTotalPlayerCount() const;

	/** Get all player NetIDs currently on a map. */
	UFUNCTION(BlueprintCallable, Category = "World")
	TArray<FString> GetPlayersOnMap(FName MapID) const;

	/** Get per-map player info for all maps. */
	UFUNCTION(BlueprintCallable, Category = "World")
	TArray<FROMapPlayerInfo> GetAllMapPlayerInfo() const;

	/** Get the map a specific player is on. Returns NAME_None if not found. */
	UFUNCTION(BlueprintCallable, Category = "World")
	FName GetPlayerMap(const FString& PlayerNetID) const;

	/** Get player count on a specific map. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World")
	int32 GetMapPlayerCount(FName MapID) const;

	// ---- Player Disconnect / Cleanup ----

	/**
	 * Completely remove a player from all tracking (disconnect cleanup).
	 * Looks up the player's current map, removes them from MapPlayers and PlayerToMap,
	 * cleans up empty map entries, and broadcasts the disconnect delegate.
	 * @param PlayerNetID The player's network identifier (AccountID_CharacterID or equivalent unique key).
	 */
	UFUNCTION(BlueprintCallable, Category = "World")
	void RemovePlayerCompletely(const FString& PlayerNetID);

	// ---- Broadcast Functions ----

	/**
	 * Broadcast a message to all players on a specific map.
	 * @param MapID   The target map.
	 * @param Message The message content.
	 */
	UFUNCTION(BlueprintCallable, Category = "World")
	void BroadcastToMap(FName MapID, const FString& Message);

	/**
	 * Broadcast a message to all players on all maps (server-wide).
	 * @param Message The message content.
	 */
	UFUNCTION(BlueprintCallable, Category = "World")
	void BroadcastToAll(const FString& Message);

	/**
	 * Broadcast a message to all players within a radius on a specific map.
	 * @param MapID    The target map.
	 * @param Origin   The center of the broadcast area.
	 * @param Radius   The broadcast radius in UE units.
	 * @param Message  The message content.
	 */
	UFUNCTION(BlueprintCallable, Category = "World")
	void BroadcastToArea(FName MapID, FVector Origin, float Radius, const FString& Message);

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category = "World")
	FOnPlayerEnteredMap OnPlayerEnteredMap;

	UPROPERTY(BlueprintAssignable, Category = "World")
	FOnPlayerLeftMap OnPlayerLeftMap;

	UPROPERTY(BlueprintAssignable, Category = "World")
	FOnWorldPlayerDisconnected OnPlayerDisconnected;

protected:
	/** Per-map player tracking. */
	TMap<FName, TArray<FString>> MapPlayers;

	/** Reverse lookup: player -> current map. */
	TMap<FString, FName> PlayerToMap;
};
