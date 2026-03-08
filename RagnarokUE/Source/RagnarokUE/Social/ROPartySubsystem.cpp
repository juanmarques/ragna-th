// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROPartySubsystem.h"
#include "RagnarokUE/Core/ROPlayerState.h"
#include "RagnarokUE/Character/ROLevelingComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

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

	// Capture remaining member count and leader status before any removal
	const int32 RemainingMembers = Party->MemberPlayerIDs.Num();
	const bool bWasLeader = (Party->LeaderPlayerID == PlayerID);

	if (RemainingMembers == 0)
	{
		// Party is now empty, remove it. Do not access Party pointer after this.
		ActiveParties.Remove(PartyID);
		return;
	}

	// If the leader left, assign a new leader
	if (bWasLeader)
	{
		Party->LeaderPlayerID = Party->MemberPlayerIDs[0];
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

	// Find eligible members: must be within ShareRadius of the kill location
	const UWorld* World = GetWorld();
	AGameStateBase* GS = World ? World->GetGameState() : nullptr;

	for (int32 MemberID : Party->MemberPlayerIDs)
	{
		if (!GS)
		{
			// Fallback: include all members if we can't query positions
			EligibleMembers.Add(MemberID);
			continue;
		}

		// Look up the member's pawn by PlayerState ID
		APawn* MemberPawn = nullptr;
		for (APlayerState* PS : GS->PlayerArray)
		{
			if (PS && PS->GetPlayerId() == MemberID)
			{
				MemberPawn = PS->GetPawn();
				break;
			}
		}

		if (!MemberPawn)
		{
			continue; // Player not found or offline
		}

		// Distance check: must be within ShareRadius of the kill
		const float Distance = FVector::Dist(MemberPawn->GetActorLocation(), KillLocation);
		if (Distance > ShareRadius)
		{
			continue; // Too far away
		}

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
		// First, determine the highest base level among eligible members.
		int32 HighestLevel = 0;
		TMap<int32, int32> MemberLevels;

		for (int32 MemberID : EligibleMembers)
		{
			int32 MemberLevel = 1;
			if (GS)
			{
				for (APlayerState* PS : GS->PlayerArray)
				{
					if (PS && PS->GetPlayerId() == MemberID)
					{
						AROPlayerState* ROPS = Cast<AROPlayerState>(PS);
						if (ROPS)
						{
							MemberLevel = ROPS->GetBaseLevel();
						}
						break;
					}
				}
			}
			MemberLevels.Add(MemberID, MemberLevel);
			HighestLevel = FMath::Max(HighestLevel, MemberLevel);
		}

		// Filter out members whose level difference exceeds EvenShareLevelRange
		TArray<int32> LevelEligible;
		for (int32 MemberID : EligibleMembers)
		{
			const int32* Level = MemberLevels.Find(MemberID);
			if (Level && (HighestLevel - *Level) <= EvenShareLevelRange)
			{
				LevelEligible.Add(MemberID);
			}
		}

		// If no members are level-eligible, no one receives EXP
		if (LevelEligible.Num() == 0)
		{
			UE_LOG(LogTemp, Log, TEXT("Party EvenShare: No level-eligible members (level gap > %d). EXP not distributed."), EvenShareLevelRange);
			return;
		}

		const int32 ShareCount = LevelEligible.Num();
		const int64 SharedBaseExp = BaseExp / ShareCount;
		const int64 SharedJobExp = JobExp / ShareCount;

		for (int32 MemberID : LevelEligible)
		{
			// Grant EXP via the leveling component
			if (GS)
			{
				for (APlayerState* PS : GS->PlayerArray)
				{
					if (PS && PS->GetPlayerId() == MemberID)
					{
						APawn* MemberPawn = PS->GetPawn();
						if (MemberPawn)
						{
							UROLevelingComponent* LevelComp = MemberPawn->FindComponentByClass<UROLevelingComponent>();
							if (LevelComp)
							{
								LevelComp->AddBaseExp(SharedBaseExp);
								LevelComp->AddJobExp(SharedJobExp);
							}
						}
						break;
					}
				}
			}

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
