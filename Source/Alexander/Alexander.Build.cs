// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Alexander : ModuleRules
{
	public Alexander(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		   PublicDependencyModuleNames.AddRange(new string[] { 
			   "Core", "CoreUObject", "Engine", "InputCore", "Niagara", 
			   "HTTP", "Json", "JsonUtilities", "RHI", "RenderCore",
			   "Slate", "SlateCore", "UMG", "ToolMenus", "ProceduralMeshComponent",
			   "GameplayTags", "GameplayTasks", "AIModule", "NavigationSystem", 
			   "AudioMixer", "DeveloperSettings"
		   });



// Add editor-only modules when building for editor
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { 
				"UnrealEd", "EditorWidgets", "ToolMenus", "EditorStyle", "LevelEditor"
			});
		}
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
