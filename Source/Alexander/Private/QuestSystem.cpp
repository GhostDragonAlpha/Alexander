// Copyright Epic Games, Inc. All Rights Reserved.

#include "QuestSystem.h"
#include "EconomySystem.h"
#include "AIDialogueSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"

UQuestSystem::UQuestSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // 1 Hz for quest updates

    // Default configuration
    MaxActiveQuests = 10;
    QuestBoardSize = 5;
    QuestBoardRefreshInterval = 3600.0f; // 1 hour
    bAutoGenerateQuests = true;
    QuestBoardRefreshTimer = 0.0f;
}

void UQuestSystem::BeginPlay()
{
    Super::BeginPlay();

    // Generate initial quest board if auto-generate is enabled
    if (bAutoGenerateQuests && QuestBoardSize > 0)
    {
        FQuestGenerationParams DefaultParams;
        GenerateQuestBoard(QuestBoardSize, DefaultParams);
    }
}

void UQuestSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update timed quests
    UpdateTimedQuests(DeltaTime);

    // Update quest board refresh timer
    if (bAutoGenerateQuests)
    {
        QuestBoardRefreshTimer += DeltaTime;
        if (QuestBoardRefreshTimer >= QuestBoardRefreshInterval)
        {
            RefreshQuestBoard();
            QuestBoardRefreshTimer = 0.0f;
        }
    }
}

// ===== Quest Management =====

FQuestData UQuestSystem::GenerateProceduralQuest(const FQuestGenerationParams& Params)
{
    FQuestData NewQuest;

    // Generate unique ID
    NewQuest.QuestID = GenerateQuestID();

    // Set basic properties
    NewQuest.Type = Params.PreferredType;
    NewQuest.Difficulty = Params.TargetDifficulty;
    NewQuest.Status = EQuestStatus::Available;

    // Generate title and description
    NewQuest.Title = GenerateQuestTitle(Params.PreferredType);

    // Generate objectives based on type and difficulty
    int32 NumObjectives = GetRandomObjectiveCount(Params.PreferredType, Params.TargetDifficulty);
    for (int32 i = 0; i < NumObjectives; ++i)
    {
        FQuestObjective Objective = GenerateObjective(Params.PreferredType, Params);
        Objective.ObjectiveID = FString::Printf(TEXT("%s_Obj_%d"), *NewQuest.QuestID, i);
        NewQuest.Objectives.Add(Objective);
    }

    // Generate description based on objectives
    NewQuest.Description = GenerateQuestDescription(NewQuest);

    // Calculate rewards
    NewQuest.Reward = CalculateReward(Params.TargetDifficulty, Params.PreferredType, NumObjectives);

    // Set faction if specified
    NewQuest.RequiredFactionID = Params.FactionID;

    // Set level requirements
    NewQuest.RequiredPlayerLevel = FMath::Max(1, Params.PlayerLevel - 2);

    // Add time limit for timed missions if allowed
    if (Params.bAllowTimedMissions && FMath::RandRange(0.0f, 1.0f) < 0.3f)
    {
        NewQuest.TimeLimit = FMath::RandRange(600.0f, 3600.0f); // 10-60 minutes
        NewQuest.TimeRemaining = NewQuest.TimeLimit;
    }

    // Determine if repeatable (20% chance for non-story quests)
    if (NewQuest.Type != EQuestType::Story)
    {
        NewQuest.bIsRepeatable = FMath::RandRange(0.0f, 1.0f) < 0.2f;
    }

    return NewQuest;
}

bool UQuestSystem::AcceptQuest(const FString& QuestID)
{
    // Check if quest exists
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuestData& Quest = AllQuests[QuestID];

    // Check if can accept
    if (!CanAcceptQuest(QuestID))
    {
        return false;
    }

    // Check active quest limit
    if (ActiveQuestIDs.Num() >= MaxActiveQuests)
    {
        return false;
    }

    // Accept quest
    Quest.Status = EQuestStatus::Active;
    Quest.AcceptedTime = FDateTime::Now();
    ActiveQuestIDs.Add(QuestID);

    // Trigger quest events
    TriggerQuestEvents(Quest);

    // Broadcast status change
    OnQuestStatusChanged.Broadcast(QuestID, EQuestStatus::Active);

    return true;
}

bool UQuestSystem::CompleteQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuestData& Quest = AllQuests[QuestID];

    // Check if all required objectives are completed
    for (const FQuestObjective& Objective : Quest.Objectives)
    {
        if (!Objective.bIsOptional && !Objective.IsCompleted())
        {
            return false;
        }
    }

    // Complete quest
    Quest.Status = EQuestStatus::Completed;
    Quest.TimesCompleted++;
    ActiveQuestIDs.Remove(QuestID);

    if (!CompletedQuestIDs.Contains(QuestID))
    {
        CompletedQuestIDs.Add(QuestID);
    }

    // Grant rewards
    GrantQuestRewards(Quest.Reward);

    // Broadcast completion
    OnQuestCompleted.Broadcast(QuestID, Quest.Reward);
    OnQuestStatusChanged.Broadcast(QuestID, EQuestStatus::Completed);

    // Check for quest chain advancement
    for (auto& ChainPair : QuestChains)
    {
        FQuestChain& Chain = ChainPair.Value;
        if (Chain.CurrentQuestIndex < Chain.QuestIDs.Num() &&
            Chain.QuestIDs[Chain.CurrentQuestIndex] == QuestID)
        {
            AdvanceQuestChain(Chain.ChainID);
        }
    }

    return true;
}

bool UQuestSystem::FailQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuestData& Quest = AllQuests[QuestID];

    Quest.Status = EQuestStatus::Failed;
    ActiveQuestIDs.Remove(QuestID);

    OnQuestStatusChanged.Broadcast(QuestID, EQuestStatus::Failed);

    return true;
}

bool UQuestSystem::AbandonQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuestData& Quest = AllQuests[QuestID];

    // Can't abandon main storyline quests
    if (Quest.bIsMainStoryline)
    {
        return false;
    }

    Quest.Status = EQuestStatus::Abandoned;
    ActiveQuestIDs.Remove(QuestID);

    OnQuestStatusChanged.Broadcast(QuestID, EQuestStatus::Abandoned);

    return true;
}

// ===== Objective Management =====

bool UQuestSystem::UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuestData& Quest = AllQuests[QuestID];

    for (FQuestObjective& Objective : Quest.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            // Check if objective is accessible
            if (!IsObjectiveAccessible(Quest, Objective))
            {
                return false;
            }

            Objective.CurrentQuantity += Progress;
            Objective.CurrentQuantity = FMath::Clamp(Objective.CurrentQuantity, 0, Objective.RequiredQuantity);

            OnObjectiveUpdated.Broadcast(QuestID, ObjectiveID, Objective.CurrentQuantity);

            // Check if objective completed
            if (Objective.IsCompleted())
            {
                CheckObjectivePrerequisites(Quest);

                // Check if all objectives completed
                bool bAllCompleted = true;
                for (const FQuestObjective& Obj : Quest.Objectives)
                {
                    if (!Obj.bIsOptional && !Obj.IsCompleted())
                    {
                        bAllCompleted = false;
                        break;
                    }
                }

                if (bAllCompleted)
                {
                    CompleteQuest(QuestID);
                }
            }

            return true;
        }
    }

    return false;
}

bool UQuestSystem::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuestData& Quest = AllQuests[QuestID];

    for (FQuestObjective& Objective : Quest.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.CurrentQuantity = Objective.RequiredQuantity;
            OnObjectiveUpdated.Broadcast(QuestID, ObjectiveID, Objective.CurrentQuantity);

            CheckObjectivePrerequisites(Quest);

            return true;
        }
    }

    return false;
}

TArray<FQuestObjective> UQuestSystem::GetActiveObjectives(const FString& QuestID) const
{
    TArray<FQuestObjective> ActiveObjectives;

    if (AllQuests.Contains(QuestID))
    {
        const FQuestData& Quest = AllQuests[QuestID];

        for (const FQuestObjective& Objective : Quest.Objectives)
        {
            if (IsObjectiveAccessible(Quest, Objective) && !Objective.IsCompleted() && !Objective.bIsHidden)
            {
                ActiveObjectives.Add(Objective);
            }
        }
    }

    return ActiveObjectives;
}

// ===== Quest Queries =====

TArray<FQuestData> UQuestSystem::GetActiveQuests() const
{
    TArray<FQuestData> ActiveQuests;

    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (AllQuests.Contains(QuestID))
        {
            ActiveQuests.Add(AllQuests[QuestID]);
        }
    }

    return ActiveQuests;
}

TArray<FQuestData> UQuestSystem::GetAvailableQuests() const
{
    TArray<FQuestData> AvailableQuests;

    for (const auto& QuestPair : AllQuests)
    {
        if (QuestPair.Value.Status == EQuestStatus::Available && CanAcceptQuest(QuestPair.Key))
        {
            AvailableQuests.Add(QuestPair.Value);
        }
    }

    return AvailableQuests;
}

TArray<FQuestData> UQuestSystem::GetCompletedQuests() const
{
    TArray<FQuestData> CompletedQuests;

    for (const FString& QuestID : CompletedQuestIDs)
    {
        if (AllQuests.Contains(QuestID))
        {
            CompletedQuests.Add(AllQuests[QuestID]);
        }
    }

    return CompletedQuests;
}

FQuestData UQuestSystem::GetQuestData(const FString& QuestID) const
{
    if (AllQuests.Contains(QuestID))
    {
        return AllQuests[QuestID];
    }

    return FQuestData();
}

bool UQuestSystem::IsQuestActive(const FString& QuestID) const
{
    return ActiveQuestIDs.Contains(QuestID);
}

bool UQuestSystem::IsQuestCompleted(const FString& QuestID) const
{
    return CompletedQuestIDs.Contains(QuestID);
}

// ===== Quest Chain Management =====

bool UQuestSystem::StartQuestChain(const FString& ChainID)
{
    if (!QuestChains.Contains(ChainID))
    {
        return false;
    }

    FQuestChain& Chain = QuestChains[ChainID];

    if (Chain.QuestIDs.Num() == 0)
    {
        return false;
    }

    // Accept first quest in chain
    return AcceptQuest(Chain.QuestIDs[0]);
}

bool UQuestSystem::AdvanceQuestChain(const FString& ChainID)
{
    if (!QuestChains.Contains(ChainID))
    {
        return false;
    }

    FQuestChain& Chain = QuestChains[ChainID];

    Chain.CurrentQuestIndex++;

    if (Chain.CurrentQuestIndex >= Chain.QuestIDs.Num())
    {
        Chain.bIsCompleted = true;
        return true;
    }

    // Make next quest available
    FString NextQuestID = Chain.QuestIDs[Chain.CurrentQuestIndex];
    if (AllQuests.Contains(NextQuestID))
    {
        AllQuests[NextQuestID].Status = EQuestStatus::Available;
    }

    return true;
}

FQuestChain UQuestSystem::GetQuestChain(const FString& ChainID) const
{
    if (QuestChains.Contains(ChainID))
    {
        return QuestChains[ChainID];
    }

    return FQuestChain();
}

// ===== Dynamic Quest Generation =====

void UQuestSystem::GenerateQuestBoard(int32 NumQuests, const FQuestGenerationParams& Params)
{
    QuestBoard.Empty();

    for (int32 i = 0; i < NumQuests; ++i)
    {
        // Vary quest types for diversity
        FQuestGenerationParams ModifiedParams = Params;

        // Random type selection weighted by player preferences
        TArray<EQuestType> PossibleTypes = {
            EQuestType::Combat,
            EQuestType::Delivery,
            EQuestType::Exploration,
            EQuestType::Escort,
            EQuestType::Mining,
            EQuestType::Salvage,
            EQuestType::Reconnaissance,
            EQuestType::Rescue,
            EQuestType::Bounty
        };

        ModifiedParams.PreferredType = PossibleTypes[FMath::RandRange(0, PossibleTypes.Num() - 1)];

        // Generate quest
        FQuestData NewQuest = GenerateProceduralQuest(ModifiedParams);

        // Add to board and quest storage
        QuestBoard.Add(NewQuest);
        AllQuests.Add(NewQuest.QuestID, NewQuest);
    }
}

void UQuestSystem::RefreshQuestBoard()
{
    // Remove old non-active quests from board
    for (const FQuestData& Quest : QuestBoard)
    {
        if (!IsQuestActive(Quest.QuestID))
        {
            AllQuests.Remove(Quest.QuestID);
        }
    }

    // Generate new board
    FQuestGenerationParams DefaultParams;
    if (AActor* Owner = GetOwner())
    {
        DefaultParams.PlayerLocation = Owner->GetActorLocation();
    }

    GenerateQuestBoard(QuestBoardSize, DefaultParams);
}

// ===== Quest Validation =====

bool UQuestSystem::CanAcceptQuest(const FString& QuestID) const
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    const FQuestData& Quest = AllQuests[QuestID];

    // Check if already active or completed (and not repeatable)
    if (ActiveQuestIDs.Contains(QuestID))
    {
        return false;
    }

    if (CompletedQuestIDs.Contains(QuestID) && !Quest.bIsRepeatable)
    {
        return false;
    }

    // Check prerequisites
    return ArePrerequisitesMet(Quest);
}

TArray<FText> UQuestSystem::GetQuestRequirements(const FString& QuestID) const
{
    TArray<FText> Requirements;

    if (!AllQuests.Contains(QuestID))
    {
        return Requirements;
    }

    const FQuestData& Quest = AllQuests[QuestID];

    if (Quest.RequiredPlayerLevel > 1)
    {
        Requirements.Add(FText::FromString(FString::Printf(TEXT("Required Level: %d"), Quest.RequiredPlayerLevel)));
    }

    if (!Quest.RequiredFactionID.IsEmpty())
    {
        Requirements.Add(FText::FromString(FString::Printf(TEXT("Required Faction: %s"), *Quest.RequiredFactionID)));
        if (Quest.MinimumFactionReputation > 0.0f)
        {
            Requirements.Add(FText::FromString(FString::Printf(TEXT("Minimum Reputation: %.1f"), Quest.MinimumFactionReputation)));
        }
    }

    if (Quest.PrerequisiteQuestIDs.Num() > 0)
    {
        Requirements.Add(FText::FromString(TEXT("Required Quests: Complete prerequisite quests")));
    }

    return Requirements;
}

// ===== Reward Distribution =====

void UQuestSystem::GrantQuestRewards(const FQuestReward& Reward)
{
    // Get economy system for credit rewards
    if (UEconomySystem* EconomySystem = Cast<UEconomySystem>(GetOwner()->GetComponentByClass(UEconomySystem::StaticClass())))
    {
        if (Reward.Credits > 0)
        {
            // EconomySystem->AddCredits(Reward.Credits);
        }

        // Grant items
        for (const auto& ItemPair : Reward.Items)
        {
            // EconomySystem->AddItem(ItemPair.Key, ItemPair.Value);
        }
    }

    // Update faction reputation
    for (const auto& ReputationPair : Reward.FactionReputation)
    {
        // FactionSystem->ModifyReputation(ReputationPair.Key, ReputationPair.Value);
    }

    // Unlock follow-up quests
    for (const FString& QuestID : Reward.UnlockedQuests)
    {
        if (AllQuests.Contains(QuestID))
        {
            AllQuests[QuestID].Status = EQuestStatus::Available;
        }
    }
}

// ===== Helper Functions =====

FQuestObjective UQuestSystem::GenerateObjective(EQuestType QuestType, const FQuestGenerationParams& Params)
{
    FQuestObjective Objective;

    switch (QuestType)
    {
    case EQuestType::Combat:
    case EQuestType::Assassination:
    case EQuestType::Bounty:
        Objective.Type = EObjectiveType::KillTarget;
        Objective.RequiredQuantity = FMath::RandRange(5, 20);
        Objective.Description = FText::FromString(FString::Printf(TEXT("Eliminate %d enemies"), Objective.RequiredQuantity));
        break;

    case EQuestType::Delivery:
    case EQuestType::Smuggling:
        Objective.Type = EObjectiveType::DeliverItem;
        Objective.RequiredQuantity = FMath::RandRange(1, 5);
        Objective.Description = FText::FromString(TEXT("Deliver cargo to destination"));
        break;

    case EQuestType::Exploration:
        Objective.Type = EObjectiveType::GoToLocation;
        Objective.RequiredQuantity = FMath::RandRange(1, 3);
        Objective.Description = FText::FromString(TEXT("Explore marked location"));
        break;

    case EQuestType::Escort:
        Objective.Type = EObjectiveType::EscortTarget;
        Objective.RequiredQuantity = 1;
        Objective.Description = FText::FromString(TEXT("Escort target safely"));
        break;

    case EQuestType::Mining:
        Objective.Type = EObjectiveType::MineResource;
        Objective.RequiredQuantity = FMath::RandRange(50, 200);
        Objective.Description = FText::FromString(FString::Printf(TEXT("Mine %d units of resources"), Objective.RequiredQuantity));
        break;

    case EQuestType::Salvage:
        Objective.Type = EObjectiveType::CollectItem;
        Objective.RequiredQuantity = FMath::RandRange(5, 15);
        Objective.Description = FText::FromString(TEXT("Salvage ship components"));
        break;

    case EQuestType::Reconnaissance:
        Objective.Type = EObjectiveType::ScanObject;
        Objective.RequiredQuantity = FMath::RandRange(3, 10);
        Objective.Description = FText::FromString(TEXT("Scan designated targets"));
        break;

    case EQuestType::Rescue:
        Objective.Type = EObjectiveType::EscortTarget;
        Objective.RequiredQuantity = FMath::RandRange(1, 5);
        Objective.Description = FText::FromString(TEXT("Rescue stranded personnel"));
        break;

    default:
        Objective.Type = EObjectiveType::GoToLocation;
        Objective.RequiredQuantity = 1;
        Objective.Description = FText::FromString(TEXT("Complete mission objective"));
        break;
    }

    // Generate target location within reasonable distance
    if (Params.MaxDistance > 0.0f)
    {
        FVector RandomDirection = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-0.5f, 0.5f)
        ).GetSafeNormal();

        float Distance = FMath::RandRange(Params.MaxDistance * 0.3f, Params.MaxDistance);
        Objective.TargetLocation = Params.PlayerLocation + (RandomDirection * Distance);
    }

    return Objective;
}

FQuestReward UQuestSystem::CalculateReward(EQuestDifficulty Difficulty, EQuestType Type, int32 NumObjectives)
{
    FQuestReward Reward;

    float DifficultyMultiplier = CalculateDifficultyMultiplier(Difficulty);

    // Base rewards
    int32 BaseCredits = 1000;
    int32 BaseXP = 500;

    // Apply multipliers
    Reward.Credits = FMath::RoundToInt(BaseCredits * DifficultyMultiplier * NumObjectives);
    Reward.ExperiencePoints = FMath::RoundToInt(BaseXP * DifficultyMultiplier * NumObjectives);

    // Add bonus rewards for certain types
    if (Type == EQuestType::Combat || Type == EQuestType::Assassination)
    {
        Reward.Credits = FMath::RoundToInt(Reward.Credits * 1.2f);
    }
    else if (Type == EQuestType::Exploration)
    {
        Reward.ExperiencePoints = FMath::RoundToInt(Reward.ExperiencePoints * 1.3f);
    }

    return Reward;
}

FString UQuestSystem::GenerateQuestID()
{
    static int32 QuestCounter = 0;
    return FString::Printf(TEXT("Quest_%d_%lld"), ++QuestCounter, FDateTime::Now().GetTicks());
}

FText UQuestSystem::GenerateQuestTitle(EQuestType Type)
{
    TArray<FString> Prefixes = { TEXT("Urgent"), TEXT("Critical"), TEXT("Routine"), TEXT("Priority"), TEXT("Emergency") };
    FString Prefix = Prefixes[FMath::RandRange(0, Prefixes.Num() - 1)];

    FString TypeName;
    switch (Type)
    {
    case EQuestType::Combat: TypeName = TEXT("Combat Operation"); break;
    case EQuestType::Delivery: TypeName = TEXT("Delivery Mission"); break;
    case EQuestType::Exploration: TypeName = TEXT("Exploration Contract"); break;
    case EQuestType::Escort: TypeName = TEXT("Escort Duty"); break;
    case EQuestType::Mining: TypeName = TEXT("Mining Operation"); break;
    case EQuestType::Salvage: TypeName = TEXT("Salvage Operation"); break;
    case EQuestType::Assassination: TypeName = TEXT("Elimination Contract"); break;
    case EQuestType::Reconnaissance: TypeName = TEXT("Recon Mission"); break;
    case EQuestType::Rescue: TypeName = TEXT("Rescue Operation"); break;
    case EQuestType::Bounty: TypeName = TEXT("Bounty Hunt"); break;
    default: TypeName = TEXT("Mission"); break;
    }

    return FText::FromString(FString::Printf(TEXT("%s: %s"), *Prefix, *TypeName));
}

FText UQuestSystem::GenerateQuestDescription(const FQuestData& Quest)
{
    FString Description = TEXT("Mission Briefing:\n\n");

    for (int32 i = 0; i < Quest.Objectives.Num(); ++i)
    {
        Description += FString::Printf(TEXT("%d. %s\n"), i + 1, *Quest.Objectives[i].Description.ToString());
    }

    if (Quest.TimeLimit > 0.0f)
    {
        int32 Minutes = FMath::FloorToInt(Quest.TimeLimit / 60.0f);
        Description += FString::Printf(TEXT("\nTime Limit: %d minutes"), Minutes);
    }

    return FText::FromString(Description);
}

bool UQuestSystem::ArePrerequisitesMet(const FQuestData& Quest) const
{
    // Check prerequisite quests
    for (const FString& PrereqID : Quest.PrerequisiteQuestIDs)
    {
        if (!CompletedQuestIDs.Contains(PrereqID))
        {
            return false;
        }
    }

    // Additional checks (level, faction, etc.) would go here
    // For now, return true if prerequisite quests are met

    return true;
}

void UQuestSystem::UpdateTimedQuests(float DeltaTime)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (AllQuests.Contains(QuestID))
        {
            FQuestData& Quest = AllQuests[QuestID];

            if (Quest.TimeLimit > 0.0f)
            {
                Quest.TimeRemaining -= DeltaTime;

                if (Quest.TimeRemaining <= 0.0f)
                {
                    FailQuest(QuestID);
                }
            }
        }
    }
}

void UQuestSystem::CheckObjectivePrerequisites(FQuestData& Quest)
{
    // Check if any objectives have their prerequisites met
    for (FQuestObjective& Objective : Quest.Objectives)
    {
        if (Objective.bIsHidden && Objective.PrerequisiteObjectiveIDs.Num() > 0)
        {
            bool bAllPrereqsMet = true;

            for (const FString& PrereqID : Objective.PrerequisiteObjectiveIDs)
            {
                bool bPrereqComplete = false;

                for (const FQuestObjective& OtherObjective : Quest.Objectives)
                {
                    if (OtherObjective.ObjectiveID == PrereqID && OtherObjective.IsCompleted())
                    {
                        bPrereqComplete = true;
                        break;
                    }
                }

                if (!bPrereqComplete)
                {
                    bAllPrereqsMet = false;
                    break;
                }
            }

            if (bAllPrereqsMet)
            {
                Objective.bIsHidden = false;
            }
        }
    }
}

bool UQuestSystem::IsObjectiveAccessible(const FQuestData& Quest, const FQuestObjective& Objective) const
{
    // Check if objective prerequisites are met
    for (const FString& PrereqID : Objective.PrerequisiteObjectiveIDs)
    {
        bool bPrereqComplete = false;

        for (const FQuestObjective& OtherObjective : Quest.Objectives)
        {
            if (OtherObjective.ObjectiveID == PrereqID && OtherObjective.IsCompleted())
            {
                bPrereqComplete = true;
                break;
            }
        }

        if (!bPrereqComplete)
        {
            return false;
        }
    }

    return true;
}

void UQuestSystem::TriggerQuestEvents(const FQuestData& Quest)
{
    // Trigger dialogue or narrative events based on quest type
    if (UAIDialogueSystem* DialogueSystem = Cast<UAIDialogueSystem>(GetOwner()->GetComponentByClass(UAIDialogueSystem::StaticClass())))
    {
        // DialogueSystem->TriggerQuestDialogue(Quest.QuestID);
    }

    // Additional event triggering logic
}

float UQuestSystem::CalculateDifficultyMultiplier(EQuestDifficulty Difficulty) const
{
    switch (Difficulty)
    {
    case EQuestDifficulty::Trivial: return 0.5f;
    case EQuestDifficulty::Easy: return 0.75f;
    case EQuestDifficulty::Normal: return 1.0f;
    case EQuestDifficulty::Hard: return 1.5f;
    case EQuestDifficulty::VeryHard: return 2.0f;
    case EQuestDifficulty::Elite: return 3.0f;
    case EQuestDifficulty::Legendary: return 5.0f;
    default: return 1.0f;
    }
}

int32 UQuestSystem::GetRandomObjectiveCount(EQuestType Type, EQuestDifficulty Difficulty) const
{
    int32 BaseCount = 1;

    // More complex quest types have more objectives
    switch (Type)
    {
    case EQuestType::Story:
    case EQuestType::Exploration:
        BaseCount = FMath::RandRange(2, 4);
        break;
    case EQuestType::Combat:
    case EQuestType::Mining:
    case EQuestType::Salvage:
        BaseCount = FMath::RandRange(1, 3);
        break;
    default:
        BaseCount = FMath::RandRange(1, 2);
        break;
    }

    // Harder quests may have more objectives
    if (Difficulty >= EQuestDifficulty::Hard)
    {
        BaseCount += 1;
    }
    if (Difficulty >= EQuestDifficulty::Elite)
    {
        BaseCount += 1;
    }

    return FMath::Clamp(BaseCount, 1, 5);
}
