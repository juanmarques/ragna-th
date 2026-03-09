// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWorldSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"

// Fix #5: Dedicated log category instead of LogTemp
DEFINE_LOG_CATEGORY_STATIC(LogROWorld, Log, All);

void UROWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogROWorld, Log, TEXT("ROWorldSubsystem: Initialized"));
}

void UROWorldSubsystem::Deinitialize()
{
	MapPlayers.Empty();
	PlayerToMap.Empty();

	Super::Deinitialize();
}

void UROWorldSubsystem::PlayerEnteredMap(FName MapID, const FString& PlayerNetID)
{
	if (MapID.IsNone() || PlayerNetID.IsEmpty())
	{
		return;
	}

	// FIX 14: If the player is already tracked on a different map, remove them from the old map first
	const FName* CurrentMap = PlayerToMap.Find(PlayerNetID);
	if (CurrentMap && *CurrentMap != MapID)
	{
		PlayerLeftMap(*CurrentMap, PlayerNetID);
	}

	TArray<FString>& Players = MapPlayers.FindOrAdd(MapID);
	if (!Players.Contains(PlayerNetID))
	{
		Players.Add(PlayerNetID);
	}

	PlayerToMap.Add(PlayerNetID, MapID);

	UE_LOG(LogROWorld, Log, TEXT("ROWorldSubsystem: Player %s entered map %s (%d players)"),
		*PlayerNetID, *MapID.ToString(), Players.Num());

	OnPlayerEnteredMap.Broadcast(MapID, PlayerNetID);
}

void UROWorldSubsystem::PlayerLeftMap(FName MapID, const FString& PlayerNetID)
{
	if (MapID.IsNone() || PlayerNetID.IsEmpty())
	{
		return;
	}

	// Fix #7: Validate the player is actually on the specified map before removal
	const FName* CurrentMap = PlayerToMap.Find(PlayerNetID);
	if (CurrentMap && *CurrentMap != MapID)
	{
		UE_LOG(LogROWorld, Warning, TEXT("ROWorldSubsystem: Player %s is on map %s, not %s. Skipping removal."),
			*PlayerNetID, *CurrentMap->ToString(), *MapID.ToString());
		return;
	}

	TArray<FString>* Players = MapPlayers.Find(MapID);
	if (Players)
	{
		Players->Remove(PlayerNetID);

		UE_LOG(LogROWorld, Log, TEXT("ROWorldSubsystem: Player %s left map %s (%d players remaining)"),
			*PlayerNetID, *MapID.ToString(), Players->Num());

		// Fix #4: Remove empty map entries after last player leaves
		if (Players->Num() == 0)
		{
			MapPlayers.Remove(MapID);
		}
	}

	// Only remove from reverse lookup if they were on this map
	if (CurrentMap && *CurrentMap == MapID)
	{
		PlayerToMap.Remove(PlayerNetID);
	}

	OnPlayerLeftMap.Broadcast(MapID, PlayerNetID);
}

void UROWorldSubsystem::PlayerChangedMap(const FString& PlayerNetID, FName OldMapID, FName NewMapID)
{
	// Fix #6: Guard against no-op map transitions
	if (OldMapID == NewMapID)
	{
		return;
	}

	if (!OldMapID.IsNone())
	{
		PlayerLeftMap(OldMapID, PlayerNetID);
	}

	if (!NewMapID.IsNone())
	{
		PlayerEnteredMap(NewMapID, PlayerNetID);
	}
}

int32 UROWorldSubsystem::GetPlayerCountOnMap(FName MapID) const
{
	const TArray<FString>* Players = MapPlayers.Find(MapID);
	return Players ? Players->Num() : 0;
}

int32 UROWorldSubsystem::GetTotalPlayerCount() const
{
	return PlayerToMap.Num();
}

int32 UROWorldSubsystem::GetMapPlayerCount(FName MapID) const
{
	// Fix #12: Explicit map player count helper (same as GetPlayerCountOnMap, provided for API consistency)
	const TArray<FString>* Players = MapPlayers.Find(MapID);
	return Players ? Players->Num() : 0;
}

TArray<FString> UROWorldSubsystem::GetPlayersOnMap(FName MapID) const
{
	const TArray<FString>* Players = MapPlayers.Find(MapID);
	return Players ? *Players : TArray<FString>();
}

TArray<FROMapPlayerInfo> UROWorldSubsystem::GetAllMapPlayerInfo() const
{
	TArray<FROMapPlayerInfo> Result;
	for (const auto& Pair : MapPlayers)
	{
		FROMapPlayerInfo Info;
		Info.MapID = Pair.Key;
		Info.PlayerNetIDs = Pair.Value;
		Info.PlayerCount = Pair.Value.Num();
		Result.Add(Info);
	}
	return Result;
}

FName UROWorldSubsystem::GetPlayerMap(const FString& PlayerNetID) const
{
	const FName* MapID = PlayerToMap.Find(PlayerNetID);
	return MapID ? *MapID : NAME_None;
}

// Fix #8: Disconnect / cleanup path
void UROWorldSubsystem::RemovePlayerCompletely(const FString& PlayerNetID)
{
	if (PlayerNetID.IsEmpty())
	{
		return;
	}

	// Look up current map
	const FName* CurrentMapPtr = PlayerToMap.Find(PlayerNetID);
	if (CurrentMapPtr)
	{
		const FName CurrentMap = *CurrentMapPtr;

		// Remove from MapPlayers
		TArray<FString>* Players = MapPlayers.Find(CurrentMap);
		if (Players)
		{
			Players->Remove(PlayerNetID);

			// Clean up empty map entry
			if (Players->Num() == 0)
			{
				MapPlayers.Remove(CurrentMap);
			}
		}

		OnPlayerLeftMap.Broadcast(CurrentMap, PlayerNetID);
	}

	// Remove from reverse lookup
	PlayerToMap.Remove(PlayerNetID);

	UE_LOG(LogROWorld, Log, TEXT("ROWorldSubsystem: Player %s completely removed (disconnect)"), *PlayerNetID);

	// Broadcast disconnect delegate
	OnPlayerDisconnected.Broadcast(PlayerNetID);
}

// Fix #9: Implement BroadcastToMap with actual player controller iteration
void UROWorldSubsystem::BroadcastToMap(FName MapID, const FString& Message)
{
	const TArray<FString>* Players = MapPlayers.Find(MapID);
	if (!Players || Players->Num() == 0)
	{
		return;
	}

	UE_LOG(LogROWorld, Log, TEXT("ROWorldSubsystem: Broadcast to map %s (%d players): %s"),
		*MapID.ToString(), Players->Num(), *Message);

	// Build a set of target NetIDs for fast lookup
	TSet<FString> TargetNetIDs;
	for (const FString& NetID : *Players)
	{
		TargetNetIDs.Add(NetID);
	}

	// Iterate world player controllers and deliver message to those on the target map
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			continue;
		}

		// Use the player's UniqueId as the NetID key
		const FString NetID = PC->GetPlayerState<APlayerState>()
			? PC->GetPlayerState<APlayerState>()->GetUniqueId()->ToString()
			: FString();

		if (TargetNetIDs.Contains(NetID))
		{
			// Deliver via ClientMessage (standard UE server-to-client text message)
			PC->ClientMessage(Message);
		}
	}
}

void UROWorldSubsystem::BroadcastToAll(const FString& Message)
{
	UE_LOG(LogROWorld, Log, TEXT("ROWorldSubsystem: Server-wide broadcast (%d total players): %s"),
		GetTotalPlayerCount(), *Message);

	// Broadcast to all connected player controllers directly
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC)
		{
			PC->ClientMessage(Message);
		}
	}
}

// Fix #10: Implement BroadcastToArea with spatial filtering
void UROWorldSubsystem::BroadcastToArea(FName MapID, FVector Origin, float Radius, const FString& Message)
{
	const TArray<FString>* Players = MapPlayers.Find(MapID);
	if (!Players || Players->Num() == 0)
	{
		return;
	}

	UE_LOG(LogROWorld, Log, TEXT("ROWorldSubsystem: Area broadcast on map %s at %s (radius=%.0f): %s"),
		*MapID.ToString(), *Origin.ToString(), Radius, *Message);

	// Build set of target NetIDs on this map
	TSet<FString> TargetNetIDs;
	for (const FString& NetID : *Players)
	{
		TargetNetIDs.Add(NetID);
	}

	const float RadiusSq = Radius * Radius;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			continue;
		}

		const FString NetID = PC->GetPlayerState<APlayerState>()
			? PC->GetPlayerState<APlayerState>()->GetUniqueId()->ToString()
			: FString();

		if (!TargetNetIDs.Contains(NetID))
		{
			continue;
		}

		// Spatial filtering: check pawn distance to origin
		APawn* Pawn = PC->GetPawn();
		if (!Pawn)
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(Pawn->GetActorLocation(), Origin);
		if (DistSq <= RadiusSq)
		{
			PC->ClientMessage(Message);
		}
	}
}
