// AIVisionSystem.cpp - Main module implementation

#include "AIVisionSystem.h"

#define LOCTEXT_NAMESPACE "FAIVisionSystemModule"

void FAIVisionSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Log, TEXT("AI Vision System: Module Started"));
}

void FAIVisionSystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Log, TEXT("AI Vision System: Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAIVisionSystemModule, AIVisionSystem)
