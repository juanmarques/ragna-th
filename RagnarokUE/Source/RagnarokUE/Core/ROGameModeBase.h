// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ROGameModeBase.generated.h"

class AROPlayerController;
class AROPlayerState;
class AROGameStateBase;

/**
 * AROGameModeBase
 *
 * Server-authoritative game mode for the Ragnarok Online UE5 recreation.
 * Manages player login/logout, spawning at save points, death handling,
 * and sets the default framework classes (pawn, controller, player state,
 * game state, HUD).
 */
UCLASS(Blueprintable)
class RAGNAROKUE_API AROGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AROGameModeBase();

	//~ AGameModeBase interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	//~ End AGameModeBase interface

	// ---------------------------------------------------------------
	// Player management
	// ---------------------------------------------------------------

	/**
	 * Called after PostLogin to set up RO-specific state for the new player.
	 * Assigns character data from the session, notifies other systems.
	 */
	UFUNCTION(BlueprintCallable, Category = "Players")
	void HandleNewPlayer(APlayerController* NewPlayer);

	/**
	 * Respawn a player at the appropriate save point for their map/level.
	 * Destroys the old pawn if still alive, then spawns a new one.
	 * @param PlayerController The controller to respawn.
	 */
	UFUNCTION(BlueprintCallable, Category = "Players")
	void RespawnPlayer(APlayerController* PlayerController);

	/**
	 * Handle a player death event. Applies death penalty (EXP loss),
	 * notifies the party/guild, then queues a respawn after a delay.
	 * @param DeadController The controller whose pawn died.
	 * @param KillerController The controller responsible for the kill (may be nullptr for environmental death).
	 */
	UFUNCTION(BlueprintCallable, Category = "Players")
	void HandlePlayerDeath(APlayerController* DeadController, AController* KillerController);

	// ---------------------------------------------------------------
	// Configuration
	// ---------------------------------------------------------------

	/** Time in seconds before a dead player is automatically respawned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	float RespawnDelay;

	/** Base experience loss percentage on death (e.g. 0.01 = 1%). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	float DeathExpPenaltyPercent;

	/** Tag used to find save-point PlayerStarts on the map. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	FName SavePointTag;

private:
	/**
	 * Find the save-point PlayerStart closest to the given location.
	 * Falls back to any available PlayerStart if no tagged ones exist.
	 */
	AActor* FindSavePointForPlayer(AController* Player) const;

	/**
	 * Apply the death EXP penalty to a player.
	 */
	void ApplyDeathPenalty(AROPlayerState* PS) const;

	/** Pending respawn timers keyed by controller unique ID, so they can be cancelled on logout. */
	TMap<int32, FTimerHandle> PendingRespawnTimers;
};
