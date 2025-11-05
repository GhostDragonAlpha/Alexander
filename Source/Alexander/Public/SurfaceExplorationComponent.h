#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/PlayerState.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "SurfaceExplorationComponent.generated.h"

UENUM(BlueprintType)
enum class EExplorationState : uint8
{
    None,
    Scanning,
    Traveling,
    Analyzing,
    Collecting,
    Documenting,
    Returning
};

UENUM(BlueprintType)
enum class ETerrainDifficulty : uint8
{
    Easy,
    Normal,
    Hard,
    Extreme,
    Lethal
};

UENUM(BlueprintType)
enum class EDiscoveryType : uint8
{
    Resource,
    Artifact,
    Ruin,
    Cave,
    Lifeform,
    Anomaly,
    Mineral,
    Plant,
    Technology,
    Hazard
};

UENUM(BlueprintType)
enum class EEnvironmentalHazard : uint8
{
    None,
    Radiation,
    Temperature,
    Toxicity,
    LowOxygen,
    HighPressure,
    LowPressure,
    Corrosion,
    ElectroMagnetic,
    Gravity,
    Storm,
    Volcanic,
    Seismic,
    Debris,
    Wildlife
};

UENUM(BlueprintType)
enum class EExplorationTool : uint8
{
    None,
    Scanner,
    Drill,
    SampleCollector,
    Analyzer,
    Camera,
    Drone,
    Rover,
    Submersible,
    ClimbingGear,
    RadiationSuit
};

USTRUCT(BlueprintType)
struct FExplorationPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PointID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDiscoveryType DiscoveryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETerrainDifficulty TerrainDifficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsDiscovered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsExplored;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DiscoveryValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FString> PointProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RequiredTools;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EEnvironmentalHazard> EnvironmentalHazards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* PointIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UParticleSystem* DiscoveryEffect;
};

USTRUCT(BlueprintType)
struct FExplorationPath
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PathID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName StartPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EndPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PathLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETerrainDifficulty PathDifficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EstimatedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EExplorationTool, int32> RequiredTools;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> PathHazards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPathClear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PathSafety;
};

USTRUCT(BlueprintType)
struct FExplorationSession
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SessionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APlayerState* Explorer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EExplorationState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CurrentLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentObjective;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> DiscoveredPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> ExploredPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EExplorationTool, bool> AvailableTools;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SessionStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExplorationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceTraveled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DiscoveriesMade;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> ResourcesCollected;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExperienceGained;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FString> SessionNotes;
};

USTRUCT(BlueprintType)
struct FExplorationReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Credits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, int32> Resources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> Discoveries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> Reputation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlockItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RewardDescription;
};

/**
 * Wrapper for player session names
 */
USTRUCT()
struct FPlayerSessionList
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FName> SessionNames;
};

/**
 * Wrapper for explored area points
 */
USTRUCT()
struct FExploredAreaPoints
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FVector> Points;
};

/**
 * Wrapper for map markers
 */
USTRUCT()
struct FMapMarkerData
{
    GENERATED_BODY()

    UPROPERTY()
    TMap<FVector, FName> Markers;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API USurfaceExplorationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurfaceExplorationComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Exploration Management
    UFUNCTION(BlueprintCallable, Category = "Exploration Management")
    void InitializeExploration();

    UFUNCTION(BlueprintCallable, Category = "Exploration Management")
    FName StartExplorationSession(APlayerState* Explorer, const FVector& StartLocation);

    UFUNCTION(BlueprintCallable, Category = "Exploration Management")
    void EndExplorationSession(const FName& SessionID);

    UFUNCTION(BlueprintCallable, Category = "Exploration Management")
    FExplorationSession GetExplorationSession(const FName& SessionID);

    UFUNCTION(BlueprintCallable, Category = "Exploration Management")
    TArray<FExplorationSession> GetPlayerSessions(APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Exploration Management")
    bool IsPlayerExploring(APlayerState* Player);

    // Point of Interest Management
    UFUNCTION(BlueprintCallable, Category = "Points of Interest")
    void AddExplorationPoint(const FExplorationPoint& Point);

    UFUNCTION(BlueprintCallable, Category = "Points of Interest")
    void RemoveExplorationPoint(const FName& PointID);

    UFUNCTION(BlueprintCallable, Category = "Points of Interest")
    FExplorationPoint GetExplorationPoint(const FName& PointID);

    UFUNCTION(BlueprintCallable, Category = "Points of Interest")
    TArray<FExplorationPoint> GetAllExplorationPoints();

    UFUNCTION(BlueprintCallable, Category = "Points of Interest")
    TArray<FExplorationPoint> GetNearbyPoints(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Points of Interest")
    bool DiscoverPoint(const FName& SessionID, const FName& PointID);

    UFUNCTION(BlueprintCallable, Category = "Points of Interest")
    bool ExplorePoint(const FName& SessionID, const FName& PointID);

    // Path Management
    UFUNCTION(BlueprintCallable, Category = "Path Management")
    void AddExplorationPath(const FExplorationPath& Path);

    UFUNCTION(BlueprintCallable, Category = "Path Management")
    void RemoveExplorationPath(const FName& PathID);

    UFUNCTION(BlueprintCallable, Category = "Path Management")
    FExplorationPath GetExplorationPath(const FName& PathID);

    UFUNCTION(BlueprintCallable, Category = "Path Management")
    TArray<FExplorationPath> GetAvailablePaths(const FName& FromPoint);

    UFUNCTION(BlueprintCallable, Category = "Path Management")
    TArray<FVector> CalculatePath(const FVector& Start, const FVector& End);

    UFUNCTION(BlueprintCallable, Category = "Path Management")
    float EstimatePathTime(const FName& PathID, APlayerState* Player);

    // Tool Management
    UFUNCTION(BlueprintCallable, Category = "Tool Management")
    bool EquipTool(const FName& SessionID, EExplorationTool Tool);

    UFUNCTION(BlueprintCallable, Category = "Tool Management")
    bool UnequipTool(const FName& SessionID, EExplorationTool Tool);

    UFUNCTION(BlueprintCallable, Category = "Tool Management")
    bool HasTool(const FName& SessionID, EExplorationTool Tool);

    UFUNCTION(BlueprintCallable, Category = "Tool Management")
    TArray<EExplorationTool> GetAvailableTools(const FName& SessionID);

    UFUNCTION(BlueprintCallable, Category = "Tool Management")
    bool UseTool(const FName& SessionID, const FVector& TargetLocation);

    // Exploration Actions
    UFUNCTION(BlueprintCallable, Category = "Exploration Actions")
    bool StartScanning(const FName& SessionID, const FVector& ScanLocation);

    UFUNCTION(BlueprintCallable, Category = "Exploration Actions")
    bool StartAnalysis(const FName& SessionID, const FName& PointID);

    UFUNCTION(BlueprintCallable, Category = "Exploration Actions")
    bool CollectSample(const FName& SessionID, const FName& PointID);

    UFUNCTION(BlueprintCallable, Category = "Exploration Actions")
    bool DocumentDiscovery(const FName& SessionID, const FName& PointID, const FString& Notes);

    UFUNCTION(BlueprintCallable, Category = "Exploration Actions")
    bool DeployDrone(const FName& SessionID, const FVector& DeployLocation);

    UFUNCTION(BlueprintCallable, Category = "Exploration Actions")
    bool RecallDrone(const FName& SessionID);

    // Movement and Navigation
    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool MoveToLocation(const FName& SessionID, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool FollowPath(const FName& SessionID, const FName& PathID);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopMovement(const FName& SessionID);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector GetCurrentLocation(const FName& SessionID);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool IsMoving(const FName& SessionID);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    float GetDistanceToObjective(const FName& SessionID);

    // Discovery and Analysis
    UFUNCTION(BlueprintCallable, Category = "Discovery")
    FExplorationReward AnalyzeDiscovery(const FName& SessionID, const FName& PointID);

    UFUNCTION(BlueprintCallable, Category = "Discovery")
    bool IdentifyResource(const FName& SessionID, const FName& PointID);

    UFUNCTION(BlueprintCallable, Category = "Discovery")
    bool ScanForLifeforms(const FName& SessionID, const FVector& ScanLocation);

    UFUNCTION(BlueprintCallable, Category = "Discovery")
    bool DetectAnomalies(const FName& SessionID, const FVector& ScanLocation);

    UFUNCTION(BlueprintCallable, Category = "Discovery")
    TArray<FName> GetNearbyDiscoveries(const FName& SessionID, float Radius);

    // Environmental Interaction
    UFUNCTION(BlueprintCallable, Category = "Environment")
    bool CanTraverseTerrain(const FName& SessionID, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    ETerrainDifficulty GetTerrainDifficulty(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    TArray<EEnvironmentalHazard> GetEnvironmentalHazards(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    bool IsLocationSafe(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    FVector FindNearestSafeLocation(const FVector& DangerousLocation);

    // Progress and Rewards
    UFUNCTION(BlueprintCallable, Category = "Progress")
    void UpdateExplorationProgress(const FName& SessionID, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    float GetExplorationProgress(const FName& SessionID);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    FExplorationReward CalculateSessionRewards(const FName& SessionID);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void GrantSessionRewards(const FName& SessionID);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    int32 GetPlayerExplorationLevel(APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    float GetPlayerExplorationExperience(APlayerState* Player);

    // Mapping and Cartography
    UFUNCTION(BlueprintCallable, Category = "Mapping")
    void UpdatePlayerMap(const FName& SessionID, const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Mapping")
    TArray<FVector> GetExploredArea(const FName& SessionID);

    UFUNCTION(BlueprintCallable, Category = "Mapping")
    float GetExploredPercentage(const FName& SessionID, const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Mapping")
    bool IsAreaExplored(const FName& SessionID, const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Mapping")
    void CreateMapMarker(const FName& SessionID, const FVector& Location, const FName& MarkerType, const FString& Label);

    // Utilities
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float CalculateExplorationTime(const FVector& Start, const FVector& End, ETerrainDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool CanPlayerExplore(APlayerState* Player, ETerrainDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    TArray<EExplorationTool> GetRequiredTools(const FName& PointID);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float GetDiscoveryValue(const FName& PointID);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void GenerateExplorationReport(const FName& SessionID);

protected:
    // Exploration Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Exploration Data")
    TMap<FName, FExplorationPoint> ExplorationPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Exploration Data")
    TMap<FName, FExplorationPath> ExplorationPaths;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Exploration Data")
    TMap<FName, FExplorationSession> ActiveSessions;

    // Player Data
    TMap<APlayerState*, FPlayerSessionList> PlayerSessions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Data")
    TMap<APlayerState*, float> PlayerExperience;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Data")
    TMap<APlayerState*, int32> PlayerLevels;

    // Mapping Data
    TMap<FName, FExploredAreaPoints> ExploredAreas;

    TMap<FName, FMapMarkerData> MapMarkers;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ExplorationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float BaseExplorationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ScanRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float AnalysisDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableAutoMapping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableEnvironmentalHazards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableToolRequirements;

private:
    // Internal Functions
    void UpdateActiveSessions(float DeltaTime);
    void ProcessExplorationActions(float DeltaTime);
    void UpdateEnvironmentalConditions(float DeltaTime);
    void HandleDiscoveries(float DeltaTime);

    // Session Management Internal
    FName GenerateSessionID();
    void UpdateSessionState(const FName& SessionID, EExplorationState NewState);
    void UpdateSessionLocation(const FName& SessionID, const FVector& NewLocation);
    void CompleteSession(const FName& SessionID);

    // Point Management Internal
    void GenerateRandomPoints();
    bool ValidatePoint(const FExplorationPoint& Point);
    void UpdatePointDiscovery(const FName& PointID, APlayerState* Discoverer);

    // Path Management Internal
    void CalculatePathDifficulty(FExplorationPath& Path);
    bool ValidatePath(const FExplorationPath& Path);
    TArray<FVector> FindOptimalPath(const FVector& Start, const FVector& End);

    // Tool Management Internal
    bool CanUseTool(const FName& SessionID, EExplorationTool Tool);
    void ProcessToolUsage(const FName& SessionID, EExplorationTool Tool, const FVector& Target);
    float GetToolEfficiency(EExplorationTool Tool);

    // Movement Internal
    void ProcessMovement(const FName& SessionID, float DeltaTime);
    bool CanMoveToLocation(const FName& SessionID, const FVector& Location);
    float CalculateMovementSpeed(const FName& SessionID, ETerrainDifficulty Terrain);

    // Discovery Internal
    void ProcessScanning(const FName& SessionID, float DeltaTime);
    void ProcessAnalysis(const FName& SessionID, float DeltaTime);
    void ProcessCollection(const FName& SessionID, float DeltaTime);
    FExplorationReward GenerateDiscoveryReward(const FExplorationPoint& Point, APlayerState* Discoverer);

    // Environmental Internal
    void ApplyEnvironmentalEffects(const FName& SessionID, float DeltaTime);
    bool CheckEnvironmentalSafety(const FVector& Location);
    void ProcessEnvironmentalHazards(const FName& SessionID, float DeltaTime);

    // Progress Internal
    void UpdateExplorationStats(const FName& SessionID);
    void CalculateExperienceGain(const FName& SessionID);
    void UpdatePlayerLevel(APlayerState* Player);

    // Mapping Internal
    void UpdateAutoMapping(const FName& SessionID, float DeltaTime);
    void ProcessMapMarkers(const FName& SessionID);
    void OptimizeExploredAreas(const FName& SessionID);

    // Validation
    bool ValidateSession(const FExplorationSession& Session);
    bool CanPlayerStartSession(APlayerState* Player);

    // Timers
    UPROPERTY()
    FTimerHandle ExplorationUpdateTimer;

    UPROPERTY()
    FTimerHandle EnvironmentalUpdateTimer;

    UPROPERTY()
    FTimerHandle DiscoveryUpdateTimer;

    // Constants
    static constexpr float DEFAULT_EXPLORATION_UPDATE_INTERVAL = 0.1f;
    static constexpr float DEFAULT_BASE_EXPLORATION_SPEED = 200.0f; // units per second
    static constexpr float DEFAULT_SCAN_RANGE = 500.0f;
    static constexpr float DEFAULT_ANALYSIS_DURATION = 5.0f;
    static constexpr float MOVEMENT_UPDATE_INTERVAL = 0.05f;
    static constexpr float EXPERIENCE_PER_DISCOVERY = 100.0f;
    static constexpr float EXPERIENCE_PER_KILOMETER = 10.0f;
    static constexpr int32 MAX_SESSIONS_PER_PLAYER = 3;

    // Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExplorationSessionStarted, const FName&, SessionID, APlayerState*, Explorer);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExplorationSessionEnded, const FName&, SessionID, const FExplorationReward&, Rewards);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPointDiscovered, const FName&, PointID, APlayerState*, Discoverer, const FExplorationPoint&, PointData);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPointExplored, const FName&, PointID, APlayerState*, Explorer, const FExplorationReward&, Rewards);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnToolUsed, const FName&, SessionID, EExplorationTool, Tool);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnvironmentalHazard, const FName&, SessionID, EEnvironmentalHazard, Hazard);

public:
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Exploration Events")
    FOnExplorationSessionStarted OnExplorationSessionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Exploration Events")
    FOnExplorationSessionEnded OnExplorationSessionEnded;

    UPROPERTY(BlueprintAssignable, Category = "Exploration Events")
    FOnPointDiscovered OnPointDiscovered;

    UPROPERTY(BlueprintAssignable, Category = "Exploration Events")
    FOnPointExplored OnPointExplored;

    UPROPERTY(BlueprintAssignable, Category = "Exploration Events")
    FOnToolUsed OnToolUsed;

    UPROPERTY(BlueprintAssignable, Category = "Exploration Events")
    FOnEnvironmentalHazard OnEnvironmentalHazard;
};
