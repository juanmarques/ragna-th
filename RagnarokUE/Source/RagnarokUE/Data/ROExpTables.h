// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "ROExpTables.generated.h"

/**
 * UROExpTables
 * Static function library providing the actual Ragnarok Online EXP tables.
 * Contains base EXP (levels 1-99), job EXP by tier, and stat point calculations.
 */
UCLASS()
class RAGNAROKUE_API UROExpTables : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Get the base EXP required to advance from Level to Level+1.
	 * @param Level Current base level (1-98). Level 99 returns 0 (max).
	 * @return EXP needed, or 0 if at max level.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|EXP")
	static int64 GetBaseExpRequired(int32 Level);

	/**
	 * Get the job EXP required to advance from JobLevel to JobLevel+1.
	 * @param JobLevel Current job level.
	 * @param Tier Job tier (affects table used and max level).
	 * @return EXP needed, or 0 if at max job level.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|EXP")
	static int64 GetJobExpRequired(int32 JobLevel, EROJobTier Tier);

	/**
	 * Get the number of stat points earned when reaching a given level.
	 * Formula: floor((Level - 1) / 5) + 3
	 * Level 1 returns 0 (no points for being level 1).
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|EXP")
	static int32 GetStatPointsForLevel(int32 Level);

	/**
	 * Get total cumulative stat points earned from leveling between two levels (inclusive).
	 * @param FromLevel Starting level (exclusive - points earned AFTER this level).
	 * @param ToLevel Ending level (inclusive).
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|EXP")
	static int32 GetTotalStatPoints(int32 FromLevel, int32 ToLevel);

	/**
	 * Get total cumulative base EXP from level 1 to a given level.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|EXP")
	static int64 GetTotalBaseExp(int32 Level);

private:
	/** Internal: returns reference to the base EXP table array (index 0 = level 1). */
	static const TArray<int64>& GetBaseExpTable();

	/** Internal: returns reference to the 1st class job EXP table. */
	static const TArray<int64>& GetFirstClassJobExpTable();

	/** Internal: returns reference to the 2nd class job EXP table. */
	static const TArray<int64>& GetSecondClassJobExpTable();

	/** Internal: returns reference to the transcendent 2nd class job EXP table. */
	static const TArray<int64>& GetTransSecondClassJobExpTable();

	/** Internal: returns reference to the novice job EXP table. */
	static const TArray<int64>& GetNoviceJobExpTable();
};
