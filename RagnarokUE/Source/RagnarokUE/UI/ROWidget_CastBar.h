// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ROWidget_CastBar.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;

/**
 * UROWidget_CastBar
 * Progress bar showing cast progress with skill name/icon display.
 * Positioned above character or at screen bottom.
 */
UCLASS()
class RAGNAROKUE_API UROWidget_CastBar : public UUserWidget
{
	GENERATED_BODY()

public:
	UROWidget_CastBar(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// --- Cast Control ---

	/** Start showing a cast bar for a skill. */
	UFUNCTION(BlueprintCallable, Category = "RO|CastBar")
	void StartCast(const FString& SkillName, UTexture2D* SkillIcon, float CastDuration);

	/** Cancel the current cast (interrupted). */
	UFUNCTION(BlueprintCallable, Category = "RO|CastBar")
	void CancelCast();

	/** Complete the cast (hide bar). */
	UFUNCTION(BlueprintCallable, Category = "RO|CastBar")
	void CompleteCast();

	/** Whether a cast is currently active. */
	UFUNCTION(BlueprintPure, Category = "RO|CastBar")
	bool IsCasting() const { return bIsCasting; }

	/** Get the current cast progress (0 to 1). */
	UFUNCTION(BlueprintPure, Category = "RO|CastBar")
	float GetCastProgress() const;

	/** Show the movement cancellation indicator. */
	UFUNCTION(BlueprintCallable, Category = "RO|CastBar")
	void ShowMovementCancelIndicator(bool bShow);

	// --- Delegates ---

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCastComplete);

	UPROPERTY(BlueprintAssignable, Category = "RO|CastBar")
	FOnCastComplete OnCastComplete;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCastCancelled);

	UPROPERTY(BlueprintAssignable, Category = "RO|CastBar")
	FOnCastCancelled OnCastCancelled;

protected:
	// --- Bound Widgets ---

	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* CastProgressBar;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_SkillName;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_SkillIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_CancelWarning;

private:
	/** Whether a cast is active. */
	bool bIsCasting = false;

	/** Total cast duration. */
	float TotalCastTime = 0.0f;

	/** Elapsed cast time. */
	float ElapsedCastTime = 0.0f;
};
