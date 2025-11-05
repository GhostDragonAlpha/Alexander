// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoiceChatSystem.generated.h"

// Forward declarations
class AActor;
class USoundWave;

/**
 * Voice chat channel types
 */
UENUM(BlueprintType)
enum class EVoiceChannelType : uint8
{
    Proximity       UMETA(DisplayName = "Proximity Chat"),
    Team            UMETA(DisplayName = "Team Chat"),
    Guild           UMETA(DisplayName = "Guild Chat"),
    Party           UMETA(DisplayName = "Party Chat"),
    Global          UMETA(DisplayName = "Global Chat"),
    Private         UMETA(DisplayName = "Private Chat"),
    Radio           UMETA(DisplayName = "Radio Channel"),
    Emergency       UMETA(DisplayName = "Emergency Channel")
};

/**
 * Voice quality settings
 */
UENUM(BlueprintType)
enum class EVoiceQuality : uint8
{
    Low         UMETA(DisplayName = "Low (8kHz)"),
    Medium      UMETA(DisplayName = "Medium (16kHz)"),
    High        UMETA(DisplayName = "High (24kHz)"),
    VeryHigh    UMETA(DisplayName = "Very High (48kHz)")
};

/**
 * Voice codec types
 */
UENUM(BlueprintType)
enum class EVoiceCodec : uint8
{
    Opus        UMETA(DisplayName = "Opus"),
    PCM         UMETA(DisplayName = "PCM"),
    AAC         UMETA(DisplayName = "AAC"),
    Vorbis      UMETA(DisplayName = "Vorbis")
};

/**
 * Speaker state
 */
UENUM(BlueprintType)
enum class ESpeakerState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Speaking    UMETA(DisplayName = "Speaking"),
    Muted       UMETA(DisplayName = "Muted"),
    Deafened    UMETA(DisplayName = "Deafened")
};

/**
 * Voice chat participant information
 */
USTRUCT(BlueprintType)
struct FVoiceParticipant
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    FString ParticipantID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    AActor* Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    ESpeakerState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    bool bIsTalking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    float VoiceActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    float DistanceFromListener;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    TArray<EVoiceChannelType> ActiveChannels;

    FVoiceParticipant()
        : Actor(nullptr)
        , State(ESpeakerState::Idle)
        , Volume(1.0f)
        , bIsTalking(false)
        , VoiceActivity(0.0f)
        , DistanceFromListener(0.0f)
    {}
};

/**
 * Voice channel configuration
 */
USTRUCT(BlueprintType)
struct FAlexanderVoiceChannel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    FString ChannelID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    EVoiceChannelType ChannelType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    FString ChannelName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    TArray<FString> ParticipantIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    float MaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    bool bIsEncrypted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    bool bRequiresPassword;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    int32 MaxParticipants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    float VolumeMultiplier;

    FAlexanderVoiceChannel()
        : ChannelType(EVoiceChannelType::Proximity)
        , MaxDistance(5000.0f)
        , bIsEncrypted(false)
        , bRequiresPassword(false)
        , MaxParticipants(100)
        , VolumeMultiplier(1.0f)
    {}
};

/**
 * Voice transmission data
 */
USTRUCT(BlueprintType)
struct FVoiceTransmission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    FString SenderID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    FString ChannelID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    TArray<uint8> AudioData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    int32 SequenceNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    EVoiceCodec Codec;

    FVoiceTransmission()
        : Timestamp(0.0f)
        , SequenceNumber(0)
        , Codec(EVoiceCodec::Opus)
    {}
};

/**
 * Voice chat statistics
 */
USTRUCT(BlueprintType)
struct FVoiceChatStatistics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    int32 ActiveSpeakers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    int32 TotalParticipants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    float AverageBitrate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    float PacketLoss;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    float AverageLatency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    int32 BytesSent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    int32 BytesReceived;

    FVoiceChatStatistics()
        : ActiveSpeakers(0)
        , TotalParticipants(0)
        , AverageBitrate(0.0f)
        , PacketLoss(0.0f)
        , AverageLatency(0.0f)
        , BytesSent(0)
        , BytesReceived(0)
    {}
};

/**
 * Spatial audio settings
 */
USTRUCT(BlueprintType)
struct FSpatialAudioSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    bool bEnableSpatialAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float AttenuationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float DopplerScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    bool bEnableOcclusion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float OcclusionAttenuation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    bool bEnableReverb;

    FSpatialAudioSettings()
        : bEnableSpatialAudio(true)
        , AttenuationDistance(5000.0f)
        , DopplerScale(1.0f)
        , bEnableOcclusion(true)
        , OcclusionAttenuation(0.5f)
        , bEnableReverb(true)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnParticipantJoined, const FString&, ParticipantID, const FString&, ChannelID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnParticipantLeft, const FString&, ParticipantID, const FString&, ChannelID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnParticipantStartedTalking, const FString&, ParticipantID, const FString&, ChannelID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnParticipantStoppedTalking, const FString&, ParticipantID, const FString&, ChannelID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVoiceDataReceived, const FString&, SenderID, const TArray<uint8>&, AudioData, const FString&, ChannelID);

/**
 * Voice Chat System Component
 * Manages voice communication, spatial audio, and channel management
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UVoiceChatSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVoiceChatSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Participant Management
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    FString RegisterParticipant(AActor* Actor, const FString& DisplayName);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool UnregisterParticipant(const FString& ParticipantID);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    FVoiceParticipant GetParticipant(const FString& ParticipantID) const;

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    TArray<FVoiceParticipant> GetAllParticipants() const;

    // Channel Management
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    FString CreateChannel(EVoiceChannelType ChannelType, const FString& ChannelName, float MaxDistance);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool DeleteChannel(const FString& ChannelID);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool JoinChannel(const FString& ParticipantID, const FString& ChannelID);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool LeaveChannel(const FString& ParticipantID, const FString& ChannelID);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    TArray<FAlexanderVoiceChannel> GetAvailableChannels() const;

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    FAlexanderVoiceChannel GetChannel(const FString& ChannelID) const;

    // Voice Transmission
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool StartTransmitting(const FString& ParticipantID, const FString& ChannelID);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool StopTransmitting(const FString& ParticipantID);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool TransmitVoiceData(const FString& ParticipantID, const TArray<uint8>& AudioData, const FString& ChannelID);

    // Mute Controls
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void SetParticipantMuted(const FString& ParticipantID, bool bMuted);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void SetParticipantDeafened(const FString& ParticipantID, bool bDeafened);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool IsParticipantMuted(const FString& ParticipantID) const;

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool IsParticipantTalking(const FString& ParticipantID) const;

    // Volume Controls
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void SetParticipantVolume(const FString& ParticipantID, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void SetChannelVolume(const FString& ChannelID, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void SetMasterVolume(float Volume);

    // Proximity Chat
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    TArray<FString> GetParticipantsInProximity(const FVector& Location, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void UpdateProximityChat(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    float CalculateProximityVolume(float Distance, float MaxDistance) const;

    // Spatial Audio
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void EnableSpatialAudio(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void SetSpatialAudioSettings(const FSpatialAudioSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    FSpatialAudioSettings GetSpatialAudioSettings() const;

    // Voice Activity Detection
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void EnableVoiceActivityDetection(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    float GetVoiceActivity(const FString& ParticipantID) const;

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void SetVoiceActivityThreshold(float Threshold);

    // Push-to-Talk
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void EnablePushToTalk(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool IsPushToTalkEnabled() const;

    // Quality Settings
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void SetVoiceQuality(EVoiceQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void SetVoiceCodec(EVoiceCodec Codec);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    EVoiceQuality GetVoiceQuality() const;

    // Statistics
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    FVoiceChatStatistics GetStatistics() const;

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void ResetStatistics();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Voice Chat")
    FOnParticipantJoined OnParticipantJoined;

    UPROPERTY(BlueprintAssignable, Category = "Voice Chat")
    FOnParticipantLeft OnParticipantLeft;

    UPROPERTY(BlueprintAssignable, Category = "Voice Chat")
    FOnParticipantStartedTalking OnParticipantStartedTalking;

    UPROPERTY(BlueprintAssignable, Category = "Voice Chat")
    FOnParticipantStoppedTalking OnParticipantStoppedTalking;

    UPROPERTY(BlueprintAssignable, Category = "Voice Chat")
    FOnVoiceDataReceived OnVoiceDataReceived;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    bool bEnableVoiceChat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    bool bEnablePushToTalk;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    bool bEnableVoiceActivityDetection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    float VoiceActivityThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    EVoiceQuality DefaultVoiceQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    EVoiceCodec DefaultCodec;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    float ProximityRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Chat")
    FSpatialAudioSettings SpatialSettings;

private:
    // Participant storage
    UPROPERTY()
    TMap<FString, FVoiceParticipant> Participants;

    UPROPERTY()
    TMap<FString, FAlexanderVoiceChannel> Channels;

    UPROPERTY()
    TMap<FString, FString> ActiveTransmissions; // ParticipantID -> ChannelID

    int32 NextParticipantID;
    int32 NextChannelID;
    FVoiceChatStatistics Statistics;

    // Helper functions
    bool CanJoinChannel(const FString& ParticipantID, const FString& ChannelID) const;
    void ProcessVoiceTransmission(const FVoiceTransmission& Transmission);
    void UpdateVoiceActivity(float DeltaTime);
    void UpdateSpatialAudio(float DeltaTime);
    bool CheckLineOfSight(const FVector& From, const FVector& To) const;
    float CalculateOcclusion(const FVector& From, const FVector& To) const;
    void BroadcastToChannel(const FString& ChannelID, const FVoiceTransmission& Transmission);
    int32 GetSampleRate(EVoiceQuality Quality) const;
    int32 GetBitrate(EVoiceQuality Quality, EVoiceCodec Codec) const;
};
