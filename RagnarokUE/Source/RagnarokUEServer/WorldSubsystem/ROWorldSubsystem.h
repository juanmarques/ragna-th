// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ROWorldSubsystem.generated.h"

/**
 * UROWorldSubsystem
 * Server-only subsystem that manages multiple map zones,
 * tracks players per map, and provides broadcasting functions.
 */
UCLASS()
class RAGNAROKUESERVER_API UROWorldSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Player Map Tracking ---

	/**
	 * Notify that a player has changed maps.
	 * @param PlayerID Character ID of the player.
	 * @param FromMap Previous map ID (NAME_None if first login).
	 * @param ToMap New map ID.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|World")
	void OnPlayerChangeMap(int32 PlayerID, FName FromMap, FName ToMap);

	/**
	 * Remove a player from tracking (logout).
	 * @param PlayerID Character ID of the player.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|World")
	void OnPlayerLogout(int32 PlayerID);

	/**
	 * Get all player IDs in a specific map.
	 * @param MapID Map to query.
	 * @return Array of character IDs.
	 */
	UFUNCTION(BlueprintPure, Category = "RO|World")
	TArray<int32> GetPlayersInMap(FName MapID) const;

	/**
	 * Get the total number of online players across all maps.
	 * @return Total online player count.
	 */
	UFUNCTION(BlueprintPure, Category = "RO|World")
	int32 GetPlayerCount() const;

	/**
	 * Get the number of players in a specific map.
	 * @param MapID Map to query.
	 * @return Player count in that map.
	 */
	UFUNCTION(BlueprintPure, Category = "RO|World")
	int32 GetPlayerCountInMap(FName MapID) const;

	/**
	 * Get the map a player is currently on.
	 * @param PlayerID Character ID.
	 * @return Map ID (NAME_None if not tracked).
	 */
	UFUNCTION(BlueprintPure, Category = "RO|World")
	FName GetPlayerMap(int32 PlayerID) const;

	/**
	 * Get all active map IDs.
	 * @return Array of map names that have players.
	 */
	UFUNCTION(BlueprintPure, Category = "RO|World")
	TArray<FName> GetActiveMaps() const;

	// --- Broadcasting ---

	/**
	 * Broadcast a message to all players in a specific map.
	 * @param MapID Map to broadcast to.
	 * @param Message Message text.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|World")
	void BroadcastToMap(FName MapID, const FString& Message);

	/**
	 * Broadcast a message to all online players.
	 * @param Message Message text.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|World")
	void BroadcastToAll(const FString& Message);

	// --- Delegates ---

	/** Broadcast when a player changes map. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerMapChanged, int32, PlayerID, FName, FromMap, FName, ToMap);

	UPROPERTY(BlueprintAssignable, Category = "RO|World")
	FOnPlayerMapChanged OnPlayerMapChanged;

	/** Broadcast when a map-wide message is sent. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMapBroadcast, FName, MapID, const FString&, Message);

	UPROPERTY(BlueprintAssignable, Category = "RO|World")
	FOnMapBroadcast OnMapBroadcast;

	/** Broadcast when a server-wide message is sent. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnServerBroadcast, const FString&, Message);

	UPROPERTY(BlueprintAssignable, Category = "RO|World")
	FOnServerBroadcast OnServerBroadcast;

private:
	/** Map of MapID -> Array of PlayerIDs. */
	TMap<FName, TArray<int32>> PlayersPerMap;

	/** Reverse lookup: PlayerID -> MapID. */
	TMap<int32, FName> PlayerMapLookup;
};
