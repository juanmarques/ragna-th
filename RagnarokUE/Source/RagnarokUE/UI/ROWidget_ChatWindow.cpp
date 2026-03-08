// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWidget_ChatWindow.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/RichTextBlock.h"

UROWidget_ChatWindow::UROWidget_ChatWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UROWidget_ChatWindow::NativeConstruct()
{
	Super::NativeConstruct();

	if (ChatInputField)
	{
		ChatInputField->OnTextCommitted.AddDynamic(this, &UROWidget_ChatWindow::OnInputCommitted);
	}

	if (Btn_TabAll) Btn_TabAll->OnClicked.AddDynamic(this, &UROWidget_ChatWindow::OnTabAllClicked);
	if (Btn_TabParty) Btn_TabParty->OnClicked.AddDynamic(this, &UROWidget_ChatWindow::OnTabPartyClicked);
	if (Btn_TabGuild) Btn_TabGuild->OnClicked.AddDynamic(this, &UROWidget_ChatWindow::OnTabGuildClicked);
	if (Btn_TabWhisper) Btn_TabWhisper->OnClicked.AddDynamic(this, &UROWidget_ChatWindow::OnTabWhisperClicked);
	if (Btn_TabBattle) Btn_TabBattle->OnClicked.AddDynamic(this, &UROWidget_ChatWindow::OnTabBattleClicked);
}

void UROWidget_ChatWindow::AddMessage(const FROChatMessage& Message)
{
	MessageHistory.Add(Message);

	// Trim history if exceeding max
	while (MessageHistory.Num() > MAX_CHAT_HISTORY)
	{
		MessageHistory.RemoveAt(0);
	}

	if (PassesFilter(Message))
	{
		AppendMessageToLog(Message);
	}
}

void UROWidget_ChatWindow::AddSystemMessage(const FString& Message)
{
	FROChatMessage SysMsg;
	SysMsg.Channel = EChatChannel::System;
	SysMsg.SenderName = TEXT("System");
	SysMsg.Message = Message;
	SysMsg.Timestamp = FDateTime::Now();

	AddMessage(SysMsg);
}

void UROWidget_ChatWindow::ClearMessages()
{
	MessageHistory.Empty();
	if (ChatLogScrollBox)
	{
		ChatLogScrollBox->ClearChildren();
	}
}

void UROWidget_ChatWindow::SetActiveChannel(EChatChannel Channel)
{
	ActiveChannel = Channel;

	if (Text_ChannelIndicator)
	{
		FString ChannelName;
		switch (Channel)
		{
		case EChatChannel::Local:   ChannelName = TEXT("[Local]"); break;
		case EChatChannel::Party:   ChannelName = TEXT("[Party]"); break;
		case EChatChannel::Guild:   ChannelName = TEXT("[Guild]"); break;
		case EChatChannel::Whisper: ChannelName = TEXT("[Whisper]"); break;
		case EChatChannel::Global:  ChannelName = TEXT("[Global]"); break;
		default:                      ChannelName = TEXT("[Local]"); break;
		}

		Text_ChannelIndicator->SetText(FText::FromString(ChannelName));
		Text_ChannelIndicator->SetColorAndOpacity(FSlateColor(GetChannelColor(Channel)));
	}
}

void UROWidget_ChatWindow::SwitchTab(EROChatTab Tab)
{
	CurrentTab = Tab;
	RefreshChatLog();
}

FLinearColor UROWidget_ChatWindow::GetChannelColor(EChatChannel Channel)
{
	switch (Channel)
	{
	case EChatChannel::Local:   return FLinearColor::White;
	case EChatChannel::Party:   return FLinearColor(0.3f, 0.5f, 1.0f, 1.0f);   // Blue
	case EChatChannel::Guild:   return FLinearColor(0.3f, 1.0f, 0.3f, 1.0f);   // Green
	case EChatChannel::Whisper: return FLinearColor(1.0f, 1.0f, 0.3f, 1.0f);   // Yellow
	case EChatChannel::Global:  return FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);   // Orange
	case EChatChannel::System:  return FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);   // Red
	case EChatChannel::Battle:  return FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);   // Light gray
	default:                      return FLinearColor::White;
	}
}

void UROWidget_ChatWindow::OnInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}

	const FString Input = Text.ToString().TrimStartAndEnd();
	if (Input.IsEmpty())
	{
		return;
	}

	EChatChannel Channel = ActiveChannel;
	FString Message;
	FString TargetName;

	ParseChatCommand(Input, Channel, Message, TargetName);

	if (!Message.IsEmpty())
	{
		OnChatMessageSent.Broadcast(Channel, Message, TargetName);
	}

	// Clear the input field
	if (ChatInputField)
	{
		ChatInputField->SetText(FText::GetEmpty());
	}
}

bool UROWidget_ChatWindow::ParseChatCommand(const FString& Input, EChatChannel& OutChannel, FString& OutMessage, FString& OutTargetName)
{
	OutTargetName.Empty();

	// /p message - party chat
	if (Input.StartsWith(TEXT("/p ")))
	{
		OutChannel = EChatChannel::Party;
		OutMessage = Input.Mid(3);
		return true;
	}

	// /g message - guild chat
	if (Input.StartsWith(TEXT("/g ")))
	{
		OutChannel = EChatChannel::Guild;
		OutMessage = Input.Mid(3);
		return true;
	}

	// /w "name" message - whisper
	if (Input.StartsWith(TEXT("/w ")))
	{
		OutChannel = EChatChannel::Whisper;
		const FString Remainder = Input.Mid(3);

		// Parse "name" format
		if (Remainder.StartsWith(TEXT("\"")))
		{
			int32 EndQuote = INDEX_NONE;
			if (Remainder.FindChar(TEXT('"'), EndQuote) && EndQuote > 0)
			{
				// Find the second quote
				const FString AfterFirstQuote = Remainder.Mid(1);
				int32 SecondQuote = INDEX_NONE;
				if (AfterFirstQuote.FindChar(TEXT('"'), SecondQuote))
				{
					OutTargetName = AfterFirstQuote.Left(SecondQuote);
					OutMessage = AfterFirstQuote.Mid(SecondQuote + 1).TrimStart();
				}
			}
		}
		else
		{
			// /w name message (no quotes, name is first word)
			int32 SpaceIdx = INDEX_NONE;
			if (Remainder.FindChar(TEXT(' '), SpaceIdx))
			{
				OutTargetName = Remainder.Left(SpaceIdx);
				OutMessage = Remainder.Mid(SpaceIdx + 1);
			}
		}
		return true;
	}

	// /s message - shout/global
	if (Input.StartsWith(TEXT("/s ")))
	{
		OutChannel = EChatChannel::Global;
		OutMessage = Input.Mid(3);
		return true;
	}

	// No command prefix - use active channel
	OutMessage = Input;
	return false;
}

void UROWidget_ChatWindow::RefreshChatLog()
{
	if (!ChatLogScrollBox)
	{
		return;
	}

	ChatLogScrollBox->ClearChildren();

	for (const FROChatMessage& Msg : MessageHistory)
	{
		if (PassesFilter(Msg))
		{
			AppendMessageToLog(Msg);
		}
	}
}

void UROWidget_ChatWindow::AppendMessageToLog(const FROChatMessage& Message)
{
	if (!ChatLogScrollBox)
	{
		return;
	}

	// Create a text block for this message with the scroll box as outer
	UTextBlock* MsgText = NewObject<UTextBlock>(ChatLogScrollBox);
	if (!MsgText)
	{
		return;
	}

	FString FormattedMsg;
	switch (Message.Channel)
	{
	case EChatChannel::Whisper:
		FormattedMsg = FString::Printf(TEXT("[From %s]: %s"), *Message.SenderName, *Message.Message);
		break;
	case EChatChannel::System:
		FormattedMsg = FString::Printf(TEXT("[System]: %s"), *Message.Message);
		break;
	case EChatChannel::Battle:
		FormattedMsg = Message.Message;
		break;
	default:
		FormattedMsg = FString::Printf(TEXT("%s: %s"), *Message.SenderName, *Message.Message);
		break;
	}

	MsgText->SetText(FText::FromString(FormattedMsg));
	MsgText->SetColorAndOpacity(FSlateColor(GetChannelColor(Message.Channel)));

	FSlateFontInfo FontInfo = MsgText->GetFont();
	FontInfo.Size = 12;
	MsgText->SetFont(FontInfo);

	ChatLogScrollBox->AddChild(MsgText);

	// Auto-scroll to bottom unless manually scrolled up
	if (!bScrollLocked)
	{
		ChatLogScrollBox->ScrollToEnd();
	}
}

bool UROWidget_ChatWindow::PassesFilter(const FROChatMessage& Message) const
{
	switch (CurrentTab)
	{
	case EROChatTab::All:
		return true;
	case EROChatTab::Party:
		return Message.Channel == EChatChannel::Party || Message.Channel == EChatChannel::System;
	case EROChatTab::Guild:
		return Message.Channel == EChatChannel::Guild || Message.Channel == EChatChannel::System;
	case EROChatTab::Whisper:
		return Message.Channel == EChatChannel::Whisper;
	case EROChatTab::Battle:
		return Message.Channel == EChatChannel::Battle;
	default:
		return true;
	}
}

void UROWidget_ChatWindow::OnTabAllClicked() { SwitchTab(EROChatTab::All); }
void UROWidget_ChatWindow::OnTabPartyClicked() { SwitchTab(EROChatTab::Party); }
void UROWidget_ChatWindow::OnTabGuildClicked() { SwitchTab(EROChatTab::Guild); }
void UROWidget_ChatWindow::OnTabWhisperClicked() { SwitchTab(EROChatTab::Whisper); }
void UROWidget_ChatWindow::OnTabBattleClicked() { SwitchTab(EROChatTab::Battle); }
