// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "TerrainTile.generated.h"

/**
 * Terrain tile data structure
 */
USTRUCT(BlueprintType)
struct FTerrainTileData
{
	GENERATED_BODY()

	// Tile identification
	UPROPERTY(BlueprintReadWrite, Category = "Tile")
	FVector2D WorldPosition;

	UPROPERTY(BlueprintReadWrite, Category = "Tile")
	int32 LODLevel = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Tile")
	float TileSize = 1000.0f; // meters

	UPROPERTY(BlueprintReadWrite, Category = "Tile")
	int32 Resolution = 64; // vertices per side

	// Mesh data
	UPROPERTY()
	TArray<FVector> Vertices;

	UPROPERTY()
	TArray<int32> Triangles;

	UPROPERTY()
	TArray<FVector> Normals;

	UPROPERTY()
	TArray<FVector2D> UVs;

	UPROPERTY()
	TArray<FLinearColor> VertexColors;

	UPROPERTY()
	TArray<FProcMeshTangent> Tangents;

	// Heightmap data
	UPROPERTY()
	TArray<float> HeightData;

	// State
	UPROPERTY(BlueprintReadWrite, Category = "Tile")
	bool bIsGenerated = false;

	UPROPERTY(BlueprintReadWrite, Category = "Tile")
	bool bIsVisible = false;

	// Clear all data
	void Clear()
	{
		Vertices.Empty();
		Triangles.Empty();
		Normals.Empty();
		UVs.Empty();
		VertexColors.Empty();
		Tangents.Empty();
		HeightData.Empty();
		bIsGenerated = false;
	}

	// Get height at local coordinates (0-1 range)
	float GetHeightAtLocalCoords(float U, float V) const
	{
		if (HeightData.Num() == 0 || Resolution <= 0)
			return 0.0f;

		// Clamp to valid range
		U = FMath::Clamp(U, 0.0f, 1.0f);
		V = FMath::Clamp(V, 0.0f, 1.0f);

		// Convert to grid coordinates
		float X = U * (Resolution - 1);
		float Y = V * (Resolution - 1);

		int32 X0 = FMath::FloorToInt(X);
		int32 Y0 = FMath::FloorToInt(Y);
		int32 X1 = FMath::Min(X0 + 1, Resolution - 1);
		int32 Y1 = FMath::Min(Y0 + 1, Resolution - 1);

		float Fx = X - X0;
		float Fy = Y - Y0;

		// Bilinear interpolation
		float H00 = HeightData[Y0 * Resolution + X0];
		float H10 = HeightData[Y0 * Resolution + X1];
		float H01 = HeightData[Y1 * Resolution + X0];
		float H11 = HeightData[Y1 * Resolution + X1];

		float H0 = FMath::Lerp(H00, H10, Fx);
		float H1 = FMath::Lerp(H01, H11, Fx);

		return FMath::Lerp(H0, H1, Fy);
	}
};

/**
 * Terrain tile generation configuration
 */
USTRUCT(BlueprintType)
struct FTerrainGenerationConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 Seed = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float BaseElevation = 0.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float ElevationRange = 1000.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float NoiseScale = 0.001f; // Scale factor for noise coordinates

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 NoiseOctaves = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float NoisePersistence = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float NoiseLacunarity = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	bool bEnableDomainWarping = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float WarpStrength = 50.0f;
};

/**
 * Terrain tile generator subsystem
 */
UCLASS()
class ALEXANDER_API UTerrainTileGenerator : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Generate terrain tile with heightmap and mesh data
	 * @param TilePosition World position of tile center
	 * @param TileSize Size of tile in meters
	 * @param Resolution Number of vertices per side
	 * @param LODLevel Level of detail (0 = highest)
	 * @param Config Generation configuration
	 * @return Generated terrain tile data
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static FTerrainTileData GenerateTerrainTile(
		FVector2D TilePosition,
		float TileSize,
		int32 Resolution,
		int32 LODLevel,
		const FTerrainGenerationConfig& Config
	);

	/**
	 * Generate heightmap for terrain tile
	 * @param TilePosition World position of tile center
	 * @param TileSize Size of tile in meters
	 * @param Resolution Number of height samples per side
	 * @param Config Generation configuration
	 * @return Array of height values
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static TArray<float> GenerateHeightmap(
		FVector2D TilePosition,
		float TileSize,
		int32 Resolution,
		const FTerrainGenerationConfig& Config
	);

	/**
	 * Generate mesh from heightmap data
	 * @param HeightData Height values
	 * @param TileSize Size of tile in meters
	 * @param Resolution Number of vertices per side
	 * @param OutVertices Generated vertices
	 * @param OutTriangles Generated triangle indices
	 * @param OutNormals Generated normals
	 * @param OutUVs Generated UV coordinates
	 * @param OutTangents Generated tangents
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static void GenerateMeshFromHeightmap(
		const TArray<float>& HeightData,
		float TileSize,
		int32 Resolution,
		TArray<FVector>& OutVertices,
		TArray<int32>& OutTriangles,
		TArray<FVector>& OutNormals,
		TArray<FVector2D>& OutUVs,
		TArray<FProcMeshTangent>& OutTangents
	);

	/**
	 * Calculate normals from heightmap
	 * @param HeightData Height values
	 * @param Resolution Number of vertices per side
	 * @param TileSize Size of tile in meters
	 * @return Array of normal vectors
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static TArray<FVector> CalculateNormals(
		const TArray<float>& HeightData,
		int32 Resolution,
		float TileSize
	);

	/**
	 * Calculate tangents from normals
	 * @param Normals Normal vectors
	 * @return Array of tangent vectors
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static TArray<FProcMeshTangent> CalculateTangents(const TArray<FVector>& Normals);

	/**
	 * Generate UV coordinates for terrain tile
	 * @param Resolution Number of vertices per side
	 * @return Array of UV coordinates
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static TArray<FVector2D> GenerateUVs(int32 Resolution);

	/**
	 * Generate triangle indices for terrain mesh
	 * @param Resolution Number of vertices per side
	 * @return Array of triangle indices
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static TArray<int32> GenerateTriangleIndices(int32 Resolution);

private:
	/**
	 * Get height value at world position using noise
	 * @param WorldX World X coordinate
	 * @param WorldY World Y coordinate
	 * @param Config Generation configuration
	 * @return Height value in meters
	 */
	static float GetHeightAtPosition(float WorldX, float WorldY, const FTerrainGenerationConfig& Config);
};
