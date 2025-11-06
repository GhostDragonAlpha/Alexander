// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Testing/TestStation.h"
#include "SpaceshipFlightTestStation.generated.h"

// Forward declarations
class ASpaceship;
class UFlightController;
class UCockpitComponent;

/**
 * Spaceship Flight Test Station
 *
 * Tests:
 * 1. Flight Controls - 6DOF movement, rotation, throttle control
 * 2. Physics Validation - Newtonian mechanics, momentum conservation
 * 3. Navigation - Waypoint following, distance accuracy
 * 4. Collision Detection - Obstacle avoidance, damage system
 * 5. VR Cockpit Integration - Cockpit interaction, instrument panels
 *
 * Requirements: 2.1, 2.2, 2.3, 2.4, 2.5
 */
UCLASS(ClassGroup=(Testing), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API USpaceshipFlightTestStation : public UTestStation
{
	GENERATED_BODY()

public:
	USpaceshipFlightTestStation();

protected:
	virtual void BeginPlay() override;
	virtual void RegisterTests_Implementation() override;

public:
	// Test configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Testing|Config")
	TSubclassOf<ASpaceship> TestSpaceshipClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Testing|Config")
	TArray<FVector> WaypointLocations = {
		FVector(10000, 0, 0),      // 100m
		FVector(100000, 0, 0),     // 1km
		FVector(1000000, 0, 0),    // 10km
		FVector(10000000, 0, 0)    // 100km
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Testing|Config")
	float PositionAccuracyTolerance = 100.0f; // cm

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Testing|Config")
	float VelocityAccuracyTolerance = 10.0f; // cm/s

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Testing|Config")
	float TestFlightDuration = 10.0f; // seconds

protected:
	// Test case implementations

	/** Test 1.1: Flight Controls - Basic Movement */
	void Test_BasicMovement();

	/** Test 1.2: Flight Controls - Rotation */
	void Test_RotationControl();

	/** Test 1.3: Flight Controls - Throttle */
	void Test_ThrottleControl();

	/** Test 1.4: Flight Controls - 6DOF (Six Degrees of Freedom) */
	void Test_SixDOF();

	/** Test 2.1: Physics - Newtonian Mechanics */
	void Test_NewtonianMechanics();

	/** Test 2.2: Physics - Momentum Conservation */
	void Test_MomentumConservation();

	/** Test 2.3: Physics - Inertial Dampening */
	void Test_InertialDampening();

	/** Test 3.1: Navigation - Waypoint Following (100m) */
	void Test_Waypoint_100m();

	/** Test 3.2: Navigation - Waypoint Following (1km) */
	void Test_Waypoint_1km();

	/** Test 3.3: Navigation - Waypoint Following (10km) */
	void Test_Waypoint_10km();

	/** Test 3.4: Navigation - Waypoint Following (100km) */
	void Test_Waypoint_100km();

	/** Test 4.1: Collision - Obstacle Detection */
	void Test_ObstacleDetection();

	/** Test 4.2: Collision - Collision Response */
	void Test_CollisionResponse();

	/** Test 5.1: VR Cockpit - Instrument Interaction */
	void Test_CockpitInteraction();

	/** Test 5.2: VR Cockpit - Control Panel */
	void Test_ControlPanel();

	// Helper functions
	bool SpawnTestSpaceship();
	void CleanupTestSpaceship();
	bool NavigateToWaypoint(const FVector& Waypoint, float TimeoutSeconds);
	float MeasureVelocity();
	FVector MeasureAcceleration(float DeltaTime);
	bool TestPhysicsResponse(const FVector& InitialVelocity, float Duration);

private:
	// References
	UPROPERTY()
	ASpaceship* TestSpaceship = nullptr;

	UPROPERTY()
	UFlightController* FlightController = nullptr;

	UPROPERTY()
	UCockpitComponent* CockpitComponent = nullptr;

	// Test tracking data
	TArray<FVector> PositionHistory;
	TArray<FVector> VelocityHistory;
	FVector LastPosition;
	float LastPositionTime;
};
