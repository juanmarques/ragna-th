// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROServerValidation.h"

FROValidationResult UROServerValidation::ValidateMovement(
	const FVector& PreviousLocation,
	const FVector& NewLocation,
	float DeltaTime,
	float MaxMoveSpeed,
	const FString& PlayerNetID)
{
	if (DeltaTime <= 0.0f)
	{
		// Zero or negative DeltaTime is suspicious; still check teleport distance
		const float Distance = FVector::Dist(PreviousLocation, NewLocation);
		if (Distance > MaxTeleportDistance)
		{
			const FString Details = FString::Printf(
				TEXT("Teleport with invalid DeltaTime: Distance=%.1f (max=%.1f), DeltaTime=%.3f"),
				Distance, MaxTeleportDistance, DeltaTime);
			LogSuspiciousActivity(PlayerNetID, TEXT("TeleportHack"), Details, 3);
			return FROValidationResult::Failure(Details, 3);
		}
		return FROValidationResult::Success();
	}

	const float Distance = FVector::Dist(PreviousLocation, NewLocation);
	const float MaxAllowedDistance = MaxMoveSpeed * DeltaTime * SpeedToleranceMultiplier;

	// Check for teleport hacking (massive position jump)
	if (Distance > MaxTeleportDistance)
	{
		const FString Details = FString::Printf(
			TEXT("Teleport detected: Distance=%.1f (max=%.1f), From=%s To=%s"),
			Distance, MaxTeleportDistance,
			*PreviousLocation.ToString(), *NewLocation.ToString());

		LogSuspiciousActivity(PlayerNetID, TEXT("TeleportHack"), Details, 3);
		return FROValidationResult::Failure(Details, 3);
	}

	// Check for speed hacking
	if (Distance > MaxAllowedDistance)
	{
		const float ActualSpeed = Distance / DeltaTime;
		const FString Details = FString::Printf(
			TEXT("Speed hack: ActualSpeed=%.1f, MaxAllowed=%.1f (base=%.1f), Distance=%.1f, DeltaTime=%.3f"),
			ActualSpeed, MaxMoveSpeed * SpeedToleranceMultiplier, MaxMoveSpeed, Distance, DeltaTime);

		LogSuspiciousActivity(PlayerNetID, TEXT("SpeedHack"), Details, 2);
		return FROValidationResult::Failure(Details, 2);
	}

	return FROValidationResult::Success();
}

FROValidationResult UROServerValidation::ValidateDamage(
	int32 DamageAmount,
	int32 AttackerATK,
	int32 DefenderDEF,
	float SkillMultiplier,
	const FString& PlayerNetID)
{
	if (DamageAmount < 0)
	{
		const FString Details = TEXT("Negative damage amount");
		LogSuspiciousActivity(PlayerNetID, TEXT("InvalidDamage"), Details, 3);
		return FROValidationResult::Failure(Details, 3);
	}

	if (DamageAmount == 0)
	{
		// Zero damage is valid (miss, blocked, etc.)
		return FROValidationResult::Success();
	}

	// Calculate expected maximum damage
	// RO damage formula: ATK - DEF, modified by skill multiplier, elements, cards, etc.
	// We use a generous tolerance to account for all possible bonuses.
	const int32 RawDamage = FMath::Max(1, AttackerATK - DefenderDEF);
	const float MaxExpectedDamage = static_cast<float>(RawDamage) * SkillMultiplier * DamageToleranceMultiplier;

	if (static_cast<float>(DamageAmount) > MaxExpectedDamage)
	{
		const FString Details = FString::Printf(
			TEXT("Damage exceeds expected: Actual=%d, MaxExpected=%.0f (ATK=%d, DEF=%d, Skill=%.1f)"),
			DamageAmount, MaxExpectedDamage, AttackerATK, DefenderDEF, SkillMultiplier);

		LogSuspiciousActivity(PlayerNetID, TEXT("DamageHack"), Details, 2);
		return FROValidationResult::Failure(Details, 2);
	}

	return FROValidationResult::Success();
}

FROValidationResult UROServerValidation::ValidateItemOperation(
	int32 ItemID,
	int32 Amount,
	int32 MaxStack,
	int32 CurrentWeight,
	int32 MaxWeight,
	int32 ItemWeight,
	const FString& PlayerNetID)
{
	// Validate item ID
	if (ItemID <= 0)
	{
		const FString Details = FString::Printf(TEXT("Invalid ItemID: %d"), ItemID);
		LogSuspiciousActivity(PlayerNetID, TEXT("InvalidItem"), Details, 2);
		return FROValidationResult::Failure(Details, 2);
	}

	// Validate amount
	if (Amount <= 0)
	{
		const FString Details = FString::Printf(TEXT("Invalid amount: %d for ItemID %d"), Amount, ItemID);
		LogSuspiciousActivity(PlayerNetID, TEXT("InvalidItemAmount"), Details, 2);
		return FROValidationResult::Failure(Details, 2);
	}

	// Validate stack limit
	if (Amount > MaxStack)
	{
		const FString Details = FString::Printf(
			TEXT("Stack overflow: Amount=%d, MaxStack=%d for ItemID %d"),
			Amount, MaxStack, ItemID);
		LogSuspiciousActivity(PlayerNetID, TEXT("StackOverflow"), Details, 2);
		return FROValidationResult::Failure(Details, 2);
	}

	// Validate weight capacity
	const int32 TotalItemWeight = Amount * ItemWeight;
	if (CurrentWeight + TotalItemWeight > MaxWeight)
	{
		const FString Details = FString::Printf(
			TEXT("Weight exceeded: Current=%d, Adding=%d, Max=%d for ItemID %d"),
			CurrentWeight, TotalItemWeight, MaxWeight, ItemID);
		// Weight overflow is not suspicious, just a normal limit
		return FROValidationResult::Failure(Details, 0);
	}

	return FROValidationResult::Success();
}

FROValidationResult UROServerValidation::ValidateCooldown(
	float LastUseTime,
	float CurrentTime,
	float RequiredCooldown,
	const FString& ActionName,
	const FString& PlayerNetID)
{
	if (LastUseTime <= 0.0f)
	{
		// Never used before, always valid
		return FROValidationResult::Success();
	}

	const float ElapsedTime = CurrentTime - LastUseTime;
	const float AdjustedCooldown = RequiredCooldown - CooldownToleranceSeconds;

	if (ElapsedTime < AdjustedCooldown)
	{
		const float RemainingCooldown = AdjustedCooldown - ElapsedTime;
		const FString Details = FString::Printf(
			TEXT("Cooldown bypass: Action=%s, Elapsed=%.2fs, Required=%.2fs, Remaining=%.2fs"),
			*ActionName, ElapsedTime, RequiredCooldown, RemainingCooldown);

		LogSuspiciousActivity(PlayerNetID, TEXT("CooldownBypass"), Details, 2);
		return FROValidationResult::Failure(Details, 2);
	}

	return FROValidationResult::Success();
}

void UROServerValidation::LogSuspiciousActivity(
	const FString& PlayerNetID,
	const FString& ActivityType,
	const FString& Details,
	int32 Severity)
{
	const TCHAR* SeverityLabel;
	switch (Severity)
	{
	case 0:  SeverityLabel = TEXT("INFO"); break;
	case 1:  SeverityLabel = TEXT("WARNING"); break;
	case 2:  SeverityLabel = TEXT("SUSPICIOUS"); break;
	case 3:  SeverityLabel = TEXT("CHEAT"); break;
	default: SeverityLabel = TEXT("UNKNOWN"); break;
	}

	UE_LOG(LogTemp, Warning, TEXT("[ANTICHEAT][%s] Player=%s, Type=%s, Details=%s"),
		SeverityLabel, *PlayerNetID, *ActivityType, *Details);

	// TODO: In production, write to a database table for GM review:
	// INSERT INTO anticheat_logs (player_net_id, activity_type, details, severity, timestamp)
	// VALUES (...);

	// TODO: For severity >= 3, auto-kick or auto-ban the player.
	// TODO: Send notification to online GMs.
}
