// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROGuildSubsystem.h"

void UROGuildSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	NextGuildID = 1;
}

void UROGuildSubsystem::Deinitialize()
{
	ActiveGuilds.Empty();
	PendingInvites.Empty();
	PlayerGuildMap.Empty();
	Super::Deinitialize();
}

int32 UROGuildSubsystem::CreateGuild(const FString& Name, int32 MasterID)
{
	// Player must not already be in a guild
	if (IsInGuild(MasterID))
	{
		return 0;
	}

	// NOTE: Emperium item consumption should be checked and handled by the caller
	// before invoking CreateGuild.

	FROGuildInfo NewGuild;
	NewGuild.GuildID = NextGuildID++;
	NewGuild.GuildName = Name;
	NewGuild.MasterPlayerID = MasterID;
	NewGuild.GuildLevel = 1;
	NewGuild.GuildExp = 0;
	NewGuild.ExpTaxRate = 0.0f;

	// Initialize guild skill levels (10 guild skill slots)
	NewGuild.GuildSkillLevels.SetNum(10);
	for (int32& Level : NewGuild.GuildSkillLevels)
	{
		Level = 0;
	}

	// Add the master as the first member
	FROGuildMember MasterMember;
	MasterMember.PlayerID = MasterID;
	MasterMember.CharacterName = TEXT(""); // Caller should set this
	MasterMember.Rank = 0; // Guild Master
	MasterMember.JobClass = EROJobClass::Novice;
	MasterMember.BaseLevel = 1;
	MasterMember.bOnline = true;
	NewGuild.Members.Add(MasterMember);

	ActiveGuilds.Add(NewGuild.GuildID, NewGuild);
	PlayerGuildMap.Add(MasterID, NewGuild.GuildID);

	return NewGuild.GuildID;
}

bool UROGuildSubsystem::InviteToGuild(int32 GuildID, int32 InviterID, int32 InviteeID)
{
	FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	if (!Guild)
	{
		return false;
	}

	// Only guild members can invite
	const FROGuildMember* Inviter = FindMember(*Guild, InviterID);
	if (!Inviter)
	{
		return false;
	}

	// Check guild capacity
	if (Guild->Members.Num() >= Guild->GetMaxMembers())
	{
		return false;
	}

	// Target must not already be in a guild
	if (IsInGuild(InviteeID))
	{
		return false;
	}

	PendingInvites.Add(InviteeID, GuildID);
	return true;
}

bool UROGuildSubsystem::AcceptGuildInvite(int32 GuildID, int32 PlayerID)
{
	const int32* PendingGuildID = PendingInvites.Find(PlayerID);
	if (!PendingGuildID || *PendingGuildID != GuildID)
	{
		return false;
	}

	FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	if (!Guild)
	{
		PendingInvites.Remove(PlayerID);
		return false;
	}

	if (Guild->Members.Num() >= Guild->GetMaxMembers())
	{
		PendingInvites.Remove(PlayerID);
		return false;
	}

	if (IsInGuild(PlayerID))
	{
		PendingInvites.Remove(PlayerID);
		return false;
	}

	FROGuildMember NewMember;
	NewMember.PlayerID = PlayerID;
	NewMember.Rank = 2; // Regular member
	NewMember.bOnline = true;
	Guild->Members.Add(NewMember);

	PlayerGuildMap.Add(PlayerID, GuildID);
	PendingInvites.Remove(PlayerID);

	OnGuildMemberJoined.Broadcast(GuildID, PlayerID);
	return true;
}

void UROGuildSubsystem::LeaveGuild(int32 GuildID, int32 PlayerID)
{
	FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	if (!Guild)
	{
		return;
	}

	// Guild master cannot leave; must disband instead
	if (Guild->MasterPlayerID == PlayerID)
	{
		return;
	}

	for (int32 i = Guild->Members.Num() - 1; i >= 0; --i)
	{
		if (Guild->Members[i].PlayerID == PlayerID)
		{
			Guild->Members.RemoveAt(i);
			break;
		}
	}

	PlayerGuildMap.Remove(PlayerID);
	OnGuildMemberLeft.Broadcast(GuildID, PlayerID);
}

bool UROGuildSubsystem::ExpelFromGuild(int32 GuildID, int32 ExpellerID, int32 TargetID)
{
	FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	if (!Guild)
	{
		return false;
	}

	// Cannot expel yourself
	if (ExpellerID == TargetID)
	{
		return false;
	}

	const FROGuildMember* Expeller = FindMember(*Guild, ExpellerID);
	const FROGuildMember* Target = FindMember(*Guild, TargetID);
	if (!Expeller || !Target)
	{
		return false;
	}

	// Only master (0) or sub-leaders (1) can expel, and only lower ranks
	if (Expeller->Rank > 1)
	{
		return false;
	}
	if (Expeller->Rank >= Target->Rank)
	{
		return false;
	}

	for (int32 i = Guild->Members.Num() - 1; i >= 0; --i)
	{
		if (Guild->Members[i].PlayerID == TargetID)
		{
			Guild->Members.RemoveAt(i);
			break;
		}
	}

	PlayerGuildMap.Remove(TargetID);
	OnGuildMemberLeft.Broadcast(GuildID, TargetID);
	return true;
}

void UROGuildSubsystem::DisbandGuild(int32 GuildID, int32 MasterID)
{
	FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	if (!Guild)
	{
		return;
	}

	if (Guild->MasterPlayerID != MasterID)
	{
		return;
	}

	for (const FROGuildMember& Member : Guild->Members)
	{
		PlayerGuildMap.Remove(Member.PlayerID);
		OnGuildMemberLeft.Broadcast(GuildID, Member.PlayerID);
	}

	ActiveGuilds.Remove(GuildID);
}

void UROGuildSubsystem::SetExpTax(int32 GuildID, float Rate)
{
	FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	if (Guild)
	{
		Guild->ExpTaxRate = FMath::Clamp(Rate, 0.0f, 50.0f);
	}
}

void UROGuildSubsystem::SetMemberRank(int32 GuildID, int32 SetterID, int32 TargetID, int32 NewRank)
{
	FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	if (!Guild)
	{
		return;
	}

	// Only the guild master can set ranks
	if (Guild->MasterPlayerID != SetterID)
	{
		return;
	}

	// Cannot change own rank
	if (SetterID == TargetID)
	{
		return;
	}

	// Valid ranks: 0=Master, 1=Sub-leader, 2=Member
	// Cannot assign rank 0 (master) to someone else through this method
	if (NewRank < 1 || NewRank > 2)
	{
		return;
	}

	FROGuildMember* Target = FindMember(*Guild, TargetID);
	if (Target)
	{
		Target->Rank = NewRank;
	}
}

FROGuildInfo UROGuildSubsystem::GetGuildInfo(int32 GuildID) const
{
	const FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	if (Guild)
	{
		return *Guild;
	}
	return FROGuildInfo();
}

int32 UROGuildSubsystem::GetGuildForPlayer(int32 PlayerID) const
{
	const int32* GuildID = PlayerGuildMap.Find(PlayerID);
	return GuildID ? *GuildID : 0;
}

bool UROGuildSubsystem::IsInGuild(int32 PlayerID) const
{
	return PlayerGuildMap.Contains(PlayerID);
}

FROGuildMember* UROGuildSubsystem::FindMember(FROGuildInfo& Guild, int32 PlayerID)
{
	for (FROGuildMember& Member : Guild.Members)
	{
		if (Member.PlayerID == PlayerID)
		{
			return &Member;
		}
	}
	return nullptr;
}

const FROGuildMember* UROGuildSubsystem::FindMember(const FROGuildInfo& Guild, int32 PlayerID) const
{
	for (const FROGuildMember& Member : Guild.Members)
	{
		if (Member.PlayerID == PlayerID)
		{
			return &Member;
		}
	}
	return nullptr;
}
