// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROMapZone.generated.h"

class UBoxComponent;

/**
 * AROMapZone
 * Volume actor that defines zone properties within a map.
 * When a player enters the volume, zone rules are applied (PvP enabled, teleport blocked, etc.).
 */
UCLASS(ClassGroup=(RagnarokUE), Blueprintable)
class RAGNAROKUE_API AROMapZone : public AActor
{
	GENERATED_BODY()

public:
	AROMapZone();

	// ---- Zone Properties ----

	/** Whether PvP is enabled in this zone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	bool bIsPvPZone = false;

	/** Whether teleport skills (Warp Portal, Teleport) are blocked. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	bool bIsNoTeleportZone = false;

	/** Whether this is a safe town zone (no monster attacks, no PvP). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	bool bIsTownZone = false;

	/** Whether this is a guild-controlled zone (WoE castle territory). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	bool bIsGuildZone = false;

	/** Display name for this zone (shown to players entering). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	FString ZoneName;

	/** Recommended character level for this zone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	int32 RecommendedLevel = 0;

	/** Optional: the guild ID that owns this zone (for guild territories). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	int32 OwnerGuildID = 0;

	/**
	 * Static tracking of which zones each character is currently in.
	 * Used to correctly recompute flags when overlapping zones are exited.
	 */
	static TMap<TWeakObjectPtr<AActor>, TArray<TWeakObjectPtr<AROMapZone>>> CharacterZones;

protected:
	virtual void BeginPlay() override;

	/** Trigger volume for zone detection. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone")
	TObjectPtr<UBoxComponent> ZoneVolume;

	/** Root scene component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Called when an actor enters the zone. */
	UFUNCTION()
	void OnZoneOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	/** Called when an actor leaves the zone. */
	UFUNCTION()
	void OnZoneOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Apply zone rules to a player entering the zone. */
	void ApplyZoneRules(AActor* PlayerCharacter);

	/** Remove zone rules from a player leaving the zone. Recomputes flags from remaining zones. */
	void RemoveZoneRules(AActor* PlayerCharacter);
};
