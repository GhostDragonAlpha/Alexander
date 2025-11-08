// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrbitalBody.h"
#include "Asteroid.generated.h"

/**
 * Asteroid Actor - Individual asteroid in space
 * Inherits from AOrbitalBody for n-body physics integration
 * Part of the hybrid 5-tier asteroid field system
 *
 * PHASE 1 IMPLEMENTATION - Core Infrastructure Only
 * - Basic properties and structure
 * - Inheritance from AOrbitalBody
 * - Tier management stubs
 * - Collision handling stubs
 * - Object pooling support
 */
UCLASS(Blueprintable)
class ALEXANDER_API AAsteroid : public AOrbitalBody
{
	GENERATED_BODY()

public:
	AAsteroid();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ============================================================================
	// ASTEROID PROPERTIES
	// ============================================================================

	// Asteroid size category (affects physics tier eligibility)
	// In meters - typical range: 5m (small) to 500m (large)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid")
	float AsteroidRadius = 50.0f; // meters

	// Asteroid density (kg/m³) - affects Mass calculation
	// Typical asteroid densities: 2000-3500 kg/m³ (rocky composition)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid")
	float Density = 2500.0f; // Rock density

	// Unique ID for this asteroid (for network replication and tracking)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Asteroid")
	int32 AsteroidID = -1;

	// Current physics tier (0-4)
	// Tier 0: Critical (full n-body)
	// Tier 1: Active (simplified physics)
	// Tier 2: Distant (kinematic)
	// Tier 3: Far (minimal updates)
	// Tier 4: Procedural (GPU instanced)
	UPROPERTY(BlueprintReadOnly, Category = "Asteroid|Tier")
	int32 CurrentTier = 1;

	// Visual mesh variant index (for variety in asteroid appearance)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid|Visual")
	int32 MeshVariantIndex = 0;

	// ============================================================================
	// TIER MANAGEMENT
	// ============================================================================

	// Assign this asteroid to a physics tier
	UFUNCTION(BlueprintCallable, Category = "Asteroid|Tier")
	void SetPhysicsTier(int32 NewTier);

	// Get distance to player ship (cached for performance)
	UFUNCTION(BlueprintCallable, Category = "Asteroid|Tier")
	float GetDistanceToPlayer() const;

	// Update tier based on distance thresholds
	// Called by AsteroidFieldManager - do not call directly every frame
	UFUNCTION(BlueprintCallable, Category = "Asteroid|Tier")
	void UpdateTierBasedOnDistance(float DistanceToPlayer);

	// ============================================================================
	// PHYSICS UPDATES (Tier-Specific)
	// ============================================================================

	// Tier 0: Full n-body gravity simulation with all nearby objects
	void UpdateTier0_Critical(float DeltaTime);

	// Tier 1: Simplified physics - gravity from major bodies only
	void UpdateTier1_Active(float DeltaTime);

	// Tier 2: Kinematic motion - pre-calculated orbit path
	void UpdateTier2_Distant(float DeltaTime);

	// Tier 3: Minimal updates - position only, no physics
	void UpdateTier3_Far(float DeltaTime);

	// Tier 4: GPU instanced - no individual updates (managed by renderer)
	// No function needed - handled by AsteroidVFXManager

	// ============================================================================
	// COLLISION & FRAGMENTATION
	// ============================================================================

	// Handle collision with ship or other asteroid
	UFUNCTION()
	void OnAsteroidHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                   FVector NormalImpulse, const FHitResult& Hit);

	// Can this asteroid fragment? (based on size threshold)
	// Minimum size for fragmentation: 10m radius
	UFUNCTION(BlueprintCallable, Category = "Asteroid|Fragmentation")
	bool CanFragment() const;

	// Fragment into smaller asteroids
	// PHASE 1: Stub implementation - just destroys asteroid
	// PHASE 2: Will spawn fragment children
	UFUNCTION(BlueprintCallable, Category = "Asteroid|Fragmentation")
	void Fragment(FVector ImpactPoint, FVector ImpactVelocity);

	// Minimum radius for fragmentation (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid|Fragmentation")
	float MinFragmentRadius = 10.0f;

	// Number of fragments to spawn when fragmenting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid|Fragmentation")
	int32 FragmentCount = 3;

	// ============================================================================
	// POOLING SUPPORT
	// ============================================================================

	// Is this asteroid currently active?
	UPROPERTY(BlueprintReadOnly, Category = "Asteroid|Pooling")
	bool bIsActive = false;

	// Activate from pool
	UFUNCTION(BlueprintCallable, Category = "Asteroid|Pooling")
	void ActivateFromPool(FVector SpawnLocation, FVector InitialVelocity, float Size);

	// Deactivate back to pool
	UFUNCTION(BlueprintCallable, Category = "Asteroid|Pooling")
	void DeactivateToPool();

	// ============================================================================
	// DEBUG & VISUALIZATION
	// ============================================================================

	// Draw debug information for this asteroid
	UFUNCTION(BlueprintCallable, Category = "Asteroid|Debug")
	void DrawAsteroidDebug() const;

	// Show tier assignment visually
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid|Debug")
	bool bShowTierDebug = false;

	// Calculate mass from radius and density
	// Formula: M = ρ * V, where V = (4/3) * π * r³
	UFUNCTION(BlueprintCallable, Category = "Asteroid")
	void UpdateMassFromSize();

protected:

	// Apply tier-specific physics settings
	void ApplyTierPhysicsSettings();

	// Cached player ship reference (updated periodically by manager)
	UPROPERTY()
	class ASpaceship* CachedPlayerShip;

	// Last known distance to player (cached for tier transitions)
	UPROPERTY()
	float CachedDistanceToPlayer = 0.0f;

	// Time since last tier evaluation (for throttling)
	UPROPERTY()
	float TimeSinceLastTierCheck = 0.0f;

	// Tier check interval (seconds) - prevents every-frame distance checks
	UPROPERTY(EditAnywhere, Category = "Asteroid|Performance")
	float TierCheckInterval = 1.0f;

	// Color map for tier visualization
	static const FColor TierColors[5];
};
