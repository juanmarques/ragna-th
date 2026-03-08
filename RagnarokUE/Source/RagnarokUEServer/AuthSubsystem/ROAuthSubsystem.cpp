// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROAuthSubsystem.h"
#include "Misc/SecureHash.h"
#include "Misc/Guid.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformMisc.h"

void UROAuthSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// FIX 15: Scan existing records to find max ID and avoid collisions on restart
	NextAccountID = 1;
	for (const auto& Pair : AccountsByUsername)
	{
		NextAccountID = FMath::Max(NextAccountID, Pair.Value.AccountID + 1);
	}

	// Start periodic cleanup of expired sessions (every 5 minutes)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(SessionCleanupTimerHandle, this,
			&UROAuthSubsystem::CleanupExpiredSessions, 300.0f, true);
	}

	UE_LOG(LogTemp, Log, TEXT("ROAuthSubsystem: Initialized"));
}

void UROAuthSubsystem::Deinitialize()
{
	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SessionCleanupTimerHandle);
	}

	AccountsByUsername.Empty();
	AccountIDToUsername.Empty();
	ActiveSessions.Empty();

	Super::Deinitialize();
}

FRORegistrationResult UROAuthSubsystem::RegisterAccount(const FString& Username, const FString& Password, const FString& Email)
{
	FRORegistrationResult Result;

	// Validate username
	if (!IsValidUsername(Username))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Invalid username. Must be 3-24 alphanumeric characters.");
		return Result;
	}

	// Validate password
	if (!IsValidPassword(Password))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Invalid password. Must be at least 6 characters.");
		return Result;
	}

	// Check for duplicate username
	if (AccountsByUsername.Contains(Username.ToLower()))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Username already exists.");
		return Result;
	}

	// Create account with per-account salt for password hashing
	FROAccountData Account;
	Account.AccountID = NextAccountID++;
	Account.Username = Username.ToLower();
	Account.PasswordSalt = GenerateSalt();
	Account.PasswordHash = HashPassword(Password, Account.PasswordSalt);
	Account.Email = Email;
	Account.CreatedAt = FDateTime::Now();
	Account.LastLoginAt = FDateTime::MinValue();
	Account.LockoutStartedAt = FDateTime::MinValue();
	Account.bIsBanned = false;
	Account.LoginAttempts = 0;

	AccountsByUsername.Add(Account.Username, Account);
	AccountIDToUsername.Add(Account.AccountID, Account.Username);

	Result.bSuccess = true;
	Result.AccountID = Account.AccountID;

	UE_LOG(LogTemp, Log, TEXT("ROAuthSubsystem: Account registered - ID=%d, Username=%s"),
		Account.AccountID, *Account.Username);

	return Result;
}

FROAuthResult UROAuthSubsystem::Authenticate(const FString& Username, const FString& Password, const FString& IPAddress)
{
	FROAuthResult Result;

	const FString LowerUsername = Username.ToLower();
	FROAccountData* Account = AccountsByUsername.Find(LowerUsername);

	if (!Account)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Invalid username or password.");

		OnAuthFailure.Broadcast(Result);
		return Result;
	}

	// Check ban
	if (Account->bIsBanned)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("Account is banned: %s"), *Account->BanReason);

		OnAuthFailure.Broadcast(Result);
		return Result;
	}

	// FIX 3: Check lockout using dedicated LockoutStartedAt field instead of LastLoginAt
	if (Account->LoginAttempts >= MaxLoginAttempts)
	{
		const FTimespan TimeSinceLockout = FDateTime::Now() - Account->LockoutStartedAt;
		if (TimeSinceLockout.GetTotalMinutes() < LockoutDurationMinutes)
		{
			Result.bSuccess = false;
			Result.ErrorMessage = TEXT("Account temporarily locked due to too many failed login attempts.");

			// Do NOT update LockoutStartedAt here - that would reset the lockout timer
			OnAuthFailure.Broadcast(Result);
			return Result;
		}
		// Lockout expired, reset attempts
		Account->LoginAttempts = 0;
	}

	// FIX 1: Verify password using the account's stored salt
	const FString InputHash = HashPassword(Password, Account->PasswordSalt);
	if (InputHash != Account->PasswordHash)
	{
		Account->LoginAttempts++;

		// FIX 3: Only set LockoutStartedAt when attempts first reach the threshold
		if (Account->LoginAttempts >= MaxLoginAttempts)
		{
			Account->LockoutStartedAt = FDateTime::Now();
		}

		// Do NOT update LastLoginAt on failed attempts - only on successful login

		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Invalid username or password.");

		UE_LOG(LogTemp, Warning, TEXT("ROAuthSubsystem: Failed login for '%s' (attempt %d/%d)"),
			*LowerUsername, Account->LoginAttempts, MaxLoginAttempts);

		OnAuthFailure.Broadcast(Result);
		return Result;
	}

	// Authentication successful
	Account->LoginAttempts = 0;
	Account->LastLoginAt = FDateTime::Now();

	// Generate session token
	FROSessionData Session;
	Session.SessionToken = GenerateSessionToken();
	Session.AccountID = Account->AccountID;
	Session.CreatedAt = FDateTime::Now();
	Session.ExpiresAt = Session.CreatedAt + FTimespan::FromHours(SessionExpirationHours);
	Session.IPAddress = IPAddress;

	ActiveSessions.Add(Session.SessionToken, Session);

	Result.bSuccess = true;
	Result.AccountID = Account->AccountID;
	Result.SessionToken = Session.SessionToken;

	UE_LOG(LogTemp, Log, TEXT("ROAuthSubsystem: Authentication successful - AccountID=%d, Username=%s, IP=%s"),
		Account->AccountID, *LowerUsername, *IPAddress);

	OnAuthSuccess.Broadcast(Result);
	return Result;
}

bool UROAuthSubsystem::ValidateSessionToken(const FString& SessionToken)
{
	const FROSessionData* Session = ActiveSessions.Find(SessionToken);
	if (!Session)
	{
		return false;
	}

	// FIX 12: Remove expired sessions inline instead of leaving them until periodic cleanup
	if (FDateTime::Now() > Session->ExpiresAt)
	{
		ActiveSessions.Remove(SessionToken);
		return false;
	}

	return true;
}

int32 UROAuthSubsystem::GetAccountIDFromSession(const FString& SessionToken) const
{
	const FROSessionData* Session = ActiveSessions.Find(SessionToken);
	if (Session && FDateTime::Now() <= Session->ExpiresAt)
	{
		return Session->AccountID;
	}
	return 0;
}

bool UROAuthSubsystem::ValidateAndGetAccountID(const FString& SessionToken, int32& OutAccountID)
{
	OutAccountID = 0;

	FROSessionData* Session = ActiveSessions.Find(SessionToken);
	if (!Session)
	{
		return false;
	}

	// FIX 2: Atomically validate and get account ID in one operation.
	// If expired, remove immediately and return false - no TOCTOU window.
	if (FDateTime::Now() > Session->ExpiresAt)
	{
		ActiveSessions.Remove(SessionToken);
		return false;
	}

	OutAccountID = Session->AccountID;
	return true;
}

void UROAuthSubsystem::InvalidateSession(const FString& SessionToken)
{
	if (ActiveSessions.Remove(SessionToken) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("ROAuthSubsystem: Session invalidated"));
	}
}

void UROAuthSubsystem::InvalidateAllSessions(int32 AccountID)
{
	TArray<FString> TokensToRemove;
	for (const auto& Pair : ActiveSessions)
	{
		if (Pair.Value.AccountID == AccountID)
		{
			TokensToRemove.Add(Pair.Key);
		}
	}

	for (const FString& Token : TokensToRemove)
	{
		ActiveSessions.Remove(Token);
	}

	UE_LOG(LogTemp, Log, TEXT("ROAuthSubsystem: Invalidated %d sessions for AccountID=%d"),
		TokensToRemove.Num(), AccountID);
}

bool UROAuthSubsystem::ChangePassword(int32 AccountID, const FString& OldPassword, const FString& NewPassword)
{
	const FString* Username = AccountIDToUsername.Find(AccountID);
	if (!Username)
	{
		return false;
	}

	FROAccountData* Account = AccountsByUsername.Find(*Username);
	if (!Account)
	{
		return false;
	}

	// Verify old password using the account's existing salt
	if (HashPassword(OldPassword, Account->PasswordSalt) != Account->PasswordHash)
	{
		return false;
	}

	// Validate new password
	if (!IsValidPassword(NewPassword))
	{
		return false;
	}

	// Generate a new salt for the new password
	Account->PasswordSalt = GenerateSalt();
	Account->PasswordHash = HashPassword(NewPassword, Account->PasswordSalt);

	// Invalidate all existing sessions for security
	InvalidateAllSessions(AccountID);

	UE_LOG(LogTemp, Log, TEXT("ROAuthSubsystem: Password changed for AccountID=%d"), AccountID);
	return true;
}

void UROAuthSubsystem::BanAccount(int32 AccountID, const FString& Reason)
{
	const FString* Username = AccountIDToUsername.Find(AccountID);
	FROAccountData* Account = Username ? AccountsByUsername.Find(*Username) : nullptr;
	if (Account)
	{
		Account->bIsBanned = true;
		Account->BanReason = Reason;

		// Force logout
		InvalidateAllSessions(AccountID);

		UE_LOG(LogTemp, Log, TEXT("ROAuthSubsystem: Account %d banned - Reason: %s"), AccountID, *Reason);
	}
}

void UROAuthSubsystem::UnbanAccount(int32 AccountID)
{
	const FString* Username = AccountIDToUsername.Find(AccountID);
	FROAccountData* Account = Username ? AccountsByUsername.Find(*Username) : nullptr;
	if (Account)
	{
		Account->bIsBanned = false;
		Account->BanReason.Empty();

		UE_LOG(LogTemp, Log, TEXT("ROAuthSubsystem: Account %d unbanned"), AccountID);
	}
}

bool UROAuthSubsystem::IsAccountBanned(int32 AccountID) const
{
	const FString* Username = AccountIDToUsername.Find(AccountID);
	const FROAccountData* Account = Username ? AccountsByUsername.Find(*Username) : nullptr;
	if (Account)
	{
		return Account->bIsBanned;
	}
	return false;
}

FString UROAuthSubsystem::GenerateSalt()
{
	// Generate 16 random bytes as 32 hex characters
	FString Salt;
	Salt.Reserve(32);
	for (int32 i = 0; i < 16; ++i)
	{
		Salt += FString::Printf(TEXT("%02x"), FMath::RandRange(0, 255));
	}
	return Salt;
}

FString UROAuthSubsystem::HashPassword(const FString& Password, const FString& Salt)
{
	// FIX 1: Salted SHA-256 hash. Salt is prepended to the password before hashing.
	// FPlatformMisc::GetSha256Hash converts the FString to UTF-8 internally and
	// returns a lowercase hex-encoded SHA-256 digest.
	// In production, consider bcrypt/scrypt/argon2 for better resistance to brute-force attacks.
	const FString Combined = Salt + Password;
	return FPlatformMisc::GetSha256Hash(*Combined);
}

FString UROAuthSubsystem::GenerateSessionToken()
{
	// Generate a unique token using two GUIDs for sufficient entropy
	const FGuid Guid1 = FGuid::NewGuid();
	const FGuid Guid2 = FGuid::NewGuid();
	return Guid1.ToString(EGuidFormats::DigitsWithHyphens) + Guid2.ToString(EGuidFormats::Digits);
}

bool UROAuthSubsystem::IsValidUsername(const FString& Username)
{
	if (Username.Len() < 3 || Username.Len() > 24)
	{
		return false;
	}

	for (const TCHAR& Char : Username)
	{
		if (!FChar::IsAlnum(Char) && Char != '_')
		{
			return false;
		}
	}

	return true;
}

bool UROAuthSubsystem::IsValidPassword(const FString& Password)
{
	return Password.Len() >= 6;
}

void UROAuthSubsystem::CleanupExpiredSessions()
{
	const FDateTime Now = FDateTime::Now();
	TArray<FString> ExpiredTokens;

	for (const auto& Pair : ActiveSessions)
	{
		if (Now > Pair.Value.ExpiresAt)
		{
			ExpiredTokens.Add(Pair.Key);
		}
	}

	for (const FString& Token : ExpiredTokens)
	{
		ActiveSessions.Remove(Token);
	}

	if (ExpiredTokens.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("ROAuthSubsystem: Cleaned up %d expired sessions"), ExpiredTokens.Num());
	}
}
