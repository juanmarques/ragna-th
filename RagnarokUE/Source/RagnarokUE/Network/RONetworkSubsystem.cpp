// Copyright Ragna-TH Project. All Rights Reserved.

#include "RONetworkSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void URONetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("RONetworkSubsystem: Initialized"));
}

void URONetworkSubsystem::Deinitialize()
{
	ActiveConnections.Empty();
	PingHistory.Empty();

	Super::Deinitialize();
}

void URONetworkSubsystem::RegisterConnection(const FString& PlayerNetID, int32 AccountID)
{
	if (PlayerNetID.IsEmpty())
	{
		return;
	}

	FROConnectionInfo Info;
	Info.PlayerNetID = PlayerNetID;
	Info.AccountID = AccountID;
	Info.PingMS = 0.0f;
	Info.ConnectedAt = FDateTime::Now();
	Info.bIsFlagged = false;

	ActiveConnections.Add(PlayerNetID, Info);
	PingHistory.Add(PlayerNetID, TArray<float>());

	UE_LOG(LogTemp, Log, TEXT("RONetworkSubsystem: Player connected - NetID=%s, AccountID=%d"),
		*PlayerNetID, AccountID);

	OnPlayerConnected.Broadcast(Info);
}

void URONetworkSubsystem::UnregisterConnection(const FString& PlayerNetID)
{
	if (ActiveConnections.Remove(PlayerNetID) > 0)
	{
		PingHistory.Remove(PlayerNetID);

		UE_LOG(LogTemp, Log, TEXT("RONetworkSubsystem: Player disconnected - NetID=%s"), *PlayerNetID);

		OnPlayerDisconnected.Broadcast(PlayerNetID);
	}
}

void URONetworkSubsystem::UpdatePing(const FString& PlayerNetID, float PingMS)
{
	FROConnectionInfo* Info = ActiveConnections.Find(PlayerNetID);
	if (!Info)
	{
		return;
	}

	// Add to ping history
	TArray<float>* History = PingHistory.Find(PlayerNetID);
	if (History)
	{
		History->Add(PingMS);
		if (History->Num() > MaxPingSamples)
		{
			History->RemoveAt(0);
		}
	}

	// Update with smoothed ping
	Info->PingMS = CalculateSmoothedPing(PlayerNetID);
}

void URONetworkSubsystem::UpdatePlayerMap(const FString& PlayerNetID, FName MapID)
{
	FROConnectionInfo* Info = ActiveConnections.Find(PlayerNetID);
	if (Info)
	{
		Info->CurrentMapID = MapID;
	}
}

FROConnectionInfo URONetworkSubsystem::GetConnectionInfo(const FString& PlayerNetID) const
{
	const FROConnectionInfo* Info = ActiveConnections.Find(PlayerNetID);
	if (Info)
	{
		return *Info;
	}
	return FROConnectionInfo();
}

TArray<FROConnectionInfo> URONetworkSubsystem::GetAllConnections() const
{
	TArray<FROConnectionInfo> Result;
	ActiveConnections.GenerateValueArray(Result);
	return Result;
}

int32 URONetworkSubsystem::GetPlayerCount() const
{
	return ActiveConnections.Num();
}

float URONetworkSubsystem::GetAveragePing() const
{
	if (ActiveConnections.Num() == 0)
	{
		return 0.0f;
	}

	float TotalPing = 0.0f;
	for (const auto& Pair : ActiveConnections)
	{
		TotalPing += Pair.Value.PingMS;
	}
	return TotalPing / static_cast<float>(ActiveConnections.Num());
}

bool URONetworkSubsystem::IsPlayerConnected(const FString& PlayerNetID) const
{
	return ActiveConnections.Contains(PlayerNetID);
}

void URONetworkSubsystem::RequestServerTransfer(const FString& PlayerNetID, FName DestinationMapID, FVector DestinationLocation)
{
	if (!ActiveConnections.Contains(PlayerNetID))
	{
		UE_LOG(LogTemp, Warning, TEXT("RONetworkSubsystem: Transfer requested for unknown player %s"), *PlayerNetID);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("RONetworkSubsystem: Server transfer requested - Player=%s, Map=%s, Location=%s"),
		*PlayerNetID, *DestinationMapID.ToString(), *DestinationLocation.ToString());

	OnServerTransferRequested.Broadcast(PlayerNetID, DestinationMapID);

	// TODO: In a multi-server setup, this would:
	// 1. Save player state to database
	// 2. Find/allocate the destination map server
	// 3. Generate a transfer token
	// 4. Send the client the new server address + token
	// 5. Client disconnects and reconnects to the destination server
}

void URONetworkSubsystem::ExecuteServerTravel(const FString& MapAssetPath)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("RONetworkSubsystem: Cannot execute server travel - no world"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("RONetworkSubsystem: Executing server travel to %s"), *MapAssetPath);

	const FString TravelURL = FString::Printf(TEXT("%s?listen"), *MapAssetPath);
	World->ServerTravel(TravelURL);
}

void URONetworkSubsystem::FlagConnection(const FString& PlayerNetID, const FString& Reason)
{
	FROConnectionInfo* Info = ActiveConnections.Find(PlayerNetID);
	if (Info)
	{
		Info->bIsFlagged = true;
		UE_LOG(LogTemp, Warning, TEXT("RONetworkSubsystem: Player %s flagged - Reason: %s"),
			*PlayerNetID, *Reason);
	}
}

float URONetworkSubsystem::CalculateSmoothedPing(const FString& PlayerNetID) const
{
	const TArray<float>* History = PingHistory.Find(PlayerNetID);
	if (!History || History->Num() == 0)
	{
		return 0.0f;
	}

	// Simple average of recent ping samples
	float Total = 0.0f;
	for (float Sample : *History)
	{
		Total += Sample;
	}
	return Total / static_cast<float>(History->Num());
}
