// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "ROStatusEffectComponent.generated.h"

/**
 * FROActiveStatusEffect
 * Runtime data for a currently active status effect.
 */
USTRUCT(BlueprintType)
struct FROActiveStatusEffect
{
	GENERATED_BODY()

	/** The status effect type. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	EROStatusEffect Effect = EROStatusEffect::Stun;

	/** Remaining duration in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	float RemainingDuration = 0.0f;

	/** Total duration when applied. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	float TotalDuration = 0.0f;

	/** Effect level/intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	int32 Level = 1;

	/** Tick accumulator for periodic effects. */
	UPROPERTY()
	float TickAccumulator = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatusEffectChanged, EROStatusEffect, Effect, bool, bApplied);

/**
 * UROStatusEffectComponent
 * Manages active status effects on a character with duration tracking,
 * periodic ticks, and gameplay behavior modifications.
 * Replicates for visual display on remote clients.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RAGNAROKUE_API UROStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UROStatusEffectComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Status Effect Management ---

	/**
	 * Attempt to apply a status effect with a success chance.
	 * @param Effect - The status effect to apply
	 * @param Duration - Duration in seconds
	 * @param Level - Effect level/intensity
	 * @param Chance - Success chance 0-100 (100 = guaranteed)
	 * @return True if the effect was successfully applied
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|StatusEffects")
	bool ApplyStatusEffect(EROStatusEffect Effect, float Duration, int32 Level = 1, float Chance = 100.0f);

	/**
	 * Remove a specific status effect.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|StatusEffects")
	void RemoveStatusEffect(EROStatusEffect Effect);

	/**
	 * Remove all active status effects.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|StatusEffects")
	void RemoveAllStatusEffects();

	/**
	 * Check if a specific status effect is active.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|StatusEffects")
	bool HasStatusEffect(EROStatusEffect Effect) const;

	/**
	 * Get the remaining duration of a status effect. Returns 0 if not active.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|StatusEffects")
	float GetRemainingDuration(EROStatusEffect Effect) const;

	/**
	 * Get the level of an active status effect. Returns 0 if not active.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|StatusEffects")
	int32 GetEffectLevel(EROStatusEffect Effect) const;

	/**
	 * Check if the character can act (not stunned, frozen, or stone cursed).
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|StatusEffects")
	bool CanAct() const;

	/**
	 * Check if the character can cast spells (not silenced).
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|StatusEffects")
	bool CanCast() const;

	/**
	 * Check if the character can move (not stunned, frozen, stone cursed, or rooted).
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|StatusEffects")
	bool CanMove() const;

	/**
	 * Get all currently active status effects.
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|StatusEffects")
	TArray<FROActiveStatusEffect> GetAllActiveEffects() const;

	// --- Delegates ---

	/** Broadcast when a status effect is applied or removed. */
	UPROPERTY(BlueprintAssignable, Category = "RO|StatusEffects|Events")
	FOnStatusEffectChanged OnStatusEffectChanged;

protected:
	virtual void BeginPlay() override;

private:
	/** Map of active status effects. */
	UPROPERTY(Replicated)
	TArray<FROActiveStatusEffect> ActiveEffectsArray;

	/** Tick interval for periodic status effects (e.g., poison damage). */
	static constexpr float StatusTickInterval = 1.0f;

	/** Process periodic effect ticks (poison HP drain, etc.). */
	void ProcessPeriodicEffects(float DeltaTime);

	/** Apply the gameplay tag for a status effect to the owning actor's ASC. */
	void ApplyStatusTag(EROStatusEffect Effect);

	/** Remove the gameplay tag for a status effect from the owning actor's ASC. */
	void RemoveStatusTag(EROStatusEffect Effect);

	/** Get the gameplay tag name for a status effect. */
	static FName GetTagNameForEffect(EROStatusEffect Effect);

	/** Find an active effect by type. Returns INDEX_NONE if not found. */
	int32 FindEffectIndex(EROStatusEffect Effect) const;
};
