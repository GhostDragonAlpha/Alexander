// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PlanetConfiguration.h"
#include "TerrainMaterialSystem.generated.h"

/**
 * Material layer slot enumeration (8 layers as per requirements)
 */
UENUM(BlueprintType)
enum class ETerrainMaterialSlot : uint8
{
	BaseRock = 0    UMETA(DisplayName = "Base Rock"),
	DirtSoil = 1    UMETA(DisplayName = "Dirt/Soil"),
	Grass = 2       UMETA(DisplayName = "Grass"),
	Sand = 3        UMETA(DisplayName = "Sand"),
	Snow = 4        UMETA(DisplayName = "Snow"),
	CliffRock = 5   UMETA(DisplayName = "Cliff Rock"),
	WetSurface = 6  UMETA(DisplayName = "Wet Surface"),
	Custom = 7      UMETA(DisplayName = "Custom/Special")
};

/**
 * PBR texture set for a material layer
 */
USTRUCT(BlueprintType)
struct FAlexanderPBRTextureSet
{
	GENERATED_BODY()

	// Base color (albedo) texture
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* BaseColor = nullptr;

	// Normal map texture
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* Normal = nullptr;

	// Roughness texture (grayscale)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* Roughness = nullptr;

	// Metallic texture (grayscale)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* Metallic = nullptr;

	// Ambient occlusion texture (grayscale)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* AmbientOcclusion = nullptr;

	// Height/displacement texture (grayscale)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* Height = nullptr;

	// Texture tiling scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	float TilingScale = 1.0f;
};

/**
 * Material layer configuration
 */
USTRUCT(BlueprintType)
struct FTerrainMaterialLayerConfig
{
	GENERATED_BODY()

	// Layer name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
	FString LayerName;

	// Material slot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
	ETerrainMaterialSlot Slot = ETerrainMaterialSlot::BaseRock;

	// PBR texture set
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
	FAlexanderPBRTextureSet TextureSet;

	// Macro detail texture set (for distance blending)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
	FAlexanderPBRTextureSet MacroTextureSet;

	// Distance at which to blend to macro textures (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
	float MacroBlendDistance = 100.0f;

	// Slope angle range for automatic blending (degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
	FVector2D SlopeRange = FVector2D(0.0f, 90.0f);

	// Altitude range for automatic blending (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
	FVector2D AltitudeRange = FVector2D(-1000.0f, 10000.0f);

	// Enable triplanar projection for this layer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
	bool bUseTriplanarProjection = false;

	// Triplanar blend sharpness (higher = sharper transitions)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
	float TriplanarSharpness = 4.0f;
};

/**
 * Advanced material features configuration
 */
USTRUCT(BlueprintType)
struct FAdvancedMaterialFeatures
{
	GENERATED_BODY()

	// Enable parallax occlusion mapping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
	bool bEnableParallaxOcclusion = true;

	// Parallax depth scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
	float ParallaxDepthScale = 0.05f;

	// Enable tessellation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
	bool bEnableTessellation = true;

	// Tessellation multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
	float TessellationMultiplier = 1.0f;

	// Maximum tessellation distance (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
	float MaxTessellationDistance = 50.0f;

	// Enable dynamic wetness
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
	bool bEnableDynamicWetness = true;

	// Wetness intensity (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
	float WetnessIntensity = 0.0f;

	// Enable footprint/track deformation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
	bool bEnableDeformation = false;
};

/**
 * Manages terrain material system with layered PBR materials
 */
UCLASS(BlueprintType)
class ALEXANDER_API UTerrainMaterialSystem : public UObject
{
	GENERATED_BODY()

public:
	UTerrainMaterialSystem();

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize material system with master material
	 * @param InMasterMaterial Master material to use as base
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	void Initialize(UMaterialInterface* InMasterMaterial);

	/**
	 * Create material instance for terrain
	 * @return Dynamic material instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	UMaterialInstanceDynamic* CreateTerrainMaterialInstance();

	// ============================================================================
	// LAYER MANAGEMENT
	// ============================================================================

	/**
	 * Configure material layer
	 * @param Slot Material slot to configure
	 * @param LayerConfig Layer configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	void ConfigureLayer(ETerrainMaterialSlot Slot, const FTerrainMaterialLayerConfig& LayerConfig);

	/**
	 * Get layer configuration
	 * @param Slot Material slot
	 * @return Layer configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	FTerrainMaterialLayerConfig GetLayerConfig(ETerrainMaterialSlot Slot) const;

	/**
	 * Update layer textures in material instance
	 * @param MaterialInstance Material instance to update
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	void UpdateLayerTextures(UMaterialInstanceDynamic* MaterialInstance);

	// ============================================================================
	// BLENDING PARAMETERS
	// ============================================================================

	/**
	 * Set biome blend weights in material
	 * @param MaterialInstance Material instance to update
	 * @param BiomeWeights Map of biome indices to weights
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	void SetBiomeBlendWeights(UMaterialInstanceDynamic* MaterialInstance, const TMap<int32, float>& BiomeWeights);

	/**
	 * Update slope-based blending
	 * @param MaterialInstance Material instance to update
	 * @param SlopeAngle Current slope angle in degrees
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	void UpdateSlopeBlending(UMaterialInstanceDynamic* MaterialInstance, float SlopeAngle);

	/**
	 * Update altitude-based blending
	 * @param MaterialInstance Material instance to update
	 * @param Altitude Current altitude in meters
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	void UpdateAltitudeBlending(UMaterialInstanceDynamic* MaterialInstance, float Altitude);

	/**
	 * Update moisture-based blending
	 * @param MaterialInstance Material instance to update
	 * @param Moisture Moisture level (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	void UpdateMoistureBlending(UMaterialInstanceDynamic* MaterialInstance, float Moisture);

	/**
	 * Update temperature-based blending
	 * @param MaterialInstance Material instance to update
	 * @param Temperature Temperature in Celsius
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	void UpdateTemperatureBlending(UMaterialInstanceDynamic* MaterialInstance, float Temperature);

	/**
	 * Calculate complete material blend weights for a location
	 * @param MaterialInstance Material instance to update
	 * @param SlopeAngle Slope angle in degrees
	 * @param Altitude Altitude in meters
	 * @param BiomeWeights Biome weights at location
	 * @param Moisture Moisture level (0-1)
	 * @param Temperature Temperature in Celsius
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	void CalculateAndApplyBlendWeights(
		UMaterialInstanceDynamic* MaterialInstance,
		float SlopeAngle,
		float Altitude,
		const TMap<int32, float>& BiomeWeights,
		float Moisture,
		float Temperature
	);

	/**
	 * Get material layer weights for given parameters
	 * @param SlopeAngle Slope angle in degrees
	 * @param Altitude Altitude in meters
	 * @param Moisture Moisture level (0-1)
	 * @param Temperature Temperature in Celsius
	 * @return Map of material slots to blend weights
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	TMap<ETerrainMaterialSlot, float> CalculateLayerWeights(
		float SlopeAngle,
		float Altitude,
		float Moisture,
		float Temperature
	) const;

	// ============================================================================
	// ADVANCED FEATURES
	// ============================================================================

	/**
	 * Configure advanced material features
	 * @param Features Feature configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	void ConfigureAdvancedFeatures(const FAdvancedMaterialFeatures& Features);

	/**
	 * Update wetness parameter (for weather effects)
	 * @param MaterialInstance Material instance to update
	 * @param WetnessAmount Wetness amount (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	void UpdateWetness(UMaterialInstanceDynamic* MaterialInstance, float WetnessAmount);

	/**
	 * Update tessellation parameters
	 * @param MaterialInstance Material instance to update
	 * @param ViewDistance Distance from camera
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	void UpdateTessellation(UMaterialInstanceDynamic* MaterialInstance, float ViewDistance);

	/**
	 * Update parallax occlusion mapping parameters
	 * @param MaterialInstance Material instance to update
	 * @param ViewDistance Distance from camera
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	void UpdateParallaxOcclusion(UMaterialInstanceDynamic* MaterialInstance, float ViewDistance);

	/**
	 * Update macro/micro detail blending
	 * @param MaterialInstance Material instance to update
	 * @param ViewDistance Distance from camera
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	void UpdateMacroMicroBlending(UMaterialInstanceDynamic* MaterialInstance, float ViewDistance);

	/**
	 * Update all advanced features based on view distance
	 * @param MaterialInstance Material instance to update
	 * @param ViewDistance Distance from camera
	 */
	UFUNCTION(BlueprintCallable, Category = "Material System")
	void UpdateAdvancedFeatures(UMaterialInstanceDynamic* MaterialInstance, float ViewDistance);

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Master material reference
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	UMaterialInterface* MasterMaterial = nullptr;

	// Layer configurations (8 layers)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TMap<ETerrainMaterialSlot, FTerrainMaterialLayerConfig> LayerConfigs;

	// Advanced features configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FAdvancedMaterialFeatures AdvancedFeatures;

	// Global texture tiling scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float GlobalTilingScale = 1.0f;

protected:
	// ============================================================================
	// INTERNAL METHODS
	// ============================================================================

	/**
	 * Calculate layer blend weight based on slope and altitude
	 */
	float CalculateLayerWeight(const FTerrainMaterialLayerConfig& LayerConfig, float SlopeAngle, float Altitude) const;

	/**
	 * Calculate layer weight based on all environmental factors
	 */
	float CalculateLayerWeightAdvanced(
		const FTerrainMaterialLayerConfig& LayerConfig,
		float SlopeAngle,
		float Altitude,
		float Moisture,
		float Temperature
	) const;

	/**
	 * Calculate slope influence on material selection
	 */
	float CalculateSlopeInfluence(float SlopeAngle, const FVector2D& SlopeRange) const;

	/**
	 * Calculate altitude influence on material selection
	 */
	float CalculateAltitudeInfluence(float Altitude, const FVector2D& AltitudeRange) const;

	/**
	 * Calculate moisture influence on material selection
	 */
	float CalculateMoistureInfluence(float Moisture, ETerrainMaterialSlot Slot) const;

	/**
	 * Calculate temperature influence on material selection
	 */
	float CalculateTemperatureInfluence(float Temperature, ETerrainMaterialSlot Slot) const;

	/**
	 * Apply triplanar projection parameters
	 */
	void ApplyTriplanarProjection(UMaterialInstanceDynamic* MaterialInstance, ETerrainMaterialSlot Slot);

	/**
	 * Update distance-based texture blending
	 */
	void UpdateDistanceBlending(UMaterialInstanceDynamic* MaterialInstance, float ViewDistance);

	/**
	 * Normalize layer weights to sum to 1.0
	 */
	void NormalizeLayerWeights(TMap<ETerrainMaterialSlot, float>& Weights) const;
};
