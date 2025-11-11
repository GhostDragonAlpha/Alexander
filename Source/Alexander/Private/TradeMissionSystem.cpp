// Copyright Epic Games, Inc. All Rights Reserved.

#include "TradeMissionSystem.h"
#include "TradeShipAutomation.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"

UTradeMissionSystem::UTradeMissionSystem()
{
	MaxMissionsPerStation = 10;
	MissionExpirationTime = 24.0f; // 24 hours
	MissionGenerationInterval = 300.0f; // 5 minutes
	BaseRewardMultiplier = 1.0f;
	bEnableMissionExpiration = true;
	bShowDebugMessages = false;

	MissionGenerationTimer = 0.0f;
	TotalMissionsGenerated = 0;
	TotalMissionsCompleted = 0;
	TotalMissionsFailed = 0;
}

void UTradeMissionSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("TradeMissionSystem initialized"));

	// Get trade ship automation
	UWorld* World = GetWorld();
	if (World)
	{
		TradeShipAutomation = World->GetSubsystem<UTradeShipAutomation>();
	}

	// Initialize default mission templates
	InitializeDefaultTemplates();
}

void UTradeMissionSystem::Deinitialize()
{
	// Clear all data
	MissionDatabase.Empty();
	ActiveMissions.Empty();
	MissionTemplates.Empty();
	PlayerMissionHistory.Empty();
	MissionStatistics.Empty();

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("TradeMissionSystem deinitialized"));
}

void UTradeMissionSystem::Tick(float DeltaTime)
{
	// Update mission timers
	UpdateMissionTimers(DeltaTime);

	// Generate missions periodically
	MissionGenerationTimer += DeltaTime;
	if (MissionGenerationTimer >= MissionGenerationInterval)
	{
		GenerateMissionsFromTemplates();
		MissionGenerationTimer = 0.0f;
	}

	// Update active mission progress
	UpdateActiveMissionProgress(DeltaTime);
}

TArray<FTradeMission> UTradeMissionSystem::GetAvailableMissions(FString StationID) const
{
	TArray<FTradeMission> AvailableMissions;

	for (const auto& Pair : MissionDatabase)
	{
		const FTradeMission& Mission = Pair.Value;
		
		// Check if mission is available and not expired
		if (Mission.Status == EMissionStatus::Available && !IsMissionExpired(Mission))
		{
			// Filter by station if specified
			if (StationID.IsEmpty() || Mission.StartStationID == StationID)
			{
				AvailableMissions.Add(Mission);
			}
		}
	}

	return AvailableMissions;
}

FTradeMission UTradeMissionSystem::GetMission(FString MissionID) const
{
	const FTradeMission* Mission = MissionDatabase.Find(MissionID);
	return Mission ? *Mission : FTradeMission();
}

bool UTradeMissionSystem::AcceptMission(FString MissionID, FString PlayerID, ASpaceship* AssignedShip)
{
	FTradeMission* Mission = MissionDatabase.Find(MissionID);
	if (!Mission || Mission->Status != EMissionStatus::Available)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot accept mission %s"), *MissionID);
		return false;
	}

	// Check if player already has this mission
	for (const auto& Pair : ActiveMissions)
	{
		if (Pair.Value.MissionData.MissionID == MissionID && Pair.Value.MissionData.AssignedPlayerID == PlayerID)
		{
			UE_LOG(LogTemp, Warning, TEXT("Player %s already has mission %s"), *PlayerID, *MissionID);
			return false;
		}
	}

	// Update mission status
	Mission->Status = EMissionStatus::Accepted;
	Mission->AssignedPlayerID = PlayerID;

	// Create active mission tracking
	FActiveMission ActiveMission;
	ActiveMission.MissionData = *Mission;
	ActiveMission.Progress = 0.0f;
	ActiveMission.CurrentObjective = TEXT("Proceed to start location");
	ActiveMission.AcceptanceTime = FDateTime::Now();
	ActiveMission.AssignedShip = AssignedShip;
	ActiveMission.bIsTimeLimited = (Mission->ExpirationTime > Mission->CreationTime);

	ActiveMissions.Add(MissionID, ActiveMission);

	// Add to player mission history
	PlayerMissionHistory.FindOrAdd(PlayerID).Add(MissionID);

	// Broadcast mission acceptance
	OnMissionAccepted.Broadcast(*Mission);

	UE_LOG(LogTemp, Log, TEXT("Mission %s accepted by player %s"), *MissionID, *PlayerID);

	return true;
}

bool UTradeMissionSystem::CompleteMission(FString MissionID)
{
	FTradeMission* Mission = MissionDatabase.Find(MissionID);
	FActiveMission* ActiveMission = ActiveMissions.Find(MissionID);

	if (!Mission || !ActiveMission)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot complete mission %s - not found"), *MissionID);
		return false;
	}

	// Check if mission requirements are met
	if (!CheckMissionRequirements(MissionID))
	{
		UE_LOG(LogTemp, Warning, TEXT("Mission %s requirements not met"), *MissionID);
		return false;
	}

	// Update mission status
	Mission->Status = EMissionStatus::Completed;
	Mission->CompletionTime = FDateTime::Now();
	ActiveMission->Progress = 100.0f;

	// Pay rewards
	PayMissionRewards(MissionID, Mission->AssignedPlayerID);

	// Update statistics
	TotalMissionsCompleted++;
	UpdateMissionStatistics(Mission->AssignedPlayerID, *Mission, true);

	// Remove from active missions
	ActiveMissions.Remove(MissionID);

	// Broadcast completion
	OnMissionCompleted.Broadcast(*Mission);

	UE_LOG(LogTemp, Log, TEXT("Mission %s completed by player %s"), *MissionID, *Mission->AssignedPlayerID);

	return true;
}

bool UTradeMissionSystem::FailMission(FString MissionID, FString Reason)
{
	FTradeMission* Mission = MissionDatabase.Find(MissionID);
	FActiveMission* ActiveMission = ActiveMissions.Find(MissionID);

	if (!Mission || !ActiveMission)
	{
		return false;
	}

	// Update mission status
	Mission->Status = EMissionStatus::Failed;
	Mission->FailureReason = Reason;

	// Update statistics
	TotalMissionsFailed++;
	UpdateMissionStatistics(Mission->AssignedPlayerID, *Mission, false);

	// Remove from active missions
	ActiveMissions.Remove(MissionID);

	// Broadcast failure
	OnMissionFailed.Broadcast(*Mission, Reason);

	UE_LOG(LogTemp, Log, TEXT("Mission %s failed: %s"), *MissionID, *Reason);

	return true;
}

bool UTradeMissionSystem::CancelMission(FString MissionID)
{
	FTradeMission* Mission = MissionDatabase.Find(MissionID);
	if (!Mission)
	{
		return false;
	}

	// Update mission status
	Mission->Status = EMissionStatus::Cancelled;

	// Remove from active missions
	ActiveMissions.Remove(MissionID);

	UE_LOG(LogTemp, Log, TEXT("Mission %s cancelled"), *MissionID);

	return true;
}

bool UTradeMissionSystem::AbandonMission(FString MissionID)
{
	FTradeMission* Mission = MissionDatabase.Find(MissionID);
	if (!Mission)
	{
		return false;
	}

	// Update mission status
	Mission->Status = EMissionStatus::Failed;
	Mission->FailureReason = TEXT("Mission abandoned by player");

	// Apply reputation penalty
	// Would modify faction reputation here

	// Remove from active missions
	ActiveMissions.Remove(MissionID);

	UE_LOG(LogTemp, Log, TEXT("Mission %s abandoned"), *MissionID);

	return true;
}

void UTradeMissionSystem::GenerateMissionsForStation(FString StationID, int32 Count)
{
	for (int32 i = 0; i < Count; i++)
	{
		// Random mission type
		ETradeMissionType MissionType = static_cast<ETradeMissionType>(FMath::RandRange(0, (int32)ETradeMissionType::Diplomatic));
		
		// Random faction
		FString FactionID = FString::Printf(TEXT("Faction_%d"), FMath::RandRange(1, 5));
		
		FTradeMission Mission = GenerateMission(MissionType, FactionID, StationID);
		MissionDatabase.Add(Mission.MissionID, Mission);
		TotalMissionsGenerated++;

		// Broadcast new mission
		OnMissionAvailable.Broadcast(Mission);
	}

	UE_LOG(LogTemp, Log, TEXT("Generated %d missions for station %s"), Count, *StationID);
}

void UTradeMissionSystem::GenerateFactionMissions(FString FactionID, int32 Count)
{
	for (int32 i = 0; i < Count; i++)
	{
		// Random mission type (weighted towards certain types for factions)
		ETradeMissionType MissionType = ETradeMissionType::SupplyRun; // Simplified
		
		FTradeMission Mission = GenerateMission(MissionType, FactionID, "");
		MissionDatabase.Add(Mission.MissionID, Mission);
		TotalMissionsGenerated++;

		OnMissionAvailable.Broadcast(Mission);
	}

	UE_LOG(LogTemp, Log, TEXT("Generated %d missions for faction %s"), Count, *FactionID);
}

FTradeMission UTradeMissionSystem::GenerateMission(ETradeMissionType MissionType, FString IssuingFactionID, FString StationID)
{
	FTradeMission Mission;
	Mission.MissionID = GenerateMissionID();
	Mission.MissionType = MissionType;
	Mission.IssuingFactionID = IssuingFactionID;
	Mission.StartStationID = StationID.IsEmpty() ? FString::Printf(TEXT("Station_%d"), FMath::RandRange(1, 10)) : StationID;
	Mission.EndStationID = FString::Printf(TEXT("Station_%d"), FMath::RandRange(1, 10));
	Mission.CreationTime = FDateTime::Now();
	Mission.ExpirationTime = Mission.CreationTime + FTimespan(MissionExpirationTime, 0, 0);
	Mission.Status = EMissionStatus::Available;
	Mission.bIsRepeatable = FMath::RandBool();
	Mission.MaxCompletions = Mission.bIsRepeatable ? FMath::RandRange(3, 10) : 1;

	// Get template for this mission type
	FMissionTemplate Template = GetMissionTemplate(MissionType);
	Mission.Difficulty = Template.BaseDifficulty;
	Mission.Title = Template.TitleFormat;
	Mission.Description = Template.DescriptionFormat;
	Mission.DangerLevel = Template.BaseDangerLevel;

	// Generate commodities based on mission type
	switch (MissionType)
	{
		case ETradeMissionType::Courier:
			Mission.RequiredCommodities.Add(FName("ConsumerGoods"));
			Mission.RequiredQuantities.Add(FMath::RandRange(10, 50));
			Mission.Title = FString::Printf(TEXT("Courier Run - %s"), *Mission.RequiredCommodities[0].ToString());
			break;

		case ETradeMissionType::SupplyRun:
			Mission.RequiredCommodities.Add(FName("FoodSupplies"));
			Mission.RequiredQuantities.Add(FMath::RandRange(20, 100));
			Mission.Title = FString::Printf(TEXT("Supply Delivery to %s"), *Mission.EndStationID);
			break;

		case ETradeMissionType::Smuggling:
			Mission.RequiredCommodities.Add(FName("Contraband"));
			Mission.RequiredQuantities.Add(FMath::RandRange(5, 25));
			Mission.Title = TEXT("Special Delivery");
			Mission.Description = TEXT("Transport sensitive cargo without detection");
			Mission.DangerLevel = 0.7f;
			break;

		case ETradeMissionType::TradeContract:
			Mission.RequiredCommodities.Add(FName("ElectronicComponents"));
			Mission.RequiredQuantities.Add(FMath::RandRange(15, 75));
			Mission.Title = TEXT("Trade Contract Fulfillment");
			break;

		case ETradeMissionType::Escort:
			Mission.Title = FString::Printf(TEXT("Escort Convoy to %s"), *Mission.EndStationID);
			Mission.Description = TEXT("Protect a trade convoy along its route");
			Mission.DangerLevel = 0.5f;
			break;

		case ETradeMissionType::MarketManipulation:
			Mission.Title = TEXT("Market Manipulation");
			Mission.Description = TEXT("Create artificial shortage to drive up prices");
			Mission.DangerLevel = 0.8f;
			break;

		case ETradeMissionType::Rescue:
			Mission.Title = TEXT("Rescue Operation");
			Mission.Description = TEXT("Rescue stranded traders and recover cargo");
			Mission.DangerLevel = 0.6f;
			break;

		case ETradeMissionType::Salvage:
			Mission.RequiredCommodities.Add(FName("StolenTech"));
			Mission.RequiredQuantities.Add(FMath::RandRange(1, 10));
			Mission.Title = TEXT("Salvage Operation");
			Mission.Description = TEXT("Recover valuable technology from wreckage");
			break;

		case ETradeMissionType::Exploration:
			Mission.Title = TEXT("Trade Route Exploration");
			Mission.Description = TEXT("Discover new profitable trade routes");
			break;

		case ETradeMissionType::Diplomatic:
			Mission.Title = TEXT("Diplomatic Courier");
			Mission.Description = TEXT("Deliver important diplomatic documents");
			Mission.DangerLevel = 0.3f;
			break;
	}

	// Calculate rewards
	CalculateMissionRewards(Mission);

	return Mission;
}

void UTradeMissionSystem::ClearExpiredMissions()
{
	TArray<FString> MissionsToRemove;

	for (auto& Pair : MissionDatabase)
	{
		if (IsMissionExpired(Pair.Value) && Pair.Value.Status == EMissionStatus::Available)
		{
			MissionsToRemove.Add(Pair.Key);
			OnMissionExpired.Broadcast(Pair.Value);
		}
	}

	// Remove expired missions
	for (const FString& MissionID : MissionsToRemove)
	{
		MissionDatabase.Remove(MissionID);
		UE_LOG(LogTemp, Log, TEXT("Expired mission removed: %s"), *MissionID);
	}
}

void UTradeMissionSystem::ClearAllMissions()
{
	MissionDatabase.Empty();
	ActiveMissions.Empty();
	UE_LOG(LogTemp, Log, TEXT("All missions cleared"));
}

TArray<FActiveMission> UTradeMissionSystem::GetPlayerActiveMissions(FString PlayerID) const
{
	TArray<FActiveMission> PlayerMissions;

	for (const auto& Pair : ActiveMissions)
	{
		if (Pair.Value.MissionData.AssignedPlayerID == PlayerID)
		{
			PlayerMissions.Add(Pair.Value);
		}
	}

	return PlayerMissions;
}

FActiveMission UTradeMissionSystem::GetActiveMission(FString MissionID) const
{
	const FActiveMission* ActiveMission = ActiveMissions.Find(MissionID);
	return ActiveMission ? *ActiveMission : FActiveMission();
}

bool UTradeMissionSystem::UpdateMissionProgress(FString MissionID, float ProgressDelta)
{
	FActiveMission* ActiveMission = ActiveMissions.Find(MissionID);
	if (!ActiveMission)
	{
		return false;
	}

	ActiveMission->Progress = FMath::Clamp(ActiveMission->Progress + ProgressDelta, 0.0f, 100.0f);

	// Check for completion
	if (ActiveMission->Progress >= 100.0f)
	{
		CheckMissionCompletion(*ActiveMission);
	}

	// Broadcast progress update
	OnMissionProgressUpdated.Broadcast(*ActiveMission);

	return true;
}

bool UTradeMissionSystem::SetMissionObjective(FString MissionID, FString NewObjective)
{
	FActiveMission* ActiveMission = ActiveMissions.Find(MissionID);
	if (!ActiveMission)
	{
		return false;
	}

	ActiveMission->CurrentObjective = NewObjective;
	return true;
}

bool UTradeMissionSystem::AddCollectedCommodity(FString MissionID, FName CommodityID, int32 Quantity)
{
	FActiveMission* ActiveMission = ActiveMissions.Find(MissionID);
	if (!ActiveMission)
	{
		return false;
	}

	// Add to collected commodities
	ActiveMission->CollectedCommodities.Add(CommodityID);
	
	// Update progress based on commodity collection
	float ProgressPerItem = 100.0f / ActiveMission->MissionData.RequiredCommodities.Num();
	UpdateMissionProgress(MissionID, ProgressPerItem * Quantity);

	return true;
}

bool UTradeMissionSystem::CheckMissionRequirements(FString MissionID) const
{
	const FTradeMission* Mission = MissionDatabase.Find(MissionID);
	const FActiveMission* ActiveMission = ActiveMissions.Find(MissionID);

	if (!Mission || !ActiveMission)
	{
		return false;
	}

	// Check if all required commodities are collected
	for (int32 i = 0; i < Mission->RequiredCommodities.Num(); i++)
	{
		FName RequiredCommodity = Mission->RequiredCommodities[i];
		int32 RequiredQuantity = Mission->RequiredQuantities[i];
		
		// Simplified check - would check actual cargo
		bool bHasCommodity = ActiveMission->CollectedCommodities.Contains(RequiredCommodity);
		if (!bHasCommodity)
		{
			return false;
		}
	}

	return true;
}

void UTradeMissionSystem::LoadMissionTemplates(UDataTable* TemplateTable)
{
	if (!TemplateTable)
	{
		return;
	}

	// Would load from data table
	UE_LOG(LogTemp, Log, TEXT("Mission templates loaded from data table"));
}

void UTradeMissionSystem::AddMissionTemplate(const FMissionTemplate& Template)
{
	MissionTemplates.Add(Template.MissionType, Template);
}

FMissionTemplate UTradeMissionSystem::GetMissionTemplate(ETradeMissionType MissionType) const
{
	const FMissionTemplate* Template = MissionTemplates.Find(MissionType);
	return Template ? *Template : FMissionTemplate();
}

void UTradeMissionSystem::InitializeDefaultTemplates()
{
	// Courier Template
	{
		FMissionTemplate Template;
		Template.MissionType = ETradeMissionType::Courier;
		Template.BaseDifficulty = EMissionDifficulty::Easy;
		Template.BaseRewardCredits = 1500.0f;
		Template.BaseRewardReputation = 15.0f;
		Template.BaseDangerLevel = 0.1f;
		Template.TitleFormat = TEXT("Courier Delivery");
		Template.DescriptionFormat = TEXT("Deliver important documents and small cargo");
		Template.bIsRepeatable = true;
		Template.MaxSpawns = 5;
		Template.SpawnChance = 0.8f;
		MissionTemplates.Add(ETradeMissionType::Courier, Template);
	}

	// Supply Run Template
	{
		FMissionTemplate Template;
		Template.MissionType = ETradeMissionType::SupplyRun;
		Template.BaseDifficulty = EMissionDifficulty::Easy;
		Template.BaseRewardCredits = 2500.0f;
		Template.BaseRewardReputation = 20.0f;
		Template.BaseDangerLevel = 0.2f;
		Template.TitleFormat = TEXT("Supply Delivery");
		Template.DescriptionFormat = TEXT("Deliver essential supplies to a station in need");
		Template.bIsRepeatable = true;
		Template.MaxSpawns = 3;
		Template.SpawnChance = 0.6f;
		MissionTemplates.Add(ETradeMissionType::SupplyRun, Template);
	}

	// Smuggling Template
	{
		FMissionTemplate Template;
		Template.MissionType = ETradeMissionType::Smuggling;
		Template.BaseDifficulty = EMissionDifficulty::Moderate;
		Template.BaseRewardCredits = 5000.0f;
		Template.BaseRewardReputation = -10.0f; // Negative reputation with authorities
		Template.BaseDangerLevel = 0.7f;
		Template.TitleFormat = TEXT("Special Cargo Transport");
		Template.DescriptionFormat = TEXT("Transport sensitive cargo without detection");
		Template.bIsRepeatable = true;
		Template.MaxSpawns = 2;
		Template.SpawnChance = 0.3f;
		MissionTemplates.Add(ETradeMissionType::Smuggling, Template);
	}

	// Trade Contract Template
	{
		FMissionTemplate Template;
		Template.MissionType = ETradeMissionType::TradeContract;
		Template.BaseDifficulty = EMissionDifficulty::Moderate;
		Template.BaseRewardCredits = 4000.0f;
		Template.BaseRewardReputation = 25.0f;
		Template.BaseDangerLevel = 0.3f;
		Template.TitleFormat = TEXT("Trade Contract");
		Template.DescriptionFormat = TEXT("Fulfill a large trade contract for a faction");
		Template.bIsRepeatable = false;
		Template.MaxSpawns = 1;
		Template.SpawnChance = 0.4f;
		MissionTemplates.Add(ETradeMissionType::TradeContract, Template);
	}

	// Escort Template
	{
		FMissionTemplate Template;
		Template.MissionType = ETradeMissionType::Escort;
		Template.BaseDifficulty = EMissionDifficulty::Hard;
		Template.BaseRewardCredits = 6000.0f;
		Template.BaseRewardReputation = 30.0f;
		Template.BaseDangerLevel = 0.6f;
		Template.TitleFormat = TEXT("Convoy Escort");
		Template.DescriptionFormat = TEXT("Protect a trade convoy from pirates");
		Template.bIsRepeatable = true;
		Template.MaxSpawns = 2;
		Template.SpawnChance = 0.5f;
		MissionTemplates.Add(ETradeMissionType::Escort, Template);
	}

	UE_LOG(LogTemp, Log, TEXT("Initialized %d mission templates"), MissionTemplates.Num());
}

void UTradeMissionSystem::CalculateMissionRewards(FTradeMission& Mission) const
{
	// Get difficulty multiplier
	float DifficultyMultiplier = GetDifficultyMultiplier(Mission.Difficulty);

	// Get template for base values
	FMissionTemplate Template = GetMissionTemplate(Mission.MissionType);

	// Calculate scaled rewards
	Mission.RewardCredits = Template.BaseRewardCredits * DifficultyMultiplier * BaseRewardMultiplier;
	Mission.RewardReputation = Template.BaseRewardReputation * DifficultyMultiplier;
	Mission.RewardExperience = Template.BaseRewardReputation * DifficultyMultiplier * 10.0f; // 10 XP per rep point

	// Adjust for danger level
	float DangerBonus = CalculateDangerMultiplier(Mission.DangerLevel);
	Mission.RewardCredits *= DangerBonus;
}

void UTradeMissionSystem::PayMissionRewards(FString MissionID, FString PlayerID)
{
	const FTradeMission* Mission = MissionDatabase.Find(MissionID);
	if (!Mission)
	{
		return;
	}

	// Would actually pay rewards to player
	UE_LOG(LogTemp, Log, TEXT("Mission rewards paid to player %s: %.0f credits, %.0f reputation, %.0f XP"),
		*PlayerID,
		Mission->RewardCredits,
		Mission->RewardReputation,
		Mission->RewardExperience);
}

float UTradeMissionSystem::GetDifficultyMultiplier(EMissionDifficulty Difficulty) const
{
	switch (Difficulty)
	{
		case EMissionDifficulty::Trivial: return 0.5f;
		case EMissionDifficulty::Easy: return 1.0f;
		case EMissionDifficulty::Moderate: return 2.0f;
		case EMissionDifficulty::Hard: return 4.0f;
		case EMissionDifficulty::Extreme: return 8.0f;
		case EMissionDifficulty::Legendary: return 16.0f;
		default: return 1.0f;
	}
}

TArray<FTradeMission> UTradeMissionSystem::GetMissionsByType(ETradeMissionType MissionType) const
{
	TArray<FTradeMission> Missions;

	for (const auto& Pair : MissionDatabase)
	{
		if (Pair.Value.MissionType == MissionType)
		{
			Missions.Add(Pair.Value);
		}
	}

	return Missions;
}

TArray<FTradeMission> UTradeMissionSystem::GetMissionsByFaction(FString FactionID) const
{
	TArray<FTradeMission> Missions;

	for (const auto& Pair : MissionDatabase)
	{
		if (Pair.Value.IssuingFactionID == FactionID)
		{
			Missions.Add(Pair.Value);
		}
	}

	return Missions;
}

TArray<FTradeMission> UTradeMissionSystem::GetMissionsByDifficulty(EMissionDifficulty Difficulty) const
{
	TArray<FTradeMission> Missions;

	for (const auto& Pair : MissionDatabase)
	{
		if (Pair.Value.Difficulty == Difficulty)
		{
			Missions.Add(Pair.Value);
		}
	}

	return Missions;
}

TArray<FTradeMission> UTradeMissionSystem::SearchMissions(const FString& SearchTerm) const
{
	TArray<FTradeMission> Results;

	for (const auto& Pair : MissionDatabase)
	{
		if (Pair.Value.Title.Contains(SearchTerm) || Pair.Value.Description.Contains(SearchTerm))
		{
			Results.Add(Pair.Value);
		}
	}

	return Results;
}

int32 UTradeMissionSystem::GetCompletedMissionCount(FString PlayerID) const
{
	const TArray<FString>* History = PlayerMissionHistory.Find(PlayerID);
	if (!History)
	{
		return 0;
	}

	int32 CompletedCount = 0;
	for (const FString& MissionID : *History)
	{
		const FTradeMission* Mission = MissionDatabase.Find(MissionID);
		if (Mission && Mission->Status == EMissionStatus::Completed)
		{
			CompletedCount++;
		}
	}

	return CompletedCount;
}

float UTradeMissionSystem::GetMissionSuccessRate(FString PlayerID) const
{
	const TArray<FString>* History = PlayerMissionHistory.Find(PlayerID);
	if (!History || History->Num() == 0)
	{
		return 0.0f;
	}

	int32 CompletedCount = GetCompletedMissionCount(PlayerID);
	return (float)CompletedCount / History->Num();
}

float UTradeMissionSystem::GetTotalCreditsEarned(FString PlayerID) const
{
	const TArray<FString>* History = PlayerMissionHistory.Find(PlayerID);
	if (!History)
	{
		return 0.0f;
	}

	float TotalCredits = 0.0f;
	for (const FString& MissionID : *History)
	{
		const FTradeMission* Mission = MissionDatabase.Find(MissionID);
		if (Mission && Mission->Status == EMissionStatus::Completed)
		{
			TotalCredits += Mission->RewardCredits;
		}
	}

	return TotalCredits;
}

float UTradeMissionSystem::GetTotalReputationEarned(FString PlayerID) const
{
	const TArray<FString>* History = PlayerMissionHistory.Find(PlayerID);
	if (!History)
	{
		return 0.0f;
	}

	float TotalReputation = 0.0f;
	for (const FString& MissionID : *History)
	{
		const FTradeMission* Mission = MissionDatabase.Find(MissionID);
		if (Mission && Mission->Status == EMissionStatus::Completed)
		{
			TotalReputation += Mission->RewardReputation;
		}
	}

	return TotalReputation;
}

ETradeMissionType UTradeMissionSystem::GetFavoriteMissionType(FString PlayerID) const
{
	const TArray<FString>* History = PlayerMissionHistory.Find(PlayerID);
	if (!History)
	{
		return ETradeMissionType::Courier;
	}

	TMap<ETradeMissionType, int32> TypeCounts;

	for (const FString& MissionID : *History)
	{
		const FTradeMission* Mission = MissionDatabase.Find(MissionID);
		if (Mission && Mission->Status == EMissionStatus::Completed)
		{
			TypeCounts.FindOrAdd(Mission->MissionType)++;
		}
	}

	// Find most common type
	ETradeMissionType FavoriteType = ETradeMissionType::Courier;
	int32 HighestCount = 0;

	for (const auto& Pair : TypeCounts)
	{
		if (Pair.Value > HighestCount)
		{
			HighestCount = Pair.Value;
			FavoriteType = Pair.Key;
		}
	}

	return FavoriteType;
}

FString UTradeMissionSystem::GenerateMissionStatistics(FString PlayerID) const
{
	FString Report = FString::Printf(TEXT("=== MISSION STATISTICS FOR PLAYER: %s ===\n\n"), *PlayerID);

	Report += FString::Printf(TEXT("Total Missions Completed: %d\n"), GetCompletedMissionCount(PlayerID));
	Report += FString::Printf(TEXT("Mission Success Rate: %.1f%%\n"), GetMissionSuccessRate(PlayerID) * 100.0f);
	Report += FString::Printf(TEXT("Total Credits Earned: %.0f\n"), GetTotalCreditsEarned(PlayerID));
	Report += FString::Printf(TEXT("Total Reputation Earned: %.0f\n"), GetTotalReputationEarned(PlayerID));
	Report += FString::Printf(TEXT("Favorite Mission Type: %d\n"), (int32)GetFavoriteMissionType(PlayerID));

	return Report;
}

void UTradeMissionSystem::UpdateMissionTimers(float DeltaTime)
{
	// Check for mission expiration
	if (bEnableMissionExpiration)
	{
		ClearExpiredMissions();
	}
}

void UTradeMissionSystem::GenerateMissionsFromTemplates()
{
	// Generate missions based on templates
	for (const auto& TemplatePair : MissionTemplates)
	{
		const FMissionTemplate& Template = TemplatePair.Value;
		
		// Check spawn chance
		if (FMath::FRand() <= Template.SpawnChance)
		{
			// Check max spawns
			TArray<FTradeMission> ExistingMissions = GetMissionsByType(Template.MissionType);
			int32 CurrentCount = 0;
			
			for (const FTradeMission& Mission : ExistingMissions)
			{
				if (Mission.Status == EMissionStatus::Available)
				{
					CurrentCount++;
				}
			}

			if (CurrentCount < Template.MaxSpawns)
			{
				// Generate mission
				FString FactionID = FString::Printf(TEXT("Faction_%d"), FMath::RandRange(1, 5));
				FString StationID = FString::Printf(TEXT("Station_%d"), FMath::RandRange(1, 10));
				
				FTradeMission Mission = GenerateMission(Template.MissionType, FactionID, StationID);
				MissionDatabase.Add(Mission.MissionID, Mission);
				TotalMissionsGenerated++;

				OnMissionAvailable.Broadcast(Mission);
			}
		}
	}
}

void UTradeMissionSystem::UpdateActiveMissionProgress(float DeltaTime)
{
	// Update progress for time-based missions
	for (auto& Pair : ActiveMissions)
	{
		FActiveMission& ActiveMission = Pair.Value;
		
		// Check for time expiration
		if (ActiveMission.bIsTimeLimited)
		{
			FTimespan Elapsed = FDateTime::Now() - ActiveMission.AcceptanceTime;
			if (Elapsed >= ActiveMission.TimeLimit)
			{
				// Mission failed due to time limit
				FailMission(Pair.Key, TEXT("Mission time limit expired"));
			}
		}

		// Auto-progress for certain mission types
		switch (ActiveMission.MissionData.MissionType)
		{
			case ETradeMissionType::Courier:
			case ETradeMissionType::SupplyRun:
				// Progress based on travel (simplified)
				if (ActiveMission.Progress < 50.0f)
				{
					ActiveMission.Progress += DeltaTime * 5.0f; // 5% per second while traveling
				}
				break;
		}
	}
}

void UTradeMissionSystem::CheckMissionCompletion(FActiveMission& ActiveMission)
{
	// Check if mission requirements are met
	if (CheckMissionRequirements(ActiveMission.MissionData.MissionID))
	{
		CompleteMission(ActiveMission.MissionData.MissionID);
	}
}

void UTradeMissionSystem::ProcessMissionRewards(const FTradeMission& Mission, FString PlayerID)
{
	// Would actually pay rewards to player
	UE_LOG(LogTemp, Log, TEXT("Mission rewards processed for player %s"), *PlayerID);
}

float UTradeMissionSystem::CalculateDangerMultiplier(float DangerLevel) const
{
	// Higher danger = higher rewards
	return 1.0f + (DangerLevel * 0.5f);
}

FString UTradeMissionSystem::GenerateMissionID() const
{
	return FString::Printf(TEXT("Mission_%d"), FMath::RandRange(10000, 99999));
}

void UTradeMissionSystem::LogMissionEvent(const FString& MissionID, const FString& Event, const FString& Details) const
{
	if (bShowDebugMessages)
	{
		UE_LOG(LogTemp, Log, TEXT("[MissionSystem] %s - %s: %s"), *MissionID, *Event, *Details);
	}
}

bool UTradeMissionSystem::IsMissionExpired(const FTradeMission& Mission) const
{
	if (!bEnableMissionExpiration)
	{
		return false;
	}

	FDateTime CurrentTime = FDateTime::Now();
	return CurrentTime > Mission.ExpirationTime;
}

void UTradeMissionSystem::CleanUpOldMissions()
{
	// Remove old completed/failed missions
	TArray<FString> MissionsToRemove;

	for (const auto& Pair : MissionDatabase)
	{
		const FTradeMission& Mission = Pair.Value;
		
		// Remove missions that are completed/failed and older than 1 day
		if ((Mission.Status == EMissionStatus::Completed || Mission.Status == EMissionStatus::Failed) &&
			(FDateTime::Now() - Mission.CompletionTime).GetTotalHours() > 24.0f)
		{
			MissionsToRemove.Add(Pair.Key);
		}
	}

	for (const FString& MissionID : MissionsToRemove)
	{
		MissionDatabase.Remove(MissionID);
	}
}

void UTradeMissionSystem::UpdateMissionStatistics(FString PlayerID, const FTradeMission& Mission, bool bSuccess)
{
	TMap<FString, float>& Stats = MissionStatistics.FindOrAdd(PlayerID);
	
	// Update general stats
	Stats.FindOrAdd(TEXT("TotalMissions")) += 1.0f;
	Stats.FindOrAdd(TEXT("TotalCredits")) += Mission.RewardCredits;
	Stats.FindOrAdd(TEXT("TotalReputation")) += Mission.RewardReputation;
	
	if (bSuccess)
	{
		Stats.FindOrAdd(TEXT("SuccessfulMissions")) += 1.0f;
	}
	else
	{
		Stats.FindOrAdd(TEXT("FailedMissions")) += 1.0f;
	}

	// Update type-specific stats
	FString TypeKey = FString::Printf(TEXT("Type_%d"), (int32)Mission.MissionType);
	Stats.FindOrAdd(TypeKey) += 1.0f;
}