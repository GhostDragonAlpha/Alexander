// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlanetFarmingComponent.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Planet.h"

UPlanetFarmingComponent::UPlanetFarmingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlanetFarmingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlanetFarmingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	UpdateFarms(DeltaTime);
}

AAlexanderFarmPlot* UPlanetFarmingComponent::CreateFarmPlot(FVector Location, FVector2D Size)
{
	// Check if location is suitable for farming
	if (!IsSuitableForFarming(Location))
	{
		UE_LOG(LogTemp, Warning, TEXT("Location not suitable for farming"));
		return nullptr;
	}
	
	// This functionality is now handled by FarmingSubsystem
	UE_LOG(LogTemp, Warning, TEXT("PlanetFarmingComponent::CreateFarmPlot is deprecated - use FarmingSubsystem instead"));
	return nullptr;
}

bool UPlanetFarmingComponent::IsSuitableForFarming(FVector Location) const
{
	if (!OwningPlanet)
	{
		return false;
	}

	// Calculate slope using terrain normal
	// Sample nearby points to determine slope
	const float SampleDistance = 50.0f;

	FVector Right = Location + FVector(SampleDistance, 0, 0);
	FVector Left = Location + FVector(-SampleDistance, 0, 0);
	FVector Forward = Location + FVector(0, SampleDistance, 0);
	FVector Back = Location + FVector(0, -SampleDistance, 0);

	// Get terrain heights at each point (using placeholder - would query actual terrain)
	// For now, use simple noise-based approach
	float HeightCenter = Location.Z;
	float HeightRight = Right.Z + FMath::PerlinNoise2D(FVector2D(Right.X * 0.01f, Right.Y * 0.01f)) * 10.0f;
	float HeightLeft = Left.Z + FMath::PerlinNoise2D(FVector2D(Left.X * 0.01f, Left.Y * 0.01f)) * 10.0f;
	float HeightForward = Forward.Z + FMath::PerlinNoise2D(FVector2D(Forward.X * 0.01f, Forward.Y * 0.01f)) * 10.0f;
	float HeightBack = Back.Z + FMath::PerlinNoise2D(FVector2D(Back.X * 0.01f, Back.Y * 0.01f)) * 10.0f;

	// Calculate gradients
	FVector TangentX = FVector(SampleDistance * 2.0f, 0, HeightRight - HeightLeft);
	FVector TangentY = FVector(0, SampleDistance * 2.0f, HeightForward - HeightBack);

	// Calculate normal and slope angle
	FVector Normal = FVector::CrossProduct(TangentY, TangentX);
	Normal.Normalize();

	float SlopeAngle = FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)) * (180.0f / PI);

	// Maximum slope for farming is typically around 15-20 degrees
	const float MaxFarmingSlope = 15.0f;
	if (SlopeAngle > MaxFarmingSlope)
	{
		return false;
	}

	// Check soil quality based on biome
	float BaseQuality = GetBaseSoilQuality(Location);
	if (BaseQuality < 0.2f) // Too poor for farming
	{
		return false;
	}

	// Check water availability based on proximity to water bodies
	// Use noise to determine water table depth and surface water proximity
	float WaterNoise = FMath::PerlinNoise2D(FVector2D(Location.X * 0.001f, Location.Y * 0.001f));
	WaterNoise = (WaterNoise + 1.0f) * 0.5f; // Normalize to [0, 1]

	// Water availability is based on:
	// 1. Surface water nearby (low elevation areas)
	// 2. Water table depth (noise-based approximation)
	// 3. Rainfall/climate (biome-based)
	float ElevationFactor = FMath::Clamp(1.0f - (HeightCenter / 1000.0f), 0.0f, 1.0f);
	float WaterTableFactor = WaterNoise;
	float WaterAvailability = (ElevationFactor * 0.6f) + (WaterTableFactor * 0.4f);

	const float MinWaterAvailability = 0.3f;
	if (WaterAvailability < MinWaterAvailability)
	{
		return false;
	}

	return true;
}

void UPlanetFarmingComponent::UpdateFarms(float DeltaTime)
{
	// This functionality is now handled by FarmingSubsystem
	// Remove destroyed farms
	ActiveFarms.RemoveAll([](AAlexanderFarmPlot* Farm) { return !IsValid(Farm); });
}

float UPlanetFarmingComponent::GetBaseSoilQuality(FVector Location) const
{
	// Placeholder - will be based on biome type
	// Default to medium quality
	return 0.7f;
}

void UPlanetFarmingComponent::InitializeFarming(APlanet* Planet)
{
	OwningPlanet = Planet;
	UE_LOG(LogTemp, Log, TEXT("PlanetFarmingComponent initialized for planet"));
}
