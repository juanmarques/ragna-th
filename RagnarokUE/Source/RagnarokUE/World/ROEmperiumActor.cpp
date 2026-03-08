// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROEmperiumActor.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "RagnarokUE/World/ROWoEManager.h"

AROEmperiumActor::AROEmperiumActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetupAttachment(SceneRoot);
	CollisionComponent->SetSphereRadius(80.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAll"));
}

void AROEmperiumActor::BeginPlay()
{
	Super::BeginPlay();

	// Initialize HP to max on spawn
	if (HP <= 0)
	{
		HP = MaxHP;
	}
}

void AROEmperiumActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AROEmperiumActor, HP);
	DOREPLIFETIME(AROEmperiumActor, MaxHP);
	DOREPLIFETIME(AROEmperiumActor, OwnerGuildID);
	DOREPLIFETIME(AROEmperiumActor, bIsVulnerable);
	DOREPLIFETIME(AROEmperiumActor, bHasSafetyWall);
	DOREPLIFETIME(AROEmperiumActor, bHasPneuma);
	DOREPLIFETIME(AROEmperiumActor, bHasSanctuary);
}

void AROEmperiumActor::InitializeEmperium(int32 InCastleID, int32 InOwnerGuildID, int32 InMaxHP)
{
	CastleID = InCastleID;
	OwnerGuildID = InOwnerGuildID;
	MaxHP = InMaxHP;
	HP = MaxHP;
	bIsVulnerable = false;
	bHasSafetyWall = false;
	bHasPneuma = false;
	bHasSanctuary = false;

	UE_LOG(LogTemp, Log, TEXT("Emperium initialized: Castle=%d, Guild=%d, HP=%d"),
		CastleID, OwnerGuildID, MaxHP);
}

void AROEmperiumActor::ResetHP()
{
	HP = MaxHP;
	UE_LOG(LogTemp, Log, TEXT("Emperium HP reset to %d (Castle %d)"), MaxHP, CastleID);
}

void AROEmperiumActor::SetVulnerable(bool bVulnerable)
{
	bIsVulnerable = bVulnerable;
	UE_LOG(LogTemp, Log, TEXT("Emperium (Castle %d) vulnerability set to %s"),
		CastleID, bVulnerable ? TEXT("TRUE") : TEXT("FALSE"));
}

void AROEmperiumActor::SetSafetyWall(bool bActive)
{
	bHasSafetyWall = bActive;
}

void AROEmperiumActor::SetPneuma(bool bActive)
{
	bHasPneuma = bActive;
}

void AROEmperiumActor::SetSanctuary(bool bActive)
{
	bHasSanctuary = bActive;
}

float AROEmperiumActor::GetHPPercent() const
{
	if (MaxHP <= 0)
	{
		return 0.0f;
	}
	return static_cast<float>(HP) / static_cast<float>(MaxHP);
}

bool AROEmperiumActor::IsAlive() const
{
	return HP > 0;
}

int32 AROEmperiumActor::ApplyEmperiumDamage(int32 DamageAmount, int32 AttackerGuildID, bool bIsSkillDamage, bool bIsRanged)
{
	// Only process on the server
	if (!HasAuthority())
	{
		return 0;
	}

	// Must be vulnerable (WoE active)
	if (!bIsVulnerable)
	{
		return 0;
	}

	// Already destroyed
	if (HP <= 0)
	{
		return 0;
	}

	// Emperium only takes normal attacks, no skill damage
	if (bIsSkillDamage)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Emperium: Skill damage rejected from guild %d"), AttackerGuildID);
		return 0;
	}

	// Must belong to a guild to attack the Emperium
	if (AttackerGuildID <= 0)
	{
		return 0;
	}

	// Only non-guild members can attack (owning guild cannot damage their own Emperium)
	if (AttackerGuildID == OwnerGuildID)
	{
		return 0;
	}

	// Pneuma blocks ranged attacks
	if (bIsRanged && bHasPneuma)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Emperium: Ranged attack blocked by Pneuma"));
		return 0;
	}

	// Safety Wall blocks melee attacks
	if (!bIsRanged && bHasSafetyWall)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Emperium: Melee attack blocked by Safety Wall"));
		return 0;
	}

	// Apply damage
	const int32 ActualDamage = FMath::Min(DamageAmount, HP);
	HP -= ActualDamage;

	UE_LOG(LogTemp, Log, TEXT("Emperium (Castle %d): Took %d damage from guild %d. HP: %d/%d"),
		CastleID, ActualDamage, AttackerGuildID, HP, MaxHP);

	// Check for destruction
	if (HP <= 0)
	{
		HandleDestruction(AttackerGuildID);
	}

	return ActualDamage;
}

float AROEmperiumActor::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	// Redirect to our custom damage system.
	// The UE damage pipeline doesn't carry our RO-specific info (guild ID, skill vs normal),
	// so we log a warning. Use ApplyEmperiumDamage() directly for proper behavior.
	UE_LOG(LogTemp, Warning, TEXT("Emperium: TakeDamage called via UE pipeline. Use ApplyEmperiumDamage() for proper RO behavior."));
	return 0.0f;
}

void AROEmperiumActor::HandleDestruction(int32 AttackingGuildID)
{
	UE_LOG(LogTemp, Log, TEXT("=== EMPERIUM DESTROYED! Castle %d captured by Guild %d ==="),
		CastleID, AttackingGuildID);

	// Broadcast the destruction event
	OnEmperiumDestroyed.Broadcast(CastleID, AttackingGuildID);

	// Notify the WoE manager to handle castle ownership transfer
	if (UGameInstance* GI = GetGameInstance())
	{
		UROWoEManager* WoEManager = GI->GetSubsystem<UROWoEManager>();
		if (WoEManager)
		{
			WoEManager->OnEmperiumDestroyed(CastleID, AttackingGuildID);
		}
	}
}
