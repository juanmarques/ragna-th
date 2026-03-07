// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "ROWidget_TargetInfo.generated.h"

class UTextBlock;
class UProgressBar;
class UImage;

/**
 * Target type: monster, player, or NPC.
 */
UENUM(BlueprintType)
enum class EROTargetType : uint8
{
	None		UMETA(DisplayName = "None"),
	Monster		UMETA(DisplayName = "Monster"),
	Player		UMETA(DisplayName = "Player"),
	NPC			UMETA(DisplayName = "NPC")
};

/**
 * UROWidget_TargetInfo
 * Displays information about the currently selected target:
 * name, level, HP bar (monsters), job class (players), and element icon.
 */
UCLASS()
class RAGNAROKUE_API UROWidget_TargetInfo : public UUserWidget
{
	GENERATED_BODY()

public:
	UROWidget_TargetInfo(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	// --- Target Control ---

	/** Set the current target information for a monster. */
	UFUNCTION(BlueprintCallable, Category = "RO|Target")
	void SetMonsterTarget(const FString& Name, int32 Level, float HPPercent, EROElement Element);

	/** Set the current target information for a player. */
	UFUNCTION(BlueprintCallable, Category = "RO|Target")
	void SetPlayerTarget(const FString& Name, int32 Level, EROJobClass JobClass);

	/** Set the current target information for an NPC. */
	UFUNCTION(BlueprintCallable, Category = "RO|Target")
	void SetNPCTarget(const FString& Name);

	/** Clear the target and hide the widget. */
	UFUNCTION(BlueprintCallable, Category = "RO|Target")
	void ClearTarget();

	/** Update the HP bar percentage (for monsters). */
	UFUNCTION(BlueprintCallable, Category = "RO|Target")
	void UpdateHPPercent(float Percent);

	/** Set the element icon texture. */
	UFUNCTION(BlueprintCallable, Category = "RO|Target")
	void SetElementIcon(UTexture2D* Icon);

	/** Get current target type. */
	UFUNCTION(BlueprintPure, Category = "RO|Target")
	EROTargetType GetTargetType() const { return CurrentTargetType; }

	/** Whether a target is currently set. */
	UFUNCTION(BlueprintPure, Category = "RO|Target")
	bool HasTarget() const { return CurrentTargetType != EROTargetType::None; }

protected:
	// --- Bound Widgets ---

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_TargetName;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_TargetLevel;

	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* HPBar;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_JobClass;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_Element;

private:
	/** Helper to get display name string for a job class. */
	static FString GetJobClassName(EROJobClass JobClass);

	/** Current target type. */
	EROTargetType CurrentTargetType = EROTargetType::None;
};
