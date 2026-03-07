// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROCardSystem.generated.h"

class UROItemBase;
class UROCardData;
class UROItemDatabase;

/**
 * UROCardSystem
 * Static utility class for card insertion, removal, and compound naming.
 * Cards are the RO system for socketing bonus effects into equipment.
 */
UCLASS()
class RAGNAROKUE_API UROCardSystem : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Insert a card into an equipment item's card slot.
	 * @param Equipment The equipment item instance to insert the card into.
	 * @param CardID The item ID of the card to insert.
	 * @param SlotIndex Which card slot to use (0-3).
	 * @return True if the card was successfully inserted.
	 */
	UFUNCTION(BlueprintCallable, Category = "Card System")
	static bool InsertCard(UPARAM(ref) FROItemInstance& Equipment, int32 CardID, int32 SlotIndex);

	/**
	 * Remove a card from an equipment item's card slot.
	 * @param Equipment The equipment item instance to remove the card from.
	 * @param SlotIndex Which card slot to remove from (0-3).
	 * @return The item ID of the removed card, or 0 if no card was present.
	 */
	UFUNCTION(BlueprintCallable, Category = "Card System")
	static int32 RemoveCard(UPARAM(ref) FROItemInstance& Equipment, int32 SlotIndex);

	/**
	 * Get all card data objects for cards socketed in the equipment.
	 * @param Equipment The equipment to query.
	 * @param ItemDatabase The item database to look up card data.
	 * @return Array of card data objects for all socketed cards.
	 */
	UFUNCTION(BlueprintCallable, Category = "Card System")
	static TArray<UROCardData*> GetCardEffects(const FROItemInstance& Equipment, UROItemDatabase* ItemDatabase);

	/**
	 * Validate whether a card can be inserted into a given equipment piece.
	 * Checks slot type compatibility (weapon cards go in weapons, armor cards in armor, etc.).
	 * @param Equipment The equipment data asset.
	 * @param Card The card data asset.
	 * @return True if the card is compatible with this equipment type.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Card System")
	static bool ValidateCardSlot(const UROItemBase* Equipment, const UROCardData* Card);

	/**
	 * Generate the compound name for an item with socketed cards.
	 * In RO, cards add prefixes/suffixes to item names (e.g., "Double Bloody Katar").
	 * @param BaseItem The base equipment data.
	 * @param Cards Array of card data objects socketed in the equipment.
	 * @return The compound name string.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Card System")
	static FString GetCompoundName(const UROItemBase* BaseItem, const TArray<UROCardData*>& Cards);

private:
	/** Prefix multiplier names for multiple of the same card. */
	static FString GetMultiplierPrefix(int32 Count);
};
