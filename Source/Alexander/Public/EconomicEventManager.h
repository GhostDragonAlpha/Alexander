// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
#include "TradingEconomySystem.h"
#include "SystemSelfTestInterface.h"
#include "EconomicEventManager.generated.h"

// Forward declarations
class UDynamicMarketManager;
class UFactionEconomyManager;

/**
 * Economic event categories
 */
UENUM(BlueprintType)
enum class EEconomicEventCategory : uint8
{
	Market			UMETA(DisplayName = "Market Event"),
	Faction			UMETA(DisplayName = "Faction Event"),
	Global			UMETA(DisplayName = "Global Event"),
	Seasonal		UMETA(DisplayName = "Seasonal Event"),
	Random			UMETA(DisplayName = "Random Event"),
	PlayerTriggered	UMETA(DisplayName = "Player Triggered")
};

/**
 * Economic event scope
 */
UENUM(BlueprintType)
enum class EEconomicEventScope : uint8
{
	Local			UMETA(DisplayName = "Local"),
	Regional		UMETA(DisplayName = "Regional"),
	Global			UMETA(DisplayName = "Global"),
	Universal		UMETA(DisplayName = "Universal")
};

/**
 * Economic event impact type
 */
UENUM(BlueprintType)
enum class EEconomicImpactType : uint8
{
	Positive		UMETA(DisplayName = "Positive"),
	Negative		UMETA(DisplayName = "Negative"),
	Mixed			UMETA(DisplayName = "Mixed"),
	Neutral			UMETA(DisplayName = "Neutral")
};

/**
 * Economic event data
 */
USTRUCT(BlueprintType)
struct FEconomicEventData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString EventID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString EventName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	EEconomicEventCategory Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	EEconomicEventScope Scope;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	EEconomicImpactType ImpactType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	float Severity; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	float DurationDays;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FDateTime EndTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	bool bIsActive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TMap<FName, float> CommodityPriceMultipliers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TMap<FName, float> CommodityDemandMultipliers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TMap<FName, float> CommoditySupplyMultipliers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TMap<FString, float> FactionImpactMultipliers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString TriggeringFactionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString TargetFactionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	bool bCanBeTriggeredByPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	float PlayerTriggerChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TMap<FString, FString> EventParameters;

	FEconomicEventData()
	{
		Category = EEconomicEventCategory::Random;
		Scope = EEconomicEventScope::Local;
		ImpactType = EEconomicImpactType::Mixed;
		Severity = 0.5f;
		DurationDays = 7.0f;
		bIsActive = false;
		bCanBeTriggeredByPlayer = false;
		PlayerTriggerChance = 0.1f;
	}
};

/**
 * Event chain for sequential events
 */
USTRUCT(BlueprintType)
struct FEventChain
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
	FString ChainID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
	FString ChainName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
	TArray<FEconomicEventData> Events;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
	float ChainDurationDays;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
	bool bIsSequential;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
	FTimespan DelayBetweenEvents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
	bool bIsActive;

	FEventChain()
	{
		ChainDurationDays = 30.0f;
		bIsSequential = true;
		DelayBetweenEvents = FTimespan(1, 0, 0, 0); // 1 day
		bIsActive = false;
	}
};

/**
 * Event history entry
 */
USTRUCT(BlueprintType)
struct FEventHistoryEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	FString EventID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	FDateTime TriggerTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	FDateTime EndTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	float ActualSeverity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	FString TriggeringPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	TMap<FString, float> ActualImpacts;

	FEventHistoryEntry()
	{
		ActualSeverity = 0.0f;
	}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEconomicEventStarted, const FEconomicEventData&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEconomicEventEnded, const FEconomicEventData&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventChainStarted, const FEventChain&, Chain);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventChainEnded, const FEventChain&, Chain);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerTriggeredEvent, const FEconomicEventData&, Event, const FString&, PlayerID);

/**
 * Economic Event Manager - Handles random and scripted economic events, event chains, and global economic simulation
 */
UCLASS()
class ALEXANDER_API UEconomicEventManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UEconomicEventManager();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tick function
	virtual void Tick(float DeltaTime);
	virtual bool IsTickable() const { return true; }

	// ============================================================================
	// EVENT MANAGEMENT
	// ============================================================================

	// Trigger economic event
	UFUNCTION(BlueprintCallable, Category = "Events")
	FEconomicEventData TriggerEvent(
		FString EventID,
		EEconomicEventCategory Category,
		float Severity,
		float DurationDays,
		FString TriggeringFactionID = ""
	);

	// End economic event
	UFUNCTION(BlueprintCallable, Category = "Events")
	void EndEvent(FString EventID);

	// Get active events
	UFUNCTION(BlueprintCallable, Category = "Events")
	TArray<FEconomicEventData> GetActiveEvents() const;

	// Get event by ID
	UFUNCTION(BlueprintCallable, Category = "Events")
	FEconomicEventData GetEvent(FString EventID) const;

	// Get events by category
	UFUNCTION(BlueprintCallable, Category = "Events")
	TArray<FEconomicEventData> GetEventsByCategory(EEconomicEventCategory Category) const;

	// Get events by scope
	UFUNCTION(BlueprintCallable, Category = "Events")
	TArray<FEconomicEventData> GetEventsByScope(EEconomicEventScope Scope) const;

	// Clear all events
	UFUNCTION(BlueprintCallable, Category = "Events")
	void ClearAllEvents();

	// ============================================================================
	// EVENT CHAINS
	// ============================================================================

	// Start event chain
	UFUNCTION(BlueprintCallable, Category = "Events|Chains")
	FEventChain StartEventChain(FString ChainID);

	// End event chain
	UFUNCTION(BlueprintCallable, Category = "Events|Chains")
	void EndEventChain(FString ChainID);

	// Get active event chains
	UFUNCTION(BlueprintCallable, Category = "Events|Chains")
	TArray<FEventChain> GetActiveEventChains() const;

	// Create event chain
	UFUNCTION(BlueprintCallable, Category = "Events|Chains")
	FEventChain CreateEventChain(
		FString ChainName,
		TArray<FEconomicEventData> Events,
		bool bIsSequential,
		FTimespan DelayBetweenEvents
	);

	// Create event chain with default delay (helper function)
	UFUNCTION(BlueprintCallable, Category = "Events|Chains", meta = (DisplayName = "Create Event Chain (Default Delay)"))
	FEventChain CreateEventChainWithDefaultDelay(
		FString ChainName,
		TArray<FEconomicEventData> Events,
		bool bIsSequential = true
	);

	// ============================================================================
	// RANDOM EVENTS
	// ============================================================================

	// Generate random event
	UFUNCTION(BlueprintCallable, Category = "Events|Random")
	FEconomicEventData GenerateRandomEvent(
		EEconomicEventCategory Category,
		EEconomicEventScope Scope
	);

	// Generate random event chain
	UFUNCTION(BlueprintCallable, Category = "Events|Random")
	FEventChain GenerateRandomEventChain(EEconomicEventScope Scope);

	// Set random event chance
	UFUNCTION(BlueprintCallable, Category = "Events|Random")
	void SetRandomEventChance(float Chance);

	// Get random event chance
	UFUNCTION(BlueprintCallable, Category = "Events|Random")
	float GetRandomEventChance() const;

	// ============================================================================
	// PLAYER TRIGGERED EVENTS
	// ============================================================================

	// Player attempts to trigger event
	UFUNCTION(BlueprintCallable, Category = "Events|Player")
	bool PlayerTriggerEvent(FString EventID, FString PlayerID);

	// Get player triggerable events
	UFUNCTION(BlueprintCallable, Category = "Events|Player")
	TArray<FEconomicEventData> GetPlayerTriggerableEvents() const;

	// Set player trigger chance
	UFUNCTION(BlueprintCallable, Category = "Events|Player")
	void SetPlayerTriggerChance(float Chance);

	// ============================================================================
	// MARKET EVENTS
	// ============================================================================

	// Trigger market crash
	UFUNCTION(BlueprintCallable, Category = "Events|Market")
	FEconomicEventData TriggerMarketCrash(
		ECommodityCategory Category,
		float Severity,
		float DurationDays
	);

	// Trigger market boom
	UFUNCTION(BlueprintCallable, Category = "Events|Market")
	FEconomicEventData TriggerMarketBoom(
		ECommodityCategory Category,
		float Magnitude,
		float DurationDays
	);

	// Trigger supply shortage
	UFUNCTION(BlueprintCallable, Category = "Events|Market")
	FEconomicEventData TriggerSupplyShortage(
		FName CommodityID,
		float Severity,
		float DurationDays
	);

	// Trigger supply glut
	UFUNCTION(BlueprintCallable, Category = "Events|Market")
	FEconomicEventData TriggerSupplyGlut(
		FName CommodityID,
		float Severity,
		float DurationDays
	);

	// Trigger trade war
	UFUNCTION(BlueprintCallable, Category = "Events|Market")
	FEconomicEventData TriggerTradeWar(
		FString Faction1ID,
		FString Faction2ID,
		float Severity,
		float DurationDays
	);

	// Trigger pirate blockade
	UFUNCTION(BlueprintCallable, Category = "Events|Market")
	FEconomicEventData TriggerPirateBlockade(
		FString TargetStationID,
		float Severity,
		float DurationDays
	);

	// ============================================================================
	// FACTION EVENTS
	// ============================================================================

	// Trigger faction economic boom
	UFUNCTION(BlueprintCallable, Category = "Events|Faction")
	FEconomicEventData TriggerFactionEconomicBoom(
		FString FactionID,
		float Magnitude,
		float DurationDays
	);

	// Trigger faction economic crisis
	UFUNCTION(BlueprintCallable, Category = "Events|Faction")
	FEconomicEventData TriggerFactionEconomicCrisis(
		FString FactionID,
		float Severity,
		float DurationDays
	);

	// Trigger faction civil war
	UFUNCTION(BlueprintCallable, Category = "Events|Faction")
	FEconomicEventData TriggerFactionCivilWar(
		FString FactionID,
		float Severity,
		float DurationDays
	);

	// Trigger faction revolution
	UFUNCTION(BlueprintCallable, Category = "Events|Faction")
	FEconomicEventData TriggerFactionRevolution(
		FString FactionID,
		float Severity,
		float DurationDays
	);

	// ============================================================================
	// SEASONAL EVENTS
	// ============================================================================

	// Trigger seasonal festival
	UFUNCTION(BlueprintCallable, Category = "Events|Seasonal")
	FEconomicEventData TriggerSeasonalFestival(
		FString StationID,
		float Magnitude,
		float DurationDays
	);

	// Trigger harvest season
	UFUNCTION(BlueprintCallable, Category = "Events|Seasonal")
	FEconomicEventData TriggerHarvestSeason(
		float Magnitude,
		float DurationDays
	);

	// Trigger holiday season
	UFUNCTION(BlueprintCallable, Category = "Events|Seasonal")
	FEconomicEventData TriggerHolidaySeason(
		float Magnitude,
		float DurationDays
	);

	// ============================================================================
	// GLOBAL EVENTS
	// ============================================================================

	// Trigger galactic economic boom
	UFUNCTION(BlueprintCallable, Category = "Events|Global")
	FEconomicEventData TriggerGalacticEconomicBoom(
		float Magnitude,
		float DurationDays
	);

	// Trigger galactic recession
	UFUNCTION(BlueprintCallable, Category = "Events|Global")
	FEconomicEventData TriggerGalacticRecession(
		float Severity,
		float DurationDays
	);

	// Trigger technological revolution
	UFUNCTION(BlueprintCallable, Category = "Events|Global")
	FEconomicEventData TriggerTechRevolution(
		float Magnitude,
		float DurationDays
	);

	// Trigger resource discovery
	UFUNCTION(BlueprintCallable, Category = "Events|Global")
	FEconomicEventData TriggerResourceDiscovery(
		FName CommodityID,
		float Magnitude,
		float DurationDays
	);

	// ============================================================================
	// EVENT HISTORY
	// ============================================================================

	// Get event history
	UFUNCTION(BlueprintCallable, Category = "Events|History")
	TArray<FEventHistoryEntry> GetEventHistory(int32 Count) const;

	// Get events in date range
	UFUNCTION(BlueprintCallable, Category = "Events|History")
	TArray<FEventHistoryEntry> GetEventsInDateRange(
		FDateTime StartDate,
		FDateTime EndDate
	) const;

	// Clear event history
	UFUNCTION(BlueprintCallable, Category = "Events|History")
	void ClearEventHistory();

	// Export event history
	UFUNCTION(BlueprintCallable, Category = "Events|History")
	FString ExportEventHistory() const;

	// ============================================================================
	// EVENT STATISTICS
	// ============================================================================

	// Get event statistics
	UFUNCTION(BlueprintCallable, Category = "Events|Statistics")
	TMap<EEconomicEventCategory, int32> GetEventStatistics() const;

	// Get average event severity
	UFUNCTION(BlueprintCallable, Category = "Events|Statistics")
	float GetAverageEventSeverity() const;

	// Get most common event type
	UFUNCTION(BlueprintCallable, Category = "Events|Statistics")
	EEconomicEventCategory GetMostCommonEventType() const;

	// Generate event report
	UFUNCTION(BlueprintCallable, Category = "Events|Statistics")
	FString GenerateEventReport() const;

	// ============================================================================
	// EVENT TEMPLATES
	// ============================================================================

	// Load event templates
	UFUNCTION(BlueprintCallable, Category = "Events|Templates")
	void LoadEventTemplates(UDataTable* TemplateTable);

	// Initialize default events
	UFUNCTION(BlueprintCallable, Category = "Events|Templates")
	void InitializeDefaultEvents();

	// ============================================================================
	// EVENTS
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Economic Events")
	FOnEconomicEventStarted OnEconomicEventStarted;

	UPROPERTY(BlueprintAssignable, Category = "Economic Events")
	FOnEconomicEventEnded OnEconomicEventEnded;

	UPROPERTY(BlueprintAssignable, Category = "Economic Events")
	FOnEventChainStarted OnEventChainStarted;

	UPROPERTY(BlueprintAssignable, Category = "Economic Events")
	FOnEventChainEnded OnEventChainEnded;

	UPROPERTY(BlueprintAssignable, Category = "Economic Events")
	FOnPlayerTriggeredEvent OnPlayerTriggeredEvent;

protected:
	// Dynamic market manager reference
	UPROPERTY()
	TWeakObjectPtr<UDynamicMarketManager> DynamicMarketManager;

	// Faction economy manager reference
	UPROPERTY()
	TWeakObjectPtr<UFactionEconomyManager> FactionEconomyManager;

	// Active events
	UPROPERTY()
	TArray<FEconomicEventData> ActiveEvents;

	// Event history
	UPROPERTY()
	TArray<FEventHistoryEntry> EventHistory;

	// Event chains
	UPROPERTY()
	TArray<FEventChain> ActiveEventChains;

	// Event templates
	UPROPERTY()
	TMap<FString, FEconomicEventData> EventTemplates;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float RandomEventCheckInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float BaseRandomEventChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MaxEventSeverity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MinEventDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MaxEventDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 MaxActiveEvents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bEnableRandomEvents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bEnableEventChains;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bShowDebugMessages;

private:
	// Update timers
	float RandomEventTimer;
	float EventChainTimer;

	// Statistics
	int32 TotalEventsTriggered;
	int32 TotalEventsEnded;
	int32 TotalEventChainsStarted;
	int32 PlayerTriggeredEvents;

	// Helper functions
	void UpdateActiveEvents(float DeltaTime);
	void UpdateEventChains(float DeltaTime);
	void CheckForRandomEvents();
	void ProcessEventStart(const FEconomicEventData& Event);
	void ProcessEventEnd(const FEconomicEventData& Event);
	void RecordEventHistory(const FEconomicEventData& Event, FString PlayerID = "");
	void ApplyEventEffects(const FEconomicEventData& Event);
	void RemoveEventEffects(const FEconomicEventData& Event);
	FString GenerateEventID() const;
	FString GenerateChainID() const;
	void LogEvent(const FString& EventID, const FString& Action, const FString& Details) const;
	FEconomicEventData CreateEventFromTemplate(const FEconomicEventData& Template);
	void ProcessEventChain(FEventChain& Chain);
	bool CanTriggerEvent(const FEconomicEventData& Event) const;
};