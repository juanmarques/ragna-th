// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/Skills/ROGameplayAbility.h"
#include "ROAbility_ColdBolt.generated.h"

/**
 * UROAbility_ColdBolt
 * Mage skill - Multi-hit water magic attack.
 * Same structure as FireBolt but Water element.
 * Number of hits = skill level. Each bolt does MATK * 100% water damage.
 * SP cost: 12 + 3 * Level. Has variable cast time.
 */
UCLASS()
class RAGNAROKUE_API UROAbility_ColdBolt : public UROGameplayAbility
{
	GENERATED_BODY()

public:
	UROAbility_ColdBolt();

	/** Delay between each bolt hit in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RO Skill|ColdBolt")
	float BoltInterval;

protected:
	virtual void OnCastComplete() override;

private:
	int32 GetBoltCount() const;
	float GetPerBoltDamageModifier() const;
};
