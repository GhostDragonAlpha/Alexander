// Copyright Epic Games, Inc. All Rights Reserved.

#include "EconomySystem.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UEconomySystem::UEconomySystem()
{
PrimaryComponentTick.bCanEverTick = true;
PrimaryComponentTick.TickInterval = 1.0f; // 1 Hz for economy updates

// Initialize faction reputation
FactionReputation.Add(EEconomicFaction::Industrial, 0.5f);
FactionReputation.Add(EEconomicFaction::Agricultural, 0.5f);
FactionReputation.Add(EEconomicFaction::Mining, 0.5f);
FactionReputation.Add(EEconomicFaction::Research, 0.5f);
FactionReputation.Add(EEconomicFaction::Military, 0.5f);
FactionReputation.Add(EEconomicFaction::Pirate, 0.0f);
FactionReputation.Add(EEconomicFaction::Trading, 0.5f);
FactionReputation.Add(EEconomicFaction::Independent, 0.5f);
}

void UEconomySystem::BeginPlay()
{
Super::BeginPlay();

InitializeMarket();

UE_LOG(LogTemp, Log, TEXT("EconomySystem initialized with %d commodities"), 
CurrentMarket.Num());
}

void UEconomySystem::TickComponent(float DeltaTime, ELevelTick TickType, 
FActorComponentTickFunction* ThisTickFunction)
{
Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

// Update market prices periodically
TimeSinceLastUpdate += DeltaTime;
if (TimeSinceLastUpdate >= MarketUpdateInterval)
{
UpdateMarketPrices(TimeSinceLastUpdate);
TimeSinceLastUpdate = 0.0f;
}

// Update cargo condition (decay)
UpdateCargoCondition(DeltaTime);
}

// ============================================================================
// PLAYER ECONOMY
// ============================================================================

float UEconomySystem::GetCurrentCargoVolume() const
{
float TotalVolume = 0.0f;
for (const FCargoItem& Item : CargoHold)
{
TotalVolume += Item.Commodity.VolumePerUnit * Item.Quantity;
}
return TotalVolume;
}

float UEconomySystem::GetAvailableCargoSpace() const
{
return MaxCargoCapacity - GetCurrentCargoVolume();
}

bool UEconomySystem::HasCargoSpace(const FCommodityData& Commodity, int32 Quantity) const
{
float RequiredSpace = Commodity.VolumePerUnit * Quantity;
return GetAvailableCargoSpace() >= RequiredSpace;
}

// ============================================================================
// TRADING
// ============================================================================

bool UEconomySystem::BuyCommodity(const FName& CommodityName, int32 Quantity, 
float PricePerUnit)
{
// Get commodity data
FMarketListing* Listing = CurrentMarket.Find(CommodityName);
if (!Listing || Listing->AvailableQuantity < Quantity)
{
UE_LOG(LogTemp, Warning, TEXT("Insufficient quantity available for %s"), 
*CommodityName.ToString());
return false;
}

// Calculate total cost
float TotalCost = (PricePerUnit * Quantity) + CalculateTradeFees(PricePerUnit * Quantity);

// Check if player can afford
if (!CanAffordTransaction(TotalCost))
{
UE_LOG(LogTemp, Warning, TEXT("Insufficient credits for purchase"));
return false;
}

// Check cargo space
if (!HasCargoSpace(Listing->Commodity, Quantity))
{
UE_LOG(LogTemp, Warning, TEXT("Insufficient cargo space"));
return false;
}

// Execute transaction
PlayerCredits -= TotalCost;
AddToCargo(Listing->Commodity, Quantity, PricePerUnit);
Listing->AvailableQuantity -= Quantity;

// Update market
Listing->Demand *= 1.05f; // Increase demand
Listing->Supply *= 0.95f; // Decrease supply

// Record transaction
FTransactionRecord Transaction;
Transaction.Timestamp = FDateTime::Now();
Transaction.CommodityName = CommodityName;
Transaction.Quantity = Quantity;
Transaction.PricePerUnit = PricePerUnit;
Transaction.TotalValue = TotalCost;
Transaction.bWasPurchase = true;
Transaction.Location = TEXT("CurrentStation"); // Would get from world
RecordTransaction(Transaction);

TotalExpenses += TotalCost;

UE_LOG(LogTemp, Log, TEXT("Purchased %d %s for %.2f credits"), 
Quantity, *CommodityName.ToString(), TotalCost);

return true;
}

bool UEconomySystem::SellCommodity(const FName& CommodityName, int32 Quantity, 
float PricePerUnit)
{
// Check if player has commodity
int32 AvailableQuantity = GetCommodityQuantityInCargo(CommodityName);
if (AvailableQuantity < Quantity)
{
UE_LOG(LogTemp, Warning, TEXT("Insufficient quantity in cargo"));
return false;
}

// Calculate revenue
float SaleRevenue = (PricePerUnit * Quantity) - CalculateTradeFees(PricePerUnit * Quantity);

// Execute transaction
PlayerCredits += SaleRevenue;
RemoveFromCargo(CommodityName, Quantity);

// Update market
FMarketListing* Listing = CurrentMarket.Find(CommodityName);
if (Listing)
{
Listing->AvailableQuantity += Quantity;
Listing->Demand *= 0.95f; // Decrease demand
Listing->Supply *= 1.05f; // Increase supply
}

// Calculate profit
float AveragePurchasePrice = GetAveragePurchasePrice(CommodityName);
float Profit = (PricePerUnit - AveragePurchasePrice) * Quantity;

// Record transaction
FTransactionRecord Transaction;
Transaction.Timestamp = FDateTime::Now();
Transaction.CommodityName = CommodityName;
Transaction.Quantity = Quantity;
Transaction.PricePerUnit = PricePerUnit;
Transaction.TotalValue = SaleRevenue;
Transaction.bWasPurchase = false;
Transaction.Location = TEXT("CurrentStation");
Transaction.Profit = Profit;
RecordTransaction(Transaction);

TotalRevenue += SaleRevenue;
TotalProfit += Profit;

UE_LOG(LogTemp, Log, TEXT("Sold %d %s for %.2f credits (Profit: %.2f)"),
Quantity, *CommodityName.ToString(), SaleRevenue, Profit);

return true;
}

float UEconomySystem::CalculateTradeFees(float TransactionValue) const
{
return TransactionValue * TradeFeePercentage;
}

bool UEconomySystem::CanAffordTransaction(float TotalCost) const
{
return PlayerCredits >= TotalCost;
}

int32 UEconomySystem::GetCommodityQuantityInCargo(const FName& CommodityName) const
{
int32 TotalQuantity = 0;
for (const FCargoItem& Item : CargoHold)
{
if (Item.Commodity.CommodityName == CommodityName)
{
TotalQuantity += Item.Quantity;
}
}
return TotalQuantity;
}

float UEconomySystem::GetAveragePurchasePrice(const FName& CommodityName) const
{
float TotalCost = 0.0f;
int32 TotalQuantity = 0;

for (const FCargoItem& Item : CargoHold)
{
if (Item.Commodity.CommodityName == CommodityName)
{
TotalCost += Item.PurchasePrice * Item.Quantity;
TotalQuantity += Item.Quantity;
}
}

return TotalQuantity > 0 ? TotalCost / TotalQuantity : 0.0f;
}

// ============================================================================
// MARKET SIMULATION
// ============================================================================

void UEconomySystem::UpdateMarketPrices(float DeltaTime)
{
SimulateMarketDynamics(DeltaTime);
ApplyRandomFluctuations();
UpdateSupplyDemand();

if (bShowDebug)
{
UE_LOG(LogTemp, Log, TEXT("Market prices updated"));
}
}

FMarketListing UEconomySystem::GetMarketListing(const FName& CommodityName) const
{
const FMarketListing* Listing = CurrentMarket.Find(CommodityName);
return Listing ? *Listing : FMarketListing();
}

TArray<FMarketListing> UEconomySystem::GetAllMarketListings() const
{
TArray<FMarketListing> Listings;
for (const auto& Pair : CurrentMarket)
{
Listings.Add(Pair.Value);
}
return Listings;
}

void UEconomySystem::SetMarketDemand(const FName& CommodityName, float NewDemand)
{
FMarketListing* Listing = CurrentMarket.Find(CommodityName);
if (Listing)
{
Listing->Demand = FMath::Clamp(NewDemand, 0.1f, 3.0f);
Listing->CurrentBuyPrice = CalculateDynamicPrice(Listing->Commodity, 
Listing->Demand, Listing->Supply);
}
}

void UEconomySystem::SetMarketSupply(const FName& CommodityName, float NewSupply)
{
FMarketListing* Listing = CurrentMarket.Find(CommodityName);
if (Listing)
{
Listing->Supply = FMath::Clamp(NewSupply, 0.1f, 3.0f);
Listing->CurrentBuyPrice = CalculateDynamicPrice(Listing->Commodity, 
Listing->Demand, Listing->Supply);
}
}

float UEconomySystem::CalculateDynamicPrice(const FCommodityData& Commodity, 
float Demand, float Supply) const
{
// Base price modified by supply and demand
float Price = Commodity.BasePrice * (Demand / Supply);

// Add some randomness based on volatility
float Variation = FMath::FRandRange(-Commodity.PriceVolatility, Commodity.PriceVolatility);
Price *= (1.0f + Variation);

return FMath::Max(Price, 1.0f); // Minimum price of 1 credit
}

// ============================================================================
// TRADE ROUTES
// ============================================================================

TArray<FTradeRoute> UEconomySystem::FindProfitableRoutes(float MaxDistance, 
float MinProfit) const
{
TArray<FTradeRoute> ProfitableRoutes;

for (const FTradeRoute& Route : KnownTradeRoutes)
{
if (Route.Distance <= MaxDistance && 
Route.EstimatedProfit >= MinProfit &&
Route.Status == ETradeRouteStatus::Active)
{
ProfitableRoutes.Add(Route);
}
}

// Sort by profit (highest first)
ProfitableRoutes.Sort([](const FTradeRoute& A, const FTradeRoute& B) {
return A.EstimatedProfit > B.EstimatedProfit;
});

return ProfitableRoutes;
}

FTradeRoute UEconomySystem::CalculateTradeRoute(const FVector& StartLocation, 
const FVector& EndLocation) const
{
FTradeRoute Route;
Route.RouteName = FName(*FString::Printf(TEXT("Route_%d"), 
FMath::RandRange(1000, 9999)));
Route.StartLocation = StartLocation;
Route.EndLocation = EndLocation;
Route.Distance = FVector::Dist(StartLocation, EndLocation);
Route.Status = ETradeRouteStatus::Active;
Route.DangerLevel = FMath::FRandRange(0.0f, 1.0f);
Route.TravelTime = Route.Distance / 100000.0f; // Simplified
Route.EstimatedProfit = FMath::FRandRange(1000.0f, 10000.0f);

return Route;
}

void UEconomySystem::UpdateTradeRouteStatus(const FName& RouteName, 
ETradeRouteStatus NewStatus)
{
for (FTradeRoute& Route : KnownTradeRoutes)
{
if (Route.RouteName == RouteName)
{
Route.Status = NewStatus;
break;
}
}
}

// ============================================================================
// ECONOMIC EVENTS
// ============================================================================

void UEconomySystem::TriggerEconomicEvent(const FName& EventName)
{
// Implement specific economic events
UE_LOG(LogTemp, Log, TEXT("Economic event triggered: %s"), *EventName.ToString());
}

void UEconomySystem::SimulateMarketCrash(ECommodityType AffectedType, float Severity)
{
for (auto& Pair : CurrentMarket)
{
if (Pair.Value.Commodity.CommodityType == AffectedType)
{
Pair.Value.Demand *= (1.0f - Severity);
Pair.Value.CurrentBuyPrice *= (1.0f - Severity * 0.5f);
}
}

UE_LOG(LogTemp, Warning, TEXT("Market crash! %s prices down %.0f%%"), 
*UEnum::GetValueAsString(AffectedType), Severity * 100.0f);
}

void UEconomySystem::SimulateMarketBoom(ECommodityType AffectedType, float Magnitude)
{
for (auto& Pair : CurrentMarket)
{
if (Pair.Value.Commodity.CommodityType == AffectedType)
{
Pair.Value.Demand *= (1.0f + Magnitude);
Pair.Value.CurrentBuyPrice *= (1.0f + Magnitude * 0.5f);
}
}

UE_LOG(LogTemp, Log, TEXT("Market boom! %s prices up %.0f%%"), 
*UEnum::GetValueAsString(AffectedType), Magnitude * 100.0f);
}

void UEconomySystem::SimulateSupplyShortage(const FName& CommodityName, float Severity)
{
FMarketListing* Listing = CurrentMarket.Find(CommodityName);
if (Listing)
{
Listing->Supply *= (1.0f - Severity);
Listing->AvailableQuantity = FMath::Max(0, 
int32(Listing->AvailableQuantity * (1.0f - Severity)));
Listing->CurrentBuyPrice *= (1.0f + Severity);

UE_LOG(LogTemp, Warning, TEXT("Supply shortage! %s availability reduced %.0f%%"), 
*CommodityName.ToString(), Severity * 100.0f);
}
}

// ============================================================================
// TRANSACTION HISTORY
// ============================================================================

void UEconomySystem::RecordTransaction(const FTransactionRecord& Transaction)
{
TransactionHistory.Add(Transaction);
TotalTransactions++;

// Keep history manageable
if (TransactionHistory.Num() > 1000)
{
TransactionHistory.RemoveAt(0, 100);
}
}

TArray<FTransactionRecord> UEconomySystem::GetTransactionHistory(int32 Count) const
{
TArray<FTransactionRecord> RecentTransactions;
int32 StartIndex = FMath::Max(0, TransactionHistory.Num() - Count);

for (int32 i = StartIndex; i < TransactionHistory.Num(); i++)
{
RecentTransactions.Add(TransactionHistory[i]);
}

return RecentTransactions;
}

float UEconomySystem::GetTotalProfit() const
{
return TotalProfit;
}

float UEconomySystem::GetAverageProfit() const
{
return TotalTransactions > 0 ? TotalProfit / TotalTransactions : 0.0f;
}

FTransactionRecord UEconomySystem::GetBestTrade() const
{
FTransactionRecord BestTrade;
float MaxProfit = -99999.0f;

for (const FTransactionRecord& Transaction : TransactionHistory)
{
if (!Transaction.bWasPurchase && Transaction.Profit > MaxProfit)
{
MaxProfit = Transaction.Profit;
BestTrade = Transaction;
}
}

return BestTrade;
}

// ============================================================================
// CARGO MANAGEMENT
// ============================================================================

bool UEconomySystem::AddToCargo(const FCommodityData& Commodity, int32 Quantity, 
float PurchasePrice)
{
if (!HasCargoSpace(Commodity, Quantity))
{
return false;
}

// Check if we already have this commodity
bool bFound = false;
for (FCargoItem& Item : CargoHold)
{
if (Item.Commodity.CommodityName == Commodity.CommodityName)
{
// Add to existing stack
Item.Quantity += Quantity;
bFound = true;
break;
}
}

if (!bFound)
{
// Create new cargo item
FCargoItem NewItem;
NewItem.Commodity = Commodity;
NewItem.Quantity = Quantity;
NewItem.PurchasePrice = PurchasePrice;
NewItem.PurchaseTime = FDateTime::Now();
CargoHold.Add(NewItem);
}

return true;
}

bool UEconomySystem::RemoveFromCargo(const FName& CommodityName, int32 Quantity)
{
int32 RemainingToRemove = Quantity;

for (int32 i = CargoHold.Num() - 1; i >= 0; i--)
{
if (CargoHold[i].Commodity.CommodityName == CommodityName)
{
if (CargoHold[i].Quantity <= RemainingToRemove)
{
RemainingToRemove -= CargoHold[i].Quantity;
CargoHold.RemoveAt(i);
}
else
{
CargoHold[i].Quantity -= RemainingToRemove;
RemainingToRemove = 0;
}

if (RemainingToRemove == 0)
{
break;
}
}
}

return RemainingToRemove == 0;
}

void UEconomySystem::DumpCargo(const FName& CommodityName)
{
for (int32 i = CargoHold.Num() - 1; i >= 0; i--)
{
if (CargoHold[i].Commodity.CommodityName == CommodityName)
{
UE_LOG(LogTemp, Warning, TEXT("Dumped %d units of %s"), 
CargoHold[i].Quantity, *CommodityName.ToString());
CargoHold.RemoveAt(i);
}
}
}

void UEconomySystem::DumpAllCargo()
{
UE_LOG(LogTemp, Warning, TEXT("Dumped all cargo (%d items)"), CargoHold.Num());
CargoHold.Empty();
}

void UEconomySystem::UpdateCargoCondition(float DeltaTime)
{
// Decay perishable goods
for (FCargoItem& Item : CargoHold)
{
if (Item.Commodity.bIsPerishable && Item.Commodity.DecayRate > 0.0f)
{
float DecayAmount = Item.Commodity.DecayRate * (DeltaTime / 3600.0f); // Per hour
int32 DecayedQuantity = FMath::FloorToInt(Item.Quantity * DecayAmount);

if (DecayedQuantity > 0)
{
Item.Quantity = FMath::Max(0, Item.Quantity - DecayedQuantity);

if (bShowDebug)
{
UE_LOG(LogTemp, Warning, TEXT("%s decayed by %d units"), 
*Item.Commodity.CommodityName.ToString(), DecayedQuantity);
}
}
}
}

// Remove empty cargo items
CleanupExpiredCargo();
}

float UEconomySystem::GetCargoValue() const
{
float TotalValue = 0.0f;

for (const FCargoItem& Item : CargoHold)
{
// Use current market price if available
FMarketListing Listing = GetMarketListing(Item.Commodity.CommodityName);
float Price = Listing.CurrentSellPrice > 0.0f ? 
Listing.CurrentSellPrice : Item.Commodity.BasePrice;

TotalValue += Price * Item.Quantity;
}

return TotalValue;
}

// ============================================================================
// FACTION ECONOMICS
// ============================================================================

float UEconomySystem::GetFactionDiscount(EEconomicFaction Faction) const
{
const float* Reputation = FactionReputation.Find(Faction);
if (!Reputation)
{
return 0.0f;
}

// Higher reputation = better discount (up to 20%)
return FMath::Clamp(*Reputation * 0.2f, 0.0f, 0.2f);
}

void UEconomySystem::ModifyFactionReputation(EEconomicFaction Faction, float Delta)
{
float* Reputation = FactionReputation.Find(Faction);
if (Reputation)
{
*Reputation = FMath::Clamp(*Reputation + Delta, -1.0f, 1.0f);
}
}

bool UEconomySystem::CanTradeWithFaction(EEconomicFaction Faction) const
{
const float* Reputation = FactionReputation.Find(Faction);
return Reputation && *Reputation > -0.5f; // Need > -50% reputation
}

// ============================================================================
// SMUGGLING & ILLEGAL GOODS
// ============================================================================

bool UEconomySystem::HasIllegalCargo() const
{
for (const FCargoItem& Item : CargoHold)
{
if (Item.Commodity.bIsIllegal)
{
return true;
}
}
return false;
}

float UEconomySystem::CalculateFineForIllegalCargo() const
{
float TotalFine = 0.0f;

for (const FCargoItem& Item : CargoHold)
{
if (Item.Commodity.bIsIllegal)
{
TotalFine += Item.Commodity.BasePrice * Item.Quantity * 2.0f; // 2x value
}
}

return TotalFine;
}

void UEconomySystem::ConfiscateIllegalCargo()
{
float ConfiscatedValue = 0.0f;

for (int32 i = CargoHold.Num() - 1; i >= 0; i--)
{
if (CargoHold[i].Commodity.bIsIllegal)
{
ConfiscatedValue += CargoHold[i].Commodity.BasePrice * CargoHold[i].Quantity;
UE_LOG(LogTemp, Warning, TEXT("Confiscated %d units of %s"), 
CargoHold[i].Quantity, *CargoHold[i].Commodity.CommodityName.ToString());
CargoHold.RemoveAt(i);
}
}

UE_LOG(LogTemp, Warning, TEXT("Total confiscated value: %.2f credits"), 
ConfiscatedValue);
}

float UEconomySystem::GetSmugglingRisk(const FName& CommodityName) const
{
FMarketListing Listing = GetMarketListing(CommodityName);
return Listing.Commodity.bIsIllegal ? 0.3f : 0.0f; // 30% base risk
}

// ============================================================================
// STATISTICS & REPORTING
// ============================================================================

FString UEconomySystem::GetEconomyReport() const
{
FString Report = TEXT("=== ECONOMY REPORT ===\n\n");

Report += FString::Printf(TEXT("Credits: %.2f\n"), PlayerCredits);
Report += FString::Printf(TEXT("Cargo: %.1f / %.1f m³ (%.1f%%)\n"), 
GetCurrentCargoVolume(), MaxCargoCapacity, 
(GetCurrentCargoVolume() / MaxCargoCapacity) * 100.0f);
Report += FString::Printf(TEXT("Cargo Value: %.2f credits\n\n"), GetCargoValue());

Report += FString::Printf(TEXT("Total Transactions: %d\n"), TotalTransactions);
Report += FString::Printf(TEXT("Total Profit: %.2f credits\n"), TotalProfit);
Report += FString::Printf(TEXT("Average Profit: %.2f credits\n"), GetAverageProfit());
Report += FString::Printf(TEXT("Total Revenue: %.2f credits\n"), TotalRevenue);
Report += FString::Printf(TEXT("Total Expenses: %.2f credits\n\n"), TotalExpenses);

if (HasIllegalCargo())
{
Report += TEXT("⚠️ WARNING: Illegal cargo detected!\n");
Report += FString::Printf(TEXT("Potential Fine: %.2f credits\n"), 
CalculateFineForIllegalCargo());
}

return Report;
}

FString UEconomySystem::GetCargoManifest() const
{
FString Manifest = TEXT("=== CARGO MANIFEST ===\n\n");

for (const FCargoItem& Item : CargoHold)
{
Manifest += FString::Printf(TEXT("%s: %d units\n"), 
*Item.Commodity.CommodityName.ToString(), Item.Quantity);
Manifest += FString::Printf(TEXT("  Purchase Price: %.2f credits/unit\n"), 
Item.PurchasePrice);
Manifest += FString::Printf(TEXT("  Total Value: %.2f credits\n"), 
Item.PurchasePrice * Item.Quantity);

if (Item.Commodity.bIsIllegal)
{
Manifest += TEXT("  [ILLEGAL]\n");
}
if (Item.Commodity.bIsPerishable)
{
Manifest += TEXT("  [PERISHABLE]\n");
}

Manifest += TEXT("\n");
}

return Manifest;
}

FString UEconomySystem::GetProfitReport() const
{
FString Report = TEXT("=== PROFIT REPORT ===\n\n");

TArray<FTransactionRecord> RecentTrades = GetTransactionHistory(10);

for (const FTransactionRecord& Trade : RecentTrades)
{
if (!Trade.bWasPurchase)
{
Report += FString::Printf(TEXT("%s: %d units @ %.2f = %.2f profit\n"),
*Trade.CommodityName.ToString(), Trade.Quantity, 
Trade.PricePerUnit, Trade.Profit);
}
}

FTransactionRecord BestTrade = GetBestTrade();
if (BestTrade.Quantity > 0)
{
Report += TEXT("\nBest Trade:\n");
Report += FString::Printf(TEXT("%s: %.2f profit\n"), 
*BestTrade.CommodityName.ToString(), BestTrade.Profit);
}

return Report;
}

// ============================================================================
// PRIVATE HELPER FUNCTIONS
// ============================================================================

void UEconomySystem::InitializeMarket()
{
// Create default commodities (would normally load from data table)
TArray<FName> CommodityNames = {
TEXT("Iron Ore"), TEXT("Gold"), TEXT("Fuel Cells"), TEXT("Food Supplies"),
TEXT("Medical Supplies"), TEXT("Computer Chips"), TEXT("Laser Weapons"),
TEXT("Luxury Goods"), TEXT("Contraband"), TEXT("Waste")
};

for (int32 i = 0; i < CommodityNames.Num(); i++)
{
FCommodityData Commodity;
Commodity.CommodityName = CommodityNames[i];
Commodity.CommodityType = ECommodityType(i);
Commodity.BasePrice = FMath::FRandRange(50.0f, 500.0f);
Commodity.PriceVolatility = FMath::FRandRange(0.1f, 0.3f);
Commodity.VolumePerUnit = FMath::FRandRange(0.5f, 2.0f);
Commodity.Mass = FMath::FRandRange(0.5f, 3.0f);
Commodity.bIsIllegal = (i == 8); // Contraband
Commodity.bIsPerishable = (i == 3 || i == 4); // Food, Medicine

FMarketListing Listing;
Listing.Commodity = Commodity;
Listing.AvailableQuantity = FMath::RandRange(100, 1000);
Listing.Demand = FMath::FRandRange(0.8f, 1.2f);
Listing.Supply = FMath::FRandRange(0.8f, 1.2f);
Listing.CurrentBuyPrice = CalculateDynamicPrice(Commodity, Listing.Demand, Listing.Supply);
Listing.CurrentSellPrice = Listing.CurrentBuyPrice * 0.9f; // 10% spread
Listing.LastUpdated = FDateTime::Now();

CurrentMarket.Add(CommodityNames[i], Listing);
}
}

void UEconomySystem::SimulateMarketDynamics(float DeltaTime)
{
// Natural market forces
for (auto& Pair : CurrentMarket)
{
FMarketListing& Listing = Pair.Value;

// Demand and supply gradually return to equilibrium
Listing.Demand = FMath::Lerp(Listing.Demand, 1.0f, 0.01f);
Listing.Supply = FMath::Lerp(Listing.Supply, 1.0f, 0.01f);

// Update prices
Listing.CurrentBuyPrice = CalculateDynamicPrice(Listing.Commodity, 
Listing.Demand, Listing.Supply);
Listing.CurrentSellPrice = Listing.CurrentBuyPrice * 0.9f;
Listing.LastUpdated = FDateTime::Now();
}
}

void UEconomySystem::ApplyRandomFluctuations()
{
// Random market events
for (auto& Pair : CurrentMarket)
{
if (FMath::FRand() < 0.1f) // 10% chance per update
{
float Change = FMath::FRandRange(-0.05f, 0.05f);
Pair.Value.Demand *= (1.0f + Change);
Pair.Value.Demand = FMath::Clamp(Pair.Value.Demand, 0.5f, 2.0f);
}
}
}

void UEconomySystem::UpdateSupplyDemand()
{
// Refresh available quantities
for (auto& Pair : CurrentMarket)
{
// Slowly replenish stock
Pair.Value.AvailableQuantity = FMath::Min(
Pair.Value.AvailableQuantity + FMath::RandRange(1, 10),
1000
);
}
}

float UEconomySystem::CalculateTransactionProfit(const FCargoItem& Item, 
float SellPrice) const
{
return (SellPrice - Item.PurchasePrice) * Item.Quantity;
}

void UEconomySystem::CleanupExpiredCargo()
{
for (int32 i = CargoHold.Num() - 1; i >= 0; i--)
{
if (CargoHold[i].Quantity <= 0)
{
CargoHold.RemoveAt(i);
}
}
}
