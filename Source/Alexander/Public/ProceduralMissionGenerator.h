// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MissionBoardComponent.h"
#include "ProceduralMissionGenerator.generated.h"

UENUM(BlueprintType)
enum class EMissionTemplateType : uint8
{
    None,
    Patrol,
    Escort,
    Delivery,
    Bounty,
    Exploration,
    Mining,
    Salvage,
    Transport,
    Investigation,
    Rescue,
    Diplomatic,
    Trading,
    Racing,
    Defense,
    Assault
};

UENUM(BlueprintType)
enum class EMissionContext : uint8
{
    None,
    Space,
    Planet,
    Station,
    Asteroid,
    Nebula,
    Anomaly,
    Wreckage,
    Outpost,
    Colony,
    Research,
    Military,
    Civilian,
    Corporate,
    Criminal,
    Alien
};

UENUM(BlueprintType)
enum class EMissionComplexity : uint8
{
    Simple,
    Moderate,
    Complex,
    Intricate,
    Epic
};

USTRUCT(BlueprintType)
struct FMissionTemplate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    FName TemplateName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    EMissionTemplateType TemplateType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    EMissionContext MissionContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    EMissionComplexity Complexity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    TArray<EMissionType> SupportedMissionTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    TArray<EMissionDifficulty> DifficultyRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    TArray<EMissionFaction> AllowedFactions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    float BaseProbability = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    int32 MinObjectives = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    int32 MaxObjectives = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    float BaseRewardMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    TArray<FName> RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    TArray<FName> ExcludedTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    TMap<FName, float> ContextModifiers;
};

USTRUCT(BlueprintType)
struct FMissionParameter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    FName ParameterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    FString ParameterValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    float MinValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    float MaxValue = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    bool bIsRandom = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    TArray<FString> PossibleValues;
};

USTRUCT(BlueprintType)
struct FMissionObjectiveTemplate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FName ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString DescriptionTemplate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    TArray<FMissionParameter> Parameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsRequired = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float Weight = 1.0f;
};

USTRUCT(BlueprintType)
struct FMissionGenerationContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    APlayerState* Player;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FVector PlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    EMissionFaction PlayerFaction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    int32 PlayerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TMap<EMissionType, int32> PlayerMissionHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TMap<EMissionFaction, float> PlayerReputation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<FName> PlayerTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float CurrentTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    EMissionContext CurrentEnvironment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<FName> ActiveWorldEvents;
};

USTRUCT(BlueprintType)
struct FMissionGenerationRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    FName RuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    FString Condition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    FString Action;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    bool bIsEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    TArray<FName> RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    TMap<FName, float> RuleParameters;
};

USTRUCT(BlueprintType)
struct FMissionChain
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
    FName ChainName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
    TArray<FName> MissionIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
    TArray<int32> MissionOrder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
    bool bIsSequential;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
    FName ChainReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
    float ChainBonusMultiplier;
};

/**
 * Procedural Mission Generator with context-aware mission creation and dynamic event generation
 */
UCLASS(ClassGroup=(Mission), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UProceduralMissionGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UProceduralMissionGenerator();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ===== Mission Generation =====
    
    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    FMissionData GenerateMission(const FMissionGenerationContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    TArray<FMissionData> GenerateMissionBatch(const FMissionGenerationContext& Context, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    FMissionData GenerateMissionFromTemplate(const FName& TemplateName, const FMissionGenerationContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    FMissionChain GenerateMissionChain(const TArray<FName>& TemplateNames, const FMissionGenerationContext& Context);

    // ===== Template Management =====
    
    UFUNCTION(BlueprintCallable, Category = "Templates")
    void RegisterMissionTemplate(const FMissionTemplate& Template);

    UFUNCTION(BlueprintCallable, Category = "Templates")
    void UnregisterMissionTemplate(const FName& TemplateName);

    UFUNCTION(BlueprintCallable, Category = "Templates")
    FMissionTemplate GetMissionTemplate(const FName& TemplateName);

    UFUNCTION(BlueprintCallable, Category = "Templates")
    TArray<FMissionTemplate> GetAllTemplates() const { return MissionTemplates; }

    UFUNCTION(BlueprintCallable, Category = "Templates")
    TArray<FMissionTemplate> GetTemplatesByType(EMissionTemplateType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Templates")
    TArray<FMissionTemplate> GetTemplatesByContext(EMissionContext Context) const;

    // ===== Objective Generation =====
    
    UFUNCTION(BlueprintCallable, Category = "Objectives")
    TArray<FMissionObjective> GenerateObjectives(const FMissionTemplate& Template, const FMissionGenerationContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Objectives")
    FMissionObjective GenerateObjective(const FMissionObjectiveTemplate& ObjectiveTemplate, const FMissionGenerationContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Objectives")
    void RegisterObjectiveTemplate(const FMissionObjectiveTemplate& ObjectiveTemplate);

    UFUNCTION(BlueprintCallable, Category = "Objectives")
    TArray<FMissionObjectiveTemplate> GetObjectiveTemplatesByType(const FName& ObjectiveType) const;

    // ===== Dynamic Event Generation =====
    
    UFUNCTION(BlueprintCallable, Category = "Dynamic Events")
    void GenerateWorldEvent(const FMissionGenerationContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Events")
    void UpdateWorldEvents(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Events")
    TArray<FName> GetActiveWorldEvents() const { return ActiveWorldEvents; }

    UFUNCTION(BlueprintCallable, Category = "Dynamic Events")
    bool IsWorldEventActive(const FName& EventName) const;

    // ===== Mission Scaling =====
    
    UFUNCTION(BlueprintCallable, Category = "Scaling")
    FMissionData ScaleMissionForPlayer(const FMissionData& BaseMission, const FMissionGenerationContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Scaling")
    FMissionReward ScaleMissionRewards(const FMissionReward& BaseReward, int32 PlayerLevel, EMissionDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Scaling")
    float CalculateMissionDifficulty(const FMissionGenerationContext& Context, const FMissionTemplate& Template);

    UFUNCTION(BlueprintCallable, Category = "Scaling")
    int32 CalculateRecommendedLevel(const FMissionGenerationContext& Context, EMissionDifficulty TargetDifficulty);

    // ===== Faction-Based Missions =====
    
    UFUNCTION(BlueprintCallable, Category = "Factions")
    TArray<FMissionTemplate> GetFactionSpecificTemplates(EMissionFaction Faction) const;

    UFUNCTION(BlueprintCallable, Category = "Factions")
    FMissionData GenerateFactionMission(EMissionFaction Faction, const FMissionGenerationContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Factions")
    bool CanPlayerAcceptFactionMission(EMissionFaction Faction, const FMissionGenerationContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Factions")
    void UpdateFactionRelationships(const FMissionData& Mission, bool bCompleted);

    // ===== Mission Chain System =====
    
    UFUNCTION(BlueprintCallable, Category = "Mission Chains")
    void RegisterMissionChain(const FMissionChain& Chain);

    UFUNCTION(BlueprintCallable, Category = "Mission Chains")
    FMissionChain GetMissionChain(const FName& ChainName);

    UFUNCTION(BlueprintCallable, Category = "Mission Chains")
    TArray<FMissionChain> GetAvailableChains(const FMissionGenerationContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Mission Chains")
    bool IsMissionPartOfChain(const FName& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission Chains")
    FMissionChain GetMissionChainForMission(const FName& MissionID);

    // ===== Generation Rules =====
    
    UFUNCTION(BlueprintCallable, Category = "Rules")
    void RegisterGenerationRule(const FMissionGenerationRule& Rule);

    UFUNCTION(BlueprintCallable, Category = "Rules")
    void UnregisterGenerationRule(const FName& RuleName);

    UFUNCTION(BlueprintCallable, Category = "Rules")
    bool EvaluateGenerationRules(const FMissionGenerationContext& Context, const FMissionTemplate& Template);

    UFUNCTION(BlueprintCallable, Category = "Rules")
    TArray<FMissionGenerationRule> GetApplicableRules(const FMissionGenerationContext& Context);

    // ===== Configuration =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableDynamicGeneration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableWorldEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableMissionChains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableFactionMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float GenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxMissionsPerInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float DifficultyScalingFactor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float RewardScalingFactor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxMissionChainsPerPlayer;

    // ===== Integration =====
    
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void SetMissionBoardComponent(class UMissionBoardComponent* MissionBoard);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void AutoGenerateMissions();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RefreshMissionGeneration();

    // ===== Events =====
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionGenerated, const FMissionData&, Mission, const FMissionGenerationContext&, Context);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldEventGenerated, const FName&, EventName);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionChainGenerated, const FMissionChain&, Chain, const FMissionGenerationContext&, Context);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGenerationRuleTriggered, const FMissionGenerationRule&, Rule, const FMissionGenerationContext&, Context);

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnMissionGenerated OnMissionGenerated;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnWorldEventGenerated OnWorldEventGenerated;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnMissionChainGenerated OnMissionChainGenerated;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnGenerationRuleTriggered OnGenerationRuleTriggered;

protected:
    // ===== Template Storage =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Templates")
    TArray<FMissionTemplate> MissionTemplates;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Templates")
    TMap<FName, FMissionTemplate> TemplateMap;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Templates")
    TArray<FMissionObjectiveTemplate> ObjectiveTemplates;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Templates")
    TMap<FName, FMissionObjectiveTemplate> ObjectiveTemplateMap;

    // ===== Mission Chains =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Chains")
    TArray<FMissionChain> MissionChains;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Chains")
    TMap<FName, FMissionChain> ChainMap;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Chains")
    TMap<FName, FName> MissionToChainMap;

    // ===== Generation Rules =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rules")
    TArray<FMissionGenerationRule> GenerationRules;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rules")
    TMap<FName, FMissionGenerationRule> RuleMap;

    // ===== World Events =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Events")
    TArray<FName> ActiveWorldEvents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Events")
    TMap<FName, float> EventStartTimes;

    // Event parameters (C++ only due to nested TMap limitation)
    TMap<FName, TMap<FName, FString>> EventParameters;

    // ===== System References =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System References")
    class UMissionBoardComponent* MissionBoardComponent;

    // ===== Timers =====
    
    UPROPERTY()
    FTimerHandle GenerationTimer;

    UPROPERTY()
    FTimerHandle WorldEventTimer;

    UPROPERTY()
    FTimerHandle RefreshTimer;

private:
    // ===== Internal Functions =====
    
    void InitializeDefaultTemplates();
    void InitializeDefaultObjectiveTemplates();
    void InitializeDefaultRules();
    void SetupTimers();
    
    // Template Selection
    FMissionTemplate* SelectTemplate(const FMissionGenerationContext& Context);
    TArray<FMissionTemplate> FilterTemplates(const FMissionGenerationContext& Context);
    float CalculateTemplateWeight(const FMissionTemplate& Template, const FMissionGenerationContext& Context);
    
    // Mission Generation
    FMissionData CreateMissionFromTemplate(const FMissionTemplate& Template, const FMissionGenerationContext& Context);
    FString GenerateMissionTitle(const FMissionTemplate& Template, const FMissionGenerationContext& Context);
    FString GenerateMissionDescription(const FMissionTemplate& Template, const FMissionGenerationContext& Context);
    FVector GenerateMissionLocation(const FMissionTemplate& Template, const FMissionGenerationContext& Context);
    FMissionReward GenerateMissionRewards(const FMissionTemplate& Template, const FMissionGenerationContext& Context);
    
    // Objective Generation
    FString ProcessObjectiveTemplate(const FString& Template, const TArray<FMissionParameter>& Parameters, const FMissionGenerationContext& Context);
    TArray<FMissionParameter> GenerateObjectiveParameters(const FMissionObjectiveTemplate& ObjectiveTemplate, const FMissionGenerationContext& Context);
    
    // World Events
    void GenerateRandomWorldEvent();
    void ProcessWorldEvent(const FName& EventName, float DeltaTime);
    void EndWorldEvent(const FName& EventName);
    
    // Utility Functions
    FString ReplaceParameters(const FString& Template, const TMap<FName, FString>& Parameters);
    TMap<FName, FString> BuildParameterMap(const TArray<FMissionParameter>& Parameters, const FMissionGenerationContext& Context);
    float GetRandomFloatInRange(float Min, float Max);
    int32 GetRandomIntInRange(int32 Min, int32 Max);
    FString GetRandomStringFromArray(const TArray<FString>& Strings);
    
    // Constants
    static constexpr float DEFAULT_GENERATION_INTERVAL = 300.0f; // 5 minutes
    static constexpr int32 DEFAULT_MAX_MISSIONS_PER_INTERVAL = 5;
    static constexpr float DEFAULT_DIFFICULTY_SCALING = 1.0f;
    static constexpr float DEFAULT_REWARD_SCALING = 1.0f;
    static constexpr int32 DEFAULT_MAX_MISSION_CHAINS_PER_PLAYER = 3;
    static constexpr float WORLD_EVENT_INTERVAL = 600.0f; // 10 minutes
    static constexpr float REFRESH_INTERVAL = 3600.0f; // 1 hour
};