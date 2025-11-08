// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TutorialOnboardingSystem.h"
#include "FirstMissionGameMode.generated.h"

// Forward declarations
class ASpaceship;
class APlanet;
class UTutorialOnboardingSystem;
class UQuestSystem;
class UAudioManager;
class UVFXManager;

/**
 * Mission Stages
 */
UENUM(BlueprintType)
enum class EMissionStage : uint8
{
    NotStarted UMETA(DisplayName = "Not Started"),
    Briefing UMETA(DisplayName = "Mission Briefing"),
    PlanetSurface UMETA(DisplayName = "Planet Surface"),
    EnteringShip UMETA(DisplayName = "Entering Ship"),
    ShipTutorial UMETA(DisplayName = "Ship Tutorial"),
    Launch UMETA(DisplayName = "Launch"),
    SpaceFlight UMETA(DisplayName = "Space Flight"),
    MoonApproach UMETA(DisplayName = "Moon Approach"),
    MoonLanding UMETA(DisplayName = "Moon Landing"),
    MoonSurface UMETA(DisplayName = "Moon Surface"),
    MoonExploration UMETA(DisplayName = "Moon Exploration"),
    ReturnPrep UMETA(DisplayName = "Return Preparation"),
    ReturnFlight UMETA(DisplayName = "Return Flight"),
    PlanetApproach UMETA(DisplayName = "Planet Approach"),
    PlanetLanding UMETA(DisplayName = "Planet Landing"),
    MissionComplete UMETA(DisplayName = "Mission Complete"),
    MissionFailed UMETA(DisplayName = "Mission Failed")
};

/**
 * Mission Objective Definition
 */
USTRUCT(BlueprintType)
struct FMissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FText ObjectiveTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FText ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EMissionStage RequiredStage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 ExperienceReward;

    FMissionObjective()
        : RequiredStage(EMissionStage::NotStarted)
        , bIsComplete(false)
        , bIsOptional(false)
        , TargetLocation(FVector::ZeroVector)
        , CompletionRadius(100.0f)
        , ExperienceReward(100)
    {
    }
};

/**
 * Mission State Data
 */
USTRUCT(BlueprintType)
struct FMissionState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EMissionStage CurrentStage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float MissionStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float MissionElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ObjectivesCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 TotalObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bPlayerInShip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bShipLaunched;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bReachedMoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bLandedOnMoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bExploredMoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bReturnedToPlanet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 TotalExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 TotalCredits;

    FMissionState()
        : CurrentStage(EMissionStage::NotStarted)
        , MissionStartTime(0.0f)
        , MissionElapsedTime(0.0f)
        , ObjectivesCompleted(0)
        , TotalObjectives(0)
        , bPlayerInShip(false)
        , bShipLaunched(false)
        , bReachedMoon(false)
        , bLandedOnMoon(false)
        , bExploredMoon(false)
        , bReturnedToPlanet(false)
        , TotalExperience(0)
        , TotalCredits(0)
    {
    }
};

/**
 * Event Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionStageChanged, EMissionStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, const FString&, ObjectiveID, int32, ExperienceReward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionCompleted, const FMissionState&, FinalState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionFailed, const FString&, FailureReason);

/**
 * First Mission Game Mode
 * Orchestrates the "Moon Walk" first mission experience
 */
UCLASS()
class ALEXANDER_API AFirstMissionGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AFirstMissionGameMode();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Mission Control
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void StartMission();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void FailMission(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void RestartMission();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void SetMissionStage(EMissionStage NewStage);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    EMissionStage GetCurrentStage() const { return MissionState.CurrentStage; }

    UFUNCTION(BlueprintCallable, Category = "Mission")
    FMissionState GetMissionState() const { return MissionState; }

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void RegisterObjective(const FMissionObjective& Objective);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    bool IsObjectiveComplete(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission")
    TArray<FMissionObjective> GetActiveObjectives() const;

    UFUNCTION(BlueprintCallable, Category = "Mission")
    FMissionObjective GetCurrentObjective() const;

    // Player State Tracking
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void SetPlayerInShip(bool bInShip);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void NotifyShipLaunched();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void NotifyReachedMoon();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void NotifyLandedOnMoon();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void NotifyMoonExplorationComplete();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void NotifyReturnedToPlanet();

    // System References
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void RegisterPlayerShip(ASpaceship* Ship);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void RegisterPlanet(APlanet* Planet);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void RegisterMoon(APlanet* Moon);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    ASpaceship* GetPlayerShip() const { return PlayerShip; }

    UFUNCTION(BlueprintCallable, Category = "Mission")
    APlanet* GetPlanet() const { return Planet; }

    UFUNCTION(BlueprintCallable, Category = "Mission")
    APlanet* GetMoon() const { return Moon; }

    // Tutorial Integration
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void ShowTutorialStep(const FString& StepID);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void CompleteTutorialStep(const FString& StepID);

    // Distance & Navigation
    UFUNCTION(BlueprintCallable, Category = "Mission")
    float GetDistanceToMoon() const;

    UFUNCTION(BlueprintCallable, Category = "Mission")
    float GetDistanceToPlanet() const;

    UFUNCTION(BlueprintCallable, Category = "Mission")
    FVector GetMoonDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Mission")
    FVector GetPlanetDirection() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Mission|Events")
    FOnMissionStageChanged OnMissionStageChanged;

    UPROPERTY(BlueprintAssignable, Category = "Mission|Events")
    FOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Mission|Events")
    FOnMissionCompleted OnMissionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Mission|Events")
    FOnMissionFailed OnMissionFailed;

protected:
    // Internal Functions
    void InitializeMission();
    void SetupObjectives();
    void UpdateMissionProgress(float DeltaTime);
    void CheckObjectiveCompletion();
    void ProcessStageTransitions();
    void UpdateTutorialSteps();
    void PlayStageAudio(EMissionStage Stage);
    void SpawnStageVFX(EMissionStage Stage);

    // Mission Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FMissionState MissionState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TMap<FString, FMissionObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> ObjectiveOrder;

    // System References
    UPROPERTY()
    ASpaceship* PlayerShip;

    UPROPERTY()
    APlanet* Planet;

    UPROPERTY()
    APlanet* Moon;

    UPROPERTY()
    UTutorialOnboardingSystem* TutorialSystem;

    UPROPERTY()
    UQuestSystem* QuestSystem;

    UPROPERTY()
    UAudioManager* AudioManager;

    UPROPERTY()
    UVFXManager* VFXManager;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Config")
    float MoonDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Config")
    float LandingAltitudeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Config")
    float LandingVelocityThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Config")
    float ExplorationDistanceRequired;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Config")
    bool bUseRealisticDistances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Config")
    float TimeCompressionFactor;

    // Tracking
    FVector InitialPlayerLocation;
    FVector MoonExplorationStartLocation;
    float TotalDistanceTraveled;
    int32 CurrentObjectiveIndex;
};
