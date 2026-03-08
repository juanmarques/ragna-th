// Copyright Ragna-TH Project. All Rights Reserved.

#include "RODatabaseSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void URODatabaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// FIX 15: Scan existing records to find max IDs and avoid collisions on restart
	NextCharacterID = 1;
	for (const auto& Pair : CharacterDatabase)
	{
		NextCharacterID = FMath::Max(NextCharacterID, Pair.Key + 1);
	}

	NextGuildID = 1;
	for (const auto& Pair : GuildDatabase)
	{
		NextGuildID = FMath::Max(NextGuildID, Pair.Key + 1);
	}

	UE_LOG(LogTemp, Log, TEXT("RODatabaseSubsystem: Initialized"));
}

void URODatabaseSubsystem::Deinitialize()
{
	StopAutoSave();

	// FIX 11: Check validity before attempting final save during shutdown.
	// The world or subsystem may already be torn down at this point.
	if (GetWorld() && DirtyCharacters.Num() > 0 && CharacterDatabase.Num() > 0)
	{
		SaveAllDirtyCharacters();
	}

	CharacterDatabase.Empty();
	GuildDatabase.Empty();
	DirtyCharacters.Empty();

	Super::Deinitialize();
}

bool URODatabaseSubsystem::SaveCharacter(const FROCharacterSaveData& SaveData)
{
	if (!SaveData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("RODatabaseSubsystem: Attempted to save invalid character data"));
		return false;
	}

	FROCharacterSaveData DataToSave = SaveData;
	DataToSave.LastSaveTime = FDateTime::Now();

	CharacterDatabase.Add(SaveData.CharacterID, DataToSave);
	DirtyCharacters.Remove(SaveData.CharacterID);

	UE_LOG(LogTemp, Log, TEXT("RODatabaseSubsystem: Character %d (%s) saved"),
		SaveData.CharacterID, *SaveData.CharacterName);

	OnCharacterSaved.Broadcast(SaveData.CharacterID, true);
	return true;
}

FROCharacterSaveData URODatabaseSubsystem::LoadCharacter(int32 CharacterID) const
{
	const FROCharacterSaveData* Data = CharacterDatabase.Find(CharacterID);
	if (Data)
	{
		UE_LOG(LogTemp, Log, TEXT("RODatabaseSubsystem: Character %d loaded"), CharacterID);
		return *Data;
	}

	UE_LOG(LogTemp, Warning, TEXT("RODatabaseSubsystem: Character %d not found"), CharacterID);
	return FROCharacterSaveData();
}

TArray<FROCharacterSaveData> URODatabaseSubsystem::GetCharactersForAccount(int32 AccountID) const
{
	TArray<FROCharacterSaveData> Result;
	for (const auto& Pair : CharacterDatabase)
	{
		if (Pair.Value.AccountID == AccountID)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

bool URODatabaseSubsystem::DeleteCharacter(int32 CharacterID, int32 AccountID)
{
	// FIX 8: Verify the character belongs to the requesting account before deletion
	const FROCharacterSaveData* CharData = CharacterDatabase.Find(CharacterID);
	if (!CharData)
	{
		UE_LOG(LogTemp, Warning, TEXT("RODatabaseSubsystem: Delete failed - Character %d not found"), CharacterID);
		return false;
	}

	if (CharData->AccountID != AccountID)
	{
		UE_LOG(LogTemp, Warning, TEXT("RODatabaseSubsystem: Delete failed - Character %d does not belong to Account %d (actual owner: %d)"),
			CharacterID, AccountID, CharData->AccountID);
		return false;
	}

	CharacterDatabase.Remove(CharacterID);
	DirtyCharacters.Remove(CharacterID);
	UE_LOG(LogTemp, Log, TEXT("RODatabaseSubsystem: Character %d deleted by Account %d"), CharacterID, AccountID);
	return true;
}

int32 URODatabaseSubsystem::CreateCharacter(int32 AccountID, const FString& CharacterName, EROJobClass StartingClass)
{
	// FIX 8: Validate that AccountID is a positive value (in a real implementation,
	// verify it exists in the auth system's registered accounts)
	if (AccountID <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RODatabaseSubsystem: CreateCharacter failed - invalid AccountID %d"), AccountID);
		return 0;
	}

	if (!IsCharacterNameAvailable(CharacterName))
	{
		UE_LOG(LogTemp, Warning, TEXT("RODatabaseSubsystem: Character name '%s' already taken"), *CharacterName);
		return 0;
	}

	FROCharacterSaveData NewChar;
	NewChar.CharacterID = NextCharacterID++;
	NewChar.AccountID = AccountID;
	NewChar.CharacterName = CharacterName;
	NewChar.JobClass = StartingClass;
	NewChar.BaseLevel = 1;
	NewChar.JobLevel = 1;
	NewChar.BaseEXP = 0;
	NewChar.JobEXP = 0;
	NewChar.CurrentHP = 40;  // Novice starting HP
	NewChar.CurrentSP = 10;  // Novice starting SP
	NewChar.Zeny = 0;
	NewChar.StatusPoints = 48; // Starting stat points
	NewChar.SkillPoints = 0;
	NewChar.SavedMapID = FName(TEXT("prontera"));
	NewChar.SavedLocation = FVector(0.0f, 0.0f, 0.0f);
	NewChar.SavedSpawnMapID = FName(TEXT("prontera"));
	NewChar.LastSaveTime = FDateTime::Now();

	// Default stats (all 1)
	NewChar.Stats.BaseSTR = 1;
	NewChar.Stats.BaseAGI = 1;
	NewChar.Stats.BaseVIT = 1;
	NewChar.Stats.BaseINT = 1;
	NewChar.Stats.BaseDEX = 1;
	NewChar.Stats.BaseLUK = 1;
	NewChar.Stats.RecalculateTotals();

	CharacterDatabase.Add(NewChar.CharacterID, NewChar);

	UE_LOG(LogTemp, Log, TEXT("RODatabaseSubsystem: Character created - ID=%d, Name=%s, Account=%d"),
		NewChar.CharacterID, *CharacterName, AccountID);

	return NewChar.CharacterID;
}

bool URODatabaseSubsystem::SaveGuild(const FROGuildSaveData& SaveData)
{
	if (!SaveData.IsValid())
	{
		return false;
	}

	GuildDatabase.Add(SaveData.GuildID, SaveData);

	UE_LOG(LogTemp, Log, TEXT("RODatabaseSubsystem: Guild %d (%s) saved"),
		SaveData.GuildID, *SaveData.GuildName);

	return true;
}

FROGuildSaveData URODatabaseSubsystem::LoadGuild(int32 GuildID) const
{
	const FROGuildSaveData* Data = GuildDatabase.Find(GuildID);
	if (Data)
	{
		return *Data;
	}
	return FROGuildSaveData();
}

int32 URODatabaseSubsystem::CreateGuild(const FString& GuildName, int32 LeaderCharacterID)
{
	FROGuildSaveData NewGuild;
	NewGuild.GuildID = NextGuildID++;
	NewGuild.GuildName = GuildName;
	NewGuild.LeaderCharacterID = LeaderCharacterID;
	NewGuild.GuildLevel = 1;
	NewGuild.GuildEXP = 0;
	NewGuild.MemberCharacterIDs.Add(LeaderCharacterID);

	GuildDatabase.Add(NewGuild.GuildID, NewGuild);

	UE_LOG(LogTemp, Log, TEXT("RODatabaseSubsystem: Guild created - ID=%d, Name=%s, Leader=%d"),
		NewGuild.GuildID, *GuildName, LeaderCharacterID);

	return NewGuild.GuildID;
}

void URODatabaseSubsystem::StartAutoSave()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		AutoSaveTimerHandle,
		this,
		&URODatabaseSubsystem::OnAutoSaveTimer,
		AutoSaveIntervalSeconds,
		true // Loop
	);

	UE_LOG(LogTemp, Log, TEXT("RODatabaseSubsystem: Auto-save started (interval: %.0f seconds)"),
		AutoSaveIntervalSeconds);
}

void URODatabaseSubsystem::StopAutoSave()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("RODatabaseSubsystem: Auto-save stopped"));
}

void URODatabaseSubsystem::SaveAllDirtyCharacters()
{
	if (DirtyCharacters.Num() == 0)
	{
		return;
	}

	int32 SavedCount = 0;
	TArray<int32> ToSave = DirtyCharacters.Array();

	for (int32 CharID : ToSave)
	{
		const FROCharacterSaveData* Data = CharacterDatabase.Find(CharID);
		if (Data)
		{
			// In production, this would be an async database write
			FROCharacterSaveData UpdatedData = *Data;
			UpdatedData.LastSaveTime = FDateTime::Now();
			CharacterDatabase.Add(CharID, UpdatedData);
			SavedCount++;
		}
	}

	DirtyCharacters.Empty();

	UE_LOG(LogTemp, Log, TEXT("RODatabaseSubsystem: Auto-saved %d characters"), SavedCount);
}

void URODatabaseSubsystem::MarkCharacterDirty(int32 CharacterID)
{
	if (CharacterDatabase.Contains(CharacterID))
	{
		DirtyCharacters.Add(CharacterID);
	}
}

bool URODatabaseSubsystem::IsCharacterNameAvailable(const FString& CharacterName) const
{
	const FString LowerName = CharacterName.ToLower();
	for (const auto& Pair : CharacterDatabase)
	{
		if (Pair.Value.CharacterName.ToLower() == LowerName)
		{
			return false;
		}
	}
	return true;
}

int32 URODatabaseSubsystem::GetTotalCharacterCount() const
{
	return CharacterDatabase.Num();
}

void URODatabaseSubsystem::OnAutoSaveTimer()
{
	UE_LOG(LogTemp, Log, TEXT("RODatabaseSubsystem: Auto-save triggered (%d dirty characters)"),
		DirtyCharacters.Num());

	SaveAllDirtyCharacters();
}
