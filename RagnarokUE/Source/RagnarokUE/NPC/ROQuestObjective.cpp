// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROQuestObjective.h"

UROQuestObjective::UROQuestObjective()
{
	CurrentProgress = 0;
	ObjectiveIndex = 0;
	bInitialized = false;
}

void UROQuestObjective::Initialize(const FROQuestObjectiveData& InObjectiveData, int32 InObjectiveIndex)
{
	ObjectiveData = InObjectiveData;
	ObjectiveIndex = InObjectiveIndex;
	CurrentProgress = 0;
	bInitialized = true;
}

void UROQuestObjective::OnMonsterKilled(int32 MonsterID)
{
	if (!bInitialized || ObjectiveData.ObjectiveType != EROQuestObjectiveType::KillMonster)
	{
		return;
	}

	if (MonsterID == ObjectiveData.TargetID && CurrentProgress < ObjectiveData.RequiredAmount)
	{
		CurrentProgress++;
		UE_LOG(LogTemp, Verbose, TEXT("QuestObjective[%d]: Monster %d killed. Progress: %d/%d"),
			ObjectiveIndex, MonsterID, CurrentProgress, ObjectiveData.RequiredAmount);
	}
}

void UROQuestObjective::OnItemCollected(int32 ItemID, int32 CurrentCount)
{
	if (!bInitialized || ObjectiveData.ObjectiveType != EROQuestObjectiveType::CollectItem)
	{
		return;
	}

	if (ItemID == ObjectiveData.TargetID)
	{
		// For collection objectives, track the total count the player has
		CurrentProgress = FMath::Min(CurrentCount, ObjectiveData.RequiredAmount);
		UE_LOG(LogTemp, Verbose, TEXT("QuestObjective[%d]: Item %d collected. Progress: %d/%d"),
			ObjectiveIndex, ItemID, CurrentProgress, ObjectiveData.RequiredAmount);
	}
}

void UROQuestObjective::OnNPCInteracted(int32 NPCID)
{
	if (!bInitialized || ObjectiveData.ObjectiveType != EROQuestObjectiveType::TalkToNPC)
	{
		return;
	}

	// Only allow progress if not yet complete (prevents over-counting)
	if (NPCID == ObjectiveData.TargetID && CurrentProgress < ObjectiveData.RequiredAmount)
	{
		// Ordering enforcement: check bCanAdvance flag set by quest manager
		// If this is not the first objective, the quest manager will only
		// call OnNPCInteracted if prior objectives are complete
		CurrentProgress++;
		UE_LOG(LogTemp, Verbose, TEXT("QuestObjective[%d]: NPC %d talked to. Progress: %d/%d"),
			ObjectiveIndex, NPCID, CurrentProgress, ObjectiveData.RequiredAmount);
	}
}

void UROQuestObjective::OnLocationReached(FVector PlayerLocation)
{
	if (!bInitialized || ObjectiveData.ObjectiveType != EROQuestObjectiveType::ReachLocation)
	{
		return;
	}

	if (CurrentProgress >= ObjectiveData.RequiredAmount)
	{
		return;
	}

	const float DistSq = FVector::DistSquared(PlayerLocation, ObjectiveData.TargetLocation);
	const float RadiusSq = ObjectiveData.LocationRadius * ObjectiveData.LocationRadius;

	if (DistSq <= RadiusSq)
	{
		CurrentProgress++;
		UE_LOG(LogTemp, Verbose, TEXT("QuestObjective[%d]: Location reached. Progress: %d/%d"),
			ObjectiveIndex, CurrentProgress, ObjectiveData.RequiredAmount);
	}
}

void UROQuestObjective::OnSkillUsed(int32 SkillID)
{
	if (!bInitialized || ObjectiveData.ObjectiveType != EROQuestObjectiveType::UseSkill)
	{
		return;
	}

	if (SkillID == ObjectiveData.TargetID && CurrentProgress < ObjectiveData.RequiredAmount)
	{
		CurrentProgress++;
		UE_LOG(LogTemp, Verbose, TEXT("QuestObjective[%d]: Skill %d used. Progress: %d/%d"),
			ObjectiveIndex, SkillID, CurrentProgress, ObjectiveData.RequiredAmount);
	}
}

bool UROQuestObjective::CheckCompletion() const
{
	if (!bInitialized)
	{
		return false;
	}
	return CurrentProgress >= ObjectiveData.RequiredAmount;
}

FText UROQuestObjective::GetProgressText() const
{
	if (!bInitialized)
	{
		return FText::FromString(TEXT("Not initialized"));
	}

	// If the objective has a description, use it with progress numbers
	if (!ObjectiveData.ObjectiveDescription.IsEmpty())
	{
		return FText::Format(
			NSLOCTEXT("ROQuest", "ObjectiveProgressFormat", "{0} ({1}/{2})"),
			ObjectiveData.ObjectiveDescription,
			FText::AsNumber(CurrentProgress),
			FText::AsNumber(ObjectiveData.RequiredAmount));
	}

	// Fallback: generate based on type
	FString TypeString;
	switch (ObjectiveData.ObjectiveType)
	{
	case EROQuestObjectiveType::KillMonster:
		TypeString = FString::Printf(TEXT("Kill target %d: %d/%d"),
			ObjectiveData.TargetID, CurrentProgress, ObjectiveData.RequiredAmount);
		break;
	case EROQuestObjectiveType::CollectItem:
		TypeString = FString::Printf(TEXT("Collect item %d: %d/%d"),
			ObjectiveData.TargetID, CurrentProgress, ObjectiveData.RequiredAmount);
		break;
	case EROQuestObjectiveType::TalkToNPC:
		TypeString = FString::Printf(TEXT("Talk to NPC %d: %d/%d"),
			ObjectiveData.TargetID, CurrentProgress, ObjectiveData.RequiredAmount);
		break;
	case EROQuestObjectiveType::ReachLocation:
		TypeString = FString::Printf(TEXT("Reach location: %d/%d"),
			CurrentProgress, ObjectiveData.RequiredAmount);
		break;
	case EROQuestObjectiveType::UseSkill:
		TypeString = FString::Printf(TEXT("Use skill %d: %d/%d"),
			ObjectiveData.TargetID, CurrentProgress, ObjectiveData.RequiredAmount);
		break;
	}

	return FText::FromString(TypeString);
}

void UROQuestObjective::SetProgress(int32 NewProgress)
{
	CurrentProgress = FMath::Clamp(NewProgress, 0, ObjectiveData.RequiredAmount);
}
