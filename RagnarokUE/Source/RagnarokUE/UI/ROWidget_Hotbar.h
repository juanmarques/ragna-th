// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "ROWidget_Hotbar.generated.h"

class UHorizontalBox;
class UTextBlock;
class UImage;
class UProgressBar;
class UROItemDatabase;
class UROAbilitySystemComponent;

/**
 * FROHotbarSlot
 * Represents a single hotbar slot that can hold a skill or item.
 */
USTRUCT(BlueprintType)
struct FROHotbarSlot
{
	GENERATED_BODY()

	/** Whether this slot contains a skill (true) or item (false). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hotbar")
	bool bIsSkill = false;

	/** Skill or Item ID depending on bIsSkill. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hotbar")
	int32 ID = 0;

	/** Skill level (only relevant for skills). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hotbar")
	int32 Level = 0;

	/** Icon texture for the slot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hotbar")
	UTexture2D* Icon = nullptr;

	/** SP cost display (for skills). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hotbar")
	int32 SPCost = 0;

	/** Current cooldown remaining (0 = ready). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hotbar")
	float CooldownRemaining = 0.0f;

	/** Total cooldown duration for overlay calculation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hotbar")
	float CooldownTotal = 0.0f;

	/** Item stack count (for items). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hotbar")
	int32 StackCount = 0;

	bool IsEmpty() const { return ID == 0; }

	float GetCooldownPercent() const
	{
		return (CooldownTotal > 0.0f) ? (CooldownRemaining / CooldownTotal) : 0.0f;
	}
};

/**
 * UROWidget_Hotbar
 * Skill/item shortcut bar with 9 slots per row and 4 switchable rows (F1-F9).
 */
UCLASS()
class RAGNAROKUE_API UROWidget_Hotbar : public UUserWidget
{
	GENERATED_BODY()

public:
	UROWidget_Hotbar(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Number of slots per row. */
	static constexpr int32 SLOTS_PER_ROW = 9;

	/** Number of switchable rows. */
	static constexpr int32 NUM_ROWS = 4;

	/** Assign a skill to a specific slot in the current row. */
	UFUNCTION(BlueprintCallable, Category = "RO|Hotbar")
	void AssignSkillToSlot(int32 SlotIndex, int32 SkillID);

	/** Assign an item to a specific slot in the current row. */
	UFUNCTION(BlueprintCallable, Category = "RO|Hotbar")
	void AssignItemToSlot(int32 SlotIndex, int32 ItemID);

	/** Activate the slot (use skill or item). */
	UFUNCTION(BlueprintCallable, Category = "RO|Hotbar")
	void UseSlot(int32 Index);

	/** Switch to a different hotbar row (0-3). */
	UFUNCTION(BlueprintCallable, Category = "RO|Hotbar")
	void SwitchRow(int32 RowIndex);

	/** Get the current active row index. */
	UFUNCTION(BlueprintPure, Category = "RO|Hotbar")
	int32 GetCurrentRow() const { return CurrentRow; }

	/** Clear a slot. */
	UFUNCTION(BlueprintCallable, Category = "RO|Hotbar")
	void ClearSlot(int32 SlotIndex);

	/** Get slot data for UI display. */
	UFUNCTION(BlueprintPure, Category = "RO|Hotbar")
	FROHotbarSlot GetSlotData(int32 SlotIndex) const;

	/** Start cooldown on a slot (called when skill is used). */
	UFUNCTION(BlueprintCallable, Category = "RO|Hotbar")
	void StartCooldown(int32 SlotIndex, float Duration);

	/** Delegate broadcast when a slot is used. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHotbarSlotUsed, int32, SlotIndex, const FROHotbarSlot&, SlotData);

	UPROPERTY(BlueprintAssignable, Category = "RO|Hotbar")
	FOnHotbarSlotUsed OnHotbarSlotUsed;

protected:
	/** Refresh the visual display of the hotbar. */
	UFUNCTION(BlueprintCallable, Category = "RO|Hotbar")
	void RefreshDisplay();

	/** All hotbar rows. Indexed as [RowIndex * SLOTS_PER_ROW + SlotIndex]. */
	UPROPERTY(BlueprintReadOnly, Category = "RO|Hotbar")
	TArray<FROHotbarSlot> AllSlots;

	/** Currently active row. */
	UPROPERTY(BlueprintReadOnly, Category = "RO|Hotbar")
	int32 CurrentRow = 0;

	/** Bound widgets from UMG. */
	UPROPERTY(meta = (BindWidgetOptional))
	UHorizontalBox* SlotContainer;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* RowIndicatorText;
};
