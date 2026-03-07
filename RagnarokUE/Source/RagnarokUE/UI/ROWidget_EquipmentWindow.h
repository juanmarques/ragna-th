// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROWidget_EquipmentWindow.generated.h"

class UTextBlock;
class UImage;
class UButton;

/**
 * UROWidget_EquipmentWindow
 * Paper doll equipment display with 10 equipment slots arranged around a character silhouette.
 * Slots: HeadTop, HeadMid, HeadLow, Weapon, Shield, Armor, Garment, Footgear, AccessoryL, AccessoryR
 */
UCLASS()
class RAGNAROKUE_API UROWidget_EquipmentWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	UROWidget_EquipmentWindow(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	/** Number of equipment slots. */
	static constexpr int32 NUM_EQUIP_SLOTS = 10;

	// --- Data Update ---

	/** Set the item in a specific equipment slot. */
	UFUNCTION(BlueprintCallable, Category = "RO|Equipment")
	void SetEquippedItem(EROEquipSlot Slot, const FROItemInstance& Item);

	/** Clear (unequip) a specific slot. */
	UFUNCTION(BlueprintCallable, Category = "RO|Equipment")
	void ClearEquipSlot(EROEquipSlot Slot);

	/** Get the item currently in a slot. */
	UFUNCTION(BlueprintPure, Category = "RO|Equipment")
	FROItemInstance GetEquippedItem(EROEquipSlot Slot) const;

	/** Set total equipment-derived stats for display. */
	UFUNCTION(BlueprintCallable, Category = "RO|Equipment")
	void SetEquipmentStats(int32 TotalATK, int32 TotalDEF, int32 TotalMDEF);

	/** Refresh all slot visuals. */
	UFUNCTION(BlueprintCallable, Category = "RO|Equipment")
	void RefreshDisplay();

	// --- Delegates ---

	/** Broadcast when the player clicks an equipped slot to unequip. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnequipItem, EROEquipSlot, Slot, const FROItemInstance&, Item);

	UPROPERTY(BlueprintAssignable, Category = "RO|Equipment")
	FOnUnequipItem OnUnequipItem;

	/** Broadcast when an item is dragged from inventory onto an equipment slot. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipItemFromInventory, EROEquipSlot, Slot, const FROItemInstance&, Item);

	UPROPERTY(BlueprintAssignable, Category = "RO|Equipment")
	FOnEquipItemFromInventory OnEquipItemFromInventory;

protected:
	/** Slot click handlers for unequipping. */
	UFUNCTION()
	void OnSlotClicked_HeadTop();
	UFUNCTION()
	void OnSlotClicked_HeadMid();
	UFUNCTION()
	void OnSlotClicked_HeadLow();
	UFUNCTION()
	void OnSlotClicked_Weapon();
	UFUNCTION()
	void OnSlotClicked_Shield();
	UFUNCTION()
	void OnSlotClicked_Armor();
	UFUNCTION()
	void OnSlotClicked_Garment();
	UFUNCTION()
	void OnSlotClicked_Footgear();
	UFUNCTION()
	void OnSlotClicked_AccessoryL();
	UFUNCTION()
	void OnSlotClicked_AccessoryR();

	/** Helper to handle unequip for a given slot. */
	void HandleSlotClicked(EROEquipSlot Slot);

	/** Convert enum to array index. */
	static int32 SlotToIndex(EROEquipSlot Slot);

	// --- Bound Widgets ---

	// Slot buttons (click to unequip)
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_HeadTop;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_HeadMid;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_HeadLow;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Weapon;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Shield;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Armor;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Garment;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Footgear;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_AccessoryL;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_AccessoryR;

	// Slot icon images
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_HeadTop;
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_HeadMid;
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_HeadLow;
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_Weapon;
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_Shield;
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_Armor;
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_Garment;
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_Footgear;
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_AccessoryL;
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_AccessoryR;

	// Refine level text per slot
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_RefineHeadTop;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_RefineWeapon;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_RefineShield;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_RefineArmor;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_RefineGarment;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_RefineFootgear;

	// Equipment stat summaries
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_TotalATK;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_TotalDEF;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_TotalMDEF;

private:
	/** Equipped items indexed by SlotToIndex. */
	UPROPERTY()
	TArray<FROItemInstance> EquippedItems;
};
