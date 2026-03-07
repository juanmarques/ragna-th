// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWidget_PartyWindow.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

UROWidget_PartyWindow::UROWidget_PartyWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UROWidget_PartyWindow::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshDisplay();
}

void UROWidget_PartyWindow::SetPartyMembers(const TArray<FROPartyMemberInfo>& Members)
{
	PartyMembers = Members;

	// Enforce max party size
	if (PartyMembers.Num() > MAX_PARTY_MEMBERS)
	{
		PartyMembers.SetNum(MAX_PARTY_MEMBERS);
	}

	RefreshDisplay();
}

void UROWidget_PartyWindow::UpdateMemberHP(int32 CharacterID, float HPPercent, float SPPercent)
{
	for (FROPartyMemberInfo& Member : PartyMembers)
	{
		if (Member.CharacterID == CharacterID)
		{
			Member.HPPercent = FMath::Clamp(HPPercent, 0.0f, 1.0f);
			Member.SPPercent = FMath::Clamp(SPPercent, 0.0f, 1.0f);
			RefreshDisplay();
			break;
		}
	}
}

void UROWidget_PartyWindow::SetExpShareMode(EROPartyExpShareMode Mode)
{
	ExpShareMode = Mode;

	if (Text_ExpShareMode)
	{
		switch (Mode)
		{
		case EROPartyExpShareMode::EachTake:
			Text_ExpShareMode->SetText(FText::FromString(TEXT("EXP: Each Take")));
			break;
		case EROPartyExpShareMode::EvenShare:
			Text_ExpShareMode->SetText(FText::FromString(TEXT("EXP: Even Share")));
			break;
		}
	}
}

void UROWidget_PartyWindow::SetIsLocalPlayerLeader(bool bIsLeader)
{
	bLocalPlayerIsLeader = bIsLeader;
	RefreshDisplay();
}

void UROWidget_PartyWindow::RefreshDisplay()
{
	// Visual refresh is handled in Blueprint.
	// Blueprint iterates PartyMembers array and creates member entry widgets,
	// each showing name, job icon, level, HP/SP bars, and leader star icon.
	// Right-click context menu (kick, whisper) is created in Blueprint
	// and calls OnKickPartyMember / OnWhisperPartyMember delegates.
}
