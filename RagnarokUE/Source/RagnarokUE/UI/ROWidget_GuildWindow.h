// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "ROWidget_GuildWindow.generated.h"

class UTextBlock;
class UButton;
class UScrollBox;
class UImage;
class USlider;
class UWidgetSwitcher;

/**
 * Guild tab enumeration.
 */
UENUM(BlueprintType)
enum class EROGuildTab : uint8
{
	Info		UMETA(DisplayName = "Info"),
	Members		UMETA(DisplayName = "Members"),
	Settings	UMETA(DisplayName = "Settings")
};

/**
 * FROGuildMemberInfo
 * Display data for a single guild member.
 */
USTRUCT(BlueprintType)
struct FROGuildMemberInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 CharacterID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	EROJobClass JobClass = EROJobClass::Novice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 BaseLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FString RankTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	bool bIsOnline = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	bool bIsMaster = false;
};

/**
 * FROGuildInfo
 * Full guild information.
 */
USTRUCT(BlueprintType)
struct FROGuildInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 GuildID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FString GuildName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 GuildLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 MemberCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 MaxMembers = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FString MasterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 ExpTaxPercent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	UTexture2D* GuildEmblem = nullptr;
};

/**
 * UROWidget_GuildWindow
 * Guild information window with Info, Members, and Settings tabs.
 * Guild emblem display, member list, EXP tax management.
 */
UCLASS()
class RAGNAROKUE_API UROWidget_GuildWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	UROWidget_GuildWindow(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	// --- Data Update ---

	/** Set guild info for the Info tab. */
	UFUNCTION(BlueprintCallable, Category = "RO|Guild")
	void SetGuildInfo(const FROGuildInfo& Info);

	/** Set the guild member list. */
	UFUNCTION(BlueprintCallable, Category = "RO|Guild")
	void SetGuildMembers(const TArray<FROGuildMemberInfo>& Members);

	/** Set whether the local player is the guild master. */
	UFUNCTION(BlueprintCallable, Category = "RO|Guild")
	void SetIsGuildMaster(bool bIsMaster);

	/** Switch the active tab. */
	UFUNCTION(BlueprintCallable, Category = "RO|Guild")
	void SwitchTab(EROGuildTab Tab);

	// --- Delegates ---

	/** Broadcast when the guild master changes EXP tax. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExpTaxChanged, int32, NewTaxPercent);

	UPROPERTY(BlueprintAssignable, Category = "RO|Guild")
	FOnExpTaxChanged OnExpTaxChanged;

	/** Broadcast when the player leaves the guild. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeaveGuild);

	UPROPERTY(BlueprintAssignable, Category = "RO|Guild")
	FOnLeaveGuild OnLeaveGuild;

protected:
	/** Refresh the display for the current tab. */
	UFUNCTION(BlueprintCallable, Category = "RO|Guild")
	void RefreshDisplay();

	// --- Tab button handlers ---
	UFUNCTION()
	void OnTabInfoClicked();
	UFUNCTION()
	void OnTabMembersClicked();
	UFUNCTION()
	void OnTabSettingsClicked();
	UFUNCTION()
	void OnLeaveGuildClicked();
	UFUNCTION()
	void OnExpTaxSliderChanged(float Value);

	// --- Bound Widgets ---

	UPROPERTY(meta = (BindWidgetOptional))
	UWidgetSwitcher* TabSwitcher;

	// Info tab
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_GuildEmblem;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_GuildName;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_GuildLevel;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_MemberCount;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_MasterName;

	// Members tab
	UPROPERTY(meta = (BindWidgetOptional))
	UScrollBox* MemberListScrollBox;

	// Settings tab (master only)
	UPROPERTY(meta = (BindWidgetOptional))
	USlider* Slider_ExpTax;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_ExpTaxValue;

	// Tab buttons
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_TabInfo;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_TabMembers;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_TabSettings;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_LeaveGuild;

private:
	/** Cached guild info. */
	FROGuildInfo CachedGuildInfo;

	/** Guild member list. */
	TArray<FROGuildMemberInfo> GuildMembers;

	/** Whether local player is guild master. */
	bool bIsGuildMaster = false;

	/** Current active tab. */
	EROGuildTab CurrentTab = EROGuildTab::Info;
};
