// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROExpTables.h"
#include "RagnarokUE/Data/ROConstants.h"

// ============================================================================
// Base EXP Table (Levels 1-98)
// Index 0 = EXP to go from level 1 to level 2
// Index 97 = EXP to go from level 98 to level 99
// These are the actual pre-renewal Ragnarok Online base EXP values.
// ============================================================================

const TArray<int64>& UROExpTables::GetBaseExpTable()
{
	static const TArray<int64> Table = {
		// Level 1-10
		9,          // 1 -> 2
		16,         // 2 -> 3
		25,         // 3 -> 4
		36,         // 4 -> 5
		77,         // 5 -> 6
		112,        // 6 -> 7
		153,        // 7 -> 8
		200,        // 8 -> 9
		253,        // 9 -> 10
		320,        // 10 -> 11
		// Level 11-20
		385,        // 11 -> 12
		490,        // 12 -> 13
		585,        // 13 -> 14
		700,        // 14 -> 15
		830,        // 15 -> 16
		970,        // 16 -> 17
		1120,       // 17 -> 18
		1280,       // 18 -> 19
		1450,       // 19 -> 20
		1630,       // 20 -> 21
		// Level 21-30
		1830,       // 21 -> 22
		2040,       // 22 -> 23
		2260,       // 23 -> 24
		2500,       // 24 -> 25
		2760,       // 25 -> 26
		3040,       // 26 -> 27
		3340,       // 27 -> 28
		3680,       // 28 -> 29
		4040,       // 29 -> 30
		4440,       // 30 -> 31
		// Level 31-40
		4880,       // 31 -> 32
		5360,       // 32 -> 33
		5880,       // 33 -> 34
		6440,       // 34 -> 35
		7050,       // 35 -> 36
		7720,       // 36 -> 37
		8440,       // 37 -> 38
		9220,       // 38 -> 39
		10070,      // 39 -> 40
		10990,      // 40 -> 41
		// Level 41-50
		12000,      // 41 -> 42
		13100,      // 42 -> 43
		14300,      // 43 -> 44
		15600,      // 44 -> 45
		17000,      // 45 -> 46
		18500,      // 46 -> 47
		20100,      // 47 -> 48
		21800,      // 48 -> 49
		23600,      // 49 -> 50
		25500,      // 50 -> 51
		// Level 51-60
		27600,      // 51 -> 52
		29800,      // 52 -> 53
		32200,      // 53 -> 54
		34700,      // 54 -> 55
		37300,      // 55 -> 56
		40100,      // 56 -> 57
		43100,      // 57 -> 58
		46300,      // 58 -> 59
		49700,      // 59 -> 60
		53400,      // 60 -> 61
		// Level 61-70
		57400,      // 61 -> 62
		61800,      // 62 -> 63
		66500,      // 63 -> 64
		71600,      // 64 -> 65
		77100,      // 65 -> 66
		83100,      // 66 -> 67
		89600,      // 67 -> 68
		96600,      // 68 -> 69
		104200,     // 69 -> 70
		112500,     // 70 -> 71
		// Level 71-80
		121500,     // 71 -> 72
		131300,     // 72 -> 73
		141900,     // 73 -> 74
		153400,     // 74 -> 75
		165800,     // 75 -> 76
		179300,     // 76 -> 77
		193800,     // 77 -> 78
		209600,     // 78 -> 79
		226800,     // 79 -> 80
		245400,     // 80 -> 81
		// Level 81-90
		265700,     // 81 -> 82
		287600,     // 82 -> 83
		311500,     // 83 -> 84
		337500,     // 84 -> 85
		365800,     // 85 -> 86
		396600,     // 86 -> 87
		430100,     // 87 -> 88
		466600,     // 88 -> 89
		506400,     // 89 -> 90
		549700,     // 90 -> 91
		// Level 91-98
		597000,     // 91 -> 92
		648700,     // 92 -> 93
		705300,     // 93 -> 94
		767300,     // 94 -> 95
		835200,     // 95 -> 96
		909800,     // 96 -> 97
		991800,     // 97 -> 98
		1081900     // 98 -> 99
	};
	return Table;
}

// ============================================================================
// Novice Job EXP Table (Job Levels 1-10)
// ============================================================================

const TArray<int64>& UROExpTables::GetNoviceJobExpTable()
{
	static const TArray<int64> Table = {
		10,     // 1 -> 2
		18,     // 2 -> 3
		28,     // 3 -> 4
		40,     // 4 -> 5
		91,     // 5 -> 6
		151,    // 6 -> 7
		205,    // 7 -> 8
		268,    // 8 -> 9
		340,    // 9 -> 10
		0       // 10 (max)
	};
	return Table;
}

// ============================================================================
// 1st Class Job EXP Table (Job Levels 1-50)
// ============================================================================

const TArray<int64>& UROExpTables::GetFirstClassJobExpTable()
{
	static const TArray<int64> Table = {
		// Job Level 1-10
		30,
		43,
		58,
		76,
		116,
		180,
		220,
		272,
		336,
		520,
		// Job Level 11-20
		604,
		699,
		802,
		948,
		1125,
		1668,
		1937,
		2226,
		3040,
		3988,
		// Job Level 21-30
		5564,
		6460,
		7560,
		8784,
		10200,
		13800,
		16200,
		18800,
		21800,
		25200,
		// Job Level 31-40
		29200,
		33800,
		39200,
		45400,
		52600,
		61000,
		70600,
		81800,
		94800,
		109800,
		// Job Level 41-50
		127200,
		147400,
		170800,
		197800,
		229200,
		265400,
		307400,
		356200,
		412600,
		0
	};
	return Table;
}

// ============================================================================
// 2nd Class Job EXP Table (Job Levels 1-50)
// ============================================================================

const TArray<int64>& UROExpTables::GetSecondClassJobExpTable()
{
	static const TArray<int64> Table = {
		// Job Level 1-10
		144,
		184,
		284,
		348,
		603,
		887,
		1096,
		1598,
		2540,
		3676,
		// Job Level 11-20
		4290,
		7960,
		9364,
		12047,
		15290,
		14250,
		16352,
		19504,
		22734,
		27468,
		// Job Level 21-30
		31988,
		37580,
		44544,
		52340,
		61356,
		67428,
		73500,
		82404,
		92652,
		105264,
		// Job Level 31-40
		120780,
		137916,
		157380,
		181140,
		201600,
		227280,
		258528,
		295488,
		337764,
		390600,
		// Job Level 41-50
		441000,
		498960,
		566640,
		639840,
		726300,
		818640,
		921600,
		1044300,
		1178400,
		0
	};
	return Table;
}

// ============================================================================
// Transcendent 2nd Class Job EXP Table (Job Levels 1-70)
// ============================================================================

const TArray<int64>& UROExpTables::GetTransSecondClassJobExpTable()
{
	static const TArray<int64> Table = {
		// Job Level 1-10
		144,
		184,
		284,
		348,
		603,
		887,
		1096,
		1598,
		2540,
		3676,
		// Job Level 11-20
		4290,
		7960,
		9364,
		12047,
		15290,
		14250,
		16352,
		19504,
		22734,
		27468,
		// Job Level 21-30
		31988,
		37580,
		44544,
		52340,
		61356,
		67428,
		73500,
		82404,
		92652,
		105264,
		// Job Level 31-40
		120780,
		137916,
		157380,
		181140,
		201600,
		227280,
		258528,
		295488,
		337764,
		390600,
		// Job Level 41-50
		441000,
		498960,
		566640,
		639840,
		726300,
		818640,
		921600,
		1044300,
		1178400,
		1332000,
		// Job Level 51-60
		1512000,
		1710000,
		1935000,
		2190000,
		2475000,
		2790000,
		3150000,
		3555000,
		4020000,
		4545000,
		// Job Level 61-70
		5130000,
		5790000,
		6540000,
		7380000,
		8325000,
		9405000,
		10620000,
		11985000,
		13530000,
		0
	};
	return Table;
}

// ============================================================================
// Public API
// ============================================================================

int64 UROExpTables::GetBaseExpRequired(int32 Level)
{
	if (Level < 1 || Level >= ROConstants::MaxBaseLevel)
	{
		return 0;
	}

	const auto& Table = GetBaseExpTable();
	const int32 Index = Level - 1;

	if (!Table.IsValidIndex(Index))
	{
		return 0;
	}

	return Table[Index];
}

int64 UROExpTables::GetJobExpRequired(int32 JobLevel, EROJobTier Tier)
{
	if (JobLevel < 1)
	{
		return 0;
	}

	const TArray<int64>* TablePtr = nullptr;
	int32 MaxLevel = 0;

	switch (Tier)
	{
	case EROJobTier::Novice_Tier:
		TablePtr = &GetNoviceJobExpTable();
		MaxLevel = ROConstants::MaxJobLevelNovice;
		break;
	case EROJobTier::First:
	case EROJobTier::Transcendent: // Trans 1st classes use same table as 1st class
		TablePtr = &GetFirstClassJobExpTable();
		MaxLevel = ROConstants::MaxJobLevel1st;
		break;
	case EROJobTier::Second:
		TablePtr = &GetSecondClassJobExpTable();
		MaxLevel = ROConstants::MaxJobLevel2nd;
		break;
	case EROJobTier::TranscendentSecond:
		TablePtr = &GetTransSecondClassJobExpTable();
		MaxLevel = ROConstants::MaxJobLevelTrans;
		break;
	default:
		return 0;
	}

	if (JobLevel >= MaxLevel || TablePtr == nullptr)
	{
		return 0;
	}

	const int32 Index = JobLevel - 1;
	if (!TablePtr->IsValidIndex(Index))
	{
		return 0;
	}

	return (*TablePtr)[Index];
}

int32 UROExpTables::GetStatPointsForLevel(int32 Level)
{
	if (Level <= 1)
	{
		return 0;
	}
	// Formula: floor((Level - 1) / 5) + 3
	return (Level - 1) / 5 + 3;
}

int32 UROExpTables::GetTotalStatPoints(int32 FromLevel, int32 ToLevel)
{
	if (FromLevel >= ToLevel)
	{
		return 0;
	}

	int32 Total = 0;
	const int32 Start = FMath::Max(2, FromLevel + 1);
	const int32 End = FMath::Min(ROConstants::MaxBaseLevel, ToLevel);

	for (int32 L = Start; L <= End; ++L)
	{
		Total += GetStatPointsForLevel(L);
	}

	return Total;
}

int64 UROExpTables::GetTotalBaseExp(int32 Level)
{
	if (Level <= 1)
	{
		return 0;
	}

	const auto& Table = GetBaseExpTable();
	int64 Total = 0;

	const int32 MaxIdx = FMath::Min(Level - 1, Table.Num());
	for (int32 i = 0; i < MaxIdx; ++i)
	{
		Total += Table[i];
	}

	return Total;
}
