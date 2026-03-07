// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWorldSubsystem.h"

void UROWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("ROWorldSubsystem: Initialized"));
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

	TArray<FString>& Players = MapPlayers.FindOrAdd(MapID);
	if (!Players.Contains(PlayerNetID))
	{
		Players.Add(PlayerNetID);
	}

	PlayerToMap.Add(PlayerNetID, MapID);

	UE_LOG(LogTemp, Log, TEXT("ROWorldSubsystem: Player %s entered map %s (%d players)"),
		*PlayerNetID, *MapID.ToString(), Players.Num());

	OnPlayerEnteredMap.Broadcast(MapID, PlayerNetID);
}

void UROWorldSubsystem::PlayerLeftMap(FName MapID, const FString& PlayerNetID)
{
	if (MapID.IsNone() || PlayerNetID.IsEmpty())
	{
		return;
	}

	TArray<FString>* Players = MapPlayers.Find(MapID);
	if (Players)
	{
		Players->Remove(PlayerNetID);

		UE_LOG(LogTemp, Log, TEXT("ROWorldSubsystem: Player %s left map %s (%d players remaining)"),
			*PlayerNetID, *MapID.ToString(), Players->Num());
	}

	// Only remove from reverse lookup if they were on this map
	const FName* CurrentMap = PlayerToMap.Find(PlayerNetID);
	if (CurrentMap && *CurrentMap == MapID)
	{
		PlayerToMap.Remove(PlayerNetID);
	}

	OnPlayerLeftMap.Broadcast(MapID, PlayerNetID);
}

void UROWorldSubsystem::PlayerChangedMap(const FString& PlayerNetID, FName OldMapID, FName NewMapID)
{
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

void UROWorldSubsystem::BroadcastToMap(FName MapID, const FString& Message)
{
	const TArray<FString>* Players = MapPlayers.Find(MapID);
	if (!Players)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ROWorldSubsystem: Broadcast to map %s (%d players): %s"),
		*MapID.ToString(), Players->Num(), *Message);

	// TODO: Send the message to each player's client via RPC.
	// for (const FString& NetID : *Players)
	// {
	//     // Find the player controller by NetID and call a client RPC
	//     // PlayerController->ClientShowMessage(Message);
	// }
}

void UROWorldSubsystem::BroadcastToAll(const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("ROWorldSubsystem: Server-wide broadcast (%d total players): %s"),
		GetTotalPlayerCount(), *Message);

	for (const auto& Pair : MapPlayers)
	{
		BroadcastToMap(Pair.Key, Message);
	}
}

void UROWorldSubsystem::BroadcastToArea(FName MapID, FVector Origin, float Radius, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("ROWorldSubsystem: Area broadcast on map %s at %s (radius=%.0f): %s"),
		*MapID.ToString(), *Origin.ToString(), Radius, *Message);

	// TODO: In production:
	// 1. Get all player controllers on the target map
	// 2. Check distance from Origin to each player's location
	// 3. Send message only to players within Radius
	// This requires integration with the player character location tracking.
}
