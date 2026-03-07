// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/Skills/ROGameplayAbility.h"
#include "ROAbility_Bash.generated.h"

/**
 * UROAbility_Bash
 * Swordsman skill - Physical single target attack.
 * Damage: ATK * (100% + 30% * SkillLevel)
 * SP cost: 8 + 2 * Level
 * Stun chance at high levels (Lv6+)
 */
UCLASS()
class RAGNAROKUE_API UROAbility_Bash : public UROGameplayAbility
{
	GENERATED_BODY()

public:
	UROAbility_Bash();

	/** Stun chance per level (0 for levels below 6). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill|Bash")
	float StunChanceBase;

	/** Stun duration in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill|Bash")
	float StunDuration;

protected:
	virtual void OnCastComplete() override;

private:
	/** Calculate the damage modifier for the current skill level. */
	float GetDamageModifier() const;

	/** Calculate the stun chance for the current skill level. */
	float GetStunChance() const;
};
