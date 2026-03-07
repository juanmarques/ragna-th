// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWidget_StatWindow.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"

UROWidget_StatWindow::UROWidget_StatWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UROWidget_StatWindow::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind allocation buttons
	if (Btn_AllocSTR) Btn_AllocSTR->OnClicked.AddDynamic(this, &UROWidget_StatWindow::OnAllocateSTR);
	if (Btn_AllocAGI) Btn_AllocAGI->OnClicked.AddDynamic(this, &UROWidget_StatWindow::OnAllocateAGI);
	if (Btn_AllocVIT) Btn_AllocVIT->OnClicked.AddDynamic(this, &UROWidget_StatWindow::OnAllocateVIT);
	if (Btn_AllocINT) Btn_AllocINT->OnClicked.AddDynamic(this, &UROWidget_StatWindow::OnAllocateINT);
	if (Btn_AllocDEX) Btn_AllocDEX->OnClicked.AddDynamic(this, &UROWidget_StatWindow::OnAllocateDEX);
	if (Btn_AllocLUK) Btn_AllocLUK->OnClicked.AddDynamic(this, &UROWidget_StatWindow::OnAllocateLUK);

	RefreshAllocationButtons();
}

void UROWidget_StatWindow::UpdateStats(const FROStatBlock& Stats)
{
	CachedStats = Stats;

	auto FormatStat = [](const FString& Label, int32 Base, int32 Bonus) -> FText
	{
		return FText::FromString(FString::Printf(TEXT("%s: %d + %d"), *Label, Base, Bonus));
	};

	if (Text_STR) Text_STR->SetText(FormatStat(TEXT("STR"), Stats.BaseSTR, Stats.BonusSTR));
	if (Text_AGI) Text_AGI->SetText(FormatStat(TEXT("AGI"), Stats.BaseAGI, Stats.BonusAGI));
	if (Text_VIT) Text_VIT->SetText(FormatStat(TEXT("VIT"), Stats.BaseVIT, Stats.BonusVIT));
	if (Text_INT) Text_INT->SetText(FormatStat(TEXT("INT"), Stats.BaseINT, Stats.BonusINT));
	if (Text_DEX) Text_DEX->SetText(FormatStat(TEXT("DEX"), Stats.BaseDEX, Stats.BonusDEX));
	if (Text_LUK) Text_LUK->SetText(FormatStat(TEXT("LUK"), Stats.BaseLUK, Stats.BonusLUK));

	// Update point cost displays
	if (Text_STRCost) Text_STRCost->SetText(FText::AsNumber(GetStatPointCost(Stats.BaseSTR)));
	if (Text_AGICost) Text_AGICost->SetText(FText::AsNumber(GetStatPointCost(Stats.BaseAGI)));
	if (Text_VITCost) Text_VITCost->SetText(FText::AsNumber(GetStatPointCost(Stats.BaseVIT)));
	if (Text_INTCost) Text_INTCost->SetText(FText::AsNumber(GetStatPointCost(Stats.BaseINT)));
	if (Text_DEXCost) Text_DEXCost->SetText(FText::AsNumber(GetStatPointCost(Stats.BaseDEX)));
	if (Text_LUKCost) Text_LUKCost->SetText(FText::AsNumber(GetStatPointCost(Stats.BaseLUK)));

	RefreshAllocationButtons();
}

void UROWidget_StatWindow::SetAvailableStatPoints(int32 Points)
{
	AvailableStatPoints = Points;

	if (Text_StatPoints)
	{
		Text_StatPoints->SetText(FText::FromString(FString::Printf(TEXT("Stat Points: %d"), Points)));
	}

	RefreshAllocationButtons();
}

void UROWidget_StatWindow::SetLevels(int32 BaseLevel, int32 JobLevel)
{
	if (Text_BaseLevel)
	{
		Text_BaseLevel->SetText(FText::FromString(FString::Printf(TEXT("Base Lv: %d"), BaseLevel)));
	}
	if (Text_JobLevel)
	{
		Text_JobLevel->SetText(FText::FromString(FString::Printf(TEXT("Job Lv: %d"), JobLevel)));
	}
}

void UROWidget_StatWindow::SetDerivedStats(int32 ATK, int32 MATK, int32 DEF, int32 MDEF, int32 HIT, int32 FLEE, int32 ASPD, int32 CRIT)
{
	if (Text_ATK) Text_ATK->SetText(FText::FromString(FString::Printf(TEXT("ATK: %d"), ATK)));
	if (Text_MATK) Text_MATK->SetText(FText::FromString(FString::Printf(TEXT("MATK: %d"), MATK)));
	if (Text_DEF) Text_DEF->SetText(FText::FromString(FString::Printf(TEXT("DEF: %d"), DEF)));
	if (Text_MDEF) Text_MDEF->SetText(FText::FromString(FString::Printf(TEXT("MDEF: %d"), MDEF)));
	if (Text_HIT) Text_HIT->SetText(FText::FromString(FString::Printf(TEXT("HIT: %d"), HIT)));
	if (Text_FLEE) Text_FLEE->SetText(FText::FromString(FString::Printf(TEXT("FLEE: %d"), FLEE)));
	if (Text_ASPD) Text_ASPD->SetText(FText::FromString(FString::Printf(TEXT("ASPD: %d"), ASPD)));
	if (Text_CRIT) Text_CRIT->SetText(FText::FromString(FString::Printf(TEXT("CRIT: %d"), CRIT)));
}

void UROWidget_StatWindow::SetHPSP(int32 CurrentHP, int32 MaxHP, int32 CurrentSP, int32 MaxSP)
{
	if (Bar_HP)
	{
		Bar_HP->SetPercent(MaxHP > 0 ? static_cast<float>(CurrentHP) / static_cast<float>(MaxHP) : 0.0f);
	}
	if (Text_HP)
	{
		Text_HP->SetText(FText::FromString(FString::Printf(TEXT("HP: %d / %d"), CurrentHP, MaxHP)));
	}

	if (Bar_SP)
	{
		Bar_SP->SetPercent(MaxSP > 0 ? static_cast<float>(CurrentSP) / static_cast<float>(MaxSP) : 0.0f);
	}
	if (Text_SP)
	{
		Text_SP->SetText(FText::FromString(FString::Printf(TEXT("SP: %d / %d"), CurrentSP, MaxSP)));
	}
}

void UROWidget_StatWindow::SetEXP(int64 BaseEXP, int64 BaseEXPToNext, int64 JobEXP, int64 JobEXPToNext)
{
	if (Bar_BaseEXP)
	{
		Bar_BaseEXP->SetPercent(BaseEXPToNext > 0 ? static_cast<float>(BaseEXP) / static_cast<float>(BaseEXPToNext) : 0.0f);
	}
	if (Text_BaseEXP)
	{
		const float Pct = BaseEXPToNext > 0 ? (static_cast<float>(BaseEXP) / static_cast<float>(BaseEXPToNext)) * 100.0f : 0.0f;
		Text_BaseEXP->SetText(FText::FromString(FString::Printf(TEXT("Base EXP: %.1f%%"), Pct)));
	}

	if (Bar_JobEXP)
	{
		Bar_JobEXP->SetPercent(JobEXPToNext > 0 ? static_cast<float>(JobEXP) / static_cast<float>(JobEXPToNext) : 0.0f);
	}
	if (Text_JobEXP)
	{
		const float Pct = JobEXPToNext > 0 ? (static_cast<float>(JobEXP) / static_cast<float>(JobEXPToNext)) * 100.0f : 0.0f;
		Text_JobEXP->SetText(FText::FromString(FString::Printf(TEXT("Job EXP: %.1f%%"), Pct)));
	}
}

int32 UROWidget_StatWindow::GetStatPointCost(int32 CurrentBaseStat)
{
	// RO formula: cost = floor((CurrentBase - 1) / 10) + 2
	// At stat 1-10: costs 2, at 11-20: costs 3, etc.
	return (CurrentBaseStat - 1) / 10 + 2;
}

void UROWidget_StatWindow::OnAllocateSTR() { OnAllocateStat.Broadcast(EROStat::STR); }
void UROWidget_StatWindow::OnAllocateAGI() { OnAllocateStat.Broadcast(EROStat::AGI); }
void UROWidget_StatWindow::OnAllocateVIT() { OnAllocateStat.Broadcast(EROStat::VIT); }
void UROWidget_StatWindow::OnAllocateINT() { OnAllocateStat.Broadcast(EROStat::INT_STAT); }
void UROWidget_StatWindow::OnAllocateDEX() { OnAllocateStat.Broadcast(EROStat::DEX); }
void UROWidget_StatWindow::OnAllocateLUK() { OnAllocateStat.Broadcast(EROStat::LUK); }

void UROWidget_StatWindow::RefreshAllocationButtons()
{
	auto SetButtonVisible = [this](UButton* Btn, int32 BaseStat)
	{
		if (!Btn) return;
		const int32 Cost = GetStatPointCost(BaseStat);
		const bool bCanAllocate = AvailableStatPoints >= Cost && BaseStat < 99;
		Btn->SetVisibility(bCanAllocate ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	};

	SetButtonVisible(Btn_AllocSTR, CachedStats.BaseSTR);
	SetButtonVisible(Btn_AllocAGI, CachedStats.BaseAGI);
	SetButtonVisible(Btn_AllocVIT, CachedStats.BaseVIT);
	SetButtonVisible(Btn_AllocINT, CachedStats.BaseINT);
	SetButtonVisible(Btn_AllocDEX, CachedStats.BaseDEX);
	SetButtonVisible(Btn_AllocLUK, CachedStats.BaseLUK);
}
