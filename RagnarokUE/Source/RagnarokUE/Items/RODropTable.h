// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "RODropTable.generated.h"

/**
 * FRODropTableEntry
 * Wrapper struct for TMap value since TMap<int32, TArray<>> requires a USTRUCT wrapper.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FRODropTableEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	TArray<FRODropInfo> Drops;
};

/**
 * URODropTable
 * Static data class that maps monster IDs to their drop tables.
 * Used by the loot system to determine what items drop when a monster is killed.
 */
UCLASS(BlueprintType)
class RAGNAROKUE_API URODropTable : public UObject
{
	GENERATED_BODY()

public:
	URODropTable();

	/**
	 * Get the drop table for a specific monster.
	 * @param MonsterID The database ID of the monster.
	 * @return Array of drop info entries. Empty array if no drops defined.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Drop Table")
	TArray<FRODropInfo> GetDropsForMonster(int32 MonsterID) const;

	/**
	 * Roll all drops for a monster kill, applying drop rate modifiers.
	 * Each drop entry is rolled independently against its drop rate.
	 * @param MonsterID The database ID of the killed monster.
	 * @param DropRateModifier Multiplier on drop rates (1.0 = normal, 2.0 = double rates).
	 * @return Array of item IDs that successfully dropped.
	 */
	UFUNCTION(BlueprintCallable, Category = "Drop Table")
	TArray<int32> RollDrops(int32 MonsterID, float DropRateModifier = 1.0f) const;

	/**
	 * Register a drop table for a monster.
	 * @param MonsterID The monster's database ID.
	 * @param Drops Array of drop entries.
	 */
	UFUNCTION(BlueprintCallable, Category = "Drop Table")
	void RegisterDropTable(int32 MonsterID, const TArray<FRODropInfo>& Drops);

	/**
	 * Load drop tables from monster data structs.
	 * @param MonsterDataArray Array of monster data with embedded drop tables.
	 */
	void LoadFromMonsterData(const TArray<FROMonsterData>& MonsterDataArray);

	/** Check if a monster has any drops defined. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Drop Table")
	bool HasDrops(int32 MonsterID) const;

private:
	/** Master drop table: MonsterID -> Array of drop entries. */
	UPROPERTY()
	TMap<int32, FRODropTableEntry> MonsterDropTables;
};
