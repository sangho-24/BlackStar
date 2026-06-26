// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BlackStar : ModuleRules
{
	public BlackStar(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"Niagara",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"BlackStar",
			"BlackStar/Variant_Combat",
			"BlackStar/Variant_Combat/AI",
			"BlackStar/Variant_Combat/Animation",
			"BlackStar/Variant_Combat/Gameplay",
			"BlackStar/Variant_Combat/Interfaces",
			"BlackStar/Variant_Combat/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
