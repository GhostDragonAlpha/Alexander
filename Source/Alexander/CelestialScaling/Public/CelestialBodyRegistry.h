// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HAL/CriticalSection.h"
#include "CelestialBodyRegistry.generated.h"

// Forward declarations
class UCelestialBodyComponent;

/**
 * World subsystem for managing celestial body registration and tracking
 * Provides centralized registry for all celestial bodies in the game world
 * Thread-safe for access from multiple components
 */
UCLASS()
class ALEXANDER_API UCelestialBodyRegistry : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// ========== Subsystem Lifecycle ==========

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// ========== Body Registration (Thread-Safe) ==========

	/**
	 * Register a celestial body with the registry
	 * Thread-safe for registration during initialization
	 * @param Body - The celestial body component to register
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Registry")
	void RegisterCelestialBody(UCelestialBodyComponent* Body);

	/**
	 * Unregister a celestial body from the registry
	 * Thread-safe for cleanup during destruction
	 * @param Body - The celestial body component to unregister
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Registry")
	void UnregisterCelestialBody(UCelestialBodyComponent* Body);

	// ========== Body Lookup ==========

	/**
	 * Get all registered celestial bodies
	 * @return Array of all celestial bodies in the world
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Registry")
	TArray<UCelestialBodyComponent*> GetAllCelestialBodies() const;

	/**
	 * Find a celestial body by name
	 * @param BodyName - Name of the body to find
	 * @return The celestial body component, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Registry")
	UCelestialBodyComponent* FindBodyByName(FName BodyName) const;

	/**
	 * Get bodies within a specific distance from a reference point
	 * @param ReferencePoint - Center point for distance calculation
	 * @param MaxDistance - Maximum distance in Unreal units
	 * @return Array of bodies within range
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Registry")
	TArray<UCelestialBodyComponent*> GetBodiesInRange(const FVector& ReferencePoint, float MaxDistance) const;

	/**
	 * Get the N nearest bodies to a reference point
	 * @param ReferencePoint - Point to measure distance from
	 * @param Count - Number of nearest bodies to return
	 * @return Array of nearest bodies, sorted by distance
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Registry")
	TArray<UCelestialBodyComponent*> GetNearestBodies(const FVector& ReferencePoint, int32 Count) const;

	// ========== Universe Translation (for PlayerOriginManager) ==========

	/**
	 * Update all body positions by a translation offset
	 * Called by PlayerOriginManager when shifting the world origin
	 * @param OffsetDelta - The offset to apply to all bodies
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Registry")
	void UpdateAllBodyPositions(const FVector& OffsetDelta);

	/**
	 * Update all body scales based on player position
	 * Implements inverse-square law scaling for distant objects
	 * @param PlayerPosition - Current player position for distance calculations
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Registry")
	void UpdateAllBodyScales(const FVector& PlayerPosition);

	// ========== Optimization ==========

	/**
	 * Sort registered bodies by distance from a reference point
	 * Useful for culling and LOD calculations
	 * @param ReferencePoint - Point to measure distance from
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Registry")
	void SortBodiesByDistance(const FVector& ReferencePoint);

	/**
	 * Enable or disable automatic position updates
	 * @param bEnabled - Whether to enable automatic updates
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Registry")
	void SetAutoUpdateEnabled(bool bEnabled) { bAutoUpdateEnabled = bEnabled; }

	/**
	 * Get whether automatic updates are enabled
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Registry")
	bool IsAutoUpdateEnabled() const { return bAutoUpdateEnabled; }

	// ========== Statistics ==========

	/**
	 * Get the total number of registered bodies
	 * @return Number of registered celestial bodies
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Registry")
	int32 GetRegisteredBodyCount() const { return RegisteredBodies.Num(); }

	/**
	 * Clear all registered bodies
	 * WARNING: Only use during world cleanup
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Registry")
	void ClearAllBodies();

	// ========== Network Replication ==========

	/**
	 * Multicast RPC to update body positions on all clients
	 * @param OffsetDelta - The offset to apply
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastUpdateBodyPositions(const FVector& OffsetDelta);

protected:
	// ========== Internal State ==========

	/** All registered celestial bodies */
	UPROPERTY()
	TArray<UCelestialBodyComponent*> RegisteredBodies;

	/** Name-based lookup map for fast queries */
	UPROPERTY()
	TMap<FName, UCelestialBodyComponent*> BodyLookup;

	/** Thread-safety lock for registration operations */
	mutable FCriticalSection RegistryLock;

	/** Whether automatic updates are enabled */
	UPROPERTY()
	bool bAutoUpdateEnabled;

	/** Update frequency for automatic scaling (seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "Celestial|Registry")
	float AutoUpdateFrequency;

	/** Time since last automatic update */
	float TimeSinceLastUpdate;

	// ========== Configuration ==========

	/** Maximum number of bodies to process per frame for updates */
	UPROPERTY(EditDefaultsOnly, Category = "Celestial|Registry|Performance")
	int32 MaxBodiesPerFrame;

	/** Enable debug logging for registration operations */
	UPROPERTY(EditDefaultsOnly, Category = "Celestial|Registry|Debug")
	bool bEnableDebugLogging;

	// ========== Internal Methods ==========

	/** Validate a body component before registration */
	bool IsValidBodyComponent(UCelestialBodyComponent* Body) const;

	/** Log registry statistics (for debugging) */
	void LogRegistryStatistics() const;
};
