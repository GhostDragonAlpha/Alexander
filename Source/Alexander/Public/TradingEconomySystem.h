// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
#include "EconomySystem.h"
#include "FactionTerritorySystem.h"
#include "ResourceGatheringSystem.h"
#include "CraftingSystem.h"
#include "SystemSelfTestInterface.h"
#include "Containers/Map.h"
#include "TradingEconomySystem.generated.h"

// Forward declarations
class UDynamicMarketManager;
class UFactionEconomyManager;
class UTradeMissionSystem;
class UEconomicEventManager;
class UTradeShipAutomation;
class ATradeStation;
class ASpaceship;

/**
 * Commodity categories for detailed market segmentation
 */
UENUM(BlueprintType)
enum class ECommodityCategory : uint8
{
	// Raw Materials
	MetallicOre		UMETA(DisplayName = "Metallic Ore"),
	Crystalline		UMETA(DisplayName = "Crystalline"),
	Organic			UMETA(DisplayName = "Organic"),
	Radioactive		UMETA(DisplayName = "Radioactive"),
	Gaseous			UMETA(DisplayName = "Gaseous"),
	
	// Processed Materials
	Ingots			UMETA(DisplayName = "Ingots"),
	Components		UMETA(DisplayName = "Components"),
	Electronics		UMETA(DisplayName = "Electronics"),
	Chemicals		UMETA(DisplayName = "Chemicals"),
	
	// Food & Medical
	Foodstuffs		UMETA(DisplayName = "Foodstuffs"),
	MedicalSupplies	UMETA(DisplayName = "Medical Supplies"),
	Pharmaceuticals	UMETA(DisplayName = "Pharmaceuticals"),
	
	// Ship Components
	Engines			UMETA(DisplayName = "Engines"),
	Shields			UMETA(DisplayName = "Shields"),
	Weapons			UMETA(DisplayName = "Weapons"),
	Thrusters		UMETA(DisplayName = "Thrusters"),
	PowerSystems	UMETA(DisplayName = "Power Systems"),
	
	// Consumer Goods
	LuxuryItems		UMETA(DisplayName = "Luxury Items"),
	ConsumerGoods	UMETA(DisplayName = "Consumer Goods"),
	Textiles		UMETA(DisplayName = "Textiles"),
	
	// Illegal Goods
	SpaceWeed		UMETA(DisplayName = "Space Weed"),
	Contraband		UMETA(DisplayName = "Contraband"),
	StolenTech		UMETA(DisplayName = "Stolen Tech"),
	Slaves			UMETA(DisplayName = "Slaves"),
	
	// Exotic & Rare
	ExoticMatter	UMETA(DisplayName = "Exotic Matter"),
	AncientTech		UMETA(DisplayName = "Ancient Technology"),
	DarkMatter		UMETA(DisplayName = "Dark Matter"),
	Antimatter		UMETA(DisplayName = "Antimatter")
};

/**
 * Market volatility levels
 */
UENUM(BlueprintType)
enum class EMarketVolatility : uint8
{
	Stable		UMETA(DisplayName = "Stable"),
	Moderate	UMETA(DisplayName = "Moderate"),
	Volatile	UMETA(DisplayName = "Volatile"),
	Extreme		UMETA(DisplayName = "Extreme")
};

/**
 * Trade route risk levels
 */
UENUM(BlueprintType)
enum class ETradeRiskLevel : uint8
{
	Safe		UMETA(DisplayName = "Safe"),
	Low			UMETA(DisplayName = "Low Risk"),
	Moderate	UMETA(DisplayName = "Moderate Risk"),
	High		UMETA(DisplayName = "High Risk"),
	Extreme		UMETA(DisplayName = "Extreme Risk")
};

/**
 * Enhanced commodity data with detailed market properties
 */
USTRUCT(BlueprintType)
struct FCommodityDefinition : public FTableRowBase
{
	GENERATED_BODY()

	// Commodity ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	FName CommodityID;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	FString DisplayName;

	// Commodity category
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	ECommodityCategory Category;

	// Base value in credits
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	float BaseValue;

	// Weight per unit (kg)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	float WeightPerUnit;

	// Volume per unit (cubic meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	float VolumePerUnit;

	// Market volatility
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	EMarketVolatility Volatility;

	// Is this commodity illegal?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	bool bIsIllegal;

	// Is this commodity perishable?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	bool bIsPerishable;

	// Decay rate (per hour)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	float DecayRate;

	// Production source (for price calculations)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	FString PrimarySource;

	// Consumption destinations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	TArray<FString> ConsumptionDestinations;

	// Icon texture
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity|UI")
	UTexture2D* Icon;

	// Description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	FString Description;

	FCommodityDefinition()
	{
		BaseValue = 100.0f;
		WeightPerUnit = 1.0f;
		VolumePerUnit = 0.01f;
		Volatility = EMarketVolatility::Stable;
		bIsIllegal = false;
		bIsPerishable = false;
		DecayRate = 0.0f;
	}
};

/**
 * Market price history entry
 */
USTRUCT(BlueprintType)
struct FPriceHistoryEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	float Price;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	float Demand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	float Supply;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	int32 TradeVolume;

	FPriceHistoryEntry()
	{
		Timestamp = FDateTime::Now();
		Price = 0.0f;
		Demand = 1.0f;
		Supply = 1.0f;
		TradeVolume = 0;
	}
};

/**
 * Enhanced market listing with price history
 */
USTRUCT(BlueprintType)
struct FEnhancedMarketListing
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	FCommodityDefinition Commodity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	int32 AvailableQuantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	float CurrentBuyPrice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	float CurrentSellPrice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	float Demand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	float Supply;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	float BasePriceMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	TArray<FPriceHistoryEntry> PriceHistory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	FDateTime LastUpdated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	FString StationID;

	FEnhancedMarketListing()
	{
		AvailableQuantity = 0;
		CurrentBuyPrice = 0.0f;
		CurrentSellPrice = 0.0f;
		Demand = 1.0f;
		Supply = 1.0f;
		BasePriceMultiplier = 1.0f;
		LastUpdated = FDateTime::Now();
	}
};

// Wrapper struct for enhanced market listings map
USTRUCT(BlueprintType)
struct FEnhancedMarketListingMap
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FName, FEnhancedMarketListing> Listings;

	FEnhancedMarketListingMap() {}
};

/**
 * Trade route with detailed analysis
 */
USTRUCT(BlueprintType)
struct FDetailedTradeRoute
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FName RouteID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FString StartStationID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FString EndStationID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FVector StartLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FVector EndLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float Distance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float TravelTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float FuelCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	ETradeRiskLevel RiskLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float EstimatedProfit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	TArray<FCommodityDefinition> RecommendedCommodities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	TMap<FName, float> ProfitPerCommodity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FDateTime LastUpdated;

	FDetailedTradeRoute()
	{
		Distance = 0.0f;
		TravelTime = 0.0f;
		FuelCost = 0.0f;
		RiskLevel = ETradeRiskLevel::Safe;
		EstimatedProfit = 0.0f;
		LastUpdated = FDateTime::Now();
	}
};

/**
 * Market data update wrapper for delegate parameters
 */
USTRUCT(BlueprintType)
struct FMarketDataUpdate
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FName, FEnhancedMarketListing> MarketData;

	FMarketDataUpdate() {}
};

/**
 * Enhanced transaction record
 */
USTRUCT(BlueprintType)
struct FEnhancedTransactionRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transaction")
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transaction")
	FName CommodityID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transaction")
	int32 Quantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transaction")
	float PricePerUnit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transaction")
	float TotalValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transaction")
	bool bWasPurchase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transaction")
	FString StationID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transaction")
	FString FactionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transaction")
	float Profit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transaction")
	float ReputationImpact;

	FEnhancedTransactionRecord()
	{
		Timestamp = FDateTime::Now();
		Quantity = 0;
		PricePerUnit = 0.0f;
		TotalValue = 0.0f;
		bWasPurchase = true;
		Profit = 0.0f;
		ReputationImpact = 0.0f;
	}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMarketPricesUpdated, const FMarketDataUpdate&, MarketDataUpdate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTradeRouteDiscovered, const FDetailedTradeRoute&, Route);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEconomicEventTriggered, const FString&, EventName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerMarketTransaction, const FEnhancedTransactionRecord&, Transaction, float, NewBalance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReputationChanged, const FString&, FactionID, float, NewReputation);

/**
 * Trading Economy System - Comprehensive system for dynamic markets, trade routes, and economic simulation
 */
UCLASS()
class ALEXANDER_API UTradingEconomySystem : public UWorldSubsystem, public ISystemSelfTestInterface
{
	GENERATED_BODY()

public:
	UTradingEconomySystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tick function
	virtual void Tick(float DeltaTime);
	virtual bool IsTickable() const { return true; }

	//~ Begin ISystemSelfTestInterface interface
	virtual bool RunSelfTest_Implementation(FSystemTestResult& OutResult) override;
	virtual FString GetSystemName_Implementation() const override;
	virtual FString GetTestDescription_Implementation() const override;
	virtual bool IsReadyForTesting_Implementation() const override;
	//~ End ISystemSelfTestInterface interface

	// ============================================================================
	// MARKET MANAGEMENT
	// ============================================================================

	// Get current market listings
	UFUNCTION(BlueprintCallable, Category = "Trading|Market")
	TMap<FName, FEnhancedMarketListing> GetMarketData() const;

	// Get market listing for specific commodity
	UFUNCTION(BlueprintCallable, Category = "Trading|Market")
	FEnhancedMarketListing GetCommodityMarketData(FName CommodityID) const;

	// Get commodities by category
	UFUNCTION(BlueprintCallable, Category = "Trading|Market")
	TArray<FCommodityDefinition> GetCommoditiesByCategory(ECommodityCategory Category) const;

	// Update market prices (called automatically)
	UFUNCTION(BlueprintCallable, Category = "Trading|Market")
	void UpdateMarketPrices(float DeltaTime);

	// Force market update
	UFUNCTION(BlueprintCallable, Category = "Trading|Market")
	void ForceMarketUpdate();

	// ============================================================================
	// TRADING OPERATIONS
	// ============================================================================

	// Buy commodity from station
	UFUNCTION(BlueprintCallable, Category = "Trading|Operations")
	bool BuyCommodity(ASpaceship* Ship, FName CommodityID, int32 Quantity, ATradeStation* Station);

	// Sell commodity to station
	UFUNCTION(BlueprintCallable, Category = "Trading|Operations")
	bool SellCommodity(ASpaceship* Ship, FName CommodityID, int32 Quantity, ATradeStation* Station);

	// Get commodity value in cargo
	UFUNCTION(BlueprintCallable, Category = "Trading|Operations")
	int32 GetCommodityQuantityInCargo(ASpaceship* Ship, FName CommodityID) const;

	// Calculate transaction cost (including fees)
	UFUNCTION(BlueprintCallable, Category = "Trading|Operations")
	float CalculateTransactionCost(FName CommodityID, int32 Quantity, float PricePerUnit, FString FactionID) const;

	// ============================================================================
	// TRADE ROUTES
	// ============================================================================

	// Find profitable trade routes
	UFUNCTION(BlueprintCallable, Category = "Trading|Routes")
	TArray<FDetailedTradeRoute> FindProfitableTradeRoutes(ASpaceship* Ship, float MaxDistance, float MinProfit) const;

	// Calculate specific trade route
	UFUNCTION(BlueprintCallable, Category = "Trading|Routes")
	FDetailedTradeRoute CalculateTradeRoute(FString StartStationID, FString EndStationID) const;

	// Get all known trade routes
	UFUNCTION(BlueprintCallable, Category = "Trading|Routes")
	TArray<FDetailedTradeRoute> GetAllTradeRoutes() const;

	// Discover new trade route
	UFUNCTION(BlueprintCallable, Category = "Trading|Routes")
	bool DiscoverTradeRoute(FString StartStationID, FString EndStationID);

	// ============================================================================
	// FACTION ECONOMICS
	// ============================================================================

	// Get faction reputation impact on prices
	UFUNCTION(BlueprintCallable, Category = "Trading|Faction")
	float GetFactionPriceModifier(FString FactionID) const;

	// Modify faction reputation
	UFUNCTION(BlueprintCallable, Category = "Trading|Faction")
	void ModifyFactionReputation(FString FactionID, float ReputationChange);

	// Get current faction reputation
	UFUNCTION(BlueprintCallable, Category = "Trading|Faction")
	float GetFactionReputation(FString FactionID) const;

	// Can trade with faction?
	UFUNCTION(BlueprintCallable, Category = "Trading|Faction")
	bool CanTradeWithFaction(FString FactionID) const;

	// ============================================================================
	// ECONOMIC EVENTS
	// ============================================================================

	// Trigger economic event
	UFUNCTION(BlueprintCallable, Category = "Trading|Events")
	void TriggerEconomicEvent(FString EventName, float Severity);

	// Simulate market crash
	UFUNCTION(BlueprintCallable, Category = "Trading|Events")
	void SimulateMarketCrash(ECommodityCategory Category, float Severity);

	// Simulate market boom
	UFUNCTION(BlueprintCallable, Category = "Trading|Events")
	void SimulateMarketBoom(ECommodityCategory Category, float Magnitude);

	// Simulate supply shortage
	UFUNCTION(BlueprintCallable, Category = "Trading|Events")
	void SimulateSupplyShortage(FName CommodityID, float Severity, int32 DurationHours);

	// Simulate trade war
	UFUNCTION(BlueprintCallable, Category = "Trading|Events")
	void SimulateTradeWar(FString Faction1ID, FString Faction2ID, float Severity);

	// ============================================================================
	// BLACK MARKET
	// ============================================================================

	// Check if station has black market
	UFUNCTION(BlueprintCallable, Category = "Trading|BlackMarket")
	bool HasBlackMarket(ATradeStation* Station) const;

	// Get black market prices (higher risk, higher reward)
	UFUNCTION(BlueprintCallable, Category = "Trading|BlackMarket")
	FEnhancedMarketListing GetBlackMarketListing(ATradeStation* Station, FName CommodityID) const;

	// Buy from black market
	UFUNCTION(BlueprintCallable, Category = "Trading|BlackMarket")
	bool BuyFromBlackMarket(ASpaceship* Ship, FName CommodityID, int32 Quantity, ATradeStation* Station);

	// Get smuggling risk
	UFUNCTION(BlueprintCallable, Category = "Trading|BlackMarket")
	float GetSmugglingRisk(FName CommodityID, FString FactionID) const;

	// ============================================================================
	// PLAYER-OWNED MARKETS
	// ============================================================================

	// Create player market stall
	UFUNCTION(BlueprintCallable, Category = "Trading|PlayerMarkets")
	bool CreatePlayerMarketStall(FString StationID, FString PlayerID);

	// Set custom prices for player market
	UFUNCTION(BlueprintCallable, Category = "Trading|PlayerMarkets")
	bool SetPlayerMarketPrice(FString StationID, FName CommodityID, float CustomPrice);

	// Get player market listings
	UFUNCTION(BlueprintCallable, Category = "Trading|PlayerMarkets")
	TMap<FName, FEnhancedMarketListing> GetPlayerMarketListings(FString StationID) const;

	// Update player market (AI traders visit)
	UFUNCTION(BlueprintCallable, Category = "Trading|PlayerMarkets")
	void UpdatePlayerMarket(FString StationID, float DeltaTime);

	// ============================================================================
	// TRADE SHIP AUTOMATION
	// ============================================================================

	// Create automated trade route
	UFUNCTION(BlueprintCallable, Category = "Trading|Automation")
	bool CreateAutomatedTradeRoute(ASpaceship* Ship, FString StartStationID, FString EndStationID, TArray<FName> Commodities);

	// Get automated trade route status
	UFUNCTION(BlueprintCallable, Category = "Trading|Automation")
	FDetailedTradeRoute GetAutomatedTradeRouteStatus(ASpaceship* Ship) const;

	// Start automated trading
	UFUNCTION(BlueprintCallable, Category = "Trading|Automation")
	bool StartAutomatedTrading(ASpaceship* Ship);

	// Stop automated trading
	UFUNCTION(BlueprintCallable, Category = "Trading|Automation")
	bool StopAutomatedTrading(ASpaceship* Ship);

	// Get fleet trading profit
	UFUNCTION(BlueprintCallable, Category = "Trading|Automation")
	float GetFleetTradingProfit(FString PlayerID) const;

	// ============================================================================
	// STATISTICS & REPORTING
	// ============================================================================

	// Get transaction history
	UFUNCTION(BlueprintCallable, Category = "Trading|Statistics")
	TArray<FEnhancedTransactionRecord> GetTransactionHistory(int32 Count) const;

	// Get total profit
	UFUNCTION(BlueprintCallable, Category = "Trading|Statistics")
	float GetTotalProfit() const;

	// Get profit by faction
	UFUNCTION(BlueprintCallable, Category = "Trading|Statistics")
	TMap<FString, float> GetProfitByFaction() const;

	// Get best trades
	UFUNCTION(BlueprintCallable, Category = "Trading|Statistics")
	TArray<FEnhancedTransactionRecord> GetBestTrades(int32 Count) const;

	// Generate economy report
	UFUNCTION(BlueprintCallable, Category = "Trading|Statistics")
	FString GenerateEconomyReport() const;

	// ============================================================================
	// COMMODITY DEFINITIONS
	// ============================================================================

	// Initialize default commodities (50+ types)
	UFUNCTION(BlueprintCallable, Category = "Trading|Commodities")
	void InitializeDefaultCommodities();

	// Load commodities from data table
	UFUNCTION(BlueprintCallable, Category = "Trading|Commodities")
	void LoadCommoditiesFromDataTable(UDataTable* CommodityTable);

	// Register commodity definition
	UFUNCTION(BlueprintCallable, Category = "Trading|Commodities")
	void RegisterCommodity(const FCommodityDefinition& Commodity);

	// Get commodity definition
	UFUNCTION(BlueprintCallable, Category = "Trading|Commodities")
	FCommodityDefinition GetCommodityDefinition(FName CommodityID) const;

	// ============================================================================
	// EVENTS
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Trading Events")
	FOnMarketPricesUpdated OnMarketPricesUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Trading Events")
	FOnTradeRouteDiscovered OnTradeRouteDiscovered;

	UPROPERTY(BlueprintAssignable, Category = "Trading Events")
	FOnEconomicEventTriggered OnEconomicEventTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Trading Events")
	FOnPlayerMarketTransaction OnPlayerMarketTransaction;

	UPROPERTY(BlueprintAssignable, Category = "Trading Events")
	FOnReputationChanged OnReputationChanged;

protected:
	// Subsystem references
	UPROPERTY()
	TWeakObjectPtr<UDynamicMarketManager> DynamicMarketManager;

	UPROPERTY()
	TWeakObjectPtr<UFactionEconomyManager> FactionEconomyManager;

	UPROPERTY()
	TWeakObjectPtr<UTradeMissionSystem> TradeMissionSystem;

	UPROPERTY()
	TWeakObjectPtr<UEconomicEventManager> EconomicEventManager;

	UPROPERTY()
	TWeakObjectPtr<UTradeShipAutomation> TradeShipAutomation;

	UPROPERTY()
	TWeakObjectPtr<UEconomySystem> LegacyEconomySystem;

	UPROPERTY()
	TWeakObjectPtr<UFactionTerritorySystem> FactionTerritorySystem;

	UPROPERTY()
	TWeakObjectPtr<UResourceGatheringSystem> ResourceGatheringSystem;

	UPROPERTY()
	TWeakObjectPtr<UCraftingSystem> CraftingSystem;

	// Market data
	UPROPERTY()
	TMap<FName, FEnhancedMarketListing> GlobalMarketData;

	// Station and player market caches are internal and not exposed to reflection
	TMap<FString, FEnhancedMarketListingMap> StationMarkets; // StationID -> CommodityData

	TMap<FString, FEnhancedMarketListingMap> PlayerMarkets; // StationID -> CommodityData

	// Trade routes
	UPROPERTY()
	TArray<FDetailedTradeRoute> DiscoveredTradeRoutes;

	// Transaction history
	UPROPERTY()
	TArray<FEnhancedTransactionRecord> TransactionHistory;

	// Player credits
	UPROPERTY()
	TMap<FString, float> PlayerCredits; // PlayerID -> Credits

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MarketUpdateInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float TradeRouteDiscoveryDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MaxReputationDiscount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MaxReputationPenalty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UDataTable* CommodityDefinitionsTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugInfo;

private:
	// Market update timer
	float TimeSinceLastMarketUpdate;

	// Statistics
	float TotalProfitGenerated;
	int32 TotalTransactionsProcessed;
	int32 TotalTradeRoutesDiscovered;

	// Helper functions
	void InitializeSubsystems();
	void UpdateMarketSimulation(float DeltaTime);
	void DiscoverNearbyTradeRoutes();
	void ProcessAutomatedTradeShips(float DeltaTime);
	void SaveMarketData();
	void LoadMarketData();
};