// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWidget_ShopWindow.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableTextBox.h"

UROWidget_ShopWindow::UROWidget_ShopWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UROWidget_ShopWindow::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_TabBuy) Btn_TabBuy->OnClicked.AddDynamic(this, &UROWidget_ShopWindow::OnTabBuyClicked);
	if (Btn_TabSell) Btn_TabSell->OnClicked.AddDynamic(this, &UROWidget_ShopWindow::OnTabSellClicked);

	RefreshDisplay();
}

void UROWidget_ShopWindow::SetShopItems(const TArray<FROShopItem>& Items)
{
	ShopItems = Items;
	RefreshDisplay();
}

void UROWidget_ShopWindow::SetSellableItems(const TArray<FROSellableItem>& Items)
{
	SellableItems = Items;
	RefreshDisplay();
}

void UROWidget_ShopWindow::SetDiscountModifier(float InDiscountPercent)
{
	DiscountPercent = InDiscountPercent;

	if (Text_DiscountModifier)
	{
		if (DiscountPercent > 0.0f)
		{
			Text_DiscountModifier->SetText(FText::FromString(FString::Printf(TEXT("Discount: -%.0f%%"), DiscountPercent)));
			Text_DiscountModifier->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			Text_DiscountModifier->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UROWidget_ShopWindow::SetOverchargeModifier(float InOverchargePercent)
{
	OverchargePercent = InOverchargePercent;

	if (Text_OverchargeModifier)
	{
		if (OverchargePercent > 0.0f)
		{
			Text_OverchargeModifier->SetText(FText::FromString(FString::Printf(TEXT("Overcharge: +%.0f%%"), OverchargePercent)));
			Text_OverchargeModifier->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			Text_OverchargeModifier->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UROWidget_ShopWindow::SwitchTab(EROShopTab Tab)
{
	CurrentTab = Tab;

	if (ShopTabSwitcher)
	{
		ShopTabSwitcher->SetActiveWidgetIndex(static_cast<int32>(Tab));
	}

	RefreshDisplay();
}

void UROWidget_ShopWindow::UpdateTotal(int64 TotalAmount)
{
	if (Text_Total)
	{
		if (CurrentTab == EROShopTab::Buy)
		{
			Text_Total->SetText(FText::FromString(FString::Printf(TEXT("Total Cost: %lld z"), TotalAmount)));
		}
		else
		{
			Text_Total->SetText(FText::FromString(FString::Printf(TEXT("Total Earnings: %lld z"), TotalAmount)));
		}
	}
}

void UROWidget_ShopWindow::RefreshDisplay()
{
	// Visual item list display is handled in Blueprint.
	// Blueprint creates shop entry widgets in BuyItemList / SellItemList
	// with icon, name, price, quantity input, and buy/sell buttons.

	if (Text_Total)
	{
		Text_Total->SetText(FText::FromString(TEXT("Total: 0 z")));
	}
}

void UROWidget_ShopWindow::OnTabBuyClicked() { SwitchTab(EROShopTab::Buy); }
void UROWidget_ShopWindow::OnTabSellClicked() { SwitchTab(EROShopTab::Sell); }
