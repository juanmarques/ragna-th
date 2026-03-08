// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROMonsterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterDied, AROMonsterBase*, Monster, AActor*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMonsterDamaged, AROMonsterBase*, Monster, float, Damage, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMonsterHPChanged, AROMonsterBase*, Monster, int32, NewHP, int32, MaxHP);

/**
 * AROMonsterBase
 * Base character class for all monsters in Ragnarok Online UE5 recreation.
 * Handles stats, combat, death, loot generation, and EXP distribution.
 */
UCLASS(Blueprintable)
class RAGNAROKUE_API AROMonsterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AROMonsterBase();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ---- Identity ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Monster|Identity")
	int32 MonsterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Monster|Identity")
	FName MonsterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Identity")
	FText DisplayName;

	// ---- Combat Stats ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_HP, Category = "Monster|Stats")
	int32 HP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Monster|Stats")
	int32 MaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	int32 ATK;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	int32 ATKMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	int32 ATKMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	int32 MATK;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	int32 DEF;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	int32 MDEF;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	int32 HIT;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	int32 FLEE;

	// ---- Element & Type ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Monster|Element")
	EROElement Element;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Element")
	EROElementLevel ElementLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Type")
	EROMonsterSize Size;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Type")
	EROMonsterRace Race;

	// ---- Behavior ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|AI")
	EROMonsterBehavior Behavior;

	// ---- Rewards ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Reward")
	int64 BaseExpReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Reward")
	int64 JobExpReward;

	// ---- Range & Speed ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Combat")
	float AggroRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Combat")
	float ChaseRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Combat")
	float AttackRange;

	/** Delay between attacks in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Combat")
	float AttackSpeed;

	// ---- Drop Table ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Drops")
	TArray<FRODropInfo> DropTable;

	// ---- Skills ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Skills")
	TArray<FROMonsterSkillEntry> SkillList;

	/** Cooldown tracker: SkillID -> world time when cooldown expires. */
	UPROPERTY()
	TMap<int32, float> SkillCooldowns;

	// ---- Boss / MVP ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Monster|Type")
	bool bIsMVP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Monster|Type")
	bool bIsBoss;

	// ---- State ----

	UPROPERTY(ReplicatedUsing = OnRep_IsDead, BlueprintReadOnly, Category = "Monster|State")
	bool bIsDead;

	UPROPERTY(BlueprintReadOnly, Category = "Monster|State")
	FVector SpawnLocation;

	/** Index into the spawn manager's SpawnDefinitions array. Used to track which definition spawned this monster. */
	UPROPERTY()
	int32 SpawnDefIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Monster|State")
	TObjectPtr<AActor> CurrentTarget;

	/** Threat table: maps attacker to total damage dealt. Used for EXP/loot distribution. */
	UPROPERTY()
	TMap<TObjectPtr<AActor>, float> ThreatTable;

	/** Time of last attack, for attack speed gating. */
	UPROPERTY()
	float LastAttackTime;

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category = "Monster|Events")
	FOnMonsterDied OnMonsterDied;

	UPROPERTY(BlueprintAssignable, Category = "Monster|Events")
	FOnMonsterDamaged OnMonsterDamaged;

	UPROPERTY(BlueprintAssignable, Category = "Monster|Events")
	FOnMonsterHPChanged OnMonsterHPChanged;

	// ---- Functions ----

	/** Initialize this monster from database data. Call on server after spawn. */
	UFUNCTION(BlueprintCallable, Category = "Monster")
	void InitializeFromData(const FROMonsterData& Data);

	/** Override TakeDamage for RO damage pipeline. */
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	/** Kill this monster, generate loot and distribute EXP. */
	UFUNCTION(BlueprintCallable, Category = "Monster")
	void Die(AActor* Killer);

	/** Get the actor with the highest threat. */
	UFUNCTION(BlueprintCallable, Category = "Monster|AI")
	AActor* GetHighestThreatTarget() const;

	/** Add threat for an attacker. */
	UFUNCTION(BlueprintCallable, Category = "Monster|AI")
	void AddThreat(AActor* Attacker, float Amount);

	/** Check if a skill is off cooldown. */
	UFUNCTION(BlueprintCallable, Category = "Monster|Skills")
	bool IsSkillReady(int32 SkillID) const;

	/** Put a skill on cooldown. */
	UFUNCTION(BlueprintCallable, Category = "Monster|Skills")
	void SetSkillCooldown(int32 SkillID, float CooldownDuration);

	/** Reset monster to full HP and clear state (used on return-to-home). */
	UFUNCTION(BlueprintCallable, Category = "Monster")
	void ResetToIdle();

	/** Can this monster currently attack (attack speed gating). */
	UFUNCTION(BlueprintCallable, Category = "Monster|Combat")
	bool CanAttack() const;

	/** Record that an attack was performed. */
	UFUNCTION(BlueprintCallable, Category = "Monster|Combat")
	void MarkAttackPerformed();

protected:
	UFUNCTION()
	void OnRep_HP();

	UFUNCTION()
	void OnRep_IsDead();

	/** Multicast death notification for VFX/SFX. */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath(AActor* Killer);

	/** Generate loot drops at death location. Server only. */
	void GenerateLoot();

	/** Distribute EXP to attackers based on threat table. Server only. */
	void DistributeExp();
};
