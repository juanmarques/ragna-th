// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "ROWidget_PartyWindow.generated.h"

class UVerticalBox;
class UTextBlock;
class UProgressBar;

/**
 * EROPartyExpShareMode
 * How party EXP is distributed.
 */
UENUM(BlueprintType)
enum class EROPartyExpShareMode : uint8
{
	EachTake	UMETA(DisplayName = "Each Take"),
	EvenShare	UMETA(DisplayName = "Even Share")
};

/**
 * FROPartyMemberInfo
 * Display data for a single party member.
 */
USTRUCT(BlueprintType)
struct FROPartyMemberInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int32 CharacterID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	EROJobClass JobClass = EROJobClass::Novice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int32 BaseLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	float HPPercent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	float SPPercent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	bool bIsLeader = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	bool bIsOnline = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FString CurrentMap;
};

/**
 * UROWidget_PartyWindow
 * Displays party members (up to 12) with HP/SP bars, job icons,
 * leader indicator, and right-click context menu.
 */
UCLASS()
class RAGNAROKUE_API UROWidget_PartyWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	UROWidget_PartyWindow(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	/** Maximum party members in RO. */
	static constexpr int32 MAX_PARTY_MEMBERS = 12;

	// --- Data Update ---

	/** Set the full party member list. */
	UFUNCTION(BlueprintCallable, Category = "RO|Party")
	void SetPartyMembers(const TArray<FROPartyMemberInfo>& Members);

	/** Update a single member's HP/SP. */
	UFUNCTION(BlueprintCallable, Category = "RO|Party")
	void UpdateMemberHP(int32 CharacterID, float HPPercent, float SPPercent);

	/** Set the EXP share mode indicator. */
	UFUNCTION(BlueprintCallable, Category = "RO|Party")
	void SetExpShareMode(EROPartyExpShareMode Mode);

	/** Set whether the local player is the party leader. */
	UFUNCTION(BlueprintCallable, Category = "RO|Party")
	void SetIsLocalPlayerLeader(bool bIsLeader);

	// --- Delegates ---

	/** Broadcast when leader kicks a member. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKickPartyMember, int32, CharacterID);

	UPROPERTY(BlueprintAssignable, Category = "RO|Party")
	FOnKickPartyMember OnKickPartyMember;

	/** Broadcast when a member is whispered. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWhisperPartyMember, const FString&, MemberName);

	UPROPERTY(BlueprintAssignable, Category = "RO|Party")
	FOnWhisperPartyMember OnWhisperPartyMember;

protected:
	/** Refresh the party member display. */
	UFUNCTION(BlueprintCallable, Category = "RO|Party")
	void RefreshDisplay();

	// --- Bound Widgets ---

	UPROPERTY(meta = (BindWidgetOptional))
	UVerticalBox* MemberListBox;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_ExpShareMode;

private:
	/** Party member data. */
	TArray<FROPartyMemberInfo> PartyMembers;

	/** Whether the local player is the leader. */
	bool bLocalPlayerIsLeader = false;

	/** Current EXP share mode. */
	EROPartyExpShareMode ExpShareMode = EROPartyExpShareMode::EachTake;
};
