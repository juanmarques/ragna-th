// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROLootManager.h"
#include "ROInventoryComponent.h"
#include "ROItemBase.h"
#include "ROItemDatabase.h"
#include "RODropTable.h"
#include "Net/UnrealNetwork.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================================
// UROLootManager
// ============================================================================

void UROLootManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Create the default drop table
	DropTable = NewObject<URODropTable>(this);

	UE_LOG(LogTemp, Log, TEXT("ROLootManager: Initialized."));
}

void UROLootManager::Deinitialize()
{
	DropTable = nullptr;
	Super::Deinitialize();
}

void UROLootManager::GenerateLoot(int32 MonsterID, AActor* Killer, FVector DeathLocation, float DropRateModifier)
{
	if (!DropTable || MonsterID <= 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || !World->IsServer())
	{
		return;
	}

	// Roll drops
	TArray<int32> DroppedItemIDs = DropTable->RollDrops(MonsterID, DropRateModifier);

	// Spawn each dropped item as a loot actor
	const float SpreadRadius = 75.0f;
	for (int32 i = 0; i < DroppedItemIDs.Num(); ++i)
	{
		// Spread items in a circle around the death location
		float Angle = (2.0f * PI * i) / FMath::Max(DroppedItemIDs.Num(), 1);
		FVector Offset(FMath::Cos(Angle) * SpreadRadius, FMath::Sin(Angle) * SpreadRadius, 0.0f);
		FVector SpawnLocation = DeathLocation + Offset;

		SpawnLootActor(DroppedItemIDs[i], 1, SpawnLocation, Killer);
	}
}

bool UROLootManager::PickupLoot(AActor* Character, AROLootActor* LootActor)
{
	// Only allow pickup processing on the server
	if (GetWorld() && GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("ROLootManager: PickupLoot called on client - rejected"));
		return false;
	}

	if (!Character || !LootActor)
	{
		return false;
	}

	// Check ownership
	if (!LootActor->CanPickup(Character))
	{
		return false;
	}

	// Get inventory component from character
	UROInventoryComponent* Inventory = Character->FindComponentByClass<UROInventoryComponent>();
	if (!Inventory)
	{
		return false;
	}

	int32 ItemID = LootActor->GetItemID();
	int32 Amount = LootActor->GetAmount();

	// Check if item can be added
	if (!Inventory->CanAddItem(ItemID, Amount))
	{
		return false;
	}

	// Add to inventory
	int32 SlotIndex = Inventory->Internal_AddItem(ItemID, Amount);
	if (SlotIndex < 0)
	{
		return false;
	}

	Inventory->UpdateWeight();

	// Destroy the loot actor
	LootActor->Destroy();

	return true;
}

AROLootActor* UROLootManager::SpawnLootActor(int32 ItemID, int32 Amount, FVector Location, AActor* Owner)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Validate item exists in the database
	UROItemDatabase* DB = GetItemDatabase();
	if (!DB || !DB->GetItemData(ItemID))
	{
		UE_LOG(LogTemp, Warning, TEXT("ROLootManager: Cannot spawn loot for unknown ItemID %d"), ItemID);
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	TSubclassOf<AROLootActor> ClassToSpawn = LootActorClass;
	if (!ClassToSpawn)
	{
		ClassToSpawn = AROLootActor::StaticClass();
	}

	AROLootActor* LootActor = World->SpawnActor<AROLootActor>(
		ClassToSpawn, Location, FRotator::ZeroRotator, SpawnParams);

	if (LootActor)
	{
		LootActor->InitializeLoot(ItemID, Amount, Owner);
	}

	return LootActor;
}

UROItemDatabase* UROLootManager::GetItemDatabase() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (GI)
	{
		return GI->GetSubsystem<UROItemDatabase>();
	}
	return nullptr;
}

// ============================================================================
// AROLootActor
// ============================================================================

AROLootActor::AROLootActor()
	: ItemID(0)
	, Amount(0)
	, bOwnershipExpired(false)
	, SpawnTime(0.0f)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = false;
	NetUpdateFrequency = 1.0f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void AROLootActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AROLootActor, ItemID);
	DOREPLIFETIME(AROLootActor, Amount);
	DOREPLIFETIME(AROLootActor, LootOwner);
	DOREPLIFETIME(AROLootActor, bOwnershipExpired);
}

void AROLootActor::BeginPlay()
{
	Super::BeginPlay();

	SpawnTime = GetWorld()->GetTimeSeconds();

	if (HasAuthority())
	{
		// Set up despawn timer
		GetWorldTimerManager().SetTimer(
			DespawnTimerHandle, this, &AROLootActor::OnDespawnTimerExpired,
			DESPAWN_TIME, false);

		// Set up ownership expiry timer
		if (LootOwner != nullptr)
		{
			GetWorldTimerManager().SetTimer(
				OwnershipTimerHandle, this, &AROLootActor::OnOwnershipTimerExpired,
				OWNERSHIP_DURATION, false);
		}
		else
		{
			bOwnershipExpired = true;
		}
	}
}

void AROLootActor::InitializeLoot(int32 InItemID, int32 InAmount, AActor* InOwner)
{
	ItemID = InItemID;
	Amount = InAmount;
	LootOwner = InOwner;
	bOwnershipExpired = (InOwner == nullptr);
}

bool AROLootActor::CanPickup(AActor* Character) const
{
	if (!Character)
	{
		return false;
	}

	// If ownership has expired, anyone can pick up
	if (bOwnershipExpired)
	{
		return true;
	}

	// During ownership period, only the owner can pick up
	return LootOwner != nullptr && Character == LootOwner;
}

float AROLootActor::GetRemainingDespawnTime() const
{
	if (!GetWorld())
	{
		return 0.0f;
	}

	float Elapsed = GetWorld()->GetTimeSeconds() - SpawnTime;
	return FMath::Max(DESPAWN_TIME - Elapsed, 0.0f);
}

bool AROLootActor::IsOwnershipExpired() const
{
	return bOwnershipExpired;
}

void AROLootActor::OnDespawnTimerExpired()
{
	Destroy();
}

void AROLootActor::OnOwnershipTimerExpired()
{
	bOwnershipExpired = true;
}
