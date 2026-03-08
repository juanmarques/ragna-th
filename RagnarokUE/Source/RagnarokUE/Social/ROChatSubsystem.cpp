// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROChatSubsystem.h"
#include "ROPartySubsystem.h"
#include "ROGuildSubsystem.h"
#include "RagnarokUE/Core/ROPlayerState.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
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

					TargetID = ResolvePlayerName(TargetName);
				}
			}
		}
		else
		{
			// Unquoted: first word is the name
			FString TargetName;
			Rest.Split(TEXT(" "), &TargetName, &MessageText);
			TargetID = ResolvePlayerName(TargetName);
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
	UE_LOG(LogTemp, Log, TEXT("[Local] %s: %s"), *ChatMessage.SenderName, *ChatMessage.Message);

	// Deliver to all players within LocalChatRadius of the sender
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		OnChatMessageReceived.Broadcast(ChatMessage);
		return;
	}

	UWorld* World = GI->GetWorld();
	if (!World || !World->GetGameState())
	{
		OnChatMessageReceived.Broadcast(ChatMessage);
		return;
	}

	// Find sender's pawn location
	FVector SenderLocation = FVector::ZeroVector;
	bool bFoundSender = false;
	for (APlayerState* PS : World->GetGameState()->PlayerArray)
	{
		if (PS && PS->GetPlayerId() == ChatMessage.SenderID)
		{
			APawn* SenderPawn = PS->GetPawn();
			if (SenderPawn)
			{
				SenderLocation = SenderPawn->GetActorLocation();
				bFoundSender = true;
			}
			break;
		}
	}

	if (!bFoundSender)
	{
		// Fallback: broadcast to everyone
		OnChatMessageReceived.Broadcast(ChatMessage);
		return;
	}

	// Deliver to nearby players
	const float RadiusSq = LocalChatRadius * LocalChatRadius;
	for (APlayerState* PS : World->GetGameState()->PlayerArray)
	{
		if (!PS)
		{
			continue;
		}
		APawn* Pawn = PS->GetPawn();
		if (Pawn && FVector::DistSquared(Pawn->GetActorLocation(), SenderLocation) <= RadiusSq)
		{
			DeliverMessageToPlayer(PS->GetPlayerId(), ChatMessage);
		}
	}
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
	UE_LOG(LogTemp, Log, TEXT("[%s] %s: %s"),
		ChatMessage.Channel == EChatChannel::Trade ? TEXT("Trade") : TEXT("Global"),
		*ChatMessage.SenderName, *ChatMessage.Message);

	// Deliver to all connected players
	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	if (World && World->GetGameState())
	{
		for (APlayerState* PS : World->GetGameState()->PlayerArray)
		{
			if (PS)
			{
				DeliverMessageToPlayer(PS->GetPlayerId(), ChatMessage);
			}
		}
	}
	else
	{
		OnChatMessageReceived.Broadcast(ChatMessage);
	}
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
	// Look up player name from game state's player array
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return FString::Printf(TEXT("Player_%d"), PlayerID);
	}

	UWorld* World = GI->GetWorld();
	if (!World || !World->GetGameState())
	{
		return FString::Printf(TEXT("Player_%d"), PlayerID);
	}

	for (APlayerState* PS : World->GetGameState()->PlayerArray)
	{
		AROPlayerState* ROPS = Cast<AROPlayerState>(PS);
		if (ROPS && ROPS->GetPlayerId() == PlayerID)
		{
			const FString Name = ROPS->GetCharacterName();
			return Name.IsEmpty() ? PS->GetPlayerName() : Name;
		}
	}

	return FString::Printf(TEXT("Player_%d"), PlayerID);
}

int32 UROChatSubsystem::ResolvePlayerName(const FString& Name) const
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return 0;
	}

	UWorld* World = GI->GetWorld();
	if (!World || !World->GetGameState())
	{
		return 0;
	}

	for (APlayerState* PS : World->GetGameState()->PlayerArray)
	{
		AROPlayerState* ROPS = Cast<AROPlayerState>(PS);
		if (ROPS && ROPS->GetCharacterName().Equals(Name, ESearchCase::IgnoreCase))
		{
			return ROPS->GetPlayerId();
		}
	}

	return 0;
}
