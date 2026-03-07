// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RagnarokUE/Data/ROEnums.h"
#include "RagnarokUE/Data/ROStructs.h"
#include "ROEmperiumActor.generated.h"

class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmperiumDestroyedDelegate, int32, CastleID, int32, AttackingGuildID);

/**
 * AROEmperiumActor
 * The Emperium crystal that guilds must destroy to capture a castle during War of Emperium.
 * Only accepts normal physical attacks (no skills).
 * Only non-guild members can attack it.
 * Can be protected by Safety Wall, Pneuma, and Sanctuary.
 */
UCLASS(ClassGroup=(RagnarokUE), Blueprintable)
class RAGNAROKUE_API AROEmperiumActor : public AActor
{
	GENERATED_BODY()

public:
	AROEmperiumActor();

	// ---- Emperium Properties ----

	/** Current HP of the Emperium. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Emperium")
	int32 HP = 0;

	/** Maximum HP (based on guild investment). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Emperium")
	int32 MaxHP = 50000;

	/** Castle ID this Emperium belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emperium")
	int32 CastleID = 0;

	/** Guild ID that owns this Emperium. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Emperium")
	int32 OwnerGuildID = 0;

	/** Whether the Emperium is currently vulnerable (WoE must be active). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Emperium")
	bool bIsVulnerable = false;

	/** Whether a Safety Wall is protecting the Emperium. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Emperium")
	bool bHasSafetyWall = false;

	/** Whether Pneuma is protecting the Emperium. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Emperium")
	bool bHasPneuma = false;

	/** Whether Sanctuary is active on the Emperium. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Emperium")
	bool bHasSanctuary = false;

	// ---- Functions ----

	/** Initialize the Emperium with castle and guild data. */
	UFUNCTION(BlueprintCallable, Category = "Emperium")
	void InitializeEmperium(int32 InCastleID, int32 InOwnerGuildID, int32 InMaxHP);

	/** Reset HP to maximum (used when castle ownership changes). */
	UFUNCTION(BlueprintCallable, Category = "Emperium")
	void ResetHP();

	/** Set whether the Emperium is vulnerable (called by WoE manager). */
	UFUNCTION(BlueprintCallable, Category = "Emperium")
	void SetVulnerable(bool bVulnerable);

	/** Apply protection effects. */
	UFUNCTION(BlueprintCallable, Category = "Emperium")
	void SetSafetyWall(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "Emperium")
	void SetPneuma(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "Emperium")
	void SetSanctuary(bool bActive);

	/** Get HP as a percentage (0.0 - 1.0). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Emperium")
	float GetHPPercent() const;

	/** Check if the Emperium is alive. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Emperium")
	bool IsAlive() const;

	/**
	 * Apply damage to the Emperium.
	 * Only accepts normal attacks (bIsSkillDamage must be false).
	 * Only from non-guild members.
	 * @param DamageAmount   Amount of damage to apply.
	 * @param AttackerGuildID Guild of the attacking player.
	 * @param bIsSkillDamage Whether this damage comes from a skill (rejected).
	 * @param bIsRanged      Whether this is a ranged attack (blocked by Pneuma).
	 * @return Actual damage dealt.
	 */
	UFUNCTION(BlueprintCallable, Category = "Emperium")
	int32 ApplyEmperiumDamage(int32 DamageAmount, int32 AttackerGuildID, bool bIsSkillDamage, bool bIsRanged);

	/** UE5 TakeDamage override for integration with the engine damage system. */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	// ---- Delegates ----

	/** Broadcast when the Emperium is destroyed. */
	UPROPERTY(BlueprintAssignable, Category = "Emperium")
	FOnEmperiumDestroyedDelegate OnEmperiumDestroyed;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Root scene component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Emperium")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Collision for the Emperium crystal. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Emperium")
	TObjectPtr<USphereComponent> CollisionComponent;

	/** Handle the Emperium being destroyed. */
	void HandleDestruction(int32 AttackingGuildID);
};
