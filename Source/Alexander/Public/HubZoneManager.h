#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerVolume.h"
#include "SpaceStationHub.h"
#include "HubZoneManager.generated.h"

UENUM(BlueprintType)
enum class EZoneTransitionType : uint8
{
    Instant,
    Fade,
    Door,
    Elevator,
    Teleporter
};

UENUM(BlueprintType)
enum class EZoneEnvironmentType : uint8
{
    Indoor,
    Outdoor,
    ZeroG,
    LowGravity,
    HighGravity,
    Toxic,
    Vacuum
};

USTRUCT(BlueprintType)
struct FZoneTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName FromZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ToZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EZoneTransitionType TransitionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TransitionLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator TransitionRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresAccess;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UCurveFloat* TransitionCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundBase* TransitionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UParticleSystem* TransitionEffect;
};

USTRUCT(BlueprintType)
struct FZoneEnvironment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EZoneEnvironmentType EnvironmentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GravityScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AirPressure;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasOxygen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasRadiation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RadiationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* EnvironmentMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UParticleSystem* EnvironmentEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundBase* AmbientSound;
};

USTRUCT(BlueprintType)
struct FZoneLighting
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Brightness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FColor LightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SunDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDynamicLighting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* SkyMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class ULightComponent*> LightComponents;
};

USTRUCT(BlueprintType)
struct FZoneAudio
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundBase* BackgroundMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundBase* AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDuckingEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DuckingVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, USoundBase*> ZoneSpecificSounds;
};

USTRUCT(BlueprintType)
struct FZoneAdjacencyList
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> AdjacentZones;
};

USTRUCT(BlueprintType)
struct FZonePlayerList
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<APlayerState*> Players;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UHubZoneManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UHubZoneManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Zone Management
    UFUNCTION(BlueprintCallable, Category = "Zone Management")
    void InitializeZoneManager();

    UFUNCTION(BlueprintCallable, Category = "Zone Management")
    void RegisterZoneVolume(ATriggerVolume* ZoneVolume, const FHubZone& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Zone Management")
    void UnregisterZoneVolume(const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Zone Management")
    void UpdateZoneProperties(const FName& ZoneName, const FHubZone& NewZoneData);

    FHubZone* GetZoneData(const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Zone Management")
    TArray<FName> GetAllZoneNames() const;

    // Zone Transitions
    UFUNCTION(BlueprintCallable, Category = "Zone Transitions")
    void CreateZoneTransition(const FZoneTransition& Transition);

    UFUNCTION(BlueprintCallable, Category = "Zone Transitions")
    void RemoveZoneTransition(const FName& FromZone, const FName& ToZone);

    UFUNCTION(BlueprintCallable, Category = "Zone Transitions")
    bool CanTransitionBetweenZones(const FName& FromZone, const FName& ToZone, APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Zone Transitions")
    void ExecuteZoneTransition(APlayerState* Player, const FName& FromZone, const FName& ToZone);

    UFUNCTION(BlueprintCallable, Category = "Zone Transitions")
    TArray<FZoneTransition> GetAvailableTransitions(const FName& FromZone);

    // Zone Environment
    UFUNCTION(BlueprintCallable, Category = "Zone Environment")
    void SetZoneEnvironment(const FName& ZoneName, const FZoneEnvironment& Environment);

    UFUNCTION(BlueprintCallable, Category = "Zone Environment")
    void ApplyZoneEnvironment(APlayerState* Player, const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Zone Environment")
    void RemoveZoneEnvironment(APlayerState* Player, const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Zone Environment")
    FZoneEnvironment GetZoneEnvironment(const FName& ZoneName);

    // Zone Lighting
    UFUNCTION(BlueprintCallable, Category = "Zone Lighting")
    void SetZoneLighting(const FName& ZoneName, const FZoneLighting& Lighting);

    UFUNCTION(BlueprintCallable, Category = "Zone Lighting")
    void ApplyZoneLighting(APlayerState* Player, const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Zone Lighting")
    void UpdateZoneLighting(const FName& ZoneName, float DeltaTime);

    // Zone Audio
    UFUNCTION(BlueprintCallable, Category = "Zone Audio")
    void SetZoneAudio(const FName& ZoneName, const FZoneAudio& Audio);

    UFUNCTION(BlueprintCallable, Category = "Zone Audio")
    void PlayZoneAudio(APlayerState* Player, const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Zone Audio")
    void StopZoneAudio(APlayerState* Player, const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Zone Audio")
    void UpdateZoneAudio(const FName& ZoneName, float DeltaTime);

    // Zone Events
    UFUNCTION(BlueprintCallable, Category = "Zone Events")
    void OnPlayerEnterZone(APlayerState* Player, const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Zone Events")
    void OnPlayerLeaveZone(APlayerState* Player, const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Zone Events")
    void OnZoneStateChanged(const FName& ZoneName, EHubZoneType NewState);

    // Zone Utilities
    UFUNCTION(BlueprintCallable, Category = "Zone Utilities")
    bool IsPlayerInZone(APlayerState* Player, const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Zone Utilities")
    FName GetPlayerCurrentZone(APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Zone Utilities")
    TArray<APlayerState*> GetPlayersInZone(const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Zone Utilities")
    float GetDistanceToZone(APlayerState* Player, const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Zone Utilities")
    bool IsZoneAccessible(const FName& ZoneName, APlayerState* Player);

    // Zone Debugging
    UFUNCTION(BlueprintCallable, Category = "Zone Debugging")
    void DrawZoneDebugInfo();

    UFUNCTION(BlueprintCallable, Category = "Zone Debugging")
    void LogZoneStatistics();

    UFUNCTION(BlueprintCallable, Category = "Zone Debugging")
    bool ValidateZoneConfiguration();

protected:
    // Zone Data Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone Data")
    TMap<FName, FHubZone> ZoneDataMap;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone Data")
    TMap<FName, ATriggerVolume*> ZoneVolumes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone Data")
    TMap<FName, FZoneEnvironment> ZoneEnvironments;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone Data")
    TMap<FName, FZoneLighting> ZoneLighting;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone Data")
    TMap<FName, FZoneAudio> ZoneAudio;

    // Zone Transitions
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone Transitions")
    TArray<FZoneTransition> ZoneTransitions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone Transitions")
    TMap<FName, FZoneAdjacencyList> ZoneAdjacency;

    // Player Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Tracking")
    TMap<APlayerState*, FName> PlayerCurrentZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Tracking")
    TMap<FName, FZonePlayerList> ZonePlayers;

    // Active Transitions
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Transitions")
    TMap<APlayerState*, FTimerHandle> ActiveTransitions;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableDebugDrawing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableZoneAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableZoneLighting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableEnvironmentEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ZoneUpdateInterval;

private:
    // Internal Functions
    void SetupZoneTriggers();
    void UpdateZoneStates(float DeltaTime);
    void ProcessZoneTransitions(float DeltaTime);
    void HandleZoneOverlaps();

    // Zone Trigger Management
    void OnZoneVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    void OnZoneVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Transition Management
    void StartTransition(APlayerState* Player, const FZoneTransition& Transition);
    void CompleteTransition(APlayerState* Player, const FZoneTransition& Transition);
    void CancelTransition(APlayerState* Player);

    // Environment Application
    void ApplyGravityChanges(APlayerState* Player, const FZoneEnvironment& Environment);
    void ApplyAtmosphericEffects(APlayerState* Player, const FZoneEnvironment& Environment);
    void ApplyRadiationEffects(APlayerState* Player, const FZoneEnvironment& Environment);

    // Audio Management
    void FadeZoneAudio(APlayerState* Player, const FName& ZoneName, float TargetVolume, float Duration);

    // Lighting Management
    void InterpolateLighting(APlayerState* Player, const FZoneLighting& TargetLighting, float Duration);

    // Validation
    bool ValidateZoneTransition(const FZoneTransition& Transition);
    bool ValidateZoneEnvironment(const FZoneEnvironment& Environment);

    // Debug
    void DrawZoneBounds(const FName& ZoneName, const FColor& Color);
    void DrawTransitionPaths();

    // Timers
    UPROPERTY()
    FTimerHandle ZoneUpdateTimer;

    UPROPERTY()
    FTimerHandle TransitionTimer;

    // Constants
    static constexpr float DEFAULT_ZONE_UPDATE_INTERVAL = 0.1f;
    static constexpr float DEFAULT_TRANSITION_DURATION = 2.0f;
    static constexpr float AUDIO_FADE_DURATION = 1.0f;
    static constexpr float LIGHTING_INTERPOLATION_SPEED = 2.0f;

    // Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerEnteredZone, APlayerState*, Player, const FName&, ZoneName, const FHubZone&, ZoneData);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerLeftZone, APlayerState*, Player, const FName&, ZoneName, const FHubZone&, ZoneData);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneTransitionStarted, APlayerState*, Player, const FZoneTransition&, Transition);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneTransitionCompleted, APlayerState*, Player, const FZoneTransition&, Transition);

public:
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Zone Events")
    FOnPlayerEnteredZone OnPlayerEnteredZone;

    UPROPERTY(BlueprintAssignable, Category = "Zone Events")
    FOnPlayerLeftZone OnPlayerLeftZone;

    UPROPERTY(BlueprintAssignable, Category = "Zone Events")
    FOnZoneTransitionStarted OnZoneTransitionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Zone Events")
    FOnZoneTransitionCompleted OnZoneTransitionCompleted;
};