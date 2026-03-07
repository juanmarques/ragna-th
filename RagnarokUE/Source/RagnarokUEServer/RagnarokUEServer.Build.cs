// Copyright Ragna-TH Project. All Rights Reserved.

using UnrealBuildTool;

public class RagnarokUEServer : ModuleRules
{
	public RagnarokUEServer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"RagnarokUE",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"Json",
			"JsonUtilities",
			"HTTP"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"NetCore"
		});
	}
}
