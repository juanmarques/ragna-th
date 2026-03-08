// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROCharacterBase.h"
#include "ROStatsComponent.h"
#include "ROJobComponent.h"
#include "ROLevelingComponent.h"
#include "ROCharacterMovement.h"
#include "RagnarokUE/Skills/ROAbilitySystemComponent.h"
#include "RagnarokUE/Skills/ROAttributeSet.h"
#include "RagnarokUE/Core/ROPlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"

AROCharacterBase::AROCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UROCharacterMovement>(
		ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAbilitySystemInitialized = false;

	// ---- Create Components ----

	StatsComponent = CreateDefaultSubobject<UROStatsComponent>(TEXT("StatsComponent"));
	JobComponent = CreateDefaultSubobject<UROJobComponent>(TEXT("JobComponent"));
	LevelingComponent = CreateDefaultSubobject<UROLevelingComponent>(TEXT("LevelingComponent"));
	AbilitySystemComponent = CreateDefaultSubobject<UROAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// AbilitySystemComponent replication mode
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create the AttributeSet as a subobject so GAS can find it
	AttributeSet = CreateDefaultSubobject<UROAttributeSet>(TEXT("AttributeSet"));

	// ---- Camera Setup (Isometric RO-style) ----

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);
	CameraArm->TargetArmLength = 2000.0f;
	CameraArm->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
	CameraArm->bDoCollisionTest = false; // No collision for isometric camera
	CameraArm->bUsePawnControlRotation = false;
	CameraArm->bInheritPitch = false;
	CameraArm->bInheritYaw = false;
	CameraArm->bInheritRoll = false;
	CameraArm->bEnableCameraLag = true;
	CameraArm->CameraLagSpeed = 10.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	FollowCamera->SetFieldOfView(30.0f); // Narrow FOV for isometric feel
	FollowCamera->bUsePawnControlRotation = false;

	// Don't rotate character with camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// ---- Initialize Replicated State ----
	CurrentHP = 0;
	CurrentSP = 0;
	MaxHP = 0;
	MaxSP = 0;
	bIsDead = false;
	bIsSitting = false;
	bPvPEnabled = false;
	bTeleportBlocked = false;
	bInTown = false;
	bInGuildZone = false;
	GuildZoneOwnerID = 0;
	SavedSpawnMapID = FName(TEXT("prontera"));
	SavedSpawnLocation = FVector::ZeroVector;
}

void AROCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AROCharacterBase, CurrentHP);
	DOREPLIFETIME(AROCharacterBase, CurrentSP);
	DOREPLIFETIME(AROCharacterBase, MaxHP);
	DOREPLIFETIME(AROCharacterBase, MaxSP);
	DOREPLIFETIME(AROCharacterBase, bIsDead);
	DOREPLIFETIME(AROCharacterBase, bIsSitting);
}

void AROCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// Bind delegates
	if (LevelingComponent)
	{
		LevelingComponent->OnBaseLevelUp.AddDynamic(this, &AROCharacterBase::OnBaseLevelUp);
	}

	if (JobComponent)
	{
		JobComponent->OnJobChanged.AddDynamic(this, &AROCharacterBase::OnJobChanged);
	}

	// Initialize HP/SP from stats on server
	if (HasAuthority())
	{
		SyncVitalsFromStats();

		// Set current HP/SP to max on initial spawn
		if (CurrentHP <= 0)
		{
			CurrentHP = MaxHP;
		}
		if (CurrentSP <= 0)
		{
			CurrentSP = MaxSP;
		}
	}
}

void AROCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Server-side: initialize the ability system
	if (HasAuthority())
	{
		InitializeAbilitySystem();

		// Sync player state with authoritative character data
		if (AROPlayerState* PS = NewController->GetPlayerState<AROPlayerState>())
		{
			PS->SyncFromCharacter(this);
		}
	}
}

void AROCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Client-side: initialize ability system actor info
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

// ---- IAbilitySystemInterface ----

UAbilitySystemComponent* AROCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// ---- OnRep Callbacks ----

void AROCharacterBase::OnRep_CurrentHP()
{
	// UI update hook - can be bound via Blueprint
	if (CurrentHP <= 0 && !bIsDead)
	{
		// Client prediction: character should appear dead
	}
}

void AROCharacterBase::OnRep_CurrentSP()
{
	// UI update hook
}

void AROCharacterBase::OnRep_MaxHP()
{
	// UI update hook
}

void AROCharacterBase::OnRep_MaxSP()
{
	// UI update hook
}

void AROCharacterBase::OnRep_bIsDead()
{
	if (bIsDead)
	{
		// Play death animation on client
		// Disable collision (capsule + mesh, consistent with Die())
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		// Restore after respawn (capsule + mesh, consistent with Respawn())
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void AROCharacterBase::OnRep_bIsSitting()
{
	// Trigger sit/stand animation on client
}

// ---- Character Actions ----

void AROCharacterBase::Die()
{
	if (bIsDead)
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	bIsDead = true;
	CurrentHP = 0;

	// Stop all movement
	if (UROCharacterMovement* ROMovement = GetROMovementComponent())
	{
		ROMovement->StopMovementCommand();
	}

	// Cancel all abilities
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAllAbilities();
	}

	// Disable input
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}

	// Disable collision (capsule + mesh, consistent with OnRep_bIsDead)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UE_LOG(LogTemp, Log, TEXT("ROCharacterBase: %s has died."), *GetName());
}

void AROCharacterBase::Respawn(FVector RespawnLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	bIsDead = false;

	// Teleport to respawn location
	SetActorLocation(RespawnLocation);

	// Restore HP/SP (RO respawns with reduced HP/SP, we use 50%)
	SyncVitalsFromStats();
	CurrentHP = FMath::Max(1, MaxHP / 2);
	CurrentSP = FMath::Max(0, MaxSP / 2);

	// Re-enable collision (capsule + mesh, consistent with Die())
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// Re-enable input
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		EnableInput(PC);
	}

	UE_LOG(LogTemp, Log, TEXT("ROCharacterBase: %s respawned at %s."),
		*GetName(), *RespawnLocation.ToString());
}

void AROCharacterBase::SitDown()
{
	if (!HasAuthority())
	{
		ServerSitDown();
		return;
	}
	if (bIsDead || bIsSitting)
	{
		return;
	}

	// Stop movement first
	if (UROCharacterMovement* ROMovement = GetROMovementComponent())
	{
		ROMovement->StopMovementCommand();
	}

	bIsSitting = true;

	UE_LOG(LogTemp, Log, TEXT("ROCharacterBase: %s is sitting."), *GetName());
}

void AROCharacterBase::StandUp()
{
	if (!HasAuthority())
	{
		ServerStandUp();
		return;
	}
	if (!bIsSitting)
	{
		return;
	}

	bIsSitting = false;

	UE_LOG(LogTemp, Log, TEXT("ROCharacterBase: %s stood up."), *GetName());
}

void AROCharacterBase::ServerSitDown_Implementation()
{
	SitDown();
}

void AROCharacterBase::ServerStandUp_Implementation()
{
	StandUp();
}

float AROCharacterBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	// Only server should process damage — CurrentHP is replicated
	if (!HasAuthority())
	{
		return 0.0f;
	}

	if (bIsDead)
	{
		return 0.0f;
	}

	// RO Damage Pipeline:
	// 1. The actual damage calculation (ATK vs DEF, elemental modifiers, etc.)
	//    is expected to be computed by the combat system before calling TakeDamage.
	//    DamageAmount here is the final post-calculation damage.

	// 2. Apply sitting penalty: in RO, sitting characters take double damage from player attacks only
	float FinalDamage = DamageAmount;
	if (bIsSitting)
	{
		// Only apply 2x penalty if damage source is a player-controlled character
		if (EventInstigator && EventInstigator->IsPlayerController())
		{
			FinalDamage *= 2.0f;
		}
		StandUp(); // Getting hit forces you to stand regardless of source
	}

	// 3. Handle 0 (miss/immune) and negative (absorb/heal) damage
	if (FinalDamage < 0.0f)
	{
		// Absorb: heal the character
		CurrentHP = FMath::Min(CurrentHP + FMath::Abs(FMath::RoundToInt32(FinalDamage)), MaxHP);
		OnRep_CurrentHP();
		return 0.0f; // No damage dealt
	}
	else if (FinalDamage == 0.0f)
	{
		// Miss/immune — no damage
		return 0.0f;
	}

	// Ensure minimum 1 damage for actual hits
	FinalDamage = FMath::Max(1.0f, FinalDamage);

	// 4. Apply damage to HP
	const int32 DamageInt = FMath::CeilToInt(FinalDamage);
	CurrentHP = FMath::Max(0, CurrentHP - DamageInt);

	// 5. Call parent for engine-level damage handling
	const float ActualDamage = Super::TakeDamage(FinalDamage, DamageEvent, EventInstigator, DamageCauser);

	// 6. Check for death
	if (CurrentHP <= 0)
	{
		Die();
	}

	UE_LOG(LogTemp, Verbose, TEXT("ROCharacterBase: %s took %d damage. HP: %d/%d"),
		*GetName(), DamageInt, CurrentHP, MaxHP);

	return ActualDamage;
}

// ---- Ability System ----

void AROCharacterBase::InitializeAbilitySystem()
{
	if (bAbilitySystemInitialized || !AbilitySystemComponent)
	{
		return;
	}

	// Initialize ability actor info - owner and avatar are both this character
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// Grant default abilities
	GrantDefaultAbilities();

	bAbilitySystemInitialized = true;

	UE_LOG(LogTemp, Log, TEXT("ROCharacterBase: Ability system initialized for %s."), *GetName());
}

void AROCharacterBase::GrantDefaultAbilities()
{
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	// Grant universal default abilities
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			GrantAbility(AbilityClass);
		}
	}

	// Grant job-specific abilities
	if (JobComponent)
	{
		if (TSubclassOf<UGameplayAbility>* JobAbility = JobAbilities.Find(JobComponent->CurrentJobClass))
		{
			if (*JobAbility)
			{
				GrantAbility(*JobAbility);
			}
		}
	}
}

void AROCharacterBase::GrantAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level)
{
	if (!HasAuthority() || !AbilitySystemComponent || !AbilityClass)
	{
		return;
	}

	FGameplayAbilitySpec AbilitySpec(AbilityClass, Level, INDEX_NONE, this);
	AbilitySystemComponent->GiveAbility(AbilitySpec);
}

void AROCharacterBase::RemoveAbility(TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!HasAuthority() || !AbilitySystemComponent || !AbilityClass)
	{
		return;
	}

	// Find and remove the ability
	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass);
	if (Spec)
	{
		AbilitySystemComponent->ClearAbility(Spec->Handle);
	}
}

UROCharacterMovement* AROCharacterBase::GetROMovementComponent() const
{
	return Cast<UROCharacterMovement>(GetCharacterMovement());
}

// ---- Delegate Callbacks ----

void AROCharacterBase::OnBaseLevelUp(int32 NewLevel)
{
	// Update MaxHP/MaxSP from newly recalculated stats
	SyncVitalsFromStats();

	// Heal to full on level up (classic RO behavior)
	if (HasAuthority())
	{
		CurrentHP = MaxHP;
		CurrentSP = MaxSP;
	}
}

void AROCharacterBase::OnJobChanged(EROJobClass OldJob, EROJobClass NewJob)
{
	if (!HasAuthority())
	{
		return;
	}

	// Remove old job abilities
	if (TSubclassOf<UGameplayAbility>* OldAbility = JobAbilities.Find(OldJob))
	{
		if (*OldAbility)
		{
			RemoveAbility(*OldAbility);
		}
	}

	// Grant new job abilities
	if (TSubclassOf<UGameplayAbility>* NewAbility = JobAbilities.Find(NewJob))
	{
		if (*NewAbility)
		{
			GrantAbility(*NewAbility);
		}
	}

	// Recalculate stats (job affects MaxHP/MaxSP formulas)
	if (StatsComponent)
	{
		StatsComponent->RecalculateDerivedStats();
	}

	SyncVitalsFromStats();
}

void AROCharacterBase::SyncVitalsFromStats()
{
	if (!StatsComponent)
	{
		return;
	}

	MaxHP = StatsComponent->MaxHP;
	MaxSP = StatsComponent->MaxSP;

	// Clamp current values to new max
	CurrentHP = FMath::Clamp(CurrentHP, 0, MaxHP);
	CurrentSP = FMath::Clamp(CurrentSP, 0, MaxSP);
}
