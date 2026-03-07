// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RONetworkSubsystem.generated.h"

/** Information about a connected player's network state. */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROConnectionInfo
{
	GENERATED_BODY()

	/** Player's unique net ID string. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	FString PlayerNetID;

	/** Player's account ID. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	int32 AccountID = 0;

	/** Current ping in milliseconds. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	float PingMS = 0.0f;

	/** Map the player is currently on. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	FName CurrentMapID;

	/** Time of connection. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	FDateTime ConnectedAt;

	/** Whether this connection is flagged for suspicious activity. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	bool bIsFlagged = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerConnected, const FROConnectionInfo&, ConnectionInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDisconnected, const FString&, PlayerNetID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnServerTransferRequested, const FString&, PlayerNetID, FName, DestinationMapID);

/**
 * URONetworkSubsystem
 * Manages network connections, ping tracking, and server transfers for map changes.
 * Provides a central place to monitor and control player connections.
 */
UCLASS()
class RAGNAROKUE_API URONetworkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- Connection Management ----

	/** Register a new player connection. */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void RegisterConnection(const FString& PlayerNetID, int32 AccountID);

	/** Unregister a player connection. */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void UnregisterConnection(const FString& PlayerNetID);

	/** Update the ping for a specific player. */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void UpdatePing(const FString& PlayerNetID, float PingMS);

	/** Update the current map for a player. */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void UpdatePlayerMap(const FString& PlayerNetID, FName MapID);

	// ---- Queries ----

	/** Get connection info for a specific player. */
	UFUNCTION(BlueprintCallable, Category = "Network")
	FROConnectionInfo GetConnectionInfo(const FString& PlayerNetID) const;

	/** Get all active connections. */
	UFUNCTION(BlueprintCallable, Category = "Network")
	TArray<FROConnectionInfo> GetAllConnections() const;

	/** Get the number of connected players. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Network")
	int32 GetPlayerCount() const;

	/** Get the average ping across all connections. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Network")
	float GetAveragePing() const;

	/** Check if a player is connected. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Network")
	bool IsPlayerConnected(const FString& PlayerNetID) const;

	// ---- Server Transfer ----

	/**
	 * Request a server transfer for a player to a different map.
	 * Used when map changes require a different server instance or level streaming.
	 * @param PlayerNetID The player's network ID.
	 * @param DestinationMapID The target map.
	 * @param DestinationLocation The spawn location on the destination map.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void RequestServerTransfer(const FString& PlayerNetID, FName DestinationMapID, FVector DestinationLocation);

	/**
	 * Execute a seamless server travel for all players on the current server.
	 * @param MapAssetPath The UE level asset path (e.g., "/Game/Maps/prontera").
	 */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void ExecuteServerTravel(const FString& MapAssetPath);

	// ---- Flagging ----

	/** Flag a player connection for suspicious activity. */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void FlagConnection(const FString& PlayerNetID, const FString& Reason);

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnPlayerConnected OnPlayerConnected;

	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnPlayerDisconnected OnPlayerDisconnected;

	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnServerTransferRequested OnServerTransferRequested;

protected:
	/** Active player connections keyed by NetID. */
	TMap<FString, FROConnectionInfo> ActiveConnections;

	/** Ping history for smoothing (last N samples per player). */
	TMap<FString, TArray<float>> PingHistory;

	/** Maximum ping samples to keep per player. */
	static constexpr int32 MaxPingSamples = 10;

	/** Calculate smoothed ping from history. */
	float CalculateSmoothedPing(const FString& PlayerNetID) const;
};
