// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROPartySubsystem.h"

void UROPartySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	NextPartyID = 1;
}

void UROPartySubsystem::Deinitialize()
{
	ActiveParties.Empty();
	PendingInvites.Empty();
	PlayerPartyMap.Empty();
	Super::Deinitialize();
}

int32 UROPartySubsystem::CreateParty(const FString& Name, int32 LeaderID)
{
	// Player must not already be in a party
	if (IsInParty(LeaderID))
	{
		return 0;
	}

	FROPartyInfo NewParty;
	NewParty.PartyID = NextPartyID++;
	NewParty.PartyName = Name;
	NewParty.LeaderPlayerID = LeaderID;
	NewParty.MemberPlayerIDs.Add(LeaderID);
	NewParty.bEvenShare = false;

	ActiveParties.Add(NewParty.PartyID, NewParty);
	PlayerPartyMap.Add(LeaderID, NewParty.PartyID);

	return NewParty.PartyID;
}

bool UROPartySubsystem::InviteToParty(int32 PartyID, int32 InviterID, int32 InviteeID)
{
	FROPartyInfo* Party = ActiveParties.Find(PartyID);
	if (!Party)
	{
		return false;
	}

	// Only party members can invite
	if (!Party->MemberPlayerIDs.Contains(InviterID))
	{
		return false;
	}

	// Party full check
	if (Party->MemberPlayerIDs.Num() >= MaxPartyMembers)
	{
		return false;
	}

	// Target must not already be in a party
	if (IsInParty(InviteeID))
	{
		return false;
	}

	// Store pending invite
	PendingInvites.Add(InviteeID, PartyID);
	return true;
}

bool UROPartySubsystem::AcceptInvite(int32 PartyID, int32 PlayerID)
{
	// Verify pending invite exists
	const int32* PendingPartyID = PendingInvites.Find(PlayerID);
	if (!PendingPartyID || *PendingPartyID != PartyID)
	{
		return false;
	}

	FROPartyInfo* Party = ActiveParties.Find(PartyID);
	if (!Party)
	{
		PendingInvites.Remove(PlayerID);
		return false;
	}

	// Party full check
	if (Party->MemberPlayerIDs.Num() >= MaxPartyMembers)
	{
		PendingInvites.Remove(PlayerID);
		return false;
	}

	// Player must not already be in a party
	if (IsInParty(PlayerID))
	{
		PendingInvites.Remove(PlayerID);
		return false;
	}

	Party->MemberPlayerIDs.Add(PlayerID);
	PlayerPartyMap.Add(PlayerID, PartyID);
	PendingInvites.Remove(PlayerID);

	OnMemberJoined.Broadcast(PartyID, PlayerID);
	return true;
}

void UROPartySubsystem::LeaveParty(int32 PartyID, int32 PlayerID)
{
	FROPartyInfo* Party = ActiveParties.Find(PartyID);
	if (!Party)
	{
		return;
	}

	if (!Party->MemberPlayerIDs.Contains(PlayerID))
	{
		return;
	}

	Party->MemberPlayerIDs.Remove(PlayerID);
	PlayerPartyMap.Remove(PlayerID);

	OnMemberLeft.Broadcast(PartyID, PlayerID);

	// If the leader left, assign a new leader or disband
	if (Party->LeaderPlayerID == PlayerID)
	{
		if (Party->MemberPlayerIDs.Num() > 0)
		{
			Party->LeaderPlayerID = Party->MemberPlayerIDs[0];
		}
		else
		{
			ActiveParties.Remove(PartyID);
		}
	}

	// If party is now empty, remove it
	if (Party->MemberPlayerIDs.Num() == 0)
	{
		ActiveParties.Remove(PartyID);
	}
}

void UROPartySubsystem::DisbandParty(int32 PartyID)
{
	FROPartyInfo* Party = ActiveParties.Find(PartyID);
	if (!Party)
	{
		return;
	}

	// Remove all members from the reverse lookup
	for (int32 MemberID : Party->MemberPlayerIDs)
	{
		PlayerPartyMap.Remove(MemberID);
		OnMemberLeft.Broadcast(PartyID, MemberID);
	}

	ActiveParties.Remove(PartyID);
}

bool UROPartySubsystem::KickFromParty(int32 PartyID, int32 KickerID, int32 TargetID)
{
	FROPartyInfo* Party = ActiveParties.Find(PartyID);
	if (!Party)
	{
		return false;
	}

	// Only the leader can kick
	if (Party->LeaderPlayerID != KickerID)
	{
		return false;
	}

	// Cannot kick yourself (use LeaveParty or DisbandParty)
	if (KickerID == TargetID)
	{
		return false;
	}

	if (!Party->MemberPlayerIDs.Contains(TargetID))
	{
		return false;
	}

	Party->MemberPlayerIDs.Remove(TargetID);
	PlayerPartyMap.Remove(TargetID);

	OnMemberLeft.Broadcast(PartyID, TargetID);
	return true;
}

void UROPartySubsystem::SetExpShareMode(int32 PartyID, bool bEvenShare)
{
	FROPartyInfo* Party = ActiveParties.Find(PartyID);
	if (Party)
	{
		Party->bEvenShare = bEvenShare;
	}
}

FROPartyInfo UROPartySubsystem::GetPartyInfo(int32 PartyID) const
{
	const FROPartyInfo* Party = ActiveParties.Find(PartyID);
	if (Party)
	{
		return *Party;
	}
	return FROPartyInfo();
}

bool UROPartySubsystem::IsInParty(int32 PlayerID) const
{
	return PlayerPartyMap.Contains(PlayerID);
}

int32 UROPartySubsystem::GetPartyForPlayer(int32 PlayerID) const
{
	const int32* PartyID = PlayerPartyMap.Find(PlayerID);
	return PartyID ? *PartyID : 0;
}

void UROPartySubsystem::DistributeExp(int32 PartyID, int64 BaseExp, int64 JobExp, FVector KillLocation)
{
	const FROPartyInfo* Party = ActiveParties.Find(PartyID);
	if (!Party)
	{
		return;
	}

	// In a real implementation, we would query each member's location and base level.
	// For now, we collect eligible member IDs (those within range of the kill).
	// The share radius is typically the screen range, roughly 1500 units in our scale.
	constexpr float ShareRadius = 1500.0f;

	TArray<int32> EligibleMembers;

	for (int32 MemberID : Party->MemberPlayerIDs)
	{
		// TODO: Query actual player positions and base levels from the world.
		// For the subsystem logic, we assume all members are eligible placeholders.
		// In production, filter by distance <= ShareRadius from KillLocation.
		EligibleMembers.Add(MemberID);
	}

	if (EligibleMembers.Num() == 0)
	{
		return;
	}

	if (Party->bEvenShare)
	{
		// Even Share: split EXP evenly among eligible members within the level range.
		// In RO, if level difference > 15, that member gets no share.
		// TODO: Filter by base level difference <= EvenShareLevelRange.

		const int32 ShareCount = FMath::Max(1, EligibleMembers.Num());
		const int64 SharedBaseExp = BaseExp / ShareCount;
		const int64 SharedJobExp = JobExp / ShareCount;

		for (int32 MemberID : EligibleMembers)
		{
			// TODO: Call leveling component to grant SharedBaseExp, SharedJobExp to MemberID.
			UE_LOG(LogTemp, Log, TEXT("Party EvenShare: Player %d receives BaseExp=%lld, JobExp=%lld"),
				MemberID, SharedBaseExp, SharedJobExp);
		}
	}
	else
	{
		// Each Take: only the killing player receives EXP (handled by caller).
		// Party members do not receive EXP in this mode; the caller should grant directly.
		UE_LOG(LogTemp, Log, TEXT("Party EachTake: EXP goes to killer only (BaseExp=%lld, JobExp=%lld)"),
			BaseExp, JobExp);
	}
}
