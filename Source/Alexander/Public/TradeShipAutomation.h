// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
// #include "TradingEconomySystem.h" // Removed to break circular dependency
#include "Spaceship.h"
#include "TradeShipAutomation.generated.h"

// Forward declarations
class ATradeStation;
class UTradeMissionSystem;

/**
 * Trade ship classes
 */
UENUM(BlueprintType)
enum class ETradeShipClass : uint8
{
	LightFreighter		UMETA(DisplayName = "Light Freighter"),
	HeavyFreighter		UMETA(DisplayName = "Heavy Freighter"),
	BlockadeRunner		UMETA(DisplayName = "Blockade Runner"),
	TradeCruiser		UMETA(DisplayName = "Trade Cruiser"),
	Transport			UMETA(DisplayName = "Transport"),
	Smuggler			UMETA(DisplayName = "Smuggler"),
	MerchantRaider		UMETA(DisplayName = "Merchant Raider")
};

/**
 * Automated trade route status
 */
UENUM(BlueprintType)
enum class EAutomationStatus : uint8
{
	Idle			UMETA(DisplayName = "Idle"),
	Active			UMETA(DisplayName = "Active"),
	Traveling		UMETA(DisplayName = "Traveling"),
	Loading			UMETA(DisplayName = "Loading"),
	Unloading		UMETA(DisplayName = "Unloading"),
	Trading			UMETA(DisplayName = "Trading"),
	Waiting			UMETA(DisplayName = "Waiting"),
	UnderAttack		UMETA(DisplayName = "Under Attack"),
	Disabled		UMETA(DisplayName = "Disabled"),
	Destroyed		UMETA(DisplayName = "Destroyed")
};

/**
 * Trade route efficiency
 */
UENUM(BlueprintType)
enum class ERouteEfficiency : uint8
{
	Terrible	UMETA(DisplayName = "Terrible"),
	VeryPoor	UMETA(DisplayName = "Very Poor"),
	Poor		UMETA(DisplayName = "Poor"),
	Average		UMETA(DisplayName = "Average"),
	Good		UMETA(DisplayName = "Good"),
	Excellent	UMETA(DisplayName = "Excellent")
};

/**
 * Automated trade route
 */
USTRUCT(BlueprintType)
struct FAutomatedTradeRoute
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FString RouteID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FString StartStationID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FString EndStationID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	TArray<FName> TradeCommodities; // Commodities to trade

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	TMap<FName, float> BuyPriceLimits; // Max price to pay

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	TMap<FName, float> SellPriceLimits; // Min price to sell

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	int32 MinCargoLoad; // Minimum cargo before traveling

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float ProfitMarginTarget; // Target profit margin

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	bool bAvoidDangerousRoutes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	bool bUseBlackMarket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float RoutePriority; // 0-1 (higher = more important)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float EstimatedProfit; // Estimated profit for this route

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float DangerLevel; // 0-1 (higher = more dangerous)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	ERouteEfficiency Efficiency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float TotalProfit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	int32 TotalRuns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float AverageProfitPerRun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float EstimatedDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	bool bIsActive;

	FAutomatedTradeRoute()
	{
		MinCargoLoad = 50; // 50% cargo capacity
		ProfitMarginTarget = 0.2f; // 20% profit margin
		bAvoidDangerousRoutes = true;
		bUseBlackMarket = false;
		RoutePriority = 0.5f;
		Efficiency = ERouteEfficiency::Average;
		TotalProfit = 0.0f;
		TotalRuns = 0;
		AverageProfitPerRun = 0.0f;
		EstimatedDuration = 3600.0f; // 1 hour default
		bIsActive = true;
	}
};

/**
 * Automated ship data
 */
USTRUCT(BlueprintType)
struct FAutomatedShipData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	TWeakObjectPtr<ASpaceship> Ship;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	ETradeShipClass ShipClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	FString CurrentRouteID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	EAutomationStatus CurrentStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	float CargoCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	float CurrentCargoLoad;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	float TravelSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	float DefenseRating;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	float EvasionRating;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	float CurrentShield;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	float MaxShield;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	float CurrentFuel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	float MaxFuel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	float TotalProfitGenerated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	float TotalProfit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	int32 SuccessfulTrades;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	int32 FailedTrades;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	int32 TimesAttacked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	FString OwnerPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	bool bIsPlayerControlled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	TArray<FString> PreferredRoutes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	EAutomationStatus AutomationStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	float RouteProgress;

	FAutomatedShipData()
	{
		ShipClass = ETradeShipClass::LightFreighter;
		CurrentStatus = EAutomationStatus::Idle;
		CargoCapacity = 100.0f;
		CurrentCargoLoad = 0.0f;
		TravelSpeed = 1000.0f;
		DefenseRating = 10.0f;
		EvasionRating = 20.0f;
		CurrentHealth = 100.0f;
		MaxHealth = 100.0f;
		CurrentShield = 100.0f;
		MaxShield = 100.0f;
		CurrentFuel = 100.0f;
		MaxFuel = 100.0f;
		TotalProfitGenerated = 0.0f;
		SuccessfulTrades = 0;
		FailedTrades = 0;
		TimesAttacked = 0;
		bIsPlayerControlled = false;
		AutomationStatus = EAutomationStatus::Idle;
		RouteProgress = 0.0f;
	}
};

/**
 * Fleet management data
 */
USTRUCT(BlueprintType)
struct FTradeFleetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	FString FleetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	FString FleetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	FString OwnerPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	TArray<FString> ShipIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	float TotalFleetProfit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	float MonthlyOperatingCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	TMap<FString, float> ProfitByRoute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	ERouteEfficiency FleetEfficiency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	bool bIsActive;

	FTradeFleetData()
	{
		TotalFleetProfit = 0.0f;
		MonthlyOperatingCost = 10000.0f;
		FleetEfficiency = ERouteEfficiency::Average;
		bIsActive = true;
	}
};

/**
 * Trade automation settings
 */
USTRUCT(BlueprintType)
struct FTradeAutomationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAutoBuy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAutoSell;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float MinProfitMargin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float MaxDangerLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAvoidHostileFactions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bUseJumpDrives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float FuelReservePercentage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAutoRepair;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAutoRefuel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 MaxRouteRetries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bEnableAutomaticFlee;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float FuelReserveThreshold;

	FTradeAutomationSettings()
	{
		bAutoBuy = true;
		bAutoSell = true;
		MinProfitMargin = 0.15f; // 15% minimum profit
		MaxDangerLevel = 0.7f; // Avoid routes > 70% danger
		bAvoidHostileFactions = true;
		bUseJumpDrives = true;
		FuelReservePercentage = 0.2f; // Keep 20% fuel reserve
		bAutoRepair = true;
		bAutoRefuel = true;
		MaxRouteRetries = 3;
		bEnableAutomaticFlee = true;
		FuelReserveThreshold = 0.1f; // 10% fuel threshold for fleeing
	}
};

// Status information for automated trade routes
USTRUCT(BlueprintType)
struct FAutomatedTradeRouteStatus
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	FString RouteID;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	bool bIsActive;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	float CurrentProgress;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	float EstimatedCompletionTime;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	float TotalProfit;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	int32 ShipsAssigned;

	FAutomatedTradeRouteStatus()
		: bIsActive(false)
		, CurrentProgress(0.0f)
		, EstimatedCompletionTime(0.0f)
		, TotalProfit(0.0f)
		, ShipsAssigned(0)
	{}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAutomatedTradeStarted, const FAutomatedShipData&, ShipData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAutomatedTradeCompleted, const FAutomatedShipData&, ShipData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTradeRouteCompleted, const FAutomatedTradeRoute&, Route);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFleetProfitUpdated, const FTradeFleetData&, Fleet);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShipAttacked, const FAutomatedShipData&, ShipData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShipDestroyed, const FAutomatedShipData&, ShipData);

/**
 * Trade Ship Automation - Handles AI-controlled trade ships, automated routes, and fleet management
 */
UCLASS()
class ALEXANDER_API UTradeShipAutomation : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UTradeShipAutomation();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tick function
	virtual void Tick(float DeltaTime);
	virtual bool IsTickable() const { return true; }

	// ============================================================================
	// SHIP REGISTRATION
	// ============================================================================

	// Register ship for automation
	UFUNCTION(BlueprintCallable, Category = "Automation|Ships")
	bool RegisterShip(ASpaceship* Ship, ETradeShipClass ShipClass, FString OwnerPlayerID);

	// Unregister ship
	UFUNCTION(BlueprintCallable, Category = "Automation|Ships")
	bool UnregisterShip(ASpaceship* Ship);

	// Get ship data
	UFUNCTION(BlueprintCallable, Category = "Automation|Ships")
	FAutomatedShipData GetShipData(ASpaceship* Ship) const;

	// Update ship class
	UFUNCTION(BlueprintCallable, Category = "Automation|Ships")
	void UpdateShipClass(ASpaceship* Ship, ETradeShipClass NewClass);

	// Get all automated ships
	UFUNCTION(BlueprintCallable, Category = "Automation|Ships")
	TArray<FAutomatedShipData> GetAllAutomatedShips() const;

	// Get player's ships
	UFUNCTION(BlueprintCallable, Category = "Automation|Ships")
	TArray<FAutomatedShipData> GetPlayerShips(FString PlayerID) const;

	// ============================================================================
	// TRADE ROUTE MANAGEMENT
	// ============================================================================

	// Create automated trade route
	UFUNCTION(BlueprintCallable, Category = "Automation|Routes")
	bool CreateAutomatedTradeRoute(
		ASpaceship* Ship,
		FString StartStationID,
		FString EndStationID,
		TArray<FName> Commodities
	);

	// Remove trade route
	UFUNCTION(BlueprintCallable, Category = "Automation|Routes")
	bool RemoveTradeRoute(FString RouteID);

	// Get trade route
	UFUNCTION(BlueprintCallable, Category = "Automation|Routes")
	FAutomatedTradeRoute GetTradeRoute(FString RouteID) const;

	// Get ship's current route
	UFUNCTION(BlueprintCallable, Category = "Automation|Routes")
	FAutomatedTradeRoute GetShipRoute(ASpaceship* Ship) const;

	// Update trade route
	UFUNCTION(BlueprintCallable, Category = "Automation|Routes")
	bool UpdateTradeRoute(FString RouteID, const FAutomatedTradeRoute& NewRoute);

	// Get all trade routes
	UFUNCTION(BlueprintCallable, Category = "Automation|Routes")
	TArray<FAutomatedTradeRoute> GetAllTradeRoutes() const;

	// Get profitable routes for ship
	UFUNCTION(BlueprintCallable, Category = "Automation|Routes")
	TArray<FAutomatedTradeRoute> GetProfitableRoutes(ASpaceship* Ship, int32 Count) const;

	// Check if route is suitable for ship
	bool IsRouteSuitableForShip(const FAutomatedTradeRoute& Route, ETradeShipClass ShipClass) const;

	// ============================================================================
	// AUTOMATION CONTROL
	// ============================================================================

	// Start automated trading
	UFUNCTION(BlueprintCallable, Category = "Automation|Control")
	bool StartAutomatedTrading(ASpaceship* Ship);

	// Stop automated trading
	UFUNCTION(BlueprintCallable, Category = "Automation|Control")
	bool StopAutomatedTrading(ASpaceship* Ship);

	// Pause automation
	UFUNCTION(BlueprintCallable, Category = "Automation|Control")
	bool PauseAutomation(ASpaceship* Ship);

	// Resume automation
	UFUNCTION(BlueprintCallable, Category = "Automation|Control")
	bool ResumeAutomation(ASpaceship* Ship);

	// Is ship automated?
	UFUNCTION(BlueprintCallable, Category = "Automation|Control")
	bool IsShipAutomated(ASpaceship* Ship) const;

	// Get automation status
	UFUNCTION(BlueprintCallable, Category = "Automation|Control")
	EAutomationStatus GetAutomationStatus(ASpaceship* Ship) const;

	// ============================================================================
	// FLEET MANAGEMENT
	// ============================================================================

	// Create fleet
	UFUNCTION(BlueprintCallable, Category = "Automation|Fleet")
	FString CreateFleet(FString FleetName, FString OwnerPlayerID);

	// Disband fleet
	UFUNCTION(BlueprintCallable, Category = "Automation|Fleet")
	bool DisbandFleet(FString FleetID);

	// Add ship to fleet
	UFUNCTION(BlueprintCallable, Category = "Automation|Fleet")
	bool AddShipToFleet(FString ShipID, FString FleetID);

	// Remove ship from fleet
	UFUNCTION(BlueprintCallable, Category = "Automation|Fleet")
	bool RemoveShipFromFleet(FString ShipID, FString FleetID);

	// Get fleet data
	UFUNCTION(BlueprintCallable, Category = "Automation|Fleet")
	FTradeFleetData GetFleetData(FString FleetID) const;

	// Get player's fleets
	UFUNCTION(BlueprintCallable, Category = "Automation|Fleet")
	TArray<FTradeFleetData> GetPlayerFleets(FString PlayerID) const;

	// Get fleet ships
	UFUNCTION(BlueprintCallable, Category = "Automation|Fleet")
	TArray<FAutomatedShipData> GetFleetShips(FString FleetID) const;

	// Update fleet settings
	UFUNCTION(BlueprintCallable, Category = "Automation|Fleet")
	bool UpdateFleetSettings(FString FleetID, const FTradeAutomationSettings& Settings);

	// Calculate fleet efficiency
	float CalculateFleetEfficiency(FString FleetID) const;

	// ============================================================================
	// TRADE EXECUTION
	// ============================================================================

	// Process automated trade
	UFUNCTION(BlueprintCallable, Category = "Automation|Trade")
	void ProcessAutomatedTrade(ASpaceship* Ship, float DeltaTime);

	// Find best trade opportunity
	UFUNCTION(BlueprintCallable, Category = "Automation|Trade")
	bool FindBestTradeOpportunity(ASpaceship* Ship, FAutomatedTradeRoute& OutRoute) const;

	// Execute trade
	UFUNCTION(BlueprintCallable, Category = "Automation|Trade")
	bool ExecuteTrade(ASpaceship* Ship, const FAutomatedTradeRoute& Route);

	// Calculate route profitability
	UFUNCTION(BlueprintCallable, Category = "Automation|Trade")
	float CalculateRouteProfitability(const FAutomatedTradeRoute& Route) const;

	// Update trade statistics
	UFUNCTION(BlueprintCallable, Category = "Automation|Trade")
	void UpdateTradeStatistics(ASpaceship* Ship, float Profit, bool bSuccess);

	// ============================================================================
	// SHIP BEHAVIOR
	// ============================================================================

	// Update ship behavior
	UFUNCTION(BlueprintCallable, Category = "Automation|Behavior")
	void UpdateShipBehavior(ASpaceship* Ship, float DeltaTime);

	// Handle ship attack
	UFUNCTION(BlueprintCallable, Category = "Automation|Behavior")
	void HandleShipAttack(ASpaceship* Ship, float Damage);

	// Handle ship destruction
	UFUNCTION(BlueprintCallable, Category = "Automation|Behavior")
	void HandleShipDestruction(ASpaceship* Ship);

	// Flee from danger
	UFUNCTION(BlueprintCallable, Category = "Automation|Behavior")
	void FleeFromDanger(ASpaceship* Ship);

	// Request assistance
	UFUNCTION(BlueprintCallable, Category = "Automation|Behavior")
	void RequestAssistance(ASpaceship* Ship);

	// ============================================================================
	// PROFIT ANALYSIS
	// ============================================================================

	// Get ship profit
	UFUNCTION(BlueprintCallable, Category = "Automation|Profit")
	float GetShipProfit(ASpaceship* Ship) const;

	// Get fleet profit
	UFUNCTION(BlueprintCallable, Category = "Automation|Profit")
	float GetFleetProfit(FString FleetID) const;

	// Get player total profit
	UFUNCTION(BlueprintCallable, Category = "Automation|Profit")
	float GetPlayerTotalProfit(FString PlayerID) const;

	// Get profit by route
	UFUNCTION(BlueprintCallable, Category = "Automation|Profit")
	TMap<FString, float> GetProfitByRoute(FString PlayerID) const;

	// Get profit by commodity
	UFUNCTION(BlueprintCallable, Category = "Automation|Profit")
	TMap<FName, float> GetProfitByCommodity(FString PlayerID) const;

	// Calculate ROI
	UFUNCTION(BlueprintCallable, Category = "Automation|Profit")
	float CalculateROI(FString PlayerID) const;

	// ============================================================================
	// STATISTICS & REPORTING
	// ============================================================================

	// Get automation statistics
	UFUNCTION(BlueprintCallable, Category = "Automation|Statistics")
	TMap<FString, int32> GetAutomationStatistics() const;

	// Get ship statistics
	UFUNCTION(BlueprintCallable, Category = "Automation|Statistics")
	FString GetShipStatistics(ASpaceship* Ship) const;

	// Get fleet report
	UFUNCTION(BlueprintCallable, Category = "Automation|Statistics")
	FString GetFleetReport(FString FleetID) const;

	// Get player automation report
	UFUNCTION(BlueprintCallable, Category = "Automation|Statistics")
	FString GetPlayerAutomationReport(FString PlayerID) const;

	// Export trade data
	UFUNCTION(BlueprintCallable, Category = "Automation|Statistics")
	FString ExportTradeData(FString PlayerID) const;

	// ============================================================================
	// SETTINGS & CONFIGURATION
	// ============================================================================

	// Get automation settings
	UFUNCTION(BlueprintCallable, Category = "Automation|Settings")
	FTradeAutomationSettings GetAutomationSettings(ASpaceship* Ship) const;

	// Set automation settings
	UFUNCTION(BlueprintCallable, Category = "Automation|Settings")
	bool SetAutomationSettings(ASpaceship* Ship, const FTradeAutomationSettings& Settings);

	// Reset automation settings to default
	UFUNCTION(BlueprintCallable, Category = "Automation|Settings")
	bool ResetAutomationSettings(ASpaceship* Ship);

	// ============================================================================
	// EVENTS
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Automation Events")
	FOnAutomatedTradeStarted OnAutomatedTradeStarted;

	UPROPERTY(BlueprintAssignable, Category = "Automation Events")
	FOnAutomatedTradeCompleted OnAutomatedTradeCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Automation Events")
	FOnTradeRouteCompleted OnTradeRouteCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Automation Events")
	FOnFleetProfitUpdated OnFleetProfitUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Automation Events")
	FOnShipAttacked OnShipAttacked;

	UPROPERTY(BlueprintAssignable, Category = "Automation Events")
	FOnShipDestroyed OnShipDestroyed;

	// ============================================================================
	// STATUS & PROFIT QUERIES
	// ============================================================================

	// Get automated trade route status
	UFUNCTION(BlueprintCallable, Category = "Automation|Status")
	FAutomatedTradeRouteStatus GetAutomatedTradeRouteStatus(FString RouteID) const;

	// Get fleet trading profit
	UFUNCTION(BlueprintCallable, Category = "Automation|Profit")
	float GetFleetTradingProfit(FString FleetID) const;

protected:
	// Trade mission system reference
	UPROPERTY()
	TWeakObjectPtr<UTradeMissionSystem> TradeMissionSystem;

	// Automated ships: Ship -> ShipData
	UPROPERTY()
	TMap<ASpaceship*, FAutomatedShipData> AutomatedShips;

	// Trade routes: RouteID -> RouteData
	UPROPERTY()
	TMap<FString, FAutomatedTradeRoute> TradeRoutes;

	// Fleets: FleetID -> FleetData
	UPROPERTY()
	TMap<FString, FTradeFleetData> Fleets;

	// Ship settings: Ship -> Settings
	UPROPERTY()
	TMap<ASpaceship*, FTradeAutomationSettings> ShipSettings;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float AutomationUpdateInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float RouteDiscoveryRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MaxDangerThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float ProfitabilityThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float FuelWarningThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bEnableAutomaticFlee;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bEnableFleetCoordination;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bShowDebugMessages;

private:
	// Update timer
	float AutomationTimer;

	// Statistics
	int32 TotalTradesExecuted;
	int32 TotalRoutesCompleted;
	float TotalProfitGenerated;
	int32 ShipsLost;

	// Helper functions
	void UpdateAutomation(float DeltaTime);
	void ProcessShipAutomation(ASpaceship* Ship, float DeltaTime);
	void ProcessFleetAutomation(FTradeFleetData& Fleet, float DeltaTime);
	void FindAndExecuteTrade(ASpaceship* Ship);
	void TravelToStation(ASpaceship* Ship, FString StationID);
	void LoadCargo(ASpaceship* Ship, FString StationID);
	void UnloadCargo(ASpaceship* Ship, FString StationID);
	bool ShouldFlee(ASpaceship* Ship) const;
	void RepairShip(ASpaceship* Ship);
	void RefuelShip(ASpaceship* Ship);
	FString GenerateRouteID() const;
	FString GenerateFleetID() const;
	void LogAutomationEvent(const FString& ShipID, const FString& Event, const FString& Details) const;
	float CalculateDangerLevel(const FAutomatedTradeRoute& Route) const;
	ERouteEfficiency CalculateRouteEfficiency(const FAutomatedTradeRoute& Route) const;
	void UpdateFleetStatistics(FString FleetID);
	void HandleShipCombat(ASpaceship* Ship, float DeltaTime);
	void ProcessShipDamage(ASpaceship* Ship, float Damage);
};