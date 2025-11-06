// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Testing/TestStation.h"
#include "PlanetLandingTestStation.generated.h"

// Forward declarations
class APlanet;
class APlanetaryLandingZone;
class ALandingPad;
class UAtmosphericEntryEffects;
class ULandingDetectionComponent;
class ASpaceship;

/**
 * Planet Landing Test Station
 *
 * Tests:
 * 1. Landing Zone Detection - Detect multiple landing pads with different types
 * 2. Atmospheric Entry Effects - Test entry effects, heat, drag
 * 3. Landing Guidance UI - Validate landing guidance display
 * 4. Terrain LOD Transitions - Test LOD changes during approach from orbit
 * 5. Physics Transition - Test gravity/drag/flight model changes
 * 6. Scale Transition - Test transition from scaled space to 1:1 at surface
 * 7. Altitude Detection - Verify accurate altitude measurement
 * 8. Landing Accuracy - Test precision landing on designated pads
 * 9. Weather Integration - Test landing in different weather conditions
 * 10. Multi-pad Management - Test switching between landing zones
 *
 * Requirements: 4.1, 4.2, 4.3, 4.4, 4.5
 */
UCLASS(ClassGroup=(Testing), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UPlanetLandingTestStation : public UTestStation
{
	GENERATED_BODY()

public:
	UPlanetLandingTestStation();

protected:
	virtual void BeginPlay() override;
	virtual void RegisterTests_Implementation() override;

public:
	// Test configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Landing Testing|Config")
	TSubclassOf<APlanet> TestPlanetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Landing Testing|Config")
	TSubclassOf<APlanetaryLandingZone> LandingZoneClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Landing Testing|Config")
	TSubclassOf<ALandingPad> LandingPadClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Landing Testing|Config")
	TSubclassOf<ASpaceship> TestSpaceshipClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Landing Testing|Config")
	float PlanetRadius = 6371.0f; // km (Earth-like)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Landing Testing|Config")
	float OrbitalAltitude = 100.0f; // km

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Landing Testing|Config")
	float AtmosphereHeight = 80.0f; // km

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Landing Testing|Config")
	float LandingAccuracyTolerance = 500.0f; // cm (5 meters)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Landing Testing|Config")
	int32 NumberOfTestPads = 5;

protected:
	// Test case implementations

	/** Test 4.1.1: Landing Zone Detection - Multiple Landing Pads */
	void Test_LandingZoneDetection();

	/** Test 4.1.2: Landing Zone Detection - Different Pad Types */
	void Test_DifferentPadTypes();

	/** Test 4.1.3: Landing Zone Detection - Pad Reservation System */
	void Test_PadReservationSystem();

	/** Test 4.2.1: Atmospheric Entry Effects - Heat Shimmer */
	void Test_HeatShimmerEffects();

	/** Test 4.2.2: Atmospheric Entry Effects - Plasma Effects */
	void Test_PlasmaEffects();

	/** Test 4.2.3: Atmospheric Entry Effects - Drag Calculation */
	void Test_AtmosphericDrag();

	/** Test 4.2.4: Atmospheric Entry Effects - Entry Sound Effects */
	void Test_EntrySoundEffects();

	/** Test 4.3.1: Landing Guidance UI - Approach Vector Display */
	void Test_ApproachVectorDisplay();

	/** Test 4.3.2: Landing Guidance UI - Altitude Display */
	void Test_AltitudeDisplay();

	/** Test 4.3.3: Landing Guidance UI - Velocity Display */
	void Test_VelocityDisplay();

	/** Test 4.3.4: Landing Guidance UI - Landing Pad Indicators */
	void Test_LandingPadIndicators();

	/** Test 4.4.1: Terrain LOD Transitions - Orbital to Atmospheric */
	void Test_OrbitalToAtmosphericLOD();

	/** Test 4.4.2: Terrain LOD Transitions - Atmospheric to Surface */
	void Test_AtmosphericToSurfaceLOD();

	/** Test 4.4.3: Terrain LOD Transitions - No Visible Pop */
	void Test_NoVisibleLODPop();

	/** Test 4.5.1: Physics Transition - Gravity Changes */
	void Test_GravityTransition();

	/** Test 4.5.2: Physics Transition - Drag Model Changes */
	void Test_DragModelTransition();

	/** Test 4.5.3: Physics Transition - Flight Model Changes */
	void Test_FlightModelTransition();

	/** Test 4.6.1: Scale Transition - Scaled Space to 1:1 */
	void Test_ScaleTransition();

	/** Test 4.6.2: Scale Transition - Position Accuracy */
	void Test_ScaleTransitionAccuracy();

	/** Test 4.7.1: Altitude Detection - Orbital Altitude */
	void Test_OrbitalAltitudeDetection();

	/** Test 4.7.2: Altitude Detection - Atmospheric Altitude */
	void Test_AtmosphericAltitudeDetection();

	/** Test 4.7.3: Altitude Detection - Surface Altitude (AGL) */
	void Test_SurfaceAltitudeDetection();

	/** Test 4.8.1: Landing Accuracy - Precision Landing on Pad */
	void Test_PrecisionLanding();

	/** Test 4.8.2: Landing Accuracy - Landing Velocity */
	void Test_LandingVelocity();

	/** Test 4.8.3: Landing Accuracy - Landing Angle */
	void Test_LandingAngle();

	/** Test 4.9.1: Weather Integration - Clear Weather Landing */
	void Test_ClearWeatherLanding();

	/** Test 4.9.2: Weather Integration - Stormy Weather Landing */
	void Test_StormyWeatherLanding();

	/** Test 4.9.3: Weather Integration - Reduced Visibility Landing */
	void Test_ReducedVisibilityLanding();

	/** Test 4.10.1: Multi-pad Management - Switch Between Pads */
	void Test_SwitchBetweenPads();

	/** Test 4.10.2: Multi-pad Management - Simultaneous Landings */
	void Test_SimultaneousLandings();

	// Helper functions

	/**
	 * Spawn a test planet at a given location
	 * @param Location World location to spawn planet
	 * @return Spawned planet actor
	 */
	APlanet* SpawnTestPlanet(const FVector& Location = FVector::ZeroVector);

	/**
	 * Cleanup spawned test planet
	 */
	void CleanupTestPlanet();

	/**
	 * Spawn a landing zone on the test planet
	 * @param Location Location on planet surface
	 * @param NumPads Number of landing pads to create
	 * @return Spawned landing zone
	 */
	APlanetaryLandingZone* SpawnLandingZone(const FVector& Location, int32 NumPads = 1);

	/**
	 * Cleanup all spawned landing zones
	 */
	void CleanupLandingZones();

	/**
	 * Spawn a test spaceship at given location
	 * @param Location Spawn location
	 * @param Velocity Initial velocity
	 * @return Spawned spaceship
	 */
	ASpaceship* SpawnTestSpaceship(const FVector& Location, const FVector& Velocity = FVector::ZeroVector);

	/**
	 * Cleanup spawned test spaceship
	 */
	void CleanupTestSpaceship();

	/**
	 * Simulate a landing approach from orbit to surface
	 * @param StartAltitude Starting altitude in km
	 * @param TargetPad Target landing pad
	 * @param OutLandingData Landing result data
	 * @return True if landing was successful
	 */
	bool SimulateLanding(float StartAltitude, ALandingPad* TargetPad, struct FLandingResult& OutLandingData);

	/**
	 * Measure altitude above planet surface
	 * @param Location World location to measure from
	 * @param bAboveGroundLevel If true, measures AGL; if false, measures ASL
	 * @return Altitude in km
	 */
	float MeasureAltitude(const FVector& Location, bool bAboveGroundLevel = true) const;

	/**
	 * Check if atmospheric entry effects are active
	 * @return True if effects are visible/active
	 */
	bool AreEntryEffectsActive() const;

	/**
	 * Find all landing pads in the test level
	 * @return Array of landing pad actors
	 */
	TArray<ALandingPad*> FindAllLandingPads() const;

	/**
	 * Validate landing zone detection at a location
	 * @param Location World location to check
	 * @param OutDetectedZones Detected landing zones
	 * @return True if zones were detected
	 */
	bool ValidateLandingZoneDetection(const FVector& Location, TArray<APlanetaryLandingZone*>& OutDetectedZones) const;

	/**
	 * Measure LOD transition quality
	 * @param TransitionAltitude Altitude where transition occurs
	 * @return Quality score (0-1, 1 = perfect)
	 */
	float MeasureLODTransitionQuality(float TransitionAltitude) const;

private:
	// Test objects
	UPROPERTY()
	APlanet* TestPlanet = nullptr;

	UPROPERTY()
	TArray<APlanetaryLandingZone*> SpawnedLandingZones;

	UPROPERTY()
	TArray<ALandingPad*> SpawnedLandingPads;

	UPROPERTY()
	ASpaceship* TestSpaceship = nullptr;

	// Component references
	UPROPERTY()
	UAtmosphericEntryEffects* EntryEffectsComponent = nullptr;

	UPROPERTY()
	ULandingDetectionComponent* LandingDetectionComponent = nullptr;

	// Test state tracking
	TArray<FVector> DescentPath;
	TArray<float> AltitudeSamples;
	TArray<float> VelocitySamples;
	FVector LastPosition;
	float LastPositionTime;
};

/**
 * Landing result data structure
 */
USTRUCT(BlueprintType)
struct FLandingResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	bool bSuccessful = false;

	UPROPERTY(BlueprintReadWrite)
	FVector LandingLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	float LandingVelocity = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float LandingAngle = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float DistanceFromPad = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float DescentTime = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	ALandingPad* LandedOnPad = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FString FailureReason;
};
