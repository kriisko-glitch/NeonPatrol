// Copyright Kriisko-Studio. Licensed under project terms.

using UnrealBuildTool;

public class NeonPatrol : ModuleRules
{
	public NeonPatrol(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"HTTP",
			"Json",
			"JsonUtilities",
			"UMG",
			"Slate",
			"SlateCore",
		});

		// Access TP_ThirdPerson module for ICombatDamageable interface, etc.
		PrivateDependencyModuleNames.AddRange(new string[] { "TP_ThirdPerson" });
	}
}
