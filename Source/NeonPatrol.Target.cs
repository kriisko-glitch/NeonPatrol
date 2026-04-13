// Copyright Kriisko-Studio. Licensed under project terms.

using UnrealBuildTool;
using System.Collections.Generic;

public class NeonPatrolTarget : TargetRules
{
	public NeonPatrolTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.AddRange( new string[] { "NeonPatrol", "TP_ThirdPerson" } );
	}
}
