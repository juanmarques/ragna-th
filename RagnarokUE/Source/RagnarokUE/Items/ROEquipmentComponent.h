// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "GameplayEffect.h"
#include "ROEquipmentComponent.generated.h"

class UROInventoryComponent;
class UROItemBase;
class UROWeaponData;
class UROArmorData;
class UROCardData;
class UROItemDatabase;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemEquipped, EROEquipSlot, Slot, const FROItemInstance&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUnequipped, EROEquipSlot, Slot);

/**
 * UROEquipmentComponent
 * Manages the character's 10 equipment slots with replication.
 * Works in tandem with UROInventoryComponent for equip/unequip operations.
 */
UCLASS(ClassGroup=(RagnarokUE), meta=(BlueprintSpawnableComponent))
class RAGNAROKUE_API UROEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UROEquipmentComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	// ---- Equipment Data ----

	/** Currently equipped items, keyed by slot. */
	UPROPERTY(ReplicatedUsing = OnRep_EquippedItems, BlueprintReadOnly, Category = "Equipment")
	TMap<EROEquipSlot, FROItemInstance> EquippedItems;

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
	FOnEquipmentChanged OnEquipmentChanged;

	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
	FOnItemEquipped OnItemEquipped;

	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
	FOnItemUnequipped OnItemUnequipped;

	// ---- Server RPCs ----

	/** Server: Equip an item from inventory into the specified slot. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipItem(int32 InventorySlot, EROEquipSlot TargetSlot);

	/** Server: Unequip an item from the specified slot back to inventory. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUnequipItem(EROEquipSlot Slot);

	// ---- Queries ----

	/** Get the item instance in the given equipment slot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
	FROItemInstance GetEquippedItem(EROEquipSlot Slot) const;

	/** Check if a specific equipment slot has an item. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
	bool IsSlotOccupied(EROEquipSlot Slot) const;

	/** Get total physical DEF from all equipped armor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
	int32 GetTotalEquipDEF() const;

	/** Get total magical DEF from all equipped armor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
	int32 GetTotalEquipMDEF() const;

	/** Get total ATK from equipped weapon (including refine). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
	int32 GetTotalEquipATK() const;

	/** Recalculate all equipment stat bonuses and update the character's stats. */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void RecalculateEquipmentBonuses();

	/** Apply all currently equipped item effects via GAS. */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void ApplyEquipmentEffects();

	/** Remove all currently applied equipment effects. */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void RemoveEquipmentEffects();

protected:
	UFUNCTION()
	void OnRep_EquippedItems();

private:
	/** Active GAS effect handles for removal. */
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> ActiveEquipmentEffects;

	/** Slots currently being processed by equip operations (race condition guard). */
	TSet<int32> PendingEquipSlots;

	/** Equipment slots currently being processed by unequip operations (race condition guard). */
	TSet<EROEquipSlot> PendingUnequipSlots;

	/** Cached equipment stat bonuses for delta-based application. */
	FROStatBlock CachedEquipBonuses;

	/** Get the item database subsystem. */
	UROItemDatabase* GetItemDatabase() const;

	/** Get the inventory component on the same actor. */
	UROInventoryComponent* GetInventoryComponent() const;

	/** Get the ability system component on the same actor. */
	UAbilitySystemComponent* GetASC() const;

	/** Validate that the item can go in the target slot. May auto-unequip shield for 2H weapons. */
	bool ValidateEquipSlot(const UROItemBase* ItemData, EROEquipSlot TargetSlot);
};
