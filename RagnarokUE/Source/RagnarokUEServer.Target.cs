// Copyright Ragna-TH Project. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class RagnarokUEServerTarget : TargetRules
{
	public RagnarokUEServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
		ExtraModuleNames.AddRange(new string[] { "RagnarokUE", "RagnarokUEServer" });
	}
}
