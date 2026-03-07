// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROMonsterBase.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "RagnarokUE/Items/ROLootManager.h"
#include "RagnarokUE/Character/ROLevelingComponent.h"

AROMonsterBase::AROMonsterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	MonsterID = 0;
	MonsterName = NAME_None;
	HP = 100;
	MaxHP = 100;
	ATK = 1;
	MATK = 0;
	DEF = 0;
	MDEF = 0;
	HIT = 1;
	FLEE = 1;
	Element = EROElement::Neutral;
	ElementLevel = EROElementLevel::Level1;
	Size = EROMonsterSize::Small;
	Race = EROMonsterRace::Formless;
	Behavior = EROMonsterBehavior::Passive;
	BaseExpReward = 0;
	JobExpReward = 0;
	AggroRange = 1000.0f;
	ChaseRange = 2000.0f;
	AttackRange = 150.0f;
	AttackSpeed = 1.5f;
	bIsMVP = false;
	bIsBoss = false;
	bIsDead = false;
	CurrentTarget = nullptr;
	LastAttackTime = 0.0f;
}

void AROMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	SpawnLocation = GetActorLocation();
}

void AROMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AROMonsterBase, MonsterID);
	DOREPLIFETIME(AROMonsterBase, MonsterName);
	DOREPLIFETIME(AROMonsterBase, HP);
	DOREPLIFETIME(AROMonsterBase, MaxHP);
	DOREPLIFETIME(AROMonsterBase, Element);
	DOREPLIFETIME(AROMonsterBase, bIsMVP);
	DOREPLIFETIME(AROMonsterBase, bIsBoss);
	DOREPLIFETIME(AROMonsterBase, bIsDead);
}

void AROMonsterBase::InitializeFromData(const FROMonsterData& Data)
{
	MonsterID = Data.MonsterID;
	MonsterName = Data.MonsterName;
	DisplayName = Data.DisplayName;
	MaxHP = Data.HP;
	HP = MaxHP;
	// ATK uses average of min/max for the stored value; actual damage rolls between min and max
	ATK = (Data.ATKMin + Data.ATKMax) / 2;
	MATK = Data.MATK;
	DEF = Data.DEF;
	MDEF = Data.MDEF;
	HIT = Data.HIT;
	FLEE = Data.FLEE;
	Element = Data.Element;
	ElementLevel = Data.ElementLevel;
	Size = Data.Size;
	Race = Data.Race;
	Behavior = Data.Behavior;
	BaseExpReward = Data.BaseExpReward;
	JobExpReward = Data.JobExpReward;
	AggroRange = Data.AggroRange;
	ChaseRange = Data.ChaseRange;
	AttackRange = Data.AttackRange;
	AttackSpeed = Data.AttackSpeed;
	DropTable = Data.DropTable;
	SkillList = Data.Skills;
	bIsMVP = Data.bIsMVP;
	bIsBoss = Data.bIsBoss;
}

float AROMonsterBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead)
	{
		return 0.0f;
	}

	// Only process damage on server
	if (!HasAuthority())
	{
		return 0.0f;
	}

	// RO Damage Pipeline:
	// Physical: ATK - DEF (hard def reduces %, soft def subtracts)
	// For now, apply damage directly - full formula integration happens with combat system
	const float ActualDamage = FMath::Max(1.0f, DamageAmount);

	HP = FMath::Max(0, HP - FMath::RoundToInt32(ActualDamage));

	// Update threat table
	AActor* Attacker = DamageCauser ? DamageCauser : (EventInstigator ? EventInstigator->GetPawn() : nullptr);
	if (Attacker)
	{
		AddThreat(Attacker, ActualDamage);
	}

	// Fire damaged delegate
	OnMonsterDamaged.Broadcast(this, ActualDamage, Attacker);

	// Check for death
	if (HP <= 0)
	{
		Die(Attacker);
	}

	return ActualDamage;
}

void AROMonsterBase::Die(AActor* Killer)
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	HP = 0;

	// Generate loot drops
	GenerateLoot();

	// Distribute EXP to attackers
	DistributeExp();

	// Multicast death notification for VFX/SFX
	Multicast_OnDeath(Killer);

	// Fire death delegate (used by spawn manager for respawn)
	OnMonsterDied.Broadcast(this, Killer);
}

AActor* AROMonsterBase::GetHighestThreatTarget() const
{
	AActor* HighestTarget = nullptr;
	float HighestThreat = 0.0f;

	for (const auto& Pair : ThreatTable)
	{
		if (Pair.Key.Get() && Pair.Value > HighestThreat)
		{
			HighestThreat = Pair.Value;
			HighestTarget = Pair.Key.Get();
		}
	}

	return HighestTarget;
}

void AROMonsterBase::AddThreat(AActor* Attacker, float Amount)
{
	if (!Attacker)
	{
		return;
	}

	if (float* Existing = ThreatTable.Find(Attacker))
	{
		*Existing += Amount;
	}
	else
	{
		ThreatTable.Add(Attacker, Amount);
	}

	// Update current target to highest threat
	CurrentTarget = GetHighestThreatTarget();
}

bool AROMonsterBase::IsSkillReady(int32 SkillID) const
{
	const float* CooldownExpiry = SkillCooldowns.Find(SkillID);
	if (!CooldownExpiry)
	{
		return true; // Never used, so it's ready
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	return World->GetTimeSeconds() >= *CooldownExpiry;
}

void AROMonsterBase::SetSkillCooldown(int32 SkillID, float CooldownDuration)
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	SkillCooldowns.Add(SkillID, World->GetTimeSeconds() + CooldownDuration);
}

void AROMonsterBase::ResetToIdle()
{
	if (!HasAuthority())
	{
		return;
	}

	HP = MaxHP;
	bIsDead = false;
	CurrentTarget = nullptr;
	ThreatTable.Empty();
	SkillCooldowns.Empty();
	LastAttackTime = 0.0f;
}

bool AROMonsterBase::CanAttack() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	return (World->GetTimeSeconds() - LastAttackTime) >= AttackSpeed;
}

void AROMonsterBase::MarkAttackPerformed()
{
	const UWorld* World = GetWorld();
	if (World)
	{
		LastAttackTime = World->GetTimeSeconds();
	}
}

void AROMonsterBase::OnRep_HP()
{
	// Broadcast HP change for UI updates (health bars, damage numbers, etc.)
	OnMonsterHPChanged.Broadcast(this, HP, MaxHP);
}

void AROMonsterBase::OnRep_IsDead()
{
	// Client-side death state change - can trigger ragdoll, disable collision, etc.
	if (bIsDead)
	{
		// Disable collision and input on clients
		SetActorEnableCollision(false);
	}
}

void AROMonsterBase::Multicast_OnDeath_Implementation(AActor* Killer)
{
	// Play death VFX/SFX on all clients
	// Disable collision
	SetActorEnableCollision(false);

	// Set a timer to destroy the actor after death animation
	FTimerHandle DestroyTimer;
	GetWorldTimerManager().SetTimer(DestroyTimer, [this]()
	{
		if (HasAuthority())
		{
			Destroy();
		}
	}, 5.0f, false);
}

void AROMonsterBase::GenerateLoot()
{
	if (!HasAuthority())
	{
		return;
	}

	const FVector DeathLocation = GetActorLocation();

	// Use the LootManager world subsystem to spawn loot pickup actors
	UWorld* World = GetWorld();
	if (World)
	{
		UROLootManager* LootManager = World->GetSubsystem<UROLootManager>();
		if (LootManager)
		{
			AActor* Killer = GetHighestThreatTarget();
			LootManager->GenerateLoot(MonsterID, Killer, DeathLocation);
		}
	}
}

void AROMonsterBase::DistributeExp()
{
	if (!HasAuthority())
	{
		return;
	}

	// Calculate total damage dealt to determine shares
	float TotalDamage = 0.0f;
	for (const auto& Pair : ThreatTable)
	{
		TotalDamage += Pair.Value;
	}

	if (TotalDamage <= 0.0f)
	{
		return;
	}

	// Distribute EXP proportionally based on damage dealt
	for (const auto& Pair : ThreatTable)
	{
		AActor* Attacker = Pair.Key.Get();
		if (!Attacker)
		{
			continue;
		}

		const float DamageShare = Pair.Value / TotalDamage;
		const int64 BaseExpShare = FMath::RoundToInt64(BaseExpReward * DamageShare);
		const int64 JobExpShare = FMath::RoundToInt64(JobExpReward * DamageShare);

		// Distribute EXP via the character's leveling component
		UROLevelingComponent* LevelingComp = Attacker->FindComponentByClass<UROLevelingComponent>();
		if (LevelingComp)
		{
			LevelingComp->AddBaseExp(BaseExpShare);
			LevelingComp->AddJobExp(JobExpShare);
		}

		UE_LOG(LogTemp, Log, TEXT("Distributing %lld Base EXP and %lld Job EXP to %s"),
			BaseExpShare, JobExpShare, *Attacker->GetName());
	}
}
