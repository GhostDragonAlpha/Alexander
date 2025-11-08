#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "Animation/WidgetAnimation.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundCue.h"
#include "TutorialOnboardingSystem.generated.h"

class UUserWidget;
class UWidgetAnimation;
class USoundBase;
class USoundCue;
class APlayerController;
class UTextRenderComponent;

// Tutorial Step Types
UENUM(BlueprintType)
enum class ETutorialStepType : uint8
{
    None,
    Information,
    Interaction,
    Navigation,
    Combat,
    UI,
    VR,
    Advanced,
    Custom
};

// Tutorial Completion States
UENUM(BlueprintType)
enum class ETutorialCompletionState : uint8
{
    NotStarted,
    InProgress,
    Completed,
    Skipped,
    Failed
};

// Tutorial Difficulty Levels
UENUM(BlueprintType)
enum class ETutorialDifficultyOnboarding : uint8
{
    Beginner,
    Intermediate,
    Advanced,
    Expert
};

// Tutorial Platform Types
UENUM(BlueprintType)
enum class ETutorialPlatform : uint8
{
    All,
    Desktop,
    VR,
    Console,
    Mobile
};

// Tutorial Step Definition
USTRUCT(BlueprintType)
struct FTutorialOnboardingStep
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString StepID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETutorialStepType StepType = ETutorialStepType::Information;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETutorialDifficultyOnboarding Difficulty = ETutorialDifficultyOnboarding::Beginner;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETutorialPlatform Platform = ETutorialPlatform::All;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOptional = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanSkip = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresVR = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector HighlightLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetWidgetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredInputs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> SuccessConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> FailureConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* VoiceOver = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UWidgetAnimation* Animation = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> CustomParameters;
};

// Tutorial Progress Data
USTRUCT(BlueprintType)
struct FTutorialOnboardingProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TutorialID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETutorialCompletionState CompletionState = ETutorialCompletionState::NotStarted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentStepIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CompletionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Attempts = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Skips = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> CompletedSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> FailedSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> StepCompletionTimes;
};

// Tutorial Definition
USTRUCT(BlueprintType)
struct FTutorialDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TutorialID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETutorialDifficultyOnboarding Difficulty = ETutorialDifficultyOnboarding::Beginner;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETutorialPlatform Platform = ETutorialPlatform::All;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMandatory = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRepeatable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EstimatedDuration = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FTutorialOnboardingStep> Steps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CompletionEvent;
};

// Tutorial Highlight Data
USTRUCT(BlueprintType)
struct FTutorialHighlight
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetWidgetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor HighlightColor = FLinearColor::Yellow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HighlightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPulseEffect = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowArrow = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ArrowOffset = FVector::ZeroVector;
};

// VR Tutorial Data
USTRUCT(BlueprintType)
struct FVRTutorialData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresVR = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresHandTracking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresRoomScale = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinimumPlayArea = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredVRInputs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> VRSpecificSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> VRInstructions;
};

// Tutorial Event Data
USTRUCT(BlueprintType)
struct FTutorialEventData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TutorialID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString StepID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EventTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> EventData;
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTutorialStarted, const FString&, TutorialID, APlayerController*, PlayerController);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTutorialCompleted, const FString&, TutorialID, float, CompletionTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTutorialStepStarted, const FString&, StepID, const FString&, StepTitle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTutorialStepCompleted, const FString&, StepID, float, CompletionTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTutorialSkipped, const FString&, TutorialID, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTutorialEvent, const FString&, EventName, const FString&, TutorialID, const FString&, StepID);

/**
 * Tutorial & Onboarding System
 * Provides comprehensive tutorial and onboarding functionality with VR support, 
 * progressive unlocking, and adaptive difficulty.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Tutorial))
class ALEXANDER_API UTutorialOnboardingSystem : public UObject
{
    GENERATED_BODY()

public:
    UTutorialOnboardingSystem();

    // System Management
    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void InitializeTutorialSystem();

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void ShutdownTutorialSystem();

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void UpdateTutorialSystem(float DeltaTime);

    // Tutorial Management
    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void RegisterTutorial(const FTutorialDefinition& Tutorial);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void UnregisterTutorial(const FString& TutorialID);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void StartTutorial(const FString& TutorialID, APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void CompleteTutorial(const FString& TutorialID);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void SkipTutorial(const FString& TutorialID, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void PauseTutorial(const FString& TutorialID);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void ResumeTutorial(const FString& TutorialID);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void RestartTutorial(const FString& TutorialID);

    // Tutorial Progress
    UFUNCTION(BlueprintPure, Category = "Tutorial System")
    FTutorialOnboardingProgress GetTutorialProgress(const FString& TutorialID) const;

    UFUNCTION(BlueprintPure, Category = "Tutorial System")
    bool IsTutorialCompleted(const FString& TutorialID) const;

    UFUNCTION(BlueprintPure, Category = "Tutorial System")
    bool IsTutorialInProgress(const FString& TutorialID) const;

    UFUNCTION(BlueprintPure, Category = "Tutorial System")
    TArray<FString> GetCompletedTutorials() const;

    UFUNCTION(BlueprintPure, Category = "Tutorial System")
    TArray<FString> GetAvailableTutorials() const;

    UFUNCTION(BlueprintPure, Category = "Tutorial System")
    TArray<FString> GetMandatoryTutorials() const;

    // Tutorial Steps
    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void StartTutorialStep(const FString& StepID);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void CompleteTutorialStep(const FString& StepID);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void SkipTutorialStep(const FString& StepID);

    UFUNCTION(BlueprintPure, Category = "Tutorial System")
    FTutorialOnboardingStep GetCurrentStep() const;

    UFUNCTION(BlueprintPure, Category = "Tutorial System")
    int32 GetCurrentStepIndex() const;

    UFUNCTION(BlueprintPure, Category = "Tutorial System")
    TArray<FTutorialOnboardingStep> GetTutorialSteps(const FString& TutorialID) const;

    UFUNCTION(BlueprintPure, Category = "Tutorial System")
    bool IsTutorialStepCompleted(const FString& StepID) const;

    // Tutorial Display
    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void ShowTutorialUI(const FString& TutorialID);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void HideTutorialUI();

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void UpdateTutorialUI(const FTutorialOnboardingStep& Step);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void ShowTutorialHighlight(const FTutorialHighlight& Highlight);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void HideTutorialHighlight();

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void PlayTutorialVoiceOver(USoundCue* VoiceOver);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void StopTutorialVoiceOver();

    // VR Tutorial Support
    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void SetVRTutorialData(const FVRTutorialData& VRData);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    FVRTutorialData GetVRTutorialData() const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    bool IsVRTutorial(const FString& TutorialID) const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void ShowVRTutorialHints();

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void HideVRTutorialHints();

    // Input Handling
    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void ProcessTutorialInput(const FString& InputName, bool bPressed);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void RegisterInputBinding(const FString& StepID, const FString& InputName);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void UnregisterInputBinding(const FString& InputName);

    // Condition Checking
    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    bool CheckStepConditions(const FTutorialOnboardingStep& Step);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    bool CheckSuccessConditions(const FString& StepID);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    bool CheckFailureConditions(const FString& StepID);

    // Register condition checker (C++ only)
    void RegisterConditionChecker(const FString& ConditionName, TFunction<bool()> Checker);

    // Tutorial Events
    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void TriggerTutorialEvent(const FTutorialEventData& EventData);

    // Register tutorial event handler (C++ only)
    void RegisterTutorialEventHandler(const FString& EventName, TFunction<void(const FTutorialEventData&)> Handler);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void UnregisterTutorialEventHandler(const FString& EventName);

    // Progressive Unlocking
    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void UnlockTutorial(const FString& TutorialID);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void LockTutorial(const FString& TutorialID);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    bool IsTutorialUnlocked(const FString& TutorialID) const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    TArray<FString> GetUnlockedTutorials() const;

    // Adaptive Difficulty
    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void SetAdaptiveDifficulty(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    bool IsAdaptiveDifficultyEnabled() const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void AdjustTutorialDifficulty(const FString& TutorialID, ETutorialDifficultyOnboarding NewDifficulty);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    ETutorialDifficultyOnboarding GetRecommendedDifficulty(const FString& TutorialID) const;

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Tutorial System")
    FTutorialDefinition GetTutorialDefinition(const FString& TutorialID) const;

    UFUNCTION(BlueprintPure, Category = "Tutorial System")
    TArray<FTutorialDefinition> GetAllTutorials() const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void ResetTutorialProgress(const FString& TutorialID);

    UFUNCTION(BlueprintCallable, Category = "Tutorial System")
    void ResetAllTutorialProgress();

    UFUNCTION(BlueprintPure, Category = "Tutorial System")
    float GetTutorialCompletionPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Tutorial System")
    int32 GetTotalTutorialsCompleted() const;

    UFUNCTION(BlueprintPure, Category = "Tutorial System")
    int32 GetTotalTutorialsAvailable() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Tutorial System")
    FOnTutorialStarted OnTutorialStarted;

    UPROPERTY(BlueprintAssignable, Category = "Tutorial System")
    FOnTutorialCompleted OnTutorialCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Tutorial System")
    FOnTutorialStepStarted OnTutorialStepStarted;

    UPROPERTY(BlueprintAssignable, Category = "Tutorial System")
    FOnTutorialStepCompleted OnTutorialStepCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Tutorial System")
    FOnTutorialSkipped OnTutorialSkipped;

    UPROPERTY(BlueprintAssignable, Category = "Tutorial System")
    FOnTutorialEvent OnTutorialEvent;

protected:
    // Tutorial Data Storage
    UPROPERTY(BlueprintReadOnly, Category = "Tutorial System")
    TMap<FString, FTutorialDefinition> TutorialDefinitions;

    UPROPERTY(BlueprintReadOnly, Category = "Tutorial System")
    TMap<FString, FTutorialOnboardingProgress> TutorialProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Tutorial System")
    TMap<FString, FString> ActiveTutorials; // TutorialID -> PlayerController

    UPROPERTY(BlueprintReadOnly, Category = "Tutorial System")
    TMap<FString, FString> InputBindings; // InputName -> StepID

    // Condition checkers (C++ only)
    TMap<FString, TFunction<bool()>> ConditionCheckers;

    // Event handlers (C++ only)
    TMap<FString, TFunction<void(const FTutorialEventData&)>> EventHandlers;

    // VR Tutorial Data
    UPROPERTY(BlueprintReadOnly, Category = "Tutorial System")
    FVRTutorialData CurrentVRData;

    // UI Components
    UPROPERTY(BlueprintReadOnly, Category = "Tutorial System")
    UUserWidget* TutorialWidget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Tutorial System")
    UTextRenderComponent* HighlightText = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Tutorial System")
    UStaticMeshComponent* HighlightMesh = nullptr;

    // Settings and Configuration
    UPROPERTY(BlueprintReadOnly, Category = "Tutorial System")
    TArray<FString> UnlockedTutorials;

    UPROPERTY(BlueprintReadOnly, Category = "Tutorial System")
    bool bAdaptiveDifficulty = true;

    UPROPERTY(BlueprintReadOnly, Category = "Tutorial System")
    bool bTutorialUIVisible = false;

    UPROPERTY(BlueprintReadOnly, Category = "Tutorial System")
    bool bHighlightVisible = false;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial System")
    bool bEnableTutorials = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial System")
    bool bShowHints = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial System")
    bool bAutoAdvance = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial System")
    float TutorialUpdateInterval = 0.016f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial System")
    bool bEnableDebugTutorials = false;

private:
    float LastUpdateTime;
    FString CurrentTutorialID;
    FString CurrentStepID;
    TMap<FString, float> StepStartTimes;

    void UpdateActiveTutorials(float DeltaTime);
    void UpdateTutorialUI(float DeltaTime);
    void UpdateHighlights(float DeltaTime);

    void ProcessStepCompletion(const FString& StepID);
    void ProcessStepFailure(const FString& StepID);
    void AdvanceToNextStep();

    bool CheckPrerequisites(const FString& TutorialID);
    void UnlockPrerequisiteTutorials(const FString& TutorialID);

    void ShowStepHighlight(const FTutorialOnboardingStep& Step);
    void HideStepHighlight();

    void PlayStepAnimation(const FTutorialOnboardingStep& Step);
    void StopStepAnimation();

    void LogTutorialEvent(const FString& EventName, const FString& TutorialID, const FString& StepID, const TMap<FString, FString>& EventData);

    void SaveTutorialProgress();
    void LoadTutorialProgress();

    void AdjustDifficultyBasedOnPerformance(const FString& TutorialID);
    ETutorialDifficultyOnboarding CalculateOptimalDifficulty(const FString& TutorialID) const;

    void CreateTutorialUI();
    void DestroyTutorialUI();

    void RegisterDefaultTutorials();
    void UnlockInitialTutorials();
    ETutorialPlatform GetCurrentPlatform() const;

    void CreateHighlightComponents();
    void DestroyHighlightComponents();

    bool IsVRMode() const;
    void SetupVRTutorialMode();
    void CleanupVRTutorialMode();

    void DrawDebugTutorialInfo();
};
