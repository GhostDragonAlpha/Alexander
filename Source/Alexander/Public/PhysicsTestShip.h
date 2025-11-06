// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrbitalBody.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsTestShip.generated.h"

// Forward declarations
class UFlightController;

/**
 * Physics Test Ship - Orbital Body for HTTP API Testing
 *
 * Inherits from AOrbitalBody to integrate with custom gravity/scaling system.
 * All objects in the game are orbital bodies - even when on ground.
 * No blueprint dependencies - pure C++ workflow.
 *
 * Features:
 * - Inherits physics-enabled MeshComponent from AOrbitalBody
 * - Integrates with UOrbitalMechanics for custom gravity
 * - Optional FlightController component integration
 * - Can be spawned via HTTP API: /Script/Alexander.PhysicsTestShip
 *
 * This ship is designed for "Unreal Engine as compiler" workflow:
 * Write C++ → Compile → Launch → Test via HTTP API
 *
 * IMPORTANT: Uses custom orbital mechanics system for gravity, not Unreal's built-in physics gravity.
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API APhysicsTestShip : public AOrbitalBody
{
	GENERATED_BODY()

public:
	APhysicsTestShip();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ============================================================================
	// FLIGHT CONTROL (Optional)
	// ============================================================================

	/**
	 * Flight controller for advanced flight assists
	 * If present, HTTP API will use SetThrustInput()
	 * If absent, HTTP API will use direct physics forces
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight")
	UFlightController* FlightController;

	// ============================================================================
	// PHYSICS CONTROL METHODS
	// ============================================================================

	/**
	 * Apply thrust force in local space
	 * Called by HTTP API or blueprint
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics")
	void ApplyThrust(FVector ThrustInput);

	/**
	 * Apply torque for rotation
	 * Called by HTTP API or blueprint
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics")
	void ApplyTorque(FVector TorqueInput);

	/**
	 * Get current velocity from OrbitalBody
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics")
	FVector GetCurrentVelocity() const;

	/**
	 * Get current speed (magnitude of velocity)
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics")
	float GetCurrentSpeed() const;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Enable FlightController component? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
	bool bEnableFlightController;

protected:
	/**
	 * Configure physics settings on MeshComponent (inherited from AOrbitalBody)
	 * Called in constructor
	 */
	void ConfigurePhysics();

private:
	/** Force multiplier for thrust (Newtons per unit input) */
	float ThrustForceMagnitude;

	/** Torque multiplier for rotation (Newton-meters per unit input) */
	float TorqueMagnitude;
};
