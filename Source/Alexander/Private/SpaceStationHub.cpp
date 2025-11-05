#include "SpaceStationHub.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ASpaceStationHub::ASpaceStationHub()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create hub bounds
    HubBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("HubBounds"));
    HubBounds->SetupAttachment(RootComponent);
    HubBounds->SetBoxExtent(FVector(5000.0f, 5000.0f, 5000.0f));

    // Default configuration
    HubName = TEXT("SpaceStationHub");
    HubDescription = TEXT("Central hub for player activities");
    MaxPlayersInHub = 100;
    HubRadius = 10000.0f;
    bEnableVoiceChat = true;
    bEnableTrading = true;
    bEnableMissions = true;
    CurrentHubState = EHubState::Active;
}

void ASpaceStationHub::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeHub();
}

void ASpaceStationHub::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateHubEvents(DeltaTime);
    ProcessPlayerInteractions(DeltaTime);
    UpdateZonePopulations();
}

void ASpaceStationHub::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ShutdownHub();
    Super::EndPlay(EndPlayReason);
}

void ASpaceStationHub::InitializeHub()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Initialize system references
    NetworkManager = NewObject<UNetworkManager>(this, TEXT("NetworkManager"));
    EconomySystem = NewObject<UEconomySystem>(this, TEXT("EconomySystem"));
    MissionSystem = NewObject<UMissionSystem>(this, TEXT("MissionSystem"));
    VoiceChatSystem = NewObject<UVoiceChatSystem>(this, TEXT("VoiceChatSystem"));

    // Initialize zones
    InitializeZones();
    SetupZoneTriggers();

    // Setup timers
    World->GetTimerManager().SetTimer(EventUpdateTimer, [this]()
    {
        float DeltaTime = EVENT_UPDATE_INTERVAL;
        UpdateHubEvents(DeltaTime);
    }, EVENT_UPDATE_INTERVAL, true);

    World->GetTimerManager().SetTimer(StatisticsUpdateTimer, [this]()
    {
        UpdateHubStatistics();
    }, STATISTICS_UPDATE_INTERVAL, true);

    if (bEnableTrading)
    {
        World->GetTimerManager().SetTimer(TradingRefreshTimer, [this]()
        {
            RefreshTradingPost();
        }, TRADING_REFRESH_INTERVAL, true);
    }

    if (bEnableMissions)
    {
        World->GetTimerManager().SetTimer(MissionRefreshTimer, [this]()
        {
            RefreshMissionBoard();
        }, MISSION_REFRESH_INTERVAL, true);
    }

    UE_LOG(LogTemp, Log, TEXT("SpaceStationHub '%s' initialized"), *HubName.ToString());
}

void ASpaceStationHub::ShutdownHub()
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(EventUpdateTimer);
        World->GetTimerManager().ClearTimer(StatisticsUpdateTimer);
        World->GetTimerManager().ClearTimer(TradingRefreshTimer);
        World->GetTimerManager().ClearTimer(MissionRefreshTimer);
    }

    // Clear all players
    for (APlayerState* Player : PlayersInHub)
    {
        OnPlayerLeaveHub(Player);
    }

    PlayersInHub.Empty();
    RegisteredZones.Empty();
    PlayersInZones.Empty();
    ActiveEvents.Empty();

    UE_LOG(LogTemp, Log, TEXT("SpaceStationHub '%s' shutdown"), *HubName.ToString());
}

void ASpaceStationHub::RegisterZone(const FHubZone& Zone)
{
    if (RegisteredZones.Contains(Zone.ZoneName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Zone '%s' already registered"), *Zone.ZoneName.ToString());
        return;
    }

    RegisteredZones.Add(Zone.ZoneName, Zone);
    PlayersInZones.Add(Zone.ZoneName, FSpaceStationZonePlayerList());

    CreateZoneTrigger(Zone);

    UE_LOG(LogTemp, Log, TEXT("Zone '%s' registered in hub '%s'"), *Zone.ZoneName.ToString(), *HubName.ToString());
}

void ASpaceStationHub::UnregisterZone(const FName& ZoneName)
{
    if (!RegisteredZones.Contains(ZoneName))
    {
        return;
    }

    // Remove all players from zone
    if (PlayersInZones.Contains(ZoneName))
    {
        FSpaceStationZonePlayerList& ZonePlayerList = PlayersInZones[ZoneName];
        for (APlayerState* Player : ZonePlayerList.Players)
        {
            UnregisterPlayerFromZone(Player, ZoneName);
        }
        PlayersInZones.Remove(ZoneName);
    }

    RegisteredZones.Remove(ZoneName);

    UE_LOG(LogTemp, Log, TEXT("Zone '%s' unregistered from hub '%s'"), *ZoneName.ToString(), *HubName.ToString());
}

FHubZone* ASpaceStationHub::GetZone(const FName& ZoneName)
{
    return RegisteredZones.Find(ZoneName);
}

TArray<FHubZone> ASpaceStationHub::GetZonesByType(EHubZoneType ZoneType)
{
    TArray<FHubZone> Zones;
    for (const auto& ZonePair : RegisteredZones)
    {
        if (ZonePair.Value.ZoneType == ZoneType)
        {
            Zones.Add(ZonePair.Value);
        }
    }
    return Zones;
}

bool ASpaceStationHub::CanPlayerEnterZone(APlayerState* Player, const FName& ZoneName)
{
    if (!Player || !RegisteredZones.Contains(ZoneName))
    {
        return false;
    }

    const FHubZone& Zone = RegisteredZones[ZoneName];

    // Check player count
    if (PlayersInZones.Contains(ZoneName))
    {
        const FSpaceStationZonePlayerList& ZonePlayerList = PlayersInZones[ZoneName];
        if (ZonePlayerList.Players.Num() >= Zone.MaxPlayers)
        {
            return false;
        }
    }

    // Check access requirements
    if (Zone.bRequiresAccess)
    {
        // TODO: Implement faction/access check
        return true;
    }

    return true;
}

void ASpaceStationHub::OnPlayerEnterHub(APlayerState* Player)
{
    if (!Player || PlayersInHub.Contains(Player))
    {
        return;
    }

    PlayersInHub.Add(Player);
    PlayerEntryTimes.Add(Player, GetWorld()->GetTimeSeconds());

    // Enable voice chat if configured
    if (bEnableVoiceChat && VoiceChatSystem)
    {
        // VoiceChatSystem->EnableVoiceForPlayer(Player);
    }

    UE_LOG(LogTemp, Log, TEXT("Player entered hub '%s'"), *HubName.ToString());
}

void ASpaceStationHub::OnPlayerLeaveHub(APlayerState* Player)
{
    if (!Player || !PlayersInHub.Contains(Player))
    {
        return;
    }

    // Remove from all zones
    if (PlayerCurrentZones.Contains(Player))
    {
        FName CurrentZone = PlayerCurrentZones[Player];
        UnregisterPlayerFromZone(Player, CurrentZone);
        PlayerCurrentZones.Remove(Player);
    }

    PlayersInHub.Remove(Player);
    PlayerEntryTimes.Remove(Player);

    // Disable voice chat
    if (bEnableVoiceChat && VoiceChatSystem)
    {
        // VoiceChatSystem->DisableVoiceForPlayer(Player);
    }

    UE_LOG(LogTemp, Log, TEXT("Player left hub '%s'"), *HubName.ToString());
}

void ASpaceStationHub::MovePlayerToZone(APlayerState* Player, const FName& ZoneName)
{
    if (!Player || !CanPlayerEnterZone(Player, ZoneName))
    {
        return;
    }

    // Remove from current zone
    if (PlayerCurrentZones.Contains(Player))
    {
        FName OldZone = PlayerCurrentZones[Player];
        UnregisterPlayerFromZone(Player, OldZone);
    }

    // Add to new zone
    RegisterPlayerInZone(Player, ZoneName);
    PlayerCurrentZones.Add(Player, ZoneName);

    UE_LOG(LogTemp, Log, TEXT("Player moved to zone '%s'"), *ZoneName.ToString());
}

TArray<APlayerState*> ASpaceStationHub::GetPlayersInZone(const FName& ZoneName)
{
    if (PlayersInZones.Contains(ZoneName))
    {
        return PlayersInZones[ZoneName].Players;
    }
    return TArray<APlayerState*>();
}

void ASpaceStationHub::StartHubEvent(const FHubEvent& Event)
{
    ActiveEvents.Add(Event);
    EventStartTimes.Add(Event.EventName, GetWorld()->GetTimeSeconds());

    ApplyEventEffects(Event);
    BroadcastEventUpdate(Event, true);

    UE_LOG(LogTemp, Log, TEXT("Hub event '%s' started"), *Event.EventName.ToString());
}

void ASpaceStationHub::EndHubEvent(const FName& EventName)
{
    for (int32 i = 0; i < ActiveEvents.Num(); i++)
    {
        if (ActiveEvents[i].EventName == EventName)
        {
            RemoveEventEffects(ActiveEvents[i]);
            BroadcastEventUpdate(ActiveEvents[i], false);
            ActiveEvents.RemoveAt(i);
            EventStartTimes.Remove(EventName);
            
            UE_LOG(LogTemp, Log, TEXT("Hub event '%s' ended"), *EventName.ToString());
            return;
        }
    }
}

bool ASpaceStationHub::IsEventActive(const FName& EventName)
{
    for (const FHubEvent& Event : ActiveEvents)
    {
        if (Event.EventName == EventName)
        {
            return true;
        }
    }
    return false;
}

TArray<FHubEvent> ASpaceStationHub::GetActiveEvents()
{
    return ActiveEvents;
}

void ASpaceStationHub::EnableVoiceChatInZone(const FName& ZoneName, bool bEnabled)
{
    if (VoiceChatSystem && RegisteredZones.Contains(ZoneName))
    {
        // VoiceChatSystem->SetZoneVoiceEnabled(ZoneName, bEnabled);
    }
}

void ASpaceStationHub::SetZoneSocialRadius(const FName& ZoneName, float Radius)
{
    if (FHubZone* Zone = RegisteredZones.Find(ZoneName))
    {
        Zone->ZoneProperties.Add(TEXT("SocialRadius"), Radius);
    }
}

TArray<APlayerState*> ASpaceStationHub::GetPlayersInSocialRange(APlayerState* Player, float Range)
{
    TArray<APlayerState*> NearbyPlayers;
    // TODO: Implement spatial query
    return NearbyPlayers;
}

void ASpaceStationHub::RefreshTradingPost()
{
    if (EconomySystem)
    {
        // EconomySystem->RefreshMarket();
    }
}

TArray<FMarketItem> ASpaceStationHub::GetAvailableItems()
{
    TArray<FMarketItem> Items;
    if (EconomySystem)
    {
        // Items = EconomySystem->GetMarketItems();
    }
    return Items;
}

bool ASpaceStationHub::ProcessTrade(APlayerState* Buyer, APlayerState* Seller, const FMarketItem& Item, int32 Quantity)
{
    if (!Buyer || !Seller || !EconomySystem)
    {
        return false;
    }

    // return EconomySystem->ProcessTrade(Buyer, Seller, Item, Quantity);
    return false;
}

void ASpaceStationHub::RefreshMissionBoard()
{
    if (MissionSystem)
    {
        // MissionSystem->RefreshAvailableMissions();
    }
}

TArray<FMissionData> ASpaceStationHub::GetAvailableMissions(APlayerState* Player)
{
    TArray<FMissionData> Missions;
    if (MissionSystem && Player)
    {
        // Missions = MissionSystem->GetMissionsForPlayer(Player);
    }
    return Missions;
}

bool ASpaceStationHub::AcceptMission(APlayerState* Player, const FMissionData& Mission)
{
    if (!Player || !MissionSystem)
    {
        return false;
    }

    // return MissionSystem->AcceptMission(Player, Mission);
    return false;
}

void ASpaceStationHub::SetHubState(EHubState NewState)
{
    if (CurrentHubState == NewState)
    {
        return;
    }

    EHubState OldState = CurrentHubState;
    CurrentHubState = NewState;

    HandleHubStateChange(OldState, NewState);

    UE_LOG(LogTemp, Log, TEXT("Hub '%s' state changed to %d"), *HubName.ToString(), (int32)NewState);
}

int32 ASpaceStationHub::GetZonePlayerCount(const FName& ZoneName)
{
    if (PlayersInZones.Contains(ZoneName))
    {
        return PlayersInZones[ZoneName].Players.Num();
    }
    return 0;
}

float ASpaceStationHub::GetAveragePlayerTimeInHub() const
{
    if (PlayersInHub.Num() == 0)
    {
        return 0.0f;
    }

    float TotalTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();

    for (const auto& EntryPair : PlayerEntryTimes)
    {
        TotalTime += CurrentTime - EntryPair.Value;
    }

    return TotalTime / PlayersInHub.Num();
}

TMap<FName, int32> ASpaceStationHub::GetZonePopulationStats()
{
    TMap<FName, int32> Stats;
    for (const auto& ZonePair : PlayersInZones)
    {
        Stats.Add(ZonePair.Key, ZonePair.Value.Players.Num());
    }
    return Stats;
}

void ASpaceStationHub::InitializeZones()
{
    // Create default zones
    FHubZone SpawnZone;
    SpawnZone.ZoneName = TEXT("Spawn");
    SpawnZone.ZoneType = EHubZoneType::SpawnZone;
    SpawnZone.ZoneLocation = FVector::ZeroVector;
    SpawnZone.ZoneRotation = FRotator::ZeroRotator;
    SpawnZone.ZoneBounds = FVector(1000.0f, 1000.0f, 500.0f);
    SpawnZone.MaxPlayers = MaxPlayersInHub;
    SpawnZone.bRequiresAccess = false;
    RegisterZone(SpawnZone);

    FHubZone SocialZone;
    SocialZone.ZoneName = TEXT("Social");
    SocialZone.ZoneType = EHubZoneType::SocialZone;
    SocialZone.ZoneLocation = FVector(2000.0f, 0.0f, 0.0f);
    SocialZone.ZoneRotation = FRotator::ZeroRotator;
    SocialZone.ZoneBounds = FVector(1500.0f, 1500.0f, 500.0f);
    SocialZone.MaxPlayers = 50;
    SocialZone.bRequiresAccess = false;
    RegisterZone(SocialZone);

    if (bEnableTrading)
    {
        FHubZone TradingZone;
        TradingZone.ZoneName = TEXT("Trading");
        TradingZone.ZoneType = EHubZoneType::TradingZone;
        TradingZone.ZoneLocation = FVector(-2000.0f, 0.0f, 0.0f);
        TradingZone.ZoneRotation = FRotator::ZeroRotator;
        TradingZone.ZoneBounds = FVector(1000.0f, 1000.0f, 500.0f);
        TradingZone.MaxPlayers = 30;
        TradingZone.bRequiresAccess = false;
        RegisterZone(TradingZone);
    }

    if (bEnableMissions)
    {
        FHubZone MissionZone;
        MissionZone.ZoneName = TEXT("Mission");
        MissionZone.ZoneType = EHubZoneType::MissionZone;
        MissionZone.ZoneLocation = FVector(0.0f, 2000.0f, 0.0f);
        MissionZone.ZoneRotation = FRotator::ZeroRotator;
        MissionZone.ZoneBounds = FVector(1000.0f, 1000.0f, 500.0f);
        MissionZone.MaxPlayers = 20;
        MissionZone.bRequiresAccess = false;
        RegisterZone(MissionZone);
    }
}

void ASpaceStationHub::SetupZoneTriggers()
{
    // Zone triggers are created when zones are registered
}

void ASpaceStationHub::UpdateHubEvents(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Check for expired events
    TArray<FName> ExpiredEvents;
    for (const FHubEvent& Event : ActiveEvents)
    {
        if (EventStartTimes.Contains(Event.EventName))
        {
            float StartTime = EventStartTimes[Event.EventName];
            if (CurrentTime - StartTime >= Event.Duration)
            {
                ExpiredEvents.Add(Event.EventName);
            }
        }
    }

    // End expired events
    for (const FName& EventName : ExpiredEvents)
    {
        EndHubEvent(EventName);
    }
}

void ASpaceStationHub::ProcessPlayerInteractions(float DeltaTime)
{
    // Update player positions and interactions
    for (APlayerState* Player : PlayersInHub)
    {
        UpdatePlayerZoneLocation(Player);
    }
}

void ASpaceStationHub::UpdateZonePopulations()
{
    // Zone populations are updated when players move
}

void ASpaceStationHub::HandleHubStateChange(EHubState OldState, EHubState NewState)
{
    switch (NewState)
    {
    case EHubState::Maintenance:
        // Notify players
        break;
    case EHubState::Lockdown:
        // Restrict movement
        break;
    case EHubState::EventMode:
        // Enable special features
        break;
    default:
        break;
    }
}

void ASpaceStationHub::CreateZoneTrigger(const FHubZone& Zone)
{
    // Zone triggers would be created as separate components
}

void ASpaceStationHub::OnZoneTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Handle zone entry
}

void ASpaceStationHub::OnZoneTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Handle zone exit
}

void ASpaceStationHub::ApplyEventEffects(const FHubEvent& Event)
{
    for (const FName& ZoneName : Event.AffectedZones)
    {
        if (FHubZone* Zone = RegisteredZones.Find(ZoneName))
        {
            for (const auto& Modifier : Event.EventModifiers)
            {
                Zone->ZoneProperties.Add(Modifier.Key, Modifier.Value);
            }
        }
    }
}

void ASpaceStationHub::RemoveEventEffects(const FHubEvent& Event)
{
    for (const FName& ZoneName : Event.AffectedZones)
    {
        if (FHubZone* Zone = RegisteredZones.Find(ZoneName))
        {
            for (const auto& Modifier : Event.EventModifiers)
            {
                Zone->ZoneProperties.Remove(Modifier.Key);
            }
        }
    }
}

void ASpaceStationHub::BroadcastEventUpdate(const FHubEvent& Event, bool bStarted)
{
    // Broadcast to players
}

void ASpaceStationHub::RegisterPlayerInZone(APlayerState* Player, const FName& ZoneName)
{
    if (!PlayersInZones.Contains(ZoneName))
    {
        PlayersInZones.Add(ZoneName, FSpaceStationZonePlayerList());
    }

    FSpaceStationZonePlayerList& ZonePlayerList = PlayersInZones[ZoneName];
    if (!ZonePlayerList.Players.Contains(Player))
    {
        ZonePlayerList.Players.Add(Player);
    }
}

void ASpaceStationHub::UnregisterPlayerFromZone(APlayerState* Player, const FName& ZoneName)
{
    if (PlayersInZones.Contains(ZoneName))
    {
        FSpaceStationZonePlayerList& ZonePlayerList = PlayersInZones[ZoneName];
        ZonePlayerList.Players.Remove(Player);
    }
}

void ASpaceStationHub::UpdatePlayerZoneLocation(APlayerState* Player)
{
    // Check which zone the player is currently in
}

void ASpaceStationHub::UpdateHubStatistics()
{
    // Update statistics
}

float ASpaceStationHub::CalculateHubActivityLevel() const
{
    return static_cast<float>(PlayersInHub.Num()) / static_cast<float>(MaxPlayersInHub);
}
