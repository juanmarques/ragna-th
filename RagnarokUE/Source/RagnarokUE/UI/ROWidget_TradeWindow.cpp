// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWidget_TradeWindow.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"

UROWidget_TradeWindow::UROWidget_TradeWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UROWidget_TradeWindow::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_LockConfirm) Btn_LockConfirm->OnClicked.AddDynamic(this, &UROWidget_TradeWindow::OnLockConfirmClicked);
	if (Btn_Cancel) Btn_Cancel->OnClicked.AddDynamic(this, &UROWidget_TradeWindow::OnCancelClicked);
	if (Input_MyZeny) Input_MyZeny->OnTextCommitted.AddDynamic(this, &UROWidget_TradeWindow::OnZenyInputCommitted);

	RefreshDisplay();
}

void UROWidget_TradeWindow::SetTradePartnerName(const FString& Name)
{
	PartnerName = Name;
	if (Text_PartnerName)
	{
		Text_PartnerName->SetText(FText::FromString(Name));
	}
}

void UROWidget_TradeWindow::AddMyItem(const FROItemInstance& Item)
{
	if (MyItems.Num() >= MAX_TRADE_ITEMS || MyState != EROTradeState::Waiting)
	{
		return;
	}
	MyItems.Add(Item);
	RefreshDisplay();
}

void UROWidget_TradeWindow::RemoveMyItem(int32 SlotIndex)
{
	if (MyState != EROTradeState::Waiting || !MyItems.IsValidIndex(SlotIndex))
	{
		return;
	}
	MyItems.RemoveAt(SlotIndex);
	RefreshDisplay();
}

void UROWidget_TradeWindow::SetTheirItems(const TArray<FROItemInstance>& Items)
{
	TheirItems = Items;
	RefreshDisplay();
}

void UROWidget_TradeWindow::SetTheirZeny(int64 Zeny)
{
	TheirZeny = Zeny;
	if (Text_TheirZeny)
	{
		Text_TheirZeny->SetText(FText::FromString(FString::Printf(TEXT("%lld z"), Zeny)));
	}
}

void UROWidget_TradeWindow::SetMyState(EROTradeState State)
{
	MyState = State;

	if (Text_MyStatus)
	{
		Text_MyStatus->SetText(FText::FromString(GetStateText(State)));
	}

	// Update the lock/confirm button label
	if (Text_LockConfirmLabel)
	{
		switch (State)
		{
		case EROTradeState::Waiting:
			Text_LockConfirmLabel->SetText(FText::FromString(TEXT("Lock")));
			break;
		case EROTradeState::Locked:
			Text_LockConfirmLabel->SetText(FText::FromString(TEXT("Confirm")));
			break;
		case EROTradeState::Confirmed:
			Text_LockConfirmLabel->SetText(FText::FromString(TEXT("Confirmed")));
			break;
		}
	}

	// Disable zeny input when not in Waiting state
	if (Input_MyZeny)
	{
		Input_MyZeny->SetIsReadOnly(State != EROTradeState::Waiting);
	}

	// Disable lock/confirm button after confirmation
	if (Btn_LockConfirm)
	{
		Btn_LockConfirm->SetIsEnabled(State != EROTradeState::Confirmed);
	}

	RefreshDisplay();
}

void UROWidget_TradeWindow::SetTheirState(EROTradeState State)
{
	TheirState = State;

	if (Text_TheirStatus)
	{
		Text_TheirStatus->SetText(FText::FromString(GetStateText(State)));
	}
}

FString UROWidget_TradeWindow::GetStateText(EROTradeState State)
{
	switch (State)
	{
	case EROTradeState::Waiting:   return TEXT("Waiting");
	case EROTradeState::Locked:    return TEXT("Locked");
	case EROTradeState::Confirmed: return TEXT("Confirmed");
	default:                       return TEXT("Unknown");
	}
}

void UROWidget_TradeWindow::OnLockConfirmClicked()
{
	switch (MyState)
	{
	case EROTradeState::Waiting:
		OnTradeLock.Broadcast();
		break;
	case EROTradeState::Locked:
		OnTradeConfirm.Broadcast();
		break;
	default:
		break;
	}
}

void UROWidget_TradeWindow::OnCancelClicked()
{
	OnTradeCancel.Broadcast();
}

void UROWidget_TradeWindow::OnZenyInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (MyState != EROTradeState::Waiting)
	{
		return;
	}

	const FString ZenyStr = Text.ToString();
	MyZeny = FCString::Atoi64(*ZenyStr);
	MyZeny = FMath::Max(MyZeny, static_cast<int64>(0));

	OnTradeZenyChanged.Broadcast(MyZeny);
}

void UROWidget_TradeWindow::RefreshDisplay()
{
	// Visual item display is handled in Blueprint via MyItemsPanel and TheirItemsPanel.
	// Blueprint creates item slot widgets for each item in MyItems and TheirItems arrays.
}
