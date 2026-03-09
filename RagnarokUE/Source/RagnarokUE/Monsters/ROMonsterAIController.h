// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "ROMonsterAIController.generated.h"

class AROMonsterBase;
class UBehaviorTree;
class UBlackboardData;
class UBlackboardComponent;

/**
 * AROMonsterAIController
 * AI Controller for all monsters. Selects behavior tree based on monster type,
 * manages blackboard state, and handles target acquisition / threat updates.
 */
UCLASS()
class RAGNAROKUE_API AROMonsterAIController : public AAIController
{
	GENERATED_BODY()

public:
	AROMonsterAIController();

	// ---- Behavior Trees (assigned per behavior type) ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster AI|BehaviorTrees")
	TObjectPtr<UBehaviorTree> BT_Passive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster AI|BehaviorTrees")
	TObjectPtr<UBehaviorTree> BT_Aggressive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster AI|BehaviorTrees")
	TObjectPtr<UBehaviorTree> BT_Assist;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster AI|BehaviorTrees")
	TObjectPtr<UBehaviorTree> BT_CastSensor;

	/** Shared blackboard data asset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster AI")
	TObjectPtr<UBlackboardData> MonsterBlackboardData;

	// ---- Blackboard Key Names ----

	static const FName BB_TargetActor;
	static const FName BB_HomeLocation;
	static const FName BB_AggroRange;
	static const FName BB_AttackRange;
	static const FName BB_ChaseRange;
	static const FName BB_IsInCombat;
	static const FName BB_MonsterBehavior;

	// ---- Core Functions ----

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	/** Select the correct behavior tree based on monster's behavior enum. */
	UFUNCTION(BlueprintCallable, Category = "Monster AI")
	UBehaviorTree* SelectBehaviorTree(EROMonsterBehavior InBehavior) const;

	/** Scan for a valid target in aggro range (for aggressive monsters). */
	UFUNCTION(BlueprintCallable, Category = "Monster AI")
	AActor* FindTarget() const;

	/**
	 * Called when the possessed monster receives damage.
	 * Updates threat, sets target in blackboard, enters combat.
	 */
	UFUNCTION(BlueprintCallable, Category = "Monster AI")
	void HandleDamageReceived(AActor* Attacker, float DamageAmount);

	/**
	 * Return monster to spawn location.
	 * Resets blackboard combat state and heals to full.
	 */
	UFUNCTION(BlueprintCallable, Category = "Monster AI")
	void ReturnToHome();

	/** Check if current target is still valid and within chase range. */
	UFUNCTION(BlueprintCallable, Category = "Monster AI")
	bool IsTargetValid() const;

	/** Set the current combat target in the blackboard. */
	UFUNCTION(BlueprintCallable, Category = "Monster AI")
	void SetTargetActor(AActor* NewTarget);

	/** Enter combat mode (sets blackboard flag). */
	UFUNCTION(BlueprintCallable, Category = "Monster AI")
	void EnterCombat(AActor* Target);

	/** Exit combat mode (clears blackboard flag and target). */
	UFUNCTION(BlueprintCallable, Category = "Monster AI")
	void ExitCombat();

protected:
	/** Cached reference to the possessed monster. */
	UPROPERTY()
	TObjectPtr<AROMonsterBase> OwnerMonster;

	/** Setup blackboard with initial values from monster data. */
	void InitializeMonsterBlackboard();
};
