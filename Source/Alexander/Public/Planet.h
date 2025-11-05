// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrbitalBody.h"
#include "PlanetConfiguration.h"
#include "BiomeBlendingSystem.h"
#include "Planet.generated.h"

// Forward declarations
class UPlanetAtmosphereComponent;
class UAtmosphericFogComponent;
class UPlanetCloudComponent;
class UPlanetWeatherComponent;
class UPlanetFarmingComponent;
class UDayNightCycleComponent;
class UBiomeManager;
class UBiomeBlendingSystem;
class UBiomeFeatureGenerator;
class UTerrainMaterialSystem;

UCLASS(Blueprintable)
class ALEXANDER_API APlanet : public AOrbitalBody
{
	GENERATED_BODY()

public:
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
	UAtmosphericFogComponent* FogComponent;

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

	// ============================================================================
	// BIOME SYSTEM
	// ============================================================================

	UPROPERTY()
	UBiomeManager* BiomeManager;

	UPROPERTY()
	UBiomeBlendingSystem* BiomeBlendingSystem;

	UPROPERTY()
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

	UPROPERTY()
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
	// LEGACY PROPERTIES (for backward compatibility)
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Properties")
	float PlanetScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Properties")
	FLinearColor PlanetColor;
};
