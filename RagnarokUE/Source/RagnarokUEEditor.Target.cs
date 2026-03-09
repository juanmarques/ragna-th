// Copyright Ragna-TH Project. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class RagnarokUEEditorTarget : TargetRules
{
	public RagnarokUEEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		bOverrideBuildEnvironment = true;
		bValidateFormatStrings = false;
		ExtraModuleNames.AddRange(new string[] { "RagnarokUE" });
	}
}
