// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROMapManager.generated.h"

/** Extended map info with portal connections. */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROMapConnectionInfo
{
	GENERATED_BODY()

	/** The base map info. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	FROMapInfo MapInfo;

	/** IDs of maps connected via portals. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	TArray<FName> ConnectedMapIDs;

	/** The UE level/map asset path for server travel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	FString LevelAssetPath;

	/** Recommended level range for this map. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	int32 RecommendedMinLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	int32 RecommendedMaxLevel = 0;

	/** Whether this map is a town/safe zone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	bool bIsTown = false;

	/** Whether this map is a dungeon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	bool bIsDungeon = false;
};

/**
 * UROMapManager
 * Central registry for all game maps, their metadata, and portal connections.
 * Initializes with Prontera region maps on startup.
 */
UCLASS()
class RAGNAROKUE_API UROMapManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- Map Registration ----

	/** Register a map in the database. */
	UFUNCTION(BlueprintCallable, Category = "Map")
	void RegisterMap(const FROMapConnectionInfo& Info);

	// ---- Queries ----

	/** Get the base map info for a given MapID. */
	UFUNCTION(BlueprintCallable, Category = "Map")
	FROMapInfo GetMapInfo(FName MapID) const;

	/** Get full connection info for a map. */
	UFUNCTION(BlueprintCallable, Category = "Map")
	FROMapConnectionInfo GetMapConnectionInfo(FName MapID) const;

	/** Get all maps connected to a given map via portals. */
	UFUNCTION(BlueprintCallable, Category = "Map")
	TArray<FName> GetConnectedMaps(FName MapID) const;

	/** Check if a map ID is registered. */
	UFUNCTION(BlueprintCallable, Category = "Map")
	bool IsMapRegistered(FName MapID) const;

	/** Get all registered map IDs. */
	UFUNCTION(BlueprintCallable, Category = "Map")
	TArray<FName> GetAllMapIDs() const;

protected:
	/** Map database keyed by MapID. */
	UPROPERTY()
	TMap<FName, FROMapConnectionInfo> MapDatabase;

	/** Initialize the Prontera region maps. */
	void InitializePronteraRegion();

	/** Helper to register a simple map entry. */
	void RegisterSimpleMap(FName MapID, const FString& DisplayName, bool bTown, bool bDungeon,
		bool bPvP, int32 MinLevel, int32 MaxLevel, const TArray<FName>& Connections);
};
