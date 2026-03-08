// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROReplicationGraph.h"
#include "Net/UnrealNetwork.h"
#include "Engine/LevelScriptActor.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "RagnarokUE/World/ROEmperiumActor.h"
#include "RagnarokUE/World/ROWarpPortal.h"
#include "RagnarokUE/World/ROPortalActor.h"
#include "RagnarokUE/World/ROMapZone.h"

UROReplicationGraph::UROReplicationGraph()
{
}

void UROReplicationGraph::InitGlobalActorClassSettings()
{
	Super::InitGlobalActorClassSettings();
	InitROClassPolicies();
}

void UROReplicationGraph::InitGlobalGraphNodes()
{
	Super::InitGlobalGraphNodes();

	// Create the grid spatialization node for general actors
	GridNode = CreateNewNode<UReplicationGraphNode_GridSpatialization2D>();
	GridNode->CellSize = GridCellSize;
	GridNode->SpatialBias = GridSpatialBias;
	AddGlobalGraphNode(GridNode);

	// Create always-relevant node for global actors
	AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_ActorList>();
	AddGlobalGraphNode(AlwaysRelevantNode);
}

void UROReplicationGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager)
{
	Super::InitConnectionGraphNodes(ConnectionManager);

	// Create per-connection always-relevant node for party/guild members
	UROReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantForConnection =
		CreateNewNode<UROReplicationGraphNode_AlwaysRelevant_ForConnection>();
	AddConnectionGraphNode(AlwaysRelevantForConnection, ConnectionManager);
}

void UROReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
{
	if (!ActorInfo.Actor)
	{
		return;
	}

	// FIX 10: Emperium actors use the spatial grid instead of always-relevant.
	// This ensures they only replicate to clients on the same map / in proximity,
	// rather than broadcasting to all connected clients across all maps.
	if (ActorInfo.Actor->IsA<AROEmperiumActor>())
	{
		if (GridNode)
		{
			GridNode->NotifyAddNetworkActor(ActorInfo);
		}
		return;
	}

	// Game state and player states are always relevant
	if (ActorInfo.Actor->IsA<AGameStateBase>() || ActorInfo.Actor->IsA<APlayerState>())
	{
		AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);
		return;
	}

	// Level script actors are always relevant
	if (ActorInfo.Actor->IsA<ALevelScriptActor>())
	{
		AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);
		return;
	}

	// Default: use grid spatialization for all other actors
	if (GridNode)
	{
		GridNode->NotifyAddNetworkActor(ActorInfo);
	}
}

void UROReplicationGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
	if (!ActorInfo.Actor)
	{
		return;
	}

	// FIX 10: Emperium uses the grid node, not always-relevant
	if (ActorInfo.Actor->IsA<AROEmperiumActor>())
	{
		if (GridNode)
		{
			GridNode->NotifyRemoveNetworkActor(ActorInfo);
		}
		return;
	}

	if (ActorInfo.Actor->IsA<AGameStateBase>() ||
		ActorInfo.Actor->IsA<APlayerState>() ||
		ActorInfo.Actor->IsA<ALevelScriptActor>())
	{
		AlwaysRelevantNode->NotifyRemoveNetworkActor(ActorInfo);
		return;
	}

	if (GridNode)
	{
		GridNode->NotifyRemoveNetworkActor(ActorInfo);
	}
}

void UROReplicationGraph::InitROClassPolicies()
{
	// Configure replication frequencies and priorities for RO actor classes.
	// Player characters: high frequency (movement, combat state)
	// Monsters: medium frequency (AI movement, combat)
	// NPCs: low frequency (mostly static dialogue triggers)
	// Items on ground: low frequency (static until picked up)
	// Portals/Zones: very low frequency (static world elements)

	// Specific class settings are applied here once the full actor class hierarchy is available.
	// The routing in RouteAddNetworkActorToNodes handles the main differentiation.

	UE_LOG(LogTemp, Log, TEXT("ROReplicationGraph: Class policies initialized"));
}

// ---- UROReplicationGraphNode_AlwaysRelevant_ForConnection ----

void UROReplicationGraphNode_AlwaysRelevant_ForConnection::AddAlwaysRelevantActor(AActor* Actor)
{
	if (Actor && !PartyGuildActors.Contains(Actor))
	{
		PartyGuildActors.Add(Actor);
	}
}

void UROReplicationGraphNode_AlwaysRelevant_ForConnection::RemoveAlwaysRelevantActor(AActor* Actor)
{
	PartyGuildActors.Remove(Actor);
}

void UROReplicationGraphNode_AlwaysRelevant_ForConnection::ResetAlwaysRelevantActors()
{
	PartyGuildActors.Empty();
}
