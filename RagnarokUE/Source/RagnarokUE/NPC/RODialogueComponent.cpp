// Copyright Ragna-TH Project. All Rights Reserved.

#include "RODialogueComponent.h"
#include "RagnarokUE/Character/ROCharacterBase.h"

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

	CurrentPlayer = Player;
	CurrentNodeIndex = 0;
	bIsDialogueActive = true;

	OnDialogueStarted.Broadcast(Player);
	OnDialogueNodeChanged.Broadcast(DialogueTree[0], Player);

	return DialogueTree[0];
}

FRODialogueNode URODialogueComponent::SelectChoice(int32 ChoiceIndex)
{
	if (!bIsDialogueActive || !DialogueTree.IsValidIndex(CurrentNodeIndex))
	{
		return FRODialogueNode();
	}

	const FRODialogueNode& CurrentNode = DialogueTree[CurrentNodeIndex];

	if (!CurrentNode.Choices.IsValidIndex(ChoiceIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("RODialogueComponent: Invalid choice index %d for node %d."),
			ChoiceIndex, CurrentNode.NodeID);
		return CurrentNode;
	}

	const FRODialogueChoice& Choice = CurrentNode.Choices[ChoiceIndex];

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
	for (int32 i = 0; i < DialogueTree.Num(); ++i)
	{
		if (DialogueTree[i].NodeID == Choice.NextNodeID)
		{
			CurrentNodeIndex = i;
			break;
		}
	}

	OnDialogueNodeChanged.Broadcast(*NextNode, CurrentPlayer);

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
		bIsDialogueActive = false;
		AROCharacterBase* Player = CurrentPlayer;
		CurrentPlayer = nullptr;
		CurrentNodeIndex = 0;

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
