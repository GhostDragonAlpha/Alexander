// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ProceduralNoiseGenerator.generated.h"

/**
 * Noise type enumeration
 */
UENUM(BlueprintType)
enum class ENoiseType : uint8
{
	Perlin UMETA(DisplayName = "Perlin"),
	Simplex UMETA(DisplayName = "Simplex"),
	RidgedMultifractal UMETA(DisplayName = "Ridged Multifractal"),
	Voronoi UMETA(DisplayName = "Voronoi")
};

/**
 * Noise configuration structure
 */
USTRUCT(BlueprintType)
struct FNoiseConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	ENoiseType NoiseType = ENoiseType::Perlin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	int32 Seed = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Frequency = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Amplitude = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	int32 Octaves = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Lacunarity = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Persistence = 0.5f;

	// Domain warping parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domain Warping")
	bool bEnableDomainWarping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domain Warping")
	float WarpStrength = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domain Warping")
	float WarpFrequency = 0.5f;
};

/**
 * Voronoi cell data
 */
USTRUCT(BlueprintType)
struct FVoronoiCell
{
	GENERATED_BODY()

	UPROPERTY()
	FVector2D Position;

	UPROPERTY()
	float Value;

	UPROPERTY()
	int32 CellID;
};

/**
 * Procedural noise generator for terrain generation
 */
UCLASS(BlueprintType)
class ALEXANDER_API UProceduralNoiseGenerator : public UObject
{
	GENERATED_BODY()

public:
	UProceduralNoiseGenerator();

	// ============================================================================
	// BASIC NOISE FUNCTIONS
	// ============================================================================

	/**
	 * Generate 2D Perlin noise value
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Seed Random seed
	 * @return Noise value in range [-1, 1]
	 */
	UFUNCTION(BlueprintCallable, Category = "Noise")
	static float PerlinNoise2D(float X, float Y, int32 Seed);

	/**
	 * Generate 2D Simplex noise value
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Seed Random seed
	 * @return Noise value in range [-1, 1]
	 */
	UFUNCTION(BlueprintCallable, Category = "Noise")
	static float SimplexNoise2D(float X, float Y, int32 Seed);

	/**
	 * Generate 3D Perlin noise value
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Z Z coordinate
	 * @param Seed Random seed
	 * @return Noise value in range [-1, 1]
	 */
	UFUNCTION(BlueprintCallable, Category = "Noise")
	static float PerlinNoise3D(float X, float Y, float Z, int32 Seed);

	// ============================================================================
	// MULTI-OCTAVE NOISE
	// ============================================================================

	/**
	 * Generate multi-octave Perlin noise (Fractal Brownian Motion)
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Config Noise configuration
	 * @return Noise value in range [-1, 1]
	 */
	UFUNCTION(BlueprintCallable, Category = "Noise")
	static float FractalNoise2D(float X, float Y, const FNoiseConfig& Config);

	/**
	 * Generate ridged multifractal noise for mountains
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Config Noise configuration
	 * @return Noise value in range [0, 1]
	 */
	UFUNCTION(BlueprintCallable, Category = "Noise")
	static float RidgedMultifractalNoise2D(float X, float Y, const FNoiseConfig& Config);

	// ============================================================================
	// VORONOI NOISE
	// ============================================================================

	/**
	 * Generate Voronoi cell noise for craters
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Seed Random seed
	 * @param CellSize Size of Voronoi cells
	 * @return Distance to nearest cell center
	 */
	UFUNCTION(BlueprintCallable, Category = "Noise")
	static float VoronoiNoise2D(float X, float Y, int32 Seed, float CellSize);

	/**
	 * Get Voronoi cell information at position
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Seed Random seed
	 * @param CellSize Size of Voronoi cells
	 * @return Voronoi cell data
	 */
	UFUNCTION(BlueprintCallable, Category = "Noise")
	static FVoronoiCell GetVoronoiCell(float X, float Y, int32 Seed, float CellSize);

	// ============================================================================
	// WORLEY NOISE (CELLULAR NOISE)
	// ============================================================================

	/**
	 * Generate 3D Worley noise (cellular noise)
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Z Z coordinate
	 * @param Seed Random seed
	 * @param CellSize Size of cells
	 * @return Distance to nearest cell center
	 */
	UFUNCTION(BlueprintCallable, Category = "Noise")
	static float WorleyNoise3D(float X, float Y, float Z, int32 Seed, float CellSize);

	/**
	 * Generate 3D Worley noise with multiple distance values (F1, F2)
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Z Z coordinate
	 * @param Seed Random seed
	 * @param CellSize Size of cells
	 * @param OutF1 Distance to closest cell
	 * @param OutF2 Distance to second closest cell
	 */
	UFUNCTION(BlueprintCallable, Category = "Noise")
	static void WorleyNoise3DMulti(float X, float Y, float Z, int32 Seed, float CellSize, float& OutF1, float& OutF2);

	/**
	 * Generate 3D Perlin-Worley noise for cloud shapes
	 * Combines Perlin noise with Worley noise for realistic cloud formations
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Z Z coordinate
	 * @param Seed Random seed
	 * @param PerlinScale Scale of Perlin noise
	 * @param WorleyScale Scale of Worley noise
	 * @param WorleyWeight Weight of Worley noise (0-1)
	 * @return Combined noise value in range [0, 1]
	 */
	UFUNCTION(BlueprintCallable, Category = "Noise")
	static float PerlinWorleyNoise3D(float X, float Y, float Z, int32 Seed, float PerlinScale, float WorleyScale, float WorleyWeight);

	/**
	 * Generate multi-octave 3D Perlin-Worley noise for detailed clouds
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Z Z coordinate
	 * @param Seed Random seed
	 * @param Octaves Number of octaves
	 * @param Frequency Base frequency
	 * @param Lacunarity Frequency multiplier per octave
	 * @param Persistence Amplitude multiplier per octave
	 * @param WorleyWeight Weight of Worley noise (0-1)
	 * @return Combined noise value in range [0, 1]
	 */
	UFUNCTION(BlueprintCallable, Category = "Noise")
	static float FractalPerlinWorleyNoise3D(float X, float Y, float Z, int32 Seed, int32 Octaves, 
		float Frequency, float Lacunarity, float Persistence, float WorleyWeight);

	// ============================================================================
	// DOMAIN WARPING
	// ============================================================================

	/**
	 * Apply domain warping to coordinates for natural features
	 * @param X X coordinate (will be modified)
	 * @param Y Y coordinate (will be modified)
	 * @param Config Noise configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Noise")
	static void ApplyDomainWarping(float& X, float& Y, const FNoiseConfig& Config);

	/**
	 * Generate noise with domain warping applied
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Config Noise configuration
	 * @return Warped noise value
	 */
	UFUNCTION(BlueprintCallable, Category = "Noise")
	static float WarpedNoise2D(float X, float Y, const FNoiseConfig& Config);

private:
	// ============================================================================
	// HELPER FUNCTIONS
	// ============================================================================

	// Fade function for smooth interpolation
	static float Fade(float T);

	// Linear interpolation
	static float Lerp(float A, float B, float T);

	// Gradient function for Perlin noise
	static float Grad(int32 Hash, float X, float Y);
	static float Grad(int32 Hash, float X, float Y, float Z);

	// Hash function for pseudo-random values
	static int32 Hash(int32 X, int32 Y, int32 Seed);
	static int32 Hash(int32 X, int32 Y, int32 Z, int32 Seed);

	// Get random 2D vector for cell
	static FVector2D GetRandomVector2D(int32 X, int32 Y, int32 Seed);

	// Get random 3D vector for cell
	static FVector GetRandomVector3D(int32 X, int32 Y, int32 Z, int32 Seed);

	// Permutation table for noise generation
	static TArray<int32> GeneratePermutationTable(int32 Seed);

	// ============================================================================
	// ADVANCED TERRAIN FEATURES
	// ============================================================================

	/**
	 * Generate continental-scale terrain using multiple noise layers
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Seed Random seed
	 * @param PlanetRadius Planet radius in km
	 * @return Terrain height in meters
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static float GenerateContinentalTerrain(float X, float Y, int32 Seed, float PlanetRadius);

	/**
	 * Generate river networks using flow accumulation
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Seed Random seed
	 * @param TerrainHeight Existing terrain height
	 * @return River depth (negative values carve rivers)
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static float GenerateRiverNetwork(float X, float Y, int32 Seed, float TerrainHeight);

	/**
	 * Generate cave systems using 3D noise
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Z Z coordinate (depth)
	 * @param Seed Random seed
	 * @return Cave density (0 = solid, 1 = open cave)
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static float GenerateCaveSystem(float X, float Y, float Z, int32 Seed);

	/**
	 * Generate thermal erosion simulation
	 * @param HeightMap Current heightmap
	 * @param Width Width of heightmap
	 * @param Height Height of heightmap
	 * @param Iterations Number of erosion iterations
	 * @param TalusAngle Maximum slope angle
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static void ApplyThermalErosion(TArray<float>& HeightMap, int32 Width, int32 Height, int32 Iterations, float TalusAngle);

	/**
	 * Generate hydraulic erosion simulation
	 * @param HeightMap Current heightmap
	 * @param Width Width of heightmap
	 * @param Height Height of heightmap
	 * @param Droplets Number of water droplets to simulate
	 * @param Inertia Water inertia factor
	 * @param Capacity Water carrying capacity
	 * @param Deposition Sediment deposition rate
	 * @param Erosion Erosion rate
	 * @param Evaporation Evaporation rate
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static void ApplyHydraulicErosion(TArray<float>& HeightMap, int32 Width, int32 Height, int32 Droplets, 
		float Inertia, float Capacity, float Deposition, float Erosion, float Evaporation);

	/**
	 * Generate biome-specific terrain features
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param BiomeType Type of biome
	 * @param Seed Random seed
	 * @param BaseHeight Base terrain height
	 * @return Modified terrain height
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static float GenerateBiomeFeatures(float X, float Y, EBiomeType BiomeType, int32 Seed, float BaseHeight);

	/**
	 * Generate volcanic terrain features
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Seed Random seed
	 * @param BaseHeight Base terrain height
	 * @return Modified terrain height with volcanic features
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static float GenerateVolcanicTerrain(float X, float Y, int32 Seed, float BaseHeight);

	/**
	 * Generate crater impacts for celestial bodies
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Seed Random seed
	 * @param CraterDensity Number of craters per unit area
	 * @param MaxCraterSize Maximum crater radius
	 * @return Terrain height with craters
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static float GenerateCraterField(float X, float Y, int32 Seed, float CraterDensity, float MaxCraterSize);

	/**
	 * Generate ice crystal formations for frozen worlds
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Z Z coordinate (height)
	 * @param Seed Random seed
	 * @param Temperature Local temperature
	 * @return Ice formation density
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static float GenerateIceFormations(float X, float Y, float Z, int32 Seed, float Temperature);

	/**
	 * Blend multiple terrain layers using weights
	 * @param Layers Array of terrain layers
	 * @param Weights Array of weights for each layer
	 * @return Blended terrain height
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static float BlendTerrainLayers(const TArray<float>& Layers, const TArray<float>& Weights);

	/**
	 * Generate spherical coordinates for planet-scale terrain
	 * @param Lat Latitude in degrees
	 * @param Lon Longitude in degrees
	 * @param Radius Planet radius
	 * @return 3D position on sphere surface
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static FVector SphericalToCartesian(float Lat, float Lon, float Radius);

	/**
	 * Convert Cartesian coordinates to spherical
	 * @param Position 3D position
	 * @param Radius Planet radius
	 * @return Latitude and longitude in degrees
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	static FVector2D CartesianToSpherical(FVector Position, float Radius);
};
