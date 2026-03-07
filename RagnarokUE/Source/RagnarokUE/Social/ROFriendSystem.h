// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROFriendSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFriendEvent, int32, PlayerID, int32, FriendID);

/**
 * UROFriendSystem
 * Simple friend list management with online status tracking.
 * Friends are mutual: both players must confirm the friendship.
 */
UCLASS()
class RAGNAROKUE_API UROFriendSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- Friend Management ----

	/**
	 * Send a friend request. Both players are added mutually if no pending system is used.
	 * In this simple implementation, friendship is immediate and mutual.
	 * @return True if the friend was added successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	bool AddFriend(int32 PlayerID, int32 FriendID);

	/** Remove a friend. Removes mutually from both players' lists. */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	void RemoveFriend(int32 PlayerID, int32 FriendID);

	// ---- Queries ----

	/** Get the full friend list for a player. */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	TArray<int32> GetFriendList(int32 PlayerID) const;

	/** Check if a specific friend is online. */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	bool IsFriendOnline(int32 FriendID) const;

	/** Get all online friends for a player. */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	TArray<int32> GetOnlineFriends(int32 PlayerID) const;

	/** Check if two players are friends. */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	bool AreFriends(int32 PlayerID, int32 OtherID) const;

	// ---- Online Status ----

	/** Mark a player as online. Called when a player connects. */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	void SetPlayerOnline(int32 PlayerID);

	/** Mark a player as offline. Called when a player disconnects. */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	void SetPlayerOffline(int32 PlayerID);

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category = "Friends")
	FOnFriendEvent OnFriendAdded;

	UPROPERTY(BlueprintAssignable, Category = "Friends")
	FOnFriendEvent OnFriendRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Friends")
	FOnFriendEvent OnFriendOnline;

	UPROPERTY(BlueprintAssignable, Category = "Friends")
	FOnFriendEvent OnFriendOffline;

protected:
	/** Friend lists: PlayerID -> Array of FriendIDs. */
	TMap<int32, TArray<int32>> FriendLists;

	/** Set of currently online player IDs. */
	TSet<int32> OnlinePlayers;

	/** Maximum friends per player. */
	static constexpr int32 MaxFriends = 40;
};
