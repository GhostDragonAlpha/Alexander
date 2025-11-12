// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
#include "TradingEconomySystem.h"
#include "TradeMissionSystem.generated.h"

// Forward declarations
class UTradeShipAutomation;
class ASpaceship;
class ATradeStation;

/**
 * Trade mission types
 */
UENUM(BlueprintType)
enum class ETradeMissionType : uint8
{
	Courier			UMETA(DisplayName = "Courier"),
	SupplyRun		UMETA(DisplayName = "Supply Run"),
	Smuggling		UMETA(DisplayName = "Smuggling"),
	TradeContract	UMETA(DisplayName = "Trade Contract"),
	Escort			UMETA(DisplayName = "Escort"),
	MarketManipulation	UMETA(DisplayName = "Market Manipulation"),
	Rescue			UMETA(DisplayName = "Rescue"),
	Salvage			UMETA(DisplayName = "Salvage"),
	Exploration		UMETA(DisplayName = "Exploration"),
	Diplomatic		UMETA(DisplayName = "Diplomatic")
};

/**
 * Mission difficulty levels
 */
UENUM(BlueprintType)
enum class EMissionDifficulty : uint8
{
	Trivial		UMETA(DisplayName = "Trivial"),
	Easy		UMETA(DisplayName = "Easy"),
	Moderate	UMETA(DisplayName = "Moderate"),
	Hard		UMETA(DisplayName = "Hard"),
	Extreme		UMETA(DisplayName = "Extreme"),
	Legendary	UMETA(DisplayName = "Legendary")
};

/**
 * Mission status
 */
UENUM(BlueprintType)
enum class EMissionStatus : uint8
{
	Available		UMETA(DisplayName = "Available"),
	Accepted		UMETA(DisplayName = "Accepted"),
	InProgress		UMETA(DisplayName = "In Progress"),
	Completed		UMETA(DisplayName = "Completed"),
	Failed			UMETA(DisplayName = "Failed"),
	Expired			UMETA(DisplayName = "Expired"),
	Cancelled		UMETA(DisplayName = "Cancelled")
};

/**
 * Trade mission data
 */
USTRUCT(BlueprintType)
struct FTradeMission
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FString MissionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	ETradeMissionType MissionType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	EMissionDifficulty Difficulty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FString Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FString IssuingFactionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FString TargetFactionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FString StartStationID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FString EndStationID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	TArray<FName> RequiredCommodities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	TArray<int32> RequiredQuantities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	float RewardCredits;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	float RewardReputation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	float RewardExperience;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	EMissionStatus Status;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FDateTime CreationTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FDateTime ExpirationTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FDateTime CompletionTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FString AssignedPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FString FailureReason;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	float DangerLevel; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	bool bIsRepeatable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 MaxCompletions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 CurrentCompletions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	TMap<FString, FString> MissionParameters; // Key-value parameters

	FTradeMission()
	{
		MissionType = ETradeMissionType::Courier;
		Difficulty = EMissionDifficulty::Easy;
		RewardCredits = 1000.0f;
		RewardReputation = 10.0f;
		RewardExperience = 100.0f;
		Status = EMissionStatus::Available;
		CreationTime = FDateTime::Now();
		ExpirationTime = CreationTime + FTimespan(24, 0, 0); // 24 hours default
		DangerLevel = 0.1f;
		bIsRepeatable = false;
		MaxCompletions = 1;
		CurrentCompletions = 0;
	}
};

/**
 * Active mission tracking
 */
USTRUCT(BlueprintType)
struct FActiveMission
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active")
	FTradeMission MissionData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active")
	float Progress; // 0-100

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active")
	FString CurrentObjective;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active")
	TArray<FName> CollectedCommodities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active")
	TWeakObjectPtr<ASpaceship> AssignedShip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active")
	FDateTime AcceptanceTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active")
	FTimespan TimeLimit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active")
	bool bIsTimeLimited;

	FActiveMission()
	{
		Progress = 0.0f;
		bIsTimeLimited = false;
	}
};

/**
 * Mission template for generation
 */
USTRUCT(BlueprintType)
struct FMissionTemplate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	ETradeMissionType MissionType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	EMissionDifficulty BaseDifficulty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	float BaseRewardCredits;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	float BaseRewardReputation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	float BaseDangerLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	TArray<FName> PossibleCommodities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	FString TitleFormat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	FString DescriptionFormat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	bool bIsRepeatable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	int32 MaxSpawns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	float SpawnChance; // 0-1

	FMissionTemplate()
	{
		MissionType = ETradeMissionType::Courier;
		BaseDifficulty = EMissionDifficulty::Easy;
		BaseRewardCredits = 1000.0f;
		BaseRewardReputation = 10.0f;
		BaseDangerLevel = 0.1f;
		bIsRepeatable = false;
		MaxSpawns = 1;
		SpawnChance = 0.5f;
	}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionAvailable, const FTradeMission&, Mission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionAccepted, const FTradeMission&, Mission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionCompleted, const FTradeMission&, Mission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionFailed, const FTradeMission&, Mission, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionExpired, const FTradeMission&, Mission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionProgressUpdated, const FActiveMission&, ActiveMission);

/**
 * Trade Mission System - Handles mission generation, tracking, and completion
 */
UCLASS()
class ALEXANDER_API UTradeMissionSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UTradeMissionSystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tick function
	virtual void Tick(float DeltaTime);
	virtual bool IsTickable() const { return true; }

	// ============================================================================
	// MISSION MANAGEMENT
	// ============================================================================

	// Get available missions
	UFUNCTION(BlueprintCallable, Category = "Missions")
	TArray<FTradeMission> GetAvailableMissions(FString StationID = "") const;

	// Get mission by ID
	UFUNCTION(BlueprintCallable, Category = "Missions")
	FTradeMission GetMission(FString MissionID) const;

	// Accept mission
	UFUNCTION(BlueprintCallable, Category = "Missions")
	bool AcceptMission(FString MissionID, FString PlayerID, ASpaceship* AssignedShip);

	// Complete mission
	UFUNCTION(BlueprintCallable, Category = "Missions")
	bool CompleteMission(FString MissionID);

	// Fail mission
	UFUNCTION(BlueprintCallable, Category = "Missions")
	bool FailMission(FString MissionID, FString Reason);

	// Cancel mission
	UFUNCTION(BlueprintCallable, Category = "Missions")
	bool CancelMission(FString MissionID);

	// Abandon mission
	UFUNCTION(BlueprintCallable, Category = "Missions")
	bool AbandonMission(FString MissionID);

	// ============================================================================
	// MISSION GENERATION
	// ============================================================================

	// Generate missions for station
	UFUNCTION(BlueprintCallable, Category = "Missions")
	void GenerateMissionsForStation(FString StationID, int32 Count);

	// Generate faction missions
	UFUNCTION(BlueprintCallable, Category = "Missions")
	void GenerateFactionMissions(FString FactionID, int32 Count);

	// Generate specific mission type
	UFUNCTION(BlueprintCallable, Category = "Missions")
	FTradeMission GenerateMission(ETradeMissionType MissionType, FString IssuingFactionID, FString StationID);

	// Clear expired missions
	UFUNCTION(BlueprintCallable, Category = "Missions")
	void ClearExpiredMissions();

	// Clear all missions
	UFUNCTION(BlueprintCallable, Category = "Missions")
	void ClearAllMissions();

	// ============================================================================
	// ACTIVE MISSIONS
	// ============================================================================

	// Get player's active missions
	UFUNCTION(BlueprintCallable, Category = "Missions")
	TArray<FActiveMission> GetPlayerActiveMissions(FString PlayerID) const;

	// Get active mission by ID
	UFUNCTION(BlueprintCallable, Category = "Missions")
	FActiveMission GetActiveMission(FString MissionID) const;

	// Update mission progress
	UFUNCTION(BlueprintCallable, Category = "Missions")
	bool UpdateMissionProgress(FString MissionID, float ProgressDelta);

	// Set mission objective
	UFUNCTION(BlueprintCallable, Category = "Missions")
	bool SetMissionObjective(FString MissionID, FString NewObjective);

	// Add collected commodity
	UFUNCTION(BlueprintCallable, Category = "Missions")
	bool AddCollectedCommodity(FString MissionID, FName CommodityID, int32 Quantity);

	// Check mission requirements
	UFUNCTION(BlueprintCallable, Category = "Missions")
	bool CheckMissionRequirements(FString MissionID) const;

	// ============================================================================
	// MISSION TEMPLATES
	// ============================================================================

	// Load mission templates
	UFUNCTION(BlueprintCallable, Category = "Missions")
	void LoadMissionTemplates(UDataTable* TemplateTable);

	// Add mission template
	UFUNCTION(BlueprintCallable, Category = "Missions")
	void AddMissionTemplate(const FMissionTemplate& Template);

	// Get mission template
	UFUNCTION(BlueprintCallable, Category = "Missions")
	FMissionTemplate GetMissionTemplate(ETradeMissionType MissionType) const;

	// Initialize default templates
	UFUNCTION(BlueprintCallable, Category = "Missions")
	void InitializeDefaultTemplates();

	// ============================================================================
	// MISSION REWARDS
	// ============================================================================

	// Calculate mission rewards
	UFUNCTION(BlueprintCallable, Category = "Missions")
	void CalculateMissionRewards(FTradeMission& Mission) const;

	// Pay mission rewards
	UFUNCTION(BlueprintCallable, Category = "Missions")
	void PayMissionRewards(FString MissionID, FString PlayerID);

	// Get reward multiplier for difficulty
	UFUNCTION(BlueprintCallable, Category = "Missions")
	float GetDifficultyMultiplier(EMissionDifficulty Difficulty) const;

	// ============================================================================
	// MISSION FILTERING & SEARCH
	// ============================================================================

	// Get missions by type
	UFUNCTION(BlueprintCallable, Category = "Missions")
	TArray<FTradeMission> GetMissionsByType(ETradeMissionType MissionType) const;

	// Get missions by faction
	UFUNCTION(BlueprintCallable, Category = "Missions")
	TArray<FTradeMission> GetMissionsByFaction(FString FactionID) const;

	// Get missions by difficulty
	UFUNCTION(BlueprintCallable, Category = "Missions")
	TArray<FTradeMission> GetMissionsByDifficulty(EMissionDifficulty Difficulty) const;

	// Search missions
	UFUNCTION(BlueprintCallable, Category = "Missions")
	TArray<FTradeMission> SearchMissions(const FString& SearchTerm) const;

	// ============================================================================
	// STATISTICS & TRACKING
	// ============================================================================

	// Get completed mission count
	UFUNCTION(BlueprintCallable, Category = "Missions")
	int32 GetCompletedMissionCount(FString PlayerID) const;

	// Get mission success rate
	UFUNCTION(BlueprintCallable, Category = "Missions")
	float GetMissionSuccessRate(FString PlayerID) const;

	// Get total credits earned from missions
	UFUNCTION(BlueprintCallable, Category = "Missions")
	float GetTotalCreditsEarned(FString PlayerID) const;

	// Get total reputation earned
	UFUNCTION(BlueprintCallable, Category = "Missions")
	float GetTotalReputationEarned(FString PlayerID) const;

	// Get favorite mission type
	UFUNCTION(BlueprintCallable, Category = "Missions")
	ETradeMissionType GetFavoriteMissionType(FString PlayerID) const;

	// Generate player mission statistics
	UFUNCTION(BlueprintCallable, Category = "Missions")
	FString GenerateMissionStatistics(FString PlayerID) const;

	// ============================================================================
	// EVENTS
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Mission Events")
	FOnMissionAvailable OnMissionAvailable;

	UPROPERTY(BlueprintAssignable, Category = "Mission Events")
	FOnMissionAccepted OnMissionAccepted;

	UPROPERTY(BlueprintAssignable, Category = "Mission Events")
	FOnMissionCompleted OnMissionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Mission Events")
	FOnMissionFailed OnMissionFailed;

	UPROPERTY(BlueprintAssignable, Category = "Mission Events")
	FOnMissionExpired OnMissionExpired;

	UPROPERTY(BlueprintAssignable, Category = "Mission Events")
	FOnMissionProgressUpdated OnMissionProgressUpdated;

protected:
	// Trade ship automation reference
	UPROPERTY()
	TWeakObjectPtr<UTradeShipAutomation> TradeShipAutomation;

	// Mission database: MissionID -> MissionData
	UPROPERTY()
	TMap<FString, FTradeMission> MissionDatabase;

	// Active missions: MissionID -> ActiveMissionData
	UPROPERTY()
	TMap<FString, FActiveMission> ActiveMissions;

	// Mission templates: MissionType -> Template
	UPROPERTY()
	TMap<ETradeMissionType, FMissionTemplate> MissionTemplates;

	// Player mission history: PlayerID -> CompletedMissionIDs
	UPROPERTY()
	TMap<FString, TArray<FString>> PlayerMissionHistory;

	// Mission statistics: PlayerID -> Statistics
	UPROPERTY()
	TMap<FString, TMap<FString, float>> MissionStatistics;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 MaxMissionsPerStation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MissionExpirationTime; // Hours

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MissionGenerationInterval; // Seconds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float BaseRewardMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bEnableMissionExpiration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bShowDebugMessages;

private:
	// Generation timer
	float MissionGenerationTimer;

	// Statistics
	int32 TotalMissionsGenerated;
	int32 TotalMissionsCompleted;
	int32 TotalMissionsFailed;

	// Helper functions
	void UpdateMissionTimers(float DeltaTime);
	void GenerateMissionsFromTemplates();
	void UpdateActiveMissionProgress(float DeltaTime);
	void CheckMissionCompletion(FActiveMission& ActiveMission);
	void ProcessMissionRewards(const FTradeMission& Mission, FString PlayerID);
	float CalculateDangerMultiplier(float DangerLevel) const;
	FString GenerateMissionID() const;
	void LogMissionEvent(const FString& MissionID, const FString& Event, const FString& Details) const;
	bool IsMissionExpired(const FTradeMission& Mission) const;
	void CleanUpOldMissions();
};