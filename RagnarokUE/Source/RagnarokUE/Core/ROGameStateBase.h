// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Data/ROEnums.h"
#include "ROGameStateBase.generated.h"

/**
 * AROGameStateBase
 *
 * Replicated game-wide state: server time, War of Emperium status,
 * weather, online player count, and server identity.
 */
UCLASS(Blueprintable)
class RAGNAROKUE_API AROGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	AROGameStateBase();

	//~ AActor interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;
	//~ End AActor interface

	// ---------------------------------------------------------------
	// Replicated server state
	// ---------------------------------------------------------------

	/** Server-authoritative game time in seconds (wraps every 24h of in-game time). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Server")
	float ServerTime;

	/** Whether the War of Emperium event is currently active. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Server")
	bool bIsWoEActive;

	/** Current weather on this map. Drives VFX/audio on clients. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Server")
	EROWeatherType CurrentWeather;

	/** Display name of this server (e.g. "Loki", "Chaos", "Sakray"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Server")
	FString ServerName;

	/** Number of players currently connected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Server")
	int32 OnlinePlayerCount;

	/** In-game day/night cycle speed multiplier (1.0 = real-time). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
	float DayNightSpeedMultiplier;

	// ---------------------------------------------------------------
	// Accessors
	// ---------------------------------------------------------------

	/** Current server time in seconds. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Server")
	float GetServerTime() const { return ServerTime; }

	/** Is War of Emperium currently running? */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Server")
	bool IsWoEActive() const { return bIsWoEActive; }

	/** Current weather type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Server")
	EROWeatherType GetCurrentWeather() const { return CurrentWeather; }

	/** Number of connected players. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Server")
	int32 GetOnlinePlayerCount() const { return OnlinePlayerCount; }

	// ---------------------------------------------------------------
	// Server-only mutators
	// ---------------------------------------------------------------

	/** Set weather (server authority). */
	UFUNCTION(BlueprintCallable, Category = "Server", meta = (BlueprintAuthorityOnly))
	void SetWeather(EROWeatherType NewWeather);

	/** Activate or deactivate War of Emperium (server authority). */
	UFUNCTION(BlueprintCallable, Category = "Server", meta = (BlueprintAuthorityOnly))
	void SetWoEActive(bool bActive);
};
