// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWorldSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogROWorld, Log, All);

void UROWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogROWorld, Log, TEXT("ROWorldSubsystem initialized."));
}

void UROWorldSubsystem::Deinitialize()
{
	PlayersPerMap.Empty();
	PlayerMapLookup.Empty();
	Super::Deinitialize();
}

void UROWorldSubsystem::OnPlayerChangeMap(int32 PlayerID, FName FromMap, FName ToMap)
{
	// Remove from old map
	if (!FromMap.IsNone())
	{
		TArray<int32>* OldMapPlayers = PlayersPerMap.Find(FromMap);
		if (OldMapPlayers)
		{
			OldMapPlayers->Remove(PlayerID);

			// Clean up empty map entries
			if (OldMapPlayers->Num() == 0)
			{
				PlayersPerMap.Remove(FromMap);
			}
		}
	}

	// Add to new map
	if (!ToMap.IsNone())
	{
		TArray<int32>& NewMapPlayers = PlayersPerMap.FindOrAdd(ToMap);
		NewMapPlayers.AddUnique(PlayerID);

		PlayerMapLookup.Add(PlayerID, ToMap);
	}

	UE_LOG(LogROWorld, Log, TEXT("Player %d moved from '%s' to '%s'"),
		PlayerID,
		FromMap.IsNone() ? TEXT("None") : *FromMap.ToString(),
		ToMap.IsNone() ? TEXT("None") : *ToMap.ToString());

	OnPlayerMapChanged.Broadcast(PlayerID, FromMap, ToMap);
}

void UROWorldSubsystem::OnPlayerLogout(int32 PlayerID)
{
	const FName* CurrentMap = PlayerMapLookup.Find(PlayerID);
	if (CurrentMap)
	{
		TArray<int32>* MapPlayers = PlayersPerMap.Find(*CurrentMap);
		if (MapPlayers)
		{
			MapPlayers->Remove(PlayerID);
			if (MapPlayers->Num() == 0)
			{
				PlayersPerMap.Remove(*CurrentMap);
			}
		}
	}

	PlayerMapLookup.Remove(PlayerID);
	UE_LOG(LogROWorld, Log, TEXT("Player %d logged out."), PlayerID);
}

TArray<int32> UROWorldSubsystem::GetPlayersInMap(FName MapID) const
{
	const TArray<int32>* Players = PlayersPerMap.Find(MapID);
	if (Players)
	{
		return *Players;
	}
	return TArray<int32>();
}

int32 UROWorldSubsystem::GetPlayerCount() const
{
	return PlayerMapLookup.Num();
}

int32 UROWorldSubsystem::GetPlayerCountInMap(FName MapID) const
{
	const TArray<int32>* Players = PlayersPerMap.Find(MapID);
	return Players ? Players->Num() : 0;
}

FName UROWorldSubsystem::GetPlayerMap(int32 PlayerID) const
{
	const FName* MapID = PlayerMapLookup.Find(PlayerID);
	return MapID ? *MapID : NAME_None;
}

TArray<FName> UROWorldSubsystem::GetActiveMaps() const
{
	TArray<FName> Maps;
	PlayersPerMap.GetKeys(Maps);
	return Maps;
}

void UROWorldSubsystem::BroadcastToMap(FName MapID, const FString& Message)
{
	UE_LOG(LogROWorld, Log, TEXT("[MAP:%s] %s"), *MapID.ToString(), *Message);
	OnMapBroadcast.Broadcast(MapID, Message);

	// In a full implementation, this would iterate over all player controllers
	// in the specified map and call their chat/notification functions.
}

void UROWorldSubsystem::BroadcastToAll(const FString& Message)
{
	UE_LOG(LogROWorld, Log, TEXT("[SERVER] %s"), *Message);
	OnServerBroadcast.Broadcast(Message);

	// In a full implementation, this would iterate over all connected
	// player controllers and send the message to each.
}
