// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SurvivorLand : ModuleRules
{
	public SurvivorLand(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"GameplayTags",
			"EnhancedInput",
			"GameplayTasks",
			"GameplayAbilities",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"AnimGraphRuntime",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "AnimGraphRuntime", "AnimGraphRuntime" });

		PublicIncludePaths.AddRange(new string[] {
			"SurvivorLand"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
