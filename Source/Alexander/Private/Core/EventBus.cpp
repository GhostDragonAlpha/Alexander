// Copyright (c) 2025 Alexander Project. All rights reserved.

#include "Core/EventBus.h"
#include "Core/SystemModuleBase.h"
#include "Engine/World.h"

void UEventBus::InitializeEventBus()
{
	EventHistory.Empty();
	Subscribers.Empty();
	GlobalSubscribers.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("EventBus: Initialized"));
}

void UEventBus::ShutdownEventBus()
{
	ClearAllEvents();
	Subscribers.Empty();
	GlobalSubscribers.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("EventBus: Shut down"));
}

void UEventBus::PublishEvent(const FSystemEvent& Event)
{
	// Add to history
	EventHistory.Add(Event);
	
	// Keep history size manageable
	if (EventHistory.Num() > MAX_EVENT_HISTORY)
	{
		EventHistory.RemoveAt(0, EventHistory.Num() - MAX_EVENT_HISTORY);
	}
	
	// Notify global subscribers
	for (const auto& Callback : GlobalSubscribers)
	{
		if (Callback)
		{
			Callback(Event);
		}
	}
	
	// Notify type-specific subscribers
	if (Subscribers.Contains(Event.EventType))
	{
		for (const auto& Callback : Subscribers[Event.EventType])
		{
			if (Callback)
			{
				Callback(Event);
			}
		}
	}
	
	// Log important events
	if (!Event.EventType.StartsWith(TEXT("Tick")))
	{
		UE_LOG(LogTemp, Verbose, TEXT("EventBus: Published event '%s' from '%s'"), 
			*Event.EventType, *Event.SourceSystem);
	}
}

void UEventBus::SubscribeToEvent(const FString& EventType, TFunction<void(const FSystemEvent&)> Callback)
{
	if (!Subscribers.Contains(EventType))
	{
		Subscribers.Add(EventType, TArray<TFunction<void(const FSystemEvent&)>>());
	}
	
	Subscribers[EventType].Add(Callback);
	
	UE_LOG(LogTemp, Verbose, TEXT("EventBus: Subscribed to event type '%s'"), *EventType);
}

void UEventBus::SubscribeToAllEvents(TFunction<void(const FSystemEvent&)> Callback)
{
	GlobalSubscribers.Add(Callback);
	
	UE_LOG(LogTemp, Verbose, TEXT("EventBus: Subscribed to all events"));
}

TArray<FSystemEvent> UEventBus::GetEventsOfType(const FString& EventType, float SinceTime)
{
	TArray<FSystemEvent> MatchingEvents;
	
	for (const FSystemEvent& Event : EventHistory)
	{
		if (Event.EventType == EventType && Event.Timestamp >= SinceTime)
		{
			MatchingEvents.Add(Event);
		}
	}
	
	return MatchingEvents;
}

void UEventBus::ClearAllEvents()
{
	EventHistory.Empty();
	UE_LOG(LogTemp, Verbose, TEXT("EventBus: Cleared all events"));
}