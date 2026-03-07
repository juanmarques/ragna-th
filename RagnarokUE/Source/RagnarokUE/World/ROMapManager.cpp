// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROMapManager.h"

void UROMapManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializePronteraRegion();
}

void UROMapManager::Deinitialize()
{
	MapDatabase.Empty();
	Super::Deinitialize();
}

void UROMapManager::RegisterMap(const FROMapConnectionInfo& Info)
{
	MapDatabase.Add(Info.MapInfo.MapID, Info);
}

FROMapInfo UROMapManager::GetMapInfo(FName MapID) const
{
	const FROMapConnectionInfo* Info = MapDatabase.Find(MapID);
	if (Info)
	{
		return Info->MapInfo;
	}
	return FROMapInfo();
}

FROMapConnectionInfo UROMapManager::GetMapConnectionInfo(FName MapID) const
{
	const FROMapConnectionInfo* Info = MapDatabase.Find(MapID);
	if (Info)
	{
		return *Info;
	}
	return FROMapConnectionInfo();
}

TArray<FName> UROMapManager::GetConnectedMaps(FName MapID) const
{
	const FROMapConnectionInfo* Info = MapDatabase.Find(MapID);
	if (Info)
	{
		return Info->ConnectedMapIDs;
	}
	return TArray<FName>();
}

bool UROMapManager::IsMapRegistered(FName MapID) const
{
	return MapDatabase.Contains(MapID);
}

TArray<FName> UROMapManager::GetAllMapIDs() const
{
	TArray<FName> Result;
	MapDatabase.GetKeys(Result);
	return Result;
}

void UROMapManager::RegisterSimpleMap(FName MapID, const FString& DisplayName, bool bTown, bool bDungeon,
	bool bPvP, int32 MinLevel, int32 MaxLevel, const TArray<FName>& Connections)
{
	FROMapConnectionInfo Info;
	Info.MapInfo.MapID = MapID;
	Info.MapInfo.DisplayName = DisplayName;
	Info.MapInfo.bIsPvP = bPvP;
	Info.MapInfo.bIsIndoor = bDungeon;
	Info.bIsTown = bTown;
	Info.bIsDungeon = bDungeon;
	Info.RecommendedMinLevel = MinLevel;
	Info.RecommendedMaxLevel = MaxLevel;
	Info.ConnectedMapIDs = Connections;

	MapDatabase.Add(MapID, Info);
}

void UROMapManager::InitializePronteraRegion()
{
	// === Prontera Capital City ===
	RegisterSimpleMap(
		FName(TEXT("prontera")),
		TEXT("Prontera"),
		true, false, false, 1, 99,
		{ FName(TEXT("prt_fild01")), FName(TEXT("prt_fild02")), FName(TEXT("prt_fild03")),
		  FName(TEXT("prt_fild04")), FName(TEXT("prt_fild05")), FName(TEXT("izlude")) }
	);

	// === Prontera Fields (prt_fild01 - prt_fild08) ===
	RegisterSimpleMap(
		FName(TEXT("prt_fild01")),
		TEXT("Prontera Field 1"),
		false, false, false, 1, 15,
		{ FName(TEXT("prontera")), FName(TEXT("prt_fild02")) }
	);

	RegisterSimpleMap(
		FName(TEXT("prt_fild02")),
		TEXT("Prontera Field 2"),
		false, false, false, 10, 25,
		{ FName(TEXT("prt_fild01")), FName(TEXT("prt_fild03")), FName(TEXT("prontera")) }
	);

	RegisterSimpleMap(
		FName(TEXT("prt_fild03")),
		TEXT("Prontera Field 3"),
		false, false, false, 15, 30,
		{ FName(TEXT("prt_fild02")), FName(TEXT("prt_fild04")) }
	);

	RegisterSimpleMap(
		FName(TEXT("prt_fild04")),
		TEXT("Prontera Field 4"),
		false, false, false, 20, 40,
		{ FName(TEXT("prt_fild03")), FName(TEXT("prt_fild05")) }
	);

	RegisterSimpleMap(
		FName(TEXT("prt_fild05")),
		TEXT("Prontera Field 5"),
		false, false, false, 25, 45,
		{ FName(TEXT("prt_fild04")), FName(TEXT("prt_fild06")), FName(TEXT("prontera")) }
	);

	RegisterSimpleMap(
		FName(TEXT("prt_fild06")),
		TEXT("Prontera Field 6"),
		false, false, false, 30, 50,
		{ FName(TEXT("prt_fild05")), FName(TEXT("prt_fild07")) }
	);

	RegisterSimpleMap(
		FName(TEXT("prt_fild07")),
		TEXT("Prontera Field 7"),
		false, false, false, 35, 55,
		{ FName(TEXT("prt_fild06")), FName(TEXT("prt_fild08")) }
	);

	RegisterSimpleMap(
		FName(TEXT("prt_fild08")),
		TEXT("Prontera Field 8"),
		false, false, false, 40, 60,
		{ FName(TEXT("prt_fild07")) }
	);

	// === Prontera Culvert Dungeon (prt_sewb1 - prt_sewb4) ===
	RegisterSimpleMap(
		FName(TEXT("prt_sewb1")),
		TEXT("Prontera Culvert 1F"),
		false, true, false, 15, 30,
		{ FName(TEXT("prontera")), FName(TEXT("prt_sewb2")) }
	);

	RegisterSimpleMap(
		FName(TEXT("prt_sewb2")),
		TEXT("Prontera Culvert 2F"),
		false, true, false, 25, 45,
		{ FName(TEXT("prt_sewb1")), FName(TEXT("prt_sewb3")) }
	);

	RegisterSimpleMap(
		FName(TEXT("prt_sewb3")),
		TEXT("Prontera Culvert 3F"),
		false, true, false, 35, 60,
		{ FName(TEXT("prt_sewb2")), FName(TEXT("prt_sewb4")) }
	);

	RegisterSimpleMap(
		FName(TEXT("prt_sewb4")),
		TEXT("Prontera Culvert 4F"),
		false, true, false, 50, 75,
		{ FName(TEXT("prt_sewb3")) }
	);

	// === Izlude Satellite Town ===
	RegisterSimpleMap(
		FName(TEXT("izlude")),
		TEXT("Izlude"),
		true, false, false, 1, 99,
		{ FName(TEXT("prontera")), FName(TEXT("iz_dun01")) }
	);

	// === Byalan Dungeon (iz_dun01 - iz_dun05) ===
	RegisterSimpleMap(
		FName(TEXT("iz_dun01")),
		TEXT("Byalan Dungeon 1F"),
		false, true, false, 20, 35,
		{ FName(TEXT("izlude")), FName(TEXT("iz_dun02")) }
	);

	RegisterSimpleMap(
		FName(TEXT("iz_dun02")),
		TEXT("Byalan Dungeon 2F"),
		false, true, false, 30, 50,
		{ FName(TEXT("iz_dun01")), FName(TEXT("iz_dun03")) }
	);

	RegisterSimpleMap(
		FName(TEXT("iz_dun03")),
		TEXT("Byalan Dungeon 3F"),
		false, true, false, 40, 65,
		{ FName(TEXT("iz_dun02")), FName(TEXT("iz_dun04")) }
	);

	RegisterSimpleMap(
		FName(TEXT("iz_dun04")),
		TEXT("Byalan Dungeon 4F"),
		false, true, false, 55, 80,
		{ FName(TEXT("iz_dun03")), FName(TEXT("iz_dun05")) }
	);

	RegisterSimpleMap(
		FName(TEXT("iz_dun05")),
		TEXT("Byalan Dungeon 5F"),
		false, true, false, 70, 99,
		{ FName(TEXT("iz_dun04")) }
	);

	UE_LOG(LogTemp, Log, TEXT("ROMapManager: Initialized Prontera region with %d maps"), MapDatabase.Num());
}
