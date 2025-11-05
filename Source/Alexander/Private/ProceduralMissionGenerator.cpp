// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProceduralMissionGenerator.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"
#include "Math/UnrealMathUtility.h"

UProceduralMissionGenerator::UProceduralMissionGenerator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;

    // Configuration defaults
    bEnableDynamicGeneration = true;
    bEnableWorldEvents = true;
    bEnableMissionChains = true;
    bEnableFactionMissions = true;
    GenerationInterval = DEFAULT_GENERATION_INTERVAL;
    MaxMissionsPerInterval = DEFAULT_MAX_MISSIONS_PER_INTERVAL;
    DifficultyScalingFactor = DEFAULT_DIFFICULTY_SCALING;
    RewardScalingFactor = DEFAULT_REWARD_SCALING;
    MaxMissionChainsPerPlayer = DEFAULT_MAX_MISSION_CHAINS_PER_PLAYER;
}

void UProceduralMissionGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultTemplates();
    InitializeDefaultObjectiveTemplates();
    InitializeDefaultRules();
    SetupTimers();
    
    UE_LOG(LogTemp, Log, TEXT("ProceduralMissionGenerator initialized with %d templates"), MissionTemplates.Num());
}

void UProceduralMissionGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableWorldEvents)
    {
        UpdateWorldEvents(DeltaTime);
    }
}

void UProceduralMissionGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(GenerationTimer);
        World->GetTimerManager().ClearTimer(WorldEventTimer);
        World->GetTimerManager().ClearTimer(RefreshTimer);
    }

    // Clear all data
    MissionTemplates.Empty();
    TemplateMap.Empty();
    ObjectiveTemplates.Empty();
    ObjectiveTemplateMap.Empty();
    MissionChains.Empty();
    ChainMap.Empty();
    MissionToChainMap.Empty();
    GenerationRules.Empty();
    RuleMap.Empty();
    ActiveWorldEvents.Empty();
    EventStartTimes.Empty();
    EventParameters.Empty();

    Super::EndPlay(EndPlayReason);
}

// ===== Mission Generation =====

FMissionData UProceduralMissionGenerator::GenerateMission(const FMissionGenerationContext& Context)
{
    FMissionTemplate* SelectedTemplate = SelectTemplate(Context);
    if (!SelectedTemplate)
    {
        UE_LOG(LogTemp, Warning, TEXT("No suitable mission template found for context"));
        return FMissionData();
    }
    
    return CreateMissionFromTemplate(*SelectedTemplate, Context);
}

TArray<FMissionData> UProceduralMissionGenerator::GenerateMissionBatch(const FMissionGenerationContext& Context, int32 Count)
{
    TArray<FMissionData> GeneratedMissions;
    
    for (int32 i = 0; i < Count; i++)
    {
        FMissionData Mission = GenerateMission(Context);
        if (Mission.MissionID != FName())
        {
            GeneratedMissions.Add(Mission);
        }
    }
    
    return GeneratedMissions;
}

FMissionData UProceduralMissionGenerator::GenerateMissionFromTemplate(const FName& TemplateName, const FMissionGenerationContext& Context)
{
    FMissionTemplate* Template = GetMissionTemplate(TemplateName);
    if (!Template)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mission template '%s' not found"), *TemplateName.ToString());
        return FMissionData();
    }
    
    return CreateMissionFromTemplate(*Template, Context);
}

FMissionChain UProceduralMissionGenerator::GenerateMissionChain(const TArray<FName>& TemplateNames, const FMissionGenerationContext& Context)
{
    FMissionChain Chain;
    Chain.ChainName = FName(*FString::Printf(TEXT("Chain_%s_%d"), *Context.Player->GetPlayerName(), FMath::RandRange(1000, 9999)));
    Chain.MissionIDs = TemplateNames;
    Chain.bIsSequential = true;
    Chain.ChainBonusMultiplier = 1.5f;
    
    // Generate mission order
    for (int32 i = 0; i < TemplateNames.Num(); i++)
    {
        Chain.MissionOrder.Add(i);
    }
    
    // Shuffle order if not sequential
    if (!Chain.bIsSequential)
    {
        for (int32 i = Chain.MissionOrder.Num() - 1; i > 0; i--)
        {
            int32 j = FMath::RandRange(0, i);
            Chain.MissionOrder.Swap(i, j);
        }
    }
    
    OnMissionChainGenerated.Broadcast(Chain, Context);
    
    return Chain;
}

// ===== Template Management =====

void UProceduralMissionGenerator::RegisterMissionTemplate(const FMissionTemplate& Template)
{
    if (TemplateMap.Contains(Template.TemplateName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Mission template '%s' already registered"), *Template.TemplateName.ToString());
        return;
    }
    
    MissionTemplates.Add(Template);
    TemplateMap.Add(Template.TemplateName, Template);
    
    UE_LOG(LogTemp, Log, TEXT("Registered mission template: %s"), *Template.TemplateName.ToString());
}

void UProceduralMissionGenerator::UnregisterMissionTemplate(const FName& TemplateName)
{
    if (!TemplateMap.Contains(TemplateName))
    {
        return;
    }
    
    TemplateMap.Remove(TemplateName);
    
    for (int32 i = 0; i < MissionTemplates.Num(); i++)
    {
        if (MissionTemplates[i].TemplateName == TemplateName)
        {
            MissionTemplates.RemoveAt(i);
            break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Unregistered mission template: %s"), *TemplateName.ToString());
}

FMissionTemplate* UProceduralMissionGenerator::GetMissionTemplate(const FName& TemplateName)
{
    return TemplateMap.Find(TemplateName);
}

TArray<FMissionTemplate> UProceduralMissionGenerator::GetAllTemplates() const
{
    return MissionTemplates;
}

TArray<FMissionTemplate> UProceduralMissionGenerator::GetTemplatesByType(EMissionTemplateType Type) const
{
    TArray<FMissionTemplate> FilteredTemplates;
    
    for (const FMissionTemplate& Template : MissionTemplates)
    {
        if (Template.TemplateType == Type)
        {
            FilteredTemplates.Add(Template);
        }
    }
    
    return FilteredTemplates;
}

TArray<FMissionTemplate> UProceduralMissionGenerator::GetTemplatesByContext(EMissionContext Context) const
{
    TArray<FMissionTemplate> FilteredTemplates;
    
    for (const FMissionTemplate& Template : MissionTemplates)
    {
        if (Template.MissionContext == Context)
        {
            FilteredTemplates.Add(Template);
        }
    }
    
    return FilteredTemplates;
}

// ===== Objective Generation =====

TArray<FMissionObjective> UProceduralMissionGenerator::GenerateObjectives(const FMissionTemplate& Template, const FMissionGenerationContext& Context)
{
    TArray<FMissionObjective> Objectives;
    
    int32 ObjectiveCount = FMath::RandRange(Template.MinObjectives, Template.MaxObjectives);
    
    for (int32 i = 0; i < ObjectiveCount; i++)
    {
        // Select objective type based on template
        FName ObjectiveType = TEXT("Primary"); // Default
        
        if (i > 0)
        {
            ObjectiveType = TEXT("Secondary");
        }
        
        // Get objective template
        TArray<FMissionObjectiveTemplate> AvailableObjectives = GetObjectiveTemplatesByType(ObjectiveType);
        if (AvailableObjectives.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, AvailableObjectives.Num() - 1);
            FMissionObjectiveTemplate ObjectiveTemplate = AvailableObjectives[RandomIndex];
            
            FMissionObjective Objective = GenerateObjective(ObjectiveTemplate, Context);
            Objectives.Add(Objective);
        }
    }
    
    return Objectives;
}

FMissionBoardObjective UProceduralMissionGenerator::GenerateObjective(const FMissionObjectiveTemplate& ObjectiveTemplate, const FMissionGenerationContext& Context)
{
    FMissionBoardObjective Objective;
    Objective.ObjectiveID = FName(*FString::Printf(TEXT("%s_%d"), *ObjectiveTemplate.ObjectiveType.ToString(), FMath::RandRange(1000, 9999)));
    Objective.Description = ProcessObjectiveTemplate(ObjectiveTemplate.DescriptionTemplate, ObjectiveTemplate.Parameters, Context);
    Objective.bIsCompleted = false;
    Objective.bIsRequired = ObjectiveTemplate.bIsRequired;
    Objective.TargetCount = FMath::RandRange(1, 5);
    Objective.CurrentCount = 0;
    Objective.TimeLimit = FMath::FRandRange(300.0f, 3600.0f); // 5 minutes to 1 hour
    Objective.TimeRemaining = Objective.TimeLimit;
    
    return Objective;
}

void UProceduralMissionGenerator::RegisterObjectiveTemplate(const FMissionObjectiveTemplate& ObjectiveTemplate)
{
    if (!ObjectiveTemplateMap.Contains(ObjectiveTemplate.ObjectiveType))
    {
        ObjectiveTemplateMap.Add(ObjectiveTemplate.ObjectiveType, TArray<FMissionObjectiveTemplate>());
    }
    
    ObjectiveTemplateMap[ObjectiveTemplate.ObjectiveType].Add(ObjectiveTemplate);
    ObjectiveTemplates.Add(ObjectiveTemplate);
}

TArray<FMissionObjectiveTemplate> UProceduralMissionGenerator::GetObjectiveTemplatesByType(const FName& ObjectiveType) const
{
    if (const TArray<FMissionObjectiveTemplate>* Templates = ObjectiveTemplateMap.Find(ObjectiveType))
    {
        return *Templates;
    }
    
    return TArray<FMissionObjectiveTemplate>();
}

// ===== Dynamic Event Generation =====

void UProceduralMissionGenerator::GenerateWorldEvent(const FMissionGenerationContext& Context)
{
    TArray<FName> PossibleEvents = {TEXT("PirateAttack"), TEXT("TradeConvoy"), TEXT("ResearchDiscovery"), TEXT("DistressSignal"), TEXT("AnomalyDetection")};
    
    if (PossibleEvents.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, PossibleEvents.Num() - 1);
        FName EventName = PossibleEvents[RandomIndex];
        
        ActiveWorldEvents.AddUnique(EventName);
        EventStartTimes.Add(EventName, GetWorld()->GetTimeSeconds());
        
        // Add event parameters
        TMap<FName, FString> Parameters;
        Parameters.Add(TEXT("Location"), FString::Printf(TEXT("%.1f,%.1f"), Context.PlayerLocation.X, Context.PlayerLocation.Y));
        Parameters.Add(TEXT("Intensity"), FString::Printf(TEXT("%.1f"), FMath::FRandRange(0.3f, 1.0f)));
        EventParameters.Add(EventName, Parameters);
        
        OnWorldEventGenerated.Broadcast(EventName);
        
        UE_LOG(LogTemp, Log, TEXT("Generated world event: %s"), *EventName.ToString());
    }
}

void UProceduralMissionGenerator::UpdateWorldEvents(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update existing events
    for (int32 i = ActiveWorldEvents.Num() - 1; i >= 0; i--)
    {
        FName EventName = ActiveWorldEvents[i];
        ProcessWorldEvent(EventName, DeltaTime);
        
        // Check if event should end (simplified - events last 10 minutes)
        if (CurrentTime - EventStartTimes[EventName] >= 600.0f)
        {
            EndWorldEvent(EventName);
        }
    }
    
    // Generate new events
    static float EventAccumulator = 0.0f;
    EventAccumulator += DeltaTime;
    
    if (EventAccumulator >= WORLD_EVENT_INTERVAL)
    {
        GenerateRandomWorldEvent();
        EventAccumulator = 0.0f;
    }
}

bool UProceduralMissionGenerator::IsWorldEventActive(const FName& EventName) const
{
    return ActiveWorldEvents.Contains(EventName);
}

// ===== Mission Scaling =====

FMissionData UProceduralMissionGenerator::ScaleMissionForPlayer(const FMissionData& BaseMission, const FMissionGenerationContext& Context)
{
    FMissionData ScaledMission = BaseMission;
    
    // Scale difficulty based on player level
    float LevelMultiplier = 1.0f + (Context.PlayerLevel - 1) * 0.1f;
    
    // Scale objectives
    for (FMissionBoardObjective& Objective : ScaledMission.Objectives)
    {
        Objective.TargetCount = FMath::RoundToInt(Objective.TargetCount * LevelMultiplier);
        Objective.TimeLimit = Objective.TimeLimit * (1.0f + (LevelMultiplier - 1.0f) * 0.2f);
    }
    
    // Scale rewards
    ScaledMission.Reward.Credits = FMath::RoundToInt(ScaledMission.Reward.Credits * LevelMultiplier);
    ScaledMission.Reward.Experience = FMath::RoundToInt(ScaledMission.Reward.Experience * LevelMultiplier);
    
    return ScaledMission;
}

FMissionReward UProceduralMissionGenerator::ScaleMissionRewards(const FMissionReward& BaseReward, int32 PlayerLevel, EMissionDifficulty Difficulty)
{
    FMissionReward ScaledReward = BaseReward;
    
    // Calculate scaling factors
    float LevelMultiplier = 1.0f + (PlayerLevel - 1) * 0.15f;
    float DifficultyMultiplier = 1.0f + ((int32)Difficulty * 0.5f);
    
    // Apply scaling
    ScaledReward.Credits = FMath::RoundToInt(BaseReward.Credits * LevelMultiplier * DifficultyMultiplier * RewardScalingFactor);
    ScaledReward.Experience = FMath::RoundToInt(BaseReward.Experience * LevelMultiplier * DifficultyMultiplier * RewardScalingFactor);
    
    return ScaledReward;
}

float UProceduralMissionGenerator::CalculateMissionDifficulty(const FMissionGenerationContext& Context, const FMissionTemplate& Template)
{
    float BaseDifficulty = (int32)Template.Complexity * 0.2f;
    
    // Player level adjustment
    float LevelDifficulty = Context.PlayerLevel * 0.05f;
    
    // Reputation adjustment
    float ReputationDifficulty = 0.0f;
    for (const auto& ReputationPair : Context.PlayerReputation)
    {
        ReputationDifficulty += ReputationPair.Value * 0.02f;
    }
    
    // Context adjustment
    float ContextDifficulty = 0.0f;
    switch (Template.MissionContext)
    {
    case EMissionContext::Space:
        ContextDifficulty = 0.1f;
        break;
    case EMissionContext::Combat:
        ContextDifficulty = 0.3f;
        break;
    case EMissionContext::Anomaly:
        ContextDifficulty = 0.4f;
        break;
    default:
        ContextDifficulty = 0.0f;
        break;
    }
    
    return FMath::Clamp(BaseDifficulty + LevelDifficulty + ReputationDifficulty + ContextDifficulty, 0.0f, 1.0f);
}

int32 UProceduralMissionGenerator::CalculateRecommendedLevel(const FMissionGenerationContext& Context, EMissionDifficulty TargetDifficulty)
{
    float BaseLevel = Context.PlayerLevel;
    
    // Adjust based on target difficulty
    switch (TargetDifficulty)
    {
    case EMissionDifficulty::Easy:
        BaseLevel -= 2;
        break;
    case EMissionDifficulty::Normal:
        BaseLevel += 0;
        break;
    case EMissionDifficulty::Hard:
        BaseLevel += 3;
        break;
    case EMissionDifficulty::VeryHard:
        BaseLevel += 5;
        break;
    case EMissionDifficulty::Extreme:
        BaseLevel += 8;
        break;
    case EMissionDifficulty::Legendary:
        BaseLevel += 12;
        break;
    default:
        break;
    }
    
    return FMath::Max(1, FMath::RoundToInt(BaseLevel));
}

// ===== Faction-Based Missions =====

TArray<FMissionTemplate> UProceduralMissionGenerator::GetFactionSpecificTemplates(EMissionFaction Faction) const
{
    TArray<FMissionTemplate> FilteredTemplates;
    
    for (const FMissionTemplate& Template : MissionTemplates)
    {
        if (Template.AllowedFactions.Contains(Faction))
        {
            FilteredTemplates.Add(Template);
        }
    }
    
    return FilteredTemplates;
}

FMissionData UProceduralMissionGenerator::GenerateFactionMission(EMissionFaction Faction, const FMissionGenerationContext& Context)
{
    TArray<FMissionTemplate> FactionTemplates = GetFactionSpecificTemplates(Faction);
    
    if (FactionTemplates.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No templates available for faction %d"), (int32)Faction);
        return FMissionData();
    }
    
    int32 RandomIndex = FMath::RandRange(0, FactionTemplates.Num() - 1);
    return CreateMissionFromTemplate(FactionTemplates[RandomIndex], Context);
}

bool UProceduralMissionGenerator::CanPlayerAcceptFactionMission(EMissionFaction Faction, const FMissionGenerationContext& Context)
{
    if (!bEnableFactionMissions)
    {
        return false;
    }
    
    // Check player reputation with faction
    if (Context.PlayerReputation.Contains(Faction))
    {
        float Reputation = Context.PlayerReputation[Faction];
        return Reputation >= -0.5f; // Minimum reputation threshold
    }
    
    return true; // Neutral reputation
}

void UProceduralMissionGenerator::UpdateFactionRelationships(const FMissionData& Mission, bool bCompleted)
{
    // Update faction reputation based on mission completion
    float ReputationChange = bCompleted ? 0.1f : -0.05f;
    
    // This would interface with a faction system
    // For now, just log the change
    UE_LOG(LogTemp, Log, TEXT("Faction reputation change: %.2f for mission %s"), ReputationChange, *Mission.MissionID.ToString());
}

// ===== Mission Chain System =====

void UProceduralMissionGenerator::RegisterMissionChain(const FMissionChain& Chain)
{
    if (ChainMap.Contains(Chain.ChainName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Mission chain '%s' already registered"), *Chain.ChainName.ToString());
        return;
    }
    
    MissionChains.Add(Chain);
    ChainMap.Add(Chain.ChainName, Chain);
    
    // Map missions to chain
    for (const FName& MissionID : Chain.MissionIDs)
    {
        MissionToChainMap.Add(MissionID, Chain.ChainName);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Registered mission chain: %s"), *Chain.ChainName.ToString());
}

FMissionChain UProceduralMissionGenerator::GetMissionChain(const FName& ChainName)
{
    FMissionChain* FoundChain = ChainMap.Find(ChainName);
    return FoundChain ? *FoundChain : FMissionChain();
}

TArray<FMissionChain> UProceduralMissionGenerator::GetAvailableChains(const FMissionGenerationContext& Context)
{
    TArray<FMissionChain> AvailableChains;
    
    for (const FMissionChain& Chain : MissionChains)
    {
        // Check if player can accept chain (simplified)
        if (Context.PlayerLevel >= 5) // Minimum level for chains
        {
            AvailableChains.Add(Chain);
        }
    }
    
    return AvailableChains;
}

bool UProceduralMissionGenerator::IsMissionPartOfChain(const FName& MissionID) const
{
    return MissionToChainMap.Contains(MissionID);
}

FMissionChain UProceduralMissionGenerator::GetMissionChainForMission(const FName& MissionID)
{
    if (const FName* ChainName = MissionToChainMap.Find(MissionID))
    {
        FMissionChain* FoundChain = ChainMap.Find(*ChainName);
        return FoundChain ? *FoundChain : FMissionChain();
    }

    return FMissionChain();
}

// ===== Generation Rules =====

void UProceduralMissionGenerator::RegisterGenerationRule(const FMissionGenerationRule& Rule)
{
    if (RuleMap.Contains(Rule.RuleName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Generation rule '%s' already registered"), *Rule.RuleName.ToString());
        return;
    }
    
    GenerationRules.Add(Rule);
    RuleMap.Add(Rule.RuleName, Rule);
    
    UE_LOG(LogTemp, Log, TEXT("Registered generation rule: %s"), *Rule.RuleName.ToString());
}

void UProceduralMissionGenerator::UnregisterGenerationRule(const FName& RuleName)
{
    if (!RuleMap.Contains(RuleName))
    {
        return;
    }
    
    RuleMap.Remove(RuleName);
    
    for (int32 i = 0; i < GenerationRules.Num(); i++)
    {
        if (GenerationRules[i].RuleName == RuleName)
        {
            GenerationRules.RemoveAt(i);
            break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Unregistered generation rule: %s"), *RuleName.ToString());
}

bool UProceduralMissionGenerator::EvaluateGenerationRules(const FMissionGenerationContext& Context, const FMissionTemplate& Template)
{
    for (const FMissionGenerationRule& Rule : GenerationRules)
    {
        if (!Rule.bIsEnabled)
        {
            continue;
        }
        
        // Check if rule applies to this template
        bool bRuleApplies = true;
        for (const FName& RequiredTag : Rule.RequiredTags)
        {
            if (!Template.RequiredTags.Contains(RequiredTag))
            {
                bRuleApplies = false;
                break;
            }
        }
        
        if (bRuleApplies)
        {
            // Evaluate rule condition (simplified)
            if (Rule.Condition.Contains(TEXT("level")))
            {
                if (Context.PlayerLevel < 10)
                {
                    return false;
                }
            }
            
            OnGenerationRuleTriggered.Broadcast(Rule, Context);
        }
    }
    
    return true;
}

TArray<FMissionGenerationRule> UProceduralMissionGenerator::GetApplicableRules(const FMissionGenerationContext& Context)
{
    TArray<FMissionGenerationRule> ApplicableRules;
    
    for (const FMissionGenerationRule& Rule : GenerationRules)
    {
        if (Rule.bIsEnabled)
        {
            ApplicableRules.Add(Rule);
        }
    }
    
    return ApplicableRules;
}

// ===== Integration =====

void UProceduralMissionGenerator::SetMissionBoardComponent(class UMissionBoardComponent* MissionBoard)
{
    MissionBoardComponent = MissionBoard;
}

void UProceduralMissionGenerator::AutoGenerateMissions()
{
    if (!bEnableDynamicGeneration || !MissionBoardComponent)
    {
        return;
    }
    
    // Generate missions for each board
    TArray<FMissionBoard> Boards = MissionBoardComponent->GetAllMissionBoards();
    
    for (const FMissionBoard& Board : Boards)
    {
        FMissionGenerationContext Context;
        Context.CurrentTime = GetWorld()->GetTimeSeconds();
        Context.CurrentEnvironment = EMissionContext::Space; // Default
        
        TArray<FMissionData> NewMissions = GenerateMissionBatch(Context, MaxMissionsPerInterval);
        
        for (const FMissionData& Mission : NewMissions)
        {
            MissionBoardComponent->AddMissionToBoard(Board.BoardName, Mission);
        }
    }
}

void UProceduralMissionGenerator::RefreshMissionGeneration()
{
    // Clear old templates and reload
    MissionTemplates.Empty();
    TemplateMap.Empty();
    ObjectiveTemplates.Empty();
    ObjectiveTemplateMap.Empty();
    
    InitializeDefaultTemplates();
    InitializeDefaultObjectiveTemplates();
    
    UE_LOG(LogTemp, Log, TEXT("Mission generation refreshed"));
}

// ===== Private Functions =====

void UProceduralMissionGenerator::InitializeDefaultTemplates()
{
    // Patrol Mission Template
    FMissionTemplate PatrolTemplate;
    PatrolTemplate.TemplateName = TEXT("Patrol");
    PatrolTemplate.TemplateType = EMissionTemplateType::Patrol;
    PatrolTemplate.MissionContext = EMissionContext::Space;
    PatrolTemplate.Complexity = EMissionComplexity::Simple;
    PatrolTemplate.SupportedMissionTypes = {EMissionType::Patrol};
    PatrolTemplate.DifficultyRange = {EMissionDifficulty::Easy, EMissionDifficulty::Hard};
    PatrolTemplate.AllowedFactions = {EMissionFaction::Military, EMissionFaction::Corporation};
    PatrolTemplate.BaseProbability = 0.3f;
    PatrolTemplate.MinObjectives = 1;
    PatrolTemplate.MaxObjectives = 3;
    PatrolTemplate.BaseRewardMultiplier = 1.0f;
    RegisterMissionTemplate(PatrolTemplate);
    
    // Escort Mission Template
    FMissionTemplate EscortTemplate;
    EscortTemplate.TemplateName = TEXT("Escort");
    EscortTemplate.TemplateType = EMissionTemplateType::Escort;
    EscortTemplate.MissionContext = EMissionContext::Space;
    EscortTemplate.Complexity = EMissionComplexity::Moderate;
    EscortTemplate.SupportedMissionTypes = {EMissionType::Escort};
    EscortTemplate.DifficultyRange = {EMissionDifficulty::Normal, EMissionDifficulty::VeryHard};
    EscortTemplate.AllowedFactions = {EMissionFaction::Independent, EMissionFaction::Corporation};
    EscortTemplate.BaseProbability = 0.2f;
    EscortTemplate.MinObjectives = 1;
    EscortTemplate.MaxObjectives = 2;
    EscortTemplate.BaseRewardMultiplier = 1.5f;
    RegisterMissionTemplate(EscortTemplate);
    
    // Bounty Mission Template
    FMissionTemplate BountyTemplate;
    BountyTemplate.TemplateName = TEXT("Bounty");
    BountyTemplate.TemplateType = EMissionTemplateType::Bounty;
    BountyTemplate.MissionContext = EMissionContext::Space;
    BountyTemplate.Complexity = EMissionComplexity::Moderate;
    BountyTemplate.SupportedMissionTypes = {EMissionType::Bounty};
    BountyTemplate.DifficultyRange = {EMissionDifficulty::Normal, EMissionDifficulty::Extreme};
    BountyTemplate.AllowedFactions = {EMissionFaction::Military, EMissionFaction::Criminal};
    BountyTemplate.BaseProbability = 0.15f;
    BountyTemplate.MinObjectives = 1;
    BountyTemplate.MaxObjectives = 1;
    BountyTemplate.BaseRewardMultiplier = 2.0f;
    RegisterMissionTemplate(BountyTemplate);
    
    // Exploration Mission Template
    FMissionTemplate ExplorationTemplate;
    ExplorationTemplate.TemplateName = TEXT("Exploration");
    ExplorationTemplate.TemplateType = EMissionTemplateType::Exploration;
    ExplorationTemplate.MissionContext = EMissionContext::Anomaly;
    ExplorationTemplate.Complexity = EMissionComplexity::Complex;
    ExplorationTemplate.SupportedMissionTypes = {EMissionType::Exploration};
    ExplorationTemplate.DifficultyRange = {EMissionDifficulty::Normal, EMissionDifficulty::Legendary};
    ExplorationTemplate.AllowedFactions = {EMissionFaction::Science, EMissionFaction::Independent};
    ExplorationTemplate.BaseProbability = 0.1f;
    ExplorationTemplate.MinObjectives = 2;
    ExplorationTemplate.MaxObjectives = 5;
    ExplorationTemplate.BaseRewardMultiplier = 1.8f;
    RegisterMissionTemplate(ExplorationTemplate);
}

void UProceduralMissionGenerator::InitializeDefaultObjectiveTemplates()
{
    // Primary Objective Template
    FMissionObjectiveTemplate PrimaryObjective;
    PrimaryObjective.ObjectiveType = TEXT("Primary");
    PrimaryObjective.DescriptionTemplate = TEXT("Complete the primary objective: {Objective}");
    PrimaryObjective.bIsRequired = true;
    PrimaryObjective.Weight = 1.0f;
    RegisterObjectiveTemplate(PrimaryObjective);
    
    // Secondary Objective Template
    FMissionObjectiveTemplate SecondaryObjective;
    SecondaryObjective.ObjectiveType = TEXT("Secondary");
    SecondaryObjective.DescriptionTemplate = TEXT("Optional: {Objective}");
    SecondaryObjective.bIsRequired = false;
    SecondaryObjective.Weight = 0.5f;
    RegisterObjectiveTemplate(SecondaryObjective);
    
    // Time Limit Objective Template
    FMissionObjectiveTemplate TimeObjective;
    TimeObjective.ObjectiveType = TEXT("TimeLimit");
    TimeObjective.DescriptionTemplate = TEXT("Complete within {TimeLimit} minutes");
    TimeObjective.bIsRequired = true;
    TimeObjective.Weight = 0.8f;
    RegisterObjectiveTemplate(TimeObjective);
}

void UProceduralMissionGenerator::InitializeDefaultRules()
{
    // Level Requirement Rule
    FMissionGenerationRule LevelRule;
    LevelRule.RuleName = TEXT("LevelRequirement");
    LevelRule.Condition = TEXT("player.level >= 5");
    LevelRule.Action = TEXT("allow_complex_missions");
    LevelRule.Priority = 1.0f;
    LevelRule.bIsEnabled = true;
    RegisterGenerationRule(LevelRule);
    
    // Reputation Rule
    FMissionGenerationRule ReputationRule;
    ReputationRule.RuleName = TEXT("ReputationRequirement");
    ReputationRule.Condition = TEXT("player.reputation.faction >= 0.5");
    ReputationRule.Action = TEXT("allow_high_value_missions");
    ReputationRule.Priority = 0.8f;
    ReputationRule.bIsEnabled = true;
    RegisterGenerationRule(ReputationRule);
}

void UProceduralMissionGenerator::SetupTimers()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Mission generation timer
    World->GetTimerManager().SetTimer(GenerationTimer, [this]()
    {
        AutoGenerateMissions();
    }, GenerationInterval, true);
    
    // World event timer
    World->GetTimerManager().SetTimer(WorldEventTimer, [this]()
    {
        GenerateRandomWorldEvent();
    }, WORLD_EVENT_INTERVAL, true);
    
    // Refresh timer
    World->GetTimerManager().SetTimer(RefreshTimer, [this]()
    {
        RefreshMissionGeneration();
    }, REFRESH_INTERVAL, true);
}

FMissionTemplate* UProceduralMissionGenerator::SelectTemplate(const FMissionGenerationContext& Context)
{
    TArray<FMissionTemplate> FilteredTemplates = FilterTemplates(Context);
    
    if (FilteredTemplates.Num() == 0)
    {
        return nullptr;
    }
    
    // Calculate weights for each template
    TArray<float> Weights;
    float TotalWeight = 0.0f;
    
    for (const FMissionTemplate& Template : FilteredTemplates)
    {
        float Weight = CalculateTemplateWeight(Template, Context);
        Weights.Add(Weight);
        TotalWeight += Weight;
    }
    
    // Select template based on weight
    float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
    float CurrentWeight = 0.0f;
    
    for (int32 i = 0; i < FilteredTemplates.Num(); i++)
    {
        CurrentWeight += Weights[i];
        if (RandomValue <= CurrentWeight)
        {
            return &FilteredTemplates[i];
        }
    }
    
    return &FilteredTemplates[0]; // Fallback
}

TArray<FMissionTemplate> UProceduralMissionGenerator::FilterTemplates(const FMissionGenerationContext& Context)
{
    TArray<FMissionTemplate> FilteredTemplates;
    
    for (const FMissionTemplate& Template : MissionTemplates)
    {
        bool bTemplateValid = true;
        
        // Check faction requirements
        if (Template.AllowedFactions.Num() > 0 && !Template.AllowedFactions.Contains(Context.PlayerFaction))
        {
            bTemplateValid = false;
        }
        
        // Check generation rules
        if (!EvaluateGenerationRules(Context, Template))
        {
            bTemplateValid = false;
        }
        
        if (bTemplateValid)
        {
            FilteredTemplates.Add(Template);
        }
    }
    
    return FilteredTemplates;
}

float UProceduralMissionGenerator::CalculateTemplateWeight(const FMissionTemplate& Template, const FMissionGenerationContext& Context)
{
    float Weight = Template.BaseProbability;
    
    // Adjust weight based on player level
    float LevelAdjustment = 1.0f;
    if (Context.PlayerLevel < 5 && Template.Complexity <= EMissionComplexity::Simple)
    {
        LevelAdjustment = 2.0f; // Favor simple missions for low-level players
    }
    else if (Context.PlayerLevel >= 10 && Template.Complexity >= EMissionComplexity::Complex)
    {
        LevelAdjustment = 1.5f; // Favor complex missions for high-level players
    }
    
    // Adjust weight based on mission history
    float HistoryAdjustment = 1.0f;
    for (EMissionType MissionType : Template.SupportedMissionTypes)
    {
        if (Context.PlayerMissionHistory.Contains(MissionType))
        {
            int32 MissionCount = Context.PlayerMissionHistory[MissionType];
            HistoryAdjustment *= FMath::Max(0.1f, 1.0f - (MissionCount * 0.1f)); // Reduce weight for repeated mission types
        }
    }
    
    return Weight * LevelAdjustment * HistoryAdjustment;
}

FMissionData UProceduralMissionGenerator::CreateMissionFromTemplate(const FMissionTemplate& Template, const FMissionGenerationContext& Context)
{
    FMissionData Mission;
    
    // Generate mission ID
    Mission.MissionID = FName(*FString::Printf(TEXT("%s_%d"), *Template.TemplateName.ToString(), FMath::RandRange(10000, 99999)));
    
    // Generate mission title and description
    Mission.Title = GenerateMissionTitle(Template, Context);
    Mission.Description = GenerateMissionDescription(Template, Context);
    
    // Set mission properties
    Mission.MissionType = Template.SupportedMissionTypes[0]; // Use first supported type
    Mission.Difficulty = (EMissionDifficulty)FMath::RandRange((int32)Template.DifficultyRange[0], (int32)Template.DifficultyRange[1]);
    Mission.Status = EMissionStatus::Available;
    Mission.Faction = Context.PlayerFaction;
    Mission.AssignedPlayer = Context.Player;
    Mission.MissionLocation = GenerateMissionLocation(Template, Context);
    Mission.MissionGiver = TEXT("Mission Board");
    Mission.LevelRequirement = CalculateRecommendedLevel(Context, Mission.Difficulty);
    Mission.bIsRepeatable = Template.Complexity <= EMissionComplexity::Moderate;
    Mission.MaxCompletions = Template.Complexity <= EMissionComplexity::Simple ? -1 : 3; // Unlimited for simple, limited for complex
    Mission.CurrentCompletions = 0;
    Mission.bIsHidden = false;
    Mission.bIsLegendary = Template.Complexity >= EMissionComplexity::Epic;
    
    // Generate objectives
    Mission.Objectives = GenerateObjectives(Template, Context);
    
    // Generate rewards
    Mission.Reward = GenerateMissionRewards(Template, Context);
    
    // Scale for player
    Mission = ScaleMissionForPlayer(Mission, Context);
    
    OnMissionGenerated.Broadcast(Mission, Context);
    
    return Mission;
}

FString UProceduralMissionGenerator::GenerateMissionTitle(const FMissionTemplate& Template, const FMissionGenerationContext& Context)
{
    TArray<FString> Titles;
    
    switch (Template.TemplateType)
    {
    case EMissionTemplateType::Patrol:
        Titles = {TEXT("Patrol Sector"), TEXT("Security Sweep"), TEXT("Reconnaissance Run")};
        break;
    case EMissionTemplateType::Escort:
        Titles = {TEXT("Escort Convoy"), TEXT("Protect VIP"), TEXT("Guard Transport")};
        break;
    case EMissionTemplateType::Bounty:
        Titles = {TEXT("Bounty Hunt"), TEXT("Target Elimination"), TEXT("Wanted Criminal")};
        break;
    case EMissionTemplateType::Exploration:
        Titles = {TEXT("Explore Anomaly"), TEXT("Survey Region"), TEXT("Discover Location")};
        break;
    default:
        Titles = {TEXT("Mission"), TEXT("Task"), TEXT("Operation")};
        break;
    }
    
    FString BaseTitle = GetRandomStringFromArray(Titles);
    return FString::Printf(TEXT("%s: %s"), *BaseTitle, *FString::FromInt(FMath::RandRange(100, 999)));
}

FString UProceduralMissionGenerator::GenerateMissionDescription(const FMissionTemplate& Template, const FMissionGenerationContext& Context)
{
    TArray<FString> Descriptions;
    
    switch (Template.TemplateType)
    {
    case EMissionTemplateType::Patrol:
        Descriptions = {TEXT("Patrol the designated sector and report any unusual activity."), 
                      TEXT("Conduct a security sweep of the area to ensure safe passage."), 
                      TEXT("Perform reconnaissance run and gather intelligence.")};
        break;
    case EMissionTemplateType::Escort:
        Descriptions = {TEXT("Escort the convoy safely to its destination."), 
                      TEXT("Protect the VIP from any threats during transit."), 
                      TEXT("Guard the transport ship and its valuable cargo.")};
        break;
    case EMissionTemplateType::Bounty:
        Descriptions = {TEXT("Hunt down and eliminate the target."), 
                      TEXT("Bring the wanted criminal to justice."), 
                      TEXT("Neutralize the high-value target.")};
        break;
    case EMissionTemplateType::Exploration:
        Descriptions = {TEXT("Explore the anomaly and report your findings."), 
                      TEXT("Survey the uncharted region for valuable resources."), 
                      TEXT("Discover and catalog new locations.")};
        break;
    default:
        Descriptions = {TEXT("Complete the assigned objectives."), 
                      TEXT("Carry out the mission parameters."), 
                      TEXT("Fulfill the mission requirements.")};
        break;
    }
    
    return GetRandomStringFromArray(Descriptions);
}

FVector UProceduralMissionGenerator::GenerateMissionLocation(const FMissionTemplate& Template, const FMissionGenerationContext& Context)
{
    FVector BaseLocation = Context.PlayerLocation;
    
    // Generate location based on mission context
    float Distance = 5000.0f; // Default distance
    
    switch (Template.MissionContext)
    {
    case EMissionContext::Space:
        Distance = FMath::FRandRange(10000.0f, 50000.0f);
        break;
    case EMissionContext::Planet:
        Distance = FMath::FRandRange(2000.0f, 10000.0f);
        break;
    case EMissionContext::Station:
        Distance = FMath::FRandRange(1000.0f, 5000.0f);
        break;
    default:
        Distance = FMath::FRandRange(5000.0f, 20000.0f);
        break;
    }
    
    // Generate random direction
    float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
    FVector Offset = FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f);
    
    return BaseLocation + Offset;
}

FMissionReward UProceduralMissionGenerator::GenerateMissionRewards(const FMissionTemplate& Template, const FMissionGenerationContext& Context)
{
    FMissionReward Reward;
    
    // Base rewards based on template
    int32 BaseCredits = 1000 * (int32)Template.Complexity;
    int32 BaseExperience = 500 * (int32)Template.Complexity;
    
    Reward.Credits = FMath::RoundToInt(BaseCredits * Template.BaseRewardMultiplier);
    Reward.Experience = FMath::RoundToInt(BaseExperience * Template.BaseRewardMultiplier);
    Reward.RewardDescription = FString::Printf(TEXT("Standard reward for %s mission"), *Template.TemplateName.ToString());
    
    return Reward;
}

FString UProceduralMissionGenerator::ProcessObjectiveTemplate(const FString& Template, const TArray<FMissionParameter>& Parameters, const FMissionGenerationContext& Context)
{
    TMap<FName, FString> ParameterMap = BuildParameterMap(Parameters, Context);
    return ReplaceParameters(Template, ParameterMap);
}

TArray<FMissionParameter> UProceduralMissionGenerator::GenerateObjectiveParameters(const FMissionObjectiveTemplate& ObjectiveTemplate, const FMissionGenerationContext& Context)
{
    TArray<FMissionParameter> Parameters;
    
    for (const FMissionParameter& Param : ObjectiveTemplate.Parameters)
    {
        FMissionParameter NewParam = Param;
        
        if (Param.bIsRandom)
        {
            if (Param.PossibleValues.Num() > 0)
            {
                NewParam.ParameterValue = GetRandomStringFromArray(Param.PossibleValues);
            }
            else
            {
                NewParam.ParameterValue = FString::SanitizeFloat(FMath::FRandRange(Param.MinValue, Param.MaxValue));
            }
        }
        
        Parameters.Add(NewParam);
    }
    
    return Parameters;
}

void UProceduralMissionGenerator::GenerateRandomWorldEvent()
{
    if (!bEnableWorldEvents)
    {
        return;
    }
    
    FMissionGenerationContext Context;
    Context.CurrentTime = GetWorld()->GetTimeSeconds();
    Context.CurrentEnvironment = EMissionContext::Space;
    
    GenerateWorldEvent(Context);
}

void UProceduralMissionGenerator::ProcessWorldEvent(const FName& EventName, float DeltaTime)
{
    // Process event-specific logic
    if (EventName == TEXT("PirateAttack"))
    {
        // Generate combat missions
        FMissionGenerationContext CombatContext;
        CombatContext.CurrentTime = GetWorld()->GetTimeSeconds();
        CombatContext.CurrentEnvironment = EMissionContext::Combat;
        CombatContext.PlayerLevel = 1; // Would get from actual player

        // Create combat mission template dynamically
        FMissionTemplate CombatTemplate;
        CombatTemplate.TemplateName = TEXT("PirateAmbush");
        CombatTemplate.TemplateType = EMissionTemplateType::Assault;
        CombatTemplate.MissionContext = EMissionContext::Combat;
        CombatTemplate.Complexity = EMissionComplexity::Moderate;
        CombatTemplate.SupportedMissionTypes = {EMissionType::Combat};
        CombatTemplate.DifficultyRange = {EMissionDifficulty::Normal, EMissionDifficulty::Hard};
        CombatTemplate.MinObjectives = 1;
        CombatTemplate.MaxObjectives = 2;
        CombatTemplate.BaseRewardMultiplier = 1.5f;

        // Generate the mission
        FMissionData CombatMission = CreateMissionFromTemplate(CombatTemplate, CombatContext);
        CombatMission.Title = TEXT("Defend Against Pirate Attack");
        CombatMission.Description = TEXT("Pirates are attacking the sector! Eliminate the threat.");

        UE_LOG(LogTemp, Log, TEXT("Generated combat mission from PirateAttack event: %s"), *CombatMission.Title);
    }
    else if (EventName == TEXT("TradeConvoy"))
    {
        // Generate escort missions
        FMissionGenerationContext EscortContext;
        EscortContext.CurrentTime = GetWorld()->GetTimeSeconds();
        EscortContext.CurrentEnvironment = EMissionContext::Space;
        EscortContext.PlayerLevel = 1;

        // Create escort mission
        FMissionTemplate EscortTemplate;
        EscortTemplate.TemplateName = TEXT("ConvoyEscort");
        EscortTemplate.TemplateType = EMissionTemplateType::Escort;
        EscortTemplate.MissionContext = EMissionContext::Space;
        EscortTemplate.Complexity = EMissionComplexity::Moderate;
        EscortTemplate.SupportedMissionTypes = {EMissionType::Escort};
        EscortTemplate.DifficultyRange = {EMissionDifficulty::Easy, EMissionDifficulty::Normal};
        EscortTemplate.MinObjectives = 1;
        EscortTemplate.MaxObjectives = 2;
        EscortTemplate.BaseRewardMultiplier = 1.3f;

        FMissionData EscortMission = CreateMissionFromTemplate(EscortTemplate, EscortContext);
        EscortMission.Title = TEXT("Escort Trade Convoy");
        EscortMission.Description = TEXT("A trade convoy needs protection through dangerous space.");

        // Add escort-specific objective
        FMissionBoardObjective EscortObjective;
        EscortObjective.ObjectiveID = FName("EscortConvoy");
        EscortObjective.Description = TEXT("Keep the convoy safe until it reaches its destination");
        EscortObjective.bIsRequired = true;
        EscortObjective.TargetCount = 1;
        EscortObjective.CurrentCount = 0;
        EscortMission.Objectives.Add(EscortObjective);

        UE_LOG(LogTemp, Log, TEXT("Generated escort mission from TradeConvoy event: %s"), *EscortMission.Title);
    }
    else if (EventName == TEXT("ResearchDiscovery"))
    {
        // Generate research/exploration missions
        FMissionGenerationContext ResearchContext;
        ResearchContext.CurrentTime = GetWorld()->GetTimeSeconds();
        ResearchContext.CurrentEnvironment = EMissionContext::Anomaly;
        ResearchContext.PlayerLevel = 1;

        // Create research mission
        FMissionTemplate ResearchTemplate;
        ResearchTemplate.TemplateName = TEXT("AnomalyResearch");
        ResearchTemplate.TemplateType = EMissionTemplateType::Investigation;
        ResearchTemplate.MissionContext = EMissionContext::Anomaly;
        ResearchTemplate.Complexity = EMissionComplexity::Complex;
        ResearchTemplate.SupportedMissionTypes = {EMissionType::Investigation};
        ResearchTemplate.DifficultyRange = {EMissionDifficulty::Normal, EMissionDifficulty::VeryHard};
        ResearchTemplate.MinObjectives = 2;
        ResearchTemplate.MaxObjectives = 4;
        ResearchTemplate.BaseRewardMultiplier = 2.0f;

        FMissionData ResearchMission = CreateMissionFromTemplate(ResearchTemplate, ResearchContext);
        ResearchMission.Title = TEXT("Investigate Anomaly");
        ResearchMission.Description = TEXT("Scientists have detected an unusual anomaly that requires investigation.");

        // Add research-specific objectives
        FMissionBoardObjective ScanObjective;
        ScanObjective.ObjectiveID = FName("ScanAnomaly");
        ScanObjective.Description = TEXT("Scan the anomaly with your ship's sensors");
        ScanObjective.bIsRequired = true;
        ScanObjective.TargetCount = 3; // Multiple scan points
        ScanObjective.CurrentCount = 0;
        ResearchMission.Objectives.Add(ScanObjective);

        FMissionBoardObjective CollectObjective;
        CollectObjective.ObjectiveID = FName("CollectSamples");
        CollectObjective.Description = TEXT("Collect data samples from the anomaly");
        CollectObjective.bIsRequired = false;
        CollectObjective.TargetCount = 5;
        CollectObjective.CurrentCount = 0;
        ResearchMission.Objectives.Add(CollectObjective);

        UE_LOG(LogTemp, Log, TEXT("Generated research mission from ResearchDiscovery event: %s"), *ResearchMission.Title);
    }
}

void UProceduralMissionGenerator::EndWorldEvent(const FName& EventName)
{
    ActiveWorldEvents.Remove(EventName);
    EventStartTimes.Remove(EventName);
    EventParameters.Remove(EventName);
    
    UE_LOG(LogTemp, Log, TEXT("World event ended: %s"), *EventName.ToString());
}

FString UProceduralMissionGenerator::ReplaceParameters(const FString& Template, const TMap<FName, FString>& Parameters)
{
    FString Result = Template;
    
    for (const auto& ParamPair : Parameters)
    {
        FString Placeholder = FString::Printf(TEXT("{%s}"), *ParamPair.Key.ToString());
        Result.ReplaceInline(*Placeholder, *ParamPair.Value, ESearchCase::IgnoreCase);
    }
    
    return Result;
}

TMap<FName, FString> UProceduralMissionGenerator::BuildParameterMap(const TArray<FMissionParameter>& Parameters, const FMissionGenerationContext& Context)
{
    TMap<FName, FString> ParameterMap;
    
    for (const FMissionParameter& Param : Parameters)
    {
        FString Value = Param.ParameterValue;
        
        // Replace special placeholders
        if (Value.Contains(TEXT("{PlayerLevel}")))
        {
            Value.ReplaceInline(TEXT("{PlayerLevel}"), *FString::FromInt(Context.PlayerLevel));
        }
        if (Value.Contains(TEXT("{PlayerName}")))
        {
            Value.ReplaceInline(TEXT("{PlayerName}"), *Context.Player->GetPlayerName());
        }
        if (Value.Contains(TEXT("{Faction}")))
        {
            Value.ReplaceInline(TEXT("{Faction}"), *FString::FromInt((int32)Context.PlayerFaction));
        }
        
        ParameterMap.Add(Param.ParameterName, Value);
    }
    
    return ParameterMap;
}

float UProceduralMissionGenerator::GetRandomFloatInRange(float Min, float Max)
{
    return FMath::FRandRange(Min, Max);
}

int32 UProceduralMissionGenerator::GetRandomIntInRange(int32 Min, int32 Max)
{
    return FMath::RandRange(Min, Max);
}

FString UProceduralMissionGenerator::GetRandomStringFromArray(const TArray<FString>& Strings)
{
    if (Strings.Num() == 0)
    {
        return TEXT("");
    }
    
    int32 Index = FMath::RandRange(0, Strings.Num() - 1);
    return Strings[Index];
}