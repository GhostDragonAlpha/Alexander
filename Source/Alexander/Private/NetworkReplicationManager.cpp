// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkReplicationManager.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UNetworkReplicationManager::UNetworkReplicationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.033f; // 30Hz default tick rate
    
    TickRate = 30.0f;
    MaxEntitiesPerFrame = 100;
    bEnableClientPrediction = true;
    bEnableServerMeshing = true;
    MaxRelevancyDistance = 100000.0f; // 100km
    BandwidthLimitBytesPerSecond = 1048576; // 1 MB/s default
    
    NextNetworkID = 1;
    TimeSinceLastStatReset = 0.0f;
    CurrentFrameBytesUsed = 0;
}

void UNetworkReplicationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default server mesh zones if enabled
    if (bEnableServerMeshing)
    {
        // Create initial zone at world origin
        CreateServerZone(FVector::ZeroVector, 100000.0f, TEXT("Primary"));
    }
}

void UNetworkReplicationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Reset per-frame bandwidth counter
    CurrentFrameBytesUsed = 0;
    
    // Process replication queue
    ProcessReplicationQueue(DeltaTime);
    
    // Update network statistics
    UpdateNetworkStatistics(DeltaTime);
    
    // Balance server load if meshing is enabled
    if (bEnableServerMeshing)
    {
        BalanceServerLoad();
    }
    
    // Optimize replication based on bandwidth
    OptimizeReplicationForBandwidth();
}

int32 UNetworkReplicationManager::RegisterEntity(AActor* Actor, EEntityType EntityType, EReplicationPriority Priority)
{
    if (!Actor)
    {
        OnReplicationError.Broadcast(0, TEXT("Cannot register null actor"));
        return 0;
    }
    
    FReplicatedEntityInfo EntityInfo;
    EntityInfo.Actor = Actor;
    EntityInfo.EntityType = EntityType;
    EntityInfo.Priority = Priority;
    EntityInfo.NetworkID = NextNetworkID++;
    EntityInfo.LastReplicatedLocation = Actor->GetActorLocation();
    EntityInfo.LastReplicatedRotation = Actor->GetActorRotation();
    EntityInfo.LastReplicationTime = GetWorld()->GetTimeSeconds();
    
    // Set relevancy radius based on entity type
    switch (EntityType)
    {
    case EEntityType::Player:
    case EEntityType::PlayerShip:
        EntityInfo.RelevancyRadius = MaxRelevancyDistance;
        break;
    case EEntityType::Station:
    case EEntityType::Planet:
        EntityInfo.RelevancyRadius = 500000.0f; // 500km for large objects
        break;
    case EEntityType::Projectile:
    case EEntityType::Effect:
        EntityInfo.RelevancyRadius = 10000.0f; // 10km for small/temporary objects
        break;
    default:
        EntityInfo.RelevancyRadius = 50000.0f; // 50km default
        break;
    }
    
    RegisteredEntities.Add(EntityInfo.NetworkID, EntityInfo);
    
    // Initialize prediction state if enabled
    if (bEnableClientPrediction)
    {
        FClientPredictionState PredictionState;
        PredictionState.PredictedLocation = EntityInfo.LastReplicatedLocation;
        PredictionState.PredictedRotation = EntityInfo.LastReplicatedRotation;
        PredictionStates.Add(EntityInfo.NetworkID, PredictionState);
    }
    
    OnEntityRegistered.Broadcast(EntityInfo.NetworkID);
    Statistics.EntitiesReplicated++;
    
    UE_LOG(LogTemp, Log, TEXT("Registered entity %d: %s"), EntityInfo.NetworkID, *Actor->GetName());
    
    return EntityInfo.NetworkID;
}

bool UNetworkReplicationManager::UnregisterEntity(int32 NetworkID)
{
    if (!RegisteredEntities.Contains(NetworkID))
    {
        return false;
    }
    
    RegisteredEntities.Remove(NetworkID);
    PredictionStates.Remove(NetworkID);
    
    OnEntityUnregistered.Broadcast(NetworkID);
    Statistics.EntitiesReplicated--;
    
    return true;
}

FReplicatedEntityInfo UNetworkReplicationManager::GetEntityInfo(int32 NetworkID) const
{
    const FReplicatedEntityInfo* Info = RegisteredEntities.Find(NetworkID);
    return Info ? *Info : FReplicatedEntityInfo();
}

void UNetworkReplicationManager::SetEntityPriority(int32 NetworkID, EReplicationPriority Priority)
{
    if (FReplicatedEntityInfo* Info = RegisteredEntities.Find(NetworkID))
    {
        Info->Priority = Priority;
    }
}

void UNetworkReplicationManager::SetEntityDormancy(int32 NetworkID, bool bDormant)
{
    if (FReplicatedEntityInfo* Info = RegisteredEntities.Find(NetworkID))
    {
        Info->bIsDormant = bDormant;
    }
}

void UNetworkReplicationManager::ForceReplication(int32 NetworkID)
{
    if (FReplicatedEntityInfo* Info = RegisteredEntities.Find(NetworkID))
    {
        Info->LastReplicationTime = 0.0f; // Force immediate replication
    }
}

bool UNetworkReplicationManager::ShouldReplicateEntity(int32 NetworkID, const FVector& ViewerLocation) const
{
    const FReplicatedEntityInfo* Info = RegisteredEntities.Find(NetworkID);
    if (!Info || Info->bIsDormant || !Info->Actor)
    {
        return false;
    }
    
    // Check relevancy
    if (!IsEntityRelevant(NetworkID, ViewerLocation))
    {
        return false;
    }
    
    // Check if enough time has passed since last replication
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float MinReplicationInterval = 1.0f / TickRate;
    float TimeSinceLastReplication = CurrentTime - Info->LastReplicationTime;
    
    return TimeSinceLastReplication >= MinReplicationInterval;
}

void UNetworkReplicationManager::EnableClientPrediction(int32 NetworkID, bool bEnable)
{
    if (bEnable && !PredictionStates.Contains(NetworkID))
    {
        FClientPredictionState PredictionState;
        const FReplicatedEntityInfo* Info = RegisteredEntities.Find(NetworkID);
        if (Info)
        {
            PredictionState.PredictedLocation = Info->LastReplicatedLocation;
            PredictionState.PredictedRotation = Info->LastReplicatedRotation;
        }
        PredictionStates.Add(NetworkID, PredictionState);
    }
    else if (!bEnable)
    {
        PredictionStates.Remove(NetworkID);
    }
}

FClientPredictionState UNetworkReplicationManager::PredictEntityState(int32 NetworkID, float DeltaTime) const
{
    const FClientPredictionState* State = PredictionStates.Find(NetworkID);
    if (!State)
    {
        return FClientPredictionState();
    }
    
    FClientPredictionState PredictedState = *State;
    
    // Simple dead reckoning prediction
    PredictedState.PredictedLocation += PredictedState.PredictedVelocity * DeltaTime;
    PredictedState.PredictionTime += DeltaTime;
    
    return PredictedState;
}

void UNetworkReplicationManager::ReconcileClientPrediction(int32 NetworkID, const FVector& ServerLocation, const FRotator& ServerRotation)
{
    FClientPredictionState* State = PredictionStates.Find(NetworkID);
    if (!State)
    {
        return;
    }
    
    // Calculate prediction error
    float ErrorDistance = FVector::Dist(State->PredictedLocation, ServerLocation);
    
    // If error is within tolerance, accept the prediction
    if (ErrorDistance <= State->ErrorTolerance)
    {
        State->PredictedLocation = ServerLocation;
        State->PredictedRotation = ServerRotation;
        State->PredictionTime = 0.0f;
        return;
    }
    
    // Error too large, snap to server position with smoothing
    float BlendAlpha = FMath::Clamp(ErrorDistance / 1000.0f, 0.1f, 1.0f);
    State->PredictedLocation = FMath::Lerp(State->PredictedLocation, ServerLocation, BlendAlpha);
    State->PredictedRotation = FMath::Lerp(State->PredictedRotation, ServerRotation, BlendAlpha);
    State->PredictionTime = 0.0f;
}

TArray<int32> UNetworkReplicationManager::GetRelevantEntities(const FVector& ViewerLocation, float MaxDistance) const
{
    TArray<int32> RelevantEntities;
    
    for (const auto& EntityPair : RegisteredEntities)
    {
        if (IsEntityRelevant(EntityPair.Key, ViewerLocation))
        {
            float Distance = FVector::Dist(EntityPair.Value.LastReplicatedLocation, ViewerLocation);
            if (Distance <= MaxDistance)
            {
                RelevantEntities.Add(EntityPair.Key);
            }
        }
    }
    
    return RelevantEntities;
}

void UNetworkReplicationManager::SetRelevancyRadius(int32 NetworkID, float Radius)
{
    if (FReplicatedEntityInfo* Info = RegisteredEntities.Find(NetworkID))
    {
        Info->RelevancyRadius = Radius;
    }
}

bool UNetworkReplicationManager::IsEntityRelevant(int32 NetworkID, const FVector& ViewerLocation) const
{
    const FReplicatedEntityInfo* Info = RegisteredEntities.Find(NetworkID);
    if (!Info || !Info->Actor)
    {
        return false;
    }
    
    float Distance = FVector::Dist(Info->LastReplicatedLocation, ViewerLocation);
    return Distance <= Info->RelevancyRadius;
}

FServerMeshZone UNetworkReplicationManager::CreateServerZone(const FVector& Center, float Radius, const FString& ServerNodeID)
{
    FServerMeshZone Zone;
    Zone.ZoneID = FString::Printf(TEXT("Zone_%s_%d"), *ServerNodeID, ServerZones.Num());
    Zone.ZoneCenter = Center;
    Zone.ZoneRadius = Radius;
    Zone.ServerNodeID = ServerNodeID;
    Zone.bIsActive = true;
    
    ServerZones.Add(Zone.ZoneID, Zone);
    
    UE_LOG(LogTemp, Log, TEXT("Created server mesh zone: %s"), *Zone.ZoneID);
    
    return Zone;
}

void UNetworkReplicationManager::UpdateServerZone(const FString& ZoneID, int32 PlayerCount, int32 EntityCount, float ServerLoad)
{
    if (FServerMeshZone* Zone = ServerZones.Find(ZoneID))
    {
        Zone->PlayerCount = PlayerCount;
        Zone->EntityCount = EntityCount;
        Zone->ServerLoad = ServerLoad;
    }
}

TArray<FServerMeshZone> UNetworkReplicationManager::GetActiveZones() const
{
    TArray<FServerMeshZone> ActiveZones;
    
    for (const auto& ZonePair : ServerZones)
    {
        if (ZonePair.Value.bIsActive)
        {
            ActiveZones.Add(ZonePair.Value);
        }
    }
    
    return ActiveZones;
}

FServerMeshZone UNetworkReplicationManager::GetZoneForLocation(const FVector& Location) const
{
    for (const auto& ZonePair : ServerZones)
    {
        const FServerMeshZone& Zone = ZonePair.Value;
        if (Zone.bIsActive)
        {
            float Distance = FVector::Dist(Zone.ZoneCenter, Location);
            if (Distance <= Zone.ZoneRadius)
            {
                return Zone;
            }
        }
    }
    
    return FServerMeshZone();
}

bool UNetworkReplicationManager::RequestZoneTransfer(int32 NetworkID, const FString& TargetZoneID)
{
    if (!ServerZones.Contains(TargetZoneID))
    {
        OnReplicationError.Broadcast(NetworkID, TEXT("Target zone does not exist"));
        return false;
    }
    
    const FReplicatedEntityInfo* Info = RegisteredEntities.Find(NetworkID);
    if (!Info)
    {
        return false;
    }
    
    // In a real implementation, this would trigger server-to-server transfer
    UE_LOG(LogTemp, Log, TEXT("Zone transfer requested for entity %d to zone %s"), NetworkID, *TargetZoneID);
    
    return true;
}

void UNetworkReplicationManager::SetBandwidthLimit(int32 BytesPerSecond)
{
    BandwidthLimitBytesPerSecond = BytesPerSecond;
}

int32 UNetworkReplicationManager::GetCurrentBandwidthUsage() const
{
    return CurrentFrameBytesUsed;
}

float UNetworkReplicationManager::GetReplicationBudget() const
{
    float FrameTime = 1.0f / TickRate;
    int32 MaxBytesPerFrame = FMath::FloorToInt(BandwidthLimitBytesPerSecond * FrameTime);
    return FMath::Clamp((float)(MaxBytesPerFrame - CurrentFrameBytesUsed) / MaxBytesPerFrame, 0.0f, 1.0f);
}

FAlexanderNetworkStatistics UNetworkReplicationManager::GetNetworkStatistics() const
{
    return Statistics;
}

void UNetworkReplicationManager::ResetStatistics()
{
    Statistics = FAlexanderNetworkStatistics();
    Statistics.EntitiesReplicated = RegisteredEntities.Num();
    TimeSinceLastStatReset = 0.0f;
}

float UNetworkReplicationManager::CalculateReplicationPriority(const FReplicatedEntityInfo& Entity, const FVector& ViewerLocation) const
{
    float Priority = GetPriorityMultiplier(Entity.Priority);
    
    // Distance factor (closer = higher priority)
    float Distance = FVector::Dist(Entity.LastReplicatedLocation, ViewerLocation);
    float DistanceFactor = 1.0f - FMath::Clamp(Distance / Entity.RelevancyRadius, 0.0f, 1.0f);
    Priority *= DistanceFactor;
    
    // Entity type factor
    switch (Entity.EntityType)
    {
    case EEntityType::Player:
    case EEntityType::PlayerShip:
        Priority *= 2.0f;
        break;
    case EEntityType::Projectile:
        Priority *= 1.5f;
        break;
    case EEntityType::Effect:
        Priority *= 0.5f;
        break;
    default:
        break;
    }
    
    return Priority;
}

bool UNetworkReplicationManager::ShouldReplicateThisFrame(const FReplicatedEntityInfo& Entity, float DeltaTime) const
{
    if (Entity.bIsDormant)
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastReplication = CurrentTime - Entity.LastReplicationTime;
    float MinReplicationInterval = 1.0f / TickRate;
    
    // High priority entities replicate more frequently
    MinReplicationInterval /= GetPriorityMultiplier(Entity.Priority);
    
    return TimeSinceLastReplication >= MinReplicationInterval;
}

void UNetworkReplicationManager::UpdateEntityReplication(int32 NetworkID, float DeltaTime)
{
    FReplicatedEntityInfo* Info = RegisteredEntities.Find(NetworkID);
    if (!Info || !Info->Actor)
    {
        return;
    }
    
    // Update last replicated state
    Info->LastReplicatedLocation = Info->Actor->GetActorLocation();
    Info->LastReplicatedRotation = Info->Actor->GetActorRotation();
    Info->LastReplicationTime = GetWorld()->GetTimeSeconds();
    
    // Estimate bandwidth usage (rough approximation)
    int32 EstimatedBytes = 48; // Position (12) + Rotation (12) + Velocity (12) + Metadata (12)
    CurrentFrameBytesUsed += EstimatedBytes;
    Statistics.BytesSent += EstimatedBytes;
}

void UNetworkReplicationManager::ProcessReplicationQueue(float DeltaTime)
{
    int32 EntitiesProcessed = 0;
    
    // Get viewer location (in real implementation, would be each client's view)
    FVector ViewerLocation = FVector::ZeroVector;
    if (GetWorld() && GetWorld()->GetFirstPlayerController())
    {
        ViewerLocation = GetWorld()->GetFirstPlayerController()->GetPawn() ?
            GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation() : FVector::ZeroVector;
    }
    
    // Build priority queue
    TArray<TPair<int32, float>> PriorityQueue;
    for (const auto& EntityPair : RegisteredEntities)
    {
        if (ShouldReplicateThisFrame(EntityPair.Value, DeltaTime))
        {
            float Priority = CalculateReplicationPriority(EntityPair.Value, ViewerLocation);
            PriorityQueue.Add(TPair<int32, float>(EntityPair.Key, Priority));
        }
    }
    
    // Sort by priority
    PriorityQueue.Sort([](const TPair<int32, float>& A, const TPair<int32, float>& B) {
        return A.Value > B.Value;
    });
    
    // Process entities up to bandwidth limit
    for (const auto& QueueEntry : PriorityQueue)
    {
        if (EntitiesProcessed >= MaxEntitiesPerFrame)
        {
            break;
        }
        
        if (!IsWithinBandwidthBudget(48)) // Estimated bytes per entity
        {
            break;
        }
        
        UpdateEntityReplication(QueueEntry.Key, DeltaTime);
        EntitiesProcessed++;
    }
}

float UNetworkReplicationManager::GetPriorityMultiplier(EReplicationPriority Priority) const
{
    switch (Priority)
    {
    case EReplicationPriority::VeryLow:     return 0.5f;
    case EReplicationPriority::Low:         return 0.75f;
    case EReplicationPriority::Normal:      return 1.0f;
    case EReplicationPriority::High:        return 1.5f;
    case EReplicationPriority::VeryHigh:    return 2.0f;
    case EReplicationPriority::Critical:    return 3.0f;
    default:                                 return 1.0f;
    }
}

float UNetworkReplicationManager::GetRelevancyDistance(ERelevancyDistance Distance) const
{
    switch (Distance)
    {
    case ERelevancyDistance::Close:         return 5000.0f;
    case ERelevancyDistance::Medium:        return 15000.0f;
    case ERelevancyDistance::Far:           return 50000.0f;
    case ERelevancyDistance::VeryFar:       return 100000.0f;
    case ERelevancyDistance::Unlimited:     return TNumericLimits<float>::Max();
    default:                                 return 50000.0f;
    }
}

void UNetworkReplicationManager::UpdateNetworkStatistics(float DeltaTime)
{
    TimeSinceLastStatReset += DeltaTime;
    
    // Update active connections (simplified)
    Statistics.ActiveConnections = 1; // Would be actual connection count in multiplayer
    
    // Calculate average latency (would come from actual network measurements)
    Statistics.AverageLatency = 50.0f; // Placeholder
    
    // Calculate packet loss (would come from actual network measurements)
    Statistics.PacketLoss = 0.0f; // Placeholder
}

bool UNetworkReplicationManager::IsWithinBandwidthBudget(int32 EstimatedBytes) const
{
    float FrameTime = 1.0f / TickRate;
    int32 MaxBytesPerFrame = FMath::FloorToInt(BandwidthLimitBytesPerSecond * FrameTime);
    return (CurrentFrameBytesUsed + EstimatedBytes) <= MaxBytesPerFrame;
}

void UNetworkReplicationManager::OptimizeReplicationForBandwidth()
{
    // If approaching bandwidth limit, reduce replication quality
    float Budget = GetReplicationBudget();
    
    if (Budget < 0.2f) // Less than 20% budget remaining
    {
        // Reduce tick rate for low priority entities
        for (auto& EntityPair : RegisteredEntities)
        {
            if (EntityPair.Value.Priority == EReplicationPriority::VeryLow ||
                EntityPair.Value.Priority == EReplicationPriority::Low)
            {
                // Make them dormant temporarily
                EntityPair.Value.bIsDormant = true;
            }
        }
    }
    else if (Budget > 0.8f) // More than 80% budget available
    {
        // Wake up dormant entities
        for (auto& EntityPair : RegisteredEntities)
        {
            EntityPair.Value.bIsDormant = false;
        }
    }
}

FServerMeshZone* UNetworkReplicationManager::FindBestZoneForEntity(const FVector& Location)
{
    FServerMeshZone* BestZone = nullptr;
    float LowestLoad = TNumericLimits<float>::Max();
    
    for (auto& ZonePair : ServerZones)
    {
        FServerMeshZone& Zone = ZonePair.Value;
        if (!Zone.bIsActive)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Zone.ZoneCenter, Location);
        if (Distance <= Zone.ZoneRadius)
        {
            if (Zone.ServerLoad < LowestLoad)
            {
                LowestLoad = Zone.ServerLoad;
                BestZone = &Zone;
            }
        }
    }
    
    return BestZone;
}

void UNetworkReplicationManager::BalanceServerLoad()
{
    // Simple load balancing: move entities from high-load to low-load zones
    const float LoadThreshold = 0.8f; // 80% load
    
    for (auto& ZonePair : ServerZones)
    {
        FServerMeshZone& Zone = ZonePair.Value;
        if (Zone.ServerLoad > LoadThreshold)
        {
            // Find entities in this zone that could be moved
            for (const auto& EntityPair : RegisteredEntities)
            {
                FVector EntityLocation = EntityPair.Value.LastReplicatedLocation;
                float DistanceToZone = FVector::Dist(Zone.ZoneCenter, EntityLocation);
                
                if (DistanceToZone <= Zone.ZoneRadius)
                {
                    // Try to find a better zone
                    FServerMeshZone* BetterZone = FindBestZoneForEntity(EntityLocation);
                    if (BetterZone && BetterZone->ServerLoad < Zone.ServerLoad - 0.2f)
                    {
                        UE_LOG(LogTemp, Log, TEXT("Migrating entity %d to zone %s for load balancing"),
                            EntityPair.Key, *BetterZone->ZoneID);
                        // In real implementation, would trigger entity migration
                    }
                }
            }
        }
    }
}
