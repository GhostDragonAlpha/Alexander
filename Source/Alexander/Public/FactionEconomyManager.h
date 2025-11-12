// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
#include "TradingEconomySystem.h"
#include "SystemSelfTestInterface.h"
#include "FactionEconomyManager.generated.h"

// Forward declarations
class UFactionTerritorySystem;

/**
 * Faction economic policies
 */
UENUM(BlueprintType)
enum class EFactionEconomicPolicy : uint8
{
	FreeMarket		UMETA(DisplayName = "Free Market"),
	PlannedEconomy	UMETA(DisplayName = "Planned Economy"),
	Mercantile		UMETA(DisplayName = "Mercantile"),
	Isolationist	UMETA(DisplayName = "Isolationist"),
	Expansionist	UMETA(DisplayName = "Expansionist"),
	WarEconomy		UMETA(DisplayName = "War Economy"),
	TradeFocus		UMETA(DisplayName = "Trade Focus"),
	Industrial		UMETA(DisplayName = "Industrial")
};

/**
 * Faction trade restrictions
 */
UENUM(BlueprintType)
enum class EFactionTradeRestriction : uint8
{
	None			UMETA(DisplayName = "No Restrictions"),
	Tariffs			UMETA(DisplayName = "Tariffs"),
	Quotas			UMETA(DisplayName = "Quotas"),
	Embargo			UMETA(DisplayName = "Embargo"),
	Monopoly		UMETA(DisplayName = "State Monopoly"),
	Licensed		UMETA(DisplayName = "Licensed Trade Only")
};

/**
 * Faction economic data
 */
USTRUCT(BlueprintType)
struct FFactionEconomicData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	FString FactionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	FString FactionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	EFactionEconomicPolicy EconomicPolicy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	EFactionTradeRestriction TradeRestriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	float EconomicStrength; // 0-100

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	float TradeInfluence; // 0-100

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	float MilitaryStrength; // 0-100

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	TMap<FName, float> ResourceProduction; // Commodity -> Production level

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	TMap<FName, float> ResourceConsumption; // Commodity -> Consumption level

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	float BasePriceModifier; // Faction-wide price adjustment

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	float TariffRate; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	TMap<FName, bool> TradeBans; // Commodity -> Is banned

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	TArray<FString> ControlledStations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	TArray<FString> AlliedFactions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	TArray<FString> HostileFactions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	float CurrentTreasury;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	float MonthlyIncome;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	float MonthlyExpenses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	bool bIsPlayerFaction;

	FFactionEconomicData()
	{
		EconomicPolicy = EFactionEconomicPolicy::FreeMarket;
		TradeRestriction = EFactionTradeRestriction::None;
		EconomicStrength = 50.0f;
		TradeInfluence = 50.0f;
		MilitaryStrength = 50.0f;
		BasePriceModifier = 1.0f;
		TariffRate = 0.0f;
		CurrentTreasury = 100000.0f;
		MonthlyIncome = 10000.0f;
		MonthlyExpenses = 8000.0f;
		bIsPlayerFaction = false;
	}
};

/**
 * Player faction reputation
 */
USTRUCT(BlueprintType)
struct FPlayerFactionReputation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	FString FactionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	float ReputationScore; // -100 to +100

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	float TradeDiscount; // 0-0.3 (30% max)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	float Standing; // 0-100 (derived from reputation)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	int32 MissionsCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	float TotalTradeValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	FTimespan TimeSinceLastTrade;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	TArray<FName> UnlockedCommodities; // Commodities player can trade

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	bool bHasTradingLicense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	bool bCanAccessMilitaryTech;

	FPlayerFactionReputation()
	{
		ReputationScore = 0.0f;
		TradeDiscount = 0.0f;
		Standing = 50.0f;
		MissionsCompleted = 0;
		TotalTradeValue = 0.0f;
		bHasTradingLicense = false;
		bCanAccessMilitaryTech = false;
	}
};

/**
 * Faction trade agreement
 */
USTRUCT(BlueprintType)
struct FFactionTradeAgreement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agreement")
	FString AgreementID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agreement")
	FString Faction1ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agreement")
	FString Faction2ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agreement")
	FTimespan Duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agreement")
	FDateTime StartDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agreement")
	float TradeBonus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agreement")
	bool bIsActive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agreement")
	TArray<FName> PreferredCommodities;

	FFactionTradeAgreement()
	{
		TradeBonus = 0.1f; // 10% bonus
		bIsActive = true;
	}
};

/**
 * Faction trade sanctions
 */
USTRUCT(BlueprintType)
struct FFactionSanctions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanctions")
	FString SanctioningFactionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanctions")
	FString TargetFactionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanctions")
	FTimespan Duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanctions")
	FDateTime StartDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanctions")
	float TradePenalty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanctions")
	TArray<FName> BannedCommodities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanctions")
	bool bIsActive;

	FFactionSanctions()
	{
		TradePenalty = 0.5f; // 50% penalty
		bIsActive = true;
	}
};

// Wrapper structs for complex template containers (UHT compatibility)
USTRUCT(BlueprintType)
struct FPlayerReputationMap
{
	GENERATED_BODY()
	UPROPERTY()
	TMap<FString, FPlayerFactionReputation> ReputationData;
};

USTRUCT(BlueprintType)
struct FTradeRouteArray
{
	GENERATED_BODY()
	UPROPERTY()
	TArray<FTradeRoute> Routes;
};

USTRUCT(BlueprintType)
struct FFactionInt32ValueMap
{
	GENERATED_BODY()
	UPROPERTY()
	TMap<FName, int32> Values;
};

USTRUCT(BlueprintType)
struct FFactionRelationshipMap
{
	GENERATED_BODY()
	UPROPERTY()
	TMap<FString, float> Relationships;
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFactionReputationChanged, const FString&, FactionID, float, NewReputation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFactionPolicyChanged, const FString&, FactionID, EFactionEconomicPolicy, NewPolicy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTradeAgreementFormed, const FFactionTradeAgreement&, Agreement);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSanctionsImposed, const FFactionSanctions&, Sanctions);

/**
 * Faction Economy Manager - Handles faction-controlled markets, reputation, and diplomatic trade relations
 */
UCLASS()
class ALEXANDER_API UFactionEconomyManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFactionEconomyManager();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tick function
	virtual void Tick(float DeltaTime);
	virtual bool IsTickable() const { return true; }

	// ============================================================================
	// FACTION MANAGEMENT
	// ============================================================================

	// Create faction
	UFUNCTION(BlueprintCallable, Category = "Faction|Economy")
	FString CreateFaction(
		FString FactionName,
		EFactionEconomicPolicy InitialPolicy,
		bool bIsPlayerFaction = false
	);

	// Get faction data
	UFUNCTION(BlueprintCallable, Category = "Faction|Economy")
	FFactionEconomicData GetFactionData(FString FactionID) const;

	// Update faction economy
	UFUNCTION(BlueprintCallable, Category = "Faction|Economy")
	void UpdateFactionEconomy(FString FactionID, float DeltaTime);

	// Set faction policy
	UFUNCTION(BlueprintCallable, Category = "Faction|Economy")
	void SetFactionPolicy(FString FactionID, EFactionEconomicPolicy NewPolicy);

	// Set trade restriction
	UFUNCTION(BlueprintCallable, Category = "Faction|Economy")
	void SetTradeRestriction(FString FactionID, EFactionTradeRestriction Restriction);

	// Add controlled station
	UFUNCTION(BlueprintCallable, Category = "Faction|Economy")
	void AddControlledStation(FString FactionID, FString StationID);

	// Remove controlled station
	UFUNCTION(BlueprintCallable, Category = "Faction|Economy")
	void RemoveControlledStation(FString FactionID, FString StationID);

	// ============================================================================
	// REPUTATION SYSTEM
	// ============================================================================

	// Get player reputation with faction
	UFUNCTION(BlueprintCallable, Category = "Faction|Reputation")
	FPlayerFactionReputation GetPlayerReputation(FString FactionID, FString PlayerID) const;

	// Modify player reputation
	UFUNCTION(BlueprintCallable, Category = "Faction|Reputation")
	void ModifyPlayerReputation(
		FString FactionID,
		FString PlayerID,
		float ReputationChange,
		FString Reason
	);

	// Calculate reputation discount
	UFUNCTION(BlueprintCallable, Category = "Faction|Reputation")
	float CalculateReputationDiscount(FString FactionID, FString PlayerID) const;

	// Can player trade with faction?
	UFUNCTION(BlueprintCallable, Category = "Faction|Reputation")
	bool CanPlayerTrade(FString FactionID, FString PlayerID) const;

	// Grant trading license
	UFUNCTION(BlueprintCallable, Category = "Faction|Reputation")
	void GrantTradingLicense(FString FactionID, FString PlayerID);

	// Revoke trading license
	UFUNCTION(BlueprintCallable, Category = "Faction|Reputation")
	void RevokeTradingLicense(FString FactionID, FString PlayerID);

	// ============================================================================
	// TRADE MODIFIERS
	// ============================================================================

	// Calculate price modifier for trade
	UFUNCTION(BlueprintCallable, Category = "Faction|Trade")
	float CalculateTradePriceModifier(
		FString FactionID,
		FString PlayerID,
		FName CommodityID,
		bool bIsBuying
	) const;

	// Calculate tariff cost
	UFUNCTION(BlueprintCallable, Category = "Faction|Trade")
	float CalculateTariff(
		FString FactionID,
		FName CommodityID,
		float BasePrice,
		int32 Quantity
	) const;

	// Is commodity banned?
	UFUNCTION(BlueprintCallable, Category = "Faction|Trade")
	bool IsCommodityBanned(FString FactionID, FName CommodityID) const;

	// Ban commodity
	UFUNCTION(BlueprintCallable, Category = "Faction|Trade")
	void BanCommodity(FString FactionID, FName CommodityID);

	// Unban commodity
	UFUNCTION(BlueprintCallable, Category = "Faction|Trade")
	void UnbanCommodity(FString FactionID, FName CommodityID);

	// ============================================================================
	// DIPLOMATIC RELATIONS
	// ============================================================================

	// Form trade agreement
	UFUNCTION(BlueprintCallable, Category = "Faction|Diplomacy")
	FFactionTradeAgreement FormTradeAgreement(
		FString Faction1ID,
		FString Faction2ID,
		FTimespan Duration
	);

	// End trade agreement
	UFUNCTION(BlueprintCallable, Category = "Faction|Diplomacy")
	void EndTradeAgreement(FString AgreementID);

	// Impose sanctions
	UFUNCTION(BlueprintCallable, Category = "Faction|Diplomacy")
	FFactionSanctions ImposeSanctions(
		FString SanctioningFactionID,
		FString TargetFactionID,
		FTimespan Duration,
		float Penalty
	);

	// Lift sanctions
	UFUNCTION(BlueprintCallable, Category = "Faction|Diplomacy")
	void LiftSanctions(FString SanctionsID);

	// Declare war (affects trade)
	UFUNCTION(BlueprintCallable, Category = "Faction|Diplomacy")
	void DeclareWar(FString Faction1ID, FString Faction2ID);

	// Make peace
	UFUNCTION(BlueprintCallable, Category = "Faction|Diplomacy")
	void MakePeace(FString Faction1ID, FString Faction2ID);

	// Form alliance
	UFUNCTION(BlueprintCallable, Category = "Faction|Diplomacy")
	void FormAlliance(FString Faction1ID, FString Faction2ID);

	// Break alliance
	UFUNCTION(BlueprintCallable, Category = "Faction|Diplomacy")
	void BreakAlliance(FString Faction1ID, FString Faction2ID);

	// ============================================================================
	// ECONOMIC SIMULATION
	// ============================================================================

	// Simulate faction economy
	UFUNCTION(BlueprintCallable, Category = "Faction|Simulation")
	void SimulateFactionEconomy(FString FactionID, float DeltaTime);

	// Update faction production
	UFUNCTION(BlueprintCallable, Category = "Faction|Simulation")
	void UpdateFactionProduction(FString FactionID);

	// Update faction consumption
	UFUNCTION(BlueprintCallable, Category = "Faction|Simulation")
	void UpdateFactionConsumption(FString FactionID);

	// Calculate faction trade balance
	UFUNCTION(BlueprintCallable, Category = "Faction|Simulation")
	float CalculateTradeBalance(FString FactionID) const;

	// ============================================================================
	// RESOURCE MANAGEMENT
	// ============================================================================

	// Set faction production level
	UFUNCTION(BlueprintCallable, Category = "Faction|Resources")
	void SetFactionProduction(FString FactionID, FName CommodityID, float ProductionLevel);

	// Set faction consumption level
	UFUNCTION(BlueprintCallable, Category = "Faction|Resources")
	void SetFactionConsumption(FString FactionID, FName CommodityID, float ConsumptionLevel);

	// Get faction resource surplus/deficit
	UFUNCTION(BlueprintCallable, Category = "Faction|Resources")
	float GetResourceBalance(FString FactionID, FName CommodityID) const;

	// Get faction import needs
	UFUNCTION(BlueprintCallable, Category = "Faction|Resources")
	TArray<FName> GetFactionImportNeeds(FString FactionID) const;

	// Get faction export surplus
	UFUNCTION(BlueprintCallable, Category = "Faction|Resources")
	TArray<FName> GetFactionExportSurplus(FString FactionID) const;

	// ============================================================================
	// STATISTICS & REPORTING
	// ============================================================================

	// Get all factions
	UFUNCTION(BlueprintCallable, Category = "Faction|Statistics")
	TArray<FFactionEconomicData> GetAllFactions() const;

	// Get top trading factions
	UFUNCTION(BlueprintCallable, Category = "Faction|Statistics")
	TArray<FString> GetTopTradingFactions(int32 Count) const;

	// Get faction economic report
	UFUNCTION(BlueprintCallable, Category = "Faction|Statistics")
	FString GetFactionEconomicReport(FString FactionID) const;

	// Get player reputation report
	UFUNCTION(BlueprintCallable, Category = "Faction|Statistics")
	FString GetPlayerReputationReport(FString PlayerID) const;

	// ============================================================================
	// EVENTS
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Faction Events")
	FOnFactionReputationChanged OnFactionReputationChanged;

	UPROPERTY(BlueprintAssignable, Category = "Faction Events")
	FOnFactionPolicyChanged OnFactionPolicyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Faction Events")
	FOnTradeAgreementFormed OnTradeAgreementFormed;

	UPROPERTY(BlueprintAssignable, Category = "Faction Events")
	FOnSanctionsImposed OnSanctionsImposed;

protected:
	// Faction territory system reference
	UPROPERTY()
	TWeakObjectPtr<UFactionTerritorySystem> FactionTerritorySystem;

	// Faction data
	UPROPERTY()
	TMap<FString, FFactionEconomicData> FactionData;

	// Player reputations: PlayerID -> FactionID -> ReputationData
	UPROPERTY()
	TMap<FString, FPlayerReputationMap> PlayerReputations;

	// Active trade agreements
	UPROPERTY()
	TArray<FFactionTradeAgreement> ActiveAgreements;

	// Active sanctions
	UPROPERTY()
	TArray<FFactionSanctions> ActiveSanctions;

	// Faction relationships: FactionID1 -> FactionID2 -> RelationshipScore
	UPROPERTY()
	TMap<FString, FFactionRelationshipMap> FactionRelationships;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float ReputationDecayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MaxReputationDiscount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float BaseTariffRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float TradeAgreementBonus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float SanctionPenalty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float ReputationPerMission;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float ReputationPerTradeValue;

private:
	// Update timers
	float ReputationUpdateTimer;
	float EconomicSimulationTimer;

	// Statistics
	int32 TotalReputationChanges;
	int32 TotalAgreementsFormed;
	int32 TotalSanctionsImposed;

	// Helper functions
	void UpdateReputationDecay(float DeltaTime);
	void UpdateEconomicSimulation(float DeltaTime);
	void UpdateTradeAgreements(float DeltaTime);
	void UpdateSanctions(float DeltaTime);
	float CalculateStanding(float ReputationScore) const;
	void UnlockCommoditiesForPlayer(FString FactionID, FString PlayerID);
	void UpdateFactionTreasury(FString FactionID, float DeltaTime);
	void ProcessFactionDiplomacy(FString FactionID, float DeltaTime);
	FString GenerateFactionID() const;
	FString GenerateAgreementID() const;
	FString GenerateSanctionsID() const;
	void LogFactionEvent(const FString& FactionID, const FString& Event, const FString& Details) const;
};