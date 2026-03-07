// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROMonsterDatabase.generated.h"

/**
 * UROMonsterDatabase
 * Game instance subsystem that stores all monster definitions.
 * Populated at initialization with Prontera region monsters.
 * In production, would load from data tables.
 */
UCLASS()
class RAGNAROKUE_API UROMonsterDatabase : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Get monster data by ID.
	 * @param MonsterID The database ID of the monster.
	 * @param OutData Output parameter filled with monster data.
	 * @return True if monster was found in database.
	 */
	UFUNCTION(BlueprintCallable, Category = "Monster Database")
	bool GetMonsterData(int32 MonsterID, FROMonsterData& OutData) const;

	/**
	 * Get all monster IDs in the database.
	 */
	UFUNCTION(BlueprintCallable, Category = "Monster Database")
	TArray<int32> GetAllMonsterIDs() const;

	/**
	 * Check if a monster ID exists in the database.
	 */
	UFUNCTION(BlueprintCallable, Category = "Monster Database")
	bool HasMonster(int32 MonsterID) const;

	/**
	 * Get the number of monsters in the database.
	 */
	UFUNCTION(BlueprintCallable, Category = "Monster Database")
	int32 GetMonsterCount() const { return MonsterDB.Num(); }

protected:
	/** The monster database: MonsterID -> FROMonsterData. */
	UPROPERTY()
	TMap<int32, FROMonsterData> MonsterDB;

	/** Populate the database with hardcoded Prontera region monsters. */
	void PopulatePronteraMonsters();

	/** Helper to register a monster entry. */
	void RegisterMonster(const FROMonsterData& Data);
};
