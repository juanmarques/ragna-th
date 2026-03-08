// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROAbility_Discount.h"

UROAbility_Discount::UROAbility_Discount()
{
	SkillID = 42;
	SkillName = FName("MC_DISCOUNT");
	MaxSkillLevel = 10;
	SkillLevel = 1;
	SkillElement = EROElement::Neutral;

	// Passive skill - no SP cost
	SPCostBase = 0.0f;
	SPCostPerLevel = 0.0f;

	// No cast time
	VariableCastTimeBase = 0.0f;
	FixedCastTime = 0.0f;
	CooldownDuration = 0.0f;
}

void UROAbility_Discount::OnCastComplete()
{
	Super::OnCastComplete();

	// Discount is a passive skill - it does nothing when "activated".
	// The discount logic is applied in the shop/NPC buy system by querying
	// GetDiscountPercentage() on the character's Discount ability.
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

float UROAbility_Discount::GetDiscountPercentage() const
{
	return GetDiscountForLevel(SkillLevel);
}

int32 UROAbility_Discount::GetDiscountedPrice(int32 OriginalPrice) const
{
	const float Discount = GetDiscountPercentage();
	const float DiscountedPrice = static_cast<float>(OriginalPrice) * (1.0f - Discount);
	return FMath::Max(1, FMath::RoundToInt32(DiscountedPrice));
}

float UROAbility_Discount::GetDiscountForLevel(int32 Level)
{
	if (Level <= 0)
	{
		return 0.0f;
	}

	// Discount: 5% + 2% * SkillLevel, max 25% at level 10
	const float DiscountPercent = 5.0f + 2.0f * static_cast<float>(Level);
	const float Clamped = FMath::Clamp(DiscountPercent, 0.0f, 25.0f);
	return Clamped / 100.0f; // Return as 0-1 range
}
