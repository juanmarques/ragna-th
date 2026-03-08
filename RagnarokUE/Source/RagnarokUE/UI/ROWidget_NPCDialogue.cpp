// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWidget_NPCDialogue.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/RichTextBlock.h"

UROWidget_NPCDialogue::UROWidget_NPCDialogue(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UROWidget_NPCDialogue::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Choice0) Btn_Choice0->OnClicked.AddDynamic(this, &UROWidget_NPCDialogue::OnChoice0Clicked);
	if (Btn_Choice1) Btn_Choice1->OnClicked.AddDynamic(this, &UROWidget_NPCDialogue::OnChoice1Clicked);
	if (Btn_Choice2) Btn_Choice2->OnClicked.AddDynamic(this, &UROWidget_NPCDialogue::OnChoice2Clicked);
	if (Btn_Choice3) Btn_Choice3->OnClicked.AddDynamic(this, &UROWidget_NPCDialogue::OnChoice3Clicked);
	if (Btn_NextClose) Btn_NextClose->OnClicked.AddDynamic(this, &UROWidget_NPCDialogue::OnNextCloseClicked);

	// Hide all choice buttons initially
	if (Btn_Choice0) Btn_Choice0->SetVisibility(ESlateVisibility::Collapsed);
	if (Btn_Choice1) Btn_Choice1->SetVisibility(ESlateVisibility::Collapsed);
	if (Btn_Choice2) Btn_Choice2->SetVisibility(ESlateVisibility::Collapsed);
	if (Btn_Choice3) Btn_Choice3->SetVisibility(ESlateVisibility::Collapsed);
	if (Btn_NextClose) Btn_NextClose->SetVisibility(ESlateVisibility::Collapsed);
}

void UROWidget_NPCDialogue::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bTypewriterActive || !bTypewriterEnabled)
	{
		return;
	}

	TypewriterAccumulator += InDeltaTime;
	const float CharInterval = 1.0f / TypewriterSpeed;

	while (TypewriterAccumulator >= CharInterval && DisplayedCharCount < FullDialogueText.Len())
	{
		TypewriterAccumulator -= CharInterval;
		DisplayedCharCount++;
	}

	// Update the displayed text
	if (Text_Dialogue)
	{
		const FString DisplayText = FullDialogueText.Left(DisplayedCharCount);
		Text_Dialogue->SetText(FText::FromString(DisplayText));
	}

	// Check if typewriter is complete
	if (DisplayedCharCount >= FullDialogueText.Len())
	{
		bTypewriterActive = false;
	}
}

void UROWidget_NPCDialogue::SetNPCInfo(const FString& NPCName, UTexture2D* Portrait)
{
	if (Text_NPCName)
	{
		Text_NPCName->SetText(FText::FromString(NPCName));
	}
	if (Img_NPCPortrait && Portrait)
	{
		Img_NPCPortrait->SetBrushFromTexture(Portrait);
		Img_NPCPortrait->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else if (Img_NPCPortrait)
	{
		Img_NPCPortrait->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UROWidget_NPCDialogue::SetDialogueText(const FString& Text)
{
	FullDialogueText = Text;
	DisplayedCharCount = 0;
	TypewriterAccumulator = 0.0f;

	if (bTypewriterEnabled)
	{
		bTypewriterActive = true;
		if (Text_Dialogue)
		{
			Text_Dialogue->SetText(FText::GetEmpty());
		}
	}
	else
	{
		bTypewriterActive = false;
		DisplayedCharCount = FullDialogueText.Len();
		if (Text_Dialogue)
		{
			Text_Dialogue->SetText(FText::FromString(FullDialogueText));
		}
	}

	// Hide choices and next/close while text is being displayed
	if (Btn_Choice0) Btn_Choice0->SetVisibility(ESlateVisibility::Collapsed);
	if (Btn_Choice1) Btn_Choice1->SetVisibility(ESlateVisibility::Collapsed);
	if (Btn_Choice2) Btn_Choice2->SetVisibility(ESlateVisibility::Collapsed);
	if (Btn_Choice3) Btn_Choice3->SetVisibility(ESlateVisibility::Collapsed);
	if (Btn_NextClose) Btn_NextClose->SetVisibility(ESlateVisibility::Collapsed);
}

void UROWidget_NPCDialogue::SetChoices(const TArray<FRODialogueChoiceDisplay>& Choices)
{
	CurrentChoices = Choices;

	// Skip typewriter if still running
	SkipTypewriter();

	UButton* ChoiceButtons[] = { Btn_Choice0, Btn_Choice1, Btn_Choice2, Btn_Choice3 };
	UTextBlock* ChoiceTexts[] = { Text_Choice0, Text_Choice1, Text_Choice2, Text_Choice3 };

	for (int32 i = 0; i < MAX_CHOICES; ++i)
	{
		if (i < Choices.Num() && Choices.IsValidIndex(i))
		{
			if (ChoiceButtons[i])
			{
				ChoiceButtons[i]->SetVisibility(ESlateVisibility::Visible);
			}
			if (ChoiceTexts[i])
			{
				ChoiceTexts[i]->SetText(FText::FromString(Choices[i].ChoiceText));
			}
		}
		else
		{
			if (ChoiceButtons[i])
			{
				ChoiceButtons[i]->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}

	// Hide next/close when choices are showing
	if (Btn_NextClose) Btn_NextClose->SetVisibility(ESlateVisibility::Collapsed);
}

void UROWidget_NPCDialogue::ShowNextButton()
{
	SkipTypewriter();

	// Hide choices
	if (Btn_Choice0) Btn_Choice0->SetVisibility(ESlateVisibility::Collapsed);
	if (Btn_Choice1) Btn_Choice1->SetVisibility(ESlateVisibility::Collapsed);
	if (Btn_Choice2) Btn_Choice2->SetVisibility(ESlateVisibility::Collapsed);
	if (Btn_Choice3) Btn_Choice3->SetVisibility(ESlateVisibility::Collapsed);

	bIsCloseMode = false;
	if (Text_NextCloseLabel) Text_NextCloseLabel->SetText(FText::FromString(TEXT("Next")));
	if (Btn_NextClose) Btn_NextClose->SetVisibility(ESlateVisibility::Visible);
}

void UROWidget_NPCDialogue::ShowCloseButton()
{
	SkipTypewriter();

	// Hide choices
	if (Btn_Choice0) Btn_Choice0->SetVisibility(ESlateVisibility::Collapsed);
	if (Btn_Choice1) Btn_Choice1->SetVisibility(ESlateVisibility::Collapsed);
	if (Btn_Choice2) Btn_Choice2->SetVisibility(ESlateVisibility::Collapsed);
	if (Btn_Choice3) Btn_Choice3->SetVisibility(ESlateVisibility::Collapsed);

	bIsCloseMode = true;
	if (Text_NextCloseLabel) Text_NextCloseLabel->SetText(FText::FromString(TEXT("Close")));
	if (Btn_NextClose) Btn_NextClose->SetVisibility(ESlateVisibility::Visible);
}

void UROWidget_NPCDialogue::SetTypewriterEnabled(bool bEnabled)
{
	bTypewriterEnabled = bEnabled;
}

void UROWidget_NPCDialogue::SkipTypewriter()
{
	if (bTypewriterActive)
	{
		bTypewriterActive = false;
		DisplayedCharCount = FullDialogueText.Len();
		if (Text_Dialogue)
		{
			Text_Dialogue->SetText(FText::FromString(FullDialogueText));
		}
	}
}

void UROWidget_NPCDialogue::OnChoice0Clicked()
{
	if (CurrentChoices.IsValidIndex(0))
	{
		OnDialogueChoiceSelected.Broadcast(CurrentChoices[0].ChoiceIndex);
	}
}

void UROWidget_NPCDialogue::OnChoice1Clicked()
{
	if (CurrentChoices.IsValidIndex(1))
	{
		OnDialogueChoiceSelected.Broadcast(CurrentChoices[1].ChoiceIndex);
	}
}

void UROWidget_NPCDialogue::OnChoice2Clicked()
{
	if (CurrentChoices.IsValidIndex(2))
	{
		OnDialogueChoiceSelected.Broadcast(CurrentChoices[2].ChoiceIndex);
	}
}

void UROWidget_NPCDialogue::OnChoice3Clicked()
{
	if (CurrentChoices.IsValidIndex(3))
	{
		OnDialogueChoiceSelected.Broadcast(CurrentChoices[3].ChoiceIndex);
	}
}

void UROWidget_NPCDialogue::OnNextCloseClicked()
{
	if (bTypewriterActive)
	{
		// If typewriter is running, skip it first
		SkipTypewriter();
		return;
	}

	if (bIsCloseMode)
	{
		OnDialogueClose.Broadcast();
	}
	else
	{
		OnDialogueNext.Broadcast();
	}
}
