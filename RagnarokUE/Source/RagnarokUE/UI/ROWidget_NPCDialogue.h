// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ROWidget_NPCDialogue.generated.h"

class UTextBlock;
class UButton;
class UImage;
class UVerticalBox;
class URichTextBlock;

/**
 * FRODialogueChoice
 * A single dialogue choice button.
 */
USTRUCT(BlueprintType)
struct FRODialogueChoice
{
	GENERATED_BODY()

	/** Choice index for response. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 ChoiceIndex = 0;

	/** Choice text displayed on the button. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString ChoiceText;
};

/**
 * UROWidget_NPCDialogue
 * NPC dialogue window with portrait, typewriter text, choice buttons,
 * and optional vendor shop panel.
 */
UCLASS()
class RAGNAROKUE_API UROWidget_NPCDialogue : public UUserWidget
{
	GENERATED_BODY()

public:
	UROWidget_NPCDialogue(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Maximum dialogue choices. */
	static constexpr int32 MAX_CHOICES = 4;

	// --- Dialogue Control ---

	/** Set the NPC name and portrait. */
	UFUNCTION(BlueprintCallable, Category = "RO|Dialogue")
	void SetNPCInfo(const FString& NPCName, UTexture2D* Portrait);

	/** Set dialogue text (starts typewriter effect). */
	UFUNCTION(BlueprintCallable, Category = "RO|Dialogue")
	void SetDialogueText(const FString& Text);

	/** Set the choice buttons (up to 4). Clears existing choices. */
	UFUNCTION(BlueprintCallable, Category = "RO|Dialogue")
	void SetChoices(const TArray<FRODialogueChoice>& Choices);

	/** Show only a Next button (no choices). */
	UFUNCTION(BlueprintCallable, Category = "RO|Dialogue")
	void ShowNextButton();

	/** Show only a Close button. */
	UFUNCTION(BlueprintCallable, Category = "RO|Dialogue")
	void ShowCloseButton();

	/** Enable or disable the typewriter effect. */
	UFUNCTION(BlueprintCallable, Category = "RO|Dialogue")
	void SetTypewriterEnabled(bool bEnabled);

	/** Skip the typewriter effect and show full text immediately. */
	UFUNCTION(BlueprintCallable, Category = "RO|Dialogue")
	void SkipTypewriter();

	/** Speed of typewriter effect (characters per second). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RO|Dialogue")
	float TypewriterSpeed = 30.0f;

	// --- Delegates ---

	/** Broadcast when a dialogue choice is selected. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueChoiceSelected, int32, ChoiceIndex);

	UPROPERTY(BlueprintAssignable, Category = "RO|Dialogue")
	FOnDialogueChoiceSelected OnDialogueChoiceSelected;

	/** Broadcast when Next is clicked. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueNext);

	UPROPERTY(BlueprintAssignable, Category = "RO|Dialogue")
	FOnDialogueNext OnDialogueNext;

	/** Broadcast when Close is clicked. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueClose);

	UPROPERTY(BlueprintAssignable, Category = "RO|Dialogue")
	FOnDialogueClose OnDialogueClose;

protected:
	// --- Button handlers ---
	UFUNCTION()
	void OnChoice0Clicked();
	UFUNCTION()
	void OnChoice1Clicked();
	UFUNCTION()
	void OnChoice2Clicked();
	UFUNCTION()
	void OnChoice3Clicked();
	UFUNCTION()
	void OnNextCloseClicked();

	// --- Bound Widgets ---

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_NPCName;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_NPCPortrait;

	UPROPERTY(meta = (BindWidgetOptional))
	URichTextBlock* Text_Dialogue;

	UPROPERTY(meta = (BindWidgetOptional))
	UVerticalBox* ChoicesContainer;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Choice0;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_Choice0;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Choice1;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_Choice1;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Choice2;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_Choice2;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Choice3;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_Choice3;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_NextClose;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_NextCloseLabel;

private:
	/** Full dialogue text. */
	FString FullDialogueText;

	/** Current number of displayed characters (for typewriter). */
	int32 DisplayedCharCount = 0;

	/** Whether the typewriter effect is active. */
	bool bTypewriterActive = false;

	/** Whether typewriter is enabled. */
	bool bTypewriterEnabled = true;

	/** Accumulated time for typewriter tick. */
	float TypewriterAccumulator = 0.0f;

	/** Whether the Next/Close button acts as Next or Close. */
	bool bIsCloseMode = false;

	/** Current choice data. */
	TArray<FRODialogueChoice> CurrentChoices;
};
