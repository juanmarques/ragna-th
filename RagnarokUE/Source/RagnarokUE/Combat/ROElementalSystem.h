// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "ROElementalSystem.generated.h"

/**
 * UROElementalSystem
 * Static utility class providing the full Ragnarok Online elemental modifier table.
 * 10 attacking elements x 10 defending elements x 4 defense levels = 400 values.
 */
UCLASS()
class RAGNAROKUE_API UROElementalSystem : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Get the damage modifier when attacking with a given element against
	 * a defender with a given element and defense level.
	 * @param AttackElement - Element of the attack/skill
	 * @param DefenseElement - Element of the target
	 * @param DefLevel - Defense element level (1-4)
	 * @return Damage multiplier (e.g. 0.5 = half damage, 1.5 = 50% bonus, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "RO|Elements")
	static float GetElementalModifier(EROElement AttackElement, EROElement DefenseElement, EROElementLevel DefLevel);

private:
	/**
	 * The full elemental table. Indexed as [AttackElement][DefenseElement][DefLevel].
	 * 10 x 10 x 4 = 400 entries.
	 */
	static const float ElementalTable[10][10][4];
};
