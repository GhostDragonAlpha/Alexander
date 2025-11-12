// Copyright (c) 2025 Alexander Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SystemModuleBase.generated.h"

// Forward declarations
class UEventBus;

/**
 * SystemModuleBase - The LEGO stud pattern
 * 
 * Every system in the project inherits from this base class. It provides
 * the 5 connection points (methods) that all systems must implement:
 * 
 * 1. InitializeModule() - Called when system starts up
 * 2. ShutdownModule() - Called when system shuts down  
 * 3. UpdateModule() - Called each frame with delta time
 * 4. GetSystemName() - Returns the system's name
 * 5. IsSystemHealthy() - Returns system's health status
 * 
 * This is the foundation of the LEGO Architecture. All systems follow this
 * exact same pattern, making them interchangeable and composable.
 * 
 * THE THREE LAWS (absolute rules):
 * 
 * LAW #1: No Direct Dependencies
 *   - NEVER #include another system's header
 *   - NEVER hold direct pointers to other systems
 *   - Use SystemRegistry::Get().FindSystem<T>() to find systems at runtime
 * 
 * LAW #2: Event-Driven Communication  
 *   - NEVER call another system's methods directly
 *   - ALWAYS communicate through EventBus
 *   - Publish events when something happens, subscribe to events you're interested in
 * 
 * LAW #3: Health Monitoring
 *   - ALWAYS check IsSystemHealthy() before doing work
 *   - ALWAYS validate your own state in IsSystemHealthy()
 *   - NEVER assume a system is ready - check first
 * 
 * Violation of these laws = CHAOS and CIRCULAR DEPENDENCIES
 * Compliance with these laws = ORDER and SCALABILITY
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class ALEXANDER_API USystemModuleBase : public UObject
{
	GENERATED_BODY()
	
public:
	// Called once when the system is first started
	UFUNCTION(BlueprintCallable, Category = "System Module")
	virtual void InitializeModule();
	
	// Called once when the system is shutting down
	UFUNCTION(BlueprintCallable, Category = "System Module")
	virtual void ShutdownModule();
	
	// Called every frame with the time elapsed since last frame
	UFUNCTION(BlueprintCallable, Category = "System Module")
	virtual void UpdateModule(float DeltaTime);
	
	// Get the name of this system
	UFUNCTION(BlueprintCallable, Category = "System Module")
	virtual FString GetSystemName() const;
	
	// Check if this system is healthy and ready to operate
	UFUNCTION(BlueprintCallable, Category = "System Module")
	virtual bool IsSystemHealthy() const;
	
protected:
	// Helper method to get the EventBus (follows LAW #1)
	UFUNCTION(BlueprintCallable, Category = "System Module")
	UEventBus* GetEventBus() const;
	
	// Helper method to publish events (follows LAW #2)
	UFUNCTION(BlueprintCallable, Category = "System Module")
	void PublishEvent(const FSystemEvent& Event);
	
	// Helper method for consistent logging
	UFUNCTION(BlueprintCallable, Category = "System Module")
	void LogSystemMessage(const FString& Message, bool bError = false) const;
	
	// System health status
	UPROPERTY(BlueprintReadOnly, Category = "System Module")
	bool bIsHealthy = false;
	
	// Initialization status
	UPROPERTY(BlueprintReadOnly, Category = "System Module")
	bool bIsInitialized = false;
};