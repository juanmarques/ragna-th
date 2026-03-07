// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROFriendSystem.h"

void UROFriendSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UROFriendSystem::Deinitialize()
{
	FriendLists.Empty();
	OnlinePlayers.Empty();
	Super::Deinitialize();
}

bool UROFriendSystem::AddFriend(int32 PlayerID, int32 FriendID)
{
	// Cannot friend yourself
	if (PlayerID == FriendID)
	{
		return false;
	}

	// Check if already friends
	if (AreFriends(PlayerID, FriendID))
	{
		return false;
	}

	// Check friend list capacity for both players
	TArray<int32>& PlayerFriends = FriendLists.FindOrAdd(PlayerID);
	TArray<int32>& FriendFriends = FriendLists.FindOrAdd(FriendID);

	if (PlayerFriends.Num() >= MaxFriends || FriendFriends.Num() >= MaxFriends)
	{
		return false;
	}

	// Add mutually
	PlayerFriends.Add(FriendID);
	FriendFriends.Add(PlayerID);

	OnFriendAdded.Broadcast(PlayerID, FriendID);
	OnFriendAdded.Broadcast(FriendID, PlayerID);

	return true;
}

void UROFriendSystem::RemoveFriend(int32 PlayerID, int32 FriendID)
{
	// Remove from both lists mutually
	TArray<int32>* PlayerFriends = FriendLists.Find(PlayerID);
	if (PlayerFriends)
	{
		PlayerFriends->Remove(FriendID);
	}

	TArray<int32>* FriendFriends = FriendLists.Find(FriendID);
	if (FriendFriends)
	{
		FriendFriends->Remove(PlayerID);
	}

	OnFriendRemoved.Broadcast(PlayerID, FriendID);
	OnFriendRemoved.Broadcast(FriendID, PlayerID);
}

TArray<int32> UROFriendSystem::GetFriendList(int32 PlayerID) const
{
	const TArray<int32>* Friends = FriendLists.Find(PlayerID);
	if (Friends)
	{
		return *Friends;
	}
	return TArray<int32>();
}

bool UROFriendSystem::IsFriendOnline(int32 FriendID) const
{
	return OnlinePlayers.Contains(FriendID);
}

TArray<int32> UROFriendSystem::GetOnlineFriends(int32 PlayerID) const
{
	TArray<int32> Result;

	const TArray<int32>* Friends = FriendLists.Find(PlayerID);
	if (!Friends)
	{
		return Result;
	}

	for (int32 FriendID : *Friends)
	{
		if (OnlinePlayers.Contains(FriendID))
		{
			Result.Add(FriendID);
		}
	}

	return Result;
}

bool UROFriendSystem::AreFriends(int32 PlayerID, int32 OtherID) const
{
	const TArray<int32>* Friends = FriendLists.Find(PlayerID);
	if (Friends)
	{
		return Friends->Contains(OtherID);
	}
	return false;
}

void UROFriendSystem::SetPlayerOnline(int32 PlayerID)
{
	OnlinePlayers.Add(PlayerID);

	// Notify all friends that this player came online
	const TArray<int32>* Friends = FriendLists.Find(PlayerID);
	if (Friends)
	{
		for (int32 FriendID : *Friends)
		{
			OnFriendOnline.Broadcast(FriendID, PlayerID);
		}
	}
}

void UROFriendSystem::SetPlayerOffline(int32 PlayerID)
{
	OnlinePlayers.Remove(PlayerID);

	// Notify all friends that this player went offline
	const TArray<int32>* Friends = FriendLists.Find(PlayerID);
	if (Friends)
	{
		for (int32 FriendID : *Friends)
		{
			OnFriendOffline.Broadcast(FriendID, PlayerID);
		}
	}
}
