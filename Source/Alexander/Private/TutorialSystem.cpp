// Copyright Epic Games, Inc. All Rights Reserved.

#include "TutorialSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ATutorialSystem::ATutorialSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 Hz

    // Settings
    bTutorialsEnabled = true;
    bContextHintsEnabled = true;
    bAutoStartOnboarding = true;
    ContextHintCheckInterval = 2.0f;
    MaxActiveSteps = 3;
    bSaveProgressAutomatically = true;

    // Timing
    LastContextCheckTime = 0.0f;
    OnboardingStartTime = 0.0f;
}

void ATutorialSystem::BeginPlay()
{
    Super::BeginPlay();

    // Load saved progress
    LoadTutorialProgress();

    // Auto-start onboarding if enabled and not completed
    if (bAutoStartOnboarding && !Progress.bOnboardingCompleted)
    {
        StartOnboarding();
    }

    UpdateTutorialStats();
}

void ATutorialSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bTutorialsEnabled)
    {
        return;
    }

    // Update active tutorials
    UpdateActiveTutorials(DeltaTime);

    // Check context hints periodically
    if (bContextHintsEnabled)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastContextCheckTime >= ContextHintCheckInterval)
        {
            CheckContextHints();
            LastContextCheckTime = CurrentTime;
        }
    }

    // Update stats
    UpdateTutorialStats();
}

void ATutorialSystem::StartTutorialStep(const FString& StepID)
{
    if (!bTutorialsEnabled)
    {
        return;
    }

    FTutorialSystemStep* Step = TutorialSteps.Find(StepID);
    if (!Step)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tutorial step not found: %s"), *StepID);
        return;
    }

    // Check if already active or completed
    if (Step->State == ETutorialStepState::InProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tutorial step already active: %s"), *StepID);
        return;
    }

    if (Step->State == ETutorialStepState::Completed)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tutorial step already completed: %s"), *StepID);
        return;
    }

    // Check prerequisites
    if (!ArePrerequisitesMet(*Step))
    {
        UE_LOG(LogTemp, Warning, TEXT("Tutorial prerequisites not met: %s"), *StepID);
        return;
    }

    // Check max active steps
    if (ActiveSteps.Num() >= MaxActiveSteps)
    {
        UE_LOG(LogTemp, Warning, TEXT("Max active tutorial steps reached"));
        return;
    }

    // Start the step
    Step->State = ETutorialStepState::InProgress;
    Step->StartTime = GetWorld()->GetTimeSeconds();
    ActiveSteps.Add(StepID);

    // Update progress
    Progress.StepStates.Add(StepID, ETutorialStepState::InProgress);

    // Broadcast event
    OnTutorialStepStarted.Broadcast(StepID, *Step);

    UE_LOG(LogTemp, Log, TEXT("Started tutorial step: %s - %s"), *StepID, *Step->Title.ToString());
}

void ATutorialSystem::CompleteTutorialStep(const FString& StepID)
{
    FTutorialSystemStep* Step = TutorialSteps.Find(StepID);
    if (!Step || Step->State != ETutorialStepState::InProgress)
    {
        return;
    }

    // Calculate completion time
    float CompletionTime = GetWorld()->GetTimeSeconds() - Step->StartTime;

    // Update step state
    Step->State = ETutorialStepState::Completed;
    ActiveSteps.Remove(StepID);

    // Update progress
    Progress.StepStates.Add(StepID, ETutorialStepState::Completed);
    Progress.CompletedSteps.AddUnique(StepID);
    Progress.TotalStepsCompleted++;
    Progress.TotalTutorialTime += CompletionTime;

    // Save progress if auto-save enabled
    if (bSaveProgressAutomatically)
    {
        SaveTutorialProgress();
    }

    // Broadcast event
    OnTutorialStepCompleted.Broadcast(StepID, CompletionTime);

    UE_LOG(LogTemp, Log, TEXT("Completed tutorial step: %s in %.2f seconds"), *StepID, CompletionTime);
}

void ATutorialSystem::SkipTutorialStep(const FString& StepID)
{
    FTutorialSystemStep* Step = TutorialSteps.Find(StepID);
    if (!Step || Step->State != ETutorialStepState::InProgress)
    {
        return;
    }

    if (!Step->bCanSkip)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tutorial step cannot be skipped: %s"), *StepID);
        return;
    }

    // Update step state
    Step->State = ETutorialStepState::Skipped;
    ActiveSteps.Remove(StepID);

    // Update progress
    Progress.StepStates.Add(StepID, ETutorialStepState::Skipped);
    Progress.SkippedSteps.AddUnique(StepID);

    // Save progress
    if (bSaveProgressAutomatically)
    {
        SaveTutorialProgress();
    }

    // Broadcast event
    OnTutorialStepSkipped.Broadcast(StepID);

    UE_LOG(LogTemp, Log, TEXT("Skipped tutorial step: %s"), *StepID);
}

void ATutorialSystem::ResetTutorialStep(const FString& StepID)
{
    FTutorialSystemStep* Step = TutorialSteps.Find(StepID);
    if (!Step)
    {
        return;
    }

    // Reset step state
    Step->State = ETutorialStepState::NotStarted;
    Step->StartTime = 0.0f;
    ActiveSteps.Remove(StepID);

    // Update progress
    Progress.StepStates.Remove(StepID);
    Progress.CompletedSteps.Remove(StepID);
    Progress.SkippedSteps.Remove(StepID);

    UE_LOG(LogTemp, Log, TEXT("Reset tutorial step: %s"), *StepID);
}

bool ATutorialSystem::IsTutorialStepActive(const FString& StepID) const
{
    const FTutorialSystemStep* Step = TutorialSteps.Find(StepID);
    return Step && Step->State == ETutorialStepState::InProgress;
}

bool ATutorialSystem::IsTutorialStepCompleted(const FString& StepID) const
{
    const FTutorialSystemStep* Step = TutorialSteps.Find(StepID);
    return Step && Step->State == ETutorialStepState::Completed;
}

void ATutorialSystem::RegisterTutorialStep(const FTutorialSystemStep& Step)
{
    TutorialSteps.Add(Step.StepID, Step);
    UE_LOG(LogTemp, Log, TEXT("Registered tutorial step: %s - %s"), *Step.StepID, *Step.Title.ToString());
}

void ATutorialSystem::StartOnboarding()
{
    if (Progress.bOnboardingCompleted)
    {
        UE_LOG(LogTemp, Warning, TEXT("Onboarding already completed"));
        return;
    }

    OnboardingStartTime = GetWorld()->GetTimeSeconds();

    // Start first onboarding step if available
    if (OnboardingSteps.Num() > 0)
    {
        StartTutorialStep(OnboardingSteps[0]);
    }

    UE_LOG(LogTemp, Log, TEXT("Started onboarding"));
}

void ATutorialSystem::CompleteOnboarding()
{
    if (Progress.bOnboardingCompleted)
    {
        return;
    }

    float TotalTime = GetWorld()->GetTimeSeconds() - OnboardingStartTime;
    Progress.bOnboardingCompleted = true;

    // Save progress
    if (bSaveProgressAutomatically)
    {
        SaveTutorialProgress();
    }

    // Broadcast event
    OnOnboardingCompleted.Broadcast(TotalTime);

    UE_LOG(LogTemp, Log, TEXT("Completed onboarding in %.2f seconds"), TotalTime);
}

bool ATutorialSystem::IsOnboardingComplete() const
{
    return Progress.bOnboardingCompleted;
}

void ATutorialSystem::ResetOnboarding()
{
    Progress.bOnboardingCompleted = false;
    OnboardingStartTime = 0.0f;

    // Reset all onboarding steps
    for (const FString& StepID : OnboardingSteps)
    {
        ResetTutorialStep(StepID);
    }

    UE_LOG(LogTemp, Log, TEXT("Reset onboarding"));
}

void ATutorialSystem::RegisterContextHint(const FContextHint& Hint)
{
    ContextHints.Add(Hint.HintID, Hint);
    UE_LOG(LogTemp, Log, TEXT("Registered context hint: %s"), *Hint.HintID);
}

void ATutorialSystem::TriggerContextHint(const FString& HintID)
{
    if (!bContextHintsEnabled)
    {
        return;
    }

    FContextHint* Hint = ContextHints.Find(HintID);
    if (!Hint)
    {
        return;
    }

    if (!CanShowContextHint(HintID))
    {
        return;
    }

    // Update hint tracking
    Hint->TimesDisplayed++;
    Hint->LastDisplayTime = GetWorld()->GetTimeSeconds();

    // Broadcast event
    OnContextHintTriggered.Broadcast(HintID, Hint->HintText);

    UE_LOG(LogTemp, Log, TEXT("Triggered context hint: %s"), *HintID);
}

void ATutorialSystem::CheckContextHints()
{
    if (!bContextHintsEnabled)
    {
        return;
    }

    // This is a placeholder for context-based hint triggering
    // In a real implementation, this would check game state and trigger relevant hints
    // For now, it just updates the last check time
}

bool ATutorialSystem::CanShowContextHint(const FString& HintID) const
{
    const FContextHint* Hint = ContextHints.Find(HintID);
    if (!Hint)
    {
        return false;
    }

    // Check max display count
    if (Hint->MaxDisplayCount > 0 && Hint->TimesDisplayed >= Hint->MaxDisplayCount)
    {
        return false;
    }

    // Check cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - Hint->LastDisplayTime < Hint->CooldownSeconds)
    {
        return false;
    }

    return true;
}

FTutorialSystemProgress ATutorialSystem::GetTutorialProgress() const
{
    return Progress;
}

void ATutorialSystem::SaveTutorialProgress()
{
    // In a real implementation, this would save to a file or player save game
    // For now, it's just a placeholder
    UE_LOG(LogTemp, Log, TEXT("Saved tutorial progress - %d steps completed"), Progress.TotalStepsCompleted);
}

void ATutorialSystem::LoadTutorialProgress()
{
    // In a real implementation, this would load from a file or player save game
    // For now, it's just a placeholder
    UE_LOG(LogTemp, Log, TEXT("Loaded tutorial progress"));
}

float ATutorialSystem::GetCategoryProgress(ETutorialCategory Category) const
{
    int32 TotalSteps = 0;
    int32 CompletedSteps = 0;

    for (const auto& Pair : TutorialSteps)
    {
        if (Pair.Value.Category == Category)
        {
            TotalSteps++;
            if (Pair.Value.State == ETutorialStepState::Completed)
            {
                CompletedSteps++;
            }
        }
    }

    return TotalSteps > 0 ? (float)CompletedSteps / (float)TotalSteps : 0.0f;
}

TArray<FTutorialSystemStep> ATutorialSystem::GetStepsByCategory(ETutorialCategory Category) const
{
    TArray<FTutorialSystemStep> Steps;
    for (const auto& Pair : TutorialSteps)
    {
        if (Pair.Value.Category == Category)
        {
            Steps.Add(Pair.Value);
        }
    }
    return Steps;
}

TArray<FTutorialSystemStep> ATutorialSystem::GetAvailableSteps() const
{
    TArray<FTutorialSystemStep> AvailableSteps;
    for (const auto& Pair : TutorialSteps)
    {
        const FTutorialSystemStep& Step = Pair.Value;
        if (Step.State == ETutorialStepState::NotStarted && ArePrerequisitesMet(Step))
        {
            AvailableSteps.Add(Step);
        }
    }
    return AvailableSteps;
}

FTutorialSystemStep ATutorialSystem::GetStepInfo(const FString& StepID) const
{
    const FTutorialSystemStep* Step = TutorialSteps.Find(StepID);
    return Step ? *Step : FTutorialSystemStep();
}

FTutorialStats ATutorialSystem::GetTutorialStats() const
{
    return Stats;
}

void ATutorialSystem::SetTutorialsEnabled(bool bEnabled)
{
    bTutorialsEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Tutorials %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

bool ATutorialSystem::AreTutorialsEnabled() const
{
    return bTutorialsEnabled;
}

void ATutorialSystem::SetContextHintsEnabled(bool bEnabled)
{
    bContextHintsEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Context hints %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

bool ATutorialSystem::AreContextHintsEnabled() const
{
    return bContextHintsEnabled;
}

void ATutorialSystem::UpdateActiveTutorials(float DeltaTime)
{
    TArray<FString> StepsToCheck = ActiveSteps;
    for (const FString& StepID : StepsToCheck)
    {
        CheckStepCompletion(StepID);
        CheckStepTimeout(StepID, DeltaTime);
    }
}

void ATutorialSystem::CheckStepCompletion(const FString& StepID)
{
    // This is a placeholder for custom completion condition checking
    // In a real implementation, this would evaluate the completion condition string
    // and check if the condition is met in the game state
}

void ATutorialSystem::CheckStepTimeout(const FString& StepID, float DeltaTime)
{
    FTutorialSystemStep* Step = TutorialSteps.Find(StepID);
    if (!Step || Step->State != ETutorialStepState::InProgress)
    {
        return;
    }

    if (Step->TimeoutSeconds <= 0.0f)
    {
        return;
    }

    float ElapsedTime = GetWorld()->GetTimeSeconds() - Step->StartTime;
    if (ElapsedTime >= Step->TimeoutSeconds)
    {
        Step->State = ETutorialStepState::Failed;
        ActiveSteps.Remove(StepID);

        UE_LOG(LogTemp, Warning, TEXT("Tutorial step timed out: %s"), *StepID);
    }
}

bool ATutorialSystem::ArePrerequisitesMet(const FTutorialSystemStep& Step) const
{
    for (const FString& PrereqID : Step.PrerequisiteSteps)
    {
        const FTutorialSystemStep* PrereqStep = TutorialSteps.Find(PrereqID);
        if (!PrereqStep || PrereqStep->State != ETutorialStepState::Completed)
        {
            return false;
        }
    }
    return true;
}

void ATutorialSystem::UpdateTutorialStats()
{
    Stats.TotalSteps = TutorialSteps.Num();
    Stats.ActiveSteps = ActiveSteps.Num();
    Stats.CompletedSteps = Progress.CompletedSteps.Num();
    Stats.SkippedSteps = Progress.SkippedSteps.Num();
    Stats.TotalTime = Progress.TotalTutorialTime;

    if (Stats.CompletedSteps > 0)
    {
        Stats.AverageStepTime = Stats.TotalTime / (float)Stats.CompletedSteps;
    }
}
