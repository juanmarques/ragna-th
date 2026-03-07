// Copyright Ragna-TH Project. All Rights Reserved.

#include "RORefinementSystem.h"
#include "ROInventoryComponent.h"
#include "ROWeaponData.h"

int32 URORefinementSystem::GetSafeRefineLimit(int32 WeaponLevel)
{
	switch (WeaponLevel)
	{
	case 0: return 4;  // Armor
	case 1: return 7;  // Lv1 weapon
	case 2: return 6;  // Lv2 weapon
	case 3: return 5;  // Lv3 weapon
	case 4: return 4;  // Lv4 weapon
	default: return 4;
	}
}

float URORefinementSystem::GetRefineSuccessRate(int32 CurrentRefine, int32 WeaponLevel)
{
	int32 TargetRefine = CurrentRefine + 1;

	if (TargetRefine > MAX_REFINE)
	{
		return 0.0f;
	}

	int32 SafeLimit = GetSafeRefineLimit(WeaponLevel);
	if (CurrentRefine < SafeLimit)
	{
		return 100.0f;
	}

	if (WeaponLevel == 0)
	{
		return GetArmorRefineRate(TargetRefine);
	}

	return GetWeaponRefineRate(TargetRefine, WeaponLevel);
}

int32 URORefinementSystem::GetRefineBonus(int32 RefineLevel, int32 WeaponLevel)
{
	if (RefineLevel <= 0)
	{
		return 0;
	}

	if (WeaponLevel == 0)
	{
		// Armor: +1 DEF per refine level
		return RefineLevel;
	}

	// Weapons: bonus per refine depends on weapon level
	int32 BonusPerRefine = 0;
	switch (WeaponLevel)
	{
	case 1: BonusPerRefine = 2; break;
	case 2: BonusPerRefine = 3; break;
	case 3: BonusPerRefine = 5; break;
	case 4: BonusPerRefine = 7; break;
	default: BonusPerRefine = 2; break;
	}

	return RefineLevel * BonusPerRefine;
}

int32 URORefinementSystem::GetRequiredOre(int32 WeaponLevel)
{
	// Standard RO ore item IDs
	switch (WeaponLevel)
	{
	case 0: return 985;   // Elunium (armor)
	case 1: return 1010;  // Phracon (Lv1 weapons)
	case 2: return 1011;  // Emveretarcon (Lv2 weapons)
	case 3: return 984;   // Oridecon (Lv3 weapons)
	case 4: return 984;   // Oridecon (Lv4 weapons)
	default: return 984;
	}
}

int32 URORefinementSystem::GetRefineCost(int32 CurrentRefine)
{
	// RO refine costs scale with level
	// Base cost + increment per refine level
	switch (CurrentRefine)
	{
	case 0: return 200;
	case 1: return 200;
	case 2: return 200;
	case 3: return 200;
	case 4: return 2000;
	case 5: return 2000;
	case 6: return 2000;
	case 7: return 2000;
	case 8: return 20000;
	case 9: return 20000;
	default: return 20000;
	}
}

bool URORefinementSystem::AttemptRefine(FROItemInstance& Item, UROInventoryComponent* Inventory, int32 WeaponLevel)
{
	if (!Inventory || !Item.IsValid())
	{
		return false;
	}

	// Check max refine
	if (Item.RefineLevel >= MAX_REFINE)
	{
		return false;
	}

	// Check required ore
	int32 OreItemID = GetRequiredOre(WeaponLevel);
	int32 OreSlot = Inventory->FindItemByID(OreItemID);
	if (OreSlot < 0)
	{
		return false;
	}

	// Check Zeny cost
	int32 Cost = GetRefineCost(Item.RefineLevel);
	if (Inventory->Zeny < Cost)
	{
		return false;
	}

	// Consume ore and Zeny
	Inventory->Internal_RemoveItem(OreSlot, 1);
	Inventory->RemoveZeny(Cost);
	Inventory->UpdateWeight();

	// Calculate success
	float SuccessRate = GetRefineSuccessRate(Item.RefineLevel, WeaponLevel);
	float Roll = FMath::FRandRange(0.0f, 100.0f);

	if (Roll <= SuccessRate)
	{
		// Success: increment refine level
		Item.RefineLevel++;
		return true;
	}
	else
	{
		// Failure: destroy the equipment
		Item.ItemID = 0;
		Item.Amount = 0;
		Item.RefineLevel = 0;
		Item.CardSlots.Empty();
		return false;
	}
}

float URORefinementSystem::GetArmorRefineRate(int32 TargetRefine)
{
	// Armor refine success rates (refining TO this level)
	// +1 to +4: 100% (safe)
	// +5: 60%, +6: 40%, +7: 40%, +8: 20%, +9: 20%, +10: 10%
	switch (TargetRefine)
	{
	case 1: return 100.0f;
	case 2: return 100.0f;
	case 3: return 100.0f;
	case 4: return 100.0f;
	case 5: return 60.0f;
	case 6: return 40.0f;
	case 7: return 40.0f;
	case 8: return 20.0f;
	case 9: return 20.0f;
	case 10: return 10.0f;
	default: return 0.0f;
	}
}

float URORefinementSystem::GetWeaponRefineRate(int32 TargetRefine, int32 WeaponLevel)
{
	// Weapon refine success rates vary by weapon level
	// These are the classic pre-renewal rates

	if (WeaponLevel == 1)
	{
		// Lv1: safe to +7
		switch (TargetRefine)
		{
		case 8: return 60.0f;
		case 9: return 40.0f;
		case 10: return 19.0f;
		default: return 100.0f;
		}
	}
	else if (WeaponLevel == 2)
	{
		// Lv2: safe to +6
		switch (TargetRefine)
		{
		case 7: return 60.0f;
		case 8: return 40.0f;
		case 9: return 20.0f;
		case 10: return 19.0f;
		default: return 100.0f;
		}
	}
	else if (WeaponLevel == 3)
	{
		// Lv3: safe to +5
		switch (TargetRefine)
		{
		case 6: return 60.0f;
		case 7: return 50.0f;
		case 8: return 20.0f;
		case 9: return 20.0f;
		case 10: return 19.0f;
		default: return 100.0f;
		}
	}
	else if (WeaponLevel == 4)
	{
		// Lv4: safe to +4
		switch (TargetRefine)
		{
		case 5: return 60.0f;
		case 6: return 40.0f;
		case 7: return 40.0f;
		case 8: return 20.0f;
		case 9: return 20.0f;
		case 10: return 10.0f;
		default: return 100.0f;
		}
	}

	return 0.0f;
}
