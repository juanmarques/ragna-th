// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROStatsComponent.h"
#include "ROLevelingComponent.h"
#include "ROJobComponent.h"
#include "RagnarokUE/Data/ROConstants.h"
#include "RagnarokUE/Data/RODamageFormulas.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"

UROStatsComponent::UROStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// Base stats default to 1
	BaseSTR = 1;
	BaseAGI = 1;
	BaseVIT = 1;
	BaseINT = 1;
	BaseDEX = 1;
	BaseLUK = 1;

	// Bonus stats default to 0
	BonusSTR = 0;
	BonusAGI = 0;
	BonusVIT = 0;
	BonusINT = 0;
	BonusDEX = 0;
	BonusLUK = 0;

	AvailableStatPoints = 48; // Novice starts with 48

	// Initialize totals
	TotalSTR = 1;
	TotalAGI = 1;
	TotalVIT = 1;
	TotalINT = 1;
	TotalDEX = 1;
	TotalLUK = 1;

	// Initialize derived
	ATK = 0;
	MATK_Min = 0;
	MATK_Max = 0;
	DEF = 0;
	MDEF = 0;
	HIT = 0;
	FLEE = 0;
	PerfectDodge = 0;
	ASPD = 0.0f;
	CritRate = 1;
	MaxHP = 0;
	MaxSP = 0;
}

void UROStatsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UROStatsComponent, BaseSTR);
	DOREPLIFETIME(UROStatsComponent, BaseAGI);
	DOREPLIFETIME(UROStatsComponent, BaseVIT);
	DOREPLIFETIME(UROStatsComponent, BaseINT);
	DOREPLIFETIME(UROStatsComponent, BaseDEX);
	DOREPLIFETIME(UROStatsComponent, BaseLUK);

	DOREPLIFETIME(UROStatsComponent, BonusSTR);
	DOREPLIFETIME(UROStatsComponent, BonusAGI);
	DOREPLIFETIME(UROStatsComponent, BonusVIT);
	DOREPLIFETIME(UROStatsComponent, BonusINT);
	DOREPLIFETIME(UROStatsComponent, BonusDEX);
	DOREPLIFETIME(UROStatsComponent, BonusLUK);

	DOREPLIFETIME(UROStatsComponent, AvailableStatPoints);
}

void UROStatsComponent::BeginPlay()
{
	Super::BeginPlay();
	RecalculateTotalStats();
	RecalculateDerivedStats();
}

// ---- OnRep Callbacks ----

void UROStatsComponent::OnRep_BaseSTR()  { RecalculateTotalStats(); RecalculateDerivedStats(); }
void UROStatsComponent::OnRep_BaseAGI()  { RecalculateTotalStats(); RecalculateDerivedStats(); }
void UROStatsComponent::OnRep_BaseVIT()  { RecalculateTotalStats(); RecalculateDerivedStats(); }
void UROStatsComponent::OnRep_BaseINT()  { RecalculateTotalStats(); RecalculateDerivedStats(); }
void UROStatsComponent::OnRep_BaseDEX()  { RecalculateTotalStats(); RecalculateDerivedStats(); }
void UROStatsComponent::OnRep_BaseLUK()  { RecalculateTotalStats(); RecalculateDerivedStats(); }

void UROStatsComponent::OnRep_BonusSTR() { RecalculateTotalStats(); RecalculateDerivedStats(); }
void UROStatsComponent::OnRep_BonusAGI() { RecalculateTotalStats(); RecalculateDerivedStats(); }
void UROStatsComponent::OnRep_BonusVIT() { RecalculateTotalStats(); RecalculateDerivedStats(); }
void UROStatsComponent::OnRep_BonusINT() { RecalculateTotalStats(); RecalculateDerivedStats(); }
void UROStatsComponent::OnRep_BonusDEX() { RecalculateTotalStats(); RecalculateDerivedStats(); }
void UROStatsComponent::OnRep_BonusLUK() { RecalculateTotalStats(); RecalculateDerivedStats(); }

// ---- Core Logic ----

void UROStatsComponent::RecalculateTotalStats()
{
	TotalSTR = BaseSTR + BonusSTR;
	TotalAGI = BaseAGI + BonusAGI;
	TotalVIT = BaseVIT + BonusVIT;
	TotalINT = BaseINT + BonusINT;
	TotalDEX = BaseDEX + BonusDEX;
	TotalLUK = BaseLUK + BonusLUK;
}

void UROStatsComponent::RecalculateDerivedStats()
{
	const int32 BaseLevel = GetOwnerBaseLevel();

	// --- ATK (Status ATK, weapon ATK is separate) ---
	// StatusATK = BaseLevel/4 + STR + DEX/5 + LUK/3
	ATK = URODamageFormulas::CalculateBaseATK(TotalSTR, TotalDEX, TotalLUK);

	// --- MATK ---
	// MATK_Min = INT + (INT/7)^2
	// MATK_Max = INT + (INT/5)^2
	const FVector2D MATKRange = URODamageFormulas::CalculateBaseMATK(TotalINT);
	MATK_Min = static_cast<int32>(MATKRange.X);
	MATK_Max = static_cast<int32>(MATKRange.Y);

	// --- Soft DEF (from VIT) ---
	// SoftDEF = BaseLevel/2 + VIT/2 (hard DEF comes from equipment, stored elsewhere)
	DEF = URODamageFormulas::CalculateSoftDEF(TotalVIT);

	// --- Soft MDEF ---
	// SoftMDEF = INT + VIT/5 + DEX/5 + BaseLevel/4
	MDEF = URODamageFormulas::CalculateSoftMDEF(TotalINT, TotalVIT, TotalDEX);

	// --- HIT ---
	// HIT = BaseLevel + DEX + LUK/3 + 175
	HIT = URODamageFormulas::CalculateHitRate(TotalDEX, TotalLUK, BaseLevel);

	// --- FLEE ---
	// FLEE = BaseLevel + AGI + LUK/5 + 100
	FLEE = URODamageFormulas::CalculateFleeRate(TotalAGI, TotalLUK, BaseLevel);

	// --- Perfect Dodge ---
	// PerfectDodge = LUK/10 + 1
	PerfectDodge = TotalLUK / 10 + 1;

	// --- ASPD ---
	// Base ASPD depends on weapon type and job class; simplified formula:
	// ASPD = 200 - (BaseASPD - (AGI + DEX/4))
	// We store the effective ASPD value (higher = faster, max 190)
	ASPD = URODamageFormulas::CalculateASPD(URODamageFormulas::GetBaseASPDForJob(GetOwnerJobClass()), TotalAGI, TotalDEX);

	// --- Critical Rate ---
	// Crit = LUK * 0.3 + 1
	CritRate = FMath::Max(1, FMath::FloorToInt(TotalLUK * 0.3f) + 1);

	// --- MaxHP ---
	// Formula varies by job; simplified: base HP table[level] * (1 + VIT/100)
	MaxHP = URODamageFormulas::CalculateMaxHP(BaseLevel, TotalVIT, GetOwnerJobClass());

	// --- MaxSP ---
	// SP = base SP table[level] * (1 + INT/100)
	MaxSP = URODamageFormulas::CalculateMaxSP(BaseLevel, TotalINT, GetOwnerJobClass());
}

int32 UROStatsComponent::GetStatPointCost(int32 CurrentBaseStatValue)
{
	// In RO, cost to raise a stat by 1 = floor((CurrentBase - 1) / 10) + 2
	// At stat 1: cost 2, stat 10: cost 2, stat 11: cost 3, stat 20: cost 3, stat 21: cost 4 ...
	// This means stat 99 costs 12 points
	if (CurrentBaseStatValue < 1)
	{
		return 2;
	}
	return FMath::FloorToInt(static_cast<float>(CurrentBaseStatValue - 1) / 10.0f) + 2;
}

bool UROStatsComponent::ServerAllocateStat_Validate(EROStat Stat)
{
	// Basic validation - stat enum must be valid
	return static_cast<uint8>(Stat) <= static_cast<uint8>(EROStat::LUK);
}

void UROStatsComponent::ServerAllocateStat_Implementation(EROStat Stat)
{
	// Get pointer to the base stat we're modifying
	int32* BaseStat = nullptr;
	switch (Stat)
	{
	case EROStat::STR: BaseStat = &BaseSTR; break;
	case EROStat::AGI: BaseStat = &BaseAGI; break;
	case EROStat::VIT: BaseStat = &BaseVIT; break;
	case EROStat::INT_STAT: BaseStat = &BaseINT; break;
	case EROStat::DEX: BaseStat = &BaseDEX; break;
	case EROStat::LUK: BaseStat = &BaseLUK; break;
	default: return;
	}

	// Check cap (99 is the classic max)
	if (*BaseStat >= ROConstants::MaxStats)
	{
		return;
	}

	// Check cost
	const int32 Cost = GetStatPointCost(*BaseStat);
	if (AvailableStatPoints < Cost)
	{
		return;
	}

	// Deduct points and increment stat
	AvailableStatPoints -= Cost;
	(*BaseStat)++;

	// Recalculate everything
	RecalculateTotalStats();
	RecalculateDerivedStats();
}

int32 UROStatsComponent::GetStatValue(EROStat Stat) const
{
	switch (Stat)
	{
	case EROStat::STR: return BaseSTR;
	case EROStat::AGI: return BaseAGI;
	case EROStat::VIT: return BaseVIT;
	case EROStat::INT_STAT: return BaseINT;
	case EROStat::DEX: return BaseDEX;
	case EROStat::LUK: return BaseLUK;
	default: return 0;
	}
}

int32 UROStatsComponent::GetTotalStat(EROStat Stat) const
{
	switch (Stat)
	{
	case EROStat::STR: return TotalSTR;
	case EROStat::AGI: return TotalAGI;
	case EROStat::VIT: return TotalVIT;
	case EROStat::INT_STAT: return TotalINT;
	case EROStat::DEX: return TotalDEX;
	case EROStat::LUK: return TotalLUK;
	default: return 0;
	}
}

void UROStatsComponent::AddBonusStat(EROStat Stat, int32 Amount)
{
	switch (Stat)
	{
	case EROStat::STR: BonusSTR += Amount; break;
	case EROStat::AGI: BonusAGI += Amount; break;
	case EROStat::VIT: BonusVIT += Amount; break;
	case EROStat::INT_STAT: BonusINT += Amount; break;
	case EROStat::DEX: BonusDEX += Amount; break;
	case EROStat::LUK: BonusLUK += Amount; break;
	default: return;
	}
	RecalculateTotalStats();
	RecalculateDerivedStats();
}

void UROStatsComponent::RemoveBonusStat(EROStat Stat, int32 Amount)
{
	AddBonusStat(Stat, -Amount);
}

int32 UROStatsComponent::GetOwnerBaseLevel() const
{
	if (AActor* Owner = GetOwner())
	{
		if (UROLevelingComponent* LevelComp = Owner->FindComponentByClass<UROLevelingComponent>())
		{
			return LevelComp->BaseLevel;
		}
	}
	return 1;
}

EROJobClass UROStatsComponent::GetOwnerJobClass() const
{
	if (AActor* Owner = GetOwner())
	{
		if (UROJobComponent* JobComp = Owner->FindComponentByClass<UROJobComponent>())
		{
			return JobComp->CurrentJobClass;
		}
	}
	return EROJobClass::Novice;
}
