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
	NewGuild.GuildSkillPoints = 0;
	NewGuild.ExpToNextLevel = GetGuildExpForLevel(2);

	// Initialize 20 position slots
	InitializeDefaultPositions(NewGuild.Positions);

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

	// Clean up old invites before adding new ones
	CleanupExpiredInvites();

	PendingInvites.Add(InviteeID, GuildID);
	PendingInviteTimestamps.Add(InviteeID, FPlatformTime::Seconds());
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
		PendingInviteTimestamps.Remove(PlayerID);
		return false;
	}

	if (Guild->Members.Num() >= Guild->GetMaxMembers())
	{
		PendingInvites.Remove(PlayerID);
		PendingInviteTimestamps.Remove(PlayerID);
		return false;
	}

	if (IsInGuild(PlayerID))
	{
		PendingInvites.Remove(PlayerID);
		PendingInviteTimestamps.Remove(PlayerID);
		return false;
	}

	FROGuildMember NewMember;
	NewMember.PlayerID = PlayerID;
	NewMember.Rank = 2; // Regular member
	NewMember.bOnline = true;
	Guild->Members.Add(NewMember);

	PlayerGuildMap.Add(PlayerID, GuildID);
	PendingInvites.Remove(PlayerID);
	PendingInviteTimestamps.Remove(PlayerID);

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

// ============================================================================
// Guild Leveling & EXP
// ============================================================================

void UROGuildSubsystem::ContributeGuildExp(int32 GuildID, int32 PlayerID, int64 ExpAmount)
{
	FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	if (!Guild || ExpAmount <= 0)
	{
		return;
	}

	FROGuildMember* Member = FindMember(*Guild, PlayerID);
	if (!Member)
	{
		return;
	}

	// Track individual contribution
	Member->ContributedExp += ExpAmount;

	// Add to guild EXP pool
	Guild->GuildExp += ExpAmount;

	// Check for level up
	ProcessGuildLevelUp(*Guild);
}

bool UROGuildSubsystem::AllocateGuildSkillPoint(int32 GuildID, int32 MasterID, int32 SkillSlot)
{
	FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	if (!Guild)
	{
		return false;
	}

	// Only guild master can allocate skill points
	if (Guild->MasterPlayerID != MasterID)
	{
		return false;
	}

	if (Guild->GuildSkillPoints <= 0)
	{
		return false;
	}

	if (!Guild->GuildSkillLevels.IsValidIndex(SkillSlot))
	{
		return false;
	}

	// Max skill level is 10
	if (Guild->GuildSkillLevels[SkillSlot] >= 10)
	{
		return false;
	}

	Guild->GuildSkillLevels[SkillSlot]++;
	Guild->GuildSkillPoints--;

	return true;
}

int64 UROGuildSubsystem::GetGuildExpForLevel(int32 Level)
{
	// Guild EXP table (pre-renewal approximation)
	// Each level requires progressively more EXP
	if (Level <= 1) return 0;
	if (Level <= 5) return 6000LL * (Level - 1);
	if (Level <= 10) return 18000LL * (Level - 1);
	if (Level <= 20) return 36000LL * (Level - 1);
	if (Level <= 30) return 72000LL * (Level - 1);
	if (Level <= 40) return 144000LL * (Level - 1);
	return 216000LL * (Level - 1);
}

void UROGuildSubsystem::ProcessGuildLevelUp(FROGuildInfo& Guild)
{
	// Max guild level is 50
	while (Guild.GuildLevel < 50 && Guild.ExpToNextLevel > 0 && Guild.GuildExp >= Guild.ExpToNextLevel)
	{
		Guild.GuildExp -= Guild.ExpToNextLevel;
		Guild.GuildLevel++;
		Guild.GuildSkillPoints++;
		Guild.ExpToNextLevel = GetGuildExpForLevel(Guild.GuildLevel + 1);

		UE_LOG(LogTemp, Log, TEXT("Guild '%s' leveled up to %d! Skill point earned."),
			*Guild.GuildName, Guild.GuildLevel);
	}
}

// ============================================================================
// Positions & Titles
// ============================================================================

void UROGuildSubsystem::InitializeDefaultPositions(TArray<FROGuildPosition>& Positions)
{
	Positions.SetNum(20);

	// Position 0: Guild Master
	Positions[0].Title = TEXT("Guild Master");
	Positions[0].bCanInvite = true;
	Positions[0].bCanExpel = true;
	Positions[0].bCanAccessStorage = true;

	// Position 1: Sub-Leader
	Positions[1].Title = TEXT("Sub-Leader");
	Positions[1].bCanInvite = true;
	Positions[1].bCanExpel = true;
	Positions[1].bCanAccessStorage = true;

	// Positions 2-18: Regular members with no special perms
	for (int32 i = 2; i < 19; ++i)
	{
		Positions[i].Title = TEXT("Member");
		Positions[i].bCanInvite = false;
		Positions[i].bCanExpel = false;
		Positions[i].bCanAccessStorage = false;
	}

	// Position 19: Default position for new members
	Positions[19].Title = TEXT("New Member");
	Positions[19].bCanInvite = false;
	Positions[19].bCanExpel = false;
	Positions[19].bCanAccessStorage = false;
}

void UROGuildSubsystem::SetPosition(int32 GuildID, int32 MasterID, int32 PositionIndex,
	const FROGuildPosition& Position)
{
	FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	if (!Guild || Guild->MasterPlayerID != MasterID)
	{
		return;
	}

	if (!Guild->Positions.IsValidIndex(PositionIndex))
	{
		return;
	}

	// Cannot modify the master position (index 0)
	if (PositionIndex == 0)
	{
		return;
	}

	Guild->Positions[PositionIndex] = Position;
}

void UROGuildSubsystem::SetMemberPosition(int32 GuildID, int32 SetterID, int32 TargetID,
	int32 PositionIndex)
{
	FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	if (!Guild)
	{
		return;
	}

	// Only master or sub-leaders can assign positions
	const FROGuildMember* Setter = FindMember(*Guild, SetterID);
	if (!Setter || Setter->Rank > 1)
	{
		return;
	}

	FROGuildMember* Target = FindMember(*Guild, TargetID);
	if (!Target)
	{
		return;
	}

	if (!Guild->Positions.IsValidIndex(PositionIndex))
	{
		return;
	}

	// Cannot assign to master position (index 0) unless they are the master
	if (PositionIndex == 0 && TargetID != Guild->MasterPlayerID)
	{
		return;
	}

	Target->PositionIndex = PositionIndex;
}

// ============================================================================
// Alliances & Hostility
// ============================================================================

bool UROGuildSubsystem::RequestAlliance(int32 GuildID, int32 TargetGuildID)
{
	FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	FROGuildInfo* TargetGuild = ActiveGuilds.Find(TargetGuildID);
	if (!Guild || !TargetGuild || GuildID == TargetGuildID)
	{
		return false;
	}

	// Max 3 alliances
	if (Guild->GetAllianceCount() >= 3 || TargetGuild->GetAllianceCount() >= 3)
	{
		return false;
	}

	// Check if already related
	for (const auto& R : Guild->Relations)
	{
		if (R.GuildID == TargetGuildID)
		{
			return false;
		}
	}

	FROGuildAllianceEntry Entry;
	Entry.GuildID = TargetGuildID;
	Entry.GuildName = TargetGuild->GuildName;
	Entry.bIsAlliance = true;
	Guild->Relations.Add(Entry);

	FROGuildAllianceEntry ReverseEntry;
	ReverseEntry.GuildID = GuildID;
	ReverseEntry.GuildName = Guild->GuildName;
	ReverseEntry.bIsAlliance = true;
	TargetGuild->Relations.Add(ReverseEntry);

	return true;
}

bool UROGuildSubsystem::DeclareHostility(int32 GuildID, int32 TargetGuildID)
{
	FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	if (!Guild || !ActiveGuilds.Contains(TargetGuildID) || GuildID == TargetGuildID)
	{
		return false;
	}

	// Max 3 hostile
	if (Guild->GetHostileCount() >= 3)
	{
		return false;
	}

	// Check if already related
	for (const auto& R : Guild->Relations)
	{
		if (R.GuildID == TargetGuildID)
		{
			return false;
		}
	}

	FROGuildInfo* TargetGuild = ActiveGuilds.Find(TargetGuildID);
	if (!TargetGuild)
	{
		return false;
	}

	FROGuildAllianceEntry Entry;
	Entry.GuildID = TargetGuildID;
	Entry.GuildName = TargetGuild->GuildName;
	Entry.bIsAlliance = false;
	Guild->Relations.Add(Entry);

	// Add reverse hostility entry so the target guild knows about it
	FROGuildAllianceEntry ReverseEntry;
	ReverseEntry.GuildID = GuildID;
	ReverseEntry.GuildName = Guild->GuildName;
	ReverseEntry.bIsAlliance = false;
	TargetGuild->Relations.Add(ReverseEntry);

	return true;
}

void UROGuildSubsystem::RemoveRelation(int32 GuildID, int32 TargetGuildID)
{
	FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	if (!Guild)
	{
		return;
	}

	for (int32 i = Guild->Relations.Num() - 1; i >= 0; --i)
	{
		if (Guild->Relations[i].GuildID == TargetGuildID)
		{
			Guild->Relations.RemoveAt(i);

			// Also remove the reverse entry from the target guild
			FROGuildInfo* TargetGuild = ActiveGuilds.Find(TargetGuildID);
			if (TargetGuild)
			{
				for (int32 j = TargetGuild->Relations.Num() - 1; j >= 0; --j)
				{
					if (TargetGuild->Relations[j].GuildID == GuildID)
					{
						TargetGuild->Relations.RemoveAt(j);
						break;
					}
				}
			}
			break;
		}
	}
}

// ============================================================================
// Guild Notice
// ============================================================================

void UROGuildSubsystem::SetGuildNotice(int32 GuildID, int32 SetterID, const FString& Notice)
{
	FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	if (!Guild)
	{
		return;
	}

	// Only master or sub-leaders can set notice
	const FROGuildMember* Setter = FindMember(*Guild, SetterID);
	if (!Setter || Setter->Rank > 1)
	{
		return;
	}

	Guild->GuildNotice = Notice;
}

FString UROGuildSubsystem::GetGuildNotice(int32 GuildID) const
{
	const FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	return Guild ? Guild->GuildNotice : FString();
}

// ============================================================================
// Guild Master Transfer
// ============================================================================

bool UROGuildSubsystem::TransferGuildMaster(int32 GuildID, int32 CurrentMasterID, int32 NewMasterID)
{
	FROGuildInfo* Guild = ActiveGuilds.Find(GuildID);
	if (!Guild)
	{
		return false;
	}

	if (Guild->MasterPlayerID != CurrentMasterID)
	{
		return false;
	}

	if (CurrentMasterID == NewMasterID)
	{
		return false;
	}

	FROGuildMember* OldMaster = FindMember(*Guild, CurrentMasterID);
	FROGuildMember* NewMaster = FindMember(*Guild, NewMasterID);
	if (!OldMaster || !NewMaster)
	{
		return false;
	}

	// Transfer: swap ranks and positions
	Guild->MasterPlayerID = NewMasterID;
	const int32 NewMasterOldPosition = NewMaster->PositionIndex;
	OldMaster->Rank = NewMaster->Rank;
	OldMaster->PositionIndex = NewMasterOldPosition;
	NewMaster->Rank = 0;
	NewMaster->PositionIndex = 0;

	return true;
}

// ============================================================================
// Pending Invite Cleanup
// ============================================================================

void UROGuildSubsystem::CleanupExpiredInvites()
{
	TArray<int32> ExpiredKeys;
	const double CurrentTime = FPlatformTime::Seconds();

	for (const auto& Pair : PendingInviteTimestamps)
	{
		if (CurrentTime - Pair.Value > 30.0)
		{
			ExpiredKeys.Add(Pair.Key);
		}
	}

	for (int32 Key : ExpiredKeys)
	{
		PendingInvites.Remove(Key);
		PendingInviteTimestamps.Remove(Key);
	}
}
