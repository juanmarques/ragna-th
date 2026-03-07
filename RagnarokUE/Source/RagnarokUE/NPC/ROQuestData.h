// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ROQuestData.generated.h"

/**
 * Types of quest objectives available in the Ragnarok Online quest system.
 */
UENUM(BlueprintType)
enum class EROQuestObjectiveType : uint8
{
	KillMonster		UMETA(DisplayName = "Kill Monster"),
	CollectItem		UMETA(DisplayName = "Collect Item"),
	TalkToNPC		UMETA(DisplayName = "Talk to NPC"),
	ReachLocation	UMETA(DisplayName = "Reach Location"),
	UseSkill		UMETA(DisplayName = "Use Skill")
};

/**
 * FROQuestObjectiveData
 * Defines a single objective within a quest: what needs to be done, how many times, and a description.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROQuestObjectiveData
{
	GENERATED_BODY()

	/** The type of objective. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
	EROQuestObjectiveType ObjectiveType = EROQuestObjectiveType::KillMonster;

	/** Target ID: MonsterID for kill, ItemID for collect, NPCID for talk, SkillID for use skill, 0 for location. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
	int32 TargetID = 0;

	/** How many times the objective must be completed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective", meta = (ClampMin = "1"))
	int32 RequiredAmount = 1;

	/** Human-readable description of this objective. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
	FText ObjectiveDescription;

	/** Optional location target (for ReachLocation type). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
	FVector TargetLocation = FVector::ZeroVector;

	/** Radius around TargetLocation that counts as "reached". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective", meta = (ClampMin = "0"))
	float LocationRadius = 500.0f;
};

/**
 * FROActiveQuest
 * Runtime state for a quest that a player has accepted. Tracks per-objective progress.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROActiveQuest
{
	GENERATED_BODY()

	/** The quest definition ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 QuestID = 0;

	/** Current progress count for each objective (parallel array to quest definition's Objectives). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<int32> ObjectiveProgress;

	/** Whether this quest has been fully completed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bIsComplete = false;

	FROActiveQuest() {}

	FROActiveQuest(int32 InQuestID, int32 NumObjectives)
		: QuestID(InQuestID), bIsComplete(false)
	{
		ObjectiveProgress.SetNumZeroed(NumObjectives);
	}
};
