// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RagnarokUE/Skills/ROGameplayAbility.h"
#include "ROAbility_Discount.generated.h"

/**
 * UROAbility_Discount
 * Merchant passive skill - Reduces NPC buy prices.
 * Discount: 5% + 2% * SkillLevel (max 24% at level 10).
 * This is a data class; the actual discount is applied in shop logic.
 * The ability itself does nothing when activated.
 */
UCLASS()
class RAGNAROKUE_API UROAbility_Discount : public UROGameplayAbility
{
	GENERATED_BODY()

public:
	UROAbility_Discount();

	/**
	 * Get the discount percentage at the current skill level.
	 * @return Discount as a value 0-1 (e.g. 0.24 = 24% discount)
	 */
	UFUNCTION(BlueprintCallable, Category = "RO Skill|Discount")
	float GetDiscountPercentage() const;

	/**
	 * Calculate the discounted price for an item.
	 * @param OriginalPrice - The NPC's base price
	 * @return The price after discount
	 */
	UFUNCTION(BlueprintCallable, Category = "RO Skill|Discount")
	int32 GetDiscountedPrice(int32 OriginalPrice) const;

	/**
	 * Static helper: get discount for a given skill level (0 = not learned).
	 */
	UFUNCTION(BlueprintCallable, Category = "RO Skill|Discount")
	static float GetDiscountForLevel(int32 Level);

protected:
	virtual void OnCastComplete() override;
};
