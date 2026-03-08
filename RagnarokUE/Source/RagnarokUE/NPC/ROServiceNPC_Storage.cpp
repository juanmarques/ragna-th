// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROServiceNPC_Storage.h"
#include "RagnarokUE/Character/ROCharacterBase.h"
#include "RagnarokUE/Items/ROInventoryComponent.h"

AROServiceNPC_Storage::AROServiceNPC_Storage()
{
	bIsService = true;
	MaxStorageSlots = 600;
	StorageAccessCost = 60;
	StorageTitle = FText::FromString(TEXT("Kafra Storage"));
	CurrentStorageUser = nullptr;
}

void AROServiceNPC_Storage::OnInteract_Implementation(AROCharacterBase* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	// Call parent to broadcast the interaction delegate
	Super::OnInteract_Implementation(Interactor);

	// Check if another player is using this storage NPC
	if (CurrentStorageUser && CurrentStorageUser != Interactor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Storage NPC %s: Already in use by another player."), *NPCName.ToString());
		return;
	}

	// Deduct Zeny for storage access
	if (!DeductStorageCost(Interactor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Storage NPC %s: Player cannot afford storage access cost (%d Zeny)."),
			*NPCName.ToString(), StorageAccessCost);
		return;
	}

	CurrentStorageUser = Interactor;

	UE_LOG(LogTemp, Log, TEXT("Storage NPC %s: Player opened storage. Cost: %d Zeny."),
		*NPCName.ToString(), StorageAccessCost);
}

bool AROServiceNPC_Storage::DeductStorageCost(AROCharacterBase* Player)
{
	if (StorageAccessCost <= 0)
	{
		return true;
	}

	UROInventoryComponent* Inventory = Player->FindComponentByClass<UROInventoryComponent>();
	if (!Inventory)
	{
		return false;
	}

	return Inventory->RemoveZeny(StorageAccessCost);
}

uint32 AROServiceNPC_Storage::GetPlayerKey(AROCharacterBase* Player) const
{
	if (!Player)
	{
		return 0;
	}
	return Player->GetUniqueID();
}

TArray<FROItemInstance>& AROServiceNPC_Storage::GetOrCreateStorage(uint32 PlayerKey)
{
	TArray<FROItemInstance>& Storage = PlayerStorageData.FindOrAdd(PlayerKey);
	if (Storage.Num() == 0)
	{
		Storage.SetNum(MaxStorageSlots);
	}
	return Storage;
}

int32 AROServiceNPC_Storage::FindFreeStorageSlot(const TArray<FROItemInstance>& Storage) const
{
	for (int32 i = 0; i < Storage.Num(); ++i)
	{
		if (!Storage[i].IsValid())
		{
			return i;
		}
	}
	return -1;
}

int32 AROServiceNPC_Storage::FindStackableStorageSlot(const TArray<FROItemInstance>& Storage, int32 ItemID) const
{
	for (int32 i = 0; i < Storage.Num(); ++i)
	{
		if (Storage[i].ItemID == ItemID && Storage[i].IsValid())
		{
			// Only stackable items (non-equipment: no cards, no refine)
			if (Storage[i].RefineLevel == 0 && Storage[i].CardSlots.Num() == 0)
			{
				return i;
			}
		}
	}
	return -1;
}

void AROServiceNPC_Storage::ServerDepositItem_Implementation(int32 InventorySlot, int32 Amount)
{
	if (!CurrentStorageUser)
	{
		UE_LOG(LogTemp, Warning, TEXT("Storage: No active storage user for deposit."));
		return;
	}

	UROInventoryComponent* Inventory = CurrentStorageUser->FindComponentByClass<UROInventoryComponent>();
	if (!Inventory)
	{
		return;
	}

	// Validate the inventory slot
	FROItemInstance ItemAtSlot = Inventory->GetItemAtSlot(InventorySlot);
	if (!ItemAtSlot.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Storage: Invalid inventory slot %d for deposit."), InventorySlot);
		return;
	}

	// Clamp amount to what's available
	const int32 DepositAmount = FMath::Min(Amount, ItemAtSlot.Amount);
	if (DepositAmount <= 0)
	{
		return;
	}

	const uint32 PlayerKey = GetPlayerKey(CurrentStorageUser);
	TArray<FROItemInstance>& Storage = GetOrCreateStorage(PlayerKey);

	// Try to find a stackable slot first
	int32 TargetSlot = FindStackableStorageSlot(Storage, ItemAtSlot.ItemID);
	if (TargetSlot >= 0 && ItemAtSlot.RefineLevel == 0 && ItemAtSlot.CardSlots.Num() == 0)
	{
		// Stack onto existing slot
		Storage[TargetSlot].Amount += DepositAmount;
	}
	else
	{
		// Find an empty slot
		TargetSlot = FindFreeStorageSlot(Storage);
		if (TargetSlot < 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Storage: No free storage slots available."));
			return;
		}

		// Place item in storage
		Storage[TargetSlot] = ItemAtSlot;
		Storage[TargetSlot].Amount = DepositAmount;
	}

	// Remove from inventory
	Inventory->Internal_RemoveItem(InventorySlot, DepositAmount);

	UE_LOG(LogTemp, Verbose, TEXT("Storage: Deposited %d of item %d to storage slot %d."),
		DepositAmount, ItemAtSlot.ItemID, TargetSlot);
}

bool AROServiceNPC_Storage::ServerDepositItem_Validate(int32 InventorySlot, int32 Amount)
{
	return Amount > 0 && InventorySlot >= 0 && InventorySlot < 100;
}

void AROServiceNPC_Storage::ServerWithdrawItem_Implementation(int32 StorageSlot, int32 Amount)
{
	if (!CurrentStorageUser)
	{
		UE_LOG(LogTemp, Warning, TEXT("Storage: No active storage user for withdrawal."));
		return;
	}

	UROInventoryComponent* Inventory = CurrentStorageUser->FindComponentByClass<UROInventoryComponent>();
	if (!Inventory)
	{
		return;
	}

	const uint32 PlayerKey = GetPlayerKey(CurrentStorageUser);
	TArray<FROItemInstance>& Storage = GetOrCreateStorage(PlayerKey);

	// Validate storage slot
	if (!Storage.IsValidIndex(StorageSlot) || !Storage[StorageSlot].IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Storage: Invalid storage slot %d for withdrawal."), StorageSlot);
		return;
	}

	FROItemInstance& StoredItem = Storage[StorageSlot];
	const int32 WithdrawAmount = FMath::Min(Amount, StoredItem.Amount);
	if (WithdrawAmount <= 0)
	{
		return;
	}

	// Check if player inventory can hold the item
	if (!Inventory->CanAddItem(StoredItem.ItemID, WithdrawAmount))
	{
		UE_LOG(LogTemp, Warning, TEXT("Storage: Player inventory cannot hold withdrawn item."));
		return;
	}

	// Add to player inventory
	int32 AddedSlot = Inventory->Internal_AddItem(StoredItem.ItemID, WithdrawAmount);
	if (AddedSlot < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Storage: Failed to add item to player inventory."));
		return;
	}

	// Remove from storage
	StoredItem.Amount -= WithdrawAmount;
	if (StoredItem.Amount <= 0)
	{
		StoredItem = FROItemInstance(); // Clear the slot
	}

	UE_LOG(LogTemp, Verbose, TEXT("Storage: Withdrew %d of item from storage slot %d."),
		WithdrawAmount, StorageSlot);
}

bool AROServiceNPC_Storage::ServerWithdrawItem_Validate(int32 StorageSlot, int32 Amount)
{
	return Amount > 0 && StorageSlot >= 0;
}

bool AROServiceNPC_Storage::ServerCloseStorage_Validate()
{
	return true;
}

void AROServiceNPC_Storage::ServerCloseStorage_Implementation()
{
	if (CurrentStorageUser)
	{
		UE_LOG(LogTemp, Log, TEXT("Storage NPC %s: Player closed storage."), *NPCName.ToString());
		CurrentStorageUser = nullptr;
	}
}

TArray<FROItemInstance> AROServiceNPC_Storage::GetPlayerStorage(AROCharacterBase* Player) const
{
	if (!Player)
	{
		return TArray<FROItemInstance>();
	}

	const uint32 PlayerKey = GetPlayerKey(Player);
	const TArray<FROItemInstance>* Storage = PlayerStorageData.Find(PlayerKey);
	if (Storage)
	{
		return *Storage;
	}
	return TArray<FROItemInstance>();
}

int32 AROServiceNPC_Storage::GetUsedStorageSlots(AROCharacterBase* Player) const
{
	if (!Player)
	{
		return 0;
	}

	const uint32 PlayerKey = GetPlayerKey(Player);
	const TArray<FROItemInstance>* Storage = PlayerStorageData.Find(PlayerKey);
	if (!Storage)
	{
		return 0;
	}

	int32 UsedCount = 0;
	for (const FROItemInstance& Item : *Storage)
	{
		if (Item.IsValid())
		{
			UsedCount++;
		}
	}
	return UsedCount;
}
