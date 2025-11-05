// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "EconomySystem.generated.h"

// Forward declarations
class UTradeStation;

/**
 * Commodity types available for trading
 */
UENUM(BlueprintType)
enum class ECommodityType : uint8
{
RawMaterials UMETA(DisplayName = "Raw Materials"),
Metals UMETA(DisplayName = "Metals"),
Fuel UMETA(DisplayName = "Fuel"),
Food UMETA(DisplayName = "Food"),
Medicine UMETA(DisplayName = "Medicine"),
Technology UMETA(DisplayName = "Technology"),
Weapons UMETA(DisplayName = "Weapons"),
Luxury UMETA(DisplayName = "Luxury Goods"),
Illegal UMETA(DisplayName = "Illegal Goods"),
Waste UMETA(DisplayName = "Waste Products")
};

/**
 * Economic faction types
 */
UENUM(BlueprintType)
enum class EEconomicFaction : uint8
{
Industrial UMETA(DisplayName = "Industrial"),
Agricultural UMETA(DisplayName = "Agricultural"),
Mining UMETA(DisplayName = "Mining"),
Research UMETA(DisplayName = "Research"),
Military UMETA(DisplayName = "Military"),
Pirate UMETA(DisplayName = "Pirate"),
Trading UMETA(DisplayName = "Trading Guild"),
Independent UMETA(DisplayName = "Independent")
};

/**
 * Trade route status
 */
UENUM(BlueprintType)
enum class ETradeRouteStatus : uint8
{
Active UMETA(DisplayName = "Active"),
Disrupted UMETA(DisplayName = "Disrupted"),
Dangerous UMETA(DisplayName = "Dangerous"),
Closed UMETA(DisplayName = "Closed"),
Emerging UMETA(DisplayName = "Emerging")
};

/**
 * Commodity data definition
 */
USTRUCT(BlueprintType)
struct FCommodityData : public FTableRowBase
{
GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
FName CommodityName = NAME_None;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
ECommodityType CommodityType = ECommodityType::RawMaterials;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
float BasePrice = 100.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
float PriceVolatility = 0.2f; // 0-1

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
float Volume = 1.0f; // Cubic meters per unit

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
float Mass = 1.0f; // Kilograms per unit

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
bool bIsIllegal = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
bool bIsPerishable = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
float DecayRate = 0.0f; // Per hour

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
FString Description = TEXT("");
};

/**
 * Market listing for a commodity
 */
USTRUCT(BlueprintType)
struct FMarketListing
{
GENERATED_BODY()

UPROPERTY(BlueprintReadWrite, Category = "Market")
FCommodityData Commodity;

UPROPERTY(BlueprintReadWrite, Category = "Market")
int32 AvailableQuantity = 0;

UPROPERTY(BlueprintReadWrite, Category = "Market")
float CurrentBuyPrice = 0.0f;

UPROPERTY(BlueprintReadWrite, Category = "Market")
float CurrentSellPrice = 0.0f;

UPROPERTY(BlueprintReadWrite, Category = "Market")
float Demand = 1.0f; // 0-2+ multiplier

UPROPERTY(BlueprintReadWrite, Category = "Market")
float Supply = 1.0f; // 0-2+ multiplier

UPROPERTY(BlueprintReadWrite, Category = "Market")
FDateTime LastUpdated;
};

/**
 * Trade route definition
 */
USTRUCT(BlueprintType)
struct FTradeRoute
{
GENERATED_BODY()

UPROPERTY(BlueprintReadWrite, Category = "Trade Route")
FName RouteName = NAME_None;

UPROPERTY(BlueprintReadWrite, Category = "Trade Route")
FVector StartLocation = FVector::ZeroVector;

UPROPERTY(BlueprintReadWrite, Category = "Trade Route")
FVector EndLocation = FVector::ZeroVector;

UPROPERTY(BlueprintReadWrite, Category = "Trade Route")
float Distance = 0.0f;

UPROPERTY(BlueprintReadWrite, Category = "Trade Route")
ETradeRouteStatus Status = ETradeRouteStatus::Active;

UPROPERTY(BlueprintReadWrite, Category = "Trade Route")
TArray<FCommodityData> ProfitableCommodities;

UPROPERTY(BlueprintReadWrite, Category = "Trade Route")
float DangerLevel = 0.0f; // 0-1

UPROPERTY(BlueprintReadWrite, Category = "Trade Route")
float EstimatedProfit = 0.0f;

UPROPERTY(BlueprintReadWrite, Category = "Trade Route")
float TravelTime = 0.0f; // Hours
};

/**
 * Cargo hold item
 */
USTRUCT(BlueprintType)
struct FCargoItem
{
GENERATED_BODY()

UPROPERTY(BlueprintReadWrite, Category = "Cargo")
FCommodityData Commodity;

UPROPERTY(BlueprintReadWrite, Category = "Cargo")
int32 Quantity = 0;

UPROPERTY(BlueprintReadWrite, Category = "Cargo")
float PurchasePrice = 0.0f;

UPROPERTY(BlueprintReadWrite, Category = "Cargo")
FDateTime PurchaseTime;

UPROPERTY(BlueprintReadWrite, Category = "Cargo")
FName PurchaseLocation = NAME_None;
};

/**
 * Transaction record
 */
USTRUCT(BlueprintType)
struct FTransactionRecord
{
GENERATED_BODY()

UPROPERTY(BlueprintReadWrite, Category = "Transaction")
FDateTime Timestamp;

UPROPERTY(BlueprintReadWrite, Category = "Transaction")
FName CommodityName = NAME_None;

UPROPERTY(BlueprintReadWrite, Category = "Transaction")
int32 Quantity = 0;

UPROPERTY(BlueprintReadWrite, Category = "Transaction")
float PricePerUnit = 0.0f;

UPROPERTY(BlueprintReadWrite, Category = "Transaction")
float TotalValue = 0.0f;

UPROPERTY(BlueprintReadWrite, Category = "Transaction")
bool bWasPurchase = true;

UPROPERTY(BlueprintReadWrite, Category = "Transaction")
FName Location = NAME_None;

UPROPERTY(BlueprintReadWrite, Category = "Transaction")
float Profit = 0.0f;
};

/**
 * Economy System Component - manages trading, cargo, and market simulation
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UEconomySystem : public UActorComponent
{
GENERATED_BODY()

public:
UEconomySystem();

protected:
virtual void BeginPlay() override;
virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
// ============================================================================
// PLAYER ECONOMY
// ============================================================================

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
float PlayerCredits = 10000.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
float MaxCargoCapacity = 100.0f; // Cubic meters

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
TArray<FCargoItem> CargoHold;

UFUNCTION(BlueprintCallable, Category = "Economy")
float GetCurrentCargoVolume() const;

UFUNCTION(BlueprintCallable, Category = "Economy")
float GetAvailableCargoSpace() const;

UFUNCTION(BlueprintCallable, Category = "Economy")
bool HasCargoSpace(const FCommodityData& Commodity, int32 Quantity) const;

// ============================================================================
// TRADING
// ============================================================================

UFUNCTION(BlueprintCallable, Category = "Trading")
bool BuyCommodity(const FName& CommodityName, int32 Quantity, float PricePerUnit);

UFUNCTION(BlueprintCallable, Category = "Trading")
bool SellCommodity(const FName& CommodityName, int32 Quantity, float PricePerUnit);

UFUNCTION(BlueprintCallable, Category = "Trading")
float CalculateTradeFees(float TransactionValue) const;

UFUNCTION(BlueprintCallable, Category = "Trading")
bool CanAffordTransaction(float TotalCost) const;

UFUNCTION(BlueprintCallable, Category = "Trading")
int32 GetCommodityQuantityInCargo(const FName& CommodityName) const;

UFUNCTION(BlueprintCallable, Category = "Trading")
float GetAveragePurchasePrice(const FName& CommodityName) const;

// ============================================================================
// MARKET SIMULATION
// ============================================================================

UFUNCTION(BlueprintCallable, Category = "Market")
void UpdateMarketPrices(float DeltaTime);

UFUNCTION(BlueprintCallable, Category = "Market")
FMarketListing GetMarketListing(const FName& CommodityName) const;

UFUNCTION(BlueprintCallable, Category = "Market")
TArray<FMarketListing> GetAllMarketListings() const;

UFUNCTION(BlueprintCallable, Category = "Market")
void SetMarketDemand(const FName& CommodityName, float NewDemand);

UFUNCTION(BlueprintCallable, Category = "Market")
void SetMarketSupply(const FName& CommodityName, float NewSupply);

UFUNCTION(BlueprintCallable, Category = "Market")
float CalculateDynamicPrice(const FCommodityData& Commodity, float Demand, float Supply) const;

// ============================================================================
// TRADE ROUTES
// ============================================================================

UFUNCTION(BlueprintCallable, Category = "Trade Routes")
TArray<FTradeRoute> FindProfitableRoutes(float MaxDistance, float MinProfit) const;

UFUNCTION(BlueprintCallable, Category = "Trade Routes")
FTradeRoute CalculateTradeRoute(const FVector& StartLocation, const FVector& EndLocation) const;

UFUNCTION(BlueprintCallable, Category = "Trade Routes")
void UpdateTradeRouteStatus(const FName& RouteName, ETradeRouteStatus NewStatus);

// ============================================================================
// ECONOMIC EVENTS
// ============================================================================

UFUNCTION(BlueprintCallable, Category = "Events")
void TriggerEconomicEvent(const FName& EventName);

UFUNCTION(BlueprintCallable, Category = "Events")
void SimulateMarketCrash(ECommodityType AffectedType, float Severity);

UFUNCTION(BlueprintCallable, Category = "Events")
void SimulateMarketBoom(ECommodityType AffectedType, float Magnitude);

UFUNCTION(BlueprintCallable, Category = "Events")
void SimulateSupplyShortage(const FName& CommodityName, float Severity);

// ============================================================================
// TRANSACTION HISTORY
// ============================================================================

UFUNCTION(BlueprintCallable, Category = "History")
void RecordTransaction(const FTransactionRecord& Transaction);

UFUNCTION(BlueprintCallable, Category = "History")
TArray<FTransactionRecord> GetTransactionHistory(int32 Count = 10) const;

UFUNCTION(BlueprintCallable, Category = "History")
float GetTotalProfit() const;

UFUNCTION(BlueprintCallable, Category = "History")
float GetAverageProfit() const;

UFUNCTION(BlueprintCallable, Category = "History")
FTransactionRecord GetBestTrade() const;

// ============================================================================
// CARGO MANAGEMENT
// ============================================================================

UFUNCTION(BlueprintCallable, Category = "Cargo")
bool AddToCargo(const FCommodityData& Commodity, int32 Quantity, float PurchasePrice);

UFUNCTION(BlueprintCallable, Category = "Cargo")
bool RemoveFromCargo(const FName& CommodityName, int32 Quantity);

UFUNCTION(BlueprintCallable, Category = "Cargo")
void DumpCargo(const FName& CommodityName);

UFUNCTION(BlueprintCallable, Category = "Cargo")
void DumpAllCargo();

UFUNCTION(BlueprintCallable, Category = "Cargo")
void UpdateCargoCondition(float DeltaTime);

UFUNCTION(BlueprintCallable, Category = "Cargo")
float GetCargoValue() const;

// ============================================================================
// FACTION ECONOMICS
// ============================================================================

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factions")
TMap<EEconomicFaction, float> FactionReputation;

UFUNCTION(BlueprintCallable, Category = "Factions")
float GetFactionDiscount(EEconomicFaction Faction) const;

UFUNCTION(BlueprintCallable, Category = "Factions")
void ModifyFactionReputation(EEconomicFaction Faction, float Delta);

UFUNCTION(BlueprintCallable, Category = "Factions")
bool CanTradeWithFaction(EEconomicFaction Faction) const;

// ============================================================================
// SMUGGLING & ILLEGAL GOODS
// ============================================================================

UFUNCTION(BlueprintCallable, Category = "Smuggling")
bool HasIllegalCargo() const;

UFUNCTION(BlueprintCallable, Category = "Smuggling")
float CalculateFineForIllegalCargo() const;

UFUNCTION(BlueprintCallable, Category = "Smuggling")
void ConfiscateIllegalCargo();

UFUNCTION(BlueprintCallable, Category = "Smuggling")
float GetSmugglingRisk(const FName& CommodityName) const;

// ============================================================================
// STATISTICS & REPORTING
// ============================================================================

UFUNCTION(BlueprintCallable, Category = "Statistics")
FString GetEconomyReport() const;

UFUNCTION(BlueprintCallable, Category = "Statistics")
FString GetCargoManifest() const;

UFUNCTION(BlueprintCallable, Category = "Statistics")
FString GetProfitReport() const;

// ============================================================================
// CONFIGURATION
// ============================================================================

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
float TradeFeePercentage = 0.05f; // 5%

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
float MarketUpdateInterval = 60.0f; // Seconds

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
float PriceFluctuationRate = 0.1f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
UDataTable* CommodityDataTable = nullptr;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
bool bShowDebug = false;

protected:
// Internal market state
UPROPERTY()
TMap<FName, FMarketListing> CurrentMarket;

UPROPERTY()
TArray<FTradeRoute> KnownTradeRoutes;

UPROPERTY()
TArray<FTransactionRecord> TransactionHistory;

// Market simulation
float TimeSinceLastUpdate = 0.0f;

// Statistics
int32 TotalTransactions = 0;
float TotalProfit = 0.0f;
float TotalRevenue = 0.0f;
float TotalExpenses = 0.0f;

private:
// Helper functions
void InitializeMarket();
void SimulateMarketDynamics(float DeltaTime);
void ApplyRandomFluctuations();
void UpdateSupplyDemand();
float CalculateTransactionProfit(const FCargoItem& Item, float SellPrice) const;
void CleanupExpiredCargo();
};
