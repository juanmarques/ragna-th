// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "RODialogueComponent.generated.h"

class AROCharacterBase;

/**
 * FRODialogueChoice
 * A single selectable option within a dialogue node.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FRODialogueChoice
{
	GENERATED_BODY()

	/** Text displayed for this choice. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText ChoiceText;

	/** The NodeID to jump to when this choice is selected. -1 means end dialogue. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 NextNodeID = -1;

	/** Whether selecting this choice requires an item in inventory. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bRequiresItem = false;

	/** The item ID required if bRequiresItem is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (EditCondition = "bRequiresItem"))
	int32 RequiredItemID = 0;

	/** Whether this choice starts a quest. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bStartsQuest = false;

	/** Quest ID to start if bStartsQuest is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (EditCondition = "bStartsQuest"))
	int32 QuestID = 0;

	/** Whether selecting this choice ends the dialogue. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bEndsDialogue = false;
};

/**
 * FRODialogueNode
 * A single node in a dialogue tree containing speaker info, text, and choices.
 */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FRODialogueNode
{
	GENERATED_BODY()

	/** Unique identifier for this node within the dialogue tree. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 NodeID = 0;

	/** Name of the speaker (usually the NPC). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText SpeakerName;

	/** The dialogue text displayed to the player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText DialogueText;

	/** Available choices for the player after reading this node. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FRODialogueChoice> Choices;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, AROCharacterBase*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnd, AROCharacterBase*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueNodeChanged, const FRODialogueNode&, NewNode, AROCharacterBase*, Player);

/**
 * URODialogueComponent
 * Manages a dialogue tree for an NPC. Attached to NPC actors to provide
 * conversation flow with branching choices.
 */
UCLASS(ClassGroup=(RagnarokUE), meta=(BlueprintSpawnableComponent))
class RAGNAROKUE_API URODialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URODialogueComponent();

	// ---- Dialogue Data ----

	/** The full dialogue tree for this NPC. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FRODialogueNode> DialogueTree;

	// ---- Delegates ----

	/** Fired when dialogue begins. */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDialogueStarted OnDialogueStarted;

	/** Fired when dialogue ends. */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDialogueEnd OnDialogueEnd;

	/** Fired when the current dialogue node changes. */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDialogueNodeChanged OnDialogueNodeChanged;

	// ---- Functions ----

	/** Check if this component has any dialogue nodes. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	bool HasDialogue() const;

	/** Start dialogue with a player. Returns the first dialogue node. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FRODialogueNode StartDialogue(AROCharacterBase* Player);

	/** Select a choice by index in the current node's Choices array. Returns the next node. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FRODialogueNode SelectChoice(int32 ChoiceIndex);

	/** Get the current dialogue node. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	FRODialogueNode GetCurrentNode() const;

	/** Check if dialogue is currently active. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	bool IsDialogueActive() const { return bIsDialogueActive; }

	/** End the current dialogue session. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

protected:
	/** Find a dialogue node by its NodeID. Returns nullptr if not found. */
	const FRODialogueNode* FindNodeByID(int32 NodeID) const;

private:
	/** Per-player dialogue session state. */
	struct FDialogueSession
	{
		int32 CurrentNodeIndex = 0;
		bool bIsActive = false;
	};

	/** Active sessions keyed by player. */
	TMap<TObjectPtr<AROCharacterBase>, FDialogueSession> ActiveSessions;

	// Legacy single-player state kept for backward-compatible API.
	// Internally we route through ActiveSessions.

	/** Index into DialogueTree for the current node. */
	int32 CurrentNodeIndex;

	/** Whether a dialogue session is currently active. */
	bool bIsDialogueActive;

	/** The player currently engaged in dialogue (last interacting player for legacy API). */
	UPROPERTY()
	TObjectPtr<AROCharacterBase> CurrentPlayer;
};
