// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Testing/TestStation.h"
#include "CelestialScalingTestStation.generated.h"

// Forward declarations
class UCelestialBodyComponent;
class UCelestialBodyRegistry;
class UScalingCalculator;
class UGravitySimulator;
class AActor;

/**
 * Celestial Scaling Test Station
 *
 * Tests:
 * 1. Scale Factor Validation - Test inverse-square law scaling at various distances
 * 2. Gravitational Force Measurement - Validate gravity calculations using F=GMm/r²
 * 3. VR Precision Tests - Test player at origin, universe translation, floating point precision
 * 4. Performance Stress Test - Spawn 50+ celestial bodies and measure performance
 * 5. Dynamic Scaling - Test real-time scale factor adjustments
 * 6. Multi-body Gravity - Test N-body gravity simulation
 * 7. Distance-based Culling - Verify bodies beyond threshold are culled
 * 8. Origin Recentering - Test VR origin recentering for precision
 * 9. LOD System - Verify level-of-detail transitions
 * 10. Registry Management - Test celestial body registration/unregistration
 *
 * Requirements: 3.1, 3.2, 3.3, 3.4, 3.5
 */
UCLASS(ClassGroup=(Testing), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UCelestialScalingTestStation : public UTestStation
{
	GENERATED_BODY()

public:
	UCelestialScalingTestStation();

protected:
	virtual void BeginPlay() override;
	virtual void RegisterTests_Implementation() override;

public:
	// Test configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Testing|Config")
	int32 StressTestBodyCount = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Testing|Config")
	TSubclassOf<AActor> TestCelestialBodyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Testing|Config")
	float GravityAccuracyTolerance = 0.01f; // 1% tolerance

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Testing|Config")
	float ScaleFactorTolerance = 0.001f; // 0.1% tolerance

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Testing|Config")
	float MaxAllowedFrameTime = 16.7f; // 60 FPS target (ms)

	// Test distances (in km converted to cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Testing|Config")
	TArray<float> TestDistances = {
		100000.0f,      // 1,000 km
		1000000.0f,     // 10,000 km
		10000000.0f,    // 100,000 km
		100000000.0f,   // 1,000,000 km
		1000000000.0f   // 10,000,000 km
	};

protected:
	// Test case implementations

	/** Test 1: Scale Factor Validation - Inverse Square Law */
	void Test_ScaleFactorValidation();

	/** Test 2: Gravitational Force Measurement - F=GMm/r² */
	void Test_GravitationalForceMeasurement();

	/** Test 3.1: VR Precision - Player at Origin */
	void Test_VRPrecision_PlayerAtOrigin();

	/** Test 3.2: VR Precision - Universe Translation */
	void Test_VRPrecision_UniverseTranslation();

	/** Test 3.3: VR Precision - Floating Point Accuracy */
	void Test_VRPrecision_FloatingPoint();

	/** Test 4: Performance Stress Test - Multiple Bodies */
	void Test_PerformanceStressTest();

	/** Test 5: Dynamic Scaling - Real-time Adjustments */
	void Test_DynamicScaling();

	/** Test 6: Multi-body Gravity - N-body Simulation */
	void Test_MultiBodyGravity();

	/** Test 7: Distance-based Culling */
	void Test_DistanceBasedCulling();

	/** Test 8: Origin Recentering */
	void Test_OriginRecentering();

	/** Test 9: LOD System - Level of Detail */
	void Test_LODSystem();

	/** Test 10: Registry Management */
	void Test_RegistryManagement();

	// Helper functions

	/**
	 * Spawn a test celestial body at a specific location
	 * @param Location - World location to spawn at
	 * @param Mass - Mass of the body in kg
	 * @param Radius - Radius of the body in km
	 * @return Spawned actor with celestial body component
	 */
	AActor* SpawnTestCelestialBody(const FVector& Location, float Mass, float Radius);

	/**
	 * Cleanup all spawned test bodies
	 */
	void CleanupTestBodies();

	/**
	 * Calculate expected scale factor using inverse square law
	 * @param Distance - Distance from reference point
	 * @param ReferenceDistance - Distance where scale = 1.0
	 * @return Expected scale factor
	 */
	float CalculateExpectedScaleFactor(float Distance, float ReferenceDistance) const;

	/**
	 * Calculate expected gravitational force using F=GMm/r²
	 * @param Mass1 - Mass of first body (kg)
	 * @param Mass2 - Mass of second body (kg)
	 * @param Distance - Distance between bodies (cm)
	 * @return Expected force magnitude in Newtons
	 */
	float CalculateExpectedGravity(float Mass1, float Mass2, float Distance) const;

	/**
	 * Measure frame time over a duration
	 * @param DurationSeconds - How long to measure
	 * @return Average frame time in milliseconds
	 */
	float MeasureAverageFrameTime(float DurationSeconds);

	/**
	 * Test if a value is within tolerance
	 * @param Actual - Actual measured value
	 * @param Expected - Expected value
	 * @param TolerancePercent - Tolerance as percentage (0.01 = 1%)
	 * @return True if within tolerance
	 */
	bool IsWithinTolerance(float Actual, float Expected, float TolerancePercent) const;

	/**
	 * Get distance from player to a location
	 * @param Location - Location to measure distance to
	 * @return Distance in cm
	 */
	float GetDistanceFromPlayer(const FVector& Location) const;

	/**
	 * Simulate universe translation (origin recentering)
	 * @param Offset - Offset to apply to all bodies
	 */
	void SimulateUniverseTranslation(const FVector& Offset);

	/**
	 * Count visible celestial bodies
	 * @return Number of bodies currently visible/active
	 */
	int32 CountVisibleBodies() const;

	/**
	 * Measure LOD level of a body at distance
	 * @param Body - Celestial body to check
	 * @param Distance - Distance from observer
	 * @return Current LOD level
	 */
	int32 MeasureLODLevel(UCelestialBodyComponent* Body, float Distance) const;

private:
	// References
	UPROPERTY()
	UCelestialBodyRegistry* BodyRegistry = nullptr;

	UPROPERTY()
	UScalingCalculator* ScalingCalculator = nullptr;

	UPROPERTY()
	UGravitySimulator* GravitySimulator = nullptr;

	// Spawned test objects
	UPROPERTY()
	TArray<AActor*> SpawnedTestBodies;

	// Test tracking data
	TArray<float> FrameTimeSamples;
	TArray<float> ScaleFactorSamples;
	TArray<FVector> GravityForceSamples;

	// Constants
	static constexpr double GravitationalConstant = 6.67430e-11; // m³/(kg·s²)
	static constexpr float KilometersToCentimeters = 100000.0f; // 1 km = 100,000 cm
	static constexpr float ReferenceDistance = 1000000.0f; // 10,000 km in cm
	static constexpr float MetersToUnrealUnits = 100.0f; // 1 m = 100 cm
};
