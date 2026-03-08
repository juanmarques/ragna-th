// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/Skills/ROGameplayAbility.h"
#include "ROAbility_MagnumBreak.generated.h"

/**
 * UROAbility_MagnumBreak
 * Swordsman skill - AoE fire damage around the caster.
 * Pushes back enemies. Grants Fire element endow for 10 seconds after use.
 * 10 levels, SP cost: 30 flat.
 */
UCLASS()
class RAGNAROKUE_API UROAbility_MagnumBreak : public UROGameplayAbility
{
	GENERATED_BODY()

public:
	UROAbility_MagnumBreak();

	/** AoE radius in Unreal units. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill|MagnumBreak")
	float AoERadius;

	/** Pushback force applied to hit enemies. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill|MagnumBreak")
	float PushbackForce;

	/** Duration of fire element endow after use. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill|MagnumBreak")
	float FireEndowDuration;

protected:
	virtual void OnCastComplete() override;

private:
	/** Calculate damage modifier at current level. */
	float GetDamageModifier() const;

	/** Apply fire element endow to self. */
	void ApplyFireEndow();

	/** Timer handle for fire endow expiry, stored to clear on re-use. */
	FTimerHandle FireEndowTimerHandle;
};
