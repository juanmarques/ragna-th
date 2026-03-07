// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ROItemBase.h"
#include "GameplayEffect.h"
#include "ROCardData.generated.h"

/**
 * UROCardData
 * Data asset for card items that can be slotted into equipment.
 * Cards grant stat bonuses, elemental properties, and special effects.
 */
UCLASS(BlueprintType)
class RAGNAROKUE_API UROCardData : public UROItemBase
{
	GENERATED_BODY()

public:
	UROCardData();

	/** Which equipment slot type this card can be inserted into. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card")
	EROEquipSlot TargetSlotType;

	/** Flat stat bonuses granted when this card is active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card")
	FROStatBlock StatBonuses;

	/** Element granted to the equipment when this card is inserted. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card")
	EROElement ElementGrant;

	/** Monster race this card grants bonus damage against. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card")
	EROMonsterRace RaceBonusTarget;

	/** Percentage bonus damage against the target race (e.g., 20.0 = +20%). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card", meta = (ClampMin = "0.0"))
	float RaceBonusPercent;

	/** Optional GAS GameplayEffect applied when this card is active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card")
	TSubclassOf<UGameplayEffect> CardEffect;

	/** Prefix added to equipment name when this card is compounded (e.g., "Vital" from Pupa Card). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card|Naming")
	FString CompoundPrefix;

	/** Suffix added to equipment name when this card is compounded (e.g., "of Insight" from Fabre Card). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card|Naming")
	FString CompoundSuffix;

	/**
	 * Get the compound name component this card contributes.
	 * Returns prefix if set, otherwise suffix, otherwise empty.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Card")
	FString GetCompoundNamePart() const;
};
