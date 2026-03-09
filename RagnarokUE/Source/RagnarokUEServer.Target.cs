// Copyright Ragna-TH Project. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class RagnarokUEServerTarget : TargetRules
{
	public RagnarokUEServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		bValidateFormatStrings = false;
		ExtraModuleNames.AddRange(new string[] { "RagnarokUE", "RagnarokUEServer" });
	}
}
