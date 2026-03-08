// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "GameFramework/GameStateBase.h"
#include "ROTradeSystem.generated.h"

/** Represents an active trade session between two players. */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROTradeSession
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	int32 TradeID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	int32 Player1ID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	int32 Player2ID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	TArray<FROItemInstance> Player1Items;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	TArray<FROItemInstance> Player2Items;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	int32 Player1Zeny = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	int32 Player2Zeny = 0;

	/** Whether Player1 has locked their trade offer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	bool bPlayer1Locked = false;

	/** Whether Player2 has locked their trade offer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	bool bPlayer2Locked = false;

	/** Whether Player1 has confirmed the trade (after locking). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	bool bPlayer1Confirmed = false;

	/** Whether Player2 has confirmed the trade (after locking). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	bool bPlayer2Confirmed = false;

	bool IsValid() const { return TradeID > 0; }

	bool IsPlayerInTrade(int32 PlayerID) const
	{
		return PlayerID == Player1ID || PlayerID == Player2ID;
	}

	/** Check if both players have locked and confirmed. */
	bool IsFullyConfirmed() const
	{
		return bPlayer1Locked && bPlayer2Locked && bPlayer1Confirmed && bPlayer2Confirmed;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTradeEvent, int32, TradeID, int32, PlayerID);

/**
 * UROTradeSystem
 * Manages player-to-player item and Zeny trading.
 * Uses a double-confirm system: Lock -> Confirm (both must complete both steps).
 */
UCLASS()
class RAGNAROKUE_API UROTradeSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Maximum number of items each player can offer in a trade. */
	static constexpr int32 MaxTradeItems = 10;

	// ---- Trade Flow ----

	/** Initiate a trade with another player. Returns TradeID, or 0 on failure. */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	int32 InitiateTrade(int32 InitiatorID, int32 TargetID);

	/** Accept an incoming trade request. */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	void AcceptTradeRequest(int32 TradeID, int32 PlayerID);

	/** Add an item from inventory to the trade window. */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	bool AddItemToTrade(int32 TradeID, int32 PlayerID, int32 InventorySlot);

	/** Remove an item from the trade window. */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	bool RemoveItemFromTrade(int32 TradeID, int32 PlayerID, int32 TradeSlot);

	/** Set the Zeny amount offered by a player. */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	bool SetTradeZeny(int32 TradeID, int32 PlayerID, int32 Amount);

	/** Lock the trade offer (cannot modify after locking). */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	bool LockTrade(int32 TradeID, int32 PlayerID);

	/** Confirm the trade (after both players have locked). */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	bool ConfirmTrade(int32 TradeID, int32 PlayerID);

	/** Execute the trade once both players have locked and confirmed. */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	bool ExecuteTrade(int32 TradeID);

	/** Cancel the trade session. Either player may cancel at any time. */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	void CancelTrade(int32 TradeID, int32 PlayerID);

	// ---- Queries ----

	/** Get the trade session info. */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	FROTradeSession GetTradeSession(int32 TradeID) const;

	/** Check if a player is currently in a trade. */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	bool IsInTrade(int32 PlayerID) const;

	/** Get the TradeID for a player. Returns 0 if not trading. */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	int32 GetTradeForPlayer(int32 PlayerID) const;

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category = "Trade")
	FOnTradeEvent OnTradeRequested;

	UPROPERTY(BlueprintAssignable, Category = "Trade")
	FOnTradeEvent OnTradeAccepted;

	UPROPERTY(BlueprintAssignable, Category = "Trade")
	FOnTradeEvent OnTradeLocked;

	UPROPERTY(BlueprintAssignable, Category = "Trade")
	FOnTradeEvent OnTradeConfirmed;

	UPROPERTY(BlueprintAssignable, Category = "Trade")
	FOnTradeEvent OnTradeCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Trade")
	FOnTradeEvent OnTradeCancelled;

protected:
	UPROPERTY()
	TMap<int32, FROTradeSession> ActiveTrades;

	/** Pending trade requests: TargetPlayerID -> TradeID. */
	TMap<int32, int32> PendingRequests;

	/** Reverse lookup: PlayerID -> TradeID. */
	TMap<int32, int32> PlayerTradeMap;

	int32 NextTradeID = 1;

	/** Find a player's pawn by their PlayerState ID. Returns nullptr if not found. */
	APawn* FindPlayerPawnByID(int32 PlayerID) const;
};
