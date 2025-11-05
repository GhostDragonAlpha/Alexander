// Copyright Epic Games, Inc. All Rights Reserved.

#include "TransitionController.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Kismet/KismetMathLibrary.h"

UTransitionController::UTransitionController()
	: CurrentPhase(ETransitionPhase::Orbital)
	, TargetPhase(ETransitionPhase::Orbital)
	, TransitionProgress(0.0f)
	, bIsTransitioning(false)
	, CurrentAltitude(0.0f)
	, PlanetRadius(6371.0f)
	, TransitionElapsedTime(0.0f)
	, TurbulenceNoiseOffset(0.0f)
	, LastTurbulenceTime(0.0f)
	, CurrentTurbulence(FVector::ZeroVector)
{
	// Initialize turbulence with random offset
	TurbulenceNoiseOffset = FMath::FRandRange(0.0f, 1000.0f);
}

void UTransitionController::Initialize(float InPlanetRadius, const FTransitionPhaseConfig& InConfig)
{
	PlanetRadius = InPlanetRadius;
	Config = InConfig;
	
	// Start in orbital phase
	CurrentPhase = ETransitionPhase::Orbital;
	TargetPhase = ETransitionPhase::Orbital;
	TransitionProgress = 0.0f;
	bIsTransitioning = false;
	CurrentAltitude = Config.HighAtmosphereThreshold + 10.0f; // Start well above atmosphere
	TransitionElapsedTime = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("TransitionController initialized with planet radius %.1f km"), PlanetRadius);
}

void UTransitionController::UpdateTransition(const FVector& ActorLocation, const FVector& PlanetLocation, float DeltaTime)
{
	// Calculate distance from planet center
	float DistanceFromCenter = FVector::Dist(ActorLocation, PlanetLocation);
	
	// Convert to kilometers and calculate altitude above surface
	float DistanceKm = DistanceFromCenter / 100000.0f; // cm to km
	CurrentAltitude = DistanceKm - PlanetRadius;

	// Detect appropriate phase for current altitude
	ETransitionPhase DetectedPhase = DetectPhaseFromAltitude(CurrentAltitude);

	// Check if we need to start a new transition
	if (DetectedPhase != CurrentPhase && !bIsTransitioning)
	{
		BeginPhaseTransition(DetectedPhase);
	}

	// Update transition progress if transitioning
	if (bIsTransitioning)
	{
		UpdateTransitionProgress(DeltaTime);
	}
}

ETransitionPhase UTransitionController::DetectPhaseFromAltitude(float Altitude) const
{
	if (Altitude >= Config.HighAtmosphereThreshold)
	{
		return ETransitionPhase::Orbital;
	}
	else if (Altitude >= Config.MidAtmosphereThreshold)
	{
		return ETransitionPhase::HighAtmosphere;
	}
	else if (Altitude >= Config.LowAtmosphereThreshold)
	{
		return ETransitionPhase::MidAtmosphere;
	}
	else if (Altitude >= Config.SurfaceThreshold)
	{
		return ETransitionPhase::LowAtmosphere;
	}
	else
	{
		return ETransitionPhase::Surface;
	}
}

void UTransitionController::BeginPhaseTransition(ETransitionPhase NewPhase)
{
	// Don't transition to the same phase
	if (NewPhase == CurrentPhase)
	{
		return;
	}

	TargetPhase = NewPhase;
	bIsTransitioning = true;
	TransitionProgress = 0.0f;
	TransitionElapsedTime = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Beginning transition from %s to %s at altitude %.2f km"),
		*UEnum::GetValueAsString(CurrentPhase),
		*UEnum::GetValueAsString(TargetPhase),
		CurrentAltitude);
}

void UTransitionController::UpdateTransitionProgress(float DeltaTime)
{
	if (!bIsTransitioning)
	{
		return;
	}

	TransitionElapsedTime += DeltaTime;
	
	// Calculate progress with smooth interpolation
	float RawProgress = TransitionElapsedTime / Config.TransitionDuration;
	TransitionProgress = FMath::Clamp(RawProgress, 0.0f, 1.0f);

	// Apply smoothstep for smoother transitions
	TransitionProgress = FMath::SmoothStep(0.0f, 1.0f, TransitionProgress);

	// Complete transition when progress reaches 1.0
	if (TransitionProgress >= 1.0f)
	{
		CompletePhaseTransition();
	}
}

void UTransitionController::CompletePhaseTransition()
{
	if (!bIsTransitioning)
	{
		return;
	}

	CurrentPhase = TargetPhase;
	bIsTransitioning = false;
	TransitionProgress = 0.0f;
	TransitionElapsedTime = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Completed transition to %s at altitude %.2f km"),
		*UEnum::GetValueAsString(CurrentPhase),
		CurrentAltitude);
}

bool UTransitionController::IsInAtmosphere() const
{
	return CurrentPhase != ETransitionPhase::Orbital;
}

float UTransitionController::GetAtmosphericDensity() const
{
	// Calculate density based on altitude
	// Orbital: 0.0 (no atmosphere)
	// High atmosphere: 0.0 - 0.25
	// Mid atmosphere: 0.25 - 0.5
	// Low atmosphere: 0.5 - 0.75
	// Surface: 0.75 - 1.0

	float BaseDensity = 0.0f;
	float MaxDensity = 0.0f;
	float PhaseProgress = 0.0f;

	switch (CurrentPhase)
	{
	case ETransitionPhase::Orbital:
		BaseDensity = 0.0f;
		MaxDensity = 0.0f;
		break;

	case ETransitionPhase::HighAtmosphere:
		BaseDensity = 0.0f;
		MaxDensity = 0.25f;
		// Calculate progress within high atmosphere range
		PhaseProgress = 1.0f - ((CurrentAltitude - Config.MidAtmosphereThreshold) / 
			(Config.HighAtmosphereThreshold - Config.MidAtmosphereThreshold));
		break;

	case ETransitionPhase::MidAtmosphere:
		BaseDensity = 0.25f;
		MaxDensity = 0.5f;
		PhaseProgress = 1.0f - ((CurrentAltitude - Config.LowAtmosphereThreshold) / 
			(Config.MidAtmosphereThreshold - Config.LowAtmosphereThreshold));
		break;

	case ETransitionPhase::LowAtmosphere:
		BaseDensity = 0.5f;
		MaxDensity = 0.75f;
		PhaseProgress = 1.0f - ((CurrentAltitude - Config.SurfaceThreshold) / 
			(Config.LowAtmosphereThreshold - Config.SurfaceThreshold));
		break;

	case ETransitionPhase::Surface:
		BaseDensity = 0.75f;
		MaxDensity = 1.0f;
		// At surface, density increases as we get closer to ground
		PhaseProgress = FMath::Clamp(1.0f - (CurrentAltitude / Config.SurfaceThreshold), 0.0f, 1.0f);
		break;
	}

	PhaseProgress = FMath::Clamp(PhaseProgress, 0.0f, 1.0f);
	float Density = FMath::Lerp(BaseDensity, MaxDensity, PhaseProgress);

	// Apply transition smoothing if transitioning
	if (bIsTransitioning)
	{
		float TargetDensity = 0.0f;
		
		// Calculate what density would be at target phase
		switch (TargetPhase)
		{
		case ETransitionPhase::Orbital:
			TargetDensity = 0.0f;
			break;
		case ETransitionPhase::HighAtmosphere:
			TargetDensity = 0.125f; // Mid-point of range
			break;
		case ETransitionPhase::MidAtmosphere:
			TargetDensity = 0.375f;
			break;
		case ETransitionPhase::LowAtmosphere:
			TargetDensity = 0.625f;
			break;
		case ETransitionPhase::Surface:
			TargetDensity = 0.875f;
			break;
		}

		Density = FMath::Lerp(Density, TargetDensity, TransitionProgress);
	}

	return FMath::Clamp(Density, 0.0f, 1.0f);
}

float UTransitionController::GetInterpolatedValue(float OrbitalValue, float SurfaceValue) const
{
	// Get overall interpolation factor based on phase
	float InterpolationFactor = GetPhaseInterpolationFactor();
	
	// Interpolate between orbital and surface values
	return FMath::Lerp(OrbitalValue, SurfaceValue, InterpolationFactor);
}

float UTransitionController::GetPhaseInterpolationFactor() const
{
	// Map phases to interpolation values (0 = orbital, 1 = surface)
	float BaseInterpolation = 0.0f;

	switch (CurrentPhase)
	{
	case ETransitionPhase::Orbital:
		BaseInterpolation = 0.0f;
		break;
	case ETransitionPhase::HighAtmosphere:
		BaseInterpolation = 0.2f;
		break;
	case ETransitionPhase::MidAtmosphere:
		BaseInterpolation = 0.4f;
		break;
	case ETransitionPhase::LowAtmosphere:
		BaseInterpolation = 0.7f;
		break;
	case ETransitionPhase::Surface:
		BaseInterpolation = 1.0f;
		break;
	}

	// If transitioning, blend toward target phase
	if (bIsTransitioning)
	{
		float TargetInterpolation = 0.0f;

		switch (TargetPhase)
		{
		case ETransitionPhase::Orbital:
			TargetInterpolation = 0.0f;
			break;
		case ETransitionPhase::HighAtmosphere:
			TargetInterpolation = 0.2f;
			break;
		case ETransitionPhase::MidAtmosphere:
			TargetInterpolation = 0.4f;
			break;
		case ETransitionPhase::LowAtmosphere:
			TargetInterpolation = 0.7f;
			break;
		case ETransitionPhase::Surface:
			TargetInterpolation = 1.0f;
			break;
		}

		BaseInterpolation = FMath::Lerp(BaseInterpolation, TargetInterpolation, TransitionProgress);
	}

	return FMath::Clamp(BaseInterpolation, 0.0f, 1.0f);
}

// ============================================================================
// PHYSICS TRANSITIONS
// ============================================================================

FVector UTransitionController::CalculateAtmosphericDrag(const FVector& Velocity, float DragCoefficient, float CrossSectionalArea) const
{
	// Don't apply drag in orbital phase
	if (!ShouldApplyAtmosphericPhysics())
	{
		return FVector::ZeroVector;
	}

	// Get air density at current altitude
	float AirDensity = GetAirDensity();

	// Calculate velocity magnitude
	float Speed = Velocity.Size();
	if (Speed < 0.01f)
	{
		return FVector::ZeroVector;
	}

	// Drag force formula: F = 0.5 * ρ * v² * Cd * A
	// ρ = air density (kg/m³)
	// v = velocity (m/s, convert from cm/s)
	// Cd = drag coefficient
	// A = cross-sectional area (m²)

	float SpeedMetersPerSecond = Speed / 100.0f; // cm/s to m/s
	float DragMagnitude = 0.5f * AirDensity * SpeedMetersPerSecond * SpeedMetersPerSecond * DragCoefficient * CrossSectionalArea;

	// Convert back to Unreal units (N to force in cm/s²)
	// Assuming mass of 1000 kg for typical spacecraft
	float Mass = 1000.0f; // kg
	float DragAcceleration = DragMagnitude / Mass; // m/s²
	float DragAccelerationCm = DragAcceleration * 100.0f; // cm/s²

	// Apply drag opposite to velocity direction
	FVector DragDirection = -Velocity.GetSafeNormal();
	FVector DragForce = DragDirection * DragAccelerationCm;

	// Scale by physics interpolation factor for smooth transition
	float PhysicsBlend = GetPhysicsInterpolationFactor();
	return DragForce * PhysicsBlend;
}

FVector UTransitionController::CalculateLiftForce(const FVector& Velocity, const FVector& ForwardVector, const FVector& UpVector, float LiftCoefficient, float WingArea) const
{
	// Don't apply lift in orbital phase or at very low speeds
	if (!ShouldApplyAtmosphericPhysics())
	{
		return FVector::ZeroVector;
	}

	float Speed = Velocity.Size();
	if (Speed < 100.0f) // Minimum speed for lift (1 m/s)
	{
		return FVector::ZeroVector;
	}

	// Get air density at current altitude
	float AirDensity = GetAirDensity();

	// Calculate angle of attack (angle between velocity and forward vector)
	FVector VelocityDirection = Velocity.GetSafeNormal();
	float AngleOfAttack = FMath::Acos(FVector::DotProduct(VelocityDirection, ForwardVector));

	// Lift is most effective at small angles (0-15 degrees)
	// Use sine for lift coefficient variation
	float AngleOfAttackDegrees = FMath::RadiansToDegrees(AngleOfAttack);
	float LiftEfficiency = FMath::Sin(FMath::DegreesToRadians(FMath::Clamp(AngleOfAttackDegrees, 0.0f, 15.0f) * 6.0f)); // Peak at 15 degrees

	// Lift force formula: F = 0.5 * ρ * v² * Cl * A
	float SpeedMetersPerSecond = Speed / 100.0f; // cm/s to m/s
	float LiftMagnitude = 0.5f * AirDensity * SpeedMetersPerSecond * SpeedMetersPerSecond * LiftCoefficient * LiftEfficiency * WingArea;

	// Convert to acceleration
	float Mass = 1000.0f; // kg
	float LiftAcceleration = LiftMagnitude / Mass; // m/s²
	float LiftAccelerationCm = LiftAcceleration * 100.0f; // cm/s²

	// Lift direction is perpendicular to velocity, in the direction of the up vector
	FVector LiftDirection = FVector::CrossProduct(VelocityDirection, FVector::CrossProduct(UpVector, VelocityDirection)).GetSafeNormal();
	FVector LiftForce = LiftDirection * LiftAccelerationCm;

	// Scale by physics interpolation factor
	float PhysicsBlend = GetPhysicsInterpolationFactor();
	return LiftForce * PhysicsBlend;
}

FVector UTransitionController::CalculateTurbulence(const FVector& CurrentVelocity, float DeltaTime) const
{
	// Don't apply turbulence in orbital phase
	if (!ShouldApplyAtmosphericPhysics())
	{
		CurrentTurbulence = FVector::ZeroVector;
		return FVector::ZeroVector;
	}

	// Get turbulence intensity based on altitude and conditions
	float Intensity = GetTurbulenceIntensity();
	if (Intensity < 0.01f)
	{
		CurrentTurbulence = FVector::ZeroVector;
		return FVector::ZeroVector;
	}

	// Update turbulence at a lower frequency for performance (10 Hz)
	float CurrentTime = FPlatformTime::Seconds();
	if (CurrentTime - LastTurbulenceTime < 0.1f)
	{
		return CurrentTurbulence;
	}
	LastTurbulenceTime = CurrentTime;

	// Generate turbulence using Perlin-like noise
	// Use time-based offset for continuous variation
	float TimeOffset = CurrentTime * 0.5f + TurbulenceNoiseOffset;

	// Generate 3D noise for each axis
	float NoiseX = FMath::PerlinNoise1D(TimeOffset) * 2.0f - 1.0f;
	float NoiseY = FMath::PerlinNoise1D(TimeOffset + 100.0f) * 2.0f - 1.0f;
	float NoiseZ = FMath::PerlinNoise1D(TimeOffset + 200.0f) * 2.0f - 1.0f;

	// Create turbulence vector
	FVector TargetTurbulence = FVector(NoiseX, NoiseY, NoiseZ) * Intensity * 500.0f; // Scale to reasonable force

	// Smooth turbulence changes using interpolation
	float SmoothingFactor = FMath::Clamp(DeltaTime * 5.0f, 0.0f, 1.0f);
	CurrentTurbulence = FMath::Lerp(CurrentTurbulence, TargetTurbulence, SmoothingFactor);

	// Scale by physics interpolation factor
	float PhysicsBlend = GetPhysicsInterpolationFactor();
	return CurrentTurbulence * PhysicsBlend;
}

float UTransitionController::GetPhysicsInterpolationFactor() const
{
	// Physics transition should be more aggressive than visual transitions
	// Start applying atmospheric physics in high atmosphere
	
	float Factor = 0.0f;

	switch (CurrentPhase)
	{
	case ETransitionPhase::Orbital:
		Factor = 0.0f;
		break;
	case ETransitionPhase::HighAtmosphere:
		// Gradually introduce atmospheric physics
		Factor = 0.3f;
		break;
	case ETransitionPhase::MidAtmosphere:
		Factor = 0.6f;
		break;
	case ETransitionPhase::LowAtmosphere:
		Factor = 0.9f;
		break;
	case ETransitionPhase::Surface:
		Factor = 1.0f;
		break;
	}

	// Smooth transition between phases
	if (bIsTransitioning)
	{
		float TargetFactor = 0.0f;

		switch (TargetPhase)
		{
		case ETransitionPhase::Orbital:
			TargetFactor = 0.0f;
			break;
		case ETransitionPhase::HighAtmosphere:
			TargetFactor = 0.3f;
			break;
		case ETransitionPhase::MidAtmosphere:
			TargetFactor = 0.6f;
			break;
		case ETransitionPhase::LowAtmosphere:
			TargetFactor = 0.9f;
			break;
		case ETransitionPhase::Surface:
			TargetFactor = 1.0f;
			break;
		}

		Factor = FMath::Lerp(Factor, TargetFactor, TransitionProgress);
	}

	return FMath::Clamp(Factor, 0.0f, 1.0f);
}

bool UTransitionController::ShouldApplyAtmosphericPhysics() const
{
	// Apply atmospheric physics when in any atmosphere phase
	return IsInAtmosphere() && GetPhysicsInterpolationFactor() > 0.01f;
}

float UTransitionController::GetAirDensity() const
{
	// Air density at sea level: 1.225 kg/m³
	// Decreases exponentially with altitude
	// Using barometric formula: ρ = ρ₀ * e^(-h/H)
	// where H is scale height (~8.5 km for Earth)

	const float SeaLevelDensity = 1.225f; // kg/m³
	const float ScaleHeight = 8.5f; // km

	// Calculate density based on altitude
	float DensityFactor = FMath::Exp(-CurrentAltitude / ScaleHeight);
	float Density = SeaLevelDensity * DensityFactor;

	// Clamp to reasonable values
	return FMath::Clamp(Density, 0.0f, SeaLevelDensity);
}

float UTransitionController::GetTurbulenceIntensity() const
{
	// Turbulence is strongest in mid-atmosphere (jet stream altitudes)
	// Minimal at orbital and surface levels

	float Intensity = 0.0f;

	switch (CurrentPhase)
	{
	case ETransitionPhase::Orbital:
		Intensity = 0.0f;
		break;
	case ETransitionPhase::HighAtmosphere:
		// Increasing turbulence as we enter atmosphere
		Intensity = 0.3f;
		break;
	case ETransitionPhase::MidAtmosphere:
		// Peak turbulence in mid-atmosphere
		Intensity = 0.8f;
		break;
	case ETransitionPhase::LowAtmosphere:
		// Moderate turbulence near surface
		Intensity = 0.5f;
		break;
	case ETransitionPhase::Surface:
		// Light turbulence at surface
		Intensity = 0.2f;
		break;
	}

	// Smooth transition between phases
	if (bIsTransitioning)
	{
		float TargetIntensity = 0.0f;

		switch (TargetPhase)
		{
		case ETransitionPhase::Orbital:
			TargetIntensity = 0.0f;
			break;
		case ETransitionPhase::HighAtmosphere:
			TargetIntensity = 0.3f;
			break;
		case ETransitionPhase::MidAtmosphere:
			TargetIntensity = 0.8f;
			break;
		case ETransitionPhase::LowAtmosphere:
			TargetIntensity = 0.5f;
			break;
		case ETransitionPhase::Surface:
			TargetIntensity = 0.2f;
			break;
		}

		Intensity = FMath::Lerp(Intensity, TargetIntensity, TransitionProgress);
	}

	return FMath::Clamp(Intensity, 0.0f, 1.0f);
}

// ============================================================================
// RENDERING TRANSITIONS
// ============================================================================

float UTransitionController::GetLODDistanceScale() const
{
	// Scale LOD distances based on altitude
	// At orbital altitudes, use much larger LOD distances
	// At surface, use base LOD distances
	
	float Scale = 1.0f;

	switch (CurrentPhase)
	{
	case ETransitionPhase::Orbital:
		// Maximum LOD distance at orbital altitude
		Scale = 10.0f;
		break;
	case ETransitionPhase::HighAtmosphere:
		// Large LOD distances in high atmosphere
		Scale = 7.0f;
		break;
	case ETransitionPhase::MidAtmosphere:
		// Medium LOD distances
		Scale = 4.0f;
		break;
	case ETransitionPhase::LowAtmosphere:
		// Approaching base LOD distances
		Scale = 2.0f;
		break;
	case ETransitionPhase::Surface:
		// Base LOD distances at surface
		Scale = 1.0f;
		break;
	}

	// Smooth transition between phases
	if (bIsTransitioning)
	{
		float TargetScale = 1.0f;

		switch (TargetPhase)
		{
		case ETransitionPhase::Orbital:
			TargetScale = 10.0f;
			break;
		case ETransitionPhase::HighAtmosphere:
			TargetScale = 7.0f;
			break;
		case ETransitionPhase::MidAtmosphere:
			TargetScale = 4.0f;
			break;
		case ETransitionPhase::LowAtmosphere:
			TargetScale = 2.0f;
			break;
		case ETransitionPhase::Surface:
			TargetScale = 1.0f;
			break;
		}

		Scale = FMath::Lerp(Scale, TargetScale, TransitionProgress);
	}

	return FMath::Clamp(Scale, 1.0f, 10.0f);
}

float UTransitionController::GetLODBias() const
{
	// LOD bias adjusts detail level
	// Negative values = more detail, Positive values = less detail
	
	float Bias = 0.0f;

	switch (CurrentPhase)
	{
	case ETransitionPhase::Orbital:
		// Reduce detail at orbital distances
		Bias = 3.0f;
		break;
	case ETransitionPhase::HighAtmosphere:
		Bias = 2.0f;
		break;
	case ETransitionPhase::MidAtmosphere:
		Bias = 1.0f;
		break;
	case ETransitionPhase::LowAtmosphere:
		Bias = 0.0f;
		break;
	case ETransitionPhase::Surface:
		// Maximum detail at surface
		Bias = -1.0f;
		break;
	}

	// Smooth transition
	if (bIsTransitioning)
	{
		float TargetBias = 0.0f;

		switch (TargetPhase)
		{
		case ETransitionPhase::Orbital:
			TargetBias = 3.0f;
			break;
		case ETransitionPhase::HighAtmosphere:
			TargetBias = 2.0f;
			break;
		case ETransitionPhase::MidAtmosphere:
			TargetBias = 1.0f;
			break;
		case ETransitionPhase::LowAtmosphere:
			TargetBias = 0.0f;
			break;
		case ETransitionPhase::Surface:
			TargetBias = -1.0f;
			break;
		}

		Bias = FMath::Lerp(Bias, TargetBias, TransitionProgress);
	}

	return FMath::Clamp(Bias, -2.0f, 3.0f);
}

bool UTransitionController::ShouldEnableTerrainStreaming() const
{
	// Enable terrain streaming when entering mid-atmosphere and below
	// This is when terrain detail becomes important
	
	if (CurrentPhase == ETransitionPhase::Orbital || CurrentPhase == ETransitionPhase::HighAtmosphere)
	{
		// Check if transitioning toward lower altitudes
		if (bIsTransitioning)
		{
			return (TargetPhase == ETransitionPhase::MidAtmosphere ||
					TargetPhase == ETransitionPhase::LowAtmosphere ||
					TargetPhase == ETransitionPhase::Surface);
		}
		return false;
	}

	// Always stream at mid-atmosphere and below
	return true;
}

float UTransitionController::GetTerrainStreamingPriority() const
{
	// Streaming priority increases as we get closer to surface
	
	if (!ShouldEnableTerrainStreaming())
	{
		return 0.0f;
	}

	float Priority = 0.0f;

	switch (CurrentPhase)
	{
	case ETransitionPhase::Orbital:
		Priority = 0.0f;
		break;
	case ETransitionPhase::HighAtmosphere:
		Priority = 0.2f;
		break;
	case ETransitionPhase::MidAtmosphere:
		Priority = 0.5f;
		break;
	case ETransitionPhase::LowAtmosphere:
		Priority = 0.8f;
		break;
	case ETransitionPhase::Surface:
		Priority = 1.0f;
		break;
	}

	// Smooth transition
	if (bIsTransitioning)
	{
		float TargetPriority = 0.0f;

		switch (TargetPhase)
		{
		case ETransitionPhase::Orbital:
			TargetPriority = 0.0f;
			break;
		case ETransitionPhase::HighAtmosphere:
			TargetPriority = 0.2f;
			break;
		case ETransitionPhase::MidAtmosphere:
			TargetPriority = 0.5f;
			break;
		case ETransitionPhase::LowAtmosphere:
			TargetPriority = 0.8f;
			break;
		case ETransitionPhase::Surface:
			TargetPriority = 1.0f;
			break;
		}

		Priority = FMath::Lerp(Priority, TargetPriority, TransitionProgress);
	}

	return FMath::Clamp(Priority, 0.0f, 1.0f);
}

float UTransitionController::GetCameraFarClipDistance() const
{
	// Adjust far clip plane based on altitude
	// Orbital: Very far (100km+)
	// Surface: Moderate (10-20km)
	
	float FarClipKm = 10.0f; // Default 10km

	switch (CurrentPhase)
	{
	case ETransitionPhase::Orbital:
		// Very far clip for orbital view
		FarClipKm = 200.0f;
		break;
	case ETransitionPhase::HighAtmosphere:
		FarClipKm = 100.0f;
		break;
	case ETransitionPhase::MidAtmosphere:
		FarClipKm = 50.0f;
		break;
	case ETransitionPhase::LowAtmosphere:
		FarClipKm = 20.0f;
		break;
	case ETransitionPhase::Surface:
		FarClipKm = 10.0f;
		break;
	}

	// Smooth transition
	if (bIsTransitioning)
	{
		float TargetFarClipKm = 10.0f;

		switch (TargetPhase)
		{
		case ETransitionPhase::Orbital:
			TargetFarClipKm = 200.0f;
			break;
		case ETransitionPhase::HighAtmosphere:
			TargetFarClipKm = 100.0f;
			break;
		case ETransitionPhase::MidAtmosphere:
			TargetFarClipKm = 50.0f;
			break;
		case ETransitionPhase::LowAtmosphere:
			TargetFarClipKm = 20.0f;
			break;
		case ETransitionPhase::Surface:
			TargetFarClipKm = 10.0f;
			break;
		}

		FarClipKm = FMath::Lerp(FarClipKm, TargetFarClipKm, TransitionProgress);
	}

	// Convert km to cm (Unreal units)
	return FarClipKm * 100000.0f;
}

float UTransitionController::GetCameraNearClipDistance() const
{
	// Adjust near clip plane for better depth precision
	// Orbital: Larger near clip (100cm)
	// Surface: Smaller near clip (10cm) for close-up detail
	
	float NearClipCm = 10.0f; // Default 10cm

	switch (CurrentPhase)
	{
	case ETransitionPhase::Orbital:
		NearClipCm = 100.0f;
		break;
	case ETransitionPhase::HighAtmosphere:
		NearClipCm = 75.0f;
		break;
	case ETransitionPhase::MidAtmosphere:
		NearClipCm = 50.0f;
		break;
	case ETransitionPhase::LowAtmosphere:
		NearClipCm = 25.0f;
		break;
	case ETransitionPhase::Surface:
		NearClipCm = 10.0f;
		break;
	}

	// Smooth transition
	if (bIsTransitioning)
	{
		float TargetNearClipCm = 10.0f;

		switch (TargetPhase)
		{
		case ETransitionPhase::Orbital:
			TargetNearClipCm = 100.0f;
			break;
		case ETransitionPhase::HighAtmosphere:
			TargetNearClipCm = 75.0f;
			break;
		case ETransitionPhase::MidAtmosphere:
			TargetNearClipCm = 50.0f;
			break;
		case ETransitionPhase::LowAtmosphere:
			TargetNearClipCm = 25.0f;
			break;
		case ETransitionPhase::Surface:
			TargetNearClipCm = 10.0f;
			break;
		}

		NearClipCm = FMath::Lerp(NearClipCm, TargetNearClipCm, TransitionProgress);
	}

	return FMath::Clamp(NearClipCm, 1.0f, 1000.0f);
}

float UTransitionController::GetFogDensityMultiplier() const
{
	// Fog density increases as we approach surface
	// Orbital: No fog (0.0)
	// Surface: Full fog (1.0)
	
	float Density = 0.0f;

	switch (CurrentPhase)
	{
	case ETransitionPhase::Orbital:
		Density = 0.0f;
		break;
	case ETransitionPhase::HighAtmosphere:
		// Very light fog in high atmosphere
		Density = 0.1f;
		break;
	case ETransitionPhase::MidAtmosphere:
		// Moderate fog
		Density = 0.3f;
		break;
	case ETransitionPhase::LowAtmosphere:
		// Significant fog
		Density = 0.6f;
		break;
	case ETransitionPhase::Surface:
		// Full fog at surface
		Density = 1.0f;
		break;
	}

	// Smooth transition
	if (bIsTransitioning)
	{
		float TargetDensity = 0.0f;

		switch (TargetPhase)
		{
		case ETransitionPhase::Orbital:
			TargetDensity = 0.0f;
			break;
		case ETransitionPhase::HighAtmosphere:
			TargetDensity = 0.1f;
			break;
		case ETransitionPhase::MidAtmosphere:
			TargetDensity = 0.3f;
			break;
		case ETransitionPhase::LowAtmosphere:
			TargetDensity = 0.6f;
			break;
		case ETransitionPhase::Surface:
			TargetDensity = 1.0f;
			break;
		}

		Density = FMath::Lerp(Density, TargetDensity, TransitionProgress);
	}

	return FMath::Clamp(Density, 0.0f, 1.0f);
}

float UTransitionController::GetFogStartDistance() const
{
	// Fog starts closer at lower altitudes
	// Orbital: Very far (no visible fog)
	// Surface: Close (creates atmospheric haze)
	
	float StartDistanceKm = 1.0f; // Default 1km

	switch (CurrentPhase)
	{
	case ETransitionPhase::Orbital:
		// No fog at orbital
		StartDistanceKm = 100.0f;
		break;
	case ETransitionPhase::HighAtmosphere:
		StartDistanceKm = 50.0f;
		break;
	case ETransitionPhase::MidAtmosphere:
		StartDistanceKm = 20.0f;
		break;
	case ETransitionPhase::LowAtmosphere:
		StartDistanceKm = 5.0f;
		break;
	case ETransitionPhase::Surface:
		StartDistanceKm = 1.0f;
		break;
	}

	// Smooth transition
	if (bIsTransitioning)
	{
		float TargetStartDistanceKm = 1.0f;

		switch (TargetPhase)
		{
		case ETransitionPhase::Orbital:
			TargetStartDistanceKm = 100.0f;
			break;
		case ETransitionPhase::HighAtmosphere:
			TargetStartDistanceKm = 50.0f;
			break;
		case ETransitionPhase::MidAtmosphere:
			TargetStartDistanceKm = 20.0f;
			break;
		case ETransitionPhase::LowAtmosphere:
			TargetStartDistanceKm = 5.0f;
			break;
		case ETransitionPhase::Surface:
			TargetStartDistanceKm = 1.0f;
			break;
		}

		StartDistanceKm = FMath::Lerp(StartDistanceKm, TargetStartDistanceKm, TransitionProgress);
	}

	// Convert km to cm (Unreal units)
	return StartDistanceKm * 100000.0f;
}

float UTransitionController::GetFogHeightFalloff() const
{
	// Height falloff controls how quickly fog density decreases with altitude
	// Higher values = faster falloff
	
	float Falloff = 0.2f; // Default moderate falloff

	switch (CurrentPhase)
	{
	case ETransitionPhase::Orbital:
		// No fog, but set high falloff for when transitioning
		Falloff = 1.0f;
		break;
	case ETransitionPhase::HighAtmosphere:
		// Slow falloff in high atmosphere
		Falloff = 0.5f;
		break;
	case ETransitionPhase::MidAtmosphere:
		Falloff = 0.3f;
		break;
	case ETransitionPhase::LowAtmosphere:
		Falloff = 0.2f;
		break;
	case ETransitionPhase::Surface:
		// Slower falloff at surface for more uniform fog
		Falloff = 0.1f;
		break;
	}

	// Smooth transition
	if (bIsTransitioning)
	{
		float TargetFalloff = 0.2f;

		switch (TargetPhase)
		{
		case ETransitionPhase::Orbital:
			TargetFalloff = 1.0f;
			break;
		case ETransitionPhase::HighAtmosphere:
			TargetFalloff = 0.5f;
			break;
		case ETransitionPhase::MidAtmosphere:
			TargetFalloff = 0.3f;
			break;
		case ETransitionPhase::LowAtmosphere:
			TargetFalloff = 0.2f;
			break;
		case ETransitionPhase::Surface:
			TargetFalloff = 0.1f;
			break;
		}

		Falloff = FMath::Lerp(Falloff, TargetFalloff, TransitionProgress);
	}

	return FMath::Clamp(Falloff, 0.0f, 1.0f);
}

float UTransitionController::GetAtmosphericScatteringIntensity() const
{
	// Scattering intensity increases in atmosphere
	// Orbital: No scattering (0.0)
	// Atmosphere: Full scattering (1.0)
	
	float Intensity = 0.0f;

	switch (CurrentPhase)
	{
	case ETransitionPhase::Orbital:
		Intensity = 0.0f;
		break;
	case ETransitionPhase::HighAtmosphere:
		// Begin showing atmospheric scattering
		Intensity = 0.4f;
		break;
	case ETransitionPhase::MidAtmosphere:
		Intensity = 0.7f;
		break;
	case ETransitionPhase::LowAtmosphere:
		Intensity = 0.9f;
		break;
	case ETransitionPhase::Surface:
		// Full scattering at surface
		Intensity = 1.0f;
		break;
	}

	// Smooth transition
	if (bIsTransitioning)
	{
		float TargetIntensity = 0.0f;

		switch (TargetPhase)
		{
		case ETransitionPhase::Orbital:
			TargetIntensity = 0.0f;
			break;
		case ETransitionPhase::HighAtmosphere:
			TargetIntensity = 0.4f;
			break;
		case ETransitionPhase::MidAtmosphere:
			TargetIntensity = 0.7f;
			break;
		case ETransitionPhase::LowAtmosphere:
			TargetIntensity = 0.9f;
			break;
		case ETransitionPhase::Surface:
			TargetIntensity = 1.0f;
			break;
		}

		Intensity = FMath::Lerp(Intensity, TargetIntensity, TransitionProgress);
	}

	return FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UTransitionController::GetPostProcessSettings(float& OutExposure, float& OutBloomIntensity, float& OutVignetteIntensity) const
{
	// Adjust post-process settings based on altitude
	// These values create a more cinematic look during transitions
	
	float Exposure = 0.0f;
	float BloomIntensity = 0.5f;
	float VignetteIntensity = 0.0f;

	switch (CurrentPhase)
	{
	case ETransitionPhase::Orbital:
		// Brighter exposure in space
		Exposure = 0.5f;
		BloomIntensity = 0.3f;
		VignetteIntensity = 0.2f;
		break;
	case ETransitionPhase::HighAtmosphere:
		// Slight exposure adjustment entering atmosphere
		Exposure = 0.3f;
		BloomIntensity = 0.4f;
		VignetteIntensity = 0.15f;
		break;
	case ETransitionPhase::MidAtmosphere:
		// Neutral exposure
		Exposure = 0.0f;
		BloomIntensity = 0.5f;
		VignetteIntensity = 0.1f;
		break;
	case ETransitionPhase::LowAtmosphere:
		// Slightly darker as atmosphere thickens
		Exposure = -0.2f;
		BloomIntensity = 0.6f;
		VignetteIntensity = 0.05f;
		break;
	case ETransitionPhase::Surface:
		// Natural exposure at surface
		Exposure = 0.0f;
		BloomIntensity = 0.5f;
		VignetteIntensity = 0.0f;
		break;
	}

	// Smooth transition
	if (bIsTransitioning)
	{
		float TargetExposure = 0.0f;
		float TargetBloomIntensity = 0.5f;
		float TargetVignetteIntensity = 0.0f;

		switch (TargetPhase)
		{
		case ETransitionPhase::Orbital:
			TargetExposure = 0.5f;
			TargetBloomIntensity = 0.3f;
			TargetVignetteIntensity = 0.2f;
			break;
		case ETransitionPhase::HighAtmosphere:
			TargetExposure = 0.3f;
			TargetBloomIntensity = 0.4f;
			TargetVignetteIntensity = 0.15f;
			break;
		case ETransitionPhase::MidAtmosphere:
			TargetExposure = 0.0f;
			TargetBloomIntensity = 0.5f;
			TargetVignetteIntensity = 0.1f;
			break;
		case ETransitionPhase::LowAtmosphere:
			TargetExposure = -0.2f;
			TargetBloomIntensity = 0.6f;
			TargetVignetteIntensity = 0.05f;
			break;
		case ETransitionPhase::Surface:
			TargetExposure = 0.0f;
			TargetBloomIntensity = 0.5f;
			TargetVignetteIntensity = 0.0f;
			break;
		}

		Exposure = FMath::Lerp(Exposure, TargetExposure, TransitionProgress);
		BloomIntensity = FMath::Lerp(BloomIntensity, TargetBloomIntensity, TransitionProgress);
		VignetteIntensity = FMath::Lerp(VignetteIntensity, TargetVignetteIntensity, TransitionProgress);
	}

	OutExposure = FMath::Clamp(Exposure, -2.0f, 2.0f);
	OutBloomIntensity = FMath::Clamp(BloomIntensity, 0.0f, 1.0f);
	OutVignetteIntensity = FMath::Clamp(VignetteIntensity, 0.0f, 1.0f);
}

float UTransitionController::GetRenderingQualityScale() const
{
	// Adjust rendering quality based on altitude for performance
	// Can reduce quality at high altitudes where detail is less visible
	
	float QualityScale = 1.0f;

	switch (CurrentPhase)
	{
	case ETransitionPhase::Orbital:
		// Can reduce quality at orbital distances
		QualityScale = 0.7f;
		break;
	case ETransitionPhase::HighAtmosphere:
		QualityScale = 0.8f;
		break;
	case ETransitionPhase::MidAtmosphere:
		QualityScale = 0.9f;
		break;
	case ETransitionPhase::LowAtmosphere:
		QualityScale = 0.95f;
		break;
	case ETransitionPhase::Surface:
		// Full quality at surface
		QualityScale = 1.0f;
		break;
	}

	// Smooth transition
	if (bIsTransitioning)
	{
		float TargetQualityScale = 1.0f;

		switch (TargetPhase)
		{
		case ETransitionPhase::Orbital:
			TargetQualityScale = 0.7f;
			break;
		case ETransitionPhase::HighAtmosphere:
			TargetQualityScale = 0.8f;
			break;
		case ETransitionPhase::MidAtmosphere:
			TargetQualityScale = 0.9f;
			break;
		case ETransitionPhase::LowAtmosphere:
			TargetQualityScale = 0.95f;
			break;
		case ETransitionPhase::Surface:
			TargetQualityScale = 1.0f;
			break;
		}

		QualityScale = FMath::Lerp(QualityScale, TargetQualityScale, TransitionProgress);
	}

	return FMath::Clamp(QualityScale, 0.5f, 1.0f);
}
