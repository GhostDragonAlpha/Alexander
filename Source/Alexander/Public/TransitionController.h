// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TransitionController.generated.h"

/**
 * Transition phases for space-to-surface movement
 */
UENUM(BlueprintType)
enum class ETransitionPhase : uint8
{
	Orbital UMETA(DisplayName = "Orbital (>100km)"),
	HighAtmosphere UMETA(DisplayName = "High Atmosphere (100-50km)"),
	MidAtmosphere UMETA(DisplayName = "Mid Atmosphere (50-10km)"),
	LowAtmosphere UMETA(DisplayName = "Low Atmosphere (10-1km)"),
	Surface UMETA(DisplayName = "Surface (<1km)")
};

/**
 * Configuration for transition phase thresholds
 */
USTRUCT(BlueprintType)
struct FTransitionPhaseConfig
{
	GENERATED_BODY()

	// Altitude threshold for high atmosphere (km)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float HighAtmosphereThreshold = 100.0f;

	// Altitude threshold for mid atmosphere (km)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float MidAtmosphereThreshold = 50.0f;

	// Altitude threshold for low atmosphere (km)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float LowAtmosphereThreshold = 10.0f;

	// Altitude threshold for surface (km)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float SurfaceThreshold = 1.0f;

	// Transition smoothing duration (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float TransitionDuration = 2.0f;
};

/**
 * Manages seamless space-to-surface transitions
 * Handles phase detection, physics transitions, and rendering adjustments
 */
UCLASS(BlueprintType)
class ALEXANDER_API UTransitionController : public UObject
{
	GENERATED_BODY()

public:
	UTransitionController();

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize the transition controller
	 * @param InPlanetRadius - Radius of the planet in kilometers
	 * @param InConfig - Configuration for phase thresholds
	 */
	UFUNCTION(BlueprintCallable, Category = "Transition")
	void Initialize(float InPlanetRadius, const FTransitionPhaseConfig& InConfig);

	// ============================================================================
	// PHASE DETECTION
	// ============================================================================

	/**
	 * Update transition state based on actor altitude
	 * @param ActorLocation - Current location of the transitioning actor
	 * @param PlanetLocation - Location of the planet center
	 * @param DeltaTime - Time since last update
	 */
	UFUNCTION(BlueprintCallable, Category = "Transition")
	void UpdateTransition(const FVector& ActorLocation, const FVector& PlanetLocation, float DeltaTime);

	/**
	 * Get current transition phase
	 */
	UFUNCTION(BlueprintPure, Category = "Transition")
	ETransitionPhase GetCurrentPhase() const { return CurrentPhase; }

	/**
	 * Get target transition phase
	 */
	UFUNCTION(BlueprintPure, Category = "Transition")
	ETransitionPhase GetTargetPhase() const { return TargetPhase; }

	/**
	 * Get transition progress (0-1) between current and target phase
	 */
	UFUNCTION(BlueprintPure, Category = "Transition")
	float GetTransitionProgress() const { return TransitionProgress; }

	/**
	 * Check if currently transitioning between phases
	 */
	UFUNCTION(BlueprintPure, Category = "Transition")
	bool IsTransitioning() const { return bIsTransitioning; }

	/**
	 * Get current altitude above surface in kilometers
	 */
	UFUNCTION(BlueprintPure, Category = "Transition")
	float GetAltitude() const { return CurrentAltitude; }

	// ============================================================================
	// PHASE TRANSITIONS
	// ============================================================================

	/**
	 * Begin transition to a new phase
	 * @param NewPhase - Target phase to transition to
	 */
	void BeginPhaseTransition(ETransitionPhase NewPhase);

	/**
	 * Complete current phase transition
	 */
	void CompletePhaseTransition();

	/**
	 * Get interpolated value between phases
	 * @param OrbitalValue - Value at orbital phase
	 * @param SurfaceValue - Value at surface phase
	 * @return Interpolated value based on current phase and progress
	 */
	UFUNCTION(BlueprintPure, Category = "Transition")
	float GetInterpolatedValue(float OrbitalValue, float SurfaceValue) const;

	// ============================================================================
	// PHASE QUERIES
	// ============================================================================

	/**
	 * Check if in orbital phase
	 */
	UFUNCTION(BlueprintPure, Category = "Transition")
	bool IsInOrbitalPhase() const { return CurrentPhase == ETransitionPhase::Orbital; }

	/**
	 * Check if in any atmosphere phase
	 */
	UFUNCTION(BlueprintPure, Category = "Transition")
	bool IsInAtmosphere() const;

	/**
	 * Check if in surface phase
	 */
	UFUNCTION(BlueprintPure, Category = "Transition")
	bool IsInSurfacePhase() const { return CurrentPhase == ETransitionPhase::Surface; }

	/**
	 * Get atmospheric density factor (0-1) based on altitude
	 * 0 = no atmosphere (orbital), 1 = full atmosphere (surface)
	 */
	UFUNCTION(BlueprintPure, Category = "Transition")
	float GetAtmosphericDensity() const;

	// ============================================================================
	// PHYSICS TRANSITIONS
	// ============================================================================

	/**
	 * Calculate atmospheric drag force
	 * @param Velocity - Current velocity vector of the actor
	 * @param DragCoefficient - Drag coefficient of the actor (default 0.5)
	 * @param CrossSectionalArea - Cross-sectional area in square meters (default 10.0)
	 * @return Drag force vector opposing velocity
	 */
	UFUNCTION(BlueprintPure, Category = "Physics")
	FVector CalculateAtmosphericDrag(const FVector& Velocity, float DragCoefficient = 0.5f, float CrossSectionalArea = 10.0f) const;

	/**
	 * Calculate lift force for aerodynamic surfaces
	 * @param Velocity - Current velocity vector of the actor
	 * @param ForwardVector - Forward direction of the actor
	 * @param UpVector - Up direction of the actor
	 * @param LiftCoefficient - Lift coefficient (default 0.3)
	 * @param WingArea - Wing area in square meters (default 20.0)
	 * @return Lift force vector perpendicular to velocity
	 */
	UFUNCTION(BlueprintPure, Category = "Physics")
	FVector CalculateLiftForce(const FVector& Velocity, const FVector& ForwardVector, const FVector& UpVector, float LiftCoefficient = 0.3f, float WingArea = 20.0f) const;

	/**
	 * Calculate turbulence effect
	 * @param CurrentVelocity - Current velocity of the actor
	 * @param DeltaTime - Time since last update
	 * @return Turbulence force vector
	 */
	UFUNCTION(BlueprintPure, Category = "Physics")
	FVector CalculateTurbulence(const FVector& CurrentVelocity, float DeltaTime) const;

	/**
	 * Get physics interpolation factor for transitioning from orbital to atmospheric physics
	 * 0 = full orbital physics, 1 = full atmospheric physics
	 */
	UFUNCTION(BlueprintPure, Category = "Physics")
	float GetPhysicsInterpolationFactor() const;

	/**
	 * Check if atmospheric physics should be applied
	 */
	UFUNCTION(BlueprintPure, Category = "Physics")
	bool ShouldApplyAtmosphericPhysics() const;

	/**
	 * Get air density at current altitude (kg/m³)
	 * Sea level: ~1.225 kg/m³, decreases exponentially with altitude
	 */
	UFUNCTION(BlueprintPure, Category = "Physics")
	float GetAirDensity() const;

	/**
	 * Get turbulence intensity factor (0-1) based on altitude and weather
	 * Higher values mean more turbulence
	 */
	UFUNCTION(BlueprintPure, Category = "Physics")
	float GetTurbulenceIntensity() const;

	// ============================================================================
	// RENDERING TRANSITIONS
	// ============================================================================

	/**
	 * Get LOD distance scale factor based on altitude
	 * Higher altitudes use larger LOD distances for better performance
	 * @return Scale factor to multiply base LOD distances (1.0 at surface, up to 10.0 at orbital)
	 */
	UFUNCTION(BlueprintPure, Category = "Rendering")
	float GetLODDistanceScale() const;

	/**
	 * Get LOD bias for terrain rendering based on altitude
	 * Negative values increase detail, positive values decrease detail
	 * @return LOD bias value (-2 to +3)
	 */
	UFUNCTION(BlueprintPure, Category = "Rendering")
	float GetLODBias() const;

	/**
	 * Check if terrain streaming should be active
	 * Streaming activates when entering mid-atmosphere and below
	 */
	UFUNCTION(BlueprintPure, Category = "Rendering")
	bool ShouldEnableTerrainStreaming() const;

	/**
	 * Get terrain streaming priority based on altitude
	 * Higher priority means more aggressive streaming
	 * @return Priority value (0.0 = no streaming, 1.0 = maximum priority)
	 */
	UFUNCTION(BlueprintPure, Category = "Rendering")
	float GetTerrainStreamingPriority() const;

	/**
	 * Get camera far clip plane distance based on altitude
	 * Adjusts view distance for optimal rendering at different altitudes
	 * @return Far clip distance in centimeters
	 */
	UFUNCTION(BlueprintPure, Category = "Rendering")
	float GetCameraFarClipDistance() const;

	/**
	 * Get camera near clip plane distance based on altitude
	 * Adjusts near clip for better depth precision at different altitudes
	 * @return Near clip distance in centimeters
	 */
	UFUNCTION(BlueprintPure, Category = "Rendering")
	float GetCameraNearClipDistance() const;

	/**
	 * Get fog density multiplier based on altitude
	 * Fog increases as altitude decreases toward surface
	 * @return Fog density multiplier (0.0 = no fog, 1.0 = full fog)
	 */
	UFUNCTION(BlueprintPure, Category = "Rendering")
	float GetFogDensityMultiplier() const;

	/**
	 * Get fog start distance based on altitude
	 * Fog starts closer at lower altitudes
	 * @return Fog start distance in centimeters
	 */
	UFUNCTION(BlueprintPure, Category = "Rendering")
	float GetFogStartDistance() const;

	/**
	 * Get fog height falloff based on altitude
	 * Controls how quickly fog density decreases with height
	 * @return Height falloff value (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Rendering")
	float GetFogHeightFalloff() const;

	/**
	 * Get atmospheric scattering intensity based on altitude
	 * Controls strength of atmospheric color effects
	 * @return Scattering intensity (0.0 = none, 1.0 = full)
	 */
	UFUNCTION(BlueprintPure, Category = "Rendering")
	float GetAtmosphericScatteringIntensity() const;

	/**
	 * Get post-process settings for current altitude
	 * Returns recommended post-process values for smooth transitions
	 * @param OutExposure - Auto-exposure compensation
	 * @param OutBloomIntensity - Bloom effect intensity
	 * @param OutVignetteIntensity - Vignette effect intensity
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering")
	void GetPostProcessSettings(float& OutExposure, float& OutBloomIntensity, float& OutVignetteIntensity) const;

	/**
	 * Get rendering quality scale based on altitude
	 * Can be used to adjust rendering quality for performance
	 * @return Quality scale (0.5 to 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Rendering")
	float GetRenderingQualityScale() const;

protected:
	// ============================================================================
	// INTERNAL METHODS
	// ============================================================================

	/**
	 * Detect appropriate phase based on altitude
	 */
	ETransitionPhase DetectPhaseFromAltitude(float Altitude) const;

	/**
	 * Calculate smooth transition progress
	 */
	void UpdateTransitionProgress(float DeltaTime);

	/**
	 * Get phase-specific interpolation factor
	 */
	float GetPhaseInterpolationFactor() const;

private:
	// ============================================================================
	// STATE
	// ============================================================================

	// Current transition phase
	UPROPERTY()
	ETransitionPhase CurrentPhase;

	// Target phase for transition
	UPROPERTY()
	ETransitionPhase TargetPhase;

	// Transition progress (0-1)
	UPROPERTY()
	float TransitionProgress;

	// Whether currently transitioning
	UPROPERTY()
	bool bIsTransitioning;

	// Current altitude above surface (km)
	UPROPERTY()
	float CurrentAltitude;

	// Planet radius (km)
	UPROPERTY()
	float PlanetRadius;

	// Configuration
	UPROPERTY()
	FTransitionPhaseConfig Config;

	// Time elapsed in current transition
	float TransitionElapsedTime;

	// ============================================================================
	// PHYSICS STATE
	// ============================================================================

	// Turbulence noise offset for randomization
	mutable float TurbulenceNoiseOffset;

	// Last turbulence update time
	mutable float LastTurbulenceTime;

	// Current turbulence vector (smoothed)
	mutable FVector CurrentTurbulence;
};
