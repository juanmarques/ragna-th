// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWidget_CastBar.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

UROWidget_CastBar::UROWidget_CastBar(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UROWidget_CastBar::NativeConstruct()
{
	Super::NativeConstruct();

	// Start hidden
	SetVisibility(ESlateVisibility::Collapsed);

	if (Text_CancelWarning)
	{
		Text_CancelWarning->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UROWidget_CastBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsCasting)
	{
		return;
	}

	ElapsedCastTime += InDeltaTime;

	const float Progress = GetCastProgress();

	if (CastProgressBar)
	{
		CastProgressBar->SetPercent(Progress);
	}

	// Cast complete
	if (ElapsedCastTime >= TotalCastTime)
	{
		CompleteCast();
	}
}

void UROWidget_CastBar::StartCast(const FString& SkillName, UTexture2D* SkillIcon, float CastDuration)
{
	if (CastDuration <= 0.0f)
	{
		return;
	}

	bIsCasting = true;
	TotalCastTime = CastDuration;
	ElapsedCastTime = 0.0f;

	if (Text_SkillName)
	{
		Text_SkillName->SetText(FText::FromString(SkillName));
	}

	if (Img_SkillIcon && SkillIcon)
	{
		Img_SkillIcon->SetBrushFromTexture(SkillIcon);
		Img_SkillIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else if (Img_SkillIcon)
	{
		Img_SkillIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (CastProgressBar)
	{
		CastProgressBar->SetPercent(0.0f);
	}

	if (Text_CancelWarning)
	{
		Text_CancelWarning->SetVisibility(ESlateVisibility::Collapsed);
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UROWidget_CastBar::CancelCast()
{
	if (!bIsCasting)
	{
		return;
	}

	bIsCasting = false;
	ElapsedCastTime = 0.0f;
	TotalCastTime = 0.0f;

	OnCastCancelled.Broadcast();

	SetVisibility(ESlateVisibility::Collapsed);
}

void UROWidget_CastBar::CompleteCast()
{
	if (!bIsCasting)
	{
		return;
	}

	bIsCasting = false;
	ElapsedCastTime = 0.0f;
	TotalCastTime = 0.0f;

	OnCastComplete.Broadcast();

	SetVisibility(ESlateVisibility::Collapsed);
}

float UROWidget_CastBar::GetCastProgress() const
{
	if (TotalCastTime <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(ElapsedCastTime / TotalCastTime, 0.0f, 1.0f);
}

void UROWidget_CastBar::ShowMovementCancelIndicator(bool bShow)
{
	if (Text_CancelWarning)
	{
		Text_CancelWarning->SetVisibility(bShow ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}
