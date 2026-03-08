// Copyright Ragna-TH Project. All Rights Reserved.

#include "RONetworkSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

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

	// FIX 7: Check for duplicate NetID and clean up old connection state before overwriting
	if (ActiveConnections.Contains(PlayerNetID))
	{
		UE_LOG(LogTemp, Warning, TEXT("RONetworkSubsystem: Duplicate NetID=%s detected. Cleaning up old connection state before re-registering."),
			*PlayerNetID);

		// Clean up old connection data
		PingHistory.Remove(PlayerNetID);
		ActiveConnections.Remove(PlayerNetID);
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

	// FIX 4: Validate map path against allowlist to prevent unauthorized ServerTravel
	static const TSet<FString> AllowedMaps = {
		TEXT("/Game/Maps/prontera"),
		TEXT("/Game/Maps/prt_fild01"),
		TEXT("/Game/Maps/prt_fild02"),
		TEXT("/Game/Maps/prt_fild03"),
		TEXT("/Game/Maps/prt_fild04"),
		TEXT("/Game/Maps/prt_fild05"),
		TEXT("/Game/Maps/prt_fild06"),
		TEXT("/Game/Maps/prt_fild07"),
		TEXT("/Game/Maps/prt_fild08"),
		TEXT("/Game/Maps/prt_sewb1"),
		TEXT("/Game/Maps/prt_sewb2"),
		TEXT("/Game/Maps/prt_sewb3"),
		TEXT("/Game/Maps/prt_sewb4"),
		TEXT("/Game/Maps/izlude"),
		TEXT("/Game/Maps/iz_dun01"),
		TEXT("/Game/Maps/iz_dun02"),
		TEXT("/Game/Maps/iz_dun03"),
		TEXT("/Game/Maps/iz_dun04"),
		TEXT("/Game/Maps/iz_dun05"),
	};

	if (!AllowedMaps.Contains(MapAssetPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("RONetworkSubsystem: Blocked unauthorized ServerTravel to: %s"), *MapAssetPath);
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

		// FIX 13: Take action on flagged connections - kick the player
		UWorld* World = GetWorld();
		if (World)
		{
			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				APlayerController* PC = It->Get();
				if (!PC) continue;

				APlayerState* PS = PC->GetPlayerState<APlayerState>();
				if (PS && PS->GetUniqueId().ToString() == PlayerNetID)
				{
					PC->ClientReturnToMainMenuWithTextReason(
						FText::FromString(TEXT("Disconnected: suspicious activity detected")));
					break;
				}
			}
		}
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
