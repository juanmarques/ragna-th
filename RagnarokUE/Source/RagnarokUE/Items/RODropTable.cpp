// Copyright Ragna-TH Project. All Rights Reserved.

#include "RODropTable.h"

URODropTable::URODropTable()
{
}

TArray<FRODropInfo> URODropTable::GetDropsForMonster(int32 MonsterID) const
{
	const FRODropTableEntry* Found = MonsterDropTables.Find(MonsterID);
	if (Found)
	{
		return Found->Drops;
	}
	return TArray<FRODropInfo>();
}

TArray<int32> URODropTable::RollDrops(int32 MonsterID, float DropRateModifier) const
{
	TArray<int32> DroppedItems;

	const FRODropTableEntry* Found = MonsterDropTables.Find(MonsterID);
	if (!Found)
	{
		return DroppedItems;
	}

	for (const FRODropInfo& DropEntry : Found->Drops)
	{
		if (DropEntry.ItemID <= 0)
		{
			continue;
		}

		// Apply drop rate modifier, capped at 100%
		float EffectiveRate = FMath::Min(DropEntry.DropRate * DropRateModifier, 100.0f);

		// Roll against the drop rate
		float Roll = FMath::FRandRange(0.0f, 100.0f);
		if (Roll <= EffectiveRate)
		{
			DroppedItems.Add(DropEntry.ItemID);
		}
	}

	return DroppedItems;
}

void URODropTable::RegisterDropTable(int32 MonsterID, const TArray<FRODropInfo>& Drops)
{
	FRODropTableEntry Entry;
	Entry.Drops = Drops;
	MonsterDropTables.Add(MonsterID, Entry);
}

void URODropTable::LoadFromMonsterData(const TArray<FROMonsterData>& MonsterDataArray)
{
	for (const FROMonsterData& MonsterData : MonsterDataArray)
	{
		if (MonsterData.MonsterID > 0 && MonsterData.DropTable.Num() > 0)
		{
			RegisterDropTable(MonsterData.MonsterID, MonsterData.DropTable);
		}
	}
}

bool URODropTable::HasDrops(int32 MonsterID) const
{
	const FRODropTableEntry* Found = MonsterDropTables.Find(MonsterID);
	return Found && Found->Drops.Num() > 0;
}
