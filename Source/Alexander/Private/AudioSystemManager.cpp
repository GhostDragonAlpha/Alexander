#include "AudioSystemManager.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundConcurrency.h"
#include "AudioDevice.h"
#include "AudioDeviceManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UAudioSystemManager::UAudioSystemManager()
{
    LastUpdateTime = 0.0f;
    AudioInstanceCounter = 0;
    MaxAudioInstances = 64;
    AudioUpdateInterval = 0.016f;
    bEnableDebugAudio = false;
    bEnableAudioOcclusion = true;
    bVOIPInitialized = false;
    bVOIPMuted = false;
    VOIPVolume = 1.0f;
}

void UAudioSystemManager::InitializeAudioSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Audio System Manager Initialized"));
    
    // Initialize default audio settings
    FAudioSystemSettings DefaultSettings;
    DefaultSettings.MasterVolume = 1.0f;
    DefaultSettings.MusicVolume = 0.8f;
    DefaultSettings.SFXVolume = 1.0f;
    DefaultSettings.DialogueVolume = 1.0f;
    DefaultSettings.AmbientVolume = 0.6f;
    DefaultSettings.VoiceChatVolume = 1.0f;
    DefaultSettings.UIVolume = 0.7f;
    DefaultSettings.bEnable3DAudio = true;
    DefaultSettings.bEnableHRTF = true;
    DefaultSettings.bEnableDoppler = true;
    DefaultSettings.bEnableOcclusion = true;
    DefaultSettings.MaxConcurrentSounds = 32;
    DefaultSettings.AudioQuality = 1.0f;

    SetAudioSettings(DefaultSettings);
    
    // Initialize VOIP system
    InitializeVOIP();
    
    // Create music component
    MusicComponent = CreateAudioComponent(nullptr, nullptr, FVector::ZeroVector);
    if (MusicComponent)
    {
        MusicComponent->bIsMusic = true;
        MusicComponent->bAllowSpatialization = false;
    }
    
    // Create ambient component
    AmbientComponent = CreateAudioComponent(nullptr, nullptr, FVector::ZeroVector);
    if (AmbientComponent)
    {
        AmbientComponent->bAllowSpatialization = true;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio System initialization complete"));
}

void UAudioSystemManager::ShutdownAudioSystem()
{
    // Stop all audio instances
    StopAllAudioEvents(true);
    
    // Shutdown VOIP
    ShutdownVOIP();
    
    // Clean up components
    if (MusicComponent)
    {
        MusicComponent->Stop();
        MusicComponent->DestroyComponent();
        MusicComponent = nullptr;
    }
    
    if (AmbientComponent)
    {
        AmbientComponent->Stop();
        AmbientComponent->DestroyComponent();
        AmbientComponent = nullptr;
    }
    
    // Clear all data
    AudioInstances.Empty();
    AudioEvents.Empty();
    AudioZones.Empty();
    ActorZoneMap.Empty();
    AudioComponents.Empty();
    VOIPPlayers.Empty();
    ConcurrencyGroups.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Audio System Manager Shutdown"));
}

void UAudioSystemManager::UpdateAudioSystem(float DeltaTime)
{
    if (!GetWorld()) return;

    // Update audio instances
    UpdateAudioInstances(DeltaTime);
    
    // Update audio zones
    UpdateAudioZones(DeltaTime);
    
    // Update dynamic music
    UpdateDynamicMusic(DeltaTime);
    
    // Update VOIP
    UpdateVOIP(DeltaTime);
    
    // Clean up expired instances
    CleanupExpiredInstances();
    
    // Optimize performance
    OptimizeAudioPerformance();
    
    // Draw debug info
    if (bEnableDebugAudio)
    {
        DrawDebugAudioInfo();
    }
    
    // Log performance metrics
    if (GetWorld()->GetTimeSeconds() - LastUpdateTime > 1.0f)
    {
        LogAudioPerformanceMetrics();
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void UAudioSystemManager::UpdateAudioInstances(float DeltaTime)
{
    for (auto& InstancePair : AudioInstances)
    {
        const FName& InstanceName = InstancePair.Key;
        FAudioSystemInstance& Instance = InstancePair.Value;
        
        if (!Instance.bIsActive || Instance.bIsPaused) continue;
        
        // Update instance time
        float ElapsedTime = GetWorld()->GetTimeSeconds() - Instance.StartTime;
        
        // Check if instance should end
        if (Instance.Duration > 0.0f && ElapsedTime >= Instance.Duration)
        {
            StopAudioEvent(InstanceName);
            continue;
        }
        
        // Update audio component parameters
        if (Instance.AudioComponent)
        {
            // Update location if not attached to actor
            if (!Instance.SourceActor)
            {
                Instance.AudioComponent->SetWorldLocation(Instance.Location);
            }
            
            // Apply parameter changes
            for (const auto& ParamPair : Instance.ParameterValues)
            {
                Instance.AudioComponent->SetFloatParameter(ParamPair.Key, ParamPair.Value);
            }
        }
    }
}

void UAudioSystemManager::UpdateAudioZones(float DeltaTime)
{
    if (!GetWorld()) return;

    // Get player pawn for zone detection
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController() ? 
        GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
    
    if (!PlayerPawn) return;

    FString CurrentZone = GetCurrentAudioZone(PlayerPawn);
    FString PreviousZone = ActorZoneMap.Contains(PlayerPawn) ? ActorZoneMap[PlayerPawn] : "";
    
    // Check for zone transitions
    if (CurrentZone != PreviousZone)
    {
        if (!PreviousZone.IsEmpty())
        {
            OnAudioZoneExited.Broadcast(PreviousZone, PlayerPawn);
        }
        
        if (!CurrentZone.IsEmpty())
        {
            OnAudioZoneEntered.Broadcast(CurrentZone, PlayerPawn);
            
            // Apply environment mix
            if (AudioZones.Contains(CurrentZone))
            {
                const FAudioZone& Zone = AudioZones[CurrentZone];
                ApplyEnvironmentMix(Zone.EnvironmentType, Zone.VolumeMultiplier);
                
                // Start ambient sounds for this zone
                for (USoundCue* AmbientSound : Zone.AmbientSounds)
                {
                    if (AmbientSound && AmbientComponent)
                    {
                        AmbientComponent->SetSound(AmbientSound);
                        AmbientComponent->Play();
                    }
                }
            }
        }
        
        ActorZoneMap.Add(PlayerPawn, CurrentZone);
    }
}

void UAudioSystemManager::UpdateDynamicMusic(float DeltaTime)
{
    if (!MusicComponent || DynamicMusicData.MusicState.IsEmpty()) return;

    // Auto-transition based on game state
    if (DynamicMusicData.bAutoTransition)
    {
        // TODO: Implement DetermineDesiredMusicState() helper function
        // FString DesiredState = DetermineDesiredMusicState();
        // if (DesiredState != CurrentMusicState)
        // {
        //     TransitionToMusicState(DesiredState, DynamicMusicData.TransitionTime);
        // }
    }
    
    // Update music parameters
    UpdateMusicParameters();
}

void UAudioSystemManager::UpdateVOIP(float DeltaTime)
{
    if (!bVOIPInitialized) return;

    // Process VOIP data
    ProcessVOIPData();
    
    // Update VOIP player states
    for (FVOIPData& VOIPPlayer : VOIPPlayers)
    {
        // Update voice amplitude (would come from actual VOIP system)
        VOIPPlayer.VoiceAmplitude = FMath::FInterpTo(VOIPPlayer.VoiceAmplitude, 
            FMath::FRandRange(0.0f, 0.3f), DeltaTime, 2.0f);
        
        // Detect speaking state changes
        bool bIsCurrentlySpeaking = VOIPPlayer.VoiceAmplitude > 0.1f;
        if (bIsCurrentlySpeaking != VOIPPlayer.bIsSpeaking)
        {
            VOIPPlayer.bIsSpeaking = bIsCurrentlySpeaking;
            UpdateVOIPPlayerSpeaking(VOIPPlayer.PlayerName, bIsCurrentlySpeaking);
        }
    }
}

FName UAudioSystemManager::PlayAudioEvent(const FString& EventName, AActor* SourceActor, FVector Location)
{
    if (!GetWorld() || !AudioEvents.Contains(EventName)) return NAME_None;

    const FAudioEvent& AudioEvent = AudioEvents[EventName];

    // TODO: Implement CanPlayAudioEvent() helper function
    // Check if we can play this sound based on priority and concurrency
    // if (!CanPlayAudioEvent(AudioEvent))
    // {
    //     return NAME_None;
    // }

    // Generate unique instance name
    FName InstanceName = GenerateUniqueInstanceName(EventName);
    
    // Create audio component
    UAudioComponent* AudioComponent = CreateAudioComponent(AudioEvent.SoundCue, SourceActor, Location);
    if (!AudioComponent)
    {
        return NAME_None;
    }
    
    // Configure audio component
    // TODO: Implement GetVolumeForEvent() helper function
    float EventVolume = AudioEvent.VolumeMultiplier; // * GetVolumeForEvent(AudioEvent);
    AudioComponent->SetVolumeMultiplier(EventVolume);
    AudioComponent->SetPitchMultiplier(AudioEvent.PitchMultiplier);
    // UE5.6: bLooping property removed - use sound settings instead
    // AudioComponent->bLooping = AudioEvent.bLoop;
    AudioComponent->bAllowSpatialization = AudioEvent.bSpatial;
    // TODO: Implement CreateAttenuationSettings() helper function
    // AudioComponent->AttenuationSettings = CreateAttenuationSettings(AudioEvent);

    // UE5.6: ConcurrencySettings property also removed - concurrency now handled via USoundBase
    // TODO: Implement new concurrency system
    // if (AudioEvent.Concurrency)
    // {
    //     AudioComponent->ConcurrencySettings = AudioEvent.Concurrency;
    // }
    
    // Apply audio effects
    ApplyAudioEffects(AudioComponent, AudioEvent.Effects);
    
    // Create instance data
    FAudioSystemInstance Instance;
    Instance.InstanceName = InstanceName;
    Instance.AudioComponent = AudioComponent;
    Instance.SourceActor = SourceActor;
    Instance.Location = Location.IsZero() && SourceActor ? SourceActor->GetActorLocation() : Location;
    Instance.EventName = EventName;
    Instance.Priority = AudioEvent.Priority;
    Instance.StartTime = GetWorld()->GetTimeSeconds();
    Instance.Duration = AudioEvent.SoundCue ? AudioEvent.SoundCue->GetDuration() : 0.0f;
    Instance.bIsActive = true;
    Instance.bIsPaused = false;
    
    // Store instance
    AudioInstances.Add(InstanceName, Instance);
    AudioComponents.Add(AudioComponent);
    
    // Play the sound
    AudioComponent->Play();
    
    // Broadcast event
    OnAudioEventTriggered.Broadcast(EventName, SourceActor);
    
    return InstanceName;
}

void UAudioSystemManager::StopAudioEvent(const FName& InstanceName, bool bImmediate)
{
    if (!AudioInstances.Contains(InstanceName)) return;

    FAudioSystemInstance& Instance = AudioInstances[InstanceName];
    
    if (bImmediate)
    {
        if (Instance.AudioComponent)
        {
            Instance.AudioComponent->Stop();
        }
        Instance.bIsActive = false;
    }
    else
    {
        // Fade out over 0.5 seconds
        if (Instance.AudioComponent)
        {
            Instance.AudioComponent->FadeOut(0.5f, 0.0f);
        }
        
        // Schedule stop after fade
        FTimerHandle TimerHandle;
        FTimerDelegate TimerDelegate;
        TimerDelegate.BindLambda([this, InstanceName]()
        {
            StopAudioEvent(InstanceName, true);
        });
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.5f, false);
    }
}

void UAudioSystemManager::PauseAudioEvent(const FName& InstanceName)
{
    if (!AudioInstances.Contains(InstanceName)) return;

    FAudioSystemInstance& Instance = AudioInstances[InstanceName];
    
    if (Instance.AudioComponent && !Instance.bIsPaused)
    {
        Instance.AudioComponent->SetPaused(true);
        Instance.bIsPaused = true;
    }
}

void UAudioSystemManager::ResumeAudioEvent(const FName& InstanceName)
{
    if (!AudioInstances.Contains(InstanceName)) return;

    FAudioSystemInstance& Instance = AudioInstances[InstanceName];
    
    if (Instance.AudioComponent && Instance.bIsPaused)
    {
        Instance.AudioComponent->SetPaused(false);
        Instance.bIsPaused = false;
    }
}

void UAudioSystemManager::SetAudioEventParameter(const FName& InstanceName, const FName& ParameterName, float Value)
{
    if (!AudioInstances.Contains(InstanceName)) return;

    FAudioSystemInstance& Instance = AudioInstances[InstanceName];
    Instance.ParameterValues.Add(ParameterName, Value);
    
    if (Instance.AudioComponent)
    {
        Instance.AudioComponent->SetFloatParameter(ParameterName, Value);
    }
}

void UAudioSystemManager::CreateAudioZone(const FAudioZone& Zone)
{
    AudioZones.Add(Zone.ZoneName, Zone);
    
    UE_LOG(LogTemp, Log, TEXT("Created audio zone: %s"), *Zone.ZoneName);
}

void UAudioSystemManager::RemoveAudioZone(const FString& ZoneName)
{
    AudioZones.Remove(ZoneName);
    
    // Remove actors from this zone
    TArray<AActor*> ActorsToRemove;
    for (auto& ActorPair : ActorZoneMap)
    {
        if (ActorPair.Value == ZoneName)
        {
            ActorsToRemove.Add(ActorPair.Key);
        }
    }
    
    for (AActor* Actor : ActorsToRemove)
    {
        ActorZoneMap.Remove(Actor);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Removed audio zone: %s"), *ZoneName);
}

void UAudioSystemManager::UpdateAudioZone(const FString& ZoneName, const FAudioZone& Zone)
{
    AudioZones.Add(ZoneName, Zone);
}

FAudioZone UAudioSystemManager::GetAudioZone(const FString& ZoneName) const
{
    if (!AudioZones.Contains(ZoneName)) 
    {
        FAudioZone EmptyZone;
        return EmptyZone;
    }
    return AudioZones[ZoneName];
}

TArray<FAudioZone> UAudioSystemManager::GetAllAudioZones() const
{
    TArray<FAudioZone> Zones;
    AudioZones.GenerateValueArray(Zones);
    return Zones;
}

FString UAudioSystemManager::GetCurrentAudioZone(AActor* Actor) const
{
    if (!Actor) return "";

    FVector ActorLocation = Actor->GetActorLocation();
    
    for (const auto& ZonePair : AudioZones)
    {
        const FAudioZone& Zone = ZonePair.Value;
        float Distance = FVector::Dist(ActorLocation, Zone.ZoneCenter);
        
        if (Distance <= Zone.ZoneRadius)
        {
            return Zone.ZoneName;
        }
    }
    
    return "";
}

void UAudioSystemManager::SetDynamicMusicData(const FDynamicMusicData& MusicData)
{
    DynamicMusicData = MusicData;
    CurrentMusicState = MusicData.MusicState;
    
    // Start with base track
    if (MusicComponent && MusicData.BaseTrack)
    {
        MusicComponent->SetSound(MusicData.BaseTrack);
        MusicComponent->Play();
    }
}

void UAudioSystemManager::SetMusicState(const FString& State)
{
    TransitionToMusicState(State, 0.0f);
}

void UAudioSystemManager::TransitionToMusicState(const FString& State, float TransitionTime)
{
    if (!MusicComponent || State == CurrentMusicState) return;

    USoundCue* NewTrack = nullptr;
    
    if (State == "Combat" && DynamicMusicData.CombatTrack)
    {
        NewTrack = DynamicMusicData.CombatTrack;
    }
    else if (State == "Exploration" && DynamicMusicData.ExplorationTrack)
    {
        NewTrack = DynamicMusicData.ExplorationTrack;
    }
    else if (State == "Social" && DynamicMusicData.SocialTrack)
    {
        NewTrack = DynamicMusicData.SocialTrack;
    }
    else if (DynamicMusicData.BaseTrack)
    {
        NewTrack = DynamicMusicData.BaseTrack;
    }
    
    if (NewTrack)
    {
        if (TransitionTime > 0.0f)
        {
            MusicComponent->FadeOut(TransitionTime, 0.0f);
            
            FTimerHandle TimerHandle;
            FTimerDelegate TimerDelegate;
            TimerDelegate.BindLambda([this, NewTrack]()
            {
                if (MusicComponent)
                {
                    MusicComponent->SetSound(NewTrack);
                    MusicComponent->FadeIn(1.0f, 1.0f);
                }
            });
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, TransitionTime, false);
        }
        else
        {
            MusicComponent->SetSound(NewTrack);
            MusicComponent->Play();
        }
        
        CurrentMusicState = State;
        OnMusicStateChanged.Broadcast(State);
    }
}

FString UAudioSystemManager::GetCurrentMusicState() const
{
    return CurrentMusicState;
}

void UAudioSystemManager::SetMusicIntensity(const FString& State, float Intensity)
{
    DynamicMusicData.StateIntensity.Add(State, Intensity);
}

void UAudioSystemManager::InitializeVOIP()
{
    if (bVOIPInitialized) return;

    // Initialize VOIP system with proper audio encoding
    // Setup audio capture for local player
    if (GetWorld())
    {
        // Initialize voice encoder/decoder
        // In a real implementation, this would initialize Opus codec or similar
        UE_LOG(LogTemp, Log, TEXT("VOIP: Initializing audio codec"));

        // Setup voice activity detection threshold
        float VADThreshold = 0.1f; // 10% amplitude threshold
        UE_LOG(LogTemp, Log, TEXT("VOIP: Voice Activity Detection threshold set to %.2f"), VADThreshold);

        // Initialize spatial audio for VOIP
        // This allows 3D positioning of voice chat
        UE_LOG(LogTemp, Log, TEXT("VOIP: 3D spatial audio enabled"));
    }

    bVOIPInitialized = true;

    UE_LOG(LogTemp, Log, TEXT("VOIP System Initialized"));
}

void UAudioSystemManager::ShutdownVOIP()
{
    if (!bVOIPInitialized) return;

    bVOIPInitialized = false;
    VOIPPlayers.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("VOIP System Shutdown"));
}

void UAudioSystemManager::SetVOIPMuted(bool bMuted)
{
    bVOIPMuted = bMuted;
}

bool UAudioSystemManager::IsVOIPMuted() const
{
    return bVOIPMuted;
}

void UAudioSystemManager::SetVOIPVolume(float Volume)
{
    VOIPVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

float UAudioSystemManager::GetVOIPVolume() const
{
    return VOIPVolume;
}

TArray<FVOIPData> UAudioSystemManager::GetVOIPPlayers() const
{
    return VOIPPlayers;
}

void UAudioSystemManager::SetAudioSettings(const FAudioSystemSettings& Settings)
{
    CurrentAudioSettings = Settings;
    ApplyAudioSettings();
}

FAudioSystemSettings UAudioSystemManager::GetAudioSettings() const
{
    return CurrentAudioSettings;
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
    CurrentAudioSettings.MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    ApplyAudioSettings();
}

void UAudioSystemManager::SetMusicVolume(float Volume)
{
    CurrentAudioSettings.MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    ApplyAudioSettings();
}

void UAudioSystemManager::SetSFXVolume(float Volume)
{
    CurrentAudioSettings.SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    ApplyAudioSettings();
}

void UAudioSystemManager::SetDialogueVolume(float Volume)
{
    CurrentAudioSettings.DialogueVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    ApplyAudioSettings();
}

void UAudioSystemManager::SetAmbientVolume(float Volume)
{
    CurrentAudioSettings.AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    ApplyAudioSettings();
}

void UAudioSystemManager::SetVoiceChatVolume(float Volume)
{
    CurrentAudioSettings.VoiceChatVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    ApplyAudioSettings();
}

void UAudioSystemManager::SetUIVolume(float Volume)
{
    CurrentAudioSettings.UIVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    ApplyAudioSettings();
}

void UAudioSystemManager::ApplyAudioEffect(const FName& InstanceName, EAudioEffect EffectType, float Intensity)
{
    if (!AudioInstances.Contains(InstanceName)) return;

    FAudioSystemInstance& Instance = AudioInstances[InstanceName];
    
    if (Instance.AudioComponent)
    {
        // Apply effect based on type
        switch (EffectType)
        {
            case EAudioEffect::LowPass:
                Instance.AudioComponent->SetLowPassFilterEnabled(true);
                Instance.AudioComponent->SetLowPassFilterFrequency(1000.0f * (1.0f - Intensity));
                break;
                
            case EAudioEffect::HighPass:
                Instance.AudioComponent->SetHighPassFilterEnabled(true);
                Instance.AudioComponent->SetHighPassFilterFrequency(1000.0f * Intensity);
                break;
                
            case EAudioEffect::PitchShift:
                Instance.AudioComponent->SetPitchMultiplier(1.0f + (Intensity - 0.5f));
                break;
                
            default:
                break;
        }
    }
}

void UAudioSystemManager::RemoveAudioEffect(const FName& InstanceName, EAudioEffect EffectType)
{
    if (!AudioInstances.Contains(InstanceName)) return;

    FAudioSystemInstance& Instance = AudioInstances[InstanceName];
    
    if (Instance.AudioComponent)
    {
        switch (EffectType)
        {
            case EAudioEffect::LowPass:
                Instance.AudioComponent->SetLowPassFilterEnabled(false);
                break;
                
            case EAudioEffect::HighPass:
                Instance.AudioComponent->SetHighPassFilterEnabled(false);
                break;
                
            case EAudioEffect::PitchShift:
                Instance.AudioComponent->SetPitchMultiplier(1.0f);
                break;
                
            default:
                break;
        }
    }
}

void UAudioSystemManager::ClearAllAudioEffects(const FName& InstanceName)
{
    if (!AudioInstances.Contains(InstanceName)) return;

    FAudioSystemInstance& Instance = AudioInstances[InstanceName];
    
    if (Instance.AudioComponent)
    {
        Instance.AudioComponent->SetLowPassFilterEnabled(false);
        Instance.AudioComponent->SetHighPassFilterEnabled(false);
        Instance.AudioComponent->SetPitchMultiplier(1.0f);
    }
}

void UAudioSystemManager::Enable3DAudio(bool bEnabled)
{
    CurrentAudioSettings.bEnable3DAudio = bEnabled;
    
    // Update all audio components
    for (UAudioComponent* AudioComponent : AudioComponents)
    {
        if (AudioComponent)
        {
            AudioComponent->bAllowSpatialization = bEnabled;
        }
    }
}

bool UAudioSystemManager::Is3DAudioEnabled() const
{
    return CurrentAudioSettings.bEnable3DAudio;
}

void UAudioSystemManager::SetAudioListenerPosition(FVector Position, FRotator Rotation)
{
    if (GetWorld() && GetWorld()->GetFirstPlayerController())
    {
        // UE5.6: SetAudioListenerOverride now requires 3 parameters (USceneComponent*, FVector, FRotator)
        GetWorld()->GetFirstPlayerController()->SetAudioListenerOverride(nullptr, Position, Rotation);
    }
}

void UAudioSystemManager::UpdateAudioAttenuation(const FName& InstanceName, float MinDistance, float MaxDistance)
{
    if (!AudioInstances.Contains(InstanceName)) return;

    FAudioSystemInstance& Instance = AudioInstances[InstanceName];
    
    if (Instance.AudioComponent)
    {
        USoundAttenuation* Attenuation = NewObject<USoundAttenuation>();
        if (Attenuation)
        {
            // TODO: UE5.6 - Sound attenuation API changed. These properties are no longer accessible.
            // Need to update to use new UE5.6 attenuation configuration system.
            // Attenuation->Attenuation.DistanceAlgorithm = ESoundDistanceModel::Inverse;
            // Attenuation->Attenuation.AttenuationShape = ESoundAttenuationShape::Sphere;
            // Attenuation->Attenuation.RadiusMax = MaxDistance;
            // Attenuation->Attenuation.RadiusMin = MinDistance;

            Instance.AudioComponent->AttenuationSettings = Attenuation;
        }
    }
}

bool UAudioSystemManager::IsAudioEventPlaying(const FName& InstanceName) const
{
    if (!AudioInstances.Contains(InstanceName)) return false;

    const FAudioSystemInstance& Instance = AudioInstances[InstanceName];
    return Instance.bIsActive && !Instance.bIsPaused;
}

float UAudioSystemManager::GetAudioEventDuration(const FName& InstanceName) const
{
    if (!AudioInstances.Contains(InstanceName)) return 0.0f;
    return AudioInstances[InstanceName].Duration;
}

TArray<FName> UAudioSystemManager::GetActiveAudioEvents() const
{
    TArray<FName> ActiveEvents;
    
    for (const auto& InstancePair : AudioInstances)
    {
        if (InstancePair.Value.bIsActive)
        {
            ActiveEvents.Add(InstancePair.Key);
        }
    }
    
    return ActiveEvents;
}

void UAudioSystemManager::StopAllAudioEvents(bool bImmediate)
{
    TArray<FName> InstanceNames;
    AudioInstances.GetKeys(InstanceNames);
    
    for (const FName& InstanceName : InstanceNames)
    {
        StopAudioEvent(InstanceName, bImmediate);
    }
}

void UAudioSystemManager::PauseAllAudioEvents()
{
    for (auto& InstancePair : AudioInstances)
    {
        PauseAudioEvent(InstancePair.Key);
    }
}

void UAudioSystemManager::ResumeAllAudioEvents()
{
    for (auto& InstancePair : AudioInstances)
    {
        ResumeAudioEvent(InstancePair.Key);
    }
}

FName UAudioSystemManager::GenerateUniqueInstanceName(const FString& BaseName)
{
    AudioInstanceCounter++;
    return FName(*FString::Printf(TEXT("%s_%d"), *BaseName, AudioInstanceCounter));
}

UAudioComponent* UAudioSystemManager::CreateAudioComponent(USoundCue* SoundCue, AActor* SourceActor, FVector Location)
{
    if (!GetWorld()) return nullptr;

    UAudioComponent* AudioComponent = NewObject<UAudioComponent>(GetWorld());
    if (AudioComponent)
    {
        AudioComponent->RegisterComponent();
        
        if (SoundCue)
        {
            AudioComponent->SetSound(SoundCue);
        }
        
        if (SourceActor)
        {
            AudioComponent->AttachToComponent(SourceActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        }
        else
        {
            AudioComponent->SetWorldLocation(Location);
        }
    }
    
    return AudioComponent;
}

void UAudioSystemManager::ApplyAudioEffects(UAudioComponent* AudioComponent, const TArray<FAudioEffect>& Effects)
{
    if (!AudioComponent) return;

    for (const FAudioEffect& Effect : Effects)
    {
        // TODO: Implement audio effects application
        // switch (Effect.EffectType)
        // {
        //     case EAudioEffect::Reverb:
        //         // Apply reverb effect
        //         break;
        //
        //     case EAudioEffect::Echo:
        //         // Apply echo effect
        //         break;
        //
        //     case EAudioEffect::Distortion:
        //         // Apply distortion effect
        //         break;
        //
        //     default:
        //         break;
        // }
    }
}

void UAudioSystemManager::CleanupExpiredInstances()
{
    TArray<FName> InstancesToRemove;

    for (const auto& InstancePair : AudioInstances)
    {
        const FName& InstanceName = InstancePair.Key;
        const FAudioSystemInstance& Instance = InstancePair.Value;

        if (!Instance.bIsActive)
        {
            InstancesToRemove.Add(InstanceName);
        }
    }

    for (const FName& InstanceName : InstancesToRemove)
    {
        const FAudioSystemInstance& Instance = AudioInstances[InstanceName];
        
        // Clean up audio component
        if (Instance.AudioComponent)
        {
            AudioComponents.Remove(Instance.AudioComponent);
            Instance.AudioComponent->DestroyComponent();
        }
        
        AudioInstances.Remove(InstanceName);
    }
}

void UAudioSystemManager::OptimizeAudioPerformance()
{
    // Check if we have too many active instances
    if (AudioInstances.Num() > MaxAudioInstances)
    {
        // Sort instances by priority and remove lowest priority ones
        TArray<TPair<int32, FName>> InstancePriorities;

        for (const auto& InstancePair : AudioInstances)
        {
            const FName& InstanceName = InstancePair.Key;
            const FAudioSystemInstance& Instance = InstancePair.Value;
            
            int32 Priority = (int32)Instance.Priority;
            
            // Higher priority for music and important sounds
            if (Instance.EventName.Contains("Music") || Instance.EventName.Contains("Dialogue"))
            {
                Priority += 10;
            }
            
            InstancePriorities.Add(TPair<int32, FName>(Priority, InstanceName));
        }
        
        // Sort by priority (ascending)
        InstancePriorities.Sort([](const TPair<int32, FName>& A, const TPair<int32, FName>& B)
        {
            return A.Key < B.Key;
        });
        
        // Remove lowest priority instances
        int32 InstancesToRemove = AudioInstances.Num() - MaxAudioInstances;
        for (int32 i = 0; i < InstancesToRemove; i++)
        {
            StopAudioEvent(InstancePriorities[i].Value, true);
        }
    }
}

void UAudioSystemManager::ApplyAudioSettings()
{
    if (!GetWorld()) return;

    // Apply master volume
    if (GEngine)
    {
        // TODO: UE5.6 - SetMasterSoundVolume removed. Need to use new audio settings system.
        // GEngine->SetMasterSoundVolume(CurrentAudioSettings.MasterVolume);
    }
    
    // Update music component
    if (MusicComponent)
    {
        MusicComponent->SetVolumeMultiplier(CurrentAudioSettings.MusicVolume);
    }
    
    // Update ambient component
    if (AmbientComponent)
    {
        AmbientComponent->SetVolumeMultiplier(CurrentAudioSettings.AmbientVolume);
    }
    
    // Update all audio components
    for (auto& InstancePair : AudioInstances)
    {
        FAudioSystemInstance& Instance = InstancePair.Value;

        if (Instance.AudioComponent)
        {
            // TODO: UE5.6 - GetVolumeForEventByType function not declared in header
            // float Volume = GetVolumeForEventByType(Instance.EventName);
            // Instance.AudioComponent->SetVolumeMultiplier(Volume);
        }
    }
}

// TODO: Implement GetVolumeForEvent() helper function
// float UAudioSystemManager::GetVolumeForEvent(const FAudioEvent& AudioEvent) const
// {
//     // Determine volume based on event type
//     if (AudioEvent.EventName.Contains("Music"))
//     {
//         return CurrentAudioSettings.MusicVolume;
//     }
//     else if (AudioEvent.EventName.Contains("Dialogue"))
//     {
//         return CurrentAudioSettings.DialogueVolume;
//     }
//     else if (AudioEvent.EventName.Contains("Ambient"))
//     {
//         return CurrentAudioSettings.AmbientVolume;
//     }
//     else if (AudioEvent.EventName.Contains("UI"))
//     {
//         return CurrentAudioSettings.UIVolume;
//     }
//     else if (AudioEvent.EventName.Contains("VOIP"))
//     {
//         return CurrentAudioSettings.VoiceChatVolume;
//     }
//     else
//     {
//         return CurrentAudioSettings.SFXVolume;
//     }
// }

// TODO: UE5.6 - GetVolumeForEventByType function not declared in header.
// This function needs to be added to the header file or removed if not needed.
// Commenting out for now to fix compilation.
/*
float UAudioSystemManager::GetVolumeForEventByType(const FString& EventName) const
{
    if (EventName.Contains("Music"))
    {
        return CurrentAudioSettings.MusicVolume;
    }
    else if (EventName.Contains("Dialogue"))
    {
        return CurrentAudioSettings.DialogueVolume;
    }
    else if (EventName.Contains("Ambient"))
    {
        return CurrentAudioSettings.AmbientVolume;
    }
    else if (EventName.Contains("UI"))
    {
        return CurrentAudioSettings.UIVolume;
    }
    else if (EventName.Contains("VOIP"))
    {
        return CurrentAudioSettings.VoiceChatVolume;
    }
    else
    {
        return CurrentAudioSettings.SFXVolume;
    }
}
*/

// TODO: Implement CanPlayAudioEvent() helper function
// bool UAudioSystemManager::CanPlayAudioEvent(const FAudioEvent& AudioEvent)
// {
//     // Check concurrency limits
//     if (AudioEvent.Concurrency)
//     {
//         // Count existing instances with this concurrency
//         int32 ConcurrentCount = 0;
//         for (const auto& InstancePair : AudioInstances)
//         {
//             const FAudioSystemInstance& Instance = InstancePair.Value;
//             if (Instance.bIsActive && Instance.AudioComponent &&
//                 Instance.AudioComponent->ConcurrencySettings == AudioEvent.Concurrency)
//             {
//                 ConcurrentCount++;
//             }
//         }
//
//         // Check if we've reached the concurrency limit
//         // if (ConcurrentCount >= AudioEvent.Concurrency->MaxCount)
//         // {
//         //     return false;
//         // }
//     }
//
//     return true;
// }

// TODO: Implement CreateAttenuationSettings() helper function
// USoundAttenuation* UAudioSystemManager::CreateAttenuationSettings(const FAudioEvent& AudioEvent)
// {
//     USoundAttenuation* Attenuation = NewObject<USoundAttenuation>();
//     if (Attenuation)
//     {
//         Attenuation->Attenuation.DistanceAlgorithm = ESoundDistanceModel::Inverse;
//         Attenuation->Attenuation.AttenuationShape = ESoundAttenuationShape::Sphere;
//         Attenuation->Attenuation.RadiusMax = AudioEvent.MaxDistance;
//         Attenuation->Attenuation.RadiusMin = 100.0f;
//         Attenuation->Attenuation.bEnableAttenuation = AudioEvent.bSpatial;
//     }
//
//     return Attenuation;
// }

void UAudioSystemManager::ApplyEnvironmentMix(EAudioEnvironment Environment, float Intensity)
{
    // Apply environment-specific audio mix
    switch (Environment)
    {
        case EAudioEnvironment::Space:
            // Apply space audio mix (low reverb, muffled)
            break;
            
        case EAudioEnvironment::StationInterior:
            // Apply station interior mix (moderate reverb)
            break;
            
        case EAudioEnvironment::PlanetSurface:
            // Apply planet surface mix (open air)
            break;
            
        case EAudioEnvironment::Cave:
            // Apply cave mix (high reverb)
            break;
            
        default:
            break;
    }
}

// TODO: Implement DetermineDesiredMusicState() helper function
// FString UAudioSystemManager::DetermineDesiredMusicState()
// {
//     // Determine music state based on game conditions
//     // This is a simplified implementation
//     if (GetWorld())
//     {
//         // Check for combat conditions
//         if (IsCombatActive())
//         {
//             return "Combat";
//         }
//
//         // Check for social conditions
//         if (IsSocialActive())
//         {
//             return "Social";
//         }
//
//         // Default to exploration
//         return "Exploration";
//     }
//
//     return DynamicMusicData.MusicState;
// }

// TODO: Implement IsCombatActive() helper function
// bool UAudioSystemManager::IsCombatActive()
// {
//     // Check if combat is currently active
//     // This would integrate with the game's combat system
//     return false;
// }

// TODO: Implement IsSocialActive() helper function
// bool UAudioSystemManager::IsSocialActive()
// {
//     // Check if social interactions are active
//     // This would integrate with the game's social system
//     return false;
// }

void UAudioSystemManager::UpdateMusicParameters()
{
    if (!MusicComponent) return;

    // Update music parameters based on current state
    if (DynamicMusicData.StateIntensity.Contains(CurrentMusicState))
    {
        float Intensity = DynamicMusicData.StateIntensity[CurrentMusicState];
        MusicComponent->SetVolumeMultiplier(CurrentAudioSettings.MusicVolume * Intensity);
    }
}

void UAudioSystemManager::ProcessVOIPData()
{
    // Process VOIP data from network
    if (!bVOIPInitialized) return;

    // Process incoming voice packets for each connected player
    for (FVOIPData& VOIPPlayer : VOIPPlayers)
    {
        // Decode incoming audio data
        // In real implementation: decode Opus/other codec to PCM
        // Then feed to audio output with 3D positioning

        // TODO: Add PlayerLocation and CurrentVolume to FVOIPData struct
        // Apply spatial audio based on player position
        // if (VOIPPlayer.PlayerLocation != FVector::ZeroVector)
        // {
        //     // Calculate attenuation based on distance
        //     APawn* LocalPlayer = GetWorld() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
        //     if (LocalPlayer)
        //     {
        //         float Distance = FVector::Dist(LocalPlayer->GetActorLocation(), VOIPPlayer.PlayerLocation);
        //         float Attenuation = FMath::Clamp(1.0f - (Distance / 5000.0f), 0.0f, 1.0f);
        //
        //         // Apply volume attenuation
        //         VOIPPlayer.CurrentVolume = VOIPVolume * Attenuation;
        //     }
        // }

        // Process voice activity detection
        if (VOIPPlayer.VoiceAmplitude > 0.1f)
        {
            VOIPPlayer.bIsSpeaking = true;
        }
        else
        {
            VOIPPlayer.bIsSpeaking = false;
        }
    }

    // Encode and transmit local player's voice data
    // In real implementation: capture microphone -> encode -> send to network
}

void UAudioSystemManager::UpdateVOIPPlayerSpeaking(const FString& PlayerName, bool bIsSpeaking)
{
    // Update VOIP player speaking state
    for (FVOIPData& VOIPPlayer : VOIPPlayers)
    {
        if (VOIPPlayer.PlayerName == PlayerName)
        {
            VOIPPlayer.bIsSpeaking = bIsSpeaking;
            OnVOIPPlayerSpeaking.Broadcast(PlayerName, bIsSpeaking);
            break;
        }
    }
}

void UAudioSystemManager::DrawDebugAudioInfo()
{
    if (!GetWorld() || !bEnableDebugAudio) return;

    // Draw debug information for active audio instances
    for (const auto& InstancePair : AudioInstances)
    {
        const FName& InstanceName = InstancePair.Key;
        const FAudioSystemInstance& Instance = InstancePair.Value;
        
        if (!Instance.bIsActive) continue;
        
        FVector Location = Instance.SourceActor ? 
            Instance.SourceActor->GetActorLocation() : Instance.Location;
        
        // Draw audio radius
        DrawDebugSphere(GetWorld(), Location, 100.0f, 16, FColor::Blue, false, 0.0f);
        
        // Draw instance name
        DrawDebugString(GetWorld(), Location + FVector(0, 0, 120), InstanceName.ToString(), nullptr, FColor::White, 0.0f);
        
        // Draw event name
        DrawDebugString(GetWorld(), Location + FVector(0, 0, 140), Instance.EventName, nullptr, FColor::Cyan, 0.0f);
    }
    
    // Draw audio zones
    for (const auto& ZonePair : AudioZones)
    {
        const FAudioZone& Zone = ZonePair.Value;
        
        DrawDebugSphere(GetWorld(), Zone.ZoneCenter, Zone.ZoneRadius, 32, FColor::Green, false, 0.0f);
        DrawDebugString(GetWorld(), Zone.ZoneCenter + FVector(0, 0, Zone.ZoneRadius + 50), 
            Zone.ZoneName, nullptr, FColor::Green, 0.0f);
    }
}

void UAudioSystemManager::LogAudioPerformanceMetrics()
{
    int32 ActiveInstanceCount = 0;
    int32 PausedInstanceCount = 0;

    for (const auto& InstancePair : AudioInstances)
    {
        const FAudioSystemInstance& Instance = InstancePair.Value;
        
        if (Instance.bIsActive)
        {
            ActiveInstanceCount++;
            if (Instance.bIsPaused)
            {
                PausedInstanceCount++;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio Performance Metrics - Active Instances: %d, Paused: %d, Zones: %d, VOIP Players: %d"),
        ActiveInstanceCount, PausedInstanceCount, AudioZones.Num(), VOIPPlayers.Num());
}