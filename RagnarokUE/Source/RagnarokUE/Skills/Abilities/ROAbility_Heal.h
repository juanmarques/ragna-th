// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/Skills/ROGameplayAbility.h"
#include "ROAbility_Heal.generated.h"

/**
 * UROAbility_Heal
 * Acolyte skill - Heals a target, or damages undead monsters.
 * Heal amount: (BaseLevel + INT) / 8 * (4 + 8 * SkillLevel)
 * SP cost: 13 + 2 * Level. 10 levels.
 */
UCLASS()
class RAGNAROKUE_API UROAbility_Heal : public UROGameplayAbility
{
	GENERATED_BODY()

public:
	UROAbility_Heal();

	/** Whether to check for undead targets and deal damage instead. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill|Heal")
	bool bDamageUndead;

protected:
	virtual void OnCastComplete() override;

private:
	/**
	 * Calculate heal amount based on caster stats.
	 * Formula: (BaseLevel + INT) / 8 * (4 + 8 * SkillLevel)
	 */
	float CalculateHealAmount(int32 BaseLevel, int32 INT_Stat) const;
};
