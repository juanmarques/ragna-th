// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROMonsterDatabase.h"

void UROMonsterDatabase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	PopulatePronteraMonsters();

	UE_LOG(LogTemp, Log, TEXT("ROMonsterDatabase initialized with %d monster entries."), MonsterDB.Num());
}

void UROMonsterDatabase::Deinitialize()
{
	MonsterDB.Empty();
	Super::Deinitialize();
}

bool UROMonsterDatabase::GetMonsterData(int32 MonsterID, FROMonsterData& OutData) const
{
	const FROMonsterData* Found = MonsterDB.Find(MonsterID);
	if (Found)
	{
		OutData = *Found;
		return true;
	}
	return false;
}

TArray<int32> UROMonsterDatabase::GetAllMonsterIDs() const
{
	TArray<int32> IDs;
	MonsterDB.GetKeys(IDs);
	return IDs;
}

bool UROMonsterDatabase::HasMonster(int32 MonsterID) const
{
	return MonsterDB.Contains(MonsterID);
}

void UROMonsterDatabase::RegisterMonster(const FROMonsterData& Data)
{
	MonsterDB.Add(Data.MonsterID, Data);
}

void UROMonsterDatabase::PopulatePronteraMonsters()
{
	// ========================================================================
	// Prontera Field Monsters (Pre-Renewal stats)
	// ========================================================================

	// Poring (ID 1002)
	{
		FROMonsterData M;
		M.MonsterID = 1002;
		M.MonsterName = FName("Poring");
		M.DisplayName = FText::FromString(TEXT("Poring"));
		M.HP = 50;
		M.ATKMin = 7;
		M.ATKMax = 10;
		M.MATK = 0;
		M.DEF = 0;
		M.MDEF = 5;
		M.HIT = 16;
		M.FLEE = 16;
		M.Element = EROElement::Water;
		M.ElementLevel = EROElementLevel::Level1;
		M.Size = EROMonsterSize::Medium;
		M.Race = EROMonsterRace::Plant;
		M.Behavior = EROMonsterBehavior::Passive;
		M.BaseExpReward = 2;
		M.JobExpReward = 1;
		M.AggroRange = 0.0f;
		M.ChaseRange = 1500.0f;
		M.AttackRange = 150.0f;
		M.AttackSpeed = 1.872f;
		M.bIsMVP = false;
		M.bIsBoss = false;
		M.RespawnTime = 5.0f;

		// Drops
		M.DropTable.Add({909, 70.0f});   // Jellopy
		M.DropTable.Add({938, 4.0f});    // Sticky Mucus
		M.DropTable.Add({512, 1.5f});    // Apple
		M.DropTable.Add({713, 1.0f});    // Empty Bottle
		M.DropTable.Add({741, 0.2f});    // Unripe Apple
		M.DropTable.Add({619, 0.1f});    // Old Card Album
		M.DropTable.Add({4001, 0.01f});  // Poring Card

		RegisterMonster(M);
	}

	// Lunatic (ID 1063)
	{
		FROMonsterData M;
		M.MonsterID = 1063;
		M.MonsterName = FName("Lunatic");
		M.DisplayName = FText::FromString(TEXT("Lunatic"));
		M.HP = 60;
		M.ATKMin = 9;
		M.ATKMax = 12;
		M.MATK = 0;
		M.DEF = 0;
		M.MDEF = 20;
		M.HIT = 18;
		M.FLEE = 18;
		M.Element = EROElement::Neutral;
		M.ElementLevel = EROElementLevel::Level3;
		M.Size = EROMonsterSize::Small;
		M.Race = EROMonsterRace::Brute;
		M.Behavior = EROMonsterBehavior::Passive;
		M.BaseExpReward = 6;
		M.JobExpReward = 2;
		M.AggroRange = 0.0f;
		M.ChaseRange = 1500.0f;
		M.AttackRange = 150.0f;
		M.AttackSpeed = 1.456f;
		M.bIsMVP = false;
		M.bIsBoss = false;
		M.RespawnTime = 5.0f;

		M.DropTable.Add({705, 20.0f});   // Clover
		M.DropTable.Add({949, 10.0f});   // Feather
		M.DropTable.Add({512, 3.0f});    // Apple
		M.DropTable.Add({601, 0.8f});    // Wing of Fly
		M.DropTable.Add({515, 1.0f});    // Meat
		M.DropTable.Add({4006, 0.01f});  // Lunatic Card

		RegisterMonster(M);
	}

	// Fabre (ID 1007)
	{
		FROMonsterData M;
		M.MonsterID = 1007;
		M.MonsterName = FName("Fabre");
		M.DisplayName = FText::FromString(TEXT("Fabre"));
		M.HP = 63;
		M.ATKMin = 8;
		M.ATKMax = 11;
		M.MATK = 0;
		M.DEF = 0;
		M.MDEF = 5;
		M.HIT = 15;
		M.FLEE = 15;
		M.Element = EROElement::Earth;
		M.ElementLevel = EROElementLevel::Level1;
		M.Size = EROMonsterSize::Small;
		M.Race = EROMonsterRace::Insect;
		M.Behavior = EROMonsterBehavior::Passive;
		M.BaseExpReward = 3;
		M.JobExpReward = 2;
		M.AggroRange = 0.0f;
		M.ChaseRange = 1500.0f;
		M.AttackRange = 150.0f;
		M.AttackSpeed = 1.672f;
		M.bIsMVP = false;
		M.bIsBoss = false;
		M.RespawnTime = 5.0f;

		M.DropTable.Add({914, 60.0f});   // Fluff
		M.DropTable.Add({949, 5.0f});    // Feather
		M.DropTable.Add({4002, 0.01f});  // Fabre Card

		RegisterMonster(M);
	}

	// Pupa (ID 1008) - doesn't attack
	{
		FROMonsterData M;
		M.MonsterID = 1008;
		M.MonsterName = FName("Pupa");
		M.DisplayName = FText::FromString(TEXT("Pupa"));
		M.HP = 427;
		M.ATKMin = 1;
		M.ATKMax = 1;
		M.MATK = 0;
		M.DEF = 0;
		M.MDEF = 0;
		M.HIT = 1;
		M.FLEE = 1;
		M.Element = EROElement::Earth;
		M.ElementLevel = EROElementLevel::Level1;
		M.Size = EROMonsterSize::Small;
		M.Race = EROMonsterRace::Insect;
		M.Behavior = EROMonsterBehavior::Passive;
		M.BaseExpReward = 4;
		M.JobExpReward = 3;
		M.AggroRange = 0.0f;
		M.ChaseRange = 0.0f; // Never chases
		M.AttackRange = 0.0f; // Cannot attack
		M.AttackSpeed = 1.0f;
		M.bIsMVP = false;
		M.bIsBoss = false;
		M.RespawnTime = 5.0f;

		M.DropTable.Add({915, 55.0f});   // Chrysalis
		M.DropTable.Add({938, 5.0f});    // Sticky Mucus
		M.DropTable.Add({1002, 0.2f});   // Guard
		M.DropTable.Add({4003, 0.01f});  // Pupa Card

		RegisterMonster(M);
	}

	// Drops (ID 1113)
	{
		FROMonsterData M;
		M.MonsterID = 1113;
		M.MonsterName = FName("Drops");
		M.DisplayName = FText::FromString(TEXT("Drops"));
		M.HP = 55;
		M.ATKMin = 10;
		M.ATKMax = 13;
		M.MATK = 0;
		M.DEF = 0;
		M.MDEF = 10;
		M.HIT = 19;
		M.FLEE = 19;
		M.Element = EROElement::Fire;
		M.ElementLevel = EROElementLevel::Level1;
		M.Size = EROMonsterSize::Medium;
		M.Race = EROMonsterRace::Plant;
		M.Behavior = EROMonsterBehavior::Passive;
		M.BaseExpReward = 4;
		M.JobExpReward = 1;
		M.AggroRange = 0.0f;
		M.ChaseRange = 1500.0f;
		M.AttackRange = 150.0f;
		M.AttackSpeed = 1.748f;
		M.bIsMVP = false;
		M.bIsBoss = false;
		M.RespawnTime = 5.0f;

		M.DropTable.Add({909, 75.0f});   // Jellopy
		M.DropTable.Add({938, 5.0f});    // Sticky Mucus
		M.DropTable.Add({512, 2.0f});    // Apple
		M.DropTable.Add({620, 0.2f});    // Orange Juice
		M.DropTable.Add({4033, 0.01f});  // Drops Card

		RegisterMonster(M);
	}

	// Poporing (ID 1031)
	{
		FROMonsterData M;
		M.MonsterID = 1031;
		M.MonsterName = FName("Poporing");
		M.DisplayName = FText::FromString(TEXT("Poporing"));
		M.HP = 182;
		M.ATKMin = 24;
		M.ATKMax = 29;
		M.MATK = 0;
		M.DEF = 0;
		M.MDEF = 15;
		M.HIT = 32;
		M.FLEE = 32;
		M.Element = EROElement::Poison;
		M.ElementLevel = EROElementLevel::Level1;
		M.Size = EROMonsterSize::Medium;
		M.Race = EROMonsterRace::Plant;
		M.Behavior = EROMonsterBehavior::Passive;
		M.BaseExpReward = 81;
		M.JobExpReward = 56;
		M.AggroRange = 0.0f;
		M.ChaseRange = 1500.0f;
		M.AttackRange = 150.0f;
		M.AttackSpeed = 1.672f;
		M.bIsMVP = false;
		M.bIsBoss = false;
		M.RespawnTime = 8.0f;

		M.DropTable.Add({938, 55.0f});   // Sticky Mucus
		M.DropTable.Add({909, 30.0f});   // Jellopy
		M.DropTable.Add({511, 2.0f});    // Green Herb
		M.DropTable.Add({512, 4.0f});    // Apple
		M.DropTable.Add({713, 2.0f});    // Empty Bottle
		M.DropTable.Add({4016, 0.01f});  // Poporing Card

		RegisterMonster(M);
	}

	// Willow (ID 1010)
	{
		FROMonsterData M;
		M.MonsterID = 1010;
		M.MonsterName = FName("Willow");
		M.DisplayName = FText::FromString(TEXT("Willow"));
		M.HP = 80;
		M.ATKMin = 11;
		M.ATKMax = 14;
		M.MATK = 0;
		M.DEF = 0;
		M.MDEF = 30;
		M.HIT = 14;
		M.FLEE = 14;
		M.Element = EROElement::Fire;
		M.ElementLevel = EROElementLevel::Level1;
		M.Size = EROMonsterSize::Medium;
		M.Race = EROMonsterRace::Plant;
		M.Behavior = EROMonsterBehavior::Passive;
		M.BaseExpReward = 5;
		M.JobExpReward = 3;
		M.AggroRange = 0.0f;
		M.ChaseRange = 1500.0f;
		M.AttackRange = 150.0f;
		M.AttackSpeed = 1.672f;
		M.bIsMVP = false;
		M.bIsBoss = false;
		M.RespawnTime = 5.0f;

		M.DropTable.Add({902, 25.0f});   // Tree Root
		M.DropTable.Add({1019, 0.1f});   // Wooden Mail
		M.DropTable.Add({4004, 0.01f});  // Willow Card

		RegisterMonster(M);
	}

	// Condor (ID 1009)
	{
		FROMonsterData M;
		M.MonsterID = 1009;
		M.MonsterName = FName("Condor");
		M.DisplayName = FText::FromString(TEXT("Condor"));
		M.HP = 92;
		M.ATKMin = 13;
		M.ATKMax = 16;
		M.MATK = 0;
		M.DEF = 0;
		M.MDEF = 5;
		M.HIT = 24;
		M.FLEE = 24;
		M.Element = EROElement::Wind;
		M.ElementLevel = EROElementLevel::Level1;
		M.Size = EROMonsterSize::Medium;
		M.Race = EROMonsterRace::Brute;
		M.Behavior = EROMonsterBehavior::Passive;
		M.BaseExpReward = 6;
		M.JobExpReward = 3;
		M.AggroRange = 0.0f;
		M.ChaseRange = 1500.0f;
		M.AttackRange = 150.0f;
		M.AttackSpeed = 1.148f;
		M.bIsMVP = false;
		M.bIsBoss = false;
		M.RespawnTime = 5.0f;

		M.DropTable.Add({917, 55.0f});   // Feather of Birds
		M.DropTable.Add({949, 10.0f});   // Feather
		M.DropTable.Add({1750, 0.1f});   // Bow
		M.DropTable.Add({4005, 0.01f});  // Condor Card

		RegisterMonster(M);
	}

	// Roda Frog (ID 1012)
	{
		FROMonsterData M;
		M.MonsterID = 1012;
		M.MonsterName = FName("Roda_Frog");
		M.DisplayName = FText::FromString(TEXT("Roda Frog"));
		M.HP = 97;
		M.ATKMin = 12;
		M.ATKMax = 15;
		M.MATK = 0;
		M.DEF = 0;
		M.MDEF = 5;
		M.HIT = 20;
		M.FLEE = 20;
		M.Element = EROElement::Water;
		M.ElementLevel = EROElementLevel::Level1;
		M.Size = EROMonsterSize::Medium;
		M.Race = EROMonsterRace::Fish;
		M.Behavior = EROMonsterBehavior::Passive;
		M.BaseExpReward = 7;
		M.JobExpReward = 5;
		M.AggroRange = 0.0f;
		M.ChaseRange = 1500.0f;
		M.AttackRange = 150.0f;
		M.AttackSpeed = 1.596f;
		M.bIsMVP = false;
		M.bIsBoss = false;
		M.RespawnTime = 5.0f;

		M.DropTable.Add({908, 55.0f});   // Spawn
		M.DropTable.Add({938, 5.0f});    // Sticky Mucus
		M.DropTable.Add({4007, 0.01f});  // Roda Frog Card

		RegisterMonster(M);
	}

	// Rocker (ID 1052)
	{
		FROMonsterData M;
		M.MonsterID = 1052;
		M.MonsterName = FName("Rocker");
		M.DisplayName = FText::FromString(TEXT("Rocker"));
		M.HP = 198;
		M.ATKMin = 24;
		M.ATKMax = 31;
		M.MATK = 0;
		M.DEF = 5;
		M.MDEF = 10;
		M.HIT = 30;
		M.FLEE = 30;
		M.Element = EROElement::Earth;
		M.ElementLevel = EROElementLevel::Level1;
		M.Size = EROMonsterSize::Medium;
		M.Race = EROMonsterRace::Insect;
		M.Behavior = EROMonsterBehavior::Passive;
		M.BaseExpReward = 70;
		M.JobExpReward = 48;
		M.AggroRange = 0.0f;
		M.ChaseRange = 1500.0f;
		M.AttackRange = 150.0f;
		M.AttackSpeed = 1.864f;
		M.bIsMVP = false;
		M.bIsBoss = false;
		M.RespawnTime = 7.0f;

		M.DropTable.Add({940, 55.0f});   // Grasshopper's Leg
		M.DropTable.Add({918, 25.0f});   // Singing Plant
		M.DropTable.Add({1402, 0.06f});  // Violin
		M.DropTable.Add({4021, 0.01f});  // Rocker Card

		RegisterMonster(M);
	}

	// Thief Bug (ID 1051) - Aggressive
	{
		FROMonsterData M;
		M.MonsterID = 1051;
		M.MonsterName = FName("Thief_Bug");
		M.DisplayName = FText::FromString(TEXT("Thief Bug"));
		M.HP = 50;
		M.ATKMin = 10;
		M.ATKMax = 13;
		M.MATK = 0;
		M.DEF = 5;
		M.MDEF = 5;
		M.HIT = 18;
		M.FLEE = 18;
		M.Element = EROElement::Neutral;
		M.ElementLevel = EROElementLevel::Level1;
		M.Size = EROMonsterSize::Small;
		M.Race = EROMonsterRace::Insect;
		M.Behavior = EROMonsterBehavior::Aggressive;
		M.BaseExpReward = 8;
		M.JobExpReward = 5;
		M.AggroRange = 800.0f;
		M.ChaseRange = 2000.0f;
		M.AttackRange = 150.0f;
		M.AttackSpeed = 1.748f;
		M.bIsMVP = false;
		M.bIsBoss = false;
		M.RespawnTime = 5.0f;

		M.DropTable.Add({924, 60.0f});   // Worm Peeling
		M.DropTable.Add({938, 8.0f});    // Sticky Mucus
		M.DropTable.Add({4020, 0.01f});  // Thief Bug Card

		RegisterMonster(M);
	}

	// Steel Chonchon (ID 1042) - Aggressive
	{
		FROMonsterData M;
		M.MonsterID = 1042;
		M.MonsterName = FName("Steel_Chonchon");
		M.DisplayName = FText::FromString(TEXT("Steel Chonchon"));
		M.HP = 413;
		M.ATKMin = 40;
		M.ATKMax = 53;
		M.MATK = 0;
		M.DEF = 10;
		M.MDEF = 10;
		M.HIT = 47;
		M.FLEE = 47;
		M.Element = EROElement::Wind;
		M.ElementLevel = EROElementLevel::Level1;
		M.Size = EROMonsterSize::Small;
		M.Race = EROMonsterRace::Insect;
		M.Behavior = EROMonsterBehavior::Aggressive;
		M.BaseExpReward = 199;
		M.JobExpReward = 132;
		M.AggroRange = 1000.0f;
		M.ChaseRange = 2000.0f;
		M.AttackRange = 150.0f;
		M.AttackSpeed = 1.076f;
		M.bIsMVP = false;
		M.bIsBoss = false;
		M.RespawnTime = 10.0f;

		M.DropTable.Add({910, 55.0f});   // Insect Feeler
		M.DropTable.Add({935, 30.0f});   // Shell
		M.DropTable.Add({943, 5.0f});    // Iron
		M.DropTable.Add({999, 1.0f});    // Steel
		M.DropTable.Add({4018, 0.01f});  // Steel Chonchon Card

		RegisterMonster(M);
	}
}
