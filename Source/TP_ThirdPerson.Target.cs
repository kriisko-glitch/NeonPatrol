// Copyright Kriisko-Studio. NeonPatrol (GAME-017).

using UnrealBuildTool;
using System.Collections.Generic;

public class TP_ThirdPersonTarget : TargetRules
{
	public TP_ThirdPersonTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.AddRange( new string[] { "TP_ThirdPerson", "NeonPatrol" } );
	}
}
