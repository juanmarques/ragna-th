// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROInventoryComponent.h"
#include "ROEquipmentComponent.h"
#include "ROItemBase.h"
#include "ROConsumableData.h"
#include "ROWeaponData.h"
#include "ROArmorData.h"
#include "ROItemDatabase.h"
#include "RagnarokUE/Data/ROConstants.h"
#include "Net/UnrealNetwork.h"
#include "Engine/GameInstance.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

UROInventoryComponent::UROInventoryComponent()
	: Zeny(0)
	, CurrentWeight(0.0f)
	, MaxWeight(2000.0f)
	, MaxSlots(100)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UROInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UROInventoryComponent, InventorySlots);
	DOREPLIFETIME(UROInventoryComponent, Zeny);
	DOREPLIFETIME(UROInventoryComponent, CurrentWeight);
	DOREPLIFETIME(UROInventoryComponent, MaxWeight);
}

void UROInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Pre-allocate inventory slots on the server
	if (GetOwnerRole() == ROLE_Authority)
	{
		InventorySlots.SetNum(MaxSlots);
	}
}

// ---- Server RPCs ----

bool UROInventoryComponent::ServerAddItem_Validate(int32 ItemID, int32 Amount)
{
	return ItemID > 0 && Amount > 0;
}

void UROInventoryComponent::ServerAddItem_Implementation(int32 ItemID, int32 Amount)
{
	if (!CanAddItem(ItemID, Amount))
	{
		return;
	}

	int32 SlotIndex = Internal_AddItem(ItemID, Amount);
	if (SlotIndex >= 0)
	{
		UpdateWeight();
		OnInventoryChanged.Broadcast();
		OnItemAdded.Broadcast(SlotIndex, InventorySlots[SlotIndex]);
	}
}

bool UROInventoryComponent::ServerRemoveItem_Validate(int32 SlotIndex, int32 Amount)
{
	return SlotIndex >= 0 && SlotIndex < MaxSlots && Amount > 0;
}

void UROInventoryComponent::ServerRemoveItem_Implementation(int32 SlotIndex, int32 Amount)
{
	if (Internal_RemoveItem(SlotIndex, Amount))
	{
		UpdateWeight();
		OnInventoryChanged.Broadcast();
		OnItemRemoved.Broadcast(SlotIndex, Amount);
	}
}

bool UROInventoryComponent::ServerMoveItem_Validate(int32 FromSlot, int32 ToSlot)
{
	return FromSlot >= 0 && FromSlot < MaxSlots && ToSlot >= 0 && ToSlot < MaxSlots && FromSlot != ToSlot;
}

void UROInventoryComponent::ServerMoveItem_Implementation(int32 FromSlot, int32 ToSlot)
{
	if (!InventorySlots.IsValidIndex(FromSlot) || !InventorySlots.IsValidIndex(ToSlot))
	{
		return;
	}

	if (!InventorySlots[FromSlot].IsValid())
	{
		return;
	}

	// If both slots have the same stackable item, try to merge
	if (InventorySlots[ToSlot].IsValid() &&
		InventorySlots[FromSlot].ItemID == InventorySlots[ToSlot].ItemID)
	{
		UROItemDatabase* DB = GetItemDatabase();
		if (DB)
		{
			const UROItemBase* ItemData = DB->GetItemData(InventorySlots[FromSlot].ItemID);
			if (ItemData && ItemData->bStackable)
			{
				int32 TotalAmount = InventorySlots[FromSlot].Amount + InventorySlots[ToSlot].Amount;
				if (TotalAmount <= ItemData->MaxStack)
				{
					// Full merge: all items fit in destination
					InventorySlots[ToSlot].Amount = TotalAmount;
					InventorySlots[FromSlot] = FROItemInstance();
					OnInventoryChanged.Broadcast();
					return;
				}
				else
				{
					// Partial merge: fill destination to max, leave remainder in source
					InventorySlots[ToSlot].Amount = ItemData->MaxStack;
					InventorySlots[FromSlot].Amount = TotalAmount - ItemData->MaxStack;
					OnInventoryChanged.Broadcast();
					return;
				}
			}
		}
	}

	// Otherwise, swap slots
	FROItemInstance Temp = InventorySlots[FromSlot];
	InventorySlots[FromSlot] = InventorySlots[ToSlot];
	InventorySlots[ToSlot] = Temp;
	OnInventoryChanged.Broadcast();
}

bool UROInventoryComponent::ServerUseItem_Validate(int32 SlotIndex)
{
	return SlotIndex >= 0 && SlotIndex < MaxSlots;
}

void UROInventoryComponent::ServerUseItem_Implementation(int32 SlotIndex)
{
	if (!InventorySlots.IsValidIndex(SlotIndex) || !InventorySlots[SlotIndex].IsValid())
	{
		return;
	}

	UROItemDatabase* DB = GetItemDatabase();
	if (!DB)
	{
		return;
	}

	const UROItemBase* ItemData = DB->GetItemData(InventorySlots[SlotIndex].ItemID);
	if (!ItemData || ItemData->ItemType != EROItemType::Consumable)
	{
		return;
	}

	const UROConsumableData* Consumable = Cast<UROConsumableData>(ItemData);
	if (!Consumable)
	{
		return;
	}

	// Check item use cooldown (all consumables share group 0 by default)
	const int32 CooldownGroup = 0;
	if (!IsItemCooldownReady(CooldownGroup))
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Apply GAS effect if specified
	if (Consumable->ConsumableEffect)
	{
		UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
		if (ASC)
		{
			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			EffectContext.AddSourceObject(Owner);
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Consumable->ConsumableEffect, 1.0f, EffectContext);
			if (SpecHandle.IsValid() && SpecHandle.Data.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}

	// Consume one unit
	Internal_RemoveItem(SlotIndex, 1);

	// Set item use cooldown (0.5 second shared cooldown)
	if (const UWorld* World = GetWorld())
	{
		ItemCooldowns.Add(CooldownGroup, World->GetTimeSeconds() + 0.5f);
	}

	UpdateWeight();
	OnInventoryChanged.Broadcast();
}

// ---- Queries ----

FROItemInstance UROInventoryComponent::GetItemAtSlot(int32 Index) const
{
	if (InventorySlots.IsValidIndex(Index))
	{
		return InventorySlots[Index];
	}
	return FROItemInstance();
}

FROItemInstance* UROInventoryComponent::GetItemAtSlotRef(int32 Index)
{
	if (InventorySlots.IsValidIndex(Index))
	{
		return &InventorySlots[Index];
	}
	return nullptr;
}

int32 UROInventoryComponent::FindItemByID(int32 ItemID) const
{
	for (int32 i = 0; i < InventorySlots.Num(); ++i)
	{
		if (InventorySlots[i].ItemID == ItemID && InventorySlots[i].IsValid())
		{
			return i;
		}
	}
	return -1;
}

bool UROInventoryComponent::CanAddItem(int32 ItemID, int32 Amount) const
{
	UROItemDatabase* DB = GetItemDatabase();
	if (!DB)
	{
		return false;
	}

	const UROItemBase* ItemData = DB->GetItemData(ItemID);
	if (!ItemData)
	{
		return false;
	}

	// Calculate projected weight locally without modifying state
	float ProjectedWeight = 0.0f;
	for (const FROItemInstance& Slot : InventorySlots)
	{
		if (Slot.IsValid())
		{
			const UROItemBase* SlotItemData = DB->GetItemData(Slot.ItemID);
			if (SlotItemData)
			{
				ProjectedWeight += SlotItemData->Weight * Slot.Amount;
			}
		}
	}

	// Include equipped item weights
	if (AActor* Owner = GetOwner())
	{
		if (UROEquipmentComponent* EquipComp = Owner->FindComponentByClass<UROEquipmentComponent>())
		{
			for (const auto& Pair : EquipComp->EquippedItems)
			{
				if (Pair.Value.IsValid())
				{
					const UROItemBase* EquipItemData = DB->GetItemData(Pair.Value.ItemID);
					if (EquipItemData)
					{
						ProjectedWeight += EquipItemData->Weight;
					}
				}
			}
		}
	}

	// Weight check
	float AdditionalWeight = ItemData->Weight * Amount;
	if (ProjectedWeight + AdditionalWeight > MaxWeight)
	{
		return false;
	}

	// Slot check: if stackable, see if we can fit into existing stacks or need new slots
	if (ItemData->bStackable && ItemData->MaxStack > 0)
	{
		int32 Remaining = Amount;
		for (int32 i = 0; i < InventorySlots.Num() && Remaining > 0; ++i)
		{
			if (InventorySlots[i].ItemID == ItemID)
			{
				int32 Space = ItemData->MaxStack - InventorySlots[i].Amount;
				Remaining -= FMath::Min(Space, Remaining);
			}
		}
		// If remaining, we need free slots
		if (Remaining > 0)
		{
			int32 SlotsNeeded = FMath::CeilToInt(static_cast<float>(Remaining) / ItemData->MaxStack);
			int32 FreeSlotCount = 0;
			for (int32 i = 0; i < InventorySlots.Num(); ++i)
			{
				if (!InventorySlots[i].IsValid())
				{
					++FreeSlotCount;
				}
			}
			if (FreeSlotCount < SlotsNeeded)
			{
				return false;
			}
		}
	}
	else
	{
		// Non-stackable: need one free slot per item
		int32 FreeSlotCount = 0;
		for (int32 i = 0; i < InventorySlots.Num(); ++i)
		{
			if (!InventorySlots[i].IsValid())
			{
				++FreeSlotCount;
			}
		}
		if (FreeSlotCount < Amount)
		{
			return false;
		}
	}

	return true;
}

bool UROInventoryComponent::IsOverweight() const
{
	return CurrentWeight >= MaxWeight;
}

bool UROInventoryComponent::IsOverweight50() const
{
	return CurrentWeight >= (MaxWeight * 0.5f);
}

bool UROInventoryComponent::IsOverweight90() const
{
	return CurrentWeight >= (MaxWeight * 0.9f);
}

int32 UROInventoryComponent::GetFreeSlot() const
{
	for (int32 i = 0; i < InventorySlots.Num(); ++i)
	{
		if (!InventorySlots[i].IsValid())
		{
			return i;
		}
	}
	return -1;
}

void UROInventoryComponent::UpdateMaxWeight(int32 STR)
{
	MaxWeight = static_cast<float>(ROConstants::CalculateMaxWeight(STR));
}

void UROInventoryComponent::UpdateWeight()
{
	UROItemDatabase* DB = GetItemDatabase();
	if (!DB)
	{
		return;
	}

	float TotalWeight = 0.0f;
	for (const FROItemInstance& Slot : InventorySlots)
	{
		if (Slot.IsValid())
		{
			const UROItemBase* ItemData = DB->GetItemData(Slot.ItemID);
			if (ItemData)
			{
				TotalWeight += ItemData->Weight * Slot.Amount;
			}
		}
	}

	// Add equipped item weights
	if (UROEquipmentComponent* EquipComp = GetOwner()->FindComponentByClass<UROEquipmentComponent>())
	{
		for (const auto& Pair : EquipComp->EquippedItems)
		{
			if (Pair.Value.IsValid())
			{
				const UROItemBase* ItemData = DB->GetItemData(Pair.Value.ItemID);
				if (ItemData)
				{
					TotalWeight += ItemData->Weight;
				}
			}
		}
	}

	CurrentWeight = TotalWeight;
}

int32 UROInventoryComponent::AddZeny(int32 Amount)
{
	if (Amount <= 0)
	{
		return 0;
	}

	int32 Space = MAX_ZENY - Zeny;
	int32 Actual = FMath::Min(Amount, Space);
	Zeny += Actual;
	OnZenyChanged.Broadcast(Zeny);
	return Actual;
}

bool UROInventoryComponent::RemoveZeny(int32 Amount)
{
	if (Amount <= 0 || Zeny < Amount)
	{
		return false;
	}

	Zeny -= Amount;
	OnZenyChanged.Broadcast(Zeny);
	return true;
}

int32 UROInventoryComponent::Internal_AddItem(int32 ItemID, int32 Amount)
{
	UROItemDatabase* DB = GetItemDatabase();
	if (!DB)
	{
		return -1;
	}

	const UROItemBase* ItemData = DB->GetItemData(ItemID);
	if (!ItemData)
	{
		return -1;
	}

	int32 FirstSlot = -1;

	if (ItemData->bStackable)
	{
		int32 Remaining = Amount;

		// Try to fill existing stacks first
		for (int32 i = 0; i < InventorySlots.Num() && Remaining > 0; ++i)
		{
			if (InventorySlots[i].ItemID == ItemID)
			{
				int32 Space = ItemData->MaxStack - InventorySlots[i].Amount;
				int32 ToAdd = FMath::Min(Space, Remaining);
				if (ToAdd > 0)
				{
					InventorySlots[i].Amount += ToAdd;
					Remaining -= ToAdd;
					if (FirstSlot < 0)
					{
						FirstSlot = i;
					}
				}
			}
		}

		// Place remainder in new slots
		while (Remaining > 0)
		{
			int32 FreeSlot = GetFreeSlot();
			if (FreeSlot < 0)
			{
				break;
			}

			int32 ToAdd = FMath::Min(Remaining, ItemData->MaxStack);
			InventorySlots[FreeSlot].ItemID = ItemID;
			InventorySlots[FreeSlot].Amount = ToAdd;
			InventorySlots[FreeSlot].UniqueID = FGuid::NewGuid();
			Remaining -= ToAdd;

			if (FirstSlot < 0)
			{
				FirstSlot = FreeSlot;
			}
		}
	}
	else
	{
		// Non-stackable: create one entry per item
		for (int32 i = 0; i < Amount; ++i)
		{
			int32 FreeSlot = GetFreeSlot();
			if (FreeSlot < 0)
			{
				break;
			}

			InventorySlots[FreeSlot].ItemID = ItemID;
			InventorySlots[FreeSlot].Amount = 1;
			InventorySlots[FreeSlot].RefineLevel = 0;
			InventorySlots[FreeSlot].UniqueID = FGuid::NewGuid();

			// Initialize card slot array based on equipment data
			const UROWeaponData* WeapData = Cast<UROWeaponData>(ItemData);
			const UROArmorData* ArmorData2 = Cast<UROArmorData>(ItemData);
			int32 NumCardSlots = 0;
			if (WeapData)
			{
				NumCardSlots = WeapData->CardSlots;
			}
			else if (ArmorData2)
			{
				NumCardSlots = ArmorData2->CardSlots;
			}
			if (NumCardSlots > 0)
			{
				InventorySlots[FreeSlot].CardSlots.SetNum(NumCardSlots);
				for (int32 s = 0; s < NumCardSlots; ++s)
				{
					InventorySlots[FreeSlot].CardSlots[s] = 0; // Empty slot
				}
			}

			if (FirstSlot < 0)
			{
				FirstSlot = FreeSlot;
			}
		}
	}

	return FirstSlot;
}

int32 UROInventoryComponent::Internal_PlaceItem(const FROItemInstance& Item)
{
	if (!Item.IsValid())
	{
		return -1;
	}

	int32 FreeSlot = GetFreeSlot();
	if (FreeSlot < 0)
	{
		return -1;
	}

	InventorySlots[FreeSlot] = Item;
	return FreeSlot;
}

bool UROInventoryComponent::Internal_RemoveItem(int32 SlotIndex, int32 Amount)
{
	if (!InventorySlots.IsValidIndex(SlotIndex) || !InventorySlots[SlotIndex].IsValid())
	{
		return false;
	}

	if (InventorySlots[SlotIndex].Amount < Amount)
	{
		return false;
	}

	InventorySlots[SlotIndex].Amount -= Amount;
	if (InventorySlots[SlotIndex].Amount <= 0)
	{
		InventorySlots[SlotIndex] = FROItemInstance();
	}

	return true;
}

bool UROInventoryComponent::IsItemCooldownReady(int32 CooldownGroup) const
{
	const float* Expiry = ItemCooldowns.Find(CooldownGroup);
	if (!Expiry)
	{
		return true;
	}
	const UWorld* World = GetWorld();
	return World && World->GetTimeSeconds() >= *Expiry;
}

void UROInventoryComponent::OnRep_InventorySlots()
{
	OnInventoryChanged.Broadcast();
}

void UROInventoryComponent::OnRep_Zeny()
{
	OnZenyChanged.Broadcast(Zeny);
}

UROItemDatabase* UROInventoryComponent::GetItemDatabase() const
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (GI)
	{
		return GI->GetSubsystem<UROItemDatabase>();
	}
	return nullptr;
}
