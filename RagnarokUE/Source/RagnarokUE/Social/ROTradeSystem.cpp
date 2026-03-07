// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROTradeSystem.h"

void UROTradeSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	NextTradeID = 1;
}

void UROTradeSystem::Deinitialize()
{
	ActiveTrades.Empty();
	PendingRequests.Empty();
	PlayerTradeMap.Empty();
	Super::Deinitialize();
}

int32 UROTradeSystem::InitiateTrade(int32 InitiatorID, int32 TargetID)
{
	// Neither player should already be trading
	if (IsInTrade(InitiatorID) || IsInTrade(TargetID))
	{
		return 0;
	}

	// Cannot trade with yourself
	if (InitiatorID == TargetID)
	{
		return 0;
	}

	FROTradeSession NewTrade;
	NewTrade.TradeID = NextTradeID++;
	NewTrade.Player1ID = InitiatorID;
	NewTrade.Player2ID = TargetID;

	ActiveTrades.Add(NewTrade.TradeID, NewTrade);
	PendingRequests.Add(TargetID, NewTrade.TradeID);

	OnTradeRequested.Broadcast(NewTrade.TradeID, TargetID);
	return NewTrade.TradeID;
}

void UROTradeSystem::AcceptTradeRequest(int32 TradeID, int32 PlayerID)
{
	const int32* PendingTradeID = PendingRequests.Find(PlayerID);
	if (!PendingTradeID || *PendingTradeID != TradeID)
	{
		return;
	}

	FROTradeSession* Trade = ActiveTrades.Find(TradeID);
	if (!Trade)
	{
		PendingRequests.Remove(PlayerID);
		return;
	}

	// Register both players as actively trading
	PlayerTradeMap.Add(Trade->Player1ID, TradeID);
	PlayerTradeMap.Add(Trade->Player2ID, TradeID);
	PendingRequests.Remove(PlayerID);

	OnTradeAccepted.Broadcast(TradeID, PlayerID);
}

bool UROTradeSystem::AddItemToTrade(int32 TradeID, int32 PlayerID, int32 InventorySlot)
{
	FROTradeSession* Trade = ActiveTrades.Find(TradeID);
	if (!Trade || !Trade->IsPlayerInTrade(PlayerID))
	{
		return false;
	}

	// Cannot modify after locking
	if (PlayerID == Trade->Player1ID && Trade->bPlayer1Locked)
	{
		return false;
	}
	if (PlayerID == Trade->Player2ID && Trade->bPlayer2Locked)
	{
		return false;
	}

	TArray<FROItemInstance>& PlayerItems = (PlayerID == Trade->Player1ID) ?
		Trade->Player1Items : Trade->Player2Items;

	if (PlayerItems.Num() >= MaxTradeItems)
	{
		return false;
	}

	// TODO: Look up the actual item from the player's inventory by InventorySlot.
	// For now, create a placeholder item instance.
	FROItemInstance ItemToAdd;
	ItemToAdd.ItemID = InventorySlot; // Placeholder: use slot as item ID
	ItemToAdd.Amount = 1;
	PlayerItems.Add(ItemToAdd);

	// Reset confirmations since the offer changed
	Trade->bPlayer1Confirmed = false;
	Trade->bPlayer2Confirmed = false;

	return true;
}

bool UROTradeSystem::RemoveItemFromTrade(int32 TradeID, int32 PlayerID, int32 TradeSlot)
{
	FROTradeSession* Trade = ActiveTrades.Find(TradeID);
	if (!Trade || !Trade->IsPlayerInTrade(PlayerID))
	{
		return false;
	}

	// Cannot modify after locking
	if (PlayerID == Trade->Player1ID && Trade->bPlayer1Locked)
	{
		return false;
	}
	if (PlayerID == Trade->Player2ID && Trade->bPlayer2Locked)
	{
		return false;
	}

	TArray<FROItemInstance>& PlayerItems = (PlayerID == Trade->Player1ID) ?
		Trade->Player1Items : Trade->Player2Items;

	if (!PlayerItems.IsValidIndex(TradeSlot))
	{
		return false;
	}

	PlayerItems.RemoveAt(TradeSlot);

	// Reset confirmations since the offer changed
	Trade->bPlayer1Confirmed = false;
	Trade->bPlayer2Confirmed = false;

	return true;
}

bool UROTradeSystem::SetTradeZeny(int32 TradeID, int32 PlayerID, int32 Amount)
{
	FROTradeSession* Trade = ActiveTrades.Find(TradeID);
	if (!Trade || !Trade->IsPlayerInTrade(PlayerID))
	{
		return false;
	}

	if (Amount < 0)
	{
		return false;
	}

	// Cannot modify after locking
	if (PlayerID == Trade->Player1ID && Trade->bPlayer1Locked)
	{
		return false;
	}
	if (PlayerID == Trade->Player2ID && Trade->bPlayer2Locked)
	{
		return false;
	}

	if (PlayerID == Trade->Player1ID)
	{
		Trade->Player1Zeny = Amount;
	}
	else
	{
		Trade->Player2Zeny = Amount;
	}

	// Reset confirmations since the offer changed
	Trade->bPlayer1Confirmed = false;
	Trade->bPlayer2Confirmed = false;

	return true;
}

bool UROTradeSystem::LockTrade(int32 TradeID, int32 PlayerID)
{
	FROTradeSession* Trade = ActiveTrades.Find(TradeID);
	if (!Trade || !Trade->IsPlayerInTrade(PlayerID))
	{
		return false;
	}

	if (PlayerID == Trade->Player1ID)
	{
		Trade->bPlayer1Locked = true;
	}
	else
	{
		Trade->bPlayer2Locked = true;
	}

	OnTradeLocked.Broadcast(TradeID, PlayerID);
	return true;
}

bool UROTradeSystem::ConfirmTrade(int32 TradeID, int32 PlayerID)
{
	FROTradeSession* Trade = ActiveTrades.Find(TradeID);
	if (!Trade || !Trade->IsPlayerInTrade(PlayerID))
	{
		return false;
	}

	// Both players must have locked before confirming
	if (!Trade->bPlayer1Locked || !Trade->bPlayer2Locked)
	{
		return false;
	}

	if (PlayerID == Trade->Player1ID)
	{
		Trade->bPlayer1Confirmed = true;
	}
	else
	{
		Trade->bPlayer2Confirmed = true;
	}

	OnTradeConfirmed.Broadcast(TradeID, PlayerID);

	// If both confirmed, execute the trade
	if (Trade->IsFullyConfirmed())
	{
		ExecuteTrade(TradeID);
	}

	return true;
}

bool UROTradeSystem::ExecuteTrade(int32 TradeID)
{
	FROTradeSession* Trade = ActiveTrades.Find(TradeID);
	if (!Trade)
	{
		return false;
	}

	if (!Trade->IsFullyConfirmed())
	{
		return false;
	}

	// TODO: Actually swap items between player inventories.
	// 1. Remove Player1Items from Player1's inventory, add to Player2's inventory.
	// 2. Remove Player2Items from Player2's inventory, add to Player1's inventory.
	// 3. Transfer Zeny: Player1 loses Player1Zeny, gains Player2Zeny (and vice versa).
	// 4. Verify both players have sufficient inventory space and Zeny before executing.

	UE_LOG(LogTemp, Log, TEXT("Trade %d executed: Player %d <-> Player %d (%d items <-> %d items, %d zeny <-> %d zeny)"),
		TradeID, Trade->Player1ID, Trade->Player2ID,
		Trade->Player1Items.Num(), Trade->Player2Items.Num(),
		Trade->Player1Zeny, Trade->Player2Zeny);

	const int32 P1 = Trade->Player1ID;
	const int32 P2 = Trade->Player2ID;

	// Clean up
	PlayerTradeMap.Remove(P1);
	PlayerTradeMap.Remove(P2);
	ActiveTrades.Remove(TradeID);

	OnTradeCompleted.Broadcast(TradeID, P1);
	OnTradeCompleted.Broadcast(TradeID, P2);

	return true;
}

void UROTradeSystem::CancelTrade(int32 TradeID, int32 PlayerID)
{
	FROTradeSession* Trade = ActiveTrades.Find(TradeID);
	if (!Trade)
	{
		// May also be a pending request
		PendingRequests.Remove(PlayerID);
		return;
	}

	const int32 P1 = Trade->Player1ID;
	const int32 P2 = Trade->Player2ID;

	// TODO: Return any reserved items back to their respective inventories.

	PlayerTradeMap.Remove(P1);
	PlayerTradeMap.Remove(P2);
	PendingRequests.Remove(P1);
	PendingRequests.Remove(P2);
	ActiveTrades.Remove(TradeID);

	OnTradeCancelled.Broadcast(TradeID, PlayerID);
}

FROTradeSession UROTradeSystem::GetTradeSession(int32 TradeID) const
{
	const FROTradeSession* Trade = ActiveTrades.Find(TradeID);
	if (Trade)
	{
		return *Trade;
	}
	return FROTradeSession();
}

bool UROTradeSystem::IsInTrade(int32 PlayerID) const
{
	return PlayerTradeMap.Contains(PlayerID);
}

int32 UROTradeSystem::GetTradeForPlayer(int32 PlayerID) const
{
	const int32* TradeID = PlayerTradeMap.Find(PlayerID);
	return TradeID ? *TradeID : 0;
}
