// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "AdvancedAIBehaviors.h"
// DISABLED: EnhancedVFXSystem temporarily disabled
//#include "EnhancedVFXSystem.h"
#include "AudioSystemManager.h"
#include "TutorialOnboardingSystem.h"

/**
 * Alexander Game Module
 * Central coordination point for all Phase 7 advanced systems
 */
class ALEXANDER_API FAlexanderModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    // System Accessors
    static UAdvancedAIBehaviors* GetAISystem() { return AISystem; }
    // DISABLED: EnhancedVFXSystem temporarily disabled
    //static UEnhancedVFXSystem* GetVFXSystem() { return VFXSystem; }
    static UAudioSystemManager* GetAudioSystem() { return AudioSystem; }
    static UTutorialOnboardingSystem* GetTutorialSystem() { return TutorialSystem; }

private:
    static UAdvancedAIBehaviors* AISystem;
    // DISABLED: EnhancedVFXSystem temporarily disabled
    //static UEnhancedVFXSystem* VFXSystem;
    static UAudioSystemManager* AudioSystem;
    static UTutorialOnboardingSystem* TutorialSystem;

    void InitializePhase7Systems();
    void ShutdownPhase7Systems();
};

