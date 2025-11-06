// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayScenario.generated.h"

// Forward declarations
class UPlayerProgression;

/**
 * Scenario Phase
 * Defines the different phases of a gameplay scenario
 */
UENUM(BlueprintType)
enum class EScenarioPhase : uint8
{
    NotStarted,
    Introduction,
    Tutorial,
    Exploration,
    Mission,
    Completed,
    Failed
};

/**
 * Scenario Objective
 * Individual objective within a scenario
 */
USTRUCT(BlueprintType)
struct FScenarioObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText ObjectiveTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOptional = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProgressPercent = 0.0f;
};

/**
 * Scenario Rewards
 * Rewards granted upon scenario completion
 */
USTRUCT(BlueprintType)
struct FScenarioRewards
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ExperiencePoints = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Credits = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlockedFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlockedLocations;
};

/**
 * Gameplay Scenario
 * Manages a complete gameplay scenario from start to completion
 * Examples: Tutorial, First Colony, Trade Route, Exploration Mission
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API AGameplayScenario : public AActor
{
    GENERATED_BODY()

public:
    AGameplayScenario();

    // Lifecycle
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Scenario Control
    UFUNCTION(BlueprintCallable, Category = "Scenario")
    void StartScenario();

    UFUNCTION(BlueprintCallable, Category = "Scenario")
    void CompleteScenario();

    UFUNCTION(BlueprintCallable, Category = "Scenario")
    void FailScenario(const FText& FailureReason);

    UFUNCTION(BlueprintCallable, Category = "Scenario")
    void AdvanceToPhase(EScenarioPhase NewPhase);

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Scenario")
    void AddObjective(const FScenarioObjective& Objective);

    UFUNCTION(BlueprintCallable, Category = "Scenario")
    void CompleteObjective(const FText& ObjectiveTitle);

    UFUNCTION(BlueprintCallable, Category = "Scenario")
    void UpdateObjectiveProgress(const FText& ObjectiveTitle, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Scenario")
    bool AreAllObjectivesComplete() const;

    // Queries
    UFUNCTION(BlueprintPure, Category = "Scenario")
    EScenarioPhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Scenario")
    TArray<FScenarioObjective> GetActiveObjectives() const;

    UFUNCTION(BlueprintPure, Category = "Scenario")
    float GetScenarioProgress() const;

    UFUNCTION(BlueprintPure, Category = "Scenario")
    bool IsScenarioActive() const;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    FText ScenarioName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    FText ScenarioDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    FScenarioRewards Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    float TimeLimit = 0.0f; // 0 = no limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    bool bAutoStart = false;

    // Progression Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    bool bAutoAwardProgressionRewards = true;

    UPROPERTY(BlueprintReadOnly, Category = "Scenario")
    UPlayerProgression* PlayerProgression;

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScenarioPhaseChanged, EScenarioPhase, NewPhase);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompleted, const FScenarioObjective&, Objective);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScenarioCompleted, const FScenarioRewards&, Rewards);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScenarioFailed, const FText&, FailureReason);

    UPROPERTY(BlueprintAssignable, Category = "Scenario")
    FOnScenarioPhaseChanged OnScenarioPhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "Scenario")
    FOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Scenario")
    FOnScenarioCompleted OnScenarioCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Scenario")
    FOnScenarioFailed OnScenarioFailed;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Scenario")
    EScenarioPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Scenario")
    TArray<FScenarioObjective> Objectives;

    UPROPERTY(BlueprintReadOnly, Category = "Scenario")
    float ElapsedTime;

    UPROPERTY(BlueprintReadOnly, Category = "Scenario")
    bool bScenarioStarted;

    // Phase handlers (override in Blueprint or C++)
    UFUNCTION(BlueprintNativeEvent, Category = "Scenario")
    void OnPhaseIntroduction();

    UFUNCTION(BlueprintNativeEvent, Category = "Scenario")
    void OnPhaseTutorial();

    UFUNCTION(BlueprintNativeEvent, Category = "Scenario")
    void OnPhaseExploration();

    UFUNCTION(BlueprintNativeEvent, Category = "Scenario")
    void OnPhaseMission();

    virtual void OnPhaseIntroduction_Implementation();
    virtual void OnPhaseTutorial_Implementation();
    virtual void OnPhaseExploration_Implementation();
    virtual void OnPhaseMission_Implementation();

private:
    void CheckScenarioCompletion();
    void CheckTimeLimit();
};
