// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROItemDatabase.h"
#include "ROItemBase.h"
#include "ROWeaponData.h"
#include "ROArmorData.h"
#include "ROCardData.h"
#include "ROConsumableData.h"
#include "Engine/AssetManager.h"

void UROItemDatabase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadAllItemAssets();

	UE_LOG(LogTemp, Log, TEXT("ROItemDatabase: Initialized with %d items."), ItemDatabase.Num());
}

void UROItemDatabase::Deinitialize()
{
	ItemDatabase.Empty();
	Super::Deinitialize();
}

UROItemBase* UROItemDatabase::GetItemData(int32 ItemID) const
{
	UROItemBase* const* Found = ItemDatabase.Find(ItemID);
	return Found ? *Found : nullptr;
}

UROWeaponData* UROItemDatabase::GetWeaponData(int32 ItemID) const
{
	UROItemBase* ItemData = GetItemData(ItemID);
	return ItemData ? Cast<UROWeaponData>(ItemData) : nullptr;
}

UROArmorData* UROItemDatabase::GetArmorData(int32 ItemID) const
{
	UROItemBase* ItemData = GetItemData(ItemID);
	return ItemData ? Cast<UROArmorData>(ItemData) : nullptr;
}

UROCardData* UROItemDatabase::GetCardData(int32 ItemID) const
{
	UROItemBase* ItemData = GetItemData(ItemID);
	return ItemData ? Cast<UROCardData>(ItemData) : nullptr;
}

UROConsumableData* UROItemDatabase::GetConsumableData(int32 ItemID) const
{
	UROItemBase* ItemData = GetItemData(ItemID);
	return ItemData ? Cast<UROConsumableData>(ItemData) : nullptr;
}

void UROItemDatabase::RegisterItem(UROItemBase* ItemData)
{
	if (ItemData && ItemData->ItemID > 0)
	{
		ItemDatabase.Add(ItemData->ItemID, ItemData);
	}
}

int32 UROItemDatabase::GetItemCount() const
{
	return ItemDatabase.Num();
}

TArray<int32> UROItemDatabase::GetAllItemIDs() const
{
	TArray<int32> IDs;
	ItemDatabase.GetKeys(IDs);
	return IDs;
}

void UROItemDatabase::LoadAllItemAssets()
{
	UAssetManager& AssetManager = UAssetManager::Get();

	// Discover all primary assets of item types
	TArray<FPrimaryAssetType> ItemTypes;

	// Register the types we care about
	TArray<FString> TypeNames = {
		TEXT("Weapon"),
		TEXT("Armor"),
		TEXT("Card"),
		TEXT("Consumable"),
		TEXT("EtcItem"),
		TEXT("Ammo"),
		TEXT("PetEgg"),
		TEXT("PetArmor")
	};

	for (const FString& TypeName : TypeNames)
	{
		FPrimaryAssetType AssetType(*TypeName);

		TArray<FPrimaryAssetId> AssetIds;
		AssetManager.GetPrimaryAssetIdList(AssetType, AssetIds);

		for (const FPrimaryAssetId& AssetId : AssetIds)
		{
			FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(AssetId);
			if (AssetPath.IsValid())
			{
				UObject* LoadedAsset = AssetPath.TryLoad();
				if (UROItemBase* ItemData = Cast<UROItemBase>(LoadedAsset))
				{
					if (ItemData->ItemID > 0)
					{
						ItemDatabase.Add(ItemData->ItemID, ItemData);
					}
				}
			}
		}
	}

	// Also scan for any ROItemBase assets not registered with the asset manager
	// This handles assets placed directly in content folders
	TArray<FAssetData> AssetDataList;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Search for all classes derived from UROItemBase
	AssetRegistry.GetAssetsByClass(UROItemBase::StaticClass()->GetClassPathName(), AssetDataList, true);

	for (const FAssetData& AssetData : AssetDataList)
	{
		if (UROItemBase* ItemData = Cast<UROItemBase>(AssetData.GetAsset()))
		{
			if (ItemData->ItemID > 0 && !ItemDatabase.Contains(ItemData->ItemID))
			{
				ItemDatabase.Add(ItemData->ItemID, ItemData);
			}
		}
	}
}
