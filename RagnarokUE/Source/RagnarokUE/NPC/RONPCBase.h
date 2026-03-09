// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/HitResult.h"
#include "GameFramework/Actor.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "RONPCBase.generated.h"

class AROCharacterBase;
class USphereComponent;
class UWidgetComponent;
class URODialogueComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCInteracted, ARONPCBase*, NPC, AROCharacterBase*, Interactor);

/**
 * ARONPCBase
 * Base actor for all NPCs in the Ragnarok Online UE5 recreation.
 * Provides interaction detection, name display, and NPC role flags.
 */
UCLASS(ClassGroup=(RagnarokUE), Blueprintable)
class RAGNAROKUE_API ARONPCBase : public AActor
{
	GENERATED_BODY()

public:
	ARONPCBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	// ---- NPC Identity ----

	/** Unique NPC identifier from the database. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "NPC|Identity")
	int32 NPCID;

	/** Internal name for lookups. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "NPC|Identity")
	FName NPCName;

	/** Localized display name shown above the NPC's head. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "NPC|Identity")
	FText DisplayName;

	// ---- Visual ----

	/** Root scene component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Visual")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Skeletal mesh for animated NPCs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Visual")
	TObjectPtr<USkeletalMeshComponent> NPCSkeletalMesh;

	/** Static mesh for non-animated NPCs (signs, objects). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Visual")
	TObjectPtr<UStaticMeshComponent> NPCStaticMesh;

	// ---- Interaction ----

	/** Sphere collision for detecting player proximity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Interaction")
	TObjectPtr<USphereComponent> InteractionRadius;

	// ---- NPC Role Flags ----

	/** Whether this NPC sells items. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Role")
	bool bIsShop;

	/** Whether this NPC provides services (storage, refine, etc.). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Role")
	bool bIsService;

	/** Whether this NPC offers quests. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Role")
	bool bIsQuestGiver;

	// ---- Dialogue ----

	/** Optional dialogue component for conversation trees. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Dialogue")
	TObjectPtr<URODialogueComponent> DialogueComponent;

	// ---- Delegates ----

	/** Broadcast when a player interacts with this NPC. */
	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnNPCInteracted OnNPCInteracted;

	// ---- Functions ----

	/** Called when a player interacts with this NPC. Override in subclasses. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NPC")
	void OnInteract(AROCharacterBase* Interactor);
	virtual void OnInteract_Implementation(AROCharacterBase* Interactor);

	/** Get the display name of this NPC. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "NPC")
	FText GetDisplayName() const { return DisplayName; }

protected:
	/** Called when a player enters the interaction radius. */
	UFUNCTION()
	void OnInteractionRadiusOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	/** Called when a player exits the interaction radius. */
	UFUNCTION()
	void OnInteractionRadiusEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
};
