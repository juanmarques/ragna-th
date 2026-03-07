// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROMVPManager.h"
#include "ROMonsterBase.h"
#include "Engine/World.h"

void UROMVPManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeDefaultTimers();

	UE_LOG(LogTemp, Log, TEXT("ROMVPManager initialized with %d MVP respawn timers."), MVPRespawnTimers.Num());
}

void UROMVPManager::Deinitialize()
{
	LastMVPKillTime.Empty();
	MVPRespawnTimers.Empty();
	ActiveTombstones.Empty();

	Super::Deinitialize();
}

void UROMVPManager::NotifyMVPKilled(int32 MonsterID, const FString& KillerName, FVector DeathLocation)
{
	// Record kill time
	LastMVPKillTime.Add(MonsterID, FDateTime::UtcNow());

	// Create tombstone
	FROMVPTombstoneInfo Tombstone;
	Tombstone.MonsterID = MonsterID;
	Tombstone.MonsterName = FString::Printf(TEXT("MVP #%d"), MonsterID);
	Tombstone.KillerName = KillerName;
	Tombstone.DeathLocation = DeathLocation;
	Tombstone.KillTime = FDateTime::UtcNow();
	ActiveTombstones.Add(Tombstone);

	// Broadcast kill announcement
	OnMVPKilled.Broadcast(MonsterID, KillerName);

	UE_LOG(LogTemp, Log, TEXT("MVP %d killed by %s at %s. Tombstone placed."),
		MonsterID, *KillerName, *DeathLocation.ToString());

	// Calculate respawn time
	const float* RespawnDelay = MVPRespawnTimers.Find(MonsterID);
	const float Delay = RespawnDelay ? *RespawnDelay : 3600.0f; // Default 1 hour

	UE_LOG(LogTemp, Log, TEXT("MVP %d will respawn in %.0f seconds."), MonsterID, Delay);
}

void UROMVPManager::NotifyMVPSpawned(int32 MonsterID, FVector Location)
{
	// Remove tombstone for this MVP
	ActiveTombstones.RemoveAll([MonsterID](const FROMVPTombstoneInfo& Info)
	{
		return Info.MonsterID == MonsterID;
	});

	// Broadcast spawn announcement
	OnMVPSpawned.Broadcast(MonsterID, Location);

	UE_LOG(LogTemp, Log, TEXT("MVP %d has spawned at %s!"), MonsterID, *Location.ToString());
}

FDateTime UROMVPManager::GetNextMVPSpawnTime(int32 MonsterID) const
{
	const FDateTime* KillTime = LastMVPKillTime.Find(MonsterID);
	if (!KillTime)
	{
		return FDateTime::MinValue();
	}

	const float* RespawnDelay = MVPRespawnTimers.Find(MonsterID);
	const float Delay = RespawnDelay ? *RespawnDelay : 3600.0f;

	// Add respawn delay (in seconds) to kill time
	return *KillTime + FTimespan::FromSeconds(static_cast<double>(Delay));
}

void UROMVPManager::SetMVPRespawnTimer(int32 MonsterID, float RespawnSeconds)
{
	MVPRespawnTimers.Add(MonsterID, RespawnSeconds);
}

bool UROMVPManager::IsMVPReadyToRespawn(int32 MonsterID) const
{
	const FDateTime NextSpawn = GetNextMVPSpawnTime(MonsterID);
	if (NextSpawn == FDateTime::MinValue())
	{
		return false; // Never killed, no respawn needed
	}

	return FDateTime::UtcNow() >= NextSpawn;
}

void UROMVPManager::InitializeDefaultTimers()
{
	// Default MVP respawn timers (1 to 2 hours, configurable per MVP)
	// These are placeholder values - real values should be loaded from data

	// Common field MVPs (1 hour base)
	// Baphomet, Dark Lord, etc. would be added here as the monster DB expands

	// For now, set a generic default that will be used if no specific timer is set
	// Specific timers can be set via SetMVPRespawnTimer or data table loading
}
