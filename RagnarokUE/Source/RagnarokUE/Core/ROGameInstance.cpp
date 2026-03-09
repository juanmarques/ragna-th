// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROGameInstance.h"
#include "RagnarokUE/RagnarokUE.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UROGameInstance::UROGameInstance()
	: ServerIP(TEXT("127.0.0.1"))
	, ServerPort(6900)
	, PlayerAccountID(0)
	, SessionToken(TEXT(""))
	, bIsConnected(false)
	, SelectedCharacterSlot(-1)
	, LastSaveMap(NAME_None)
	, LastSavePosition(FVector::ZeroVector)
{
}

void UROGameInstance::Init()
{
	Super::Init();

	UE_LOG(LogRagnarokUE, Log, TEXT("ROGameInstance::Init – Ragnarok UE Game Instance initialised."));

	// Reset connection state on fresh init
	bIsConnected = false;
	ClearSessionData();
}

void UROGameInstance::Shutdown()
{
	UE_LOG(LogRagnarokUE, Log, TEXT("ROGameInstance::Shutdown – Cleaning up."));

	if (bIsConnected)
	{
		DisconnectFromServer();
	}

	Super::Shutdown();
}

// ---------------------------------------------------------------
// Connection
// ---------------------------------------------------------------

bool UROGameInstance::ConnectToServer()
{
	if (bIsConnected)
	{
		UE_LOG(LogRagnarokUE, Warning, TEXT("ConnectToServer – Already connected."));
		return true;
	}

	if (ServerIP.IsEmpty())
	{
		UE_LOG(LogRagnarokUE, Error, TEXT("ConnectToServer – ServerIP is empty."));
		return false;
	}

	if (ServerPort <= 0 || ServerPort > 65535)
	{
		UE_LOG(LogRagnarokUE, Error, TEXT("ConnectToServer – Invalid port %d."), ServerPort);
		return false;
	}

	UE_LOG(LogRagnarokUE, Log, TEXT("ConnectToServer – Attempting connection to %s:%d ..."), *ServerIP, ServerPort);

	if (!InitiateNetworkConnection())
	{
		UE_LOG(LogRagnarokUE, Error, TEXT("ConnectToServer – Network connection failed."));
		return false;
	}

	bIsConnected = true;
	UE_LOG(LogRagnarokUE, Log, TEXT("ConnectToServer – Connection established."));
	return true;
}

void UROGameInstance::DisconnectFromServer()
{
	if (!bIsConnected)
	{
		UE_LOG(LogRagnarokUE, Warning, TEXT("DisconnectFromServer – Not connected."));
		return;
	}

	UE_LOG(LogRagnarokUE, Log, TEXT("DisconnectFromServer – Disconnecting from %s:%d."), *ServerIP, ServerPort);

	// Use UE travel to disconnect any active net connection
	UWorld* World = GetWorld();
	if (World && GEngine)
	{
		GEngine->SetClientTravel(World, TEXT(""), TRAVEL_Absolute);
	}

	bIsConnected = false;
	ClearSessionData();

	UE_LOG(LogRagnarokUE, Log, TEXT("DisconnectFromServer – Disconnected."));
}

bool UROGameInstance::HasValidSession() const
{
	return !SessionToken.IsEmpty() && PlayerAccountID > 0;
}

// ---------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------

bool UROGameInstance::InitiateNetworkConnection()
{
	// Build the travel URL for UE's built-in networking.
	// In a full implementation this would go through our custom
	// packet-based login flow; for now we use UE client travel
	// so the engine sets up the net driver.
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogRagnarokUE, Error, TEXT("InitiateNetworkConnection – No valid world."));
		return false;
	}

	const FString TravelURL = FString::Printf(TEXT("%s:%d"), *ServerIP, ServerPort);
	if (!GEngine)
	{
		UE_LOG(LogRagnarokUE, Error, TEXT("InitiateNetworkConnection – GEngine is null."));
		return false;
	}
	GEngine->SetClientTravel(World, *TravelURL, TRAVEL_Absolute);
	return true;
}

void UROGameInstance::ClearSessionData()
{
	PlayerAccountID = 0;
	SessionToken.Empty();
	SelectedCharacterSlot = -1;
}
