// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROChatSubsystem.h"
#include "ROPartySubsystem.h"
#include "ROGuildSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

void UROChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UROChatSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UROChatSubsystem::SendMessage(int32 SenderID, EChatChannel Channel, const FString& Message, int32 TargetID)
{
	if (Message.IsEmpty())
	{
		return;
	}

	FROChatMessage ChatMessage;
	ChatMessage.Channel = Channel;
	ChatMessage.SenderID = SenderID;
	ChatMessage.SenderName = GetPlayerName(SenderID);
	ChatMessage.Message = Message;
	ChatMessage.Timestamp = FDateTime::UtcNow();

	switch (Channel)
	{
	case EChatChannel::Local:
		RouteLocalMessage(ChatMessage);
		break;

	case EChatChannel::Party:
		RoutePartyMessage(ChatMessage);
		break;

	case EChatChannel::Guild:
		RouteGuildMessage(ChatMessage);
		break;

	case EChatChannel::Whisper:
		RouteWhisperMessage(ChatMessage, TargetID);
		break;

	case EChatChannel::Global:
	case EChatChannel::Trade:
		RouteGlobalMessage(ChatMessage);
		break;
	}
}

void UROChatSubsystem::ParseAndSendMessage(int32 SenderID, const FString& RawInput)
{
	if (RawInput.IsEmpty())
	{
		return;
	}

	// Party chat: /p <message>
	if (RawInput.StartsWith(TEXT("/p ")))
	{
		const FString MessageText = RawInput.Mid(3);
		SendMessage(SenderID, EChatChannel::Party, MessageText);
		return;
	}

	// Guild chat: /g <message>
	if (RawInput.StartsWith(TEXT("/g ")))
	{
		const FString MessageText = RawInput.Mid(3);
		SendMessage(SenderID, EChatChannel::Guild, MessageText);
		return;
	}

	// Whisper: /w "name" <message>
	if (RawInput.StartsWith(TEXT("/w ")))
	{
		const FString Rest = RawInput.Mid(3);
		// Expect format: "name" message  or  name message
		int32 TargetID = 0;
		FString MessageText;

		if (Rest.StartsWith(TEXT("\"")))
		{
			// Quoted name
			int32 CloseQuoteIndex = INDEX_NONE;
			Rest.FindChar(TEXT('"'), CloseQuoteIndex);
			if (CloseQuoteIndex != INDEX_NONE)
			{
				// Find the second quote
				const FString AfterFirstQuote = Rest.Mid(1);
				int32 SecondQuote = INDEX_NONE;
				AfterFirstQuote.FindChar(TEXT('"'), SecondQuote);
				if (SecondQuote != INDEX_NONE)
				{
					const FString TargetName = AfterFirstQuote.Left(SecondQuote);
					MessageText = AfterFirstQuote.Mid(SecondQuote + 1).TrimStart();

					// TODO: Resolve TargetName to TargetID via player lookup
					// For now, try parsing as int for testing
					TargetID = FCString::Atoi(*TargetName);
				}
			}
		}
		else
		{
			// Unquoted: first word is the name
			FString TargetName;
			Rest.Split(TEXT(" "), &TargetName, &MessageText);
			// TODO: Resolve TargetName to TargetID
			TargetID = FCString::Atoi(*TargetName);
		}

		if (TargetID > 0 && !MessageText.IsEmpty())
		{
			SendMessage(SenderID, EChatChannel::Whisper, MessageText, TargetID);
		}
		return;
	}

	// Global shout: /s <message>
	if (RawInput.StartsWith(TEXT("/s ")))
	{
		const FString MessageText = RawInput.Mid(3);
		SendMessage(SenderID, EChatChannel::Global, MessageText);
		return;
	}

	// Default: Local chat
	SendMessage(SenderID, EChatChannel::Local, RawInput);
}

void UROChatSubsystem::RouteLocalMessage(const FROChatMessage& ChatMessage)
{
	// In a full implementation, iterate all player controllers/pawns in the world
	// and deliver to those within LocalChatRadius of the sender's position.
	// For now, broadcast via delegate for any local listeners.

	UE_LOG(LogTemp, Log, TEXT("[Local] %s: %s"), *ChatMessage.SenderName, *ChatMessage.Message);

	// TODO: Get sender's world position, iterate nearby players within LocalChatRadius
	// and call DeliverMessageToPlayer for each.
	OnChatMessageReceived.Broadcast(ChatMessage);
}

void UROChatSubsystem::RoutePartyMessage(const FROChatMessage& ChatMessage)
{
	UROPartySubsystem* PartySubsystem = GetGameInstance()->GetSubsystem<UROPartySubsystem>();
	if (!PartySubsystem)
	{
		return;
	}

	const int32 PartyID = PartySubsystem->GetPartyForPlayer(ChatMessage.SenderID);
	if (PartyID == 0)
	{
		return;
	}

	const FROPartyInfo PartyInfo = PartySubsystem->GetPartyInfo(PartyID);
	for (int32 MemberID : PartyInfo.MemberPlayerIDs)
	{
		DeliverMessageToPlayer(MemberID, ChatMessage);
	}
}

void UROChatSubsystem::RouteGuildMessage(const FROChatMessage& ChatMessage)
{
	UROGuildSubsystem* GuildSubsystem = GetGameInstance()->GetSubsystem<UROGuildSubsystem>();
	if (!GuildSubsystem)
	{
		return;
	}

	const int32 GuildID = GuildSubsystem->GetGuildForPlayer(ChatMessage.SenderID);
	if (GuildID == 0)
	{
		return;
	}

	const FROGuildInfo GuildInfo = GuildSubsystem->GetGuildInfo(GuildID);
	for (const FROGuildMember& Member : GuildInfo.Members)
	{
		if (Member.bOnline)
		{
			DeliverMessageToPlayer(Member.PlayerID, ChatMessage);
		}
	}
}

void UROChatSubsystem::RouteWhisperMessage(const FROChatMessage& ChatMessage, int32 TargetID)
{
	if (TargetID <= 0)
	{
		return;
	}

	// Deliver to the target
	DeliverMessageToPlayer(TargetID, ChatMessage);

	// Also deliver a copy back to the sender so they see their own whisper
	DeliverMessageToPlayer(ChatMessage.SenderID, ChatMessage);
}

void UROChatSubsystem::RouteGlobalMessage(const FROChatMessage& ChatMessage)
{
	// In a full implementation, iterate all connected players and deliver.
	// For now, broadcast via delegate.
	UE_LOG(LogTemp, Log, TEXT("[%s] %s: %s"),
		ChatMessage.Channel == EChatChannel::Trade ? TEXT("Trade") : TEXT("Global"),
		*ChatMessage.SenderName, *ChatMessage.Message);

	OnChatMessageReceived.Broadcast(ChatMessage);
}

void UROChatSubsystem::DeliverMessageToPlayer(int32 PlayerID, const FROChatMessage& ChatMessage)
{
	// In a networked game, this would find the player's controller and invoke a Client RPC.
	// Example: AROPlayerController->ClientReceiveChatMessage(ChatMessage);
	// For now, log and broadcast the delegate.

	UE_LOG(LogTemp, Verbose, TEXT("Chat -> Player %d [%d]: %s"),
		PlayerID, static_cast<uint8>(ChatMessage.Channel), *ChatMessage.Message);

	// The delegate is broadcast globally; UI widgets filter by relevant player.
	OnChatMessageReceived.Broadcast(ChatMessage);
}

FString UROChatSubsystem::GetPlayerName(int32 PlayerID) const
{
	// TODO: Look up player name from a player registry or game state.
	// Placeholder: return the ID as a string.
	return FString::Printf(TEXT("Player_%d"), PlayerID);
}
