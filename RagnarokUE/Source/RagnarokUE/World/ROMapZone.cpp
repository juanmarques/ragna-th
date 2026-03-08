// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROMapZone.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "RagnarokUE/Character/ROCharacterBase.h"

// Static member definition for zone tracking
TMap<TWeakObjectPtr<AActor>, TArray<TWeakObjectPtr<AROMapZone>>> AROMapZone::CharacterZones;

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

void AROMapZone::RemoveCharacterFromZoneTracking(AActor* DestroyedActor)
{
	if (DestroyedActor)
	{
		CharacterZones.Remove(DestroyedActor);
	}
}

void AROMapZone::ApplyZoneRules(AActor* PlayerCharacter)
{
	// Track this zone for the character
	TArray<TWeakObjectPtr<AROMapZone>>& Zones = CharacterZones.FindOrAdd(PlayerCharacter);
	if (Zones.Num() == 0)
	{
		// First zone for this character — bind to OnDestroyed to clean up the tracking map
		PlayerCharacter->OnDestroyed.AddDynamic(this, &AROMapZone::OnCharacterDestroyed);
	}
	Zones.AddUnique(this);

	AROCharacterBase* ROChar = Cast<AROCharacterBase>(PlayerCharacter);
	if (ROChar)
	{
		// Apply flags additively - any zone that enables a flag turns it on
		if (bIsPvPZone) ROChar->bPvPEnabled = true;
		if (bIsNoTeleportZone) ROChar->bTeleportBlocked = true;
		if (bIsTownZone) ROChar->bInTown = true;
		if (bIsGuildZone)
		{
			ROChar->bInGuildZone = true;
			ROChar->GuildZoneOwnerID = OwnerGuildID;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Player %s entered zone '%s' [PvP=%d, NoTP=%d, Town=%d, Guild=%d]"),
		*PlayerCharacter->GetName(), *ZoneName,
		bIsPvPZone, bIsNoTeleportZone, bIsTownZone, bIsGuildZone);
}

void AROMapZone::RemoveZoneRules(AActor* PlayerCharacter)
{
	// Remove this zone from the character's active zone list
	TArray<TWeakObjectPtr<AROMapZone>>* Zones = CharacterZones.Find(PlayerCharacter);
	if (Zones)
	{
		Zones->Remove(this);
	}

	AROCharacterBase* ROChar = Cast<AROCharacterBase>(PlayerCharacter);
	if (ROChar)
	{
		// Reset all flags to defaults
		ROChar->bPvPEnabled = false;
		ROChar->bTeleportBlocked = false;
		ROChar->bInTown = false;
		ROChar->bInGuildZone = false;
		ROChar->GuildZoneOwnerID = 0;

		// Re-apply flags from all remaining active zones
		if (Zones)
		{
			for (const TWeakObjectPtr<AROMapZone>& ZonePtr : *Zones)
			{
				AROMapZone* Zone = ZonePtr.Get();
				if (!Zone)
				{
					continue;
				}
				if (Zone->bIsPvPZone) ROChar->bPvPEnabled = true;
				if (Zone->bIsNoTeleportZone) ROChar->bTeleportBlocked = true;
				if (Zone->bIsTownZone) ROChar->bInTown = true;
				if (Zone->bIsGuildZone)
				{
					ROChar->bInGuildZone = true;
					ROChar->GuildZoneOwnerID = Zone->OwnerGuildID;
				}
			}

			// Clean up the entry if no zones remain
			if (Zones->Num() == 0)
			{
				CharacterZones.Remove(PlayerCharacter);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Player %s left zone '%s'"),
		*PlayerCharacter->GetName(), *ZoneName);
}

void AROMapZone::OnCharacterDestroyed(AActor* DestroyedActor)
{
	RemoveCharacterFromZoneTracking(DestroyedActor);
}
