// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWidget_GuildWindow.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/WidgetSwitcher.h"

UROWidget_GuildWindow::UROWidget_GuildWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UROWidget_GuildWindow::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_TabInfo) Btn_TabInfo->OnClicked.AddDynamic(this, &UROWidget_GuildWindow::OnTabInfoClicked);
	if (Btn_TabMembers) Btn_TabMembers->OnClicked.AddDynamic(this, &UROWidget_GuildWindow::OnTabMembersClicked);
	if (Btn_TabSettings) Btn_TabSettings->OnClicked.AddDynamic(this, &UROWidget_GuildWindow::OnTabSettingsClicked);
	if (Btn_LeaveGuild) Btn_LeaveGuild->OnClicked.AddDynamic(this, &UROWidget_GuildWindow::OnLeaveGuildClicked);
	if (Slider_ExpTax) Slider_ExpTax->OnValueChanged.AddDynamic(this, &UROWidget_GuildWindow::OnExpTaxSliderChanged);

	RefreshDisplay();
}

void UROWidget_GuildWindow::SetGuildInfo(const FROGuildDisplayInfo& Info)
{
	CachedGuildInfo = Info;

	if (Text_GuildName) Text_GuildName->SetText(FText::FromString(Info.GuildName));
	if (Text_GuildLevel) Text_GuildLevel->SetText(FText::FromString(FString::Printf(TEXT("Level: %d"), Info.GuildLevel)));
	if (Text_MemberCount) Text_MemberCount->SetText(FText::FromString(FString::Printf(TEXT("Members: %d / %d"), Info.MemberCount, Info.MaxMembers)));
	if (Text_MasterName) Text_MasterName->SetText(FText::FromString(FString::Printf(TEXT("Master: %s"), *Info.MasterName)));

	if (Img_GuildEmblem && Info.GuildEmblem)
	{
		Img_GuildEmblem->SetBrushFromTexture(Info.GuildEmblem);
	}

	if (Slider_ExpTax)
	{
		Slider_ExpTax->SetValue(static_cast<float>(Info.ExpTaxPercent) / 100.0f);
	}
	if (Text_ExpTaxValue)
	{
		Text_ExpTaxValue->SetText(FText::FromString(FString::Printf(TEXT("EXP Tax: %d%%"), Info.ExpTaxPercent)));
	}
}

void UROWidget_GuildWindow::SetGuildMembers(const TArray<FROGuildMemberInfo>& Members)
{
	GuildMembers = Members;
	RefreshDisplay();
}

void UROWidget_GuildWindow::SetIsGuildMaster(bool bIsMaster)
{
	bIsGuildMaster = bIsMaster;

	// Settings tab only visible for guild master
	if (Btn_TabSettings)
	{
		Btn_TabSettings->SetVisibility(bIsMaster ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	RefreshDisplay();
}

void UROWidget_GuildWindow::SwitchTab(EROGuildTab Tab)
{
	// Don't allow non-masters to access Settings tab
	if (Tab == EROGuildTab::Settings && !bIsGuildMaster)
	{
		return;
	}

	CurrentTab = Tab;

	if (TabSwitcher)
	{
		TabSwitcher->SetActiveWidgetIndex(static_cast<int32>(Tab));
	}

	RefreshDisplay();
}

void UROWidget_GuildWindow::RefreshDisplay()
{
	// Member list refresh is handled in Blueprint via MemberListScrollBox.
	// Blueprint creates member entry widgets showing name, class, level, rank, and online status.
}

void UROWidget_GuildWindow::OnTabInfoClicked() { SwitchTab(EROGuildTab::Info); }
void UROWidget_GuildWindow::OnTabMembersClicked() { SwitchTab(EROGuildTab::Members); }
void UROWidget_GuildWindow::OnTabSettingsClicked() { SwitchTab(EROGuildTab::Settings); }

void UROWidget_GuildWindow::OnLeaveGuildClicked()
{
	OnLeaveGuild.Broadcast();
}

void UROWidget_GuildWindow::OnExpTaxSliderChanged(float Value)
{
	if (!bIsGuildMaster)
	{
		return;
	}

	const int32 TaxPercent = FMath::RoundToInt(Value * 100.0f);

	if (Text_ExpTaxValue)
	{
		Text_ExpTaxValue->SetText(FText::FromString(FString::Printf(TEXT("EXP Tax: %d%%"), TaxPercent)));
	}

	OnExpTaxChanged.Broadcast(TaxPercent);
}
