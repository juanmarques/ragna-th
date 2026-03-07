// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "ROGameplayAbility.generated.h"

class UROAttributeSet;
class UROCastingComponent;

/**
 * UROGameplayAbility
 * Base class for all Ragnarok Online skills.
 * Provides RO-specific properties: skill levels, SP costs, variable/fixed cast times,
 * and element typing. Subclasses implement specific skill behaviors.
 */
UCLASS(Abstract)
class RAGNAROKUE_API UROGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UROGameplayAbility();

	// --- Skill Properties ---

	/** Current learned skill level (1 to MaxSkillLevel). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RO Skill", meta = (ClampMin = "1", ClampMax = "10"))
	int32 SkillLevel;

	/** Maximum skill level. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaxSkillLevel;

	/** Base SP cost at level 1. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill|Cost")
	float SPCostBase;

	/** Additional SP cost per skill level. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill|Cost")
	float SPCostPerLevel;

	/** Variable cast time base in seconds (reduced by DEX/INT). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill|Casting")
	float VariableCastTimeBase;

	/** Fixed cast time in seconds (not reduced by stats). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill|Casting")
	float FixedCastTime;

	/** Cooldown duration in seconds after skill use. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill|Casting")
	float CooldownDuration;

	/** Elemental type of this skill. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill")
	EROElement SkillElement;

	/** Unique skill identifier from RO database. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill")
	int32 SkillID;

	/** Internal skill name. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill")
	FName SkillName;

	// --- Computed Getters ---

	/** Calculate total SP cost at current skill level. */
	UFUNCTION(BlueprintCallable, Category = "RO Skill")
	float GetSPCost() const;

	/**
	 * Calculate variable cast time after DEX/INT reduction.
	 * Formula: VariableCastTimeBase * (1 - sqrt((DEX*2 + INT) / 530))
	 */
	UFUNCTION(BlueprintCallable, Category = "RO Skill")
	float GetVariableCastTime(int32 DEX, int32 INT_Stat) const;

	/** Get total cooldown duration. */
	UFUNCTION(BlueprintCallable, Category = "RO Skill")
	float GetCooldown() const;

	// --- Ability Overrides ---

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) override;

protected:
	/** Called when cast completes successfully. Override in subclasses for skill logic. */
	UFUNCTION()
	virtual void OnCastComplete();

	/** Called when cast is interrupted. */
	UFUNCTION()
	virtual void OnCastInterrupted();

	/** Deduct SP from the caster. */
	void DeductSP(const FGameplayAbilityActorInfo* ActorInfo, float Amount);

	/** Check if the owner has a specific status effect tag. */
	bool HasStatusEffectTag(const FGameplayAbilityActorInfo* ActorInfo, FName TagName) const;

	/** Get the attribute set from the owning actor. */
	const UROAttributeSet* GetROAttributeSet(const FGameplayAbilityActorInfo* ActorInfo) const;

	/** Cached activation info for use in delegates. */
	FGameplayAbilitySpecHandle CachedHandle;
	const FGameplayAbilityActorInfo* CachedActorInfo;
	FGameplayAbilityActivationInfo CachedActivationInfo;
};
