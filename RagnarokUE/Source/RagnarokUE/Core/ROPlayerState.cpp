// Copyright Ragna-TH Project. All Rights Reserved.

#include "Core/ROPlayerState.h"
#include "RagnarokUE.h"
#include "Net/UnrealNetwork.h"

AROPlayerState::AROPlayerState()
	: CharacterName(TEXT(""))
	, BaseLevel(1)
	, JobLevel(1)
	, JobClass(EROJobClass::Novice)
	, GuildID(0)
	, GuildName(TEXT(""))
	, PartyID(0)
{
	// PlayerState is always relevant to all clients so name plates,
	// guild tags, etc. work correctly.
	bAlwaysRelevant = true;
}

void AROPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AROPlayerState, CharacterName);
	DOREPLIFETIME(AROPlayerState, BaseLevel);
	DOREPLIFETIME(AROPlayerState, JobLevel);
	DOREPLIFETIME(AROPlayerState, JobClass);
	DOREPLIFETIME(AROPlayerState, GuildID);
	DOREPLIFETIME(AROPlayerState, GuildName);
	DOREPLIFETIME(AROPlayerState, PartyID);
}
