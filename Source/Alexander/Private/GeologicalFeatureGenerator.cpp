// Copyright Epic Games, Inc. All Rights Reserved.

#include "GeologicalFeatureGenerator.h"
#include "ProceduralNoiseGenerator.h"
#include "Math/UnrealMathUtility.h"

// ============================================================================
// MOUNTAIN GENERATION
// ============================================================================

float UGeologicalFeatureGenerator::GenerateMountainRange(
	float X, float Y,
	const FMountainRangeConfig& Config,
	int32 Seed)
{
	// Rotate point to align with mountain range
	FVector2D Point(X - Config.Position.X, Y - Config.Position.Y);
	Point = RotatePoint(Point, -Config.Rotation);

	// Calculate distance along and perpendicular to range
	float AlongRange = Point.X;
	float AcrossRange = FMath::Abs(Point.Y);

	// Check if within range bounds
	float HalfLength = Config.Length * 0.5f;
	if (FMath::Abs(AlongRange) > HalfLength)
		return 0.0f;

	// Calculate base height profile (Gaussian-like)
	float WidthFalloff = FalloffCurve(AcrossRange, Config.Width * 0.5f, 0.7f);
	float LengthFalloff = FalloffCurve(FMath::Abs(AlongRange), HalfLength, 0.5f);

	if (WidthFalloff <= 0.0f || LengthFalloff <= 0.0f)
		return 0.0f;

	// Generate ridged noise for mountain peaks
	FNoiseConfig NoiseConfig;
	NoiseConfig.NoiseType = ENoiseType::RidgedMultifractal;
	NoiseConfig.Seed = Seed;
	NoiseConfig.Frequency = 0.001f;
	NoiseConfig.Amplitude = 1.0f;
	NoiseConfig.Octaves = 6;
	NoiseConfig.Lacunarity = 2.0f;
	NoiseConfig.Persistence = 0.5f;

	float NoiseValue = UProceduralNoiseGenerator::RidgedMultifractalNoise2D(X, Y, NoiseConfig);

	// Add detail noise
	FNoiseConfig DetailConfig = NoiseConfig;
	DetailConfig.Frequency = 0.005f;
	DetailConfig.Octaves = 4;
	float DetailNoise = UProceduralNoiseGenerator::FractalNoise2D(X, Y, DetailConfig);
	DetailNoise = (DetailNoise + 1.0f) * 0.5f; // Convert to [0, 1]

	// Combine noise with roughness
	float CombinedNoise = FMath::Lerp(0.5f, NoiseValue, Config.Roughness);
	CombinedNoise = FMath::Lerp(CombinedNoise, DetailNoise, 0.3f);

	// Calculate final height
	float Height = Config.Height * WidthFalloff * LengthFalloff * CombinedNoise;

	return Height;
}

float UGeologicalFeatureGenerator::GenerateMountainRanges(
	float X, float Y,
	const TArray<FMountainRangeConfig>& Ranges,
	int32 Seed)
{
	float TotalHeight = 0.0f;

	for (int32 I = 0; I < Ranges.Num(); ++I)
	{
		float Height = GenerateMountainRange(X, Y, Ranges[I], Seed + I * 1000);
		TotalHeight = FMath::Max(TotalHeight, Height); // Use max to avoid stacking
	}

	return TotalHeight;
}

// ============================================================================
// VALLEY GENERATION
// ============================================================================

float UGeologicalFeatureGenerator::GenerateValley(float X, float Y, const FValleyConfig& Config)
{
	FVector2D Point(X, Y);

	// Calculate distance to valley centerline
	float Distance = DistanceToLineSegment(Point, Config.StartPosition, Config.EndPosition);

	// Calculate valley profile
	float HalfWidth = Config.Width * 0.5f;
	if (Distance > HalfWidth)
		return 0.0f;

	// Smooth valley profile (parabolic)
	float NormalizedDist = Distance / HalfWidth;
	float Profile = 1.0f - NormalizedDist * NormalizedDist;

	// Apply smoothness
	Profile = FMath::Lerp(Profile, SmoothStep(0.0f, 1.0f, Profile), Config.Smoothness);

	// Return negative height (depression)
	return -Config.Depth * Profile;
}

float UGeologicalFeatureGenerator::GenerateValleySystem(
	float X, float Y,
	const TArray<FValleyConfig>& Valleys)
{
	float TotalDepth = 0.0f;

	for (const FValleyConfig& Valley : Valleys)
	{
		float Depth = GenerateValley(X, Y, Valley);
		TotalDepth += Depth; // Accumulate depressions
	}

	return TotalDepth;
}

// ============================================================================
// CANYON GENERATION
// ============================================================================

float UGeologicalFeatureGenerator::GenerateCanyon(float X, float Y, const FCanyonConfig& Config)
{
	if (Config.PathPoints.Num() < 2)
		return 0.0f;

	FVector2D Point(X, Y);

	// Calculate distance to canyon path
	float Distance = DistanceToPolyline(Point, Config.PathPoints);

	// Calculate canyon profile
	float HalfWidth = Config.Width * 0.5f;
	if (Distance > HalfWidth)
		return 0.0f;

	// Steep canyon walls
	float NormalizedDist = Distance / HalfWidth;
	float Profile = 1.0f - FMath::Pow(NormalizedDist, 1.0f / Config.Steepness);

	// Return negative height (depression)
	return -Config.Depth * Profile;
}

float UGeologicalFeatureGenerator::GenerateCanyonSystem(
	float X, float Y,
	const TArray<FCanyonConfig>& Canyons)
{
	float TotalDepth = 0.0f;

	for (const FCanyonConfig& Canyon : Canyons)
	{
		float Depth = GenerateCanyon(X, Y, Canyon);
		TotalDepth = FMath::Min(TotalDepth, Depth); // Use min for deepest canyon
	}

	return TotalDepth;
}

// ============================================================================
// CRATER GENERATION
// ============================================================================

float UGeologicalFeatureGenerator::GenerateCrater(float X, float Y, const FCraterConfig& Config)
{
	FVector2D Point(X, Y);
	FVector2D CraterCenter = Config.Position;

	float Distance = FVector2D::Distance(Point, CraterCenter);

	// Outside crater influence
	if (Distance > Config.Radius + Config.RimWidth)
		return 0.0f;

	float Height = 0.0f;

	// Crater rim
	if (Distance > Config.Radius - Config.RimWidth && Distance < Config.Radius + Config.RimWidth)
	{
		float RimDist = FMath::Abs(Distance - Config.Radius);
		float RimProfile = 1.0f - (RimDist / Config.RimWidth);
		RimProfile = SmoothStep(0.0f, 1.0f, RimProfile);
		Height = Config.RimHeight * RimProfile;
	}

	// Crater bowl
	if (Distance < Config.Radius)
	{
		float BowlProfile = Distance / Config.Radius;
		BowlProfile = 1.0f - BowlProfile * BowlProfile; // Parabolic bowl
		float BowlDepth = -Config.Depth * BowlProfile;

		// Combine rim and bowl
		Height += BowlDepth;
	}

	return Height;
}

float UGeologicalFeatureGenerator::GenerateCraters(
	float X, float Y,
	const TArray<FCraterConfig>& Craters)
{
	float TotalHeight = 0.0f;

	for (const FCraterConfig& Crater : Craters)
	{
		float Height = GenerateCrater(X, Y, Crater);
		TotalHeight += Height;
	}

	return TotalHeight;
}

float UGeologicalFeatureGenerator::GenerateCraterField(
	float X, float Y,
	int32 Seed,
	float Density,
	float MinRadius,
	float MaxRadius)
{
	// Use Voronoi cells to place craters
	float CellSize = 1000.0f / FMath::Sqrt(Density); // Approximate cell size based on density

	FVoronoiCell Cell = UProceduralNoiseGenerator::GetVoronoiCell(X, Y, Seed, CellSize);

	// Use cell ID to determine if crater exists and its properties
	FRandomStream Random(Cell.CellID);

	// Not all cells have craters
	if (Random.FRand() > Density / 10.0f)
		return 0.0f;

	// Generate crater properties
	FCraterConfig CraterConfig;
	CraterConfig.Position = Cell.Position;
	CraterConfig.Radius = Random.FRandRange(MinRadius, MaxRadius);
	CraterConfig.Depth = CraterConfig.Radius * Random.FRandRange(0.1f, 0.3f);
	CraterConfig.RimHeight = CraterConfig.Depth * Random.FRandRange(0.1f, 0.2f);
	CraterConfig.RimWidth = CraterConfig.Radius * Random.FRandRange(0.1f, 0.3f);

	return GenerateCrater(X, Y, CraterConfig);
}

// ============================================================================
// PLATEAU GENERATION
// ============================================================================

float UGeologicalFeatureGenerator::GeneratePlateau(float X, float Y, const FPlateauConfig& Config)
{
	FVector2D Point(X, Y);
	float Distance = FVector2D::Distance(Point, Config.Position);

	// Outside plateau influence
	if (Distance > Config.Radius)
		return 0.0f;

	// Plateau profile with steep edges
	float NormalizedDist = Distance / Config.Radius;
	float Profile = 1.0f - FMath::Pow(NormalizedDist, 1.0f / Config.EdgeSteepness);

	return Config.Height * Profile;
}

float UGeologicalFeatureGenerator::GeneratePlateaus(
	float X, float Y,
	const TArray<FPlateauConfig>& Plateaus)
{
	float TotalHeight = 0.0f;

	for (const FPlateauConfig& Plateau : Plateaus)
	{
		float Height = GeneratePlateau(X, Y, Plateau);
		TotalHeight = FMath::Max(TotalHeight, Height); // Use max to avoid stacking
	}

	return TotalHeight;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

float UGeologicalFeatureGenerator::SmoothStep(float Edge0, float Edge1, float X)
{
	float T = FMath::Clamp((X - Edge0) / (Edge1 - Edge0), 0.0f, 1.0f);
	return T * T * (3.0f - 2.0f * T);
}

float UGeologicalFeatureGenerator::DistanceToLineSegment(
	FVector2D Point,
	FVector2D LineStart,
	FVector2D LineEnd)
{
	FVector2D Line = LineEnd - LineStart;
	float LineLength = Line.Size();

	if (LineLength < SMALL_NUMBER)
		return FVector2D::Distance(Point, LineStart);

	// Project point onto line
	float T = FVector2D::DotProduct(Point - LineStart, Line) / (LineLength * LineLength);
	T = FMath::Clamp(T, 0.0f, 1.0f);

	FVector2D Projection = LineStart + T * Line;
	return FVector2D::Distance(Point, Projection);
}

float UGeologicalFeatureGenerator::DistanceToPolyline(
	FVector2D Point,
	const TArray<FVector2D>& PathPoints)
{
	if (PathPoints.Num() < 2)
		return FLT_MAX;

	float MinDistance = FLT_MAX;

	for (int32 I = 0; I < PathPoints.Num() - 1; ++I)
	{
		float Distance = DistanceToLineSegment(Point, PathPoints[I], PathPoints[I + 1]);
		MinDistance = FMath::Min(MinDistance, Distance);
	}

	return MinDistance;
}

FVector2D UGeologicalFeatureGenerator::RotatePoint(FVector2D Point, float AngleDegrees)
{
	float AngleRad = FMath::DegreesToRadians(AngleDegrees);
	float CosAngle = FMath::Cos(AngleRad);
	float SinAngle = FMath::Sin(AngleRad);

	return FVector2D(
		Point.X * CosAngle - Point.Y * SinAngle,
		Point.X * SinAngle + Point.Y * CosAngle
	);
}

float UGeologicalFeatureGenerator::FalloffCurve(float Distance, float Radius, float Steepness)
{
	if (Distance >= Radius)
		return 0.0f;

	float NormalizedDist = Distance / Radius;
	return 1.0f - FMath::Pow(NormalizedDist, 1.0f / Steepness);
}
