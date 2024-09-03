// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NIX : ModuleRules
{
	public NIX(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"AIModule", 
			"GameplayAbilities", 
			"GameplayTags", 
			"GameplayTasks", 
			"Core", 
			"CoreUObject", "Engine", "InputCore", "EnhancedInput", "ModularGameplay","Slate",         
			"SlateCore", "GameplayStateTreeModule", "AIModule", "StateTreeModule"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "AnimGraphRuntime", "Niagara" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
