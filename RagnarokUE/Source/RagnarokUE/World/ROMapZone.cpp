// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROMapZone.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"

AROMapZone::AROMapZone()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ZoneVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneVolume"));
	ZoneVolume->SetupAttachment(SceneRoot);
	ZoneVolume->SetBoxExtent(FVector(500.0f, 500.0f, 300.0f));
	ZoneVolume->SetCollisionProfileName(TEXT("Trigger"));
	ZoneVolume->SetGenerateOverlapEvents(true);
}

void AROMapZone::BeginPlay()
{
	Super::BeginPlay();

	if (ZoneVolume)
	{
		ZoneVolume->OnComponentBeginOverlap.AddDynamic(this, &AROMapZone::OnZoneOverlapBegin);
		ZoneVolume->OnComponentEndOverlap.AddDynamic(this, &AROMapZone::OnZoneOverlapEnd);
	}
}

void AROMapZone::OnZoneOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
	{
		return;
	}

	ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
	if (!PlayerCharacter || !PlayerCharacter->IsPlayerControlled())
	{
		return;
	}

	ApplyZoneRules(OtherActor);
}

void AROMapZone::OnZoneOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
	if (!PlayerCharacter || !PlayerCharacter->IsPlayerControlled())
	{
		return;
	}

	RemoveZoneRules(OtherActor);
}

void AROMapZone::ApplyZoneRules(AActor* PlayerCharacter)
{
	// TODO: Apply zone rules to the AROCharacterBase.
	// In a full implementation:
	//   AROCharacterBase* ROChar = Cast<AROCharacterBase>(PlayerCharacter);
	//   if (ROChar)
	//   {
	//       ROChar->SetPvPEnabled(bIsPvPZone);
	//       ROChar->SetTeleportBlocked(bIsNoTeleportZone);
	//       ROChar->SetInTown(bIsTownZone);
	//       ROChar->SetInGuildZone(bIsGuildZone, OwnerGuildID);
	//   }

	UE_LOG(LogTemp, Log, TEXT("Player %s entered zone '%s' [PvP=%d, NoTP=%d, Town=%d, Guild=%d]"),
		*PlayerCharacter->GetName(), *ZoneName,
		bIsPvPZone, bIsNoTeleportZone, bIsTownZone, bIsGuildZone);

	// Display zone name to the player (via client RPC or UI notification)
	if (!ZoneName.IsEmpty())
	{
		// TODO: Show zone name notification to the player's HUD.
		UE_LOG(LogTemp, Log, TEXT("Zone: %s (Recommended Level: %d)"), *ZoneName, RecommendedLevel);
	}
}

void AROMapZone::RemoveZoneRules(AActor* PlayerCharacter)
{
	// TODO: Revert zone rules when leaving.
	// In a full implementation, reset the flags on the character.

	UE_LOG(LogTemp, Log, TEXT("Player %s left zone '%s'"),
		*PlayerCharacter->GetName(), *ZoneName);
}
