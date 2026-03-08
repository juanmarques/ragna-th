// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROWarpPortal.h"
#include "ROWoEManager.h"
#include "ROMapManager.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "RagnarokUE/Character/ROCharacterBase.h"
#include "Net/UnrealNetwork.h"

AROWarpPortal::AROWarpPortal()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TriggerVolume = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(SceneRoot);
	TriggerVolume->SetSphereRadius(100.0f);
	TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
	TriggerVolume->SetGenerateOverlapEvents(true);
}

void AROWarpPortal::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AROWarpPortal::OnPortalOverlapBegin);
	}
}

void AROWarpPortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		RemainingDuration -= DeltaTime;
		if (RemainingDuration <= 0.0f)
		{
			ClosePortal();
		}
	}
}

void AROWarpPortal::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AROWarpPortal, DestinationMap);
	DOREPLIFETIME(AROWarpPortal, DestinationLocation);
	DOREPLIFETIME(AROWarpPortal, MaxUses);
	DOREPLIFETIME(AROWarpPortal, RemainingUses);
	DOREPLIFETIME(AROWarpPortal, RemainingDuration);
}

void AROWarpPortal::InitializePortal(FName InDestinationMap, FVector InDestinationLocation,
	int32 InCasterPlayerID, int32 InMaxUses, float InDuration)
{
	DestinationMap = InDestinationMap;
	DestinationLocation = InDestinationLocation;
	CasterPlayerID = InCasterPlayerID;
	MaxUses = InMaxUses;
	RemainingUses = InMaxUses;
	RemainingDuration = InDuration;
}

void AROWarpPortal::OnPortalOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || !HasAuthority())
	{
		return;
	}

	ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
	if (!PlayerCharacter || !PlayerCharacter->IsPlayerControlled())
	{
		return;
	}

	UsePortal(OtherActor);
}

void AROWarpPortal::UsePortal(AActor* PlayerCharacter)
{
	if (!PlayerCharacter)
	{
		return;
	}

	if (RemainingUses <= 0)
	{
		return;
	}

	// Check WoE teleport restriction
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UROWoEManager* WoEMgr = GI->GetSubsystem<UROWoEManager>())
		{
			if (WoEMgr->IsWoEActive() && WoEMgr->IsSkillRestrictedInWoE(27)) // 27 = Warp Portal skill ID
			{
				UE_LOG(LogTemp, Log, TEXT("Warp Portal blocked during WoE"));
				return;
			}
		}
	}

	// Check zone teleport block
	AROCharacterBase* ROChar = Cast<AROCharacterBase>(PlayerCharacter);
	if (ROChar && ROChar->bTeleportBlocked)
	{
		UE_LOG(LogTemp, Log, TEXT("Teleport blocked in current zone"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Warp Portal: Teleporting player to map '%s' at %s. Uses remaining: %d"),
		*DestinationMap.ToString(), *DestinationLocation.ToString(), RemainingUses - 1);

	// Determine if this is a cross-map warp by comparing current map to destination
	const UWorld* World = GetWorld();
	const FName CurrentMapName = World ? FName(*World->GetMapName()) : NAME_None;
	const bool bIsCrossMap = !DestinationMap.IsNone() && CurrentMapName != DestinationMap;

	if (!bIsCrossMap)
	{
		// Same-map teleport: move the actor directly
		PlayerCharacter->SetActorLocation(DestinationLocation);
	}
	else
	{
		// Cross-map warp: use per-player ClientTravel with destination coordinates
		FString TravelURL;

		UGameInstance* GI = GetGameInstance();
		UROMapManager* MapManager = GI ? GI->GetSubsystem<UROMapManager>() : nullptr;
		if (MapManager)
		{
			FROMapConnectionInfo DestInfo = MapManager->GetMapConnectionInfo(DestinationMap);
			if (!DestInfo.LevelAssetPath.IsEmpty())
			{
				TravelURL = DestInfo.LevelAssetPath;
			}
		}

		// Fallback to convention-based path
		if (TravelURL.IsEmpty())
		{
			TravelURL = FString::Printf(TEXT("/Game/Maps/%s"), *DestinationMap.ToString());
		}

		// Append destination coordinates so the receiving GameMode can position the player
		TravelURL += FString::Printf(TEXT("?DestX=%.1f?DestY=%.1f?DestZ=%.1f"),
			DestinationLocation.X, DestinationLocation.Y, DestinationLocation.Z);

		UE_LOG(LogTemp, Log, TEXT("Warp Portal: Client travel to '%s'"), *TravelURL);

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

	RemainingUses--;

	if (RemainingUses <= 0)
	{
		ClosePortal();
	}
}

void AROWarpPortal::ClosePortal()
{
	UE_LOG(LogTemp, Log, TEXT("Warp Portal closed (Caster: %d)"), CasterPlayerID);
	Destroy();
}
