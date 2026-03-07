// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ROGameInstance.generated.h"

/**
 * UROGameInstance
 *
 * Core game instance for the Ragnarok Online UE5 recreation.
 * Persists across map transitions and holds server connection info,
 * account credentials, and session state.
 */
UCLASS(Blueprintable)
class RAGNAROKUE_API UROGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UROGameInstance();

	//~ UGameInstance interface
	virtual void Init() override;
	virtual void Shutdown() override;
	//~ End UGameInstance interface

	// ---------------------------------------------------------------
	// Connection
	// ---------------------------------------------------------------

	/** IP address of the game server. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	FString ServerIP;

	/** Port of the game server. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	int32 ServerPort;

	/** Account ID received after login authentication. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	int32 PlayerAccountID;

	/** Session token for the current authenticated session. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	FString SessionToken;

	/** True while the client is connected (or attempting to connect) to the server. */
	UPROPERTY(BlueprintReadOnly, Category = "Connection")
	bool bIsConnected;

	// ---------------------------------------------------------------
	// Connection functions
	// ---------------------------------------------------------------

	/**
	 * Attempt to connect to the game server using the current ServerIP / ServerPort.
	 * @return true if the connection attempt was initiated successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Connection")
	bool ConnectToServer();

	/**
	 * Disconnect from the current game server, cleaning up network state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Connection")
	void DisconnectFromServer();

	/**
	 * Check whether we hold a valid, non-expired session token.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Connection")
	bool HasValidSession() const;

	// ---------------------------------------------------------------
	// Subsystem accessors (convenience wrappers)
	// ---------------------------------------------------------------

	/** Get the local player's GameInstance subsystem of the given class. */
	template <typename T>
	T* GetSubsystemChecked() const
	{
		T* Subsystem = GetSubsystem<T>();
		check(Subsystem);
		return Subsystem;
	}

	// ---------------------------------------------------------------
	// Character selection state (carried between maps)
	// ---------------------------------------------------------------

	/** Index of the character slot the player selected at the character-select screen. */
	UPROPERTY(BlueprintReadWrite, Category = "CharacterSelect")
	int32 SelectedCharacterSlot;

	/** The map the selected character should load into. */
	UPROPERTY(BlueprintReadWrite, Category = "CharacterSelect")
	FName LastSaveMap;

	/** The position on that map. */
	UPROPERTY(BlueprintReadWrite, Category = "CharacterSelect")
	FVector LastSavePosition;

private:
	/** Internal helper – opens a network connection to ServerIP:ServerPort. */
	bool InitiateNetworkConnection();

	/** Clears all session-related data. */
	void ClearSessionData();
};
