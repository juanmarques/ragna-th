// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "ROReplicationGraph.generated.h"

class UReplicationGraphNode_GridSpatialization2D;
class UReplicationGraphNode_AlwaysRelevant_ForConnection;

/**
 * UROReplicationGraph
 * Custom replication graph for Ragnarok Online UE5.
 * Provides grid-based spatialization for efficient actor relevancy,
 * always-relevant nodes for party/guild members and global actors,
 * and custom routing per actor class.
 */
UCLASS()
class RAGNAROKUE_API UROReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()

public:
	UROReplicationGraph();

	virtual void InitGlobalActorClassSettings() override;
	virtual void InitGlobalGraphNodes() override;
	virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager) override;
	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override;
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;

	// ---- Configuration ----

	/** Cell size for spatial grid (in UE units). Larger = fewer cells, coarser relevancy. */
	UPROPERTY(EditAnywhere, Category = "Replication")
	float GridCellSize = 10000.0f;

	/** Spatial bias for the grid origin. */
	UPROPERTY(EditAnywhere, Category = "Replication")
	FVector2D GridSpatialBias = FVector2D(-100000.0f, -100000.0f);

protected:
	/** Grid spatialization node for distance-based culling. */
	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_GridSpatialization2D> GridNode;

	/** Always-relevant node for global actors (Emperium, WoE objects, world state). */
	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_ActorList> AlwaysRelevantNode;

	/** Initialize class routing policies for RO actor types. */
	void InitROClassPolicies();
};

/**
 * UROReplicationGraphNode_AlwaysRelevant_ForConnection
 * Per-connection node for actors that are always relevant to a specific player.
 * Used for party members and guild members.
 */
UCLASS()
class RAGNAROKUE_API UROReplicationGraphNode_AlwaysRelevant_ForConnection : public UReplicationGraphNode_AlwaysRelevant_ForConnection
{
	GENERATED_BODY()

public:
	/** Add a party/guild member actor that should always be relevant to this connection. */
	void AddAlwaysRelevantActor(AActor* Actor);

	/** Remove an actor from the always-relevant list. */
	void RemoveAlwaysRelevantActor(AActor* Actor);

	/** Reset the party/guild actor list. */
	void ResetAlwaysRelevantActors();

protected:
	/** Additional actors always relevant to this connection (party/guild). */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> PartyGuildActors;
};
