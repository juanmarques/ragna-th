// Copyright Ragna-TH Project. All Rights Reserved.

#include "RODialogueComponent.h"
#include "RagnarokUE/Character/ROCharacterBase.h"
#include "RagnarokUE/NPC/ROQuestManager.h"
#include "Engine/GameInstance.h"

URODialogueComponent::URODialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentNodeIndex = 0;
	bIsDialogueActive = false;
	CurrentPlayer = nullptr;
}

bool URODialogueComponent::HasDialogue() const
{
	return DialogueTree.Num() > 0;
}

FRODialogueNode URODialogueComponent::StartDialogue(AROCharacterBase* Player)
{
	if (!Player || DialogueTree.Num() == 0)
	{
		return FRODialogueNode();
	}

	// Create or reset per-player session
	FDialogueSession& Session = ActiveSessions.FindOrAdd(Player);
	Session.CurrentNodeIndex = 0;
	Session.bIsActive = true;

	// Update legacy state for backward-compatible API
	CurrentPlayer = Player;
	CurrentNodeIndex = 0;
	bIsDialogueActive = true;

	OnDialogueStarted.Broadcast(Player);
	OnDialogueNodeChanged.Broadcast(DialogueTree[0], Player);

	return DialogueTree[0];
}

FRODialogueNode URODialogueComponent::SelectChoice(int32 ChoiceIndex)
{
	// Use per-player session if available, fall back to legacy single-player state
	AROCharacterBase* Player = CurrentPlayer;

	// Find the active session for this player
	FDialogueSession* Session = Player ? ActiveSessions.Find(Player) : nullptr;

	int32 ActiveNodeIndex = Session ? Session->CurrentNodeIndex : CurrentNodeIndex;
	bool bActive = Session ? Session->bIsActive : bIsDialogueActive;

	if (!bActive || !DialogueTree.IsValidIndex(ActiveNodeIndex))
	{
		return FRODialogueNode();
	}

	const FRODialogueNode& CurrentNode = DialogueTree[ActiveNodeIndex];

	if (!CurrentNode.Choices.IsValidIndex(ChoiceIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("RODialogueComponent: Invalid choice index %d for node %d."),
			ChoiceIndex, CurrentNode.NodeID);
		return CurrentNode;
	}

	const FRODialogueChoice& Choice = CurrentNode.Choices[ChoiceIndex];

	// Handle quest trigger from this choice
	if (Choice.bStartsQuest && Choice.QuestID != 0 && Player)
	{
		UGameInstance* GI = GetWorld() ? Cast<UGameInstance>(GetWorld()->GetGameInstance()) : nullptr;
		if (GI)
		{
			UROQuestManager* QuestMgr = GI->GetSubsystem<UROQuestManager>();
			if (QuestMgr)
			{
				QuestMgr->AcceptQuest(Choice.QuestID, Player);
			}
		}
	}

	// Check if the choice ends dialogue
	if (Choice.bEndsDialogue || Choice.NextNodeID < 0)
	{
		EndDialogue();
		return FRODialogueNode();
	}

	// Find the next node by ID
	const FRODialogueNode* NextNode = FindNodeByID(Choice.NextNodeID);
	if (!NextNode)
	{
		UE_LOG(LogTemp, Warning, TEXT("RODialogueComponent: Could not find node ID %d. Ending dialogue."),
			Choice.NextNodeID);
		EndDialogue();
		return FRODialogueNode();
	}

	// Update current node index
	int32 NewNodeIndex = ActiveNodeIndex;
	for (int32 i = 0; i < DialogueTree.Num(); ++i)
	{
		if (DialogueTree[i].NodeID == Choice.NextNodeID)
		{
			NewNodeIndex = i;
			break;
		}
	}

	// Update per-player session
	if (Session)
	{
		Session->CurrentNodeIndex = NewNodeIndex;
	}

	// Update legacy state
	CurrentNodeIndex = NewNodeIndex;

	OnDialogueNodeChanged.Broadcast(*NextNode, Player);

	return *NextNode;
}

FRODialogueNode URODialogueComponent::GetCurrentNode() const
{
	if (bIsDialogueActive && DialogueTree.IsValidIndex(CurrentNodeIndex))
	{
		return DialogueTree[CurrentNodeIndex];
	}
	return FRODialogueNode();
}

void URODialogueComponent::EndDialogue()
{
	if (bIsDialogueActive)
	{
		AROCharacterBase* Player = CurrentPlayer;

		// Clean up per-player session
		if (Player)
		{
			ActiveSessions.Remove(Player);
		}

		// Update legacy state
		bIsDialogueActive = false;
		CurrentPlayer = nullptr;
		CurrentNodeIndex = 0;

		// Check if any other sessions are still active
		// (for the legacy bIsDialogueActive flag, only clear if no sessions remain)
		if (ActiveSessions.Num() > 0)
		{
			bIsDialogueActive = true;
		}

		OnDialogueEnd.Broadcast(Player);
	}
}

const FRODialogueNode* URODialogueComponent::FindNodeByID(int32 NodeID) const
{
	for (const FRODialogueNode& Node : DialogueTree)
	{
		if (Node.NodeID == NodeID)
		{
			return &Node;
		}
	}
	return nullptr;
}
