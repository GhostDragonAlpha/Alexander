// Copyright (c) 2025 Alexander Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EventBus.generated.h"

// Forward declarations
class USystemModuleBase;

/**
 * Event data structure for system communication
 * This is how systems talk to each other without direct dependencies
 */
USTRUCT(BlueprintType)
struct FSystemEvent
{
	GENERATED_BODY()
	
public:
	/** Event type identifier (e.g., "ThrustApplied", "ResourceCollected") */
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FString EventType;
	
	/** Source system that triggered the event */
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FString SourceSystem;
	
	/** Target system (optional, empty means broadcast to all) */
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FString TargetSystem;
	
	/** Event data as JSON string for flexibility */
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FString EventData;
	
	/** Timestamp when the event was created */
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	float Timestamp;
	
	FSystemEvent()
	{
		Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	}
};

/**
 * EventBus - The connector tubes between LEGO pieces
 * 
 * Systems NEVER call each other directly. They publish events here,
 * and other systems subscribe to events they're interested in.
 * 
 * This is LAW #2 in action: All communication goes through the EventBus.
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API UEventBus : public UObject
{
	GENERATED_BODY()
	
public:
	// Initialize the EventBus
	UFUNCTION(BlueprintCallable, Category = "Event Bus")
	void InitializeEventBus();
	
	// Shutdown the EventBus
	UFUNCTION(BlueprintCallable, Category = "Event Bus")
	void ShutdownEventBus();
	
	/**
	 * Publish an event to the bus
	 * @param Event - The event to publish
	 */
	UFUNCTION(BlueprintCallable, Category = "Event Bus")
	void PublishEvent(const FSystemEvent& Event);
	
	/**
	 * Subscribe to a specific event type
	 * @param EventType - Type of event to listen for
	 * @param Callback - Function to call when event occurs
	 */
	void SubscribeToEvent(const FString& EventType, TFunction<void(const FSystemEvent&)> Callback);
	
	/**
	 * Subscribe to all events (use sparingly)
	 * @param Callback - Function to call for any event
	 */
	void SubscribeToAllEvents(TFunction<void(const FSystemEvent&)> Callback);
	
	/**
	 * Get all events of a specific type since a given time
	 * @param EventType - Type of events to retrieve
	 * @param SinceTime - Only get events after this time
	 * @return Array of matching events
	 */
	UFUNCTION(BlueprintCallable, Category = "Event Bus")
	TArray<FSystemEvent> GetEventsOfType(const FString& EventType, float SinceTime = 0.0f);
	
	/** Clear all stored events */
	UFUNCTION(BlueprintCallable, Category = "Event Bus")
	void ClearAllEvents();
	
	/** Get the number of active subscribers */
	UFUNCTION(BlueprintCallable, Category = "Event Bus")
	int32 GetSubscriberCount() const { return Subscribers.Num(); }
	
	/** Get the number of events in history */
	UFUNCTION(BlueprintCallable, Category = "Event Bus")
	int32 GetEventHistoryCount() const { return EventHistory.Num(); }
	
protected:
	// Stored event history for debugging and replay
	UPROPERTY()
	TArray<FSystemEvent> EventHistory;
	
	// Active subscribers (C++ only, not exposed to Blueprint)
	TMap<FString, TArray<TFunction<void(const FSystemEvent&)>>> Subscribers;
	
	// Global subscribers that listen to all events
	TArray<TFunction<void(const FSystemEvent&)>> GlobalSubscribers;
	
	// Maximum number of events to keep in history
	static constexpr int32 MAX_EVENT_HISTORY = 1000;
};