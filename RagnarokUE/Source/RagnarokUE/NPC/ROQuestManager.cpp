// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROQuestManager.h"
#include "ROQuestObjective.h"
#include "RagnarokUE/Character/ROCharacterBase.h"
#include "RagnarokUE/Character/ROLevelingComponent.h"
#include "RagnarokUE/Character/ROJobComponent.h"
#include "RagnarokUE/Items/ROInventoryComponent.h"

void UROQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisterBuiltInQuests();
}

void UROQuestManager::Deinitialize()
{
	QuestDatabase.Empty();
	PlayerActiveQuests.Empty();
	PlayerCompletedQuests.Empty();
	PlayerObjectiveTrackers.Empty();
	Super::Deinitialize();
}

void UROQuestManager::RegisterQuest(const FROQuestDefinition& QuestDef)
{
	QuestDatabase.Add(QuestDef.QuestID, QuestDef);
}

const FROQuestDefinition* UROQuestManager::GetQuestDefinition(int32 QuestID) const
{
	return QuestDatabase.Find(QuestID);
}

uint32 UROQuestManager::GetPlayerKey(AROCharacterBase* Player) const
{
	if (!Player)
	{
		return 0;
	}
	return Player->GetUniqueID();
}

bool UROQuestManager::MeetsPrerequisites(int32 QuestID, AROCharacterBase* Player) const
{
	const FROQuestDefinition* QuestDef = GetQuestDefinition(QuestID);
	if (!QuestDef || !Player)
	{
		return false;
	}

	// Check base level requirement
	if (UROLevelingComponent* LevelComp = Player->FindComponentByClass<UROLevelingComponent>())
	{
		if (LevelComp->BaseLevel < QuestDef->RequiredLevel)
		{
			return false;
		}

		// Check job level requirement
		if (QuestDef->RequiredJobLevel > 0 && LevelComp->JobLevel < QuestDef->RequiredJobLevel)
		{
			return false;
		}
	}

	// Check job class requirement
	if (QuestDef->RequiredJobClass != EROJobClass::Novice)
	{
		if (UROJobComponent* JobComp = Player->FindComponentByClass<UROJobComponent>())
		{
			if (JobComp->CurrentJobClass != QuestDef->RequiredJobClass)
			{
				return false;
			}
		}
	}

	// Check prerequisite quests
	const uint32 PlayerKey = GetPlayerKey(Player);
	const TArray<int32>* CompletedQuests = PlayerCompletedQuests.Find(PlayerKey);

	for (int32 PrereqID : QuestDef->PrerequisiteQuests)
	{
		if (!CompletedQuests || !CompletedQuests->Contains(PrereqID))
		{
			return false;
		}
	}

	return true;
}

bool UROQuestManager::AcceptQuest(int32 QuestID, AROCharacterBase* Player)
{
	if (!Player)
	{
		return false;
	}

	const FROQuestDefinition* QuestDef = GetQuestDefinition(QuestID);
	if (!QuestDef)
	{
		UE_LOG(LogTemp, Warning, TEXT("ROQuestManager: Quest ID %d not found in database."), QuestID);
		return false;
	}

	const uint32 PlayerKey = GetPlayerKey(Player);

	// Check if player already has this quest active
	if (HasActiveQuest(QuestID, Player))
	{
		UE_LOG(LogTemp, Warning, TEXT("ROQuestManager: Player already has quest %d active."), QuestID);
		return false;
	}

	// Check if player already completed this quest
	if (HasCompletedQuest(QuestID, Player))
	{
		UE_LOG(LogTemp, Warning, TEXT("ROQuestManager: Player already completed quest %d."), QuestID);
		return false;
	}

	// Check prerequisites
	if (!MeetsPrerequisites(QuestID, Player))
	{
		UE_LOG(LogTemp, Warning, TEXT("ROQuestManager: Player does not meet prerequisites for quest %d."), QuestID);
		return false;
	}

	// Create active quest entry
	FROActiveQuest ActiveQuest(QuestID, QuestDef->Objectives.Num());
	PlayerActiveQuests.FindOrAdd(PlayerKey).Add(QuestID, ActiveQuest);

	// Create objective trackers
	CreateObjectiveTrackers(PlayerKey, QuestID, *QuestDef);

	OnQuestAccepted.Broadcast(QuestID, Player);

	UE_LOG(LogTemp, Log, TEXT("ROQuestManager: Player accepted quest '%s' (ID: %d)."),
		*QuestDef->QuestName.ToString(), QuestID);

	return true;
}

void UROQuestManager::CompleteQuest(int32 QuestID, AROCharacterBase* Player)
{
	if (!Player)
	{
		return;
	}

	const uint32 PlayerKey = GetPlayerKey(Player);
	TMap<int32, FROActiveQuest>* ActiveQuests = PlayerActiveQuests.Find(PlayerKey);
	if (!ActiveQuests)
	{
		return;
	}

	FROActiveQuest* ActiveQuest = ActiveQuests->Find(QuestID);
	if (!ActiveQuest)
	{
		UE_LOG(LogTemp, Warning, TEXT("ROQuestManager: Quest %d not active for this player."), QuestID);
		return;
	}

	const FROQuestDefinition* QuestDef = GetQuestDefinition(QuestID);
	if (!QuestDef)
	{
		return;
	}

	// Check if all objectives are complete
	bool bAllComplete = true;
	TMap<int32, TArray<TObjectPtr<UROQuestObjective>>>* QuestTrackers = PlayerObjectiveTrackers.Find(PlayerKey);
	if (QuestTrackers)
	{
		TArray<TObjectPtr<UROQuestObjective>>* Trackers = QuestTrackers->Find(QuestID);
		if (Trackers)
		{
			for (const TObjectPtr<UROQuestObjective>& Tracker : *Trackers)
			{
				if (Tracker && !Tracker->CheckCompletion())
				{
					bAllComplete = false;
					break;
				}
			}
		}
	}

	if (!bAllComplete)
	{
		UE_LOG(LogTemp, Warning, TEXT("ROQuestManager: Not all objectives complete for quest %d."), QuestID);
		return;
	}

	// Mark as complete
	ActiveQuest->bIsComplete = true;

	// Award rewards
	AwardQuestRewards(Player, *QuestDef);

	// Move to completed list
	PlayerCompletedQuests.FindOrAdd(PlayerKey).AddUnique(QuestID);

	// Clean up active quest and trackers
	ActiveQuests->Remove(QuestID);
	if (QuestTrackers)
	{
		QuestTrackers->Remove(QuestID);
	}

	OnQuestCompleted.Broadcast(QuestID, Player);

	UE_LOG(LogTemp, Log, TEXT("ROQuestManager: Player completed quest '%s' (ID: %d)."),
		*QuestDef->QuestName.ToString(), QuestID);
}

void UROQuestManager::AbandonQuest(int32 QuestID, AROCharacterBase* Player)
{
	if (!Player)
	{
		return;
	}

	const uint32 PlayerKey = GetPlayerKey(Player);
	TMap<int32, FROActiveQuest>* ActiveQuests = PlayerActiveQuests.Find(PlayerKey);
	if (!ActiveQuests || !ActiveQuests->Contains(QuestID))
	{
		UE_LOG(LogTemp, Warning, TEXT("ROQuestManager: Quest %d not active for this player."), QuestID);
		return;
	}

	ActiveQuests->Remove(QuestID);

	// Clean up objective trackers
	TMap<int32, TArray<TObjectPtr<UROQuestObjective>>>* QuestTrackers = PlayerObjectiveTrackers.Find(PlayerKey);
	if (QuestTrackers)
	{
		QuestTrackers->Remove(QuestID);
	}

	OnQuestAbandoned.Broadcast(QuestID, Player);

	UE_LOG(LogTemp, Log, TEXT("ROQuestManager: Player abandoned quest %d."), QuestID);
}

bool UROQuestManager::IsQuestComplete(int32 QuestID, AROCharacterBase* Player) const
{
	return HasCompletedQuest(QuestID, Player);
}

bool UROQuestManager::HasActiveQuest(int32 QuestID, AROCharacterBase* Player) const
{
	if (!Player)
	{
		return false;
	}

	const uint32 PlayerKey = GetPlayerKey(Player);
	const TMap<int32, FROActiveQuest>* ActiveQuests = PlayerActiveQuests.Find(PlayerKey);
	return ActiveQuests && ActiveQuests->Contains(QuestID);
}

bool UROQuestManager::HasCompletedQuest(int32 QuestID, AROCharacterBase* Player) const
{
	if (!Player)
	{
		return false;
	}

	const uint32 PlayerKey = GetPlayerKey(Player);
	const TArray<int32>* CompletedQuests = PlayerCompletedQuests.Find(PlayerKey);
	return CompletedQuests && CompletedQuests->Contains(QuestID);
}

FROActiveQuest UROQuestManager::GetQuestProgress(int32 QuestID, AROCharacterBase* Player) const
{
	if (!Player)
	{
		return FROActiveQuest();
	}

	const uint32 PlayerKey = GetPlayerKey(Player);
	const TMap<int32, FROActiveQuest>* ActiveQuests = PlayerActiveQuests.Find(PlayerKey);
	if (ActiveQuests)
	{
		const FROActiveQuest* Quest = ActiveQuests->Find(QuestID);
		if (Quest)
		{
			return *Quest;
		}
	}
	return FROActiveQuest();
}

TArray<FROActiveQuest> UROQuestManager::GetAllActiveQuests(AROCharacterBase* Player) const
{
	TArray<FROActiveQuest> Result;
	if (!Player)
	{
		return Result;
	}

	const uint32 PlayerKey = GetPlayerKey(Player);
	const TMap<int32, FROActiveQuest>* ActiveQuests = PlayerActiveQuests.Find(PlayerKey);
	if (ActiveQuests)
	{
		ActiveQuests->GenerateValueArray(Result);
	}
	return Result;
}

TArray<int32> UROQuestManager::GetAllCompletedQuests(AROCharacterBase* Player) const
{
	if (!Player)
	{
		return TArray<int32>();
	}

	const uint32 PlayerKey = GetPlayerKey(Player);
	const TArray<int32>* CompletedQuests = PlayerCompletedQuests.Find(PlayerKey);
	return CompletedQuests ? *CompletedQuests : TArray<int32>();
}

void UROQuestManager::NotifyMonsterKilled(AROCharacterBase* Player, int32 MonsterID)
{
	if (!Player)
	{
		return;
	}

	const uint32 PlayerKey = GetPlayerKey(Player);
	TMap<int32, TArray<TObjectPtr<UROQuestObjective>>>* QuestTrackers = PlayerObjectiveTrackers.Find(PlayerKey);
	if (!QuestTrackers)
	{
		return;
	}

	for (auto& Pair : *QuestTrackers)
	{
		for (TObjectPtr<UROQuestObjective>& Tracker : Pair.Value)
		{
			if (Tracker)
			{
				int32 OldProgress = Tracker->GetCurrentProgress();
				Tracker->OnMonsterKilled(MonsterID);
				if (Tracker->GetCurrentProgress() != OldProgress)
				{
					OnQuestObjectiveUpdated.Broadcast(Pair.Key, Tracker->GetObjectiveIndex(), Tracker->GetCurrentProgress());
					SyncQuestProgress(PlayerKey, Pair.Key);
				}
			}
		}
	}
}

void UROQuestManager::NotifyItemCollected(AROCharacterBase* Player, int32 ItemID, int32 CurrentCount)
{
	if (!Player)
	{
		return;
	}

	const uint32 PlayerKey = GetPlayerKey(Player);
	TMap<int32, TArray<TObjectPtr<UROQuestObjective>>>* QuestTrackers = PlayerObjectiveTrackers.Find(PlayerKey);
	if (!QuestTrackers)
	{
		return;
	}

	for (auto& Pair : *QuestTrackers)
	{
		for (TObjectPtr<UROQuestObjective>& Tracker : Pair.Value)
		{
			if (Tracker)
			{
				int32 OldProgress = Tracker->GetCurrentProgress();
				Tracker->OnItemCollected(ItemID, CurrentCount);
				if (Tracker->GetCurrentProgress() != OldProgress)
				{
					OnQuestObjectiveUpdated.Broadcast(Pair.Key, Tracker->GetObjectiveIndex(), Tracker->GetCurrentProgress());
					SyncQuestProgress(PlayerKey, Pair.Key);
				}
			}
		}
	}
}

void UROQuestManager::NotifyNPCInteracted(AROCharacterBase* Player, int32 NPCID)
{
	if (!Player)
	{
		return;
	}

	const uint32 PlayerKey = GetPlayerKey(Player);
	TMap<int32, TArray<TObjectPtr<UROQuestObjective>>>* QuestTrackers = PlayerObjectiveTrackers.Find(PlayerKey);
	if (!QuestTrackers)
	{
		return;
	}

	for (auto& Pair : *QuestTrackers)
	{
		for (TObjectPtr<UROQuestObjective>& Tracker : Pair.Value)
		{
			if (Tracker)
			{
				int32 OldProgress = Tracker->GetCurrentProgress();
				Tracker->OnNPCInteracted(NPCID);
				if (Tracker->GetCurrentProgress() != OldProgress)
				{
					OnQuestObjectiveUpdated.Broadcast(Pair.Key, Tracker->GetObjectiveIndex(), Tracker->GetCurrentProgress());
					SyncQuestProgress(PlayerKey, Pair.Key);
				}
			}
		}
	}
}

void UROQuestManager::NotifyLocationReached(AROCharacterBase* Player, FVector Location)
{
	if (!Player)
	{
		return;
	}

	const uint32 PlayerKey = GetPlayerKey(Player);
	TMap<int32, TArray<TObjectPtr<UROQuestObjective>>>* QuestTrackers = PlayerObjectiveTrackers.Find(PlayerKey);
	if (!QuestTrackers)
	{
		return;
	}

	for (auto& Pair : *QuestTrackers)
	{
		for (TObjectPtr<UROQuestObjective>& Tracker : Pair.Value)
		{
			if (Tracker)
			{
				int32 OldProgress = Tracker->GetCurrentProgress();
				Tracker->OnLocationReached(Location);
				if (Tracker->GetCurrentProgress() != OldProgress)
				{
					OnQuestObjectiveUpdated.Broadcast(Pair.Key, Tracker->GetObjectiveIndex(), Tracker->GetCurrentProgress());
					SyncQuestProgress(PlayerKey, Pair.Key);
				}
			}
		}
	}
}

void UROQuestManager::NotifySkillUsed(AROCharacterBase* Player, int32 SkillID)
{
	if (!Player)
	{
		return;
	}

	const uint32 PlayerKey = GetPlayerKey(Player);
	TMap<int32, TArray<TObjectPtr<UROQuestObjective>>>* QuestTrackers = PlayerObjectiveTrackers.Find(PlayerKey);
	if (!QuestTrackers)
	{
		return;
	}

	for (auto& Pair : *QuestTrackers)
	{
		for (TObjectPtr<UROQuestObjective>& Tracker : Pair.Value)
		{
			if (Tracker)
			{
				int32 OldProgress = Tracker->GetCurrentProgress();
				Tracker->OnSkillUsed(SkillID);
				if (Tracker->GetCurrentProgress() != OldProgress)
				{
					OnQuestObjectiveUpdated.Broadcast(Pair.Key, Tracker->GetObjectiveIndex(), Tracker->GetCurrentProgress());
					SyncQuestProgress(PlayerKey, Pair.Key);
				}
			}
		}
	}
}

void UROQuestManager::CreateObjectiveTrackers(uint32 PlayerKey, int32 QuestID, const FROQuestDefinition& QuestDef)
{
	TArray<TObjectPtr<UROQuestObjective>>& Trackers =
		PlayerObjectiveTrackers.FindOrAdd(PlayerKey).FindOrAdd(QuestID);

	Trackers.Empty();

	for (int32 i = 0; i < QuestDef.Objectives.Num(); ++i)
	{
		UROQuestObjective* Tracker = NewObject<UROQuestObjective>(this);
		Tracker->Initialize(QuestDef.Objectives[i], i);
		Trackers.Add(Tracker);
	}
}

void UROQuestManager::AwardQuestRewards(AROCharacterBase* Player, const FROQuestDefinition& QuestDef)
{
	if (!Player)
	{
		return;
	}

	UROInventoryComponent* Inventory = Player->FindComponentByClass<UROInventoryComponent>();
	UROLevelingComponent* Leveling = Player->FindComponentByClass<UROLevelingComponent>();

	for (const FROQuestReward& Reward : QuestDef.Rewards)
	{
		// Award items
		if (Reward.ItemID > 0 && Reward.Amount > 0 && Inventory)
		{
			Inventory->Internal_AddItem(Reward.ItemID, Reward.Amount);
		}

		// Award experience
		if (Leveling)
		{
			if (Reward.BaseExp > 0)
			{
				Leveling->AddBaseExp(Reward.BaseExp);
			}
			if (Reward.JobExp > 0)
			{
				Leveling->AddJobExp(Reward.JobExp);
			}
		}

		// Award Zeny
		if (Reward.Zeny > 0 && Inventory)
		{
			Inventory->AddZeny(Reward.Zeny);
		}
	}
}

void UROQuestManager::SyncQuestProgress(uint32 PlayerKey, int32 QuestID)
{
	TMap<int32, FROActiveQuest>* ActiveQuests = PlayerActiveQuests.Find(PlayerKey);
	if (!ActiveQuests)
	{
		return;
	}

	FROActiveQuest* ActiveQuest = ActiveQuests->Find(QuestID);
	if (!ActiveQuest)
	{
		return;
	}

	TMap<int32, TArray<TObjectPtr<UROQuestObjective>>>* QuestTrackers = PlayerObjectiveTrackers.Find(PlayerKey);
	if (!QuestTrackers)
	{
		return;
	}

	TArray<TObjectPtr<UROQuestObjective>>* Trackers = QuestTrackers->Find(QuestID);
	if (!Trackers)
	{
		return;
	}

	bool bAllComplete = true;
	for (int32 i = 0; i < Trackers->Num(); ++i)
	{
		if ((*Trackers)[i])
		{
			if (ActiveQuest->ObjectiveProgress.IsValidIndex(i))
			{
				ActiveQuest->ObjectiveProgress[i] = (*Trackers)[i]->GetCurrentProgress();
			}
			if (!(*Trackers)[i]->CheckCompletion())
			{
				bAllComplete = false;
			}
		}
	}

	ActiveQuest->bIsComplete = bAllComplete;
}

void UROQuestManager::RegisterBuiltInQuests()
{
	// ---- Quest 1000: Novice Training Quest (Tutorial) ----
	{
		FROQuestDefinition Quest;
		Quest.QuestID = QUEST_NOVICE_TRAINING;
		Quest.QuestName = FText::FromString(TEXT("Novice Training"));
		Quest.Description = FText::FromString(TEXT("Welcome to the world of Rune-Midgarts! Complete basic training to prepare for your adventures. Talk to the Training Instructor, defeat some Porings, and collect a few items."));
		Quest.RequiredLevel = 1;

		FROQuestObjectiveData Obj1;
		Obj1.ObjectiveType = EROQuestObjectiveType::TalkToNPC;
		Obj1.TargetID = 100; // Training Instructor NPC ID
		Obj1.RequiredAmount = 1;
		Obj1.ObjectiveDescription = FText::FromString(TEXT("Talk to the Training Instructor"));
		Quest.Objectives.Add(Obj1);

		FROQuestObjectiveData Obj2;
		Obj2.ObjectiveType = EROQuestObjectiveType::KillMonster;
		Obj2.TargetID = 1002; // Poring Monster ID
		Obj2.RequiredAmount = 5;
		Obj2.ObjectiveDescription = FText::FromString(TEXT("Defeat 5 Porings"));
		Quest.Objectives.Add(Obj2);

		FROQuestObjectiveData Obj3;
		Obj3.ObjectiveType = EROQuestObjectiveType::CollectItem;
		Obj3.TargetID = 909; // Jellopy Item ID
		Obj3.RequiredAmount = 3;
		Obj3.ObjectiveDescription = FText::FromString(TEXT("Collect 3 Jellopies"));
		Quest.Objectives.Add(Obj3);

		FROQuestReward Reward;
		Reward.BaseExp = 50;
		Reward.JobExp = 50;
		Reward.Zeny = 500;
		Reward.ItemID = 501; // Red Potion
		Reward.Amount = 10;
		Quest.Rewards.Add(Reward);

		RegisterQuest(Quest);
	}

	// ---- Quest 1001: Swordsman Job Change Quest ----
	{
		FROQuestDefinition Quest;
		Quest.QuestID = QUEST_SWORDSMAN_CHANGE;
		Quest.QuestName = FText::FromString(TEXT("Path of the Swordsman"));
		Quest.Description = FText::FromString(TEXT("Prove your strength to the Swordsman Guild in Izlude. Defeat monsters and retrieve the Swordsman's Badge to earn the right to become a Swordsman."));
		Quest.RequiredLevel = 1;
		Quest.RequiredJobClass = EROJobClass::Novice;
		Quest.RequiredJobLevel = 10;
		Quest.PrerequisiteQuests.Add(QUEST_NOVICE_TRAINING);

		FROQuestObjectiveData Obj1;
		Obj1.ObjectiveType = EROQuestObjectiveType::TalkToNPC;
		Obj1.TargetID = 200; // Swordsman Guild NPC
		Obj1.RequiredAmount = 1;
		Obj1.ObjectiveDescription = FText::FromString(TEXT("Speak with the Swordsman Instructor in Izlude"));
		Quest.Objectives.Add(Obj1);

		FROQuestObjectiveData Obj2;
		Obj2.ObjectiveType = EROQuestObjectiveType::KillMonster;
		Obj2.TargetID = 1014; // Chonchon
		Obj2.RequiredAmount = 5;
		Obj2.ObjectiveDescription = FText::FromString(TEXT("Defeat 5 Chonchons"));
		Quest.Objectives.Add(Obj2);

		FROQuestObjectiveData Obj3;
		Obj3.ObjectiveType = EROQuestObjectiveType::CollectItem;
		Obj3.TargetID = 7001; // Swordsman Badge
		Obj3.RequiredAmount = 1;
		Obj3.ObjectiveDescription = FText::FromString(TEXT("Collect the Swordsman's Badge"));
		Quest.Objectives.Add(Obj3);

		FROQuestReward Reward;
		Reward.BaseExp = 0;
		Reward.JobExp = 0;
		Reward.Zeny = 0;
		Reward.ItemID = 1101; // Starter Sword
		Reward.Amount = 1;
		Quest.Rewards.Add(Reward);

		RegisterQuest(Quest);
	}

	// ---- Quest 1002: Mage Job Change Quest ----
	{
		FROQuestDefinition Quest;
		Quest.QuestID = QUEST_MAGE_CHANGE;
		Quest.QuestName = FText::FromString(TEXT("The Way of Magic"));
		Quest.Description = FText::FromString(TEXT("Seek the Mage Guild in Geffen. Answer the Sage's questions and demonstrate your potential for the arcane arts."));
		Quest.RequiredLevel = 1;
		Quest.RequiredJobClass = EROJobClass::Novice;
		Quest.RequiredJobLevel = 10;
		Quest.PrerequisiteQuests.Add(QUEST_NOVICE_TRAINING);

		FROQuestObjectiveData Obj1;
		Obj1.ObjectiveType = EROQuestObjectiveType::TalkToNPC;
		Obj1.TargetID = 201; // Mage Guild NPC
		Obj1.RequiredAmount = 1;
		Obj1.ObjectiveDescription = FText::FromString(TEXT("Speak with the Mage Instructor in Geffen"));
		Quest.Objectives.Add(Obj1);

		FROQuestObjectiveData Obj2;
		Obj2.ObjectiveType = EROQuestObjectiveType::CollectItem;
		Obj2.TargetID = 7002; // Mage Test Reagent
		Obj2.RequiredAmount = 2;
		Obj2.ObjectiveDescription = FText::FromString(TEXT("Collect 2 Mage Test Reagents"));
		Quest.Objectives.Add(Obj2);

		FROQuestObjectiveData Obj3;
		Obj3.ObjectiveType = EROQuestObjectiveType::TalkToNPC;
		Obj3.TargetID = 201; // Return to Mage Guild NPC
		Obj3.RequiredAmount = 2; // Talk twice (initial + return)
		Obj3.ObjectiveDescription = FText::FromString(TEXT("Return to the Mage Instructor"));
		Quest.Objectives.Add(Obj3);

		FROQuestReward Reward;
		Reward.ItemID = 1601; // Starter Rod
		Reward.Amount = 1;
		Quest.Rewards.Add(Reward);

		RegisterQuest(Quest);
	}

	// ---- Quest 1003: Archer Job Change Quest ----
	{
		FROQuestDefinition Quest;
		Quest.QuestID = QUEST_ARCHER_CHANGE;
		Quest.QuestName = FText::FromString(TEXT("The Archer's Calling"));
		Quest.Description = FText::FromString(TEXT("Travel to the Archer Village in Payon. Pass the marksmanship test by demonstrating your aim and collecting trunks from Willows."));
		Quest.RequiredLevel = 1;
		Quest.RequiredJobClass = EROJobClass::Novice;
		Quest.RequiredJobLevel = 10;
		Quest.PrerequisiteQuests.Add(QUEST_NOVICE_TRAINING);

		FROQuestObjectiveData Obj1;
		Obj1.ObjectiveType = EROQuestObjectiveType::TalkToNPC;
		Obj1.TargetID = 202; // Archer Guild NPC
		Obj1.RequiredAmount = 1;
		Obj1.ObjectiveDescription = FText::FromString(TEXT("Speak with the Archer Instructor in Payon"));
		Quest.Objectives.Add(Obj1);

		FROQuestObjectiveData Obj2;
		Obj2.ObjectiveType = EROQuestObjectiveType::CollectItem;
		Obj2.TargetID = 1019; // Trunk (from Willows)
		Obj2.RequiredAmount = 10;
		Obj2.ObjectiveDescription = FText::FromString(TEXT("Collect 10 Trunks from Willows"));
		Quest.Objectives.Add(Obj2);

		FROQuestReward Reward;
		Reward.ItemID = 1701; // Starter Bow
		Reward.Amount = 1;
		Quest.Rewards.Add(Reward);

		FROQuestReward ArrowReward;
		ArrowReward.ItemID = 1750; // Arrows
		ArrowReward.Amount = 200;
		Quest.Rewards.Add(ArrowReward);

		RegisterQuest(Quest);
	}

	// ---- Quest 1004: Thief Job Change Quest ----
	{
		FROQuestDefinition Quest;
		Quest.QuestID = QUEST_THIEF_CHANGE;
		Quest.QuestName = FText::FromString(TEXT("Shadows of the Thief Guild"));
		Quest.Description = FText::FromString(TEXT("Find the hidden Thief Guild in the Morocc Pyramid area. Prove your stealth and cunning to earn the title of Thief."));
		Quest.RequiredLevel = 1;
		Quest.RequiredJobClass = EROJobClass::Novice;
		Quest.RequiredJobLevel = 10;
		Quest.PrerequisiteQuests.Add(QUEST_NOVICE_TRAINING);

		FROQuestObjectiveData Obj1;
		Obj1.ObjectiveType = EROQuestObjectiveType::TalkToNPC;
		Obj1.TargetID = 203; // Thief Guild NPC
		Obj1.RequiredAmount = 1;
		Obj1.ObjectiveDescription = FText::FromString(TEXT("Find and speak with the Thief Guild Leader in Morocc"));
		Quest.Objectives.Add(Obj1);

		FROQuestObjectiveData Obj2;
		Obj2.ObjectiveType = EROQuestObjectiveType::CollectItem;
		Obj2.TargetID = 7003; // Thief Guild Token
		Obj2.RequiredAmount = 1;
		Obj2.ObjectiveDescription = FText::FromString(TEXT("Steal the Mushroom from the Mushroom Farm"));
		Quest.Objectives.Add(Obj2);

		FROQuestObjectiveData Obj3;
		Obj3.ObjectiveType = EROQuestObjectiveType::KillMonster;
		Obj3.TargetID = 1031; // Familiars
		Obj3.RequiredAmount = 10;
		Obj3.ObjectiveDescription = FText::FromString(TEXT("Defeat 10 Familiars"));
		Quest.Objectives.Add(Obj3);

		FROQuestReward Reward;
		Reward.ItemID = 1201; // Starter Knife
		Reward.Amount = 1;
		Quest.Rewards.Add(Reward);

		RegisterQuest(Quest);
	}

	// ---- Quest 1005: Merchant Job Change Quest ----
	{
		FROQuestDefinition Quest;
		Quest.QuestID = QUEST_MERCHANT_CHANGE;
		Quest.QuestName = FText::FromString(TEXT("The Merchant's Enterprise"));
		Quest.Description = FText::FromString(TEXT("Visit the Merchant Guild in Alberta. Complete a delivery mission to prove your worth as a reliable trader and earn the title of Merchant."));
		Quest.RequiredLevel = 1;
		Quest.RequiredJobClass = EROJobClass::Novice;
		Quest.RequiredJobLevel = 10;
		Quest.PrerequisiteQuests.Add(QUEST_NOVICE_TRAINING);

		FROQuestObjectiveData Obj1;
		Obj1.ObjectiveType = EROQuestObjectiveType::TalkToNPC;
		Obj1.TargetID = 204; // Merchant Guild NPC
		Obj1.RequiredAmount = 1;
		Obj1.ObjectiveDescription = FText::FromString(TEXT("Speak with the Merchant Guild Master in Alberta"));
		Quest.Objectives.Add(Obj1);

		FROQuestObjectiveData Obj2;
		Obj2.ObjectiveType = EROQuestObjectiveType::CollectItem;
		Obj2.TargetID = 7004; // Delivery Package
		Obj2.RequiredAmount = 3;
		Obj2.ObjectiveDescription = FText::FromString(TEXT("Collect 3 Delivery Packages"));
		Quest.Objectives.Add(Obj2);

		FROQuestObjectiveData Obj3;
		Obj3.ObjectiveType = EROQuestObjectiveType::TalkToNPC;
		Obj3.TargetID = 205; // Delivery Target NPC
		Obj3.RequiredAmount = 1;
		Obj3.ObjectiveDescription = FText::FromString(TEXT("Deliver the packages to the Customer in Prontera"));
		Quest.Objectives.Add(Obj3);

		FROQuestReward Reward;
		Reward.Zeny = 3000;
		Reward.ItemID = 1301; // Starter Axe
		Reward.Amount = 1;
		Quest.Rewards.Add(Reward);

		RegisterQuest(Quest);
	}

	// ---- Quest 1006: Acolyte Job Change Quest ----
	{
		FROQuestDefinition Quest;
		Quest.QuestID = QUEST_ACOLYTE_CHANGE;
		Quest.QuestName = FText::FromString(TEXT("The Path of Devotion"));
		Quest.Description = FText::FromString(TEXT("Journey to the Prontera Church and prove your devotion. Defeat undead monsters and heal the wounded to earn the title of Acolyte."));
		Quest.RequiredLevel = 1;
		Quest.RequiredJobClass = EROJobClass::Novice;
		Quest.RequiredJobLevel = 10;
		Quest.PrerequisiteQuests.Add(QUEST_NOVICE_TRAINING);

		FROQuestObjectiveData Obj1;
		Obj1.ObjectiveType = EROQuestObjectiveType::TalkToNPC;
		Obj1.TargetID = 206; // Prontera Church NPC
		Obj1.RequiredAmount = 1;
		Obj1.ObjectiveDescription = FText::FromString(TEXT("Speak with the High Priest in Prontera Church"));
		Quest.Objectives.Add(Obj1);

		FROQuestObjectiveData Obj2;
		Obj2.ObjectiveType = EROQuestObjectiveType::KillMonster;
		Obj2.TargetID = 1028; // Zombie
		Obj2.RequiredAmount = 10;
		Obj2.ObjectiveDescription = FText::FromString(TEXT("Purify 10 Zombies"));
		Quest.Objectives.Add(Obj2);

		FROQuestObjectiveData Obj3;
		Obj3.ObjectiveType = EROQuestObjectiveType::TalkToNPC;
		Obj3.TargetID = 207; // Wounded NPC
		Obj3.RequiredAmount = 3;
		Obj3.ObjectiveDescription = FText::FromString(TEXT("Heal 3 Wounded Travelers"));
		Quest.Objectives.Add(Obj3);

		FROQuestReward Reward;
		Reward.ItemID = 1501; // Starter Mace
		Reward.Amount = 1;
		Quest.Rewards.Add(Reward);

		FROQuestReward PotionReward;
		PotionReward.ItemID = 505; // Blue Potion
		PotionReward.Amount = 5;
		Quest.Rewards.Add(PotionReward);

		RegisterQuest(Quest);
	}
}
