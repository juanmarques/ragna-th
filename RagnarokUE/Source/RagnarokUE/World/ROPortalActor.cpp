// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROPortalActor.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AROPortalActor::AROPortalActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(SceneRoot);
	TriggerVolume->SetBoxExtent(FVector(100.0f, 100.0f, 200.0f));
	TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
	TriggerVolume->SetGenerateOverlapEvents(true);
}

void AROPortalActor::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AROPortalActor::OnTriggerOverlapBegin);
	}
}

void AROPortalActor::OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
	{
		return;
	}

	// Only process on the server
	if (!HasAuthority())
	{
		return;
	}

	// Check if the overlapping actor is a player character
	ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
	if (!PlayerCharacter)
	{
		return;
	}

	// Verify the actor is player-controlled
	if (!PlayerCharacter->IsPlayerControlled())
	{
		return;
	}

	// Check level requirement
	if (!MeetsLevelRequirement(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Portal %s: Player does not meet level requirement (%d)"),
			*GetName(), RequiredBaseLevel);
		return;
	}

	TeleportPlayer(OtherActor);
}

bool AROPortalActor::MeetsLevelRequirement(AActor* PlayerCharacter) const
{
	if (RequiredBaseLevel <= 0)
	{
		return true;
	}

	// TODO: Get the player's base level from their AROCharacterBase stats component.
	// For now, we check if the cast to ACharacter succeeds (assume level requirement met).
	// In production:
	//   AROCharacterBase* ROChar = Cast<AROCharacterBase>(PlayerCharacter);
	//   if (ROChar) { return ROChar->GetBaseLevel() >= RequiredBaseLevel; }
	return true;
}

void AROPortalActor::TeleportPlayer(AActor* PlayerCharacter)
{
	if (DestinationMapID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("Portal %s: No destination map set"), *GetName());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Portal %s: Teleporting player to map '%s' at location %s"),
		*GetName(), *DestinationMapID.ToString(), *DestinationLocation.ToString());

	// For same-level teleportation (within the same UE map):
	// Simply teleport the actor to the destination location.
	const FRotator DestRotation(0.0f, DestinationRotation, 0.0f);
	PlayerCharacter->SetActorLocationAndRotation(DestinationLocation, DestRotation);

	// For cross-level travel (different UE maps), use server travel:
	// APlayerController* PC = Cast<ACharacter>(PlayerCharacter)->GetController<APlayerController>();
	// if (PC)
	// {
	//     // Seamless travel to the destination level
	//     GetWorld()->ServerTravel(FString::Printf(TEXT("/Game/Maps/%s?listen"), *DestinationMapID.ToString()));
	// }
}
