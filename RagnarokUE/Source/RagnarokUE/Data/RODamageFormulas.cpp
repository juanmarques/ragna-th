// Copyright Ragna-TH Project. All Rights Reserved.

#include "RODamageFormulas.h"
#include "RagnarokUE/Data/ROConstants.h"

// ============================================================================
// Offensive Stats
// ============================================================================

int32 URODamageFormulas::CalculateBaseATK(int32 STR, int32 DEX, int32 LUK)
{
	// Formula: STR + floor(STR/10)^2 + DEX/5 + LUK/5
	const int32 StrBonus = (STR / 10) * (STR / 10);
	return STR + StrBonus + DEX / 5 + LUK / 5;
}

FVector2D URODamageFormulas::CalculateBaseMATK(int32 INT)
{
	// Min: INT + floor(INT/7)^2
	// Max: INT + floor(INT/5)^2
	const int32 MinBonus = (INT / 7) * (INT / 7);
	const int32 MaxBonus = (INT / 5) * (INT / 5);
	return FVector2D(
		static_cast<double>(INT + MinBonus),
		static_cast<double>(INT + MaxBonus)
	);
}

// ============================================================================
// Defensive Stats
// ============================================================================

int32 URODamageFormulas::CalculateHardDEF(int32 EquipDEF)
{
	// In pre-renewal, equipment DEF is subtracted directly from damage.
	return FMath::Max(0, EquipDEF);
}

int32 URODamageFormulas::CalculateSoftDEF(int32 VIT)
{
	// Pre-renewal: VIT + floor(VIT/2) + floor(VIT^2/100)
	const int32 VitHalf = VIT / 2;
	const int32 VitSquared = (VIT * VIT) / 100;
	return FMath::Max(0, VIT + VitHalf + VitSquared);
}

int32 URODamageFormulas::CalculateSoftMDEF(int32 INT, int32 VIT, int32 DEX)
{
	// Formula: INT + floor(INT/5)^2 + DEX/5 + VIT/5
	const int32 IntBonus = (INT / 5) * (INT / 5);
	return INT + IntBonus + DEX / 5 + VIT / 5;
}

// ============================================================================
// Accuracy & Evasion
// ============================================================================

int32 URODamageFormulas::CalculateHitRate(int32 DEX, int32 LUK, int32 BaseLevel)
{
	// Formula: 175 + BaseLevel + DEX + floor(LUK/3)
	return 175 + BaseLevel + DEX + LUK / 3;
}

int32 URODamageFormulas::CalculateFleeRate(int32 AGI, int32 LUK, int32 BaseLevel)
{
	// Formula: 100 + BaseLevel + AGI + floor(LUK/5)
	return 100 + BaseLevel + AGI + LUK / 5;
}

float URODamageFormulas::CalculateCritRate(int32 LUK)
{
	// Pre-renewal formula: 1 + floor(LUK/3) (integer division)
	return 1.0f + static_cast<float>(LUK / 3);
}

// ============================================================================
// Attack Speed
// ============================================================================

int32 URODamageFormulas::GetBaseASPDForJob(EROJobClass Job)
{
	switch (Job)
	{
	// Novice
	case EROJobClass::Novice:
	case EROJobClass::HighNovice:
		return 150;

	// Swordsman line
	case EROJobClass::Swordsman:
	case EROJobClass::HighSwordsman:
		return 145;
	case EROJobClass::Knight:
	case EROJobClass::Crusader:
	case EROJobClass::LordKnight:
	case EROJobClass::Paladin:
		return 145;

	// Magician line
	case EROJobClass::Magician:
	case EROJobClass::HighMagician:
		return 150;
	case EROJobClass::Wizard:
	case EROJobClass::Sage:
	case EROJobClass::HighWizard:
	case EROJobClass::Professor:
		return 150;

	// Archer line
	case EROJobClass::Archer:
	case EROJobClass::HighArcher:
		return 145;
	case EROJobClass::Hunter:
	case EROJobClass::Bard:
	case EROJobClass::Dancer:
	case EROJobClass::Sniper:
	case EROJobClass::Minstrel:
	case EROJobClass::Gypsy:
		return 145;

	// Thief line
	case EROJobClass::Thief:
	case EROJobClass::HighThief:
		return 140;
	case EROJobClass::Assassin:
	case EROJobClass::Rogue:
	case EROJobClass::AssassinCross:
	case EROJobClass::Stalker:
		return 140;

	// Merchant line
	case EROJobClass::Merchant:
	case EROJobClass::HighMerchant:
		return 150;
	case EROJobClass::Blacksmith:
	case EROJobClass::Alchemist:
	case EROJobClass::Whitesmith:
	case EROJobClass::Creator:
		return 150;

	// Acolyte line
	case EROJobClass::Acolyte:
	case EROJobClass::HighAcolyte:
		return 150;
	case EROJobClass::Priest:
	case EROJobClass::HighPriest:
		return 150;
	case EROJobClass::Monk:
	case EROJobClass::Champion:
		return 145;

	default:
		return 150;
	}
}

float URODamageFormulas::CalculateASPD(int32 BaseASPD, int32 AGI, int32 DEX)
{
	// ASPD = 200 - (BaseASPD - floor((AGI*4 + DEX) / 5))
	// BaseASPD is the weapon-type delay (e.g., 145 for daggers, 160 for two-hand swords)
	// Higher ASPD = faster attacks. Clamped to [0, 190].
	const float StatBonus = static_cast<float>((AGI * 4 + DEX) / 5);
	const float RawASPD = 200.0f - (static_cast<float>(BaseASPD) - StatBonus);
	return FMath::Clamp(RawASPD, 0.0f, ROConstants::MaxASPD);
}

// ============================================================================
// HP / SP
// ============================================================================

int32 URODamageFormulas::CalculateMaxHP(int32 BaseLevel, int32 VIT, EROJobClass Job)
{
	// Base formula: (35 + BaseLevel * JobHPMod) * (1 + VIT/100)
	// Additional VIT bonus: floor(VIT/5)^2 at higher levels
	const float JobMod = GetJobHPModifier(Job);
	const float BaseHP = 35.0f + static_cast<float>(BaseLevel) * JobMod;
	const float VITMultiplier = 1.0f + static_cast<float>(VIT) / 100.0f;
	const int32 VITBonus = (VIT / 5) * (VIT / 5);
	return FMath::Max(1, static_cast<int32>(BaseHP * VITMultiplier) + VITBonus);
}

int32 URODamageFormulas::CalculateMaxSP(int32 BaseLevel, int32 INT, EROJobClass Job)
{
	// Base formula: (10 + BaseLevel * JobSPMod) * (1 + INT/100)
	const float JobMod = GetJobSPModifier(Job);
	const float BaseSP = 10.0f + static_cast<float>(BaseLevel) * JobMod;
	const float INTMultiplier = 1.0f + static_cast<float>(INT) / 100.0f;
	const int32 INTBonus = (INT / 6) * (INT / 6);
	return FMath::Max(1, static_cast<int32>(BaseSP * INTMultiplier) + INTBonus);
}

float URODamageFormulas::GetJobHPModifier(EROJobClass Job)
{
	switch (Job)
	{
	// Novice tier
	case EROJobClass::Novice:
	case EROJobClass::HighNovice:
		return 5.0f;

	// Swordsman line (high HP)
	case EROJobClass::Swordsman:
	case EROJobClass::HighSwordsman:
		return 8.0f;
	case EROJobClass::Knight:
	case EROJobClass::Crusader:
		return 11.0f;
	case EROJobClass::LordKnight:
		return 14.0f;
	case EROJobClass::Paladin:
		return 13.0f;

	// Magician line (low HP)
	case EROJobClass::Magician:
	case EROJobClass::HighMagician:
		return 4.0f;
	case EROJobClass::Wizard:
	case EROJobClass::Sage:
		return 6.0f;
	case EROJobClass::HighWizard:
	case EROJobClass::Professor:
		return 7.5f;

	// Archer line
	case EROJobClass::Archer:
	case EROJobClass::HighArcher:
		return 6.0f;
	case EROJobClass::Hunter:
	case EROJobClass::Bard:
	case EROJobClass::Dancer:
		return 8.0f;
	case EROJobClass::Sniper:
	case EROJobClass::Minstrel:
	case EROJobClass::Gypsy:
		return 10.0f;

	// Thief line
	case EROJobClass::Thief:
	case EROJobClass::HighThief:
		return 6.5f;
	case EROJobClass::Assassin:
	case EROJobClass::Rogue:
		return 8.5f;
	case EROJobClass::AssassinCross:
	case EROJobClass::Stalker:
		return 10.5f;

	// Merchant line
	case EROJobClass::Merchant:
	case EROJobClass::HighMerchant:
		return 7.0f;
	case EROJobClass::Blacksmith:
	case EROJobClass::Alchemist:
		return 9.5f;
	case EROJobClass::Whitesmith:
	case EROJobClass::Creator:
		return 11.5f;

	// Acolyte line
	case EROJobClass::Acolyte:
	case EROJobClass::HighAcolyte:
		return 6.0f;
	case EROJobClass::Priest:
		return 8.0f;
	case EROJobClass::Monk:
		return 9.0f;
	case EROJobClass::HighPriest:
		return 10.0f;
	case EROJobClass::Champion:
		return 11.0f;

	default:
		return 5.0f;
	}
}

float URODamageFormulas::GetJobSPModifier(EROJobClass Job)
{
	switch (Job)
	{
	// Novice
	case EROJobClass::Novice:
	case EROJobClass::HighNovice:
		return 1.0f;

	// Swordsman line (low SP)
	case EROJobClass::Swordsman:
	case EROJobClass::HighSwordsman:
		return 1.5f;
	case EROJobClass::Knight:
	case EROJobClass::Crusader:
		return 2.0f;
	case EROJobClass::LordKnight:
	case EROJobClass::Paladin:
		return 2.5f;

	// Magician line (high SP)
	case EROJobClass::Magician:
	case EROJobClass::HighMagician:
		return 3.0f;
	case EROJobClass::Wizard:
	case EROJobClass::Sage:
		return 5.0f;
	case EROJobClass::HighWizard:
	case EROJobClass::Professor:
		return 6.0f;

	// Archer line
	case EROJobClass::Archer:
	case EROJobClass::HighArcher:
		return 2.0f;
	case EROJobClass::Hunter:
	case EROJobClass::Bard:
	case EROJobClass::Dancer:
		return 3.0f;
	case EROJobClass::Sniper:
	case EROJobClass::Minstrel:
	case EROJobClass::Gypsy:
		return 3.5f;

	// Thief line
	case EROJobClass::Thief:
	case EROJobClass::HighThief:
		return 1.5f;
	case EROJobClass::Assassin:
	case EROJobClass::Rogue:
		return 2.0f;
	case EROJobClass::AssassinCross:
	case EROJobClass::Stalker:
		return 2.5f;

	// Merchant line
	case EROJobClass::Merchant:
	case EROJobClass::HighMerchant:
		return 1.5f;
	case EROJobClass::Blacksmith:
	case EROJobClass::Alchemist:
		return 2.5f;
	case EROJobClass::Whitesmith:
	case EROJobClass::Creator:
		return 3.0f;

	// Acolyte line (high SP)
	case EROJobClass::Acolyte:
	case EROJobClass::HighAcolyte:
		return 3.0f;
	case EROJobClass::Priest:
		return 5.0f;
	case EROJobClass::Monk:
		return 3.5f;
	case EROJobClass::HighPriest:
		return 6.0f;
	case EROJobClass::Champion:
		return 4.0f;

	default:
		return 1.0f;
	}
}

// ============================================================================
// Elemental System
// ============================================================================

const TArray<TArray<TArray<float>>>& URODamageFormulas::GetElementalTable()
{
	// Full 10x10x4 elemental effectiveness table from Ragnarok Online.
	// Outer index: attacking element (EROElement: 0=Neutral..9=Undead)
	// Middle index: defending element (EROElement: 0=Neutral..9=Undead)
	// Inner index: defending element level (0=Lv1, 1=Lv2, 2=Lv3, 3=Lv4)
	//
	// Values are damage multipliers (1.0 = 100%, 0.0 = immune, negative = absorb/heal).

	static const TArray<TArray<TArray<float>>> Table = []()
	{
		TArray<TArray<TArray<float>>> T;
		T.SetNum(10);
		for (auto& Row : T)
		{
			Row.SetNum(10);
			for (auto& Col : Row)
			{
				Col.SetNum(4);
			}
		}

		// =====================================================================
		// Neutral attacking
		// =====================================================================
		// vs Neutral Lv1-4
		T[0][0][0] = 1.00f; T[0][0][1] = 1.00f; T[0][0][2] = 1.00f; T[0][0][3] = 1.00f;
		// vs Water Lv1-4
		T[0][1][0] = 1.00f; T[0][1][1] = 1.00f; T[0][1][2] = 1.00f; T[0][1][3] = 1.00f;
		// vs Earth Lv1-4
		T[0][2][0] = 1.00f; T[0][2][1] = 1.00f; T[0][2][2] = 1.00f; T[0][2][3] = 1.00f;
		// vs Fire Lv1-4
		T[0][3][0] = 1.00f; T[0][3][1] = 1.00f; T[0][3][2] = 1.00f; T[0][3][3] = 1.00f;
		// vs Wind Lv1-4
		T[0][4][0] = 1.00f; T[0][4][1] = 1.00f; T[0][4][2] = 1.00f; T[0][4][3] = 1.00f;
		// vs Poison Lv1-4
		T[0][5][0] = 1.00f; T[0][5][1] = 1.00f; T[0][5][2] = 1.00f; T[0][5][3] = 1.00f;
		// vs Holy Lv1-4
		T[0][6][0] = 1.00f; T[0][6][1] = 1.00f; T[0][6][2] = 1.00f; T[0][6][3] = 1.00f;
		// vs Shadow Lv1-4
		T[0][7][0] = 1.00f; T[0][7][1] = 1.00f; T[0][7][2] = 1.00f; T[0][7][3] = 1.00f;
		// vs Ghost Lv1-4
		T[0][8][0] = 0.25f; T[0][8][1] = 0.25f; T[0][8][2] = 0.25f; T[0][8][3] = 0.25f;
		// vs Undead Lv1-4
		T[0][9][0] = 1.00f; T[0][9][1] = 1.00f; T[0][9][2] = 1.00f; T[0][9][3] = 1.00f;

		// =====================================================================
		// Water attacking
		// =====================================================================
		// vs Neutral Lv1-4
		T[1][0][0] = 1.00f; T[1][0][1] = 1.00f; T[1][0][2] = 1.00f; T[1][0][3] = 1.00f;
		// vs Water Lv1-4
		T[1][1][0] = 0.25f; T[1][1][1] = 0.00f; T[1][1][2] = -0.25f; T[1][1][3] = -0.50f;
		// vs Earth Lv1-4
		T[1][2][0] = 1.00f; T[1][2][1] = 1.00f; T[1][2][2] = 1.00f; T[1][2][3] = 1.00f;
		// vs Fire Lv1-4
		T[1][3][0] = 1.50f; T[1][3][1] = 1.75f; T[1][3][2] = 2.00f; T[1][3][3] = 2.00f;
		// vs Wind Lv1-4
		T[1][4][0] = 0.75f; T[1][4][1] = 0.50f; T[1][4][2] = 0.25f; T[1][4][3] = 0.00f;
		// vs Poison Lv1-4
		T[1][5][0] = 1.00f; T[1][5][1] = 0.75f; T[1][5][2] = 0.50f; T[1][5][3] = 0.25f;
		// vs Holy Lv1-4
		T[1][6][0] = 0.75f; T[1][6][1] = 0.50f; T[1][6][2] = 0.25f; T[1][6][3] = 0.00f;
		// vs Shadow Lv1-4
		T[1][7][0] = 1.00f; T[1][7][1] = 0.75f; T[1][7][2] = 0.50f; T[1][7][3] = 0.25f;
		// vs Ghost Lv1-4
		T[1][8][0] = 1.00f; T[1][8][1] = 0.75f; T[1][8][2] = 0.50f; T[1][8][3] = 0.25f;
		// vs Undead Lv1-4
		T[1][9][0] = 1.00f; T[1][9][1] = 1.00f; T[1][9][2] = 1.25f; T[1][9][3] = 1.50f;

		// =====================================================================
		// Earth attacking
		// =====================================================================
		// vs Neutral Lv1-4
		T[2][0][0] = 1.00f; T[2][0][1] = 1.00f; T[2][0][2] = 1.00f; T[2][0][3] = 1.00f;
		// vs Water Lv1-4
		T[2][1][0] = 1.00f; T[2][1][1] = 1.00f; T[2][1][2] = 1.00f; T[2][1][3] = 1.00f;
		// vs Earth Lv1-4
		T[2][2][0] = 0.25f; T[2][2][1] = 0.00f; T[2][2][2] = -0.25f; T[2][2][3] = -0.50f;
		// vs Fire Lv1-4
		T[2][3][0] = 0.75f; T[2][3][1] = 0.50f; T[2][3][2] = 0.25f; T[2][3][3] = 0.00f;
		// vs Wind Lv1-4
		T[2][4][0] = 1.50f; T[2][4][1] = 1.75f; T[2][4][2] = 2.00f; T[2][4][3] = 2.00f;
		// vs Poison Lv1-4
		T[2][5][0] = 1.00f; T[2][5][1] = 0.75f; T[2][5][2] = 0.50f; T[2][5][3] = 0.25f;
		// vs Holy Lv1-4
		T[2][6][0] = 0.75f; T[2][6][1] = 0.50f; T[2][6][2] = 0.25f; T[2][6][3] = 0.00f;
		// vs Shadow Lv1-4
		T[2][7][0] = 1.00f; T[2][7][1] = 0.75f; T[2][7][2] = 0.50f; T[2][7][3] = 0.25f;
		// vs Ghost Lv1-4
		T[2][8][0] = 1.00f; T[2][8][1] = 0.75f; T[2][8][2] = 0.50f; T[2][8][3] = 0.25f;
		// vs Undead Lv1-4
		T[2][9][0] = 1.00f; T[2][9][1] = 1.00f; T[2][9][2] = 1.00f; T[2][9][3] = 1.00f;

		// =====================================================================
		// Fire attacking
		// =====================================================================
		// vs Neutral Lv1-4
		T[3][0][0] = 1.00f; T[3][0][1] = 1.00f; T[3][0][2] = 1.00f; T[3][0][3] = 1.00f;
		// vs Water Lv1-4 (Fire is weak against Water)
		T[3][1][0] = 0.50f; T[3][1][1] = 0.25f; T[3][1][2] = 0.00f; T[3][1][3] = -0.25f;
		// vs Earth Lv1-4
		T[3][2][0] = 1.50f; T[3][2][1] = 1.75f; T[3][2][2] = 2.00f; T[3][2][3] = 2.00f;
		// vs Fire Lv1-4
		T[3][3][0] = 0.25f; T[3][3][1] = 0.00f; T[3][3][2] = -0.25f; T[3][3][3] = -0.50f;
		// vs Wind Lv1-4
		T[3][4][0] = 1.00f; T[3][4][1] = 1.00f; T[3][4][2] = 1.00f; T[3][4][3] = 1.00f;
		// vs Poison Lv1-4
		T[3][5][0] = 1.00f; T[3][5][1] = 0.75f; T[3][5][2] = 0.50f; T[3][5][3] = 0.25f;
		// vs Holy Lv1-4
		T[3][6][0] = 0.75f; T[3][6][1] = 0.50f; T[3][6][2] = 0.25f; T[3][6][3] = 0.00f;
		// vs Shadow Lv1-4
		T[3][7][0] = 1.00f; T[3][7][1] = 0.75f; T[3][7][2] = 0.50f; T[3][7][3] = 0.25f;
		// vs Ghost Lv1-4
		T[3][8][0] = 1.00f; T[3][8][1] = 0.75f; T[3][8][2] = 0.50f; T[3][8][3] = 0.25f;
		// vs Undead Lv1-4
		T[3][9][0] = 1.25f; T[3][9][1] = 1.50f; T[3][9][2] = 1.75f; T[3][9][3] = 2.00f;

		// =====================================================================
		// Wind attacking
		// =====================================================================
		// vs Neutral Lv1-4
		T[4][0][0] = 1.00f; T[4][0][1] = 1.00f; T[4][0][2] = 1.00f; T[4][0][3] = 1.00f;
		// vs Water Lv1-4
		T[4][1][0] = 1.50f; T[4][1][1] = 1.75f; T[4][1][2] = 2.00f; T[4][1][3] = 2.00f;
		// vs Earth Lv1-4
		T[4][2][0] = 0.50f; T[4][2][1] = 0.25f; T[4][2][2] = 0.00f; T[4][2][3] = -0.25f;
		// vs Fire Lv1-4
		T[4][3][0] = 1.00f; T[4][3][1] = 1.00f; T[4][3][2] = 1.00f; T[4][3][3] = 1.00f;
		// vs Wind Lv1-4
		T[4][4][0] = 0.25f; T[4][4][1] = 0.00f; T[4][4][2] = -0.25f; T[4][4][3] = -0.50f;
		// vs Poison Lv1-4
		T[4][5][0] = 1.00f; T[4][5][1] = 0.75f; T[4][5][2] = 0.50f; T[4][5][3] = 0.25f;
		// vs Holy Lv1-4
		T[4][6][0] = 0.75f; T[4][6][1] = 0.50f; T[4][6][2] = 0.25f; T[4][6][3] = 0.00f;
		// vs Shadow Lv1-4
		T[4][7][0] = 1.00f; T[4][7][1] = 0.75f; T[4][7][2] = 0.50f; T[4][7][3] = 0.25f;
		// vs Ghost Lv1-4
		T[4][8][0] = 1.00f; T[4][8][1] = 0.75f; T[4][8][2] = 0.50f; T[4][8][3] = 0.25f;
		// vs Undead Lv1-4
		T[4][9][0] = 1.00f; T[4][9][1] = 1.00f; T[4][9][2] = 1.25f; T[4][9][3] = 1.50f;

		// =====================================================================
		// Poison attacking
		// =====================================================================
		// vs Neutral Lv1-4
		T[5][0][0] = 1.00f; T[5][0][1] = 1.00f; T[5][0][2] = 1.00f; T[5][0][3] = 1.00f;
		// vs Water Lv1-4
		T[5][1][0] = 1.00f; T[5][1][1] = 1.00f; T[5][1][2] = 1.00f; T[5][1][3] = 1.00f;
		// vs Earth Lv1-4
		T[5][2][0] = 1.00f; T[5][2][1] = 1.00f; T[5][2][2] = 1.00f; T[5][2][3] = 1.00f;
		// vs Fire Lv1-4
		T[5][3][0] = 1.00f; T[5][3][1] = 1.00f; T[5][3][2] = 1.00f; T[5][3][3] = 1.00f;
		// vs Wind Lv1-4
		T[5][4][0] = 1.00f; T[5][4][1] = 1.00f; T[5][4][2] = 1.00f; T[5][4][3] = 1.00f;
		// vs Poison Lv1-4
		T[5][5][0] = 0.00f; T[5][5][1] = 0.00f; T[5][5][2] = 0.00f; T[5][5][3] = -0.25f;
		// vs Holy Lv1-4
		T[5][6][0] = 0.50f; T[5][6][1] = 0.25f; T[5][6][2] = 0.00f; T[5][6][3] = 0.00f;
		// vs Shadow Lv1-4
		T[5][7][0] = 0.50f; T[5][7][1] = 0.25f; T[5][7][2] = 0.00f; T[5][7][3] = -0.25f;
		// vs Ghost Lv1-4
		T[5][8][0] = 1.00f; T[5][8][1] = 0.75f; T[5][8][2] = 0.50f; T[5][8][3] = 0.25f;
		// vs Undead Lv1-4
		T[5][9][0] = -0.25f; T[5][9][1] = -0.50f; T[5][9][2] = -0.75f; T[5][9][3] = -1.00f;

		// =====================================================================
		// Holy attacking
		// =====================================================================
		// vs Neutral Lv1-4
		T[6][0][0] = 1.00f; T[6][0][1] = 1.00f; T[6][0][2] = 1.00f; T[6][0][3] = 1.00f;
		// vs Water Lv1-4
		T[6][1][0] = 0.75f; T[6][1][1] = 0.50f; T[6][1][2] = 0.25f; T[6][1][3] = 0.00f;
		// vs Earth Lv1-4
		T[6][2][0] = 0.75f; T[6][2][1] = 0.50f; T[6][2][2] = 0.25f; T[6][2][3] = 0.00f;
		// vs Fire Lv1-4
		T[6][3][0] = 0.75f; T[6][3][1] = 0.50f; T[6][3][2] = 0.25f; T[6][3][3] = 0.00f;
		// vs Wind Lv1-4
		T[6][4][0] = 0.75f; T[6][4][1] = 0.50f; T[6][4][2] = 0.25f; T[6][4][3] = 0.00f;
		// vs Poison Lv1-4
		T[6][5][0] = 1.00f; T[6][5][1] = 0.75f; T[6][5][2] = 0.50f; T[6][5][3] = 0.25f;
		// vs Holy Lv1-4
		T[6][6][0] = 0.00f; T[6][6][1] = -0.25f; T[6][6][2] = -0.50f; T[6][6][3] = -0.75f;
		// vs Shadow Lv1-4
		T[6][7][0] = 1.25f; T[6][7][1] = 1.50f; T[6][7][2] = 1.75f; T[6][7][3] = 2.00f;
		// vs Ghost Lv1-4
		T[6][8][0] = 1.00f; T[6][8][1] = 0.75f; T[6][8][2] = 0.50f; T[6][8][3] = 0.25f;
		// vs Undead Lv1-4
		T[6][9][0] = 1.50f; T[6][9][1] = 1.75f; T[6][9][2] = 2.00f; T[6][9][3] = 2.00f;

		// =====================================================================
		// Shadow attacking
		// =====================================================================
		// vs Neutral Lv1-4
		T[7][0][0] = 1.00f; T[7][0][1] = 1.00f; T[7][0][2] = 1.00f; T[7][0][3] = 1.00f;
		// vs Water Lv1-4
		T[7][1][0] = 1.00f; T[7][1][1] = 0.75f; T[7][1][2] = 0.50f; T[7][1][3] = 0.25f;
		// vs Earth Lv1-4
		T[7][2][0] = 1.00f; T[7][2][1] = 0.75f; T[7][2][2] = 0.50f; T[7][2][3] = 0.25f;
		// vs Fire Lv1-4
		T[7][3][0] = 1.00f; T[7][3][1] = 0.75f; T[7][3][2] = 0.50f; T[7][3][3] = 0.25f;
		// vs Wind Lv1-4
		T[7][4][0] = 1.00f; T[7][4][1] = 0.75f; T[7][4][2] = 0.50f; T[7][4][3] = 0.25f;
		// vs Poison Lv1-4
		T[7][5][0] = 0.50f; T[7][5][1] = 0.25f; T[7][5][2] = 0.00f; T[7][5][3] = -0.25f;
		// vs Holy Lv1-4
		T[7][6][0] = 1.25f; T[7][6][1] = 1.50f; T[7][6][2] = 1.75f; T[7][6][3] = 2.00f;
		// vs Shadow Lv1-4
		T[7][7][0] = 0.00f; T[7][7][1] = 0.00f; T[7][7][2] = -0.25f; T[7][7][3] = -0.50f;
		// vs Ghost Lv1-4
		T[7][8][0] = 1.00f; T[7][8][1] = 0.75f; T[7][8][2] = 0.50f; T[7][8][3] = 0.25f;
		// vs Undead Lv1-4
		T[7][9][0] = 1.00f; T[7][9][1] = 1.25f; T[7][9][2] = 1.50f; T[7][9][3] = 1.75f;

		// =====================================================================
		// Ghost attacking
		// =====================================================================
		// vs Neutral Lv1-4
		T[8][0][0] = 0.25f; T[8][0][1] = 0.25f; T[8][0][2] = 0.25f; T[8][0][3] = 0.25f;
		// vs Water Lv1-4
		T[8][1][0] = 1.00f; T[8][1][1] = 0.75f; T[8][1][2] = 0.50f; T[8][1][3] = 0.25f;
		// vs Earth Lv1-4
		T[8][2][0] = 1.00f; T[8][2][1] = 0.75f; T[8][2][2] = 0.50f; T[8][2][3] = 0.25f;
		// vs Fire Lv1-4
		T[8][3][0] = 1.00f; T[8][3][1] = 0.75f; T[8][3][2] = 0.50f; T[8][3][3] = 0.25f;
		// vs Wind Lv1-4
		T[8][4][0] = 1.00f; T[8][4][1] = 0.75f; T[8][4][2] = 0.50f; T[8][4][3] = 0.25f;
		// vs Poison Lv1-4
		T[8][5][0] = 0.75f; T[8][5][1] = 0.50f; T[8][5][2] = 0.25f; T[8][5][3] = 0.00f;
		// vs Holy Lv1-4
		T[8][6][0] = 0.75f; T[8][6][1] = 0.50f; T[8][6][2] = 0.25f; T[8][6][3] = 0.00f;
		// vs Shadow Lv1-4
		T[8][7][0] = 0.75f; T[8][7][1] = 0.50f; T[8][7][2] = 0.25f; T[8][7][3] = 0.00f;
		// vs Ghost Lv1-4
		T[8][8][0] = 1.25f; T[8][8][1] = 1.50f; T[8][8][2] = 1.75f; T[8][8][3] = 2.00f;
		// vs Undead Lv1-4
		T[8][9][0] = 1.00f; T[8][9][1] = 1.00f; T[8][9][2] = 1.00f; T[8][9][3] = 1.00f;

		// =====================================================================
		// Undead attacking
		// =====================================================================
		// vs Neutral Lv1-4
		T[9][0][0] = 1.00f; T[9][0][1] = 1.00f; T[9][0][2] = 1.00f; T[9][0][3] = 1.00f;
		// vs Water Lv1-4
		T[9][1][0] = 1.00f; T[9][1][1] = 0.75f; T[9][1][2] = 0.50f; T[9][1][3] = 0.25f;
		// vs Earth Lv1-4
		T[9][2][0] = 1.00f; T[9][2][1] = 0.75f; T[9][2][2] = 0.50f; T[9][2][3] = 0.25f;
		// vs Fire Lv1-4
		T[9][3][0] = 1.00f; T[9][3][1] = 0.75f; T[9][3][2] = 0.50f; T[9][3][3] = 0.25f;
		// vs Wind Lv1-4
		T[9][4][0] = 1.00f; T[9][4][1] = 0.75f; T[9][4][2] = 0.50f; T[9][4][3] = 0.25f;
		// vs Poison Lv1-4
		T[9][5][0] = -0.25f; T[9][5][1] = -0.50f; T[9][5][2] = -0.75f; T[9][5][3] = -1.00f;
		// vs Holy Lv1-4
		T[9][6][0] = 1.00f; T[9][6][1] = 1.25f; T[9][6][2] = 1.50f; T[9][6][3] = 1.75f;
		// vs Shadow Lv1-4
		T[9][7][0] = 1.00f; T[9][7][1] = 1.25f; T[9][7][2] = 1.50f; T[9][7][3] = 1.75f;
		// vs Ghost Lv1-4
		T[9][8][0] = 1.00f; T[9][8][1] = 0.75f; T[9][8][2] = 0.50f; T[9][8][3] = 0.25f;
		// vs Undead Lv1-4
		T[9][9][0] = 0.00f; T[9][9][1] = 0.00f; T[9][9][2] = 0.00f; T[9][9][3] = -0.25f;

		return T;
	}();

	return Table;
}

float URODamageFormulas::GetElementalModifier(EROElement AtkElement, EROElement DefElement, EROElementLevel DefLevel)
{
	const int32 AtkIdx = static_cast<int32>(AtkElement);
	const int32 DefIdx = static_cast<int32>(DefElement);
	const int32 LvlIdx = static_cast<int32>(DefLevel);

	const auto& Table = GetElementalTable();

	if (AtkIdx < 0 || AtkIdx >= 10 || DefIdx < 0 || DefIdx >= 10 || LvlIdx < 0 || LvlIdx >= 4)
	{
		return 1.0f; // Fallback
	}

	return Table[AtkIdx][DefIdx][LvlIdx];
}

// ============================================================================
// Stat Point Cost
// ============================================================================

int32 URODamageFormulas::CalculateStatPointCost(int32 CurrentStatValue)
{
	// floor((CurrentStatValue - 1) / 10) + 2
	return (CurrentStatValue - 1) / 10 + 2;
}

// ============================================================================
// Full Damage Calculations
// ============================================================================

int32 URODamageFormulas::CalculatePhysicalDamage(
	int32 BaseATK,
	int32 WeaponATK,
	float SkillModifier,
	int32 TargetHardDEF,
	int32 TargetSoftDEF,
	EROElement AtkElement,
	EROElement DefElement,
	EROElementLevel DefElementLevel,
	float SizeModifier,
	float RaceModifier,
	bool bIsCritical)
{
	// Step 1: Total ATK = BaseATK + WeaponATK
	float TotalATK = static_cast<float>(BaseATK + WeaponATK);

	// Step 2: Apply skill modifier (percentage, 100 = 100%)
	TotalATK = TotalATK * (SkillModifier / 100.0f);

	// Step 3: Apply size modifier
	TotalATK *= SizeModifier;

	// Step 4: Apply race modifier
	TotalATK *= RaceModifier;

	// Step 5: Apply elemental modifier
	const float ElemMod = GetElementalModifier(AtkElement, DefElement, DefElementLevel);
	TotalATK *= ElemMod;

	// Step 6: Subtract Hard DEF (flat reduction)
	// Critical hits ignore hard DEF in pre-renewal
	if (!bIsCritical)
	{
		TotalATK -= static_cast<float>(CalculateHardDEF(TargetHardDEF));
	}

	// Step 7: Subtract Soft DEF (flat reduction from VIT)
	// Critical hits also ignore soft DEF
	if (!bIsCritical)
	{
		TotalATK -= static_cast<float>(TargetSoftDEF);
	}

	// Minimum damage is 1
	return FMath::Max(1, static_cast<int32>(TotalATK));
}

int32 URODamageFormulas::CalculateMagicalDamage(
	int32 MATK,
	float SkillModifier,
	int32 TargetMDEF_Hard,
	int32 TargetMDEF_Soft,
	EROElement AtkElement,
	EROElement DefElement,
	EROElementLevel DefElementLevel,
	float RaceModifier)
{
	// Step 1: Base magic damage
	float TotalDmg = static_cast<float>(MATK);

	// Step 2: Apply skill modifier
	TotalDmg = TotalDmg * (SkillModifier / 100.0f);

	// Step 3: Apply race modifier
	TotalDmg *= RaceModifier;

	// Step 4: Apply elemental modifier
	const float ElemMod = GetElementalModifier(AtkElement, DefElement, DefElementLevel);
	TotalDmg *= ElemMod;

	// Step 5: Subtract Hard MDEF (flat reduction)
	TotalDmg -= static_cast<float>(FMath::Max(0, TargetMDEF_Hard));

	// Step 6: Subtract Soft MDEF (flat reduction from INT)
	TotalDmg -= static_cast<float>(TargetMDEF_Soft);

	// Minimum damage is 1
	return FMath::Max(1, static_cast<int32>(TotalDmg));
}
