// Copyright Epic Games, Inc. All Rights Reserved.

#include "TerrainTile.h"
#include "ProceduralNoiseGenerator.h"

FTerrainTileData UTerrainTileGenerator::GenerateTerrainTile(
	FVector2D TilePosition,
	float TileSize,
	int32 Resolution,
	int32 LODLevel,
	const FTerrainGenerationConfig& Config)
{
	FTerrainTileData TileData;
	TileData.WorldPosition = TilePosition;
	TileData.TileSize = TileSize;
	TileData.Resolution = Resolution;
	TileData.LODLevel = LODLevel;

	// Generate heightmap
	TileData.HeightData = GenerateHeightmap(TilePosition, TileSize, Resolution, Config);

	// Generate mesh from heightmap
	GenerateMeshFromHeightmap(
		TileData.HeightData,
		TileSize,
		Resolution,
		TileData.Vertices,
		TileData.Triangles,
		TileData.Normals,
		TileData.UVs,
		TileData.Tangents
	);

	TileData.bIsGenerated = true;

	return TileData;
}

TArray<float> UTerrainTileGenerator::GenerateHeightmap(
	FVector2D TilePosition,
	float TileSize,
	int32 Resolution,
	const FTerrainGenerationConfig& Config)
{
	TArray<float> HeightData;
	HeightData.SetNum(Resolution * Resolution);

	// Calculate tile bounds
	float HalfSize = TileSize * 0.5f;
	FVector2D TileMin = TilePosition - FVector2D(HalfSize, HalfSize);

	// Generate height for each vertex
	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			// Calculate world position
			float U = static_cast<float>(X) / (Resolution - 1);
			float V = static_cast<float>(Y) / (Resolution - 1);

			float WorldX = TileMin.X + U * TileSize;
			float WorldY = TileMin.Y + V * TileSize;

			// Get height at position
			float Height = GetHeightAtPosition(WorldX, WorldY, Config);

			// Store height
			int32 Index = Y * Resolution + X;
			HeightData[Index] = Height;
		}
	}

	return HeightData;
}

void UTerrainTileGenerator::GenerateMeshFromHeightmap(
	const TArray<float>& HeightData,
	float TileSize,
	int32 Resolution,
	TArray<FVector>& OutVertices,
	TArray<int32>& OutTriangles,
	TArray<FVector>& OutNormals,
	TArray<FVector2D>& OutUVs,
	TArray<FProcMeshTangent>& OutTangents)
{
	// Generate vertices
	OutVertices.SetNum(Resolution * Resolution);
	float HalfSize = TileSize * 0.5f;

	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			int32 Index = Y * Resolution + X;

			// Calculate local position
			float U = static_cast<float>(X) / (Resolution - 1);
			float V = static_cast<float>(Y) / (Resolution - 1);

			float LocalX = (U - 0.5f) * TileSize;
			float LocalY = (V - 0.5f) * TileSize;
			float LocalZ = HeightData[Index];

			OutVertices[Index] = FVector(LocalX, LocalY, LocalZ);
		}
	}

	// Generate triangle indices
	OutTriangles = GenerateTriangleIndices(Resolution);

	// Calculate normals
	OutNormals = CalculateNormals(HeightData, Resolution, TileSize);

	// Generate UVs
	OutUVs = GenerateUVs(Resolution);

	// Calculate tangents
	OutTangents = CalculateTangents(OutNormals);
}

TArray<FVector> UTerrainTileGenerator::CalculateNormals(
	const TArray<float>& HeightData,
	int32 Resolution,
	float TileSize)
{
	TArray<FVector> Normals;
	Normals.SetNum(Resolution * Resolution);

	float CellSize = TileSize / (Resolution - 1);

	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			int32 Index = Y * Resolution + X;

			// Get neighboring heights
			float HeightL = (X > 0) ? HeightData[Y * Resolution + (X - 1)] : HeightData[Index];
			float HeightR = (X < Resolution - 1) ? HeightData[Y * Resolution + (X + 1)] : HeightData[Index];
			float HeightD = (Y > 0) ? HeightData[(Y - 1) * Resolution + X] : HeightData[Index];
			float HeightU = (Y < Resolution - 1) ? HeightData[(Y + 1) * Resolution + X] : HeightData[Index];

			// Calculate tangent vectors
			FVector TangentX(CellSize * 2.0f, 0.0f, HeightR - HeightL);
			FVector TangentY(0.0f, CellSize * 2.0f, HeightU - HeightD);

			// Calculate normal as cross product
			FVector Normal = FVector::CrossProduct(TangentY, TangentX);
			Normal.Normalize();

			Normals[Index] = Normal;
		}
	}

	return Normals;
}

TArray<FProcMeshTangent> UTerrainTileGenerator::CalculateTangents(const TArray<FVector>& Normals)
{
	TArray<FProcMeshTangent> Tangents;
	Tangents.SetNum(Normals.Num());

	for (int32 I = 0; I < Normals.Num(); ++I)
	{
		FVector Normal = Normals[I];

		// Calculate tangent perpendicular to normal
		FVector Tangent;
		if (FMath::Abs(Normal.Z) < 0.999f)
		{
			Tangent = FVector::CrossProduct(FVector::UpVector, Normal);
		}
		else
		{
			Tangent = FVector::CrossProduct(FVector::ForwardVector, Normal);
		}
		Tangent.Normalize();

		Tangents[I] = FProcMeshTangent(Tangent, false);
	}

	return Tangents;
}

TArray<FVector2D> UTerrainTileGenerator::GenerateUVs(int32 Resolution)
{
	TArray<FVector2D> UVs;
	UVs.SetNum(Resolution * Resolution);

	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			int32 Index = Y * Resolution + X;

			float U = static_cast<float>(X) / (Resolution - 1);
			float V = static_cast<float>(Y) / (Resolution - 1);

			UVs[Index] = FVector2D(U, V);
		}
	}

	return UVs;
}

TArray<int32> UTerrainTileGenerator::GenerateTriangleIndices(int32 Resolution)
{
	TArray<int32> Triangles;
	int32 QuadCount = (Resolution - 1) * (Resolution - 1);
	Triangles.Reserve(QuadCount * 6); // 2 triangles per quad, 3 indices per triangle

	for (int32 Y = 0; Y < Resolution - 1; ++Y)
	{
		for (int32 X = 0; X < Resolution - 1; ++X)
		{
			int32 I0 = Y * Resolution + X;
			int32 I1 = Y * Resolution + (X + 1);
			int32 I2 = (Y + 1) * Resolution + X;
			int32 I3 = (Y + 1) * Resolution + (X + 1);

			// First triangle
			Triangles.Add(I0);
			Triangles.Add(I2);
			Triangles.Add(I1);

			// Second triangle
			Triangles.Add(I1);
			Triangles.Add(I2);
			Triangles.Add(I3);
		}
	}

	return Triangles;
}

float UTerrainTileGenerator::GetHeightAtPosition(
	float WorldX,
	float WorldY,
	const FTerrainGenerationConfig& Config)
{
	// Create noise configuration
	FNoiseConfig NoiseConfig;
	NoiseConfig.NoiseType = ENoiseType::Perlin;
	NoiseConfig.Seed = Config.Seed;
	NoiseConfig.Frequency = Config.NoiseScale;
	NoiseConfig.Amplitude = 1.0f;
	NoiseConfig.Octaves = Config.NoiseOctaves;
	NoiseConfig.Lacunarity = Config.NoiseLacunarity;
	NoiseConfig.Persistence = Config.NoisePersistence;
	NoiseConfig.bEnableDomainWarping = Config.bEnableDomainWarping;
	NoiseConfig.WarpStrength = Config.WarpStrength * Config.NoiseScale;
	NoiseConfig.WarpFrequency = Config.NoiseScale * 0.5f;

	// Generate noise value
	float NoiseValue = UProceduralNoiseGenerator::FractalNoise2D(WorldX, WorldY, NoiseConfig);

	// Convert from [-1, 1] to [0, 1]
	NoiseValue = (NoiseValue + 1.0f) * 0.5f;

	// Apply elevation range
	float Height = Config.BaseElevation + NoiseValue * Config.ElevationRange;

	return Height;
}
