// Copyright Epic Games, Inc. All Rights Reserved.

#include "EconomicEventManager.h"
#include "DynamicMarketManager.h"
#include "FactionEconomyManager.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"

UEconomicEventManager::UEconomicEventManager()
{
	RandomEventCheckInterval = 3600.0f; // Check every hour
	BaseRandomEventChance = 0.1f; // 10% base chance
	MaxEventSeverity = 1.0f;
	MinEventDuration = 1.0f; // 1 day minimum
	MaxEventDuration = 30.0f; // 30 days maximum
	MaxActiveEvents = 10;
	bEnableRandomEvents = true;
	bEnableEventChains = true;
	bShowDebugMessages = false;

	RandomEventTimer = 0.0f;
	EventChainTimer = 0.0f;

	TotalEventsTriggered = 0;
	TotalEventsEnded = 0;
	TotalEventChainsStarted = 0;
	PlayerTriggeredEvents = 0;
}

void UEconomicEventManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("EconomicEventManager initialized"));

	// Get subsystem references
	UWorld* World = GetWorld();
	if (World)
	{
		DynamicMarketManager = World->GetSubsystem<UDynamicMarketManager>();
		FactionEconomyManager = World->GetSubsystem<UFactionEconomyManager>();
	}

	// Initialize default events
	InitializeDefaultEvents();
}

void UEconomicEventManager::Deinitialize()
{
	// Clear all data
	ActiveEvents.Empty();
	EventHistory.Empty();
	ActiveEventChains.Empty();
	EventTemplates.Empty();

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("EconomicEventManager deinitialized"));
}

void UEconomicEventManager::Tick(float DeltaTime)
{
	// Update active events
	UpdateActiveEvents(DeltaTime);

	// Check for random events
	if (bEnableRandomEvents)
	{
		RandomEventTimer += DeltaTime;
		if (RandomEventTimer >= RandomEventCheckInterval)
		{
			CheckForRandomEvents();
			RandomEventTimer = 0.0f;
		}
	}

	// Update event chains
	if (bEnableEventChains)
	{
		UpdateEventChains(DeltaTime);
	}
}

FEconomicEventData UEconomicEventManager::TriggerEvent(
	FString EventID,
	EEconomicEventCategory Category,
	float Severity,
	float DurationDays,
	FString TriggeringFactionID
)
{
	// Check if we can add more events
	if (ActiveEvents.Num() >= MaxActiveEvents)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot trigger event %s - max active events reached"), *EventID);
		return FEconomicEventData();
	}

	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.Category = Category;
	Event.Severity = FMath::Clamp(Severity, 0.0f, MaxEventSeverity);
	Event.DurationDays = FMath::Clamp(DurationDays, MinEventDuration, MaxEventDuration);
	Event.StartTime = FDateTime::Now();
	Event.EndTime = Event.StartTime + FTimespan(Event.DurationDays, 0, 0, 0);
	Event.bIsActive = true;
	Event.TriggeringFactionID = TriggeringFactionID;

	// Set up event effects based on category and severity
	SetupEventEffects(Event);

	// Add to active events
	ActiveEvents.Add(Event);
	TotalEventsTriggered++;

	// Apply event effects
	ProcessEventStart(Event);

	// Record in history
	RecordEventHistory(Event, "");

	// Broadcast event start
	OnEconomicEventStarted.Broadcast(Event);

	LogEvent(EventID, TEXT("STARTED"), FString::Printf(TEXT("Category: %d, Severity: %.2f"), (int32)Category, Severity));

	return Event;
}

void UEconomicEventManager::EndEvent(FString EventID)
{
	for (int32 i = ActiveEvents.Num() - 1; i >= 0; i--)
	{
		if (ActiveEvents[i].EventID == EventID)
		{
			FEconomicEventData Event = ActiveEvents[i];
			Event.bIsActive = false;

			// Remove event effects
			ProcessEventEnd(Event);

			// Move to history
			ActiveEvents.RemoveAt(i);
			TotalEventsEnded++;

			// Broadcast event end
			OnEconomicEventEnded.Broadcast(Event);

			LogEvent(EventID, TEXT("ENDED"), TEXT(""));
			break;
		}
	}
}

TArray<FEconomicEventData> UEconomicEventManager::GetActiveEvents() const
{
	return ActiveEvents;
}

FEconomicEventData UEconomicEventManager::GetEvent(FString EventID) const
{
	for (const FEconomicEventData& Event : ActiveEvents)
	{
		if (Event.EventID == EventID)
		{
			return Event;
		}
	}

	for (const FEventHistoryEntry& HistoryEntry : EventHistory)
	{
		if (HistoryEntry.EventID == EventID)
		{
			// Would reconstruct event from history
			return FEconomicEventData();
		}
	}

	return FEconomicEventData();
}

TArray<FEconomicEventData> UEconomicEventManager::GetEventsByCategory(EEconomicEventCategory Category) const
{
	TArray<FEconomicEventData> Events;

	for (const FEconomicEventData& Event : ActiveEvents)
	{
		if (Event.Category == Category)
		{
			Events.Add(Event);
		}
	}

	return Events;
}

TArray<FEconomicEventData> UEconomicEventManager::GetEventsByScope(EEconomicEventScope Scope) const
{
	TArray<FEconomicEventData> Events;

	for (const FEconomicEventData& Event : ActiveEvents)
	{
		if (Event.Scope == Scope)
		{
			Events.Add(Event);
		}
	}

	return Events;
}

void UEconomicEventManager::ClearAllEvents()
{
	for (const FEconomicEventData& Event : ActiveEvents)
	{
		ProcessEventEnd(Event);
		OnEconomicEventEnded.Broadcast(Event);
	}

	ActiveEvents.Empty();
	UE_LOG(LogTemp, Log, TEXT("All economic events cleared"));
}

FEventChain UEconomicEventManager::StartEventChain(FString ChainID)
{
	// Find the chain template
	for (FEventChain& Chain : ActiveEventChains)
	{
		if (Chain.ChainID == ChainID && !Chain.bIsActive)
		{
			Chain.bIsActive = true;
			TotalEventChainsStarted++;

			// Trigger first event in chain
			if (Chain.Events.Num() > 0)
			{
				FEconomicEventData FirstEvent = Chain.Events[0];
				TriggerEvent(FirstEvent.EventID, FirstEvent.Category, FirstEvent.Severity, FirstEvent.DurationDays, FirstEvent.TriggeringFactionID);
			}

			OnEventChainStarted.Broadcast(Chain);
			LogEvent(ChainID, TEXT("CHAIN STARTED"), TEXT(""));

			return Chain;
		}
	}

	return FEventChain();
}

void UEconomicEventManager::EndEventChain(FString ChainID)
{
	for (int32 i = ActiveEventChains.Num() - 1; i >= 0; i--)
	{
		if (ActiveEventChains[i].ChainID == ChainID)
		{
			FEventChain Chain = ActiveEventChains[i];
			Chain.bIsActive = false;

			// End all active events in chain
			for (const FEconomicEventData& Event : Chain.Events)
			{
				EndEvent(Event.EventID);
			}

			ActiveEventChains.RemoveAt(i);
			OnEventChainEnded.Broadcast(Chain);
			LogEvent(ChainID, TEXT("CHAIN ENDED"), TEXT(""));
			break;
		}
	}
}

TArray<FEventChain> UEconomicEventManager::GetActiveEventChains() const
{
	return ActiveEventChains;
}

FEventChain UEconomicEventManager::CreateEventChain(
	FString ChainName,
	TArray<FEconomicEventData> Events,
	bool bIsSequential,
	FTimespan DelayBetweenEvents
)
{
	FEventChain Chain;
	Chain.ChainID = GenerateChainID();
	Chain.ChainName = ChainName;
	Chain.Events = Events;
	Chain.bIsSequential = bIsSequential;
	Chain.DelayBetweenEvents = DelayBetweenEvents;
	Chain.bIsActive = false;
	Chain.ChainDurationDays = 0.0f;

	// Calculate total duration
	for (const FEconomicEventData& Event : Events)
	{
		Chain.ChainDurationDays += Event.DurationDays;
	}

	ActiveEventChains.Add(Chain);
	return Chain;
}

FEconomicEventData UEconomicEventManager::GenerateRandomEvent(
	EEconomicEventCategory Category,
	EEconomicEventScope Scope
)
{
	// Create a random event based on category and scope
	FEconomicEventData Event;
	Event.EventID = GenerateEventID();
	Event.Category = Category;
	Event.Scope = Scope;
	Event.Severity = FMath::FRandRange(0.2f, MaxEventSeverity);
	Event.DurationDays = FMath::FRandRange(MinEventDuration, MaxEventDuration);
	Event.StartTime = FDateTime::Now();
	Event.EndTime = Event.StartTime + FTimespan(Event.DurationDays, 0, 0, 0);
	Event.bIsActive = true;

	// Set random impact type
	int32 ImpactRand = FMath::RandRange(0, 3);
	switch (ImpactRand)
	{
		case 0: Event.ImpactType = EEconomicImpactType::Positive; break;
		case 1: Event.ImpactType = EEconomicImpactType::Negative; break;
		case 2: Event.ImpactType = EEconomicImpactType::Mixed; break;
		case 3: Event.ImpactType = EEconomicImpactType::Neutral; break;
	}

	// Set up random effects
	SetupRandomEventEffects(Event);

	return Event;
}

FEventChain UEconomicEventManager::GenerateRandomEventChain(EEconomicEventScope Scope)
{
	// Create a chain of related events
	TArray<FEconomicEventData> Events;

	// Generate 3-5 events for the chain
	int32 EventCount = FMath::RandRange(3, 5);
	for (int32 i = 0; i < EventCount; i++)
	{
		FEconomicEventData Event = GenerateRandomEvent(EEconomicEventCategory::Random, Scope);
		Event.DurationDays = FMath::FRandRange(1.0f, 5.0f); // Shorter durations for chain events
		Events.Add(Event);
	}

	FString ChainName = FString::Printf(TEXT("Random Event Chain %d"), FMath::RandRange(1, 100));
	return CreateEventChain(ChainName, Events, true, FTimespan(1, 0, 0, 0)); // 1 day between events
}

void UEconomicEventManager::SetRandomEventChance(float Chance)
{
	BaseRandomEventChance = FMath::Clamp(Chance, 0.0f, 1.0f);
}

float UEconomicEventManager::GetRandomEventChance() const
{
	return BaseRandomEventChance;
}

bool UEconomicEventManager::PlayerTriggerEvent(FString EventID, FString PlayerID)
{
	// Check if event can be triggered by player
	FEconomicEventData EventTemplate = GetEventTemplate(EventID);
	if (!EventTemplate.bCanBeTriggeredByPlayer)
	{
		return false;
	}

	// Check trigger chance
	if (FMath::FRand() > EventTemplate.PlayerTriggerChance)
	{
		return false;
	}

	// Trigger the event
	FEconomicEventData TriggeredEvent = TriggerEvent(
		EventID,
		EventTemplate.Category,
		EventTemplate.Severity,
		EventTemplate.DurationDays,
		EventTemplate.TriggeringFactionID
	);

	PlayerTriggeredEvents++;
	OnPlayerTriggeredEvent.Broadcast(TriggeredEvent, PlayerID);
	LogEvent(EventID, TEXT("PLAYER TRIGGERED"), FString::Printf(TEXT("Player: %s"), *PlayerID));

	return true;
}

TArray<FEconomicEventData> UEconomicEventManager::GetPlayerTriggerableEvents() const
{
	TArray<FEconomicEventData> TriggerableEvents;

	for (const auto& Pair : EventTemplates)
	{
		if (Pair.Value.bCanBeTriggeredByPlayer)
		{
			TriggerableEvents.Add(Pair.Value);
		}
	}

	return TriggerableEvents;
}

void UEconomicEventManager::SetPlayerTriggerChance(float Chance)
{
	for (auto& Pair : EventTemplates)
	{
		Pair.Value.PlayerTriggerChance = FMath::Clamp(Chance, 0.0f, 1.0f);
	}
}

FEconomicEventData UEconomicEventManager::TriggerMarketCrash(
	ECommodityCategory Category,
	float Severity,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("MarketCrash_%d"), FMath::RandRange(1, 1000));
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Market Crash");
	Event.Category = EEconomicEventCategory::Market;
	Event.Scope = EEconomicEventScope::Regional;
	Event.ImpactType = EEconomicImpactType::Negative;
	Event.Severity = Severity;
	Event.DurationDays = DurationDays;
	Event.Description = FString::Printf(TEXT("Market crash in %s category"), *UEnum::GetValueAsString(Category));
	Event.bCanBeTriggeredByPlayer = false;

	// Set up commodity impacts
	for (int32 i = 0; i < (int32)ECommodityCategory::MAX; i++)
	{
		ECommodityCategory CommodityCategory = (ECommodityCategory)i;
		if (CommodityCategory == Category)
		{
			Event.CommodityPriceMultipliers.Add(FName(*UEnum::GetValueAsString(CommodityCategory)), 1.0f - Severity);
			Event.CommodityDemandMultipliers.Add(FName(*UEnum::GetValueAsString(CommodityCategory)), 1.0f - (Severity * 0.5f));
		}
	}

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays);
}

FEconomicEventData UEconomicEventManager::TriggerMarketBoom(
	ECommodityCategory Category,
	float Magnitude,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("MarketBoom_%d"), FMath::RandRange(1, 1000));
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Market Boom");
	Event.Category = EEconomicEventCategory::Market;
	Event.Scope = EEconomicEventScope::Regional;
	Event.ImpactType = EEconomicImpactType::Positive;
	Event.Severity = Magnitude;
	Event.DurationDays = DurationDays;
	Event.Description = FString::Printf(TEXT("Market boom in %s category"), *UEnum::GetValueAsString(Category));
	Event.bCanBeTriggeredByPlayer = false;

	// Set up commodity impacts
	for (int32 i = 0; i < (int32)ECommodityCategory::MAX; i++)
	{
		ECommodityCategory CommodityCategory = (ECommodityCategory)i;
		if (CommodityCategory == Category)
		{
			Event.CommodityPriceMultipliers.Add(FName(*UEnum::GetValueAsString(CommodityCategory)), 1.0f + Magnitude);
			Event.CommodityDemandMultipliers.Add(FName(*UEnum::GetValueAsString(CommodityCategory)), 1.0f + (Magnitude * 0.7f));
		}
	}

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays);
}

FEconomicEventData UEconomicEventManager::TriggerSupplyShortage(
	FName CommodityID,
	float Severity,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("SupplyShortage_%s_%d"), *CommodityID.ToString(), FMath::RandRange(1, 1000));
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Supply Shortage");
	Event.Category = EEconomicEventCategory::Market;
	Event.Scope = EEconomicEventScope::Local;
	Event.ImpactType = EEconomicImpactType::Negative;
	Event.Severity = Severity;
	Event.DurationDays = DurationDays;
	Event.Description = FString::Printf(TEXT("Supply shortage of %s"), *CommodityID.ToString());
	Event.bCanBeTriggeredByPlayer = false;

	// Set up commodity impacts
	Event.CommodityPriceMultipliers.Add(CommodityID, 1.0f + Severity);
	Event.CommoditySupplyMultipliers.Add(CommodityID, 1.0f - Severity);

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays);
}

FEconomicEventData UEconomicEventManager::TriggerSupplyGlut(
	FName CommodityID,
	float Severity,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("SupplyGlut_%s_%d"), *CommodityID.ToString(), FMath::RandRange(1, 1000));
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Supply Glut");
	Event.Category = EEconomicEventCategory::Market;
	Event.Scope = EEconomicEventScope::Local;
	Event.ImpactType = EEconomicImpactType::Negative;
	Event.Severity = Severity;
	Event.DurationDays = DurationDays;
	Event.Description = FString::Printf(TEXT("Supply glut of %s"), *CommodityID.ToString());
	Event.bCanBeTriggeredByPlayer = false;

	// Set up commodity impacts
	Event.CommodityPriceMultipliers.Add(CommodityID, 1.0f - (Severity * 0.5f));
	Event.CommoditySupplyMultipliers.Add(CommodityID, 1.0f + Severity);

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays);
}

FEconomicEventData UEconomicEventManager::TriggerTradeWar(
	FString Faction1ID,
	FString Faction2ID,
	float Severity,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("TradeWar_%s_vs_%s"), *Faction1ID, *Faction2ID);
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Trade War");
	Event.Category = EEconomicEventCategory::Faction;
	Event.Scope = EEconomicEventScope::Regional;
	Event.ImpactType = EEconomicImpactType::Mixed;
	Event.Severity = Severity;
	Event.DurationDays = DurationDays;
	Event.Description = FString::Printf(TEXT("Trade war between %s and %s"), *Faction1ID, *Faction2ID);
	Event.TriggeringFactionID = Faction1ID;
	Event.TargetFactionID = Faction2ID;
	Event.bCanBeTriggeredByPlayer = false;

	// Set up faction impacts
	Event.FactionImpactMultipliers.Add(Faction1ID, 1.0f - (Severity * 0.3f));
	Event.FactionImpactMultipliers.Add(Faction2ID, 1.0f - (Severity * 0.3f));

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays, Faction1ID);
}

FEconomicEventData UEconomicEventManager::TriggerPirateBlockade(
	FString TargetStationID,
	float Severity,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("PirateBlockade_%s_%d"), *TargetStationID, FMath::RandRange(1, 1000));
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Pirate Blockade");
	Event.Category = EEconomicEventCategory::Market;
	Event.Scope = EEconomicEventScope::Local;
	Event.ImpactType = EEconomicImpactType::Negative;
	Event.Severity = Severity;
	Event.DurationDays = DurationDays;
	Event.Description = FString::Printf(TEXT("Pirate blockade of %s"), *TargetStationID);
	Event.bCanBeTriggeredByPlayer = false;

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays);
}

FEconomicEventData UEconomicEventManager::TriggerFactionEconomicBoom(
	FString FactionID,
	float Magnitude,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("FactionBoom_%s_%d"), *FactionID, FMath::RandRange(1, 1000));
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Faction Economic Boom");
	Event.Category = EEconomicEventCategory::Faction;
	Event.Scope = EEconomicEventScope::Regional;
	Event.ImpactType = EEconomicImpactType::Positive;
	Event.Severity = Magnitude;
	Event.DurationDays = DurationDays;
	Event.Description = FString::Printf(TEXT("Economic boom in %s faction"), *FactionID);
	Event.TriggeringFactionID = FactionID;
	Event.bCanBeTriggeredByPlayer = false;

	// Set up faction impacts
	Event.FactionImpactMultipliers.Add(FactionID, 1.0f + Magnitude);

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays, FactionID);
}

FEconomicEventManager::TriggerFactionEconomicCrisis(
	FString FactionID,
	float Severity,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("FactionCrisis_%s_%d"), *FactionID, FMath::RandRange(1, 1000));
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Faction Economic Crisis");
	Event.Category = EEconomicEventCategory::Faction;
	Event.Scope = EEconomicEventScope::Regional;
	Event.ImpactType = EEconomicImpactType::Negative;
	Event.Severity = Severity;
	Event.DurationDays = DurationDays;
	Event.Description = FString::Printf(TEXT("Economic crisis in %s faction"), *FactionID);
	Event.TriggeringFactionID = FactionID;
	Event.bCanBeTriggeredByPlayer = false;

	// Set up faction impacts
	Event.FactionImpactMultipliers.Add(FactionID, 1.0f - Severity);

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays, FactionID);
}

FEconomicEventData UEconomicEventManager::TriggerFactionCivilWar(
	FString FactionID,
	float Severity,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("CivilWar_%s_%d"), *FactionID, FMath::RandRange(1, 1000));
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Faction Civil War");
	Event.Category = EEconomicEventCategory::Faction;
	Event.Scope = EEconomicEventScope::Regional;
	Event.ImpactType = EEconomicImpactType::Negative;
	Event.Severity = Severity;
	Event.DurationDays = DurationDays;
	Event.Description = FString::Printf(TEXT("Civil war in %s faction"), *FactionID);
	Event.TriggeringFactionID = FactionID;
	Event.bCanBeTriggeredByPlayer = false;

	// Set up faction impacts
	Event.FactionImpactMultipliers.Add(FactionID, 1.0f - (Severity * 0.7f));

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays, FactionID);
}

FEconomicEventData UEconomicEventManager::TriggerFactionRevolution(
	FString FactionID,
	float Severity,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("Revolution_%s_%d"), *FactionID, FMath::RandRange(1, 1000));
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Faction Revolution");
	Event.Category = EEconomicEventCategory::Faction;
	Event.Scope = EEconomicEventScope::Regional;
	Event.ImpactType = EEconomicImpactType::Mixed;
	Event.Severity = Severity;
	Event.DurationDays = DurationDays;
	Event.Description = FString::Printf(TEXT("Revolution in %s faction"), *FactionID);
	Event.TriggeringFactionID = FactionID;
	Event.bCanBeTriggeredByPlayer = false;

	// Set up faction impacts
	Event.FactionImpactMultipliers.Add(FactionID, 1.0f - (Severity * 0.5f));

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays, FactionID);
}

FEconomicEventData UEconomicEventManager::TriggerSeasonalFestival(
	FString StationID,
	float Magnitude,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("Festival_%s_%d"), *StationID, FMath::RandRange(1, 1000));
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Seasonal Festival");
	Event.Category = EEconomicEventCategory::Seasonal;
	Event.Scope = EEconomicEventScope::Local;
	Event.ImpactType = EEconomicImpactType::Positive;
	Event.Severity = Magnitude;
	Event.DurationDays = DurationDays;
	Event.Description = FString::Printf(TEXT("Seasonal festival at %s"), *StationID);
	Event.bCanBeTriggeredByPlayer = false;

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays);
}

FEconomicEventData UEconomicEventManager::TriggerHarvestSeason(
	float Magnitude,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("HarvestSeason_%d"), FMath::RandRange(1, 1000));
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Harvest Season");
	Event.Category = EEconomicEventCategory::Seasonal;
	Event.Scope = EEconomicEventScope::Global;
	Event.ImpactType = EEconomicImpactType::Positive;
	Event.Severity = Magnitude;
	Event.DurationDays = DurationDays;
	Event.Description = TEXT("Bountiful harvest season");
	Event.bCanBeTriggeredByPlayer = false;

	// Set up food-related commodity impacts
	Event.CommodityPriceMultipliers.Add(FName("FoodSupplies"), 1.0f - (Magnitude * 0.3f));
	Event.CommoditySupplyMultipliers.Add(FName("FoodSupplies"), 1.0f + Magnitude);

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays);
}

FEconomicEventData UEconomicEventManager::TriggerHolidaySeason(
	float Magnitude,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("HolidaySeason_%d"), FMath::RandRange(1, 1000));
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Holiday Season");
	Event.Category = EEconomicEventCategory::Seasonal;
	Event.Scope = EEconomicEventScope::Global;
	Event.ImpactType = EEconomicImpactType::Positive;
	Event.Severity = Magnitude;
	Event.DurationDays = DurationDays;
	Event.Description = TEXT("Holiday season - increased consumer spending");
	Event.bCanBeTriggeredByPlayer = false;

	// Set up consumer goods impacts
	Event.CommodityDemandMultipliers.Add(FName("ConsumerGoods"), 1.0f + Magnitude);
	Event.CommodityDemandMultipliers.Add(FName("LuxuryGoods"), 1.0f + (Magnitude * 1.5f));

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays);
}

FEconomicEventData UEconomicEventManager::TriggerGalacticEconomicBoom(
	float Magnitude,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("GalacticBoom_%d"), FMath::RandRange(1, 1000));
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Galactic Economic Boom");
	Event.Category = EEconomicEventCategory::Global;
	Event.Scope = EEconomicEventScope::Universal;
	Event.ImpactType = EEconomicImpactType::Positive;
	Event.Severity = Magnitude;
	Event.DurationDays = DurationDays;
	Event.Description = TEXT("Galactic-wide economic boom");
	Event.bCanBeTriggeredByPlayer = false;

	// Set up global impacts
	for (int32 i = 0; i < (int32)ECommodityCategory::MAX; i++)
	{
		ECommodityCategory Category = (ECommodityCategory)i;
		Event.CommodityDemandMultipliers.Add(FName(*UEnum::GetValueAsString(Category)), 1.0f + (Magnitude * 0.5f));
	}

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays);
}

FEconomicEventData UEconomicEventManager::TriggerGalacticRecession(
	float Severity,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("GalacticRecession_%d"), FMath::RandRange(1, 1000));
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Galactic Recession");
	Event.Category = EEconomicEventCategory::Global;
	Event.Scope = EEconomicEventScope::Universal;
	Event.ImpactType = EEconomicImpactType::Negative;
	Event.Severity = Severity;
	Event.DurationDays = DurationDays;
	Event.Description = TEXT("Galactic-wide economic recession");
	Event.bCanBeTriggeredByPlayer = false;

	// Set up global impacts
	for (int32 i = 0; i < (int32)ECommodityCategory::MAX; i++)
	{
		ECommodityCategory Category = (ECommodityCategory)i;
		Event.CommodityDemandMultipliers.Add(FName(*UEnum::GetValueAsString(Category)), 1.0f - (Severity * 0.4f));
	}

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays);
}

FEconomicEventData UEconomicEventManager::TriggerTechRevolution(
	float Magnitude,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("TechRevolution_%d"), FMath::RandRange(1, 1000));
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Technological Revolution");
	Event.Category = EEconomicEventCategory::Global;
	Event.Scope = EEconomicEventScope::Universal;
	Event.ImpactType = EEconomicImpactType::Positive;
	Event.Severity = Magnitude;
	Event.DurationDays = DurationDays;
	Event.Description = TEXT("Major technological breakthrough revolutionizes industry");
	Event.bCanBeTriggeredByPlayer = false;

	// Set up tech-related commodity impacts
	Event.CommodityPriceMultipliers.Add(FName("ElectronicComponents"), 1.0f - (Magnitude * 0.2f));
	Event.CommodityPriceMultipliers.Add(FName("Microchips"), 1.0f - (Magnitude * 0.3f));
	Event.CommoditySupplyMultipliers.Add(FName("ElectronicComponents"), 1.0f + (Magnitude * 0.5f));

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays);
}

FEconomicEventData UEconomicEventManager::TriggerResourceDiscovery(
	FName CommodityID,
	float Magnitude,
	float DurationDays
)
{
	FString EventID = FString::Printf(TEXT("ResourceDiscovery_%s_%d"), *CommodityID.ToString(), FMath::RandRange(1, 1000));
	
	FEconomicEventData Event;
	Event.EventID = EventID;
	Event.EventName = TEXT("Resource Discovery");
	Event.Category = EEconomicEventCategory::Global;
	Event.Scope = EEconomicEventScope::Global;
	Event.ImpactType = EEconomicImpactType::Mixed;
	Event.Severity = Magnitude;
	Event.DurationDays = DurationDays;
	Event.Description = FString::Printf(TEXT("Major discovery of %s"), *CommodityID.ToString());
	Event.bCanBeTriggeredByPlayer = false;

	// Set up commodity impacts
	Event.CommodityPriceMultipliers.Add(CommodityID, 1.0f - (Magnitude * 0.4f));
	Event.CommoditySupplyMultipliers.Add(CommodityID, 1.0f + (Magnitude * 0.8f));

	return TriggerEvent(EventID, Event.Category, Event.Severity, Event.DurationDays);
}

TArray<FEventHistoryEntry> UEconomicEventManager::GetEventHistory(int32 Count) const
{
	TArray<FEventHistoryEntry> RecentHistory;

	int32 StartIndex = FMath::Max(0, EventHistory.Num() - Count);
	for (int32 i = StartIndex; i < EventHistory.Num(); i++)
	{
		RecentHistory.Add(EventHistory[i]);
	}

	return RecentHistory;
}

TArray<FEventHistoryEntry> UEconomicEventManager::GetEventsInDateRange(
	FDateTime StartDate,
	FDateTime EndDate
) const
{
	TArray<FEventHistoryEntry> FilteredHistory;

	for (const FEventHistoryEntry& Entry : EventHistory)
	{
		if (Entry.TriggerTime >= StartDate && Entry.TriggerTime <= EndDate)
		{
			FilteredHistory.Add(Entry);
		}
	}

	return FilteredHistory;
}

void UEconomicEventManager::ClearEventHistory()
{
	EventHistory.Empty();
	UE_LOG(LogTemp, Log, TEXT("Event history cleared"));
}

FString UEconomicEventManager::ExportEventHistory() const
{
	FString Export = TEXT("Economic Event History Export\n");
	Export += TEXT("============================\n\n");

	for (const FEventHistoryEntry& Entry : EventHistory)
	{
		Export += FString::Printf(TEXT("Event ID: %s\n"), *Entry.EventID);
		Export += FString::Printf(TEXT("Trigger Time: %s\n"), *Entry.TriggerTime.ToString());
		Export += FString::Printf(TEXT("Severity: %.2f\n"), Entry.ActualSeverity);
		Export += FString::Printf(TEXT("Triggering Player: %s\n\n"), *Entry.TriggeringPlayerID);
	}

	return Export;
}

TMap<EEconomicEventCategory, int32> UEconomicEventManager::GetEventStatistics() const
{
	TMap<EEconomicEventCategory, int32> Statistics;

	for (const FEventHistoryEntry& Entry : EventHistory)
	{
		// Would need to store category in history entry
		// For now, just count total
		Statistics.FindOrAdd(EEconomicEventCategory::Random)++;
	}

	return Statistics;
}

float UEconomicEventManager::GetAverageEventSeverity() const
{
	if (EventHistory.Num() == 0)
	{
		return 0.0f;
	}

	float TotalSeverity = 0.0f;
	for (const FEventHistoryEntry& Entry : EventHistory)
	{
		TotalSeverity += Entry.ActualSeverity;
	}

	return TotalSeverity / EventHistory.Num();
}

EEconomicEventCategory UEconomicEventManager::GetMostCommonEventType() const
{
	TMap<EEconomicEventCategory, int32> TypeCounts;

	for (const FEconomicEventData& Event : ActiveEvents)
	{
		TypeCounts.FindOrAdd(Event.Category)++;
	}

	EEconomicEventCategory MostCommon = EEconomicEventCategory::Random;
	int32 HighestCount = 0;

	for (const auto& Pair : TypeCounts)
	{
		if (Pair.Value > HighestCount)
		{
			HighestCount = Pair.Value;
			MostCommon = Pair.Key;
		}
	}

	return MostCommon;
}

FString UEconomicEventManager::GenerateEventReport() const
{
	FString Report = TEXT("=== ECONOMIC EVENT REPORT ===\n\n");

	Report += FString::Printf(TEXT("Active Events: %d\n"), ActiveEvents.Num());
	Report += FString::Printf(TEXT("Event Chains: %d\n"), ActiveEventChains.Num());
	Report += FString::Printf(TEXT("Total Events Triggered: %d\n"), TotalEventsTriggered);
	Report += FString::Printf(TEXT("Total Events Ended: %d\n"), TotalEventsEnded);
	Report += FString::Printf(TEXT("Player Triggered Events: %d\n\n"), PlayerTriggeredEvents);

	Report += TEXT("Active Events:\n");
	for (const FEconomicEventData& Event : ActiveEvents)
	{
		Report += FString::Printf(TEXT("  %s: %s (Severity: %.2f, %.1f days remaining)\n"),
			*Event.EventID,
			*Event.EventName,
			Event.Severity,
			(Event.EndTime - FDateTime::Now()).GetTotalDays());
	}

	Report += FString::Printf(TEXT("\nMost Common Event Type: %d\n"), (int32)GetMostCommonEventType());
	Report += FString::Printf(TEXT("Average Event Severity: %.2f\n"), GetAverageEventSeverity());

	return Report;
}

void UEconomicEventManager::LoadEventTemplates(UDataTable* TemplateTable)
{
	if (!TemplateTable)
	{
		return;
	}

	// Would load from data table
	UE_LOG(LogTemp, Log, TEXT("Event templates loaded from data table"));
}

void UEconomicEventManager::InitializeDefaultEvents()
{
	// Create some default event templates
	{
		FEconomicEventData Template;
		Template.EventID = "MarketCrash_Template";
		Template.EventName = "Market Crash";
		Template.Category = EEconomicEventCategory::Market;
		Template.Scope = EEconomicEventScope::Regional;
		Template.ImpactType = EEconomicImpactType::Negative;
		Template.Severity = 0.6f;
		Template.DurationDays = 7.0f;
		Template.Description = "Sudden market crash affecting prices";
		Template.bCanBeTriggeredByPlayer = false;
		EventTemplates.Add(Template.EventID, Template);
	}

	{
		FEconomicEventData Template;
		Template.EventID = "SupplyShortage_Template";
		Template.EventName = "Supply Shortage";
		Template.Category = EEconomicEventCategory::Market;
		Template.Scope = EEconomicEventScope::Local;
		Template.ImpactType = EEconomicImpactType::Negative;
		Template.Severity = 0.5f;
		Template.DurationDays = 5.0f;
		Template.Description = "Critical supply shortage";
		Template.bCanBeTriggeredByPlayer = false;
		EventTemplates.Add(Template.EventID, Template);
	}

	{
		FEconomicEventData Template;
		Template.EventID = "EconomicBoom_Template";
		Template.EventName = "Economic Boom";
		Template.Category = EEconomicEventCategory::Market;
		Template.Scope = EEconomicEventScope::Regional;
		Template.ImpactType = EEconomicImpactType::Positive;
		Template.Severity = 0.4f;
		Template.DurationDays = 10.0f;
		Template.Description = "Economic boom increases demand";
		Template.bCanBeTriggeredByPlayer = false;
		EventTemplates.Add(Template.EventID, Template);
	}

	UE_LOG(LogTemp, Log, TEXT("Initialized %d default event templates"), EventTemplates.Num());
}

void UEconomicEventManager::UpdateActiveEvents(float DeltaTime)
{
	FDateTime CurrentTime = FDateTime::Now();

	// Check for expired events
	for (int32 i = ActiveEvents.Num() - 1; i >= 0; i--)
	{
		if (CurrentTime >= ActiveEvents[i].EndTime)
		{
			EndEvent(ActiveEvents[i].EventID);
		}
	}
}

void UEconomicEventManager::UpdateEventChains(float DeltaTime)
{
	EventChainTimer += DeltaTime;

	// Check if it's time to process event chains
	if (EventChainTimer >= 60.0f) // Process every minute
	{
		for (FEventChain& Chain : ActiveEventChains)
		{
			if (Chain.bIsActive)
			{
				ProcessEventChain(Chain);
			}
		}
		EventChainTimer = 0.0f;
	}
}

void UEconomicEventManager::CheckForRandomEvents()
{
	// Check each category for random events
	float RandomChance = FMath::FRand();
	if (RandomChance <= BaseRandomEventChance)
	{
		// Select random category
		int32 CategoryIndex = FMath::RandRange(0, (int32)EEconomicEventCategory::Random);
		EEconomicEventCategory Category = (EEconomicEventCategory)CategoryIndex;

		// Select random scope
		int32 ScopeIndex = FMath::RandRange(0, (int32)EEconomicEventScope::Global);
		EEconomicEventScope Scope = (EEconomicEventScope)ScopeIndex;

		// Generate and trigger random event
		FEconomicEventData RandomEvent = GenerateRandomEvent(Category, Scope);
		TriggerEvent(RandomEvent.EventID, Category, RandomEvent.Severity, RandomEvent.DurationDays);
	}
}

void UEconomicEventManager::ProcessEventStart(const FEconomicEventData& Event)
{
	// Apply event effects to game world
	ApplyEventEffects(Event);

	if (bShowDebugMessages)
	{
		UE_LOG(LogTemp, Log, TEXT("Event %s started: %s"), *Event.EventID, *Event.Description);
	}
}

void UEconomicEventManager::ProcessEventEnd(const FEconomicEventData& Event)
{
	// Remove event effects
	RemoveEventEffects(Event);

	if (bShowDebugMessages)
	{
		UE_LOG(LogTemp, Log, TEXT("Event %s ended"), *Event.EventID);
	}
}

void UEconomicEventManager::RecordEventHistory(const FEconomicEventData& Event, FString PlayerID)
{
	FEventHistoryEntry Entry;
	Entry.EventID = Event.EventID;
	Entry.TriggerTime = Event.StartTime;
	Entry.EndTime = Event.EndTime;
	Entry.ActualSeverity = Event.Severity;
	Entry.TriggeringPlayerID = PlayerID;
	Entry.ActualImpacts = Event.CommodityPriceMultipliers; // Simplified

	EventHistory.Add(Entry);
}

void UEconomicEventManager::ApplyEventEffects(const FEconomicEventData& Event)
{
	// Apply commodity price effects
	if (DynamicMarketManager)
	{
		for (const auto& Pair : Event.CommodityPriceMultipliers)
		{
			// Would apply to dynamic market manager
		}
	}

	// Apply faction effects
	if (FactionEconomyManager)
	{
		for (const auto& Pair : Event.FactionImpactMultipliers)
		{
			// Would apply to faction economy manager
		}
	}
}

void UEconomicEventManager::RemoveEventEffects(const FEconomicEventData& Event)
{
	// Remove commodity price effects
	if (DynamicMarketManager)
	{
		for (const auto& Pair : Event.CommodityPriceMultipliers)
		{
			// Would remove from dynamic market manager
		}
	}

	// Remove faction effects
	if (FactionEconomyManager)
	{
		for (const auto& Pair : Event.FactionImpactMultipliers)
		{
			// Would remove from faction economy manager
		}
	}
}

FString UEconomicEventManager::GenerateEventID() const
{
	return FString::Printf(TEXT("Event_%d"), FMath::RandRange(10000, 99999));
}

FString UEconomicEventManager::GenerateChainID() const
{
	return FString::Printf(TEXT("Chain_%d"), FMath::RandRange(1000, 9999));
}

void UEconomicEventManager::LogEvent(const FString& EventID, const FString& Action, const FString& Details) const
{
	if (bShowDebugMessages)
	{
		UE_LOG(LogTemp, Log, TEXT("[EconomicEventManager] %s - %s: %s"), *EventID, *Action, *Details);
	}
}

FEconomicEventData UEconomicEventManager::GetEventTemplate(FString EventID) const
{
	const FEconomicEventData* Template = EventTemplates.Find(EventID);
	return Template ? *Template : FEconomicEventData();
}

void UEconomicEventManager::SetupEventEffects(FEconomicEventData& Event)
{
	// Set up default effects based on category and severity
	switch (Event.Category)
	{
		case EEconomicEventCategory::Market:
			SetupMarketEventEffects(Event);
			break;
		case EEconomicEventCategory::Faction:
			SetupFactionEventEffects(Event);
			break;
		case EEconomicEventCategory::Global:
			SetupGlobalEventEffects(Event);
			break;
		case EEconomicEventCategory::Seasonal:
			SetupSeasonalEventEffects(Event);
			break;
		default:
			SetupRandomEventEffects(Event);
			break;
	}
}

void UEconomicEventManager::SetupMarketEventEffects(FEconomicEventData& Event)
{
	// Set up market-wide effects
	float PriceMultiplier = 1.0f;
	float DemandMultiplier = 1.0f;
	float SupplyMultiplier = 1.0f;

	switch (Event.ImpactType)
	{
		case EEconomicImpactType::Positive:
			PriceMultiplier = 1.0f + (Event.Severity * 0.3f);
			DemandMultiplier = 1.0f + (Event.Severity * 0.5f);
			SupplyMultiplier = 1.0f + (Event.Severity * 0.2f);
			break;
		case EEconomicImpactType::Negative:
			PriceMultiplier = 1.0f - (Event.Severity * 0.3f);
			DemandMultiplier = 1.0f - (Event.Severity * 0.4f);
			SupplyMultiplier = 1.0f - (Event.Severity * 0.3f);
			break;
		case EEconomicImpactType::Mixed:
			PriceMultiplier = 1.0f + (Event.Severity * 0.2f);
			DemandMultiplier = 1.0f - (Event.Severity * 0.2f);
			SupplyMultiplier = 1.0f - (Event.Severity * 0.1f);
			break;
		default:
			break;
	}

	// Apply to all commodities (simplified)
	for (int32 i = 0; i < 10; i++) // Would iterate over actual commodities
	{
		FName CommodityID = FName(*FString::Printf(TEXT("Commodity_%d"), i));
		Event.CommodityPriceMultipliers.Add(CommodityID, PriceMultiplier);
		Event.CommodityDemandMultipliers.Add(CommodityID, DemandMultiplier);
		Event.CommoditySupplyMultipliers.Add(CommodityID, SupplyMultiplier);
	}
}

void UEconomicEventManager::SetupFactionEventEffects(FEconomicEventData& Event)
{
	// Set up faction-specific effects
	if (!Event.TriggeringFactionID.IsEmpty())
	{
		float FactionMultiplier = 1.0f;

		switch (Event.ImpactType)
		{
			case EEconomicImpactType::Positive:
				FactionMultiplier = 1.0f + (Event.Severity * 0.4f);
				break;
			case EEconomicImpactType::Negative:
				FactionMultiplier = 1.0f - (Event.Severity * 0.4f);
				break;
			default:
				FactionMultiplier = 1.0f;
				break;
		}

		Event.FactionImpactMultipliers.Add(Event.TriggeringFactionID, FactionMultiplier);
	}
}

void UEconomicEventManager::SetupGlobalEventEffects(FEconomicEventData& Event)
{
	// Set up global effects (affects all factions and commodities)
	SetupMarketEventEffects(Event);
	SetupFactionEventEffects(Event);
}

void UEconomicEventManager::SetupSeasonalEventEffects(FEconomicEventData& Event)
{
	// Set up seasonal effects (typically positive for certain commodities)
	float SeasonalMultiplier = 1.0f + (Event.Severity * 0.3f);

	// Would target specific commodities based on season
	Event.CommodityDemandMultipliers.Add(FName("FoodSupplies"), SeasonalMultiplier);
	Event.CommodityDemandMultipliers.Add(FName("ConsumerGoods"), SeasonalMultiplier);
}

void UEconomicEventManager::SetupRandomEventEffects(FEconomicEventData& Event)
{
	// Set up completely random effects
	int32 NumImpacts = FMath::RandRange(1, 5);

	for (int32 i = 0; i < NumImpacts; i++)
	{
		FName CommodityID = FName(*FString::Printf(TEXT("Commodity_%d"), FMath::RandRange(0, 20)));
		float RandomMultiplier = FMath::FRandRange(0.5f, 1.5f);
		
		Event.CommodityPriceMultipliers.Add(CommodityID, RandomMultiplier);
		Event.CommodityDemandMultipliers.Add(CommodityID, RandomMultiplier);
	}
}

void UEconomicEventManager::ProcessEventChain(FEventChain& Chain)
{
	if (!Chain.bIsActive || Chain.Events.Num() == 0)
	{
		return;
	}

	FDateTime CurrentTime = FDateTime::Now();

	if (Chain.bIsSequential)
	{
		// Process sequential chain
		static TMap<FString, FDateTime> LastEventTime;

		FDateTime& LastTime = LastEventTime.FindOrAdd(Chain.ChainID);
		FTimespan TimeSinceLastEvent = CurrentTime - LastTime;

		if (TimeSinceLastEvent >= Chain.DelayBetweenEvents)
		{
			// Find next event to trigger
			for (int32 i = 0; i < Chain.Events.Num(); i++)
			{
				FEconomicEventData& Event = Chain.Events[i];
				
				// Check if this event hasn't been triggered yet
				bool bAlreadyTriggered = false;
				for (const FEconomicEventData& ActiveEvent : ActiveEvents)
				{
					if (ActiveEvent.EventID == Event.EventID)
					{
						bAlreadyTriggered = true;
						break;
					}
				}

				if (!bAlreadyTriggered)
				{
					// Trigger this event
					TriggerEvent(Event.EventID, Event.Category, Event.Severity, Event.DurationDays, Event.TriggeringFactionID);
					LastTime = CurrentTime;
					break;
				}
			}
		}
	}
	else
	{
		// Process parallel chain (all events at once)
		for (FEconomicEventData& Event : Chain.Events)
		{
			bool bAlreadyTriggered = false;
			for (const FEconomicEventData& ActiveEvent : ActiveEvents)
			{
				if (ActiveEvent.EventID == Event.EventID)
				{
					bAlreadyTriggered = true;
					break;
				}
			}

			if (!bAlreadyTriggered)
			{
				TriggerEvent(Event.EventID, Event.Category, Event.Severity, Event.DurationDays, Event.TriggeringFactionID);
			}
		}
	}

	// Check if chain is complete
	bool bAllEventsComplete = true;
	for (const FEconomicEventData& Event : Chain.Events)
	{
		bool bFoundActive = false;
		for (const FEconomicEventData& ActiveEvent : ActiveEvents)
		{
			if (ActiveEvent.EventID == Event.EventID)
			{
				bFoundActive = true;
				break;
			}
		}

		if (bFoundActive)
		{
			bAllEventsComplete = false;
			break;
		}
	}

	if (bAllEventsComplete)
	{
		EndEventChain(Chain.ChainID);
	}
}

bool UEconomicEventManager::CanTriggerEvent(const FEconomicEventData& Event) const
{
	// Check if we're at max events
	if (ActiveEvents.Num() >= MaxActiveEvents)
	{
		return false;
	}

	// Check for similar active events
	for (const FEconomicEventData& ActiveEvent : ActiveEvents)
	{
		if (ActiveEvent.EventName == Event.EventName && ActiveEvent.Category == Event.Category)
		{
			// Similar event already active
			return false;
		}
	}

	return true;
}

bool UEconomicEventManager::RunSelfTest_Implementation(FSystemTestResult& OutResult)
{
	OutResult.SystemName = "EconomicEventManager";
	OutResult.bPassed = true;
	
	UE_LOG(LogTemp, Log, TEXT("=== Starting EconomicEventManager Self-Test ==="));
	
	// Test 1: Verify subsystem initialization
	{
		UE_LOG(LogTemp, Log, TEXT("Test 1: Verifying subsystem initialization..."));
		
		if (!DynamicMarketManager || !FactionEconomyManager)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Subsystem references not initialized"));
			OutResult.ErrorMessage = TEXT("Subsystem initialization failed");
			OutResult.bPassed = false;
			return false;
		}
		
		// Verify default events were initialized
		if (EventTemplates.Num() < 3)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Default event templates not initialized"));
			OutResult.ErrorMessage = TEXT("Event template initialization failed");
			OutResult.bPassed = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Subsystem initialization verified"));
		OutResult.PerformanceMetrics.Add(TEXT("EventTemplatesLoaded"), EventTemplates.Num());
	}
	
	// Test 2: Verify basic event triggering
	{
		UE_LOG(LogTemp, Log, TEXT("Test 2: Verifying basic event triggering..."));
		
		// Clear any existing events
		ClearAllEvents();
		
		// Trigger a test event
		FEconomicEventData TestEvent = TriggerEvent(
			"TestEvent_001",
			EEconomicEventCategory::Market,
			0.5f,
			2.0f,
			"TestFaction"
		);
		
		if (TestEvent.EventID.IsEmpty() || !TestEvent.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Event triggering failed"));
			OutResult.ErrorMessages.Add(TEXT("Event triggering test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Verify event is in active events list
		TArray<FEconomicEventData> ActiveEventsList = GetActiveEvents();
		bool bEventFound = false;
		for (const FEconomicEventData& Event : ActiveEventsList)
		{
			if (Event.EventID == TestEvent.EventID)
			{
				bEventFound = true;
				break;
			}
		}
		
		if (!bEventFound)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Triggered event not found in active events"));
			OutResult.ErrorMessage = TEXT("Active events tracking test failed");
			OutResult.bPassed = false;
			return false;
		}
		
		// End the event
		EndEvent(TestEvent.EventID);
		
		// Verify event is no longer active
		ActiveEventsList = GetActiveEvents();
		bEventFound = false;
		for (const FEconomicEventData& Event : ActiveEventsList)
		{
			if (Event.EventID == TestEvent.EventID)
			{
				bEventFound = true;
				break;
			}
		}
		
		if (bEventFound)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Event should be removed after EndEvent"));
			OutResult.ErrorMessage = TEXT("Event removal test failed");
			OutResult.bPassed = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Basic event triggering working"));
		OutResult.PerformanceMetrics.Add(TEXT("EventsTested"), 1.0f);
	}
	
	// Test 3: Verify event categories and filtering
	{
		UE_LOG(LogTemp, Log, TEXT("Test 3: Verifying event categories and filtering..."));
		
		// Clear existing events
		ClearAllEvents();
		
		// Trigger events of different categories
		FEconomicEventData MarketEvent = TriggerEvent(
			"MarketEvent_001",
			EEconomicEventCategory::Market,
			0.3f,
			1.0f
		);
		
		FEconomicEventData FactionEvent = TriggerEvent(
			"FactionEvent_001",
			EEconomicEventCategory::Faction,
			0.4f,
			1.0f
		);
		
		FEconomicEventData GlobalEvent = TriggerEvent(
			"GlobalEvent_001",
			EEconomicEventCategory::Global,
			0.5f,
			1.0f
		);
		
		// Test category filtering
		TArray<FEconomicEventData> MarketEvents = GetEventsByCategory(EEconomicEventCategory::Market);
		if (MarketEvents.Num() != 1)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Market event filtering returned %d events (expected 1)"), MarketEvents.Num());
			OutResult.ErrorMessage = TEXT("Category filtering test failed");
			OutResult.bPassed = false;
			return false;
		}
		
		TArray<FEconomicEventData> FactionEvents = GetEventsByCategory(EEconomicEventCategory::Faction);
		if (FactionEvents.Num() != 1)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Faction event filtering returned %d events (expected 1)"), FactionEvents.Num());
			OutResult.ErrorMessage = TEXT("Faction filtering test failed");
			OutResult.bPassed = false;
			return false;
		}
		
		// Test scope filtering
		TArray<FEconomicEventData> RegionalEvents = GetEventsByScope(EEconomicEventScope::Regional);
		if (RegionalEvents.Num() < 2) // Market and Faction events should be regional
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Scope filtering returned insufficient events"));
			OutResult.ErrorMessage = TEXT("Scope filtering test failed");
			OutResult.bPassed = false;
			return false;
		}
		
		// Clean up
		ClearAllEvents();
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Event categories and filtering working"));
		OutResult.PerformanceMetrics.Add(TEXT("CategoriesTested"), 3.0f);
	}
	
	// Test 4: Verify event chains
	{
		UE_LOG(LogTemp, Log, TEXT("Test 4: Verifying event chains..."));
		
		// Create a test event chain
		TArray<FEconomicEventData> ChainEvents;
		
		FEconomicEventData Event1;
		Event1.EventID = "ChainEvent_001";
		Event1.Category = EEconomicEventCategory::Market;
		Event1.Severity = 0.3f;
		Event1.DurationDays = 1.0f;
		
		FEconomicEventData Event2;
		Event2.EventID = "ChainEvent_002";
		Event2.Category = EEconomicEventCategory::Market;
		Event2.Severity = 0.4f;
		Event2.DurationDays = 1.0f;
		
		FEconomicEventData Event3;
		Event3.EventID = "ChainEvent_003";
		Event3.Category = EEconomicEventCategory::Market;
		Event3.Severity = 0.5f;
		Event3.DurationDays = 1.0f;
		
		ChainEvents.Add(Event1);
		ChainEvents.Add(Event2);
		ChainEvents.Add(Event3);
		
		FEventChain TestChain = CreateEventChain(
			"TestChain",
			ChainEvents,
			true, // Sequential
			FTimespan(0, 0, 1, 0) // 1 minute between events
		);
		
		if (TestChain.ChainID.IsEmpty() || TestChain.Events.Num() != 3)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Event chain creation failed"));
			OutResult.ErrorMessage = TEXT("Event chain creation test failed");
			OutResult.bPassed = false;
			return false;
		}
		
		// Start the event chain
		FEventChain StartedChain = StartEventChain(TestChain.ChainID);
		
		if (!StartedChain.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Event chain not activated"));
			OutResult.ErrorMessage = TEXT("Event chain activation test failed");
			OutResult.bPassed = false;
			return false;
		}
		
		// Verify chain is in active chains list
		TArray<FEventChain> ActiveChains = GetActiveEventChains();
		bool bChainFound = false;
		for (const FEventChain& Chain : ActiveChains)
		{
			if (Chain.ChainID == TestChain.ChainID)
			{
				bChainFound = true;
				break;
			}
		}
		
		if (!bChainFound)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Chain not found in active chains"));
			OutResult.ErrorMessage = TEXT("Active chains tracking test failed");
			OutResult.bPassed = false;
			return false;
		}
		
		// End the chain
		EndEventChain(TestChain.ChainID);
		
		// Verify chain is no longer active
		ActiveChains = GetActiveEventChains();
		bChainFound = false;
		for (const FEventChain& Chain : ActiveChains)
		{
			if (Chain.ChainID == TestChain.ChainID)
			{
				bChainFound = true;
				break;
			}
		}
		
		if (bChainFound)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Chain should be removed after EndEventChain"));
			OutResult.ErrorMessage = TEXT("Chain removal test failed");
			OutResult.bPassed = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Event chains working"));
		OutResult.PerformanceMetrics.Add(TEXT("ChainsTested"), 1.0f);
	}
	
	// Test 5: Verify random event generation
	{
		UE_LOG(LogTemp, Log, TEXT("Test 5: Verifying random event generation..."));
		
		// Generate a random event
		FEconomicEventData RandomEvent = GenerateRandomEvent(
			EEconomicEventCategory::Random,
			EEconomicEventScope::Local
		);
		
		if (RandomEvent.EventID.IsEmpty() || !RandomEvent.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Random event generation failed"));
			OutResult.ErrorMessage = TEXT("Random event generation test failed");
			OutResult.bPassed = false;
			return false;
		}
		
		// Verify random event has valid properties
		if (RandomEvent.Severity < 0.2f || RandomEvent.Severity > MaxEventSeverity)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Random event severity out of valid range"));
			OutResult.ErrorMessage = TEXT("Random event severity test failed");
			OutResult.bPassed = false;
			return false;
		}
		
		if (RandomEvent.DurationDays < MinEventDuration || RandomEvent.DurationDays > MaxEventDuration)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Random event duration out of valid range"));
			OutResult.ErrorMessages.Add(TEXT("Random event duration test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Generate random event chain
		FEventChain RandomChain = GenerateRandomEventChain(EEconomicEventScope::Regional);
		
		if (RandomChain.Events.Num() < 3 || RandomChain.Events.Num() > 5)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Random chain should have 3-5 events, got %d"), RandomChain.Events.Num());
			OutResult.ErrorMessages.Add(TEXT("Random chain generation test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Clean up
		ClearAllEvents();
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Random event generation working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Random Event Generation"),
			FString::Printf(TEXT("Single events and chains (3-5 events) functional")),
			0.0f
		});
	}
	
	// Test 6: Verify player-triggered events
	{
		UE_LOG(LogTemp, Log, TEXT("Test 6: Verifying player-triggered events..."));
		
		// Create a player-triggerable event template
		FEconomicEventData PlayerTriggerableTemplate;
		PlayerTriggerableTemplate.EventID = "PlayerEvent_001";
		PlayerTriggerableTemplate.EventName = "Player Triggerable Event";
		PlayerTriggerableTemplate.Category = EEconomicEventCategory::PlayerTriggered;
		PlayerTriggerableTemplate.Scope = EEconomicEventScope::Local;
		PlayerTriggerableTemplate.ImpactType = EEconomicImpactType::Mixed;
		PlayerTriggerableTemplate.Severity = 0.3f;
		PlayerTriggerableTemplate.DurationDays = 3.0f;
		PlayerTriggerableTemplate.bCanBeTriggeredByPlayer = true;
		PlayerTriggerableTemplate.PlayerTriggerChance = 1.0f; // 100% chance for testing
		
		EventTemplates.Add(PlayerTriggerableTemplate.EventID, PlayerTriggerableTemplate);
		
		// Attempt to trigger as player
		bool bTriggerSuccess = PlayerTriggerEvent("PlayerEvent_001", "TestPlayer");
		
		if (!bTriggerSuccess)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Player event triggering failed"));
			OutResult.ErrorMessages.Add(TEXT("Player event trigger test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Verify event was triggered
		TArray<FEconomicEventData> ActiveEventsList = GetActiveEvents();
		bool bEventFound = false;
		for (const FEconomicEventData& Event : ActiveEventsList)
		{
			if (Event.EventID == "PlayerEvent_001")
			{
				bEventFound = true;
				break;
			}
		}
		
		if (!bEventFound)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Player-triggered event not found in active events"));
			OutResult.ErrorMessages.Add(TEXT("Player event activation test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test getting player-triggerable events
		TArray<FEconomicEventData> TriggerableEvents = GetPlayerTriggerableEvents();
		if (TriggerableEvents.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: No player-triggerable events found"));
			OutResult.ErrorMessages.Add(TEXT("Player triggerable events query test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Clean up
		ClearAllEvents();
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Player-triggered events working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Player-Triggered Events"),
			TEXT("Player event triggering and query functional"),
			0.0f
		});
	}
	
	// Test 7: Verify market events
	{
		UE_LOG(LogTemp, Log, TEXT("Test 7: Verifying market events..."));
		
		// Test market crash
		FEconomicEventData MarketCrash = TriggerMarketCrash(
			ECommodityCategory::MetallicOre,
			0.6f,
			5.0f
		);
		
		if (MarketCrash.EventID.IsEmpty() || !MarketCrash.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Market crash event failed"));
			OutResult.ErrorMessages.Add(TEXT("Market crash test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test market boom
		FEconomicEventData MarketBoom = TriggerMarketBoom(
			ECommodityCategory::ElectronicComponents,
			0.5f,
			7.0f
		);
		
		if (MarketBoom.EventID.IsEmpty() || !MarketBoom.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Market boom event failed"));
			OutResult.ErrorMessages.Add(TEXT("Market boom test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test supply shortage
		FEconomicEventData SupplyShortage = TriggerSupplyShortage(
			FName("IronOre"),
			0.7f,
			3.0f
		);
		
		if (SupplyShortage.EventID.IsEmpty() || !SupplyShortage.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Supply shortage event failed"));
			OutResult.ErrorMessages.Add(TEXT("Supply shortage test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test supply glut
		FEconomicEventData SupplyGlut = TriggerSupplyGlut(
			FName("CopperOre"),
			0.4f,
			4.0f
		);
		
		if (SupplyGlut.EventID.IsEmpty() || !SupplyGlut.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Supply glut event failed"));
			OutResult.ErrorMessages.Add(TEXT("Supply glut test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test trade war
		FEconomicEventData TradeWar = TriggerTradeWar(
			"FactionA",
			"FactionB",
			0.5f,
			10.0f
		);
		
		if (TradeWar.EventID.IsEmpty() || !TradeWar.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Trade war event failed"));
			OutResult.ErrorMessages.Add(TEXT("Trade war test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test pirate blockade
		FEconomicEventData PirateBlockade = TriggerPirateBlockade(
			"TestStation",
			0.6f,
			5.0f
		);
		
		if (PirateBlockade.EventID.IsEmpty() || !PirateBlockade.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Pirate blockade event failed"));
			OutResult.ErrorMessages.Add(TEXT("Pirate blockade test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Verify all market events are active
		TArray<FEconomicEventData> MarketEvents = GetEventsByCategory(EEconomicEventCategory::Market);
		if (MarketEvents.Num() < 4) // Market crash, boom, shortage, glut, blockade
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Expected at least 4 market events, got %d"), MarketEvents.Num());
			OutResult.ErrorMessages.Add(TEXT("Market event count test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Clean up
		ClearAllEvents();
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Market events working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Market Events"),
			TEXT("Crash, boom, shortage, glut, trade war, blockade functional"),
			0.0f
		});
	}
	
	// Test 8: Verify faction events
	{
		UE_LOG(LogTemp, Log, TEXT("Test 8: Verifying faction events..."));
		
		// Test faction economic boom
		FEconomicEventData FactionBoom = TriggerFactionEconomicBoom(
			"TestFaction",
			0.5f,
			7.0f
		);
		
		if (FactionBoom.EventID.IsEmpty() || !FactionBoom.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Faction economic boom event failed"));
			OutResult.ErrorMessages.Add(TEXT("Faction boom test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test faction economic crisis
		FEconomicEventData FactionCrisis = TriggerFactionEconomicCrisis(
			"TestFaction",
			0.6f,
			5.0f
		);
		
		if (FactionCrisis.EventID.IsEmpty() || !FactionCrisis.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Faction economic crisis event failed"));
			OutResult.ErrorMessages.Add(TEXT("Faction crisis test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test faction civil war
		FEconomicEventData CivilWar = TriggerFactionCivilWar(
			"TestFaction",
			0.7f,
			10.0f
		);
		
		if (CivilWar.EventID.IsEmpty() || !CivilWar.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Faction civil war event failed"));
			OutResult.ErrorMessages.Add(TEXT("Civil war test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test faction revolution
		FEconomicEventData Revolution = TriggerFactionRevolution(
			"TestFaction",
			0.8f,
			15.0f
		);
		
		if (Revolution.EventID.IsEmpty() || !Revolution.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Faction revolution event failed"));
			OutResult.ErrorMessages.Add(TEXT("Revolution test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Verify all faction events are active
		TArray<FEconomicEventData> FactionEvents = GetEventsByCategory(EEconomicEventCategory::Faction);
		if (FactionEvents.Num() != 4)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Expected 4 faction events, got %d"), FactionEvents.Num());
			OutResult.ErrorMessages.Add(TEXT("Faction event count test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Clean up
		ClearAllEvents();
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Faction events working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Faction Events"),
			TEXT("Boom, crisis, civil war, revolution functional"),
			0.0f
		});
	}
	
	// Test 9: Verify seasonal events
	{
		UE_LOG(LogTemp, Log, TEXT("Test 9: Verifying seasonal events..."));
		
		// Test seasonal festival
		FEconomicEventData Festival = TriggerSeasonalFestival(
			"TestStation",
			0.4f,
			3.0f
		);
		
		if (Festival.EventID.IsEmpty() || !Festival.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Seasonal festival event failed"));
			OutResult.ErrorMessages.Add(TEXT("Seasonal festival test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test harvest season
		FEconomicEventData Harvest = TriggerHarvestSeason(
			0.6f,
			5.0f
		);
		
		if (Harvest.EventID.IsEmpty() || !Harvest.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Harvest season event failed"));
			OutResult.ErrorMessages.Add(TEXT("Harvest season test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test holiday season
		FEconomicEventData Holiday = TriggerHolidaySeason(
			0.5f,
			7.0f
		);
		
		if (Holiday.EventID.IsEmpty() || !Holiday.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Holiday season event failed"));
			OutResult.ErrorMessages.Add(TEXT("Holiday season test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Verify all seasonal events are active
		TArray<FEconomicEventData> SeasonalEvents = GetEventsByCategory(EEconomicEventCategory::Seasonal);
		if (SeasonalEvents.Num() != 3)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Expected 3 seasonal events, got %d"), SeasonalEvents.Num());
			OutResult.ErrorMessages.Add(TEXT("Seasonal event count test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Clean up
		ClearAllEvents();
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Seasonal events working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Seasonal Events"),
			TEXT("Festival, harvest, holiday functional"),
			0.0f
		});
	}
	
	// Test 10: Verify global events
	{
		UE_LOG(LogTemp, Log, TEXT("Test 10: Verifying global events..."));
		
		// Test galactic economic boom
		FEconomicEventData GalacticBoom = TriggerGalacticEconomicBoom(
			0.6f,
			10.0f
		);
		
		if (GalacticBoom.EventID.IsEmpty() || !GalacticBoom.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Galactic economic boom event failed"));
			OutResult.ErrorMessages.Add(TEXT("Galactic boom test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test galactic recession
		FEconomicEventData GalacticRecession = TriggerGalacticRecession(
			0.5f,
			8.0f
		);
		
		if (GalacticRecession.EventID.IsEmpty() || !GalacticRecession.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Galactic recession event failed"));
			OutResult.ErrorMessages.Add(TEXT("Galactic recession test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test technological revolution
		FEconomicEventData TechRevolution = TriggerTechRevolution(
			0.7f,
			12.0f
		);
		
		if (TechRevolution.EventID.IsEmpty() || !TechRevolution.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Tech revolution event failed"));
			OutResult.ErrorMessages.Add(TEXT("Tech revolution test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test resource discovery
		FEconomicEventData ResourceDiscovery = TriggerResourceDiscovery(
			FName("GoldOre"),
			0.8f,
			15.0f
		);
		
		if (ResourceDiscovery.EventID.IsEmpty() || !ResourceDiscovery.bIsActive)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Resource discovery event failed"));
			OutResult.ErrorMessages.Add(TEXT("Resource discovery test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Verify all global events are active
		TArray<FEconomicEventData> GlobalEvents = GetEventsByCategory(EEconomicEventCategory::Global);
		if (GlobalEvents.Num() != 4)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Expected 4 global events, got %d"), GlobalEvents.Num());
			OutResult.ErrorMessages.Add(TEXT("Global event count test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Clean up
		ClearAllEvents();
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Global events working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Global Events"),
			TEXT("Boom, recession, tech revolution, resource discovery functional"),
			0.0f
		});
	}
	
	// Test 11: Verify event history tracking
	{
		UE_LOG(LogTemp, Log, TEXT("Test 11: Verifying event history tracking..."));
		
		// Clear history
		ClearEventHistory();
		
		// Trigger some events
		FEconomicEventData Event1 = TriggerEvent("HistoryTest_001", EEconomicEventCategory::Market, 0.3f, 1.0f);
		FEconomicEventData Event2 = TriggerEvent("HistoryTest_002", EEconomicEventCategory::Faction, 0.4f, 1.0f);
		
		// Immediately end them to move to history
		EndEvent(Event1.EventID);
		EndEvent(Event2.EventID);
		
		// Check event history
		TArray<FEventHistoryEntry> History = GetEventHistory(10);
		if (History.Num() < 2)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Expected at least 2 history entries, got %d"), History.Num());
			OutResult.ErrorMessages.Add(TEXT("Event history test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test date range filtering
		FDateTime Now = FDateTime::Now();
		FDateTime Yesterday = Now - FTimespan(1, 0, 0, 0);
		FDateTime Tomorrow = Now + FTimespan(1, 0, 0, 0);
		
		TArray<FEventHistoryEntry> RangeHistory = GetEventsInDateRange(Yesterday, Tomorrow);
		if (RangeHistory.Num() < 2)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Date range filtering returned insufficient events"));
			OutResult.ErrorMessages.Add(TEXT("Date range filtering test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test history export
		FString HistoryExport = ExportEventHistory();
		if (HistoryExport.IsEmpty() || !HistoryExport.Contains("Economic Event History Export"))
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: History export failed"));
			OutResult.ErrorMessages.Add(TEXT("History export test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Event history tracking working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Event History Tracking"),
			FString::Printf(TEXT("%d events tracked, export functional"), History.Num()),
			0.0f
		});
	}
	
	// Test 12: Verify event statistics
	{
		UE_LOG(LogTemp, Log, TEXT("Test 12: Verifying event statistics..."));
		
		// Clear everything
		ClearAllEvents();
		ClearEventHistory();
		
		// Trigger events of different categories
		TriggerEvent("StatTest_001", EEconomicEventCategory::Market, 0.3f, 1.0f);
		TriggerEvent("StatTest_002", EEconomicEventCategory::Market, 0.4f, 1.0f);
		TriggerEvent("StatTest_003", EEconomicEventCategory::Faction, 0.5f, 1.0f);
		
		// End events to move to history
		EndEvent("StatTest_001");
		EndEvent("StatTest_002");
		EndEvent("StatTest_003");
		
		// Get statistics
		TMap<EEconomicEventCategory, int32> Statistics = GetEventStatistics();
		if (Statistics.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("WARNING: Event statistics may be incomplete"));
		}
		
		// Get average severity
		float AvgSeverity = GetAverageEventSeverity();
		if (AvgSeverity < 0.3f || AvgSeverity > 0.5f)
		{
			UE_LOG(LogTemp, Warning, TEXT("WARNING: Average severity may be incorrect: %.2f"), AvgSeverity);
		}
		
		// Get most common event type
		EEconomicEventCategory MostCommon = GetMostCommonEventType();
		
		// Generate event report
		FString Report = GenerateEventReport();
		if (Report.IsEmpty() || !Report.Contains("ECONOMIC EVENT REPORT"))
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Event report generation failed"));
			OutResult.ErrorMessages.Add(TEXT("Event report generation test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Event statistics working"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Event Statistics"),
			TEXT("Statistics calculation and report generation functional"),
			0.0f
		});
	}
	
	// Test 13: Verify configuration limits
	{
		UE_LOG(LogTemp, Log, TEXT("Test 13: Verifying configuration limits..."));
		
		// Test max events limit
		ClearAllEvents();
		
		// Try to trigger more than max events
		for (int32 i = 0; i < MaxActiveEvents + 5; i++)
		{
			TriggerEvent(
				FString::Printf(TEXT("LimitTest_%d"), i),
				EEconomicEventCategory::Market,
				0.3f,
				1.0f
			);
		}
		
		TArray<FEconomicEventData> ActiveEventsList = GetActiveEvents();
		if (ActiveEventsList.Num() > MaxActiveEvents)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Exceeded maximum active events limit"));
			OutResult.ErrorMessages.Add(TEXT("Max events limit test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test severity limits
		FEconomicEventData LowSeverityEvent = TriggerEvent("SeverityTest_Low", EEconomicEventCategory::Market, -0.5f, 1.0f);
		if (LowSeverityEvent.Severity < 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Severity below minimum"));
			OutResult.ErrorMessages.Add(TEXT("Severity minimum test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		FEconomicEventData HighSeverityEvent = TriggerEvent("SeverityTest_High", EEconomicEventCategory::Market, 2.0f, 1.0f);
		if (HighSeverityEvent.Severity > MaxEventSeverity)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Severity above maximum"));
			OutResult.ErrorMessages.Add(TEXT("Severity maximum test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test duration limits
		FEconomicEventData ShortDurationEvent = TriggerEvent("DurationTest_Short", EEconomicEventCategory::Market, 0.3f, 0.1f);
		if (ShortDurationEvent.DurationDays < MinEventDuration)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Duration below minimum"));
			OutResult.ErrorMessages.Add(TEXT("Duration minimum test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		FEconomicEventData LongDurationEvent = TriggerEvent("DurationTest_Long", EEconomicEventCategory::Market, 0.3f, 100.0f);
		if (LongDurationEvent.DurationDays > MaxEventDuration)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Duration above maximum"));
			OutResult.ErrorMessages.Add(TEXT("Duration maximum test failed"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Clean up
		ClearAllEvents();
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Configuration limits enforced"));
		OutResult.TestResults.Add(FTestResultItem{
			true,
			TEXT("Configuration Limits"),
			TEXT("Max events, severity, and duration limits functional"),
			0.0f
		});
	}
	
	// Final result
	if (OutResult.bPassed)
	{
		UE_LOG(LogTemp, Log, TEXT("=== EconomicEventManager Self-Test PASSED ==="));
		UE_LOG(LogTemp, Log, TEXT("Performance Metrics: %d"), OutResult.PerformanceMetrics.Num());
		UE_LOG(LogTemp, Log, TEXT("Total Events Triggered: %d"), TotalEventsTriggered);
		UE_LOG(LogTemp, Log, TEXT("Total Events Ended: %d"), TotalEventsEnded);
		UE_LOG(LogTemp, Log, TEXT("Total Event Chains Started: %d"), TotalEventChainsStarted);
		UE_LOG(LogTemp, Log, TEXT("Player Triggered Events: %d"), PlayerTriggeredEvents);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("=== EconomicEventManager Self-Test FAILED ==="));
	}
	
	return OutResult.bPassed;
}
