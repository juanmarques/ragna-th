// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROInventoryComponent.generated.h"

class UROItemBase;
class UROItemDatabase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAdded, int32, SlotIndex, const FROItemInstance&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, int32, SlotIndex, int32, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZenyChanged, int32, NewZeny);

/**
 * UROInventoryComponent
 * Manages the player's inventory with RO-style weight limits and item stacking.
 * All modifications are server-authoritative with replication to owning client.
 */
UCLASS(ClassGroup=(RagnarokUE), meta=(BlueprintSpawnableComponent))
class RAGNAROKUE_API UROInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UROInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	// ---- Inventory Data ----

	/** Array of item slots. Max 100 slots (RO standard). */
	UPROPERTY(ReplicatedUsing = OnRep_InventorySlots, BlueprintReadOnly, Category = "Inventory")
	TArray<FROItemInstance> InventorySlots;

	/** Current Zeny (currency). Max 1,000,000,000. */
	UPROPERTY(ReplicatedUsing = OnRep_Zeny, EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "0", ClampMax = "1000000000"))
	int32 Zeny;

	/** Current total weight of all inventory items. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
	float CurrentWeight;

	/** Maximum carry weight (derived from STR). */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	float MaxWeight;

	/** Maximum number of inventory slots. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 MaxSlots;

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnInventoryChanged OnInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnZenyChanged OnZenyChanged;

	// ---- Server RPCs ----

	/** Server: Add an item to inventory by ID and amount. Returns success via client RPC. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddItem(int32 ItemID, int32 Amount);

	/** Server: Remove items from a specific slot. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRemoveItem(int32 SlotIndex, int32 Amount);

	/** Server: Move an item from one slot to another (swap if destination occupied). */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMoveItem(int32 FromSlot, int32 ToSlot);

	/** Server: Use a consumable item from the given slot. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUseItem(int32 SlotIndex);

	// ---- Blueprint Callable Queries ----

	/** Get item data at the specified inventory slot. Returns empty instance if invalid. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	FROItemInstance GetItemAtSlot(int32 Index) const;

	/** Find the first inventory slot containing the given ItemID. Returns -1 if not found. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 FindItemByID(int32 ItemID) const;

	/** Check if the inventory can hold the specified item and amount. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool CanAddItem(int32 ItemID, int32 Amount) const;

	/** Check if the character is overweight (CurrentWeight >= MaxWeight). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool IsOverweight() const;

	/** Get the first available (empty) inventory slot. Returns -1 if full. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetFreeSlot() const;

	/** Recalculate CurrentWeight from all inventory items. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UpdateWeight();

	/** Add Zeny (clamped to max). Returns actual amount added. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddZeny(int32 Amount);

	/** Remove Zeny. Returns false if insufficient funds. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveZeny(int32 Amount);

	/** Internal: Add item directly (server-side only). Returns the slot index or -1 on failure. */
	int32 Internal_AddItem(int32 ItemID, int32 Amount);

	/** Internal: Remove item directly (server-side only). Returns true on success. */
	bool Internal_RemoveItem(int32 SlotIndex, int32 Amount);

protected:
	UFUNCTION()
	void OnRep_InventorySlots();

	UFUNCTION()
	void OnRep_Zeny();

private:
	/** Maximum Zeny cap. */
	static constexpr int32 MAX_ZENY = 1000000000;

	/** Get the item database subsystem. */
	UROItemDatabase* GetItemDatabase() const;
};
