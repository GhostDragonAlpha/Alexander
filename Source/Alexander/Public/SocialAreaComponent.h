#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerState.h"
#include "VoiceChatSystem.h"
#include "SocialAreaComponent.generated.h"

UENUM(BlueprintType)
enum class ESocialInteractionType : uint8
{
    None,
    VoiceChat,
    TextChat,
    Emote,
    Gesture,
    Trade,
    GroupInvite,
    FriendRequest,
    Duel
};

UENUM(BlueprintType)
enum class ESocialAreaType : uint8
{
    General,
    Bar,
    Lounge,
    Cantina,
    Recreation,
    Meeting,
    Private,
    VIP
};

USTRUCT(BlueprintType)
struct FSocialInteraction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESocialInteractionType InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APlayerState* InitiatingPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APlayerState* TargetPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresConsent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UAnimMontage* InteractionAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundBase* InteractionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UParticleSystem* InteractionEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FString> InteractionParameters;
};

USTRUCT(BlueprintType)
struct FSocialGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName GroupName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APlayerState* GroupLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<APlayerState*> GroupMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPrivate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowInvites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime GroupCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FString> GroupProperties;
};

USTRUCT(BlueprintType)
struct FSocialZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESocialAreaType AreaType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxPlayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableVoiceChat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableTextChat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableEmotes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableGestures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VoiceChatRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<ESocialInteractionType, bool> AllowedInteractions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* ZoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundBase* ZoneAmbientSound;
};

/**
 * Wrapper for players in zones
 */
USTRUCT()
struct FPlayersInZone
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<APlayerState*> Players;
};

/**
 * Wrapper for player voice zones
 */
USTRUCT()
struct FPlayerVoiceZone
{
	GENERATED_BODY()

	UPROPERTY()
	FName ZoneName;
};

/**
 * Wrapper for player group memberships
 */
USTRUCT()
struct FPlayerGroupMemberships
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FName> GroupNames;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API USocialAreaComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USocialAreaComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Social Zone Management
    UFUNCTION(BlueprintCallable, Category = "Social Zone")
    void InitializeSocialArea();

    UFUNCTION(BlueprintCallable, Category = "Social Zone")
    void RegisterSocialZone(const FSocialZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Social Zone")
    void UnregisterSocialZone(const FName& ZoneName);

    FSocialZone* GetSocialZone(const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Social Zone")
    TArray<FSocialZone> GetAllSocialZones();

    // Player Management
    UFUNCTION(BlueprintCallable, Category = "Player Management")
    void OnPlayerEnterSocialZone(APlayerState* Player, const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Player Management")
    void OnPlayerLeaveSocialZone(APlayerState* Player, const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Player Management")
    bool IsPlayerInSocialZone(APlayerState* Player, const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Player Management")
    TArray<APlayerState*> GetPlayersInSocialZone(const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Player Management")
    TArray<APlayerState*> GetPlayersInSocialRange(APlayerState* Player, float Range);

    // Voice Chat Management
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void EnableVoiceChatForPlayer(APlayerState* Player, const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void DisableVoiceChatForPlayer(APlayerState* Player, const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void SetVoiceChatRange(APlayerState* Player, float Range);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    TArray<APlayerState*> GetPlayersInVoiceRange(APlayerState* Player);

    // Social Interactions
    UFUNCTION(BlueprintCallable, Category = "Social Interactions")
    void InitiateSocialInteraction(const FSocialInteraction& Interaction);

    UFUNCTION(BlueprintCallable, Category = "Social Interactions")
    void RespondToSocialInteraction(APlayerState* Player, const FSocialInteraction& Interaction, bool bAccept);

    UFUNCTION(BlueprintCallable, Category = "Social Interactions")
    void CancelSocialInteraction(APlayerState* Player, const FSocialInteraction& Interaction);

    UFUNCTION(BlueprintCallable, Category = "Social Interactions")
    bool CanInitiateInteraction(APlayerState* Player, ESocialInteractionType InteractionType, const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Social Interactions")
    TArray<ESocialInteractionType> GetAvailableInteractions(APlayerState* Player, const FName& ZoneName);

    // Group Management
    UFUNCTION(BlueprintCallable, Category = "Group Management")
    void CreateSocialGroup(const FSocialGroup& Group);

    UFUNCTION(BlueprintCallable, Category = "Group Management")
    void DissolveSocialGroup(const FName& GroupName);

    UFUNCTION(BlueprintCallable, Category = "Group Management")
    void InvitePlayerToGroup(APlayerState* Inviter, APlayerState* Invitee, const FName& GroupName);

    UFUNCTION(BlueprintCallable, Category = "Group Management")
    void JoinGroup(APlayerState* Player, const FName& GroupName);

    UFUNCTION(BlueprintCallable, Category = "Group Management")
    void LeaveGroup(APlayerState* Player, const FName& GroupName);

    UFUNCTION(BlueprintCallable, Category = "Group Management")
    void PromoteGroupMember(const FName& GroupName, APlayerState* Member);

    UFUNCTION(BlueprintCallable, Category = "Group Management")
    void KickFromGroup(const FName& GroupName, APlayerState* Member);

    FSocialGroup* GetSocialGroup(const FName& GroupName);

    UFUNCTION(BlueprintCallable, Category = "Group Management")
    TArray<FSocialGroup> GetAllSocialGroups();

    UFUNCTION(BlueprintCallable, Category = "Group Management")
    TArray<FSocialGroup> GetPlayerGroups(APlayerState* Player);

    // Emotes and Gestures
    UFUNCTION(BlueprintCallable, Category = "Emotes")
    void PlayEmote(APlayerState* Player, const FName& EmoteName);

    UFUNCTION(BlueprintCallable, Category = "Emotes")
    void PlayGesture(APlayerState* Player, const FName& GestureName);

    UFUNCTION(BlueprintCallable, Category = "Emotes")
    void RegisterEmote(const FName& EmoteName, UAnimMontage* Animation, USoundBase* Sound);

    UFUNCTION(BlueprintCallable, Category = "Emotes")
    void RegisterGesture(const FName& GestureName, UAnimMontage* Animation, USoundBase* Sound);

    UFUNCTION(BlueprintCallable, Category = "Emotes")
    TArray<FName> GetAvailableEmotes();

    UFUNCTION(BlueprintCallable, Category = "Emotes")
    TArray<FName> GetAvailableGestures();

    // Social Events
    UFUNCTION(BlueprintCallable, Category = "Social Events")
    void StartSocialEvent(const FName& EventName, const FString& Description, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Social Events")
    void EndSocialEvent(const FName& EventName);

    UFUNCTION(BlueprintCallable, Category = "Social Events")
    void BroadcastSocialMessage(const FName& ZoneName, const FString& Message, APlayerState* Sender = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Social Events")
    void SendPrivateMessage(APlayerState* Sender, APlayerState* Recipient, const FString& Message);

    // Utilities
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float GetSocialActivityLevel(const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    TMap<FName, int32> GetZonePopulationStats();

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    TArray<APlayerState*> GetNearbyPlayers(APlayerState* Player, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool ArePlayersInSocialRange(APlayerState* Player1, APlayerState* Player2, float Range);

protected:
    // Social Zones
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Social Zones")
    TMap<FName, FSocialZone> SocialZones;

    TMap<FName, FPlayersInZone> PlayersInZones;

    // Voice Chat
    TMap<APlayerState*, FPlayerVoiceZone> PlayerVoiceZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voice Chat")
    TMap<APlayerState*, float> PlayerVoiceRanges;

    // Active Interactions
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactions")
    TArray<FSocialInteraction> ActiveInteractions;

    // Social Groups
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Groups")
    TMap<FName, FSocialGroup> SocialGroups;

    TMap<APlayerState*, FPlayerGroupMemberships> PlayerGroupMemberships;

    // Emotes and Gestures
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Emotes")
    TMap<FName, UAnimMontage*> EmoteAnimations;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Emotes")
    TMap<FName, USoundBase*> EmoteSounds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gestures")
    TMap<FName, UAnimMontage*> GestureAnimations;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gestures")
    TMap<FName, USoundBase*> GestureSounds;

    // Social Events
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Events")
    TMap<FName, float> SocialEventStartTimes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Events")
    TMap<FName, FString> SocialEventDescriptions;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float DefaultVoiceChatRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float DefaultSocialRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxGroupSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableProximityVoiceChat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableSocialEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableGroupSystem;

    // System References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    class UVoiceChatSystem* VoiceChatSystem;

private:
    // Internal Functions
    void UpdateSocialZones(float DeltaTime);
    void ProcessVoiceChat(float DeltaTime);
    void UpdateSocialInteractions(float DeltaTime);
    void ProcessSocialEvents(float DeltaTime);

    // Voice Chat Internal
    void UpdateProximityVoiceChat(APlayerState* Player);
    void AddPlayerToVoiceChannel(APlayerState* Player, const FName& ChannelName);
    void RemovePlayerFromVoiceChannel(APlayerState* Player, const FName& ChannelName);

    // Interaction Internal
    void ExecuteInteraction(const FSocialInteraction& Interaction);
    void CompleteInteraction(const FSocialInteraction& Interaction);
    bool ValidateInteraction(const FSocialInteraction& Interaction);

    // Group Internal
    void AddPlayerToGroup(APlayerState* Player, const FName& GroupName);
    void RemovePlayerFromGroup(APlayerState* Player, const FName& GroupName);
    bool CanJoinGroup(APlayerState* Player, const FName& GroupName);
    void NotifyGroupMembers(const FName& GroupName, const FString& Message);

    // Emote Internal
    void PlayEmoteForPlayer(APlayerState* Player, const FName& EmoteName);
    void PlayGestureForPlayer(APlayerState* Player, const FName& GestureName);

    // Event Internal
    void ProcessSocialEvent(const FName& EventName, float DeltaTime);
    void BroadcastEventMessage(const FName& EventName, const FString& Message);

    // Utilities Internal
    float CalculateZoneActivity(const FName& ZoneName);
    void UpdateZonePopulation(const FName& ZoneName);

    // Timers
    UPROPERTY()
    FTimerHandle SocialUpdateTimer;

    UPROPERTY()
    FTimerHandle VoiceChatUpdateTimer;

    UPROPERTY()
    FTimerHandle InteractionUpdateTimer;

    // Constants
    static constexpr float SOCIAL_UPDATE_INTERVAL = 0.1f;
    static constexpr float VOICE_CHAT_UPDATE_INTERVAL = 0.05f;
    static constexpr float INTERACTION_UPDATE_INTERVAL = 0.1f;
    static constexpr float DEFAULT_VOICE_CHAT_RANGE = 500.0f;
    static constexpr float DEFAULT_SOCIAL_RADIUS = 1000.0f;
    static constexpr int32 DEFAULT_MAX_GROUP_SIZE = 8;

    // Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerEnteredSocialZone, APlayerState*, Player, const FName&, ZoneName, const FSocialZone&, ZoneData);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerLeftSocialZone, APlayerState*, Player, const FName&, ZoneName, const FSocialZone&, ZoneData);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSocialInteractionStarted, const FSocialInteraction&, Interaction, bool, bAccepted);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSocialInteractionCompleted, const FSocialInteraction&, Interaction);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGroupCreated, const FName&, GroupName, const FSocialGroup&, GroupData);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGroupDisbanded, const FName&, GroupName, const FSocialGroup&, GroupData);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerJoinedGroup, APlayerState*, Player, const FName&, GroupName, const FSocialGroup&, GroupData);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerLeftGroup, APlayerState*, Player, const FName&, GroupName, const FSocialGroup&, GroupData);

public:
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Social Events")
    FOnPlayerEnteredSocialZone OnPlayerEnteredSocialZone;

    UPROPERTY(BlueprintAssignable, Category = "Social Events")
    FOnPlayerLeftSocialZone OnPlayerLeftSocialZone;

    UPROPERTY(BlueprintAssignable, Category = "Social Events")
    FOnSocialInteractionStarted OnSocialInteractionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Social Events")
    FOnSocialInteractionCompleted OnSocialInteractionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Social Events")
    FOnGroupCreated OnGroupCreated;

    UPROPERTY(BlueprintAssignable, Category = "Social Events")
    FOnGroupDisbanded OnGroupDisbanded;

    UPROPERTY(BlueprintAssignable, Category = "Social Events")
    FOnPlayerJoinedGroup OnPlayerJoinedGroup;

    UPROPERTY(BlueprintAssignable, Category = "Social Events")
    FOnPlayerLeftGroup OnPlayerLeftGroup;
};