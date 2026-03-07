// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROStatsComponent.generated.h"

/**
 * UROStatsComponent
 * Manages the 6-stat system (STR, AGI, VIT, INT, DEX, LUK) plus derived combat stats.
 * All base/bonus stats are replicated. Derived stats are computed on both server and client.
 */
UCLASS(ClassGroup=(RagnarokUE), meta=(BlueprintSpawnableComponent))
class RAGNAROKUE_API UROStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UROStatsComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	// ---- Base Stats (allocated by player) ----
	UPROPERTY(ReplicatedUsing=OnRep_BaseSTR, EditAnywhere, BlueprintReadOnly, Category="Stats|Base")
	int32 BaseSTR;

	UPROPERTY(ReplicatedUsing=OnRep_BaseAGI, EditAnywhere, BlueprintReadOnly, Category="Stats|Base")
	int32 BaseAGI;

	UPROPERTY(ReplicatedUsing=OnRep_BaseVIT, EditAnywhere, BlueprintReadOnly, Category="Stats|Base")
	int32 BaseVIT;

	UPROPERTY(ReplicatedUsing=OnRep_BaseINT, EditAnywhere, BlueprintReadOnly, Category="Stats|Base")
	int32 BaseINT;

	UPROPERTY(ReplicatedUsing=OnRep_BaseDEX, EditAnywhere, BlueprintReadOnly, Category="Stats|Base")
	int32 BaseDEX;

	UPROPERTY(ReplicatedUsing=OnRep_BaseLUK, EditAnywhere, BlueprintReadOnly, Category="Stats|Base")
	int32 BaseLUK;

	// ---- Bonus Stats (from equipment, buffs, cards) ----
	UPROPERTY(ReplicatedUsing=OnRep_BonusSTR, EditAnywhere, BlueprintReadOnly, Category="Stats|Bonus")
	int32 BonusSTR;

	UPROPERTY(ReplicatedUsing=OnRep_BonusAGI, EditAnywhere, BlueprintReadOnly, Category="Stats|Bonus")
	int32 BonusAGI;

	UPROPERTY(ReplicatedUsing=OnRep_BonusVIT, EditAnywhere, BlueprintReadOnly, Category="Stats|Bonus")
	int32 BonusVIT;

	UPROPERTY(ReplicatedUsing=OnRep_BonusINT, EditAnywhere, BlueprintReadOnly, Category="Stats|Bonus")
	int32 BonusINT;

	UPROPERTY(ReplicatedUsing=OnRep_BonusDEX, EditAnywhere, BlueprintReadOnly, Category="Stats|Bonus")
	int32 BonusDEX;

	UPROPERTY(ReplicatedUsing=OnRep_BonusLUK, EditAnywhere, BlueprintReadOnly, Category="Stats|Bonus")
	int32 BonusLUK;

	// ---- Available stat points from leveling ----
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category="Stats")
	int32 AvailableStatPoints;

	// ---- Computed Total Stats (Base + Bonus, not replicated) ----
	UPROPERTY(BlueprintReadOnly, Category="Stats|Total")
	int32 TotalSTR;

	UPROPERTY(BlueprintReadOnly, Category="Stats|Total")
	int32 TotalAGI;

	UPROPERTY(BlueprintReadOnly, Category="Stats|Total")
	int32 TotalVIT;

	UPROPERTY(BlueprintReadOnly, Category="Stats|Total")
	int32 TotalINT;

	UPROPERTY(BlueprintReadOnly, Category="Stats|Total")
	int32 TotalDEX;

	UPROPERTY(BlueprintReadOnly, Category="Stats|Total")
	int32 TotalLUK;

	// ---- Derived Combat Stats ----
	UPROPERTY(BlueprintReadOnly, Category="Stats|Derived")
	int32 ATK;

	UPROPERTY(BlueprintReadOnly, Category="Stats|Derived")
	int32 MATK_Min;

	UPROPERTY(BlueprintReadOnly, Category="Stats|Derived")
	int32 MATK_Max;

	UPROPERTY(BlueprintReadOnly, Category="Stats|Derived")
	int32 DEF;

	UPROPERTY(BlueprintReadOnly, Category="Stats|Derived")
	int32 MDEF;

	UPROPERTY(BlueprintReadOnly, Category="Stats|Derived")
	int32 HIT;

	UPROPERTY(BlueprintReadOnly, Category="Stats|Derived")
	int32 FLEE;

	UPROPERTY(BlueprintReadOnly, Category="Stats|Derived")
	int32 PerfectDodge;

	UPROPERTY(BlueprintReadOnly, Category="Stats|Derived")
	float ASPD;

	UPROPERTY(BlueprintReadOnly, Category="Stats|Derived")
	int32 CritRate;

	UPROPERTY(BlueprintReadOnly, Category="Stats|Derived")
	int32 MaxHP;

	UPROPERTY(BlueprintReadOnly, Category="Stats|Derived")
	int32 MaxSP;

	// ---- Functions ----

	/** Server-authoritative stat allocation. Validates cost, deducts points, increments stat. */
	UFUNCTION(Server, Reliable, WithValidation, Category="Stats")
	void ServerAllocateStat(EROStat Stat);

	/** Get the base value of a stat. */
	UFUNCTION(BlueprintCallable, Category="Stats")
	int32 GetStatValue(EROStat Stat) const;

	/** Get the total value (base + bonus) of a stat. */
	UFUNCTION(BlueprintCallable, Category="Stats")
	int32 GetTotalStat(EROStat Stat) const;

	/** Recalculate all derived stats from current base + bonus values. */
	UFUNCTION(BlueprintCallable, Category="Stats")
	void RecalculateDerivedStats();

	/** Get the stat point cost to raise a stat by 1 from its current base value. */
	UFUNCTION(BlueprintCallable, Category="Stats")
	static int32 GetStatPointCost(int32 CurrentBaseStatValue);

	/** Add bonus stats (e.g., from equipping an item). Triggers recalculation. */
	UFUNCTION(BlueprintCallable, Category="Stats")
	void AddBonusStat(EROStat Stat, int32 Amount);

	/** Remove bonus stats (e.g., from unequipping an item). Triggers recalculation. */
	UFUNCTION(BlueprintCallable, Category="Stats")
	void RemoveBonusStat(EROStat Stat, int32 Amount);

protected:
	UFUNCTION()
	void OnRep_BaseSTR();
	UFUNCTION()
	void OnRep_BaseAGI();
	UFUNCTION()
	void OnRep_BaseVIT();
	UFUNCTION()
	void OnRep_BaseINT();
	UFUNCTION()
	void OnRep_BaseDEX();
	UFUNCTION()
	void OnRep_BaseLUK();

	UFUNCTION()
	void OnRep_BonusSTR();
	UFUNCTION()
	void OnRep_BonusAGI();
	UFUNCTION()
	void OnRep_BonusVIT();
	UFUNCTION()
	void OnRep_BonusINT();
	UFUNCTION()
	void OnRep_BonusDEX();
	UFUNCTION()
	void OnRep_BonusLUK();

private:
	void RecalculateTotalStats();

	/** Helper to get the base level from the owning character's leveling component. */
	int32 GetOwnerBaseLevel() const;

	/** Helper to get the job class from the owning character's job component. */
	EROJobClass GetOwnerJobClass() const;
};
