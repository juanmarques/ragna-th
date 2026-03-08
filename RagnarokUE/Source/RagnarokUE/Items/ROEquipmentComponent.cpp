// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROEquipmentComponent.h"
#include "ROInventoryComponent.h"
#include "ROItemBase.h"
#include "ROWeaponData.h"
#include "ROArmorData.h"
#include "ROCardData.h"
#include "ROItemDatabase.h"
#include "Net/UnrealNetwork.h"
#include "Engine/GameInstance.h"
#include "AbilitySystemComponent.h"

UROEquipmentComponent::UROEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UROEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UROEquipmentComponent, EquippedItems);
}

void UROEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
}

// ---- Server RPCs ----

bool UROEquipmentComponent::ServerEquipItem_Validate(int32 InventorySlot, EROEquipSlot TargetSlot)
{
	// Validate slot index is within a sane range (actual bounds checked in Implementation)
	if (InventorySlot < 0 || InventorySlot >= 200)
	{
		return false;
	}

	// Validate TargetSlot is a known enum value
	if (TargetSlot < EROEquipSlot::Weapon || TargetSlot > EROEquipSlot::HeadLow)
	{
		return false;
	}

	return true;
}

void UROEquipmentComponent::ServerEquipItem_Implementation(int32 InventorySlot, EROEquipSlot TargetSlot)
{
	UROInventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory)
	{
		return;
	}

	FROItemInstance ItemToEquip = Inventory->GetItemAtSlot(InventorySlot);
	if (!ItemToEquip.IsValid())
	{
		return;
	}

	UROItemDatabase* DB = GetItemDatabase();
	if (!DB)
	{
		return;
	}

	const UROItemBase* ItemData = DB->GetItemData(ItemToEquip.ItemID);
	if (!ItemData)
	{
		return;
	}

	// Validate that the item can go in the target slot
	if (!ValidateEquipSlot(ItemData, TargetSlot))
	{
		return;
	}

	// If there's already something equipped in that slot, unequip it first
	if (IsSlotOccupied(TargetSlot))
	{
		FROItemInstance CurrentlyEquipped = EquippedItems[TargetSlot];

		// Check if inventory has space for the unequipped item
		if (!Inventory->CanAddItem(CurrentlyEquipped.ItemID, 1))
		{
			return;
		}

		// Remove equipment effects before swapping
		RemoveEquipmentEffects();

		// Put the old item back in inventory (preserving refine, cards, etc)
		Inventory->Internal_PlaceItem(CurrentlyEquipped);
		EquippedItems.Remove(TargetSlot);
	}
	else
	{
		RemoveEquipmentEffects();
	}

	// Remove item from inventory
	Inventory->Internal_RemoveItem(InventorySlot, 1);

	// Equip the new item
	EquippedItems.Add(TargetSlot, ItemToEquip);

	// Reapply all equipment effects
	ApplyEquipmentEffects();
	RecalculateEquipmentBonuses();

	Inventory->UpdateWeight();

	OnEquipmentChanged.Broadcast();
	OnItemEquipped.Broadcast(TargetSlot, ItemToEquip);
}

bool UROEquipmentComponent::ServerUnequipItem_Validate(EROEquipSlot Slot)
{
	return true;
}

void UROEquipmentComponent::ServerUnequipItem_Implementation(EROEquipSlot Slot)
{
	if (!IsSlotOccupied(Slot))
	{
		return;
	}

	UROInventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory)
	{
		return;
	}

	FROItemInstance EquippedItem = EquippedItems[Slot];

	// Check if inventory has space
	if (!Inventory->CanAddItem(EquippedItem.ItemID, 1))
	{
		return;
	}

	// Remove equipment effects
	RemoveEquipmentEffects();

	// Remove from equipment
	EquippedItems.Remove(Slot);

	// Put back in inventory (preserving refine, cards, etc)
	Inventory->Internal_PlaceItem(EquippedItem);

	// Reapply remaining equipment effects
	ApplyEquipmentEffects();
	RecalculateEquipmentBonuses();

	Inventory->UpdateWeight();

	OnEquipmentChanged.Broadcast();
	OnItemUnequipped.Broadcast(Slot);
}

// ---- Queries ----

FROItemInstance UROEquipmentComponent::GetEquippedItem(EROEquipSlot Slot) const
{
	if (const FROItemInstance* Found = EquippedItems.Find(Slot))
	{
		return *Found;
	}
	return FROItemInstance();
}

bool UROEquipmentComponent::IsSlotOccupied(EROEquipSlot Slot) const
{
	const FROItemInstance* Found = EquippedItems.Find(Slot);
	return Found && Found->IsValid();
}

int32 UROEquipmentComponent::GetTotalEquipDEF() const
{
	UROItemDatabase* DB = GetItemDatabase();
	if (!DB)
	{
		return 0;
	}

	int32 TotalDEF = 0;
	for (const auto& Pair : EquippedItems)
	{
		if (!Pair.Value.IsValid())
		{
			continue;
		}

		const UROArmorData* ArmorData = DB->GetArmorData(Pair.Value.ItemID);
		if (ArmorData)
		{
			TotalDEF += ArmorData->DEF;

			// Add refine bonus for armor: +1 DEF per refine level
			TotalDEF += Pair.Value.RefineLevel;
		}
	}
	return TotalDEF;
}

int32 UROEquipmentComponent::GetTotalEquipMDEF() const
{
	UROItemDatabase* DB = GetItemDatabase();
	if (!DB)
	{
		return 0;
	}

	int32 TotalMDEF = 0;
	for (const auto& Pair : EquippedItems)
	{
		if (!Pair.Value.IsValid())
		{
			continue;
		}

		const UROArmorData* ArmorData = DB->GetArmorData(Pair.Value.ItemID);
		if (ArmorData)
		{
			TotalMDEF += ArmorData->MDEF;
		}
	}
	return TotalMDEF;
}

int32 UROEquipmentComponent::GetTotalEquipATK() const
{
	UROItemDatabase* DB = GetItemDatabase();
	if (!DB)
	{
		return 0;
	}

	int32 TotalATK = 0;

	const FROItemInstance* WeaponSlot = EquippedItems.Find(EROEquipSlot::Weapon);
	if (WeaponSlot && WeaponSlot->IsValid())
	{
		const UROWeaponData* WeaponData = DB->GetWeaponData(WeaponSlot->ItemID);
		if (WeaponData)
		{
			TotalATK += WeaponData->ATK;
			TotalATK += WeaponData->GetTotalRefineBonus(WeaponSlot->RefineLevel);
		}
	}

	return TotalATK;
}

void UROEquipmentComponent::RecalculateEquipmentBonuses()
{
	UROItemDatabase* DB = GetItemDatabase();
	if (!DB)
	{
		return;
	}

	FROStatBlock EquipBonuses;
	EquipBonuses.BaseSTR = 0;
	EquipBonuses.BaseAGI = 0;
	EquipBonuses.BaseVIT = 0;
	EquipBonuses.BaseINT = 0;
	EquipBonuses.BaseDEX = 0;
	EquipBonuses.BaseLUK = 0;

	for (const auto& Pair : EquippedItems)
	{
		if (!Pair.Value.IsValid())
		{
			continue;
		}

		// Sum card stat bonuses from all socketed cards
		for (int32 CardID : Pair.Value.CardSlots)
		{
			if (CardID <= 0)
			{
				continue;
			}

			const UROCardData* CardData = DB->GetCardData(CardID);
			if (CardData)
			{
				EquipBonuses.BonusSTR += CardData->StatBonuses.BonusSTR;
				EquipBonuses.BonusAGI += CardData->StatBonuses.BonusAGI;
				EquipBonuses.BonusVIT += CardData->StatBonuses.BonusVIT;
				EquipBonuses.BonusINT += CardData->StatBonuses.BonusINT;
				EquipBonuses.BonusDEX += CardData->StatBonuses.BonusDEX;
				EquipBonuses.BonusLUK += CardData->StatBonuses.BonusLUK;
			}
		}
	}

	// The actual stat application goes through the StatsComponent.
	// Callers (ServerEquipItem/ServerUnequipItem) broadcast OnEquipmentChanged.
}

void UROEquipmentComponent::ApplyEquipmentEffects()
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	UROItemDatabase* DB = GetItemDatabase();
	if (!DB)
	{
		return;
	}

	for (const auto& Pair : EquippedItems)
	{
		if (!Pair.Value.IsValid())
		{
			continue;
		}

		// Apply card effects from socketed cards
		for (int32 CardID : Pair.Value.CardSlots)
		{
			if (CardID <= 0)
			{
				continue;
			}

			const UROCardData* CardData = DB->GetCardData(CardID);
			if (CardData && CardData->CardEffect)
			{
				FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
				EffectContext.AddSourceObject(GetOwner());
				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
					CardData->CardEffect, 1.0f, EffectContext);
				if (SpecHandle.IsValid())
				{
					FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectSpecToSelf(
						*SpecHandle.Data.Get());
					if (Handle.IsValid())
					{
						ActiveEquipmentEffects.Add(Handle);
					}
				}
			}
		}
	}
}

void UROEquipmentComponent::RemoveEquipmentEffects()
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		ActiveEquipmentEffects.Empty();
		return;
	}

	for (const FActiveGameplayEffectHandle& Handle : ActiveEquipmentEffects)
	{
		if (Handle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(Handle);
		}
	}
	ActiveEquipmentEffects.Empty();
}

void UROEquipmentComponent::OnRep_EquippedItems()
{
	OnEquipmentChanged.Broadcast();
}

// ---- Private Helpers ----

UROItemDatabase* UROEquipmentComponent::GetItemDatabase() const
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (GI)
	{
		return GI->GetSubsystem<UROItemDatabase>();
	}
	return nullptr;
}

UROInventoryComponent* UROEquipmentComponent::GetInventoryComponent() const
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		return Owner->FindComponentByClass<UROInventoryComponent>();
	}
	return nullptr;
}

UAbilitySystemComponent* UROEquipmentComponent::GetASC() const
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		return Owner->FindComponentByClass<UAbilitySystemComponent>();
	}
	return nullptr;
}

bool UROEquipmentComponent::ValidateEquipSlot(const UROItemBase* ItemData, EROEquipSlot TargetSlot) const
{
	if (!ItemData)
	{
		return false;
	}

	// Weapons go in the weapon slot
	if (ItemData->ItemType == EROItemType::Weapon)
	{
		// Two-handed weapons: cannot equip if shield is occupied (and vice versa)
		const UROWeaponData* WeapData = Cast<UROWeaponData>(ItemData);
		if (WeapData)
		{
			// Check if this is a two-handed weapon type
			bool bIsTwoHanded = (WeapData->WeaponType == EROWeaponType::TwoHandSword ||
								WeapData->WeaponType == EROWeaponType::TwoHandSpear ||
								WeapData->WeaponType == EROWeaponType::TwoHandAxe ||
								WeapData->WeaponType == EROWeaponType::TwoHandMace ||
								WeapData->WeaponType == EROWeaponType::TwoHandRod ||
								WeapData->WeaponType == EROWeaponType::Bow ||
								WeapData->WeaponType == EROWeaponType::Katar);
			if (bIsTwoHanded && IsSlotOccupied(EROEquipSlot::Shield))
			{
				return false; // Can't equip 2H weapon with shield
			}
		}
		return TargetSlot == EROEquipSlot::Weapon;
	}

	// Armor goes in its designated slot
	if (ItemData->ItemType == EROItemType::Armor)
	{
		const UROArmorData* ArmorData = Cast<UROArmorData>(ItemData);
		if (ArmorData)
		{
			// Shield: cannot equip if a two-handed weapon is equipped
			if (TargetSlot == EROEquipSlot::Shield)
			{
				const FROItemInstance* CurrentWeapon = EquippedItems.Find(EROEquipSlot::Weapon);
				if (CurrentWeapon && CurrentWeapon->IsValid())
				{
					UROItemDatabase* WeapDB = GetItemDatabase();
					if (WeapDB)
					{
						const UROWeaponData* EquippedWeaponData = WeapDB->GetWeaponData(CurrentWeapon->ItemID);
						if (EquippedWeaponData)
						{
							bool bIsTwoHanded = (EquippedWeaponData->WeaponType == EROWeaponType::TwoHandSword ||
												EquippedWeaponData->WeaponType == EROWeaponType::TwoHandSpear ||
												EquippedWeaponData->WeaponType == EROWeaponType::TwoHandAxe ||
												EquippedWeaponData->WeaponType == EROWeaponType::TwoHandMace ||
												EquippedWeaponData->WeaponType == EROWeaponType::TwoHandRod ||
												EquippedWeaponData->WeaponType == EROWeaponType::Bow ||
												EquippedWeaponData->WeaponType == EROWeaponType::Katar);
							if (bIsTwoHanded)
							{
								return false; // Can't equip shield with 2H weapon
							}
						}
					}
				}
			}

			// Accessories can go in either left or right slot
			if (ArmorData->EquipSlot == EROEquipSlot::AccessoryL ||
				ArmorData->EquipSlot == EROEquipSlot::AccessoryR)
			{
				return TargetSlot == EROEquipSlot::AccessoryL ||
					   TargetSlot == EROEquipSlot::AccessoryR;
			}
			return ArmorData->EquipSlot == TargetSlot;
		}
	}

	return false;
}
