// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "ROAttributeSet.generated.h"

// Macros for attribute accessors following GAS best practices
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UROAttributeSet
 * Core attribute set for Ragnarok Online characters.
 * Contains all combat-relevant stats as GAS attributes with replication support.
 */
UCLASS()
class RAGNAROKUE_API UROAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UROAttributeSet();

	// --- Vital Attributes ---

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_HP)
	FGameplayAttributeData HP;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, HP)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_MaxHP)
	FGameplayAttributeData MaxHP;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, MaxHP)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_SP)
	FGameplayAttributeData SP;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, SP)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_MaxSP)
	FGameplayAttributeData MaxSP;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, MaxSP)

	// --- Offensive Attributes ---

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Offensive", ReplicatedUsing = OnRep_ATK)
	FGameplayAttributeData ATK;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, ATK)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Offensive", ReplicatedUsing = OnRep_MATK)
	FGameplayAttributeData MATK;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, MATK)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Offensive", ReplicatedUsing = OnRep_HIT)
	FGameplayAttributeData HIT;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, HIT)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Offensive", ReplicatedUsing = OnRep_CritRate)
	FGameplayAttributeData CritRate;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, CritRate)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Offensive", ReplicatedUsing = OnRep_ASPD)
	FGameplayAttributeData ASPD;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, ASPD)

	// --- Defensive Attributes ---

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Defensive", ReplicatedUsing = OnRep_DEF)
	FGameplayAttributeData DEF;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, DEF)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Defensive", ReplicatedUsing = OnRep_MDEF)
	FGameplayAttributeData MDEF;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, MDEF)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Defensive", ReplicatedUsing = OnRep_FLEE)
	FGameplayAttributeData FLEE;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, FLEE)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Defensive", ReplicatedUsing = OnRep_PerfectDodge)
	FGameplayAttributeData PerfectDodge;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, PerfectDodge)

	// --- Regeneration / Movement ---

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Regen", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, MoveSpeed)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Regen", ReplicatedUsing = OnRep_HPRegen)
	FGameplayAttributeData HPRegen;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, HPRegen)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Regen", ReplicatedUsing = OnRep_SPRegen)
	FGameplayAttributeData SPRegen;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, SPRegen)

	// --- Meta Attribute (transient, not replicated) ---

	/** Incoming damage meta-attribute. Applied by damage execution, consumed in PostGameplayEffectExecute. */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, IncomingDamage)

	/** Incoming healing meta-attribute. */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData IncomingHealing;
	ATTRIBUTE_ACCESSORS(UROAttributeSet, IncomingHealing)

	// --- Overrides ---

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// RepNotify functions
	UFUNCTION()
	void OnRep_HP(const FGameplayAttributeData& OldHP);

	UFUNCTION()
	void OnRep_MaxHP(const FGameplayAttributeData& OldMaxHP);

	UFUNCTION()
	void OnRep_SP(const FGameplayAttributeData& OldSP);

	UFUNCTION()
	void OnRep_MaxSP(const FGameplayAttributeData& OldMaxSP);

	UFUNCTION()
	void OnRep_ATK(const FGameplayAttributeData& OldATK);

	UFUNCTION()
	void OnRep_MATK(const FGameplayAttributeData& OldMATK);

	UFUNCTION()
	void OnRep_DEF(const FGameplayAttributeData& OldDEF);

	UFUNCTION()
	void OnRep_MDEF(const FGameplayAttributeData& OldMDEF);

	UFUNCTION()
	void OnRep_HIT(const FGameplayAttributeData& OldHIT);

	UFUNCTION()
	void OnRep_FLEE(const FGameplayAttributeData& OldFLEE);

	UFUNCTION()
	void OnRep_ASPD(const FGameplayAttributeData& OldASPD);

	UFUNCTION()
	void OnRep_CritRate(const FGameplayAttributeData& OldCritRate);

	UFUNCTION()
	void OnRep_PerfectDodge(const FGameplayAttributeData& OldPerfectDodge);

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);

	UFUNCTION()
	void OnRep_HPRegen(const FGameplayAttributeData& OldHPRegen);

	UFUNCTION()
	void OnRep_SPRegen(const FGameplayAttributeData& OldSPRegen);

private:
	/** Helper: clamp attribute between min and max. */
	void ClampAttribute(const FGameplayAttribute& Attribute, float MinValue, float MaxValue, float& NewValue) const;
};
