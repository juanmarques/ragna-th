// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROVendingSystem.h"

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
	}

	FROVendingShop NewShop;
	NewShop.VendorPlayerID = PlayerID;
	NewShop.ShopTitle = Title;
	NewShop.Items = Items;
	// TODO: Set ShopLocation from the player's current world position.
	NewShop.ShopLocation = FVector::ZeroVector;

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

	// TODO: Verify buyer has enough Zeny:
	//   - Deduct TotalCost from buyer's Zeny
	//   - Add TotalCost to vendor's Zeny
	//   - Add purchased items to buyer's inventory
	// For now, just update the shop state.

	UE_LOG(LogTemp, Log, TEXT("Player %d bought %d x Item %d from Player %d's shop for %lld Zeny"),
		BuyerID, Amount, VendItem.Item.ItemID, VendorID, TotalCost);

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
