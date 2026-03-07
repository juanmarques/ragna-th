// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROWidget_SkillWindow.generated.h"

class UTextBlock;
class UButton;
class UScrollBox;
class UImage;

/**
 * FROSkillTreeEntry
 * Represents a single skill node in the skill tree display.
 */
USTRUCT(BlueprintType)
struct FROSkillTreeEntry
{
	GENERATED_BODY()

	/** Skill info data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	FROSkillInfo SkillInfo;

	/** Grid position in the skill tree layout. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	FVector2D TreePosition = FVector2D::ZeroVector;

	/** Prerequisite skill IDs (for drawing connecting lines). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	TArray<int32> PrerequisiteSkillIDs;

	/** Prerequisite levels required for each prerequisite. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	TArray<int32> PrerequisiteLevels;

	/** Skill icon texture. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	UTexture2D* Icon = nullptr;

	/** Display name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	FText DisplayName;

	/** Whether all prerequisites are met. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	bool bPrerequisitesMet = false;

	/** Whether this skill can be leveled up (prereqs met + points available + not max). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	bool bCanLevelUp = false;
};

/**
 * UROWidget_SkillWindow
 * Skill tree display per job class with level up buttons, prerequisite lines,
 * and drag-to-hotbar functionality.
 */
UCLASS()
class RAGNAROKUE_API UROWidget_SkillWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	UROWidget_SkillWindow(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	// --- Data Update ---

	/** Set the skill tree data for the current job. */
	UFUNCTION(BlueprintCallable, Category = "RO|Skills")
	void SetSkillTree(const TArray<FROSkillTreeEntry>& Skills);

	/** Update a single skill's learned level. */
	UFUNCTION(BlueprintCallable, Category = "RO|Skills")
	void UpdateSkillLevel(int32 SkillID, int32 NewLevel);

	/** Set the available skill points. */
	UFUNCTION(BlueprintCallable, Category = "RO|Skills")
	void SetSkillPoints(int32 Points);

	/** Switch between job tier tabs. */
	UFUNCTION(BlueprintCallable, Category = "RO|Skills")
	void SwitchJobTierTab(EROJobTier Tier);

	/** Get the current job tier tab. */
	UFUNCTION(BlueprintPure, Category = "RO|Skills")
	EROJobTier GetCurrentTierTab() const { return CurrentTierTab; }

	/** Request to level up a skill. */
	UFUNCTION(BlueprintCallable, Category = "RO|Skills")
	void RequestLevelUpSkill(int32 SkillID);

	// --- Delegates ---

	/** Broadcast when player clicks level up on a skill. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillLevelUp, int32, SkillID);

	UPROPERTY(BlueprintAssignable, Category = "RO|Skills")
	FOnSkillLevelUp OnSkillLevelUp;

	/** Broadcast when a skill is dragged to the hotbar. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillDraggedToHotbar, int32, SkillID);

	UPROPERTY(BlueprintAssignable, Category = "RO|Skills")
	FOnSkillDraggedToHotbar OnSkillDraggedToHotbar;

protected:
	/** Refresh the skill tree display. */
	UFUNCTION(BlueprintCallable, Category = "RO|Skills")
	void RefreshDisplay();

	/** Recalculate which skills can be leveled up based on prerequisites and points. */
	void RecalculatePrerequisites();

	// --- Tab button handlers ---
	UFUNCTION()
	void OnTab1stClassClicked();
	UFUNCTION()
	void OnTab2ndClassClicked();

	// --- Bound Widgets ---

	UPROPERTY(meta = (BindWidgetOptional))
	UScrollBox* SkillTreeScrollBox;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_SkillPoints;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Tab1stClass;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Tab2ndClass;

private:
	/** All skill tree entries. */
	UPROPERTY()
	TArray<FROSkillTreeEntry> SkillEntries;

	/** Current job tier tab being displayed. */
	EROJobTier CurrentTierTab = EROJobTier::First;

	/** Available skill points. */
	int32 AvailableSkillPoints = 0;
};
