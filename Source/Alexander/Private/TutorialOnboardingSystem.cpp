#include "TutorialOnboardingSystem.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/BoxComponent.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetTextLibrary.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "HAL/IConsoleManager.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"             // For UCameraComponent
#include "Components/InputComponent.h"          // For UInputComponent
#include "GameFramework/PlayerInput.h"          // For player input handling

UTutorialOnboardingSystem::UTutorialOnboardingSystem()
{
    LastUpdateTime = 0.0f;
    bEnableTutorials = true;
    bShowHints = true;
    bAutoAdvance = false;
    TutorialUpdateInterval = 0.016f;
    bEnableDebugTutorials = false;
    bAdaptiveDifficulty = true;
    bTutorialUIVisible = false;
    bHighlightVisible = false;
}

void UTutorialOnboardingSystem::InitializeTutorialSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Tutorial & Onboarding System Initialized"));
    
    // Load tutorial progress
    LoadTutorialProgress();
    
    // Create UI components
    CreateTutorialUI();
    CreateHighlightComponents();
    
    // Setup VR mode if needed
    if (IsVRMode())
    {
        SetupVRTutorialMode();
    }
    
    // Register default tutorials
    RegisterDefaultTutorials();
    
    // Unlock initial tutorials
    UnlockInitialTutorials();
    
    UE_LOG(LogTemp, Log, TEXT("Tutorial System initialization complete"));
}

void UTutorialOnboardingSystem::ShutdownTutorialSystem()
{
    // Save tutorial progress
    SaveTutorialProgress();
    
    // Stop all active tutorials
    TArray<FString> ActiveTutorialIDs;
    ActiveTutorials.GetKeys(ActiveTutorialIDs);
    
    for (const FString& TutorialID : ActiveTutorialIDs)
    {
        PauseTutorial(TutorialID);
    }
    
    // Cleanup VR mode
    if (IsVRMode())
    {
        CleanupVRTutorialMode();
    }
    
    // Destroy UI components
    DestroyTutorialUI();
    DestroyHighlightComponents();
    
    // Clear all data
    TutorialDefinitions.Empty();
    TutorialProgress.Empty();
    ActiveTutorials.Empty();
    InputBindings.Empty();
    ConditionCheckers.Empty();
    EventHandlers.Empty();
    UnlockedTutorials.Empty();
    StepStartTimes.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Tutorial & Onboarding System Shutdown"));
}

void UTutorialOnboardingSystem::UpdateTutorialSystem(float DeltaTime)
{
    if (!GetWorld() || !bEnableTutorials) return;

    // Update active tutorials
    UpdateActiveTutorials(DeltaTime);
    
    // Update tutorial UI
    UpdateTutorialUI(DeltaTime);
    
    // Update highlights
    UpdateHighlights(DeltaTime);
    
    // Draw debug info
    if (bEnableDebugTutorials)
    {
        DrawDebugTutorialInfo();
    }
}

void UTutorialOnboardingSystem::UpdateActiveTutorials(float DeltaTime)
{
    TArray<FString> TutorialsToUpdate;
    ActiveTutorials.GetKeys(TutorialsToUpdate);
    
    for (const FString& TutorialID : TutorialsToUpdate)
    {
        if (!TutorialProgress.Contains(TutorialID)) continue;
        
        FTutorialOnboardingProgress& Progress = TutorialProgress[TutorialID];
        
        if (Progress.CompletionState != ETutorialCompletionState::InProgress) continue;
        
        // Check current step conditions
        if (TutorialDefinitions.Contains(TutorialID))
        {
            const FTutorialDefinition& Tutorial = TutorialDefinitions[TutorialID];
            
            if (Progress.CurrentStepIndex < Tutorial.Steps.Num())
            {
                const FTutorialOnboardingStep& CurrentStep = Tutorial.Steps[Progress.CurrentStepIndex];
                
                // Check if step should auto-complete
                if (bAutoAdvance && CurrentStep.Duration > 0.0f)
                {
                    float StepElapsedTime = GetWorld()->GetTimeSeconds() - StepStartTimes.FindRef(CurrentStep.StepID);
                    if (StepElapsedTime >= CurrentStep.Duration)
                    {
                        CompleteTutorialStep(CurrentStep.StepID);
                    }
                }
                
                // Check step conditions
                if (CheckStepConditions(CurrentStep))
                {
                    CompleteTutorialStep(CurrentStep.StepID);
                }
            }
        }
    }
}

void UTutorialOnboardingSystem::UpdateTutorialUI(float DeltaTime)
{
    if (!TutorialWidget || !bTutorialUIVisible) return;
    
    // Update UI based on current step
    if (!CurrentStepID.IsEmpty() && TutorialDefinitions.Contains(CurrentTutorialID))
    {
        const FTutorialDefinition& Tutorial = TutorialDefinitions[CurrentTutorialID];
        
        for (const FTutorialOnboardingStep& Step : Tutorial.Steps)
        {
            if (Step.StepID == CurrentStepID)
            {
                UpdateTutorialUI(Step);
                break;
            }
        }
    }
}

void UTutorialOnboardingSystem::UpdateHighlights(float DeltaTime)
{
    if (!bHighlightVisible) return;
    
    // Update highlight effects
    if (HighlightMesh)
    {
        // Pulse effect
        float PulseTime = GetWorld()->GetTimeSeconds();
        float PulseScale = 1.0f + FMath::Sin(PulseTime * 3.0f) * 0.1f;
        HighlightMesh->SetRelativeScale3D(FVector(PulseScale));
    }
}

void UTutorialOnboardingSystem::RegisterTutorial(const FTutorialDefinition& Tutorial)
{
    TutorialDefinitions.Add(Tutorial.TutorialID, Tutorial);
    
    // Initialize progress if not exists
    if (!TutorialProgress.Contains(Tutorial.TutorialID))
    {
        FTutorialOnboardingProgress Progress;
        Progress.TutorialID = Tutorial.TutorialID;
        Progress.CompletionState = ETutorialCompletionState::NotStarted;
        Progress.CurrentStepIndex = 0;
        Progress.Attempts = 0;
        Progress.Skips = 0;
        
        TutorialProgress.Add(Tutorial.TutorialID, Progress);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Registered tutorial: %s"), *Tutorial.TutorialID);
}

void UTutorialOnboardingSystem::UnregisterTutorial(const FString& TutorialID)
{
    TutorialDefinitions.Remove(TutorialID);
    TutorialProgress.Remove(TutorialID);
    ActiveTutorials.Remove(TutorialID);
    
    UE_LOG(LogTemp, Log, TEXT("Unregistered tutorial: %s"), *TutorialID);
}

void UTutorialOnboardingSystem::StartTutorial(const FString& TutorialID, APlayerController* PlayerController)
{
    if (!GetWorld() || !TutorialDefinitions.Contains(TutorialID)) return;

    const FTutorialDefinition& Tutorial = TutorialDefinitions[TutorialID];
    
    // Check prerequisites
    if (!CheckPrerequisites(TutorialID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Tutorial %s prerequisites not met"), *TutorialID);
        return;
    }
    
    // Check if tutorial is unlocked
    if (!IsTutorialUnlocked(TutorialID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Tutorial %s is not unlocked"), *TutorialID);
        return;
    }
    
    // Check platform compatibility
    if (Tutorial.Platform != ETutorialPlatform::All)
    {
        ETutorialPlatform CurrentPlatform = GetCurrentPlatform();
        if (Tutorial.Platform != CurrentPlatform)
        {
            UE_LOG(LogTemp, Warning, TEXT("Tutorial %s not compatible with current platform"), *TutorialID);
            return;
        }
    }
    
    // Initialize or update progress
    FTutorialOnboardingProgress& Progress = TutorialProgress[TutorialID];
    Progress.CompletionState = ETutorialCompletionState::InProgress;
    Progress.StartTime = GetWorld()->GetTimeSeconds();
    Progress.Attempts++;
    
    // Set as active tutorial
    ActiveTutorials.Add(TutorialID, PlayerController ? PlayerController->GetName() : "None");
    CurrentTutorialID = TutorialID;
    
    // Start first step
    if (Tutorial.Steps.Num() > 0)
    {
        Progress.CurrentStepIndex = 0;
        StartTutorialStep(Tutorial.Steps[0].StepID);
    }
    
    // Show tutorial UI
    ShowTutorialUI(TutorialID);
    
    // Broadcast event
    OnTutorialStarted.Broadcast(TutorialID, PlayerController);
    
    UE_LOG(LogTemp, Log, TEXT("Started tutorial: %s"), *TutorialID);
}

void UTutorialOnboardingSystem::CompleteTutorial(const FString& TutorialID)
{
    if (!TutorialProgress.Contains(TutorialID)) return;

    FTutorialOnboardingProgress& Progress = TutorialProgress[TutorialID];
    Progress.CompletionState = ETutorialCompletionState::Completed;
    Progress.CompletionTime = GetWorld()->GetTimeSeconds();
    
    // Remove from active tutorials
    ActiveTutorials.Remove(TutorialID);
    
    // Hide tutorial UI
    if (CurrentTutorialID == TutorialID)
    {
        HideTutorialUI();
        HideTutorialHighlight();
        CurrentTutorialID = "";
        CurrentStepID = "";
    }
    
    // Unlock dependent tutorials
    UnlockPrerequisiteTutorials(TutorialID);
    
    // Trigger completion event
    if (TutorialDefinitions.Contains(TutorialID))
    {
        const FTutorialDefinition& Tutorial = TutorialDefinitions[TutorialID];
        if (!Tutorial.CompletionEvent.IsEmpty())
        {
            TriggerTutorialEvent(FTutorialEventData{Tutorial.CompletionEvent, TutorialID, "", static_cast<float>(GetWorld()->GetTimeSeconds())});
        }
    }
    
    // Adjust difficulty based on performance
    if (bAdaptiveDifficulty)
    {
        AdjustDifficultyBasedOnPerformance(TutorialID);
    }
    
    // Save progress
    SaveTutorialProgress();
    
    // Broadcast event
    OnTutorialCompleted.Broadcast(TutorialID, Progress.CompletionTime - Progress.StartTime);
    
    UE_LOG(LogTemp, Log, TEXT("Completed tutorial: %s in %.2f seconds"), 
        *TutorialID, Progress.CompletionTime - Progress.StartTime);
}

void UTutorialOnboardingSystem::SkipTutorial(const FString& TutorialID, const FString& Reason)
{
    if (!TutorialProgress.Contains(TutorialID)) return;

    FTutorialOnboardingProgress& Progress = TutorialProgress[TutorialID];
    Progress.CompletionState = ETutorialCompletionState::Skipped;
    Progress.Skips++;
    
    // Remove from active tutorials
    ActiveTutorials.Remove(TutorialID);
    
    // Hide tutorial UI
    if (CurrentTutorialID == TutorialID)
    {
        HideTutorialUI();
        HideTutorialHighlight();
        CurrentTutorialID = "";
        CurrentStepID = "";
    }
    
    // Broadcast event
    OnTutorialSkipped.Broadcast(TutorialID, Reason);
    
    UE_LOG(LogTemp, Log, TEXT("Skipped tutorial: %s - Reason: %s"), *TutorialID, *Reason);
}

void UTutorialOnboardingSystem::PauseTutorial(const FString& TutorialID)
{
    if (!TutorialProgress.Contains(TutorialID)) return;

    FTutorialOnboardingProgress& Progress = TutorialProgress[TutorialID];
    
    if (Progress.CompletionState == ETutorialCompletionState::InProgress)
    {
        Progress.CompletionState = ETutorialCompletionState::NotStarted; // Use as paused state
        
        // Hide tutorial UI
        if (CurrentTutorialID == TutorialID)
        {
            HideTutorialUI();
            HideTutorialHighlight();
        }
        
        UE_LOG(LogTemp, Log, TEXT("Paused tutorial: %s"), *TutorialID);
    }
}

void UTutorialOnboardingSystem::ResumeTutorial(const FString& TutorialID)
{
    if (!TutorialProgress.Contains(TutorialID)) return;

    FTutorialOnboardingProgress& Progress = TutorialProgress[TutorialID];
    
    if (Progress.CompletionState == ETutorialCompletionState::NotStarted && 
        TutorialDefinitions.Contains(TutorialID))
    {
        Progress.CompletionState = ETutorialCompletionState::InProgress;
        
        // Set as active tutorial
        ActiveTutorials.Add(TutorialID, "None");
        CurrentTutorialID = TutorialID;
        
        // Show tutorial UI
        ShowTutorialUI(TutorialID);
        
        // Resume current step
        const FTutorialDefinition& Tutorial = TutorialDefinitions[TutorialID];
        if (Progress.CurrentStepIndex < Tutorial.Steps.Num())
        {
            StartTutorialStep(Tutorial.Steps[Progress.CurrentStepIndex].StepID);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Resumed tutorial: %s"), *TutorialID);
    }
}

void UTutorialOnboardingSystem::RestartTutorial(const FString& TutorialID)
{
    if (!TutorialProgress.Contains(TutorialID)) return;

    FTutorialOnboardingProgress& Progress = TutorialProgress[TutorialID];
    Progress.CompletionState = ETutorialCompletionState::NotStarted;
    Progress.CurrentStepIndex = 0;
    Progress.StartTime = 0.0f;
    Progress.CompletionTime = 0.0f;
    Progress.CompletedSteps.Empty();
    Progress.FailedSteps.Empty();
    Progress.StepCompletionTimes.Empty();
    
    // Clear step start times
    for (auto& StepTimePair : StepStartTimes)
    {
        if (StepTimePair.Key.Contains(TutorialID))
        {
            StepStartTimes.Remove(StepTimePair.Key);
        }
    }
    
    // Start tutorial again
    StartTutorial(TutorialID, nullptr);
    
    UE_LOG(LogTemp, Log, TEXT("Restarted tutorial: %s"), *TutorialID);
}

FTutorialOnboardingProgress UTutorialOnboardingSystem::GetTutorialProgress(const FString& TutorialID) const
{
    if (!TutorialProgress.Contains(TutorialID)) 
    {
        FTutorialOnboardingProgress EmptyProgress;
        return EmptyProgress;
    }
    return TutorialProgress[TutorialID];
}

bool UTutorialOnboardingSystem::IsTutorialCompleted(const FString& TutorialID) const
{
    if (!TutorialProgress.Contains(TutorialID)) return false;
    return TutorialProgress[TutorialID].CompletionState == ETutorialCompletionState::Completed;
}

bool UTutorialOnboardingSystem::IsTutorialInProgress(const FString& TutorialID) const
{
    if (!TutorialProgress.Contains(TutorialID)) return false;
    return TutorialProgress[TutorialID].CompletionState == ETutorialCompletionState::InProgress;
}

TArray<FString> UTutorialOnboardingSystem::GetCompletedTutorials() const
{
    TArray<FString> CompletedTutorials;
    
    for (const auto& ProgressPair : TutorialProgress)
    {
        if (ProgressPair.Value.CompletionState == ETutorialCompletionState::Completed)
        {
            CompletedTutorials.Add(ProgressPair.Key);
        }
    }
    
    return CompletedTutorials;
}

TArray<FString> UTutorialOnboardingSystem::GetAvailableTutorials() const
{
    TArray<FString> AvailableTutorials;
    
    for (const auto& TutorialPair : TutorialDefinitions)
    {
        const FString& TutorialID = TutorialPair.Key;
        const FTutorialDefinition& Tutorial = TutorialPair.Value;
        
        if (IsTutorialUnlocked(TutorialID) && !IsTutorialCompleted(TutorialID))
        {
            AvailableTutorials.Add(TutorialID);
        }
    }
    
    return AvailableTutorials;
}

TArray<FString> UTutorialOnboardingSystem::GetMandatoryTutorials() const
{
    TArray<FString> MandatoryTutorials;
    
    for (const auto& TutorialPair : TutorialDefinitions)
    {
        if (TutorialPair.Value.bIsMandatory)
        {
            MandatoryTutorials.Add(TutorialPair.Key);
        }
    }
    
    return MandatoryTutorials;
}

void UTutorialOnboardingSystem::StartTutorialStep(const FString& StepID)
{
    if (!GetWorld()) return;

    CurrentStepID = StepID;
    StepStartTimes.Add(StepID, GetWorld()->GetTimeSeconds());
    
    // Find the step definition
    FTutorialOnboardingStep CurrentStep;
    bool bFoundStep = false;
    
    if (TutorialDefinitions.Contains(CurrentTutorialID))
    {
        const FTutorialDefinition& Tutorial = TutorialDefinitions[CurrentTutorialID];
        for (const FTutorialOnboardingStep& Step : Tutorial.Steps)
        {
            if (Step.StepID == StepID)
            {
                CurrentStep = Step;
                bFoundStep = true;
                break;
            }
        }
    }
    
    if (!bFoundStep) return;
    
    // Show step highlight
    ShowStepHighlight(CurrentStep);
    
    // Update tutorial UI
    UpdateTutorialUI(CurrentStep);
    
    // Play voice over
    if (CurrentStep.VoiceOver)
    {
        PlayTutorialVoiceOver(CurrentStep.VoiceOver);
    }
    
    // Play animation
    if (CurrentStep.Animation && TutorialWidget)
    {
        PlayStepAnimation(CurrentStep);
    }
    
    // Register input bindings
    for (const FString& Input : CurrentStep.RequiredInputs)
    {
        RegisterInputBinding(StepID, Input);
    }
    
    // Broadcast event
    OnTutorialStepStarted.Broadcast(StepID, CurrentStep.Title);
    
    UE_LOG(LogTemp, Log, TEXT("Started tutorial step: %s"), *StepID);
}

void UTutorialOnboardingSystem::CompleteTutorialStep(const FString& StepID)
{
    if (!GetWorld() || !TutorialProgress.Contains(CurrentTutorialID)) return;

    FTutorialOnboardingProgress& Progress = TutorialProgress[CurrentTutorialID];
    float CompletionTime = GetWorld()->GetTimeSeconds() - StepStartTimes.FindRef(StepID);
    
    // Mark step as completed
    Progress.CompletedSteps.Add(StepID);
    Progress.StepCompletionTimes.Add(StepID, CompletionTime);
    
    // Unregister input bindings
    if (TutorialDefinitions.Contains(CurrentTutorialID))
    {
        const FTutorialDefinition& Tutorial = TutorialDefinitions[CurrentTutorialID];
        for (const FTutorialOnboardingStep& Step : Tutorial.Steps)
        {
            if (Step.StepID == StepID)
            {
                for (const FString& Input : Step.RequiredInputs)
                {
                    UnregisterInputBinding(Input);
                }
                break;
            }
        }
    }
    
    // Stop voice over
    StopTutorialVoiceOver();
    
    // Stop animation
    StopStepAnimation();
    
    // Hide highlight
    HideStepHighlight();
    
    // Broadcast event
    OnTutorialStepCompleted.Broadcast(StepID, CompletionTime);
    
    // Advance to next step
    AdvanceToNextStep();
    
    UE_LOG(LogTemp, Log, TEXT("Completed tutorial step: %s in %.2f seconds"), *StepID, CompletionTime);
}

void UTutorialOnboardingSystem::SkipTutorialStep(const FString& StepID)
{
    if (!TutorialProgress.Contains(CurrentTutorialID)) return;

    FTutorialOnboardingProgress& Progress = TutorialProgress[CurrentTutorialID];
    Progress.Skips++;
    
    // Complete step logic
    CompleteTutorialStep(StepID);
    
    UE_LOG(LogTemp, Log, TEXT("Skipped tutorial step: %s"), *StepID);
}

FTutorialOnboardingStep UTutorialOnboardingSystem::GetCurrentStep() const
{
    if (CurrentStepID.IsEmpty() || !TutorialDefinitions.Contains(CurrentTutorialID)) 
    {
        FTutorialOnboardingStep EmptyStep;
        return EmptyStep;
    }
    
    const FTutorialDefinition& Tutorial = TutorialDefinitions[CurrentTutorialID];
    for (const FTutorialOnboardingStep& Step : Tutorial.Steps)
    {
        if (Step.StepID == CurrentStepID)
        {
            return Step;
        }
    }
    
    FTutorialOnboardingStep EmptyStep;
    return EmptyStep;
}

int32 UTutorialOnboardingSystem::GetCurrentStepIndex() const
{
    if (!TutorialProgress.Contains(CurrentTutorialID)) return 0;
    return TutorialProgress[CurrentTutorialID].CurrentStepIndex;
}

TArray<FTutorialOnboardingStep> UTutorialOnboardingSystem::GetTutorialSteps(const FString& TutorialID) const
{
    if (!TutorialDefinitions.Contains(TutorialID)) return TArray<FTutorialOnboardingStep>();
    return TutorialDefinitions[TutorialID].Steps;
}

bool UTutorialOnboardingSystem::IsTutorialStepCompleted(const FString& StepID) const
{
    if (!TutorialProgress.Contains(CurrentTutorialID)) return false;

    const FTutorialOnboardingProgress& Progress = TutorialProgress[CurrentTutorialID];
    return Progress.CompletedSteps.Contains(StepID);
}

void UTutorialOnboardingSystem::ShowTutorialUI(const FString& TutorialID)
{
    if (!TutorialWidget) return;
    
    TutorialWidget->AddToViewport();
    bTutorialUIVisible = true;
    
    UE_LOG(LogTemp, Log, TEXT("Showing tutorial UI for: %s"), *TutorialID);
}

void UTutorialOnboardingSystem::HideTutorialUI()
{
    if (!TutorialWidget) return;
    
    TutorialWidget->RemoveFromParent();
    bTutorialUIVisible = false;
    
    UE_LOG(LogTemp, Log, TEXT("Hiding tutorial UI"));
}

void UTutorialOnboardingSystem::UpdateTutorialUI(const FTutorialOnboardingStep& Step)
{
    if (!TutorialWidget || !bTutorialUIVisible) return;
    
    // Update UI elements with step information
    // This would need to be implemented based on the specific widget structure
    UFunction* UpdateFunction = TutorialWidget->FindFunction(FName("UpdateTutorialStep"));
    if (UpdateFunction)
    {
        struct FParams
        {
            FTutorialOnboardingStep Step;
        } Params;
        Params.Step = Step;
        
        TutorialWidget->ProcessEvent(UpdateFunction, &Params);
    }
}

void UTutorialOnboardingSystem::ShowTutorialHighlight(const FTutorialHighlight& Highlight)
{
    if (!HighlightMesh || !HighlightText) return;
    
    // Position highlight at target
    if (Highlight.TargetActor)
    {
        HighlightMesh->SetWorldLocation(Highlight.TargetActor->GetActorLocation());
        HighlightText->SetWorldLocation(Highlight.TargetActor->GetActorLocation() + Highlight.ArrowOffset);
    }
    
    // Set highlight color
    if (HighlightMesh->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = HighlightMesh->CreateDynamicMaterialInstance(0);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue("HighlightColor", Highlight.HighlightColor);
        }
    }
    
    // Show components
    HighlightMesh->SetVisibility(true);
    HighlightText->SetVisibility(true);
    bHighlightVisible = true;
    
    UE_LOG(LogTemp, Log, TEXT("Showing tutorial highlight"));
}

void UTutorialOnboardingSystem::HideTutorialHighlight()
{
    if (!HighlightMesh || !HighlightText) return;
    
    HighlightMesh->SetVisibility(false);
    HighlightText->SetVisibility(false);
    bHighlightVisible = false;
    
    UE_LOG(LogTemp, Log, TEXT("Hiding tutorial highlight"));
}

void UTutorialOnboardingSystem::PlayTutorialVoiceOver(USoundCue* VoiceOver)
{
    if (!VoiceOver || !GetWorld()) return;
    
    UGameplayStatics::PlaySound2D(GetWorld(), VoiceOver);
    
    UE_LOG(LogTemp, Log, TEXT("Playing tutorial voice over"));
}

void UTutorialOnboardingSystem::StopTutorialVoiceOver()
{
    // Stop all tutorial voice over sounds
    // This would need implementation based on how sounds are managed
    UE_LOG(LogTemp, Log, TEXT("Stopping tutorial voice over"));
}

void UTutorialOnboardingSystem::SetVRTutorialData(const FVRTutorialData& VRData)
{
    CurrentVRData = VRData;
    
    // Setup VR mode if required
    if (VRData.bRequiresVR && IsVRMode())
    {
        SetupVRTutorialMode();
    }
}

FVRTutorialData UTutorialOnboardingSystem::GetVRTutorialData() const
{
    return CurrentVRData;
}

bool UTutorialOnboardingSystem::IsVRTutorial(const FString& TutorialID) const
{
    if (!TutorialDefinitions.Contains(TutorialID)) return false;
    
    const FTutorialDefinition& Tutorial = TutorialDefinitions[TutorialID];
    return Tutorial.Steps.ContainsByPredicate([](const FTutorialOnboardingStep& Step)
    {
        return Step.bRequiresVR;
    });
}

void UTutorialOnboardingSystem::ShowVRTutorialHints()
{
    if (!IsVRMode()) return;
    
    // Show VR-specific hints
    UE_LOG(LogTemp, Log, TEXT("Showing VR tutorial hints"));
}

void UTutorialOnboardingSystem::HideVRTutorialHints()
{
    if (!IsVRMode()) return;
    
    // Hide VR-specific hints
    UE_LOG(LogTemp, Log, TEXT("Hiding VR tutorial hints"));
}

void UTutorialOnboardingSystem::ProcessTutorialInput(const FString& InputName, bool bPressed)
{
    if (!InputBindings.Contains(InputName)) return;
    
    FString StepID = InputBindings[InputName];
    
    if (bPressed && !StepID.IsEmpty())
    {
        // Check if this input completes the current step
        if (StepID == CurrentStepID)
        {
            CompleteTutorialStep(StepID);
        }
    }
}

void UTutorialOnboardingSystem::RegisterInputBinding(const FString& StepID, const FString& InputName)
{
    InputBindings.Add(InputName, StepID);
    
    UE_LOG(LogTemp, Log, TEXT("Registered input binding: %s -> %s"), *InputName, *StepID);
}

void UTutorialOnboardingSystem::UnregisterInputBinding(const FString& InputName)
{
    InputBindings.Remove(InputName);
    
    UE_LOG(LogTemp, Log, TEXT("Unregistered input binding: %s"), *InputName);
}

bool UTutorialOnboardingSystem::CheckStepConditions(const FTutorialOnboardingStep& Step)
{
    // Check success conditions
    for (const FString& Condition : Step.SuccessConditions)
    {
        if (ConditionCheckers.Contains(Condition))
        {
            if (!ConditionCheckers[Condition]())
            {
                return false;
            }
        }
    }
    
    // Check failure conditions
    for (const FString& Condition : Step.FailureConditions)
    {
        if (ConditionCheckers.Contains(Condition))
        {
            if (ConditionCheckers[Condition]())
            {
                ProcessStepFailure(Step.StepID);
                return false;
            }
        }
    }
    
    return true;
}

bool UTutorialOnboardingSystem::CheckSuccessConditions(const FString& StepID)
{
    if (!TutorialDefinitions.Contains(CurrentTutorialID)) return false;
    
    const FTutorialDefinition& Tutorial = TutorialDefinitions[CurrentTutorialID];
    for (const FTutorialOnboardingStep& Step : Tutorial.Steps)
    {
        if (Step.StepID == StepID)
        {
            return CheckStepConditions(Step);
        }
    }
    
    return false;
}

bool UTutorialOnboardingSystem::CheckFailureConditions(const FString& StepID)
{
    // Similar to success conditions but for failure
    return false;
}

void UTutorialOnboardingSystem::RegisterConditionChecker(const FString& ConditionName, TFunction<bool()> Checker)
{
    ConditionCheckers.Add(ConditionName, Checker);
}

void UTutorialOnboardingSystem::TriggerTutorialEvent(const FTutorialEventData& EventData)
{
    // Log event
    LogTutorialEvent(EventData.EventName, EventData.TutorialID, EventData.StepID, EventData.EventData);
    
    // Call event handlers
    if (EventHandlers.Contains(EventData.EventName))
    {
        EventHandlers[EventData.EventName](EventData);
    }
    
    // Broadcast event
    OnTutorialEvent.Broadcast(EventData.EventName, EventData.TutorialID, EventData.StepID);
}

void UTutorialOnboardingSystem::RegisterTutorialEventHandler(const FString& EventName, TFunction<void(const FTutorialEventData&)> Handler)
{
    EventHandlers.Add(EventName, Handler);
}

void UTutorialOnboardingSystem::UnlockTutorial(const FString& TutorialID)
{
    if (!UnlockedTutorials.Contains(TutorialID))
    {
        UnlockedTutorials.Add(TutorialID);
        UE_LOG(LogTemp, Log, TEXT("Unlocked tutorial: %s"), *TutorialID);
    }
}

void UTutorialOnboardingSystem::LockTutorial(const FString& TutorialID)
{
    UnlockedTutorials.Remove(TutorialID);
    UE_LOG(LogTemp, Log, TEXT("Locked tutorial: %s"), *TutorialID);
}

bool UTutorialOnboardingSystem::IsTutorialUnlocked(const FString& TutorialID) const
{
    return UnlockedTutorials.Contains(TutorialID);
}

TArray<FString> UTutorialOnboardingSystem::GetUnlockedTutorials() const
{
    return UnlockedTutorials;
}

void UTutorialOnboardingSystem::SetAdaptiveDifficulty(bool bEnabled)
{
    bAdaptiveDifficulty = bEnabled;
}

bool UTutorialOnboardingSystem::IsAdaptiveDifficultyEnabled() const
{
    return bAdaptiveDifficulty;
}

void UTutorialOnboardingSystem::AdjustTutorialDifficulty(const FString& TutorialID, ETutorialDifficultyOnboarding NewDifficulty)
{
    if (!TutorialDefinitions.Contains(TutorialID)) return;
    
    FTutorialDefinition& Tutorial = TutorialDefinitions[TutorialID];
    Tutorial.Difficulty = NewDifficulty;
    
    UE_LOG(LogTemp, Log, TEXT("Adjusted tutorial difficulty for %s to %d"), *TutorialID, (int32)NewDifficulty);
}

ETutorialDifficultyOnboarding UTutorialOnboardingSystem::GetRecommendedDifficulty(const FString& TutorialID) const
{
    if (!bAdaptiveDifficulty) return ETutorialDifficultyOnboarding::Beginner;
    
    return CalculateOptimalDifficulty(TutorialID);
}

FTutorialDefinition UTutorialOnboardingSystem::GetTutorialDefinition(const FString& TutorialID) const
{
    if (!TutorialDefinitions.Contains(TutorialID)) 
    {
        FTutorialDefinition EmptyDefinition;
        return EmptyDefinition;
    }
    return TutorialDefinitions[TutorialID];
}

TArray<FTutorialDefinition> UTutorialOnboardingSystem::GetAllTutorials() const
{
    TArray<FTutorialDefinition> AllTutorials;
    TutorialDefinitions.GenerateValueArray(AllTutorials);
    return AllTutorials;
}

void UTutorialOnboardingSystem::ResetTutorialProgress(const FString& TutorialID)
{
    if (!TutorialProgress.Contains(TutorialID)) return;
    
    FTutorialOnboardingProgress& Progress = TutorialProgress[TutorialID];
    Progress.CompletionState = ETutorialCompletionState::NotStarted;
    Progress.CurrentStepIndex = 0;
    Progress.StartTime = 0.0f;
    Progress.CompletionTime = 0.0f;
    Progress.Attempts = 0;
    Progress.Skips = 0;
    Progress.CompletedSteps.Empty();
    Progress.FailedSteps.Empty();
    Progress.StepCompletionTimes.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Reset progress for tutorial: %s"), *TutorialID);
}

void UTutorialOnboardingSystem::ResetAllTutorialProgress()
{
    TArray<FString> TutorialIDs;
    TutorialProgress.GetKeys(TutorialIDs);
    
    for (const FString& TutorialID : TutorialIDs)
    {
        ResetTutorialProgress(TutorialID);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Reset all tutorial progress"));
}

float UTutorialOnboardingSystem::GetTutorialCompletionPercentage() const
{
    if (TutorialDefinitions.Num() == 0) return 0.0f;
    
    int32 CompletedCount = GetCompletedTutorials().Num();
    return (float)CompletedCount / (float)TutorialDefinitions.Num() * 100.0f;
}

int32 UTutorialOnboardingSystem::GetTotalTutorialsCompleted() const
{
    return GetCompletedTutorials().Num();
}

int32 UTutorialOnboardingSystem::GetTotalTutorialsAvailable() const
{
    return TutorialDefinitions.Num();
}

void UTutorialOnboardingSystem::AdvanceToNextStep()
{
    if (!TutorialProgress.Contains(CurrentTutorialID)) return;
    
    FTutorialOnboardingProgress& Progress = TutorialProgress[CurrentTutorialID];
    Progress.CurrentStepIndex++;
    
    if (TutorialDefinitions.Contains(CurrentTutorialID))
    {
        const FTutorialDefinition& Tutorial = TutorialDefinitions[CurrentTutorialID];
        
        if (Progress.CurrentStepIndex >= Tutorial.Steps.Num())
        {
            // Tutorial completed
            CompleteTutorial(CurrentTutorialID);
        }
        else
        {
            // Start next step
            StartTutorialStep(Tutorial.Steps[Progress.CurrentStepIndex].StepID);
        }
    }
}

void UTutorialOnboardingSystem::ProcessStepFailure(const FString& StepID)
{
    if (!TutorialProgress.Contains(CurrentTutorialID)) return;
    
    FTutorialOnboardingProgress& Progress = TutorialProgress[CurrentTutorialID];
    Progress.FailedSteps.Add(StepID);
    
    UE_LOG(LogTemp, Log, TEXT("Step failed: %s"), *StepID);
}

void UTutorialOnboardingSystem::ShowStepHighlight(const FTutorialOnboardingStep& Step)
{
    FTutorialHighlight Highlight;
    Highlight.TargetActor = Step.TargetActor;
    Highlight.TargetWidgetName = Step.TargetWidgetName;
    Highlight.HighlightColor = FLinearColor::Yellow;
    Highlight.HighlightIntensity = 1.0f;
    Highlight.bPulseEffect = true;
    Highlight.bShowArrow = true;
    Highlight.ArrowOffset = FVector(0, 0, 100);
    
    ShowTutorialHighlight(Highlight);
}

void UTutorialOnboardingSystem::HideStepHighlight()
{
    HideTutorialHighlight();
}

void UTutorialOnboardingSystem::PlayStepAnimation(const FTutorialOnboardingStep& Step)
{
    if (!Step.Animation || !TutorialWidget) return;
    
    // Play the animation on the tutorial widget
    UWidgetAnimation* Animation = Step.Animation;
    TutorialWidget->PlayAnimation(Animation);
    
    UE_LOG(LogTemp, Log, TEXT("Playing step animation: %s"), *Step.Animation->GetName());
}

void UTutorialOnboardingSystem::StopStepAnimation()
{
    if (!TutorialWidget) return;
    
    // Stop all animations on the tutorial widget
    TutorialWidget->StopAllAnimations();
    
    UE_LOG(LogTemp, Log, TEXT("Stopped step animations"));
}

bool UTutorialOnboardingSystem::CheckPrerequisites(const FString& TutorialID)
{
    if (!TutorialDefinitions.Contains(TutorialID)) return false;
    
    const FTutorialDefinition& Tutorial = TutorialDefinitions[TutorialID];
    
    for (const FString& Prerequisite : Tutorial.Prerequisites)
    {
        if (!IsTutorialCompleted(Prerequisite))
        {
            return false;
        }
    }
    
    return true;
}

void UTutorialOnboardingSystem::UnlockPrerequisiteTutorials(const FString& TutorialID)
{
    // Find tutorials that have this tutorial as a prerequisite
    for (const auto& TutorialPair : TutorialDefinitions)
    {
        const FString& OtherTutorialID = TutorialPair.Key;
        const FTutorialDefinition& OtherTutorial = TutorialPair.Value;
        
        if (OtherTutorial.Prerequisites.Contains(TutorialID))
        {
            // Check if all prerequisites are now met
            bool bAllPrerequisitesMet = true;
            for (const FString& Prerequisite : OtherTutorial.Prerequisites)
            {
                if (!IsTutorialCompleted(Prerequisite))
                {
                    bAllPrerequisitesMet = false;
                    break;
                }
            }
            
            if (bAllPrerequisitesMet)
            {
                UnlockTutorial(OtherTutorialID);
            }
        }
    }
}

void UTutorialOnboardingSystem::LogTutorialEvent(const FString& EventName, const FString& TutorialID, const FString& StepID, const TMap<FString, FString>& EventData)
{
    FString LogMessage = FString::Printf(TEXT("Tutorial Event: %s - Tutorial: %s, Step: %s"), 
        *EventName, *TutorialID, *StepID);
    
    for (const auto& DataPair : EventData)
    {
        LogMessage += FString::Printf(TEXT(", %s: %s"), *DataPair.Key, *DataPair.Value);
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
}

void UTutorialOnboardingSystem::SaveTutorialProgress()
{
    // Save tutorial progress to persistent storage
    // This would integrate with the game's save system
    UE_LOG(LogTemp, Log, TEXT("Saving tutorial progress"));
}

void UTutorialOnboardingSystem::LoadTutorialProgress()
{
    // Load tutorial progress from persistent storage
    // This would integrate with the game's save system
    UE_LOG(LogTemp, Log, TEXT("Loading tutorial progress"));
}

void UTutorialOnboardingSystem::AdjustDifficultyBasedOnPerformance(const FString& TutorialID)
{
    if (!TutorialProgress.Contains(TutorialID)) return;
    
    const FTutorialOnboardingProgress& Progress = TutorialProgress[TutorialID];
    
    // Calculate performance metrics
    float CompletionTime = Progress.CompletionTime - Progress.StartTime;
    float AverageStepTime = 0.0f;
    
    if (Progress.StepCompletionTimes.Num() > 0)
    {
        float TotalStepTime = 0.0f;
        for (const auto& StepTimePair : Progress.StepCompletionTimes)
        {
            TotalStepTime += StepTimePair.Value;
        }
        AverageStepTime = TotalStepTime / Progress.StepCompletionTimes.Num();
    }
    
    // Adjust difficulty based on performance
    ETutorialDifficultyOnboarding NewDifficulty = ETutorialDifficultyOnboarding::Beginner;
    
    if (CompletionTime < 30.0f && Progress.Skips == 0)
    {
        NewDifficulty = ETutorialDifficultyOnboarding::Advanced;
    }
    else if (CompletionTime < 60.0f && Progress.Skips <= 1)
    {
        NewDifficulty = ETutorialDifficultyOnboarding::Intermediate;
    }
    else if (CompletionTime > 120.0f || Progress.Skips > 2)
    {
        NewDifficulty = ETutorialDifficultyOnboarding::Beginner;
    }
    
    AdjustTutorialDifficulty(TutorialID, NewDifficulty);
}

ETutorialDifficultyOnboarding UTutorialOnboardingSystem::CalculateOptimalDifficulty(const FString& TutorialID) const
{
    // Calculate optimal difficulty based on player's past performance
    // This is a simplified implementation
    return ETutorialDifficultyOnboarding::Intermediate;
}

void UTutorialOnboardingSystem::CreateTutorialUI()
{
    // Create tutorial widget
    // This would load the tutorial UI widget from assets
    UE_LOG(LogTemp, Log, TEXT("Creating tutorial UI"));
}

void UTutorialOnboardingSystem::DestroyTutorialUI()
{
    if (TutorialWidget)
    {
        TutorialWidget->RemoveFromParent();
        TutorialWidget = nullptr;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Destroying tutorial UI"));
}

void UTutorialOnboardingSystem::CreateHighlightComponents()
{
    if (!GetWorld()) return;
    
    // Create highlight mesh
    HighlightMesh = NewObject<UStaticMeshComponent>(GetWorld());
    if (HighlightMesh)
    {
        HighlightMesh->RegisterComponent();
        HighlightMesh->SetVisibility(false);
    }
    
    // Create highlight text
    HighlightText = NewObject<UTextRenderComponent>(GetWorld());
    if (HighlightText)
    {
        HighlightText->RegisterComponent();
        HighlightText->SetVisibility(false);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Creating highlight components"));
}

void UTutorialOnboardingSystem::DestroyHighlightComponents()
{
    if (HighlightMesh)
    {
        HighlightMesh->DestroyComponent();
        HighlightMesh = nullptr;
    }
    
    if (HighlightText)
    {
        HighlightText->DestroyComponent();
        HighlightText = nullptr;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Destroying highlight components"));
}

bool UTutorialOnboardingSystem::IsVRMode() const
{
    // Check if the game is running in VR mode
    // This would integrate with the VR system
    // VR mode checking disabled for UE 5.6 compatibility
    return false; // GEngine && GEngine->IsStereoEnabled();
}

void UTutorialOnboardingSystem::SetupVRTutorialMode()
{
    if (!IsVRMode()) return;
    
    // Setup VR-specific tutorial features
    ShowVRTutorialHints();
    
    UE_LOG(LogTemp, Log, TEXT("Setting up VR tutorial mode"));
}

void UTutorialOnboardingSystem::CleanupVRTutorialMode()
{
    if (!IsVRMode()) return;
    
    // Cleanup VR-specific tutorial features
    HideVRTutorialHints();
    
    UE_LOG(LogTemp, Log, TEXT("Cleaning up VR tutorial mode"));
}

ETutorialPlatform UTutorialOnboardingSystem::GetCurrentPlatform() const
{
    // Determine current platform
    if (IsVRMode())
    {
        return ETutorialPlatform::VR;
    }
    
#if PLATFORM_WINDOWS
    return ETutorialPlatform::Desktop;
#elif PLATFORM_ANDROID || PLATFORM_IOS
    return ETutorialPlatform::Mobile;
#else
    return ETutorialPlatform::Desktop;
#endif
}

void UTutorialOnboardingSystem::RegisterDefaultTutorials()
{
    // Register default tutorials
    FTutorialDefinition BasicMovementTutorial;
    BasicMovementTutorial.TutorialID = "BasicMovement";
    BasicMovementTutorial.Title = "Basic Movement";
    BasicMovementTutorial.Description = "Learn how to move your character";
    BasicMovementTutorial.Difficulty = ETutorialDifficultyOnboarding::Beginner;
    BasicMovementTutorial.bIsMandatory = true;
    BasicMovementTutorial.EstimatedDuration = 60.0f;
    
    FTutorialOnboardingStep MovementStep;
    MovementStep.StepID = "Movement_Step1";
    MovementStep.Title = "Move Forward";
    MovementStep.Description = "Use the movement controls to move forward";
    MovementStep.StepType = ETutorialStepType::Navigation;
    MovementStep.Duration = 10.0f;
    MovementStep.RequiredInputs.Add("MoveForward");
    
    BasicMovementTutorial.Steps.Add(MovementStep);
    
    RegisterTutorial(BasicMovementTutorial);
    
    UE_LOG(LogTemp, Log, TEXT("Registered default tutorials"));
}

void UTutorialOnboardingSystem::UnlockInitialTutorials()
{
    // Unlock initial tutorials that don't have prerequisites
    for (const auto& TutorialPair : TutorialDefinitions)
    {
        const FString& TutorialID = TutorialPair.Key;
        const FTutorialDefinition& Tutorial = TutorialPair.Value;
        
        if (Tutorial.Prerequisites.Num() == 0)
        {
            UnlockTutorial(TutorialID);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Unlocked initial tutorials"));
}

void UTutorialOnboardingSystem::DrawDebugTutorialInfo()
{
    if (!GetWorld() || !bEnableDebugTutorials) return;

    // Draw debug information for active tutorials
    for (const auto& ProgressPair : TutorialProgress)
    {
        const FString& TutorialID = ProgressPair.Key;
        const FTutorialOnboardingProgress& Progress = ProgressPair.Value;
        
        if (Progress.CompletionState == ETutorialCompletionState::InProgress)
        {
            // Draw tutorial info at player location
            APawn* PlayerPawn = GetWorld()->GetFirstPlayerController() ? 
                GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
            
            if (PlayerPawn)
            {
                FVector Location = PlayerPawn->GetActorLocation();
                DrawDebugString(GetWorld(), Location + FVector(0, 0, 200), 
                    FString::Printf(TEXT("Tutorial: %s\nStep: %d/%d"), 
                        *TutorialID, Progress.CurrentStepIndex + 1, 
                        TutorialDefinitions.Contains(TutorialID) ? TutorialDefinitions[TutorialID].Steps.Num() : 0), 
                    nullptr, FColor::Yellow, 0.0f);
            }
        }
    }
    
    // Draw highlight info
    if (bHighlightVisible && HighlightMesh)
    {
        DrawDebugSphere(GetWorld(), HighlightMesh->GetComponentLocation(), 50.0f, 16, FColor::Yellow, false, 0.0f);
    }
}

void UTutorialOnboardingSystem::UnregisterTutorialEventHandler(const FString& EventName)
{
    // Remove the event handler for the specified event
    if (EventHandlers.Contains(EventName))
    {
        EventHandlers.Remove(EventName);
        UE_LOG(LogTemp, Log, TEXT("Unregistered tutorial event handler: %s"), *EventName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not find tutorial event handler: %s"), *EventName);
    }
}
