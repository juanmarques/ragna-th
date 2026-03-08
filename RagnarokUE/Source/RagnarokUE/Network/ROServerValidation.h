// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "ROServerValidation.generated.h"

class UROAttributeSet;

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
	 * Reads ATK and DEF from the attacker's and defender's UROAttributeSet directly
	 * to ensure server-authoritative values are used (FIX 5).
	 * @param DamageAmount      The reported damage.
	 * @param Attacker          The attacking actor (must have an AbilitySystemComponent with UROAttributeSet).
	 * @param Defender          The defending actor (must have an AbilitySystemComponent with UROAttributeSet).
	 * @param SkillMultiplier   Skill damage multiplier (1.0 for normal attacks).
	 * @param PlayerNetID       For logging purposes.
	 * @return Validation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	static FROValidationResult ValidateDamage(
		int32 DamageAmount,
		AActor* Attacker,
		AActor* Defender,
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
	 * Current time is obtained from the world internally to prevent caller manipulation (FIX 6).
	 * @param World                 The world context (used to get authoritative server time).
	 * @param LastUseTime           World time of last use (from GetTimeSeconds).
	 * @param RequiredCooldown      The cooldown duration in seconds.
	 * @param ActionName            Name of the action for logging.
	 * @param PlayerNetID           For logging purposes.
	 * @return Validation result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	static FROValidationResult ValidateCooldown(
		const UWorld* World,
		float LastUseTime,
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

	/** Damage tolerance multiplier. 2.0x accounts for critical hits, elemental advantages,
	 * card bonuses, buff stacking, and enchant effects while still catching obvious cheats. */
	static constexpr float DamageToleranceMultiplier = 2.0f;

	/** Minimum cooldown tolerance in seconds (network latency allowance). */
	static constexpr float CooldownToleranceSeconds = 0.1f;
};
