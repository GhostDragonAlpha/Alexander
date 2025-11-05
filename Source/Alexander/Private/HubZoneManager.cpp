#include "HubZoneManager.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/TriggerVolume.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UHubZoneManager::UHubZoneManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = DEFAULT_ZONE_UPDATE_INTERVAL;

    // Configuration defaults
    bEnableDebugDrawing = false;
    bEnableZoneAudio = true;
    bEnableZoneLighting = true;
    bEnableEnvironmentEffects = true;
    ZoneUpdateInterval = DEFAULT_ZONE_UPDATE_INTERVAL;
}

void UHubZoneManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeZoneManager();
}

void UHubZoneManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateZoneStates(DeltaTime);
    ProcessZoneTransitions(DeltaTime);
    HandleZoneOverlaps();

    if (bEnableDebugDrawing)
    {
        DrawZoneDebugInfo();
    }
}

void UHubZoneManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(ZoneUpdateTimer);
        World->GetTimerManager().ClearTimer(TransitionTimer);
    }

    // Clear all data
    ZoneDataMap.Empty();
    ZoneVolumes.Empty();
    ZoneEnvironments.Empty();
    ZoneLighting.Empty();
    ZoneAudio.Empty();
    ZoneTransitions.Empty();
    PlayerCurrentZones.Empty();

    Super::EndPlay(EndPlayReason);
}

void UHubZoneManager::InitializeZoneManager()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    SetupZoneTriggers();

    // Setup update timer
    World->GetTimerManager().SetTimer(ZoneUpdateTimer, [this]()
    {
        float DeltaTime = ZoneUpdateInterval;
        UpdateZoneStates(DeltaTime);
    }, ZoneUpdateInterval, true);

    UE_LOG(LogTemp, Log, TEXT("HubZoneManager initialized"));
}

void UHubZoneManager::RegisterZoneVolume(ATriggerVolume* ZoneVolume, const FHubZone& ZoneData)
{
    if (!ZoneVolume)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register null zone volume"));
        return;
    }

    if (ZoneDataMap.Contains(ZoneData.ZoneName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Zone '%s' already registered"), *ZoneData.ZoneName.ToString());
        return;
    }

    ZoneDataMap.Add(ZoneData.ZoneName, ZoneData);
    ZoneVolumes.Add(ZoneData.ZoneName, ZoneVolume);
    ZonePlayers.Add(ZoneData.ZoneName, FZonePlayerList());

    // Setup overlap events
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(ZoneVolume->GetRootComponent()))
    {
        PrimComp->OnComponentBeginOverlap.AddDynamic(this, &UHubZoneManager::OnZoneVolumeBeginOverlap);
        PrimComp->OnComponentEndOverlap.AddDynamic(this, &UHubZoneManager::OnZoneVolumeEndOverlap);
    }

    UE_LOG(LogTemp, Log, TEXT("Zone '%s' registered with volume"), *ZoneData.ZoneName.ToString());
}

void UHubZoneManager::UnregisterZoneVolume(const FName& ZoneName)
{
    ZoneDataMap.Remove(ZoneName);
    ZoneVolumes.Remove(ZoneName);
    ZoneEnvironments.Remove(ZoneName);
    ZoneLighting.Remove(ZoneName);
    ZoneAudio.Remove(ZoneName);
    ZonePlayers.Remove(ZoneName);

    // Remove zone from adjacency lists
    for (auto& AdjPair : ZoneAdjacency)
    {
        AdjPair.Value.AdjacentZones.Remove(ZoneName);
    }
    ZoneAdjacency.Remove(ZoneName);

    UE_LOG(LogTemp, Log, TEXT("Zone '%s' unregistered"), *ZoneName.ToString());
}

void UHubZoneManager::UpdateZoneProperties(const FName& ZoneName, const FHubZone& NewZoneData)
{
    if (ZoneDataMap.Contains(ZoneName))
    {
        ZoneDataMap[ZoneName] = NewZoneData;
        UE_LOG(LogTemp, Log, TEXT("Zone '%s' properties updated"), *ZoneName.ToString());
    }
}

FHubZone* UHubZoneManager::GetZoneData(const FName& ZoneName)
{
    return ZoneDataMap.Find(ZoneName);
}

TArray<FName> UHubZoneManager::GetAllZoneNames() const
{
    TArray<FName> ZoneNames;
    ZoneDataMap.GetKeys(ZoneNames);
    return ZoneNames;
}

void UHubZoneManager::CreateZoneTransition(const FZoneTransition& Transition)
{
    if (!ValidateZoneTransition(Transition))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid zone transition"));
        return;
    }

    ZoneTransitions.Add(Transition);

    // Update adjacency
    if (!ZoneAdjacency.Contains(Transition.FromZone))
    {
        ZoneAdjacency.Add(Transition.FromZone, FZoneAdjacencyList());
    }
    ZoneAdjacency[Transition.FromZone].AdjacentZones.AddUnique(Transition.ToZone);

    UE_LOG(LogTemp, Log, TEXT("Zone transition created from '%s' to '%s'"), 
        *Transition.FromZone.ToString(), *Transition.ToZone.ToString());
}

void UHubZoneManager::RemoveZoneTransition(const FName& FromZone, const FName& ToZone)
{
    for (int32 i = ZoneTransitions.Num() - 1; i >= 0; i--)
    {
        if (ZoneTransitions[i].FromZone == FromZone && ZoneTransitions[i].ToZone == ToZone)
        {
            ZoneTransitions.RemoveAt(i);
        }
    }

    if (ZoneAdjacency.Contains(FromZone))
    {
        ZoneAdjacency[FromZone].AdjacentZones.Remove(ToZone);
    }
}

bool UHubZoneManager::CanTransitionBetweenZones(const FName& FromZone, const FName& ToZone, APlayerState* Player)
{
    if (!Player || !ZoneDataMap.Contains(FromZone) || !ZoneDataMap.Contains(ToZone))
    {
        return false;
    }

    // Check if transition exists
    for (const FZoneTransition& Transition : ZoneTransitions)
    {
        if (Transition.FromZone == FromZone && Transition.ToZone == ToZone)
        {
            if (Transition.bRequiresAccess)
            {
                // Check if player has required access level
                FHubZone* ToZoneData = ZoneDataMap.Find(ToZone);
                if (ToZoneData && ToZoneData->bRequiresAccess)
                {
                    // Check player's access credentials
                    // In a real implementation, check player inventory for access cards/keys
                    // or check player state for permissions

                    // For now, check if player has minimum required items or permissions
                    // This would integrate with an inventory/permission system
                    bool bHasAccessKey = false; // Would check PlayerState or Inventory Component
                    bool bHasPermission = false; // Would check against zone's AllowedPlayers list

                    if (!bHasAccessKey && !bHasPermission)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Player does not have access to zone %s"), *ToZone.ToString());
                        return false;
                    }
                }
            }
            return true;
        }
    }

    return false;
}

void UHubZoneManager::ExecuteZoneTransition(APlayerState* Player, const FName& FromZone, const FName& ToZone)
{
    if (!CanTransitionBetweenZones(FromZone, ToZone, Player))
    {
        return;
    }

    // Find transition
    for (const FZoneTransition& Transition : ZoneTransitions)
    {
        if (Transition.FromZone == FromZone && Transition.ToZone == ToZone)
        {
            StartTransition(Player, Transition);
            return;
        }
    }
}

TArray<FZoneTransition> UHubZoneManager::GetAvailableTransitions(const FName& FromZone)
{
    TArray<FZoneTransition> Available;
    for (const FZoneTransition& Transition : ZoneTransitions)
    {
        if (Transition.FromZone == FromZone)
        {
            Available.Add(Transition);
        }
    }
    return Available;
}

void UHubZoneManager::SetZoneEnvironment(const FName& ZoneName, const FZoneEnvironment& Environment)
{
    if (!ValidateZoneEnvironment(Environment))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid zone environment"));
        return;
    }

    ZoneEnvironments.Add(ZoneName, Environment);
    UE_LOG(LogTemp, Log, TEXT("Zone '%s' environment set"), *ZoneName.ToString());
}

void UHubZoneManager::ApplyZoneEnvironment(APlayerState* Player, const FName& ZoneName)
{
    if (!Player || !ZoneEnvironments.Contains(ZoneName))
    {
        return;
    }

    const FZoneEnvironment& Environment = ZoneEnvironments[ZoneName];

    if (bEnableEnvironmentEffects)
    {
        ApplyGravityChanges(Player, Environment);
        ApplyAtmosphericEffects(Player, Environment);
        
        if (Environment.bHasRadiation)
        {
            ApplyRadiationEffects(Player, Environment);
        }
    }
}

void UHubZoneManager::RemoveZoneEnvironment(APlayerState* Player, const FName& ZoneName)
{
    // Reset environment effects
}

FZoneEnvironment UHubZoneManager::GetZoneEnvironment(const FName& ZoneName)
{
    if (ZoneEnvironments.Contains(ZoneName))
    {
        return ZoneEnvironments[ZoneName];
    }
    return FZoneEnvironment();
}

void UHubZoneManager::SetZoneLighting(const FName& ZoneName, const FZoneLighting& Lighting)
{
    ZoneLighting.Add(ZoneName, Lighting);
}

void UHubZoneManager::ApplyZoneLighting(APlayerState* Player, const FName& ZoneName)
{
    if (!bEnableZoneLighting || !Player || !ZoneLighting.Contains(ZoneName))
    {
        return;
    }

    const FZoneLighting& Lighting = ZoneLighting[ZoneName];
    InterpolateLighting(Player, Lighting, LIGHTING_INTERPOLATION_SPEED);
}

void UHubZoneManager::UpdateZoneLighting(const FName& ZoneName, float DeltaTime)
{
    if (!ZoneLighting.Contains(ZoneName))
    {
        return;
    }

    FZoneLighting& Lighting = ZoneLighting[ZoneName];
    if (Lighting.bDynamicLighting)
    {
        // Update dynamic lighting
    }
}

void UHubZoneManager::SetZoneAudio(const FName& ZoneName, const FZoneAudio& Audio)
{
    ZoneAudio.Add(ZoneName, Audio);
}

void UHubZoneManager::PlayZoneAudio(APlayerState* Player, const FName& ZoneName)
{
    if (!bEnableZoneAudio || !Player || !ZoneAudio.Contains(ZoneName))
    {
        return;
    }

    const FZoneAudio& Audio = ZoneAudio[ZoneName];
    
    // Play background music
    if (Audio.BackgroundMusic)
    {
        // UGameplayStatics::PlaySound2D(GetWorld(), Audio.BackgroundMusic, Audio.MusicVolume);
    }

    // Play ambient sounds
    if (Audio.AmbientSounds)
    {
        // UGameplayStatics::PlaySound2D(GetWorld(), Audio.AmbientSounds, Audio.AmbientVolume);
    }
}

void UHubZoneManager::StopZoneAudio(APlayerState* Player, const FName& ZoneName)
{
    // Stop zone audio
}

void UHubZoneManager::UpdateZoneAudio(const FName& ZoneName, float DeltaTime)
{
    // Update audio state
}

void UHubZoneManager::OnPlayerEnterZone(APlayerState* Player, const FName& ZoneName)
{
    if (!Player || !ZoneDataMap.Contains(ZoneName))
    {
        return;
    }

    // Update player tracking
    PlayerCurrentZones.Add(Player, ZoneName);

    // Add to zone player list
    if (!ZonePlayers.Contains(ZoneName))
    {
        ZonePlayers.Add(ZoneName, FZonePlayerList());
    }
    ZonePlayers[ZoneName].Players.AddUnique(Player);

    // Apply zone effects
    ApplyZoneEnvironment(Player, ZoneName);
    ApplyZoneLighting(Player, ZoneName);
    PlayZoneAudio(Player, ZoneName);

    // Broadcast event
    const FHubZone& ZoneData = ZoneDataMap[ZoneName];
    OnPlayerEnteredZone.Broadcast(Player, ZoneName, ZoneData);

    UE_LOG(LogTemp, Log, TEXT("Player entered zone '%s'"), *ZoneName.ToString());
}

void UHubZoneManager::OnPlayerLeaveZone(APlayerState* Player, const FName& ZoneName)
{
    if (!Player)
    {
        return;
    }

    // Remove from tracking
    PlayerCurrentZones.Remove(Player);

    // Remove from zone player list
    if (ZonePlayers.Contains(ZoneName))
    {
        ZonePlayers[ZoneName].Players.Remove(Player);
    }

    // Remove zone effects
    RemoveZoneEnvironment(Player, ZoneName);
    StopZoneAudio(Player, ZoneName);

    // Broadcast event
    if (ZoneDataMap.Contains(ZoneName))
    {
        const FHubZone& ZoneData = ZoneDataMap[ZoneName];
        OnPlayerLeftZone.Broadcast(Player, ZoneName, ZoneData);
    }

    UE_LOG(LogTemp, Log, TEXT("Player left zone '%s'"), *ZoneName.ToString());
}

void UHubZoneManager::OnZoneStateChanged(const FName& ZoneName, EHubZoneType NewState)
{
    if (FHubZone* Zone = ZoneDataMap.Find(ZoneName))
    {
        Zone->ZoneType = NewState;
    }
}

bool UHubZoneManager::IsPlayerInZone(APlayerState* Player, const FName& ZoneName)
{
    if (!Player)
    {
        return false;
    }

    return PlayerCurrentZones.Contains(Player) && PlayerCurrentZones[Player] == ZoneName;
}

FName UHubZoneManager::GetPlayerCurrentZone(APlayerState* Player)
{
    if (PlayerCurrentZones.Contains(Player))
    {
        return PlayerCurrentZones[Player];
    }
    return NAME_None;
}

TArray<APlayerState*> UHubZoneManager::GetPlayersInZone(const FName& ZoneName)
{
    if (ZonePlayers.Contains(ZoneName))
    {
        return ZonePlayers[ZoneName].Players;
    }
    return TArray<APlayerState*>();
}

float UHubZoneManager::GetDistanceToZone(APlayerState* Player, const FName& ZoneName)
{
    if (!Player || !ZoneDataMap.Contains(ZoneName))
    {
        return -1.0f;
    }

    // TODO: Calculate actual distance
    return 0.0f;
}

bool UHubZoneManager::IsZoneAccessible(const FName& ZoneName, APlayerState* Player)
{
    if (!ZoneDataMap.Contains(ZoneName))
    {
        return false;
    }

    const FHubZone& Zone = ZoneDataMap[ZoneName];
    if (!Zone.bRequiresAccess)
    {
        return true;
    }

    if (!Player)
    {
        return false;
    }

    // Check player access credentials and permissions
    // Method 1: Check zone's allowed players list (if zone has whitelist)
    if (Zone.AllowedPlayers.Num() > 0 && !Zone.AllowedPlayers.Contains(Player))
    {
        UE_LOG(LogTemp, Warning, TEXT("Player %s not in allowed list for zone %s"),
            *Player->GetPlayerName(), *ZoneName.ToString());
        return false;
    }

    // Method 2: Check for required access level or security clearance
    // This would integrate with a player progression/reputation system
    // For now, assume all players have basic access if no specific whitelist exists
    bool bHasSecurityClearance = true; // Placeholder - would check player's reputation/rank

    // Method 3: Check for restricted areas based on mission progress or faction
    bool bIsRestricted = Zone.bIsRestricted; // Zones can be marked as restricted
    if (bIsRestricted && !bHasSecurityClearance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Zone %s is restricted"), *ZoneName.ToString());
        return false;
    }

    return true;
}

void UHubZoneManager::DrawZoneDebugInfo()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (const auto& ZonePair : ZoneDataMap)
    {
        DrawZoneBounds(ZonePair.Key, FColor::Green);
    }

    DrawTransitionPaths();
}

void UHubZoneManager::LogZoneStatistics()
{
    UE_LOG(LogTemp, Log, TEXT("=== Zone Manager Statistics ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Zones: %d"), ZoneDataMap.Num());
    UE_LOG(LogTemp, Log, TEXT("Total Transitions: %d"), ZoneTransitions.Num());
    UE_LOG(LogTemp, Log, TEXT("Players Tracked: %d"), PlayerCurrentZones.Num());

    for (const auto& ZonePair : ZonePlayers)
    {
        UE_LOG(LogTemp, Log, TEXT("Zone '%s': %d players"), 
            *ZonePair.Key.ToString(), ZonePair.Value.Players.Num());
    }
}

bool UHubZoneManager::ValidateZoneConfiguration()
{
    bool bValid = true;

    // Validate all zones
    for (const auto& ZonePair : ZoneDataMap)
    {
        const FHubZone& Zone = ZonePair.Value;
        if (Zone.MaxPlayers <= 0)
        {
            UE_LOG(LogTemp, Error, TEXT("Zone '%s' has invalid MaxPlayers"), *Zone.ZoneName.ToString());
            bValid = false;
        }
    }

    // Validate transitions
    for (const FZoneTransition& Transition : ZoneTransitions)
    {
        if (!ValidateZoneTransition(Transition))
        {
            bValid = false;
        }
    }

    return bValid;
}

void UHubZoneManager::SetupZoneTriggers()
{
    // Triggers are set up when zones are registered
}

void UHubZoneManager::UpdateZoneStates(float DeltaTime)
{
    // Update lighting
    for (const auto& LightPair : ZoneLighting)
    {
        UpdateZoneLighting(LightPair.Key, DeltaTime);
    }

    // Update audio
    for (const auto& AudioPair : ZoneAudio)
    {
        UpdateZoneAudio(AudioPair.Key, DeltaTime);
    }
}

void UHubZoneManager::ProcessZoneTransitions(float DeltaTime)
{
    // Process active transitions
}

void UHubZoneManager::HandleZoneOverlaps()
{
    // Handle ongoing overlaps
}

void UHubZoneManager::OnZoneVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Find which zone this volume belongs to
    FName ZoneName = NAME_None;
    for (const auto& VolumePair : ZoneVolumes)
    {
        if (VolumePair.Value && VolumePair.Value->GetRootComponent() == OverlappedComp)
        {
            ZoneName = VolumePair.Key;
            break;
        }
    }

    if (ZoneName == NAME_None)
    {
        return;
    }

    // Check if it's a player
    if (APawn* Pawn = Cast<APawn>(OtherActor))
    {
        if (APlayerState* PlayerState = Pawn->GetPlayerState())
        {
            OnPlayerEnterZone(PlayerState, ZoneName);
        }
    }
}

void UHubZoneManager::OnZoneVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Find which zone this volume belongs to
    FName ZoneName = NAME_None;
    for (const auto& VolumePair : ZoneVolumes)
    {
        if (VolumePair.Value && VolumePair.Value->GetRootComponent() == OverlappedComp)
        {
            ZoneName = VolumePair.Key;
            break;
        }
    }

    if (ZoneName == NAME_None)
    {
        return;
    }

    // Check if it's a player
    if (APawn* Pawn = Cast<APawn>(OtherActor))
    {
        if (APlayerState* PlayerState = Pawn->GetPlayerState())
        {
            OnPlayerLeaveZone(PlayerState, ZoneName);
        }
    }
}

void UHubZoneManager::StartTransition(APlayerState* Player, const FZoneTransition& Transition)
{
    if (!Player)
    {
        return;
    }

    OnZoneTransitionStarted.Broadcast(Player, Transition);

    // Setup transition timer
    UWorld* World = GetWorld();
    if (World)
    {
        FTimerHandle TransitionHandle;
        World->GetTimerManager().SetTimer(TransitionHandle, [this, Player, Transition]()
        {
            CompleteTransition(Player, Transition);
        }, Transition.TransitionDuration, false);

        ActiveTransitions.Add(Player, TransitionHandle);
    }
}

void UHubZoneManager::CompleteTransition(APlayerState* Player, const FZoneTransition& Transition)
{
    if (!Player)
    {
        return;
    }

    OnPlayerLeaveZone(Player, Transition.FromZone);
    OnPlayerEnterZone(Player, Transition.ToZone);

    OnZoneTransitionCompleted.Broadcast(Player, Transition);

    ActiveTransitions.Remove(Player);
}

void UHubZoneManager::CancelTransition(APlayerState* Player)
{
    if (ActiveTransitions.Contains(Player))
    {
        UWorld* World = GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(ActiveTransitions[Player]);
        }
        ActiveTransitions.Remove(Player);
    }
}

void UHubZoneManager::ApplyGravityChanges(APlayerState* Player, const FZoneEnvironment& Environment)
{
    // Apply gravity scale to player
}

void UHubZoneManager::ApplyAtmosphericEffects(APlayerState* Player, const FZoneEnvironment& Environment)
{
    // Apply atmospheric effects
}

void UHubZoneManager::ApplyRadiationEffects(APlayerState* Player, const FZoneEnvironment& Environment)
{
    // Apply radiation damage/effects
}

void UHubZoneManager::FadeZoneAudio(APlayerState* Player, const FName& ZoneName, float TargetVolume, float Duration)
{
    // Fade audio over duration
}

void UHubZoneManager::InterpolateLighting(APlayerState* Player, const FZoneLighting& TargetLighting, float Duration)
{
    // Interpolate lighting settings
}

bool UHubZoneManager::ValidateZoneTransition(const FZoneTransition& Transition)
{
    if (!ZoneDataMap.Contains(Transition.FromZone))
    {
        UE_LOG(LogTemp, Error, TEXT("Transition has invalid FromZone '%s'"), *Transition.FromZone.ToString());
        return false;
    }

    if (!ZoneDataMap.Contains(Transition.ToZone))
    {
        UE_LOG(LogTemp, Error, TEXT("Transition has invalid ToZone '%s'"), *Transition.ToZone.ToString());
        return false;
    }

    return true;
}

bool UHubZoneManager::ValidateZoneEnvironment(const FZoneEnvironment& Environment)
{
    if (Environment.GravityScale < 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid gravity scale"));
        return false;
    }

    return true;
}

void UHubZoneManager::DrawZoneBounds(const FName& ZoneName, const FColor& Color)
{
    if (!ZoneDataMap.Contains(ZoneName))
    {
        return;
    }

    const FHubZone& Zone = ZoneDataMap[ZoneName];
    UWorld* World = GetWorld();
    if (World)
    {
        DrawDebugBox(World, Zone.ZoneLocation, Zone.ZoneBounds, Color, false, -1.0f, 0, 5.0f);
    }
}

void UHubZoneManager::DrawTransitionPaths()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (const FZoneTransition& Transition : ZoneTransitions)
    {
        if (ZoneDataMap.Contains(Transition.FromZone) && ZoneDataMap.Contains(Transition.ToZone))
        {
            const FHubZone& FromZone = ZoneDataMap[Transition.FromZone];
            const FHubZone& ToZone = ZoneDataMap[Transition.ToZone];
            
            DrawDebugLine(World, FromZone.ZoneLocation, ToZone.ZoneLocation, 
                FColor::Yellow, false, -1.0f, 0, 3.0f);
        }
    }
}
