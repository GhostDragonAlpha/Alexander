// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "AudioManager.generated.h"

/**
 * Audio category types
 */
UENUM(BlueprintType)
enum class EAudioCategory : uint8
{
    Music           UMETA(DisplayName = "Music"),
    SFX             UMETA(DisplayName = "SFX"),
    Ambient         UMETA(DisplayName = "Ambient"),
    Voice           UMETA(DisplayName = "Voice"),
    UI              UMETA(DisplayName = "UI"),
    Engine          UMETA(DisplayName = "Engine"),
    Weapon          UMETA(DisplayName = "Weapon"),
    Explosion       UMETA(DisplayName = "Explosion"),
    Impact          UMETA(DisplayName = "Impact"),
    Weather         UMETA(DisplayName = "Weather")
};

/**
 * Audio priority levels
 */
UENUM(BlueprintType)
enum class EAudioPriority : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Normal      UMETA(DisplayName = "Normal"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

/**
 * Audio instance data
 */
USTRUCT(BlueprintType)
struct FAudioInstance
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Audio")
    FString InstanceID;

    UPROPERTY(BlueprintReadWrite, Category = "Audio")
    UAudioComponent* AudioComponent = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Audio")
    EAudioCategory Category = EAudioCategory::SFX;

    UPROPERTY(BlueprintReadWrite, Category = "Audio")
    EAudioPriority Priority = EAudioPriority::Normal;

    UPROPERTY(BlueprintReadWrite, Category = "Audio")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Audio")
    float SpawnTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Audio")
    bool bIsLooping = false;

    UPROPERTY(BlueprintReadWrite, Category = "Audio")
    bool bIs3D = true;
};

/**
 * Audio mix state
 */
USTRUCT(BlueprintType)
struct FAudioMixState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Mix")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Mix")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Mix")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Mix")
    float VoiceVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Mix")
    float AmbienceVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Mix")
    float UIVolume = 0.8f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAudioPlayed, const FString&, InstanceID, EAudioCategory, Category);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAudioStopped, const FString&, InstanceID, EAudioCategory, Category);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMusicTrackChanged, USoundBase*, NewTrack);

/**
 * Audio Manager Component
 * Centralized audio system with spatial audio, music management, and dynamic mixing
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UAudioManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ===== Audio Playback =====

    UFUNCTION(BlueprintCallable, Category = "Audio")
    FString PlaySound(USoundBase* Sound, FVector Location, float VolumeMultiplier = 1.0f, 
                     float PitchMultiplier = 1.0f, EAudioCategory Category = EAudioCategory::SFX);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    FString PlaySound2D(USoundBase* Sound, float VolumeMultiplier = 1.0f, 
                       float PitchMultiplier = 1.0f, EAudioCategory Category = EAudioCategory::SFX);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    FString PlaySoundAttached(USoundBase* Sound, USceneComponent* AttachTo, FName SocketName,
                             float VolumeMultiplier = 1.0f, EAudioCategory Category = EAudioCategory::SFX);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    FString PlaySoundLooping(USoundBase* Sound, FVector Location, float VolumeMultiplier = 1.0f,
                            EAudioCategory Category = EAudioCategory::Ambient);

    // ===== Audio Control =====

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSound(const FString& InstanceID, float FadeOutDuration = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PauseSound(const FString& InstanceID);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void ResumeSound(const FString& InstanceID);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetSoundVolume(const FString& InstanceID, float Volume, float FadeDuration = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetSoundPitch(const FString& InstanceID, float Pitch);

    // ===== Music System =====

    UFUNCTION(BlueprintCallable, Category = "Music")
    void PlayMusic(USoundBase* MusicTrack, float FadeInDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Music")
    void StopMusic(float FadeOutDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Music")
    void CrossfadeMusic(USoundBase* NewTrack, float CrossfadeDuration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Music")
    void SetMusicVolume(float Volume, float FadeDuration = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Music")
    USoundBase* GetCurrentMusicTrack() const { return CurrentMusicTrack; }

    // ===== Volume Mixing =====

    UFUNCTION(BlueprintCallable, Category = "Audio Mix")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Mix")
    void SetCategoryVolume(EAudioCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Mix")
    float GetCategoryVolume(EAudioCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Audio Mix")
    void ApplyAudioMix(const FAudioMixState& MixState);

    UFUNCTION(BlueprintCallable, Category = "Audio Mix")
    FAudioMixState GetCurrentMix() const { return CurrentMix; }

    // ===== Spatial Audio =====

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void SetListenerLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void Set3DAudioSettings(float MaxDistance, float MinDistance);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void EnableOcclusion(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void EnableReverb(bool bEnable);

    // ===== Category Management =====

    UFUNCTION(BlueprintCallable, Category = "Audio Category")
    void StopAllInCategory(EAudioCategory Category, float FadeOutDuration = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Category")
    void PauseAllInCategory(EAudioCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio Category")
    void ResumeAllInCategory(EAudioCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio Category")
    int32 GetActiveSoundsInCategory(EAudioCategory Category) const;

    // ===== Audio Queries =====

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsSoundPlaying(const FString& InstanceID) const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    int32 GetActiveSoundCount() const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    TArray<FString> GetActiveSoundInstances(EAudioCategory Category) const;

    // ===== Advanced Features =====

    UFUNCTION(BlueprintCallable, Category = "Audio Advanced")
    void PlayRandomSoundFromArray(const TArray<USoundBase*>& Sounds, FVector Location, 
                                  EAudioCategory Category = EAudioCategory::SFX);

    UFUNCTION(BlueprintCallable, Category = "Audio Advanced")
    void PlaySequentialSounds(const TArray<USoundBase*>& Sounds, FVector Location, float Delay = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Audio Advanced")
    void DuckAudioCategory(EAudioCategory Category, float DuckAmount, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Audio Advanced")
    void SetLowPassFilter(const FString& InstanceID, float Frequency);

    UFUNCTION(BlueprintCallable, Category = "Audio Advanced")
    void SetHighPassFilter(const FString& InstanceID, float Frequency);

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "Audio Events")
    FOnAudioPlayed OnAudioPlayed;

    UPROPERTY(BlueprintAssignable, Category = "Audio Events")
    FOnAudioStopped OnAudioStopped;

    UPROPERTY(BlueprintAssignable, Category = "Audio Events")
    FOnMusicTrackChanged OnMusicTrackChanged;

    // ===== Configuration =====

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    int32 MaxConcurrentSounds = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float MaxAudioDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    bool bEnableOcclusion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    bool bEnableReverb = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TMap<EAudioCategory, int32> CategoryMaxCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    FAudioMixState DefaultMix;

private:
    // Active audio tracking
    UPROPERTY()
    TMap<FString, FAudioInstance> ActiveAudio;

    // Music system
    UPROPERTY()
    UAudioComponent* MusicAudioComponent;

    UPROPERTY()
    USoundBase* CurrentMusicTrack;

    // Mix state
    FAudioMixState CurrentMix;

    // Statistics
    int32 TotalSoundsPlayed;

    // Helper functions
    FString GenerateInstanceID();
    void CleanupFinishedAudio();
    void UpdateSpatialAudio(float DeltaTime);
    void ApplyCategoryVolume(UAudioComponent* Component, EAudioCategory Category);
    float GetFinalVolume(EAudioCategory Category, float BaseVolume) const;
    void EnforceCategoryLimits();
};
