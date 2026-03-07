// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROWarpPortal.generated.h"

class UBoxComponent;
class USphereComponent;

/**
 * AROWarpPortal
 * Temporary portal spawned by the Acolyte Warp Portal skill.
 * Lasts for 30 seconds or until used 8 times, whichever comes first.
 * Players stepping on it are teleported to the caster's chosen destination.
 */
UCLASS(ClassGroup=(RagnarokUE), Blueprintable)
class RAGNAROKUE_API AROWarpPortal : public AActor
{
	GENERATED_BODY()

public:
	AROWarpPortal();

	/** Maximum number of uses before the portal closes. */
	static constexpr int32 DefaultMaxUses = 8;

	/** Duration in seconds before the portal expires. */
	static constexpr float DefaultDuration = 30.0f;

	// ---- Portal Configuration ----

	/** Destination map ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "WarpPortal")
	FName DestinationMap;

	/** Destination location within the target map. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "WarpPortal")
	FVector DestinationLocation = FVector::ZeroVector;

	/** Maximum uses remaining. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "WarpPortal")
	int32 MaxUses = DefaultMaxUses;

	/** Remaining uses. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "WarpPortal")
	int32 RemainingUses = DefaultMaxUses;

	/** Duration remaining in seconds. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "WarpPortal")
	float RemainingDuration = DefaultDuration;

	/** The player ID who cast this portal. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WarpPortal")
	int32 CasterPlayerID = 0;

	// ---- Functions ----

	/**
	 * Initialize the warp portal with destination and caster info.
	 * Call this after spawning.
	 */
	UFUNCTION(BlueprintCallable, Category = "WarpPortal")
	void InitializePortal(FName InDestinationMap, FVector InDestinationLocation,
		int32 InCasterPlayerID, int32 InMaxUses = DefaultMaxUses, float InDuration = DefaultDuration);

	/** Get the number of remaining uses. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WarpPortal")
	int32 GetRemainingUses() const { return RemainingUses; }

	/** Get the remaining duration. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WarpPortal")
	float GetRemainingDuration() const { return RemainingDuration; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Trigger volume for detecting player overlap. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WarpPortal")
	TObjectPtr<USphereComponent> TriggerVolume;

	/** Root scene component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WarpPortal")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Called when a player steps on the portal. */
	UFUNCTION()
	void OnPortalOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	/** Teleport a player and decrement uses. */
	void UsePortal(AActor* PlayerCharacter);

	/** Close the portal (destroy actor). */
	void ClosePortal();

	/** Timer handle for auto-destruction. */
	FTimerHandle ExpirationTimerHandle;
};
