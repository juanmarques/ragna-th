// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Data/ROEnums.h"
#include "ROPlayerState.generated.h"

/**
 * AROPlayerState
 *
 * Holds replicated per-player data visible to all clients:
 * character name, levels, job, guild/party membership.
 */
UCLASS(Blueprintable)
class RAGNAROKUE_API AROPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AROPlayerState();

	//~ AActor interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor interface

	// ---------------------------------------------------------------
	// Replicated character identity
	// ---------------------------------------------------------------

	/** In-game character name (not the same as the account name). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character")
	FString CharacterName;

	/** Base (adventure) level – 1 to 99 (or 255 for renewal). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character")
	int32 BaseLevel;

	/** Job level – 1 to 50/70 depending on class. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character")
	int32 JobLevel;

	/** Current job class. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character")
	EROJobClass JobClass;

	// ---------------------------------------------------------------
	// Guild & Party
	// ---------------------------------------------------------------

	/** Guild ID (0 = no guild). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Social")
	int32 GuildID;

	/** Guild name (empty if not in a guild). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Social")
	FString GuildName;

	/** Party ID (0 = no party). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Social")
	int32 PartyID;

	// ---------------------------------------------------------------
	// Accessors (BlueprintPure)
	// ---------------------------------------------------------------

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	FString GetCharacterName() const { return CharacterName; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	int32 GetBaseLevel() const { return BaseLevel; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	int32 GetJobLevel() const { return JobLevel; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	EROJobClass GetJobClass() const { return JobClass; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Social")
	bool IsInGuild() const { return GuildID > 0; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Social")
	bool IsInParty() const { return PartyID > 0; }
};
