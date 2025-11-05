// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialSystem.generated.h"

/**
 * Tutorial Step State
 */
UENUM(BlueprintType)
enum class ETutorialStepState : uint8
{
    NotStarted UMETA(DisplayName = "Not Started"),
    InProgress UMETA(DisplayName = "In Progress"),
    Completed UMETA(DisplayName = "Completed"),
    Skipped UMETA(DisplayName = "Skipped"),
    Failed UMETA(DisplayName = "Failed")
};

/**
 * Tutorial Category
 */
UENUM(BlueprintType)
enum class ETutorialCategory : uint8
{
    BasicControls UMETA(DisplayName = "Basic Controls"),
    FlightMechanics UMETA(DisplayName = "Flight Mechanics"),
    Combat UMETA(DisplayName = "Combat"),
    Trading UMETA(DisplayName = "Trading"),
    Exploration UMETA(DisplayName = "Exploration"),
    SocialFeatures UMETA(DisplayName = "Social Features"),
    StationServices UMETA(DisplayName = "Station Services"),
    MissionSystem UMETA(DisplayName = "Mission System"),
    ShipSystems UMETA(DisplayName = "Ship Systems"),
    Advanced UMETA(DisplayName = "Advanced")
};

/**
 * Prompt Display Type
 */
UENUM(BlueprintType)
enum class EPromptType : uint8
{
    Text UMETA(DisplayName = "Text Only"),
    TextWithArrow UMETA(DisplayName = "Text with Arrow"),
    Highlight UMETA(DisplayName = "Highlight Element"),
    Modal UMETA(DisplayName = "Modal Dialog"),
    Toast UMETA(DisplayName = "Toast Notification")
};

/**
 * Tutorial Step Definition
 */
USTRUCT(BlueprintType)
struct FTutorialSystemStep
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FString StepID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    ETutorialCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    EPromptType PromptType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TArray<FString> PrerequisiteSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FString CompletionCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    float TimeoutSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    bool bCanSkip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    bool bShowInHelpMenu;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FVector WorldLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    ETutorialStepState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    float StartTime;

    FTutorialSystemStep()
        : Category(ETutorialCategory::BasicControls)
        , PromptType(EPromptType::Text)
        , TimeoutSeconds(0.0f)
        , bCanSkip(true)
        , bShowInHelpMenu(true)
        , WorldLocation(FVector::ZeroVector)
        , TargetActor(nullptr)
        , State(ETutorialStepState::NotStarted)
        , StartTime(0.0f)
    {
    }
};

/**
 * Tutorial Progress Data
 */
USTRUCT(BlueprintType)
struct FTutorialSystemProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TMap<FString, ETutorialStepState> StepStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TArray<FString> CompletedSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TArray<FString> SkippedSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    bool bOnboardingCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    int32 TotalStepsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    float TotalTutorialTime;

    FTutorialSystemProgress()
        : bOnboardingCompleted(false)
        , TotalStepsCompleted(0)
        , TotalTutorialTime(0.0f)
    {
    }
};

/**
 * Context Hint Definition
 */
USTRUCT(BlueprintType)
struct FContextHint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FString HintID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FText HintText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    ETutorialCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FString TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    float CooldownSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    int32 MaxDisplayCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    int32 TimesDisplayed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    float LastDisplayTime;

    FContextHint()
        : Category(ETutorialCategory::BasicControls)
        , CooldownSeconds(60.0f)
        , MaxDisplayCount(3)
        , TimesDisplayed(0)
        , LastDisplayTime(0.0f)
    {
    }
};

/**
 * Tutorial Statistics
 */
USTRUCT(BlueprintType)
struct FTutorialStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    int32 TotalSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    int32 CompletedSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    int32 SkippedSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    int32 ActiveSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    float AverageStepTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    float TotalTime;

    FTutorialStats()
        : TotalSteps(0)
        , CompletedSteps(0)
        , SkippedSteps(0)
        , ActiveSteps(0)
        , AverageStepTime(0.0f)
        , TotalTime(0.0f)
    {
    }
};

/**
 * Event Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTutorialStepStarted, const FString&, StepID, const FTutorialSystemStep&, Step);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTutorialStepCompleted, const FString&, StepID, float, CompletionTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTutorialStepSkipped, const FString&, StepID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOnboardingCompleted, float, TotalTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnContextHintTriggered, const FString&, HintID, const FText&, HintText);

/**
 * Tutorial & Onboarding System
 * Manages interactive tutorials, onboarding flow, and context-sensitive help
 */
UCLASS(Blueprintable, ClassGroup=(Custom))
class ALEXANDER_API ATutorialSystem : public AActor
{
    GENERATED_BODY()

public:
    ATutorialSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Tutorial Management
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void StartTutorialStep(const FString& StepID);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void CompleteTutorialStep(const FString& StepID);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void SkipTutorialStep(const FString& StepID);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void ResetTutorialStep(const FString& StepID);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    bool IsTutorialStepActive(const FString& StepID) const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    bool IsTutorialStepCompleted(const FString& StepID) const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void RegisterTutorialStep(const FTutorialSystemStep& Step);

    // Onboarding Flow
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void StartOnboarding();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void CompleteOnboarding();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    bool IsOnboardingComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void ResetOnboarding();

    // Context Help
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void RegisterContextHint(const FContextHint& Hint);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void TriggerContextHint(const FString& HintID);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void CheckContextHints();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    bool CanShowContextHint(const FString& HintID) const;

    // Progress Management
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    FTutorialSystemProgress GetTutorialProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void SaveTutorialProgress();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void LoadTutorialProgress();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    float GetCategoryProgress(ETutorialCategory Category) const;

    // Query Functions
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    TArray<FTutorialSystemStep> GetStepsByCategory(ETutorialCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    TArray<FTutorialSystemStep> GetAvailableSteps() const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    FTutorialSystemStep GetStepInfo(const FString& StepID) const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    FTutorialStats GetTutorialStats() const;

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void SetTutorialsEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    bool AreTutorialsEnabled() const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void SetContextHintsEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    bool AreContextHintsEnabled() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Tutorial|Events")
    FOnTutorialStepStarted OnTutorialStepStarted;

    UPROPERTY(BlueprintAssignable, Category = "Tutorial|Events")
    FOnTutorialStepCompleted OnTutorialStepCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Tutorial|Events")
    FOnTutorialStepSkipped OnTutorialStepSkipped;

    UPROPERTY(BlueprintAssignable, Category = "Tutorial|Events")
    FOnOnboardingCompleted OnOnboardingCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Tutorial|Events")
    FOnContextHintTriggered OnContextHintTriggered;

protected:
    // Internal Functions
    void UpdateActiveTutorials(float DeltaTime);
    void CheckStepCompletion(const FString& StepID);
    void CheckStepTimeout(const FString& StepID, float DeltaTime);
    bool ArePrerequisitesMet(const FTutorialSystemStep& Step) const;
    void UpdateTutorialStats();

    // Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TMap<FString, FTutorialSystemStep> TutorialSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TMap<FString, FContextHint> ContextHints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FTutorialSystemProgress Progress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TArray<FString> ActiveSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TArray<FString> OnboardingSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FTutorialStats Stats;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Settings")
    bool bTutorialsEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Settings")
    bool bContextHintsEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Settings")
    bool bAutoStartOnboarding;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Settings")
    float ContextHintCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Settings")
    int32 MaxActiveSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Settings")
    bool bSaveProgressAutomatically;

    // Timing
    float LastContextCheckTime;
    float OnboardingStartTime;
};
