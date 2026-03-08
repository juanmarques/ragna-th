// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWoEManager.h"
#include "RagnarokUE/Social/ROChatSubsystem.h"
#include "Engine/GameInstance.h"

void UROWoEManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bWoEActive = false;
	InitializeDefaults();
}

void UROWoEManager::Deinitialize()
{
	Castles.Empty();
	Schedule.Empty();
	RestrictedWoESkills.Empty();
	Super::Deinitialize();
}

void UROWoEManager::StartWoE()
{
	if (bWoEActive)
	{
		return;
	}

	bWoEActive = true;

	// Enable siege on all castles
	for (FROCastleInfo& Castle : Castles)
	{
		Castle.bIsUnderSiege = true;
	}

	UE_LOG(LogTemp, Log, TEXT("=== War of Emperium has STARTED! ==="));

	// Broadcast server-wide announcement
	if (UGameInstance* GI = GetGameInstance())
	{
		UROChatSubsystem* Chat = GI->GetSubsystem<UROChatSubsystem>();
		if (Chat)
		{
			Chat->SendMessage(0, EChatChannel::System, TEXT("The War of Emperium has begun!"));
		}
	}
	OnWoEStarted.Broadcast();
}

void UROWoEManager::EndWoE()
{
	if (!bWoEActive)
	{
		return;
	}

	bWoEActive = false;

	// Finalize castle ownership and disable siege
	for (FROCastleInfo& Castle : Castles)
	{
		Castle.bIsUnderSiege = false;

		UE_LOG(LogTemp, Log, TEXT("Castle '%s' (ID: %d) - Owner Guild: %d"),
			*Castle.CastleName, Castle.CastleID, Castle.OwnerGuildID);
	}

	UE_LOG(LogTemp, Log, TEXT("=== War of Emperium has ENDED! ==="));

	// Broadcast server-wide announcement
	if (UGameInstance* GI = GetGameInstance())
	{
		UROChatSubsystem* Chat = GI->GetSubsystem<UROChatSubsystem>();
		if (Chat)
		{
			Chat->SendMessage(0, EChatChannel::System, TEXT("The War of Emperium has ended!"));
		}
	}
	// Castle ownership benefits (guild dungeon access, treasure chests) are applied
	// via the OnWoEEnded delegate - bind in Blueprint to enable map-specific rewards.
	OnWoEEnded.Broadcast();
}

void UROWoEManager::OnEmperiumDestroyed(int32 CastleID, int32 AttackingGuildID)
{
	FROCastleInfo* Castle = FindCastle(CastleID);
	if (!Castle)
	{
		return;
	}

	if (!Castle->bIsUnderSiege)
	{
		return;
	}

	// Validate attacker belongs to a real guild
	if (AttackingGuildID <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("WoE: Emperium destroyed but attacker has no guild (ID: %d). Ignoring."),
			AttackingGuildID);
		return;
	}

	const int32 PreviousOwner = Castle->OwnerGuildID;
	Castle->OwnerGuildID = AttackingGuildID;

	UE_LOG(LogTemp, Log, TEXT("Castle '%s' captured! Guild %d took it from Guild %d"),
		*Castle->CastleName, AttackingGuildID, PreviousOwner);

	// Broadcast castle capture announcement to all players
	if (UGameInstance* GI = GetGameInstance())
	{
		UROChatSubsystem* Chat = GI->GetSubsystem<UROChatSubsystem>();
		if (Chat)
		{
			const FString Announcement = FString::Printf(
				TEXT("The castle [%s] has been captured by Guild %d!"),
				*Castle->CastleName, AttackingGuildID);
			Chat->SendMessage(0, EChatChannel::System, Announcement);
		}
	}

	// Kick non-guild members and respawn Emperium are handled by
	// OnCastleOwnerChanged subscribers (bind in Blueprint for map-specific logic).
	OnCastleOwnerChanged.Broadcast(CastleID, AttackingGuildID);
}

int32 UROWoEManager::GetCastleOwner(int32 CastleID) const
{
	const FROCastleInfo* Castle = FindCastle(CastleID);
	return Castle ? Castle->OwnerGuildID : 0;
}

bool UROWoEManager::IsWoEActive() const
{
	return bWoEActive;
}

FROCastleInfo UROWoEManager::GetCastleInfo(int32 CastleID) const
{
	const FROCastleInfo* Castle = FindCastle(CastleID);
	if (Castle)
	{
		return *Castle;
	}
	return FROCastleInfo();
}

TArray<FROCastleInfo> UROWoEManager::GetAllCastles() const
{
	return Castles;
}

void UROWoEManager::RegisterCastle(const FROCastleInfo& CastleInfo)
{
	// Check for duplicate
	for (FROCastleInfo& Existing : Castles)
	{
		if (Existing.CastleID == CastleInfo.CastleID)
		{
			Existing = CastleInfo;
			return;
		}
	}
	Castles.Add(CastleInfo);
}

void UROWoEManager::SetSchedule(const TArray<FROWoESchedule>& NewSchedule)
{
	Schedule = NewSchedule;
}

void UROWoEManager::CheckSchedule()
{
	if (Schedule.Num() == 0)
	{
		return;
	}

	const FDateTime Now = FDateTime::Now();
	// UE EDayOfWeek: Monday=0..Sunday=6 (ISO); schedule uses Sunday=0..Saturday=6 (POSIX)
	const int32 UEDay = static_cast<int32>(Now.GetDayOfWeek());
	const int32 CurrentDay = (UEDay + 1) % 7;
	const int32 CurrentHour = Now.GetHour();
	const int32 CurrentMinute = Now.GetMinute();
	const int32 CurrentTimeMinutes = CurrentHour * 60 + CurrentMinute;

	bool bShouldBeActive = false;
	for (const FROWoESchedule& Entry : Schedule)
	{
		if (Entry.DayOfWeek == CurrentDay)
		{
			const int32 StartTimeMinutes = Entry.StartHour * 60 + Entry.StartMinute;
			const int32 EndTimeMinutes = StartTimeMinutes + Entry.DurationMinutes;

			if (CurrentTimeMinutes >= StartTimeMinutes && CurrentTimeMinutes < EndTimeMinutes)
			{
				bShouldBeActive = true;
				break;
			}
		}
	}

	if (bShouldBeActive && !bWoEActive)
	{
		StartWoE();
	}
	else if (!bShouldBeActive && bWoEActive)
	{
		EndWoE();
	}
}

bool UROWoEManager::IsSkillRestrictedInWoE(int32 SkillID) const
{
	if (!bWoEActive)
	{
		return false;
	}
	return RestrictedWoESkills.Contains(SkillID);
}

bool UROWoEManager::AreMVPCardEffectsActive() const
{
	// MVP card effects are disabled during WoE
	return !bWoEActive;
}

bool UROWoEManager::IsTeleportAllowedInWoE() const
{
	// Teleport skills are disabled during WoE on castle maps
	return !bWoEActive;
}

FROCastleInfo* UROWoEManager::FindCastle(int32 CastleID)
{
	for (FROCastleInfo& Castle : Castles)
	{
		if (Castle.CastleID == CastleID)
		{
			return &Castle;
		}
	}
	return nullptr;
}

const FROCastleInfo* UROWoEManager::FindCastle(int32 CastleID) const
{
	for (const FROCastleInfo& Castle : Castles)
	{
		if (Castle.CastleID == CastleID)
		{
			return &Castle;
		}
	}
	return nullptr;
}

void UROWoEManager::InitializeDefaults()
{
	// Default WoE schedule: Wednesday 20:00-22:00 and Saturday 20:00-22:00
	{
		FROWoESchedule WedSchedule;
		WedSchedule.DayOfWeek = 3; // Wednesday
		WedSchedule.StartHour = 20;
		WedSchedule.StartMinute = 0;
		WedSchedule.DurationMinutes = 120;
		Schedule.Add(WedSchedule);

		FROWoESchedule SatSchedule;
		SatSchedule.DayOfWeek = 6; // Saturday
		SatSchedule.StartHour = 20;
		SatSchedule.StartMinute = 0;
		SatSchedule.DurationMinutes = 120;
		Schedule.Add(SatSchedule);
	}

	// Register default Prontera castles (5 castles)
	for (int32 i = 1; i <= 5; ++i)
	{
		FROCastleInfo Castle;
		Castle.CastleID = i;
		Castle.CastleName = FString::Printf(TEXT("Prontera Castle %d"), i);
		Castle.OwnerGuildID = 0;
		Castle.MapID = FName(*FString::Printf(TEXT("prt_gld%02d"), i));
		Castle.EmperiumHP = 0;
		Castle.bIsUnderSiege = false;
		Castles.Add(Castle);
	}

	// TODO: Add restricted WoE skills by their IDs.
	// Common restricted skills: Teleport (26), Warp Portal (27), etc.
	RestrictedWoESkills.Add(26); // Teleport
	RestrictedWoESkills.Add(27); // Warp Portal

	UE_LOG(LogTemp, Log, TEXT("ROWoEManager: Initialized with %d castles and %d schedule entries"),
		Castles.Num(), Schedule.Num());
}
