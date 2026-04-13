// Copyright Kriisko-Studio. Licensed under project terms.

using UnrealBuildTool;
using System.Collections.Generic;

public class NeonPatrolEditorTarget : TargetRules
{
	public NeonPatrolEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.AddRange( new string[] { "NeonPatrol", "TP_ThirdPerson" } );
	}
}
