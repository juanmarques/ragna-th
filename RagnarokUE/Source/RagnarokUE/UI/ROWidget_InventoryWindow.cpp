// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWidget_InventoryWindow.h"
#include "Components/UniformGridPanel.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

UROWidget_InventoryWindow::UROWidget_InventoryWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UROWidget_InventoryWindow::NativeConstruct()
{
	Super::NativeConstruct();

	InventoryItems.SetNum(MAX_SLOTS);

	// Bind tab buttons
	if (Btn_TabAll) Btn_TabAll->OnClicked.AddDynamic(this, &UROWidget_InventoryWindow::OnTabAllClicked);
	if (Btn_TabUsable) Btn_TabUsable->OnClicked.AddDynamic(this, &UROWidget_InventoryWindow::OnTabUsableClicked);
	if (Btn_TabEquipment) Btn_TabEquipment->OnClicked.AddDynamic(this, &UROWidget_InventoryWindow::OnTabEquipmentClicked);
	if (Btn_TabEtc) Btn_TabEtc->OnClicked.AddDynamic(this, &UROWidget_InventoryWindow::OnTabEtcClicked);

	RefreshDisplay();
}

void UROWidget_InventoryWindow::SetInventoryItems(const TArray<FROItemInstance>& Items)
{
	InventoryItems.SetNum(MAX_SLOTS);
	for (int32 i = 0; i < FMath::Min(Items.Num(), MAX_SLOTS); ++i)
	{
		InventoryItems[i] = Items[i];
	}
	RefreshDisplay();
}

void UROWidget_InventoryWindow::UpdateSlot(int32 SlotIndex, const FROItemInstance& Item)
{
	if (InventoryItems.IsValidIndex(SlotIndex))
	{
		InventoryItems[SlotIndex] = Item;
		RefreshDisplay();
	}
}

void UROWidget_InventoryWindow::ClearSlot(int32 SlotIndex)
{
	if (InventoryItems.IsValidIndex(SlotIndex))
	{
		InventoryItems[SlotIndex] = FROItemInstance();
		InventoryItems[SlotIndex].ItemID = 0;
		RefreshDisplay();
	}
}

void UROWidget_InventoryWindow::SetWeight(int32 InCurrentWeight, int32 InMaxWeight)
{
	CurrentWeight = InCurrentWeight;
	MaxWeight = InMaxWeight;

	if (Text_Weight)
	{
		Text_Weight->SetText(FText::FromString(FString::Printf(TEXT("Weight: %d / %d"), CurrentWeight, MaxWeight)));
	}
}

void UROWidget_InventoryWindow::SetZeny(int64 Zeny)
{
	CurrentZeny = Zeny;

	if (Text_Zeny)
	{
		Text_Zeny->SetText(FText::FromString(FString::Printf(TEXT("Zeny: %lld"), Zeny)));
	}
}

void UROWidget_InventoryWindow::SwitchTab(EROInventoryTab Tab)
{
	CurrentTab = Tab;
	RefreshDisplay();
}

FROItemInstance UROWidget_InventoryWindow::GetItemAtSlot(int32 SlotIndex) const
{
	if (InventoryItems.IsValidIndex(SlotIndex))
	{
		return InventoryItems[SlotIndex];
	}
	return FROItemInstance();
}

void UROWidget_InventoryWindow::ExecuteContextAction(int32 SlotIndex, EROInventoryAction Action)
{
	if (!InventoryItems.IsValidIndex(SlotIndex))
	{
		return;
	}

	const FROItemInstance& Item = InventoryItems[SlotIndex];
	if (!Item.IsValid())
	{
		return;
	}

	OnInventoryAction.Broadcast(SlotIndex, Item, Action);
}

void UROWidget_InventoryWindow::RefreshDisplay()
{
	// Visual refresh is handled in Blueprint via the ItemGrid.
	// C++ provides the data; Blueprint iterates InventoryItems and creates slot widgets.
	// Filtering by CurrentTab is applied in Blueprint based on item type lookup.
}

void UROWidget_InventoryWindow::OnTabAllClicked() { SwitchTab(EROInventoryTab::All); }
void UROWidget_InventoryWindow::OnTabUsableClicked() { SwitchTab(EROInventoryTab::Usable); }
void UROWidget_InventoryWindow::OnTabEquipmentClicked() { SwitchTab(EROInventoryTab::Equipment); }
void UROWidget_InventoryWindow::OnTabEtcClicked() { SwitchTab(EROInventoryTab::Etc); }
