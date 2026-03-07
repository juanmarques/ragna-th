// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROWidget_StatWindow.generated.h"

class UTextBlock;
class UButton;
class UProgressBar;
class UVerticalBox;

/**
 * UROWidget_StatWindow
 * Displays all 6 stats with Base + Bonus format, derived stats,
 * HP/SP bars, EXP bars, and stat allocation buttons.
 */
UCLASS()
class RAGNAROKUE_API UROWidget_StatWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	UROWidget_StatWindow(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	// --- Data Update ---

	/** Update all displayed stats from the character's stat block. */
	UFUNCTION(BlueprintCallable, Category = "RO|StatWindow")
	void UpdateStats(const FROStatBlock& Stats);

	/** Set the available stat points for allocation. */
	UFUNCTION(BlueprintCallable, Category = "RO|StatWindow")
	void SetAvailableStatPoints(int32 Points);

	/** Set base and job levels. */
	UFUNCTION(BlueprintCallable, Category = "RO|StatWindow")
	void SetLevels(int32 BaseLevel, int32 JobLevel);

	/** Set derived combat stats. */
	UFUNCTION(BlueprintCallable, Category = "RO|StatWindow")
	void SetDerivedStats(int32 ATK, int32 MATK, int32 DEF, int32 MDEF, int32 HIT, int32 FLEE, int32 ASPD, int32 CRIT);

	/** Set HP and SP values. */
	UFUNCTION(BlueprintCallable, Category = "RO|StatWindow")
	void SetHPSP(int32 CurrentHP, int32 MaxHP, int32 CurrentSP, int32 MaxSP);

	/** Set EXP bar values. */
	UFUNCTION(BlueprintCallable, Category = "RO|StatWindow")
	void SetEXP(int64 BaseEXP, int64 BaseEXPToNext, int64 JobEXP, int64 JobEXPToNext);

	/** Get the stat point cost to raise a stat by 1. */
	UFUNCTION(BlueprintPure, Category = "RO|StatWindow")
	static int32 GetStatPointCost(int32 CurrentBaseStat);

	/** Delegate broadcast when player clicks a stat allocation button. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAllocateStat, EROStat, Stat);

	UPROPERTY(BlueprintAssignable, Category = "RO|StatWindow")
	FOnAllocateStat OnAllocateStat;

protected:
	/** Called when a stat + button is clicked. */
	UFUNCTION()
	void OnAllocateSTR();
	UFUNCTION()
	void OnAllocateAGI();
	UFUNCTION()
	void OnAllocateVIT();
	UFUNCTION()
	void OnAllocateINT();
	UFUNCTION()
	void OnAllocateDEX();
	UFUNCTION()
	void OnAllocateLUK();

	/** Refresh the allocation button visibility based on available points. */
	void RefreshAllocationButtons();

	// --- Bound Widgets ---

	// Stat text displays (format: "STR: 40 + 10")
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_STR;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_AGI;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_VIT;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_INT;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_DEX;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_LUK;

	// Stat point cost display next to each + button
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_STRCost;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_AGICost;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_VITCost;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_INTCost;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_DEXCost;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_LUKCost;

	// Allocation buttons
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_AllocSTR;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_AllocAGI;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_AllocVIT;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_AllocINT;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_AllocDEX;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_AllocLUK;

	// Derived stats
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_ATK;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_MATK;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_DEF;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_MDEF;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_HIT;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_FLEE;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_ASPD;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_CRIT;

	// Level displays
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_BaseLevel;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_JobLevel;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_StatPoints;

	// HP / SP
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* Bar_HP;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_HP;
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* Bar_SP;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_SP;

	// EXP bars
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* Bar_BaseEXP;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_BaseEXP;
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* Bar_JobEXP;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_JobEXP;

private:
	/** Cached stat block for cost calculations. */
	FROStatBlock CachedStats;

	/** Available stat points. */
	int32 AvailableStatPoints = 0;
};
