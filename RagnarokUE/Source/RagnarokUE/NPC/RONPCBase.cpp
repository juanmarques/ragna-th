// Copyright Ragna-TH Project. All Rights Reserved.

#include "RONPCBase.h"
#include "RODialogueComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "RagnarokUE/Character/ROCharacterBase.h"

ARONPCBase::ARONPCBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	NPCID = 0;
	NPCName = NAME_None;
	DisplayName = FText::FromString(TEXT("NPC"));
	bIsShop = false;
	bIsService = false;
	bIsQuestGiver = false;

	// Root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// Skeletal mesh
	NPCSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("NPCSkeletalMesh"));
	NPCSkeletalMesh->SetupAttachment(SceneRoot);

	// Static mesh (disabled by default; use one or the other)
	NPCStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCStaticMesh"));
	NPCStaticMesh->SetupAttachment(SceneRoot);
	NPCStaticMesh->SetVisibility(false);

	// Interaction radius
	InteractionRadius = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRadius"));
	InteractionRadius->SetupAttachment(SceneRoot);
	InteractionRadius->SetSphereRadius(200.0f);
	InteractionRadius->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	InteractionRadius->SetGenerateOverlapEvents(true);

	// Dialogue component
	DialogueComponent = CreateDefaultSubobject<URODialogueComponent>(TEXT("DialogueComponent"));
}

void ARONPCBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARONPCBase, NPCID);
	DOREPLIFETIME(ARONPCBase, NPCName);
	DOREPLIFETIME(ARONPCBase, DisplayName);
}

void ARONPCBase::BeginPlay()
{
	Super::BeginPlay();

	if (InteractionRadius)
	{
		InteractionRadius->OnComponentBeginOverlap.AddDynamic(this, &ARONPCBase::OnInteractionRadiusOverlap);
		InteractionRadius->OnComponentEndOverlap.AddDynamic(this, &ARONPCBase::OnInteractionRadiusEndOverlap);
	}
}

void ARONPCBase::OnInteract_Implementation(AROCharacterBase* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	OnNPCInteracted.Broadcast(this, Interactor);

	// If this NPC has dialogue, start it
	if (DialogueComponent && DialogueComponent->HasDialogue())
	{
		DialogueComponent->StartDialogue(Interactor);
	}
}

void ARONPCBase::OnInteractionRadiusOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	AROCharacterBase* Character = Cast<AROCharacterBase>(OtherActor);
	if (Character)
	{
		// Notify the player that they are in interaction range
		// The actual interaction is triggered by player input (click)
		UE_LOG(LogTemp, Verbose, TEXT("NPC %s: Player entered interaction range."), *NPCName.ToString());
	}
}

void ARONPCBase::OnInteractionRadiusEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	AROCharacterBase* Character = Cast<AROCharacterBase>(OtherActor);
	if (Character)
	{
		UE_LOG(LogTemp, Verbose, TEXT("NPC %s: Player left interaction range."), *NPCName.ToString());
	}
}
