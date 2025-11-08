// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AsteroidFieldManager.generated.h"

// Forward declarations
class AAsteroid;
class ASpaceship;

/**
 * Asteroid Field Manager - Master coordinator for asteroid field system
 *
 * PHASE 1 IMPLEMENTATION - Core Infrastructure Only
 *
 * Responsibilities:
 * - Spawn and manage asteroid population
 * - Track all active asteroids
 * - Update asteroid tiers based on player distance (Phase 2)
 * - Manage object pooling (Phase 2)
 * - Performance monitoring (Phase 2)
 *
 * Architecture:
 * - Uses 5-tier system for performance scaling
 * - Integrates with AOrbitalBody n-body physics system
 * - Supports network replication for multiplayer
 *
 * Performance Target:
 * - Support 1000+ asteroids at 60 FPS
 * - 2.67ms CPU budget for entire asteroid system
 */
UCLASS(Blueprintable)
class ALEXANDER_API AAsteroidFieldManager : public AActor
{
	GENERATED_BODY()

public:
	AAsteroidFieldManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ============================================================================
	// FIELD SPAWNING
	// ============================================================================

	// Spawn initial asteroid field
	// Creates asteroids in a spherical distribution around spawn center
	UFUNCTION(BlueprintCallable, Category = "AsteroidField")
	void SpawnAsteroidField(int32 AsteroidCount, float FieldRadius);

	// Spawn single asteroid at specific location
	UFUNCTION(BlueprintCallable, Category = "AsteroidField")
	AAsteroid* SpawnAsteroid(FVector Location, float Radius, FVector InitialVelocity);

	// Clear all asteroids from field
	UFUNCTION(BlueprintCallable, Category = "AsteroidField")
	void ClearAsteroidField();

	// PHASE 1 TEST FUNCTION: Spawn grid of test asteroids
	// Spawns a simple 3x3x3 grid for integration testing
	UFUNCTION(BlueprintCallable, Category = "AsteroidField|Test")
	void SpawnTestGrid(int32 GridSize = 3, float Spacing = 10000.0f);

	// ============================================================================
	// TIER MANAGEMENT
	// ============================================================================

	// Update all asteroid tiers based on player distance
	// Called periodically (not every frame) for performance
	UFUNCTION(BlueprintCallable, Category = "AsteroidField|Tier")
	void UpdateAsteroidTiers();

	// Get asteroids in specific tier
	UFUNCTION(BlueprintCallable, Category = "AsteroidField|Tier")
	TArray<AAsteroid*> GetAsteroidsInTier(int32 Tier) const;

	// Get tier statistics
	UFUNCTION(BlueprintCallable, Category = "AsteroidField|Tier")
	void GetTierStats(int32& OutTier0, int32& OutTier1, int32& OutTier2, int32& OutTier3, int32& OutTier4) const;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Maximum number of active asteroids
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Spawning")
	int32 MaxActiveAsteroids = 1000;

	// Field radius (meters) - asteroids spawn within this sphere
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Spawning")
	float FieldRadius = 100000.0f; // 100 km

	// Center point of asteroid field
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Spawning")
	FVector FieldCenter = FVector::ZeroVector;

	// Minimum asteroid radius (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Spawning")
	float MinAsteroidRadius = 5.0f;

	// Maximum asteroid radius (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Spawning")
	float MaxAsteroidRadius = 500.0f;

	// Asteroid density distribution (kg/m�)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Spawning")
	float MinDensity = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Spawning")
	float MaxDensity = 3500.0f;

	// Initial velocity range (m/s) for spawned asteroids
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Spawning")
	float MinInitialVelocity = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Spawning")
	float MaxInitialVelocity = 100.0f;

	// ============================================================================
	// TIER THRESHOLDS
	// ============================================================================

	// Distance thresholds for tier assignment (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Tiers")
	float Tier0Threshold = 1000.0f;    // 0-1km: Critical (full n-body)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Tiers")
	float Tier1Threshold = 50000.0f;   // 1-50km: Active (simplified physics)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Tiers")
	float Tier2Threshold = 200000.0f;  // 50-200km: Distant (kinematic)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Tiers")
	float Tier3Threshold = 500000.0f;  // 200-500km: Far (minimal updates)
	// Beyond 500km: Tier 4 (Procedural/GPU instanced)

	// How often to update tiers (seconds) - performance optimization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Performance")
	float TierUpdateInterval = 1.0f;

	// ============================================================================
	// RUNTIME STATE
	// ============================================================================

	// All active asteroids in the field
	UPROPERTY(BlueprintReadOnly, Category = "AsteroidField")
	TArray<AAsteroid*> ActiveAsteroids;

	// Asteroids organized by tier (for efficient updates)
	UPROPERTY(BlueprintReadOnly, Category = "AsteroidField|Tier")
	TArray<AAsteroid*> Tier0Asteroids; // Critical

	UPROPERTY(BlueprintReadOnly, Category = "AsteroidField|Tier")
	TArray<AAsteroid*> Tier1Asteroids; // Active

	UPROPERTY(BlueprintReadOnly, Category = "AsteroidField|Tier")
	TArray<AAsteroid*> Tier2Asteroids; // Distant

	UPROPERTY(BlueprintReadOnly, Category = "AsteroidField|Tier")
	TArray<AAsteroid*> Tier3Asteroids; // Far

	UPROPERTY(BlueprintReadOnly, Category = "AsteroidField|Tier")
	TArray<AAsteroid*> Tier4Asteroids; // Procedural

	// Reference to player ship (cached for distance calculations)
	UPROPERTY()
	ASpaceship* CachedPlayerShip;

	// Next asteroid ID (for unique identification)
	UPROPERTY()
	int32 NextAsteroidID = 0;

	// Time since last tier update
	UPROPERTY()
	float TimeSinceLastTierUpdate = 0.0f;

	// ============================================================================
	// PERFORMANCE MONITORING
	// ============================================================================

	// Enable performance profiling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Performance")
	bool bEnablePerformanceProfiling = false;

	// Last frame time for asteroid system (ms)
	UPROPERTY(BlueprintReadOnly, Category = "Debug|Performance")
	float LastFrameTimeMS = 0.0f;

	// Average frame time over last second (ms)
	UPROPERTY(BlueprintReadOnly, Category = "Debug|Performance")
	float AverageFrameTimeMS = 0.0f;

	// ============================================================================
	// DEBUG VISUALIZATION
	// ============================================================================

	// Show debug visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebug = false;

	// Show tier boundaries
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowTierBoundaries = false;

	// Draw debug information for field
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DrawFieldDebug() const;

protected:
	// Generate random position within field sphere
	FVector GenerateRandomPositionInField() const;

	// Generate random velocity for asteroid
	FVector GenerateRandomVelocity() const;

	// Generate random size for asteroid (biased toward smaller asteroids)
	float GenerateRandomSize() const;

	// Generate random density for asteroid
	float GenerateRandomDensity() const;

	// Update tier arrays (reorganize asteroids by tier)
	void UpdateTierArrays();

	// Cache player ship reference
	void CachePlayerShip();
};
