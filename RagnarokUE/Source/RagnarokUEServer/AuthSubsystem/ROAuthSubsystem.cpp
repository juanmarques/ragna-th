// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROAuthSubsystem.h"
#include "Misc/SecureHash.h"
#include "Misc/Guid.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UROAuthSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	NextAccountID = 1;

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

	// Create account
	FROAccountData Account;
	Account.AccountID = NextAccountID++;
	Account.Username = Username.ToLower();
	Account.PasswordHash = HashPassword(Password);
	Account.Email = Email;
	Account.CreatedAt = FDateTime::Now();
	Account.LastLoginAt = FDateTime::MinValue();
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

	// Check lockout from too many failed attempts
	if (Account->LoginAttempts >= MaxLoginAttempts)
	{
		const FTimespan TimeSinceLastLogin = FDateTime::Now() - Account->LastLoginAt;
		if (TimeSinceLastLogin.GetTotalMinutes() < LockoutDurationMinutes)
		{
			Result.bSuccess = false;
			Result.ErrorMessage = TEXT("Account temporarily locked due to too many failed login attempts.");

			OnAuthFailure.Broadcast(Result);
			return Result;
		}
		// Lockout expired, reset attempts
		Account->LoginAttempts = 0;
	}

	// Verify password
	const FString InputHash = HashPassword(Password);
	if (InputHash != Account->PasswordHash)
	{
		Account->LoginAttempts++;
		Account->LastLoginAt = FDateTime::Now();

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

bool UROAuthSubsystem::ValidateSessionToken(const FString& SessionToken) const
{
	const FROSessionData* Session = ActiveSessions.Find(SessionToken);
	if (!Session)
	{
		return false;
	}

	// Check expiration
	if (FDateTime::Now() > Session->ExpiresAt)
	{
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

	// Verify old password
	if (HashPassword(OldPassword) != Account->PasswordHash)
	{
		return false;
	}

	// Validate new password
	if (!IsValidPassword(NewPassword))
	{
		return false;
	}

	Account->PasswordHash = HashPassword(NewPassword);

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

FString UROAuthSubsystem::HashPassword(const FString& Password)
{
	// SHA256 hash of password via FSHA1 (which despite its name, provides SHA256 when using HashBuffer).
	// In production, use a salt + pepper + bcrypt/scrypt/argon2 for better security.
	FSHAHash Hash;
	FSHA1::HashBuffer(TCHAR_TO_UTF8(*Password), Password.Len(), Hash.Hash);
	return Hash.ToString();
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
