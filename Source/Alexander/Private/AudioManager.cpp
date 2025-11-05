// Copyright Epic Games, Inc. All Rights Reserved.

#include "AudioManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"

UAudioManager::UAudioManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz for audio management

    // Default configuration
    MaxConcurrentSounds = 50;
    MaxAudioDistance = 10000.0f;
    bEnableOcclusion = true;
    bEnableReverb = true;
    TotalSoundsPlayed = 0;

    // Set default category limits
    CategoryMaxCounts.Add(EAudioCategory::Weapon, 10);
    CategoryMaxCounts.Add(EAudioCategory::Explosion, 5);
    CategoryMaxCounts.Add(EAudioCategory::Impact, 15);
    CategoryMaxCounts.Add(EAudioCategory::Ambient, 5);
    CategoryMaxCounts.Add(EAudioCategory::Voice, 3);

    // Initialize default mix
    CurrentMix = DefaultMix;
}

void UAudioManager::BeginPlay()
{
    Super::BeginPlay();

    // Create music audio component
    if (AActor* Owner = GetOwner())
    {
        MusicAudioComponent = NewObject<UAudioComponent>(Owner);
        if (MusicAudioComponent)
        {
            MusicAudioComponent->RegisterComponent();
            MusicAudioComponent->bAutoActivate = false;
        }
    }
}

void UAudioManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Cleanup finished audio
    CleanupFinishedAudio();

    // Update spatial audio
    UpdateSpatialAudio(DeltaTime);

    // Enforce category limits
    EnforceCategoryLimits();
}

// ===== Audio Playback =====

FString UAudioManager::PlaySound(USoundBase* Sound, FVector Location, float VolumeMultiplier, float PitchMultiplier, EAudioCategory Category)
{
    if (!Sound || !GetWorld())
    {
        return FString();
    }

    if (ActiveAudio.Num() >= MaxConcurrentSounds)
    {
        return FString();
    }

    FString InstanceID = GenerateInstanceID();

    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        Sound,
        Location,
        FRotator::ZeroRotator,
        VolumeMultiplier,
        PitchMultiplier,
        0.0f,
        nullptr,
        nullptr,
        true
    );

    if (AudioComponent)
    {
        ApplyCategoryVolume(AudioComponent, Category);

        FAudioInstance Instance;
        Instance.InstanceID = InstanceID;
        Instance.AudioComponent = AudioComponent;
        Instance.Category = Category;
        Instance.Location = Location;
        Instance.SpawnTime = GetWorld()->GetTimeSeconds();
        Instance.bIs3D = true;
        Instance.bIsLooping = false;

        ActiveAudio.Add(InstanceID, Instance);
        TotalSoundsPlayed++;

        OnAudioPlayed.Broadcast(InstanceID, Category);

        return InstanceID;
    }

    return FString();
}

FString UAudioManager::PlaySound2D(USoundBase* Sound, float VolumeMultiplier, float PitchMultiplier, EAudioCategory Category)
{
    if (!Sound || !GetWorld())
    {
        return FString();
    }

    FString InstanceID = GenerateInstanceID();

    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(
        GetWorld(),
        Sound,
        VolumeMultiplier,
        PitchMultiplier,
        0.0f,
        nullptr,
        true
    );

    if (AudioComponent)
    {
        ApplyCategoryVolume(AudioComponent, Category);

        FAudioInstance Instance;
        Instance.InstanceID = InstanceID;
        Instance.AudioComponent = AudioComponent;
        Instance.Category = Category;
        Instance.SpawnTime = GetWorld()->GetTimeSeconds();
        Instance.bIs3D = false;

        ActiveAudio.Add(InstanceID, Instance);
        TotalSoundsPlayed++;

        OnAudioPlayed.Broadcast(InstanceID, Category);

        return InstanceID;
    }

    return FString();
}

FString UAudioManager::PlaySoundAttached(USoundBase* Sound, USceneComponent* AttachTo, FName SocketName, float VolumeMultiplier, EAudioCategory Category)
{
    if (!Sound || !AttachTo || !GetWorld())
    {
        return FString();
    }

    FString InstanceID = GenerateInstanceID();

    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAttached(
        Sound,
        AttachTo,
        SocketName,
        FVector::ZeroVector,
        EAttachLocation::KeepRelativeOffset,
        true,
        VolumeMultiplier,
        1.0f,
        0.0f,
        nullptr,
        nullptr,
        true
    );

    if (AudioComponent)
    {
        ApplyCategoryVolume(AudioComponent, Category);

        FAudioInstance Instance;
        Instance.InstanceID = InstanceID;
        Instance.AudioComponent = AudioComponent;
        Instance.Category = Category;
        Instance.Location = AudioComponent->GetComponentLocation();
        Instance.SpawnTime = GetWorld()->GetTimeSeconds();
        Instance.bIs3D = true;

        ActiveAudio.Add(InstanceID, Instance);
        TotalSoundsPlayed++;

        OnAudioPlayed.Broadcast(InstanceID, Category);

        return InstanceID;
    }

    return FString();
}

FString UAudioManager::PlaySoundLooping(USoundBase* Sound, FVector Location, float VolumeMultiplier, EAudioCategory Category)
{
    if (!Sound || !GetWorld())
    {
        return FString();
    }

    FString InstanceID = GenerateInstanceID();

    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        Sound,
        Location,
        FRotator::ZeroRotator,
        VolumeMultiplier,
        1.0f,
        0.0f,
        nullptr,
        nullptr,
        true
    );

    if (AudioComponent)
    {
        AudioComponent->SetUISound(false);
        ApplyCategoryVolume(AudioComponent, Category);

        FAudioInstance Instance;
        Instance.InstanceID = InstanceID;
        Instance.AudioComponent = AudioComponent;
        Instance.Category = Category;
        Instance.Location = Location;
        Instance.SpawnTime = GetWorld()->GetTimeSeconds();
        Instance.bIsLooping = true;
        Instance.bIs3D = true;

        ActiveAudio.Add(InstanceID, Instance);
        TotalSoundsPlayed++;

        OnAudioPlayed.Broadcast(InstanceID, Category);

        return InstanceID;
    }

    return FString();
}

// ===== Audio Control =====

void UAudioManager::StopSound(const FString& InstanceID, float FadeOutDuration)
{
    if (ActiveAudio.Contains(InstanceID))
    {
        FAudioInstance& Instance = ActiveAudio[InstanceID];

        if (Instance.AudioComponent)
        {
            if (FadeOutDuration > 0.0f)
            {
                Instance.AudioComponent->FadeOut(FadeOutDuration, 0.0f);
            }
            else
            {
                Instance.AudioComponent->Stop();
            }
        }

        OnAudioStopped.Broadcast(InstanceID, Instance.Category);
        ActiveAudio.Remove(InstanceID);
    }
}

void UAudioManager::PauseSound(const FString& InstanceID)
{
    if (ActiveAudio.Contains(InstanceID))
    {
        FAudioInstance& Instance = ActiveAudio[InstanceID];
        if (Instance.AudioComponent)
        {
            Instance.AudioComponent->SetPaused(true);
        }
    }
}

void UAudioManager::ResumeSound(const FString& InstanceID)
{
    if (ActiveAudio.Contains(InstanceID))
    {
        FAudioInstance& Instance = ActiveAudio[InstanceID];
        if (Instance.AudioComponent)
        {
            Instance.AudioComponent->SetPaused(false);
        }
    }
}

void UAudioManager::SetSoundVolume(const FString& InstanceID, float Volume, float FadeDuration)
{
    if (ActiveAudio.Contains(InstanceID))
    {
        FAudioInstance& Instance = ActiveAudio[InstanceID];
        if (Instance.AudioComponent)
        {
            float FinalVolume = GetFinalVolume(Instance.Category, Volume);

            if (FadeDuration > 0.0f)
            {
                Instance.AudioComponent->FadeIn(FadeDuration, FinalVolume);
            }
            else
            {
                Instance.AudioComponent->SetVolumeMultiplier(FinalVolume);
            }
        }
    }
}

void UAudioManager::SetSoundPitch(const FString& InstanceID, float Pitch)
{
    if (ActiveAudio.Contains(InstanceID))
    {
        FAudioInstance& Instance = ActiveAudio[InstanceID];
        if (Instance.AudioComponent)
        {
            Instance.AudioComponent->SetPitchMultiplier(Pitch);
        }
    }
}

// ===== Music System =====

void UAudioManager::PlayMusic(USoundBase* MusicTrack, float FadeInDuration)
{
    if (!MusicTrack || !MusicAudioComponent)
    {
        return;
    }

    if (CurrentMusicTrack)
    {
        StopMusic(FadeInDuration);
    }

    MusicAudioComponent->SetSound(MusicTrack);
    MusicAudioComponent->SetVolumeMultiplier(CurrentMix.MusicVolume * CurrentMix.MasterVolume);
    MusicAudioComponent->FadeIn(FadeInDuration, CurrentMix.MusicVolume * CurrentMix.MasterVolume);
    MusicAudioComponent->Play();

    CurrentMusicTrack = MusicTrack;
    OnMusicTrackChanged.Broadcast(MusicTrack);
}

void UAudioManager::StopMusic(float FadeOutDuration)
{
    if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
    {
        if (FadeOutDuration > 0.0f)
        {
            MusicAudioComponent->FadeOut(FadeOutDuration, 0.0f);
        }
        else
        {
            MusicAudioComponent->Stop();
        }

        CurrentMusicTrack = nullptr;
    }
}

void UAudioManager::CrossfadeMusic(USoundBase* NewTrack, float CrossfadeDuration)
{
    if (!NewTrack)
    {
        return;
    }

    StopMusic(CrossfadeDuration);

    // Delay new track start slightly for crossfade effect
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, NewTrack, CrossfadeDuration]()
    {
        PlayMusic(NewTrack, CrossfadeDuration);
    }, CrossfadeDuration * 0.5f, false);
}

void UAudioManager::SetMusicVolume(float Volume, float FadeDuration)
{
    CurrentMix.MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

    if (MusicAudioComponent)
    {
        float FinalVolume = CurrentMix.MusicVolume * CurrentMix.MasterVolume;

        if (FadeDuration > 0.0f)
        {
            MusicAudioComponent->FadeIn(FadeDuration, FinalVolume);
        }
        else
        {
            MusicAudioComponent->SetVolumeMultiplier(FinalVolume);
        }
    }
}

// ===== Volume Mixing =====

void UAudioManager::SetMasterVolume(float Volume)
{
    CurrentMix.MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

    // Update all active audio
    for (auto& Pair : ActiveAudio)
    {
        if (Pair.Value.AudioComponent)
        {
            ApplyCategoryVolume(Pair.Value.AudioComponent, Pair.Value.Category);
        }
    }

    // Update music
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(CurrentMix.MusicVolume * CurrentMix.MasterVolume);
    }
}

void UAudioManager::SetCategoryVolume(EAudioCategory Category, float Volume)
{
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);

    switch (Category)
    {
    case EAudioCategory::Music: CurrentMix.MusicVolume = Volume; break;
    case EAudioCategory::SFX: CurrentMix.SFXVolume = Volume; break;
    case EAudioCategory::Voice: CurrentMix.VoiceVolume = Volume; break;
    case EAudioCategory::Ambient: CurrentMix.AmbienceVolume = Volume; break;
    case EAudioCategory::UI: CurrentMix.UIVolume = Volume; break;
    }

    // Update all audio in this category
    for (auto& Pair : ActiveAudio)
    {
        if (Pair.Value.Category == Category && Pair.Value.AudioComponent)
        {
            ApplyCategoryVolume(Pair.Value.AudioComponent, Category);
        }
    }
}

float UAudioManager::GetCategoryVolume(EAudioCategory Category) const
{
    switch (Category)
    {
    case EAudioCategory::Music: return CurrentMix.MusicVolume;
    case EAudioCategory::SFX: return CurrentMix.SFXVolume;
    case EAudioCategory::Voice: return CurrentMix.VoiceVolume;
    case EAudioCategory::Ambient: return CurrentMix.AmbienceVolume;
    case EAudioCategory::UI: return CurrentMix.UIVolume;
    default: return 1.0f;
    }
}

void UAudioManager::ApplyAudioMix(const FAudioMixState& MixState)
{
    CurrentMix = MixState;
    SetMasterVolume(CurrentMix.MasterVolume);
}

// ===== Spatial Audio =====

void UAudioManager::SetListenerLocation(FVector Location)
{
    // Listener location is typically handled by the player controller
    // This function can be used for custom listener positioning
}

void UAudioManager::Set3DAudioSettings(float MaxDistance, float MinDistance)
{
    MaxAudioDistance = MaxDistance;

    // Apply to all active 3D audio
    for (auto& Pair : ActiveAudio)
    {
        if (Pair.Value.bIs3D && Pair.Value.AudioComponent)
        {
            // Audio component 3D settings would be applied here
        }
    }
}

void UAudioManager::EnableOcclusion(bool bEnable)
{
    bEnableOcclusion = bEnable;
}

void UAudioManager::EnableReverb(bool bEnable)
{
    bEnableReverb = bEnable;
}

// ===== Category Management =====

void UAudioManager::StopAllInCategory(EAudioCategory Category, float FadeOutDuration)
{
    TArray<FString> InstancesToStop;

    for (const auto& Pair : ActiveAudio)
    {
        if (Pair.Value.Category == Category)
        {
            InstancesToStop.Add(Pair.Key);
        }
    }

    for (const FString& InstanceID : InstancesToStop)
    {
        StopSound(InstanceID, FadeOutDuration);
    }
}

void UAudioManager::PauseAllInCategory(EAudioCategory Category)
{
    for (auto& Pair : ActiveAudio)
    {
        if (Pair.Value.Category == Category && Pair.Value.AudioComponent)
        {
            Pair.Value.AudioComponent->SetPaused(true);
        }
    }
}

void UAudioManager::ResumeAllInCategory(EAudioCategory Category)
{
    for (auto& Pair : ActiveAudio)
    {
        if (Pair.Value.Category == Category && Pair.Value.AudioComponent)
        {
            Pair.Value.AudioComponent->SetPaused(false);
        }
    }
}

int32 UAudioManager::GetActiveSoundsInCategory(EAudioCategory Category) const
{
    int32 Count = 0;
    for (const auto& Pair : ActiveAudio)
    {
        if (Pair.Value.Category == Category)
        {
            Count++;
        }
    }
    return Count;
}

// ===== Audio Queries =====

bool UAudioManager::IsSoundPlaying(const FString& InstanceID) const
{
    if (ActiveAudio.Contains(InstanceID))
    {
        const FAudioInstance& Instance = ActiveAudio[InstanceID];
        return Instance.AudioComponent && Instance.AudioComponent->IsPlaying();
    }
    return false;
}

int32 UAudioManager::GetActiveSoundCount() const
{
    return ActiveAudio.Num();
}

TArray<FString> UAudioManager::GetActiveSoundInstances(EAudioCategory Category) const
{
    TArray<FString> Instances;

    for (const auto& Pair : ActiveAudio)
    {
        if (Pair.Value.Category == Category)
        {
            Instances.Add(Pair.Key);
        }
    }

    return Instances;
}

// ===== Advanced Features =====

void UAudioManager::PlayRandomSoundFromArray(const TArray<USoundBase*>& Sounds, FVector Location, EAudioCategory Category)
{
    if (Sounds.Num() == 0)
    {
        return;
    }

    int32 RandomIndex = FMath::RandRange(0, Sounds.Num() - 1);
    PlaySound(Sounds[RandomIndex], Location, 1.0f, 1.0f, Category);
}

void UAudioManager::PlaySequentialSounds(const TArray<USoundBase*>& Sounds, FVector Location, float Delay)
{
    for (int32 i = 0; i < Sounds.Num(); ++i)
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Sounds, i, Location]()
        {
            if (i < Sounds.Num())
            {
                PlaySound(Sounds[i], Location);
            }
        }, Delay * i, false);
    }
}

void UAudioManager::DuckAudioCategory(EAudioCategory Category, float DuckAmount, float Duration)
{
    float OriginalVolume = GetCategoryVolume(Category);
    float DuckedVolume = OriginalVolume * (1.0f - DuckAmount);

    SetCategoryVolume(Category, DuckedVolume);

    // Restore after duration
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Category, OriginalVolume]()
    {
        SetCategoryVolume(Category, OriginalVolume);
    }, Duration, false);
}

void UAudioManager::SetLowPassFilter(const FString& InstanceID, float Frequency)
{
    if (ActiveAudio.Contains(InstanceID))
    {
        FAudioInstance& Instance = ActiveAudio[InstanceID];
        if (Instance.AudioComponent)
        {
            Instance.AudioComponent->SetLowPassFilterFrequency(Frequency);
        }
    }
}

void UAudioManager::SetHighPassFilter(const FString& InstanceID, float Frequency)
{
    // High pass filter implementation
    if (ActiveAudio.Contains(InstanceID))
    {
        FAudioInstance& Instance = ActiveAudio[InstanceID];
        if (Instance.AudioComponent)
        {
            // UE5 audio component high pass filter
        }
    }
}

// ===== Helper Functions =====

FString UAudioManager::GenerateInstanceID()
{
    static int32 Counter = 0;
    return FString::Printf(TEXT("Audio_%d_%lld"), ++Counter, FDateTime::Now().GetTicks());
}

void UAudioManager::CleanupFinishedAudio()
{
    TArray<FString> FinishedInstances;

    for (const auto& Pair : ActiveAudio)
    {
        if (!Pair.Value.AudioComponent || !Pair.Value.AudioComponent->IsPlaying())
        {
            if (!Pair.Value.bIsLooping)
            {
                FinishedInstances.Add(Pair.Key);
            }
        }
    }

    for (const FString& InstanceID : FinishedInstances)
    {
        if (ActiveAudio.Contains(InstanceID))
        {
            OnAudioStopped.Broadcast(InstanceID, ActiveAudio[InstanceID].Category);
            ActiveAudio.Remove(InstanceID);
        }
    }
}

void UAudioManager::UpdateSpatialAudio(float DeltaTime)
{
    // Update 3D audio parameters based on listener position
    // Distance attenuation, occlusion checks, etc.
}

void UAudioManager::ApplyCategoryVolume(UAudioComponent* Component, EAudioCategory Category)
{
    if (!Component)
    {
        return;
    }

    float CategoryVolume = GetCategoryVolume(Category);
    float FinalVolume = CategoryVolume * CurrentMix.MasterVolume;
    Component->SetVolumeMultiplier(FinalVolume);
}

float UAudioManager::GetFinalVolume(EAudioCategory Category, float BaseVolume) const
{
    return BaseVolume * GetCategoryVolume(Category) * CurrentMix.MasterVolume;
}

void UAudioManager::EnforceCategoryLimits()
{
    for (const auto& LimitPair : CategoryMaxCounts)
    {
        EAudioCategory Category = LimitPair.Key;
        int32 MaxCount = LimitPair.Value;
        int32 CurrentCount = GetActiveSoundsInCategory(Category);

        if (CurrentCount > MaxCount)
        {
            // Find oldest instances to remove
            TArray<TPair<FString, float>> CategoryInstances;

            for (const auto& Pair : ActiveAudio)
            {
                if (Pair.Value.Category == Category)
                {
                    CategoryInstances.Add(TPair<FString, float>(Pair.Key, Pair.Value.SpawnTime));
                }
            }

            // Sort by spawn time (oldest first)
            CategoryInstances.Sort([](const TPair<FString, float>& A, const TPair<FString, float>& B)
            {
                return A.Value < B.Value;
            });

            // Remove excess instances
            int32 ToRemove = CurrentCount - MaxCount;
            for (int32 i = 0; i < ToRemove && i < CategoryInstances.Num(); ++i)
            {
                StopSound(CategoryInstances[i].Key, 0.0f);
            }
        }
    }
}
