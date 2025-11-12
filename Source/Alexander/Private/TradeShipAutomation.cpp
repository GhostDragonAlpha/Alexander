// Copyright Epic Games, Inc. All Rights Reserved.

#include "TradeShipAutomation.h"
#include "TradeMissionSystem.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "Spaceship.h"

UTradeShipAutomation::UTradeShipAutomation()
{
	AutomationUpdateInterval = 1.0f; // Update every second
	RouteDiscoveryRange = 100000.0f; // 100k units
	MaxDangerThreshold = 0.8f; // Avoid routes > 80% danger
	ProfitabilityThreshold = 0.1f; // 10% minimum profit
	FuelWarningThreshold = 0.2f; // Warn at 20% fuel
	bEnableAutomaticFlee = true;
	bEnableFleetCoordination = true;
	bShowDebugMessages = false;

	AutomationTimer = 0.0f;
	TotalTradesExecuted = 0;
	TotalRoutesCompleted = 0;
	TotalProfitGenerated = 0.0f;
	ShipsLost = 0;
}

void UTradeShipAutomation::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("TradeShipAutomation initialized"));

	// Get trade mission system
	UWorld* World = GetWorld();
	if (World)
	{
		TradeMissionSystem = World->GetSubsystem<UTradeMissionSystem>();
	}

	UE_LOG(LogTemp, Log, TEXT("TradeShipAutomation subsystems initialized"));
}

void UTradeShipAutomation::Deinitialize()
{
	// Clear all data
	AutomatedShips.Empty();
	TradeRoutes.Empty();
	Fleets.Empty();
	ShipSettings.Empty();

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("TradeShipAutomation deinitialized"));
}

void UTradeShipAutomation::Tick(float DeltaTime)
{
	// Update automation
	AutomationTimer += DeltaTime;
	if (AutomationTimer >= AutomationUpdateInterval)
	{
		UpdateAutomation(DeltaTime);
		AutomationTimer = 0.0f;
	}
}

bool UTradeShipAutomation::RegisterShip(ASpaceship* Ship, ETradeShipClass ShipClass, FString OwnerPlayerID)
{
	if (!Ship)
	{
		return false;
	}

	// Check if ship is already registered
	if (AutomatedShips.Contains(Ship))
	{
		return false;
	}

	FAutomatedShipData ShipData;
	ShipData.Ship = Ship;
	ShipData.ShipClass = ShipClass;
	ShipData.OwnerPlayerID = OwnerPlayerID;
	ShipData.bIsPlayerControlled = (OwnerPlayerID == "Player1"); // Simplified check

	// Set ship stats based on class
	switch (ShipClass)
	{
		case ETradeShipClass::LightFreighter:
			ShipData.CargoCapacity = 100.0f;
			ShipData.TravelSpeed = 1500.0f;
			ShipData.DefenseRating = 10.0f;
			ShipData.EvasionRating = 30.0f;
			ShipData.MaxHealth = 100.0f;
			ShipData.MaxShield = 50.0f;
			ShipData.MaxFuel = 100.0f;
			break;

		case ETradeShipClass::HeavyFreighter:
			ShipData.CargoCapacity = 500.0f;
			ShipData.TravelSpeed = 800.0f;
			ShipData.DefenseRating = 30.0f;
			ShipData.EvasionRating = 10.0f;
			ShipData.MaxHealth = 300.0f;
			ShipData.MaxShield = 150.0f;
			ShipData.MaxFuel = 300.0f;
			break;

		case ETradeShipClass::BlockadeRunner:
			ShipData.CargoCapacity = 150.0f;
			ShipData.TravelSpeed = 2000.0f;
			ShipData.DefenseRating = 15.0f;
			ShipData.EvasionRating = 40.0f;
			ShipData.MaxHealth = 150.0f;
			ShipData.MaxShield = 75.0f;
			ShipData.MaxFuel = 150.0f;
			break;

		case ETradeShipClass::TradeCruiser:
			ShipData.CargoCapacity = 300.0f;
			ShipData.TravelSpeed = 1200.0f;
			ShipData.DefenseRating = 50.0f;
			ShipData.EvasionRating = 20.0f;
			ShipData.MaxHealth = 400.0f;
			ShipData.MaxShield = 200.0f;
			ShipData.MaxFuel = 250.0f;
			break;

		default:
			ShipData.CargoCapacity = 200.0f;
			ShipData.TravelSpeed = 1000.0f;
			ShipData.DefenseRating = 20.0f;
			ShipData.EvasionRating = 20.0f;
			ShipData.MaxHealth = 200.0f;
			ShipData.MaxShield = 100.0f;
			ShipData.MaxFuel = 200.0f;
			break;
	}

	// Initialize current stats
	ShipData.CurrentHealth = ShipData.MaxHealth;
	ShipData.CurrentShield = ShipData.MaxShield;
	ShipData.CurrentFuel = ShipData.MaxFuel;

	AutomatedShips.Add(Ship, ShipData);

	// Set default automation settings
	FTradeAutomationSettings DefaultSettings;
	ShipSettings.Add(Ship, DefaultSettings);

	UE_LOG(LogTemp, Log, TEXT("Ship registered for automation: %s (Class: %d)"), *Ship->GetName(), (int32)ShipClass);

	return true;
}

bool UTradeShipAutomation::UnregisterShip(ASpaceship* Ship)
{
	if (!Ship || !AutomatedShips.Contains(Ship))
	{
		return false;
	}

	AutomatedShips.Remove(Ship);
	ShipSettings.Remove(Ship);

	UE_LOG(LogTemp, Log, TEXT("Ship unregistered from automation: %s"), *Ship->GetName());

	return true;
}

FAutomatedShipData UTradeShipAutomation::GetShipData(ASpaceship* Ship) const
{
	const FAutomatedShipData* Data = AutomatedShips.Find(Ship);
	return Data ? *Data : FAutomatedShipData();
}

void UTradeShipAutomation::UpdateShipClass(ASpaceship* Ship, ETradeShipClass NewClass)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (ShipData)
	{
		ShipData->ShipClass = NewClass;
		UE_LOG(LogTemp, Log, TEXT("Ship %s class updated to %d"), *Ship->GetName(), (int32)NewClass);
	}
}

TArray<FAutomatedShipData> UTradeShipAutomation::GetAllAutomatedShips() const
{
	TArray<FAutomatedShipData> Ships;
	AutomatedShips.GenerateValueArray(Ships);
	return Ships;
}

TArray<FAutomatedShipData> UTradeShipAutomation::GetPlayerShips(FString PlayerID) const
{
	TArray<FAutomatedShipData> PlayerShips;

	for (const auto& Pair : AutomatedShips)
	{
		if (Pair.Value.OwnerPlayerID == PlayerID)
		{
			PlayerShips.Add(Pair.Value);
		}
	}

	return PlayerShips;
}

bool UTradeShipAutomation::CreateAutomatedTradeRoute(
	ASpaceship* Ship,
	FString StartStationID,
	FString EndStationID,
	TArray<FName> Commodities
)
{
	if (!Ship || !AutomatedShips.Contains(Ship))
	{
		return false;
	}

	FString RouteID = GenerateRouteID();

	FAutomatedTradeRoute Route;
	Route.RouteID = RouteID;
	Route.StartStationID = StartStationID;
	Route.EndStationID = EndStationID;
	Route.TradeCommodities = Commodities;
	Route.bIsActive = true;

	// Set default price limits
	for (FName Commodity : Commodities)
	{
		Route.BuyPriceLimits.Add(Commodity, 1000.0f); // Default max buy price
		Route.SellPriceLimits.Add(Commodity, 500.0f); // Default min sell price
	}

	TradeRoutes.Add(RouteID, Route);

	// Assign route to ship
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (ShipData)
	{
		ShipData->CurrentRouteID = RouteID;
	}

	UE_LOG(LogTemp, Log, TEXT("Trade route created: %s to %s"), *StartStationID, *EndStationID);

	return true;
}

bool UTradeShipAutomation::RemoveTradeRoute(FString RouteID)
{
	return TradeRoutes.Remove(RouteID) > 0;
}

FAutomatedTradeRoute UTradeShipAutomation::GetTradeRoute(FString RouteID) const
{
	const FAutomatedTradeRoute* Route = TradeRoutes.Find(RouteID);
	return Route ? *Route : FAutomatedTradeRoute();
}

FAutomatedTradeRoute UTradeShipAutomation::GetShipRoute(ASpaceship* Ship) const
{
	const FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (ShipData && !ShipData->CurrentRouteID.IsEmpty())
	{
		return GetTradeRoute(ShipData->CurrentRouteID);
	}

	return FAutomatedTradeRoute();
}

bool UTradeShipAutomation::UpdateTradeRoute(FString RouteID, const FAutomatedTradeRoute& NewRoute)
{
	FAutomatedTradeRoute* Route = TradeRoutes.Find(RouteID);
	if (Route)
	{
		*Route = NewRoute;
		return true;
	}

	return false;
}

TArray<FAutomatedTradeRoute> UTradeShipAutomation::GetAllTradeRoutes() const
{
	TArray<FAutomatedTradeRoute> Routes;
	TradeRoutes.GenerateValueArray(Routes);
	return Routes;
}

TArray<FAutomatedTradeRoute> UTradeShipAutomation::GetProfitableRoutes(ASpaceship* Ship, int32 Count) const
{
	TArray<FAutomatedTradeRoute> ProfitableRoutes;

	for (const auto& Pair : TradeRoutes)
	{
		const FAutomatedTradeRoute& Route = Pair.Value;
		
		if (Route.bIsActive)
		{
			float Profitability = CalculateRouteProfitability(Route);
			
			if (Profitability > ProfitabilityThreshold)
			{
				ProfitableRoutes.Add(Route);
			}
		}
	}

	// Sort by profitability (highest first)
	ProfitableRoutes.Sort([](const FAutomatedTradeRoute& A, const FAutomatedTradeRoute& B) {
		return A.EstimatedProfit > B.EstimatedProfit;
	});

	// Return top N
	if (ProfitableRoutes.Num() > Count)
	{
		ProfitableRoutes.RemoveAt(Count, ProfitableRoutes.Num() - Count);
	}

	return ProfitableRoutes;
}

bool UTradeShipAutomation::StartAutomatedTrading(ASpaceship* Ship)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return false;
	}

	if (ShipData->CurrentStatus == EAutomationStatus::Idle)
	{
		ShipData->CurrentStatus = EAutomationStatus::Traveling;
		UE_LOG(LogTemp, Log, TEXT("Automated trading started for ship: %s"), *Ship->GetName());
		return true;
	}

	return false;
}

bool UTradeShipAutomation::StopAutomatedTrading(ASpaceship* Ship)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return false;
	}

	ShipData->CurrentStatus = EAutomationStatus::Idle;
	UE_LOG(LogTemp, Log, TEXT("Automated trading stopped for ship: %s"), *Ship->GetName());

	return true;
}

bool UTradeShipAutomation::PauseAutomation(ASpaceship* Ship)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return false;
	}

	if (ShipData->CurrentStatus != EAutomationStatus::Idle)
	{
		ShipData->CurrentStatus = EAutomationStatus::Waiting;
		UE_LOG(LogTemp, Log, TEXT("Automation paused for ship: %s"), *Ship->GetName());
		return true;
	}

	return false;
}

bool UTradeShipAutomation::ResumeAutomation(ASpaceship* Ship)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return false;
	}

	if (ShipData->CurrentStatus == EAutomationStatus::Waiting)
	{
		ShipData->CurrentStatus = EAutomationStatus::Traveling;
		UE_LOG(LogTemp, Log, TEXT("Automation resumed for ship: %s"), *Ship->GetName());
		return true;
	}

	return false;
}

bool UTradeShipAutomation::IsShipAutomated(ASpaceship* Ship) const
{
	const FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	return ShipData && ShipData->CurrentStatus != EAutomationStatus::Idle;
}

EAutomationStatus UTradeShipAutomation::GetAutomationStatus(ASpaceship* Ship) const
{
	const FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	return ShipData ? ShipData->CurrentStatus : EAutomationStatus::Idle;
}

FString UTradeShipAutomation::CreateFleet(FString FleetName, FString OwnerPlayerID)
{
	FString FleetID = GenerateFleetID();

	FTradeFleetData Fleet;
	Fleet.FleetID = FleetID;
	Fleet.FleetName = FleetName;
	Fleet.OwnerPlayerID = OwnerPlayerID;
	Fleet.bIsActive = true;

	Fleets.Add(FleetID, Fleet);

	UE_LOG(LogTemp, Log, TEXT("Fleet created: %s (ID: %s)"), *FleetName, *FleetID);

	return FleetID;
}

bool UTradeShipAutomation::DisbandFleet(FString FleetID)
{
	FTradeFleetData* Fleet = Fleets.Find(FleetID);
	if (!Fleet)
	{
		return false;
	}

	// Remove all ships from fleet
	for (const FString& ShipID : Fleet->ShipIDs)
	{
		// Would find ship and update its data
	}

	Fleets.Remove(FleetID);
	UE_LOG(LogTemp, Log, TEXT("Fleet disbanded: %s"), *FleetID);

	return true;
}

bool UTradeShipAutomation::AddShipToFleet(FString ShipID, FString FleetID)
{
	FTradeFleetData* Fleet = Fleets.Find(FleetID);
	if (!Fleet)
	{
		return false;
	}

	Fleet->ShipIDs.AddUnique(ShipID);
	UE_LOG(LogTemp, Log, TEXT("Ship %s added to fleet %s"), *ShipID, *FleetID);

	return true;
}

bool UTradeShipAutomation::RemoveShipFromFleet(FString ShipID, FString FleetID)
{
	FTradeFleetData* Fleet = Fleets.Find(FleetID);
	if (!Fleet)
	{
		return false;
	}

	Fleet->ShipIDs.Remove(ShipID);
	UE_LOG(LogTemp, Log, TEXT("Ship %s removed from fleet %s"), *ShipID, *FleetID);

	return true;
}

FTradeFleetData UTradeShipAutomation::GetFleetData(FString FleetID) const
{
	const FTradeFleetData* Fleet = Fleets.Find(FleetID);
	return Fleet ? *Fleet : FTradeFleetData();
}

TArray<FTradeFleetData> UTradeShipAutomation::GetPlayerFleets(FString PlayerID) const
{
	TArray<FTradeFleetData> PlayerFleets;

	for (const auto& Pair : Fleets)
	{
		if (Pair.Value.OwnerPlayerID == PlayerID)
		{
			PlayerFleets.Add(Pair.Value);
		}
	}

	return PlayerFleets;
}

TArray<FAutomatedShipData> UTradeShipAutomation::GetFleetShips(FString FleetID) const
{
	TArray<FAutomatedShipData> FleetShips;

	const FTradeFleetData* Fleet = Fleets.Find(FleetID);
	if (Fleet)
	{
		for (const auto& Pair : AutomatedShips)
		{
			if (Fleet->ShipIDs.Contains(Pair.Value.Ship->GetName()))
			{
				FleetShips.Add(Pair.Value);
			}
		}
	}

	return FleetShips;
}

bool UTradeShipAutomation::UpdateFleetSettings(FString FleetID, const FTradeAutomationSettings& Settings)
{
	FTradeFleetData* Fleet = Fleets.Find(FleetID);
	if (!Fleet)
	{
		return false;
	}

	// Update settings for all ships in fleet
	for (const FString& ShipID : Fleet->ShipIDs)
	{
		for (auto& Pair : AutomatedShips)
		{
			if (Pair.Value.Ship.IsValid() && Pair.Value.Ship->GetName() == ShipID)
			{
				ShipSettings.Add(Pair.Value.Ship.Get(), Settings);
				break;
			}
		}
	}

	return true;
}

void UTradeShipAutomation::ProcessAutomatedTrade(ASpaceship* Ship, float DeltaTime)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return;
	}

	// Update ship status based on current state
	switch (ShipData->CurrentStatus)
	{
		case EAutomationStatus::Traveling:
			// Simulate travel progress
			FindAndExecuteTrade(Ship);
			break;

		case EAutomationStatus::Loading:
			// Simulate loading cargo
			LoadCargo(Ship, ShipData->CurrentRouteID);
			break;

		case EAutomationStatus::Unloading:
			// Simulate unloading cargo
			UnloadCargo(Ship, ShipData->CurrentRouteID);
			break;

		case EAutomationStatus::Trading:
			// Execute trade
			ExecuteTrade(Ship, GetShipRoute(Ship));
			break;

		case EAutomationStatus::UnderAttack:
			// Handle combat
			HandleShipCombat(Ship, DeltaTime);
			break;
	}
}

bool UTradeShipAutomation::FindBestTradeOpportunity(ASpaceship* Ship, FAutomatedTradeRoute& OutRoute) const
{
	const FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return false;
	}

	// Get profitable routes for this ship
	TArray<FAutomatedTradeRoute> ProfitableRoutes = GetProfitableRoutes(Ship, 5);

	if (ProfitableRoutes.Num() == 0)
	{
		return false;
	}

	// Select best route based on ship capabilities and settings
	const FTradeAutomationSettings* Settings = ShipSettings.Find(Ship);
	if (!Settings)
	{
		return false;
	}

	for (const FAutomatedTradeRoute& Route : ProfitableRoutes)
	{
		// Check danger level
		float DangerLevel = CalculateDangerLevel(Route);
		if (DangerLevel > Settings->MaxDangerLevel && Settings->bAvoidHostileFactions)
		{
			continue;
		}

		// Check if route is suitable for ship class
		if (IsRouteSuitableForShip(Route, ShipData->ShipClass))
		{
			OutRoute = Route;
			return true;
		}
	}

	return false;
}

bool UTradeShipAutomation::ExecuteTrade(ASpaceship* Ship, const FAutomatedTradeRoute& Route)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return false;
	}

	// Simulate trade execution
	float TradeProfit = Route.EstimatedProfit;
	
	// Update ship statistics
	ShipData->TotalProfitGenerated += TradeProfit;
	ShipData->SuccessfulTrades++;
	TotalProfitGenerated += TradeProfit;

	// Update route statistics
	FAutomatedTradeRoute* RouteData = TradeRoutes.Find(Route.RouteID);
	if (RouteData)
	{
		RouteData->TotalProfit += TradeProfit;
		RouteData->TotalRuns++;
		RouteData->AverageProfitPerRun = RouteData->TotalProfit / RouteData->TotalRuns;
		RouteData->Efficiency = CalculateRouteEfficiency(*RouteData);
	}

	// Broadcast trade completion
	OnAutomatedTradeCompleted.Broadcast(*ShipData);

	UE_LOG(LogTemp, Log, TEXT("Trade completed for ship %s: %.0f profit"), *Ship->GetName(), TradeProfit);

	return true;
}

float UTradeShipAutomation::CalculateRouteProfitability(const FAutomatedTradeRoute& Route) const
{
	// Simplified profitability calculation
	float BaseProfit = Route.EstimatedProfit;
	float RiskFactor = 1.0f - (Route.DangerLevel * 0.5f); // Reduce profit for dangerous routes
	float EfficiencyFactor = static_cast<float>(Route.Efficiency) / static_cast<float>(ERouteEfficiency::Excellent);

	return BaseProfit * RiskFactor * EfficiencyFactor;
}

void UTradeShipAutomation::UpdateTradeStatistics(ASpaceship* Ship, float Profit, bool bSuccess)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return;
	}

	if (bSuccess)
	{
		ShipData->SuccessfulTrades++;
		ShipData->TotalProfitGenerated += Profit;
	}
	else
	{
		ShipData->FailedTrades++;
	}

	TotalTradesExecuted++;
}

void UTradeShipAutomation::UpdateShipBehavior(ASpaceship* Ship, float DeltaTime)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return;
	}

	// Update ship position and status (simplified)
	switch (ShipData->CurrentStatus)
	{
		case EAutomationStatus::Traveling:
			// Simulate travel
			break;

		case EAutomationStatus::Loading:
			// Simulate loading
			ShipData->CurrentCargoLoad = FMath::Min(ShipData->CargoCapacity, ShipData->CurrentCargoLoad + DeltaTime * 50.0f);
			if (ShipData->CurrentCargoLoad >= ShipData->CargoCapacity * 0.8f)
			{
				ShipData->CurrentStatus = EAutomationStatus::Traveling;
			}
			break;

		case EAutomationStatus::Unloading:
			// Simulate unloading
			ShipData->CurrentCargoLoad = FMath::Max(0.0f, ShipData->CurrentCargoLoad - DeltaTime * 50.0f);
			if (ShipData->CurrentCargoLoad <= 0.0f)
			{
				ShipData->CurrentStatus = EAutomationStatus::Trading;
			}
			break;

		case EAutomationStatus::Trading:
			// Simulate trading
			break;
	}
}

void UTradeShipAutomation::HandleShipAttack(ASpaceship* Ship, float Damage)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return;
	}

	// Apply damage
	ShipData->CurrentShield = FMath::Max(0.0f, ShipData->CurrentShield - Damage * 0.5f);
	float RemainingDamage = Damage * 0.5f;
	ShipData->CurrentHealth = FMath::Max(0.0f, ShipData->CurrentHealth - RemainingDamage);

	ShipData->TimesAttacked++;

	// Update status
	if (ShipData->CurrentHealth <= 0.0f)
	{
		ShipData->CurrentStatus = EAutomationStatus::Destroyed;
		HandleShipDestruction(Ship);
	}
	else if (ShipData->CurrentShield <= 0.0f || ShipData->CurrentHealth < ShipData->MaxHealth * 0.3f)
	{
		ShipData->CurrentStatus = EAutomationStatus::UnderAttack;
		
		// Flee if enabled
		const FTradeAutomationSettings* Settings = ShipSettings.Find(Ship);
		if (Settings && Settings->bEnableAutomaticFlee)
		{
			FleeFromDanger(Ship);
		}
	}

	// Broadcast attack
	OnShipAttacked.Broadcast(*ShipData);

	UE_LOG(LogTemp, Warning, TEXT("Ship %s attacked! Health: %.0f/%.0f, Shield: %.0f/%.0f"),
		*Ship->GetName(),
		ShipData->CurrentHealth, ShipData->MaxHealth,
		ShipData->CurrentShield, ShipData->MaxShield);
}

void UTradeShipAutomation::HandleShipDestruction(ASpaceship* Ship)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return;
	}

	ShipData->CurrentStatus = EAutomationStatus::Destroyed;
	ShipsLost++;

	// Broadcast destruction
	OnShipDestroyed.Broadcast(*ShipData);

	UE_LOG(LogTemp, Error, TEXT("Ship destroyed: %s"), *Ship->GetName());

	// Remove from automation
	UnregisterShip(Ship);
}

void UTradeShipAutomation::FleeFromDanger(ASpaceship* Ship)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return;
	}

	// Change route to safer path
	ShipData->CurrentStatus = EAutomationStatus::Traveling;
	
	UE_LOG(LogTemp, Log, TEXT("Ship %s fleeing from danger"), *Ship->GetName());
}

void UTradeShipAutomation::RequestAssistance(ASpaceship* Ship)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return;
	}

	// Would request assistance from nearby ships
	UE_LOG(LogTemp, Log, TEXT("Ship %s requesting assistance"), *Ship->GetName());
}

float UTradeShipAutomation::GetShipProfit(ASpaceship* Ship) const
{
	const FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	return ShipData ? ShipData->TotalProfitGenerated : 0.0f;
}

float UTradeShipAutomation::GetFleetProfit(FString FleetID) const
{
	const FTradeFleetData* Fleet = Fleets.Find(FleetID);
	return Fleet ? Fleet->TotalFleetProfit : 0.0f;
}

float UTradeShipAutomation::GetPlayerTotalProfit(FString PlayerID) const
{
	float TotalProfit = 0.0f;

	for (const auto& Pair : AutomatedShips)
	{
		if (Pair.Value.OwnerPlayerID == PlayerID)
		{
			TotalProfit += Pair.Value.TotalProfitGenerated;
		}
	}

	return TotalProfit;
}

TMap<FString, float> UTradeShipAutomation::GetProfitByRoute(FString PlayerID) const
{
	TMap<FString, float> ProfitByRoute;

	for (const auto& Pair : AutomatedShips)
	{
		if (Pair.Value.OwnerPlayerID == PlayerID && !Pair.Value.CurrentRouteID.IsEmpty())
		{
			ProfitByRoute.FindOrAdd(Pair.Value.CurrentRouteID) += Pair.Value.TotalProfitGenerated;
		}
	}

	return ProfitByRoute;
}

TMap<FName, float> UTradeShipAutomation::GetProfitByCommodity(FString PlayerID) const
{
	TMap<FName, float> ProfitByCommodity;

	// Would track profit by commodity
	return ProfitByCommodity;
}

float UTradeShipAutomation::CalculateROI(FString PlayerID) const
{
	float TotalProfit = GetPlayerTotalProfit(PlayerID);
	float TotalInvestment = 0.0f;

	// Calculate total investment in ships
	for (const auto& Pair : AutomatedShips)
	{
		if (Pair.Value.OwnerPlayerID == PlayerID)
		{
			// Simplified ship value calculation
			float ShipValue = Pair.Value.CargoCapacity * 100.0f + Pair.Value.MaxHealth * 10.0f;
			TotalInvestment += ShipValue;
		}
	}

	if (TotalInvestment <= 0.0f)
	{
		return 0.0f;
	}

	return (TotalProfit / TotalInvestment) * 100.0f; // Return as percentage
}

TMap<FString, int32> UTradeShipAutomation::GetAutomationStatistics() const
{
	TMap<FString, int32> Statistics;

	Statistics.Add(TEXT("TotalShips"), AutomatedShips.Num());
	Statistics.Add(TEXT("TotalRoutes"), TradeRoutes.Num());
	Statistics.Add(TEXT("TotalFleets"), Fleets.Num());
	Statistics.Add(TEXT("TotalTradesExecuted"), TotalTradesExecuted);
	Statistics.Add(TEXT("TotalRoutesCompleted"), TotalRoutesCompleted);
	Statistics.Add(TEXT("ShipsLost"), ShipsLost);

	return Statistics;
}

FString UTradeShipAutomation::GetShipStatistics(ASpaceship* Ship) const
{
	const FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return TEXT("Ship not found in automation system");
	}

	FString Stats = FString::Printf(TEXT("=== SHIP STATISTICS: %s ===\n\n"), *Ship->GetName());
	Stats += FString::Printf(TEXT("Class: %d\n"), (int32)ShipData->ShipClass);
	Stats += FString::Printf(TEXT("Status: %d\n"), (int32)ShipData->CurrentStatus);
	Stats += FString::Printf(TEXT("Cargo Capacity: %.0f\n"), ShipData->CargoCapacity);
	Stats += FString::Printf(TEXT("Current Cargo: %.0f\n"), ShipData->CurrentCargoLoad);
	Stats += FString::Printf(TEXT("Health: %.0f/%.0f\n"), ShipData->CurrentHealth, ShipData->MaxHealth);
	Stats += FString::Printf(TEXT("Shield: %.0f/%.0f\n"), ShipData->CurrentShield, ShipData->MaxShield);
	Stats += FString::Printf(TEXT("Fuel: %.0f/%.0f\n"), ShipData->CurrentFuel, ShipData->MaxFuel);
	Stats += FString::Printf(TEXT("Total Profit: %.0f\n"), ShipData->TotalProfitGenerated);
	Stats += FString::Printf(TEXT("Successful Trades: %d\n"), ShipData->SuccessfulTrades);
	Stats += FString::Printf(TEXT("Failed Trades: %d\n"), ShipData->FailedTrades);
	Stats += FString::Printf(TEXT("Times Attacked: %d\n"), ShipData->TimesAttacked);

	return Stats;
}

FString UTradeShipAutomation::GetFleetReport(FString FleetID) const
{
	const FTradeFleetData* Fleet = Fleets.Find(FleetID);
	if (!Fleet)
	{
		return TEXT("Fleet not found");
	}

	FString Report = FString::Printf(TEXT("=== FLEET REPORT: %s ===\n\n"), *Fleet->FleetName);
	Report += FString::Printf(TEXT("Owner: %s\n"), *Fleet->OwnerPlayerID);
	Report += FString::Printf(TEXT("Ships: %d\n"), Fleet->ShipIDs.Num());
	Report += FString::Printf(TEXT("Total Profit: %.0f\n"), Fleet->TotalFleetProfit);
	Report += FString::Printf(TEXT("Monthly Cost: %.0f\n"), Fleet->MonthlyOperatingCost);
	Report += FString::Printf(TEXT("Efficiency: %d\n"), (int32)Fleet->FleetEfficiency);

	return Report;
}

FString UTradeShipAutomation::GetPlayerAutomationReport(FString PlayerID) const
{
	FString Report = FString::Printf(TEXT("=== AUTOMATION REPORT FOR PLAYER: %s ===\n\n"), *PlayerID);

	// Get player ships
	TArray<FAutomatedShipData> PlayerShips = GetPlayerShips(PlayerID);
	Report += FString::Printf(TEXT("Automated Ships: %d\n"), PlayerShips.Num());

	// Get player fleets
	TArray<FTradeFleetData> PlayerFleets = GetPlayerFleets(PlayerID);
	Report += FString::Printf(TEXT("Fleets: %d\n"), PlayerFleets.Num());

	// Calculate total profit
	float TotalProfit = GetPlayerTotalProfit(PlayerID);
	Report += FString::Printf(TEXT("Total Profit: %.0f\n"), TotalProfit);

	// Calculate ROI
	float ROI = CalculateROI(PlayerID);
	Report += FString::Printf(TEXT("Return on Investment: %.1f%%\n"), ROI);

	// Profit by route
	TMap<FString, float> ProfitByRoute = GetProfitByRoute(PlayerID);
	Report += TEXT("\nProfit by Route:\n");
	for (const auto& Pair : ProfitByRoute)
	{
		Report += FString::Printf(TEXT("  %s: %.0f\n"), *Pair.Key, Pair.Value);
	}

	return Report;
}

FString UTradeShipAutomation::ExportTradeData(FString PlayerID) const
{
	FString Export = TEXT("Trade Data Export\n");
	Export += TEXT("=================\n\n");

	// Export ship data
	TArray<FAutomatedShipData> PlayerShips = GetPlayerShips(PlayerID);
	for (const FAutomatedShipData& Ship : PlayerShips)
	{
		Export += FString::Printf(TEXT("Ship: %s\n"), *Ship.Ship->GetName());
		Export += FString::Printf(TEXT("Profit: %.0f\n"), Ship.TotalProfitGenerated);
		Export += FString::Printf(TEXT("Trades: %d\n\n"), Ship.SuccessfulTrades);
	}

	// Export fleet data
	TArray<FTradeFleetData> PlayerFleets = GetPlayerFleets(PlayerID);
	for (const FTradeFleetData& Fleet : PlayerFleets)
	{
		Export += FString::Printf(TEXT("Fleet: %s\n"), *Fleet.FleetName);
		Export += FString::Printf(TEXT("Profit: %.0f\n"), Fleet.TotalFleetProfit);
		Export += FString::Printf(TEXT("Ships: %d\n\n"), Fleet.ShipIDs.Num());
	}

	return Export;
}

FTradeAutomationSettings UTradeShipAutomation::GetAutomationSettings(ASpaceship* Ship) const
{
	const FTradeAutomationSettings* Settings = ShipSettings.Find(Ship);
	return Settings ? *Settings : FTradeAutomationSettings();
}

bool UTradeShipAutomation::SetAutomationSettings(ASpaceship* Ship, const FTradeAutomationSettings& Settings)
{
	if (!AutomatedShips.Contains(Ship))
	{
		return false;
	}

	ShipSettings.Add(Ship, Settings);
	return true;
}

bool UTradeShipAutomation::ResetAutomationSettings(ASpaceship* Ship)
{
	if (!AutomatedShips.Contains(Ship))
	{
		return false;
	}

	FTradeAutomationSettings DefaultSettings;
	ShipSettings.Add(Ship, DefaultSettings);

	return true;
}

void UTradeShipAutomation::UpdateAutomation(float DeltaTime)
{
	// Process all automated ships
	for (auto& Pair : AutomatedShips)
	{
		ProcessShipAutomation(Pair.Key, DeltaTime);
	}

	// Process all fleets
	for (auto& Pair : Fleets)
	{
		ProcessFleetAutomation(Pair.Value, DeltaTime);
	}
}

void UTradeShipAutomation::ProcessShipAutomation(ASpaceship* Ship, float DeltaTime)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return;
	}

	// Skip destroyed ships
	if (ShipData->CurrentStatus == EAutomationStatus::Destroyed)
	{
		return;
	}

	// Update ship behavior
	UpdateShipBehavior(Ship, DeltaTime);

	// Process automated trade
	ProcessAutomatedTrade(Ship, DeltaTime);

	// Handle repairs and refueling
	const FTradeAutomationSettings* Settings = ShipSettings.Find(Ship);
	if (Settings)
	{
		if (Settings->bAutoRepair && ShipData->CurrentHealth < ShipData->MaxHealth * 0.8f)
		{
			RepairShip(Ship);
		}

		if (Settings->bAutoRefuel && ShipData->CurrentFuel < ShipData->MaxFuel * Settings->FuelReserveThreshold)
		{
			RefuelShip(Ship);
		}
	}
}

void UTradeShipAutomation::ProcessFleetAutomation(FTradeFleetData& Fleet, float DeltaTime)
{
	if (!Fleet.bIsActive)
	{
		return;
	}

	// Update fleet statistics
	UpdateFleetStatistics(Fleet.FleetID);

	// Coordinate fleet movements (if enabled)
	if (bEnableFleetCoordination)
	{
		// Would coordinate ship movements and trade strategies
	}
}

void UTradeShipAutomation::FindAndExecuteTrade(ASpaceship* Ship)
{
	FAutomatedTradeRoute BestRoute;
	if (FindBestTradeOpportunity(Ship, BestRoute))
	{
		ExecuteTrade(Ship, BestRoute);
	}
}

void UTradeShipAutomation::TravelToStation(ASpaceship* Ship, FString StationID)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return;
	}

	// Simulate travel
	ShipData->CurrentStatus = EAutomationStatus::Traveling;
	ShipData->CurrentFuel -= 1.0f; // Consume fuel

	UE_LOG(LogTemp, Log, TEXT("Ship %s traveling to station %s"), *Ship->GetName(), *StationID);
}

void UTradeShipAutomation::LoadCargo(ASpaceship* Ship, FString StationID)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return;
	}

	ShipData->CurrentStatus = EAutomationStatus::Loading;
	UE_LOG(LogTemp, Log, TEXT("Ship %s loading cargo at %s"), *Ship->GetName(), *StationID);
}

void UTradeShipAutomation::UnloadCargo(ASpaceship* Ship, FString StationID)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return;
	}

	ShipData->CurrentStatus = EAutomationStatus::Unloading;
	UE_LOG(LogTemp, Log, TEXT("Ship %s unloading cargo at %s"), *Ship->GetName(), *StationID);
}

bool UTradeShipAutomation::ShouldFlee(ASpaceship* Ship) const
{
	const FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return false;
	}

	// Flee if health is low
	return ShipData->CurrentHealth < ShipData->MaxHealth * 0.2f;
}

void UTradeShipAutomation::RepairShip(ASpaceship* Ship)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return;
	}

	// Simulate repair
	float RepairAmount = ShipData->MaxHealth * 0.1f; // Repair 10% per tick
	ShipData->CurrentHealth = FMath::Min(ShipData->MaxHealth, ShipData->CurrentHealth + RepairAmount);

	UE_LOG(LogTemp, Log, TEXT("Ship %s repairing: %.0f/%.0f HP"),
		*Ship->GetName(), ShipData->CurrentHealth, ShipData->MaxHealth);
}

void UTradeShipAutomation::RefuelShip(ASpaceship* Ship)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return;
	}

	// Simulate refueling
	float FuelAmount = ShipData->MaxFuel * 0.1f; // Refuel 10% per tick
	ShipData->CurrentFuel = FMath::Min(ShipData->MaxFuel, ShipData->CurrentFuel + FuelAmount);

	UE_LOG(LogTemp, Log, TEXT("Ship %s refueling: %.0f/%.0f fuel"),
		*Ship->GetName(), ShipData->CurrentFuel, ShipData->MaxFuel);
}

FString UTradeShipAutomation::GenerateRouteID() const
{
	return FString::Printf(TEXT("Route_%d"), FMath::RandRange(10000, 99999));
}

FString UTradeShipAutomation::GenerateFleetID() const
{
	return FString::Printf(TEXT("Fleet_%d"), FMath::RandRange(100, 999));
}

void UTradeShipAutomation::LogAutomationEvent(const FString& ShipID, const FString& Event, const FString& Details) const
{
	if (bShowDebugMessages)
	{
		UE_LOG(LogTemp, Log, TEXT("[TradeShipAutomation] %s - %s: %s"), *ShipID, *Event, *Details);
	}
}

float UTradeShipAutomation::CalculateDangerLevel(const FAutomatedTradeRoute& Route) const
{
	// Base danger on route properties
	float BaseDanger = Route.DangerLevel;

	// Would factor in faction relationships, pirate activity, etc.
	return FMath::Clamp(BaseDanger, 0.0f, 1.0f);
}

ERouteEfficiency UTradeShipAutomation::CalculateRouteEfficiency(const FAutomatedTradeRoute& Route) const
{
	// Calculate efficiency based on profit, distance, and success rate
	if (Route.TotalRuns == 0)
	{
		return ERouteEfficiency::Average;
	}

	float ProfitPerRun = Route.TotalProfit / Route.TotalRuns;
	float EfficiencyScore = ProfitPerRun / 1000.0f; // Normalize

	if (EfficiencyScore > 10.0f)
	{
		return ERouteEfficiency::Excellent;
	}
	else if (EfficiencyScore > 5.0f)
	{
		return ERouteEfficiency::Good;
	}
	else if (EfficiencyScore > 2.0f)
	{
		return ERouteEfficiency::Average;
	}
	else if (EfficiencyScore > 0.5f)
	{
		return ERouteEfficiency::Poor;
	}
	else
	{
		return ERouteEfficiency::VeryPoor;
	}
}

void UTradeShipAutomation::UpdateFleetStatistics(FString FleetID)
{
	FTradeFleetData* Fleet = Fleets.Find(FleetID);
	if (!Fleet)
	{
		return;
	}

	// Recalculate fleet statistics
	float TotalProfit = 0.0f;
	ERouteEfficiency AverageEfficiency = ERouteEfficiency::Average;
	int32 ShipCount = 0;

	for (const FString& ShipID : Fleet->ShipIDs)
	{
		for (const auto& Pair : AutomatedShips)
		{
			if (Pair.Value.Ship->GetName() == ShipID)
			{
				TotalProfit += Pair.Value.TotalProfitGenerated;
				ShipCount++;
				break;
			}
		}
	}

	Fleet->TotalFleetProfit = TotalProfit;
	float EfficiencyValue = CalculateFleetEfficiency(FleetID);
	if (EfficiencyValue >= 0.8f)
	{
		Fleet->FleetEfficiency = ERouteEfficiency::Excellent;
	}
	else if (EfficiencyValue >= 0.6f)
	{
		Fleet->FleetEfficiency = ERouteEfficiency::Good;
	}
	else if (EfficiencyValue >= 0.4f)
	{
		Fleet->FleetEfficiency = ERouteEfficiency::Average;
	}
	else if (EfficiencyValue >= 0.2f)
	{
		Fleet->FleetEfficiency = ERouteEfficiency::Poor;
	}
	else
	{
		Fleet->FleetEfficiency = ERouteEfficiency::Terrible;
	}
}

void UTradeShipAutomation::HandleShipCombat(ASpaceship* Ship, float DeltaTime)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return;
	}

	// Simulate combat
	float DamageTaken = DeltaTime * 10.0f; // 10 DPS
	ProcessShipDamage(Ship, DamageTaken);
}

void UTradeShipAutomation::ProcessShipDamage(ASpaceship* Ship, float Damage)
{
	FAutomatedShipData* ShipData = AutomatedShips.Find(Ship);
	if (!ShipData)
	{
		return;
	}

	// Apply damage to shields first
	if (ShipData->CurrentShield > 0.0f)
	{
		ShipData->CurrentShield = FMath::Max(0.0f, ShipData->CurrentShield - Damage);
		float RemainingDamage = FMath::Max(0.0f, Damage - ShipData->CurrentShield);
		ShipData->CurrentHealth = FMath::Max(0.0f, ShipData->CurrentHealth - RemainingDamage);
	}
	else
	{
		ShipData->CurrentHealth = FMath::Max(0.0f, ShipData->CurrentHealth - Damage);
	}

	// Check if ship is destroyed
	if (ShipData->CurrentHealth <= 0.0f)
	{
		HandleShipDestruction(Ship);
	}
}

bool UTradeShipAutomation::IsRouteSuitableForShip(const FAutomatedTradeRoute& Route, ETradeShipClass ShipClass) const
{
	// Check if route is suitable for ship class
	switch (ShipClass)
	{
		case ETradeShipClass::LightFreighter:
			// Can use any route
			return true;

		case ETradeShipClass::HeavyFreighter:
			// Avoid dangerous routes
			return Route.DangerLevel < 0.6f;

		case ETradeShipClass::BlockadeRunner:
			// Specialized for dangerous routes
			return true;

		case ETradeShipClass::TradeCruiser:
			// Can handle most routes
			return Route.DangerLevel < 0.8f;

		default:
			return true;
	}
}

float UTradeShipAutomation::CalculateFleetEfficiency(FString FleetID) const
{
	const FTradeFleetData* Fleet = Fleets.Find(FleetID);
	if (!Fleet)
	{
		return 0.0f;
	}

	// Calculate efficiency based on ship count, status, and performance
	int32 ActiveShips = 0;
	int32 TotalShips = Fleet->ShipIDs.Num();
	float TotalEfficiency = 0.0f;

	for (const FString& ShipID : Fleet->ShipIDs)
	{
		// Find ship by name
		const FAutomatedShipData* ShipData = nullptr;
		for (const auto& Pair : AutomatedShips)
		{
			if (Pair.Key && Pair.Key->GetName() == ShipID)
			{
				ShipData = &Pair.Value;
				break;
			}
		}
		
		if (ShipData && ShipData->AutomationStatus == EAutomationStatus::Active)
		{
			ActiveShips++;
			// Add ship-specific efficiency calculation here
			TotalEfficiency += 1.0f; // Placeholder
		}
	}

	if (TotalShips == 0)
	{
		return 0.0f;
	}

	// Efficiency is based on active ships percentage and average efficiency
	float ActivityRatio = static_cast<float>(ActiveShips) / TotalShips;
	float AverageEfficiency = TotalEfficiency / TotalShips;

	return ActivityRatio * AverageEfficiency;
}

// ============================================================================
// STATUS & PROFIT QUERIES
// ============================================================================

FAutomatedTradeRouteStatus UTradeShipAutomation::GetAutomatedTradeRouteStatus(FString RouteID) const
{
	FAutomatedTradeRouteStatus Status;
	Status.RouteID = RouteID;
	Status.bIsActive = false;
	Status.CurrentProgress = 0.0f;
	Status.EstimatedCompletionTime = 0.0f;
	Status.TotalProfit = 0.0f;
	Status.ShipsAssigned = 0;

	const FAutomatedTradeRoute* Route = TradeRoutes.Find(RouteID);
	if (!Route)
	{
		return Status;
	}

	Status.bIsActive = true;

	// Count assigned ships
	for (const auto& Pair : AutomatedShips)
	{
		if (Pair.Value.CurrentRouteID == RouteID)
		{
			Status.ShipsAssigned++;
			Status.TotalProfit += Pair.Value.TotalProfit;
		}
	}

	// Calculate progress based on active ships
	if (Status.ShipsAssigned > 0)
	{
		float TotalProgress = 0.0f;
		int32 ActiveShips = 0;

		for (const auto& Pair : AutomatedShips)
		{
			if (Pair.Value.CurrentRouteID == RouteID && Pair.Value.AutomationStatus == EAutomationStatus::Active)
			{
				// Calculate progress based on route completion
				float ShipProgress = Pair.Value.RouteProgress;
				TotalProgress += ShipProgress;
				ActiveShips++;
			}
		}

		if (ActiveShips > 0)
		{
			Status.CurrentProgress = TotalProgress / ActiveShips;
		}

		// Estimate completion time
		float RemainingProgress = 1.0f - Status.CurrentProgress;
		if (RemainingProgress > 0.0f)
		{
			Status.EstimatedCompletionTime = (RemainingProgress * Route->EstimatedDuration) / ActiveShips;
		}
	}

	return Status;
}

float UTradeShipAutomation::GetFleetTradingProfit(FString FleetID) const
{
	const FTradeFleetData* Fleet = Fleets.Find(FleetID);
	if (!Fleet)
	{
		return 0.0f;
	}

	float TotalProfit = 0.0f;

	// Sum profit from all ships in the fleet
	for (const FString& ShipID : Fleet->ShipIDs)
	{
		// Find ship by name
		const FAutomatedShipData* ShipData = nullptr;
		for (const auto& Pair : AutomatedShips)
		{
			if (Pair.Key && Pair.Key->GetName() == ShipID)
			{
				ShipData = &Pair.Value;
				break;
			}
		}
		
		if (ShipData)
		{
			TotalProfit += ShipData->TotalProfit;
		}
	}

	return TotalProfit;
}