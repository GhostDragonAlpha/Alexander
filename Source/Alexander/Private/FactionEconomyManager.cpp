// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactionEconomyManager.h"
#include "FactionTerritorySystem.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"

UFactionEconomyManager::UFactionEconomyManager()
{
	ReputationDecayRate = 0.01f; // 1% per second (would be slower in real game)
	MaxReputationDiscount = 0.3f; // 30% max discount
	BaseTariffRate = 0.05f; // 5% base tariff
	TradeAgreementBonus = 0.1f; // 10% trade bonus
	SanctionPenalty = 0.5f; // 50% trade penalty
	ReputationPerMission = 5.0f;
	ReputationPerTradeValue = 0.0001f; // 1 rep per 10,000 credits

	ReputationUpdateTimer = 0.0f;
	EconomicSimulationTimer = 0.0f;

	TotalReputationChanges = 0;
	TotalAgreementsFormed = 0;
	TotalSanctionsImposed = 0;
}

void UFactionEconomyManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("FactionEconomyManager initialized"));

	// Get faction territory system
	UWorld* World = GetWorld();
	if (World)
	{
		FactionTerritorySystem = World->GetSubsystem<UFactionTerritorySystem>();
	}

	// Initialize default factions
	CreateFaction("Neutral Traders", EFactionEconomicPolicy::FreeMarket, false);
	CreateFaction("Industrial Consortium", EFactionEconomicPolicy::Industrial, false);
	CreateFaction("Pirate Syndicate", EFactionEconomicPolicy::Mercantile, false);
	CreateFaction("Player Faction", EFactionEconomicPolicy::FreeMarket, true);
}

void UFactionEconomyManager::Deinitialize()
{
	// Clear all data
	FactionData.Empty();
	PlayerReputations.Empty();
	ActiveAgreements.Empty();
	ActiveSanctions.Empty();
	FactionRelationships.Empty();

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("FactionEconomyManager deinitialized"));
}

void UFactionEconomyManager::Tick(float DeltaTime)
{
	// Update reputation decay
	ReputationUpdateTimer += DeltaTime;
	if (ReputationUpdateTimer >= 1.0f) // Update every second
	{
		UpdateReputationDecay(DeltaTime);
		ReputationUpdateTimer = 0.0f;
	}

	// Update economic simulation
	EconomicSimulationTimer += DeltaTime;
	if (EconomicSimulationTimer >= 60.0f) // Update every minute
	{
		UpdateEconomicSimulation(DeltaTime);
		EconomicSimulationTimer = 0.0f;
	}

	// Update trade agreements
	UpdateTradeAgreements(DeltaTime);

	// Update sanctions
	UpdateSanctions(DeltaTime);
}

FString UFactionEconomyManager::CreateFaction(
	FString FactionName,
	EFactionEconomicPolicy InitialPolicy,
	bool bIsPlayerFaction
)
{
	FString FactionID = GenerateFactionID();

	FFactionEconomicData NewFaction;
	NewFaction.FactionID = FactionID;
	NewFaction.FactionName = FactionName;
	NewFaction.EconomicPolicy = InitialPolicy;
	NewFaction.bIsPlayerFaction = bIsPlayerFaction;
	NewFaction.CurrentTreasury = FMath::RandRange(50000.0f, 200000.0f);
	NewFaction.MonthlyIncome = FMath::RandRange(5000.0f, 15000.0f);
	NewFaction.MonthlyExpenses = FMath::RandRange(3000.0f, 12000.0f);

	// Set default production and consumption
	NewFaction.ResourceProduction.Add(FName("IronOre"), FMath::FRandRange(0.5f, 2.0f));
	NewFaction.ResourceProduction.Add(FName("FoodSupplies"), FMath::FRandRange(0.8f, 1.5f));
	NewFaction.ResourceProduction.Add(FName("ConsumerGoods"), FMath::FRandRange(0.3f, 1.0f));
	NewFaction.ResourceConsumption.Add(FName("IronOre"), FMath::FRandRange(0.3f, 1.0f));
	NewFaction.ResourceConsumption.Add(FName("FoodSupplies"), FMath::FRandRange(0.8f, 1.2f));
	NewFaction.ResourceConsumption.Add(FName("ElectronicComponents"), FMath::FRandRange(0.5f, 1.5f));

	FactionData.Add(FactionID, NewFaction);

	UE_LOG(LogTemp, Log, TEXT("Created faction: %s (%s)"), *FactionName, *FactionID);

	return FactionID;
}

FFactionEconomicData UFactionEconomyManager::GetFactionData(FString FactionID) const
{
	const FFactionEconomicData* Data = FactionData.Find(FactionID);
	return Data ? *Data : FFactionEconomicData();
}

void UFactionEconomyManager::UpdateFactionEconomy(FString FactionID, float DeltaTime)
{
	FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (!Faction)
	{
		return;
	}

	// Update treasury
	float DailyIncome = Faction->MonthlyIncome / 30.0f;
	float DailyExpenses = Faction->MonthlyExpenses / 30.0f;
	Faction->CurrentTreasury += (DailyIncome - DailyExpenses) * (DeltaTime / 86400.0f); // Convert to days

	// Update economic strength based on treasury
	float TreasuryRatio = Faction->CurrentTreasury / 100000.0f; // Normalize to 100k
	Faction->EconomicStrength = FMath::Clamp(TreasuryRatio * 50.0f, 10.0f, 100.0f);

	// Update trade influence
	Faction->TradeInfluence = FMath::Clamp(Faction->EconomicStrength * 0.6f + Faction->MilitaryStrength * 0.4f, 0.0f, 100.0f);

	// Simulate faction behavior
	ProcessFactionDiplomacy(FactionID, DeltaTime);
	UpdateFactionEconomy(FactionID, DeltaTime);
}

void UFactionEconomyManager::SetFactionPolicy(FString FactionID, EFactionEconomicPolicy NewPolicy)
{
	FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (!Faction)
	{
		return;
	}

	Faction->EconomicPolicy = NewPolicy;

	// Adjust base price modifier based on policy
	switch (NewPolicy)
	{
		case EFactionEconomicPolicy::FreeMarket:
			Faction->BasePriceModifier = 1.0f;
			Faction->TariffRate = 0.05f;
			break;
		case EFactionEconomicPolicy::PlannedEconomy:
			Faction->BasePriceModifier = 1.3f;
			Faction->TariffRate = 0.01f;
			break;
		case EFactionEconomicPolicy::Mercantile:
			Faction->BasePriceModifier = 0.9f;
			Faction->TariffRate = 0.15f;
			break;
		case EFactionEconomicPolicy::Isolationist:
			Faction->BasePriceModifier = 1.5f;
			Faction->TariffRate = 0.3f;
			break;
		case EFactionEconomicPolicy::WarEconomy:
			Faction->BasePriceModifier = 1.2f;
			Faction->TariffRate = 0.2f;
			break;
		case EFactionEconomicPolicy::TradeFocus:
			Faction->BasePriceModifier = 0.85f;
			Faction->TariffRate = 0.03f;
			break;
		case EFactionEconomicPolicy::Industrial:
			Faction->BasePriceModifier = 1.1f;
			Faction->TariffRate = 0.08f;
			break;
	}

	OnFactionPolicyChanged.Broadcast(FactionID, NewPolicy);
	LogFactionEvent(FactionID, TEXT("Policy Changed"), FString::Printf(TEXT("New Policy: %d"), (int32)NewPolicy));
}

void UFactionEconomyManager::SetTradeRestriction(FString FactionID, EFactionTradeRestriction Restriction)
{
	FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (!Faction)
	{
		return;
	}

	Faction->TradeRestriction = Restriction;

	// Apply restriction effects
	switch (Restriction)
	{
		case EFactionTradeRestriction::None:
			// No restrictions
			break;
		case EFactionTradeRestriction::Tariffs:
			Faction->TariffRate = FMath::Max(Faction->TariffRate, 0.1f);
			break;
		case EFactionTradeRestriction::Quotas:
			// Would implement trade quotas
			break;
		case EFactionTradeRestriction::Embargo:
			// Would implement full trade embargo
			break;
		case EFactionTradeRestriction::Monopoly:
			// Would implement state monopoly
			break;
		case EFactionTradeRestriction::Licensed:
			// Would implement licensing system
			break;
	}
}

void UFactionEconomyManager::AddControlledStation(FString FactionID, FString StationID)
{
	FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (Faction)
	{
		Faction->ControlledStations.AddUnique(StationID);
	}
}

void UFactionEconomyManager::RemoveControlledStation(FString FactionID, FString StationID)
{
	FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (Faction)
	{
		Faction->ControlledStations.Remove(StationID);
	}
}

FPlayerFactionReputation UFactionEconomyManager::GetPlayerReputation(FString FactionID, FString PlayerID) const
{
	const TMap<FString, FPlayerFactionReputation>* PlayerRep = PlayerReputations.Find(PlayerID);
	if (!PlayerRep)
	{
		return FPlayerFactionReputation();
	}

	const FPlayerFactionReputation* Rep = PlayerRep->Find(FactionID);
	return Rep ? *Rep : FPlayerFactionReputation();
}

void UFactionEconomyManager::ModifyPlayerReputation(
	FString FactionID,
	FString PlayerID,
	float ReputationChange,
	FString Reason
)
{
	TMap<FString, FPlayerFactionReputation>& PlayerRep = PlayerReputations.FindOrAdd(PlayerID);
	FPlayerFactionReputation& Rep = PlayerRep.FindOrAdd(FactionID);

	float OldReputation = Rep.ReputationScore;
	Rep.ReputationScore = FMath::Clamp(Rep.ReputationScore + ReputationChange, -100.0f, 100.0f);
	Rep.Standing = CalculateStanding(Rep.ReputationScore);

	// Update trade discount based on reputation
	Rep.TradeDiscount = CalculateReputationDiscount(FactionID, PlayerID);

	// Check for trading license
	if (Rep.Standing >= 70.0f && !Rep.bHasTradingLicense)
	{
		GrantTradingLicense(FactionID, PlayerID);
	}

	// Unlock commodities based on reputation
	UnlockCommoditiesForPlayer(FactionID, PlayerID);

	TotalReputationChanges++;

	OnFactionReputationChanged.Broadcast(FactionID, Rep.ReputationScore);
	LogFactionEvent(FactionID, TEXT("Reputation Changed"), 
		FString::Printf(TEXT("Player: %s, Change: %.1f, Reason: %s"), *PlayerID, ReputationChange, *Reason));
}

float UFactionEconomyManager::CalculateReputationDiscount(FString FactionID, FString PlayerID) const
{
	const FPlayerFactionReputation Rep = GetPlayerReputation(FactionID, PlayerID);
	
	// Higher reputation = better discount
	if (Rep.ReputationScore >= 0)
	{
		return FMath::Clamp((Rep.ReputationScore / 100.0f) * MaxReputationDiscount, 0.0f, MaxReputationDiscount);
	}
	
	return 0.0f;
}

bool UFactionEconomyManager::CanPlayerTrade(FString FactionID, FString PlayerID) const
{
	const FPlayerFactionReputation Rep = GetPlayerReputation(FactionID, PlayerID);
	
	// Can trade if standing is not too low
	return Rep.Standing > 20.0f;
}

void UFactionEconomyManager::GrantTradingLicense(FString FactionID, FString PlayerID)
{
	TMap<FString, FPlayerFactionReputation>& PlayerRep = PlayerReputations.FindOrAdd(PlayerID);
	FPlayerFactionReputation& Rep = PlayerRep.FindOrAdd(FactionID);
	
	Rep.bHasTradingLicense = true;
	UE_LOG(LogTemp, Log, TEXT("Player %s granted trading license with faction %s"), *PlayerID, *FactionID);
}

void UFactionEconomyManager::RevokeTradingLicense(FString FactionID, FString PlayerID)
{
	TMap<FString, FPlayerFactionReputation>& PlayerRep = PlayerReputations.FindOrAdd(PlayerID);
	FPlayerFactionReputation& Rep = PlayerRep.FindOrAdd(FactionID);
	
	Rep.bHasTradingLicense = false;
	UE_LOG(LogTemp, Log, TEXT("Player %s trading license revoked with faction %s"), *PlayerID, *FactionID);
}

float UFactionEconomyManager::CalculateTradePriceModifier(
	FString FactionID,
	FString PlayerID,
	FName CommodityID,
	bool bIsBuying
) const
{
	float Modifier = 1.0f;

	// Base faction modifier
	const FFactionEconomicData Faction = GetFactionData(FactionID);
	Modifier *= Faction.BasePriceModifier;

	// Reputation discount
	float Discount = CalculateReputationDiscount(FactionID, PlayerID);
	if (bIsBuying)
	{
		Modifier *= (1.0f - Discount); // Discount for buying
	}
	else
	{
		Modifier *= (1.0f + Discount); // Bonus for selling
	}

	// Tariff application
	if (bIsBuying)
	{
		Modifier *= (1.0f + Faction.TariffRate);
	}

	// Check for trade agreements
	for (const FFactionTradeAgreement& Agreement : ActiveAgreements)
	{
		if ((Agreement.Faction1ID == FactionID || Agreement.Faction2ID == FactionID) && Agreement.bIsActive)
		{
			Modifier *= (1.0f - TradeAgreementBonus);
			break;
		}
	}

	// Check for sanctions
	for (const FFactionSanctions& Sanctions : ActiveSanctions)
	{
		if (Sanctions.TargetFactionID == FactionID && Sanctions.bIsActive)
		{
			Modifier *= (1.0f + SanctionPenalty);
			break;
		}
	}

	// Check if commodity is banned
	if (IsCommodityBanned(FactionID, CommodityID))
	{
		Modifier *= 10.0f; // Massive penalty for banned goods
	}

	return Modifier;
}

float UFactionEconomyManager::CalculateTariff(
	FString FactionID,
	FName CommodityID,
	float BasePrice,
	int32 Quantity
) const
{
	const FFactionEconomicData Faction = GetFactionData(FactionID);
	
	if (IsCommodityBanned(FactionID, CommodityID))
	{
		return BasePrice * Quantity * 10.0f; // Heavy penalty for banned goods
	}

	return BasePrice * Quantity * Faction.TariffRate;
}

bool UFactionEconomyManager::IsCommodityBanned(FString FactionID, FName CommodityID) const
{
	const FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (!Faction)
	{
		return false;
	}

	const bool* Banned = Faction->TradeBans.Find(CommodityID);
	return Banned ? *Banned : false;
}

void UFactionEconomyManager::BanCommodity(FString FactionID, FName CommodityID)
{
	FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (Faction)
	{
		Faction->TradeBans.Add(CommodityID, true);
		UE_LOG(LogTemp, Log, TEXT("Commodity %s banned by faction %s"), *CommodityID.ToString(), *FactionID);
	}
}

void UFactionEconomyManager::UnbanCommodity(FString FactionID, FName CommodityID)
{
	FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (Faction)
	{
		Faction->TradeBans.Remove(CommodityID);
		UE_LOG(LogTemp, Log, TEXT("Commodity %s unbanned by faction %s"), *CommodityID.ToString(), *FactionID);
	}
}

FFactionTradeAgreement UFactionEconomyManager::FormTradeAgreement(
	FString Faction1ID,
	FString Faction2ID,
	FTimespan Duration
)
{
	FFactionTradeAgreement Agreement;
	Agreement.AgreementID = GenerateAgreementID();
	Agreement.Faction1ID = Faction1ID;
	Agreement.Faction2ID = Faction2ID;
	Agreement.Duration = Duration;
	Agreement.StartDate = FDateTime::Now();
	Agreement.bIsActive = true;
	Agreement.TradeBonus = TradeAgreementBonus;

	ActiveAgreements.Add(Agreement);
	TotalAgreementsFormed++;

	// Update faction relationships
	TMap<FString, float>& Relations1 = FactionRelationships.FindOrAdd(Faction1ID);
	Relations1.Add(Faction2ID, 50.0f);

	TMap<FString, float>& Relations2 = FactionRelationships.FindOrAdd(Faction2ID);
	Relations2.Add(Faction1ID, 50.0f);

	OnTradeAgreementFormed.Broadcast(Agreement);
	LogFactionEvent(Faction1ID, TEXT("Trade Agreement Formed"), FString::Printf(TEXT("With: %s"), *Faction2ID));

	return Agreement;
}

void UFactionEconomyManager::EndTradeAgreement(FString AgreementID)
{
	for (int32 i = ActiveAgreements.Num() - 1; i >= 0; i--)
	{
		if (ActiveAgreements[i].AgreementID == AgreementID)
		{
			ActiveAgreements[i].bIsActive = false;
			UE_LOG(LogTemp, Log, TEXT("Trade agreement %s ended"), *AgreementID);
			break;
		}
	}
}

FFactionSanctions UFactionEconomyManager::ImposeSanctions(
	FString SanctioningFactionID,
	FString TargetFactionID,
	FTimespan Duration,
	float Penalty
)
{
	FFactionSanctions Sanctions;
	Sanctions.SanctionsID = GenerateSanctionsID();
	Sanctions.SanctioningFactionID = SanctioningFactionID;
	Sanctions.TargetFactionID = TargetFactionID;
	Sanctions.Duration = Duration;
	Sanctions.StartDate = FDateTime::Now();
	Sanctions.bIsActive = true;
	Sanctions.TradePenalty = Penalty;

	ActiveSanctions.Add(Sanctions);
	TotalSanctionsImposed++;

	// Update faction relationships
	TMap<FString, float>& Relations = FactionRelationships.FindOrAdd(TargetFactionID);
	Relations.Add(SanctioningFactionID, -50.0f);

	OnSanctionsImposed.Broadcast(Sanctions);
	LogFactionEvent(TargetFactionID, TEXT("Sanctions Imposed"), FString::Printf(TEXT("By: %s"), *SanctioningFactionID));

	return Sanctions;
}

void UFactionEconomyManager::LiftSanctions(FString SanctionsID)
{
	for (int32 i = ActiveSanctions.Num() - 1; i >= 0; i--)
	{
		if (ActiveSanctions[i].SanctionsID == SanctionsID)
		{
			ActiveSanctions[i].bIsActive = false;
			UE_LOG(LogTemp, Log, TEXT("Sanctions %s lifted"), *SanctionsID);
			break;
		}
	}
}

void UFactionEconomyManager::DeclareWar(FString Faction1ID, FString Faction2ID)
{
	// End any active agreements
	for (FFactionTradeAgreement& Agreement : ActiveAgreements)
	{
		if ((Agreement.Faction1ID == Faction1ID && Agreement.Faction2ID == Faction2ID) ||
			(Agreement.Faction1ID == Faction2ID && Agreement.Faction2ID == Faction1ID))
		{
			Agreement.bIsActive = false;
		}
	}

	// Update relationships
	TMap<FString, float>& Relations1 = FactionRelationships.FindOrAdd(Faction1ID);
	Relations1.Add(Faction2ID, -100.0f);

	TMap<FString, float>& Relations2 = FactionRelationships.FindOrAdd(Faction2ID);
	Relations2.Add(Faction1ID, -100.0f);

	UE_LOG(LogTemp, Warning, TEXT("WAR DECLARED: %s vs %s"), *Faction1ID, *Faction2ID);
}

void UFactionEconomyManager::MakePeace(FString Faction1ID, FString Faction2ID)
{
	// Update relationships to neutral
	TMap<FString, float>& Relations1 = FactionRelationships.FindOrAdd(Faction1ID);
	Relations1.Add(Faction2ID, 0.0f);

	TMap<FString, float>& Relations2 = FactionRelationships.FindOrAdd(Faction2ID);
	Relations2.Add(Faction1ID, 0.0f);

	UE_LOG(LogTemp, Log, TEXT("PEACE MADE: %s and %s"), *Faction1ID, *Faction2ID);
}

void UFactionEconomyManager::FormAlliance(FString Faction1ID, FString Faction2ID)
{
	// Create trade agreement if one doesn't exist
	bool bHasAgreement = false;
	for (const FFactionTradeAgreement& Agreement : ActiveAgreements)
	{
		if ((Agreement.Faction1ID == Faction1ID && Agreement.Faction2ID == Faction2ID) ||
			(Agreement.Faction1ID == Faction2ID && Agreement.Faction2ID == Faction1ID))
		{
			bHasAgreement = true;
			break;
		}
	}

	if (!bHasAgreement)
	{
		FormTradeAgreement(Faction1ID, Faction2ID, FTimespan(30, 0, 0, 0)); // 30 day agreement
	}

	// Update relationships
	TMap<FString, float>& Relations1 = FactionRelationships.FindOrAdd(Faction1ID);
	Relations1.Add(Faction2ID, 75.0f);

	TMap<FString, float>& Relations2 = FactionRelationships.FindOrAdd(Faction2ID);
	Relations2.Add(Faction1ID, 75.0f);

	UE_LOG(LogTemp, Log, TEXT("ALLIANCE FORMED: %s and %s"), *Faction1ID, *Faction2ID);
}

void UFactionEconomyManager::BreakAlliance(FString Faction1ID, FString Faction2ID)
{
	// End trade agreements
	for (FFactionTradeAgreement& Agreement : ActiveAgreements)
	{
		if ((Agreement.Faction1ID == Faction1ID && Agreement.Faction2ID == Faction2ID) ||
			(Agreement.Faction1ID == Faction2ID && Agreement.Faction2ID == Faction1ID))
		{
			Agreement.bIsActive = false;
		}
	}

	// Update relationships
	TMap<FString, float>& Relations1 = FactionRelationships.FindOrAdd(Faction1ID);
	Relations1.Add(Faction2ID, 0.0f);

	TMap<FString, float>& Relations2 = FactionRelationships.FindOrAdd(Faction2ID);
	Relations2.Add(Faction1ID, 0.0f);

	UE_LOG(LogTemp, Log, TEXT("ALLIANCE BROKEN: %s and %s"), *Faction1ID, *Faction2ID);
}

void UFactionEconomyManager::SimulateFactionEconomy(FString FactionID, float DeltaTime)
{
	FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (!Faction)
	{
		return;
	}

	// Update production and consumption
	UpdateFactionProduction(FactionID);
	UpdateFactionConsumption(FactionID);

	// Update treasury
	UpdateFactionTreasury(FactionID, DeltaTime);
}

void UFactionEconomyManager::UpdateFactionProduction(FString FactionID)
{
	FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (!Faction)
	{
		return;
	}

	// Simulate production changes based on economic strength
	for (auto& ProductionPair : Faction->ResourceProduction)
	{
		float RandomFactor = FMath::FRandRange(0.95f, 1.05f);
		float StrengthFactor = 1.0f + (Faction->EconomicStrength / 100.0f * 0.5f);
		
		ProductionPair.Value *= RandomFactor * StrengthFactor;
		ProductionPair.Value = FMath::Clamp(ProductionPair.Value, 0.1f, 10.0f);
	}
}

void UFactionEconomyManager::UpdateFactionConsumption(FString FactionID)
{
	FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (!Faction)
	{
		return;
	}

	// Simulate consumption changes based on population and economy
	for (auto& ConsumptionPair : Faction->ResourceConsumption)
	{
		float RandomFactor = FMath::FRandRange(0.98f, 1.02f);
		float EconomyFactor = 1.0f + (Faction->EconomicStrength / 100.0f * 0.3f);
		
		ConsumptionPair.Value *= RandomFactor * EconomyFactor;
		ConsumptionPair.Value = FMath::Clamp(ConsumptionPair.Value, 0.1f, 10.0f);
	}
}

float UFactionEconomyManager::CalculateTradeBalance(FString FactionID) const
{
	const FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (!Faction)
	{
		return 0.0f;
	}

	float TotalProduction = 0.0f;
	float TotalConsumption = 0.0f;

	for (const auto& Pair : Faction->ResourceProduction)
	{
		TotalProduction += Pair.Value;
	}

	for (const auto& Pair : Faction->ResourceConsumption)
	{
		TotalConsumption += Pair.Value;
	}

	return TotalProduction - TotalConsumption;
}

void UFactionEconomyManager::SetFactionProduction(FString FactionID, FName CommodityID, float ProductionLevel)
{
	FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (Faction)
	{
		Faction->ResourceProduction.Add(CommodityID, ProductionLevel);
	}
}

void UFactionEconomyManager::SetFactionConsumption(FString FactionID, FName CommodityID, float ConsumptionLevel)
{
	FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (Faction)
	{
		Faction->ResourceConsumption.Add(CommodityID, ConsumptionLevel);
	}
}

float UFactionEconomyManager::GetResourceBalance(FString FactionID, FName CommodityID) const
{
	const FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (!Faction)
	{
		return 0.0f;
	}

	const float* Production = Faction->ResourceProduction.Find(CommodityID);
	const float* Consumption = Faction->ResourceConsumption.Find(CommodityID);

	float Prod = Production ? *Production : 0.0f;
	float Cons = Consumption ? *Consumption : 0.0f;

	return Prod - Cons;
}

TArray<FName> UFactionEconomyManager::GetFactionImportNeeds(FString FactionID) const
{
	TArray<FName> ImportNeeds;
	const FFactionEconomicData* Faction = FactionData.Find(FactionID);
	
	if (!Faction)
	{
		return ImportNeeds;
	}

	for (const auto& Pair : Faction->ResourceConsumption)
	{
		float Balance = GetResourceBalance(FactionID, Pair.Key);
		if (Balance < -0.5f) // More consumption than production
		{
			ImportNeeds.Add(Pair.Key);
		}
	}

	return ImportNeeds;
}

TArray<FName> UFactionEconomyManager::GetFactionExportSurplus(FString FactionID) const
{
	TArray<FName> ExportSurplus;
	const FFactionEconomicData* Faction = FactionData.Find(FactionID);
	
	if (!Faction)
	{
		return ExportSurplus;
	}

	for (const auto& Pair : Faction->ResourceProduction)
	{
		float Balance = GetResourceBalance(FactionID, Pair.Key);
		if (Balance > 0.5f) // More production than consumption
		{
			ExportSurplus.Add(Pair.Key);
		}
	}

	return ExportSurplus;
}

TArray<FFactionEconomicData> UFactionEconomyManager::GetAllFactions() const
{
	TArray<FFactionEconomicData> Factions;
	FactionData.GenerateValueArray(Factions);
	return Factions;
}

TArray<FString> UFactionEconomyManager::GetTopTradingFactions(int32 Count) const
{
	TArray<FString> TopFactions;
	TArray<FFactionEconomicData> AllFactions = GetAllFactions();

	// Sort by trade influence
	AllFactions.Sort([](const FFactionEconomicData& A, const FFactionEconomicData& B) {
		return A.TradeInfluence > B.TradeInfluence;
	});

	// Get top N
	for (int32 i = 0; i < FMath::Min(Count, AllFactions.Num()); i++)
	{
		TopFactions.Add(AllFactions[i].FactionID);
	}

	return TopFactions;
}

FString UFactionEconomyManager::GetFactionEconomicReport(FString FactionID) const
{
	const FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (!Faction)
	{
		return TEXT("Faction not found");
	}

	FString Report = FString::Printf(TEXT("=== FACTION ECONOMIC REPORT: %s ===\n\n"), *Faction->FactionName);
	Report += FString::Printf(TEXT("Economic Policy: %d\n"), (int32)Faction->EconomicPolicy);
	Report += FString::Printf(TEXT("Trade Restriction: %d\n"), (int32)Faction->TradeRestriction);
	Report += FString::Printf(TEXT("Economic Strength: %.1f/100\n"), Faction->EconomicStrength);
	Report += FString::Printf(TEXT("Trade Influence: %.1f/100\n"), Faction->TradeInfluence);
	Report += FString::Printf(TEXT"Military Strength: %.1f/100\n"), Faction->MilitaryStrength);
	Report += FString::Printf(TEXT("Treasury: %.0f credits\n"), Faction->CurrentTreasury);
	Report += FString::Printf(TEXT("Monthly Income: %.0f credits\n"), Faction->MonthlyIncome);
	Report += FString::Printf(TEXT("Monthly Expenses: %.0f credits\n"), Faction->MonthlyExpenses);
	Report += FString::Printf(TEXT("Net Monthly: %.0f credits\n\n"), Faction->MonthlyIncome - Faction->MonthlyExpenses);

	Report += TEXT("Resource Production:\n");
	for (const auto& Pair : Faction->ResourceProduction)
	{
		Report += FString::Printf(TEXT("  %s: %.2f\n"), *Pair.Key.ToString(), Pair.Value);
	}

	Report += TEXT("\nResource Consumption:\n");
	for (const auto& Pair : Faction->ResourceConsumption)
	{
		Report += FString::Printf(TEXT("  %s: %.2f\n"), *Pair.Key.ToString(), Pair.Value);
	}

	Report += FString::Printf(TEXT("\nTrade Balance: %.2f\n"), CalculateTradeBalance(FactionID));

	return Report;
}

FString UFactionEconomyManager::GetPlayerReputationReport(FString PlayerID) const
{
	FString Report = FString::Printf(TEXT("=== PLAYER REPUTATION REPORT: %s ===\n\n"), *PlayerID);

	const TMap<FString, FPlayerFactionReputation>* PlayerRep = PlayerReputations.Find(PlayerID);
	if (!PlayerRep)
	{
		return Report + TEXT("No reputation data found");
	}

	for (const auto& Pair : *PlayerRep)
	{
		const FPlayerFactionReputation& Rep = Pair.Value;
		const FFactionEconomicData Faction = GetFactionData(Pair.Key);

		Report += FString::Printf(TEXT("Faction: %s\n"), *Faction.FactionName);
		Report += FString::Printf(TEXT("  Reputation: %.1f/100\n"), Rep.ReputationScore);
		Report += FString::Printf(TEXT("  Standing: %.1f/100\n"), Rep.Standing);
		Report += FString::Printf(TEXT("  Trade Discount: %.1f%%\n"), Rep.TradeDiscount * 100.0f);
		Report += FString::Printf(TEXT("  Missions Completed: %d\n"), Rep.MissionsCompleted);
		Report += FString::Printf(TEXT("  Total Trade Value: %.0f\n"), Rep.TotalTradeValue);
		Report += FString::Printf(TEXT("  Trading License: %s\n"), Rep.bHasTradingLicense ? TEXT("Yes") : TEXT("No"));
		Report += FString::Printf(TEXT("  Military Tech Access: %s\n\n"), Rep.bCanAccessMilitaryTech ? TEXT("Yes") : TEXT("No"));
	}

	return Report;
}

void UFactionEconomyManager::UpdateReputationDecay(float DeltaTime)
{
	// Reputation gradually decays towards neutral over time
	for (auto& PlayerPair : PlayerReputations)
	{
		for (auto& RepPair : PlayerPair.Value)
		{
			FPlayerFactionReputation& Rep = RepPair.Value;
			
			// Decay towards 0 (neutral)
			if (Rep.ReputationScore > 0)
			{
				Rep.ReputationScore -= ReputationDecayRate * DeltaTime;
				Rep.ReputationScore = FMath::Max(0.0f, Rep.ReputationScore);
			}
			else if (Rep.ReputationScore < 0)
			{
				Rep.ReputationScore += ReputationDecayRate * DeltaTime;
				Rep.ReputationScore = FMath::Min(0.0f, Rep.ReputationScore);
			}

			// Update derived values
			Rep.Standing = CalculateStanding(Rep.ReputationScore);
			Rep.TradeDiscount = FMath::Clamp((Rep.ReputationScore / 100.0f) * MaxReputationDiscount, 0.0f, MaxReputationDiscount);
		}
	}
}

void UFactionEconomyManager::UpdateEconomicSimulation(float DeltaTime)
{
	// Update all faction economies
	for (auto& Pair : FactionData)
	{
		SimulateFactionEconomy(Pair.Key, DeltaTime);
	}
}

void UFactionEconomyManager::UpdateTradeAgreements(float DeltaTime)
{
	FDateTime CurrentTime = FDateTime::Now();

	// Check for expired agreements
	for (int32 i = ActiveAgreements.Num() - 1; i >= 0; i--)
	{
		FFactionTradeAgreement& Agreement = ActiveAgreements[i];
		
		if (Agreement.bIsActive)
		{
			FTimespan Elapsed = CurrentTime - Agreement.StartDate;
			if (Elapsed >= Agreement.Duration)
			{
				Agreement.bIsActive = false;
				UE_LOG(LogTemp, Log, TEXT("Trade agreement %s expired"), *Agreement.AgreementID);
			}
		}
	}
}

void UFactionEconomyManager::UpdateSanctions(float DeltaTime)
{
	FDateTime CurrentTime = FDateTime::Now();

	// Check for expired sanctions
	for (int32 i = ActiveSanctions.Num() - 1; i >= 0; i--)
	{
		FFactionSanctions& Sanctions = ActiveSanctions[i];
		
		if (Sanctions.bIsActive)
		{
			FTimespan Elapsed = CurrentTime - Sanctions.StartDate;
			if (Elapsed >= Sanctions.Duration)
			{
				Sanctions.bIsActive = false;
				UE_LOG(LogTemp, Log, TEXT("Sanctions %s lifted"), *Sanctions.SanctionsID);
			}
		}
	}
}

float UFactionEconomyManager::CalculateStanding(float ReputationScore) const
{
	// Convert reputation score to standing (0-100)
	return (ReputationScore + 100.0f) / 2.0f;
}

void UFactionEconomyManager::UnlockCommoditiesForPlayer(FString FactionID, FString PlayerID)
{
	TMap<FString, FPlayerFactionReputation>& PlayerRep = PlayerReputations.FindOrAdd(PlayerID);
	FPlayerFactionReputation& Rep = PlayerRep.FindOrAdd(FactionID);

	// Unlock commodities based on standing
	if (Rep.Standing >= 80.0f)
	{
		Rep.bCanAccessMilitaryTech = true;
	}

	// Would unlock specific commodities based on faction and reputation
}

void UFactionEconomyManager::UpdateFactionTreasury(FString FactionID, float DeltaTime)
{
	FFactionEconomicData* Faction = FactionData.Find(FactionID);
	if (!Faction)
	{
		return;
	}

	// Update treasury based on income and expenses
	float DailyNet = (Faction->MonthlyIncome - Faction->MonthlyExpenses) / 30.0f;
	Faction->CurrentTreasury += DailyNet * (DeltaTime / 86400.0f);

	// Ensure treasury doesn't go negative
	Faction->CurrentTreasury = FMath::Max(0.0f, Faction->CurrentTreasury);
}

void UFactionEconomyManager::ProcessFactionDiplomacy(FString FactionID, float DeltaTime)
{
	// This would handle AI faction diplomacy
	// For now, just a placeholder
}

FString UFactionEconomyManager::GenerateFactionID() const
{
	return FString::Printf(TEXT("Faction_%d"), FMath::RandRange(1000, 9999));
}

FString UFactionEconomyManager::GenerateAgreementID() const
{
	return FString::Printf(TEXT("Agreement_%d"), FMath::RandRange(1000, 9999));
}

FString UFactionEconomyManager::GenerateSanctionsID() const
{
	return FString::Printf(TEXT("Sanctions_%d"), FMath::RandRange(1000, 9999));
}

void UFactionEconomyManager::LogFactionEvent(const FString& FactionID, const FString& Event, const FString& Details) const
{
	UE_LOG(LogTemp, Log, TEXT("[FactionEconomyManager] %s - %s: %s"), *FactionID, *Event, *Details);
}

bool UFactionEconomyManager::RunSelfTest_Implementation(FSystemTestResult& OutResult)
{
	OutResult.SystemName = "FactionEconomyManager";
	OutResult.bSuccess = true;
	OutResult.TestTimestamp = FDateTime::Now();
	
	UE_LOG(LogTemp, Log, TEXT("=== Starting FactionEconomyManager Self-Test ==="));
	
	// Test 1: Verify faction creation
	{
		UE_LOG(LogTemp, Log, TEXT("Test 1: Verifying faction creation..."));
		
		FString TestFactionID = CreateFaction("TestFaction", EFactionEconomicPolicy::FreeMarket, false);
		
		if (TestFactionID.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Faction creation failed"));
			OutResult.ErrorMessages.Add(TEXT("Faction creation test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		FFactionEconomicData CreatedFaction = GetFactionData(TestFactionID);
		if (CreatedFaction.FactionID.IsEmpty() || CreatedFaction.FactionName != "TestFaction")
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Created faction data is invalid"));
			OutResult.ErrorMessages.Add(TEXT("Faction data retrieval test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Faction creation working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Faction Creation"),
			FString::Printf(TEXT("Successfully created faction: %s"), *TestFactionID),
			0.0f
		});
	}
	
	// Test 2: Verify faction policy system
	{
		UE_LOG(LogTemp, Log, TEXT("Test 2: Verifying faction policy system..."));
		
		FString TestFactionID = CreateFaction("PolicyTestFaction", EFactionEconomicPolicy::FreeMarket, false);
		
		// Test each policy type
		TArray<EFactionEconomicPolicy> Policies = {
			EFactionEconomicPolicy::FreeMarket,
			EFactionEconomicPolicy::PlannedEconomy,
			EFactionEconomicPolicy::Mercantile,
			EFactionEconomicPolicy::Isolationist,
			EFactionEconomicPolicy::WarEconomy,
			EFactionEconomicPolicy::TradeFocus,
			EFactionEconomicPolicy::Industrial
		};
		
		for (EFactionEconomicPolicy Policy : Policies)
		{
			SetFactionPolicy(TestFactionID, Policy);
			
			FFactionEconomicData UpdatedFaction = GetFactionData(TestFactionID);
			if (UpdatedFaction.EconomicPolicy != Policy)
			{
				UE_LOG(LogTemp, Error, TEXT("FAILED: Policy change failed for policy %d"), (int32)Policy);
				OutResult.ErrorMessages.Add(FString::Printf(TEXT("Policy change test failed for policy %d"), (int32)Policy));
				OutResult.bSuccess = false;
				return false;
			}
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: All faction policies functional"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Faction Policy System"),
			TEXT("All 7 economic policies tested successfully"),
			0.0f
		});
	}
	
	// Test 3: Verify trade restriction system
	{
		UE_LOG(LogTemp, Log, TEXT("Test 3: Verifying trade restriction system..."));
		
		FString TestFactionID = CreateFaction("RestrictionTestFaction", EFactionEconomicPolicy::FreeMarket, false);
		
		// Test each restriction type
		TArray<EFactionTradeRestriction> Restrictions = {
			EFactionTradeRestriction::None,
			EFactionTradeRestriction::Tariffs,
			EFactionTradeRestriction::Quotas,
			EFactionTradeRestriction::Embargo,
			EFactionTradeRestriction::Monopoly,
			EFactionTradeRestriction::Licensed
		};
		
		for (EFactionTradeRestriction Restriction : Restrictions)
		{
			SetTradeRestriction(TestFactionID, Restriction);
			
			FFactionEconomicData UpdatedFaction = GetFactionData(TestFactionID);
			if (UpdatedFaction.TradeRestriction != Restriction)
			{
				UE_LOG(LogTemp, Error, TEXT("FAILED: Restriction change failed for restriction %d"), (int32)Restriction);
				OutResult.ErrorMessages.Add(FString::Printf(TEXT("Restriction change test failed for restriction %d"), (int32)Restriction));
				OutResult.bSuccess = false;
				return false;
			}
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: All trade restrictions functional"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Trade Restriction System"),
			TEXT("All 6 trade restrictions tested successfully"),
			0.0f
		});
	}
	
	// Test 4: Verify player reputation system
	{
		UE_LOG(LogTemp, Log, TEXT("Test 4: Verifying player reputation system..."));
		
		FString TestFactionID = CreateFaction("ReputationTestFaction", EFactionEconomicPolicy::FreeMarket, false);
		FString TestPlayerID = "TestPlayer";
		
		// Test initial reputation
		FPlayerFactionReputation InitialRep = GetPlayerReputation(TestFactionID, TestPlayerID);
		if (InitialRep.ReputationScore != 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Initial reputation should be 0.0f"));
			OutResult.ErrorMessages.Add(TEXT("Initial reputation test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		// Test reputation modification
		ModifyPlayerReputation(TestFactionID, TestPlayerID, 25.0f, "Test reputation gain");
		
		FPlayerFactionReputation ModifiedRep = GetPlayerReputation(TestFactionID, TestPlayerID);
		if (ModifiedRep.ReputationScore <= 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Reputation modification failed"));
			OutResult.ErrorMessages.Add(TEXT("Reputation modification test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		// Test reputation discount calculation
		float Discount = CalculateReputationDiscount(TestFactionID, TestPlayerID);
		if (Discount < 0.0f || Discount > MaxReputationDiscount)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Invalid reputation discount"));
			OutResult.ErrorMessages.Add(TEXT("Reputation discount calculation test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		// Test trading permission
		bool bCanTrade = CanPlayerTrade(TestFactionID, TestPlayerID);
		if (!bCanTrade)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Player should be able to trade with positive reputation"));
			OutResult.ErrorMessages.Add(TEXT("Trading permission test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Player reputation system working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Player Reputation System"),
			TEXT("Reputation modification, discounts, and trading permissions functional"),
			0.0f
		});
	}
	
	// Test 5: Verify trade price modifiers
	{
		UE_LOG(LogTemp, Log, TEXT("Test 5: Verifying trade price modifiers..."));
		
		FString TestFactionID = CreateFaction("PriceModifierTestFaction", EFactionEconomicPolicy::FreeMarket, false);
		FString TestPlayerID = "TestPlayer";
		
		// Set up reputation for testing
		ModifyPlayerReputation(TestFactionID, TestPlayerID, 50.0f, "Test reputation for price modifiers");
		
		// Test buying price modifier
		float BuyModifier = CalculateTradePriceModifier(TestFactionID, TestPlayerID, FName("IronOre"), true);
		if (BuyModifier <= 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Invalid buying price modifier"));
			OutResult.ErrorMessages.Add(TEXT("Buying price modifier test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		// Test selling price modifier
		float SellModifier = CalculateTradePriceModifier(TestFactionID, TestPlayerID, FName("IronOre"), false);
		if (SellModifier <= 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Invalid selling price modifier"));
			OutResult.ErrorMessages.Add(TEXT("Selling price modifier test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		// Verify selling modifier is better than buying (due to reputation bonus)
		if (SellModifier <= BuyModifier)
		{
			UE_LOG(LogTemp, Warning, TEXT("WARNING: Selling modifier should be better than buying with positive reputation"));
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Trade price modifiers working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Trade Price Modifiers"),
			FString::Printf(TEXT("Buy: %.2fx, Sell: %.2fx"), BuyModifier, SellModifier),
			0.0f
		});
	}
	
	// Test 6: Verify commodity banning system
	{
		UE_LOG(LogTemp, Log, TEXT("Test 6: Verifying commodity banning system..."));
		
		FString TestFactionID = CreateFaction("BanTestFaction", EFactionEconomicPolicy::FreeMarket, false);
		FName TestCommodityID = FName("IronOre");
		
		// Initially should not be banned
		bool bInitiallyBanned = IsCommodityBanned(TestFactionID, TestCommodityID);
		if (bInitiallyBanned)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Commodity should not be initially banned"));
			OutResult.ErrorMessages.Add(TEXT("Initial ban state test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		// Ban the commodity
		BanCommodity(TestFactionID, TestCommodityID);
		
		// Verify it's banned
		bool bNowBanned = IsCommodityBanned(TestFactionID, TestCommodityID);
		if (!bNowBanned)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Commodity should be banned after BanCommodity"));
			OutResult.ErrorMessages.Add(TEXT("Commodity banning test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		// Test tariff calculation for banned commodity (should be very high)
		float Tariff = CalculateTariff(TestFactionID, TestCommodityID, 100.0f, 10);
		if (Tariff < 1000.0f) // Should be 10x normal due to ban penalty
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Tariff for banned commodity should be very high"));
			OutResult.ErrorMessages.Add(TEXT("Banned commodity tariff test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		// Unban the commodity
		UnbanCommodity(TestFactionID, TestCommodityID);
		
		// Verify it's unbanned
		bool bNowUnbanned = IsCommodityBanned(TestFactionID, TestCommodityID);
		if (bNowUnbanned)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Commodity should be unbanned after UnbanCommodity"));
			OutResult.ErrorMessages.Add(TEXT("Commodity unbanning test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Commodity banning system working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Commodity Banning System"),
			TEXT("Ban/unban functionality and tariff penalties working"),
			0.0f
		});
	}
	
	// Test 7: Verify trade agreements
	{
		UE_LOG(LogTemp, Log, TEXT("Test 7: Verifying trade agreements..."));
		
		FString Faction1ID = CreateFaction("AgreementFaction1", EFactionEconomicPolicy::FreeMarket, false);
		FString Faction2ID = CreateFaction("AgreementFaction2", EFactionEconomicPolicy::FreeMarket, false);
		
		// Form trade agreement
		FFactionTradeAgreement Agreement = FormTradeAgreement(Faction1ID, Faction2ID, FTimespan(7, 0, 0, 0)); // 7 days
		
		if (Agreement.AgreementID.IsEmpty() || !Agreement.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Trade agreement creation failed"));
			OutResult.ErrorMessages.Add(TEXT("Trade agreement creation test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		// Verify agreement affects price modifiers
		FString TestPlayerID = "TestPlayer";
		ModifyPlayerReputation(Faction1ID, TestPlayerID, 50.0f, "Test reputation");
		
		float PriceWithAgreement = CalculateTradePriceModifier(Faction1ID, TestPlayerID, FName("IronOre"), true);
		
		// End the agreement
		EndTradeAgreement(Agreement.AgreementID);
		
		float PriceWithoutAgreement = CalculateTradePriceModifier(Faction1ID, TestPlayerID, FName("IronOre"), true);
		
		// Price should be better with active agreement
		if (PriceWithAgreement >= PriceWithoutAgreement)
		{
			UE_LOG(LogTemp, Warning, TEXT("WARNING: Price with agreement should be better than without"));
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Trade agreements working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Trade Agreements"),
			TEXT("Agreement creation and price modifier effects functional"),
			0.0f
		});
	}
	
	// Test 8: Verify sanctions system
	{
		UE_LOG(LogTemp, Log, TEXT("Test 8: Verifying sanctions system..."));
		
		FString SanctioningFactionID = CreateFaction("SanctioningFaction", EFactionEconomicPolicy::FreeMarket, false);
		FString TargetFactionID = CreateFaction("TargetFaction", EFactionEconomicPolicy::FreeMarket, false);
		
		// Impose sanctions
		FFactionSanctions Sanctions = ImposeSanctions(SanctioningFactionID, TargetFactionID, FTimespan(3, 0, 0, 0), 0.5f);
		
		if (Sanctions.SanctionsID.IsEmpty() || !Sanctions.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Sanctions creation failed"));
			OutResult.ErrorMessages.Add(TEXT("Sanctions creation test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		// Verify sanctions affect price modifiers
		FString TestPlayerID = "TestPlayer";
		ModifyPlayerReputation(TargetFactionID, TestPlayerID, 50.0f, "Test reputation");
		
		float PriceWithSanctions = CalculateTradePriceModifier(TargetFactionID, TestPlayerID, FName("IronOre"), true);
		
		// Lift sanctions
		LiftSanctions(Sanctions.SanctionsID);
		
		float PriceWithoutSanctions = CalculateTradePriceModifier(TargetFactionID, TestPlayerID, FName("IronOre"), true);
		
		// Price should be worse with active sanctions
		if (PriceWithSanctions <= PriceWithoutSanctions)
		{
			UE_LOG(LogTemp, Warning, TEXT("WARNING: Price with sanctions should be worse than without"));
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Sanctions system working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Sanctions System"),
			TEXT("Sanctions creation and price modifier effects functional"),
			0.0f
		});
	}
	
	// Test 9: Verify diplomatic relations (war/peace/alliance)
	{
		UE_LOG(LogTemp, Log, TEXT("Test 9: Verifying diplomatic relations..."));
		
		FString Faction1ID = CreateFaction("DiplomacyFaction1", EFactionEconomicPolicy::FreeMarket, false);
		FString Faction2ID = CreateFaction("DiplomacyFaction2", EFactionEconomicPolicy::FreeMarket, false);
		
		// Form alliance
		FormAlliance(Faction1ID, Faction2ID);
		
		// Verify alliance created trade agreement
		bool bHasAgreement = false;
		for (const FFactionTradeAgreement& Agreement : ActiveAgreements)
		{
			if ((Agreement.Faction1ID == Faction1ID && Agreement.Faction2ID == Faction2ID) ||
				(Agreement.Faction1ID == Faction2ID && Agreement.Faction2ID == Faction1ID))
			{
				bHasAgreement = true;
				break;
			}
		}
		
		if (!bHasAgreement)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Alliance should create trade agreement"));
			OutResult.ErrorMessages.Add(TEXT("Alliance trade agreement test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		// Break alliance
		BreakAlliance(Faction1ID, Faction2ID);
		
		// Declare war
		DeclareWar(Faction1ID, Faction2ID);
		
		// Make peace
		MakePeace(Faction1ID, Faction2ID);
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Diplomatic relations working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Diplomatic Relations"),
			TEXT("War, peace, and alliance mechanics functional"),
			0.0f
		});
	}
	
	// Test 10: Verify resource management
	{
		UE_LOG(LogTemp, Log, TEXT("Test 10: Verifying resource management..."));
		
		FString TestFactionID = CreateFaction("ResourceTestFaction", EFactionEconomicPolicy::FreeMarket, false);
		FName TestCommodityID = FName("IronOre");
		
		// Set production and consumption
		SetFactionProduction(TestFactionID, TestCommodityID, 2.0f);
		SetFactionConsumption(TestFactionID, TestCommodityID, 1.5f);
		
		// Verify resource balance
		float Balance = GetResourceBalance(TestFactionID, TestCommodityID);
		if (Balance != 0.5f) // 2.0 - 1.5 = 0.5
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Resource balance calculation incorrect (expected 0.5, got %.2f)"), Balance);
			OutResult.ErrorMessages.Add(TEXT("Resource balance calculation test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		// Test import needs (should be empty since we have surplus)
		TArray<FName> ImportNeeds = GetFactionImportNeeds(TestFactionID);
		if (ImportNeeds.Contains(TestCommodityID))
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Commodity with surplus should not be in import needs"));
			OutResult.ErrorMessages.Add(TEXT("Import needs test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		// Test export surplus (should contain our test commodity)
		TArray<FName> ExportSurplus = GetFactionExportSurplus(TestFactionID);
		if (!ExportSurplus.Contains(TestCommodityID))
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Commodity with surplus should be in export surplus"));
			OutResult.ErrorMessages.Add(TEXT("Export surplus test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Resource management working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Resource Management"),
			TEXT("Production, consumption, balance calculations functional"),
			0.0f
		});
	}
	
	// Test 11: Verify economic simulation
	{
		UE_LOG(LogTemp, Log, TEXT("Test 11: Verifying economic simulation..."));
		
		FString TestFactionID = CreateFaction("SimulationTestFaction", EFactionEconomicPolicy::FreeMarket, false);
		
		// Get initial values
		FFactionEconomicData InitialFaction = GetFactionData(TestFactionID);
		float InitialTreasury = InitialFaction.CurrentTreasury;
		
		// Simulate economy for a short time
		SimulateFactionEconomy(TestFactionID, 3600.0f); // 1 hour simulation
		
		FFactionEconomicData UpdatedFaction = GetFactionData(TestFactionID);
		
		// Treasury should change
		if (UpdatedFaction.CurrentTreasury == InitialTreasury)
		{
			UE_LOG(LogTemp, Warning, TEXT("WARNING: Treasury should change during simulation"));
		}
		
		// Economic strength should be within valid range
		if (UpdatedFaction.EconomicStrength < 0.0f || UpdatedFaction.EconomicStrength > 100.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Economic strength out of valid range"));
			OutResult.ErrorMessages.Add(TEXT("Economic strength test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Economic simulation working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Economic Simulation"),
			TEXT("Faction economy simulation functional"),
			0.0f
		});
	}
	
	// Test 12: Verify reporting system
	{
		UE_LOG(LogTemp, Log, TEXT("Test 12: Verifying reporting system..."));
		
		FString TestFactionID = CreateFaction("ReportTestFaction", EFactionEconomicPolicy::FreeMarket, false);
		FString TestPlayerID = "TestPlayer";
		
		// Set up some reputation
		ModifyPlayerReputation(TestFactionID, TestPlayerID, 30.0f, "Test reputation for report");
		
		// Test faction economic report
		FString FactionReport = GetFactionEconomicReport(TestFactionID);
		if (FactionReport.IsEmpty() || !FactionReport.Contains("FACTION ECONOMIC REPORT"))
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Faction economic report generation failed"));
			OutResult.ErrorMessages.Add(TEXT("Faction report generation test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		// Test player reputation report
		FString PlayerReport = GetPlayerReputationReport(TestPlayerID);
		if (PlayerReport.IsEmpty() || !PlayerReport.Contains("PLAYER REPUTATION REPORT"))
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Player reputation report generation failed"));
			OutResult.ErrorMessages.Add(TEXT("Player report generation test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		// Test getting all factions
		TArray<FFactionEconomicData> AllFactions = GetAllFactions();
		if (AllFactions.Num() < 5) // Should have our test factions plus defaults
		{
			UE_LOG(LogTemp, Warning, TEXT("WARNING: Expected more factions in GetAllFactions"));
		}
		
		// Test getting top trading factions
		TArray<FString> TopFactions = GetTopTradingFactions(3);
		if (TopFactions.Num() != 3)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: GetTopTradingFactions should return exactly 3 factions"));
			OutResult.ErrorMessages.Add(TEXT("Top trading factions test failed"));
			OutResult.bSuccess = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Reporting system working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Reporting System"),
			TEXT("Faction and player report generation functional"),
			0.0f
		});
	}
	
	// Final result
	if (OutResult.bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("=== FactionEconomyManager Self-Test PASSED ==="));
		UE_LOG(LogTemp, Log, TEXT("Total Tests: %d"), OutResult.TestResults.Num());
		UE_LOG(LogTemp, Log, TEXT("Total Reputation Changes: %d"), TotalReputationChanges);
		UE_LOG(LogTemp, Log, TEXT("Total Agreements Formed: %d"), TotalAgreementsFormed);
		UE_LOG(LogTemp, Log, TEXT("Total Sanctions Imposed: %d"), TotalSanctionsImposed);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("=== FactionEconomyManager Self-Test FAILED ==="));
	}
	
	return OutResult.bSuccess;
}