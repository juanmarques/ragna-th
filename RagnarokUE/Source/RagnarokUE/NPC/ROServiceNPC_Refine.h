// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/NPC/RONPCBase.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROServiceNPC_Refine.generated.h"

class AROCharacterBase;
class UROInventoryComponent;
class URORefinementSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRefineResult, bool, bSuccess, int32, NewRefineLevel, int32, ItemID);

/**
 * AROServiceNPC_Refine
 * Refinement NPC that allows players to upgrade equipment using ores and Zeny.
 * Uses the RORefinementSystem for success rate calculations and refinement logic.
 */
UCLASS(ClassGroup=(RagnarokUE), Blueprintable)
class RAGNAROKUE_API AROServiceNPC_Refine : public ARONPCBase
{
	GENERATED_BODY()

public:
	AROServiceNPC_Refine();

	// ---- Delegates ----

	/** Broadcast when a refinement attempt completes. */
	UPROPERTY(BlueprintAssignable, Category = "Refine|Events")
	FOnRefineResult OnRefineResult;

	// ---- Configuration ----

	/** Maximum refine level this NPC supports. Standard is 10, special NPCs may allow higher. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Refine", meta = (ClampMin = "1", ClampMax = "20"))
	int32 MaxRefineLevel;

	// ---- Interaction ----

	/** Override: Opens the refinement UI. */
	virtual void OnInteract_Implementation(AROCharacterBase* Interactor) override;

	// ---- Server RPCs ----

	/**
	 * Attempt to refine an item in the player's inventory.
	 * Validates the item is refinable, player has required ore and Zeny,
	 * and the item is under the max refine level.
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRefineItem(int32 InventorySlot);

	// ---- Queries ----

	/**
	 * Check if an item can be refined by this NPC.
	 * @param Item The item to check.
	 * @param WeaponLevel The weapon level (0 = armor, 1-4 = weapon levels).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Refine")
	bool CanRefineItem(const FROItemInstance& Item, int32 WeaponLevel) const;

	/**
	 * Get the success rate for refining a specific item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Refine")
	float GetRefineSuccessRate(const FROItemInstance& Item, int32 WeaponLevel) const;

	/**
	 * Get the Zeny cost for refining a specific item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Refine")
	int32 GetRefineCost(const FROItemInstance& Item) const;

protected:
	/** Per-player refine interaction map: PlayerID -> Character reference. */
	TMap<int32, TWeakObjectPtr<AROCharacterBase>> RefineUsers;

	/** Get the calling player's character for a Server RPC on this NPC. */
	AROCharacterBase* GetRefineUserForCaller() const;

	/** Called when a player interacting with the refine NPC is destroyed (disconnect). */
	UFUNCTION()
	void OnRefinePlayerDestroyed(AActor* DestroyedActor);

	/**
	 * Determine the weapon level for an item.
	 * 0 = armor, 1-4 = weapon levels.
	 * In a full implementation, this would query the item database.
	 * For now, we infer from item ID ranges.
	 */
	int32 DetermineWeaponLevel(int32 ItemID) const;

	/** Check if an item ID represents refinable equipment. */
	bool IsRefinableEquipment(int32 ItemID) const;
};
