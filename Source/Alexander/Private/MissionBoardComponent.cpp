#include "MissionBoardComponent.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "EconomySystem.h"
#include "QuestSystem.h"
#include "FactionTerritorySystem.h"
#include "GameFramework/PlayerController.h"

// Typedef for compatibility with header
typedef FMissionBoardObjective FMissionBoardObjective;

UMissionBoardComponent::UMissionBoardComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = DEFAULT_MISSION_UPDATE_INTERVAL;

    // Configuration defaults
    MaxMissionsPerPlayer = DEFAULT_MAX_MISSIONS_PER_PLAYER;
    MaxTrackedMissions = DEFAULT_MAX_TRACKED_MISSIONS;
    MissionUpdateInterval = DEFAULT_MISSION_UPDATE_INTERVAL;
    bAutoGenerateMissions = true;
    bEnableMissionSharing = true;
    bEnableMissionTracking = true;
}

void UMissionBoardComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMissionBoard();
}

void UMissionBoardComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateMissionBoards(DeltaTime);
    UpdateActiveMissions(DeltaTime);
    ProcessMissionTimers(DeltaTime);
}

void UMissionBoardComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(MissionUpdateTimer);
        World->GetTimerManager().ClearTimer(MissionGenerationTimer);
    }

    // Clear all data
    MissionBoards.Empty();
    ActiveMissions.Empty();
    PlayerMissions.Empty();
    TrackedMissions.Empty();

    Super::EndPlay(EndPlayReason);
}

void UMissionBoardComponent::InitializeMissionBoard()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Initialize mission system
    // NOTE: UMissionSystem is not currently available - commented out
    // MissionSystem = NewObject<UMissionSystem>(this, TEXT("MissionSystem"));

    // Load mission templates
    LoadMissionTemplates();

    // Setup timers
    World->GetTimerManager().SetTimer(MissionUpdateTimer, [this]()
    {
        float DeltaTime = MissionUpdateInterval;
        UpdateMissionBoards(DeltaTime);
    }, MissionUpdateInterval, true);

    if (bAutoGenerateMissions)
    {
        World->GetTimerManager().SetTimer(MissionGenerationTimer, [this]()
        {
            // Auto-generate missions for all boards
            for (auto& BoardPair : MissionBoards)
            {
                GenerateRandomMissions(BoardPair.Key, 1);
            }
        }, DEFAULT_MISSION_GENERATION_INTERVAL, true);
    }

    UE_LOG(LogTemp, Log, TEXT("MissionBoardComponent initialized"));
}

void UMissionBoardComponent::RegisterMissionBoard(const FMissionBoard& Board)
{
    if (MissionBoards.Contains(Board.BoardName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Mission board '%s' already registered"), *Board.BoardName.ToString());
        return;
    }

    MissionBoards.Add(Board.BoardName, Board);
    UE_LOG(LogTemp, Log, TEXT("Mission board '%s' registered"), *Board.BoardName.ToString());
}

void UMissionBoardComponent::UnregisterMissionBoard(const FName& BoardName)
{
    MissionBoards.Remove(BoardName);
}

FMissionBoard* UMissionBoardComponent::GetMissionBoard(const FName& BoardName)
{
    return MissionBoards.Find(BoardName);
}

TArray<FMissionBoard> UMissionBoardComponent::GetAllMissionBoards()
{
    TArray<FMissionBoard> Boards;
    MissionBoards.GenerateValueArray(Boards);
    return Boards;
}

void UMissionBoardComponent::AddMissionToBoard(const FName& BoardName, const FMissionData& Mission)
{
    if (!ValidateMission(Mission))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid mission"));
        return;
    }

    if (FMissionBoard* Board = MissionBoards.Find(BoardName))
    {
        Board->AvailableMissions.Add(Mission);
        ActiveMissions.Add(Mission.MissionID, Mission);
        
        UE_LOG(LogTemp, Log, TEXT("Mission '%s' added to board '%s'"), 
            *Mission.MissionID.ToString(), *BoardName.ToString());
    }
}

void UMissionBoardComponent::RemoveMissionFromBoard(const FName& BoardName, const FName& MissionID)
{
    if (FMissionBoard* Board = MissionBoards.Find(BoardName))
    {
        for (int32 i = Board->AvailableMissions.Num() - 1; i >= 0; i--)
        {
            if (Board->AvailableMissions[i].MissionID == MissionID)
            {
                Board->AvailableMissions.RemoveAt(i);
                break;
            }
        }
    }

    ActiveMissions.Remove(MissionID);
}

FMissionData* UMissionBoardComponent::GetMission(const FName& MissionID)
{
    return ActiveMissions.Find(MissionID);
}

TArray<FMissionData> UMissionBoardComponent::GetAvailableMissions(const FName& BoardName)
{
    if (FMissionBoard* Board = MissionBoards.Find(BoardName))
    {
        return Board->AvailableMissions;
    }
    return TArray<FMissionData>();
}

TArray<FMissionData> UMissionBoardComponent::GetPlayerMissions(APlayerState* Player)
{
    TArray<FMissionData> Missions;
    
    if (PlayerMissions.Contains(Player))
    {
        const FPlayerMissionList& MissionList = PlayerMissions[Player];
        for (const FName& MissionID : MissionList.MissionIDs)
        {
            if (FMissionData* Mission = ActiveMissions.Find(MissionID))
            {
                Missions.Add(*Mission);
            }
        }
    }
    
    return Missions;
}

void UMissionBoardComponent::RefreshMissionBoard(const FName& BoardName)
{
    if (FMissionBoard* Board = MissionBoards.Find(BoardName))
    {
        Board->LastRefreshTime = GetWorld()->GetTimeSeconds();
        
        // Generate new missions if auto-refresh enabled
        if (Board->bAutoRefresh && bAutoGenerateMissions)
        {
            int32 MissionsToGenerate = Board->MaxMissions - Board->AvailableMissions.Num();
            if (MissionsToGenerate > 0)
            {
                GenerateRandomMissions(BoardName, MissionsToGenerate);
            }
        }

        OnMissionBoardRefreshed.Broadcast(BoardName, Board->AvailableMissions.Num());
    }
}

void UMissionBoardComponent::GenerateRandomMissions(const FName& BoardName, int32 Count)
{
    if (!MissionBoards.Contains(BoardName))
    {
        return;
    }

    for (int32 i = 0; i < Count; i++)
    {
        // Randomly select mission type, difficulty, and faction
        EMissionType Type = static_cast<EMissionType>(FMath::RandRange(0, 14));
        EMissionDifficulty Difficulty = static_cast<EMissionDifficulty>(FMath::RandRange(0, 6));
        EMissionFaction Faction = static_cast<EMissionFaction>(FMath::RandRange(0, 7));

        FMissionData Mission = GenerateMission(Type, Difficulty, Faction);
        AddMissionToBoard(BoardName, Mission);
    }
}

bool UMissionBoardComponent::CanPlayerAcceptMission(APlayerState* Player, const FName& MissionID)
{
    if (!Player)
    {
        return false;
    }

    FMissionData* Mission = GetMission(MissionID);
    if (!Mission)
    {
        return false;
    }

    // Check if player already has this mission
    if (PlayerMissions.Contains(Player))
    {
        if (PlayerMissions[Player].MissionIDs.Contains(MissionID))
        {
            return false;
        }
    }

    // Check max missions
    if (PlayerMissions.Contains(Player))
    {
        if (PlayerMissions[Player].MissionIDs.Num() >= MaxMissionsPerPlayer)
        {
            return false;
        }
    }

    return CanPlayerAttemptMission(Player, *Mission);
}

bool UMissionBoardComponent::AcceptMission(APlayerState* Player, const FName& MissionID)
{
    if (!CanPlayerAcceptMission(Player, MissionID))
    {
        return false;
    }

    FMissionData* Mission = GetMission(MissionID);
    if (!Mission)
    {
        return false;
    }

    // Add to player missions
    if (!PlayerMissions.Contains(Player))
    {
        PlayerMissions.Add(Player, FPlayerMissionList());
    }
    PlayerMissions[Player].MissionIDs.AddUnique(MissionID);

    // Update mission status
    Mission->Status = EMissionStatus::Accepted;
    Mission->AssignedPlayer = Player;

    // Find which board this mission belongs to
    FName BoardName;
    for (auto& BoardPair : MissionBoards)
    {
        for (const FMissionData& BoardMission : BoardPair.Value.AvailableMissions)
        {
            if (BoardMission.MissionID == MissionID)
            {
                BoardName = BoardPair.Key;
                break;
            }
        }
        if (BoardName != NAME_None)
        {
            break;
        }
    }

    OnMissionAccepted.Broadcast(Player, *Mission, BoardName);

    UE_LOG(LogTemp, Log, TEXT("Player accepted mission '%s'"), *MissionID.ToString());
    return true;
}

bool UMissionBoardComponent::AbandonMission(APlayerState* Player, const FName& MissionID)
{
    if (!Player || !PlayerMissions.Contains(Player))
    {
        return false;
    }

    FPlayerMissionList& MissionList = PlayerMissions[Player];
    if (!MissionList.MissionIDs.Contains(MissionID))
    {
        return false;
    }

    // Remove from player missions
    MissionList.MissionIDs.Remove(MissionID);

    // Update mission status
    if (FMissionData* Mission = GetMission(MissionID))
    {
        Mission->Status = EMissionStatus::Abandoned;
        Mission->AssignedPlayer = nullptr;
    }

    OnMissionAbandoned.Broadcast(Player, MissionID);

    UE_LOG(LogTemp, Log, TEXT("Player abandoned mission '%s'"), *MissionID.ToString());
    return true;
}

bool UMissionBoardComponent::ShareMission(APlayerState* Player, const FName& MissionID, APlayerState* TargetPlayer)
{
    if (!bEnableMissionSharing || !Player || !TargetPlayer)
    {
        return false;
    }

    // Check if the sharing player has this mission
    if (!PlayerMissions.Contains(Player) || !PlayerMissions[Player].MissionIDs.Contains(MissionID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Player does not have mission '%s' to share"), *MissionID.ToString());
        return false;
    }

    // Get the mission data
    FMissionData* Mission = GetMission(MissionID);
    if (!Mission)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mission '%s' not found"), *MissionID.ToString());
        return false;
    }

    // Check if target player can accept this mission
    if (!CanPlayerAcceptMission(TargetPlayer, MissionID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Target player cannot accept mission '%s'"), *MissionID.ToString());
        return false;
    }

    // Check if target player meets level and prerequisites
    if (!CanPlayerAttemptMission(TargetPlayer, *Mission))
    {
        UE_LOG(LogTemp, Warning, TEXT("Target player does not meet requirements for mission '%s'"), *MissionID.ToString());
        return false;
    }

    // Add mission to target player's mission list
    if (!PlayerMissions.Contains(TargetPlayer))
    {
        PlayerMissions.Add(TargetPlayer, FPlayerMissionList());
    }
    PlayerMissions[TargetPlayer].MissionIDs.AddUnique(MissionID);

    // Update mission to show shared status (can track multiple players)
    // For shared missions, we keep original player but allow progress sync
    OnMissionAccepted.Broadcast(TargetPlayer, *Mission, NAME_None);

    UE_LOG(LogTemp, Log, TEXT("Mission '%s' shared from player to target player"), *MissionID.ToString());
    return true;
}

void UMissionBoardComponent::UpdateMissionProgress(APlayerState* Player, const FName& MissionID, const FName& ObjectiveID, int32 Progress)
{
    FMissionData* Mission = GetMission(MissionID);
    if (!Mission || Mission->AssignedPlayer != Player)
    {
        return;
    }

    for (FMissionBoardObjective& Objective : Mission->Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.CurrentCount = FMath::Min(Objective.CurrentCount + Progress, Objective.TargetCount);
            
            if (Objective.CurrentCount >= Objective.TargetCount)
            {
                CompleteMissionObjective(Player, MissionID, ObjectiveID);
            }
            
            NotifyMissionUpdate(Player, *Mission);
            break;
        }
    }
}

void UMissionBoardComponent::CompleteMissionObjective(APlayerState* Player, const FName& MissionID, const FName& ObjectiveID)
{
    FMissionData* Mission = GetMission(MissionID);
    if (!Mission)
    {
        return;
    }

    for (FMissionBoardObjective& Objective : Mission->Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.bIsCompleted = true;
            OnMissionObjectiveCompleted.Broadcast(Player, MissionID, ObjectiveID);
            break;
        }
    }

    // Check if mission is complete
    CheckMissionCompletion(MissionID);
}

bool UMissionBoardComponent::IsMissionCompleted(const FName& MissionID)
{
    FMissionData* Mission = GetMission(MissionID);
    if (!Mission)
    {
        return false;
    }

    for (const FMissionBoardObjective& Objective : Mission->Objectives)
    {
        if (Objective.bIsRequired && !Objective.bIsCompleted)
        {
            return false;
        }
    }

    return true;
}

bool UMissionBoardComponent::IsMissionFailed(const FName& MissionID)
{
    FMissionData* Mission = GetMission(MissionID);
    if (!Mission)
    {
        return false;
    }

    return Mission->Status == EMissionStatus::Failed;
}

float UMissionBoardComponent::GetMissionProgress(const FName& MissionID)
{
    FMissionData* Mission = GetMission(MissionID);
    if (!Mission)
    {
        return 0.0f;
    }

    int32 TotalObjectives = 0;
    int32 CompletedObjectives = 0;

    for (const FMissionBoardObjective& Objective : Mission->Objectives)
    {
        TotalObjectives++;
        if (Objective.bIsCompleted)
        {
            CompletedObjectives++;
        }
    }

    return TotalObjectives > 0 ? static_cast<float>(CompletedObjectives) / static_cast<float>(TotalObjectives) : 0.0f;
}

TArray<FMissionBoardObjective> UMissionBoardComponent::GetMissionObjectives(const FName& MissionID)
{
    FMissionData* Mission = GetMission(MissionID);
    if (Mission)
    {
        return Mission->Objectives;
    }
    return TArray<FMissionBoardObjective>();
}

bool UMissionBoardComponent::CompleteMission(APlayerState* Player, const FName& MissionID)
{
    if (!IsMissionCompleted(MissionID))
    {
        return false;
    }

    FMissionData* Mission = GetMission(MissionID);
    if (!Mission || Mission->AssignedPlayer != Player)
    {
        return false;
    }

    Mission->Status = EMissionStatus::Completed;

    // Grant rewards
    GrantMissionRewards(Player, *Mission);

    // Update statistics
    UpdateMissionStatistics(Player, *Mission, true);

    // Save to history
    SaveMissionToHistory(Player, *Mission);

    // Remove from active missions
    if (PlayerMissions.Contains(Player))
    {
        PlayerMissions[Player].MissionIDs.Remove(MissionID);
    }

    OnMissionCompleted.Broadcast(Player, *Mission, true);

    UE_LOG(LogTemp, Log, TEXT("Player completed mission '%s'"), *MissionID.ToString());
    return true;
}

bool UMissionBoardComponent::FailMission(APlayerState* Player, const FName& MissionID)
{
    FMissionData* Mission = GetMission(MissionID);
    if (!Mission || Mission->AssignedPlayer != Player)
    {
        return false;
    }

    Mission->Status = EMissionStatus::Failed;

    // Update statistics
    UpdateMissionStatistics(Player, *Mission, false);

    // Save to history
    SaveMissionToHistory(Player, *Mission);

    // Remove from active missions
    if (PlayerMissions.Contains(Player))
    {
        PlayerMissions[Player].MissionIDs.Remove(MissionID);
    }

    OnMissionFailed.Broadcast(Player, MissionID);

    UE_LOG(LogTemp, Log, TEXT("Player failed mission '%s'"), *MissionID.ToString());
    return true;
}

void UMissionBoardComponent::GrantMissionRewards(APlayerState* Player, const FMissionData& Mission)
{
    if (!Player)
    {
        return;
    }

    // Grant credits
    if (Mission.Reward.Credits > 0)
    {
        GrantCredits(Player, Mission.Reward.Credits);
    }

    // Grant experience
    if (Mission.Reward.Experience > 0)
    {
        GrantExperience(Player, Mission.Reward.Experience);
    }

    // Grant items
    if (!Mission.Reward.Items.IsEmpty())
    {
        // Parse items from string format "ItemID1:Quantity1,ItemID2:Quantity2"
        TMap<FName, int32> ItemsToGrant;
        TArray<FString> ItemEntries;
        Mission.Reward.Items.ParseIntoArray(ItemEntries, TEXT(","), true);

        for (const FString& Entry : ItemEntries)
        {
            FString ItemID, QuantityStr;
            if (Entry.Split(TEXT(":"), &ItemID, &QuantityStr))
            {
                ItemsToGrant.Add(FName(*ItemID), FCString::Atoi(*QuantityStr));
            }
        }

        if (ItemsToGrant.Num() > 0)
        {
            GrantItems(Player, ItemsToGrant);
        }
    }

    // Grant reputation
    if (!Mission.Reward.Reputation.IsEmpty())
    {
        // Parse reputation from string format "FactionID1:Amount1,FactionID2:Amount2"
        TMap<EMissionFaction, float> ReputationToGrant;
        TArray<FString> ReputationEntries;
        Mission.Reward.Reputation.ParseIntoArray(ReputationEntries, TEXT(","), true);

        for (const FString& Entry : ReputationEntries)
        {
            FString FactionIDStr, AmountStr;
            if (Entry.Split(TEXT(":"), &FactionIDStr, &AmountStr))
            {
                // Convert faction index to enum
                int32 FactionIndex = FCString::Atoi(*FactionIDStr);
                if (FactionIndex >= 0 && FactionIndex <= 7)
                {
                    ReputationToGrant.Add(static_cast<EMissionFaction>(FactionIndex), FCString::Atof(*AmountStr));
                }
            }
        }

        if (ReputationToGrant.Num() > 0)
        {
            GrantReputation(Player, ReputationToGrant);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Granted all rewards for mission '%s'"), *Mission.MissionID.ToString());
}

void UMissionBoardComponent::ResetMission(const FName& MissionID)
{
    FMissionData* Mission = GetMission(MissionID);
    if (!Mission)
    {
        return;
    }

    Mission->Status = EMissionStatus::Available;
    Mission->AssignedPlayer = nullptr;
    Mission->TimeRemaining = Mission->TimeLimit;

    for (FMissionBoardObjective& Objective : Mission->Objectives)
    {
        Objective.bIsCompleted = false;
        Objective.CurrentCount = 0;
        Objective.TimeRemaining = Objective.TimeLimit;
    }
}

TArray<FMissionData> UMissionBoardComponent::FilterMissionsByType(const TArray<FMissionData>& Missions, EMissionType Type)
{
    TArray<FMissionData> Filtered;
    for (const FMissionData& Mission : Missions)
    {
        if (Mission.MissionType == Type)
        {
            Filtered.Add(Mission);
        }
    }
    return Filtered;
}

TArray<FMissionData> UMissionBoardComponent::FilterMissionsByDifficulty(const TArray<FMissionData>& Missions, EMissionDifficulty Difficulty)
{
    TArray<FMissionData> Filtered;
    for (const FMissionData& Mission : Missions)
    {
        if (Mission.Difficulty == Difficulty)
        {
            Filtered.Add(Mission);
        }
    }
    return Filtered;
}

TArray<FMissionData> UMissionBoardComponent::FilterMissionsByFaction(const TArray<FMissionData>& Missions, EMissionFaction Faction)
{
    TArray<FMissionData> Filtered;
    for (const FMissionData& Mission : Missions)
    {
        if (Mission.Faction == Faction)
        {
            Filtered.Add(Mission);
        }
    }
    return Filtered;
}

TArray<FMissionData> UMissionBoardComponent::FilterMissionsByLevel(const TArray<FMissionData>& Missions, int32 MinLevel, int32 MaxLevel)
{
    TArray<FMissionData> Filtered;
    for (const FMissionData& Mission : Missions)
    {
        if (Mission.LevelRequirement >= MinLevel && Mission.LevelRequirement <= MaxLevel)
        {
            Filtered.Add(Mission);
        }
    }
    return Filtered;
}

TArray<FMissionData> UMissionBoardComponent::GetRecommendedMissions(APlayerState* Player, const FName& BoardName)
{
    TArray<FMissionData> Recommended;

    if (!Player)
    {
        return Recommended;
    }

    // Get all available missions from the board
    TArray<FMissionData> AvailableMissions = GetAvailableMissions(BoardName);

    // Get player's mission history for analysis
    FPlayerMissionHistory* History = MissionHistory.Find(Player);

    // Calculate player level (estimate from completed missions)
    int32 EstimatedPlayerLevel = 1;
    if (History && History->CompletedMissions.Num() > 0)
    {
        // Estimate level from highest mission level completed
        for (const FMissionData& CompletedMission : History->CompletedMissions)
        {
            EstimatedPlayerLevel = FMath::Max(EstimatedPlayerLevel, CompletedMission.LevelRequirement);
        }
        EstimatedPlayerLevel = FMath::Min(EstimatedPlayerLevel + 5, 100); // Allow slightly higher level missions
    }

    // Get player's current faction standings (from mission board's faction data)
    TMap<EMissionFaction, int32> FactionStandings;
    if (History)
    {
        for (const FMissionData& CompletedMission : History->CompletedMissions)
        {
            int32& Standing = FactionStandings.FindOrAdd(CompletedMission.Faction, 0);
            Standing += 10; // +10 reputation per completed mission
        }
    }

    // Score and filter missions
    struct FMissionScore
    {
        FMissionData Mission;
        float Score;

        FMissionScore(const FMissionData& InMission, float InScore)
            : Mission(InMission), Score(InScore) {}
    };
    TArray<FMissionScore> ScoredMissions;

    for (const FMissionData& Mission : AvailableMissions)
    {
        float Score = 0.0f;

        // Check if player can attempt the mission
        if (!CanPlayerAttemptMission(Player, Mission))
        {
            continue; // Skip missions player can't do
        }

        // Level appropriateness (prefer missions near player level)
        int32 LevelDiff = FMath::Abs(Mission.LevelRequirement - EstimatedPlayerLevel);
        if (LevelDiff <= 3)
        {
            Score += 50.0f - (LevelDiff * 10.0f); // Higher score for closer level match
        }
        else if (LevelDiff > 10)
        {
            continue; // Skip missions too far from player level
        }

        // Faction standing bonus
        int32* FactionRep = FactionStandings.Find(Mission.Faction);
        if (FactionRep && *FactionRep > 0)
        {
            Score += FMath::Min(*FactionRep / 2.0f, 30.0f); // Up to +30 for high faction rep
        }

        // Reward value (normalize by difficulty)
        float RewardScore = (Mission.Reward.Credits / 100.0f) + (Mission.Reward.Experience / 10.0f);
        Score += FMath::Min(RewardScore, 20.0f);

        // Difficulty preference (prefer Normal to Hard for recommendations)
        if (Mission.Difficulty == EMissionDifficulty::Normal || Mission.Difficulty == EMissionDifficulty::Hard)
        {
            Score += 15.0f;
        }
        else if (Mission.Difficulty == EMissionDifficulty::Easy)
        {
            Score += 5.0f;
        }

        // Variety bonus (prefer mission types player hasn't done recently)
        if (History)
        {
            int32 RecentCount = 0;
            int32 CheckCount = FMath::Min(5, History->CompletedMissions.Num());
            for (int32 i = History->CompletedMissions.Num() - 1; i >= History->CompletedMissions.Num() - CheckCount && i >= 0; i--)
            {
                if (History->CompletedMissions[i].MissionType == Mission.MissionType)
                {
                    RecentCount++;
                }
            }
            Score -= RecentCount * 10.0f; // Penalty for repeated types
        }

        ScoredMissions.Add(FMissionScore(Mission, Score));
    }

    // Sort by score (highest first)
    ScoredMissions.Sort([](const FMissionScore& A, const FMissionScore& B) {
        return A.Score > B.Score;
    });

    // Return top 5 recommended missions
    int32 RecommendCount = FMath::Min(5, ScoredMissions.Num());
    for (int32 i = 0; i < RecommendCount; i++)
    {
        Recommended.Add(ScoredMissions[i].Mission);
    }

    UE_LOG(LogTemp, Log, TEXT("Generated %d recommended missions for player"), Recommended.Num());
    return Recommended;
}

void UMissionBoardComponent::TrackMission(APlayerState* Player, const FName& MissionID)
{
    if (!bEnableMissionTracking || !Player)
    {
        return;
    }

    if (!TrackedMissions.Contains(Player))
    {
        TrackedMissions.Add(Player, FPlayerMissionList());
    }

    FPlayerMissionList& TrackedList = TrackedMissions[Player];
    if (TrackedList.MissionIDs.Num() < MaxTrackedMissions)
    {
        TrackedList.MissionIDs.AddUnique(MissionID);
    }
}

void UMissionBoardComponent::UntrackMission(APlayerState* Player, const FName& MissionID)
{
    if (!Player || !TrackedMissions.Contains(Player))
    {
        return;
    }

    TrackedMissions[Player].MissionIDs.Remove(MissionID);
}

TArray<FName> UMissionBoardComponent::GetTrackedMissions(APlayerState* Player)
{
    if (TrackedMissions.Contains(Player))
    {
        return TrackedMissions[Player].MissionIDs;
    }
    return TArray<FName>();
}

bool UMissionBoardComponent::IsMissionTracked(APlayerState* Player, const FName& MissionID)
{
    if (!Player || !TrackedMissions.Contains(Player))
    {
        return false;
    }

    return TrackedMissions[Player].MissionIDs.Contains(MissionID);
}

void UMissionBoardComponent::TriggerMissionEvent(const FName& MissionID, const FName& EventName, const TMap<FName, FString>& EventParameters)
{
    // Handle mission events
}

void UMissionBoardComponent::OnMissionTimerExpired(const FName& MissionID)
{
    FMissionData* Mission = GetMission(MissionID);
    if (Mission && Mission->AssignedPlayer)
    {
        FailMission(Mission->AssignedPlayer, MissionID);
    }
}

void UMissionBoardComponent::OnMissionLocationReached(APlayerState* Player, const FName& MissionID)
{
    // Handle location-based mission events
}

void UMissionBoardComponent::OnMissionTargetEliminated(APlayerState* Player, const FName& MissionID, const FName& TargetID)
{
    // Handle target elimination
}

TMap<EMissionType, int32> UMissionBoardComponent::GetMissionTypeStats(APlayerState* Player)
{
    TMap<EMissionType, int32> Stats;

    if (!Player || !MissionHistory.Contains(Player))
    {
        return Stats;
    }

    const FPlayerMissionHistory& History = MissionHistory[Player];

    // Count completed missions by type
    for (const FMissionData& Mission : History.CompletedMissions)
    {
        if (Mission.Status == EMissionStatus::Completed)
        {
            int32& Count = Stats.FindOrAdd(Mission.MissionType, 0);
            Count++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Retrieved mission type statistics for player: %d types"), Stats.Num());
    return Stats;
}

TMap<EMissionDifficulty, int32> UMissionBoardComponent::GetMissionDifficultyStats(APlayerState* Player)
{
    TMap<EMissionDifficulty, int32> Stats;

    if (!Player || !MissionHistory.Contains(Player))
    {
        return Stats;
    }

    const FPlayerMissionHistory& History = MissionHistory[Player];

    // Count completed missions by difficulty
    for (const FMissionData& Mission : History.CompletedMissions)
    {
        if (Mission.Status == EMissionStatus::Completed)
        {
            int32& Count = Stats.FindOrAdd(Mission.Difficulty, 0);
            Count++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Retrieved mission difficulty statistics for player: %d difficulties"), Stats.Num());
    return Stats;
}

TMap<EMissionFaction, int32> UMissionBoardComponent::GetMissionFactionStats(APlayerState* Player)
{
    TMap<EMissionFaction, int32> Stats;

    if (!Player || !MissionHistory.Contains(Player))
    {
        return Stats;
    }

    const FPlayerMissionHistory& History = MissionHistory[Player];

    // Count completed missions by faction
    for (const FMissionData& Mission : History.CompletedMissions)
    {
        if (Mission.Status == EMissionStatus::Completed)
        {
            int32& Count = Stats.FindOrAdd(Mission.Faction, 0);
            Count++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Retrieved mission faction statistics for player: %d factions"), Stats.Num());
    return Stats;
}

int32 UMissionBoardComponent::GetTotalMissionsCompleted(APlayerState* Player)
{
    if (MissionHistory.Contains(Player))
    {
        return MissionHistory[Player].CompletedMissions.Num();
    }
    return 0;
}

float UMissionBoardComponent::GetMissionSuccessRate(APlayerState* Player)
{
    if (!Player || !MissionHistory.Contains(Player))
    {
        return 0.0f;
    }

    const FPlayerMissionHistory& History = MissionHistory[Player];

    if (History.CompletedMissions.Num() == 0)
    {
        return 0.0f;
    }

    // Count completed vs failed missions
    int32 SuccessfulMissions = 0;
    int32 FailedMissions = 0;

    for (const FMissionData& Mission : History.CompletedMissions)
    {
        if (Mission.Status == EMissionStatus::Completed)
        {
            SuccessfulMissions++;
        }
        else if (Mission.Status == EMissionStatus::Failed)
        {
            FailedMissions++;
        }
    }

    // Calculate success rate as percentage
    int32 TotalAttempted = SuccessfulMissions + FailedMissions;
    if (TotalAttempted == 0)
    {
        return 0.0f;
    }

    float SuccessRate = (static_cast<float>(SuccessfulMissions) / static_cast<float>(TotalAttempted)) * 100.0f;

    UE_LOG(LogTemp, Log, TEXT("Mission success rate for player: %.2f%% (%d/%d)"),
        SuccessRate, SuccessfulMissions, TotalAttempted);

    return SuccessRate;
}

TArray<FMissionData> UMissionBoardComponent::GetMissionHistory(APlayerState* Player)
{
    if (MissionHistory.Contains(Player))
    {
        return MissionHistory[Player].CompletedMissions;
    }
    return TArray<FMissionData>();
}

FString UMissionBoardComponent::GetMissionTypeDisplayName(EMissionType Type)
{
    switch (Type)
    {
    case EMissionType::Escort: return TEXT("Escort");
    case EMissionType::Delivery: return TEXT("Delivery");
    case EMissionType::Bounty: return TEXT("Bounty");
    case EMissionType::Exploration: return TEXT("Exploration");
    case EMissionType::Mining: return TEXT("Mining");
    case EMissionType::Salvage: return TEXT("Salvage");
    case EMissionType::Transport: return TEXT("Transport");
    case EMissionType::Patrol: return TEXT("Patrol");
    case EMissionType::Investigation: return TEXT("Investigation");
    case EMissionType::Rescue: return TEXT("Rescue");
    case EMissionType::Assassination: return TEXT("Assassination");
    case EMissionType::Sabotage: return TEXT("Sabotage");
    case EMissionType::Reconnaissance: return TEXT("Reconnaissance");
    case EMissionType::Diplomatic: return TEXT("Diplomatic");
    default: return TEXT("Unknown");
    }
}

FString UMissionBoardComponent::GetMissionDifficultyDisplayName(EMissionDifficulty Difficulty)
{
    switch (Difficulty)
    {
    case EMissionDifficulty::Trivial: return TEXT("Trivial");
    case EMissionDifficulty::Easy: return TEXT("Easy");
    case EMissionDifficulty::Normal: return TEXT("Normal");
    case EMissionDifficulty::Hard: return TEXT("Hard");
    case EMissionDifficulty::VeryHard: return TEXT("Very Hard");
    case EMissionDifficulty::Extreme: return TEXT("Extreme");
    case EMissionDifficulty::Legendary: return TEXT("Legendary");
    default: return TEXT("Unknown");
    }
}

FString UMissionBoardComponent::GetMissionFactionDisplayName(EMissionFaction Faction)
{
    switch (Faction)
    {
    case EMissionFaction::Neutral: return TEXT("Neutral");
    case EMissionFaction::Corporation: return TEXT("Corporation");
    case EMissionFaction::Military: return TEXT("Military");
    case EMissionFaction::Science: return TEXT("Science");
    case EMissionFaction::Trade: return TEXT("Trade");
    case EMissionFaction::Criminal: return TEXT("Criminal");
    case EMissionFaction::Independent: return TEXT("Independent");
    case EMissionFaction::Alien: return TEXT("Alien");
    default: return TEXT("Unknown");
    }
}

FColor UMissionBoardComponent::GetMissionDifficultyColor(EMissionDifficulty Difficulty)
{
    switch (Difficulty)
    {
    case EMissionDifficulty::Trivial: return FColor::White;
    case EMissionDifficulty::Easy: return FColor::Green;
    case EMissionDifficulty::Normal: return FColor::Yellow;
    case EMissionDifficulty::Hard: return FColor::Orange;
    case EMissionDifficulty::VeryHard: return FColor::Red;
    case EMissionDifficulty::Extreme: return FColor::Purple;
    case EMissionDifficulty::Legendary: return FColor::Cyan;
    default: return FColor::White;
    }
}

bool UMissionBoardComponent::IsValidMissionID(const FName& MissionID)
{
    return ActiveMissions.Contains(MissionID);
}

FName UMissionBoardComponent::GenerateMissionID()
{
    return FName(*FString::Printf(TEXT("MISSION_%d"), FMath::RandRange(100000, 999999)));
}

void UMissionBoardComponent::UpdateMissionBoards(float DeltaTime)
{
    for (auto& BoardPair : MissionBoards)
    {
        FMissionBoard& Board = BoardPair.Value;
        
        // Check for auto-refresh
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (Board.bAutoRefresh && (CurrentTime - Board.LastRefreshTime) >= Board.RefreshInterval)
        {
            RefreshMissionBoard(BoardPair.Key);
        }
    }
}

void UMissionBoardComponent::UpdateActiveMissions(float DeltaTime)
{
    for (auto& MissionPair : ActiveMissions)
    {
        UpdateMissionTimeLimit(MissionPair.Key, DeltaTime);
    }
}

void UMissionBoardComponent::ProcessMissionTimers(float DeltaTime)
{
    // Process mission objective timers
}

void UMissionBoardComponent::GenerateMissionTemplates()
{
    // Generate default mission templates
}

FMissionData UMissionBoardComponent::GenerateMission(EMissionType Type, EMissionDifficulty Difficulty, EMissionFaction Faction)
{
    FMissionData Mission;
    Mission.MissionID = GenerateMissionID();
    Mission.Title = GenerateMissionTitle(Type, Faction);
    Mission.Description = GenerateMissionDescription(Type, Difficulty);
    Mission.MissionType = Type;
    Mission.Difficulty = Difficulty;
    Mission.Status = EMissionStatus::Available;
    Mission.Faction = Faction;
    Mission.TimeLimit = 3600.0f; // 1 hour default
    Mission.TimeRemaining = Mission.TimeLimit;
    Mission.LevelRequirement = static_cast<int32>(Difficulty) * 5;

    GenerateMissionObjectives(Mission, Type);
    GenerateMissionRewards(Mission, Difficulty);

    return Mission;
}

void UMissionBoardComponent::GenerateMissionObjectives(FMissionData& Mission, EMissionType Type)
{
    // Generate objectives based on mission type
    FMissionBoardObjective Objective;
    Objective.ObjectiveID = FName(*FString::Printf(TEXT("OBJ_%d"), FMath::RandRange(1000, 9999)));
    Objective.Description = TEXT("Complete the mission objective");
    Objective.bIsCompleted = false;
    Objective.bIsRequired = true;
    Objective.TargetCount = 1;
    Objective.CurrentCount = 0;

    Mission.Objectives.Add(Objective);
}

void UMissionBoardComponent::GenerateMissionRewards(FMissionData& Mission, EMissionDifficulty Difficulty)
{
    int32 BaseReward = 1000;
    int32 DifficultyMultiplier = static_cast<int32>(Difficulty) + 1;

    Mission.Reward.Credits = BaseReward * DifficultyMultiplier;
    Mission.Reward.Experience = 100 * DifficultyMultiplier;
}

FString UMissionBoardComponent::GenerateMissionTitle(EMissionType Type, EMissionFaction Faction)
{
    return FString::Printf(TEXT("%s Mission - %s"), 
        *GetMissionTypeDisplayName(Type), 
        *GetMissionFactionDisplayName(Faction));
}

FString UMissionBoardComponent::GenerateMissionDescription(EMissionType Type, EMissionDifficulty Difficulty)
{
    return FString::Printf(TEXT("A %s difficulty %s mission."), 
        *GetMissionDifficultyDisplayName(Difficulty), 
        *GetMissionTypeDisplayName(Type));
}

bool UMissionBoardComponent::ValidateMission(const FMissionData& Mission)
{
    return Mission.MissionID != NAME_None && !Mission.Title.IsEmpty();
}

bool UMissionBoardComponent::ValidateMissionObjective(const FMissionBoardObjective& Objective)
{
    return Objective.ObjectiveID != NAME_None;
}

bool UMissionBoardComponent::CanPlayerAttemptMission(APlayerState* Player, const FMissionData& Mission)
{
    if (!Player)
    {
        return false;
    }

    // Get player's estimated level from mission history
    int32 PlayerLevel = 1;
    if (MissionHistory.Contains(Player))
    {
        const FPlayerMissionHistory& History = MissionHistory[Player];
        for (const FMissionData& CompletedMission : History.CompletedMissions)
        {
            if (CompletedMission.Status == EMissionStatus::Completed)
            {
                PlayerLevel = FMath::Max(PlayerLevel, CompletedMission.LevelRequirement);
            }
        }
        // Assume player gains 1 level per 5 completed missions
        PlayerLevel += (History.CompletedMissions.Num() / 5);
    }

    // Check level requirement
    if (PlayerLevel < Mission.LevelRequirement)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player level %d is below required level %d for mission '%s'"),
            PlayerLevel, Mission.LevelRequirement, *Mission.MissionID.ToString());
        return false;
    }

    // Check prerequisite missions
    if (Mission.PrerequisiteMissions.Num() > 0)
    {
        if (!MissionHistory.Contains(Player))
        {
            UE_LOG(LogTemp, Warning, TEXT("Player has not completed prerequisite missions for '%s'"),
                *Mission.MissionID.ToString());
            return false;
        }

        const FPlayerMissionHistory& History = MissionHistory[Player];
        for (const FName& PrereqMissionID : Mission.PrerequisiteMissions)
        {
            bool bPrereqCompleted = false;
            for (const FMissionData& CompletedMission : History.CompletedMissions)
            {
                if (CompletedMission.MissionID == PrereqMissionID &&
                    CompletedMission.Status == EMissionStatus::Completed)
                {
                    bPrereqCompleted = true;
                    break;
                }
            }

            if (!bPrereqCompleted)
            {
                UE_LOG(LogTemp, Warning, TEXT("Player has not completed prerequisite mission '%s' for '%s'"),
                    *PrereqMissionID.ToString(), *Mission.MissionID.ToString());
                return false;
            }
        }
    }

    // Check required stats (if any)
    if (Mission.RequiredStats.Num() > 0)
    {
        // Note: This would require integration with a player stats system
        // For now, we'll log and assume the player meets stat requirements
        UE_LOG(LogTemp, Log, TEXT("Mission '%s' has stat requirements, assuming player meets them"),
            *Mission.MissionID.ToString());
    }

    // Check faction reputation (estimate from completed missions)
    if (MissionHistory.Contains(Player))
    {
        const FPlayerMissionHistory& History = MissionHistory[Player];
        int32 FactionReputation = 0;

        for (const FMissionData& CompletedMission : History.CompletedMissions)
        {
            if (CompletedMission.Faction == Mission.Faction &&
                CompletedMission.Status == EMissionStatus::Completed)
            {
                FactionReputation += 10; // +10 rep per completed mission for that faction
            }
        }

        // Missions might require minimum reputation (stored in RequiredStats or similar)
        // For now, just log the faction reputation
        UE_LOG(LogTemp, Log, TEXT("Player faction reputation for %s: %d"),
            *GetMissionFactionDisplayName(Mission.Faction), FactionReputation);
    }

    UE_LOG(LogTemp, Log, TEXT("Player can attempt mission '%s'"), *Mission.MissionID.ToString());
    return true;
}

void UMissionBoardComponent::UpdateMissionTimeLimit(const FName& MissionID, float DeltaTime)
{
    FMissionData* Mission = GetMission(MissionID);
    if (Mission && Mission->TimeLimit > 0.0f)
    {
        Mission->TimeRemaining -= DeltaTime;
        if (Mission->TimeRemaining <= 0.0f)
        {
            OnMissionTimerExpired(MissionID);
        }
    }
}

void UMissionBoardComponent::CheckMissionCompletion(const FName& MissionID)
{
    if (IsMissionCompleted(MissionID))
    {
        FMissionData* Mission = GetMission(MissionID);
        if (Mission && Mission->AssignedPlayer)
        {
            Mission->Status = EMissionStatus::InProgress;
        }
    }
}

void UMissionBoardComponent::CheckMissionFailure(const FName& MissionID)
{
    // Check for mission failure conditions
}

void UMissionBoardComponent::NotifyMissionUpdate(APlayerState* Player, const FMissionData& Mission)
{
    // Notify player of mission updates
}

void UMissionBoardComponent::GrantCredits(APlayerState* Player, int32 Amount)
{
    if (!Player || Amount <= 0)
    {
        return;
    }

    // Try to find EconomySystem component on the player's pawn or controller
    AActor* PlayerActor = Player->GetPawn();
    if (!PlayerActor)
    {
        // Try to get from player controller
        APlayerController* PC = Cast<APlayerController>(Player->GetOwner());
        if (PC)
        {
            PlayerActor = PC->GetPawn();
        }
    }

    if (PlayerActor)
    {
        // Look for EconomySystem component
        UActorComponent* EconomyComp = PlayerActor->GetComponentByClass(UEconomySystem::StaticClass());
        if (EconomyComp)
        {
            UEconomySystem* EconomySystem = Cast<UEconomySystem>(EconomyComp);
            if (EconomySystem)
            {
                EconomySystem->PlayerCredits += Amount;
                UE_LOG(LogTemp, Log, TEXT("Granted %d credits to player. New balance: %.2f"),
                    Amount, EconomySystem->PlayerCredits);
                return;
            }
        }
    }

    // Fallback: Log the credit grant (game systems can hook into this via delegates)
    UE_LOG(LogTemp, Log, TEXT("Granted %d credits to player (no EconomySystem found, logged only)"), Amount);
}

void UMissionBoardComponent::GrantExperience(APlayerState* Player, int32 Amount)
{
    if (!Player || Amount <= 0)
    {
        return;
    }

    // Try to find QuestSystem component for experience tracking
    AActor* PlayerActor = Player->GetPawn();
    if (!PlayerActor)
    {
        // Try to get from player controller
        APlayerController* PC = Cast<APlayerController>(Player->GetOwner());
        if (PC)
        {
            PlayerActor = PC->GetPawn();
        }
    }

    if (PlayerActor)
    {
        // Look for QuestSystem component (which may track player experience)
        UActorComponent* QuestComp = PlayerActor->GetComponentByClass(UQuestSystem::StaticClass());
        if (QuestComp)
        {
            UQuestSystem* QuestSystem = Cast<UQuestSystem>(QuestComp);
            if (QuestSystem)
            {
                // QuestSystem can be extended to track total XP
                // For now, just log and broadcast via delegates
                UE_LOG(LogTemp, Log, TEXT("Granted %d experience points to player (via QuestSystem)"), Amount);
            }
        }
    }

    // Calculate estimated level-up
    // Assuming 100 XP per level as a baseline
    int32 CurrentLevel = 1;
    if (MissionHistory.Contains(Player))
    {
        const FPlayerMissionHistory& History = MissionHistory[Player];
        CurrentLevel += (History.CompletedMissions.Num() / 5); // 1 level per 5 completed missions
    }

    // Log experience grant with potential level up notification
    UE_LOG(LogTemp, Log, TEXT("Granted %d experience points to player. Estimated Level: %d"), Amount, CurrentLevel);

    // Note: For full implementation, this would integrate with a PlayerProgression component
    // that tracks total XP, current level, and handles level-up logic
}

void UMissionBoardComponent::GrantItems(APlayerState* Player, const TMap<FName, int32>& Items)
{
    if (!Player || Items.Num() == 0)
    {
        return;
    }

    // Try to find EconomySystem component for cargo/inventory management
    AActor* PlayerActor = Player->GetPawn();
    if (!PlayerActor)
    {
        // Try to get from player controller
        APlayerController* PC = Cast<APlayerController>(Player->GetOwner());
        if (PC)
        {
            PlayerActor = PC->GetPawn();
        }
    }

    if (PlayerActor)
    {
        // Look for EconomySystem component (which manages cargo/inventory)
        UActorComponent* EconomyComp = PlayerActor->GetComponentByClass(UEconomySystem::StaticClass());
        if (EconomyComp)
        {
            UEconomySystem* EconomySystem = Cast<UEconomySystem>(EconomyComp);
            if (EconomySystem)
            {
                // Add items to cargo hold
                for (const TPair<FName, int32>& ItemPair : Items)
                {
                    // Create a commodity data for the item
                    FCommodityData ItemCommodity;
                    ItemCommodity.CommodityName = ItemPair.Key;
                    ItemCommodity.BasePrice = 0.0f; // Mission rewards are free
                    ItemCommodity.VolumePerUnit = 1.0f;

                    // Check if player has cargo space
                    if (EconomySystem->HasCargoSpace(ItemCommodity, ItemPair.Value))
                    {
                        EconomySystem->AddToCargo(ItemCommodity, ItemPair.Value, 0.0f);
                        UE_LOG(LogTemp, Log, TEXT("Granted item '%s' x%d to player's cargo"),
                            *ItemPair.Key.ToString(), ItemPair.Value);
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Insufficient cargo space for item '%s' x%d"),
                            *ItemPair.Key.ToString(), ItemPair.Value);
                    }
                }
                return;
            }
        }
    }

    // Fallback: Log the item grants
    UE_LOG(LogTemp, Log, TEXT("Granting items to player (no EconomySystem found):"));
    for (const TPair<FName, int32>& ItemPair : Items)
    {
        UE_LOG(LogTemp, Log, TEXT("  - %s x%d"), *ItemPair.Key.ToString(), ItemPair.Value);
    }

    // Note: For full implementation, this would integrate with an Inventory component
    // that manages player items, equipment, and cargo
}

void UMissionBoardComponent::GrantReputation(APlayerState* Player, const TMap<EMissionFaction, float>& Reputation)
{
    if (!Player || Reputation.Num() == 0)
    {
        return;
    }

    // Try to find FactionTerritorySystem component for reputation management
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Search for FactionTerritorySystem in the world
    UFactionTerritorySystem* FactionSystem = nullptr;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor)
        {
            UFactionTerritorySystem* FoundSystem = Actor->FindComponentByClass<UFactionTerritorySystem>();
            if (FoundSystem)
            {
                FactionSystem = FoundSystem;
                break;
            }
        }
    }

    // Map EMissionFaction to faction ID strings for FactionTerritorySystem
    TMap<EMissionFaction, FString> FactionIDMap;
    FactionIDMap.Add(EMissionFaction::Neutral, TEXT("Neutral"));
    FactionIDMap.Add(EMissionFaction::Corporation, TEXT("Corporation"));
    FactionIDMap.Add(EMissionFaction::Military, TEXT("Military"));
    FactionIDMap.Add(EMissionFaction::Science, TEXT("Science"));
    FactionIDMap.Add(EMissionFaction::Trade, TEXT("Trade"));
    FactionIDMap.Add(EMissionFaction::Criminal, TEXT("Criminal"));
    FactionIDMap.Add(EMissionFaction::Independent, TEXT("Independent"));
    FactionIDMap.Add(EMissionFaction::Alien, TEXT("Alien"));

    if (FactionSystem)
    {
        // Apply reputation changes through FactionTerritorySystem
        for (const TPair<EMissionFaction, float>& RepPair : Reputation)
        {
            FString* FactionID = FactionIDMap.Find(RepPair.Key);
            if (FactionID)
            {
                // Convert float reputation to int32 for the system
                int32 ReputationChange = FMath::RoundToInt(RepPair.Value);
                FactionSystem->ModifyReputation(*FactionID, ReputationChange);

                UE_LOG(LogTemp, Log, TEXT("Granted %d reputation with faction '%s' to player"),
                    ReputationChange, **FactionID);
            }
        }
    }
    else
    {
        // Fallback: Log reputation grants
        UE_LOG(LogTemp, Log, TEXT("Granting reputation to player (no FactionTerritorySystem found):"));
        for (const TPair<EMissionFaction, float>& RepPair : Reputation)
        {
            FString FactionName = GetMissionFactionDisplayName(RepPair.Key);
            UE_LOG(LogTemp, Log, TEXT("  - %s: %+.2f"), *FactionName, RepPair.Value);
        }
    }

    // Note: For full implementation, this integrates with the FactionTerritorySystem
    // which manages faction relationships, allegiances, and unlocks based on reputation
}

void UMissionBoardComponent::UpdateMissionStatistics(APlayerState* Player, const FMissionData& Mission, bool bCompleted)
{
    // Update player mission statistics
}

void UMissionBoardComponent::SaveMissionToHistory(APlayerState* Player, const FMissionData& Mission)
{
    if (!MissionHistory.Contains(Player))
    {
        MissionHistory.Add(Player, FPlayerMissionHistory());
    }

    FPlayerMissionHistory& History = MissionHistory[Player];
    History.CompletedMissions.Add(Mission);

    if (History.CompletedMissions.Num() > MAX_MISSION_HISTORY)
    {
        History.CompletedMissions.RemoveAt(0);
    }
}

void UMissionBoardComponent::LoadMissionTemplates()
{
    // Load mission templates from assets
}

void UMissionBoardComponent::SaveMissionTemplates()
{
    // Save mission templates to assets
}
