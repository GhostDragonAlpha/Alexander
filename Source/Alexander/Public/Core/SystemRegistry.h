// Copyright (c) 2025 Alexander Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SystemRegistry.generated.h"

// Forward declarations
class USystemModuleBase;
class UEventBus;

/**
 * SystemRegistry - The LEGO baseplate
 * 
 * This manages all system modules in the project. Systems register themselves here,
 * and other systems can find them here. This is the central hub that prevents
 * direct dependencies between systems.
 * 
 * This is LAW #1 in action: No system knows about any other system directly.
 * They all go through the registry to find each other at runtime.
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API USystemRegistry : public UObject
{
	GENERATED_BODY()
	
public:
	// Get the singleton instance
	UFUNCTION(BlueprintCallable, Category = "System Registry", meta = (WorldContext = "WorldContextObject"))
	static USystemRegistry* Get(const UObject* WorldContextObject);
	
	/**
	 * Register a system module with the registry
	 * @param Module - The module to register
	 * @return true if registration succeeded, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "System Registry")
	bool RegisterModule(USystemModuleBase* Module);
	
	/**
	 * Unregister a system module
	 * @param ModuleName - Name of the module to unregister
	 * @return true if unregistration succeeded, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "System Registry")
	bool UnregisterModule(const FString& ModuleName);
	
	/**
	 * Find a registered module by name
	 * @param ModuleName - Name of the module to find
	 * @return The module if found, nullptr otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "System Registry")
	USystemModuleBase* GetModule(const FString& ModuleName) const;
	
	/**
	 * Find a registered module by class type
	 * Template version for type-safe access
	 */
	template<typename T>
	T* GetModule() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, USystemModuleBase>::Value, 
			"T must be a USystemModuleBase or derived class");
		
		FString ModuleName = T::StaticClass()->GetName();
		return Cast<T>(GetModule(ModuleName));
	}
	
	/**
	 * Get all registered module names
	 * @return Array of all registered module names
	 */
	UFUNCTION(BlueprintCallable, Category = "System Registry")
	TArray<FString> GetAllModuleNames() const;
	
	/**
	 * Get the number of registered modules
	 * @return Count of registered modules
	 */
	UFUNCTION(BlueprintCallable, Category = "System Registry")
	int32 GetModuleCount() const { return RegisteredModules.Num(); }
	
	/**
	 * Check if a module is registered
	 * @param ModuleName - Name of the module to check
	 * @return true if module is registered, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "System Registry")
	bool IsModuleRegistered(const FString& ModuleName) const;
	
	/**
	 * Initialize all registered modules
	 * Call this once at game start
	 */
	UFUNCTION(BlueprintCallable, Category = "System Registry")
	void InitializeAllModules();
	
	/**
	 * Shutdown all registered modules in reverse order
	 * Call this at game end
	 */
	UFUNCTION(BlueprintCallable, Category = "System Registry")
	void ShutdownAllModules();
	
	/**
	 * Update all registered modules
	 * Call this each frame
	 * @param DeltaTime - Time since last frame
	 */
	UFUNCTION(BlueprintCallable, Category = "System Registry")
	void UpdateAllModules(float DeltaTime);
	
	/**
	 * Get the health status of all modules
	 * @return Map of module names to health status
	 */
	UFUNCTION(BlueprintCallable, Category = "System Registry")
	TMap<FString, bool> GetAllModuleHealth() const;
	
	/**
	 * Get the EventBus instance
	 * @return The EventBus
	 */
	UFUNCTION(BlueprintCallable, Category = "System Registry")
	UEventBus* GetEventBus() const { return EventBus; }
	
protected:
	// Map of registered modules (the LEGO pieces on the baseplate)
	UPROPERTY()
	TMap<FString, TWeakObjectPtr<USystemModuleBase>> RegisteredModules;
	
	// The EventBus for inter-system communication
	UPROPERTY()
	UEventBus* EventBus = nullptr;
	
	// Order of registration (for proper shutdown sequence)
	UPROPERTY()
	TArray<FString> RegistrationOrder;
};