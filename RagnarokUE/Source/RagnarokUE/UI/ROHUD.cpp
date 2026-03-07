// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROHUD.h"
#include "Blueprint/UserWidget.h"
#include "ROWidget_Hotbar.h"
#include "ROWidget_StatWindow.h"
#include "ROWidget_InventoryWindow.h"
#include "ROWidget_EquipmentWindow.h"
#include "ROWidget_SkillWindow.h"
#include "ROWidget_ChatWindow.h"
#include "ROWidget_Minimap.h"
#include "ROWidget_PartyWindow.h"
#include "ROWidget_GuildWindow.h"
#include "ROWidget_QuestLog.h"
#include "ROWidget_TradeWindow.h"
#include "ROWidget_NPCDialogue.h"
#include "ROWidget_ShopWindow.h"
#include "ROWidget_CastBar.h"
#include "ROWidget_TargetInfo.h"

AROHUD::AROHUD()
	: HotbarWidget(nullptr)
	, StatWindowWidget(nullptr)
	, InventoryWidget(nullptr)
	, EquipmentWidget(nullptr)
	, SkillWindowWidget(nullptr)
	, ChatWidget(nullptr)
	, MinimapWidget(nullptr)
	, PartyWidget(nullptr)
	, GuildWidget(nullptr)
	, QuestLogWidget(nullptr)
	, TradeWidget(nullptr)
	, NPCDialogueWidget(nullptr)
	, ShopWidget(nullptr)
	, CastBarWidget(nullptr)
	, TargetInfoWidget(nullptr)
{
}

template<typename T>
T* AROHUD::CreateAndAddWidget(TSubclassOf<T> WidgetClass, bool bStartHidden)
{
	if (!WidgetClass)
	{
		return nullptr;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return nullptr;
	}

	T* Widget = CreateWidget<T>(PC, WidgetClass);
	if (Widget)
	{
		Widget->AddToViewport();
		if (bStartHidden)
		{
			Widget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	return Widget;
}

void AROHUD::BeginPlay()
{
	Super::BeginPlay();

	// Create always-visible widgets
	HotbarWidget = CreateAndAddWidget(HotbarWidgetClass, false);
	ChatWidget = CreateAndAddWidget(ChatWidgetClass, false);
	MinimapWidget = CreateAndAddWidget(MinimapWidgetClass, false);
	TargetInfoWidget = CreateAndAddWidget(TargetInfoWidgetClass, true);
	CastBarWidget = CreateAndAddWidget(CastBarWidgetClass, true);

	// Create toggleable windows (start hidden)
	StatWindowWidget = CreateAndAddWidget(StatWindowWidgetClass, true);
	InventoryWidget = CreateAndAddWidget(InventoryWidgetClass, true);
	EquipmentWidget = CreateAndAddWidget(EquipmentWidgetClass, true);
	SkillWindowWidget = CreateAndAddWidget(SkillWindowWidgetClass, true);
	PartyWidget = CreateAndAddWidget(PartyWidgetClass, true);
	GuildWidget = CreateAndAddWidget(GuildWidgetClass, true);
	QuestLogWidget = CreateAndAddWidget(QuestLogWidgetClass, true);

	// Create session-controlled windows (start hidden)
	TradeWidget = CreateAndAddWidget(TradeWidgetClass, true);
	NPCDialogueWidget = CreateAndAddWidget(NPCDialogueWidgetClass, true);
	ShopWidget = CreateAndAddWidget(ShopWidgetClass, true);

	// Register toggleable windows in the map
	if (StatWindowWidget) WindowMap.Add(FName("Stats"), StatWindowWidget);
	if (InventoryWidget) WindowMap.Add(FName("Inventory"), InventoryWidget);
	if (EquipmentWidget) WindowMap.Add(FName("Equipment"), EquipmentWidget);
	if (SkillWindowWidget) WindowMap.Add(FName("Skills"), SkillWindowWidget);
	if (PartyWidget) WindowMap.Add(FName("Party"), PartyWidget);
	if (GuildWidget) WindowMap.Add(FName("Guild"), GuildWidget);
	if (QuestLogWidget) WindowMap.Add(FName("QuestLog"), QuestLogWidget);
}

void AROHUD::ToggleWindow(FName WindowName)
{
	UUserWidget** FoundWidget = WindowMap.Find(WindowName);
	if (FoundWidget && *FoundWidget)
	{
		UUserWidget* Widget = *FoundWidget;
		if (Widget->GetVisibility() == ESlateVisibility::Collapsed || Widget->GetVisibility() == ESlateVisibility::Hidden)
		{
			Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			Widget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void AROHUD::ShowInventory()
{
	ToggleWindow(FName("Inventory"));
}

void AROHUD::ShowEquipment()
{
	ToggleWindow(FName("Equipment"));
}

void AROHUD::ShowSkills()
{
	ToggleWindow(FName("Skills"));
}

void AROHUD::ShowStats()
{
	ToggleWindow(FName("Stats"));
}

void AROHUD::ShowParty()
{
	ToggleWindow(FName("Party"));
}

void AROHUD::ShowGuild()
{
	ToggleWindow(FName("Guild"));
}

void AROHUD::ShowQuestLog()
{
	ToggleWindow(FName("QuestLog"));
}

void AROHUD::ShowTradeWindow()
{
	if (TradeWidget)
	{
		TradeWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void AROHUD::HideTradeWindow()
{
	if (TradeWidget)
	{
		TradeWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AROHUD::ShowNPCDialogue()
{
	if (NPCDialogueWidget)
	{
		NPCDialogueWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void AROHUD::HideNPCDialogue()
{
	if (NPCDialogueWidget)
	{
		NPCDialogueWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AROHUD::ShowShopWindow()
{
	if (ShopWidget)
	{
		ShopWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void AROHUD::HideShopWindow()
{
	if (ShopWidget)
	{
		ShopWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AROHUD::ShowCastBar()
{
	if (CastBarWidget)
	{
		CastBarWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void AROHUD::HideCastBar()
{
	if (CastBarWidget)
	{
		CastBarWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}
