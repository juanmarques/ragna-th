// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROSkillTreeComponent.h"
#include "ROAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

UROSkillTreeComponent::UROSkillTreeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	CurrentJobClass = EROJobClass::Novice;
	SkillPoints = 0;
	TotalSkillPointsSpent = 0;
}

void UROSkillTreeComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeSkillDefinitions();

	// Rebuild runtime map from replicated array (for clients receiving initial state)
	for (const FROLearnedSkillEntry& Entry : LearnedSkillEntries)
	{
		LearnedSkills.Add(Entry.SkillID, Entry.Level);
	}
}

void UROSkillTreeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UROSkillTreeComponent, LearnedSkillEntries);
	DOREPLIFETIME(UROSkillTreeComponent, SkillPoints);
	DOREPLIFETIME(UROSkillTreeComponent, CurrentJobClass);
}

bool UROSkillTreeComponent::ServerLearnSkill_Validate(int32 SkillID)
{
	// Basic sanity check: skill ID must be positive
	return SkillID > 0;
}

void UROSkillTreeComponent::ServerLearnSkill_Implementation(int32 SkillID)
{
	// Validate skill exists
	const FROSkillDefinition* SkillDef = FindSkillDefinition(SkillID);
	if (!SkillDef)
	{
		UE_LOG(LogTemp, Warning, TEXT("ROSkillTree: Skill ID %d not found in definitions"), SkillID);
		return;
	}

	// Check job class requirement
	if (SkillDef->RequiredJob != CurrentJobClass)
	{
		// Also allow if the current job is a higher tier of the required job
		// For simplicity, we just check direct match here
		// A full implementation would check job advancement paths
		bool bJobValid = false;

		// Check if current job inherits from required job
		switch (CurrentJobClass)
		{
		case EROJobClass::Knight:
		case EROJobClass::LordKnight:
		case EROJobClass::Crusader:
		case EROJobClass::Paladin:
			bJobValid = (SkillDef->RequiredJob == EROJobClass::Swordsman || SkillDef->RequiredJob == EROJobClass::HighSwordsman);
			break;
		case EROJobClass::Wizard:
		case EROJobClass::HighWizard:
		case EROJobClass::Sage:
		case EROJobClass::Professor:
			bJobValid = (SkillDef->RequiredJob == EROJobClass::Magician || SkillDef->RequiredJob == EROJobClass::HighMagician);
			break;
		case EROJobClass::Priest:
		case EROJobClass::HighPriest:
		case EROJobClass::Monk:
		case EROJobClass::Champion:
			bJobValid = (SkillDef->RequiredJob == EROJobClass::Acolyte || SkillDef->RequiredJob == EROJobClass::HighAcolyte);
			break;
		case EROJobClass::Hunter:
		case EROJobClass::Sniper:
		case EROJobClass::Bard:
		case EROJobClass::Minstrel:
		case EROJobClass::Dancer:
		case EROJobClass::Gypsy:
			bJobValid = (SkillDef->RequiredJob == EROJobClass::Archer || SkillDef->RequiredJob == EROJobClass::HighArcher);
			break;
		case EROJobClass::Assassin:
		case EROJobClass::AssassinCross:
		case EROJobClass::Rogue:
		case EROJobClass::Stalker:
			bJobValid = (SkillDef->RequiredJob == EROJobClass::Thief || SkillDef->RequiredJob == EROJobClass::HighThief);
			break;
		case EROJobClass::Blacksmith:
		case EROJobClass::Whitesmith:
		case EROJobClass::Alchemist:
		case EROJobClass::Creator:
			bJobValid = (SkillDef->RequiredJob == EROJobClass::Merchant || SkillDef->RequiredJob == EROJobClass::HighMerchant);
			break;
		default:
			break;
		}

		if (!bJobValid)
		{
			UE_LOG(LogTemp, Warning, TEXT("ROSkillTree: Job class mismatch for skill %d"), SkillID);
			return;
		}
	}

	// Get current level
	int32 CurrentLevel = GetSkillLevel(SkillID);
	int32 DesiredLevel = CurrentLevel + 1;

	// Check max level
	if (DesiredLevel > SkillDef->MaxLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("ROSkillTree: Skill %d already at max level"), SkillID);
		return;
	}

	// Check skill points
	if (SkillPoints <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ROSkillTree: No skill points available"));
		return;
	}

	// Check prerequisites
	if (!CheckPrerequisites(SkillID, DesiredLevel))
	{
		UE_LOG(LogTemp, Warning, TEXT("ROSkillTree: Prerequisites not met for skill %d level %d"), SkillID, DesiredLevel);
		return;
	}

	// All checks passed - learn/level up the skill
	LearnedSkills.FindOrAdd(SkillID) = DesiredLevel;

	// Sync to replicated array
	bool bFound = false;
	for (FROLearnedSkillEntry& Entry : LearnedSkillEntries)
	{
		if (Entry.SkillID == SkillID)
		{
			Entry.Level = DesiredLevel;
			bFound = true;
			break;
		}
	}
	if (!bFound)
	{
		LearnedSkillEntries.Add({SkillID, DesiredLevel});
	}
	SkillPoints--;
	TotalSkillPointsSpent++;

	OnSkillLearned.Broadcast(SkillID, DesiredLevel);

	UE_LOG(LogTemp, Log, TEXT("ROSkillTree: Learned skill %d at level %d"), SkillID, DesiredLevel);
}

bool UROSkillTreeComponent::CheckPrerequisites(int32 SkillID, int32 DesiredLevel) const
{
	const FROSkillDefinition* SkillDef = FindSkillDefinition(SkillID);
	if (!SkillDef)
	{
		return false;
	}

	for (const FROSkillPrerequisite& Prereq : SkillDef->Prerequisites)
	{
		int32 PrereqLevel = GetSkillLevel(Prereq.RequiredSkillID);
		if (PrereqLevel < Prereq.RequiredLevel)
		{
			return false;
		}
	}

	return true;
}

TArray<FROSkillDefinition> UROSkillTreeComponent::GetAvailableSkillsForJob(EROJobClass Job) const
{
	TArray<FROSkillDefinition> Result;

	for (const FROSkillDefinition& Def : SkillDefinitions)
	{
		if (Def.RequiredJob == Job)
		{
			Result.Add(Def);
		}
	}

	return Result;
}

int32 UROSkillTreeComponent::GetSkillLevel(int32 SkillID) const
{
	const int32* Level = LearnedSkills.Find(SkillID);
	return Level ? *Level : 0;
}

TMap<int32, int32> UROSkillTreeComponent::GetAllLearnedSkills() const
{
	return LearnedSkills;
}

void UROSkillTreeComponent::AddSkillPoints(int32 Points)
{
	if (Points > 0)
	{
		SkillPoints += Points;
	}
}

void UROSkillTreeComponent::ResetSkills()
{
	// Refund all spent skill points
	SkillPoints += TotalSkillPointsSpent;
	TotalSkillPointsSpent = 0;
	LearnedSkills.Empty();
	LearnedSkillEntries.Empty();
}

const FROSkillDefinition* UROSkillTreeComponent::FindSkillDefinition(int32 SkillID) const
{
	for (const FROSkillDefinition& Def : SkillDefinitions)
	{
		if (Def.SkillID == SkillID)
		{
			return &Def;
		}
	}
	return nullptr;
}

void UROSkillTreeComponent::InitializeSkillDefinitions()
{
	// --- Swordsman Skills ---
	{
		FROSkillDefinition Bash;
		Bash.SkillID = 5;
		Bash.SkillName = FName("SM_BASH");
		Bash.DisplayName = FText::FromString("Bash");
		Bash.MaxLevel = 10;
		Bash.RequiredJob = EROJobClass::Swordsman;
		Bash.Description = FText::FromString("A powerful single-target physical attack.");
		SkillDefinitions.Add(Bash);

		FROSkillDefinition MagnumBreak;
		MagnumBreak.SkillID = 7;
		MagnumBreak.SkillName = FName("SM_MAGNUM");
		MagnumBreak.DisplayName = FText::FromString("Magnum Break");
		MagnumBreak.MaxLevel = 10;
		MagnumBreak.RequiredJob = EROJobClass::Swordsman;
		// Requires Bash Lv5
		FROSkillPrerequisite MBPrereq;
		MBPrereq.RequiredSkillID = 5;
		MBPrereq.RequiredLevel = 5;
		MagnumBreak.Prerequisites.Add(MBPrereq);
		MagnumBreak.Description = FText::FromString("AoE fire attack that pushes enemies back.");
		SkillDefinitions.Add(MagnumBreak);
	}

	// --- Magician Skills ---
	{
		FROSkillDefinition FireBolt;
		FireBolt.SkillID = 19;
		FireBolt.SkillName = FName("MG_FIREBOLT");
		FireBolt.DisplayName = FText::FromString("Fire Bolt");
		FireBolt.MaxLevel = 10;
		FireBolt.RequiredJob = EROJobClass::Magician;
		FireBolt.Description = FText::FromString("Launches fire bolts at a target. Number of bolts = skill level.");
		SkillDefinitions.Add(FireBolt);

		FROSkillDefinition ColdBolt;
		ColdBolt.SkillID = 20;
		ColdBolt.SkillName = FName("MG_COLDBOLT");
		ColdBolt.DisplayName = FText::FromString("Cold Bolt");
		ColdBolt.MaxLevel = 10;
		ColdBolt.RequiredJob = EROJobClass::Magician;
		ColdBolt.Description = FText::FromString("Launches ice bolts at a target. Number of bolts = skill level.");
		SkillDefinitions.Add(ColdBolt);

		FROSkillDefinition LightningBolt;
		LightningBolt.SkillID = 21;
		LightningBolt.SkillName = FName("MG_LIGHTNINGBOLT");
		LightningBolt.DisplayName = FText::FromString("Lightning Bolt");
		LightningBolt.MaxLevel = 10;
		LightningBolt.RequiredJob = EROJobClass::Magician;
		LightningBolt.Description = FText::FromString("Launches lightning bolts at a target. Number of bolts = skill level.");
		SkillDefinitions.Add(LightningBolt);
	}

	// --- Acolyte Skills ---
	{
		FROSkillDefinition Heal;
		Heal.SkillID = 28;
		Heal.SkillName = FName("AL_HEAL");
		Heal.DisplayName = FText::FromString("Heal");
		Heal.MaxLevel = 10;
		Heal.RequiredJob = EROJobClass::Acolyte;
		Heal.Description = FText::FromString("Restores HP of target. Damages undead monsters.");
		SkillDefinitions.Add(Heal);
	}

	// --- Archer Skills ---
	{
		FROSkillDefinition DoubleStrafe;
		DoubleStrafe.SkillID = 46;
		DoubleStrafe.SkillName = FName("AC_DOUBLE");
		DoubleStrafe.DisplayName = FText::FromString("Double Strafe");
		DoubleStrafe.MaxLevel = 10;
		DoubleStrafe.RequiredJob = EROJobClass::Archer;
		DoubleStrafe.Description = FText::FromString("Two rapid arrow shots at a single target. Requires bow.");
		SkillDefinitions.Add(DoubleStrafe);
	}

	// --- Thief Skills ---
	{
		FROSkillDefinition Hiding;
		Hiding.SkillID = 51;
		Hiding.SkillName = FName("TF_HIDING");
		Hiding.DisplayName = FText::FromString("Hiding");
		Hiding.MaxLevel = 10;
		Hiding.RequiredJob = EROJobClass::Thief;
		Hiding.Description = FText::FromString("Hide from enemies. Drains SP over time. Detectable by certain monsters.");
		SkillDefinitions.Add(Hiding);
	}

	// --- Merchant Skills ---
	{
		FROSkillDefinition Discount;
		Discount.SkillID = 42;
		Discount.SkillName = FName("MC_DISCOUNT");
		Discount.DisplayName = FText::FromString("Discount");
		Discount.MaxLevel = 10;
		Discount.RequiredJob = EROJobClass::Merchant;
		Discount.Description = FText::FromString("Passive: Reduces NPC buy prices.");
		SkillDefinitions.Add(Discount);
	}
}
