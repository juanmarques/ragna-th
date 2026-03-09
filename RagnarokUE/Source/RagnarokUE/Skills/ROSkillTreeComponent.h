// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROSkillTreeComponent.generated.h"

/**
 * FROSkillPrerequisite
 * Defines a prerequisite skill and level needed to learn another skill.
 */
USTRUCT(BlueprintType)
struct FROSkillPrerequisite
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	int32 RequiredSkillID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	int32 RequiredLevel = 0;
};

/**
 * FROSkillDefinition
 * Complete definition for a skill in the skill tree.
 */
USTRUCT(BlueprintType)
struct FROSkillDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	int32 SkillID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	FName SkillName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	int32 MaxLevel = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	EROJobClass RequiredJob = EROJobClass::Novice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	TArray<FROSkillPrerequisite> Prerequisites;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree")
	FText Description;
};

/**
 * FROLearnedSkillEntry
 * Replicated struct representing a single learned skill.
 */
USTRUCT()
struct FROLearnedSkillEntry
{
	GENERATED_BODY()

	UPROPERTY()
	int32 SkillID = 0;

	UPROPERTY()
	int32 Level = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillLearned, int32, SkillID, int32, NewLevel);

/**
 * UROSkillTreeComponent
 * Manages the character's learned skills, skill points, and prerequisite validation.
 * Supports server-authoritative skill learning with replication.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RAGNAROKUE_API UROSkillTreeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UROSkillTreeComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Skill Management ---

	/**
	 * Server-authoritative: learn or level up a skill.
	 * Validates prerequisites, checks available skill points, and applies the change.
	 */
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ServerLearnSkill(int32 SkillID);

	/**
	 * Check if all prerequisites are met for a skill at a desired level.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|SkillTree")
	bool CheckPrerequisites(int32 SkillID, int32 DesiredLevel) const;

	/**
	 * Get all available skill definitions for a given job class.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|SkillTree")
	TArray<FROSkillDefinition> GetAvailableSkillsForJob(EROJobClass Job) const;

	/**
	 * Get the current level of a learned skill. Returns 0 if not learned.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|SkillTree")
	int32 GetSkillLevel(int32 SkillID) const;

	/**
	 * Get all learned skills as a map of SkillID -> Level.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|SkillTree")
	TMap<int32, int32> GetAllLearnedSkills() const;

	/**
	 * Get available skill points.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|SkillTree")
	int32 GetAvailableSkillPoints() const { return SkillPoints; }

	/**
	 * Add skill points (called on level up).
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|SkillTree")
	void AddSkillPoints(int32 Points);

	/**
	 * Reset all skills and refund skill points.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|SkillTree")
	void ResetSkills();

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "RO|SkillTree|Events")
	FOnSkillLearned OnSkillLearned;

	// --- Properties ---

	/** The character's current job class (determines available skills). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "RO|SkillTree")
	EROJobClass CurrentJobClass;

protected:
	virtual void BeginPlay() override;

	/** Initialize the skill definition database. Override to add custom skills. */
	virtual void InitializeSkillDefinitions();

private:
	/** Learned skills (replicated as array, accessed as map internally). */
	UPROPERTY(Replicated)
	TArray<FROLearnedSkillEntry> LearnedSkillEntries;

	/** Runtime map for fast lookup (rebuilt from LearnedSkillEntries). */
	TMap<int32, int32> LearnedSkills;

	/** Available skill points to spend. */
	UPROPERTY(Replicated)
	int32 SkillPoints;

	/** Total skill points ever spent (for reset refunding). */
	UPROPERTY()
	int32 TotalSkillPointsSpent;

	/** Database of all skill definitions. */
	UPROPERTY()
	TArray<FROSkillDefinition> SkillDefinitions;

	/** Find a skill definition by ID. Returns nullptr if not found. */
	const FROSkillDefinition* FindSkillDefinition(int32 SkillID) const;
};
