// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROCastingComponent.h"
#include "Net/UnrealNetwork.h"

UROCastingComponent::UROCastingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);

	bIsCasting = false;
	CurrentCastTime = 0.0f;
	TotalCastTime = 0.0f;
	VariableCastTime = 0.0f;
	FixedCastTime = 0.0f;
	CastingSkillID = 0;
}

void UROCastingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UROCastingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UROCastingComponent, bIsCasting);
	DOREPLIFETIME(UROCastingComponent, CurrentCastTime);
	DOREPLIFETIME(UROCastingComponent, TotalCastTime);
	DOREPLIFETIME(UROCastingComponent, CastingSkillID);
}

void UROCastingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsCasting)
	{
		return;
	}

	CurrentCastTime += DeltaTime;

	if (CurrentCastTime >= TotalCastTime)
	{
		CompleteCast();
	}
}

void UROCastingComponent::StartCasting(float InVariableCastTime, float InFixedCastTime, int32 InSkillID)
{
	// If already casting, interrupt the current cast
	if (bIsCasting)
	{
		InterruptCasting();
	}

	VariableCastTime = FMath::Max(0.0f, InVariableCastTime);
	FixedCastTime = FMath::Max(0.0f, InFixedCastTime);
	TotalCastTime = VariableCastTime + FixedCastTime;
	CastingSkillID = InSkillID;
	CurrentCastTime = 0.0f;

	if (TotalCastTime <= 0.0f)
	{
		// Instant cast - fire complete immediately
		OnCastComplete.Broadcast();
		return;
	}

	bIsCasting = true;
	SetComponentTickEnabled(true);
}

void UROCastingComponent::InterruptCasting()
{
	if (!bIsCasting)
	{
		return;
	}

	// Can only interrupt during variable cast phase
	if (IsInVariableCastPhase())
	{
		ResetCastState();
		OnCastInterrupted.Broadcast();
	}
	// During fixed cast phase, casting cannot be interrupted
}

float UROCastingComponent::GetCastProgress() const
{
	if (!bIsCasting || TotalCastTime <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(CurrentCastTime / TotalCastTime, 0.0f, 1.0f);
}

bool UROCastingComponent::IsInVariableCastPhase() const
{
	if (!bIsCasting)
	{
		return false;
	}

	return CurrentCastTime < VariableCastTime;
}

void UROCastingComponent::CompleteCast()
{
	ResetCastState();
	OnCastComplete.Broadcast();
}

void UROCastingComponent::ResetCastState()
{
	bIsCasting = false;
	CurrentCastTime = 0.0f;
	TotalCastTime = 0.0f;
	VariableCastTime = 0.0f;
	FixedCastTime = 0.0f;
	CastingSkillID = 0;
	SetComponentTickEnabled(false);
}
