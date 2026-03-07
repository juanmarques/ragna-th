// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/NPC/ROServiceNPC_Shop.h"
#include "ROServiceNPC_Tool.generated.h"

/**
 * AROServiceNPC_Tool
 * Tool Dealer NPC - a convenience shop with a preset inventory of common
 * consumables, potions, arrows, and utility items.
 *
 * Preset inventory:
 * - Red Potion (501, 50z), Orange Potion (502, 200z), Yellow Potion (503, 550z)
 * - White Potion (504, 1200z), Blue Potion (505, 5000z)
 * - Green Potion (506, 40z)
 * - Fly Wing (601, 60z), Butterfly Wing (602, 300z)
 * - Arrow (1750, 1z), Silver Arrow (1751, 3z)
 */
UCLASS(ClassGroup=(RagnarokUE), Blueprintable)
class RAGNAROKUE_API AROServiceNPC_Tool : public AROServiceNPC_Shop
{
	GENERATED_BODY()

public:
	AROServiceNPC_Tool();

	/**
	 * Reset the shop inventory to the default Tool Dealer items.
	 * Can be called to restore the preset if the inventory was modified.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Tool")
	void ResetToDefaultInventory();

protected:
	/** Populate the shop with the default tool dealer inventory. */
	void InitializeDefaultInventory();
};
