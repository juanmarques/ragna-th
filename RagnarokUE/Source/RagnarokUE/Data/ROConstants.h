// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * ROConstants
 * Game-wide constants matching original Ragnarok Online pre-renewal values.
 */
namespace ROConstants
{
	// ---- Level Caps ----

	/** Maximum base level for all classes. */
	constexpr int32 MaxBaseLevel = 99;

	/** Maximum job level for 1st class jobs (Swordsman, Mage, etc.). */
	constexpr int32 MaxJobLevel1st = 50;

	/** Maximum job level for 2nd class jobs (Knight, Wizard, etc.). */
	constexpr int32 MaxJobLevel2nd = 50;

	/** Maximum job level for transcendent 2nd class jobs (Lord Knight, High Wizard, etc.). */
	constexpr int32 MaxJobLevelTrans = 70;

	/** Maximum job level for novice class. */
	constexpr int32 MaxJobLevelNovice = 10;

	// ---- Stat Limits ----

	/** Maximum base stat value a player can allocate. */
	constexpr int32 MaxStats = 99;

	/** Minimum base stat value (all stats start at 1). */
	constexpr int32 MinStats = 1;

	// ---- Equipment ----

	/** Maximum refine level for equipment (+0 to +10, pre-renewal). */
	constexpr int32 MaxRefineLevel = 10;

	/** Maximum number of card slots in a single piece of equipment. */
	constexpr int32 MaxCardSlots = 4;

	// ---- Social ----

	/** Maximum number of members in a party. */
	constexpr int32 MaxPartySize = 12;

	/** Maximum number of members in a guild. */
	constexpr int32 MaxGuildSize = 56;

	// ---- Inventory / Storage ----

	/** Maximum inventory slot count. */
	constexpr int32 MaxInventorySlots = 100;

	/** Maximum storage slot count. */
	constexpr int32 MaxStorageSlots = 600;

	// ---- Weight ----

	/** Base weight capacity before STR bonuses. */
	constexpr int32 BaseMaxWeight = 2000;

	/** Weight capacity added per point of STR (in weight units * 10). */
	constexpr int32 WeightPerSTR = 30;

	// ---- Currency ----

	/** Maximum zeny a character can hold. */
	constexpr int64 MaxZeny = 1000000000LL;

	// ---- Timing ----

	/** Minimum attack speed delay in milliseconds. */
	constexpr float MinAttackDelay = 0.0f;

	/** Maximum ASPD value (cap). */
	constexpr float MaxASPD = 190.0f;

	/** Natural HP recovery interval in seconds. */
	constexpr float HPRecoveryInterval = 10.0f;

	/** Natural SP recovery interval in seconds. */
	constexpr float SPRecoveryInterval = 10.0f;

	// ---- Combat ----

	/** Minimum physical damage dealt. */
	constexpr int32 MinDamage = 1;

	/** Maximum hit rate percentage. */
	constexpr int32 MaxHitRate = 100;

	/** Minimum hit rate percentage. */
	constexpr int32 MinHitRate = 5;

	/** Perfect dodge cap. */
	constexpr int32 MaxPerfectDodge = 100;

	/** Critical rate cap percentage. */
	constexpr float MaxCritRate = 100.0f;

	// ---- Stat Point Cost ----

	/**
	 * Calculate the stat point cost to increase a stat from CurrentValue to CurrentValue+1.
	 * Formula: floor((CurrentValue - 1) / 10) + 2
	 * This means: stats 1-10 cost 2, 11-20 cost 3, 21-30 cost 4, ... 91-99 cost 11.
	 */
	inline int32 GetStatPointCost(int32 CurrentValue)
	{
		return (CurrentValue - 1) / 10 + 2;
	}

	/**
	 * Calculate the maximum weight capacity for a given STR value.
	 * Formula: BaseMaxWeight + STR * WeightPerSTR
	 */
	inline int32 CalculateMaxWeight(int32 STR)
	{
		return BaseMaxWeight + STR * WeightPerSTR;
	}

	/**
	 * Get the stat points earned when leveling up to a given level.
	 * Formula: floor((Level - 1) / 5) + 3
	 * Level 1 earns 3, Levels 2-5 earn 3, Levels 6-10 earn 4, etc.
	 */
	inline int32 GetStatPointsForLevel(int32 Level)
	{
		if (Level <= 1) return 0;
		return (Level - 1) / 5 + 3;
	}

	/**
	 * Get total cumulative stat points earned from level 1 to Level.
	 */
	inline int32 GetTotalStatPointsAtLevel(int32 Level)
	{
		int32 Total = 0;
		for (int32 L = 2; L <= Level; ++L)
		{
			Total += GetStatPointsForLevel(L);
		}
		return Total;
	}
}
