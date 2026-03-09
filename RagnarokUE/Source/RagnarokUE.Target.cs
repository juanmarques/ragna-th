// Copyright Ragna-TH Project. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class RagnarokUETarget : TargetRules
{
	public RagnarokUETarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		bValidateFormatStrings = false;
		ExtraModuleNames.AddRange(new string[] { "RagnarokUE" });
	}
}
