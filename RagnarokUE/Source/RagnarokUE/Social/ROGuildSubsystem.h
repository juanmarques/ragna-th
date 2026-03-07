// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROGuildSubsystem.generated.h"

/** A single guild member's data. */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROGuildMember
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 PlayerID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FString CharacterName;

	/** 0 = Guild Master, 1 = Sub-leader, 2 = Regular member. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 Rank = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	EROJobClass JobClass = EROJobClass::Novice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 BaseLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	bool bOnline = false;
};

/** Complete guild information. */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROGuildInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 GuildID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FString GuildName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 MasterPlayerID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	TArray<FROGuildMember> Members;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 GuildLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int64 GuildExp = 0;

	/** Percentage of base EXP taxed from members (0.0 - 50.0). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float ExpTaxRate = 0.0f;

	/** Guild skill levels (index = skill slot). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	TArray<int32> GuildSkillLevels;

	bool IsValid() const { return GuildID > 0; }

	/** Get current max members based on Guild Extension skill level. Base 16, +4 per level up to 56. */
	int32 GetMaxMembers() const
	{
		// Guild Extension skill is at index 0 in GuildSkillLevels
		int32 ExtensionLevel = 0;
		if (GuildSkillLevels.IsValidIndex(0))
		{
			ExtensionLevel = GuildSkillLevels[0];
		}
		return FMath::Min(16 + (ExtensionLevel * 4), 56);
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGuildMemberChanged, int32, GuildID, int32, PlayerID);

/**
 * UROGuildSubsystem
 * Manages guild creation, membership, ranks, EXP taxation, and skills.
 * Base capacity 16 members, extendable to 56 via Guild Extension skill.
 */
UCLASS()
class RAGNAROKUE_API UROGuildSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- Guild Management ----

	/** Create a new guild. Requires Emperium item (checked externally). Returns GuildID or 0. */
	UFUNCTION(BlueprintCallable, Category = "Guild")
	int32 CreateGuild(const FString& Name, int32 MasterID);

	/** Invite a player to the guild. Returns true if invitation was sent. */
	UFUNCTION(BlueprintCallable, Category = "Guild")
	bool InviteToGuild(int32 GuildID, int32 InviterID, int32 InviteeID);

	/** Accept a guild invitation. Returns true on success. */
	UFUNCTION(BlueprintCallable, Category = "Guild")
	bool AcceptGuildInvite(int32 GuildID, int32 PlayerID);

	/** Leave the guild voluntarily. Guild master cannot leave (must disband). */
	UFUNCTION(BlueprintCallable, Category = "Guild")
	void LeaveGuild(int32 GuildID, int32 PlayerID);

	/** Expel a member. Only master/sub-leaders can expel lower ranks. Returns true on success. */
	UFUNCTION(BlueprintCallable, Category = "Guild")
	bool ExpelFromGuild(int32 GuildID, int32 ExpellerID, int32 TargetID);

	/** Disband the guild entirely. Only the guild master can do this. */
	UFUNCTION(BlueprintCallable, Category = "Guild")
	void DisbandGuild(int32 GuildID, int32 MasterID);

	/** Set the EXP tax rate (0-50%). */
	UFUNCTION(BlueprintCallable, Category = "Guild")
	void SetExpTax(int32 GuildID, float Rate);

	/** Set a member's rank. Only the master can set ranks. */
	UFUNCTION(BlueprintCallable, Category = "Guild")
	void SetMemberRank(int32 GuildID, int32 SetterID, int32 TargetID, int32 NewRank);

	// ---- Queries ----

	UFUNCTION(BlueprintCallable, Category = "Guild")
	FROGuildInfo GetGuildInfo(int32 GuildID) const;

	UFUNCTION(BlueprintCallable, Category = "Guild")
	int32 GetGuildForPlayer(int32 PlayerID) const;

	UFUNCTION(BlueprintCallable, Category = "Guild")
	bool IsInGuild(int32 PlayerID) const;

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category = "Guild")
	FOnGuildMemberChanged OnGuildMemberJoined;

	UPROPERTY(BlueprintAssignable, Category = "Guild")
	FOnGuildMemberChanged OnGuildMemberLeft;

protected:
	UPROPERTY()
	TMap<int32, FROGuildInfo> ActiveGuilds;

	/** Pending invitations: InviteeID -> GuildID. */
	TMap<int32, int32> PendingInvites;

	/** Reverse lookup: PlayerID -> GuildID. */
	TMap<int32, int32> PlayerGuildMap;

	int32 NextGuildID = 1;

	/** Find a member within a guild. Returns nullptr if not found. */
	FROGuildMember* FindMember(FROGuildInfo& Guild, int32 PlayerID);
	const FROGuildMember* FindMember(const FROGuildInfo& Guild, int32 PlayerID) const;
};
