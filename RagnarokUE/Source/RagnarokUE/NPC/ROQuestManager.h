// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "RagnarokUE/NPC/ROQuestData.h"
#include "ROQuestManager.generated.h"

class AROCharacterBase;
class UROQuestObjective;

/**
 * FROQuestReward
 * Defines rewards given upon quest completion.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROQuestReward
{
	GENERATED_BODY()

	/** Item ID to reward (0 = no item reward). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
	int32 ItemID = 0;

	/** Amount of the item to reward. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
	int32 Amount = 0;

	/** Base experience rewarded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
	int64 BaseExp = 0;

	/** Job experience rewarded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
	int64 JobExp = 0;

	/** Zeny (currency) rewarded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
	int32 Zeny = 0;
};

/**
 * FROQuestDefinition
 * Static definition of a quest: objectives, rewards, prerequisites.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROQuestDefinition
{
	GENERATED_BODY()

	/** Unique quest identifier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 QuestID = 0;

	/** Display name of the quest. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText QuestName;

	/** Full description of the quest. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText Description;

	/** List of objectives that must be completed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FROQuestObjectiveData> Objectives;

	/** Rewards given upon quest completion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FROQuestReward> Rewards;

	/** Quest IDs that must be completed before this quest can be accepted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<int32> PrerequisiteQuests;

	/** Minimum base level required to accept this quest. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 RequiredLevel = 1;

	/** Required job class (Novice = no specific requirement). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EROJobClass RequiredJobClass = EROJobClass::Novice;

	/** Required job level (for job change quests). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 RequiredJobLevel = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestAccepted, int32, QuestID, AROCharacterBase*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestCompleted, int32, QuestID, AROCharacterBase*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestAbandoned, int32, QuestID, AROCharacterBase*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnQuestObjectiveUpdated, int32, QuestID, int32, ObjectiveIndex, int32, NewProgress);

/**
 * UROQuestManager
 * Game instance subsystem that manages the quest database and per-player quest tracking.
 * Provides functions to accept, progress, complete, and abandon quests.
 */
UCLASS()
class RAGNAROKUE_API UROQuestManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- Quest Database ----

	/** All quest definitions keyed by QuestID. */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	TMap<int32, FROQuestDefinition> QuestDatabase;

	/** Register a quest definition into the database. */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void RegisterQuest(const FROQuestDefinition& QuestDef);

	/** Get a quest definition by ID. Returns nullptr if not found. */
	const FROQuestDefinition* GetQuestDefinition(int32 QuestID) const;

	// ---- Per-Player Quest State ----

	/**
	 * Accept a quest for a player. Validates prerequisites and level requirements.
	 * @return True if the quest was successfully accepted.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool AcceptQuest(int32 QuestID, AROCharacterBase* Player);

	/**
	 * Complete a quest for a player. Awards rewards and marks as complete.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void CompleteQuest(int32 QuestID, AROCharacterBase* Player);

	/**
	 * Abandon an active quest for a player.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void AbandonQuest(int32 QuestID, AROCharacterBase* Player);

	/**
	 * Check if a quest is fully complete for a player.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	bool IsQuestComplete(int32 QuestID, AROCharacterBase* Player) const;

	/**
	 * Check if a player has a specific quest active (accepted but not completed).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	bool HasActiveQuest(int32 QuestID, AROCharacterBase* Player) const;

	/**
	 * Check if a player has completed a specific quest (in completed list).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	bool HasCompletedQuest(int32 QuestID, AROCharacterBase* Player) const;

	/**
	 * Get the active quest state for a player.
	 * @return The FROActiveQuest data, or a default if not found.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FROActiveQuest GetQuestProgress(int32 QuestID, AROCharacterBase* Player) const;

	/**
	 * Get all active quests for a player.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FROActiveQuest> GetAllActiveQuests(AROCharacterBase* Player) const;

	/**
	 * Get all completed quest IDs for a player.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<int32> GetAllCompletedQuests(AROCharacterBase* Player) const;

	// ---- Event Notifications (call from game systems) ----

	/** Notify that a monster was killed (updates all active quest objectives). */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void NotifyMonsterKilled(AROCharacterBase* Player, int32 MonsterID);

	/** Notify that an item was collected. */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void NotifyItemCollected(AROCharacterBase* Player, int32 ItemID, int32 CurrentCount);

	/** Notify that an NPC was interacted with. */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void NotifyNPCInteracted(AROCharacterBase* Player, int32 NPCID);

	/** Notify that a location was reached. */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void NotifyLocationReached(AROCharacterBase* Player, FVector Location);

	/** Notify that a skill was used. */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void NotifySkillUsed(AROCharacterBase* Player, int32 SkillID);

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestAccepted OnQuestAccepted;

	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestCompleted OnQuestCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestAbandoned OnQuestAbandoned;

	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestObjectiveUpdated OnQuestObjectiveUpdated;

	// ---- Built-in Quest IDs ----

	static constexpr int32 QUEST_NOVICE_TRAINING = 1000;
	static constexpr int32 QUEST_SWORDSMAN_CHANGE = 1001;
	static constexpr int32 QUEST_MAGE_CHANGE = 1002;
	static constexpr int32 QUEST_ARCHER_CHANGE = 1003;
	static constexpr int32 QUEST_THIEF_CHANGE = 1004;
	static constexpr int32 QUEST_MERCHANT_CHANGE = 1005;
	static constexpr int32 QUEST_ACOLYTE_CHANGE = 1006;

private:
	/** Get a unique key for player state tracking. */
	uint32 GetPlayerKey(AROCharacterBase* Player) const;

	/**
	 * Per-player active quests. Outer key is player unique ID,
	 * inner key is quest ID mapping to active quest state.
	 */
	TMap<uint32, TMap<int32, FROActiveQuest>> PlayerActiveQuests;

	/** Per-player completed quest IDs. */
	TMap<uint32, TArray<int32>> PlayerCompletedQuests;

	/** Per-player quest objective trackers. Outer: player key, middle: quest ID, inner: objective array. */
	TMap<uint32, TMap<int32, TArray<TObjectPtr<UROQuestObjective>>>> PlayerObjectiveTrackers;

	/** Register all built-in Prontera quest definitions. */
	void RegisterBuiltInQuests();

	/** Create objective trackers for a quest. */
	void CreateObjectiveTrackers(uint32 PlayerKey, int32 QuestID, const FROQuestDefinition& QuestDef);

	/** Award quest rewards to a player. */
	void AwardQuestRewards(AROCharacterBase* Player, const FROQuestDefinition& QuestDef);

	/** Check if a player meets the prerequisites for a quest. */
	bool MeetsPrerequisites(int32 QuestID, AROCharacterBase* Player) const;

	/** Update active quest state from objective trackers. */
	void SyncQuestProgress(uint32 PlayerKey, int32 QuestID);
};
