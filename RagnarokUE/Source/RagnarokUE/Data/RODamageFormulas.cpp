// Copyright Ragna-TH Project. All Rights Reserved.

#include "RODamageFormulas.h"
#include "RagnarokUE/Data/ROConstants.h"
#include "RagnarokUE/Combat/ROElementalSystem.h"

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
	// Pre-renewal: VIT/2 + max(VIT*0.3, VIT^2/150 - 1)
	const float VitHalf = static_cast<float>(VIT) / 2.0f;
	const float Option1 = static_cast<float>(VIT) * 0.3f;
	const float Option2 = static_cast<float>(VIT * VIT) / 150.0f - 1.0f;
	return FMath::Max(0, static_cast<int32>(VitHalf + FMath::Max(Option1, Option2)));
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
	// Pre-renewal formula: 1 + LUK * 0.3
	return 1.0f + (static_cast<float>(LUK) * 0.3f);
}

// ============================================================================
// Attack Speed
// ============================================================================

int32 URODamageFormulas::GetWeaponASPDOffset(EROWeaponType WeaponType)
{
	switch (WeaponType)
	{
	case EROWeaponType::Dagger:       return 0;
	case EROWeaponType::Sword:        return 5;
	case EROWeaponType::TwoHandSword: return 10;
	case EROWeaponType::Spear:        return 5;
	case EROWeaponType::TwoHandSpear: return 10;
	case EROWeaponType::Axe:          return 5;
	case EROWeaponType::TwoHandAxe:   return 10;
	case EROWeaponType::Mace:         return 5;
	case EROWeaponType::TwoHandMace:  return 10;
	case EROWeaponType::Rod:          return 5;
	case EROWeaponType::TwoHandRod:   return 10;
	case EROWeaponType::Bow:          return 5;
	case EROWeaponType::Knuckle:      return 0;
	case EROWeaponType::Katar:        return -5;
	case EROWeaponType::Book:         return 5;
	case EROWeaponType::Instrument:   return 5;
	case EROWeaponType::Whip:         return 5;
	case EROWeaponType::Gun:          return 0;
	case EROWeaponType::Shuriken:     return 0;
	default:                          return 0;
	}
}

int32 URODamageFormulas::GetBaseASPDForJob(EROJobClass Job, EROWeaponType WeaponType)
{
	int32 JobBase = 150;

	switch (Job)
	{
	// Novice
	case EROJobClass::Novice:
	case EROJobClass::HighNovice:
		JobBase = 150;
		break;

	// Swordsman line
	case EROJobClass::Swordsman:
	case EROJobClass::HighSwordsman:
		JobBase = 145;
		break;
	case EROJobClass::Knight:
	case EROJobClass::Crusader:
	case EROJobClass::LordKnight:
	case EROJobClass::Paladin:
		JobBase = 145;
		break;

	// Magician line
	case EROJobClass::Magician:
	case EROJobClass::HighMagician:
		JobBase = 150;
		break;
	case EROJobClass::Wizard:
	case EROJobClass::Sage:
	case EROJobClass::HighWizard:
	case EROJobClass::Professor:
		JobBase = 150;
		break;

	// Archer line
	case EROJobClass::Archer:
	case EROJobClass::HighArcher:
		JobBase = 145;
		break;
	case EROJobClass::Hunter:
	case EROJobClass::Bard:
	case EROJobClass::Dancer:
	case EROJobClass::Sniper:
	case EROJobClass::Minstrel:
	case EROJobClass::Gypsy:
		JobBase = 145;
		break;

	// Thief line
	case EROJobClass::Thief:
	case EROJobClass::HighThief:
		JobBase = 140;
		break;
	case EROJobClass::Assassin:
	case EROJobClass::Rogue:
	case EROJobClass::AssassinCross:
	case EROJobClass::Stalker:
		JobBase = 140;
		break;

	// Merchant line
	case EROJobClass::Merchant:
	case EROJobClass::HighMerchant:
		JobBase = 150;
		break;
	case EROJobClass::Blacksmith:
	case EROJobClass::Alchemist:
	case EROJobClass::Whitesmith:
	case EROJobClass::Creator:
		JobBase = 150;
		break;

	// Acolyte line
	case EROJobClass::Acolyte:
	case EROJobClass::HighAcolyte:
		JobBase = 150;
		break;
	case EROJobClass::Priest:
	case EROJobClass::HighPriest:
		JobBase = 150;
		break;
	case EROJobClass::Monk:
	case EROJobClass::Champion:
		JobBase = 145;
		break;

	default:
		JobBase = 150;
		break;
	}

	return JobBase + GetWeaponASPDOffset(WeaponType);
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
	const int32 INTBonus = (INT / 5) * (INT / 5);
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
// Size Modifiers
// ============================================================================

float URODamageFormulas::GetWeaponSizeModifier(EROWeaponType WeaponType, EROMonsterSize TargetSize)
{
	// Pre-renewal weapon-type size penalty table.
	// Values: S = Small, M = Medium, L = Large (as percentage / 100).
	//                                    Small   Medium  Large
	switch (WeaponType)
	{
	case EROWeaponType::Dagger:       // 100%    75%     50%
		return (TargetSize == EROMonsterSize::Small) ? 1.00f : (TargetSize == EROMonsterSize::Medium) ? 0.75f : 0.50f;
	case EROWeaponType::Sword:        // 75%     100%    75%
		return (TargetSize == EROMonsterSize::Medium) ? 1.00f : 0.75f;
	case EROWeaponType::TwoHandSword: // 75%     75%     100%
		return (TargetSize == EROMonsterSize::Large) ? 1.00f : 0.75f;
	case EROWeaponType::Spear:        // 75%     75%     100%
		return (TargetSize == EROMonsterSize::Large) ? 1.00f : 0.75f;
	case EROWeaponType::TwoHandSpear: // 75%     75%     100%
		return (TargetSize == EROMonsterSize::Large) ? 1.00f : 0.75f;
	case EROWeaponType::Axe:          // 50%     75%     100%
		return (TargetSize == EROMonsterSize::Small) ? 0.50f : (TargetSize == EROMonsterSize::Medium) ? 0.75f : 1.00f;
	case EROWeaponType::TwoHandAxe:   // 50%     75%     100%
		return (TargetSize == EROMonsterSize::Small) ? 0.50f : (TargetSize == EROMonsterSize::Medium) ? 0.75f : 1.00f;
	case EROWeaponType::Mace:         // 75%     100%    100%
		return (TargetSize == EROMonsterSize::Small) ? 0.75f : 1.00f;
	case EROWeaponType::TwoHandMace:  // 75%     100%    100%
		return (TargetSize == EROMonsterSize::Small) ? 0.75f : 1.00f;
	case EROWeaponType::Rod:          // 100%    100%    100%
		return 1.00f;
	case EROWeaponType::TwoHandRod:   // 100%    100%    100%
		return 1.00f;
	case EROWeaponType::Bow:          // 100%    100%    75%
		return (TargetSize == EROMonsterSize::Large) ? 0.75f : 1.00f;
	case EROWeaponType::Knuckle:      // 100%    75%     50%
		return (TargetSize == EROMonsterSize::Small) ? 1.00f : (TargetSize == EROMonsterSize::Medium) ? 0.75f : 0.50f;
	case EROWeaponType::Katar:        // 75%     100%    75%
		return (TargetSize == EROMonsterSize::Medium) ? 1.00f : 0.75f;
	case EROWeaponType::Book:         // 100%    100%    50%
		return (TargetSize == EROMonsterSize::Large) ? 0.50f : 1.00f;
	case EROWeaponType::Instrument:   // 75%     100%    75%
		return (TargetSize == EROMonsterSize::Medium) ? 1.00f : 0.75f;
	case EROWeaponType::Whip:         // 75%     100%    50%
		return (TargetSize == EROMonsterSize::Small) ? 0.75f : (TargetSize == EROMonsterSize::Medium) ? 1.00f : 0.50f;
	case EROWeaponType::Gun:          // 100%    100%    100%
		return 1.00f;
	case EROWeaponType::Shuriken:     // 100%    100%    100%
		return 1.00f;
	default:
		return 1.00f;
	}
}

// ============================================================================
// Elemental System
// ============================================================================

float URODamageFormulas::GetElementalModifier(EROElement AtkElement, EROElement DefElement, EROElementLevel DefLevel)
{
	// Delegate to the single authoritative elemental table in UROElementalSystem
	return UROElementalSystem::GetElementalModifier(AtkElement, DefElement, DefLevel);
}

// ============================================================================
// Stat Point Cost
// ============================================================================

int32 URODamageFormulas::CalculateStatPointCost(int32 CurrentStatValue)
{
	// Guard: can't raise a stat below 1; return 0 to indicate no cost / invalid
	if (CurrentStatValue < 1)
	{
		return 0;
	}
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
	bool bIsCritical,
	float CardFixModifier)
{
	// Step 1: Total ATK = BaseATK + WeaponATK
	float TotalATK = static_cast<float>(BaseATK + WeaponATK);

	// Step 2: Apply skill modifier (percentage, 100 = 100%)
	TotalATK = TotalATK * (SkillModifier / 100.0f);

	// Step 3: Apply race modifier
	TotalATK *= RaceModifier;

	// Step 3b: Apply card fix modifier
	TotalATK *= CardFixModifier;

	// Step 4: Subtract Hard DEF (flat reduction) -- before elemental/size modifiers (pre-renewal order)
	// Critical hits ignore hard DEF in pre-renewal
	if (!bIsCritical)
	{
		TotalATK -= static_cast<float>(CalculateHardDEF(TargetHardDEF));
	}

	// Step 5: Subtract Soft DEF (flat reduction from VIT)
	// Critical hits also ignore soft DEF
	if (!bIsCritical)
	{
		// Pre-renewal: soft DEF has random variance (rnd() % softDEF), range [0, softDEF-1]
		const int32 EffectiveSoftDEF = (TargetSoftDEF > 0) ? FMath::RandRange(0, TargetSoftDEF - 1) : 0;
		TotalATK -= static_cast<float>(EffectiveSoftDEF);
	}

	// Allow negative values to flow through to elemental modifier for absorb.
	// Do NOT clamp to 1 here -- that blocks elemental absorb from working.

	// Step 6: Apply elemental modifier (after DEF subtraction, pre-renewal order)
	const float ElemMod = GetElementalModifier(AtkElement, DefElement, DefElementLevel);
	TotalATK *= ElemMod;

	// Step 7: Apply size modifier
	TotalATK *= SizeModifier;

	// Allow 0 (miss/immunity) and negative (elemental absorb) through.
	// Only enforce min-1 when damage is positive (we actually want to deal damage).
	if (TotalATK > 0.0f)
	{
		return FMath::Max(1, static_cast<int32>(TotalATK));
	}
	return static_cast<int32>(TotalATK);
}

int32 URODamageFormulas::CalculateMagicalDamage(
	int32 MATK,
	float SkillModifier,
	int32 TargetMDEF_Hard,
	int32 TargetMDEF_Soft,
	EROElement AtkElement,
	EROElement DefElement,
	EROElementLevel DefElementLevel,
	float RaceModifier,
	float CardFixModifier)
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

	// Step 4b: Apply card fix modifier
	TotalDmg *= CardFixModifier;

	// Step 5: Subtract Hard MDEF (flat reduction)
	TotalDmg -= static_cast<float>(FMath::Max(0, TargetMDEF_Hard));

	// Step 6: Subtract Soft MDEF (flat reduction from INT)
	TotalDmg -= static_cast<float>(TargetMDEF_Soft);

	// Allow 0 (immunity) and negative (elemental absorb) through.
	// Only enforce min-1 when damage is positive.
	if (TotalDmg > 0.0f)
	{
		return FMath::Max(1, static_cast<int32>(TotalDmg));
	}
	return static_cast<int32>(TotalDmg);
}
