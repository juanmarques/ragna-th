// Copyright Ragna-TH Project. All Rights Reserved.

#include "Core/ROGameStateBase.h"
#include "RagnarokUE.h"
#include "Net/UnrealNetwork.h"

AROGameStateBase::AROGameStateBase()
	: ServerTime(0.0f)
	, bIsWoEActive(false)
	, CurrentWeather(EROWeatherType::Clear)
	, ServerName(TEXT("Ragna-TH"))
	, OnlinePlayerCount(0)
	, DayNightSpeedMultiplier(6.0f)  // 6x speed → 4-hour full day cycle
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.5f; // half-second ticks are sufficient

	bAlwaysRelevant = true;
}

void AROGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AROGameStateBase, ServerTime);
	DOREPLIFETIME(AROGameStateBase, bIsWoEActive);
	DOREPLIFETIME(AROGameStateBase, CurrentWeather);
	DOREPLIFETIME(AROGameStateBase, ServerName);
	DOREPLIFETIME(AROGameStateBase, OnlinePlayerCount);
}

void AROGameStateBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Only the server advances the clock
	if (HasAuthority())
	{
		// Advance in-game time (wraps at 86400 = 24 in-game hours)
		ServerTime += DeltaSeconds * DayNightSpeedMultiplier;
		if (ServerTime >= 86400.0f)
		{
			ServerTime -= 86400.0f;
		}

		// Keep a live count of connected players
		OnlinePlayerCount = PlayerArray.Num();
	}
}

void AROGameStateBase::SetWeather(EROWeatherType NewWeather)
{
	if (!HasAuthority())
	{
		UE_LOG(LogRagnarokUE, Warning, TEXT("SetWeather called on client – ignored."));
		return;
	}

	CurrentWeather = NewWeather;
	UE_LOG(LogRagnarokUE, Log, TEXT("Weather changed to %d."), static_cast<uint8>(NewWeather));
}

void AROGameStateBase::SetWoEActive(bool bActive)
{
	if (!HasAuthority())
	{
		UE_LOG(LogRagnarokUE, Warning, TEXT("SetWoEActive called on client – ignored."));
		return;
	}

	bIsWoEActive = bActive;
	UE_LOG(LogRagnarokUE, Log, TEXT("War of Emperium %s."), bActive ? TEXT("STARTED") : TEXT("ENDED"));
}
