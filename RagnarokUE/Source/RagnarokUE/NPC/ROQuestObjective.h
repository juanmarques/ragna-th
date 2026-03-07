// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "RagnarokUE/NPC/ROQuestData.h"
#include "ROQuestObjective.generated.h"

/**
 * UROQuestObjective
 * Tracks progress for a single quest objective.
 * Receives notifications from game events (kills, item pickups, NPC talks, etc.)
 * and updates progress accordingly.
 */
UCLASS(BlueprintType)
class RAGNAROKUE_API UROQuestObjective : public UObject
{
	GENERATED_BODY()

public:
	UROQuestObjective();

	/**
	 * Initialize this objective tracker from quest objective data.
	 * @param InObjectiveData The static objective definition.
	 * @param InObjectiveIndex Index of this objective within the parent quest.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
	void Initialize(const FROQuestObjectiveData& InObjectiveData, int32 InObjectiveIndex);

	/**
	 * Called when a monster is killed. Increments progress if matching.
	 * @param MonsterID The database ID of the killed monster.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
	void OnMonsterKilled(int32 MonsterID);

	/**
	 * Called when an item is collected. Checks collection objectives.
	 * @param ItemID The database ID of the collected item.
	 * @param CurrentCount The player's current total count of this item.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
	void OnItemCollected(int32 ItemID, int32 CurrentCount);

	/**
	 * Called when a player interacts with an NPC. Checks talk objectives.
	 * @param NPCID The database ID of the NPC interacted with.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
	void OnNPCInteracted(int32 NPCID);

	/**
	 * Called when the player reaches a location. Checks location objectives.
	 * @param PlayerLocation The player's current world location.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
	void OnLocationReached(FVector PlayerLocation);

	/**
	 * Called when a skill is used. Checks skill use objectives.
	 * @param SkillID The database ID of the skill used.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
	void OnSkillUsed(int32 SkillID);

	/** Check if this objective is complete (progress >= required amount). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Objective")
	bool CheckCompletion() const;

	/** Get a human-readable progress string (e.g., "5/10 Porings killed"). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Objective")
	FText GetProgressText() const;

	/** Get the current progress count. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Objective")
	int32 GetCurrentProgress() const { return CurrentProgress; }

	/** Get the required amount for completion. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Objective")
	int32 GetRequiredAmount() const { return ObjectiveData.RequiredAmount; }

	/** Get the objective type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Objective")
	EROQuestObjectiveType GetObjectiveType() const { return ObjectiveData.ObjectiveType; }

	/** Get the objective index within the parent quest. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Objective")
	int32 GetObjectiveIndex() const { return ObjectiveIndex; }

	/** Set progress directly (for loading saved state). */
	UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
	void SetProgress(int32 NewProgress);

protected:
	/** The static objective definition. */
	UPROPERTY(BlueprintReadOnly, Category = "Quest|Objective")
	FROQuestObjectiveData ObjectiveData;

	/** Current progress towards completion. */
	UPROPERTY(BlueprintReadOnly, Category = "Quest|Objective")
	int32 CurrentProgress;

	/** Index of this objective within the parent quest. */
	UPROPERTY(BlueprintReadOnly, Category = "Quest|Objective")
	int32 ObjectiveIndex;

	/** Whether this objective has been initialized. */
	bool bInitialized;
};
