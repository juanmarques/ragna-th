// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWidget_QuestLog.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/RichTextBlock.h"
#include "Components/WidgetSwitcher.h"

UROWidget_QuestLog::UROWidget_QuestLog(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UROWidget_QuestLog::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_TabActive) Btn_TabActive->OnClicked.AddDynamic(this, &UROWidget_QuestLog::OnTabActiveClicked);
	if (Btn_TabCompleted) Btn_TabCompleted->OnClicked.AddDynamic(this, &UROWidget_QuestLog::OnTabCompletedClicked);
	if (Btn_Abandon) Btn_Abandon->OnClicked.AddDynamic(this, &UROWidget_QuestLog::OnAbandonClicked);

	RefreshDisplay();
}

void UROWidget_QuestLog::SetQuests(const TArray<FROQuestEntry>& Quests)
{
	AllQuests = Quests;
	RefreshDisplay();
}

void UROWidget_QuestLog::UpdateQuestProgress(int32 QuestID, const TArray<FROQuestObjective>& Objectives)
{
	for (FROQuestEntry& Quest : AllQuests)
	{
		if (Quest.QuestID == QuestID)
		{
			Quest.Objectives = Objectives;

			// Check if all objectives are complete
			bool bAllComplete = true;
			for (const FROQuestObjective& Obj : Objectives)
			{
				if (!Obj.IsComplete())
				{
					bAllComplete = false;
					break;
				}
			}
			Quest.bIsComplete = bAllComplete;
			break;
		}
	}

	RefreshDisplay();
}

void UROWidget_QuestLog::MarkQuestComplete(int32 QuestID)
{
	for (FROQuestEntry& Quest : AllQuests)
	{
		if (Quest.QuestID == QuestID)
		{
			Quest.bIsComplete = true;
			break;
		}
	}
	RefreshDisplay();
}

void UROWidget_QuestLog::SwitchTab(EROQuestTab Tab)
{
	CurrentTab = Tab;
	SelectedQuestID = 0;
	RefreshDisplay();
}

void UROWidget_QuestLog::SelectQuest(int32 QuestID)
{
	SelectedQuestID = QuestID;

	// Find the quest and display details
	for (const FROQuestEntry& Quest : AllQuests)
	{
		if (Quest.QuestID == QuestID)
		{
			if (Text_QuestName)
			{
				Text_QuestName->SetText(FText::FromString(Quest.QuestName));
			}
			if (Text_QuestDescription)
			{
				Text_QuestDescription->SetText(FText::FromString(Quest.Description));
			}

			// Abandon button only visible for active quests
			if (Btn_Abandon)
			{
				Btn_Abandon->SetVisibility(Quest.bIsComplete ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
			}

			// Objectives are refreshed in Blueprint via ObjectivesScrollBox
			break;
		}
	}
}

void UROWidget_QuestLog::AbandonSelectedQuest()
{
	if (SelectedQuestID > 0)
	{
		OnAbandonQuest.Broadcast(SelectedQuestID);

		// Remove from local list
		AllQuests.RemoveAll([this](const FROQuestEntry& Quest)
		{
			return Quest.QuestID == SelectedQuestID;
		});

		SelectedQuestID = 0;
		RefreshDisplay();
	}
}

void UROWidget_QuestLog::RefreshDisplay()
{
	// Visual refresh handled in Blueprint.
	// Blueprint iterates AllQuests filtered by CurrentTab (Active vs Completed)
	// and creates quest list entry widgets in QuestListScrollBox.

	// Clear detail panel if no quest selected
	if (SelectedQuestID == 0)
	{
		if (Text_QuestName) Text_QuestName->SetText(FText::GetEmpty());
		if (Text_QuestDescription) Text_QuestDescription->SetText(FText::GetEmpty());
		if (Btn_Abandon) Btn_Abandon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UROWidget_QuestLog::OnTabActiveClicked() { SwitchTab(EROQuestTab::Active); }
void UROWidget_QuestLog::OnTabCompletedClicked() { SwitchTab(EROQuestTab::Completed); }
void UROWidget_QuestLog::OnAbandonClicked() { AbandonSelectedQuest(); }
