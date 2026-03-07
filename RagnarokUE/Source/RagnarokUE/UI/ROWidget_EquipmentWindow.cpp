// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWidget_EquipmentWindow.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"

UROWidget_EquipmentWindow::UROWidget_EquipmentWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UROWidget_EquipmentWindow::NativeConstruct()
{
	Super::NativeConstruct();

	EquippedItems.SetNum(NUM_EQUIP_SLOTS);

	// Bind slot click handlers
	if (Btn_HeadTop) Btn_HeadTop->OnClicked.AddDynamic(this, &UROWidget_EquipmentWindow::OnSlotClicked_HeadTop);
	if (Btn_HeadMid) Btn_HeadMid->OnClicked.AddDynamic(this, &UROWidget_EquipmentWindow::OnSlotClicked_HeadMid);
	if (Btn_HeadLow) Btn_HeadLow->OnClicked.AddDynamic(this, &UROWidget_EquipmentWindow::OnSlotClicked_HeadLow);
	if (Btn_Weapon) Btn_Weapon->OnClicked.AddDynamic(this, &UROWidget_EquipmentWindow::OnSlotClicked_Weapon);
	if (Btn_Shield) Btn_Shield->OnClicked.AddDynamic(this, &UROWidget_EquipmentWindow::OnSlotClicked_Shield);
	if (Btn_Armor) Btn_Armor->OnClicked.AddDynamic(this, &UROWidget_EquipmentWindow::OnSlotClicked_Armor);
	if (Btn_Garment) Btn_Garment->OnClicked.AddDynamic(this, &UROWidget_EquipmentWindow::OnSlotClicked_Garment);
	if (Btn_Footgear) Btn_Footgear->OnClicked.AddDynamic(this, &UROWidget_EquipmentWindow::OnSlotClicked_Footgear);
	if (Btn_AccessoryL) Btn_AccessoryL->OnClicked.AddDynamic(this, &UROWidget_EquipmentWindow::OnSlotClicked_AccessoryL);
	if (Btn_AccessoryR) Btn_AccessoryR->OnClicked.AddDynamic(this, &UROWidget_EquipmentWindow::OnSlotClicked_AccessoryR);

	RefreshDisplay();
}

int32 UROWidget_EquipmentWindow::SlotToIndex(EROEquipSlot Slot)
{
	return static_cast<int32>(Slot);
}

void UROWidget_EquipmentWindow::SetEquippedItem(EROEquipSlot Slot, const FROItemInstance& Item)
{
	const int32 Index = SlotToIndex(Slot);
	if (EquippedItems.IsValidIndex(Index))
	{
		EquippedItems[Index] = Item;
		RefreshDisplay();
	}
}

void UROWidget_EquipmentWindow::ClearEquipSlot(EROEquipSlot Slot)
{
	const int32 Index = SlotToIndex(Slot);
	if (EquippedItems.IsValidIndex(Index))
	{
		EquippedItems[Index] = FROItemInstance();
		EquippedItems[Index].ItemID = 0;
		RefreshDisplay();
	}
}

FROItemInstance UROWidget_EquipmentWindow::GetEquippedItem(EROEquipSlot Slot) const
{
	const int32 Index = SlotToIndex(Slot);
	if (EquippedItems.IsValidIndex(Index))
	{
		return EquippedItems[Index];
	}
	return FROItemInstance();
}

void UROWidget_EquipmentWindow::SetEquipmentStats(int32 TotalATK, int32 TotalDEF, int32 TotalMDEF)
{
	if (Text_TotalATK) Text_TotalATK->SetText(FText::FromString(FString::Printf(TEXT("ATK: %d"), TotalATK)));
	if (Text_TotalDEF) Text_TotalDEF->SetText(FText::FromString(FString::Printf(TEXT("DEF: %d"), TotalDEF)));
	if (Text_TotalMDEF) Text_TotalMDEF->SetText(FText::FromString(FString::Printf(TEXT("MDEF: %d"), TotalMDEF)));
}

void UROWidget_EquipmentWindow::RefreshDisplay()
{
	// Refresh refine text for slots that support refining
	auto UpdateRefineText = [](UTextBlock* Text, const FROItemInstance& Item)
	{
		if (!Text) return;
		if (Item.IsValid() && Item.RefineLevel > 0)
		{
			Text->SetText(FText::FromString(FString::Printf(TEXT("+%d"), Item.RefineLevel)));
			Text->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			Text->SetVisibility(ESlateVisibility::Collapsed);
		}
	};

	UpdateRefineText(Text_RefineHeadTop, GetEquippedItem(EROEquipSlot::HeadTop));
	UpdateRefineText(Text_RefineWeapon, GetEquippedItem(EROEquipSlot::Weapon));
	UpdateRefineText(Text_RefineShield, GetEquippedItem(EROEquipSlot::Shield));
	UpdateRefineText(Text_RefineArmor, GetEquippedItem(EROEquipSlot::Armor));
	UpdateRefineText(Text_RefineGarment, GetEquippedItem(EROEquipSlot::Garment));
	UpdateRefineText(Text_RefineFootgear, GetEquippedItem(EROEquipSlot::Footgear));

	// Icon visuals are updated in Blueprint using item data lookups
}

void UROWidget_EquipmentWindow::HandleSlotClicked(EROEquipSlot Slot)
{
	const FROItemInstance& Item = GetEquippedItem(Slot);
	if (Item.IsValid())
	{
		OnUnequipItem.Broadcast(Slot, Item);
	}
}

void UROWidget_EquipmentWindow::OnSlotClicked_HeadTop() { HandleSlotClicked(EROEquipSlot::HeadTop); }
void UROWidget_EquipmentWindow::OnSlotClicked_HeadMid() { HandleSlotClicked(EROEquipSlot::HeadMid); }
void UROWidget_EquipmentWindow::OnSlotClicked_HeadLow() { HandleSlotClicked(EROEquipSlot::HeadLow); }
void UROWidget_EquipmentWindow::OnSlotClicked_Weapon() { HandleSlotClicked(EROEquipSlot::Weapon); }
void UROWidget_EquipmentWindow::OnSlotClicked_Shield() { HandleSlotClicked(EROEquipSlot::Shield); }
void UROWidget_EquipmentWindow::OnSlotClicked_Armor() { HandleSlotClicked(EROEquipSlot::Armor); }
void UROWidget_EquipmentWindow::OnSlotClicked_Garment() { HandleSlotClicked(EROEquipSlot::Garment); }
void UROWidget_EquipmentWindow::OnSlotClicked_Footgear() { HandleSlotClicked(EROEquipSlot::Footgear); }
void UROWidget_EquipmentWindow::OnSlotClicked_AccessoryL() { HandleSlotClicked(EROEquipSlot::AccessoryL); }
void UROWidget_EquipmentWindow::OnSlotClicked_AccessoryR() { HandleSlotClicked(EROEquipSlot::AccessoryR); }
