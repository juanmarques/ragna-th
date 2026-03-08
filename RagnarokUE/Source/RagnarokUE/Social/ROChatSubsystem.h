// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROChatSubsystem.generated.h"

/** Chat channel types in Ragnarok Online. */
UENUM(BlueprintType)
enum class EChatChannel : uint8
{
	Local		UMETA(DisplayName = "Local"),
	Party		UMETA(DisplayName = "Party"),
	Guild		UMETA(DisplayName = "Guild"),
	Whisper		UMETA(DisplayName = "Whisper"),
	Global		UMETA(DisplayName = "Global"),
	Trade		UMETA(DisplayName = "Trade"),
	System		UMETA(DisplayName = "System"),
	Battle		UMETA(DisplayName = "Battle")
};

/** A single chat message. */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROChatMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	EChatChannel Channel = EChatChannel::Local;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FString SenderName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	int32 SenderID = 0;
};

/** Delegate for receiving chat messages on the client side. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatMessageReceived, const FROChatMessage&, ChatMessage);

/**
 * UROChatSubsystem
 * Handles all chat routing: Local (radius-based), Party, Guild, Whisper, Global, and Trade channels.
 * Supports chat commands: /p (party), /g (guild), /w "name" (whisper), /s (shout/global).
 */
UCLASS()
class RAGNAROKUE_API UROChatSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Radius for local chat messages (in world units). */
	static constexpr float LocalChatRadius = 500.0f;

	/**
	 * Send a chat message to the appropriate channel.
	 * @param SenderID     The sending player's ID.
	 * @param Channel      Which chat channel to use.
	 * @param Message      The text message.
	 * @param TargetID     For Whisper: the target player ID. Ignored for other channels.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendMessage(int32 SenderID, EChatChannel Channel, const FString& Message, int32 TargetID = 0);

	/**
	 * Parse a raw chat input string for commands and route accordingly.
	 * Supported commands:
	 *   /p <message>         - Party chat
	 *   /g <message>         - Guild chat
	 *   /w "name" <message>  - Whisper to player
	 *   /s <message>         - Global shout
	 * If no command prefix, defaults to Local chat.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void ParseAndSendMessage(int32 SenderID, const FString& RawInput);

	/** Delegate broadcast when a message should be displayed on a client. */
	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatMessageReceived OnChatMessageReceived;

protected:
	/** Route a local chat message to players within radius. */
	void RouteLocalMessage(const FROChatMessage& ChatMessage);

	/** Route a party chat message to all party members. */
	void RoutePartyMessage(const FROChatMessage& ChatMessage);

	/** Route a guild chat message to all online guild members. */
	void RouteGuildMessage(const FROChatMessage& ChatMessage);

	/** Route a whisper to a specific player. */
	void RouteWhisperMessage(const FROChatMessage& ChatMessage, int32 TargetID);

	/** Route a global or trade message to all connected players. */
	void RouteGlobalMessage(const FROChatMessage& ChatMessage);

	/** Deliver a message to a specific player (triggers client RPC or delegate). */
	void DeliverMessageToPlayer(int32 PlayerID, const FROChatMessage& ChatMessage);

	/** Get the display name for a player ID. */
	FString GetPlayerName(int32 PlayerID) const;

	/** Resolve a character name to a player ID. Returns 0 if not found. */
	int32 ResolvePlayerName(const FString& Name) const;
};
