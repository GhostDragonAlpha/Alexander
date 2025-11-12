// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
#include "TradingEconomySystem.h"
#include "SystemSelfTestInterface.h"
#include "DynamicMarketManager.generated.h"

// Forward declarations
class UFactionTerritorySystem;

/**
 * Market event types
 */
UENUM(BlueprintType)
enum class EMarketEventType : uint8
{
	None					UMETA(DisplayName = "None"),
	SupplyShortage			UMETA(DisplayName = "Supply Shortage"),
	SupplyGlut				UMETA(DisplayName = "Supply Glut"),
	TradeWar				UMETA(DisplayName = "Trade War"),
	PirateBlockade			UMETA(DisplayName = "Pirate Blockade"),
	TechBreakthrough		UMETA(DisplayName = "Tech Breakthrough"),
	NaturalDisaster			UMETA(DisplayName = "Natural Disaster"),
	Festival				UMETA(DisplayName = "Festival"),
	CivilUnrest				UMETA(DisplayName = "Civil Unrest"),
	WarDeclaration			UMETA(DisplayName = "War Declaration"),
	PeaceTreaty				UMETA(DisplayName = "Peace Treaty"),
	ResourceDiscovery		UMETA(DisplayName = "Resource Discovery"),
	MiningAccident			UMETA(DisplayName = "Mining Accident"),
	EconomicBoom			UMETA(DisplayName = "Economic Boom"),
	EconomicBust			UMETA(DisplayName = "Economic Bust")
};

/**
 * Active market event
 */
USTRUCT(BlueprintType)
struct FActiveMarketEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	EMarketEventType EventType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FName AffectedCommodity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	ECommodityCategory AffectedCategory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString AffectedStationID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString AffectedFactionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	float Severity; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	float PriceMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	float DemandMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	float SupplyMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	float DurationHours;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString EventDescription;

	FActiveMarketEvent()
	{
		EventType = EMarketEventType::None;
		Severity = 0.5f;
		PriceMultiplier = 1.0f;
		DemandMultiplier = 1.0f;
		SupplyMultiplier = 1.0f;
		DurationHours = 24.0f;
	}
};

/**
 * Market depth impact
 */
USTRUCT(BlueprintType)
struct FMarketDepthImpact
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MarketDepth")
	FName CommodityID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MarketDepth")
	int32 OrderSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MarketDepth")
	float PriceImpact; // Percentage change

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MarketDepth")
	float Slippage; // Actual vs expected price

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MarketDepth")
	FTimespan TimeToRecover;

	FMarketDepthImpact()
	{
		OrderSize = 0;
		PriceImpact = 0.0f;
		Slippage = 0.0f;
	}
};

/**
 * Supply/demand factors
 */
USTRUCT(BlueprintType)
struct FSupplyDemandFactors
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factors")
	float BaseDemand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factors")
	float BaseSupply;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factors")
	float DistanceFactor; // Distance from production source

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factors")
	float FactionControlFactor; // Faction stability and control

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factors")
	float SeasonalFactor; // Seasonal variations

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factors")
	float EventFactor; // Current events

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factors")
	float PlayerImpactFactor; // Player actions

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factors")
	float RandomFactor; // Random fluctuations

	FSupplyDemandFactors()
	{
		BaseDemand = 1.0f;
		BaseSupply = 1.0f;
		DistanceFactor = 1.0f;
		FactionControlFactor = 1.0f;
		SeasonalFactor = 1.0f;
		EventFactor = 1.0f;
		PlayerImpactFactor = 1.0f;
		RandomFactor = 1.0f;
	}

	float GetTotalDemand() const
	{
		return BaseDemand * DistanceFactor * FactionControlFactor * 
			   SeasonalFactor * EventFactor * PlayerImpactFactor * RandomFactor;
	}

	float GetTotalSupply() const
	{
		return BaseSupply * FactionControlFactor * EventFactor * PlayerImpactFactor;
	}
};

// Wrapper structs for complex template containers (UHT compatibility)
USTRUCT(BlueprintType)
struct FPriceHistoryArray
{
	GENERATED_BODY()
	UPROPERTY()
	TArray<FPriceHistoryEntry> Entries;
};

USTRUCT(BlueprintType)
struct FMarketDepthImpactArray
{
	GENERATED_BODY()
	UPROPERTY()
	TArray<FMarketDepthImpact> Impacts;
};

USTRUCT(BlueprintType)
struct FPriceHistoryMap
{
	GENERATED_BODY()
	UPROPERTY()
	TMap<FName, FPriceHistoryArray> CommodityHistory;
};

USTRUCT(BlueprintType)
struct FMarketDepthMap
{
	GENERATED_BODY()
	UPROPERTY()
	TMap<FName, FMarketDepthImpactArray> CommodityDepth;
};

USTRUCT(BlueprintType)
struct FFloatValueMap
{
	GENERATED_BODY()
	UPROPERTY()
	TMap<FName, float> Values;
};

USTRUCT(BlueprintType)
struct FStringFloatMap
{
	GENERATED_BODY()
	UPROPERTY()
	TMap<FString, float> Values;
};

USTRUCT(BlueprintType)
struct FMarketInt32ValueMap
{
	GENERATED_BODY()
	UPROPERTY()
	TMap<FName, int32> Values;
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMarketEventStarted, const FActiveMarketEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMarketEventEnded, const FActiveMarketEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPriceImpact, FName, CommodityID, const FMarketDepthImpact&, Impact);

/**
 * Dynamic Market Manager - Handles sophisticated price calculations, market events, and depth simulation
 */
UCLASS()
class ALEXANDER_API UDynamicMarketManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UDynamicMarketManager();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tick function
	virtual void Tick(float DeltaTime);
	virtual bool IsTickable() const { return true; }

	// ============================================================================
	// PRICE CALCULATIONS
	// ============================================================================

	// Calculate dynamic price for commodity
	UFUNCTION(BlueprintCallable, Category = "Market|Prices")
	float CalculateDynamicPrice(
		const FCommodityDefinition& Commodity,
		const FSupplyDemandFactors& Factors,
		float BasePriceMultiplier = 1.0f
	) const;

	// Calculate supply/demand factors for station
	UFUNCTION(BlueprintCallable, Category = "Market|Prices")
	FSupplyDemandFactors CalculateSupplyDemandFactors(
		FName CommodityID,
		FString StationID,
		FString FactionID
	) const;

	// Calculate market depth impact
	UFUNCTION(BlueprintCallable, Category = "Market|Depth")
	FMarketDepthImpact CalculateMarketDepthImpact(
		FName CommodityID,
		int32 OrderSize,
		FString StationID
	) const;

	// Calculate price volatility
	UFUNCTION(BlueprintCallable, Category = "Market|Prices")
	float CalculatePriceVolatility(FName CommodityID, FString StationID) const;

	// ============================================================================
	// MARKET EVENTS
	// ============================================================================

	// Trigger market event
	UFUNCTION(BlueprintCallable, Category = "Market|Events")
	FActiveMarketEvent TriggerMarketEvent(
		EMarketEventType EventType,
		FName CommodityID,
		FString StationID,
		float Severity,
		float DurationHours
	);

	// Get active market events
	UFUNCTION(BlueprintCallable, Category = "Market|Events")
	TArray<FActiveMarketEvent> GetActiveMarketEvents() const;

	// End market event
	UFUNCTION(BlueprintCallable, Category = "Market|Events")
	void EndMarketEvent(const FActiveMarketEvent& Event);

	// Clear all market events
	UFUNCTION(BlueprintCallable, Category = "Market|Events")
	void ClearAllMarketEvents();

	// ============================================================================
	// MARKET DEPTH & LIQUIDITY
	// ============================================================================

	// Process large market order
	UFUNCTION(BlueprintCallable, Category = "Market|Depth")
	FMarketDepthImpact ProcessLargeOrder(
		FName CommodityID,
		int32 Quantity,
		bool bIsBuyOrder,
		FString StationID
	);

	// Get market liquidity
	UFUNCTION(BlueprintCallable, Category = "Market|Depth")
	float GetMarketLiquidity(FName CommodityID, FString StationID) const;

	// Get average daily volume
	UFUNCTION(BlueprintCallable, Category = "Market|Depth")
	int32 GetAverageDailyVolume(FName CommodityID, FString StationID) const;

	// ============================================================================
	// PRICE HISTORY & ANALYSIS
	// ============================================================================

	// Record price history
	UFUNCTION(BlueprintCallable, Category = "Market|History")
	void RecordPriceHistory(
		FName CommodityID,
		FString StationID,
		float Price,
		float Demand,
		float Supply,
		int32 TradeVolume
	);

	// Get price history
	UFUNCTION(BlueprintCallable, Category = "Market|History")
	TArray<FPriceHistoryEntry> GetPriceHistory(
		FName CommodityID,
		FString StationID,
		int32 HoursBack
	) const;

	// Calculate price trend
	UFUNCTION(BlueprintCallable, Category = "Market|Analysis")
	float CalculatePriceTrend(FName CommodityID, FString StationID, int32 HoursBack) const;

	// Get market analysis
	UFUNCTION(BlueprintCallable, Category = "Market|Analysis")
	FString GetMarketAnalysis(FName CommodityID, FString StationID) const;

	// ============================================================================
	// STATION MARKET MANAGEMENT
	// ============================================================================

	// Initialize station market
	UFUNCTION(BlueprintCallable, Category = "Market|Stations")
	void InitializeStationMarket(
		FString StationID,
		FString FactionID,
		TArray<FName> AvailableCommodities
	);

	// Update station market
	UFUNCTION(BlueprintCallable, Category = "Market|Stations")
	void UpdateStationMarket(
		FString StationID,
		float DeltaTime
	);

	// Get station market modifier
	UFUNCTION(BlueprintCallable, Category = "Market|Stations")
	float GetStationMarketModifier(FString StationID) const;

	// Set station market modifier
	UFUNCTION(BlueprintCallable, Category = "Market|Stations")
	void SetStationMarketModifier(FString StationID, float Modifier);

	// ============================================================================
	// FACTION INFLUENCE
	// ============================================================================

	// Calculate faction price modifier
	UFUNCTION(BlueprintCallable, Category = "Market|Faction")
	float CalculateFactionPriceModifier(
		FString FactionID,
		FString PlayerFactionID,
		FName CommodityID
	) const;

	// Update faction market control
	UFUNCTION(BlueprintCallable, Category = "Market|Faction")
	void UpdateFactionMarketControl(
		FString FactionID,
		FString StationID,
		float ControlLevel
	);

	// Get faction market control
	UFUNCTION(BlueprintCallable, Category = "Market|Faction")
	float GetFactionMarketControl(FString FactionID, FString StationID) const;

	// ============================================================================
	// PLAYER IMPACT
	// ============================================================================

	// Record player trade impact
	UFUNCTION(BlueprintCallable, Category = "Market|Player")
	void RecordPlayerTradeImpact(
		FString PlayerID,
		FName CommodityID,
		int32 Quantity,
		FString StationID
	);

	// Get player market influence
	UFUNCTION(BlueprintCallable, Category = "Market|Player")
	float GetPlayerMarketInfluence(FString PlayerID, FName CommodityID) const;

	// Reset player market impact
	UFUNCTION(BlueprintCallable, Category = "Market|Player")
	void ResetPlayerMarketImpact(FString PlayerID);

	// ============================================================================
	// RANDOM EVENTS
	// ============================================================================

	// Generate random market event
	UFUNCTION(BlueprintCallable, Category = "Market|Events")
	void GenerateRandomMarketEvent(FString StationID);

	// Generate random faction event
	UFUNCTION(BlueprintCallable, Category = "Market|Events")
	void GenerateRandomFactionEvent(FString FactionID);

	// Get random event chance
	UFUNCTION(BlueprintCallable, Category = "Market|Events")
	float GetRandomEventChance(FString StationID) const;

	// ============================================================================
	// STATISTICS & ANALYSIS
	// ============================================================================

	// Get market statistics
	UFUNCTION(BlueprintCallable, Category = "Market|Statistics")
	TMap<FName, float> GetMarketVolatilityByCommodity() const;

	// Get most traded commodities
	UFUNCTION(BlueprintCallable, Category = "Market|Statistics")
	TArray<FName> GetMostTradedCommodities(int32 Count) const;

	// Get market efficiency
	UFUNCTION(BlueprintCallable, Category = "Market|Statistics")
	float GetMarketEfficiency(FString StationID) const;

	// ============================================================================
	// EVENTS
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Market Events")
	FOnMarketEventStarted OnMarketEventStarted;

	UPROPERTY(BlueprintAssignable, Category = "Market Events")
	FOnMarketEventEnded OnMarketEventEnded;

	UPROPERTY(BlueprintAssignable, Category = "Market Events")
	FOnPriceImpact OnPriceImpact;

protected:
	// Faction territory system reference
	UPROPERTY()
	TWeakObjectPtr<UFactionTerritorySystem> FactionTerritorySystem;

	// Market events
	UPROPERTY()
	TArray<FActiveMarketEvent> ActiveEvents;

	// Price history database: StationID -> CommodityID -> HistoryEntries
	UPROPERTY()
	TMap<FString, FPriceHistoryMap> PriceHistoryDatabase;

	// Market depth tracking: StationID -> CommodityID -> RecentOrders
	UPROPERTY()
	TMap<FString, FMarketDepthMap> MarketDepthTracker;

	// Player trade impact: PlayerID -> CommodityID -> ImpactScore
	UPROPERTY()
	TMap<FString, FFloatValueMap> PlayerTradeImpact;

	// Station market modifiers: StationID -> Modifier
	UPROPERTY()
	TMap<FString, float> StationMarketModifiers;

	// Faction market control: FactionID -> StationID -> ControlLevel
	UPROPERTY()
	TMap<FString, FStringFloatMap> FactionMarketControl;

	// Daily trade volumes: StationID -> CommodityID -> Volume
	UPROPERTY()
	TMap<FString, FMarketInt32ValueMap> DailyTradeVolumes;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float EventCheckInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MaxEventSeverity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float PlayerImpactDecayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 MaxPriceHistoryEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MarketDepthRecoveryTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bEnableRandomEvents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugMessages;

private:
	// Event timer
	float TimeSinceLastEventCheck;

	// Statistics
	int32 TotalEventsTriggered;
	int32 TotalPriceImpactsProcessed;

	// Helper functions
	void UpdateActiveEvents(float DeltaTime);
	void ApplyEventToMarket(const FActiveMarketEvent& Event);
	void RemoveEventFromMarket(const FActiveMarketEvent& Event);
	float CalculateDistanceFactor(FName CommodityID, FString StationID) const;
	float CalculateSeasonalFactor() const;
	float CalculateRandomFactor(FName CommodityID) const;
	void CleanupOldPriceHistory();
	void UpdateDailyVolumes(float DeltaTime);
	EMarketEventType SelectRandomEventType(FString StationID) const;
	FString GenerateEventDescription(EMarketEventType EventType, FName CommodityID) const;
	void LogMarketEvent(const FActiveMarketEvent& Event, const FString& Action) const;
};