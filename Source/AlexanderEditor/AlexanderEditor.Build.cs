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
			"Alexander"  // Main game module
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// Editor modules
			"UnrealEd",
			"AssetTools",
			"ContentBrowser",
			"EditorScriptingUtilities",
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
