// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROJobComponent.h"
#include "ROLevelingComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"

UROJobComponent::UROJobComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	CurrentJobClass = EROJobClass::Novice;
	AvailableSkillPoints = 0;
}

void UROJobComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UROJobComponent, CurrentJobClass);
	DOREPLIFETIME(UROJobComponent, AvailableSkillPoints);
}

// ---- OnRep Callbacks ----

void UROJobComponent::OnRep_CurrentJobClass()
{
	// Broadcast job change event on clients
	// Note: OldJob is not available in OnRep, so we pass the current as both
	// The actual two-param broadcast happens in ExecuteJobChange on the server
}

// ---- Static Job Tree ----

TArray<EROJobClass> UROJobComponent::GetAvailableJobChanges(EROJobClass Current)
{
	TArray<EROJobClass> Result;

	switch (Current)
	{
	// Novice -> 1st Class
	case EROJobClass::Novice:
		Result = { EROJobClass::Swordsman, EROJobClass::Magician, EROJobClass::Archer,
		           EROJobClass::Thief, EROJobClass::Merchant, EROJobClass::Acolyte };
		break;

	// 1st Class -> 2nd Class (2-1 and 2-2)
	case EROJobClass::Swordsman:
		Result = { EROJobClass::Knight, EROJobClass::Crusader };
		break;
	case EROJobClass::Magician:
		Result = { EROJobClass::Wizard, EROJobClass::Sage };
		break;
	case EROJobClass::Archer:
		Result = { EROJobClass::Hunter, EROJobClass::Bard, EROJobClass::Dancer };
		break;
	case EROJobClass::Thief:
		Result = { EROJobClass::Assassin, EROJobClass::Rogue };
		break;
	case EROJobClass::Merchant:
		Result = { EROJobClass::Blacksmith, EROJobClass::Alchemist };
		break;
	case EROJobClass::Acolyte:
		Result = { EROJobClass::Priest, EROJobClass::Monk };
		break;

	// 2nd Class -> Transcendent (rebirth path: must go through High Novice)
	case EROJobClass::Knight:
	case EROJobClass::Crusader:
	case EROJobClass::Wizard:
	case EROJobClass::Sage:
	case EROJobClass::Hunter:
	case EROJobClass::Bard:
	case EROJobClass::Dancer:
	case EROJobClass::Assassin:
	case EROJobClass::Rogue:
	case EROJobClass::Blacksmith:
	case EROJobClass::Alchemist:
	case EROJobClass::Priest:
	case EROJobClass::Monk:
		Result = { EROJobClass::HighNovice };
		break;

	// High Novice -> High 1st Classes
	case EROJobClass::HighNovice:
		Result = { EROJobClass::HighSwordsman, EROJobClass::HighMagician, EROJobClass::HighArcher,
		           EROJobClass::HighThief, EROJobClass::HighMerchant, EROJobClass::HighAcolyte };
		break;

	// High 1st -> Transcendent 2nd Classes
	case EROJobClass::HighSwordsman:
		Result = { EROJobClass::LordKnight, EROJobClass::Paladin };
		break;
	case EROJobClass::HighMagician:
		Result = { EROJobClass::HighWizard, EROJobClass::Professor };
		break;
	case EROJobClass::HighArcher:
		Result = { EROJobClass::Sniper, EROJobClass::Minstrel, EROJobClass::Gypsy };
		break;
	case EROJobClass::HighThief:
		Result = { EROJobClass::AssassinCross, EROJobClass::Stalker };
		break;
	case EROJobClass::HighMerchant:
		Result = { EROJobClass::Whitesmith, EROJobClass::Creator };
		break;
	case EROJobClass::HighAcolyte:
		Result = { EROJobClass::HighPriest, EROJobClass::Champion };
		break;

	default:
		// Terminal classes (Transcendent 2nd) have no further changes
		break;
	}

	return Result;
}

EROJobTier UROJobComponent::GetJobTier(EROJobClass Job)
{
	switch (Job)
	{
	case EROJobClass::Novice:
		return EROJobTier::Novice_Tier;

	case EROJobClass::Swordsman:
	case EROJobClass::Magician:
	case EROJobClass::Archer:
	case EROJobClass::Thief:
	case EROJobClass::Merchant:
	case EROJobClass::Acolyte:
		return EROJobTier::First;

	case EROJobClass::Knight:
	case EROJobClass::Crusader:
	case EROJobClass::Wizard:
	case EROJobClass::Sage:
	case EROJobClass::Hunter:
	case EROJobClass::Bard:
	case EROJobClass::Dancer:
	case EROJobClass::Assassin:
	case EROJobClass::Rogue:
	case EROJobClass::Blacksmith:
	case EROJobClass::Alchemist:
	case EROJobClass::Priest:
	case EROJobClass::Monk:
		return EROJobTier::Second;

	case EROJobClass::HighNovice:
		return EROJobTier::Novice_Tier;

	case EROJobClass::HighSwordsman:
	case EROJobClass::HighMagician:
	case EROJobClass::HighArcher:
	case EROJobClass::HighThief:
	case EROJobClass::HighMerchant:
	case EROJobClass::HighAcolyte:
		return EROJobTier::First;

	case EROJobClass::LordKnight:
	case EROJobClass::Paladin:
	case EROJobClass::HighWizard:
	case EROJobClass::Professor:
	case EROJobClass::Sniper:
	case EROJobClass::Minstrel:
	case EROJobClass::Gypsy:
	case EROJobClass::AssassinCross:
	case EROJobClass::Stalker:
	case EROJobClass::Whitesmith:
	case EROJobClass::Creator:
	case EROJobClass::HighPriest:
	case EROJobClass::Champion:
		return EROJobTier::TranscendentSecond;

	default:
		return EROJobTier::Novice_Tier;
	}
}

int32 UROJobComponent::GetMaxJobLevel() const
{
	return GetMaxJobLevelForTier(GetJobTier(CurrentJobClass));
}

int32 UROJobComponent::GetMaxJobLevelForTier(EROJobTier Tier)
{
	switch (Tier)
	{
	case EROJobTier::Novice_Tier:
		return 10;
	case EROJobTier::First:
		return 50;
	case EROJobTier::Second:
		return 50;
	case EROJobTier::Transcendent:
		// High 1st classes also cap at 50 (they level through quickly)
		return 50;
	case EROJobTier::TranscendentSecond:
		return 70; // Trans 2nd classes get 70 job levels
	default:
		return 50;
	}
}

EROJobClass UROJobComponent::GetBaseJobForClass(EROJobClass Job)
{
	switch (Job)
	{
	// Swordsman line
	case EROJobClass::Swordsman:
	case EROJobClass::Knight:
	case EROJobClass::Crusader:
	case EROJobClass::HighSwordsman:
	case EROJobClass::LordKnight:
	case EROJobClass::Paladin:
		return EROJobClass::Swordsman;

	// Magician line
	case EROJobClass::Magician:
	case EROJobClass::Wizard:
	case EROJobClass::Sage:
	case EROJobClass::HighMagician:
	case EROJobClass::HighWizard:
	case EROJobClass::Professor:
		return EROJobClass::Magician;

	// Archer line
	case EROJobClass::Archer:
	case EROJobClass::Hunter:
	case EROJobClass::Bard:
	case EROJobClass::Dancer:
	case EROJobClass::HighArcher:
	case EROJobClass::Sniper:
	case EROJobClass::Minstrel:
	case EROJobClass::Gypsy:
		return EROJobClass::Archer;

	// Thief line
	case EROJobClass::Thief:
	case EROJobClass::Assassin:
	case EROJobClass::Rogue:
	case EROJobClass::HighThief:
	case EROJobClass::AssassinCross:
	case EROJobClass::Stalker:
		return EROJobClass::Thief;

	// Merchant line
	case EROJobClass::Merchant:
	case EROJobClass::Blacksmith:
	case EROJobClass::Alchemist:
	case EROJobClass::HighMerchant:
	case EROJobClass::Whitesmith:
	case EROJobClass::Creator:
		return EROJobClass::Merchant;

	// Acolyte line
	case EROJobClass::Acolyte:
	case EROJobClass::Priest:
	case EROJobClass::Monk:
	case EROJobClass::HighAcolyte:
	case EROJobClass::HighPriest:
	case EROJobClass::Champion:
		return EROJobClass::Acolyte;

	default:
		return EROJobClass::Novice;
	}
}

void UROJobComponent::AddSkillPoints(int32 Amount)
{
	AvailableSkillPoints += Amount;
}

int32 UROJobComponent::GetRequiredJobLevelForChange(EROJobTier TargetTier)
{
	switch (TargetTier)
	{
	case EROJobTier::Novice_Tier:
		return 50; // 2nd class needs max job level for rebirth to High Novice
	case EROJobTier::First:
		return 10; // Novice needs job level 10
	case EROJobTier::Second:
		return 40; // 1st class needs job level 40 minimum (50 for all skills)
	case EROJobTier::Transcendent:
		return 50; // 2nd class needs max job level for rebirth
	case EROJobTier::TranscendentSecond:
		return 40; // High 1st class needs job level 40 minimum
	default:
		return 1;
	}
}

// ---- Server RPCs ----

bool UROJobComponent::ServerRequestJobChange_Validate(EROJobClass NewJob)
{
	// Can't change TO novice — everything else is validated by CanChangeToJob
	return NewJob != EROJobClass::Novice;
}

void UROJobComponent::ServerRequestJobChange_Implementation(EROJobClass NewJob)
{
	if (!CanChangeToJob(NewJob))
	{
		UE_LOG(LogTemp, Warning, TEXT("ROJobComponent: Job change to %d denied - prerequisites not met."),
			static_cast<uint8>(NewJob));
		return;
	}

	ExecuteJobChange(NewJob);
}

bool UROJobComponent::CanChangeToJob(EROJobClass TargetJob) const
{
	// Check if TargetJob is in the list of available changes from current job
	TArray<EROJobClass> AvailableChanges = GetAvailableJobChanges(CurrentJobClass);
	if (!AvailableChanges.Contains(TargetJob))
	{
		return false;
	}

	// Check job level requirement
	EROJobTier TargetTier = GetJobTier(TargetJob);
	int32 RequiredJobLevel = GetRequiredJobLevelForChange(TargetTier);

	// Get current job level from the leveling component
	if (AActor* Owner = GetOwner())
	{
		if (UROLevelingComponent* LevelComp = Owner->FindComponentByClass<UROLevelingComponent>())
		{
			if (LevelComp->JobLevel < RequiredJobLevel)
			{
				return false;
			}

			// Rebirth to HighNovice requires base level 99
			if (TargetJob == EROJobClass::HighNovice && LevelComp->BaseLevel < 99)
			{
				return false;
			}
		}
	}

	return true;
}

void UROJobComponent::ExecuteJobChange(EROJobClass NewJob)
{
	EROJobClass OldJob = CurrentJobClass;
	CurrentJobClass = NewJob;

	// Reset job level via the leveling component
	if (AActor* Owner = GetOwner())
	{
		if (UROLevelingComponent* LevelComp = Owner->FindComponentByClass<UROLevelingComponent>())
		{
			LevelComp->ResetJobLevel();
		}
	}

	// Reset skill points for new job
	AvailableSkillPoints = 0;

	// Grant initial skill point for job level 1
	AddSkillPoints(1);

	// Broadcast the change
	OnJobChanged.Broadcast(OldJob, NewJob);

	UE_LOG(LogTemp, Log, TEXT("ROJobComponent: Job changed from %d to %d"),
		static_cast<uint8>(OldJob), static_cast<uint8>(NewJob));
}
