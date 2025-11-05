// Fill out your copyright notice in the Description page of Project Settings.

#include "Alexander.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

// Static member definitions
UAdvancedAIBehaviors* FAlexanderModule::AISystem = nullptr;
UEnhancedVFXSystem* FAlexanderModule::VFXSystem = nullptr;
UAudioSystemManager* FAlexanderModule::AudioSystem = nullptr;
UTutorialOnboardingSystem* FAlexanderModule::TutorialSystem = nullptr;

IMPLEMENT_PRIMARY_GAME_MODULE(FAlexanderModule, Alexander, "Alexander");

void FAlexanderModule::StartupModule()
{
    UE_LOG(LogTemp, Log, TEXT("Alexander Module Starting Up - Phase 7 Systems Integration"));
    
    // Initialize Phase 7 systems
    InitializePhase7Systems();
    
    UE_LOG(LogTemp, Log, TEXT("Alexander Module Startup Complete"));
}

void FAlexanderModule::ShutdownModule()
{
    UE_LOG(LogTemp, Log, TEXT("Alexander Module Shutting Down"));
    
    // Shutdown Phase 7 systems
    ShutdownPhase7Systems();
    
    UE_LOG(LogTemp, Log, TEXT("Alexander Module Shutdown Complete"));
}

void FAlexanderModule::InitializePhase7Systems()
{
    // Create AI Behavior System
    AISystem = NewObject<UAdvancedAIBehaviors>();
    if (AISystem)
    {
        AISystem->InitializeAI(nullptr); // Initialize system without specific AI actor
        UE_LOG(LogTemp, Log, TEXT("Advanced AI Behaviors System Initialized"));
    }
    
    // Create Enhanced VFX System
    VFXSystem = NewObject<UEnhancedVFXSystem>();
    if (VFXSystem)
    {
        VFXSystem->InitializeVFXSystem();
        UE_LOG(LogTemp, Log, TEXT("Enhanced VFX System Initialized"));
    }
    
    // Create Audio System Manager
    AudioSystem = NewObject<UAudioSystemManager>();
    if (AudioSystem)
    {
        AudioSystem->InitializeAudioSystem();
        UE_LOG(LogTemp, Log, TEXT("Audio System Manager Initialized"));
    }
    
    // Create Tutorial & Onboarding System
    TutorialSystem = NewObject<UTutorialOnboardingSystem>();
    if (TutorialSystem)
    {
        TutorialSystem->InitializeTutorialSystem();
        UE_LOG(LogTemp, Log, TEXT("Tutorial & Onboarding System Initialized"));
    }
}

void FAlexanderModule::ShutdownPhase7Systems()
{
    // Shutdown Tutorial System
    if (TutorialSystem)
    {
        TutorialSystem->ShutdownTutorialSystem();
        TutorialSystem = nullptr;
        UE_LOG(LogTemp, Log, TEXT("Tutorial & Onboarding System Shutdown"));
    }
    
    // Shutdown Audio System
    if (AudioSystem)
    {
        AudioSystem->ShutdownAudioSystem();
        AudioSystem = nullptr;
        UE_LOG(LogTemp, Log, TEXT("Audio System Manager Shutdown"));
    }
    
    // Shutdown VFX System
    if (VFXSystem)
    {
        VFXSystem->ShutdownVFXSystem();
        VFXSystem = nullptr;
        UE_LOG(LogTemp, Log, TEXT("Enhanced VFX System Shutdown"));
    }
    
    // Shutdown AI System
    if (AISystem)
    {
        AISystem->ShutdownAI(nullptr);
        AISystem = nullptr;
        UE_LOG(LogTemp, Log, TEXT("Advanced AI Behaviors System Shutdown"));
    }
}
