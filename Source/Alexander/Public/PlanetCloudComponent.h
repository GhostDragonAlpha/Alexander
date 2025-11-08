// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "PlanetCloudComponent.generated.h"

/**
 * Enhanced component for managing volumetric cloud rendering with realistic lighting and wind simulation
 * Implements task 7.1: Create cloud component with volumetric rendering, configuration, and wind movement
 */
UCLASS(ClassGroup=(Planet), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UPlanetCloudComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPlanetCloudComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// CLOUD LAYER CONFIGURATION
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Layer")
	float CloudLayerHeight = 5.0f; // km above surface

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Layer")
	float CloudLayerThickness = 2.0f; // km

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Layer")
	float CloudLayerBottomAltitude = 3.0f; // km - bottom of cloud layer

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Layer")
	float CloudLayerTopAltitude = 8.0f; // km - top of cloud layer

	// ============================================================================
	// CLOUD APPEARANCE
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CloudCoverage = 0.5f; // 0-1, percentage of sky covered

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CloudDensity = 0.5f; // 0-1, opacity of clouds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Appearance")
	FLinearColor CloudAlbedo = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f); // Cloud base color

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CloudExtinction = 0.5f; // Light absorption through clouds

	// ============================================================================
	// WIND MOVEMENT SYSTEM
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
	FVector2D WindDirection = FVector2D(1.0f, 0.0f); // Normalized wind direction

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
	float WindSpeed = 10.0f; // m/s

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
	float WindVariation = 0.2f; // 0-1, amount of wind direction/speed variation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
	float WindVariationFrequency = 0.1f; // Hz, how often wind varies

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wind")
	FVector2D CurrentWindOffset; // Accumulated wind offset for cloud noise

	// ============================================================================
	// VOLUMETRIC CLOUD RENDERING
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Rendering")
	bool bEnableVolumetricClouds = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Rendering")
	int32 RayMarchSteps = 64; // Number of ray march samples

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Rendering")
	float ViewSampleScale = 1.0f; // Scale for view ray samples

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Rendering")
	float ShadowSampleScale = 1.0f; // Scale for shadow ray samples

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Rendering")
	float ShadowTracingDistance = 15.0f; // km - distance to trace shadows

	// ============================================================================
	// NOISE CONFIGURATION
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	int32 CloudSeed = 54321;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float NoiseScale = 1.0f; // Scale of cloud noise patterns

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float DetailNoiseScale = 5.0f; // Scale of detail noise

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float DetailStrength = 0.3f; // Influence of detail noise

	// ============================================================================
	// CLOUD SHAPE GENERATION (Task 7.2)
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Shape")
	int32 BaseShapeOctaves = 4; // Number of octaves for base cloud shape

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Shape")
	float BaseShapeFrequency = 0.5f; // Base frequency for cloud shapes

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Shape")
	float BaseShapeLacunarity = 2.0f; // Frequency multiplier per octave

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Shape")
	float BaseShapePersistence = 0.5f; // Amplitude multiplier per octave

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Shape", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WorleyWeight = 0.6f; // Weight of Worley noise in Perlin-Worley combination

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Shape")
	int32 DetailOctaves = 3; // Number of octaves for detail noise

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Shape")
	float DetailFrequency = 2.0f; // Base frequency for detail noise

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Shape")
	float DetailLacunarity = 2.0f; // Frequency multiplier for detail

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Shape")
	float DetailPersistence = 0.5f; // Amplitude multiplier for detail

	// ============================================================================
	// CLOUD DENSITY CALCULATION
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Density", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DensityThreshold = 0.4f; // Minimum density for cloud formation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Density", meta = (ClampMin = "0.0"))
	float DensityMultiplier = 1.5f; // Multiplier for cloud density

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Density")
	bool bUseHeightGradient = true; // Apply height-based density gradient

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Density", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HeightGradientBottom = 0.1f; // Density at bottom of cloud layer

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Density", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HeightGradientTop = 0.1f; // Density at top of cloud layer

	// ============================================================================
	// CLOUD COVERAGE PATTERNS
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Coverage")
	float CoverageFrequency = 0.2f; // Frequency of coverage patterns

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Coverage")
	int32 CoverageOctaves = 2; // Octaves for coverage variation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Coverage", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CoverageVariation = 0.3f; // Amount of coverage variation across sky

	// ============================================================================
	// LIGHTING (Task 7.3)
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float ScatteringIntensity = 1.0f; // Brightness of cloud scattering

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float PhaseG = 0.6f; // Henyey-Greenstein phase function parameter (-1 to 1)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float MultiScatteringContribution = 0.3f; // Amount of multi-scattering

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float MultiScatteringOcclusion = 0.5f; // Occlusion for multi-scattering

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	int32 LightRayMarchSteps = 6; // Number of steps for light ray marching

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float ShadowStepSize = 0.5f; // km - step size for shadow ray marching

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float ShadowDensityMultiplier = 1.0f; // Multiplier for shadow darkness

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float AmbientLightIntensity = 0.2f; // Ambient light contribution (0-1)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FLinearColor AmbientLightColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f); // Sky ambient color

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float TerrainShadowIntensity = 0.7f; // Intensity of cloud shadows on terrain (0-1)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float SelfShadowIntensity = 0.8f; // Intensity of cloud self-shadowing (0-1)

	// ============================================================================
	// VOLUMETRIC CLOUD COMPONENT REFERENCE
	// ============================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UVolumetricCloudComponent* VolumetricCloudComponent;

	// ============================================================================
	// PUBLIC METHODS
	// ============================================================================

	// Update cloud positions based on wind
	UFUNCTION(BlueprintCallable, Category = "Clouds")
	void UpdateCloudMovement(float DeltaTime);

	// Adjust cloud coverage for weather (smooth transition)
	UFUNCTION(BlueprintCallable, Category = "Clouds")
	void SetWeatherCoverage(float Coverage, float TransitionTime);

	// Set cloud density (smooth transition)
	UFUNCTION(BlueprintCallable, Category = "Clouds")
	void SetCloudDensity(float Density, float TransitionTime);

	// Set wind parameters
	UFUNCTION(BlueprintCallable, Category = "Wind")
	void SetWindParameters(FVector2D Direction, float Speed);

	// Initialize volumetric cloud rendering
	UFUNCTION(BlueprintCallable, Category = "Clouds")
	void InitializeVolumetricClouds();

	// Update volumetric cloud material parameters
	UFUNCTION(BlueprintCallable, Category = "Clouds")
	void UpdateCloudMaterialParameters();

	// Get current cloud coverage (interpolated)
	UFUNCTION(BlueprintPure, Category = "Clouds")
	float GetCurrentCloudCoverage() const { return CloudCoverage; }

	// Get current cloud density (interpolated)
	UFUNCTION(BlueprintPure, Category = "Clouds")
	float GetCurrentCloudDensity() const { return CloudDensity; }

	// Get wind offset for material parameters
	UFUNCTION(BlueprintPure, Category = "Wind")
	FVector2D GetWindOffset() const { return CurrentWindOffset; }

	// ============================================================================
	// CLOUD SHAPE GENERATION (Task 7.2)
	// ============================================================================

	/**
	 * Generate base cloud shape using 3D Perlin-Worley noise
	 * @param Position 3D position in cloud space
	 * @return Cloud density value [0, 1]
	 */
	UFUNCTION(BlueprintCallable, Category = "Cloud Shape")
	float GenerateBaseCloudShape(FVector Position);

	/**
	 * Generate detail noise for cloud shapes
	 * @param Position 3D position in cloud space
	 * @return Detail noise value [0, 1]
	 */
	UFUNCTION(BlueprintCallable, Category = "Cloud Shape")
	float GenerateCloudDetail(FVector Position);

	/**
	 * Calculate cloud density at position with all modifiers
	 * @param Position 3D position in cloud space
	 * @param HeightInLayer Normalized height in cloud layer [0, 1]
	 * @return Final cloud density [0, 1]
	 */
	UFUNCTION(BlueprintCallable, Category = "Cloud Shape")
	float CalculateCloudDensity(FVector Position, float HeightInLayer);

	/**
	 * Generate coverage pattern for clouds
	 * @param Position 2D position (X, Y) in world space
	 * @return Coverage value [0, 1]
	 */
	UFUNCTION(BlueprintCallable, Category = "Cloud Shape")
	float GenerateCoveragePattern(FVector2D Position);

	/**
	 * Sample cloud density at world position
	 * @param WorldPosition World space position
	 * @return Cloud density [0, 1], 0 if outside cloud layer
	 */
	UFUNCTION(BlueprintCallable, Category = "Cloud Shape")
	float SampleCloudDensityAtPosition(FVector WorldPosition);

	// ============================================================================
	// CLOUD LIGHTING AND SHADOWS (Task 7.3)
	// ============================================================================

	/**
	 * Calculate lighting for a point in the cloud using ray marching
	 * @param Position Position in cloud space
	 * @param HeightInLayer Normalized height in cloud layer [0, 1]
	 * @param SunDirection Direction to the sun (normalized)
	 * @return Light energy reaching this point [0, 1]
	 */
	UFUNCTION(BlueprintCallable, Category = "Cloud Lighting")
	float CalculateCloudLighting(FVector Position, float HeightInLayer, FVector SunDirection);

	/**
	 * Ray march through clouds toward sun to calculate shadow
	 * @param StartPosition Starting position in cloud space
	 * @param SunDirection Direction to march toward sun
	 * @param MaxDistance Maximum distance to march (km)
	 * @return Shadow factor [0, 1], 0=fully shadowed, 1=fully lit
	 */
	UFUNCTION(BlueprintCallable, Category = "Cloud Lighting")
	float RayMarchShadow(FVector StartPosition, FVector SunDirection, float MaxDistance);

	/**
	 * Calculate Henyey-Greenstein phase function for scattering
	 * @param CosAngle Cosine of angle between view and light direction
	 * @param GParam Anisotropy parameter (-1 to 1)
	 * @return Phase function value
	 */
	UFUNCTION(BlueprintCallable, Category = "Cloud Lighting")
	float HenyeyGreensteinPhase(float CosAngle, float GParam);

	/**
	 * Calculate cloud shadow intensity at terrain position
	 * @param TerrainPosition World position on terrain
	 * @param SunDirection Direction to the sun (normalized)
	 * @return Shadow intensity [0, 1], 0=no shadow, 1=full shadow
	 */
	UFUNCTION(BlueprintCallable, Category = "Cloud Lighting")
	float CalculateTerrainShadow(FVector TerrainPosition, FVector SunDirection);

	/**
	 * Get sun direction from day-night cycle component
	 * @return Normalized sun direction vector
	 */
	UFUNCTION(BlueprintCallable, Category = "Cloud Lighting")
	FVector GetSunDirection();

	/**
	 * Set reference to day-night cycle component for sun direction
	 * @param DayNightCycle The day-night cycle component
	 */
	UFUNCTION(BlueprintCallable, Category = "Cloud Lighting")
	void SetDayNightCycleComponent(class UDayNightCycleComponent* DayNightCycle);

private:
	// Transition targets
	float TargetCoverage = 0.5f;
	float TargetDensity = 0.5f;
	
	// Transition speeds
	float CoverageTransitionSpeed = 0.0f;
	float DensityTransitionSpeed = 0.0f;

	// Wind variation
	float WindVariationTime = 0.0f;
	FVector2D BaseWindDirection;

	// Material instance for cloud rendering
	UPROPERTY()
	UMaterialInstanceDynamic* CloudMaterialInstance;

	// Reference to day-night cycle component for sun direction
	UPROPERTY()
	class UDayNightCycleComponent* DayNightCycleComponent;

	// Update wind variation over time
	void UpdateWindVariation(float DeltaTime);

	// Smooth interpolation for coverage and density
	void UpdateTransitions(float DeltaTime);

	// Helper function to convert world position to cloud space
	FVector WorldToCloudSpace(FVector WorldPosition) const;

	// Helper function to check if position is in cloud layer
	bool IsInCloudLayer(FVector WorldPosition, float& OutHeightInLayer) const;
};
