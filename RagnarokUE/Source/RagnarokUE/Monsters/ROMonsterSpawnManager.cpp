// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROMonsterSpawnManager.h"
#include "ROMonsterBase.h"
#include "ROMonsterDatabase.h"
#include "RagnarokUE/Core/ROGameStateBase.h"
#include "RagnarokUE/Social/ROChatSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

AROMonsterSpawnManager::AROMonsterSpawnManager()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	MonsterClass = AROMonsterBase::StaticClass();
}

void AROMonsterSpawnManager::BeginPlay()
{
	Super::BeginPlay();

	// Only spawn on server
	if (HasAuthority())
	{
		PerformInitialSpawn();
	}
}

void AROMonsterSpawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && RespawnQueue.Num() > 0)
	{
		const float CurrentTime = GetWorld()->GetTimeSeconds();
		ProcessRespawnQueue(CurrentTime);
	}
}

void AROMonsterSpawnManager::PerformInitialSpawn()
{
	for (int32 DefIndex = 0; DefIndex < SpawnDefinitions.Num(); ++DefIndex)
	{
		const FROMonsterSpawnInfo& Def = SpawnDefinitions[DefIndex];
		AliveCountPerDef.Add(DefIndex, 0);

		if (!IsSpawnDefinitionActiveAtCurrentTime(Def))
		{
			continue;
		}

		for (int32 i = 0; i < Def.Count; ++i)
		{
			AROMonsterBase* Monster = SpawnMonster(Def);
			if (Monster)
			{
				Monster->SpawnDefIndex = DefIndex;
				int32& Count = AliveCountPerDef.FindOrAdd(DefIndex);
				Count++;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("SpawnManager %s: Initial spawn complete. %d active monsters."),
		*GetName(), ActiveMonsters.Num());
}

AROMonsterBase* AROMonsterSpawnManager::SpawnMonster(const FROMonsterSpawnInfo& Info)
{
	if (!IsSpawnDefinitionActiveAtCurrentTime(Info))
	{
		return nullptr;
	}

	if (!MonsterClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnManager: No MonsterClass set!"));
		return nullptr;
	}

	const FVector SpawnLoc = GetRandomSpawnLocation(Info);
	const FRotator SpawnRot = FRotator(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Owner = this;

	AROMonsterBase* Monster = GetWorld()->SpawnActor<AROMonsterBase>(MonsterClass, SpawnLoc, SpawnRot, SpawnParams);
	if (!Monster)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnManager: Failed to spawn monster ID %d"), Info.MonsterID);
		return nullptr;
	}

	// Initialize from database
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UROMonsterDatabase* MonsterDB = GI->GetSubsystem<UROMonsterDatabase>();
		if (MonsterDB)
		{
			FROMonsterData Data;
			if (MonsterDB->GetMonsterData(Info.MonsterID, Data))
			{
				Monster->InitializeFromData(Data);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("SpawnManager: No database entry for monster ID %d"), Info.MonsterID);
			}
		}
	}

	// Store spawn location for return-to-home
	Monster->SpawnLocation = SpawnLoc;

	// Bind death delegate
	Monster->OnMonsterDied.AddDynamic(this, &AROMonsterSpawnManager::OnMonsterDied);

	ActiveMonsters.Add(Monster);

	return Monster;
}

AROMonsterBase* AROMonsterSpawnManager::SpawnSingleMonster(int32 MonsterID, FVector Location, int32 InSpawnDefIndex)
{
	FROMonsterSpawnInfo TempInfo;
	TempInfo.MonsterID = MonsterID;
	TempInfo.SpawnCenter = Location;
	TempInfo.SpawnRadius = 0.0f;
	TempInfo.Count = 1;
	TempInfo.RespawnDelay = 0.0f;

	AROMonsterBase* Monster = SpawnMonster(TempInfo);
	if (Monster)
	{
		Monster->SpawnDefIndex = InSpawnDefIndex;
	}
	return Monster;
}

void AROMonsterSpawnManager::OnMonsterDied(AROMonsterBase* Monster, AActor* Killer)
{
	if (!Monster)
	{
		return;
	}

	// Remove from active list
	ActiveMonsters.Remove(Monster);

	// Find which spawn definition this monster belongs to
	const int32 DefIndex = FindSpawnDefIndex(Monster);
	if (DefIndex >= 0)
	{
		// Decrement alive count
		int32& Count = AliveCountPerDef.FindOrAdd(DefIndex);
		Count = FMath::Max(0, Count - 1);

		// Queue respawn with delay + random variance, enforcing minimum 5-second delay
		const FROMonsterSpawnInfo& Def = SpawnDefinitions[DefIndex];
		const float Variance = FMath::FRandRange(-Def.RespawnVariance, Def.RespawnVariance);
		constexpr float MinRespawnDelay = 5.0f;
		const float EffectiveDelay = FMath::Max(MinRespawnDelay, Def.RespawnDelay + Variance);
		const float RespawnTime = GetWorld()->GetTimeSeconds() + EffectiveDelay;

		RespawnQueue.Add(FRORespawnEntry(DefIndex, RespawnTime));

		// Boss/MVP death announcement
		if (Monster->bIsMVP || Monster->bIsBoss)
		{
			const FString KillerName = Killer ? Killer->GetName() : TEXT("Unknown");
			UE_LOG(LogTemp, Warning, TEXT("BOSS KILLED: %s has been defeated by %s!"),
				*Monster->MonsterName.ToString(), *KillerName);

			// Broadcast boss kill to all connected players
			if (UGameInstance* GI = GetGameInstance())
			{
				UROChatSubsystem* Chat = GI->GetSubsystem<UROChatSubsystem>();
				if (Chat)
				{
					const FString Announcement = FString::Printf(
						TEXT("[MVP] %s has been defeated by %s!"),
						*Monster->MonsterName.ToString(), *KillerName);
					Chat->SendMessage(0, EChatChannel::System, Announcement);
				}
			}
		}

		UE_LOG(LogTemp, Log, TEXT("SpawnManager: Monster %s died. Respawn in %.1f sec."),
			*Monster->MonsterName.ToString(), EffectiveDelay);
	}
}

void AROMonsterSpawnManager::ProcessRespawnQueue(float CurrentTime)
{
	for (int32 i = RespawnQueue.Num() - 1; i >= 0; --i)
	{
		const FRORespawnEntry& Entry = RespawnQueue[i];
		if (CurrentTime >= Entry.RespawnTime)
		{
			const int32 DefIndex = Entry.SpawnDefIndex;
			if (SpawnDefinitions.IsValidIndex(DefIndex))
			{
				const FROMonsterSpawnInfo& Def = SpawnDefinitions[DefIndex];
				if (!IsSpawnDefinitionActiveAtCurrentTime(Def))
				{
					// Keep this entry in queue until the definition becomes active.
					continue;
				}

				const int32 AliveCount = AliveCountPerDef.FindRef(DefIndex);

				// Count how many respawns for this definition are already queued
				int32 PendingCount = 0;
				for (int32 j = 0; j < RespawnQueue.Num(); ++j)
				{
					if (j != i && RespawnQueue[j].SpawnDefIndex == DefIndex)
					{
						PendingCount++;
					}
				}

				// Only spawn if alive + pending (excluding self) is below count cap
				if (AliveCount + PendingCount < Def.Count)
				{
					AROMonsterBase* Monster = SpawnMonster(Def);
					if (Monster)
					{
						Monster->SpawnDefIndex = DefIndex;
						int32& Count = AliveCountPerDef.FindOrAdd(DefIndex);
						Count++;
					}
					else
					{
						// Spawn failed - keep entry in queue for retry next tick
						continue;
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("SpawnManager: Dropping respawn for def %d (alive=%d, pending=%d, target=%d)."),
						DefIndex, AliveCount, PendingCount, Def.Count);
				}
			}

			RespawnQueue.RemoveAt(i);
		}
	}
}

FVector AROMonsterSpawnManager::GetRandomSpawnLocation(const FROMonsterSpawnInfo& Info) const
{
	if (Info.SpawnRadius <= 0.0f)
	{
		return Info.SpawnCenter;
	}

	// Try to find a navigable point within the spawn radius
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FNavLocation NavResult;
		if (NavSys->GetRandomReachablePointInRadius(Info.SpawnCenter, Info.SpawnRadius, NavResult))
		{
			return NavResult.Location;
		}
	}

	// Fallback: random point in circle (no nav check)
	const float RandomAngle = FMath::FRandRange(0.0f, 2.0f * PI);
	const float RandomDist = FMath::FRandRange(0.0f, Info.SpawnRadius);
	return Info.SpawnCenter + FVector(
		FMath::Cos(RandomAngle) * RandomDist,
		FMath::Sin(RandomAngle) * RandomDist,
		0.0f
	);
}

int32 AROMonsterSpawnManager::FindSpawnDefIndex(const AROMonsterBase* Monster) const
{
	if (!Monster)
	{
		return -1;
	}

	// Use the stored definition index to correctly identify which spawn definition
	// this monster belongs to, even when multiple definitions share the same MonsterID
	if (SpawnDefinitions.IsValidIndex(Monster->SpawnDefIndex))
	{
		return Monster->SpawnDefIndex;
	}

	return -1;
}

bool AROMonsterSpawnManager::IsCurrentlyNightTime() const
{
	const AROGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState<AROGameStateBase>() : nullptr;
	if (!GameState)
	{
		return false;
	}

	const float TimeOfDaySeconds = FMath::Fmod(GameState->GetServerTime(), 86400.0f);
	constexpr float DayStartSeconds = 6.0f * 3600.0f;
	constexpr float NightStartSeconds = 18.0f * 3600.0f;

	return TimeOfDaySeconds < DayStartSeconds || TimeOfDaySeconds >= NightStartSeconds;
}

bool AROMonsterSpawnManager::IsSpawnDefinitionActiveAtCurrentTime(const FROMonsterSpawnInfo& Info) const
{
	if (Info.bSpawnDuringDay && Info.bSpawnDuringNight)
	{
		return true;
	}

	const bool bNightTime = IsCurrentlyNightTime();
	return bNightTime ? Info.bSpawnDuringNight : Info.bSpawnDuringDay;
}
