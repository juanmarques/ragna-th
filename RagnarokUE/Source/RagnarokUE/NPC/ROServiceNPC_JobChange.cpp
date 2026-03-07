// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROServiceNPC_JobChange.h"
#include "RagnarokUE/Character/ROCharacterBase.h"
#include "RagnarokUE/Character/ROJobComponent.h"
#include "RagnarokUE/Character/ROLevelingComponent.h"

AROServiceNPC_JobChange::AROServiceNPC_JobChange()
{
	bIsService = true;
	bIsQuestGiver = true;
	TargetJobClass = EROJobClass::Swordsman;
	RequiredCurrentJob = EROJobClass::Novice;
	MinJobLevel = 40;
	OptimalJobLevel = 50;
	InitialSkillPoints = 1;
	CurrentInteractor = nullptr;
	DisplayName = FText::FromString(TEXT("Job Change NPC"));
}

void AROServiceNPC_JobChange::OnInteract_Implementation(AROCharacterBase* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	Super::OnInteract_Implementation(Interactor);

	CurrentInteractor = Interactor;

	// Check if the player can change jobs and log feedback
	if (CanPlayerChangeJob(Interactor))
	{
		UE_LOG(LogTemp, Log, TEXT("JobChange NPC %s: Player meets requirements for %s."),
			*NPCName.ToString(),
			*UEnum::GetValueAsString(TargetJobClass));
	}
	else
	{
		FText FailureReason = GetRequirementFailureReason(Interactor);
		UE_LOG(LogTemp, Log, TEXT("JobChange NPC %s: Player does not meet requirements. Reason: %s"),
			*NPCName.ToString(),
			*FailureReason.ToString());
	}
}

void AROServiceNPC_JobChange::ServerAttemptJobChange_Implementation()
{
	if (!CurrentInteractor)
	{
		UE_LOG(LogTemp, Warning, TEXT("JobChange NPC: No player currently interacting."));
		return;
	}

	if (!CanPlayerChangeJob(CurrentInteractor))
	{
		EROJobClass OldJob = EROJobClass::Novice;
		if (UROJobComponent* JobComp = CurrentInteractor->FindComponentByClass<UROJobComponent>())
		{
			OldJob = JobComp->CurrentJobClass;
		}

		OnJobChangeResult.Broadcast(false, OldJob, TargetJobClass);
		UE_LOG(LogTemp, Warning, TEXT("JobChange NPC: Player does not meet job change requirements."));
		return;
	}

	ExecuteJobChange(CurrentInteractor);
}

bool AROServiceNPC_JobChange::ServerAttemptJobChange_Validate()
{
	return true;
}

bool AROServiceNPC_JobChange::CanPlayerChangeJob(AROCharacterBase* Player) const
{
	if (!Player)
	{
		return false;
	}

	// Check current job class
	UROJobComponent* JobComp = Player->FindComponentByClass<UROJobComponent>();
	if (!JobComp)
	{
		return false;
	}

	if (JobComp->CurrentJobClass != RequiredCurrentJob)
	{
		return false;
	}

	// Check job level
	UROLevelingComponent* LevelComp = Player->FindComponentByClass<UROLevelingComponent>();
	if (!LevelComp)
	{
		return false;
	}

	if (LevelComp->JobLevel < MinJobLevel)
	{
		return false;
	}

	// Verify the job component also agrees this transition is valid
	if (!JobComp->CanChangeToJob(TargetJobClass))
	{
		return false;
	}

	return true;
}

FText AROServiceNPC_JobChange::GetRequirementFailureReason(AROCharacterBase* Player) const
{
	if (!Player)
	{
		return FText::FromString(TEXT("Invalid player."));
	}

	UROJobComponent* JobComp = Player->FindComponentByClass<UROJobComponent>();
	if (!JobComp)
	{
		return FText::FromString(TEXT("No job component found."));
	}

	if (JobComp->CurrentJobClass != RequiredCurrentJob)
	{
		return FText::Format(
			NSLOCTEXT("ROJobChange", "WrongClass", "You must be a {0} to become a {1}."),
			FText::FromString(UEnum::GetValueAsString(RequiredCurrentJob)),
			FText::FromString(UEnum::GetValueAsString(TargetJobClass)));
	}

	UROLevelingComponent* LevelComp = Player->FindComponentByClass<UROLevelingComponent>();
	if (!LevelComp)
	{
		return FText::FromString(TEXT("No leveling component found."));
	}

	if (LevelComp->JobLevel < MinJobLevel)
	{
		return FText::Format(
			NSLOCTEXT("ROJobChange", "LowJobLevel", "You need Job Level {0} to change jobs. Current: {1}."),
			FText::AsNumber(MinJobLevel),
			FText::AsNumber(LevelComp->JobLevel));
	}

	if (!JobComp->CanChangeToJob(TargetJobClass))
	{
		return FText::Format(
			NSLOCTEXT("ROJobChange", "InvalidTransition", "Cannot change from {0} to {1}."),
			FText::FromString(UEnum::GetValueAsString(JobComp->CurrentJobClass)),
			FText::FromString(UEnum::GetValueAsString(TargetJobClass)));
	}

	return FText::GetEmpty();
}

void AROServiceNPC_JobChange::ExecuteJobChange(AROCharacterBase* Player)
{
	if (!Player)
	{
		return;
	}

	UROJobComponent* JobComp = Player->FindComponentByClass<UROJobComponent>();
	UROLevelingComponent* LevelComp = Player->FindComponentByClass<UROLevelingComponent>();

	if (!JobComp || !LevelComp)
	{
		return;
	}

	const EROJobClass OldJob = JobComp->CurrentJobClass;
	const bool bIsOptimal = (LevelComp->JobLevel >= OptimalJobLevel);

	// Execute job change through the job component (handles replication, validation, etc.)
	JobComp->ServerRequestJobChange(TargetJobClass);

	// Reset job level to 1 (handled by JobComponent/LevelingComponent internally)
	LevelComp->ResetJobLevel();

	// Grant initial skill points
	int32 SkillPointsToGrant = InitialSkillPoints;
	if (bIsOptimal)
	{
		// Bonus skill point for reaching optimal job level before changing
		SkillPointsToGrant += 1;
	}
	JobComp->AddSkillPoints(SkillPointsToGrant);

	OnJobChangeResult.Broadcast(true, OldJob, TargetJobClass);

	UE_LOG(LogTemp, Log, TEXT("JobChange NPC: Player changed from %s to %s. Skill points granted: %d. Optimal: %s"),
		*UEnum::GetValueAsString(OldJob),
		*UEnum::GetValueAsString(TargetJobClass),
		SkillPointsToGrant,
		bIsOptimal ? TEXT("Yes") : TEXT("No"));
}
