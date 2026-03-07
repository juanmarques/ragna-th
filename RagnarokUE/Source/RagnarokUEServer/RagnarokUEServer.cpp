// Copyright Ragna-TH Project. All Rights Reserved.

#include "RagnarokUEServer.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogRagnarokUEServer);

void FRagnarokUEServerModule::StartupModule()
{
	UE_LOG(LogRagnarokUEServer, Log, TEXT("RagnarokUEServer Module Started"));
}

void FRagnarokUEServerModule::ShutdownModule()
{
	UE_LOG(LogRagnarokUEServer, Log, TEXT("RagnarokUEServer Module Shutdown"));
}

IMPLEMENT_MODULE(FRagnarokUEServerModule, RagnarokUEServer);
