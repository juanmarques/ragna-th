// Copyright Ragna-TH Project. All Rights Reserved.

#include "RagnarokUE.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogRagnarokUE);

void FRagnarokUEModule::StartupModule()
{
	UE_LOG(LogRagnarokUE, Log, TEXT("RagnarokUE Module Started"));
}

void FRagnarokUEModule::ShutdownModule()
{
	UE_LOG(LogRagnarokUE, Log, TEXT("RagnarokUE Module Shutdown"));
}

IMPLEMENT_PRIMARY_GAME_MODULE(FRagnarokUEModule, RagnarokUE, "RagnarokUE");
