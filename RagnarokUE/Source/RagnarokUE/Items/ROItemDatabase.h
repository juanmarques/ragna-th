// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ROItemDatabase.generated.h"

class UROItemBase;
class UROWeaponData;
class UROArmorData;
class UROCardData;
class UROConsumableData;
class UDataTable;

/**
 * UROItemDatabase
 * GameInstance subsystem that loads and caches all item data assets.
 * Provides fast lookups by ItemID for the entire item database.
 */
UCLASS()
class RAGNAROKUE_API UROItemDatabase : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Initialize the subsystem: load all item data from data tables and asset registry. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Deinitialize the subsystem. */
	virtual void Deinitialize() override;

	/**
	 * Get any item data by ItemID.
	 * @param ItemID The numeric item identifier.
	 * @return The item data asset, or nullptr if not found.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item Database")
	UROItemBase* GetItemData(int32 ItemID) const;

	/**
	 * Get weapon data by ItemID. Returns nullptr if the item isn't a weapon.
	 * @param ItemID The numeric item identifier.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item Database")
	UROWeaponData* GetWeaponData(int32 ItemID) const;

	/**
	 * Get armor data by ItemID. Returns nullptr if the item isn't armor.
	 * @param ItemID The numeric item identifier.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item Database")
	UROArmorData* GetArmorData(int32 ItemID) const;

	/**
	 * Get card data by ItemID. Returns nullptr if the item isn't a card.
	 * @param ItemID The numeric item identifier.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item Database")
	UROCardData* GetCardData(int32 ItemID) const;

	/**
	 * Get consumable data by ItemID. Returns nullptr if the item isn't a consumable.
	 * @param ItemID The numeric item identifier.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item Database")
	UROConsumableData* GetConsumableData(int32 ItemID) const;

	/**
	 * Register an item data asset at runtime.
	 * @param ItemData The item data asset to register.
	 */
	UFUNCTION(BlueprintCallable, Category = "Item Database")
	void RegisterItem(UROItemBase* ItemData);

	/** Get the total number of registered items. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item Database")
	int32 GetItemCount() const;

	/** Get all registered item IDs. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item Database")
	TArray<int32> GetAllItemIDs() const;

private:
	/** Master item lookup table: ItemID -> Item Data Asset. */
	UPROPERTY()
	TMap<int32, UROItemBase*> ItemDatabase;

	/** Load all UROItemBase assets from the asset registry. */
	void LoadAllItemAssets();
};
