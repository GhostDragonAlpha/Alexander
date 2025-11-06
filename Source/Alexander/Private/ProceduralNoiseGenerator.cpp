// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProceduralNoiseGenerator.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector.h"

UProceduralNoiseGenerator::UProceduralNoiseGenerator()
{
}

// ============================================================================
// BASIC NOISE FUNCTIONS
// ============================================================================

float UProceduralNoiseGenerator::PerlinNoise2D(float X, float Y, int32 Seed)
{
	// Get integer coordinates
	int32 X0 = FMath::FloorToInt(X);
	int32 Y0 = FMath::FloorToInt(Y);
	int32 X1 = X0 + 1;
	int32 Y1 = Y0 + 1;

	// Get fractional coordinates
	float Fx = X - X0;
	float Fy = Y - Y0;

	// Fade curves
	float U = Fade(Fx);
	float V = Fade(Fy);

	// Hash coordinates
	int32 AA = Hash(X0, Y0, Seed);
	int32 AB = Hash(X0, Y1, Seed);
	int32 BA = Hash(X1, Y0, Seed);
	int32 BB = Hash(X1, Y1, Seed);

	// Calculate gradients
	float G00 = Grad(AA, Fx, Fy);
	float G10 = Grad(BA, Fx - 1.0f, Fy);
	float G01 = Grad(AB, Fx, Fy - 1.0f);
	float G11 = Grad(BB, Fx - 1.0f, Fy - 1.0f);

	// Interpolate
	float Lerp0 = Lerp(G00, G10, U);
	float Lerp1 = Lerp(G01, G11, U);
	float Result = Lerp(Lerp0, Lerp1, V);

	return Result;
}

float UProceduralNoiseGenerator::SimplexNoise2D(float X, float Y, int32 Seed)
{
	// Simplex noise constants
	const float F2 = 0.5f * (FMath::Sqrt(3.0f) - 1.0f);
	const float G2 = (3.0f - FMath::Sqrt(3.0f)) / 6.0f;

	// Skew input space
	float S = (X + Y) * F2;
	int32 I = FMath::FloorToInt(X + S);
	int32 J = FMath::FloorToInt(Y + S);

	float T = (I + J) * G2;
	float X0 = X - (I - T);
	float Y0 = Y - (J - T);

	// Determine simplex
	int32 I1, J1;
	if (X0 > Y0)
	{
		I1 = 1;
		J1 = 0;
	}
	else
	{
		I1 = 0;
		J1 = 1;
	}

	// Offsets for corners
	float X1 = X0 - I1 + G2;
	float Y1 = Y0 - J1 + G2;
	float X2 = X0 - 1.0f + 2.0f * G2;
	float Y2 = Y0 - 1.0f + 2.0f * G2;

	// Calculate contributions
	float N0 = 0.0f, N1 = 0.0f, N2 = 0.0f;

	float T0 = 0.5f - X0 * X0 - Y0 * Y0;
	if (T0 > 0.0f)
	{
		T0 *= T0;
		N0 = T0 * T0 * Grad(Hash(I, J, Seed), X0, Y0);
	}

	float T1 = 0.5f - X1 * X1 - Y1 * Y1;
	if (T1 > 0.0f)
	{
		T1 *= T1;
		N1 = T1 * T1 * Grad(Hash(I + I1, J + J1, Seed), X1, Y1);
	}

	float T2 = 0.5f - X2 * X2 - Y2 * Y2;
	if (T2 > 0.0f)
	{
		T2 *= T2;
		N2 = T2 * T2 * Grad(Hash(I + 1, J + 1, Seed), X2, Y2);
	}

	// Scale to [-1, 1]
	return 70.0f * (N0 + N1 + N2);
}

float UProceduralNoiseGenerator::PerlinNoise3D(float X, float Y, float Z, int32 Seed)
{
	// Get integer coordinates
	int32 X0 = FMath::FloorToInt(X);
	int32 Y0 = FMath::FloorToInt(Y);
	int32 Z0 = FMath::FloorToInt(Z);
	int32 X1 = X0 + 1;
	int32 Y1 = Y0 + 1;
	int32 Z1 = Z0 + 1;

	// Get fractional coordinates
	float Fx = X - X0;
	float Fy = Y - Y0;
	float Fz = Z - Z0;

	// Fade curves
	float U = Fade(Fx);
	float V = Fade(Fy);
	float W = Fade(Fz);

	// Hash coordinates
	int32 AAA = Hash(X0, Y0, Z0, Seed);
	int32 ABA = Hash(X0, Y1, Z0, Seed);
	int32 AAB = Hash(X0, Y0, Z1, Seed);
	int32 ABB = Hash(X0, Y1, Z1, Seed);
	int32 BAA = Hash(X1, Y0, Z0, Seed);
	int32 BBA = Hash(X1, Y1, Z0, Seed);
	int32 BAB = Hash(X1, Y0, Z1, Seed);
	int32 BBB = Hash(X1, Y1, Z1, Seed);

	// Calculate gradients
	float G000 = Grad(AAA, Fx, Fy, Fz);
	float G100 = Grad(BAA, Fx - 1.0f, Fy, Fz);
	float G010 = Grad(ABA, Fx, Fy - 1.0f, Fz);
	float G110 = Grad(BBA, Fx - 1.0f, Fy - 1.0f, Fz);
	float G001 = Grad(AAB, Fx, Fy, Fz - 1.0f);
	float G101 = Grad(BAB, Fx - 1.0f, Fy, Fz - 1.0f);
	float G011 = Grad(ABB, Fx, Fy - 1.0f, Fz - 1.0f);
	float G111 = Grad(BBB, Fx - 1.0f, Fy - 1.0f, Fz - 1.0f);

	// Interpolate
	float Lerp00 = Lerp(G000, G100, U);
	float Lerp10 = Lerp(G010, G110, U);
	float Lerp01 = Lerp(G001, G101, U);
	float Lerp11 = Lerp(G011, G111, U);

	float Lerp0 = Lerp(Lerp00, Lerp10, V);
	float Lerp1 = Lerp(Lerp01, Lerp11, V);

	return Lerp(Lerp0, Lerp1, W);
}

// ============================================================================
// MULTI-OCTAVE NOISE
// ============================================================================

float UProceduralNoiseGenerator::FractalNoise2D(float X, float Y, const FNoiseConfig& Config)
{
	float Result = 0.0f;
	float Amplitude = Config.Amplitude;
	float Frequency = Config.Frequency;
	float MaxValue = 0.0f;

	for (int32 Octave = 0; Octave < Config.Octaves; ++Octave)
	{
		float NoiseValue = 0.0f;

		switch (Config.NoiseType)
		{
		case ENoiseType::Perlin:
			NoiseValue = PerlinNoise2D(X * Frequency, Y * Frequency, Config.Seed + Octave);
			break;
		case ENoiseType::Simplex:
			NoiseValue = SimplexNoise2D(X * Frequency, Y * Frequency, Config.Seed + Octave);
			break;
		default:
			NoiseValue = PerlinNoise2D(X * Frequency, Y * Frequency, Config.Seed + Octave);
			break;
		}

		Result += NoiseValue * Amplitude;
		MaxValue += Amplitude;

		Amplitude *= Config.Persistence;
		Frequency *= Config.Lacunarity;
	}

	// Normalize to [-1, 1]
	return Result / MaxValue;
}

float UProceduralNoiseGenerator::RidgedMultifractalNoise2D(float X, float Y, const FNoiseConfig& Config)
{
	float Result = 0.0f;
	float Amplitude = Config.Amplitude;
	float Frequency = Config.Frequency;
	float Weight = 1.0f;

	for (int32 Octave = 0; Octave < Config.Octaves; ++Octave)
	{
		float NoiseValue = 0.0f;

		switch (Config.NoiseType)
		{
		case ENoiseType::Perlin:
			NoiseValue = PerlinNoise2D(X * Frequency, Y * Frequency, Config.Seed + Octave);
			break;
		case ENoiseType::Simplex:
			NoiseValue = SimplexNoise2D(X * Frequency, Y * Frequency, Config.Seed + Octave);
			break;
		default:
			NoiseValue = PerlinNoise2D(X * Frequency, Y * Frequency, Config.Seed + Octave);
			break;
		}

		// Invert and sharpen
		NoiseValue = 1.0f - FMath::Abs(NoiseValue);
		NoiseValue *= NoiseValue;

		// Weight successive octaves
		NoiseValue *= Weight;
		Weight = FMath::Clamp(NoiseValue * 2.0f, 0.0f, 1.0f);

		Result += NoiseValue * Amplitude;
		Amplitude *= Config.Persistence;
		Frequency *= Config.Lacunarity;
	}

	return Result;
}

// ============================================================================
// VORONOI NOISE
// ============================================================================

float UProceduralNoiseGenerator::VoronoiNoise2D(float X, float Y, int32 Seed, float CellSize)
{
	// Scale coordinates
	X /= CellSize;
	Y /= CellSize;

	// Get cell coordinates
	int32 CellX = FMath::FloorToInt(X);
	int32 CellY = FMath::FloorToInt(Y);

	float MinDist = FLT_MAX;

	// Check neighboring cells
	for (int32 OffsetY = -1; OffsetY <= 1; ++OffsetY)
	{
		for (int32 OffsetX = -1; OffsetX <= 1; ++OffsetX)
		{
			int32 NeighborX = CellX + OffsetX;
			int32 NeighborY = CellY + OffsetY;

			// Get random point in cell
			FVector2D CellPoint = GetRandomVector2D(NeighborX, NeighborY, Seed);
			CellPoint.X += NeighborX;
			CellPoint.Y += NeighborY;

			// Calculate distance
			float Dx = X - CellPoint.X;
			float Dy = Y - CellPoint.Y;
			float Dist = FMath::Sqrt(Dx * Dx + Dy * Dy);

			MinDist = FMath::Min(MinDist, Dist);
		}
	}

	return MinDist;
}

FVoronoiCell UProceduralNoiseGenerator::GetVoronoiCell(float X, float Y, int32 Seed, float CellSize)
{
	// Scale coordinates
	X /= CellSize;
	Y /= CellSize;

	// Get cell coordinates
	int32 CellX = FMath::FloorToInt(X);
	int32 CellY = FMath::FloorToInt(Y);

	float MinDist = FLT_MAX;
	FVoronoiCell ClosestCell;

	// Check neighboring cells
	for (int32 OffsetY = -1; OffsetY <= 1; ++OffsetY)
	{
		for (int32 OffsetX = -1; OffsetX <= 1; ++OffsetX)
		{
			int32 NeighborX = CellX + OffsetX;
			int32 NeighborY = CellY + OffsetY;

			// Get random point in cell
			FVector2D CellPoint = GetRandomVector2D(NeighborX, NeighborY, Seed);
			CellPoint.X += NeighborX;
			CellPoint.Y += NeighborY;

			// Calculate distance
			float Dx = X - CellPoint.X;
			float Dy = Y - CellPoint.Y;
			float Dist = FMath::Sqrt(Dx * Dx + Dy * Dy);

			if (Dist < MinDist)
			{
				MinDist = Dist;
				ClosestCell.Position = CellPoint * CellSize;
				ClosestCell.Value = MinDist;
				ClosestCell.CellID = Hash(NeighborX, NeighborY, Seed);
			}
		}
	}

	return ClosestCell;
}

// ============================================================================
// DOMAIN WARPING
// ============================================================================

void UProceduralNoiseGenerator::ApplyDomainWarping(float& X, float& Y, const FNoiseConfig& Config)
{
	if (!Config.bEnableDomainWarping)
		return;

	// Generate offset noise
	float OffsetX = PerlinNoise2D(X * Config.WarpFrequency, Y * Config.WarpFrequency, Config.Seed + 1000);
	float OffsetY = PerlinNoise2D(X * Config.WarpFrequency, Y * Config.WarpFrequency, Config.Seed + 2000);

	// Apply warping
	X += OffsetX * Config.WarpStrength;
	Y += OffsetY * Config.WarpStrength;
}

float UProceduralNoiseGenerator::WarpedNoise2D(float X, float Y, const FNoiseConfig& Config)
{
	// Apply domain warping
	float WarpedX = X;
	float WarpedY = Y;
	ApplyDomainWarping(WarpedX, WarpedY, Config);

	// Generate noise at warped coordinates
	return FractalNoise2D(WarpedX, WarpedY, Config);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

float UProceduralNoiseGenerator::Fade(float T)
{
	// 6t^5 - 15t^4 + 10t^3
	return T * T * T * (T * (T * 6.0f - 15.0f) + 10.0f);
}

float UProceduralNoiseGenerator::Lerp(float A, float B, float T)
{
	return A + T * (B - A);
}

float UProceduralNoiseGenerator::Grad(int32 Hash, float X, float Y)
{
	// Convert hash to gradient direction
	int32 H = Hash & 3;
	float U = (H & 1) ? -X : X;
	float V = (H & 2) ? -Y : Y;
	return U + V;
}

float UProceduralNoiseGenerator::Grad(int32 Hash, float X, float Y, float Z)
{
	// Convert hash to gradient direction
	int32 H = Hash & 15;
	float U = H < 8 ? X : Y;
	float V = H < 4 ? Y : (H == 12 || H == 14 ? X : Z);
	return ((H & 1) ? -U : U) + ((H & 2) ? -V : V);
}

int32 UProceduralNoiseGenerator::Hash(int32 X, int32 Y, int32 Seed)
{
	// Simple hash function
	int32 Hash = Seed;
	Hash = (Hash ^ X) * 0x45d9f3b;
	Hash = (Hash ^ Y) * 0x45d9f3b;
	Hash = (Hash ^ (Hash >> 16)) * 0x45d9f3b;
	return Hash;
}

int32 UProceduralNoiseGenerator::Hash(int32 X, int32 Y, int32 Z, int32 Seed)
{
	// Simple hash function
	int32 Hash = Seed;
	Hash = (Hash ^ X) * 0x45d9f3b;
	Hash = (Hash ^ Y) * 0x45d9f3b;
	Hash = (Hash ^ Z) * 0x45d9f3b;
	Hash = (Hash ^ (Hash >> 16)) * 0x45d9f3b;
	return Hash;
}

FVector2D UProceduralNoiseGenerator::GetRandomVector2D(int32 X, int32 Y, int32 Seed)
{
	int32 H = Hash(X, Y, Seed);
	float Angle = (H & 0xFFFF) / 65535.0f * 2.0f * PI;
	return FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * 0.5f + FVector2D(0.5f, 0.5f);
}

FVector UProceduralNoiseGenerator::GetRandomVector3D(int32 X, int32 Y, int32 Z, int32 Seed)
{
	int32 H = Hash(X, Y, Z, Seed);
	
	// Generate random point in unit cube using hash
	float Rx = ((H & 0xFF) / 255.0f);
	float Ry = (((H >> 8) & 0xFF) / 255.0f);
	float Rz = (((H >> 16) & 0xFF) / 255.0f);
	
	return FVector(Rx, Ry, Rz);
}

TArray<int32> UProceduralNoiseGenerator::GeneratePermutationTable(int32 Seed)
{
	TArray<int32> Permutation;
	Permutation.SetNum(512);

	// Initialize with sequential values
	for (int32 I = 0; I < 256; ++I)
	{
		Permutation[I] = I;
	}

	// Shuffle using seed
	FRandomStream Random(Seed);
	for (int32 I = 255; I > 0; --I)
	{
		int32 J = Random.RandRange(0, I);
		int32 Temp = Permutation[I];
		Permutation[I] = Permutation[J];
		Permutation[J] = Temp;
	}

	// Duplicate for wrapping
	for (int32 I = 0; I < 256; ++I)
	{
		Permutation[256 + I] = Permutation[I];
	}

	return Permutation;
}

// ============================================================================
// WORLEY NOISE (CELLULAR NOISE)
// ============================================================================

float UProceduralNoiseGenerator::WorleyNoise3D(float X, float Y, float Z, int32 Seed, float CellSize)
{
	// Scale coordinates
	X /= CellSize;
	Y /= CellSize;
	Z /= CellSize;

	// Get cell coordinates
	int32 CellX = FMath::FloorToInt(X);
	int32 CellY = FMath::FloorToInt(Y);
	int32 CellZ = FMath::FloorToInt(Z);

	float MinDist = FLT_MAX;

	// Check neighboring cells (3x3x3 grid)
	for (int32 OffsetZ = -1; OffsetZ <= 1; ++OffsetZ)
	{
		for (int32 OffsetY = -1; OffsetY <= 1; ++OffsetY)
		{
			for (int32 OffsetX = -1; OffsetX <= 1; ++OffsetX)
			{
				int32 NeighborX = CellX + OffsetX;
				int32 NeighborY = CellY + OffsetY;
				int32 NeighborZ = CellZ + OffsetZ;

				// Get random point in cell
				FVector CellPoint = GetRandomVector3D(NeighborX, NeighborY, NeighborZ, Seed);
				CellPoint.X += NeighborX;
				CellPoint.Y += NeighborY;
				CellPoint.Z += NeighborZ;

				// Calculate distance
				float Dx = X - CellPoint.X;
				float Dy = Y - CellPoint.Y;
				float Dz = Z - CellPoint.Z;
				float Dist = FMath::Sqrt(Dx * Dx + Dy * Dy + Dz * Dz);

				MinDist = FMath::Min(MinDist, Dist);
			}
		}
	}

	return MinDist;
}

void UProceduralNoiseGenerator::WorleyNoise3DMulti(float X, float Y, float Z, int32 Seed, float CellSize, float& OutF1, float& OutF2)
{
	// Scale coordinates
	X /= CellSize;
	Y /= CellSize;
	Z /= CellSize;

	// Get cell coordinates
	int32 CellX = FMath::FloorToInt(X);
	int32 CellY = FMath::FloorToInt(Y);
	int32 CellZ = FMath::FloorToInt(Z);

	float MinDist1 = FLT_MAX;
	float MinDist2 = FLT_MAX;

	// Check neighboring cells (3x3x3 grid)
	for (int32 OffsetZ = -1; OffsetZ <= 1; ++OffsetZ)
	{
		for (int32 OffsetY = -1; OffsetY <= 1; ++OffsetY)
		{
			for (int32 OffsetX = -1; OffsetX <= 1; ++OffsetX)
			{
				int32 NeighborX = CellX + OffsetX;
				int32 NeighborY = CellY + OffsetY;
				int32 NeighborZ = CellZ + OffsetZ;

				// Get random point in cell
				FVector CellPoint = GetRandomVector3D(NeighborX, NeighborY, NeighborZ, Seed);
				CellPoint.X += NeighborX;
				CellPoint.Y += NeighborY;
				CellPoint.Z += NeighborZ;

				// Calculate distance
				float Dx = X - CellPoint.X;
				float Dy = Y - CellPoint.Y;
				float Dz = Z - CellPoint.Z;
				float Dist = FMath::Sqrt(Dx * Dx + Dy * Dy + Dz * Dz);

				// Update closest and second closest distances
				if (Dist < MinDist1)
				{
					MinDist2 = MinDist1;
					MinDist1 = Dist;
				}
				else if (Dist < MinDist2)
				{
					MinDist2 = Dist;
				}
			}
		}
	}

	OutF1 = MinDist1;
	OutF2 = MinDist2;
}

float UProceduralNoiseGenerator::PerlinWorleyNoise3D(float X, float Y, float Z, int32 Seed, float PerlinScale, float WorleyScale, float WorleyWeight)
{
	// Generate Perlin noise
	float PerlinValue = PerlinNoise3D(X * PerlinScale, Y * PerlinScale, Z * PerlinScale, Seed);
	
	// Normalize Perlin from [-1, 1] to [0, 1]
	PerlinValue = (PerlinValue + 1.0f) * 0.5f;
	
	// Generate Worley noise
	float WorleyValue = WorleyNoise3D(X, Y, Z, Seed + 1000, WorleyScale);
	
	// Normalize Worley (typical range is 0 to ~0.866 for 3D)
	WorleyValue = FMath::Clamp(WorleyValue / 0.866f, 0.0f, 1.0f);
	
	// Invert Worley so cells are high values (clouds) and edges are low
	WorleyValue = 1.0f - WorleyValue;
	
	// Combine: Use Perlin as base shape, subtract Worley to create cellular structure
	// This creates the characteristic "cauliflower" cloud appearance
	float Result = PerlinValue - (WorleyValue * WorleyWeight);
	
	// Clamp to valid range
	return FMath::Clamp(Result, 0.0f, 1.0f);
}

float UProceduralNoiseGenerator::FractalPerlinWorleyNoise3D(float X, float Y, float Z, int32 Seed, int32 Octaves, 
	float Frequency, float Lacunarity, float Persistence, float WorleyWeight)
{
	float Result = 0.0f;
	float Amplitude = 1.0f;
	float MaxValue = 0.0f;
	float CurrentFrequency = Frequency;

	for (int32 Octave = 0; Octave < Octaves; ++Octave)
	{
		// Generate Perlin-Worley noise at current frequency
		float NoiseValue = PerlinWorleyNoise3D(
			X * CurrentFrequency, 
			Y * CurrentFrequency, 
			Z * CurrentFrequency, 
			Seed + Octave,
			1.0f,  // PerlinScale (already scaled by frequency)
			1.0f,  // WorleyScale (already scaled by frequency)
			WorleyWeight
		);

		Result += NoiseValue * Amplitude;
		MaxValue += Amplitude;

		Amplitude *= Persistence;
		CurrentFrequency *= Lacunarity;
	}

	// Normalize to [0, 1]
	return Result / MaxValue;
}

float UProceduralNoiseGenerator::GenerateIceFormations(float X, float Y, float Z, int32 Seed, float Temperature)
{
	// Use Perlin noise to create ice crystal patterns
	float IceNoise = PerlinNoise3D(X * 0.5f, Y * 0.5f, Z * 0.5f, Seed);
	
	// Normalize to [0, 1]
	IceNoise = (IceNoise + 1.0f) * 0.5f;
	
	// Temperature factor: colder = more ice
	float TemperatureFactor = FMath::Max(0.0f, 1.0f - (Temperature / 100.0f));
	
	// Combine with octaves for complexity (fractal-like)
	float DetailNoise = PerlinNoise3D(X * 2.0f, Y * 2.0f, Z * 2.0f, Seed + 1);
	DetailNoise = (DetailNoise + 1.0f) * 0.5f;
	
	float ComplexNoise = PerlinNoise3D(X * 4.0f, Y * 4.0f, Z * 4.0f, Seed + 2);
	ComplexNoise = (ComplexNoise + 1.0f) * 0.5f;
	
	// Combine layers
	float FractalIce = (IceNoise * 0.5f + DetailNoise * 0.3f + ComplexNoise * 0.2f) / 1.0f;
	
	// Weight by temperature
	float Result = FractalIce * TemperatureFactor;
	
	return FMath::Clamp(Result, 0.0f, 1.0f);
}

float UProceduralNoiseGenerator::BlendTerrainLayers(const TArray<float>& Layers, const TArray<float>& Weights)
{
	if (Layers.Num() == 0)
	{
		return 0.0f;
	}
	
	if (Layers.Num() != Weights.Num())
	{
		// Weights and layers must be same size
		return Layers[0];
	}
	
	float BlendedValue = 0.0f;
	float TotalWeight = 0.0f;
	
	for (int32 i = 0; i < Layers.Num(); ++i)
	{
		BlendedValue += Layers[i] * Weights[i];
		TotalWeight += Weights[i];
	}
	
	if (TotalWeight > 0.0f)
	{
		BlendedValue /= TotalWeight;
	}
	
	return BlendedValue;
}

FVector UProceduralNoiseGenerator::SphericalToCartesian(float Lat, float Lon, float Radius)
{
	// Convert latitude/longitude (in degrees) to Cartesian coordinates
	float LatRad = FMath::DegreesToRadians(Lat);
	float LonRad = FMath::DegreesToRadians(Lon);
	
	float X = Radius * FMath::Cos(LatRad) * FMath::Cos(LonRad);
	float Y = Radius * FMath::Cos(LatRad) * FMath::Sin(LonRad);
	float Z = Radius * FMath::Sin(LatRad);
	
	return FVector(X, Y, Z);
}

FVector2D UProceduralNoiseGenerator::CartesianToSpherical(FVector Position, float Radius)
{
	// Convert Cartesian coordinates to latitude/longitude (in degrees)
	float Lat = FMath::RadiansToDegrees(FMath::Asin(Position.Z / Radius));
	float Lon = FMath::RadiansToDegrees(FMath::Atan2(Position.Y, Position.X));
	
	return FVector2D(Lat, Lon);
}

float UProceduralNoiseGenerator::GenerateContinentalTerrain(float X, float Y, int32 Seed, float PlanetRadius)
{
	// Continental terrain generation using layered Perlin noise
	float BaseNoise = PerlinNoise2D(X * 0.1f, Y * 0.1f, Seed);
	BaseNoise = (BaseNoise + 1.0f) * 0.5f;
	
	// Add detail layers
	float DetailNoise = PerlinNoise2D(X * 0.5f, Y * 0.5f, Seed + 1);
	DetailNoise = (DetailNoise + 1.0f) * 0.5f;
	
	float FineNoise = PerlinNoise2D(X * 2.0f, Y * 2.0f, Seed + 2);
	FineNoise = (FineNoise + 1.0f) * 0.5f;
	
	// Blend layers: large scale (50%) + medium (30%) + fine (20%)
	float Result = BaseNoise * 0.5f + DetailNoise * 0.3f + FineNoise * 0.2f;
	
	return FMath::Clamp(Result, 0.0f, 1.0f);
}

float UProceduralNoiseGenerator::GenerateRiverNetwork(float X, float Y, int32 Seed, float TerrainHeight)
{
	// River networks based on terrain height gradients
	// Create main river channels using low-frequency noise
	float RiverNoise = PerlinNoise2D(X * 0.05f, Y * 0.05f, Seed + 100);
	RiverNoise = (RiverNoise + 1.0f) * 0.5f;
	
	// Add tributaries with higher frequency
	float TributaryNoise = PerlinNoise2D(X * 0.2f, Y * 0.2f, Seed + 101);
	TributaryNoise = (TributaryNoise + 1.0f) * 0.5f;
	
	// Combine: main rivers follow low noise, tributaries branch from high noise
	float Result = RiverNoise * 0.7f + TributaryNoise * 0.3f;
	
	// Modulate by terrain height (rivers flow downward from high terrain)
	Result *= (1.0f - FMath::Clamp(TerrainHeight, 0.0f, 1.0f));
	
	return FMath::Clamp(Result, 0.0f, 1.0f);
}

float UProceduralNoiseGenerator::GenerateCaveSystem(float X, float Y, float Z, int32 Seed)
{
	// 3D cave system generation using Perlin noise
	float CaveNoise = PerlinNoise3D(X * 0.1f, Y * 0.1f, Z * 0.1f, Seed + 200);
	CaveNoise = (CaveNoise + 1.0f) * 0.5f;
	
	// Add larger cave chambers
	float ChamberNoise = PerlinNoise3D(X * 0.05f, Y * 0.05f, Z * 0.05f, Seed + 201);
	ChamberNoise = (ChamberNoise + 1.0f) * 0.5f;
	
	// Add fine detail (cave decorations)
	float DetailNoise = PerlinNoise3D(X * 0.3f, Y * 0.3f, Z * 0.3f, Seed + 202);
	DetailNoise = (DetailNoise + 1.0f) * 0.5f;
	
	// Combine layers
	float Result = CaveNoise * 0.4f + ChamberNoise * 0.4f + DetailNoise * 0.2f;
	
	return FMath::Clamp(Result, 0.0f, 1.0f);
}

void UProceduralNoiseGenerator::ApplyThermalErosion(TArray<float>& HeightMap, int32 Width, int32 Height, int32 Iterations, float TalusAngle)
{
	// Thermal erosion: material falls from steep slopes
	if (HeightMap.Num() == 0 || Width <= 0 || Height <= 0)
	{
		return;
	}
	
	TArray<float> ErosionMap = HeightMap;
	float TalusAngleRad = FMath::DegreesToRadians(TalusAngle);
	float TalusSlope = FMath::Tan(TalusAngleRad);
	
	for (int32 Iteration = 0; Iteration < Iterations; ++Iteration)
	{
		for (int32 Y = 1; Y < Height - 1; ++Y)
		{
			for (int32 X = 1; X < Width - 1; ++X)
			{
				int32 CenterIdx = Y * Width + X;
				float CenterHeight = HeightMap[CenterIdx];
				
				// Check 8 neighbors
				for (int32 DY = -1; DY <= 1; ++DY)
				{
					for (int32 DX = -1; DX <= 1; ++DX)
					{
						if (DX == 0 && DY == 0) continue;
						
						int32 NeighborIdx = (Y + DY) * Width + (X + DX);
						float NeighborHeight = HeightMap[NeighborIdx];
						float HeightDiff = CenterHeight - NeighborHeight;
						
						// Distance accounting for diagonal neighbors
						float Distance = (DX != 0 && DY != 0) ? 1.414f : 1.0f;
						float Slope = HeightDiff / Distance;
						
						if (Slope > TalusSlope && HeightDiff > 0.0f)
						{
							// Material slides down
							float Material = HeightDiff - TalusSlope * Distance;
							ErosionMap[CenterIdx] -= Material * 0.5f;
							ErosionMap[NeighborIdx] += Material * 0.5f;
						}
					}
				}
			}
		}
		HeightMap = ErosionMap;
	}
}

void UProceduralNoiseGenerator::ApplyHydraulicErosion(TArray<float>& HeightMap, int32 Width, int32 Height, int32 Droplets, float Solubility, float EvaporationRate, float Friction, float Gravity, float CapacityFactor)
{
	// Hydraulic erosion: water droplets carve valleys
	if (HeightMap.Num() == 0 || Width <= 0 || Height <= 0 || Droplets <= 0)
	{
		return;
	}
	
	for (int32 DropletIdx = 0; DropletIdx < Droplets; ++DropletIdx)
	{
		// Random starting position
		float PosX = FMath::Rand() % Width;
		float PosY = FMath::Rand() % Height;
		float Velocity = 0.0f;
		float Water = 1.0f;
		float Sediment = 0.0f;
		
		for (int32 Step = 0; Step < 50; ++Step)
		{
			int32 NodeX = FMath::FloorToInt(PosX);
			int32 NodeY = FMath::FloorToInt(PosY);
			
			// Clamp to bounds
			if (NodeX < 0 || NodeX >= Width - 1 || NodeY < 0 || NodeY >= Height - 1)
				break;
			
			int32 NodeIdx = NodeY * Width + NodeX;
			float CurrentHeight = HeightMap[NodeIdx];
			
			// Find lowest neighbor
			float LowestHeight = CurrentHeight;
			int32 LowestIdx = NodeIdx;
			
			for (int32 DY = -1; DY <= 1; ++DY)
			{
				for (int32 DX = -1; DX <= 1; ++DX)
				{
					if (DX == 0 && DY == 0) continue;
					int32 NeighborIdx = (NodeY + DY) * Width + (NodeX + DX);
					if (HeightMap[NeighborIdx] < LowestHeight)
					{
						LowestHeight = HeightMap[NeighborIdx];
						LowestIdx = NeighborIdx;
					}
				}
			}
			
			// Accelerate downhill
			float HeightDiff = CurrentHeight - LowestHeight;
			Velocity = FMath::Sqrt(Velocity * Velocity + HeightDiff * Gravity);
			Velocity *= (1.0f - Friction);
			
			// Erosion based on velocity
			float Capacity = CapacityFactor * Velocity * Water;
			if (Sediment > Capacity)
			{
				// Deposit excess sediment
				float Deposit = Solubility * (Sediment - Capacity);
				HeightMap[NodeIdx] += Deposit;
				Sediment -= Deposit;
			}
			else
			{
				// Erode
				float Erosion = Solubility * (Capacity - Sediment);
				Erosion = FMath::Min(Erosion, Capacity);
				HeightMap[NodeIdx] -= Erosion;
				Sediment += Erosion;
			}
			
			// Evaporate water
			Water *= (1.0f - EvaporationRate);
			if (Water < 0.01f) break;
			
			// Move to lowest neighbor
			PosX = LowestIdx % Width;
			PosY = LowestIdx / Width;
		}
	}
}

float UProceduralNoiseGenerator::GenerateBiomeFeatures(float X, float Y, EBiomeType BiomeType, int32 Seed, float BaseHeight)
{
	// Generate biome-specific features
	float BiomeFeature = 0.0f;
	
	switch (BiomeType)
	{
		case EBiomeType::Desert:
		{
			// Smooth dunes using low-frequency noise
			float DuneNoise = PerlinNoise2D(X * 0.05f, Y * 0.05f, Seed + 300);
			BiomeFeature = (DuneNoise + 1.0f) * 0.5f;
			break;
		}
		case EBiomeType::Forest:
		{
			// Varied terrain for forest
			float ForestNoise = PerlinNoise2D(X * 0.2f, Y * 0.2f, Seed + 301);
			BiomeFeature = (ForestNoise + 1.0f) * 0.5f;
			break;
		}
		case EBiomeType::Mountain:
		{
			// Rough, high-frequency terrain
			float MountainNoise = PerlinNoise2D(X * 0.5f, Y * 0.5f, Seed + 302);
			MountainNoise = (MountainNoise + 1.0f) * 0.5f;
			
			// Add sharp ridges
			float RidgeNoise = PerlinNoise2D(X * 1.0f, Y * 1.0f, Seed + 303);
			RidgeNoise = (RidgeNoise + 1.0f) * 0.5f;
			RidgeNoise = FMath::Abs(RidgeNoise - 0.5f) * 2.0f; // Create ridges
			
			BiomeFeature = MountainNoise * 0.6f + RidgeNoise * 0.4f;
			break;
		}
		case EBiomeType::Tundra:
		{
			// Flat with small variations
			float TundraNoiseFloat = PerlinNoise2D(X * 0.15f, Y * 0.15f, Seed + 304);
			BiomeFeature = (TundraNoiseFloat + 1.0f) * 0.5f * 0.5f; // Reduced variation
			break;
		}
		default:
		{
			// Generic biome
			float GenericNoise = PerlinNoise2D(X * 0.1f, Y * 0.1f, Seed + 305);
			BiomeFeature = (GenericNoise + 1.0f) * 0.5f;
			break;
		}
	}
	
	// Blend with base height
	return BaseHeight * 0.7f + BiomeFeature * 0.3f;
}

float UProceduralNoiseGenerator::GenerateVolcanicTerrain(float X, float Y, int32 Seed, float BaseHeight)
{
	// Volcanic terrain with sharp peaks
	float VulcanicNoise = PerlinNoise2D(X * 0.1f, Y * 0.1f, Seed + 400);
	VulcanicNoise = (VulcanicNoise + 1.0f) * 0.5f;
	
	// Create volcanic cones
	float DistFromCenter = FMath::Sqrt(X * X + Y * Y);
	float ConeNoise = FMath::Max(0.0f, 1.0f - DistFromCenter * 0.01f);
	
	// Add lava flow patterns (sharp ridges)
	float LavaNoise = PerlinNoise2D(X * 0.3f, Y * 0.3f, Seed + 401);
	LavaNoise = FMath::Abs(LavaNoise); // Ridges
	
	float Result = BaseHeight + VulcanicNoise * 0.3f + ConeNoise * 0.5f + LavaNoise * 0.2f;
	
	return FMath::Clamp(Result, 0.0f, 1.0f);
}

float UProceduralNoiseGenerator::GenerateCraterField(float X, float Y, int32 Seed, float CraterDensity, float MaxCraterSize)
{
	// Generate impact craters
	float CraterNoise = PerlinNoise2D(X * 0.1f, Y * 0.1f, Seed + 500);
	CraterNoise = (CraterNoise + 1.0f) * 0.5f;
	
	// Dense crater pattern
	float CraterPattern = PerlinNoise2D(X * CraterDensity, Y * CraterDensity, Seed + 501);
	CraterPattern = (CraterPattern + 1.0f) * 0.5f;
	
	// Crater size varies
	float CraterSize = PerlinNoise2D(X * 0.5f, Y * 0.5f, Seed + 502);
	CraterSize = (CraterSize + 1.0f) * 0.5f * MaxCraterSize;
	
	// Create depression effect (negative height)
	float Result = CraterNoise * 0.5f - CraterPattern * CraterSize * 0.5f;
	
	return FMath::Clamp(Result, 0.0f, 1.0f);
}
