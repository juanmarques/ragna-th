// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROCharacterBase.generated.h"

class UROStatsComponent;
class UROJobComponent;
class UROLevelingComponent;
class UROCharacterMovement;
class UROAbilitySystemComponent;
class UROAttributeSet;
class USpringArmComponent;
class UCameraComponent;
class UGameplayAbility;

/**
 * AROCharacterBase
 * Main player character for Ragnarok Online UE5 recreation.
 * Integrates the stats, job, and leveling components with the Gameplay Ability System.
 * Features an isometric-style camera and RO damage pipeline.
 */
UCLASS(ClassGroup=(RagnarokUE))
class RAGNAROKUE_API AROCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AROCharacterBase(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	// ---- IAbilitySystemInterface ----
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// ---- Components ----

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UROStatsComponent> StatsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UROJobComponent> JobComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UROLevelingComponent> LevelingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UROAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UROAttributeSet> AttributeSet;

	// ---- Camera ----

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<USpringArmComponent> CameraArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	// ---- Replicated State ----

	UPROPERTY(ReplicatedUsing=OnRep_CurrentHP, BlueprintReadOnly, Category="Character|Vitals")
	int32 CurrentHP;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentSP, BlueprintReadOnly, Category="Character|Vitals")
	int32 CurrentSP;

	UPROPERTY(ReplicatedUsing=OnRep_MaxHP, BlueprintReadOnly, Category="Character|Vitals")
	int32 MaxHP;

	UPROPERTY(ReplicatedUsing=OnRep_MaxSP, BlueprintReadOnly, Category="Character|Vitals")
	int32 MaxSP;

	UPROPERTY(ReplicatedUsing=OnRep_bIsDead, BlueprintReadOnly, Category="Character|State")
	bool bIsDead;

	UPROPERTY(ReplicatedUsing=OnRep_bIsSitting, BlueprintReadOnly, Category="Character|State")
	bool bIsSitting;

	// ---- Zone Flags ----

	/** Whether PvP is enabled for this character (set by zone rules). */
	UPROPERTY(BlueprintReadWrite, Category="Character|Zone")
	bool bPvPEnabled;

	/** Whether teleport is blocked (set by zone rules). */
	UPROPERTY(BlueprintReadWrite, Category="Character|Zone")
	bool bTeleportBlocked;

	/** Whether the character is currently in a town zone. */
	UPROPERTY(BlueprintReadWrite, Category="Character|Zone")
	bool bInTown;

	/** Whether the character is in a guild-controlled zone, and which guild owns it. */
	UPROPERTY(BlueprintReadWrite, Category="Character|Zone")
	bool bInGuildZone;

	UPROPERTY(BlueprintReadWrite, Category="Character|Zone")
	int32 GuildZoneOwnerID;

	// ---- Save Point ----

	/** Map ID of the character's saved respawn point. */
	UPROPERTY(BlueprintReadWrite, Category="Character|SavePoint")
	FName SavedSpawnMapID;

	/** World location of the character's saved respawn point. */
	UPROPERTY(BlueprintReadWrite, Category="Character|SavePoint")
	FVector SavedSpawnLocation;

	// ---- Character Actions ----

	/** Handle character death. Stops movement, disables input, plays death animation. */
	UFUNCTION(BlueprintCallable, Category="Character")
	void Die();

	/** Respawn character at a save point. Restores HP/SP. */
	UFUNCTION(BlueprintCallable, Category="Character")
	void Respawn(FVector RespawnLocation);

	/** Sit down (RO sitting for HP/SP regen). */
	UFUNCTION(BlueprintCallable, Category="Character")
	void SitDown();

	/** Stand up from sitting. */
	UFUNCTION(BlueprintCallable, Category="Character")
	void StandUp();

	/** Server RPC: request sit down from client. */
	UFUNCTION(Server, Reliable)
	void ServerSitDown();

	/** Server RPC: request stand up from client. */
	UFUNCTION(Server, Reliable)
	void ServerStandUp();

	/** Override TakeDamage for RO damage pipeline. */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	// ---- Ability System ----

	/** Grant a gameplay ability to this character. */
	UFUNCTION(BlueprintCallable, Category="Abilities")
	void GrantAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level = 1);

	/** Remove a gameplay ability from this character. */
	UFUNCTION(BlueprintCallable, Category="Abilities")
	void RemoveAbility(TSubclassOf<UGameplayAbility> AbilityClass);

	/** Default abilities granted to all characters. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	/** Job-specific abilities mapped by job class (configured in Blueprint). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Abilities")
	TMap<EROJobClass, TSubclassOf<UGameplayAbility>> JobAbilities;

	// ---- Getters ----

	UFUNCTION(BlueprintCallable, Category="Character")
	UROCharacterMovement* GetROMovementComponent() const;

protected:
	UFUNCTION()
	void OnRep_CurrentHP();

	UFUNCTION()
	void OnRep_CurrentSP();

	UFUNCTION()
	void OnRep_MaxHP();

	UFUNCTION()
	void OnRep_MaxSP();

	UFUNCTION()
	void OnRep_bIsDead();

	UFUNCTION()
	void OnRep_bIsSitting();

	/** Initialize the ability system component. Called on server in PossessedBy. */
	void InitializeAbilitySystem();

	/** Grant default abilities based on current job class. */
	void GrantDefaultAbilities();

	/** Callback when base level changes to update HP/SP caps. */
	UFUNCTION()
	void OnBaseLevelUp(int32 NewLevel);

	/** Callback when job changes to grant new abilities. */
	UFUNCTION()
	void OnJobChanged(EROJobClass OldJob, EROJobClass NewJob);

	/** Synchronize MaxHP/MaxSP from stats component and clamp current values. */
	void SyncVitalsFromStats();

	/** Whether ability system has been initialized. */
	bool bAbilitySystemInitialized;
};
