// Copyright Epic Games, Inc. All Rights Reserved.

#include "VoiceChatSystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UVoiceChatSystem::UVoiceChatSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz for voice updates
    
    bEnableVoiceChat = true;
    bEnablePushToTalk = false;
    bEnableVoiceActivityDetection = true;
    VoiceActivityThreshold = 0.1f;
    DefaultVoiceQuality = EVoiceQuality::Medium;
    DefaultCodec = EVoiceCodec::Opus;
    MasterVolume = 1.0f;
    ProximityRadius = 5000.0f;
    
    NextParticipantID = 1;
    NextChannelID = 1;
}

void UVoiceChatSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Create default proximity channel
    CreateChannel(EVoiceChannelType::Proximity, TEXT("Proximity"), ProximityRadius);
}

void UVoiceChatSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableVoiceChat)
    {
        return;
    }
    
    // Update voice activity detection
    if (bEnableVoiceActivityDetection)
    {
        UpdateVoiceActivity(DeltaTime);
    }
    
    // Update proximity chat
    UpdateProximityChat(DeltaTime);
    
    // Update spatial audio
    if (SpatialSettings.bEnableSpatialAudio)
    {
        UpdateSpatialAudio(DeltaTime);
    }
}

FString UVoiceChatSystem::RegisterParticipant(AActor* Actor, const FString& DisplayName)
{
    if (!Actor)
    {
        return FString();
    }
    
    FVoiceParticipant Participant;
    Participant.ParticipantID = FString::Printf(TEXT("P_%d"), NextParticipantID++);
    Participant.DisplayName = DisplayName;
    Participant.Actor = Actor;
    Participant.State = ESpeakerState::Idle;
    Participant.Volume = MasterVolume;
    
    Participants.Add(Participant.ParticipantID, Participant);
    Statistics.TotalParticipants++;
    
    UE_LOG(LogTemp, Log, TEXT("Registered voice participant: %s (%s)"), *DisplayName, *Participant.ParticipantID);
    
    return Participant.ParticipantID;
}

bool UVoiceChatSystem::UnregisterParticipant(const FString& ParticipantID)
{
    if (!Participants.Contains(ParticipantID))
    {
        return false;
    }
    
    // Leave all channels
    for (auto& ChannelPair : Channels)
    {
        ChannelPair.Value.ParticipantIDs.Remove(ParticipantID);
    }
    
    // Stop any active transmissions
    ActiveTransmissions.Remove(ParticipantID);
    
    Participants.Remove(ParticipantID);
    Statistics.TotalParticipants--;
    
    return true;
}

FVoiceParticipant UVoiceChatSystem::GetParticipant(const FString& ParticipantID) const
{
    const FVoiceParticipant* Participant = Participants.Find(ParticipantID);
    return Participant ? *Participant : FVoiceParticipant();
}

TArray<FVoiceParticipant> UVoiceChatSystem::GetAllParticipants() const
{
    TArray<FVoiceParticipant> AllParticipants;
    for (const auto& ParticipantPair : Participants)
    {
        AllParticipants.Add(ParticipantPair.Value);
    }
    return AllParticipants;
}

FString UVoiceChatSystem::CreateChannel(EVoiceChannelType ChannelType, const FString& ChannelName, float MaxDistance)
{
    FAlexanderVoiceChannel Channel;
    Channel.ChannelID = FString::Printf(TEXT("C_%d"), NextChannelID++);
    Channel.ChannelType = ChannelType;
    Channel.ChannelName = ChannelName;
    Channel.MaxDistance = MaxDistance;
    
    Channels.Add(Channel.ChannelID, Channel);
    
    UE_LOG(LogTemp, Log, TEXT("Created voice channel: %s (%s)"), *ChannelName, *Channel.ChannelID);
    
    return Channel.ChannelID;
}

bool UVoiceChatSystem::DeleteChannel(const FString& ChannelID)
{
    if (!Channels.Contains(ChannelID))
    {
        return false;
    }
    
    // Remove all participants
    FAlexanderVoiceChannel* Channel = Channels.Find(ChannelID);
    if (Channel)
    {
        for (const FString& ParticipantID : Channel->ParticipantIDs)
        {
            if (FVoiceParticipant* Participant = Participants.Find(ParticipantID))
            {
                Participant->ActiveChannels.Remove(Channel->ChannelType);
            }
        }
    }
    
    Channels.Remove(ChannelID);
    return true;
}

bool UVoiceChatSystem::JoinChannel(const FString& ParticipantID, const FString& ChannelID)
{
    if (!Participants.Contains(ParticipantID) || !Channels.Contains(ChannelID))
    {
        return false;
    }
    
    if (!CanJoinChannel(ParticipantID, ChannelID))
    {
        return false;
    }
    
    FAlexanderVoiceChannel* Channel = Channels.Find(ChannelID);
    FVoiceParticipant* Participant = Participants.Find(ParticipantID);
    
    if (Channel && Participant)
    {
        Channel->ParticipantIDs.AddUnique(ParticipantID);
        Participant->ActiveChannels.AddUnique(Channel->ChannelType);
        
        OnParticipantJoined.Broadcast(ParticipantID, ChannelID);
        return true;
    }
    
    return false;
}

bool UVoiceChatSystem::LeaveChannel(const FString& ParticipantID, const FString& ChannelID)
{
    FAlexanderVoiceChannel* Channel = Channels.Find(ChannelID);
    FVoiceParticipant* Participant = Participants.Find(ParticipantID);
    
    if (Channel && Participant)
    {
        Channel->ParticipantIDs.Remove(ParticipantID);
        Participant->ActiveChannels.Remove(Channel->ChannelType);
        
        OnParticipantLeft.Broadcast(ParticipantID, ChannelID);
        return true;
    }
    
    return false;
}

TArray<FAlexanderVoiceChannel> UVoiceChatSystem::GetAvailableChannels() const
{
    TArray<FAlexanderVoiceChannel> AvailableChannels;
    for (const auto& ChannelPair : Channels)
    {
        AvailableChannels.Add(ChannelPair.Value);
    }
    return AvailableChannels;
}

FAlexanderVoiceChannel UVoiceChatSystem::GetChannel(const FString& ChannelID) const
{
    const FAlexanderVoiceChannel* Channel = Channels.Find(ChannelID);
    return Channel ? *Channel : FAlexanderVoiceChannel();
}

bool UVoiceChatSystem::StartTransmitting(const FString& ParticipantID, const FString& ChannelID)
{
    if (!Participants.Contains(ParticipantID) || !Channels.Contains(ChannelID))
    {
        return false;
    }
    
    FVoiceParticipant* Participant = Participants.Find(ParticipantID);
    if (Participant && Participant->State != ESpeakerState::Muted)
    {
        Participant->State = ESpeakerState::Speaking;
        Participant->bIsTalking = true;
        ActiveTransmissions.Add(ParticipantID, ChannelID);
        Statistics.ActiveSpeakers++;
        
        OnParticipantStartedTalking.Broadcast(ParticipantID, ChannelID);
        return true;
    }
    
    return false;
}

bool UVoiceChatSystem::StopTransmitting(const FString& ParticipantID)
{
    FVoiceParticipant* Participant = Participants.Find(ParticipantID);
    if (Participant && Participant->bIsTalking)
    {
        FString* ChannelID = ActiveTransmissions.Find(ParticipantID);
        if (ChannelID)
        {
            OnParticipantStoppedTalking.Broadcast(ParticipantID, *ChannelID);
        }
        
        Participant->State = ESpeakerState::Idle;
        Participant->bIsTalking = false;
        ActiveTransmissions.Remove(ParticipantID);
        Statistics.ActiveSpeakers--;
        
        return true;
    }
    
    return false;
}

bool UVoiceChatSystem::TransmitVoiceData(const FString& ParticipantID, const TArray<uint8>& AudioData, const FString& ChannelID)
{
    if (!Participants.Contains(ParticipantID) || !Channels.Contains(ChannelID))
    {
        return false;
    }
    
    FVoiceTransmission Transmission;
    Transmission.SenderID = ParticipantID;
    Transmission.ChannelID = ChannelID;
    Transmission.AudioData = AudioData;
    Transmission.Timestamp = GetWorld()->GetTimeSeconds();
    Transmission.Codec = DefaultCodec;
    
    BroadcastToChannel(ChannelID, Transmission);
    Statistics.BytesSent += AudioData.Num();
    
    return true;
}

void UVoiceChatSystem::SetParticipantMuted(const FString& ParticipantID, bool bMuted)
{
    FVoiceParticipant* Participant = Participants.Find(ParticipantID);
    if (Participant)
    {
        Participant->State = bMuted ? ESpeakerState::Muted : ESpeakerState::Idle;
        if (bMuted)
        {
            StopTransmitting(ParticipantID);
        }
    }
}

void UVoiceChatSystem::SetParticipantDeafened(const FString& ParticipantID, bool bDeafened)
{
    FVoiceParticipant* Participant = Participants.Find(ParticipantID);
    if (Participant)
    {
        Participant->State = bDeafened ? ESpeakerState::Deafened : ESpeakerState::Idle;
        if (bDeafened)
        {
            StopTransmitting(ParticipantID);
        }
    }
}

bool UVoiceChatSystem::IsParticipantMuted(const FString& ParticipantID) const
{
    const FVoiceParticipant* Participant = Participants.Find(ParticipantID);
    return Participant && Participant->State == ESpeakerState::Muted;
}

bool UVoiceChatSystem::IsParticipantTalking(const FString& ParticipantID) const
{
    const FVoiceParticipant* Participant = Participants.Find(ParticipantID);
    return Participant && Participant->bIsTalking;
}

void UVoiceChatSystem::SetParticipantVolume(const FString& ParticipantID, float Volume)
{
    FVoiceParticipant* Participant = Participants.Find(ParticipantID);
    if (Participant)
    {
        Participant->Volume = FMath::Clamp(Volume, 0.0f, 2.0f);
    }
}

void UVoiceChatSystem::SetChannelVolume(const FString& ChannelID, float Volume)
{
    FAlexanderVoiceChannel* Channel = Channels.Find(ChannelID);
    if (Channel)
    {
        Channel->VolumeMultiplier = FMath::Clamp(Volume, 0.0f, 2.0f);
    }
}

void UVoiceChatSystem::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
}

TArray<FString> UVoiceChatSystem::GetParticipantsInProximity(const FVector& Location, float Radius) const
{
    TArray<FString> NearbyParticipants;
    
    for (const auto& ParticipantPair : Participants)
    {
        if (ParticipantPair.Value.Actor)
        {
            float Distance = FVector::Dist(ParticipantPair.Value.Actor->GetActorLocation(), Location);
            if (Distance <= Radius)
            {
                NearbyParticipants.Add(ParticipantPair.Key);
            }
        }
    }
    
    return NearbyParticipants;
}

void UVoiceChatSystem::UpdateProximityChat(float DeltaTime)
{
    // Find proximity channel
    FString ProximityChannelID;
    for (const auto& ChannelPair : Channels)
    {
        if (ChannelPair.Value.ChannelType == EVoiceChannelType::Proximity)
        {
            ProximityChannelID = ChannelPair.Key;
            break;
        }
    }
    
    if (ProximityChannelID.IsEmpty())
    {
        return;
    }
    
    // Update distances for all participants
    for (auto& ParticipantPair : Participants)
    {
        if (ParticipantPair.Value.Actor && GetWorld()->GetFirstPlayerController())
        {
            APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
            if (PlayerPawn)
            {
                float Distance = FVector::Dist(
                    ParticipantPair.Value.Actor->GetActorLocation(),
                    PlayerPawn->GetActorLocation()
                );
                ParticipantPair.Value.DistanceFromListener = Distance;
            }
        }
    }
}

float UVoiceChatSystem::CalculateProximityVolume(float Distance, float MaxDistance) const
{
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    // Inverse square falloff
    float NormalizedDistance = Distance / MaxDistance;
    return FMath::Square(1.0f - NormalizedDistance);
}

void UVoiceChatSystem::EnableSpatialAudio(bool bEnable)
{
    SpatialSettings.bEnableSpatialAudio = bEnable;
}

void UVoiceChatSystem::SetSpatialAudioSettings(const FSpatialAudioSettings& Settings)
{
    SpatialSettings = Settings;
}

FSpatialAudioSettings UVoiceChatSystem::GetSpatialAudioSettings() const
{
    return SpatialSettings;
}

void UVoiceChatSystem::EnableVoiceActivityDetection(bool bEnable)
{
    bEnableVoiceActivityDetection = bEnable;
}

float UVoiceChatSystem::GetVoiceActivity(const FString& ParticipantID) const
{
    const FVoiceParticipant* Participant = Participants.Find(ParticipantID);
    return Participant ? Participant->VoiceActivity : 0.0f;
}

void UVoiceChatSystem::SetVoiceActivityThreshold(float Threshold)
{
    VoiceActivityThreshold = FMath::Clamp(Threshold, 0.0f, 1.0f);
}

void UVoiceChatSystem::EnablePushToTalk(bool bEnable)
{
    bEnablePushToTalk = bEnable;
}

bool UVoiceChatSystem::IsPushToTalkEnabled() const
{
    return bEnablePushToTalk;
}

void UVoiceChatSystem::SetVoiceQuality(EVoiceQuality Quality)
{
    DefaultVoiceQuality = Quality;
}

void UVoiceChatSystem::SetVoiceCodec(EVoiceCodec Codec)
{
    DefaultCodec = Codec;
}

EVoiceQuality UVoiceChatSystem::GetVoiceQuality() const
{
    return DefaultVoiceQuality;
}

FVoiceChatStatistics UVoiceChatSystem::GetStatistics() const
{
    return Statistics;
}

void UVoiceChatSystem::ResetStatistics()
{
    Statistics = FVoiceChatStatistics();
    Statistics.TotalParticipants = Participants.Num();
}

bool UVoiceChatSystem::CanJoinChannel(const FString& ParticipantID, const FString& ChannelID) const
{
    const FAlexanderVoiceChannel* Channel = Channels.Find(ChannelID);
    if (!Channel)
    {
        return false;
    }
    
    // Check max participants
    if (Channel->ParticipantIDs.Num() >= Channel->MaxParticipants)
    {
        return false;
    }
    
    return true;
}

void UVoiceChatSystem::ProcessVoiceTransmission(const FVoiceTransmission& Transmission)
{
    // Process and decode voice data (placeholder for actual audio processing)
    Statistics.BytesReceived += Transmission.AudioData.Num();
}

void UVoiceChatSystem::UpdateVoiceActivity(float DeltaTime)
{
    for (auto& ParticipantPair : Participants)
    {
        FVoiceParticipant& Participant = ParticipantPair.Value;
        
        if (Participant.bIsTalking)
        {
            // Simulate voice activity (in real implementation, would analyze audio)
            Participant.VoiceActivity = FMath::FRandRange(0.5f, 1.0f);
        }
        else
        {
            Participant.VoiceActivity = FMath::Max(0.0f, Participant.VoiceActivity - DeltaTime * 2.0f);
        }
    }
}

void UVoiceChatSystem::UpdateSpatialAudio(float DeltaTime)
{
    // Update 3D audio positioning for all active speakers
    for (const auto& TransmissionPair : ActiveTransmissions)
    {
        const FVoiceParticipant* Participant = Participants.Find(TransmissionPair.Key);
        if (Participant && Participant->Actor)
        {
            // Calculate occlusion if enabled
            if (SpatialSettings.bEnableOcclusion && GetWorld()->GetFirstPlayerController())
            {
                APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
                if (PlayerPawn)
                {
                    float Occlusion = CalculateOcclusion(
                        PlayerPawn->GetActorLocation(),
                        Participant->Actor->GetActorLocation()
                    );
                    // Apply occlusion to volume (would be done in audio system)
                }
            }
        }
    }
}

bool UVoiceChatSystem::CheckLineOfSight(const FVector& From, const FVector& To) const
{
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    return !GetWorld()->LineTraceSingleByChannel(
        HitResult,
        From,
        To,
        ECC_Visibility,
        QueryParams
    );
}

float UVoiceChatSystem::CalculateOcclusion(const FVector& From, const FVector& To) const
{
    if (CheckLineOfSight(From, To))
    {
        return 1.0f; // No occlusion
    }
    
    return SpatialSettings.OcclusionAttenuation;
}

void UVoiceChatSystem::BroadcastToChannel(const FString& ChannelID, const FVoiceTransmission& Transmission)
{
    const FAlexanderVoiceChannel* Channel = Channels.Find(ChannelID);
    if (!Channel)
    {
        return;
    }
    
    for (const FString& ParticipantID : Channel->ParticipantIDs)
    {
        if (ParticipantID != Transmission.SenderID)
        {
            const FVoiceParticipant* Participant = Participants.Find(ParticipantID);
            if (Participant && Participant->State != ESpeakerState::Deafened)
            {
                OnVoiceDataReceived.Broadcast(Transmission.SenderID, Transmission.AudioData, ChannelID);
            }
        }
    }
}

int32 UVoiceChatSystem::GetSampleRate(EVoiceQuality Quality) const
{
    switch (Quality)
    {
    case EVoiceQuality::Low:        return 8000;
    case EVoiceQuality::Medium:     return 16000;
    case EVoiceQuality::High:       return 24000;
    case EVoiceQuality::VeryHigh:   return 48000;
    default:                        return 16000;
    }
}

int32 UVoiceChatSystem::GetBitrate(EVoiceQuality Quality, EVoiceCodec Codec) const
{
    int32 BaseBitrate = 24000; // 24 kbps baseline
    
    switch (Quality)
    {
    case EVoiceQuality::Low:        BaseBitrate = 12000; break;
    case EVoiceQuality::Medium:     BaseBitrate = 24000; break;
    case EVoiceQuality::High:       BaseBitrate = 48000; break;
    case EVoiceQuality::VeryHigh:   BaseBitrate = 96000; break;
    }
    
    // Codec efficiency multipliers
    switch (Codec)
    {
    case EVoiceCodec::Opus:     return BaseBitrate;
    case EVoiceCodec::AAC:      return BaseBitrate * 1.2f;
    case EVoiceCodec::Vorbis:   return BaseBitrate * 1.1f;
    case EVoiceCodec::PCM:      return BaseBitrate * 8.0f;
    default:                    return BaseBitrate;
    }
}
