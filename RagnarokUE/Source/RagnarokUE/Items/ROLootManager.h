// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "GameFramework/Actor.h"
#include "ROLootManager.generated.h"

class UROInventoryComponent;
class UROItemBase;
class UROItemDatabase;
class URODropTable;
class AROLootActor;

/**
 * UROLootManager
 * World subsystem that handles loot generation, ground item spawning,
 * and item pickup logic. Manages the lifecycle of dropped items.
 */
UCLASS()
class RAGNAROKUE_API UROLootManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Generate and spawn loot when a monster is killed.
	 * Creates AROLootActor instances on the ground near the monster's death location.
	 * @param MonsterID The database ID of the killed monster.
	 * @param Killer The character that killed the monster (used for loot ownership).
	 * @param DeathLocation World location where the monster died.
	 * @param DropRateModifier Drop rate multiplier (default 1.0).
	 */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	void GenerateLoot(int32 MonsterID, AActor* Killer, FVector DeathLocation, float DropRateModifier = 1.0f);

	/**
	 * Attempt to pick up a loot actor and add it to a character's inventory.
	 * Respects ownership rules (killer-only period).
	 * @param Character The character attempting to pick up loot.
	 * @param LootActor The loot actor being picked up.
	 * @return True if the item was successfully picked up.
	 */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	bool PickupLoot(AActor* Character, AROLootActor* LootActor);

	/**
	 * Spawn a single item on the ground.
	 * @param ItemID The item to spawn.
	 * @param Amount Stack amount.
	 * @param Location World location to spawn at.
	 * @param Owner Optional owner who has exclusive pickup rights initially.
	 * @return The spawned loot actor, or nullptr on failure.
	 */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	AROLootActor* SpawnLootActor(int32 ItemID, int32 Amount, FVector Location, AActor* Owner = nullptr);

	/** Get the drop table used by this manager. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Loot")
	URODropTable* GetDropTable() const { return DropTable; }

	/** Set a custom drop table. */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	void SetDropTable(URODropTable* NewDropTable) { DropTable = NewDropTable; }

private:
	/** The drop table containing all monster drop data. */
	UPROPERTY()
	URODropTable* DropTable;

	/** Class to spawn for loot actors. */
	UPROPERTY()
	TSubclassOf<AROLootActor> LootActorClass;

	/** Get the item database subsystem. */
	UROItemDatabase* GetItemDatabase() const;
};

// ============================================================================

/**
 * AROLootActor
 * Represents a dropped item on the ground.
 * Features:
 * - Visual representation of the dropped item.
 * - Ownership system: only the killer can pick up for the first 10 seconds.
 * - Auto-despawn after 60 seconds.
 * - Replication for multiplayer visibility.
 */
UCLASS(BlueprintType)
class RAGNAROKUE_API AROLootActor : public AActor
{
	GENERATED_BODY()

public:
	AROLootActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	/**
	 * Initialize the loot actor with item data.
	 * @param InItemID The item database ID.
	 * @param InAmount The stack amount.
	 * @param InOwner The actor that has exclusive pickup rights.
	 */
	void InitializeLoot(int32 InItemID, int32 InAmount, AActor* InOwner);

	/**
	 * Check if a character can pick up this loot.
	 * During the ownership period, only the owner can pick up.
	 * @param Character The character trying to pick up.
	 * @return True if pickup is allowed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Loot")
	bool CanPickup(AActor* Character) const;

	/** Get the item ID of this loot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Loot")
	int32 GetItemID() const { return ItemID; }

	/** Get the stack amount. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Loot")
	int32 GetAmount() const { return Amount; }

	/** Get the remaining time before this loot despawns. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Loot")
	float GetRemainingDespawnTime() const;

	/** Check if the ownership period has expired (anyone can pick up). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Loot")
	bool IsOwnershipExpired() const;

protected:
	/** Called when the despawn timer expires. */
	UFUNCTION()
	void OnDespawnTimerExpired();

	/** Called when the ownership period expires. */
	UFUNCTION()
	void OnOwnershipTimerExpired();

	// ---- Replicated Properties ----

	/** The item database ID. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Loot")
	int32 ItemID;

	/** Stack amount. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Loot")
	int32 Amount;

	/** The actor that has exclusive pickup rights during ownership period. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Loot")
	TWeakObjectPtr<AActor> LootOwner;

	/** Whether the ownership period has expired. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Loot")
	bool bOwnershipExpired;

	/** Scene root component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	USceneComponent* SceneRoot;

private:
	/** Time in seconds before the loot despawns. */
	static constexpr float DESPAWN_TIME = 60.0f;

	/** Time in seconds that the killer has exclusive pickup rights. */
	static constexpr float OWNERSHIP_DURATION = 10.0f;

	/** World time when this loot was spawned. */
	float SpawnTime;

	/** Timer handles. */
	FTimerHandle DespawnTimerHandle;
	FTimerHandle OwnershipTimerHandle;
};
