// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ROHUD.generated.h"

class UROWidget_Hotbar;
class UROWidget_StatWindow;
class UROWidget_InventoryWindow;
class UROWidget_EquipmentWindow;
class UROWidget_SkillWindow;
class UROWidget_ChatWindow;
class UROWidget_Minimap;
class UROWidget_PartyWindow;
class UROWidget_GuildWindow;
class UROWidget_QuestLog;
class UROWidget_TradeWindow;
class UROWidget_NPCDialogue;
class UROWidget_ShopWindow;
class UROWidget_CastBar;
class UROWidget_TargetInfo;

/**
 * AROHUD
 * Main HUD class that contains and manages all UI widgets for the Ragnarok Online recreation.
 */
UCLASS()
class RAGNAROKUE_API AROHUD : public AHUD
{
	GENERATED_BODY()

public:
	AROHUD();

	virtual void BeginPlay() override;

	// --- Widget accessors ---

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	UROWidget_Hotbar* GetHotbar() const { return HotbarWidget; }

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	UROWidget_StatWindow* GetStatWindow() const { return StatWindowWidget; }

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	UROWidget_InventoryWindow* GetInventoryWindow() const { return InventoryWidget; }

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	UROWidget_EquipmentWindow* GetEquipmentWindow() const { return EquipmentWidget; }

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	UROWidget_SkillWindow* GetSkillWindow() const { return SkillWindowWidget; }

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	UROWidget_ChatWindow* GetChatWindow() const { return ChatWidget; }

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	UROWidget_Minimap* GetMinimap() const { return MinimapWidget; }

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	UROWidget_PartyWindow* GetPartyWindow() const { return PartyWidget; }

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	UROWidget_GuildWindow* GetGuildWindow() const { return GuildWidget; }

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	UROWidget_QuestLog* GetQuestLog() const { return QuestLogWidget; }

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	UROWidget_TradeWindow* GetTradeWindow() const { return TradeWidget; }

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	UROWidget_NPCDialogue* GetNPCDialogue() const { return NPCDialogueWidget; }

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	UROWidget_ShopWindow* GetShopWindow() const { return ShopWidget; }

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	UROWidget_CastBar* GetCastBar() const { return CastBarWidget; }

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	UROWidget_TargetInfo* GetTargetInfo() const { return TargetInfoWidget; }

	// --- Window management ---

	/** Toggle visibility of a named window. */
	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	void ToggleWindow(FName WindowName);

	/** Show/hide specific windows called from PlayerController. */
	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	void ShowInventory();

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	void ShowEquipment();

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	void ShowSkills();

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	void ShowStats();

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	void ShowParty();

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	void ShowGuild();

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	void ShowQuestLog();

	/** Show the trade window (non-toggleable - controlled by trade session). */
	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	void ShowTradeWindow();

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	void HideTradeWindow();

	/** Show NPC dialogue (non-toggleable - controlled by NPC interaction). */
	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	void ShowNPCDialogue();

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	void HideNPCDialogue();

	/** Show/hide shop window. */
	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	void ShowShopWindow();

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	void HideShopWindow();

	/** Show/hide cast bar. */
	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	void ShowCastBar();

	UFUNCTION(BlueprintCallable, Category = "RO|HUD")
	void HideCastBar();

protected:
	// --- Widget class references (set in Blueprint or constructor) ---

	UPROPERTY(EditDefaultsOnly, Category = "RO|HUD|Classes")
	TSubclassOf<UROWidget_Hotbar> HotbarWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RO|HUD|Classes")
	TSubclassOf<UROWidget_StatWindow> StatWindowWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RO|HUD|Classes")
	TSubclassOf<UROWidget_InventoryWindow> InventoryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RO|HUD|Classes")
	TSubclassOf<UROWidget_EquipmentWindow> EquipmentWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RO|HUD|Classes")
	TSubclassOf<UROWidget_SkillWindow> SkillWindowWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RO|HUD|Classes")
	TSubclassOf<UROWidget_ChatWindow> ChatWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RO|HUD|Classes")
	TSubclassOf<UROWidget_Minimap> MinimapWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RO|HUD|Classes")
	TSubclassOf<UROWidget_PartyWindow> PartyWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RO|HUD|Classes")
	TSubclassOf<UROWidget_GuildWindow> GuildWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RO|HUD|Classes")
	TSubclassOf<UROWidget_QuestLog> QuestLogWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RO|HUD|Classes")
	TSubclassOf<UROWidget_TradeWindow> TradeWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RO|HUD|Classes")
	TSubclassOf<UROWidget_NPCDialogue> NPCDialogueWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RO|HUD|Classes")
	TSubclassOf<UROWidget_ShopWindow> ShopWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RO|HUD|Classes")
	TSubclassOf<UROWidget_CastBar> CastBarWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RO|HUD|Classes")
	TSubclassOf<UROWidget_TargetInfo> TargetInfoWidgetClass;

	// --- Widget instances ---

	UPROPERTY()
	UROWidget_Hotbar* HotbarWidget;

	UPROPERTY()
	UROWidget_StatWindow* StatWindowWidget;

	UPROPERTY()
	UROWidget_InventoryWindow* InventoryWidget;

	UPROPERTY()
	UROWidget_EquipmentWindow* EquipmentWidget;

	UPROPERTY()
	UROWidget_SkillWindow* SkillWindowWidget;

	UPROPERTY()
	UROWidget_ChatWindow* ChatWidget;

	UPROPERTY()
	UROWidget_Minimap* MinimapWidget;

	UPROPERTY()
	UROWidget_PartyWindow* PartyWidget;

	UPROPERTY()
	UROWidget_GuildWindow* GuildWidget;

	UPROPERTY()
	UROWidget_QuestLog* QuestLogWidget;

	UPROPERTY()
	UROWidget_TradeWindow* TradeWidget;

	UPROPERTY()
	UROWidget_NPCDialogue* NPCDialogueWidget;

	UPROPERTY()
	UROWidget_ShopWindow* ShopWidget;

	UPROPERTY()
	UROWidget_CastBar* CastBarWidget;

	UPROPERTY()
	UROWidget_TargetInfo* TargetInfoWidget;

private:
	/** Helper to create and add a widget to viewport. */
	template<typename T>
	T* CreateAndAddWidget(TSubclassOf<T> WidgetClass, bool bStartHidden = false);

	/** Map of window name to widget for ToggleWindow. */
	UPROPERTY()
	TMap<FName, UUserWidget*> WindowMap;
};
