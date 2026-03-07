// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/NPC/RONPCBase.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROServiceNPC_Storage.generated.h"

class AROCharacterBase;
class UROInventoryComponent;

/**
 * AROServiceNPC_Storage
 * Kafra Storage NPC that provides 600-slot storage access to players.
 * Each storage access costs Zeny. Items are stored per-player on the server.
 */
UCLASS(ClassGroup=(RagnarokUE), Blueprintable)
class RAGNAROKUE_API AROServiceNPC_Storage : public ARONPCBase
{
	GENERATED_BODY()

public:
	AROServiceNPC_Storage();

	// ---- Configuration ----

	/** Maximum number of storage slots (RO Kafra standard: 600). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage")
	int32 MaxStorageSlots;

	/** Cost in Zeny to open storage. Varies by Kafra NPC (30-80). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage", meta = (ClampMin = "0"))
	int32 StorageAccessCost;

	/** Display name shown in the storage window title. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage")
	FText StorageTitle;

	// ---- Interaction ----

	/** Override: Opens the storage window after deducting Zeny. */
	virtual void OnInteract_Implementation(AROCharacterBase* Interactor) override;

	// ---- Server RPCs ----

	/** Deposit an item from player inventory into storage. */
	UFUNCTION(Server, Reliable, WithValidation, Category = "Storage")
	void ServerDepositItem(int32 InventorySlot, int32 Amount);

	/** Withdraw an item from storage into player inventory. */
	UFUNCTION(Server, Reliable, WithValidation, Category = "Storage")
	void ServerWithdrawItem(int32 StorageSlot, int32 Amount);

	/** Close the storage window for the current user. */
	UFUNCTION(Server, Reliable, Category = "Storage")
	void ServerCloseStorage();

	// ---- Queries ----

	/** Get the storage contents for a player. */
	UFUNCTION(BlueprintCallable, Category = "Storage")
	TArray<FROItemInstance> GetPlayerStorage(AROCharacterBase* Player) const;

	/** Get the number of used storage slots for a player. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Storage")
	int32 GetUsedStorageSlots(AROCharacterBase* Player) const;

	/** Check if storage is currently open for this NPC. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Storage")
	bool IsStorageOpen() const { return CurrentStorageUser != nullptr; }

protected:
	/**
	 * Per-player storage data.
	 * In a production system, this would be on a PlayerState or database subsystem.
	 * For the NPC module, we store it here keyed by player unique ID.
	 */
	TMap<uint32, TArray<FROItemInstance>> PlayerStorageData;

	/** The player currently using this storage NPC (only one at a time). */
	UPROPERTY()
	TObjectPtr<AROCharacterBase> CurrentStorageUser;

	/** Get the unique key for a player. */
	uint32 GetPlayerKey(AROCharacterBase* Player) const;

	/** Ensure a player has a storage array initialized. */
	TArray<FROItemInstance>& GetOrCreateStorage(uint32 PlayerKey);

	/** Find the first empty storage slot for a player. Returns -1 if full. */
	int32 FindFreeStorageSlot(const TArray<FROItemInstance>& Storage) const;

	/** Find a storage slot with the same stackable item. Returns -1 if not found. */
	int32 FindStackableStorageSlot(const TArray<FROItemInstance>& Storage, int32 ItemID) const;

	/** Deduct storage access cost from player. Returns false if insufficient Zeny. */
	bool DeductStorageCost(AROCharacterBase* Player);
};
