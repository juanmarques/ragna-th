// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROPortalActor.h"
#include "ROMapManager.h"
#include "ROWoEManager.h"
#include "Components/BoxComponent.h"
#include "RagnarokUE/Character/ROLevelingComponent.h"
#include "RagnarokUE/Character/ROCharacterBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
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

	if (!PlayerCharacter)
	{
		return false;
	}

	UROLevelingComponent* LevelComp = PlayerCharacter->FindComponentByClass<UROLevelingComponent>();
	if (LevelComp)
	{
		return LevelComp->BaseLevel >= RequiredBaseLevel;
	}

	return true;
}

void AROPortalActor::TeleportPlayer(AActor* PlayerCharacter)
{
	if (DestinationMapID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("Portal %s: No destination map set"), *GetName());
		return;
	}

	// Check WoE teleport restriction
	{
		UGameInstance* WoEGI = GetGameInstance();
		if (WoEGI)
		{
			if (UROWoEManager* WoEMgr = WoEGI->GetSubsystem<UROWoEManager>())
			{
				if (WoEMgr->IsWoEActive() && WoEMgr->IsSkillRestrictedInWoE(27)) // 27 = Warp Portal skill ID
				{
					UE_LOG(LogTemp, Log, TEXT("Portal %s: Teleport blocked during WoE"), *GetName());
					return;
				}
			}
		}
	}

	// Check zone teleport block
	AROCharacterBase* ROChar = Cast<AROCharacterBase>(PlayerCharacter);
	if (ROChar && ROChar->bTeleportBlocked)
	{
		UE_LOG(LogTemp, Log, TEXT("Portal %s: Teleport blocked in current zone"), *GetName());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Portal %s: Teleporting player to map '%s' at location %s"),
		*GetName(), *DestinationMapID.ToString(), *DestinationLocation.ToString());

	// Determine if this is a same-map or cross-map teleport
	const bool bIsCrossMap = !SourceMapID.IsNone() && SourceMapID != DestinationMapID;

	if (!bIsCrossMap)
	{
		// Same-level teleportation: move the actor within the current UE map
		const FRotator DestRotation(0.0f, DestinationRotation, 0.0f);
		PlayerCharacter->SetActorLocationAndRotation(DestinationLocation, DestRotation);
		return;
	}

	// Cross-level travel: look up the destination level path from the map manager
	FString TravelURL;

	UGameInstance* GI = GetGameInstance();
	UROMapManager* MapManager = GI ? GI->GetSubsystem<UROMapManager>() : nullptr;
	if (MapManager)
	{
		FROMapConnectionInfo DestInfo = MapManager->GetMapConnectionInfo(DestinationMapID);
		if (!DestInfo.LevelAssetPath.IsEmpty())
		{
			TravelURL = DestInfo.LevelAssetPath;
		}
	}

	// Fallback to convention-based path if map manager doesn't have it
	if (TravelURL.IsEmpty())
	{
		TravelURL = FString::Printf(TEXT("/Game/Maps/%s"), *DestinationMapID.ToString());
	}

	// Append destination coordinates as travel options so the player spawns at the right location
	TravelURL += FString::Printf(TEXT("?DestX=%.1f?DestY=%.1f?DestZ=%.1f?DestYaw=%.1f"),
		DestinationLocation.X, DestinationLocation.Y, DestinationLocation.Z, DestinationRotation);

	UE_LOG(LogTemp, Log, TEXT("Portal %s: Client travel to '%s'"), *GetName(), *TravelURL);

	// Use per-player ClientTravel instead of ServerTravel to avoid disconnecting all players
	ACharacter* Character = Cast<ACharacter>(PlayerCharacter);
	if (Character)
	{
		APlayerController* PC = Cast<APlayerController>(Character->GetController());
		if (PC)
		{
			PC->ClientTravel(TravelURL, TRAVEL_Absolute);
		}
	}
}
