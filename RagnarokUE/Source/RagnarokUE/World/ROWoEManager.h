// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROWoEManager.generated.h"

/** Information about a War of Emperium castle. */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROCastleInfo
{
	GENERATED_BODY()

	/** Unique castle identifier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WoE")
	int32 CastleID = 0;

	/** Display name of the castle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WoE")
	FString CastleName;

	/** Guild ID of the current owner (0 = unowned). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WoE")
	int32 OwnerGuildID = 0;

	/** Map ID where this castle is located. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WoE")
	FName MapID;

	/** Current HP of the Emperium in this castle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WoE")
	int32 EmperiumHP = 0;

	/** Whether this castle is currently under siege (WoE active). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WoE")
	bool bIsUnderSiege = false;

	bool IsValid() const { return CastleID > 0; }
};

/** WoE schedule entry. */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROWoESchedule
{
	GENERATED_BODY()

	/** Day of week (0=Sunday, 1=Monday, ..., 6=Saturday). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WoE")
	int32 DayOfWeek = 3; // Wednesday

	/** Start hour (24-hour format). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WoE")
	int32 StartHour = 20;

	/** Start minute. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WoE")
	int32 StartMinute = 0;

	/** Duration in minutes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WoE")
	int32 DurationMinutes = 120;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWoEStateChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCastleOwnerChanged, int32, CastleID, int32, NewOwnerGuildID);

/**
 * UROWoEManager
 * Manages War of Emperium siege warfare.
 * Handles castle ownership, siege scheduling, Emperium destruction events,
 * and WoE-specific rules (no MVP card effects, restricted skills, no teleporting).
 */
UCLASS()
class RAGNAROKUE_API UROWoEManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- WoE Control ----

	/** Start War of Emperium: enable siege on all castles. */
	UFUNCTION(BlueprintCallable, Category = "WoE")
	void StartWoE();

	/** End War of Emperium: finalize castle ownership and disable siege. */
	UFUNCTION(BlueprintCallable, Category = "WoE")
	void EndWoE();

	/** Called when an Emperium is destroyed in a castle. Transfers ownership. */
	UFUNCTION(BlueprintCallable, Category = "WoE")
	void OnEmperiumDestroyed(int32 CastleID, int32 AttackingGuildID);

	// ---- Queries ----

	/** Get the guild ID that owns a specific castle. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WoE")
	int32 GetCastleOwner(int32 CastleID) const;

	/** Check if WoE is currently active. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WoE")
	bool IsWoEActive() const;

	/** Get information about a specific castle. */
	UFUNCTION(BlueprintCallable, Category = "WoE")
	FROCastleInfo GetCastleInfo(int32 CastleID) const;

	/** Get all castles. */
	UFUNCTION(BlueprintCallable, Category = "WoE")
	TArray<FROCastleInfo> GetAllCastles() const;

	// ---- Castle Management ----

	/** Register a castle in the system. */
	UFUNCTION(BlueprintCallable, Category = "WoE")
	void RegisterCastle(const FROCastleInfo& CastleInfo);

	// ---- Schedule ----

	/** Set the WoE schedule. Typically Wednesday and Saturday. */
	UFUNCTION(BlueprintCallable, Category = "WoE")
	void SetSchedule(const TArray<FROWoESchedule>& NewSchedule);

	/** Check WoE schedule and start/stop accordingly. Call from game mode tick. */
	UFUNCTION(BlueprintCallable, Category = "WoE")
	void CheckSchedule();

	// ---- WoE Rules ----

	/** Check if a skill is restricted during WoE. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WoE")
	bool IsSkillRestrictedInWoE(int32 SkillID) const;

	/** Check if MVP card effects are active (disabled during WoE). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WoE")
	bool AreMVPCardEffectsActive() const;

	/** Check if teleport is allowed on WoE maps (disabled during siege). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WoE")
	bool IsTeleportAllowedInWoE() const;

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category = "WoE")
	FOnWoEStateChanged OnWoEStarted;

	UPROPERTY(BlueprintAssignable, Category = "WoE")
	FOnWoEStateChanged OnWoEEnded;

	UPROPERTY(BlueprintAssignable, Category = "WoE")
	FOnCastleOwnerChanged OnCastleOwnerChanged;

protected:
	/** All castles in the game. */
	UPROPERTY()
	TArray<FROCastleInfo> Castles;

	/** WoE schedule entries. */
	TArray<FROWoESchedule> Schedule;

	/** Whether WoE is currently active. */
	bool bWoEActive = false;

	/** Skill IDs restricted during WoE. */
	TSet<int32> RestrictedWoESkills;

	/** Find a castle by ID. */
	FROCastleInfo* FindCastle(int32 CastleID);
	const FROCastleInfo* FindCastle(int32 CastleID) const;

	/** Initialize default castles and schedules. */
	void InitializeDefaults();
};
