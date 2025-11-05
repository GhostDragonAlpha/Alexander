#include "SocialAreaComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

USocialAreaComponent::USocialAreaComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = SOCIAL_UPDATE_INTERVAL;

    // Configuration defaults
    DefaultVoiceChatRange = DEFAULT_VOICE_CHAT_RANGE;
    DefaultSocialRadius = DEFAULT_SOCIAL_RADIUS;
    MaxGroupSize = DEFAULT_MAX_GROUP_SIZE;
    bEnableProximityVoiceChat = true;
    bEnableSocialEvents = true;
    bEnableGroupSystem = true;
}

void USocialAreaComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeSocialArea();
}

void USocialAreaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateSocialZones(DeltaTime);
    
    if (bEnableProximityVoiceChat)
    {
        ProcessVoiceChat(DeltaTime);
    }
    
    UpdateSocialInteractions(DeltaTime);
    ProcessSocialEvents(DeltaTime);
}

void USocialAreaComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(SocialUpdateTimer);
        World->GetTimerManager().ClearTimer(VoiceChatUpdateTimer);
        World->GetTimerManager().ClearTimer(InteractionUpdateTimer);
    }

    // Clear all data
    SocialZones.Empty();
    PlayersInZones.Empty();
    PlayerVoiceZones.Empty();
    SocialGroups.Empty();
    ActiveInteractions.Empty();

    Super::EndPlay(EndPlayReason);
}

void USocialAreaComponent::InitializeSocialArea()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Initialize voice chat system
    VoiceChatSystem = NewObject<UVoiceChatSystem>(this, TEXT("VoiceChatSystem"));

    // Setup timers
    World->GetTimerManager().SetTimer(SocialUpdateTimer, [this]()
    {
        float DeltaTime = SOCIAL_UPDATE_INTERVAL;
        UpdateSocialZones(DeltaTime);
    }, SOCIAL_UPDATE_INTERVAL, true);

    if (bEnableProximityVoiceChat)
    {
        World->GetTimerManager().SetTimer(VoiceChatUpdateTimer, [this]()
        {
            float DeltaTime = VOICE_CHAT_UPDATE_INTERVAL;
            ProcessVoiceChat(DeltaTime);
        }, VOICE_CHAT_UPDATE_INTERVAL, true);
    }

    World->GetTimerManager().SetTimer(InteractionUpdateTimer, [this]()
    {
        float DeltaTime = INTERACTION_UPDATE_INTERVAL;
        UpdateSocialInteractions(DeltaTime);
    }, INTERACTION_UPDATE_INTERVAL, true);

    UE_LOG(LogTemp, Log, TEXT("SocialAreaComponent initialized"));
}

void USocialAreaComponent::RegisterSocialZone(const FSocialZone& Zone)
{
    if (SocialZones.Contains(Zone.ZoneName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Social zone '%s' already registered"), *Zone.ZoneName.ToString());
        return;
    }

    SocialZones.Add(Zone.ZoneName, Zone);
    PlayersInZones.Add(Zone.ZoneName, FPlayersInZone());

    UE_LOG(LogTemp, Log, TEXT("Social zone '%s' registered"), *Zone.ZoneName.ToString());
}

void USocialAreaComponent::UnregisterSocialZone(const FName& ZoneName)
{
    SocialZones.Remove(ZoneName);
    PlayersInZones.Remove(ZoneName);
}

FSocialZone* USocialAreaComponent::GetSocialZone(const FName& ZoneName)
{
    return SocialZones.Find(ZoneName);
}

TArray<FSocialZone> USocialAreaComponent::GetAllSocialZones()
{
    TArray<FSocialZone> Zones;
    SocialZones.GenerateValueArray(Zones);
    return Zones;
}

void USocialAreaComponent::OnPlayerEnterSocialZone(APlayerState* Player, const FName& ZoneName)
{
    if (!Player || !SocialZones.Contains(ZoneName))
    {
        return;
    }

    // Add to zone player list
    if (!PlayersInZones.Contains(ZoneName))
    {
        PlayersInZones.Add(ZoneName, FPlayersInZone());
    }
    PlayersInZones[ZoneName].Players.AddUnique(Player);

    const FSocialZone& Zone = SocialZones[ZoneName];

    // Enable voice chat if configured
    if (Zone.bEnableVoiceChat && bEnableProximityVoiceChat)
    {
        EnableVoiceChatForPlayer(Player, ZoneName);
    }

    OnPlayerEnteredSocialZone.Broadcast(Player, ZoneName, Zone);

    UE_LOG(LogTemp, Log, TEXT("Player entered social zone '%s'"), *ZoneName.ToString());
}

void USocialAreaComponent::OnPlayerLeaveSocialZone(APlayerState* Player, const FName& ZoneName)
{
    if (!Player)
    {
        return;
    }

    // Remove from zone player list
    if (PlayersInZones.Contains(ZoneName))
    {
        PlayersInZones[ZoneName].Players.Remove(Player);
    }

    // Disable voice chat
    DisableVoiceChatForPlayer(Player, ZoneName);

    if (SocialZones.Contains(ZoneName))
    {
        const FSocialZone& Zone = SocialZones[ZoneName];
        OnPlayerLeftSocialZone.Broadcast(Player, ZoneName, Zone);
    }

    UE_LOG(LogTemp, Log, TEXT("Player left social zone '%s'"), *ZoneName.ToString());
}

bool USocialAreaComponent::IsPlayerInSocialZone(APlayerState* Player, const FName& ZoneName)
{
    if (!Player || !PlayersInZones.Contains(ZoneName))
    {
        return false;
    }

    return PlayersInZones[ZoneName].Players.Contains(Player);
}

TArray<APlayerState*> USocialAreaComponent::GetPlayersInSocialZone(const FName& ZoneName)
{
    if (PlayersInZones.Contains(ZoneName))
    {
        return PlayersInZones[ZoneName].Players;
    }
    return TArray<APlayerState*>();
}

TArray<APlayerState*> USocialAreaComponent::GetPlayersInSocialRange(APlayerState* Player, float Range)
{
    return GetNearbyPlayers(Player, Range);
}

void USocialAreaComponent::EnableVoiceChatForPlayer(APlayerState* Player, const FName& ZoneName)
{
    if (!Player || !VoiceChatSystem)
    {
        return;
    }

    FPlayerVoiceZone VoiceZone;
    VoiceZone.ZoneName = ZoneName;
    PlayerVoiceZones.Add(Player, VoiceZone);

    AddPlayerToVoiceChannel(Player, ZoneName);
}

void USocialAreaComponent::DisableVoiceChatForPlayer(APlayerState* Player, const FName& ZoneName)
{
    if (!Player)
    {
        return;
    }

    PlayerVoiceZones.Remove(Player);
    RemovePlayerFromVoiceChannel(Player, ZoneName);
}

void USocialAreaComponent::SetVoiceChatRange(APlayerState* Player, float Range)
{
    if (Player)
    {
        PlayerVoiceRanges.Add(Player, Range);
    }
}

TArray<APlayerState*> USocialAreaComponent::GetPlayersInVoiceRange(APlayerState* Player)
{
    float Range = DefaultVoiceChatRange;
    if (PlayerVoiceRanges.Contains(Player))
    {
        Range = PlayerVoiceRanges[Player];
    }

    return GetNearbyPlayers(Player, Range);
}

void USocialAreaComponent::InitiateSocialInteraction(const FSocialInteraction& Interaction)
{
    if (!ValidateInteraction(Interaction))
    {
        return;
    }

    ActiveInteractions.Add(Interaction);
    
    OnSocialInteractionStarted.Broadcast(Interaction, false);

    if (!Interaction.bRequiresConsent)
    {
        ExecuteInteraction(Interaction);
    }
}

void USocialAreaComponent::RespondToSocialInteraction(APlayerState* Player, const FSocialInteraction& Interaction, bool bAccept)
{
    if (!Player || !ActiveInteractions.Contains(Interaction))
    {
        return;
    }

    if (bAccept)
    {
        ExecuteInteraction(Interaction);
        OnSocialInteractionStarted.Broadcast(Interaction, true);
    }
    else
    {
        CancelSocialInteraction(Player, Interaction);
    }
}

void USocialAreaComponent::CancelSocialInteraction(APlayerState* Player, const FSocialInteraction& Interaction)
{
    ActiveInteractions.Remove(Interaction);
}

bool USocialAreaComponent::CanInitiateInteraction(APlayerState* Player, ESocialInteractionType InteractionType, const FName& ZoneName)
{
    if (!Player || !SocialZones.Contains(ZoneName))
    {
        return false;
    }

    const FSocialZone& Zone = SocialZones[ZoneName];
    return Zone.AllowedInteractions.Contains(InteractionType) && Zone.AllowedInteractions[InteractionType];
}

TArray<ESocialInteractionType> USocialAreaComponent::GetAvailableInteractions(APlayerState* Player, const FName& ZoneName)
{
    TArray<ESocialInteractionType> Available;

    if (!SocialZones.Contains(ZoneName))
    {
        return Available;
    }

    const FSocialZone& Zone = SocialZones[ZoneName];
    for (const auto& InteractionPair : Zone.AllowedInteractions)
    {
        if (InteractionPair.Value)
        {
            Available.Add(InteractionPair.Key);
        }
    }

    return Available;
}

void USocialAreaComponent::CreateSocialGroup(const FSocialGroup& Group)
{
    if (SocialGroups.Contains(Group.GroupName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Social group '%s' already exists"), *Group.GroupName.ToString());
        return;
    }

    SocialGroups.Add(Group.GroupName, Group);
    
    OnGroupCreated.Broadcast(Group.GroupName, Group);

    UE_LOG(LogTemp, Log, TEXT("Social group '%s' created"), *Group.GroupName.ToString());
}

void USocialAreaComponent::DissolveSocialGroup(const FName& GroupName)
{
    if (!SocialGroups.Contains(GroupName))
    {
        return;
    }

    FSocialGroup Group = SocialGroups[GroupName];
    
    // Remove all members
    for (APlayerState* Member : Group.GroupMembers)
    {
        RemovePlayerFromGroup(Member, GroupName);
    }

    OnGroupDisbanded.Broadcast(GroupName, Group);
    
    SocialGroups.Remove(GroupName);

    UE_LOG(LogTemp, Log, TEXT("Social group '%s' dissolved"), *GroupName.ToString());
}

void USocialAreaComponent::InvitePlayerToGroup(APlayerState* Inviter, APlayerState* Invitee, const FName& GroupName)
{
    if (!Inviter || !Invitee || !SocialGroups.Contains(GroupName))
    {
        return;
    }

    FSocialGroup& Group = SocialGroups[GroupName];

    // Check if inviter has permission (must be leader or member if group allows invites)
    if (Group.GroupLeader != Inviter && !Group.bAllowInvites)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player %s cannot invite to group %s"), *Inviter->GetPlayerName(), *GroupName.ToString());
        return;
    }

    // Check if invitee is already in group
    if (Group.GroupMembers.Contains(Invitee))
    {
        UE_LOG(LogTemp, Warning, TEXT("Player %s is already in group %s"), *Invitee->GetPlayerName(), *GroupName.ToString());
        return;
    }

    // Check if group is full
    if (Group.GroupMembers.Num() >= Group.MaxMembers)
    {
        UE_LOG(LogTemp, Warning, TEXT("Group %s is full"), *GroupName.ToString());
        return;
    }

    // Create invite notification
    // In a real implementation, this would send a UI notification to the invitee
    UE_LOG(LogTemp, Log, TEXT("Group invite sent from %s to %s for group %s"),
        *Inviter->GetPlayerName(), *Invitee->GetPlayerName(), *GroupName.ToString());

    // For now, automatically accept the invite (in real implementation, player would accept/decline)
    // The actual accept/decline logic would call AddPlayerToGroup or reject the invite
}

void USocialAreaComponent::JoinGroup(APlayerState* Player, const FName& GroupName)
{
    if (!CanJoinGroup(Player, GroupName))
    {
        return;
    }

    AddPlayerToGroup(Player, GroupName);
}

void USocialAreaComponent::LeaveGroup(APlayerState* Player, const FName& GroupName)
{
    RemovePlayerFromGroup(Player, GroupName);
}

void USocialAreaComponent::PromoteGroupMember(const FName& GroupName, APlayerState* Member)
{
    if (!SocialGroups.Contains(GroupName) || !Member)
    {
        return;
    }

    FSocialGroup& Group = SocialGroups[GroupName];
    if (Group.GroupMembers.Contains(Member))
    {
        Group.GroupLeader = Member;
    }
}

void USocialAreaComponent::KickFromGroup(const FName& GroupName, APlayerState* Member)
{
    if (!SocialGroups.Contains(GroupName) || !Member)
    {
        return;
    }

    RemovePlayerFromGroup(Member, GroupName);
}

FSocialGroup* USocialAreaComponent::GetSocialGroup(const FName& GroupName)
{
    return SocialGroups.Find(GroupName);
}

TArray<FSocialGroup> USocialAreaComponent::GetAllSocialGroups()
{
    TArray<FSocialGroup> Groups;
    SocialGroups.GenerateValueArray(Groups);
    return Groups;
}

TArray<FSocialGroup> USocialAreaComponent::GetPlayerGroups(APlayerState* Player)
{
    TArray<FSocialGroup> PlayerGroupList;

    if (!Player || !PlayerGroupMemberships.Contains(Player))
    {
        return PlayerGroupList;
    }

    const FPlayerGroupMemberships& Memberships = PlayerGroupMemberships[Player];
    for (const FName& GroupName : Memberships.GroupNames)
    {
        if (SocialGroups.Contains(GroupName))
        {
            PlayerGroupList.Add(SocialGroups[GroupName]);
        }
    }

    return PlayerGroupList;
}

void USocialAreaComponent::PlayEmote(APlayerState* Player, const FName& EmoteName)
{
    if (!Player || !EmoteAnimations.Contains(EmoteName))
    {
        return;
    }

    PlayEmoteForPlayer(Player, EmoteName);
}

void USocialAreaComponent::PlayGesture(APlayerState* Player, const FName& GestureName)
{
    if (!Player || !GestureAnimations.Contains(GestureName))
    {
        return;
    }

    PlayGestureForPlayer(Player, GestureName);
}

void USocialAreaComponent::RegisterEmote(const FName& EmoteName, UAnimMontage* Animation, USoundBase* Sound)
{
    if (Animation)
    {
        EmoteAnimations.Add(EmoteName, Animation);
    }
    if (Sound)
    {
        EmoteSounds.Add(EmoteName, Sound);
    }
}

void USocialAreaComponent::RegisterGesture(const FName& GestureName, UAnimMontage* Animation, USoundBase* Sound)
{
    if (Animation)
    {
        GestureAnimations.Add(GestureName, Animation);
    }
    if (Sound)
    {
        GestureSounds.Add(GestureName, Sound);
    }
}

TArray<FName> USocialAreaComponent::GetAvailableEmotes()
{
    TArray<FName> Emotes;
    EmoteAnimations.GetKeys(Emotes);
    return Emotes;
}

TArray<FName> USocialAreaComponent::GetAvailableGestures()
{
    TArray<FName> Gestures;
    GestureAnimations.GetKeys(Gestures);
    return Gestures;
}

void USocialAreaComponent::StartSocialEvent(const FName& EventName, const FString& Description, float Duration)
{
    SocialEventStartTimes.Add(EventName, GetWorld()->GetTimeSeconds());
    SocialEventDescriptions.Add(EventName, Description);

    UE_LOG(LogTemp, Log, TEXT("Social event '%s' started"), *EventName.ToString());
}

void USocialAreaComponent::EndSocialEvent(const FName& EventName)
{
    SocialEventStartTimes.Remove(EventName);
    SocialEventDescriptions.Remove(EventName);

    UE_LOG(LogTemp, Log, TEXT("Social event '%s' ended"), *EventName.ToString());
}

void USocialAreaComponent::BroadcastSocialMessage(const FName& ZoneName, const FString& Message, APlayerState* Sender)
{
    if (!PlayersInZones.Contains(ZoneName))
    {
        return;
    }

    // Broadcast message to all players in zone
    UE_LOG(LogTemp, Log, TEXT("Social message in zone '%s': %s"), *ZoneName.ToString(), *Message);
}

void USocialAreaComponent::SendPrivateMessage(APlayerState* Sender, APlayerState* Recipient, const FString& Message)
{
    if (!Sender || !Recipient)
    {
        return;
    }

    // Send private message
}

float USocialAreaComponent::GetSocialActivityLevel(const FName& ZoneName)
{
    return CalculateZoneActivity(ZoneName);
}

TMap<FName, int32> USocialAreaComponent::GetZonePopulationStats()
{
    TMap<FName, int32> Stats;
    for (const auto& ZonePair : PlayersInZones)
    {
        Stats.Add(ZonePair.Key, ZonePair.Value.Players.Num());
    }
    return Stats;
}

TArray<APlayerState*> USocialAreaComponent::GetNearbyPlayers(APlayerState* Player, float Radius)
{
    TArray<APlayerState*> NearbyPlayers;

    if (!Player || !GetWorld())
    {
        return NearbyPlayers;
    }

    // Get player pawn location
    APawn* PlayerPawn = Player->GetPawn();
    if (!PlayerPawn)
    {
        return NearbyPlayers;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Iterate through all zones and check players
    for (const auto& ZonePair : PlayersInZones)
    {
        const FPlayersInZone& ZonePlayerList = ZonePair.Value;

        for (APlayerState* OtherPlayer : ZonePlayerList.Players)
        {
            if (OtherPlayer && OtherPlayer != Player)
            {
                APawn* OtherPawn = OtherPlayer->GetPawn();
                if (OtherPawn)
                {
                    float Distance = FVector::Dist(PlayerLocation, OtherPawn->GetActorLocation());
                    if (Distance <= Radius)
                    {
                        NearbyPlayers.AddUnique(OtherPlayer);
                    }
                }
            }
        }
    }

    return NearbyPlayers;
}

bool USocialAreaComponent::ArePlayersInSocialRange(APlayerState* Player1, APlayerState* Player2, float Range)
{
    if (!Player1 || !Player2)
    {
        return false;
    }

    APawn* Pawn1 = Player1->GetPawn();
    APawn* Pawn2 = Player2->GetPawn();

    if (!Pawn1 || !Pawn2)
    {
        return false;
    }

    float Distance = FVector::Dist(Pawn1->GetActorLocation(), Pawn2->GetActorLocation());
    return Distance <= Range;
}

void USocialAreaComponent::UpdateSocialZones(float DeltaTime)
{
    for (auto& ZonePair : SocialZones)
    {
        UpdateZonePopulation(ZonePair.Key);
    }
}

void USocialAreaComponent::ProcessVoiceChat(float DeltaTime)
{
    for (const auto& VoicePair : PlayerVoiceZones)
    {
        UpdateProximityVoiceChat(VoicePair.Key);
    }
}

void USocialAreaComponent::UpdateSocialInteractions(float DeltaTime)
{
    TArray<FSocialInteraction> CompletedInteractions;

    for (FSocialInteraction& Interaction : ActiveInteractions)
    {
        Interaction.Duration -= DeltaTime;
        if (Interaction.Duration <= 0.0f)
        {
            CompletedInteractions.Add(Interaction);
        }
    }

    for (const FSocialInteraction& Completed : CompletedInteractions)
    {
        CompleteInteraction(Completed);
        ActiveInteractions.Remove(Completed);
    }
}

void USocialAreaComponent::ProcessSocialEvents(float DeltaTime)
{
    if (!bEnableSocialEvents)
    {
        return;
    }

    // Process active social events
}

void USocialAreaComponent::UpdateProximityVoiceChat(APlayerState* Player)
{
    if (!Player || !VoiceChatSystem)
    {
        return;
    }

    // Update voice chat based on proximity
}

void USocialAreaComponent::AddPlayerToVoiceChannel(APlayerState* Player, const FName& ChannelName)
{
    // Add player to voice channel
}

void USocialAreaComponent::RemovePlayerFromVoiceChannel(APlayerState* Player, const FName& ChannelName)
{
    // Remove player from voice channel
}

void USocialAreaComponent::ExecuteInteraction(const FSocialInteraction& Interaction)
{
    // Execute the interaction
}

void USocialAreaComponent::CompleteInteraction(const FSocialInteraction& Interaction)
{
    OnSocialInteractionCompleted.Broadcast(Interaction);
}

bool USocialAreaComponent::ValidateInteraction(const FSocialInteraction& Interaction)
{
    return Interaction.InitiatingPlayer != nullptr;
}

void USocialAreaComponent::AddPlayerToGroup(APlayerState* Player, const FName& GroupName)
{
    if (!SocialGroups.Contains(GroupName) || !Player)
    {
        return;
    }

    FSocialGroup& Group = SocialGroups[GroupName];
    if (!Group.GroupMembers.Contains(Player))
    {
        Group.GroupMembers.Add(Player);

        // Update player memberships
        if (!PlayerGroupMemberships.Contains(Player))
        {
            PlayerGroupMemberships.Add(Player, FPlayerGroupMemberships());
        }
        PlayerGroupMemberships[Player].GroupNames.AddUnique(GroupName);

        OnPlayerJoinedGroup.Broadcast(Player, GroupName, Group);
    }
}

void USocialAreaComponent::RemovePlayerFromGroup(APlayerState* Player, const FName& GroupName)
{
    if (!SocialGroups.Contains(GroupName) || !Player)
    {
        return;
    }

    FSocialGroup& Group = SocialGroups[GroupName];
    Group.GroupMembers.Remove(Player);

    // Update player memberships
    if (PlayerGroupMemberships.Contains(Player))
    {
        PlayerGroupMemberships[Player].GroupNames.Remove(GroupName);
    }

    OnPlayerLeftGroup.Broadcast(Player, GroupName, Group);
}

bool USocialAreaComponent::CanJoinGroup(APlayerState* Player, const FName& GroupName)
{
    if (!Player || !SocialGroups.Contains(GroupName))
    {
        return false;
    }

    const FSocialGroup& Group = SocialGroups[GroupName];
    
    // Check max members
    if (Group.GroupMembers.Num() >= Group.MaxMembers)
    {
        return false;
    }

    // Check if private and allowed
    if (Group.bIsPrivate && !Group.bAllowInvites)
    {
        return false;
    }

    return true;
}

void USocialAreaComponent::NotifyGroupMembers(const FName& GroupName, const FString& Message)
{
    if (!SocialGroups.Contains(GroupName))
    {
        return;
    }

    // Notify all group members
}

void USocialAreaComponent::PlayEmoteForPlayer(APlayerState* Player, const FName& EmoteName)
{
    // Play emote animation and sound
}

void USocialAreaComponent::PlayGestureForPlayer(APlayerState* Player, const FName& GestureName)
{
    // Play gesture animation and sound
}

void USocialAreaComponent::ProcessSocialEvent(const FName& EventName, float DeltaTime)
{
    // Process event logic
}

void USocialAreaComponent::BroadcastEventMessage(const FName& EventName, const FString& Message)
{
    // Broadcast event message to all players
}

float USocialAreaComponent::CalculateZoneActivity(const FName& ZoneName)
{
    if (!PlayersInZones.Contains(ZoneName))
    {
        return 0.0f;
    }

    return static_cast<float>(PlayersInZones[ZoneName].Players.Num());
}

void USocialAreaComponent::UpdateZonePopulation(const FName& ZoneName)
{
    // Update zone population metrics
}
