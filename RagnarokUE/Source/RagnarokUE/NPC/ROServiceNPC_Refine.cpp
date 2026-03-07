// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROServiceNPC_Refine.h"
#include "RagnarokUE/Character/ROCharacterBase.h"
#include "RagnarokUE/Items/ROInventoryComponent.h"
#include "RagnarokUE/Items/RORefinementSystem.h"

AROServiceNPC_Refine::AROServiceNPC_Refine()
{
	bIsService = true;
	MaxRefineLevel = 10;
	CurrentRefineUser = nullptr;
	DisplayName = FText::FromString(TEXT("Refine NPC"));
}

void AROServiceNPC_Refine::OnInteract_Implementation(AROCharacterBase* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	Super::OnInteract_Implementation(Interactor);

	CurrentRefineUser = Interactor;

	UE_LOG(LogTemp, Log, TEXT("Refine NPC %s: Player opened refinement window."), *NPCName.ToString());
}

void AROServiceNPC_Refine::ServerRefineItem_Implementation(int32 InventorySlot)
{
	if (!CurrentRefineUser)
	{
		UE_LOG(LogTemp, Warning, TEXT("Refine NPC: No active user for refinement."));
		return;
	}

	UROInventoryComponent* Inventory = CurrentRefineUser->FindComponentByClass<UROInventoryComponent>();
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("Refine NPC: Player has no inventory component."));
		return;
	}

	// Get the item to refine
	FROItemInstance ItemToRefine = Inventory->GetItemAtSlot(InventorySlot);
	if (!ItemToRefine.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Refine NPC: Invalid inventory slot %d."), InventorySlot);
		return;
	}

	// Check if the item is refinable
	if (!IsRefinableEquipment(ItemToRefine.ItemID))
	{
		UE_LOG(LogTemp, Warning, TEXT("Refine NPC: Item %d is not refinable."), ItemToRefine.ItemID);
		return;
	}

	// Determine weapon level
	const int32 WeaponLevel = DetermineWeaponLevel(ItemToRefine.ItemID);

	// Check max refine
	if (ItemToRefine.RefineLevel >= MaxRefineLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("Refine NPC: Item already at max refine level %d."), MaxRefineLevel);
		return;
	}

	// Check required ore
	const int32 RequiredOreID = URORefinementSystem::GetRequiredOre(WeaponLevel);
	const int32 OreSlot = Inventory->FindItemByID(RequiredOreID);
	if (OreSlot < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Refine NPC: Player missing required ore (ID: %d)."), RequiredOreID);
		return;
	}

	// Check Zeny cost
	const int32 ZenyCost = URORefinementSystem::GetRefineCost(ItemToRefine.RefineLevel);
	if (Inventory->Zeny < ZenyCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("Refine NPC: Player cannot afford %d Zeny for refinement."), ZenyCost);
		return;
	}

	// Attempt refinement using the refinement system
	// Note: AttemptRefine handles ore and zeny deduction internally
	// We work on a copy from inventory, then apply result back
	bool bSuccess = URORefinementSystem::AttemptRefine(ItemToRefine, Inventory, WeaponLevel);

	if (bSuccess)
	{
		// Update the item in inventory with the new refine level
		// We need to update the actual slot in the inventory
		// Since we got a copy, we write it back via direct slot access
		if (Inventory->InventorySlots.IsValidIndex(InventorySlot))
		{
			Inventory->InventorySlots[InventorySlot].RefineLevel = ItemToRefine.RefineLevel;
		}

		UE_LOG(LogTemp, Log, TEXT("Refine NPC: Refinement SUCCESS! Item %d now +%d."),
			ItemToRefine.ItemID, ItemToRefine.RefineLevel);
	}
	else
	{
		// On failure, the item is destroyed (standard RO behavior)
		Inventory->Internal_RemoveItem(InventorySlot, 1);

		UE_LOG(LogTemp, Log, TEXT("Refine NPC: Refinement FAILED! Item %d destroyed."), ItemToRefine.ItemID);
	}

	// Broadcast result
	OnRefineResult.Broadcast(bSuccess, bSuccess ? ItemToRefine.RefineLevel : 0, ItemToRefine.ItemID);
}

bool AROServiceNPC_Refine::ServerRefineItem_Validate(int32 InventorySlot)
{
	return InventorySlot >= 0;
}

bool AROServiceNPC_Refine::CanRefineItem(const FROItemInstance& Item, int32 WeaponLevel) const
{
	if (!Item.IsValid())
	{
		return false;
	}

	if (!IsRefinableEquipment(Item.ItemID))
	{
		return false;
	}

	if (Item.RefineLevel >= MaxRefineLevel)
	{
		return false;
	}

	return true;
}

float AROServiceNPC_Refine::GetRefineSuccessRate(const FROItemInstance& Item, int32 WeaponLevel) const
{
	if (!Item.IsValid())
	{
		return 0.0f;
	}

	return URORefinementSystem::GetRefineSuccessRate(Item.RefineLevel, WeaponLevel);
}

int32 AROServiceNPC_Refine::GetRefineCost(const FROItemInstance& Item) const
{
	if (!Item.IsValid())
	{
		return 0;
	}

	return URORefinementSystem::GetRefineCost(Item.RefineLevel);
}

int32 AROServiceNPC_Refine::DetermineWeaponLevel(int32 ItemID) const
{
	// Item ID ranges for RO equipment classification:
	// 1100-1149: Swords (Level 1-3 depending on specific ID)
	// 1150-1199: Two-Hand Swords (Level 2-4)
	// 1200-1249: Daggers (Level 1-3)
	// 1250-1299: Katars (Level 1-4)
	// 1300-1349: Axes (Level 1-3)
	// 1350-1399: Two-Hand Axes (Level 2-4)
	// 1400-1449: Spears (Level 1-3)
	// 1450-1499: Two-Hand Spears (Level 2-4)
	// 1500-1549: Maces (Level 1-3)
	// 1550-1599: Books (Level 1-3)
	// 1600-1649: Rods (Level 1-3)
	// 1700-1749: Bows (Level 1-3)
	// 1800-1849: Knuckles (Level 1-3)
	// 1900-1949: Instruments (Level 1-3)
	// 1950-1999: Whips (Level 1-3)
	// 2100-2199: Shields (Armor = 0)
	// 2200-2299: Headgear (Armor = 0)
	// 2300-2399: Armor (Armor = 0)
	// 2400-2499: Garments (Armor = 0)
	// 2500-2599: Footgear (Armor = 0)
	// 2600-2699: Accessories (Not refinable)

	// Weapons: 1100-1999
	if (ItemID >= 1100 && ItemID < 2000)
	{
		// Simplified: classify based on sub-ranges
		// Most basic weapons are Level 1, mid-tier Level 2, high-tier Level 3-4
		// This is a simplification; full implementation would use item database
		if (ItemID < 1150)
		{
			return 1; // Basic Swords
		}
		else if (ItemID < 1200)
		{
			return 2; // Two-Hand Swords
		}
		else if (ItemID < 1250)
		{
			return 1; // Daggers
		}
		else if (ItemID < 1300)
		{
			return 3; // Katars
		}
		else if (ItemID < 1350)
		{
			return 1; // Axes
		}
		else if (ItemID < 1400)
		{
			return 2; // Two-Hand Axes
		}
		else if (ItemID < 1450)
		{
			return 1; // Spears
		}
		else if (ItemID < 1500)
		{
			return 2; // Two-Hand Spears
		}
		else if (ItemID < 1550)
		{
			return 1; // Maces
		}
		else if (ItemID < 1600)
		{
			return 2; // Books
		}
		else if (ItemID < 1650)
		{
			return 1; // Rods
		}
		else if (ItemID < 1700)
		{
			return 2; // Guns (placeholder)
		}
		else if (ItemID < 1750)
		{
			return 1; // Bows
		}
		else if (ItemID < 1800)
		{
			return 1; // Ammo (not refinable, but handled)
		}
		else if (ItemID < 1850)
		{
			return 1; // Knuckles
		}
		else if (ItemID < 1900)
		{
			return 2; // Reserved
		}
		else if (ItemID < 1950)
		{
			return 1; // Instruments
		}
		else
		{
			return 1; // Whips
		}
	}

	// Armor: 2100-2599
	if (ItemID >= 2100 && ItemID < 2600)
	{
		return 0; // All armor has weapon level 0
	}

	return -1; // Not refinable
}

bool AROServiceNPC_Refine::IsRefinableEquipment(int32 ItemID) const
{
	// Weapons (1100-1999) and Armor (2100-2599) are refinable
	// Accessories (2600+) and consumables (<1100) are not
	if (ItemID >= 1100 && ItemID < 2000)
	{
		// Ammo range (1750-1799) is not refinable
		if (ItemID >= 1750 && ItemID < 1800)
		{
			return false;
		}
		return true;
	}

	if (ItemID >= 2100 && ItemID < 2600)
	{
		return true;
	}

	return false;
}
