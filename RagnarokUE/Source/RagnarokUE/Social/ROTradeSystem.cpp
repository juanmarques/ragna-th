// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROTradeSystem.h"
#include "RagnarokUE/Items/ROInventoryComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"

void UROTradeSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	NextTradeID = 1;

	// Set up periodic timer to clean up expired trade requests
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(CleanupTimerHandle, this,
			&UROTradeSystem::CleanupExpiredRequests, 5.0f, true);
	}
}

void UROTradeSystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CleanupTimerHandle);
	}
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

	// Prevent multiple pending trade requests from the same initiator
	if (PendingRequests.Contains(InitiatorID))
	{
		return 0;
	}

	// Check initiator doesn't already have a pending outgoing request
	for (const auto& Pair : PendingRequests)
	{
		const FROTradeSession* ExistingTrade = ActiveTrades.Find(Pair.Value);
		if (ExistingTrade && ExistingTrade->Player1ID == InitiatorID)
		{
			return 0;
		}
	}

	FROTradeSession NewTrade;
	NewTrade.TradeID = NextTradeID++;
	NewTrade.Player1ID = InitiatorID;
	NewTrade.Player2ID = TargetID;
	NewTrade.RequestTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

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

	// Look up the actual item from the player's inventory
	APawn* PlayerPawn = FindPlayerPawnByID(PlayerID);
	if (!PlayerPawn)
	{
		return false;
	}

	UROInventoryComponent* Inventory = PlayerPawn->FindComponentByClass<UROInventoryComponent>();
	if (!Inventory)
	{
		return false;
	}

	FROItemInstance ItemToAdd = Inventory->GetItemAtSlot(InventorySlot);
	if (!ItemToAdd.IsValid())
	{
		return false;
	}

	// Check the item isn't already offered in the trade window
	for (const FROItemInstance& Existing : PlayerItems)
	{
		if (Existing.UniqueID == ItemToAdd.UniqueID)
		{
			return false; // Already in trade
		}
	}

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

	// Verify the player actually has this much Zeny (prevent scam/griefing)
	APawn* PlayerPawn = FindPlayerPawnByID(PlayerID);
	if (PlayerPawn)
	{
		UROInventoryComponent* Inventory = PlayerPawn->FindComponentByClass<UROInventoryComponent>();
		if (Inventory && Inventory->Zeny < Amount)
		{
			return false;
		}
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
		// Snapshot current trade items and Zeny at lock time for tamper detection
		Trade->Player1LockedItems = Trade->Player1Items;
		Trade->Player1LockedZeny = Trade->Player1Zeny;
	}
	else
	{
		Trade->bPlayer2Locked = true;
		// Snapshot current trade items and Zeny at lock time for tamper detection
		Trade->Player2LockedItems = Trade->Player2Items;
		Trade->Player2LockedZeny = Trade->Player2Zeny;
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

	// Verify trade items and Zeny haven't changed since lock (tamper detection)
	if (Trade->Player1Items != Trade->Player1LockedItems ||
		Trade->Player1Zeny != Trade->Player1LockedZeny)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trade %d aborted: Player1 trade contents changed after lock."), TradeID);
		CancelTradeWithReason(TradeID, TEXT("Trade items changed after lock"));
		return false;
	}
	if (Trade->Player2Items != Trade->Player2LockedItems ||
		Trade->Player2Zeny != Trade->Player2LockedZeny)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trade %d aborted: Player2 trade contents changed after lock."), TradeID);
		CancelTradeWithReason(TradeID, TEXT("Trade items changed after lock"));
		return false;
	}

	// Look up both players' inventory components
	APawn* Pawn1 = FindPlayerPawnByID(Trade->Player1ID);
	APawn* Pawn2 = FindPlayerPawnByID(Trade->Player2ID);
	if (!Pawn1 || !Pawn2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trade %d failed: could not find both player pawns."), TradeID);
		return false;
	}

	UROInventoryComponent* Inv1 = Pawn1->FindComponentByClass<UROInventoryComponent>();
	UROInventoryComponent* Inv2 = Pawn2->FindComponentByClass<UROInventoryComponent>();
	if (!Inv1 || !Inv2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trade %d failed: missing inventory component."), TradeID);
		return false;
	}

	// Pre-validate: both players must have sufficient Zeny
	if (Inv1->Zeny < Trade->Player1Zeny || Inv2->Zeny < Trade->Player2Zeny)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trade %d failed: insufficient Zeny."), TradeID);
		return false;
	}

	// Pre-validate: both players must have space for incoming items
	// (rough check - each incoming item needs at least 1 slot)
	for (const FROItemInstance& Item : Trade->Player2Items)
	{
		if (!Inv1->CanAddItem(Item.ItemID, Item.Amount))
		{
			UE_LOG(LogTemp, Warning, TEXT("Trade %d failed: Player1 has no space for incoming items."), TradeID);
			return false;
		}
	}
	for (const FROItemInstance& Item : Trade->Player1Items)
	{
		if (!Inv2->CanAddItem(Item.ItemID, Item.Amount))
		{
			UE_LOG(LogTemp, Warning, TEXT("Trade %d failed: Player2 has no space for incoming items."), TradeID);
			return false;
		}
	}

	// Re-validate: verify all traded items still exist in each player's inventory
	// by UniqueID (prevents item duplication if items were moved/dropped after locking)
	for (const FROItemInstance& Item : Trade->Player1Items)
	{
		bool bFound = false;
		for (int32 i = 0; i < Inv1->InventorySlots.Num(); ++i)
		{
			if (Inv1->InventorySlots[i].UniqueID == Item.UniqueID)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			UE_LOG(LogTemp, Warning, TEXT("Trade %d failed: Player1 no longer has item %s."), TradeID, *Item.UniqueID.ToString());
			return false;
		}
	}
	for (const FROItemInstance& Item : Trade->Player2Items)
	{
		bool bFound = false;
		for (int32 i = 0; i < Inv2->InventorySlots.Num(); ++i)
		{
			if (Inv2->InventorySlots[i].UniqueID == Item.UniqueID)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			UE_LOG(LogTemp, Warning, TEXT("Trade %d failed: Player2 no longer has item %s."), TradeID, *Item.UniqueID.ToString());
			return false;
		}
	}

	// Execute: Remove items from Player1 by UniqueID, give to Player2
	for (const FROItemInstance& Item : Trade->Player1Items)
	{
		for (int32 i = 0; i < Inv1->InventorySlots.Num(); ++i)
		{
			if (Inv1->InventorySlots[i].UniqueID == Item.UniqueID)
			{
				Inv1->Internal_RemoveItem(i, Item.Amount);
				break;
			}
		}
	}
	for (const FROItemInstance& Item : Trade->Player1Items)
	{
		int32 PlacedSlot = Inv2->Internal_PlaceItem(Item);
		if (PlacedSlot < 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Trade %d CRITICAL: Failed to place Player1 item %d into Player2 inventory. Item lost!"),
				TradeID, Item.ItemID);
		}
	}

	// Execute: Remove items from Player2 by UniqueID, give to Player1
	for (const FROItemInstance& Item : Trade->Player2Items)
	{
		for (int32 i = 0; i < Inv2->InventorySlots.Num(); ++i)
		{
			if (Inv2->InventorySlots[i].UniqueID == Item.UniqueID)
			{
				Inv2->Internal_RemoveItem(i, Item.Amount);
				break;
			}
		}
	}
	for (const FROItemInstance& Item : Trade->Player2Items)
	{
		int32 PlacedSlot = Inv1->Internal_PlaceItem(Item);
		if (PlacedSlot < 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Trade %d CRITICAL: Failed to place Player2 item %d into Player1 inventory. Item lost!"),
				TradeID, Item.ItemID);
		}
	}

	// Transfer Zeny
	if (Trade->Player1Zeny > 0)
	{
		Inv1->RemoveZeny(Trade->Player1Zeny);
		Inv2->AddZeny(Trade->Player1Zeny);
	}
	if (Trade->Player2Zeny > 0)
	{
		Inv2->RemoveZeny(Trade->Player2Zeny);
		Inv1->AddZeny(Trade->Player2Zeny);
	}

	// Update weights
	Inv1->UpdateWeight();
	Inv2->UpdateWeight();

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

	// Items are not reserved from inventory during trade - they remain in
	// the player's inventory until ExecuteTrade. No items to return.

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

void UROTradeSystem::CancelTradeWithReason(int32 TradeID, const FString& Reason)
{
	FROTradeSession* Trade = ActiveTrades.Find(TradeID);
	if (!Trade)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Trade %d cancelled: %s"), TradeID, *Reason);

	const int32 P1 = Trade->Player1ID;
	const int32 P2 = Trade->Player2ID;

	PlayerTradeMap.Remove(P1);
	PlayerTradeMap.Remove(P2);
	PendingRequests.Remove(P1);
	PendingRequests.Remove(P2);
	ActiveTrades.Remove(TradeID);

	OnTradeCancelled.Broadcast(TradeID, 0);
}

void UROTradeSystem::CleanupExpiredRequests()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float CurrentTime = World->GetTimeSeconds();
	TArray<int32> ExpiredTrades;

	for (const auto& Pair : PendingRequests)
	{
		const FROTradeSession* Session = ActiveTrades.Find(Pair.Value);
		if (Session && CurrentTime - Session->RequestTime > TradeRequestTimeout)
		{
			ExpiredTrades.Add(Pair.Value);
		}
	}

	for (int32 TradeID : ExpiredTrades)
	{
		CancelTradeWithReason(TradeID, TEXT("Trade request timed out"));
	}
}

APawn* UROTradeSystem::FindPlayerPawnByID(int32 PlayerID) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AGameStateBase* GS = World->GetGameState();
	if (!GS)
	{
		return nullptr;
	}

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (PS && PS->GetPlayerId() == PlayerID)
		{
			return PS->GetPawn();
		}
	}
	return nullptr;
}
