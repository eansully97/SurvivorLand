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
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"SurvivorLand",
			"SurvivorLand/Variant_Platforming",
			"SurvivorLand/Variant_Platforming/Animation",
			"SurvivorLand/Variant_Combat",
			"SurvivorLand/Variant_Combat/AI",
			"SurvivorLand/Variant_Combat/Animation",
			"SurvivorLand/Variant_Combat/Gameplay",
			"SurvivorLand/Variant_Combat/Interfaces",
			"SurvivorLand/Variant_Combat/UI",
			"SurvivorLand/Variant_SideScrolling",
			"SurvivorLand/Variant_SideScrolling/AI",
			"SurvivorLand/Variant_SideScrolling/Gameplay",
			"SurvivorLand/Variant_SideScrolling/Interfaces",
			"SurvivorLand/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
