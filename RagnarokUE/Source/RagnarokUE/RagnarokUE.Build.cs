// Copyright Ragna-TH Project. All Rights Reserved.

using UnrealBuildTool;

public class RagnarokUE : ModuleRules
{
	public RagnarokUE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"UMG",
			"Slate",
			"SlateCore",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"NavigationSystem",
			"AIModule",
			"NetCore",
			"ReplicationGraph"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Json",
			"JsonUtilities",
			"HTTP",
			"ImageWrapper"
		});
	}
}
