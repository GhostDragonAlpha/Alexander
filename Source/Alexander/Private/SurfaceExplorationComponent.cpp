#include "SurfaceExplorationComponent.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

USurfaceExplorationComponent::USurfaceExplorationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize configuration with default values
    ExplorationUpdateInterval = DEFAULT_EXPLORATION_UPDATE_INTERVAL;
    BaseExplorationSpeed = DEFAULT_BASE_EXPLORATION_SPEED;
    ScanRange = DEFAULT_SCAN_RANGE;
    AnalysisDuration = DEFAULT_ANALYSIS_DURATION;
    bEnableAutoMapping = true;
    bEnableEnvironmentalHazards = true;
    bEnableToolRequirements = true;
}

void USurfaceExplorationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeExploration();
}

void USurfaceExplorationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateActiveSessions(DeltaTime);
    ProcessExplorationActions(DeltaTime);
    
    if (bEnableEnvironmentalHazards)
    {
        UpdateEnvironmentalConditions(DeltaTime);
    }
    
    HandleDiscoveries(DeltaTime);
}

void USurfaceExplorationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up all active sessions
    for (auto& SessionPair : ActiveSessions)
    {
        CompleteSession(SessionPair.Key);
    }
    
    ActiveSessions.Empty();
    
    Super::EndPlay(EndPlayReason);
}

// Exploration Management Implementation

void USurfaceExplorationComponent::InitializeExploration()
{
    ExplorationPoints.Empty();
    ExplorationPaths.Empty();
    ActiveSessions.Empty();
    PlayerSessions.Empty();
    PlayerExperience.Empty();
    PlayerLevels.Empty();
    ExploredAreas.Empty();
    MapMarkers.Empty();
    
    // Generate some default exploration points for testing
    GenerateRandomPoints();
}

FName USurfaceExplorationComponent::StartExplorationSession(APlayerState* Explorer, const FVector& StartLocation)
{
    if (!Explorer || !CanPlayerStartSession(Explorer))
    {
        return NAME_None;
    }
    
    FName SessionID = GenerateSessionID();
    
    FExplorationSession NewSession;
    NewSession.SessionID = SessionID;
    NewSession.Explorer = Explorer;
    NewSession.CurrentState = EExplorationState::None;
    NewSession.CurrentLocation = StartLocation;
    NewSession.SessionStartTime = GetWorld()->GetTimeSeconds();
    NewSession.ExplorationTime = 0.0f;
    NewSession.DistanceTraveled = 0.0f;
    NewSession.DiscoveriesMade = 0;
    NewSession.ExperienceGained = 0.0f;
    
    // Initialize with basic tools
    NewSession.AvailableTools.Add(EExplorationTool::Scanner, true);
    NewSession.AvailableTools.Add(EExplorationTool::Camera, true);
    
    ActiveSessions.Add(SessionID, NewSession);
    
    // Track player sessions
    if (!PlayerSessions.Contains(Explorer))
    {
        PlayerSessions.Add(Explorer, FPlayerSessionList());
    }
    PlayerSessions[Explorer].SessionNames.Add(SessionID);
    
    // Initialize exploration area tracking
    ExploredAreas.Add(SessionID, FExploredAreaPoints());
    MapMarkers.Add(SessionID, FMapMarkerData());
    
    OnExplorationSessionStarted.Broadcast(SessionID, Explorer);
    
    return SessionID;
}

void USurfaceExplorationComponent::EndExplorationSession(const FName& SessionID)
{
    if (!ActiveSessions.Contains(SessionID))
    {
        return;
    }
    
    FExplorationSession& Session = ActiveSessions[SessionID];
    
    // Calculate final rewards
    FExplorationReward Rewards = CalculateSessionRewards(SessionID);
    GrantSessionRewards(SessionID);
    
    OnExplorationSessionEnded.Broadcast(SessionID, Rewards);
    
    CompleteSession(SessionID);
}

FExplorationSession USurfaceExplorationComponent::GetExplorationSession(const FName& SessionID)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    return Session ? *Session : FExplorationSession();
}

TArray<FExplorationSession> USurfaceExplorationComponent::GetPlayerSessions(APlayerState* Player)
{
    TArray<FExplorationSession> Sessions;
    
    if (PlayerSessions.Contains(Player))
    {
        for (const FName& SessionID : PlayerSessions[Player].SessionNames)
        {
            if (ActiveSessions.Contains(SessionID))
            {
                Sessions.Add(ActiveSessions[SessionID]);
            }
        }
    }
    
    return Sessions;
}

bool USurfaceExplorationComponent::IsPlayerExploring(APlayerState* Player)
{
    if (!PlayerSessions.Contains(Player))
    {
        return false;
    }
    
    for (const FName& SessionID : PlayerSessions[Player].SessionNames)
    {
        if (ActiveSessions.Contains(SessionID))
        {
            return true;
        }
    }
    
    return false;
}

// Point of Interest Management Implementation

void USurfaceExplorationComponent::AddExplorationPoint(const FExplorationPoint& Point)
{
    if (ValidatePoint(Point))
    {
        ExplorationPoints.Add(Point.PointID, Point);
    }
}

void USurfaceExplorationComponent::RemoveExplorationPoint(const FName& PointID)
{
    ExplorationPoints.Remove(PointID);
}

FExplorationPoint USurfaceExplorationComponent::GetExplorationPoint(const FName& PointID)
{
    FExplorationPoint* Point = ExplorationPoints.Find(PointID);
    return Point ? *Point : FExplorationPoint();
}

TArray<FExplorationPoint> USurfaceExplorationComponent::GetAllExplorationPoints()
{
    TArray<FExplorationPoint> Points;
    ExplorationPoints.GenerateValueArray(Points);
    return Points;
}

TArray<FExplorationPoint> USurfaceExplorationComponent::GetNearbyPoints(const FVector& Location, float Radius)
{
    TArray<FExplorationPoint> NearbyPoints;
    
    for (const auto& PointPair : ExplorationPoints)
    {
        float Distance = FVector::Dist(Location, PointPair.Value.Location);
        if (Distance <= Radius)
        {
            NearbyPoints.Add(PointPair.Value);
        }
    }
    
    return NearbyPoints;
}

bool USurfaceExplorationComponent::DiscoverPoint(const FName& SessionID, const FName& PointID)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    FExplorationPoint* Point = ExplorationPoints.Find(PointID);
    
    if (!Session || !Point || Point->bIsDiscovered)
    {
        return false;
    }
    
    Point->bIsDiscovered = true;
    Session->DiscoveredPoints.Add(PointID);
    Session->DiscoveriesMade++;
    
    UpdatePointDiscovery(PointID, Session->Explorer);
    
    OnPointDiscovered.Broadcast(PointID, Session->Explorer, *Point);
    
    return true;
}

bool USurfaceExplorationComponent::ExplorePoint(const FName& SessionID, const FName& PointID)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    FExplorationPoint* Point = ExplorationPoints.Find(PointID);
    
    if (!Session || !Point || !Point->bIsDiscovered || Point->bIsExplored)
    {
        return false;
    }
    
    // Check if player is close enough
    float Distance = FVector::Dist(Session->CurrentLocation, Point->Location);
    if (Distance > ScanRange)
    {
        return false;
    }
    
    Point->bIsExplored = true;
    Session->ExploredPoints.Add(PointID);
    
    FExplorationReward Reward = GenerateDiscoveryReward(*Point, Session->Explorer);
    Session->ExperienceGained += Reward.ExperiencePoints;
    
    OnPointExplored.Broadcast(PointID, Session->Explorer, Reward);
    
    return true;
}

// Path Management Implementation

void USurfaceExplorationComponent::AddExplorationPath(const FExplorationPath& Path)
{
    if (ValidatePath(Path))
    {
        ExplorationPaths.Add(Path.PathID, Path);
    }
}

void USurfaceExplorationComponent::RemoveExplorationPath(const FName& PathID)
{
    ExplorationPaths.Remove(PathID);
}

FExplorationPath USurfaceExplorationComponent::GetExplorationPath(const FName& PathID)
{
    FExplorationPath* Path = ExplorationPaths.Find(PathID);
    return Path ? *Path : FExplorationPath();
}

TArray<FExplorationPath> USurfaceExplorationComponent::GetAvailablePaths(const FName& FromPoint)
{
    TArray<FExplorationPath> AvailablePaths;
    
    for (const auto& PathPair : ExplorationPaths)
    {
        if (PathPair.Value.StartPoint == FromPoint)
        {
            AvailablePaths.Add(PathPair.Value);
        }
    }
    
    return AvailablePaths;
}

TArray<FVector> USurfaceExplorationComponent::CalculatePath(const FVector& Start, const FVector& End)
{
    return FindOptimalPath(Start, End);
}

float USurfaceExplorationComponent::EstimatePathTime(const FName& PathID, APlayerState* Player)
{
    FExplorationPath* Path = ExplorationPaths.Find(PathID);
    if (!Path)
    {
        return 0.0f;
    }
    
    float SpeedMultiplier = 1.0f;
    
    switch (Path->PathDifficulty)
    {
        case ETerrainDifficulty::Easy: SpeedMultiplier = 1.2f; break;
        case ETerrainDifficulty::Normal: SpeedMultiplier = 1.0f; break;
        case ETerrainDifficulty::Hard: SpeedMultiplier = 0.7f; break;
        case ETerrainDifficulty::Extreme: SpeedMultiplier = 0.4f; break;
        case ETerrainDifficulty::Lethal: SpeedMultiplier = 0.2f; break;
    }
    
    return Path->PathLength / (BaseExplorationSpeed * SpeedMultiplier);
}

// Tool Management Implementation

bool USurfaceExplorationComponent::EquipTool(const FName& SessionID, EExplorationTool Tool)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (!Session)
    {
        return false;
    }
    
    Session->AvailableTools.Add(Tool, true);
    return true;
}

bool USurfaceExplorationComponent::UnequipTool(const FName& SessionID, EExplorationTool Tool)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (!Session)
    {
        return false;
    }
    
    Session->AvailableTools.Remove(Tool);
    return true;
}

bool USurfaceExplorationComponent::HasTool(const FName& SessionID, EExplorationTool Tool)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (!Session)
    {
        return false;
    }
    
    return Session->AvailableTools.Contains(Tool) && Session->AvailableTools[Tool];
}

TArray<EExplorationTool> USurfaceExplorationComponent::GetAvailableTools(const FName& SessionID)
{
    TArray<EExplorationTool> Tools;
    
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (Session)
    {
        Session->AvailableTools.GetKeys(Tools);
    }
    
    return Tools;
}

bool USurfaceExplorationComponent::UseTool(const FName& SessionID, const FVector& TargetLocation)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (!Session)
    {
        return false;
    }
    
    // Basic tool usage implementation
    // This would be expanded based on specific tool functionality
    
    return true;
}

// Exploration Actions Implementation

bool USurfaceExplorationComponent::StartScanning(const FName& SessionID, const FVector& ScanLocation)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (!Session || !HasTool(SessionID, EExplorationTool::Scanner))
    {
        return false;
    }
    
    UpdateSessionState(SessionID, EExplorationState::Scanning);
    
    // Check for nearby points within scan range
    TArray<FExplorationPoint> NearbyPoints = GetNearbyPoints(ScanLocation, ScanRange);
    for (const FExplorationPoint& Point : NearbyPoints)
    {
        if (!Point.bIsDiscovered)
        {
            DiscoverPoint(SessionID, Point.PointID);
        }
    }
    
    return true;
}

bool USurfaceExplorationComponent::StartAnalysis(const FName& SessionID, const FName& PointID)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    FExplorationPoint* Point = ExplorationPoints.Find(PointID);
    
    if (!Session || !Point || !HasTool(SessionID, EExplorationTool::Analyzer))
    {
        return false;
    }
    
    UpdateSessionState(SessionID, EExplorationState::Analyzing);
    Session->CurrentObjective = PointID;
    
    return true;
}

bool USurfaceExplorationComponent::CollectSample(const FName& SessionID, const FName& PointID)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    FExplorationPoint* Point = ExplorationPoints.Find(PointID);
    
    if (!Session || !Point || !HasTool(SessionID, EExplorationTool::SampleCollector))
    {
        return false;
    }
    
    UpdateSessionState(SessionID, EExplorationState::Collecting);
    
    // Add sample to resources
    FName ResourceName = FName(*FString::Printf(TEXT("Sample_%s"), *PointID.ToString()));
    float CurrentAmount = Session->ResourcesCollected.FindRef(ResourceName);
    Session->ResourcesCollected.Add(ResourceName, CurrentAmount + 1.0f);
    
    return true;
}

bool USurfaceExplorationComponent::DocumentDiscovery(const FName& SessionID, const FName& PointID, const FString& Notes)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    FExplorationPoint* Point = ExplorationPoints.Find(PointID);
    
    if (!Session || !Point || !HasTool(SessionID, EExplorationTool::Camera))
    {
        return false;
    }
    
    UpdateSessionState(SessionID, EExplorationState::Documenting);
    Session->SessionNotes.Add(PointID, Notes);
    
    return true;
}

bool USurfaceExplorationComponent::DeployDrone(const FName& SessionID, const FVector& DeployLocation)
{
    return HasTool(SessionID, EExplorationTool::Drone);
}

bool USurfaceExplorationComponent::RecallDrone(const FName& SessionID)
{
    return HasTool(SessionID, EExplorationTool::Drone);
}

// Movement and Navigation Implementation

bool USurfaceExplorationComponent::MoveToLocation(const FName& SessionID, const FVector& TargetLocation)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (!Session || !CanMoveToLocation(SessionID, TargetLocation))
    {
        return false;
    }
    
    UpdateSessionState(SessionID, EExplorationState::Traveling);
    // Movement is processed in ProcessMovement()
    
    return true;
}

bool USurfaceExplorationComponent::FollowPath(const FName& SessionID, const FName& PathID)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    FExplorationPath* Path = ExplorationPaths.Find(PathID);
    
    if (!Session || !Path)
    {
        return false;
    }
    
    UpdateSessionState(SessionID, EExplorationState::Traveling);
    Session->CurrentObjective = PathID;
    
    return true;
}

void USurfaceExplorationComponent::StopMovement(const FName& SessionID)
{
    UpdateSessionState(SessionID, EExplorationState::None);
}

FVector USurfaceExplorationComponent::GetCurrentLocation(const FName& SessionID)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    return Session ? Session->CurrentLocation : FVector::ZeroVector;
}

bool USurfaceExplorationComponent::IsMoving(const FName& SessionID)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    return Session && Session->CurrentState == EExplorationState::Traveling;
}

float USurfaceExplorationComponent::GetDistanceToObjective(const FName& SessionID)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (!Session || Session->CurrentObjective == NAME_None)
    {
        return 0.0f;
    }
    
    FExplorationPoint* Point = ExplorationPoints.Find(Session->CurrentObjective);
    if (Point)
    {
        return FVector::Dist(Session->CurrentLocation, Point->Location);
    }
    
    return 0.0f;
}

// Discovery and Analysis Implementation

FExplorationReward USurfaceExplorationComponent::AnalyzeDiscovery(const FName& SessionID, const FName& PointID)
{
    FExplorationPoint* Point = ExplorationPoints.Find(PointID);
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    
    if (Point && Session)
    {
        return GenerateDiscoveryReward(*Point, Session->Explorer);
    }
    
    return FExplorationReward();
}

bool USurfaceExplorationComponent::IdentifyResource(const FName& SessionID, const FName& PointID)
{
    FExplorationPoint* Point = ExplorationPoints.Find(PointID);
    return Point && Point->DiscoveryType == EDiscoveryType::Resource;
}

bool USurfaceExplorationComponent::ScanForLifeforms(const FName& SessionID, const FVector& ScanLocation)
{
    return HasTool(SessionID, EExplorationTool::Scanner);
}

bool USurfaceExplorationComponent::DetectAnomalies(const FName& SessionID, const FVector& ScanLocation)
{
    return HasTool(SessionID, EExplorationTool::Scanner);
}

TArray<FName> USurfaceExplorationComponent::GetNearbyDiscoveries(const FName& SessionID, float Radius)
{
    TArray<FName> Discoveries;
    
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (!Session)
    {
        return Discoveries;
    }
    
    TArray<FExplorationPoint> NearbyPoints = GetNearbyPoints(Session->CurrentLocation, Radius);
    for (const FExplorationPoint& Point : NearbyPoints)
    {
        if (Point.bIsDiscovered)
        {
            Discoveries.Add(Point.PointID);
        }
    }
    
    return Discoveries;
}

// Environmental Interaction Implementation

bool USurfaceExplorationComponent::CanTraverseTerrain(const FName& SessionID, const FVector& Location)
{
    if (!bEnableEnvironmentalHazards)
    {
        return true;
    }
    
    return IsLocationSafe(Location);
}

ETerrainDifficulty USurfaceExplorationComponent::GetTerrainDifficulty(const FVector& Location)
{
    // This would integrate with actual terrain data
    // For now, return a default value
    return ETerrainDifficulty::Normal;
}

TArray<EEnvironmentalHazard> USurfaceExplorationComponent::GetEnvironmentalHazards(const FVector& Location)
{
    TArray<EEnvironmentalHazard> Hazards;
    
    if (!bEnableEnvironmentalHazards)
    {
        return Hazards;
    }
    
    // Check exploration points for known hazards
    for (const auto& PointPair : ExplorationPoints)
    {
        if (FVector::Dist(Location, PointPair.Value.Location) < 1000.0f)
        {
            Hazards.Append(PointPair.Value.EnvironmentalHazards);
        }
    }
    
    return Hazards;
}

bool USurfaceExplorationComponent::IsLocationSafe(const FVector& Location)
{
    return CheckEnvironmentalSafety(Location);
}

FVector USurfaceExplorationComponent::FindNearestSafeLocation(const FVector& DangerousLocation)
{
    // Simple implementation: search in expanding radius
    float SearchRadius = 100.0f;
    const float MaxSearchRadius = 5000.0f;
    const float RadiusIncrement = 100.0f;
    
    while (SearchRadius < MaxSearchRadius)
    {
        for (int32 i = 0; i < 8; i++)
        {
            float Angle = i * 45.0f;
            FVector TestLocation = DangerousLocation + FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)) * SearchRadius,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * SearchRadius,
                0.0f
            );
            
            if (IsLocationSafe(TestLocation))
            {
                return TestLocation;
            }
        }
        
        SearchRadius += RadiusIncrement;
    }
    
    return DangerousLocation;
}

// Progress and Rewards Implementation

void USurfaceExplorationComponent::UpdateExplorationProgress(const FName& SessionID, float DeltaTime)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (!Session)
    {
        return;
    }
    
    Session->ExplorationTime += DeltaTime;
    UpdateExplorationStats(SessionID);
}

float USurfaceExplorationComponent::GetExplorationProgress(const FName& SessionID)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (!Session)
    {
        return 0.0f;
    }
    
    // Calculate progress based on discovered vs total points
    int32 TotalPoints = ExplorationPoints.Num();
    if (TotalPoints == 0)
    {
        return 0.0f;
    }
    
    return (float)Session->DiscoveredPoints.Num() / (float)TotalPoints;
}

FExplorationReward USurfaceExplorationComponent::CalculateSessionRewards(const FName& SessionID)
{
    FExplorationReward Reward;
    
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (!Session)
    {
        return Reward;
    }
    
    // Base experience from discoveries
    Reward.ExperiencePoints = Session->DiscoveriesMade * 100;
    
    // Distance traveled bonus
    Reward.ExperiencePoints += FMath::FloorToInt(Session->DistanceTraveled / 100.0f * EXPERIENCE_PER_KILOMETER);
    
    // Credits based on discoveries
    Reward.Credits = Session->DiscoveriesMade * 50;
    
    // Add discovered resources
    for (const auto& ResourcePair : Session->ResourcesCollected)
    {
        Reward.Resources.Add(ResourcePair.Key, FMath::FloorToInt(ResourcePair.Value));
    }
    
    return Reward;
}

void USurfaceExplorationComponent::GrantSessionRewards(const FName& SessionID)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (!Session || !Session->Explorer)
    {
        return;
    }
    
    FExplorationReward Rewards = CalculateSessionRewards(SessionID);
    
    // Add experience to player
    float CurrentXP = PlayerExperience.FindRef(Session->Explorer);
    PlayerExperience.Add(Session->Explorer, CurrentXP + Rewards.ExperiencePoints);
    
    UpdatePlayerLevel(Session->Explorer);
}

int32 USurfaceExplorationComponent::GetPlayerExplorationLevel(APlayerState* Player)
{
    return PlayerLevels.FindRef(Player);
}

float USurfaceExplorationComponent::GetPlayerExplorationExperience(APlayerState* Player)
{
    return PlayerExperience.FindRef(Player);
}

// Mapping and Cartography Implementation

void USurfaceExplorationComponent::UpdatePlayerMap(const FName& SessionID, const FVector& Location, float Radius)
{
    if (!bEnableAutoMapping)
    {
        return;
    }
    
    if (!ExploredAreas.Contains(SessionID))
    {
        ExploredAreas.Add(SessionID, FExploredAreaPoints());
    }
    
    ExploredAreas[SessionID].Points.Add(Location);
}

TArray<FVector> USurfaceExplorationComponent::GetExploredArea(const FName& SessionID)
{
    if (ExploredAreas.Contains(SessionID))
    {
        return ExploredAreas[SessionID].Points;
    }
    
    return TArray<FVector>();
}

float USurfaceExplorationComponent::GetExploredPercentage(const FName& SessionID, const FVector& Center, float Radius)
{
    // Simplified calculation
    if (!ExploredAreas.Contains(SessionID))
    {
        return 0.0f;
    }
    
    int32 PointsInRadius = 0;
    for (const FVector& Point : ExploredAreas[SessionID].Points)
    {
        if (FVector::Dist(Point, Center) <= Radius)
        {
            PointsInRadius++;
        }
    }
    
    // Assume 100 points represents 100% coverage
    return FMath::Clamp((float)PointsInRadius / 100.0f, 0.0f, 1.0f);
}

bool USurfaceExplorationComponent::IsAreaExplored(const FName& SessionID, const FVector& Location, float Radius)
{
    if (!ExploredAreas.Contains(SessionID))
    {
        return false;
    }
    
    for (const FVector& Point : ExploredAreas[SessionID].Points)
    {
        if (FVector::Dist(Point, Location) <= Radius)
        {
            return true;
        }
    }
    
    return false;
}

void USurfaceExplorationComponent::CreateMapMarker(const FName& SessionID, const FVector& Location, const FName& MarkerType, const FString& Label)
{
    if (!MapMarkers.Contains(SessionID))
    {
        MapMarkers.Add(SessionID, FMapMarkerData());
    }
    
    MapMarkers[SessionID].Markers.Add(Location, MarkerType);
}

// Utilities Implementation

float USurfaceExplorationComponent::CalculateExplorationTime(const FVector& Start, const FVector& End, ETerrainDifficulty Difficulty)
{
    float Distance = FVector::Dist(Start, End);
    float SpeedMultiplier = 1.0f;
    
    switch (Difficulty)
    {
        case ETerrainDifficulty::Easy: SpeedMultiplier = 1.2f; break;
        case ETerrainDifficulty::Normal: SpeedMultiplier = 1.0f; break;
        case ETerrainDifficulty::Hard: SpeedMultiplier = 0.7f; break;
        case ETerrainDifficulty::Extreme: SpeedMultiplier = 0.4f; break;
        case ETerrainDifficulty::Lethal: SpeedMultiplier = 0.2f; break;
    }
    
    return Distance / (BaseExplorationSpeed * SpeedMultiplier);
}

bool USurfaceExplorationComponent::CanPlayerExplore(APlayerState* Player, ETerrainDifficulty Difficulty)
{
    int32 PlayerLevel = GetPlayerExplorationLevel(Player);
    
    switch (Difficulty)
    {
        case ETerrainDifficulty::Easy: return true;
        case ETerrainDifficulty::Normal: return PlayerLevel >= 1;
        case ETerrainDifficulty::Hard: return PlayerLevel >= 3;
        case ETerrainDifficulty::Extreme: return PlayerLevel >= 5;
        case ETerrainDifficulty::Lethal: return PlayerLevel >= 10;
        default: return false;
    }
}

TArray<EExplorationTool> USurfaceExplorationComponent::GetRequiredTools(const FName& PointID)
{
    TArray<EExplorationTool> Tools;
    
    FExplorationPoint* Point = ExplorationPoints.Find(PointID);
    if (Point && bEnableToolRequirements)
    {
        // Convert FName tool requirements to EExplorationTool
        // This is a simplified implementation
        if (Point->RequiredTools.Num() > 0)
        {
            Tools.Add(EExplorationTool::Scanner);
        }
    }
    
    return Tools;
}

float USurfaceExplorationComponent::GetDiscoveryValue(const FName& PointID)
{
    FExplorationPoint* Point = ExplorationPoints.Find(PointID);
    return Point ? Point->DiscoveryValue : 0.0f;
}

void USurfaceExplorationComponent::GenerateExplorationReport(const FName& SessionID)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (!Session)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== Exploration Report for Session: %s ==="), *SessionID.ToString());
    UE_LOG(LogTemp, Log, TEXT("Duration: %.2f seconds"), Session->ExplorationTime);
    UE_LOG(LogTemp, Log, TEXT("Distance Traveled: %.2f units"), Session->DistanceTraveled);
    UE_LOG(LogTemp, Log, TEXT("Discoveries Made: %d"), Session->DiscoveriesMade);
    UE_LOG(LogTemp, Log, TEXT("Experience Gained: %.2f"), Session->ExperienceGained);
    UE_LOG(LogTemp, Log, TEXT("=========================================="));
}

// Private/Internal Implementation

void USurfaceExplorationComponent::UpdateActiveSessions(float DeltaTime)
{
    for (auto& SessionPair : ActiveSessions)
    {
        FExplorationSession& Session = SessionPair.Value;
        Session.ExplorationTime += DeltaTime;
        
        UpdateExplorationProgress(SessionPair.Key, DeltaTime);
        
        if (bEnableAutoMapping)
        {
            UpdateAutoMapping(SessionPair.Key, DeltaTime);
        }
    }
}

void USurfaceExplorationComponent::ProcessExplorationActions(float DeltaTime)
{
    for (auto& SessionPair : ActiveSessions)
    {
        FExplorationSession& Session = SessionPair.Value;
        
        switch (Session.CurrentState)
        {
            case EExplorationState::Scanning:
                ProcessScanning(SessionPair.Key, DeltaTime);
                break;
            case EExplorationState::Traveling:
                ProcessMovement(SessionPair.Key, DeltaTime);
                break;
            case EExplorationState::Analyzing:
                ProcessAnalysis(SessionPair.Key, DeltaTime);
                break;
            case EExplorationState::Collecting:
                ProcessCollection(SessionPair.Key, DeltaTime);
                break;
            default:
                break;
        }
    }
}

void USurfaceExplorationComponent::UpdateEnvironmentalConditions(float DeltaTime)
{
    for (auto& SessionPair : ActiveSessions)
    {
        ProcessEnvironmentalHazards(SessionPair.Key, DeltaTime);
    }
}

void USurfaceExplorationComponent::HandleDiscoveries(float DeltaTime)
{
    // Auto-discover nearby points
    for (auto& SessionPair : ActiveSessions)
    {
        FExplorationSession& Session = SessionPair.Value;
        
        TArray<FExplorationPoint> NearbyPoints = GetNearbyPoints(Session.CurrentLocation, ScanRange * 0.5f);
        for (const FExplorationPoint& Point : NearbyPoints)
        {
            if (!Point.bIsDiscovered)
            {
                DiscoverPoint(SessionPair.Key, Point.PointID);
            }
        }
    }
}

FName USurfaceExplorationComponent::GenerateSessionID()
{
    static int32 SessionCounter = 0;
    SessionCounter++;
    
    return FName(*FString::Printf(TEXT("Session_%d_%f"), SessionCounter, GetWorld()->GetTimeSeconds()));
}

void USurfaceExplorationComponent::UpdateSessionState(const FName& SessionID, EExplorationState NewState)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (Session)
    {
        Session->CurrentState = NewState;
    }
}

void USurfaceExplorationComponent::UpdateSessionLocation(const FName& SessionID, const FVector& NewLocation)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (Session)
    {
        float Distance = FVector::Dist(Session->CurrentLocation, NewLocation);
        Session->DistanceTraveled += Distance;
        Session->CurrentLocation = NewLocation;
    }
}

void USurfaceExplorationComponent::CompleteSession(const FName& SessionID)
{
    ActiveSessions.Remove(SessionID);
    ExploredAreas.Remove(SessionID);
    MapMarkers.Remove(SessionID);
}

void USurfaceExplorationComponent::GenerateRandomPoints()
{
    // Generate some sample exploration points for testing
    for (int32 i = 0; i < 10; i++)
    {
        FExplorationPoint Point;
        Point.PointID = FName(*FString::Printf(TEXT("Point_%d"), i));
        Point.Location = FVector(
            FMath::FRandRange(-10000.0f, 10000.0f),
            FMath::FRandRange(-10000.0f, 10000.0f),
            FMath::FRandRange(0.0f, 1000.0f)
        );
        Point.Description = FString::Printf(TEXT("Exploration Point %d"), i);
        Point.DiscoveryType = static_cast<EDiscoveryType>(FMath::RandRange(0, 9));
        Point.TerrainDifficulty = static_cast<ETerrainDifficulty>(FMath::RandRange(0, 4));
        Point.bIsDiscovered = false;
        Point.bIsExplored = false;
        Point.DiscoveryValue = FMath::FRandRange(50.0f, 500.0f);
        
        ExplorationPoints.Add(Point.PointID, Point);
    }
}

bool USurfaceExplorationComponent::ValidatePoint(const FExplorationPoint& Point)
{
    return Point.PointID != NAME_None && !ExplorationPoints.Contains(Point.PointID);
}

void USurfaceExplorationComponent::UpdatePointDiscovery(const FName& PointID, APlayerState* Discoverer)
{
    // Additional processing when a point is discovered
    CalculateExperienceGain(NAME_None); // Generic XP calculation
}

void USurfaceExplorationComponent::CalculatePathDifficulty(FExplorationPath& Path)
{
    // Simple implementation based on distance and waypoints
    if (Path.PathLength > 5000.0f)
    {
        Path.PathDifficulty = ETerrainDifficulty::Hard;
    }
    else if (Path.PathLength > 2000.0f)
    {
        Path.PathDifficulty = ETerrainDifficulty::Normal;
    }
    else
    {
        Path.PathDifficulty = ETerrainDifficulty::Easy;
    }
}

bool USurfaceExplorationComponent::ValidatePath(const FExplorationPath& Path)
{
    return Path.PathID != NAME_None && !ExplorationPaths.Contains(Path.PathID);
}

TArray<FVector> USurfaceExplorationComponent::FindOptimalPath(const FVector& Start, const FVector& End)
{
    TArray<FVector> Path;
    Path.Add(Start);
    Path.Add(End);
    return Path;
}

bool USurfaceExplorationComponent::CanUseTool(const FName& SessionID, EExplorationTool Tool)
{
    return HasTool(SessionID, Tool);
}

void USurfaceExplorationComponent::ProcessToolUsage(const FName& SessionID, EExplorationTool Tool, const FVector& Target)
{
    OnToolUsed.Broadcast(SessionID, Tool);
}

float USurfaceExplorationComponent::GetToolEfficiency(EExplorationTool Tool)
{
    return 1.0f;
}

void USurfaceExplorationComponent::ProcessMovement(const FName& SessionID, float DeltaTime)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (!Session || Session->CurrentObjective == NAME_None)
    {
        return;
    }
    
    FExplorationPoint* TargetPoint = ExplorationPoints.Find(Session->CurrentObjective);
    if (!TargetPoint)
    {
        return;
    }
    
    FVector Direction = (TargetPoint->Location - Session->CurrentLocation).GetSafeNormal();
    float MovementSpeed = CalculateMovementSpeed(SessionID, TargetPoint->TerrainDifficulty);
    FVector NewLocation = Session->CurrentLocation + Direction * MovementSpeed * DeltaTime;
    
    UpdateSessionLocation(SessionID, NewLocation);
    
    // Check if reached destination
    if (FVector::Dist(NewLocation, TargetPoint->Location) < 100.0f)
    {
        UpdateSessionState(SessionID, EExplorationState::None);
        Session->CurrentObjective = NAME_None;
    }
}

bool USurfaceExplorationComponent::CanMoveToLocation(const FName& SessionID, const FVector& Location)
{
    return CanTraverseTerrain(SessionID, Location);
}

float USurfaceExplorationComponent::CalculateMovementSpeed(const FName& SessionID, ETerrainDifficulty Terrain)
{
    float BaseSpeed = BaseExplorationSpeed;
    
    switch (Terrain)
    {
        case ETerrainDifficulty::Easy: return BaseSpeed * 1.2f;
        case ETerrainDifficulty::Normal: return BaseSpeed;
        case ETerrainDifficulty::Hard: return BaseSpeed * 0.7f;
        case ETerrainDifficulty::Extreme: return BaseSpeed * 0.4f;
        case ETerrainDifficulty::Lethal: return BaseSpeed * 0.2f;
        default: return BaseSpeed;
    }
}

void USurfaceExplorationComponent::ProcessScanning(const FName& SessionID, float DeltaTime)
{
    // Scanning is handled in StartScanning
    // This could be extended for continuous scanning
}

void USurfaceExplorationComponent::ProcessAnalysis(const FName& SessionID, float DeltaTime)
{
    // Analysis processing over time
}

void USurfaceExplorationComponent::ProcessCollection(const FName& SessionID, float DeltaTime)
{
    // Collection processing over time
}

FExplorationReward USurfaceExplorationComponent::GenerateDiscoveryReward(const FExplorationPoint& Point, APlayerState* Discoverer)
{
    FExplorationReward Reward;
    
    Reward.ExperiencePoints = FMath::FloorToInt(Point.DiscoveryValue);
    Reward.Credits = FMath::FloorToInt(Point.DiscoveryValue * 0.5f);
    
    // Add discovery to list
    Reward.Discoveries.Add(Point.PointID);
    
    return Reward;
}

void USurfaceExplorationComponent::ApplyEnvironmentalEffects(const FName& SessionID, float DeltaTime)
{
    // Apply environmental effects to session
}

bool USurfaceExplorationComponent::CheckEnvironmentalSafety(const FVector& Location)
{
    if (!bEnableEnvironmentalHazards)
    {
        return true;
    }
    
    TArray<EEnvironmentalHazard> Hazards = GetEnvironmentalHazards(Location);
    return Hazards.Num() == 0;
}

void USurfaceExplorationComponent::ProcessEnvironmentalHazards(const FName& SessionID, float DeltaTime)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (!Session)
    {
        return;
    }
    
    TArray<EEnvironmentalHazard> Hazards = GetEnvironmentalHazards(Session->CurrentLocation);
    for (EEnvironmentalHazard Hazard : Hazards)
    {
        OnEnvironmentalHazard.Broadcast(SessionID, Hazard);
    }
}

void USurfaceExplorationComponent::UpdateExplorationStats(const FName& SessionID)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (Session)
    {
        CalculateExperienceGain(SessionID);
    }
}

void USurfaceExplorationComponent::CalculateExperienceGain(const FName& SessionID)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (!Session)
    {
        return;
    }
    
    // Distance-based XP
    float DistanceXP = Session->DistanceTraveled / 1000.0f * EXPERIENCE_PER_KILOMETER;
    Session->ExperienceGained = DistanceXP + (Session->DiscoveriesMade * EXPERIENCE_PER_DISCOVERY);
}

void USurfaceExplorationComponent::UpdatePlayerLevel(APlayerState* Player)
{
    if (!Player)
    {
        return;
    }
    
    float Experience = PlayerExperience.FindRef(Player);
    int32 NewLevel = FMath::FloorToInt(Experience / 1000.0f) + 1;
    
    int32 CurrentLevel = PlayerLevels.FindRef(Player);
    if (NewLevel > CurrentLevel)
    {
        PlayerLevels.Add(Player, NewLevel);
        UE_LOG(LogTemp, Log, TEXT("Player reached exploration level %d!"), NewLevel);
    }
}

void USurfaceExplorationComponent::UpdateAutoMapping(const FName& SessionID, float DeltaTime)
{
    FExplorationSession* Session = ActiveSessions.Find(SessionID);
    if (Session)
    {
        UpdatePlayerMap(SessionID, Session->CurrentLocation, 500.0f);
    }
}

void USurfaceExplorationComponent::ProcessMapMarkers(const FName& SessionID)
{
    // Process and update map markers
}

void USurfaceExplorationComponent::OptimizeExploredAreas(const FName& SessionID)
{
    // Optimize explored area data for performance
}

bool USurfaceExplorationComponent::ValidateSession(const FExplorationSession& Session)
{
    return Session.SessionID != NAME_None && Session.Explorer != nullptr;
}

bool USurfaceExplorationComponent::CanPlayerStartSession(APlayerState* Player)
{
    if (!Player)
    {
        return false;
    }
    
    // Check if player already has too many active sessions
    TArray<FExplorationSession> PlayerActiveSessions = GetPlayerSessions(Player);
    return PlayerActiveSessions.Num() < MAX_SESSIONS_PER_PLAYER;
}
