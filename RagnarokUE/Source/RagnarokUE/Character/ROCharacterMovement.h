// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ROCharacterMovement.generated.h"

class UROStatsComponent;

/**
 * UROCharacterMovement
 * Custom character movement component implementing RO-style click-to-move.
 * Uses NavMesh pathfinding for movement and factors in AGI for speed.
 */
UCLASS(ClassGroup=(RagnarokUE))
class RAGNAROKUE_API UROCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UROCharacterMovement();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// ---- Movement Properties ----

	/** Whether the character is currently moving to a click destination. */
	UPROPERTY(BlueprintReadOnly, Category="RO Movement")
	bool bIsMovingToDestination;

	/** The target destination for click-to-move. */
	UPROPERTY(BlueprintReadOnly, Category="RO Movement")
	FVector MoveDestination;

	/** How close to the destination before we consider it reached. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RO Movement")
	float MoveAcceptanceRadius;

	/** Base movement speed (before AGI bonuses). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RO Movement")
	float BaseMovementSpeed;

	/** Bonus movement speed from buffs (additive percentage, e.g. 0.25 = +25%). */
	UPROPERTY(BlueprintReadWrite, Category="RO Movement")
	float MovementSpeedBonusPercent;

	// ---- Functions ----

	/** Start moving to a world location using NavMesh pathfinding. */
	UFUNCTION(BlueprintCallable, Category="RO Movement")
	void MoveToLocation(FVector Destination);

	/** Stop all movement immediately. */
	UFUNCTION(BlueprintCallable, Category="RO Movement")
	void StopMovementCommand();

	/** Is the character currently moving to a destination? */
	UFUNCTION(BlueprintCallable, Category="RO Movement")
	bool IsMoving() const;

	/** Override to factor in AGI bonuses and movement buffs. */
	virtual float GetMaxSpeed() const override;

protected:
	/** Cached reference to the stats component (resolved in BeginPlay). */
	UPROPERTY()
	TWeakObjectPtr<UROStatsComponent> CachedStatsComponent;

	virtual void BeginPlay() override;

private:
	/** Request a move via the AI navigation system. */
	void RequestNavMeshMove();
};
