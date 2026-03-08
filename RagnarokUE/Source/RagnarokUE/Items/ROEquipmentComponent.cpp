// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROEquipmentComponent.h"
#include "ROInventoryComponent.h"
#include "ROItemBase.h"
#include "ROWeaponData.h"
#include "ROArmorData.h"
#include "ROCardData.h"
#include "ROItemDatabase.h"
#include "RagnarokUE/Character/ROStatsComponent.h"
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
	// Validate slot index is within a sane range (actual bounds checked in Implementation via IsValidIndex)
	if (InventorySlot < 0)
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
	// Guard against race condition: reject if this slot is already being processed
	if (PendingEquipSlots.Contains(InventorySlot))
	{
		return;
	}
	PendingEquipSlots.Add(InventorySlot);

	UROInventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory)
	{
		PendingEquipSlots.Remove(InventorySlot);
		return;
	}

	FROItemInstance ItemToEquip = Inventory->GetItemAtSlot(InventorySlot);
	if (!ItemToEquip.IsValid())
	{
		PendingEquipSlots.Remove(InventorySlot);
		return;
	}

	UROItemDatabase* DB = GetItemDatabase();
	if (!DB)
	{
		PendingEquipSlots.Remove(InventorySlot);
		return;
	}

	const UROItemBase* ItemData = DB->GetItemData(ItemToEquip.ItemID);
	if (!ItemData)
	{
		PendingEquipSlots.Remove(InventorySlot);
		return;
	}

	// Validate that the item can go in the target slot (may auto-unequip shield for 2H weapons)
	if (!ValidateEquipSlot(ItemData, TargetSlot))
	{
		PendingEquipSlots.Remove(InventorySlot);
		return;
	}

	// If there's already something equipped in that slot, unequip it first
	if (IsSlotOccupied(TargetSlot))
	{
		FROItemInstance CurrentlyEquipped = EquippedItems[TargetSlot];

		// Check if inventory has space for the unequipped item
		if (!Inventory->CanAddItem(CurrentlyEquipped.ItemID, 1))
		{
			PendingEquipSlots.Remove(InventorySlot);
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

	// Remove from inventory FIRST, before placing in equipment
	if (!Inventory->Internal_RemoveItem(InventorySlot, 1))
	{
		// Item no longer there (consumed by race condition or other logic)
		PendingEquipSlots.Remove(InventorySlot);
		// Re-apply effects since we removed them above
		ApplyEquipmentEffects();
		RecalculateEquipmentBonuses();
		return;
	}

	// THEN place in equipment
	EquippedItems.Add(TargetSlot, ItemToEquip);

	// Reapply all equipment effects
	ApplyEquipmentEffects();
	RecalculateEquipmentBonuses();

	Inventory->UpdateWeight();

	PendingEquipSlots.Remove(InventorySlot);

	OnEquipmentChanged.Broadcast();
	OnItemEquipped.Broadcast(TargetSlot, ItemToEquip);
}

bool UROEquipmentComponent::ServerUnequipItem_Validate(EROEquipSlot Slot)
{
	return true;
}

void UROEquipmentComponent::ServerUnequipItem_Implementation(EROEquipSlot Slot)
{
	// Guard against race condition: reject if this slot is already being processed
	if (PendingUnequipSlots.Contains(Slot))
	{
		return;
	}
	PendingUnequipSlots.Add(Slot);

	if (!IsSlotOccupied(Slot))
	{
		PendingUnequipSlots.Remove(Slot);
		return;
	}

	UROInventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory)
	{
		PendingUnequipSlots.Remove(Slot);
		return;
	}

	FROItemInstance EquippedItem = EquippedItems[Slot];

	// Check if inventory has space
	if (!Inventory->CanAddItem(EquippedItem.ItemID, 1))
	{
		PendingUnequipSlots.Remove(Slot);
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

	PendingUnequipSlots.Remove(Slot);

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

	FROStatBlock NewEquipBonuses;
	NewEquipBonuses.BonusSTR = 0;
	NewEquipBonuses.BonusAGI = 0;
	NewEquipBonuses.BonusVIT = 0;
	NewEquipBonuses.BonusINT = 0;
	NewEquipBonuses.BonusDEX = 0;
	NewEquipBonuses.BonusLUK = 0;

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
				NewEquipBonuses.BonusSTR += CardData->StatBonuses.BonusSTR;
				NewEquipBonuses.BonusAGI += CardData->StatBonuses.BonusAGI;
				NewEquipBonuses.BonusVIT += CardData->StatBonuses.BonusVIT;
				NewEquipBonuses.BonusINT += CardData->StatBonuses.BonusINT;
				NewEquipBonuses.BonusDEX += CardData->StatBonuses.BonusDEX;
				NewEquipBonuses.BonusLUK += CardData->StatBonuses.BonusLUK;
			}
		}
	}

	// Apply the delta between old and new equipment bonuses to the StatsComponent.
	// This preserves non-equipment bonus stats (e.g., from buffs).
	if (AActor* Owner = GetOwner())
	{
		if (UROStatsComponent* StatsComp = Owner->FindComponentByClass<UROStatsComponent>())
		{
			// Remove old equipment bonuses
			StatsComp->BonusSTR -= CachedEquipBonuses.BonusSTR;
			StatsComp->BonusAGI -= CachedEquipBonuses.BonusAGI;
			StatsComp->BonusVIT -= CachedEquipBonuses.BonusVIT;
			StatsComp->BonusINT -= CachedEquipBonuses.BonusINT;
			StatsComp->BonusDEX -= CachedEquipBonuses.BonusDEX;
			StatsComp->BonusLUK -= CachedEquipBonuses.BonusLUK;

			// Apply new equipment bonuses
			StatsComp->AddBonusStat(EROStat::STR, NewEquipBonuses.BonusSTR);
			StatsComp->AddBonusStat(EROStat::AGI, NewEquipBonuses.BonusAGI);
			StatsComp->AddBonusStat(EROStat::VIT, NewEquipBonuses.BonusVIT);
			StatsComp->AddBonusStat(EROStat::INT_STAT, NewEquipBonuses.BonusINT);
			StatsComp->AddBonusStat(EROStat::DEX, NewEquipBonuses.BonusDEX);
			StatsComp->AddBonusStat(EROStat::LUK, NewEquipBonuses.BonusLUK);
		}
	}

	// Cache the new equipment bonuses for next delta calculation
	CachedEquipBonuses = NewEquipBonuses;
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
				if (SpecHandle.IsValid() && SpecHandle.Data.IsValid())
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

bool UROEquipmentComponent::ValidateEquipSlot(const UROItemBase* ItemData, EROEquipSlot TargetSlot)
{
	if (!ItemData)
	{
		return false;
	}

	// Weapons go in the weapon slot
	if (ItemData->ItemType == EROItemType::Weapon)
	{
		// Two-handed weapons: auto-unequip shield if occupied
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
				// Auto-unequip shield to inventory
				UROInventoryComponent* Inventory = GetInventoryComponent();
				if (Inventory && Inventory->CanAddItem(EquippedItems[EROEquipSlot::Shield].ItemID, 1))
				{
					FROItemInstance ShieldItem = EquippedItems[EROEquipSlot::Shield];
					EquippedItems.Remove(EROEquipSlot::Shield);
					Inventory->Internal_PlaceItem(ShieldItem);
				}
				else
				{
					// Can't unequip shield - inventory full
					return false;
				}
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
