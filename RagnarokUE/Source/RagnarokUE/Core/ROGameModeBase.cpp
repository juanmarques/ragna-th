// Copyright Ragna-TH Project. All Rights Reserved.

#include "Core/ROGameModeBase.h"
#include "Core/ROPlayerController.h"
#include "Core/ROPlayerState.h"
#include "Core/ROGameStateBase.h"
#include "RagnarokUE.h"
#include "RagnarokUE/Character/ROLevelingComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

AROGameModeBase::AROGameModeBase()
	: RespawnDelay(3.0f)
	, DeathExpPenaltyPercent(0.01f)
	, SavePointTag(TEXT("SavePoint"))
{
	// Set the default framework classes for the Ragnarok game mode.
	// These can be overridden in Blueprint child classes.
	PlayerControllerClass = AROPlayerController::StaticClass();
	PlayerStateClass = AROPlayerState::StaticClass();
	GameStateClass = AROGameStateBase::StaticClass();

	// Default pawn and HUD are left to engine defaults here;
	// projects should override in a Blueprint subclass or via
	// DefaultPawnClass / HUDClass assignments once those classes exist.
	// DefaultPawnClass = AROCharacterBase::StaticClass();  // TODO: set when character class is ready
	// HUDClass = AROHUD::StaticClass();                    // TODO: set when HUD class is ready

	// Disable seamless travel by default (RO uses explicit map loads)
	bUseSeamlessTravel = false;
}

void AROGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogRagnarokUE, Log, TEXT("ROGameModeBase::InitGame – Map: %s  Options: %s"), *MapName, *Options);
}

// ---------------------------------------------------------------
// Login / Logout
// ---------------------------------------------------------------

void AROGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	UE_LOG(LogRagnarokUE, Log, TEXT("ROGameModeBase::PostLogin – Player %s joined."),
		*NewPlayer->GetName());

	HandleNewPlayer(NewPlayer);
}

void AROGameModeBase::Logout(AController* Exiting)
{
	if (Exiting)
	{
		UE_LOG(LogRagnarokUE, Log, TEXT("ROGameModeBase::Logout – Player %s leaving."),
			*Exiting->GetName());

		// Update online player count on the game state
		AROGameStateBase* GS = GetGameState<AROGameStateBase>();
		if (GS)
		{
			// Count will be corrected on next tick, but do an immediate decrement
			// for responsiveness.
			GS->OnlinePlayerCount = FMath::Max(0, GS->OnlinePlayerCount - 1);
		}
	}

	Super::Logout(Exiting);
}

// ---------------------------------------------------------------
// Player Start selection
// ---------------------------------------------------------------

AActor* AROGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	// Try to find a save-point tagged PlayerStart for this player
	AActor* SavePoint = FindSavePointForPlayer(Player);
	if (SavePoint)
	{
		return SavePoint;
	}

	// Fall back to the engine default
	return Super::ChoosePlayerStart_Implementation(Player);
}

UClass* AROGameModeBase::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// In the future this would look at the player's job class and return
	// the correct pawn class. For now, return the mode's default.
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

// ---------------------------------------------------------------
// HandleNewPlayer
// ---------------------------------------------------------------

void AROGameModeBase::HandleNewPlayer(APlayerController* NewPlayer)
{
	if (!NewPlayer)
	{
		return;
	}

	AROPlayerState* PS = NewPlayer->GetPlayerState<AROPlayerState>();
	if (!PS)
	{
		UE_LOG(LogRagnarokUE, Warning,
			TEXT("HandleNewPlayer – PlayerState is not AROPlayerState for %s."),
			*NewPlayer->GetName());
		return;
	}

	// Log the new player with their character identity
	UE_LOG(LogRagnarokUE, Log,
		TEXT("HandleNewPlayer – %s (Lv%d %s) entered the game."),
		*PS->GetCharacterName(),
		PS->GetBaseLevel(),
		*UEnum::GetValueAsString(PS->GetJobClass()));

	// If the player doesn't have a pawn yet (e.g., first spawn), spawn one
	if (!NewPlayer->GetPawn())
	{
		RestartPlayer(NewPlayer);
	}
}

// ---------------------------------------------------------------
// Respawn
// ---------------------------------------------------------------

void AROGameModeBase::RespawnPlayer(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	UE_LOG(LogRagnarokUE, Log, TEXT("RespawnPlayer – Respawning %s."),
		*PlayerController->GetName());

	// Destroy the old pawn if it still exists
	APawn* OldPawn = PlayerController->GetPawn();
	if (OldPawn)
	{
		PlayerController->UnPossess();
		OldPawn->Destroy();
	}

	// Use the engine's RestartPlayer which calls ChoosePlayerStart and
	// spawns the default pawn at that location.
	RestartPlayer(PlayerController);
}

// ---------------------------------------------------------------
// Death handling
// ---------------------------------------------------------------

void AROGameModeBase::HandlePlayerDeath(APlayerController* DeadController, AController* KillerController)
{
	if (!DeadController)
	{
		return;
	}

	const FString KillerName = KillerController ? KillerController->GetName() : TEXT("Environment");
	UE_LOG(LogRagnarokUE, Log, TEXT("HandlePlayerDeath – %s killed by %s."),
		*DeadController->GetName(), *KillerName);

	// Apply death EXP penalty
	AROPlayerState* PS = DeadController->GetPlayerState<AROPlayerState>();
	if (PS)
	{
		ApplyDeathPenalty(PS);
	}

	// Queue respawn after the configured delay
	FTimerHandle RespawnTimerHandle;
	TWeakObjectPtr<APlayerController> WeakDeadController(DeadController);

	GetWorldTimerManager().SetTimer(
		RespawnTimerHandle,
		[this, WeakDeadController]()
		{
			if (APlayerController* PC = WeakDeadController.Get())
			{
				RespawnPlayer(PC);
			}
		},
		RespawnDelay,
		false
	);
}

// ---------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------

AActor* AROGameModeBase::FindSavePointForPlayer(AController* Player) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AActor* BestStart = nullptr;
	float BestDistSq = FLT_MAX;

	// Get the player's current location (or zero if no pawn yet)
	FVector PlayerLocation = FVector::ZeroVector;
	if (Player && Player->GetPawn())
	{
		PlayerLocation = Player->GetPawn()->GetActorLocation();
	}

	// Iterate all PlayerStart actors looking for ones tagged as save points
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* Start = *It;
		if (!Start)
		{
			continue;
		}

		if (Start->ActorHasTag(SavePointTag))
		{
			const float DistSq = FVector::DistSquared(Start->GetActorLocation(), PlayerLocation);
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestStart = Start;
			}
		}
	}

	// If no tagged save point was found, try to find any PlayerStart
	if (!BestStart)
	{
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			BestStart = *It;
			break;
		}
	}

	return BestStart;
}

void AROGameModeBase::ApplyDeathPenalty(AROPlayerState* PS) const
{
	if (!PS || DeathExpPenaltyPercent <= 0.0f)
	{
		return;
	}

	// In Ragnarok Online, death costs a percentage of the current level's
	// total required EXP. The actual EXP deduction would reference the
	// EXP table; for now we log the intent. The stats/exp component will
	// implement the actual deduction.
	UE_LOG(LogRagnarokUE, Log,
		TEXT("ApplyDeathPenalty – %s (Lv%d) loses %.1f%% base EXP."),
		*PS->GetCharacterName(),
		PS->GetBaseLevel(),
		DeathExpPenaltyPercent * 100.0f);

	APawn* Pawn = PS->GetPawn();
	if (Pawn)
	{
		UROLevelingComponent* LevelComp = Pawn->FindComponentByClass<UROLevelingComponent>();
		if (LevelComp)
		{
			const int64 RequiredExp = LevelComp->GetRequiredBaseExp();
			const int64 Penalty = static_cast<int64>(static_cast<float>(RequiredExp) * DeathExpPenaltyPercent);
			LevelComp->CurrentBaseExp = FMath::Max(static_cast<int64>(0), LevelComp->CurrentBaseExp - Penalty);
		}
	}
}
