// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "ROStructs.generated.h"

/**
 * FROStatBlock
 * Holds all 6 base stats, their bonus values, and computed totals.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROStatBlock
{
	GENERATED_BODY()

	// ---- Base Stats (allocated by player) ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Base")
	int32 BaseSTR = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Base")
	int32 BaseAGI = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Base")
	int32 BaseVIT = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Base")
	int32 BaseINT = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Base")
	int32 BaseDEX = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Base")
	int32 BaseLUK = 1;

	// ---- Bonus Stats (from equipment, buffs, cards) ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Bonus")
	int32 BonusSTR = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Bonus")
	int32 BonusAGI = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Bonus")
	int32 BonusVIT = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Bonus")
	int32 BonusINT = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Bonus")
	int32 BonusDEX = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Bonus")
	int32 BonusLUK = 0;

	// ---- Computed Totals (Base + Bonus) ----

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Total")
	int32 TotalSTR = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Total")
	int32 TotalAGI = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Total")
	int32 TotalVIT = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Total")
	int32 TotalINT = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Total")
	int32 TotalDEX = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Total")
	int32 TotalLUK = 1;

	/** Recalculate all total stats from base + bonus. */
	void RecalculateTotals()
	{
		TotalSTR = BaseSTR + BonusSTR;
		TotalAGI = BaseAGI + BonusAGI;
		TotalVIT = BaseVIT + BonusVIT;
		TotalINT = BaseINT + BonusINT;
		TotalDEX = BaseDEX + BonusDEX;
		TotalLUK = BaseLUK + BonusLUK;
	}

	/** Get a base stat value by enum. */
	int32 GetBaseStat(EROStat Stat) const
	{
		switch (Stat)
		{
		case EROStat::STR:      return BaseSTR;
		case EROStat::AGI:      return BaseAGI;
		case EROStat::VIT:      return BaseVIT;
		case EROStat::INT_STAT: return BaseINT;
		case EROStat::DEX:      return BaseDEX;
		case EROStat::LUK:      return BaseLUK;
		default:                return 0;
		}
	}

	/** Get a total stat value by enum. */
	int32 GetTotalStat(EROStat Stat) const
	{
		switch (Stat)
		{
		case EROStat::STR:      return TotalSTR;
		case EROStat::AGI:      return TotalAGI;
		case EROStat::VIT:      return TotalVIT;
		case EROStat::INT_STAT: return TotalINT;
		case EROStat::DEX:      return TotalDEX;
		case EROStat::LUK:      return TotalLUK;
		default:                return 0;
		}
	}

	/** Set a base stat value by enum. */
	void SetBaseStat(EROStat Stat, int32 Value)
	{
		switch (Stat)
		{
		case EROStat::STR:      BaseSTR = Value; break;
		case EROStat::AGI:      BaseAGI = Value; break;
		case EROStat::VIT:      BaseVIT = Value; break;
		case EROStat::INT_STAT: BaseINT = Value; break;
		case EROStat::DEX:      BaseDEX = Value; break;
		case EROStat::LUK:      BaseLUK = Value; break;
		default: break;
		}
	}

	/** Add to a bonus stat value by enum. */
	void AddBonusStat(EROStat Stat, int32 Amount)
	{
		switch (Stat)
		{
		case EROStat::STR:      BonusSTR += Amount; break;
		case EROStat::AGI:      BonusAGI += Amount; break;
		case EROStat::VIT:      BonusVIT += Amount; break;
		case EROStat::INT_STAT: BonusINT += Amount; break;
		case EROStat::DEX:      BonusDEX += Amount; break;
		case EROStat::LUK:      BonusLUK += Amount; break;
		default: break;
		}
	}
};

/**
 * FROItemInstance
 * Represents a specific item instance in inventory with refine level, cards, and unique ID.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROItemInstance
{
	GENERATED_BODY()

	/** Database item ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 ItemID = 0;

	/** Stack amount (1 for equipment, 0 for empty/default slots). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Amount = 0;

	/** Refine level (+0 to +10, pre-renewal). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "0", ClampMax = "10"))
	int32 RefineLevel = 0;

	/** Card IDs socketed into this item (up to 4 slots). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<int32> CardSlots;

	/** Unique instance identifier for this specific item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FGuid UniqueID;

	FROItemInstance()
		: ItemID(0), Amount(0), RefineLevel(0)
	{
		// Don't generate a GUID for empty/default instances
		// UniqueID will be invalid (all zeros) for empty slots
	}

	/** Factory method to create a new item instance with a valid GUID. */
	static FROItemInstance CreateNew(int32 InItemID, int32 InAmount)
	{
		FROItemInstance Instance;
		Instance.ItemID = InItemID;
		Instance.Amount = InAmount;
		Instance.UniqueID = FGuid::NewGuid();
		return Instance;
	}

	bool IsValid() const
	{
		return ItemID > 0;
	}

	bool IsEquipment() const
	{
		// An item is equipment if it's non-stackable (Amount == 1) with a valid item ID
		return Amount == 1 && ItemID > 0;
	}

	bool operator==(const FROItemInstance& Other) const
	{
		return UniqueID == Other.UniqueID;
	}

	bool operator!=(const FROItemInstance& Other) const
	{
		return UniqueID != Other.UniqueID;
	}
};

/**
 * FROSkillInfo
 * Runtime data for a learned skill including level, costs, and cast times.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROSkillInfo
{
	GENERATED_BODY()

	/** Database skill ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	int32 SkillID = 0;

	/** Internal skill name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FName SkillName;

	/** Current skill level (0 = not learned). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	int32 CurrentLevel = 0;

	/** Maximum learnable level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	int32 MaxLevel = 0;

	/** SP cost at each skill level (index 0 = level 1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TArray<int32> SPCostPerLevel;

	/** Variable cast time in seconds (reduced by DEX/INT). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float VariableCastTime = 0.0f;

	/** Fixed cast time in seconds (not reduced by stats). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float FixedCastTime = 0.0f;

	/** Global cooldown in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float Cooldown = 0.0f;

	/** Skill's innate element. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	EROElement Element = EROElement::Neutral;

	/** Get SP cost for the current level. Returns 0 if invalid. */
	int32 GetCurrentSPCost() const
	{
		const int32 Index = CurrentLevel - 1;
		if (SPCostPerLevel.IsValidIndex(Index))
		{
			return SPCostPerLevel[Index];
		}
		return 0;
	}
};

/**
 * FROMonsterSpawnInfo
 * Defines a monster spawn point with respawn timing.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROMonsterSpawnInfo
{
	GENERATED_BODY()

	/** Database monster ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 MonsterID = 0;

	/** Number of monsters to maintain at this spawn point. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "1"))
	int32 Count = 1;

	/** Center of the spawn area. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FVector SpawnCenter = FVector::ZeroVector;

	/** Radius of the spawn area around SpawnCenter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float SpawnRadius = 500.0f;

	/** Base time in seconds before a killed monster respawns. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float RespawnDelay = 5.0f;

	/** Random variance added to respawn delay (+/- this value in seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float RespawnVariance = 1.0f;
};

/**
 * FRODropInfo
 * Defines a single drop entry for a monster's drop table.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FRODropInfo
{
	GENERATED_BODY()

	/** Database item ID of the dropped item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	int32 ItemID = 0;

	/** Drop rate as a percentage (0.01 = 0.01%, 100.0 = 100%). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop", meta = (ClampMin = "0.01", ClampMax = "100.0"))
	float DropRate = 1.0f;
};

/**
 * FROJobClassInfo
 * Metadata about a job class: tier, advancement paths, and max job level.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROJobClassInfo
{
	GENERATED_BODY()

	/** The job class this info describes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job")
	EROJobClass JobClass = EROJobClass::Novice;

	/** Which advancement tier this job belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job")
	EROJobTier Tier = EROJobTier::Novice_Tier;

	/** Job classes this class can advance to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job")
	TArray<EROJobClass> CanChangeTo;

	/** Maximum job level for this class. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job")
	int32 MaxJobLevel = 50;

	/** User-facing display name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job")
	FString DisplayName;
};

/**
 * FROMapInfo
 * Metadata about a game map/zone.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROMapInfo
{
	GENERATED_BODY()

	/** Internal map identifier (e.g., "prontera", "prt_fild08"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	FName MapID;

	/** User-facing display name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	FString DisplayName;

	/** Whether PvP is enabled on this map. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	bool bIsPvP = false;

	/** Whether this map is indoors (affects skills, weather, etc.). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	bool bIsIndoor = false;

	/** Default elemental property of the map. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	EROElement MapElement = EROElement::Neutral;
};

/**
 * FROMonsterSkillEntry
 * A single skill that a monster can use, with cooldown and conditions.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROMonsterSkillEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MonsterSkill")
	int32 SkillID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MonsterSkill")
	int32 SkillLevel = 1;

	/** Cooldown in seconds between uses. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MonsterSkill")
	float Cooldown = 10.0f;

	/** Cast time in seconds (0 = instant). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MonsterSkill")
	float CastTime = 0.0f;

	/** Chance to use this skill when conditions are met (0-100). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MonsterSkill")
	float UseChance = 50.0f;

	/** HP percentage threshold below which the monster may use this skill. 100 = always eligible. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MonsterSkill")
	float HPThresholdPercent = 100.0f;

	/** Skill range. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MonsterSkill")
	float Range = 300.0f;
};

/**
 * FROMonsterData
 * Complete static data for a monster type, loaded from data tables or populated in code.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROMonsterData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	int32 MonsterID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	FName MonsterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	int32 HP = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	int32 ATKMin = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	int32 ATKMax = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	int32 MATK = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	int32 DEF = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	int32 MDEF = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	int32 HIT = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	int32 FLEE = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Element")
	EROElement Element = EROElement::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Element")
	EROElementLevel ElementLevel = EROElementLevel::Level1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Type")
	EROMonsterSize Size = EROMonsterSize::Small;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Type")
	EROMonsterRace Race = EROMonsterRace::Formless;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|AI")
	EROMonsterBehavior Behavior = EROMonsterBehavior::Passive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Reward")
	int64 BaseExpReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Reward")
	int64 JobExpReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Combat")
	float AggroRange = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Combat")
	float ChaseRange = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Combat")
	float AttackRange = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Combat")
	float AttackSpeed = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Drops")
	TArray<FRODropInfo> DropTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Skills")
	TArray<FROMonsterSkillEntry> Skills;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Type")
	bool bIsMVP = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Type")
	bool bIsBoss = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Spawn")
	float RespawnTime = 5.0f;
};
