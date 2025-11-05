#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundConcurrency.h"
#include "AudioDevice.h"
#include "AudioDeviceManager.h"
#include "AudioSystemManager.generated.h"

class USoundBase;
class USoundCue;
class USoundClass;
class USoundMix;
class USoundConcurrency;
class UAudioComponent;
class AActor;
class UAudioBus;

// Audio Effect Data
USTRUCT(BlueprintType)
struct FAudioEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EffectIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EffectDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLooping;

    FAudioEffect()
    {
        EffectName = TEXT("");
        EffectIntensity = 1.0f;
        EffectDuration = 1.0f;
        bIsLooping = false;
    }
};

// Audio Mix Types
UENUM(BlueprintType)
enum class EAudioMixType : uint8
{
    None,
    Master,
    Music,
    SFX,
    Ambient,
    Dialogue,
    UI,
    VoiceChat,
    Spatial,
    Dynamic
};

// Audio Priority Levels
UENUM(BlueprintType)
enum class EAudioSystemPriority : uint8
{
    Lowest,
    Low,
    Normal,
    High,
    Highest,
    Critical
};

// Audio Environment Types
UENUM(BlueprintType)
enum class EAudioEnvironment : uint8
{
    None,
    Space,
    StationInterior,
    PlanetSurface,
    Cave,
    Underwater,
    Vehicle,
    Building,
    OpenSpace,
    DenseAtmosphere
};

// Audio Effect Types
UENUM(BlueprintType)
enum class EAudioEffect : uint8
{
    None,
    Reverb,
    Echo,
    Distortion,
    LowPass,
    HighPass,
    PitchShift,
    TimeStretch,
    Doppler,
    Occlusion,
    Spatialization
};

// Audio Zone Definition
USTRUCT(BlueprintType)
struct FAudioZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ZoneRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudioEnvironment EnvironmentType = EAudioEnvironment::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundMix* EnvironmentMix = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReverbLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bApplyOcclusion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<USoundCue*> AmbientSounds;
};

// Audio Event Definition
USTRUCT(BlueprintType)
struct FAudioEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* SoundCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudioSystemPriority Priority = EAudioSystemPriority::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PitchMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLoop = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSpatial = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundConcurrency* Concurrency = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FAudioEffect> Effects;
};

// Audio Instance Data
USTRUCT(BlueprintType)
struct FAudioSystemInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InstanceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UAudioComponent* AudioComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* SourceActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudioSystemPriority Priority = EAudioSystemPriority::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPaused = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> ParameterValues;
};

// Audio Settings (Simple)
USTRUCT(BlueprintType)
struct FAudioSystemSettingsSimple
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MusicVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnable3DAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableHRTF = true;
};

// Alias for backward compatibility
typedef FAudioSystemSettingsSimple FAudioSettings;

// Audio Settings
USTRUCT(BlueprintType)
struct FAudioSystemSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MusicVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DialogueVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AmbientVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VoiceChatVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float UIVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnable3DAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableHRTF = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableDoppler = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableOcclusion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxConcurrentSounds = 32;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AudioQuality = 1.0f;
};

// VOIP Data
USTRUCT(BlueprintType)
struct FVOIPData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsSpeaking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VolumeLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VoiceAmplitude = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMuted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLocalPlayer = false;
};

// Dynamic Music Data
USTRUCT(BlueprintType)
struct FDynamicMusicData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MusicState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* BaseTrack = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* CombatTrack = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* ExplorationTrack = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* SocialTrack = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoTransition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> StateIntensity;
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAudioEventTriggered, const FString&, EventName, AActor*, SourceActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAudioZoneEntered, const FString&, ZoneName, AActor*, Actor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAudioZoneExited, const FString&, ZoneName, AActor*, Actor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMusicStateChanged, const FString&, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVOIPPlayerSpeaking, const FString&, PlayerName, bool, bIsSpeaking);

/**
 * Audio System Manager
 * Provides comprehensive audio management including 3D spatial audio, dynamic music, 
 * VOIP integration, and environmental audio effects.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Audio))
class ALEXANDER_API UAudioSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // System Management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void ShutdownAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioSystem(float DeltaTime);

    // Audio Event System
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    FName PlayAudioEvent(const FString& EventName, AActor* SourceActor = nullptr, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAudioEvent(const FName& InstanceName, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PauseAudioEvent(const FName& InstanceName);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void ResumeAudioEvent(const FName& InstanceName);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAudioEventParameter(const FName& InstanceName, const FName& ParameterName, float Value);

    // Audio Zones
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void CreateAudioZone(const FAudioZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RemoveAudioZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioZone(const FString& ZoneName, const FAudioZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    FAudioZone GetAudioZone(const FString& ZoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    TArray<FAudioZone> GetAllAudioZones() const;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    FString GetCurrentAudioZone(AActor* Actor) const;

    // Dynamic Music System
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetDynamicMusicData(const FDynamicMusicData& MusicData);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMusicState(const FString& State);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TransitionToMusicState(const FString& State, float TransitionTime);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    FString GetCurrentMusicState() const;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMusicIntensity(const FString& State, float Intensity);

    // VOIP Integration
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeVOIP();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void ShutdownVOIP();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetVOIPMuted(bool bMuted);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    bool IsVOIPMuted() const;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetVOIPVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    float GetVOIPVolume() const;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    TArray<FVOIPData> GetVOIPPlayers() const;

    // Audio Settings
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAudioSettings(const FAudioSystemSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    FAudioSystemSettings GetAudioSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetSFXVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetDialogueVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetVoiceChatVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetUIVolume(float Volume);

    // Audio Effects
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void ApplyAudioEffect(const FName& InstanceName, EAudioEffect EffectType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RemoveAudioEffect(const FName& InstanceName, EAudioEffect EffectType);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void ClearAllAudioEffects(const FName& InstanceName);

    // 3D Audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void Enable3DAudio(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    bool Is3DAudioEnabled() const;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAudioListenerPosition(FVector Position, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioAttenuation(const FName& InstanceName, float MinDistance, float MaxDistance);

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Audio System")
    bool IsAudioEventPlaying(const FName& InstanceName) const;

    UFUNCTION(BlueprintPure, Category = "Audio System")
    float GetAudioEventDuration(const FName& InstanceName) const;

    UFUNCTION(BlueprintPure, Category = "Audio System")
    TArray<FName> GetActiveAudioEvents() const;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAllAudioEvents(bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PauseAllAudioEvents();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void ResumeAllAudioEvents();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Audio System")
    FOnAudioEventTriggered OnAudioEventTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Audio System")
    FOnAudioZoneEntered OnAudioZoneEntered;

    UPROPERTY(BlueprintAssignable, Category = "Audio System")
    FOnAudioZoneExited OnAudioZoneExited;

    UPROPERTY(BlueprintAssignable, Category = "Audio System")
    FOnMusicStateChanged OnMusicStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Audio System")
    FOnVOIPPlayerSpeaking OnVOIPPlayerSpeaking;

protected:
    // Audio Data Storage
    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    TMap<FString, FAudioEvent> AudioEvents;

    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    TMap<FName, FAudioSystemInstance> AudioInstances;

    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    TMap<FString, FAudioZone> AudioZones;

    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    TMap<AActor*, FString> ActorZoneMap;

    // Settings and Configuration
    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    FAudioSystemSettings CurrentAudioSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    FDynamicMusicData DynamicMusicData;

    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    FString CurrentMusicState;

    // VOIP System
    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    TArray<FVOIPData> VOIPPlayers;

    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    bool bVOIPInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    bool bVOIPMuted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    float VOIPVolume = 1.0f;

    // Audio Components
    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    TArray<UAudioComponent*> AudioComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    UAudioComponent* MusicComponent = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    UAudioComponent* AmbientComponent = nullptr;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    int32 MaxAudioInstances = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float AudioUpdateInterval = 0.016f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    bool bEnableDebugAudio = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    bool bEnableAudioOcclusion = true;

private:
    float LastUpdateTime;
    int32 AudioInstanceCounter;
    TMap<FName, USoundConcurrency*> ConcurrencyGroups;

    void UpdateAudioInstances(float DeltaTime);
    void UpdateAudioZones(float DeltaTime);
    void UpdateDynamicMusic(float DeltaTime);
    void UpdateVOIP(float DeltaTime);

    void CleanupExpiredInstances();
    void OptimizeAudioPerformance();
    void ApplyAudioSettings();

    FName GenerateUniqueInstanceName(const FString& BaseName);
    UAudioComponent* CreateAudioComponent(USoundCue* SoundCue, AActor* SourceActor, FVector Location);
    void ApplyAudioEffects(UAudioComponent* AudioComponent, const TArray<FAudioEffect>& Effects);

    void ProcessAudioZoneTransitions(AActor* Actor);
    void ApplyEnvironmentMix(EAudioEnvironment Environment, float Intensity);

    void HandleMusicStateTransition(const FString& NewState, float TransitionTime);
    void UpdateMusicParameters();

    void ProcessVOIPData();
    void UpdateVOIPPlayerSpeaking(const FString& PlayerName, bool bIsSpeaking);

    void DrawDebugAudioInfo();
    void LogAudioPerformanceMetrics();
};