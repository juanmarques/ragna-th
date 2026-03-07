// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROWidget_ShopWindow.generated.h"

class UTextBlock;
class UButton;
class UScrollBox;
class UWidgetSwitcher;
class UEditableTextBox;

/**
 * Shop tab: Buy or Sell.
 */
UENUM(BlueprintType)
enum class EROShopTab : uint8
{
	Buy		UMETA(DisplayName = "Buy"),
	Sell	UMETA(DisplayName = "Sell")
};

/**
 * FROShopItem
 * An item available for purchase at a shop.
 */
USTRUCT(BlueprintType)
struct FROShopItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 ItemID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 Price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	UTexture2D* Icon = nullptr;
};

/**
 * FROSellableItem
 * An inventory item that can be sold.
 */
USTRUCT(BlueprintType)
struct FROSellableItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FROItemInstance Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 SellPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	UTexture2D* Icon = nullptr;
};

/**
 * UROWidget_ShopWindow
 * Buy/Sell shop interface with item lists, quantity input, total display,
 * and Discount/Overcharge modifier indicators.
 */
UCLASS()
class RAGNAROKUE_API UROWidget_ShopWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	UROWidget_ShopWindow(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	// --- Data Update ---

	/** Set the shop's buy item list. */
	UFUNCTION(BlueprintCallable, Category = "RO|Shop")
	void SetShopItems(const TArray<FROShopItem>& Items);

	/** Set the player's sellable items. */
	UFUNCTION(BlueprintCallable, Category = "RO|Shop")
	void SetSellableItems(const TArray<FROSellableItem>& Items);

	/** Set the Discount skill modifier (percentage reduction, e.g., 24 = 24% off). */
	UFUNCTION(BlueprintCallable, Category = "RO|Shop")
	void SetDiscountModifier(float DiscountPercent);

	/** Set the Overcharge skill modifier (percentage increase in sell price). */
	UFUNCTION(BlueprintCallable, Category = "RO|Shop")
	void SetOverchargeModifier(float OverchargePercent);

	/** Switch between Buy and Sell tabs. */
	UFUNCTION(BlueprintCallable, Category = "RO|Shop")
	void SwitchTab(EROShopTab Tab);

	/** Update the total cost/earnings display. */
	UFUNCTION(BlueprintCallable, Category = "RO|Shop")
	void UpdateTotal(int64 TotalAmount);

	// --- Delegates ---

	/** Broadcast when the player buys an item. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuyItem, int32, ItemID, int32, Quantity);

	UPROPERTY(BlueprintAssignable, Category = "RO|Shop")
	FOnBuyItem OnBuyItem;

	/** Broadcast when the player sells an item. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSellItem, int32, InventorySlotIndex, int32, Quantity);

	UPROPERTY(BlueprintAssignable, Category = "RO|Shop")
	FOnSellItem OnSellItem;

protected:
	/** Refresh the display. */
	UFUNCTION(BlueprintCallable, Category = "RO|Shop")
	void RefreshDisplay();

	// --- Tab button handlers ---
	UFUNCTION()
	void OnTabBuyClicked();
	UFUNCTION()
	void OnTabSellClicked();

	// --- Bound Widgets ---

	UPROPERTY(meta = (BindWidgetOptional))
	UWidgetSwitcher* ShopTabSwitcher;

	UPROPERTY(meta = (BindWidgetOptional))
	UScrollBox* BuyItemList;

	UPROPERTY(meta = (BindWidgetOptional))
	UScrollBox* SellItemList;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_Total;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_DiscountModifier;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_OverchargeModifier;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_TabBuy;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_TabSell;

private:
	/** Shop items for buying. */
	TArray<FROShopItem> ShopItems;

	/** Items the player can sell. */
	TArray<FROSellableItem> SellableItems;

	/** Current tab. */
	EROShopTab CurrentTab = EROShopTab::Buy;

	/** Discount percentage. */
	float DiscountPercent = 0.0f;

	/** Overcharge percentage. */
	float OverchargePercent = 0.0f;
};
