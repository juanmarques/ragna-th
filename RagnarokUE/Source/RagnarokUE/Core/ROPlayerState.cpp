// Copyright Ragna-TH Project. All Rights Reserved.

#include "Core/ROPlayerState.h"
#include "RagnarokUE.h"
#include "Net/UnrealNetwork.h"
#include "RagnarokUE/Character/ROLevelingComponent.h"
#include "RagnarokUE/Character/ROJobComponent.h"

AROPlayerState::AROPlayerState()
	: CharacterName(TEXT(""))
	, BaseLevel(1)
	, JobLevel(1)
	, JobClass(EROJobClass::Novice)
	, GuildID(0)
	, GuildName(TEXT(""))
	, PartyID(0)
{
	// PlayerState is always relevant to all clients so name plates,
	// guild tags, etc. work correctly.
	bAlwaysRelevant = true;
}

void AROPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AROPlayerState, CharacterName);
	DOREPLIFETIME(AROPlayerState, BaseLevel);
	DOREPLIFETIME(AROPlayerState, JobLevel);
	DOREPLIFETIME(AROPlayerState, JobClass);
	DOREPLIFETIME(AROPlayerState, GuildID);
	DOREPLIFETIME(AROPlayerState, GuildName);
	DOREPLIFETIME(AROPlayerState, PartyID);
}

void AROPlayerState::SyncFromCharacter(APawn* NewPawn)
{
	if (!NewPawn || !HasAuthority())
	{
		return;
	}

	if (UROLevelingComponent* LevelComp = NewPawn->FindComponentByClass<UROLevelingComponent>())
	{
		BaseLevel = LevelComp->BaseLevel;
		JobLevel = LevelComp->JobLevel;
		LevelComp->OnBaseLevelUp.RemoveDynamic(this, &AROPlayerState::OnCharacterBaseLevelUp);
		LevelComp->OnBaseLevelUp.AddDynamic(this, &AROPlayerState::OnCharacterBaseLevelUp);
		LevelComp->OnJobLevelUp.RemoveDynamic(this, &AROPlayerState::OnCharacterJobLevelUp);
		LevelComp->OnJobLevelUp.AddDynamic(this, &AROPlayerState::OnCharacterJobLevelUp);
	}

	if (UROJobComponent* JobComp = NewPawn->FindComponentByClass<UROJobComponent>())
	{
		JobClass = JobComp->CurrentJobClass;
		JobComp->OnJobChanged.RemoveDynamic(this, &AROPlayerState::OnCharacterJobChanged);
		JobComp->OnJobChanged.AddDynamic(this, &AROPlayerState::OnCharacterJobChanged);
	}
}

void AROPlayerState::OnCharacterBaseLevelUp(int32 NewBaseLevel)
{
	BaseLevel = NewBaseLevel;
}

void AROPlayerState::OnCharacterJobLevelUp(int32 NewJobLevel)
{
	JobLevel = NewJobLevel;
}

void AROPlayerState::OnCharacterJobChanged(EROJobClass OldJob, EROJobClass NewJob)
{
	JobClass = NewJob;
	JobLevel = 1; // Job level resets on job change
}
