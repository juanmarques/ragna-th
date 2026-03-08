// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROServiceNPC_Shop.h"
#include "RagnarokUE/Character/ROCharacterBase.h"
#include "RagnarokUE/Items/ROInventoryComponent.h"
#include "RagnarokUE/Skills/ROSkillTreeComponent.h"

AROServiceNPC_Shop::AROServiceNPC_Shop()
{
	bIsShop = true;
	ShopName = FText::FromString(TEXT("Shop"));
	CurrentShopUser = nullptr;
}

void AROServiceNPC_Shop::OnInteract_Implementation(AROCharacterBase* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	Super::OnInteract_Implementation(Interactor);

	CurrentShopUser = Interactor;

	UE_LOG(LogTemp, Log, TEXT("Shop NPC %s: Player opened shop '%s' with %d items."),
		*NPCName.ToString(), *ShopName.ToString(), ShopInventory.Num());
}

void AROServiceNPC_Shop::ServerBuyItem_Implementation(int32 ShopIndex, int32 Amount)
{
	if (!IsValid(CurrentShopUser))
	{
		UE_LOG(LogTemp, Warning, TEXT("Shop NPC: No active shop user for purchase."));
		CurrentShopUser = nullptr;
		return;
	}

	if (!ShopInventory.IsValidIndex(ShopIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Shop NPC: Invalid shop index %d."), ShopIndex);
		return;
	}

	UROInventoryComponent* Inventory = CurrentShopUser->FindComponentByClass<UROInventoryComponent>();
	if (!Inventory)
	{
		return;
	}

	const FROShopItem& ShopItem = ShopInventory[ShopIndex];

	// Calculate total cost with Discount modifier
	const int32 PricePerUnit = GetBuyPrice(ShopIndex, CurrentShopUser);
	const int64 TotalCost = static_cast<int64>(PricePerUnit) * Amount;

	// Reject purchases that exceed int32 Zeny range
	if (TotalCost > static_cast<int64>(MAX_int32))
	{
		UE_LOG(LogTemp, Warning, TEXT("Shop NPC: Purchase cost exceeds max Zeny."));
		return;
	}

	// Validate Zeny
	if (Inventory->Zeny < TotalCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("Shop NPC: Player cannot afford %lld Zeny (has %d)."),
			TotalCost, Inventory->Zeny);
		return;
	}

	// Check if inventory can hold the items
	if (!Inventory->CanAddItem(ShopItem.ItemID, Amount))
	{
		UE_LOG(LogTemp, Warning, TEXT("Shop NPC: Player inventory cannot hold %d of item %d."),
			Amount, ShopItem.ItemID);
		return;
	}

	// Deduct Zeny
	if (!Inventory->RemoveZeny(static_cast<int32>(FMath::Min(TotalCost, static_cast<int64>(MAX_int32)))))
	{
		UE_LOG(LogTemp, Warning, TEXT("Shop NPC: Failed to deduct Zeny."));
		return;
	}

	// Add items to inventory
	int32 AddedSlot = Inventory->Internal_AddItem(ShopItem.ItemID, Amount);
	if (AddedSlot < 0)
	{
		// Refund Zeny if item addition failed
		Inventory->AddZeny(static_cast<int32>(FMath::Min(TotalCost, static_cast<int64>(MAX_int32))));
		UE_LOG(LogTemp, Warning, TEXT("Shop NPC: Failed to add item to inventory. Zeny refunded."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Shop NPC: Player bought %d x item %d for %lld Zeny."),
		Amount, ShopItem.ItemID, TotalCost);
}

bool AROServiceNPC_Shop::ServerBuyItem_Validate(int32 ShopIndex, int32 Amount)
{
	return Amount > 0 && Amount <= 30000 && ShopIndex >= 0 && ShopIndex < ShopInventory.Num();
}

void AROServiceNPC_Shop::ServerSellItem_Implementation(int32 InventorySlot, int32 Amount)
{
	if (!IsValid(CurrentShopUser))
	{
		UE_LOG(LogTemp, Warning, TEXT("Shop NPC: No active shop user for sale."));
		CurrentShopUser = nullptr;
		return;
	}

	UROInventoryComponent* Inventory = CurrentShopUser->FindComponentByClass<UROInventoryComponent>();
	if (!Inventory)
	{
		return;
	}

	// Get the item to sell
	FROItemInstance ItemToSell = Inventory->GetItemAtSlot(InventorySlot);
	if (!ItemToSell.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Shop NPC: Invalid inventory slot %d for sale."), InventorySlot);
		return;
	}

	const int32 SellAmount = FMath::Min(Amount, ItemToSell.Amount);
	if (SellAmount <= 0)
	{
		return;
	}

	// Calculate sell price with Overcharge modifier (use int64 to avoid overflow)
	const int32 PricePerUnit = GetSellPrice(ItemToSell.ItemID, CurrentShopUser);
	const int32 TotalSellPrice = static_cast<int32>(FMath::Min(
		static_cast<int64>(PricePerUnit) * SellAmount, static_cast<int64>(MAX_int32)));

	// Remove items from inventory
	if (!Inventory->Internal_RemoveItem(InventorySlot, SellAmount))
	{
		UE_LOG(LogTemp, Warning, TEXT("Shop NPC: Failed to remove item from inventory."));
		return;
	}

	// Add Zeny
	Inventory->AddZeny(TotalSellPrice);

	UE_LOG(LogTemp, Log, TEXT("Shop NPC: Player sold %d x item %d for %d Zeny."),
		SellAmount, ItemToSell.ItemID, TotalSellPrice);
}

bool AROServiceNPC_Shop::ServerSellItem_Validate(int32 InventorySlot, int32 Amount)
{
	// MaxSlots is 100 in ROInventoryComponent - reject out-of-range indices
	return Amount > 0 && Amount <= 30000 && InventorySlot >= 0 && InventorySlot < 100;
}

int32 AROServiceNPC_Shop::GetBuyPrice(int32 ShopIndex, AROCharacterBase* Buyer) const
{
	const int32 BaseItemPrice = GetBasePrice(ShopIndex);
	if (BaseItemPrice <= 0)
	{
		return 0;
	}

	if (!Buyer)
	{
		return BaseItemPrice;
	}

	// Apply Discount skill modifier
	const int32 DiscountLevel = GetDiscountSkillLevel(Buyer);
	if (DiscountLevel > 0)
	{
		const float DiscountPercent = CalculateDiscountPercent(DiscountLevel);
		const int32 DiscountedPrice = FMath::Max(1, FMath::RoundToInt32(BaseItemPrice * (1.0f - DiscountPercent)));
		return DiscountedPrice;
	}

	return BaseItemPrice;
}

int32 AROServiceNPC_Shop::GetSellPrice(int32 ItemID, AROCharacterBase* Seller) const
{
	// Base sell price is half the buy price
	const int32 BaseSellPrice = GetDefaultItemSellPrice(ItemID);
	if (BaseSellPrice <= 0)
	{
		return 0;
	}

	if (!Seller)
	{
		return BaseSellPrice;
	}

	// Apply Overcharge skill modifier
	const int32 OverchargeLevel = GetOverchargeSkillLevel(Seller);
	if (OverchargeLevel > 0)
	{
		const float OverchargePercent = CalculateOverchargePercent(OverchargeLevel);
		const int32 BoostedPrice = FMath::RoundToInt32(BaseSellPrice * (1.0f + OverchargePercent));
		return BoostedPrice;
	}

	return BaseSellPrice;
}

int32 AROServiceNPC_Shop::GetBasePrice(int32 ShopIndex) const
{
	if (!ShopInventory.IsValidIndex(ShopIndex))
	{
		return 0;
	}

	const FROShopItem& ShopItem = ShopInventory[ShopIndex];

	// If a custom price is set, use it
	if (ShopItem.CustomPrice > 0)
	{
		return ShopItem.CustomPrice;
	}

	// Fallback: derive default price from item database
	// In a full implementation, this would query the item database subsystem.
	// For now, return a placeholder price based on item ID.
	return GetDefaultItemSellPrice(ShopItem.ItemID) * 2;
}

FROShopItem AROServiceNPC_Shop::GetShopItemAtIndex(int32 Index) const
{
	if (ShopInventory.IsValidIndex(Index))
	{
		return ShopInventory[Index];
	}
	return FROShopItem();
}

int32 AROServiceNPC_Shop::GetDiscountSkillLevel(AROCharacterBase* Player) const
{
	if (!Player)
	{
		return 0;
	}

	UROSkillTreeComponent* SkillTree = Player->FindComponentByClass<UROSkillTreeComponent>();
	if (!SkillTree)
	{
		return 0;
	}

	return SkillTree->GetSkillLevel(SKILL_DISCOUNT);
}

int32 AROServiceNPC_Shop::GetOverchargeSkillLevel(AROCharacterBase* Player) const
{
	if (!Player)
	{
		return 0;
	}

	UROSkillTreeComponent* SkillTree = Player->FindComponentByClass<UROSkillTreeComponent>();
	if (!SkillTree)
	{
		return 0;
	}

	return SkillTree->GetSkillLevel(SKILL_OVERCHARGE);
}

float AROServiceNPC_Shop::CalculateDiscountPercent(int32 SkillLevel) const
{
	if (SkillLevel <= 0)
	{
		return 0.0f;
	}

	// RO Discount formula: (5 + 2 * SkillLevel) %
	// Level 1 = 7%, Level 5 = 15%, Level 10 = 25%
	return (5.0f + 2.0f * FMath::Clamp(SkillLevel, 1, 10)) / 100.0f;
}

float AROServiceNPC_Shop::CalculateOverchargePercent(int32 SkillLevel) const
{
	if (SkillLevel <= 0)
	{
		return 0.0f;
	}

	// RO Overcharge formula: (5 + 2 * SkillLevel) %
	// Level 1 = 7%, Level 5 = 15%, Level 10 = 25%
	return (5.0f + 2.0f * FMath::Clamp(SkillLevel, 1, 10)) / 100.0f;
}

int32 AROServiceNPC_Shop::GetDefaultItemSellPrice(int32 ItemID) const
{
	// Default sell price lookup.
	// In a full implementation, this queries the item database.
	// For common items, we provide hardcoded values:
	switch (ItemID)
	{
	case 501: return 25;    // Red Potion: sell 25 (buy 50)
	case 502: return 100;   // Orange Potion: sell 100 (buy 200)
	case 503: return 275;   // Yellow Potion: sell 275 (buy 550)
	case 504: return 600;   // White Potion: sell 600 (buy 1200)
	case 505: return 2500;  // Blue Potion: sell 2500 (buy 5000)
	case 506: return 20;    // Green Potion: sell 20 (buy 40)
	case 601: return 30;    // Fly Wing: sell 30 (buy 60)
	case 602: return 150;   // Butterfly Wing: sell 150 (buy 300)
	case 1750: return 0;    // Arrow: sell 0 (buy 1, not worth selling)
	case 1751: return 1;    // Silver Arrow: sell 1 (buy 3)
	default:
		// Generic fallback: 1 Zeny per item
		return 1;
	}
}
