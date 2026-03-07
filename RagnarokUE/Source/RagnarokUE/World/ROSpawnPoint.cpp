// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROSpawnPoint.h"
#include "Components/BillboardComponent.h"

AROSpawnPoint::AROSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

#if WITH_EDITORONLY_DATA
	EditorSprite = CreateDefaultSubobject<UBillboardComponent>(TEXT("EditorSprite"));
	if (EditorSprite)
	{
		EditorSprite->SetupAttachment(SceneRoot);
		EditorSprite->SetHiddenInGame(true);
	}
#endif
}

void AROSpawnPoint::SetPlayerSpawnPoint(AActor* Player)
{
	if (!Player)
	{
		return;
	}

	// TODO: Store the spawn point on the player character.
	// In the full implementation, the AROCharacterBase should have a SavedSpawnMapID and
	// SavedSpawnLocation property that we set here.
	// Example:
	//   AROCharacterBase* ROChar = Cast<AROCharacterBase>(Player);
	//   if (ROChar)
	//   {
	//       ROChar->SavedSpawnMapID = MapID;
	//       ROChar->SavedSpawnLocation = GetActorLocation();
	//   }

	UE_LOG(LogTemp, Log, TEXT("Spawn point '%s' on map '%s' saved for player %s"),
		*SpawnName, *MapID.ToString(), *Player->GetName());
}

FTransform AROSpawnPoint::GetSpawnTransform() const
{
	return GetActorTransform();
}

FVector AROSpawnPoint::GetSpawnLocation() const
{
	return GetActorLocation();
}

FRotator AROSpawnPoint::GetSpawnRotation() const
{
	return GetActorRotation();
}
