// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProceduralNoiseGenerator.h"
#include "Math/UnrealMathUtility.h"

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
