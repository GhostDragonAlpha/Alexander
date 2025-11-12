// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrbitalBody.h"
#include "PlanetConfiguration.h"
#include "BiomeBlendingSystem.h"
#include "ProceduralNoiseGenerator.h"
#include "Planet.generated.h"

// Forward declarations (UBiomeManager, UBiomeBlendingSystem, UBiomeFeatureGenerator,
// and UTerrainMaterialSystem are already declared in included headers)
class UPlanetAtmosphereComponent;
class USkyAtmosphereComponent;
class UPlanetCloudComponent;
class UPlanetWeatherComponent;
class UPlanetFarmingComponent;
class UDayNightCycleComponent;
class UBiomeFeatureGenerator;
class UTerrainMaterialSystem;

UCLASS(Blueprintable)
class ALEXANDER_API APlanet : public AOrbitalBody
{
	GENERATED_BODY()

public:
	// Constants for planet generation and configuration
	static constexpr float EARTH_RADIUS_KM = 6371.0f; // Earth's radius in kilometers
	static constexpr float EARTH_MASS_KG = 5.972e24f; // Earth's mass in kilograms
	static constexpr float DEFAULT_ORBIT_RADIUS = 10000.0f; // Default orbit distance in Unreal units
	static constexpr float DEFAULT_ORBIT_SPEED = 10.0f; // Default orbital speed in degrees per second
	static constexpr int32 DEFAULT_TERRAIN_SEED = 12345; // Default seed for terrain generation

	APlanet();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ============================================================================
	// COMPONENTS
	// ============================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PlanetMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPlanetAtmosphereComponent* AtmosphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkyAtmosphereComponent* FogComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPlanetCloudComponent* CloudComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPlanetWeatherComponent* WeatherComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UDayNightCycleComponent* DayNightCycleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPlanetFarmingComponent* FarmingComponent;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Planet configuration data asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	UPlanetConfiguration* PlanetConfig;

	// Apply configuration from data asset
	UFUNCTION(BlueprintCallable, Category = "Configuration")
	void ApplyConfiguration();

	// ============================================================================
	// TERRAIN GENERATION
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	FPlanetTerrainConfig TerrainConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	int32 TerrainSeed = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float PlanetRadius = 6371.0f; // km

	// Generate planet terrain (placeholder for future implementation)
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void GeneratePlanetTerrain();

	// Get terrain height at location (placeholder)
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	float GetTerrainHeightAtLocation(FVector2D Coordinates) const;

	// Configure terrain noise parameters
	void ConfigureTerrainNoise(FNoiseConfig& ContinentalNoise, FNoiseConfig& MountainNoise, FNoiseConfig& DetailNoise);

	// Log terrain configuration
	void LogTerrainConfiguration(const FNoiseConfig& ContinentalNoise, const FNoiseConfig& MountainNoise, const FNoiseConfig& DetailNoise);

	// Generate terrain height at location
	float GenerateTerrainHeightAtLocation(float Latitude, float Longitude, const FNoiseConfig& ContinentalNoise, const FNoiseConfig& MountainNoise, const FNoiseConfig& DetailNoise);

	// ============================================================================
	// BIOME SYSTEM
	// ============================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
	UBiomeManager* BiomeManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
	UBiomeBlendingSystem* BiomeBlendingSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
	UBiomeFeatureGenerator* BiomeFeatureGenerator;

	// Initialize biome systems
	UFUNCTION(BlueprintCallable, Category = "Biomes")
	void InitializeBiomeSystems();

	// Get biome at world location
	UFUNCTION(BlueprintCallable, Category = "Biomes")
	int32 GetBiomeAtLocation(FVector WorldLocation) const;

	// Get blended biome parameters at location
	UFUNCTION(BlueprintCallable, Category = "Biomes")
	FBlendedTerrainParameters GetBlendedBiomeParameters(FVector WorldLocation) const;

	// ============================================================================
	// MATERIAL SYSTEM
	// ============================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
	UTerrainMaterialSystem* MaterialSystem;

	// Master terrain material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* TerrainMasterMaterial;

	// Dynamic material instances for terrain
	UPROPERTY()
	TMap<FString, UMaterialInstanceDynamic*> TerrainMaterials;

	// Initialize material system
	UFUNCTION(BlueprintCallable, Category = "Materials")
	void InitializeMaterialSystem();

	// Update terrain materials based on location
	UFUNCTION(BlueprintCallable, Category = "Materials")
	void UpdateTerrainMaterials(FVector ViewerLocation);

	// Apply biome materials to terrain
	UFUNCTION(BlueprintCallable, Category = "Materials")
	void ApplyBiomeMaterials();

	// ============================================================================
	// DEBUG VISUALIZATION
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugInfo = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowAtmosphereBounds = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowCloudBounds = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowTerrainGrid = false;

	// Draw debug visualization
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DrawDebugVisualization();

	// ============================================================================
	// LEGACY PROPERTIES (Deprecated - use PlanetConfig data asset instead)
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Properties (Legacy)",
		meta = (DeprecatedProperty, DeprecationMessage = "Use PlanetConfig data asset instead"))
	float PlanetScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Properties (Legacy)",
		meta = (DeprecatedProperty, DeprecationMessage = "Use PlanetConfig data asset instead"))
	FLinearColor PlanetColor;
};
