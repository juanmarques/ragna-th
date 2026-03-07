// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWidget_SkillWindow.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/Image.h"

UROWidget_SkillWindow::UROWidget_SkillWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UROWidget_SkillWindow::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Tab1stClass) Btn_Tab1stClass->OnClicked.AddDynamic(this, &UROWidget_SkillWindow::OnTab1stClassClicked);
	if (Btn_Tab2ndClass) Btn_Tab2ndClass->OnClicked.AddDynamic(this, &UROWidget_SkillWindow::OnTab2ndClassClicked);

	RefreshDisplay();
}

void UROWidget_SkillWindow::SetSkillTree(const TArray<FROSkillTreeEntry>& Skills)
{
	SkillEntries = Skills;
	RecalculatePrerequisites();
	RefreshDisplay();
}

void UROWidget_SkillWindow::UpdateSkillLevel(int32 SkillID, int32 NewLevel)
{
	for (FROSkillTreeEntry& Entry : SkillEntries)
	{
		if (Entry.SkillInfo.SkillID == SkillID)
		{
			Entry.SkillInfo.CurrentLevel = NewLevel;
			break;
		}
	}
	RecalculatePrerequisites();
	RefreshDisplay();
}

void UROWidget_SkillWindow::SetSkillPoints(int32 Points)
{
	AvailableSkillPoints = Points;

	if (Text_SkillPoints)
	{
		Text_SkillPoints->SetText(FText::FromString(FString::Printf(TEXT("Skill Points: %d"), Points)));
	}

	RecalculatePrerequisites();
	RefreshDisplay();
}

void UROWidget_SkillWindow::SwitchJobTierTab(EROJobTier Tier)
{
	CurrentTierTab = Tier;
	RefreshDisplay();
}

void UROWidget_SkillWindow::RequestLevelUpSkill(int32 SkillID)
{
	// Find the skill entry and validate
	for (const FROSkillTreeEntry& Entry : SkillEntries)
	{
		if (Entry.SkillInfo.SkillID == SkillID)
		{
			if (Entry.bCanLevelUp)
			{
				OnSkillLevelUp.Broadcast(SkillID);
			}
			break;
		}
	}
}

void UROWidget_SkillWindow::RecalculatePrerequisites()
{
	// Build a map of SkillID -> CurrentLevel for quick lookup
	TMap<int32, int32> SkillLevels;
	for (const FROSkillTreeEntry& Entry : SkillEntries)
	{
		SkillLevels.Add(Entry.SkillInfo.SkillID, Entry.SkillInfo.CurrentLevel);
	}

	for (FROSkillTreeEntry& Entry : SkillEntries)
	{
		// Check all prerequisites
		bool bAllMet = true;
		for (int32 i = 0; i < Entry.PrerequisiteSkillIDs.Num(); ++i)
		{
			const int32 ReqSkillID = Entry.PrerequisiteSkillIDs[i];
			const int32 ReqLevel = Entry.PrerequisiteLevels.IsValidIndex(i) ? Entry.PrerequisiteLevels[i] : 1;

			const int32* CurrentLv = SkillLevels.Find(ReqSkillID);
			if (!CurrentLv || *CurrentLv < ReqLevel)
			{
				bAllMet = false;
				break;
			}
		}

		Entry.bPrerequisitesMet = bAllMet;
		Entry.bCanLevelUp = bAllMet
			&& AvailableSkillPoints > 0
			&& Entry.SkillInfo.CurrentLevel < Entry.SkillInfo.MaxLevel;
	}
}

void UROWidget_SkillWindow::RefreshDisplay()
{
	// Visual refresh: Blueprint iterates SkillEntries and creates/updates
	// skill node widgets in the SkillTreeScrollBox, drawing prerequisite lines
	// and showing level up buttons where bCanLevelUp is true.

	if (Text_SkillPoints)
	{
		Text_SkillPoints->SetText(FText::FromString(FString::Printf(TEXT("Skill Points: %d"), AvailableSkillPoints)));
	}
}

void UROWidget_SkillWindow::OnTab1stClassClicked()
{
	SwitchJobTierTab(EROJobTier::First);
}

void UROWidget_SkillWindow::OnTab2ndClassClicked()
{
	SwitchJobTierTab(EROJobTier::Second);
}
