// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "PlanetConfiguration.generated.h"

/**
 * Planet type enumeration
 */
UENUM(BlueprintType)
enum class EPlanetType : uint8
{
	Terrestrial UMETA(DisplayName = "Terrestrial"),
	GasGiant UMETA(DisplayName = "Gas Giant"),
	Ice UMETA(DisplayName = "Ice"),
	Desert UMETA(DisplayName = "Desert"),
	Ocean UMETA(DisplayName = "Ocean"),
	Volcanic UMETA(DisplayName = "Volcanic")
};

/**
 * Weather type enumeration
 */
UENUM(BlueprintType)
enum class EWeatherType : uint8
{
	Clear UMETA(DisplayName = "Clear"),
	PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
	Cloudy UMETA(DisplayName = "Cloudy"),
	Overcast UMETA(DisplayName = "Overcast"),
	Fog UMETA(DisplayName = "Fog"),
	Foggy UMETA(DisplayName = "Foggy"),
	LightRain UMETA(DisplayName = "Light Rain"),
	Rain UMETA(DisplayName = "Rain"),
	HeavyRain UMETA(DisplayName = "Heavy Rain"),
	Thunderstorm UMETA(DisplayName = "Thunderstorm"),
	LightSnow UMETA(DisplayName = "Light Snow"),
	Snow UMETA(DisplayName = "Snow"),
	HeavySnow UMETA(DisplayName = "Heavy Snow"),
	Blizzard UMETA(DisplayName = "Blizzard"),
	Sandstorm UMETA(DisplayName = "Sandstorm"),
	AcidRain UMETA(DisplayName = "Acid Rain"),
	ToxicFog UMETA(DisplayName = "Toxic Fog")
};

/**
 * Terrain configuration structure
 */
USTRUCT(BlueprintType)
struct FPlanetTerrainConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	int32 Seed = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Roughness = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Scale = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Amplitude = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	int32 Octaves = 6;
};

/**
 * Atmosphere configuration structure
 */
USTRUCT(BlueprintType)
struct FAtmosphereConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	FLinearColor RayleighScatteringCoefficient = FLinearColor(0.0058f, 0.0135f, 0.0331f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	float RayleighScaleHeight = 8.0f; // km

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	FLinearColor MieScatteringCoefficient = FLinearColor(0.004f, 0.004f, 0.004f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	float MieScaleHeight = 1.2f; // km

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	float MieAnisotropy = 0.8f; // -1 to 1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	float AtmosphereHeight = 100.0f; // km

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	FLinearColor GroundAlbedo = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	float FogDensity = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	float FogHeightFalloff = 0.2f;
};

/**
 * Biome type enumeration (6+ types as per requirements)
 */
UENUM(BlueprintType)
enum class EBiomeType : uint8
{
	Desert UMETA(DisplayName = "Desert"),
	Grassland UMETA(DisplayName = "Grassland"),
	Forest UMETA(DisplayName = "Forest"),
	Tundra UMETA(DisplayName = "Tundra"),
	Volcanic UMETA(DisplayName = "Volcanic"),
	Ocean UMETA(DisplayName = "Ocean"),
	Savanna UMETA(DisplayName = "Savanna"),
	Rainforest UMETA(DisplayName = "Rainforest"),
	Alpine UMETA(DisplayName = "Alpine"),
	Swamp UMETA(DisplayName = "Swamp")
};

/**
 * Terrain material layer for biome-specific textures
 */
USTRUCT(BlueprintType)
struct FTerrainMaterialLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	UTexture2D* BaseColorTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	UTexture2D* NormalTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	UTexture2D* RoughnessTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	UTexture2D* MetallicTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	float TilingScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	float BlendSharpness = 0.5f;
};

/**
 * Vegetation definition for biome-specific flora
 */
USTRUCT(BlueprintType)
struct FVegetationDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	UStaticMesh* VegetationMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	float SpawnProbability = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	float MinSlopeAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	float MaxSlopeAngle = 30.0f;
};

/**
 * Biome definition structure
 */
USTRUCT(BlueprintType)
struct FBiomeDefinition
{
	GENERATED_BODY()

	// Identity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString BiomeName = TEXT("Unnamed Biome");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	EBiomeType BiomeType = EBiomeType::Grassland;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FLinearColor BiomeColor = FLinearColor::Green; // For debug visualization

	// Terrain Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float BaseElevation = 0.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float ElevationVariance = 100.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Roughness = 0.5f; // 0-1, affects terrain detail

	// Material Layers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	TArray<FTerrainMaterialLayer> MaterialLayers;

	// Vegetation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	TArray<FVegetationDefinition> VegetationTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	float VegetationDensity = 0.5f; // 0-1

	// Environmental Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float Temperature = 20.0f; // Celsius

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float Humidity = 0.5f; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	TArray<EWeatherType> AllowedWeather;

	// Farming Properties (for farming system integration)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming")
	float BaseSoilQuality = 0.5f; // 0-1, range 0.3-1.0 per requirements

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming")
	float WaterTableDepth = 10.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming")
	FLinearColor SoilColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f); // Base soil color

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming")
	float SoilRoughness = 0.8f; // 0-1, affects visual texture appearance

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming")
	float SoilMoisture = 0.5f; // 0-1, base moisture level

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming")
	float SoilDrainage = 0.5f; // 0-1, how quickly water drains
};



USTRUCT(BlueprintType)
struct FWeatherPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	EWeatherType WeatherType = EWeatherType::Clear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float CloudCoverage = 0.0f; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float PrecipitationIntensity = 0.0f; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float WindSpeed = 0.0f; // m/s

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float Probability = 1.0f; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float TransitionDuration = 5.0f; // seconds to transition between weather types
};

/**
 * Planet configuration data asset
 */
UCLASS(BlueprintType)
class ALEXANDER_API UPlanetConfiguration : public UDataAsset
{
	GENERATED_BODY()

public:
	// Identity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString PlanetName = TEXT("Unnamed Planet");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	EPlanetType PlanetType = EPlanetType::Terrestrial;

	// Physical Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
	float Radius = 6371.0f; // km (Earth default)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
	float PlanetMass = 5.972e24f; // kg (Earth default)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
	float RotationPeriod = 24.0f; // hours

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
	float AxialTilt = 23.5f; // degrees

	// Terrain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	FPlanetTerrainConfig TerrainConfig;

	// Biomes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
	TArray<FBiomeDefinition> Biomes;

	// Atmosphere
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	bool bHasAtmosphere = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (EditCondition = "bHasAtmosphere"))
	FAtmosphereConfig AtmosphereConfig;

	// Weather
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	TArray<FWeatherPreset> WeatherPresets;
};
