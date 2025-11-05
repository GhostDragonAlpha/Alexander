// Copyright Epic Games, Inc. All Rights Reserved.

#include "AstronomicalConstants.h"
#include "Net/UnrealNetwork.h"

// ============================================================================
// FVirtualPosition network serialization moved to PlayerOriginManager.cpp

// ============================================================================
// UAstronomicalConstantsLibrary Implementation
// ============================================================================

FCelestialBodyData UAstronomicalConstantsLibrary::GetCelestialBodyData(const FString& BodyName)
{
	// Normalize body name for comparison
	FString NormalizedName = BodyName.ToLower();
	
	if (NormalizedName == TEXT("sun"))
	{
		return FCelestialBodyData(
			CelestialScalingConstants::SolSystem::Sun::Mass,
			CelestialScalingConstants::SolSystem::Sun::Radius,
			CelestialScalingConstants::SolSystem::Sun::StandardGravParam,
			0.0,  // Sun doesn't orbit anything
			0.0,
			TEXT("Sun")
		);
	}
	else if (NormalizedName == TEXT("earth"))
	{
		return FCelestialBodyData(
			CelestialScalingConstants::SolSystem::Earth::Mass,
			CelestialScalingConstants::SolSystem::Earth::Radius,
			CelestialScalingConstants::SolSystem::Earth::StandardGravParam,
			CelestialScalingConstants::SolSystem::Earth::OrbitalRadius,
			CelestialScalingConstants::SolSystem::Earth::OrbitalPeriod,
			TEXT("Earth")
		);
	}
	else if (NormalizedName == TEXT("moon"))
	{
		return FCelestialBodyData(
			CelestialScalingConstants::SolSystem::Moon::Mass,
			CelestialScalingConstants::SolSystem::Moon::Radius,
			CelestialScalingConstants::SolSystem::Moon::StandardGravParam,
			CelestialScalingConstants::SolSystem::Moon::OrbitalRadius,
			CelestialScalingConstants::SolSystem::Moon::OrbitalPeriod,
			TEXT("Moon")
		);
	}
	else if (NormalizedName == TEXT("mars"))
	{
		return FCelestialBodyData(
			CelestialScalingConstants::SolSystem::Mars::Mass,
			CelestialScalingConstants::SolSystem::Mars::Radius,
			CelestialScalingConstants::SolSystem::Mars::StandardGravParam,
			CelestialScalingConstants::SolSystem::Mars::OrbitalRadius,
			CelestialScalingConstants::SolSystem::Mars::OrbitalPeriod,
			TEXT("Mars")
		);
	}
	else if (NormalizedName == TEXT("jupiter"))
	{
		return FCelestialBodyData(
			CelestialScalingConstants::SolSystem::Jupiter::Mass,
			CelestialScalingConstants::SolSystem::Jupiter::Radius,
			CelestialScalingConstants::SolSystem::Jupiter::StandardGravParam,
			CelestialScalingConstants::SolSystem::Jupiter::OrbitalRadius,
			CelestialScalingConstants::SolSystem::Jupiter::OrbitalPeriod,
			TEXT("Jupiter")
		);
	}
	
	// Return empty data if body not found
	return FCelestialBodyData();
}

float UAstronomicalConstantsLibrary::CalculateSphereOfInfluence(double SemiMajorAxis, double PrimaryMass, double SecondaryMass)
{
	// Laplace sphere of influence: r_SOI = a * (m_secondary / m_primary)^(2/5)
	if (PrimaryMass <= 0.0 || SecondaryMass <= 0.0 || SemiMajorAxis <= 0.0)
	{
		return 0.0f;
	}
	
	double MassRatio = SecondaryMass / PrimaryMass;
	double SOI = SemiMajorAxis * FMath::Pow(MassRatio, 0.4); // 2/5 = 0.4
	
	return static_cast<float>(SOI);
}

FVector UAstronomicalConstantsLibrary::CalculateGravitationalForce(double Mass1, double Mass2, const FVector& Position1, const FVector& Position2)
{
	// F = G * M1 * M2 / r²
	FVector Direction = Position2 - Position1;
	double Distance = Direction.Size();
	
	if (Distance < SMALL_NUMBER)
	{
		return FVector::ZeroVector;
	}
	
	Direction.Normalize();
	
	// Using gravitational constant in km³/kg/s²
	double ForceMagnitude = (CelestialScalingConstants::SolSystem::G * Mass1 * Mass2) / (Distance * Distance);
	
	// Convert to Unreal units (1 UU = 1 cm)
	// Force is already in m/s², convert to cm/s²
	FVector Force = Direction * ForceMagnitude * 100.0; // m to cm
	
	return Force;
}

float UAstronomicalConstantsLibrary::CalculateEscapeVelocity(double Mass, double Radius)
{
	// v_escape = sqrt(2 * G * M / R)
	if (Mass <= 0.0 || Radius <= 0.0)
	{
		return 0.0f;
	}
	
	// Convert radius from km to m
	double RadiusM = Radius * 1000.0;
	
	// Calculate escape velocity in m/s
	double VelocitySquared = 2.0 * CelestialScalingConstants::SolSystem::G * 1000000.0 * Mass / RadiusM; // G is in km³, convert to m³
	double Velocity = FMath::Sqrt(VelocitySquared);
	
	return static_cast<float>(Velocity);
}

bool UAstronomicalConstantsLibrary::IsValidScaleFactor(float ScaleFactor)
{
	return ScaleFactor >= CelestialScalingConstants::MinScaleFactor && 
	       ScaleFactor <= CelestialScalingConstants::MaxScaleFactor;
}

float UAstronomicalConstantsLibrary::ClampScaleFactor(float ScaleFactor)
{
	return FMath::Clamp(ScaleFactor, 
	                    CelestialScalingConstants::MinScaleFactor,
	                    CelestialScalingConstants::MaxScaleFactor);
}

bool UAstronomicalConstantsLibrary::RequiresRecentering(const FVector& PositionOffset)
{
	// Convert from Unreal units (cm) to km
	double OffsetKm = PositionOffset.Size() / 100000.0;
	
	return OffsetKm >= CelestialScalingConstants::RecenterThreshold;
}

