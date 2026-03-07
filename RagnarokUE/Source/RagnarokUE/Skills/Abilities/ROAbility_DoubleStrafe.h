// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/Skills/ROGameplayAbility.h"
#include "ROAbility_DoubleStrafe.generated.h"

/**
 * UROAbility_DoubleStrafe
 * Archer skill - 2-hit ranged physical attack.
 * Requires bow equipped. 10 levels.
 * Damage: ATK * (100% + 10% * SkillLevel) * 2 hits
 * SP cost: 12 flat.
 */
UCLASS()
class RAGNAROKUE_API UROAbility_DoubleStrafe : public UROGameplayAbility
{
	GENERATED_BODY()

public:
	UROAbility_DoubleStrafe();

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void OnCastComplete() override;

private:
	/** Check if a bow is equipped. */
	bool IsBowEquipped(const FGameplayAbilityActorInfo* ActorInfo) const;

	/** Get per-hit damage modifier. */
	float GetPerHitDamageModifier() const;
};
