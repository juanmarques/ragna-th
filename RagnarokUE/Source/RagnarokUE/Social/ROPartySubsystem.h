// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROPartySubsystem.generated.h"

/** Information about a party. */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROPartyInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int32 PartyID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FString PartyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int32 LeaderPlayerID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	TArray<int32> MemberPlayerIDs;

	/** If true, EXP is shared evenly among nearby members (Even Share mode). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	bool bEvenShare = false;

	bool IsValid() const { return PartyID > 0; }
};

/** Delegate broadcast when party membership changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartyChanged, int32, PartyID, int32, PlayerID);

/**
 * UROPartySubsystem
 * Manages party creation, membership, EXP distribution, and dissolution.
 * Maximum 12 members per party. Even Share mode requires members within 15 base levels.
 */
UCLASS()
class RAGNAROKUE_API UROPartySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Maximum number of members in a party. */
	static constexpr int32 MaxPartyMembers = 12;

	/** Level range for Even Share EXP distribution. */
	static constexpr int32 EvenShareLevelRange = 15;

	// ---- Party Management ----

	/** Create a new party. Returns the new PartyID, or 0 on failure. */
	UFUNCTION(BlueprintCallable, Category = "Party")
	int32 CreateParty(const FString& Name, int32 LeaderID);

	/** Invite a player to a party. Returns true if the invitation was sent. */
	UFUNCTION(BlueprintCallable, Category = "Party")
	bool InviteToParty(int32 PartyID, int32 InviterID, int32 InviteeID);

	/** Accept a party invitation. Returns true on success. */
	UFUNCTION(BlueprintCallable, Category = "Party")
	bool AcceptInvite(int32 PartyID, int32 PlayerID);

	/** Leave a party voluntarily. */
	UFUNCTION(BlueprintCallable, Category = "Party")
	void LeaveParty(int32 PartyID, int32 PlayerID);

	/** Disband the party (removes all members). Only the leader can disband. */
	UFUNCTION(BlueprintCallable, Category = "Party")
	void DisbandParty(int32 PartyID);

	/** Kick a member from the party. Only the leader can kick. Returns true on success. */
	UFUNCTION(BlueprintCallable, Category = "Party")
	bool KickFromParty(int32 PartyID, int32 KickerID, int32 TargetID);

	/** Set EXP sharing mode. True = Even Share, False = Each Take. */
	UFUNCTION(BlueprintCallable, Category = "Party")
	void SetExpShareMode(int32 PartyID, bool bEvenShare);

	// ---- Queries ----

	/** Get party info by PartyID. Returns an invalid struct if not found. */
	UFUNCTION(BlueprintCallable, Category = "Party")
	FROPartyInfo GetPartyInfo(int32 PartyID) const;

	/** Check whether a player is currently in any party. */
	UFUNCTION(BlueprintCallable, Category = "Party")
	bool IsInParty(int32 PlayerID) const;

	/** Get the PartyID for a player. Returns 0 if not in a party. */
	UFUNCTION(BlueprintCallable, Category = "Party")
	int32 GetPartyForPlayer(int32 PlayerID) const;

	// ---- EXP Distribution ----

	/**
	 * Distribute EXP among party members near a kill location.
	 * In Even Share mode, EXP is split among eligible members (within 15 base-level range).
	 * In Each Take mode, each nearby member receives the full amount.
	 */
	UFUNCTION(BlueprintCallable, Category = "Party")
	void DistributeExp(int32 PartyID, int64 BaseExp, int64 JobExp, FVector KillLocation);

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category = "Party")
	FOnPartyChanged OnMemberJoined;

	UPROPERTY(BlueprintAssignable, Category = "Party")
	FOnPartyChanged OnMemberLeft;

protected:
	/** All active parties, keyed by PartyID. */
	UPROPERTY()
	TMap<int32, FROPartyInfo> ActiveParties;

	/** Pending invitations: InviteeID -> PartyID. */
	UPROPERTY()
	TMap<int32, int32> PendingInvites;

	/** Reverse lookup: PlayerID -> PartyID. */
	TMap<int32, int32> PlayerPartyMap;

	/** Auto-incrementing party ID counter. */
	int32 NextPartyID = 1;
};
