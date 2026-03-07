// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWidget_TargetInfo.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"

UROWidget_TargetInfo::UROWidget_TargetInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UROWidget_TargetInfo::NativeConstruct()
{
	Super::NativeConstruct();

	// Start hidden until a target is selected
	SetVisibility(ESlateVisibility::Collapsed);
}

void UROWidget_TargetInfo::SetMonsterTarget(const FString& Name, int32 Level, float HPPercent, EROElement Element)
{
	CurrentTargetType = EROTargetType::Monster;

	if (Text_TargetName)
	{
		Text_TargetName->SetText(FText::FromString(Name));
	}

	if (Text_TargetLevel)
	{
		Text_TargetLevel->SetText(FText::FromString(FString::Printf(TEXT("Lv. %d"), Level)));
		Text_TargetLevel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (HPBar)
	{
		HPBar->SetPercent(FMath::Clamp(HPPercent, 0.0f, 1.0f));
		HPBar->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (Text_JobClass)
	{
		Text_JobClass->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (Img_Element)
	{
		Img_Element->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UROWidget_TargetInfo::SetPlayerTarget(const FString& Name, int32 Level, EROJobClass JobClass)
{
	CurrentTargetType = EROTargetType::Player;

	if (Text_TargetName)
	{
		Text_TargetName->SetText(FText::FromString(Name));
	}

	if (Text_TargetLevel)
	{
		Text_TargetLevel->SetText(FText::FromString(FString::Printf(TEXT("Lv. %d"), Level)));
		Text_TargetLevel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	// Players show job class instead of HP bar
	if (HPBar)
	{
		HPBar->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (Text_JobClass)
	{
		Text_JobClass->SetText(FText::FromString(GetJobClassName(JobClass)));
		Text_JobClass->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	// No element icon for players
	if (Img_Element)
	{
		Img_Element->SetVisibility(ESlateVisibility::Collapsed);
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UROWidget_TargetInfo::SetNPCTarget(const FString& Name)
{
	CurrentTargetType = EROTargetType::NPC;

	if (Text_TargetName)
	{
		Text_TargetName->SetText(FText::FromString(Name));
	}

	if (Text_TargetLevel)
	{
		Text_TargetLevel->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (HPBar)
	{
		HPBar->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (Text_JobClass)
	{
		Text_JobClass->SetText(FText::FromString(TEXT("NPC")));
		Text_JobClass->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (Img_Element)
	{
		Img_Element->SetVisibility(ESlateVisibility::Collapsed);
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UROWidget_TargetInfo::ClearTarget()
{
	CurrentTargetType = EROTargetType::None;
	SetVisibility(ESlateVisibility::Collapsed);
}

void UROWidget_TargetInfo::UpdateHPPercent(float Percent)
{
	if (HPBar && CurrentTargetType == EROTargetType::Monster)
	{
		HPBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
	}
}

void UROWidget_TargetInfo::SetElementIcon(UTexture2D* Icon)
{
	if (Img_Element && Icon)
	{
		Img_Element->SetBrushFromTexture(Icon);
		Img_Element->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else if (Img_Element)
	{
		Img_Element->SetVisibility(ESlateVisibility::Collapsed);
	}
}

FString UROWidget_TargetInfo::GetJobClassName(EROJobClass JobClass)
{
	const UEnum* EnumPtr = StaticEnum<EROJobClass>();
	if (EnumPtr)
	{
		return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(JobClass)).ToString();
	}
	return TEXT("Unknown");
}
