// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROWidget_InventoryWindow.generated.h"

class UUniformGridPanel;
class UTextBlock;
class UButton;

/**
 * Inventory filter tab enumeration.
 */
UENUM(BlueprintType)
enum class EROInventoryTab : uint8
{
	All			UMETA(DisplayName = "All"),
	Usable		UMETA(DisplayName = "Usable"),
	Equipment	UMETA(DisplayName = "Equipment"),
	Etc			UMETA(DisplayName = "Etc")
};

/**
 * Context menu action for inventory items.
 */
UENUM(BlueprintType)
enum class EROInventoryAction : uint8
{
	Use,
	Equip,
	Drop,
	Info
};

/**
 * UROWidget_InventoryWindow
 * Grid-based inventory with 100 slots (10x10), tab filtering,
 * drag-and-drop, context menu, weight and zeny display.
 */
UCLASS()
class RAGNAROKUE_API UROWidget_InventoryWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	UROWidget_InventoryWindow(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	/** Total number of inventory slots. */
	static constexpr int32 INVENTORY_COLUMNS = 10;
	static constexpr int32 INVENTORY_ROWS = 10;
	static constexpr int32 MAX_SLOTS = INVENTORY_COLUMNS * INVENTORY_ROWS;

	// --- Data Update ---

	/** Set the full inventory contents. */
	UFUNCTION(BlueprintCallable, Category = "RO|Inventory")
	void SetInventoryItems(const TArray<FROItemInstance>& Items);

	/** Update a single slot. */
	UFUNCTION(BlueprintCallable, Category = "RO|Inventory")
	void UpdateSlot(int32 SlotIndex, const FROItemInstance& Item);

	/** Clear a slot. */
	UFUNCTION(BlueprintCallable, Category = "RO|Inventory")
	void ClearSlot(int32 SlotIndex);

	/** Set weight display values. */
	UFUNCTION(BlueprintCallable, Category = "RO|Inventory")
	void SetWeight(int32 CurrentWeight, int32 MaxWeight);

	/** Set zeny display. */
	UFUNCTION(BlueprintCallable, Category = "RO|Inventory")
	void SetZeny(int64 Zeny);

	/** Switch the active filter tab. */
	UFUNCTION(BlueprintCallable, Category = "RO|Inventory")
	void SwitchTab(EROInventoryTab Tab);

	/** Get the current tab filter. */
	UFUNCTION(BlueprintPure, Category = "RO|Inventory")
	EROInventoryTab GetCurrentTab() const { return CurrentTab; }

	/** Get item in a specific slot. */
	UFUNCTION(BlueprintPure, Category = "RO|Inventory")
	FROItemInstance GetItemAtSlot(int32 SlotIndex) const;

	// --- Delegates ---

	/** Broadcast when a context menu action is chosen on an item. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInventoryAction, int32, SlotIndex, const FROItemInstance&, Item, EROInventoryAction, Action);

	UPROPERTY(BlueprintAssignable, Category = "RO|Inventory")
	FOnInventoryAction OnInventoryAction;

	/** Broadcast when an item is dragged to another widget (hotbar, equipment, trade). */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemDragged, int32, SlotIndex, const FROItemInstance&, Item);

	UPROPERTY(BlueprintAssignable, Category = "RO|Inventory")
	FOnInventoryItemDragged OnInventoryItemDragged;

protected:
	/** Refresh the grid display based on current tab filter. */
	UFUNCTION(BlueprintCallable, Category = "RO|Inventory")
	void RefreshDisplay();

	/** Handle right-click context menu action. */
	UFUNCTION(BlueprintCallable, Category = "RO|Inventory")
	void ExecuteContextAction(int32 SlotIndex, EROInventoryAction Action);

	// --- Tab buttons ---
	UFUNCTION()
	void OnTabAllClicked();
	UFUNCTION()
	void OnTabUsableClicked();
	UFUNCTION()
	void OnTabEquipmentClicked();
	UFUNCTION()
	void OnTabEtcClicked();

	// --- Bound Widgets ---

	UPROPERTY(meta = (BindWidgetOptional))
	UUniformGridPanel* ItemGrid;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_Weight;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_Zeny;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_TabAll;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_TabUsable;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_TabEquipment;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_TabEtc;

private:
	/** All inventory items. */
	UPROPERTY()
	TArray<FROItemInstance> InventoryItems;

	/** Current filter tab. */
	EROInventoryTab CurrentTab = EROInventoryTab::All;

	/** Current weight values. */
	int32 CurrentWeight = 0;
	int32 MaxWeight = 0;

	/** Current zeny. */
	int64 CurrentZeny = 0;
};
