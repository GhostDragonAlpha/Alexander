// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "QuestSystem.generated.h"

// Forward declarations
class AActor;
class UEconomySystem;
class UAIDialogueSystem;

/**
 * Quest types available in the game
 */
UENUM(BlueprintType)
enum class EQuestType : uint8
{
    Story           UMETA(DisplayName = "Story Quest"),
    Combat          UMETA(DisplayName = "Combat Mission"),
    Delivery        UMETA(DisplayName = "Delivery Mission"),
    Exploration     UMETA(DisplayName = "Exploration Mission"),
    Escort          UMETA(DisplayName = "Escort Mission"),
    Mining          UMETA(DisplayName = "Mining Mission"),
    Salvage         UMETA(DisplayName = "Salvage Mission"),
    Assassination   UMETA(DisplayName = "Assassination Mission"),
    Reconnaissance  UMETA(DisplayName = "Reconnaissance Mission"),
    Rescue          UMETA(DisplayName = "Rescue Mission"),
    Smuggling       UMETA(DisplayName = "Smuggling Mission"),
    Bounty          UMETA(DisplayName = "Bounty Mission")
};

/**
 * Quest difficulty levels
 */
UENUM(BlueprintType)
enum class EQuestDifficulty : uint8
{
    Trivial     UMETA(DisplayName = "Trivial"),
    Easy        UMETA(DisplayName = "Easy"),
    Normal      UMETA(DisplayName = "Normal"),
    Hard        UMETA(DisplayName = "Hard"),
    VeryHard    UMETA(DisplayName = "Very Hard"),
    Elite       UMETA(DisplayName = "Elite"),
    Legendary   UMETA(DisplayName = "Legendary")
};

/**
 * Quest status
 */
UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned"),
    OnHold          UMETA(DisplayName = "On Hold")
};

/**
 * Objective types for quests
 */
UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
    KillTarget      UMETA(DisplayName = "Kill Target"),
    DeliverItem     UMETA(DisplayName = "Deliver Item"),
    GoToLocation    UMETA(DisplayName = "Go To Location"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    TalkToNPC       UMETA(DisplayName = "Talk To NPC"),
    ScanObject      UMETA(DisplayName = "Scan Object"),
    DefendTarget    UMETA(DisplayName = "Defend Target"),
    HackTerminal    UMETA(DisplayName = "Hack Terminal"),
    MineResource    UMETA(DisplayName = "Mine Resource"),
    Survive         UMETA(DisplayName = "Survive"),
    EscortTarget    UMETA(DisplayName = "Escort Target"),
    DestroyObject   UMETA(DisplayName = "Destroy Object")
};

/**
 * Quest objective structure
 */
USTRUCT(BlueprintType)
struct FQuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TargetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsHidden;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> PrerequisiteObjectiveIDs;

    FQuestObjective()
        : Type(EObjectiveType::GoToLocation)
        , TargetLocation(FVector::ZeroVector)
        , RequiredQuantity(1)
        , CurrentQuantity(0)
        , bIsOptional(false)
        , bIsHidden(false)
    {}

    bool IsCompleted() const { return CurrentQuantity >= RequiredQuantity; }
};

/**
 * Quest reward structure
 */
USTRUCT(BlueprintType)
struct FQuestReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Credits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TMap<FString, int32> Items; // ItemID -> Quantity

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TMap<FString, float> FactionReputation; // FactionID -> ReputationChange

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> UnlockedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString SpecialRewardID;

    FQuestReward()
        : Credits(0)
        , ExperiencePoints(0)
    {}
};

/**
 * Quest data structure
 */
USTRUCT(BlueprintType)
struct FQuestData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuestObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuestReward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString GiverNPCID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RequiredFactionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float MinimumFactionReputation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredPlayerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> PrerequisiteQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit; // In seconds, 0 = no limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainStoryline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 TimesCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FDateTime AcceptedTime;

    FQuestData()
        : Type(EQuestType::Story)
        , Difficulty(EQuestDifficulty::Normal)
        , Status(EQuestStatus::NotStarted)
        , MinimumFactionReputation(0.0f)
        , RequiredPlayerLevel(1)
        , TimeLimit(0.0f)
        , TimeRemaining(0.0f)
        , bIsRepeatable(false)
        , bIsMainStoryline(false)
        , TimesCompleted(0)
    {}
};

/**
 * Quest generation parameters
 */
USTRUCT(BlueprintType)
struct FQuestGenerationParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestType PreferredType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestDifficulty TargetDifficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString FactionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector PlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 PlayerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> RecentQuestTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bAllowTimedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float MaxDistance;

    FQuestGenerationParams()
        : PreferredType(EQuestType::Combat)
        , TargetDifficulty(EQuestDifficulty::Normal)
        , PlayerLocation(FVector::ZeroVector)
        , PlayerLevel(1)
        , bAllowTimedMissions(true)
        , MaxDistance(100000.0f)
    {}
};

/**
 * Quest chain structure for story arcs
 */
USTRUCT(BlueprintType)
struct FQuestChain
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ChainID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText ChainTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> QuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentQuestIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuestChain()
        : CurrentQuestIndex(0)
        , bIsCompleted(false)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, const FString&, QuestID, EQuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveUpdated, const FString&, QuestID, const FString&, ObjectiveID, int32, NewProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestCompleted, const FString&, QuestID, const FQuestReward&, Reward);

/**
 * Quest System Component
 * Manages procedural quest generation, tracking, and completion
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UQuestSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuestData GenerateProceduralQuest(const FQuestGenerationParams& Params);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AcceptQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AbandonQuest(const FString& QuestID);

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuestObjective> GetActiveObjectives(const FString& QuestID) const;

    // Quest Queries
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuestData> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuestData> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuestData GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestCompleted(const FString& QuestID) const;

    // Quest Chain Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuestChain(const FString& ChainID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AdvanceQuestChain(const FString& ChainID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuestChain GetQuestChain(const FString& ChainID) const;

    // Dynamic Quest Generation
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GenerateQuestBoard(int32 NumQuests, const FQuestGenerationParams& Params);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RefreshQuestBoard();

    // Quest Validation
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CanAcceptQuest(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FText> GetQuestRequirements(const FString& QuestID) const;

    // Reward Distribution
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GrantQuestRewards(const FQuestReward& Reward);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnObjectiveUpdated OnObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnQuestCompleted OnQuestCompleted;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 QuestBoardSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float QuestBoardRefreshInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    bool bAutoGenerateQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    UDataTable* QuestTemplateTable;

private:
    // Quest storage
    UPROPERTY()
    TMap<FString, FQuestData> AllQuests;

    UPROPERTY()
    TMap<FString, FQuestChain> QuestChains;

    UPROPERTY()
    TArray<FString> ActiveQuestIDs;

    UPROPERTY()
    TArray<FString> CompletedQuestIDs;

    UPROPERTY()
    TArray<FQuestData> QuestBoard;

    float QuestBoardRefreshTimer;

    // Helper functions
    FQuestObjective GenerateObjective(EQuestType QuestType, const FQuestGenerationParams& Params);
    FQuestReward CalculateReward(EQuestDifficulty Difficulty, EQuestType Type, int32 NumObjectives);
    FString GenerateQuestID();
    FText GenerateQuestTitle(EQuestType Type);
    FText GenerateQuestDescription(const FQuestData& Quest);
    bool ArePrerequisitesMet(const FQuestData& Quest) const;
    void UpdateTimedQuests(float DeltaTime);
    void CheckObjectivePrerequisites(FQuestData& Quest);
    bool IsObjectiveAccessible(const FQuestData& Quest, const FQuestObjective& Objective) const;
    void TriggerQuestEvents(const FQuestData& Quest);
    float CalculateDifficultyMultiplier(EQuestDifficulty Difficulty) const;
    int32 GetRandomObjectiveCount(EQuestType Type, EQuestDifficulty Difficulty) const;
};
