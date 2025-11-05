#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Components/ActorComponent.h"
#include "AdvancedAIBehaviors.h"
#include "EnhancedVFXSystem.h"
#include "AudioSystemManager.h"
#include "TutorialOnboardingSystem.h"
#include "MissionBoardComponent.h"
#include "PlanetWeatherComponent.h"
#include "SpaceStationHub.h"
#include "GameSystemCoordinator.generated.h"

class UWorld;
class APlayerController;
class AGameModeBase;

// System Integration Status
UENUM(BlueprintType)
enum class ESystemIntegrationStatus : uint8
{
    NotInitialized,
    Initializing,
    Initialized,
    Error,
    Shutdown
};

// Cross-System Event Types
UENUM(BlueprintType)
enum class ECrossSystemEvent : uint8
{
    None,
    PlayerEnteredStation,
    PlayerLeftStation,
    CombatStarted,
    CombatEnded,
    WeatherChanged,
    MissionAccepted,
    MissionCompleted,
    TutorialStarted,
    TutorialCompleted,
    VRModeToggled,
    PerformanceModeChanged
};

// System Performance Data
USTRUCT(BlueprintType)
struct FSystemPerformanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AISystemLoad = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VFXSystemLoad = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AudioSystemLoad = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TutorialSystemLoad = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalSystemLoad = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveAIActors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveVFXEffects = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveAudioEvents = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveTutorials = 0;
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrossSystemEvent, ECrossSystemEvent, EventType, AActor*, SourceActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemPerformanceUpdated, const FSystemPerformanceData&, PerformanceData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSystemIntegrationStatusChanged, const FString&, SystemName, ESystemIntegrationStatus, Status);

/**
 * Game System Coordinator
 * Central coordination hub for all Phase 7 systems integration
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=SystemIntegration)
class ALEXANDER_API UGameSystemCoordinator : public UObject
{
    GENERATED_BODY()

public:
    UGameSystemCoordinator();

    // System Lifecycle
    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void InitializeSystems();

    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void ShutdownSystems();

    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void UpdateSystems(float DeltaTime);

    // System Access
    UFUNCTION(BlueprintPure, Category = "System Coordinator")
    static UGameSystemCoordinator* Get();

    UFUNCTION(BlueprintPure, Category = "System Coordinator")
    UAdvancedAIBehaviors* GetAISystem() const;

    UFUNCTION(BlueprintPure, Category = "System Coordinator")
    UEnhancedVFXSystem* GetVFXSystem() const;

    UFUNCTION(BlueprintPure, Category = "System Coordinator")
    UAudioSystemManager* GetAudioSystem() const;

    UFUNCTION(BlueprintPure, Category = "System Coordinator")
    UTutorialOnboardingSystem* GetTutorialSystem() const;

    // Cross-System Events
    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void TriggerCrossSystemEvent(ECrossSystemEvent EventType, AActor* SourceActor = nullptr);

    // Register cross-system event handler (C++ only)
    void RegisterCrossSystemEventHandler(ECrossSystemEvent EventType, TFunction<void(AActor*)> Handler);

    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void UnregisterCrossSystemEventHandler(ECrossSystemEvent EventType);

    // Integration Points
    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void IntegrateAIWithMissionSystem();

    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void IntegrateVFXWithWeatherSystem();

    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void IntegrateAudioWithEnvironmentSystem();

    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void IntegrateTutorialWithAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void SetupVRIntegration();

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void OptimizeSystemPerformance();

    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void SetPerformanceMode(bool bHighPerformance);

    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    FSystemPerformanceData GetSystemPerformanceData() const;

    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void EnableAdaptivePerformance(bool bEnabled);

    // System Status
    UFUNCTION(BlueprintPure, Category = "System Coordinator")
    bool AreSystemsInitialized() const;

    UFUNCTION(BlueprintPure, Category = "System Coordinator")
    ESystemIntegrationStatus GetSystemStatus(const FString& SystemName) const;

    UFUNCTION(BlueprintPure, Category = "System Coordinator")
    TArray<FString> GetSystemStatusList() const;

    // Player Integration
    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void OnPlayerJoined(APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void OnPlayerLeft(APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void SetupPlayerIntegration(APlayerController* PlayerController);

    // World Integration
    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void OnWorldBeginPlay(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void OnWorldEndPlay(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "System Coordinator")
    void SetupWorldIntegration(UWorld* World);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "System Coordinator")
    FOnCrossSystemEvent OnCrossSystemEvent;

    UPROPERTY(BlueprintAssignable, Category = "System Coordinator")
    FOnSystemPerformanceUpdated OnSystemPerformanceUpdated;

    UPROPERTY(BlueprintAssignable, Category = "System Coordinator")
    FOnSystemIntegrationStatusChanged OnSystemIntegrationStatusChanged;

protected:
    // System References
    UPROPERTY(BlueprintReadOnly, Category = "System Coordinator")
    UAdvancedAIBehaviors* AISystem;

    UPROPERTY(BlueprintReadOnly, Category = "System Coordinator")
    UEnhancedVFXSystem* VFXSystem;

    UPROPERTY(BlueprintReadOnly, Category = "System Coordinator")
    UAudioSystemManager* AudioSystem;

    UPROPERTY(BlueprintReadOnly, Category = "System Coordinator")
    UTutorialOnboardingSystem* TutorialSystem;

    // System Status
    UPROPERTY(BlueprintReadOnly, Category = "System Coordinator")
    TMap<FString, ESystemIntegrationStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "System Coordinator")
    bool bSystemsInitialized = false;

    // Performance
    UPROPERTY(BlueprintReadOnly, Category = "System Coordinator")
    FSystemPerformanceData PerformanceData;

    UPROPERTY(BlueprintReadOnly, Category = "System Coordinator")
    bool bAdaptivePerformance = true;

    UPROPERTY(BlueprintReadOnly, Category = "System Coordinator")
    bool bHighPerformanceMode = false;

    // Event Handlers
    // Cross-system event handlers
    TMap<ECrossSystemEvent, TArray<TFunction<void(AActor*)>>> CrossSystemEventHandlers;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Coordinator")
    float SystemUpdateInterval = 0.016f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Coordinator")
    bool bEnableDebugLogging = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Coordinator")
    bool bEnablePerformanceMonitoring = true;

private:
    static UGameSystemCoordinator* Instance;
    float LastUpdateTime;

    void InitializeIndividualSystems();
    void SetupSystemEventBindings();
    void SetupCrossSystemCommunication();
    void InitializePerformanceMonitoring();

    void UpdateSystemPerformance(float DeltaTime);
    void LogSystemPerformance();
    void AdjustPerformanceBasedOnLoad();

    void HandleAIEvents(AActor* SourceActor);
    void HandleVFXEvents(AActor* SourceActor);
    void HandleAudioEvents(AActor* SourceActor);
    void HandleTutorialEvents(AActor* SourceActor);

    void SetSystemStatus(const FString& SystemName, ESystemIntegrationStatus Status);
    void BroadcastSystemStatusChange(const FString& SystemName, ESystemIntegrationStatus Status);

    void CleanupSystemReferences();

    // Event Handlers
    UFUNCTION()
    void HandleAIStateChanged(AActor* AIActor, EAIState OldState, EAIState NewState);

    UFUNCTION()
    void HandleAIEmergencyTriggered(AActor* AIActor, const FString& EmergencyType);

    UFUNCTION()
    void HandleVFXEffectStarted(const FName& EffectName, EVFXEffectType EffectType);

    UFUNCTION()
    void HandleVFXEffectEnded(const FName& EffectName, float Duration);

    UFUNCTION()
    void HandleAudioEventTriggered(const FString& EventName, AActor* SourceActor);

    UFUNCTION()
    void HandleAudioZoneEntered(const FString& ZoneName, AActor* Actor);

    UFUNCTION()
    void HandleTutorialStartedEvent(const FString& TutorialID, APlayerController* PlayerController);

    UFUNCTION()
    void HandleTutorialCompletedEvent(const FString& TutorialID, float CompletionTime);

    // Cross-System Event Handlers
    void HandlePlayerEnteredStation(AActor* SourceActor);
    void HandlePlayerLeftStation(AActor* SourceActor);
    void HandleCombatStarted(AActor* SourceActor);
    void HandleCombatEnded(AActor* SourceActor);
    void HandleWeatherChanged(AActor* SourceActor);
    void HandleMissionAccepted(AActor* SourceActor);
    void HandleMissionCompleted(AActor* SourceActor);
    void HandleTutorialStarted(AActor* SourceActor);
    void HandleTutorialCompleted(AActor* SourceActor);
    void HandleVRModeToggled(AActor* SourceActor);
    void HandlePerformanceModeChanged(AActor* SourceActor);

    // Helper Functions
    TArray<AActor*> GetNearbyActors(AActor* SourceActor, float Radius);
    bool IsVRMode() const;
};