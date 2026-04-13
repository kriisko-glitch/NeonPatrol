// Copyright Kriisko-Studio. NeonPatrol (GAME-017).

using UnrealBuildTool;
using System.Collections.Generic;

public class TP_ThirdPersonEditorTarget : TargetRules
{
	public TP_ThirdPersonEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.AddRange( new string[] { "TP_ThirdPerson", "NeonPatrol" } );
	}
}
