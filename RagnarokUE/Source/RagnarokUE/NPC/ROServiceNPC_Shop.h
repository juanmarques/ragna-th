// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/NPC/RONPCBase.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROServiceNPC_Shop.generated.h"

class AROCharacterBase;
class UROInventoryComponent;

/**
 * FROShopItem
 * A single item entry in a shop's inventory with optional price override.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROShopItem
{
	GENERATED_BODY()

	/** Database item ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 ItemID = 0;

	/** Custom price override. 0 = use the item's default price from database. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop", meta = (ClampMin = "0"))
	int32 CustomPrice = 0;

	FROShopItem() {}

	FROShopItem(int32 InItemID, int32 InPrice)
		: ItemID(InItemID), CustomPrice(InPrice) {}
};

/**
 * AROServiceNPC_Shop
 * Buy/Sell NPC that maintains a shop inventory.
 * Supports Discount skill modifier on buy and Overcharge skill modifier on sell.
 */
UCLASS(ClassGroup=(RagnarokUE), Blueprintable)
class RAGNAROKUE_API AROServiceNPC_Shop : public ARONPCBase
{
	GENERATED_BODY()

public:
	AROServiceNPC_Shop();

	// ---- Configuration ----

	/** Items available for purchase at this shop. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	TArray<FROShopItem> ShopInventory;

	/** Name of the shop displayed in the UI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FText ShopName;

	// ---- Interaction ----

	/** Override: Opens the shop window. */
	virtual void OnInteract_Implementation(AROCharacterBase* Interactor) override;

	// ---- Server RPCs ----

	/**
	 * Buy an item from this shop.
	 * Deducts Zeny (with Discount modifier) and adds item to inventory.
	 */
	UFUNCTION(Server, Reliable, WithValidation, Category = "Shop")
	void ServerBuyItem(int32 ShopIndex, int32 Amount);

	/**
	 * Sell an item from the player's inventory.
	 * Removes item and adds Zeny (sell price, with Overcharge modifier).
	 */
	UFUNCTION(Server, Reliable, WithValidation, Category = "Shop")
	void ServerSellItem(int32 InventorySlot, int32 Amount);

	// ---- Price Calculation ----

	/**
	 * Get the buy price of an item, factoring in the buyer's Discount skill.
	 * Formula: base_price * (1 - discount%)
	 * Discount skill: 5 + 2*SkillLevel percent discount (max level 10 = 25% discount)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Shop")
	int32 GetBuyPrice(int32 ShopIndex, AROCharacterBase* Buyer) const;

	/**
	 * Get the sell price of an item, factoring in the seller's Overcharge skill.
	 * Formula: base_price / 2 * (1 + overcharge%)
	 * Overcharge skill: 5 + 2*SkillLevel percent bonus (max level 10 = 25% bonus)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Shop")
	int32 GetSellPrice(int32 ItemID, AROCharacterBase* Seller) const;

	/**
	 * Get the base price for a shop item at the given index.
	 * Returns CustomPrice if set, otherwise would query item database.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Shop")
	int32 GetBasePrice(int32 ShopIndex) const;

	/**
	 * Get the number of items in the shop inventory.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Shop")
	int32 GetShopItemCount() const { return ShopInventory.Num(); }

	/**
	 * Get shop item data at an index.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Shop")
	FROShopItem GetShopItemAtIndex(int32 Index) const;

protected:
	/** The player currently using the shop. */
	UPROPERTY()
	TObjectPtr<AROCharacterBase> CurrentShopUser;

	/**
	 * Get the Discount skill level for a player.
	 * Skill ID 41 = Discount in RO.
	 * Returns 0 if player doesn't have the skill.
	 */
	int32 GetDiscountSkillLevel(AROCharacterBase* Player) const;

	/**
	 * Get the Overcharge skill level for a player.
	 * Skill ID 42 = Overcharge in RO.
	 * Returns 0 if player doesn't have the skill.
	 */
	int32 GetOverchargeSkillLevel(AROCharacterBase* Player) const;

	/**
	 * Calculate the discount percentage from Discount skill level.
	 * Formula: (5 + 2 * SkillLevel) / 100.0
	 */
	float CalculateDiscountPercent(int32 SkillLevel) const;

	/**
	 * Calculate the overcharge percentage from Overcharge skill level.
	 * Formula: (5 + 2 * SkillLevel) / 100.0
	 */
	float CalculateOverchargePercent(int32 SkillLevel) const;

	/**
	 * Get the default sell price for an item from the database.
	 * In a full implementation, this queries the item database.
	 * Placeholder: returns a default based on item ID.
	 */
	int32 GetDefaultItemSellPrice(int32 ItemID) const;

	/** Skill IDs for Discount and Overcharge. */
	static constexpr int32 SKILL_DISCOUNT = 41;
	static constexpr int32 SKILL_OVERCHARGE = 42;
};
