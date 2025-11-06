// Copyright Epic Games, Inc. All Rights Reserved.

#include "AlexanderEditor.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "FAlexanderEditorModule"

void FAlexanderEditorModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("AlexanderEditor module starting..."));

	RegisterMenuExtensions();
	RegisterCommands();

	UE_LOG(LogTemp, Log, TEXT("AlexanderEditor module started successfully"));
}

void FAlexanderEditorModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("AlexanderEditor module shutting down"));
}

void FAlexanderEditorModule::RegisterMenuExtensions()
{
	// TODO: Add menu items for automation tools
	// Will appear in Editor menu bar
}

void FAlexanderEditorModule::RegisterCommands()
{
	// TODO: Register editor commands
	// Can be called from console or command line
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAlexanderEditorModule, AlexanderEditor)
