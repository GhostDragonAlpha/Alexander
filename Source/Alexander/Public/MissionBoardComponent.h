#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/PlayerState.h"
#include "PerformanceProfiler.h"
#include "MissionBoardComponent.generated.h"

UENUM(BlueprintType)
enum class EMissionType : uint8
{
    None,
    Escort,
    Delivery,
    Bounty,
    Exploration,
    Mining,
    Salvage,
    Transport,
    Patrol,
    Investigation,
    Rescue,
    Assassination,
    Sabotage,
    Reconnaissance,
    Diplomatic,
    Combat
};

UENUM(BlueprintType)
enum class EMissionDifficulty : uint8
{
    Trivial,
    Easy,
    Normal,
    Hard,
    VeryHard,
    Extreme,
    Legendary
};

UENUM(BlueprintType)
enum class EMissionStatus : uint8
{
    Available,
    Accepted,
    InProgress,
    Completed,
    Failed,
    Abandoned,
    Expired
};

UENUM(BlueprintType)
enum class EMissionFaction : uint8
{
    Neutral,
    Corporation,
    Military,
    Science,
    Trade,
    Criminal,
    Independent,
    Alien
};

USTRUCT(BlueprintType)
struct FMissionBoardObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRequired;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ObjectiveParameters; // Changed from TMap to FString for serialization

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> PrerequisiteObjectives;
};

USTRUCT(BlueprintType)
struct FMissionReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Credits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Experience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Items; // Changed from TMap to FString for serialization

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Reputation; // Changed from TMap to FString for serialization

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlockItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlockMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RewardDescription;
};

USTRUCT(BlueprintType)
struct FMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMissionDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMissionStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMissionFaction Faction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APlayerState* AssignedPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMissionBoardObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMissionReward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MissionLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MissionGiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> PrerequisiteMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LevelRequirement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> RequiredStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxCompletions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentCompletions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHidden;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLegendary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* MissionIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> MissionTags;
};

USTRUCT(BlueprintType)
struct FMissionBoard
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BoardName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMissionData> AvailableMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EMissionFaction, int32> FactionReputation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EMissionType, int32> MissionTypeCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RefreshInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastRefreshTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoRefresh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EMissionDifficulty, float> DifficultyWeights;
};

USTRUCT(BlueprintType)
struct FPlayerMissionList
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FName> MissionIDs;
};

USTRUCT(BlueprintType)
struct FPlayerMissionHistory
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FMissionData> CompletedMissions;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UMissionBoardComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMissionBoardComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Mission Board Management
    UFUNCTION(BlueprintCallable, Category = "Mission Board")
    void InitializeMissionBoard();

    UFUNCTION(BlueprintCallable, Category = "Mission Board")
    void RegisterMissionBoard(const FMissionBoard& Board);

    UFUNCTION(BlueprintCallable, Category = "Mission Board")
    void UnregisterMissionBoard(const FName& BoardName);

    FMissionBoard* GetMissionBoard(const FName& BoardName);

    UFUNCTION(BlueprintCallable, Category = "Mission Board")
    TArray<FMissionBoard> GetAllMissionBoards();

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    void AddMissionToBoard(const FName& BoardName, const FMissionData& Mission);

    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    void RemoveMissionFromBoard(const FName& BoardName, const FName& MissionID);

    FMissionData* GetMission(const FName& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    TArray<FMissionData> GetAvailableMissions(const FName& BoardName);

    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    TArray<FMissionData> GetPlayerMissions(APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    void RefreshMissionBoard(const FName& BoardName);

    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    void GenerateRandomMissions(const FName& BoardName, int32 Count);

    // Mission Acceptance
    UFUNCTION(BlueprintCallable, Category = "Mission Acceptance")
    bool CanPlayerAcceptMission(APlayerState* Player, const FName& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Acceptance")
    bool AcceptMission(APlayerState* Player, const FName& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Acceptance")
    bool AbandonMission(APlayerState* Player, const FName& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Acceptance")
    bool ShareMission(APlayerState* Player, const FName& MissionID, APlayerState* TargetPlayer);

    // Mission Progress
    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    void UpdateMissionProgress(APlayerState* Player, const FName& MissionID, const FName& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    void CompleteMissionObjective(APlayerState* Player, const FName& MissionID, const FName& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    bool IsMissionCompleted(const FName& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    bool IsMissionFailed(const FName& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    float GetMissionProgress(const FName& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    TArray<FMissionBoardObjective> GetMissionObjectives(const FName& MissionID);

    // Mission Completion
    UFUNCTION(BlueprintCallable, Category = "Mission Completion")
    bool CompleteMission(APlayerState* Player, const FName& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Completion")
    bool FailMission(APlayerState* Player, const FName& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Completion")
    void GrantMissionRewards(APlayerState* Player, const FMissionData& Mission);

    UFUNCTION(BlueprintCallable, Category = "Mission Completion")
    void ResetMission(const FName& MissionID);

    // Mission Filtering
    UFUNCTION(BlueprintCallable, Category = "Mission Filtering")
    TArray<FMissionData> FilterMissionsByType(const TArray<FMissionData>& Missions, EMissionType Type);

    UFUNCTION(BlueprintCallable, Category = "Mission Filtering")
    TArray<FMissionData> FilterMissionsByDifficulty(const TArray<FMissionData>& Missions, EMissionDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Mission Filtering")
    TArray<FMissionData> FilterMissionsByFaction(const TArray<FMissionData>& Missions, EMissionFaction Faction);

    UFUNCTION(BlueprintCallable, Category = "Mission Filtering")
    TArray<FMissionData> FilterMissionsByLevel(const TArray<FMissionData>& Missions, int32 MinLevel, int32 MaxLevel);

    UFUNCTION(BlueprintCallable, Category = "Mission Filtering")
    TArray<FMissionData> GetRecommendedMissions(APlayerState* Player, const FName& BoardName);

    // Mission Tracking
    UFUNCTION(BlueprintCallable, Category = "Mission Tracking")
    void TrackMission(APlayerState* Player, const FName& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Tracking")
    void UntrackMission(APlayerState* Player, const FName& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Tracking")
    TArray<FName> GetTrackedMissions(APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Mission Tracking")
    bool IsMissionTracked(APlayerState* Player, const FName& MissionID);

    // Mission Events
    UFUNCTION(BlueprintCallable, Category = "Mission Events")
    void TriggerMissionEvent(const FName& MissionID, const FName& EventName, const TMap<FName, FString>& EventParameters);

    UFUNCTION(BlueprintCallable, Category = "Mission Events")
    void OnMissionTimerExpired(const FName& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Events")
    void OnMissionLocationReached(APlayerState* Player, const FName& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Events")
    void OnMissionTargetEliminated(APlayerState* Player, const FName& MissionID, const FName& TargetID);

    // Mission Statistics
    UFUNCTION(BlueprintCallable, Category = "Mission Statistics")
    TMap<EMissionType, int32> GetMissionTypeStats(APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Mission Statistics")
    TMap<EMissionDifficulty, int32> GetMissionDifficultyStats(APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Mission Statistics")
    TMap<EMissionFaction, int32> GetMissionFactionStats(APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Mission Statistics")
    int32 GetTotalMissionsCompleted(APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Mission Statistics")
    float GetMissionSuccessRate(APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Mission Statistics")
    TArray<FMissionData> GetMissionHistory(APlayerState* Player);

    // Utilities
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FString GetMissionTypeDisplayName(EMissionType Type);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FString GetMissionDifficultyDisplayName(EMissionDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FString GetMissionFactionDisplayName(EMissionFaction Faction);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FColor GetMissionDifficultyColor(EMissionDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool IsValidMissionID(const FName& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FName GenerateMissionID();

protected:
    // Mission Boards
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Boards")
    TMap<FName, FMissionBoard> MissionBoards;

    // Active Missions
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Missions")
    TMap<FName, FMissionData> ActiveMissions;

    // Player Missions
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Missions")
    TMap<APlayerState*, FPlayerMissionList> PlayerMissions;

    // Tracked Missions
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tracked Missions")
    TMap<APlayerState*, FPlayerMissionList> TrackedMissions;

    // Mission History
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission History")
    TMap<APlayerState*, FPlayerMissionHistory> MissionHistory;

    // Mission Templates
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Templates")
    TArray<FMissionData> MissionTemplates;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxMissionsPerPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxTrackedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MissionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoGenerateMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableMissionSharing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableMissionTracking;

    // System References
    // UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    // class UMissionSystem* MissionSystem;

private:
    // Internal Functions
    void UpdateMissionBoards(float DeltaTime);
    void UpdateActiveMissions(float DeltaTime);
    void ProcessMissionTimers(float DeltaTime);
    void GenerateMissionTemplates();

    // Mission Generation
    FMissionData GenerateMission(EMissionType Type, EMissionDifficulty Difficulty, EMissionFaction Faction);
    void GenerateMissionObjectives(FMissionData& Mission, EMissionType Type);
    void GenerateMissionRewards(FMissionData& Mission, EMissionDifficulty Difficulty);
    FString GenerateMissionTitle(EMissionType Type, EMissionFaction Faction);
    FString GenerateMissionDescription(EMissionType Type, EMissionDifficulty Difficulty);

    // Mission Validation
    bool ValidateMission(const FMissionData& Mission);
    bool ValidateMissionObjective(const FMissionBoardObjective& Objective);
    bool CanPlayerAttemptMission(APlayerState* Player, const FMissionData& Mission);

    // Mission Progress Internal
    void UpdateMissionTimeLimit(const FName& MissionID, float DeltaTime);
    void CheckMissionCompletion(const FName& MissionID);
    void CheckMissionFailure(const FName& MissionID);
    void NotifyMissionUpdate(APlayerState* Player, const FMissionData& Mission);

    // Mission Rewards Internal
    void GrantCredits(APlayerState* Player, int32 Amount);
    void GrantExperience(APlayerState* Player, int32 Amount);
    void GrantItems(APlayerState* Player, const TMap<FName, int32>& Items);
    void GrantReputation(APlayerState* Player, const TMap<EMissionFaction, float>& Reputation);

    // Mission Statistics Internal
    void UpdateMissionStatistics(APlayerState* Player, const FMissionData& Mission, bool bCompleted);
    void SaveMissionToHistory(APlayerState* Player, const FMissionData& Mission);

    // Mission Templates Internal
    void LoadMissionTemplates();
    void SaveMissionTemplates();

    // Timers
    UPROPERTY()
    FTimerHandle MissionUpdateTimer;

    UPROPERTY()
    FTimerHandle MissionGenerationTimer;

    // Constants
    static constexpr float DEFAULT_MISSION_UPDATE_INTERVAL = 1.0f;
    static constexpr float DEFAULT_MISSION_GENERATION_INTERVAL = 300.0f; // 5 minutes
    static constexpr int32 DEFAULT_MAX_MISSIONS_PER_PLAYER = 10;
    static constexpr int32 DEFAULT_MAX_TRACKED_MISSIONS = 5;
    static constexpr int32 MAX_MISSION_HISTORY = 100;

    // Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMissionAccepted, APlayerState*, Player, const FMissionData&, Mission, const FName&, BoardName);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMissionCompleted, APlayerState*, Player, const FMissionData&, Mission, bool, bSuccess);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionAbandoned, APlayerState*, Player, const FName&, MissionID);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMissionObjectiveCompleted, APlayerState*, Player, const FName&, MissionID, const FName&, ObjectiveID);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionBoardRefreshed, const FName&, BoardName, int32, NewMissionsCount);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionFailed, APlayerState*, Player, const FName&, MissionID);

public:
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnMissionAccepted OnMissionAccepted;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnMissionCompleted OnMissionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnMissionAbandoned OnMissionAbandoned;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnMissionObjectiveCompleted OnMissionObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnMissionBoardRefreshed OnMissionBoardRefreshed;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnMissionFailed OnMissionFailed;
};