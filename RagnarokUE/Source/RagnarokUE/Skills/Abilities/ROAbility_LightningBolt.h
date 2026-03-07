// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/Skills/ROGameplayAbility.h"
#include "ROAbility_LightningBolt.generated.h"

/**
 * UROAbility_LightningBolt
 * Mage skill - Multi-hit wind magic attack.
 * Same structure as FireBolt/ColdBolt but Wind element.
 * Number of hits = skill level. Each bolt does MATK * 100% wind damage.
 * SP cost: 12 + 3 * Level. Has variable cast time.
 */
UCLASS()
class RAGNAROKUE_API UROAbility_LightningBolt : public UROGameplayAbility
{
	GENERATED_BODY()

public:
	UROAbility_LightningBolt();

	/** Delay between each bolt hit in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill|LightningBolt")
	float BoltInterval;

protected:
	virtual void OnCastComplete() override;

private:
	int32 GetBoltCount() const;
	float GetPerBoltDamageModifier() const;
};
