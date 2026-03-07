// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ROServerValidation.generated.h"

/** Result of a server-side validation check. */
USTRUCT(BlueprintType)
struct RAGNAROKUE_API FROValidationResult
{
	GENERATED_BODY()

	/** Whether the action is valid. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
	bool bIsValid = true;

	/** Reason for failure (empty if valid). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
	FString FailureReason;

	/** Severity level (0=info, 1=warning, 2=suspicious, 3=cheat). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
	int32 Severity = 0;

	static FROValidationResult Success()
	{
		FROValidationResult Result;
		Result.bIsValid = true;
		return Result;
	}

	static FROValidationResult Failure(const FString& Reason, int32 InSeverity = 1)
	{
		FROValidationResult Result;
		Result.bIsValid = false;
		Result.FailureReason = Reason;
		Result.Severity = InSeverity;
		return Result;
	}
};

/**
 * UROServerValidation
 * Static utility class for server-side validation of player actions.
 * All methods are static and can be called from any server-side code.
 * Used to detect cheating, speed hacking, and invalid operations.
 */
UCLASS()
class RAGNAROKUE_API UROServerValidation : public UObject
{
	GENERATED_BODY()

public:
	// ---- Movement Validation ----

	/**
	 * Validate a player's movement to detect speed hacking or teleport hacking.
	 * @param PreviousLocation  The player's last known location.
	 * @param NewLocation       The player's reported new location.
	 * @param DeltaTime         Time since last position update.
	 * @param MaxMoveSpeed      The player's maximum allowed move speed.
	 * @param PlayerNetID       For logging purposes.
	 * @return Validation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	static FROValidationResult ValidateMovement(
		const FVector& PreviousLocation,
		const FVector& NewLocation,
		float DeltaTime,
		float MaxMoveSpeed,
		const FString& PlayerNetID);

	// ---- Damage Validation ----

	/**
	 * Validate a damage amount to ensure it falls within expected bounds.
	 * @param DamageAmount      The reported damage.
	 * @param AttackerATK       The attacker's ATK value.
	 * @param DefenderDEF       The defender's DEF value.
	 * @param SkillMultiplier   Skill damage multiplier (1.0 for normal attacks).
	 * @param PlayerNetID       For logging purposes.
	 * @return Validation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	static FROValidationResult ValidateDamage(
		int32 DamageAmount,
		int32 AttackerATK,
		int32 DefenderDEF,
		float SkillMultiplier,
		const FString& PlayerNetID);

	// ---- Item Validation ----

	/**
	 * Validate an item operation (pickup, use, drop, trade).
	 * Checks item existence, stack limits, weight capacity, and trade restrictions.
	 * @param ItemID            The item being operated on.
	 * @param Amount            The amount involved.
	 * @param MaxStack          Maximum stack size for this item type.
	 * @param CurrentWeight     Player's current inventory weight.
	 * @param MaxWeight         Player's maximum carry weight.
	 * @param ItemWeight        Weight of a single unit of this item.
	 * @param PlayerNetID       For logging purposes.
	 * @return Validation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	static FROValidationResult ValidateItemOperation(
		int32 ItemID,
		int32 Amount,
		int32 MaxStack,
		int32 CurrentWeight,
		int32 MaxWeight,
		int32 ItemWeight,
		const FString& PlayerNetID);

	// ---- Cooldown Validation ----

	/**
	 * Validate that a skill or action cooldown has elapsed.
	 * @param LastUseTime           World time of last use.
	 * @param CurrentTime           Current world time.
	 * @param RequiredCooldown      The cooldown duration in seconds.
	 * @param ActionName            Name of the action for logging.
	 * @param PlayerNetID           For logging purposes.
	 * @return Validation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	static FROValidationResult ValidateCooldown(
		float LastUseTime,
		float CurrentTime,
		float RequiredCooldown,
		const FString& ActionName,
		const FString& PlayerNetID);

	// ---- Logging ----

	/**
	 * Log suspicious activity to the server log.
	 * In production, this would also write to a database for GM review.
	 * @param PlayerNetID   The player's network ID.
	 * @param ActivityType  Category of suspicious activity.
	 * @param Details       Detailed description.
	 * @param Severity      0=info, 1=warning, 2=suspicious, 3=cheat.
	 */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	static void LogSuspiciousActivity(
		const FString& PlayerNetID,
		const FString& ActivityType,
		const FString& Details,
		int32 Severity);

	// ---- Thresholds ----

	/** Speed tolerance multiplier (allows for some network jitter). */
	static constexpr float SpeedToleranceMultiplier = 1.5f;

	/** Maximum allowed teleport distance before flagging (in UE units). */
	static constexpr float MaxTeleportDistance = 5000.0f;

	/** Damage tolerance multiplier (allows for critical hits, elemental bonuses). */
	static constexpr float DamageToleranceMultiplier = 3.0f;

	/** Minimum cooldown tolerance in seconds (network latency allowance). */
	static constexpr float CooldownToleranceSeconds = 0.1f;
};
