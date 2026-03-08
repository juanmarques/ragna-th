// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "RODatabaseSubsystem.generated.h"

/** Save data for a single character. */
USTRUCT(BlueprintType)
struct FROCharacterSaveData
{
	GENERATED_BODY()

	/** Unique character ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 CharacterID = 0;

	/** Owning account ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 AccountID = 0;

	/** Character name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString CharacterName;

	/** Job class. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	EROJobClass JobClass = EROJobClass::Novice;

	/** Base level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 BaseLevel = 1;

	/** Job level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 JobLevel = 1;

	/** Base experience. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int64 BaseEXP = 0;

	/** Job experience. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int64 JobEXP = 0;

	/** Character stats. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FROStatBlock Stats;

	/** Current HP. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 CurrentHP = 0;

	/** Current SP. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 CurrentSP = 0;

	/** Zeny (currency). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int64 Zeny = 0;

	/** Remaining stat points. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 StatusPoints = 0;

	/** Remaining skill points. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 SkillPoints = 0;

	/** Last saved map ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FName SavedMapID;

	/** Last saved world position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FVector SavedLocation = FVector::ZeroVector;

	/** Saved spawn point map (for respawn on death). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FName SavedSpawnMapID;

	/** Inventory items. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TArray<FROItemInstance> Inventory;

	/** Equipped item unique IDs per slot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TMap<EROEquipSlot, FGuid> EquippedItems;

	/** Learned skills. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TArray<FROSkillInfo> Skills;

	/** Guild ID (0 if none). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 GuildID = 0;

	/** Party ID (0 if none). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 PartyID = 0;

	/** Last save timestamp. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FDateTime LastSaveTime;

	bool IsValid() const { return CharacterID > 0 && AccountID > 0; }
};

/** Save data for a guild. */
USTRUCT(BlueprintType)
struct FROGuildSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 GuildID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString GuildName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 LeaderCharacterID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 GuildLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int64 GuildEXP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TArray<int32> MemberCharacterIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString GuildMessage;

	bool IsValid() const { return GuildID > 0; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterSaved, int32, CharacterID, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterLoaded, int32, CharacterID, bool, bSuccess);

/**
 * URODatabaseSubsystem
 * Handles async save/load for characters, inventory, guilds.
 * Includes an auto-save timer (5 minute default).
 * In production, this would interface with a real database backend.
 */
UCLASS()
class RAGNAROKUESERVER_API URODatabaseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- Character Operations ----

	/** Save a character to the database. */
	UFUNCTION(BlueprintCallable, Category = "Database")
	bool SaveCharacter(const FROCharacterSaveData& SaveData);

	/** Load a character from the database. */
	UFUNCTION(BlueprintCallable, Category = "Database")
	FROCharacterSaveData LoadCharacter(int32 CharacterID) const;

	/** Get all characters for an account. */
	UFUNCTION(BlueprintCallable, Category = "Database")
	TArray<FROCharacterSaveData> GetCharactersForAccount(int32 AccountID) const;

	/** Delete a character. Verifies the character belongs to the given AccountID before deletion. */
	UFUNCTION(BlueprintCallable, Category = "Database")
	bool DeleteCharacter(int32 CharacterID, int32 AccountID);

	/** Create a new character and return its ID. */
	UFUNCTION(BlueprintCallable, Category = "Database")
	int32 CreateCharacter(int32 AccountID, const FString& CharacterName, EROJobClass StartingClass);

	// ---- Guild Operations ----

	/** Save a guild to the database. */
	UFUNCTION(BlueprintCallable, Category = "Database")
	bool SaveGuild(const FROGuildSaveData& SaveData);

	/** Load a guild from the database. */
	UFUNCTION(BlueprintCallable, Category = "Database")
	FROGuildSaveData LoadGuild(int32 GuildID) const;

	/** Create a new guild and return its ID. */
	UFUNCTION(BlueprintCallable, Category = "Database")
	int32 CreateGuild(const FString& GuildName, int32 LeaderCharacterID);

	// ---- Auto-Save ----

	/** Start the auto-save timer. */
	UFUNCTION(BlueprintCallable, Category = "Database")
	void StartAutoSave();

	/** Stop the auto-save timer. */
	UFUNCTION(BlueprintCallable, Category = "Database")
	void StopAutoSave();

	/** Manually trigger a save of all dirty characters. */
	UFUNCTION(BlueprintCallable, Category = "Database")
	void SaveAllDirtyCharacters();

	/** Mark a character as needing to be saved on the next auto-save pass. */
	UFUNCTION(BlueprintCallable, Category = "Database")
	void MarkCharacterDirty(int32 CharacterID);

	// ---- Queries ----

	/** Check if a character name is available. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database")
	bool IsCharacterNameAvailable(const FString& CharacterName) const;

	/** Get total character count. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database")
	int32 GetTotalCharacterCount() const;

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category = "Database")
	FOnCharacterSaved OnCharacterSaved;

	UPROPERTY(BlueprintAssignable, Category = "Database")
	FOnCharacterLoaded OnCharacterLoaded;

	// ---- Configuration ----

	/** Auto-save interval in seconds (default 300 = 5 minutes). */
	UPROPERTY(EditAnywhere, Category = "Database")
	float AutoSaveIntervalSeconds = 300.0f;

protected:
	/** In-memory character database (in production, use a real DB). */
	TMap<int32, FROCharacterSaveData> CharacterDatabase;

	/** In-memory guild database. */
	TMap<int32, FROGuildSaveData> GuildDatabase;

	/** Set of character IDs that need saving. */
	TSet<int32> DirtyCharacters;

	/** Next available character ID. */
	int32 NextCharacterID = 1;

	/** Next available guild ID. */
	int32 NextGuildID = 1;

	/** Auto-save timer handle. */
	FTimerHandle AutoSaveTimerHandle;

	/** Called by the auto-save timer. */
	void OnAutoSaveTimer();
};
