// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROServiceNPC_Tool.h"

AROServiceNPC_Tool::AROServiceNPC_Tool()
{
	ShopName = FText::FromString(TEXT("Tool Dealer"));
	DisplayName = FText::FromString(TEXT("Tool Dealer"));

	InitializeDefaultInventory();
}

void AROServiceNPC_Tool::InitializeDefaultInventory()
{
	ShopInventory.Empty();

	// Potions
	ShopInventory.Add(FROShopItem(501, 50));      // Red Potion
	ShopInventory.Add(FROShopItem(502, 200));     // Orange Potion
	ShopInventory.Add(FROShopItem(503, 550));     // Yellow Potion
	ShopInventory.Add(FROShopItem(504, 1200));    // White Potion
	ShopInventory.Add(FROShopItem(505, 5000));    // Blue Potion

	// Status Cure
	ShopInventory.Add(FROShopItem(506, 40));      // Green Potion

	// Utility Wings
	ShopInventory.Add(FROShopItem(602, 300));     // Butterfly Wing
	ShopInventory.Add(FROShopItem(601, 60));      // Fly Wing

	// Ammunition
	ShopInventory.Add(FROShopItem(1750, 1));      // Arrow
	ShopInventory.Add(FROShopItem(1751, 3));      // Silver Arrow
}

void AROServiceNPC_Tool::ResetToDefaultInventory()
{
	InitializeDefaultInventory();
}
