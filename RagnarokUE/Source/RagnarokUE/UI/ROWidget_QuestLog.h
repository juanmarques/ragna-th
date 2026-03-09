// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ROWidget_QuestLog.generated.h"

class UScrollBox;
class UTextBlock;
class UButton;
class UProgressBar;
class UWidgetSwitcher;
class URichTextBlock;

/**
 * Quest log tab.
 */
UENUM(BlueprintType)
enum class EROQuestTab : uint8
{
	Active		UMETA(DisplayName = "Active"),
	Completed	UMETA(DisplayName = "Completed")
};

/**
 * FROQuestObjectiveDisplay
 * A single objective within a quest.
 */
USTRUCT(BlueprintType)
struct FROQuestObjectiveDisplay
{
	GENERATED_BODY()

	/** Objective description text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FString Description;

	/** Current progress count. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 CurrentProgress = 0;

	/** Required count for completion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 RequiredProgress = 1;

	/** Whether this objective is complete. */
	bool IsComplete() const { return CurrentProgress >= RequiredProgress; }

	/** Get progress as a fraction. */
	float GetProgressPercent() const
	{
		return RequiredProgress > 0 ? static_cast<float>(CurrentProgress) / static_cast<float>(RequiredProgress) : 0.0f;
	}
};

/**
 * FROQuestEntry
 * Represents a single quest.
 */
USTRUCT(BlueprintType)
struct FROQuestEntry
{
	GENERATED_BODY()

	/** Quest ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 QuestID = 0;

	/** Quest name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FString QuestName;

	/** Quest description / flavor text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FString Description;

	/** Quest objectives. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FROQuestObjectiveDisplay> Objectives;

	/** Whether the quest is completed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bIsComplete = false;
};

/**
 * UROWidget_QuestLog
 * Quest log displaying active and completed quests with objectives,
 * progress bars, and abandon functionality.
 */
UCLASS()
class RAGNAROKUE_API UROWidget_QuestLog : public UUserWidget
{
	GENERATED_BODY()

public:
	UROWidget_QuestLog(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	// --- Data Update ---

	/** Set the full quest list. */
	UFUNCTION(BlueprintCallable, Category = "RO|Quest")
	void SetQuests(const TArray<FROQuestEntry>& Quests);

	/** Update a single quest's objectives. */
	UFUNCTION(BlueprintCallable, Category = "RO|Quest")
	void UpdateQuestProgress(int32 QuestID, const TArray<FROQuestObjectiveDisplay>& Objectives);

	/** Mark a quest as completed. */
	UFUNCTION(BlueprintCallable, Category = "RO|Quest")
	void MarkQuestComplete(int32 QuestID);

	/** Switch the active tab. */
	UFUNCTION(BlueprintCallable, Category = "RO|Quest")
	void SwitchTab(EROQuestTab Tab);

	/** Select a quest to show details. */
	UFUNCTION(BlueprintCallable, Category = "RO|Quest")
	void SelectQuest(int32 QuestID);

	/** Request to abandon the selected quest. */
	UFUNCTION(BlueprintCallable, Category = "RO|Quest")
	void AbandonSelectedQuest();

	// --- Delegates ---

	/** Broadcast when the player abandons a quest. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbandonQuest, int32, QuestID);

	UPROPERTY(BlueprintAssignable, Category = "RO|Quest")
	FOnAbandonQuest OnAbandonQuest;

protected:
	/** Refresh the quest list and detail panels. */
	UFUNCTION(BlueprintCallable, Category = "RO|Quest")
	void RefreshDisplay();

	// --- Tab button handlers ---
	UFUNCTION()
	void OnTabActiveClicked();
	UFUNCTION()
	void OnTabCompletedClicked();
	UFUNCTION()
	void OnAbandonClicked();

	// --- Bound Widgets ---

	UPROPERTY(meta = (BindWidgetOptional))
	UScrollBox* QuestListScrollBox;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_QuestName;

	UPROPERTY(meta = (BindWidgetOptional))
	URichTextBlock* Text_QuestDescription;

	UPROPERTY(meta = (BindWidgetOptional))
	UScrollBox* ObjectivesScrollBox;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_TabActive;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_TabCompleted;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Abandon;

private:
	/** All quests. */
	TArray<FROQuestEntry> AllQuests;

	/** Currently selected quest ID. */
	int32 SelectedQuestID = 0;

	/** Current tab. */
	EROQuestTab CurrentTab = EROQuestTab::Active;
};
