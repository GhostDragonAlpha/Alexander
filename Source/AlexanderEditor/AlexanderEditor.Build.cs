// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AlexanderEditor : ModuleRules
{
	public AlexanderEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Alexander",  // Main game module
			"UMG",
			"UMGEditor"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// Editor modules
			"UnrealEd",
			"AssetTools",
			"ContentBrowser",
			"EditorScriptingUtilities",
			"Blutility",
			"Kismet",
			"KismetCompiler",
			"BlueprintGraph",

			// Asset factories
			"AssetRegistry",
			"EditorSubsystem",

			// Material editor
			"MaterialEditor",

			// Level editor
			"LevelEditor",
			"PlacementMode",

			// UI (moved to PublicDependencyModuleNames)

			// Utility
			"Slate",
			"SlateCore",
			"Json",
			"JsonUtilities"
		});

		// Enable editor-only code
		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}
	}
}
