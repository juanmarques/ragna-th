// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ROAuthSubsystem.generated.h"

/** Result of an authentication attempt. */
USTRUCT(BlueprintType)
struct FROAuthResult
{
	GENERATED_BODY()

	/** Whether authentication succeeded. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Auth")
	bool bSuccess = false;

	/** Account ID (valid only on success). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Auth")
	int32 AccountID = 0;

	/** Session token (valid only on success). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Auth")
	FString SessionToken;

	/** Error message (non-empty on failure). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Auth")
	FString ErrorMessage;
};

/** Result of an account registration attempt. */
USTRUCT(BlueprintType)
struct FRORegistrationResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Auth")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Auth")
	int32 AccountID = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Auth")
	FString ErrorMessage;
};

/** Stored account data. */
USTRUCT()
struct FROAccountData
{
	GENERATED_BODY()

	int32 AccountID = 0;
	FString Username;
	FString PasswordHash; // SHA-256 hash (salted)
	FString PasswordSalt; // Per-account random salt
	FString Email;
	FDateTime CreatedAt;
	FDateTime LastLoginAt;
	bool bIsBanned = false;
	FString BanReason;
	int32 LoginAttempts = 0;
	FDateTime LockoutStartedAt; // Timestamp when lockout began (set when LoginAttempts first reaches MaxLoginAttempts)
};

/** Active session data. */
USTRUCT()
struct FROSessionData
{
	GENERATED_BODY()

	FString SessionToken;
	int32 AccountID = 0;
	FDateTime CreatedAt;
	FDateTime ExpiresAt;
	FString IPAddress;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAuthSuccess, const FROAuthResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAuthFailure, const FROAuthResult&, Result);

/**
 * UROAuthSubsystem
 * Handles account registration, authentication with salted SHA-256 password hashing,
 * and session token generation/validation with atomic session operations.
 */
UCLASS()
class RAGNAROKUESERVER_API UROAuthSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- Registration ----

	/**
	 * Register a new account.
	 * @param Username  The desired username (3-24 alphanumeric characters).
	 * @param Password  The plaintext password (will be salted and hashed with SHA-256).
	 * @param Email     The email address for account recovery.
	 * @return Registration result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Auth")
	FRORegistrationResult RegisterAccount(const FString& Username, const FString& Password, const FString& Email);

	// ---- Authentication ----

	/**
	 * Authenticate a user with username and password.
	 * @param Username  The account username.
	 * @param Password  The plaintext password.
	 * @param IPAddress The connecting client's IP address.
	 * @return Authentication result with session token.
	 */
	UFUNCTION(BlueprintCallable, Category = "Auth")
	FROAuthResult Authenticate(const FString& Username, const FString& Password, const FString& IPAddress);

	// ---- Session Management ----

	/**
	 * Validate a session token. Removes expired sessions inline.
	 * @param SessionToken The token to validate.
	 * @return True if the session is valid and not expired.
	 */
	UFUNCTION(BlueprintCallable, Category = "Auth")
	bool ValidateSessionToken(const FString& SessionToken);

	/**
	 * Atomically validate a session token and retrieve the account ID.
	 * If the session is expired, it is removed immediately and the method returns false.
	 * Use this instead of separate ValidateSessionToken + GetAccountIDFromSession calls.
	 * @param SessionToken The token to validate.
	 * @param OutAccountID The account ID (only valid if return is true).
	 * @return True if the session is valid and not expired.
	 */
	UFUNCTION(BlueprintCallable, Category = "Auth")
	bool ValidateAndGetAccountID(const FString& SessionToken, int32& OutAccountID);

	/**
	 * Get the account ID associated with a session token.
	 * @param SessionToken The session token.
	 * @return The account ID, or 0 if invalid.
	 * @note Prefer ValidateAndGetAccountID for atomic validate+get.
	 */
	UFUNCTION(BlueprintCallable, Category = "Auth")
	int32 GetAccountIDFromSession(const FString& SessionToken) const;

	/**
	 * Invalidate a session token (logout).
	 * @param SessionToken The session token to invalidate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Auth")
	void InvalidateSession(const FString& SessionToken);

	/** Invalidate all sessions for an account (force logout everywhere). */
	UFUNCTION(BlueprintCallable, Category = "Auth")
	void InvalidateAllSessions(int32 AccountID);

	// ---- Account Management ----

	/** Change password for an account. */
	UFUNCTION(BlueprintCallable, Category = "Auth")
	bool ChangePassword(int32 AccountID, const FString& OldPassword, const FString& NewPassword);

	/** Ban an account. */
	UFUNCTION(BlueprintCallable, Category = "Auth")
	void BanAccount(int32 AccountID, const FString& Reason);

	/** Unban an account. */
	UFUNCTION(BlueprintCallable, Category = "Auth")
	void UnbanAccount(int32 AccountID);

	/** Check if an account is banned. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Auth")
	bool IsAccountBanned(int32 AccountID) const;

	// ---- Delegates ----

	UPROPERTY(BlueprintAssignable, Category = "Auth")
	FOnAuthSuccess OnAuthSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Auth")
	FOnAuthFailure OnAuthFailure;

	// ---- Configuration ----

	/** Session token expiration time in hours. */
	UPROPERTY(EditAnywhere, Category = "Auth")
	float SessionExpirationHours = 24.0f;

	/** Maximum failed login attempts before temporary lockout. */
	UPROPERTY(EditAnywhere, Category = "Auth")
	int32 MaxLoginAttempts = 5;

	/** Lockout duration in minutes after max failed attempts. */
	UPROPERTY(EditAnywhere, Category = "Auth")
	float LockoutDurationMinutes = 15.0f;

protected:
	/** In-memory account database (in production, use a real database). */
	TMap<FString, FROAccountData> AccountsByUsername;
	/** Maps AccountID -> Username key for safe lookup into AccountsByUsername (avoids dangling pointers). */
	TMap<int32, FString> AccountIDToUsername;

	/** Active sessions. */
	TMap<FString, FROSessionData> ActiveSessions;

	/** Next available account ID. */
	int32 NextAccountID = 1;

	/** Generate a cryptographic random salt (32 hex characters). */
	static FString GenerateSalt();

	/** Hash a password with SHA-256 using the provided salt. Salt is prepended before hashing. */
	static FString HashPassword(const FString& Password, const FString& Salt);

	/** Generate a unique session token. */
	static FString GenerateSessionToken();

	/** Validate username format. */
	static bool IsValidUsername(const FString& Username);

	/** Validate password strength. */
	static bool IsValidPassword(const FString& Password);

	/** Clean up expired sessions. Called periodically. */
	void CleanupExpiredSessions();

	/** Timer handle for session cleanup. */
	FTimerHandle SessionCleanupTimerHandle;
};
