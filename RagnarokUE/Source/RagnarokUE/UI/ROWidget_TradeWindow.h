// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROWidget_TradeWindow.generated.h"

class UTextBlock;
class UButton;
class UEditableTextBox;
class UScrollBox;

/**
 * Trade session state.
 */
UENUM(BlueprintType)
enum class EROTradeState : uint8
{
	Waiting		UMETA(DisplayName = "Waiting"),
	Locked		UMETA(DisplayName = "Locked"),
	Confirmed	UMETA(DisplayName = "Confirmed")
};

/**
 * UROWidget_TradeWindow
 * Two-panel trade window with item offers, zeny input, lock/confirm
 * two-step process, and cancel button.
 */
UCLASS()
class RAGNAROKUE_API UROWidget_TradeWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	UROWidget_TradeWindow(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	/** Max items per trade side. */
	static constexpr int32 MAX_TRADE_ITEMS = 10;

	// --- Data Update ---

	/** Set the other player's name. */
	UFUNCTION(BlueprintCallable, Category = "RO|Trade")
	void SetTradePartnerName(const FString& Name);

	/** Add an item to my offer. */
	UFUNCTION(BlueprintCallable, Category = "RO|Trade")
	void AddMyItem(const FROItemInstance& Item);

	/** Remove an item from my offer. */
	UFUNCTION(BlueprintCallable, Category = "RO|Trade")
	void RemoveMyItem(int32 SlotIndex);

	/** Set the other player's offered items. */
	UFUNCTION(BlueprintCallable, Category = "RO|Trade")
	void SetTheirItems(const TArray<FROItemInstance>& Items);

	/** Set the other player's offered zeny. */
	UFUNCTION(BlueprintCallable, Category = "RO|Trade")
	void SetTheirZeny(int64 Zeny);

	/** Set my trade state. */
	UFUNCTION(BlueprintCallable, Category = "RO|Trade")
	void SetMyState(EROTradeState State);

	/** Set their trade state. */
	UFUNCTION(BlueprintCallable, Category = "RO|Trade")
	void SetTheirState(EROTradeState State);

	/** Get my current trade state. */
	UFUNCTION(BlueprintPure, Category = "RO|Trade")
	EROTradeState GetMyState() const { return MyState; }

	// --- Delegates ---

	/** Broadcast when the player locks their offer. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTradeLock);
	UPROPERTY(BlueprintAssignable, Category = "RO|Trade")
	FOnTradeLock OnTradeLock;

	/** Broadcast when the player confirms the trade. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTradeConfirm);
	UPROPERTY(BlueprintAssignable, Category = "RO|Trade")
	FOnTradeConfirm OnTradeConfirm;

	/** Broadcast when the player cancels the trade. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTradeCancel);
	UPROPERTY(BlueprintAssignable, Category = "RO|Trade")
	FOnTradeCancel OnTradeCancel;

	/** Broadcast when zeny amount changes. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTradeZenyChanged, int64, Amount);
	UPROPERTY(BlueprintAssignable, Category = "RO|Trade")
	FOnTradeZenyChanged OnTradeZenyChanged;

protected:
	/** Refresh the display. */
	UFUNCTION(BlueprintCallable, Category = "RO|Trade")
	void RefreshDisplay();

	// --- Button handlers ---
	UFUNCTION()
	void OnLockConfirmClicked();
	UFUNCTION()
	void OnCancelClicked();
	UFUNCTION()
	void OnZenyInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	// --- Bound Widgets ---

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_PartnerName;

	UPROPERTY(meta = (BindWidgetOptional))
	UScrollBox* MyItemsPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	UScrollBox* TheirItemsPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	UEditableTextBox* Input_MyZeny;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_TheirZeny;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_LockConfirm;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_LockConfirmLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Cancel;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_MyStatus;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_TheirStatus;

private:
	/** Items I am offering. */
	TArray<FROItemInstance> MyItems;

	/** Items the partner is offering. */
	TArray<FROItemInstance> TheirItems;

	/** My offered zeny. */
	int64 MyZeny = 0;

	/** Their offered zeny. */
	int64 TheirZeny = 0;

	/** My current trade state. */
	EROTradeState MyState = EROTradeState::Waiting;

	/** Their current trade state. */
	EROTradeState TheirState = EROTradeState::Waiting;

	/** Partner name. */
	FString PartnerName;

	/** Helper to get status display text. */
	static FString GetStateText(EROTradeState State);
};
