// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/CriticalSection.h"
#include "GravitySimulator.generated.h"

// Forward declarations
class UCelestialBodyComponent;
class UPrimitiveComponent;
class AActor;

/**
 * Gravity simulation mode
 */
UENUM(BlueprintType)
enum class EGravitySimulationMode : uint8
{
	/** Single dominant body (fastest) */
	SingleBody UMETA(DisplayName = "Single Body"),

	/** Multiple bodies with sphere of influence (realistic) */
	MultiBody UMETA(DisplayName = "Multi-Body"),

	/** Full N-body simulation (slowest, most accurate) */
	NBody UMETA(DisplayName = "N-Body"),

	/** Disabled */
	Disabled UMETA(DisplayName = "Disabled")
};

/**
 * Game instance subsystem for simulating gravitational forces
 * Handles multi-body gravitational calculations and physics integration
 * Network prediction with server validation
 */
UCLASS()
class ALEXANDER_API UGravitySimulator : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ========== Subsystem Lifecycle ==========

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== Gravitational Force Calculation ==========

	/**
	 * Calculate total gravitational force on a target actor from all celestial bodies
	 * @param Target - Actor to calculate force for
	 * @param TargetPosition - Position of the target (can differ from actor location)
	 * @return Total gravitational force vector in Newtons
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity")
	FVector CalculateTotalGravitationalForce(AActor* Target, const FVector& TargetPosition) const;

	/**
	 * Calculate gravitational force from a specific celestial body
	 * Uses F = G * (m1 * m2) / r²
	 * @param Body - Celestial body exerting gravity
	 * @param TargetPosition - Position of the target object
	 * @param TargetMass - Mass of the target object in kg
	 * @return Gravitational force vector in Newtons
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity")
	FVector CalculateGravityFromBody(UCelestialBodyComponent* Body, const FVector& TargetPosition, float TargetMass) const;

	/**
	 * Calculate gravitational acceleration (force per unit mass)
	 * @param Body - Celestial body exerting gravity
	 * @param TargetPosition - Position of the target object
	 * @return Gravitational acceleration vector in m/s²
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity")
	FVector CalculateGravitationalAcceleration(UCelestialBodyComponent* Body, const FVector& TargetPosition) const;

	/**
	 * Get the dominant gravitational body at a position
	 * Returns the body with the strongest gravitational influence
	 * @param Position - Position to query
	 * @return Dominant celestial body, or nullptr if none found
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity")
	UCelestialBodyComponent* GetDominantGravitationalBody(const FVector& Position) const;

	// ========== Physics Integration ==========

	/**
	 * Apply gravitational forces to an actor with physics simulation
	 * Integrates with Unreal's physics system
	 * @param Target - Actor to apply forces to
	 * @param DeltaTime - Time step for integration
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity")
	void ApplyGravityToActor(AActor* Target, float DeltaTime);

	/**
	 * Apply gravity to a primitive component
	 * @param Component - Component to apply gravity to
	 * @param DeltaTime - Time step for integration
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity")
	void ApplyGravityToComponent(UPrimitiveComponent* Component, float DeltaTime);

	/**
	 * Convert force in Newtons to Unreal force units
	 * Unreal uses different force scaling
	 * @param ForceInNewtons - Force in Newtons
	 * @param TargetMass - Mass of the target in kg
	 * @return Force in Unreal units
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity")
	FVector ConvertNewtonsToUnrealForce(const FVector& ForceInNewtons, float TargetMass) const;

	/**
	 * Clamp gravitational force to prevent extreme values
	 * @param Force - Force vector to clamp
	 * @param MaxGForce - Maximum G-force allowed
	 * @return Clamped force vector
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity")
	FVector ClampGravitationalForce(const FVector& Force, float MaxGForce) const;

	// ========== Sphere of Influence ==========

	/**
	 * Check if a position is within a body's gravitational influence radius
	 * @param Body - Celestial body to check
	 * @param Position - Position to test
	 * @param InfluenceRadius - Radius to check (0 = use body's calculated radius)
	 * @return True if within influence radius
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity")
	bool IsWithinInfluenceRadius(UCelestialBodyComponent* Body, const FVector& Position, float InfluenceRadius = 0.0f) const;

	/**
	 * Calculate sphere of influence radius for a body
	 * Uses Laplace sphere formula
	 * @param Body - Celestial body
	 * @return Sphere of influence radius in Unreal units
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity")
	float CalculateSphereOfInfluence(UCelestialBodyComponent* Body) const;

	/**
	 * Get all bodies influencing a specific position
	 * @param Position - Position to check
	 * @param MaxBodies - Maximum number of bodies to return
	 * @return Array of influencing bodies, sorted by influence strength
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity")
	TArray<UCelestialBodyComponent*> GetInfluencingBodies(const FVector& Position, int32 MaxBodies = 3) const;

	// ========== Configuration ==========

	/**
	 * Set the gravity simulation mode
	 * @param Mode - Simulation mode to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity")
	void SetSimulationMode(EGravitySimulationMode Mode);

	/**
	 * Get the current simulation mode
	 * @return Active simulation mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity")
	EGravitySimulationMode GetSimulationMode() const { return CurrentSimulationMode; }

	/**
	 * Set maximum G-force limit for safety
	 * @param MaxG - Maximum G-force allowed
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity")
	void SetMaxGForce(float MaxG) { MaxGForce = MaxG; }

	/**
	 * Enable or disable gravity simulation globally
	 * @param bEnabled - Whether to enable gravity
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity")
	void SetGravityEnabled(bool bEnabled) { bGravityEnabled = bEnabled; }

	/**
	 * Get whether gravity is enabled
	 * @return True if gravity simulation is active
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity")
	bool IsGravityEnabled() const { return bGravityEnabled; }

	// ========== Network Prediction ==========

	/**
	 * Server-side gravity calculation and validation
	 * Clients predict locally, server validates and corrects
	 * @param ActorID - ID of the actor being simulated
	 * @param ClientPosition - Client's predicted position
	 * @param ClientVelocity - Client's predicted velocity
	 * @param OutCorrectedPosition - Server-corrected position
	 * @param OutCorrectedVelocity - Server-corrected velocity
	 * @return True if correction was needed
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity|Network")
	bool ValidateClientPrediction(int32 ActorID, const FVector& ClientPosition, const FVector& ClientVelocity,
		FVector& OutCorrectedPosition, FVector& OutCorrectedVelocity);

	// ========== Debug ==========

	/**
	 * Enable debug visualization of gravitational forces
	 * @param bEnabled - Whether to enable debug drawing
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity|Debug")
	void SetDebugVisualization(bool bEnabled) { bEnableDebugVisualization = bEnabled; }

	/**
	 * Draw debug visualization for gravity forces on an actor
	 * @param Target - Actor to visualize forces for
	 * @param Duration - How long to display debug lines
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity|Debug")
	void DrawGravityDebug(AActor* Target, float Duration = 0.0f) const;

	/**
	 * Get simulation statistics
	 * @param OutCalculationsPerFrame - Number of gravity calculations per frame
	 * @param OutAverageCalculationTime - Average time per calculation in ms
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Gravity|Debug")
	void GetSimulationStatistics(int32& OutCalculationsPerFrame, float& OutAverageCalculationTime) const;

protected:
	// ========== Configuration Properties ==========

	/** Current gravity simulation mode */
	UPROPERTY(EditDefaultsOnly, Category = "Gravity")
	EGravitySimulationMode CurrentSimulationMode;

	/** Whether gravity simulation is enabled */
	UPROPERTY(EditDefaultsOnly, Category = "Gravity")
	bool bGravityEnabled;

	/** Maximum G-force limit to prevent extreme accelerations */
	UPROPERTY(EditDefaultsOnly, Category = "Gravity|Safety")
	float MaxGForce;

	/** Minimum distance for gravity calculations (prevents singularities) */
	UPROPERTY(EditDefaultsOnly, Category = "Gravity|Safety")
	float MinGravityDistance;

	/** Gravitational constant (G) in appropriate units */
	UPROPERTY(EditDefaultsOnly, Category = "Gravity|Physics")
	double GravitationalConstant;

	/** Scale factor for converting between real and game physics */
	UPROPERTY(EditDefaultsOnly, Category = "Gravity|Physics")
	float PhysicsScaleFactor;

	/** Enable automatic body discovery from CelestialBodyRegistry */
	UPROPERTY(EditDefaultsOnly, Category = "Gravity|Performance")
	bool bAutoDiscoverBodies;

	/** Maximum distance for gravity influence (optimization) */
	UPROPERTY(EditDefaultsOnly, Category = "Gravity|Performance")
	float MaxInfluenceDistance;

	/** Update frequency for gravity calculations (Hz) */
	UPROPERTY(EditDefaultsOnly, Category = "Gravity|Performance")
	float GravityUpdateFrequency;

	// ========== Debug ==========

	/** Enable debug visualization */
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bEnableDebugVisualization;

	/** Enable debug logging */
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bEnableDebugLogging;

	/** Color for debug force vectors */
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	FColor DebugForceColor;

	// ========== Statistics ==========

	/** Number of gravity calculations this frame */
	mutable int32 CalculationsThisFrame;

	/** Total calculation time this frame (ms) */
	mutable float TotalCalculationTime;

	/** Frame counter for statistics */
	mutable int32 FrameCounter;

	// ========== Thread Safety ==========

	/** Lock for thread-safe calculations */
	mutable FCriticalSection SimulationLock;

	// ========== Internal Methods ==========

	/** Calculate force using single-body mode */
	FVector CalculateSingleBodyGravity(const FVector& TargetPosition, float TargetMass) const;

	/** Calculate force using multi-body mode */
	FVector CalculateMultiBodyGravity(const FVector& TargetPosition, float TargetMass) const;

	/** Calculate force using N-body mode */
	FVector CalculateNBodyGravity(const FVector& TargetPosition, float TargetMass) const;

	/** Get all celestial bodies for simulation */
	TArray<UCelestialBodyComponent*> GetCelestialBodies() const;

	/** Calculate influence strength of a body at a position */
	float CalculateInfluenceStrength(UCelestialBodyComponent* Body, const FVector& Position) const;

	/** Validate and clamp force values */
	FVector ValidateForce(const FVector& Force, float TargetMass) const;
};
