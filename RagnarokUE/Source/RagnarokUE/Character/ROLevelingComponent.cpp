// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROLevelingComponent.h"
#include "ROStatsComponent.h"
#include "ROJobComponent.h"
#include "RagnarokUE/Data/ROExpTables.h"
#include "RagnarokUE/Data/ROConstants.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"

UROLevelingComponent::UROLevelingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	BaseLevel = 1;
	JobLevel = 1;
	CurrentBaseExp = 0;
	CurrentJobExp = 0;
}

void UROLevelingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UROLevelingComponent, BaseLevel);
	DOREPLIFETIME(UROLevelingComponent, JobLevel);
	DOREPLIFETIME(UROLevelingComponent, CurrentBaseExp);
	DOREPLIFETIME(UROLevelingComponent, CurrentJobExp);
}

void UROLevelingComponent::BeginPlay()
{
	Super::BeginPlay();
}

// ---- OnRep Callbacks ----

void UROLevelingComponent::OnRep_BaseLevel()
{
	// Client-side: recalculate derived stats since base level affects them
	if (AActor* Owner = GetOwner())
	{
		if (UROStatsComponent* StatsComp = Owner->FindComponentByClass<UROStatsComponent>())
		{
			StatsComp->RecalculateDerivedStats();
		}
	}
}

void UROLevelingComponent::OnRep_JobLevel()
{
	// Client-side notification that job level changed
}

// ---- Experience / Leveling ----

void UROLevelingComponent::AddBaseExp(int64 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	// Only process on server (or standalone)
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	const int32 MaxLevel = GetMaxBaseLevel();

	// Already at max level
	if (BaseLevel >= MaxLevel)
	{
		CurrentBaseExp = 0;
		return;
	}

	CurrentBaseExp += Amount;

	// Loop for multi-level ups
	int64 RequiredExp = GetRequiredBaseExp();
	while (RequiredExp > 0 && CurrentBaseExp >= RequiredExp && BaseLevel < MaxLevel)
	{
		CurrentBaseExp -= RequiredExp;
		BaseLevel++;
		ProcessBaseLevelUp();

		if (BaseLevel >= MaxLevel)
		{
			CurrentBaseExp = 0;
			break;
		}

		RequiredExp = GetRequiredBaseExp();
	}
}

void UROLevelingComponent::AddJobExp(int64 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	// Only process on server (or standalone)
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	const int32 MaxLevel = GetMaxJobLevel();

	// Already at max job level
	if (JobLevel >= MaxLevel)
	{
		CurrentJobExp = 0;
		return;
	}

	CurrentJobExp += Amount;

	// Loop for multi-level ups
	int64 RequiredExp = GetRequiredJobExp();
	while (RequiredExp > 0 && CurrentJobExp >= RequiredExp && JobLevel < MaxLevel)
	{
		CurrentJobExp -= RequiredExp;
		JobLevel++;
		ProcessJobLevelUp();

		if (JobLevel >= MaxLevel)
		{
			CurrentJobExp = 0;
			break;
		}

		RequiredExp = GetRequiredJobExp();
	}
}

int64 UROLevelingComponent::GetRequiredBaseExp() const
{
	if (BaseLevel >= GetMaxBaseLevel())
	{
		return 0;
	}
	return UROExpTables::GetBaseExpRequired(BaseLevel);
}

int64 UROLevelingComponent::GetRequiredJobExp() const
{
	if (JobLevel >= GetMaxJobLevel())
	{
		return 0;
	}

	// Get job class to determine which exp table to use
	EROJobClass JobClass = EROJobClass::Novice;
	if (AActor* Owner = GetOwner())
	{
		if (UROJobComponent* JobComp = Owner->FindComponentByClass<UROJobComponent>())
		{
			JobClass = JobComp->CurrentJobClass;
		}
	}

	return UROExpTables::GetJobExpRequired(JobLevel, UROJobComponent::GetJobTier(JobClass));
}

float UROLevelingComponent::GetBaseExpPercentage() const
{
	const int64 Required = GetRequiredBaseExp();
	if (Required <= 0)
	{
		return (BaseLevel >= GetMaxBaseLevel()) ? 1.0f : 0.0f;
	}
	return FMath::Clamp(static_cast<float>(CurrentBaseExp) / static_cast<float>(Required), 0.0f, 1.0f);
}

float UROLevelingComponent::GetJobExpPercentage() const
{
	const int64 Required = GetRequiredJobExp();
	if (Required <= 0)
	{
		return (JobLevel >= GetMaxJobLevel()) ? 1.0f : 0.0f;
	}
	return FMath::Clamp(static_cast<float>(CurrentJobExp) / static_cast<float>(Required), 0.0f, 1.0f);
}

void UROLevelingComponent::ResetJobLevel()
{
	JobLevel = 1;
	CurrentJobExp = 0;
}

int32 UROLevelingComponent::GetMaxBaseLevel() const
{
	// In classic RO, max base level is 99 for all classes
	// Transcendent classes also cap at 99 (in pre-renewal)
	return 99;
}

int32 UROLevelingComponent::GetMaxJobLevel() const
{
	if (AActor* Owner = GetOwner())
	{
		if (UROJobComponent* JobComp = Owner->FindComponentByClass<UROJobComponent>())
		{
			return JobComp->GetMaxJobLevel();
		}
	}
	return 10; // Default to novice max
}

// ---- Internal Processing ----

void UROLevelingComponent::ProcessBaseLevelUp()
{
	// Grant stat points for this level
	const int32 StatPoints = GetStatPointsForLevel(BaseLevel);

	if (AActor* Owner = GetOwner())
	{
		if (UROStatsComponent* StatsComp = Owner->FindComponentByClass<UROStatsComponent>())
		{
			StatsComp->AvailableStatPoints += StatPoints;

			// Recalculate derived stats (base level affects HIT, FLEE, DEF, etc.)
			StatsComp->RecalculateDerivedStats();
		}
	}

	// Broadcast level up event
	OnBaseLevelUp.Broadcast(BaseLevel);

	UE_LOG(LogTemp, Log, TEXT("ROLevelingComponent: Base Level Up! New Level: %d (+%d stat points)"),
		BaseLevel, StatPoints);
}

void UROLevelingComponent::ProcessJobLevelUp()
{
	// Grant 1 skill point per job level
	if (AActor* Owner = GetOwner())
	{
		if (UROJobComponent* JobComp = Owner->FindComponentByClass<UROJobComponent>())
		{
			JobComp->AddSkillPoints(1);
		}
	}

	// Broadcast level up event
	OnJobLevelUp.Broadcast(JobLevel);

	UE_LOG(LogTemp, Log, TEXT("ROLevelingComponent: Job Level Up! New Level: %d"), JobLevel);
}

int32 UROLevelingComponent::GetStatPointsForLevel(int32 Level)
{
	// In RO, stat points per level = floor((Level - 1) / 5) + 3
	// Level 1: 0 points (no points for being level 1), Level 2-5: 3 points, Level 6-10: 4 points, etc.
	if (Level <= 1)
	{
		return 0;
	}
	return FMath::FloorToInt(static_cast<float>(Level - 1) / 5.0f) + 3;
}
