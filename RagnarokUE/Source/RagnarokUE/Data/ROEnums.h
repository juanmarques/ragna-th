// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ROEnums.generated.h"

/**
 * Primary character stats in Ragnarok Online.
 * INT_STAT is used instead of INT to avoid C++ keyword conflict.
 */
UENUM(BlueprintType)
enum class EROStat : uint8
{
	STR			UMETA(DisplayName = "STR"),
	AGI			UMETA(DisplayName = "AGI"),
	VIT			UMETA(DisplayName = "VIT"),
	INT_STAT	UMETA(DisplayName = "INT"),
	DEX			UMETA(DisplayName = "DEX"),
	LUK			UMETA(DisplayName = "LUK")
};

/**
 * All job classes in Ragnarok Online (Pre-Renewal through Transcendent).
 */
UENUM(BlueprintType)
enum class EROJobClass : uint8
{
	// Novice
	Novice				UMETA(DisplayName = "Novice"),

	// 1st Classes
	Swordsman			UMETA(DisplayName = "Swordsman"),
	Magician			UMETA(DisplayName = "Magician"),
	Archer				UMETA(DisplayName = "Archer"),
	Thief				UMETA(DisplayName = "Thief"),
	Merchant			UMETA(DisplayName = "Merchant"),
	Acolyte				UMETA(DisplayName = "Acolyte"),

	// 2nd Classes (2-1)
	Knight				UMETA(DisplayName = "Knight"),
	Crusader			UMETA(DisplayName = "Crusader"),
	Wizard				UMETA(DisplayName = "Wizard"),
	Sage				UMETA(DisplayName = "Sage"),
	Hunter				UMETA(DisplayName = "Hunter"),
	Bard				UMETA(DisplayName = "Bard"),
	Dancer				UMETA(DisplayName = "Dancer"),
	Assassin			UMETA(DisplayName = "Assassin"),
	Rogue				UMETA(DisplayName = "Rogue"),
	Blacksmith			UMETA(DisplayName = "Blacksmith"),
	Alchemist			UMETA(DisplayName = "Alchemist"),
	Priest				UMETA(DisplayName = "Priest"),
	Monk				UMETA(DisplayName = "Monk"),

	// Transcendent 2nd Classes
	LordKnight			UMETA(DisplayName = "Lord Knight"),
	Paladin				UMETA(DisplayName = "Paladin"),
	HighWizard			UMETA(DisplayName = "High Wizard"),
	Professor			UMETA(DisplayName = "Professor"),
	Sniper				UMETA(DisplayName = "Sniper"),
	Minstrel			UMETA(DisplayName = "Minstrel"),
	Gypsy				UMETA(DisplayName = "Gypsy"),
	AssassinCross		UMETA(DisplayName = "Assassin Cross"),
	Stalker				UMETA(DisplayName = "Stalker"),
	Whitesmith			UMETA(DisplayName = "Whitesmith"),
	Creator				UMETA(DisplayName = "Creator"),
	HighPriest			UMETA(DisplayName = "High Priest"),
	Champion			UMETA(DisplayName = "Champion"),

	// Transcendent 1st Classes (High 1st)
	HighNovice			UMETA(DisplayName = "High Novice"),
	HighSwordsman		UMETA(DisplayName = "High Swordsman"),
	HighMagician		UMETA(DisplayName = "High Magician"),
	HighArcher			UMETA(DisplayName = "High Archer"),
	HighThief			UMETA(DisplayName = "High Thief"),
	HighMerchant		UMETA(DisplayName = "High Merchant"),
	HighAcolyte			UMETA(DisplayName = "High Acolyte")
};

/**
 * Elemental types in Ragnarok Online.
 */
UENUM(BlueprintType)
enum class EROElement : uint8
{
	Neutral		UMETA(DisplayName = "Neutral"),
	Water		UMETA(DisplayName = "Water"),
	Earth		UMETA(DisplayName = "Earth"),
	Fire		UMETA(DisplayName = "Fire"),
	Wind		UMETA(DisplayName = "Wind"),
	Poison		UMETA(DisplayName = "Poison"),
	Holy		UMETA(DisplayName = "Holy"),
	Shadow		UMETA(DisplayName = "Shadow"),
	Ghost		UMETA(DisplayName = "Ghost"),
	Undead		UMETA(DisplayName = "Undead")
};

/**
 * Elemental defense levels (1-4) for monsters and armor.
 */
UENUM(BlueprintType)
enum class EROElementLevel : uint8
{
	Level1		UMETA(DisplayName = "Level 1"),
	Level2		UMETA(DisplayName = "Level 2"),
	Level3		UMETA(DisplayName = "Level 3"),
	Level4		UMETA(DisplayName = "Level 4")
};

/**
 * Equipment slots available to characters.
 */
UENUM(BlueprintType)
enum class EROEquipSlot : uint8
{
	Weapon		UMETA(DisplayName = "Weapon"),
	Shield		UMETA(DisplayName = "Shield"),
	Armor		UMETA(DisplayName = "Armor"),
	Garment		UMETA(DisplayName = "Garment"),
	Footgear	UMETA(DisplayName = "Footgear"),
	AccessoryL	UMETA(DisplayName = "Accessory Left"),
	AccessoryR	UMETA(DisplayName = "Accessory Right"),
	HeadTop		UMETA(DisplayName = "Head Top"),
	HeadMid		UMETA(DisplayName = "Head Mid"),
	HeadLow		UMETA(DisplayName = "Head Low")
};

/**
 * Status effects (debuffs) in Ragnarok Online.
 */
UENUM(BlueprintType)
enum class EROStatusEffect : uint8
{
	Stun			UMETA(DisplayName = "Stun"),
	Poison			UMETA(DisplayName = "Poison"),
	Freeze			UMETA(DisplayName = "Freeze"),
	Stone			UMETA(DisplayName = "Stone Curse"),
	Sleep			UMETA(DisplayName = "Sleep"),
	Blind			UMETA(DisplayName = "Blind"),
	Silence			UMETA(DisplayName = "Silence"),
	Curse			UMETA(DisplayName = "Curse"),
	Bleeding		UMETA(DisplayName = "Bleeding"),
	Confusion		UMETA(DisplayName = "Confusion"),
	DeadlyPoison	UMETA(DisplayName = "Deadly Poison"),
	Fear			UMETA(DisplayName = "Fear"),
	Hallucination	UMETA(DisplayName = "Hallucination")
};

/**
 * Monster AI behavior flags.
 */
UENUM(BlueprintType)
enum class EROMonsterBehavior : uint8
{
	Passive			UMETA(DisplayName = "Passive"),
	Aggressive		UMETA(DisplayName = "Aggressive"),
	Assist			UMETA(DisplayName = "Assist"),
	CastSensor		UMETA(DisplayName = "Cast Sensor"),
	Detector		UMETA(DisplayName = "Detector"),
	Looter			UMETA(DisplayName = "Looter"),
	ChangeTarget	UMETA(DisplayName = "Change Target")
};

/**
 * Item categories in Ragnarok Online.
 */
UENUM(BlueprintType)
enum class EROItemType : uint8
{
	Weapon		UMETA(DisplayName = "Weapon"),
	Armor		UMETA(DisplayName = "Armor"),
	Card		UMETA(DisplayName = "Card"),
	Consumable	UMETA(DisplayName = "Consumable"),
	EtcItem		UMETA(DisplayName = "Etc Item"),
	Ammo		UMETA(DisplayName = "Ammo"),
	PetEgg		UMETA(DisplayName = "Pet Egg"),
	PetArmor	UMETA(DisplayName = "Pet Armor")
};

/**
 * Weapon sub-types in Ragnarok Online.
 */
UENUM(BlueprintType)
enum class EROWeaponType : uint8
{
	Dagger			UMETA(DisplayName = "Dagger"),
	Sword			UMETA(DisplayName = "Sword"),
	TwoHandSword	UMETA(DisplayName = "Two-Hand Sword"),
	Spear			UMETA(DisplayName = "Spear"),
	TwoHandSpear	UMETA(DisplayName = "Two-Hand Spear"),
	Axe				UMETA(DisplayName = "Axe"),
	TwoHandAxe		UMETA(DisplayName = "Two-Hand Axe"),
	Mace			UMETA(DisplayName = "Mace"),
	TwoHandMace		UMETA(DisplayName = "Two-Hand Mace"),
	Rod				UMETA(DisplayName = "Rod"),
	TwoHandRod		UMETA(DisplayName = "Two-Hand Rod"),
	Bow				UMETA(DisplayName = "Bow"),
	Knuckle			UMETA(DisplayName = "Knuckle"),
	Katar			UMETA(DisplayName = "Katar"),
	Book			UMETA(DisplayName = "Book"),
	Instrument		UMETA(DisplayName = "Instrument"),
	Whip			UMETA(DisplayName = "Whip"),
	Gun				UMETA(DisplayName = "Gun"),
	Shuriken		UMETA(DisplayName = "Shuriken")
};

/**
 * Monster size categories affecting damage modifiers.
 */
UENUM(BlueprintType)
enum class EROMonsterSize : uint8
{
	Small	UMETA(DisplayName = "Small"),
	Medium	UMETA(DisplayName = "Medium"),
	Large	UMETA(DisplayName = "Large")
};

/**
 * Monster race categories affecting racial damage modifiers.
 */
UENUM(BlueprintType)
enum class EROMonsterRace : uint8
{
	Formless	UMETA(DisplayName = "Formless"),
	Undead		UMETA(DisplayName = "Undead"),
	Brute		UMETA(DisplayName = "Brute"),
	Plant		UMETA(DisplayName = "Plant"),
	Insect		UMETA(DisplayName = "Insect"),
	Fish		UMETA(DisplayName = "Fish"),
	Demon		UMETA(DisplayName = "Demon"),
	DemiHuman	UMETA(DisplayName = "Demi-Human"),
	Angel		UMETA(DisplayName = "Angel"),
	Dragon		UMETA(DisplayName = "Dragon")
};

/**
 * Damage calculation type.
 */
UENUM(BlueprintType)
enum class ERODamageType : uint8
{
	Physical	UMETA(DisplayName = "Physical"),
	Magical		UMETA(DisplayName = "Magical"),
	Misc		UMETA(DisplayName = "Misc")
};

/**
 * Job advancement tiers.
 */
UENUM(BlueprintType)
enum class EROJobTier : uint8
{
	Novice_Tier				UMETA(DisplayName = "Novice"),
	First					UMETA(DisplayName = "1st Class"),
	Second					UMETA(DisplayName = "2nd Class"),
	Transcendent			UMETA(DisplayName = "Transcendent"),
	TranscendentSecond		UMETA(DisplayName = "Transcendent 2nd Class")
};

/**
 * Weather types that can occur on maps.
 */
UENUM(BlueprintType)
enum class EROWeatherType : uint8
{
	Clear		UMETA(DisplayName = "Clear"),
	Clouds		UMETA(DisplayName = "Clouds"),
	Rain		UMETA(DisplayName = "Rain"),
	Snow		UMETA(DisplayName = "Snow"),
	Sakura		UMETA(DisplayName = "Sakura Petals"),
	Fog			UMETA(DisplayName = "Fog"),
	Sandstorm	UMETA(DisplayName = "Sandstorm"),
	Leaves		UMETA(DisplayName = "Autumn Leaves"),
	Storm		UMETA(DisplayName = "Storm"),
	Night		UMETA(DisplayName = "Night")
};
