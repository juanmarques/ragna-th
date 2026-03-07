// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/Skills/ROGameplayAbility.h"
#include "ROAbility_FireBolt.generated.h"

/**
 * UROAbility_FireBolt
 * Mage skill - Multi-hit fire magic attack.
 * Number of hits = skill level. Each bolt does MATK * 100% fire damage.
 * SP cost: 12 + 3 * Level. Has variable cast time.
 */
UCLASS()
class RAGNAROKUE_API UROAbility_FireBolt : public UROGameplayAbility
{
	GENERATED_BODY()

public:
	UROAbility_FireBolt();

	/** Delay between each bolt hit in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill|FireBolt")
	float BoltInterval;

protected:
	virtual void OnCastComplete() override;

private:
	/** Get number of bolt hits at current level. */
	int32 GetBoltCount() const;

	/** Get per-bolt damage modifier. */
	float GetPerBoltDamageModifier() const;
};
