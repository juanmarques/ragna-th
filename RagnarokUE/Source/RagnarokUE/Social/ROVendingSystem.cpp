// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROVendingSystem.h"
#include "RagnarokUE/Items/ROInventoryComponent.h"
#include "RagnarokUE/Data/ROConstants.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"

void UROVendingSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UROVendingSystem::Deinitialize()
{
	ActiveShops.Empty();
	Super::Deinitialize();
}

bool UROVendingSystem::OpenShop(int32 PlayerID, const FString& Title, const TArray<FROVendingItem>& Items)
{
	// Player cannot have multiple shops open
	if (HasActiveShop(PlayerID))
	{
		return false;
	}

	// Must have at least one item to sell
	if (Items.Num() == 0)
	{
		return false;
	}

	// Enforce maximum item count
	if (Items.Num() > MaxVendingItems)
	{
		return false;
	}

	// Validate all items have valid prices and amounts
	for (const FROVendingItem& VendItem : Items)
	{
		if (VendItem.Price <= 0 || VendItem.Amount <= 0)
		{
			return false;
		}
		if (!VendItem.Item.IsValid())
		{
			return false;
		}
		// Enforce maximum price cap to prevent INT32_MAX deadlock
		if (VendItem.Price > ROConstants::MaxZeny)
		{
			UE_LOG(LogTemp, Warning, TEXT("Vending price %d exceeds maximum allowed (%d)"),
				VendItem.Price, ROConstants::MaxZeny);
			return false;
		}
	}

	FROVendingShop NewShop;
	NewShop.VendorPlayerID = PlayerID;
	NewShop.ShopTitle = Title;
	NewShop.Items = Items;

	// Set shop location from the player's current world position
	NewShop.ShopLocation = FVector::ZeroVector;
	if (UGameInstance* GI = GetGameInstance())
	{
		UWorld* World = GI->GetWorld();
		if (World && World->GetGameState())
		{
			for (APlayerState* PS : World->GetGameState()->PlayerArray)
			{
				if (PS && PS->GetPlayerId() == PlayerID)
				{
					APawn* Pawn = PS->GetPawn();
					if (Pawn)
					{
						NewShop.ShopLocation = Pawn->GetActorLocation();
					}
					break;
				}
			}
		}
	}

	ActiveShops.Add(PlayerID, NewShop);

	OnShopOpened.Broadcast(PlayerID);

	UE_LOG(LogTemp, Log, TEXT("Vending shop opened by Player %d: '%s' with %d items"),
		PlayerID, *Title, Items.Num());

	return true;
}

void UROVendingSystem::CloseShop(int32 PlayerID)
{
	if (!ActiveShops.Contains(PlayerID))
	{
		return;
	}

	ActiveShops.Remove(PlayerID);
	OnShopClosed.Broadcast(PlayerID);

	UE_LOG(LogTemp, Log, TEXT("Vending shop closed by Player %d"), PlayerID);
}

TArray<FROVendingItem> UROVendingSystem::BrowseShop(int32 BrowserID, int32 VendorID) const
{
	const FROVendingShop* Shop = ActiveShops.Find(VendorID);
	if (!Shop)
	{
		return TArray<FROVendingItem>();
	}

	return Shop->Items;
}

bool UROVendingSystem::BuyFromShop(int32 BuyerID, int32 VendorID, int32 ItemIndex, int32 Amount)
{
	FROVendingShop* Shop = ActiveShops.Find(VendorID);
	if (!Shop)
	{
		return false;
	}

	// Cannot buy from your own shop
	if (BuyerID == VendorID)
	{
		return false;
	}

	if (!Shop->Items.IsValidIndex(ItemIndex))
	{
		return false;
	}

	FROVendingItem& VendItem = Shop->Items[ItemIndex];

	// Check requested amount is available
	if (Amount <= 0 || Amount > VendItem.Amount)
	{
		return false;
	}

	const int64 TotalCost = static_cast<int64>(VendItem.Price) * Amount;

	// Guard against int32 overflow
	if (TotalCost > 1000000000LL)
	{
		return false;
	}

	const int32 TotalCostInt = static_cast<int32>(TotalCost);

	// Look up buyer and vendor pawns for inventory access
	APawn* BuyerPawn = FindPlayerPawnByID(BuyerID);
	APawn* VendorPawn = FindPlayerPawnByID(VendorID);
	if (!BuyerPawn || !VendorPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Vending purchase failed: could not find player pawns."));
		return false;
	}

	UROInventoryComponent* BuyerInv = BuyerPawn->FindComponentByClass<UROInventoryComponent>();
	UROInventoryComponent* VendorInv = VendorPawn->FindComponentByClass<UROInventoryComponent>();
	if (!BuyerInv || !VendorInv)
	{
		UE_LOG(LogTemp, Warning, TEXT("Vending purchase failed: missing inventory component."));
		return false;
	}

	// Verify buyer has enough Zeny
	if (BuyerInv->Zeny < TotalCostInt)
	{
		return false;
	}

	// Verify buyer can hold the items
	if (!BuyerInv->CanAddItem(VendItem.Item.ItemID, Amount))
	{
		return false;
	}

	// Find the item in vendor's inventory by UniqueID (prevent duplication)
	int32 VendorSlot = INDEX_NONE;
	for (int32 i = 0; i < VendorInv->InventorySlots.Num(); ++i)
	{
		if (VendorInv->InventorySlots[i].UniqueID == VendItem.Item.UniqueID)
		{
			VendorSlot = i;
			break;
		}
	}
	if (VendorSlot == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("Vending purchase failed: vendor no longer has the item in inventory."));
		return false;
	}

	// Re-verify buyer can receive BEFORE removing from vendor to avoid item loss
	FROItemInstance PurchasedItem = VendItem.Item;
	PurchasedItem.Amount = Amount;
	if (!BuyerInv->CanAddItem(PurchasedItem.ItemID, Amount))
	{
		UE_LOG(LogTemp, Warning, TEXT("Buyer inventory full - purchase rejected"));
		return false;
	}

	// Now safe to proceed - remove from vendor, then add to buyer
	VendorInv->Internal_RemoveItem(VendorSlot, Amount);
	int32 PlacedSlot = BuyerInv->Internal_PlaceItem(PurchasedItem);
	if (PlacedSlot < 0)
	{
		// Placement failed despite pre-check — return item to vendor to avoid item loss
		UE_LOG(LogTemp, Error, TEXT("Vending purchase CRITICAL: placement failed after pre-check passed. Returning item to vendor."));
		VendorInv->Internal_PlaceItem(PurchasedItem);
		return false;
	}

	// Deduct Zeny from buyer, add to vendor
	BuyerInv->RemoveZeny(TotalCostInt);
	VendorInv->AddZeny(TotalCostInt);

	// Update weights for both parties
	BuyerInv->UpdateWeight();
	VendorInv->UpdateWeight();

	UE_LOG(LogTemp, Log, TEXT("Player %d bought %d x Item %d from Player %d's shop for %d Zeny"),
		BuyerID, Amount, VendItem.Item.ItemID, VendorID, TotalCostInt);

	VendItem.Amount -= Amount;

	// Remove the entry if sold out
	if (VendItem.Amount <= 0)
	{
		Shop->Items.RemoveAt(ItemIndex);
	}

	// If shop is now empty, close it
	if (Shop->Items.Num() == 0)
	{
		CloseShop(VendorID);
	}

	return true;
}

TArray<FROVendingShop> UROVendingSystem::GetActiveShops() const
{
	TArray<FROVendingShop> Result;
	for (const auto& Pair : ActiveShops)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

bool UROVendingSystem::HasActiveShop(int32 PlayerID) const
{
	return ActiveShops.Contains(PlayerID);
}

FROVendingShop UROVendingSystem::GetShopInfo(int32 VendorID) const
{
	const FROVendingShop* Shop = ActiveShops.Find(VendorID);
	if (Shop)
	{
		return *Shop;
	}
	return FROVendingShop();
}

APawn* UROVendingSystem::FindPlayerPawnByID(int32 PlayerID) const
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
