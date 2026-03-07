// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "ROWidget_ChatWindow.generated.h"

class UScrollBox;
class UTextBlock;
class UEditableTextBox;
class UButton;
class URichTextBlock;

/**
 * Chat channel types.
 */
UENUM(BlueprintType)
enum class EROChatChannel : uint8
{
	Local		UMETA(DisplayName = "Local"),
	Party		UMETA(DisplayName = "Party"),
	Guild		UMETA(DisplayName = "Guild"),
	Whisper		UMETA(DisplayName = "Whisper"),
	Global		UMETA(DisplayName = "Global"),
	System		UMETA(DisplayName = "System"),
	Battle		UMETA(DisplayName = "Battle")
};

/**
 * Chat tab filter types.
 */
UENUM(BlueprintType)
enum class EROChatTab : uint8
{
	All			UMETA(DisplayName = "All"),
	Party		UMETA(DisplayName = "Party"),
	Guild		UMETA(DisplayName = "Guild"),
	Whisper		UMETA(DisplayName = "Whisper"),
	Battle		UMETA(DisplayName = "Battle Log")
};

/**
 * FROChatMessage
 * Represents a single chat message.
 */
USTRUCT(BlueprintType)
struct FROChatMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	EROChatChannel Channel = EROChatChannel::Local;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FString SenderName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FDateTime Timestamp;
};

/**
 * UROWidget_ChatWindow
 * Chat log with scrollable text, color-coded channels, input field,
 * tab filters, and chat command parsing.
 */
UCLASS()
class RAGNAROKUE_API UROWidget_ChatWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	UROWidget_ChatWindow(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	/** Maximum number of messages to keep in the log. */
	static constexpr int32 MAX_CHAT_HISTORY = 500;

	// --- Message Management ---

	/** Add a new message to the chat log. */
	UFUNCTION(BlueprintCallable, Category = "RO|Chat")
	void AddMessage(const FROChatMessage& Message);

	/** Add a system message. */
	UFUNCTION(BlueprintCallable, Category = "RO|Chat")
	void AddSystemMessage(const FString& Message);

	/** Clear all messages. */
	UFUNCTION(BlueprintCallable, Category = "RO|Chat")
	void ClearMessages();

	/** Set the active channel for outgoing messages. */
	UFUNCTION(BlueprintCallable, Category = "RO|Chat")
	void SetActiveChannel(EROChatChannel Channel);

	/** Get the active outgoing channel. */
	UFUNCTION(BlueprintPure, Category = "RO|Chat")
	EROChatChannel GetActiveChannel() const { return ActiveChannel; }

	/** Switch the filter tab. */
	UFUNCTION(BlueprintCallable, Category = "RO|Chat")
	void SwitchTab(EROChatTab Tab);

	/** Get the color for a given channel. */
	UFUNCTION(BlueprintPure, Category = "RO|Chat")
	static FLinearColor GetChannelColor(EROChatChannel Channel);

	// --- Delegates ---

	/** Broadcast when the user sends a message. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChatMessageSent, EROChatChannel, Channel, const FString&, Message, const FString&, TargetName);

	UPROPERTY(BlueprintAssignable, Category = "RO|Chat")
	FOnChatMessageSent OnChatMessageSent;

protected:
	/** Handle the user pressing Enter in the input field. */
	UFUNCTION()
	void OnInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	/** Parse chat commands from input string. Returns true if a command was found. */
	bool ParseChatCommand(const FString& Input, EROChatChannel& OutChannel, FString& OutMessage, FString& OutTargetName);

	/** Refresh the chat log display based on current tab filter. */
	void RefreshChatLog();

	/** Add a formatted message widget to the scroll box. */
	void AppendMessageToLog(const FROChatMessage& Message);

	/** Check if a message passes the current tab filter. */
	bool PassesFilter(const FROChatMessage& Message) const;

	// --- Tab button handlers ---
	UFUNCTION()
	void OnTabAllClicked();
	UFUNCTION()
	void OnTabPartyClicked();
	UFUNCTION()
	void OnTabGuildClicked();
	UFUNCTION()
	void OnTabWhisperClicked();
	UFUNCTION()
	void OnTabBattleClicked();

	// --- Bound Widgets ---

	UPROPERTY(meta = (BindWidgetOptional))
	UScrollBox* ChatLogScrollBox;

	UPROPERTY(meta = (BindWidgetOptional))
	UEditableTextBox* ChatInputField;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_ChannelIndicator;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_TabAll;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_TabParty;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_TabGuild;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_TabWhisper;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_TabBattle;

private:
	/** All chat messages. */
	TArray<FROChatMessage> MessageHistory;

	/** Current active outgoing channel. */
	EROChatChannel ActiveChannel = EROChatChannel::Local;

	/** Current filter tab. */
	EROChatTab CurrentTab = EROChatTab::All;

	/** Whether we are scroll-locked (manually scrolled up). */
	bool bScrollLocked = false;
};
