// Copyright Epic Games, Inc. All Rights Reserved.

#include "Planet.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PlanetAtmosphereComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "PlanetCloudComponent.h"
#include "PlanetWeatherComponent.h"
#include "DayNightCycleComponent.h"
#include "PlanetFarmingComponent.h"
#include "BiomeManager.h"
#include "BiomeBlendingSystem.h"
#include "ProceduralNoiseGenerator.h"
#include "BiomeFeatureGenerator.h"
#include "TerrainMaterialSystem.h"
#include "ProceduralNoiseGenerator.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"

APlanet::APlanet()
{
	PrimaryActorTick.bCanEverTick = true;

	// Planets orbit the Sun by default
	OrbitMode = EOrbitMode::Orbit;
	Mass = EARTH_MASS_KG; // Mass of Earth in kg (default)
	OrbitRadius = DEFAULT_ORBIT_RADIUS; // Default orbit distance
	OrbitSpeed = DEFAULT_ORBIT_SPEED; // Default orbital speed (degrees per second)
	OrbitInclination = 0.0f; // Default to same plane as Sun

	// RootComponent already created by parent class AOrbitalBody

	// Create mesh component
	PlanetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlanetMesh"));
	PlanetMesh->SetupAttachment(RootComponent);

	// Load sphere mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		PlanetMesh->SetStaticMesh(SphereMesh.Object);
	}

	// Create enhanced components
	AtmosphereComponent = CreateDefaultSubobject<UPlanetAtmosphereComponent>(TEXT("AtmosphereComponent"));
	AtmosphereComponent->SetupAttachment(RootComponent);

	FogComponent = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("FogComponent"));
	FogComponent->SetupAttachment(RootComponent);

	CloudComponent = CreateDefaultSubobject<UPlanetCloudComponent>(TEXT("CloudComponent"));
	CloudComponent->SetupAttachment(RootComponent);

	WeatherComponent = CreateDefaultSubobject<UPlanetWeatherComponent>(TEXT("WeatherComponent"));

	DayNightCycleComponent = CreateDefaultSubobject<UDayNightCycleComponent>(TEXT("DayNightCycleComponent"));

	FarmingComponent = CreateDefaultSubobject<UPlanetFarmingComponent>(TEXT("FarmingComponent"));

	// Initialize biome systems (will be properly initialized in BeginPlay)
	BiomeManager = nullptr;
	BiomeBlendingSystem = nullptr;
	BiomeFeatureGenerator = nullptr;

	// Initialize material system
	MaterialSystem = nullptr;
	TerrainMasterMaterial = nullptr;

	// Default visual properties
	PlanetScale = 0.5f;
	PlanetColor = FLinearColor(0.5f, 0.5f, 0.8f, 1.0f); // Default blue-ish
}

void APlanet::BeginPlay()
{
	Super::BeginPlay();

	// Apply configuration if available
	if (PlanetConfig)
	{
		ApplyConfiguration();
	}
	else
	{
		// Apply legacy properties with enhanced defaults
		PlanetMesh->SetWorldScale3D(FVector(PlanetScale));

		// Apply color to material
		if (PlanetMesh->GetStaticMesh())
		{
			UMaterialInterface* BaseMaterial = PlanetMesh->GetMaterial(0);
			if (BaseMaterial)
			{
				UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
				if (DynMaterial)
				{
					DynMaterial->SetVectorParameterValue(FName("Color"), PlanetColor);
					DynMaterial->SetScalarParameterValue(FName("PlanetRadius"), PlanetRadius);
					PlanetMesh->SetMaterial(0, DynMaterial);
				}
			}
		}
	}

	// Initialize biome systems before material system
	InitializeBiomeSystems();

	// Initialize material system after biomes are ready
	InitializeMaterialSystem();

	// Initialize atmosphere component
	if (AtmosphereComponent && PlanetConfig && PlanetConfig->bHasAtmosphere)
	{
		AtmosphereComponent->InitializeAtmosphere(PlanetRadius, PlanetConfig->AtmosphereConfig);
		UE_LOG(LogTemp, Log, TEXT("Planet '%s' atmosphere initialized"), *GetName());
	}

	// Initialize fog component
	if (FogComponent && PlanetConfig && PlanetConfig->bHasAtmosphere)
	{
		// Configure sky atmosphere component based on atmosphere settings
		// NOTE: SetDensityFalloff removed in UE5.6 - density falloff is handled internally by SkyAtmosphereComponent
		// FogComponent->SetDensityFalloff(PlanetConfig->AtmosphereConfig.FogHeightFalloff);

		// SkyAtmosphereComponent settings are configured via the component interface
		// The actual fog parameters are set through the component's properties
		UE_LOG(LogTemp, Log, TEXT("Planet '%s' sky atmosphere component initialized"), *GetName());
	}

	// Initialize weather system
	if (WeatherComponent && PlanetConfig)
	{
		WeatherComponent->InitializeWeather(PlanetConfig->WeatherPresets);
		
		// Connect weather component to cloud component for weather-driven cloud changes
		if (CloudComponent)
		{
			WeatherComponent->SetCloudComponent(CloudComponent);
			UE_LOG(LogTemp, Log, TEXT("Planet '%s' weather-cloud integration established"), *GetName());
		}
		
		UE_LOG(LogTemp, Log, TEXT("Planet '%s' weather system initialized with %d presets"), 
			*GetName(), PlanetConfig->WeatherPresets.Num());
	}

	// Initialize day-night cycle
	if (DayNightCycleComponent)
	{
		// Configure day-night cycle based on planet properties
		if (PlanetConfig)
		{
			FDayNightCycleConfig CycleConfig;
			CycleConfig.DayLengthInSeconds = PlanetConfig->RotationPeriod * 3600.0f; // Convert hours to seconds
			CycleConfig.AxialTilt = PlanetConfig->AxialTilt;
			CycleConfig.StartTimeOfDay = 12.0f; // Start at noon
			CycleConfig.bAutoProgress = true;
			CycleConfig.TimeSpeed = 1.0f;
			
			DayNightCycleComponent->CycleConfig = CycleConfig;
		}
		
		// Connect day-night cycle to cloud component for sun direction
		if (CloudComponent)
		{
			CloudComponent->SetDayNightCycleComponent(DayNightCycleComponent);
			UE_LOG(LogTemp, Log, TEXT("Planet '%s' cloud-daynight integration established"), *GetName());
		}
		
		UE_LOG(LogTemp, Log, TEXT("Planet '%s' day-night cycle initialized"), *GetName());
	}

	// Initialize farming system
	if (FarmingComponent)
	{
		FarmingComponent->InitializeFarming(this);
		UE_LOG(LogTemp, Log, TEXT("Planet '%s' farming system initialized"), *GetName());
	}

	// Generate initial terrain
	GeneratePlanetTerrain();

	// Apply biome materials after all systems are initialized
	ApplyBiomeMaterials();

	UE_LOG(LogTemp, Log, TEXT("Planet '%s' fully initialized with radius %.1f km and %d biomes"), 
		*GetName(), PlanetRadius, PlanetConfig ? PlanetConfig->Biomes.Num() : 0);
}

void APlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Draw debug visualization if enabled
	if (bShowDebugInfo)
	{
		DrawDebugVisualization();
	}
}

void APlanet::ApplyConfiguration()
{
	if (!PlanetConfig)
		return;

	// Apply physical properties
	PlanetRadius = PlanetConfig->Radius;
	Mass = PlanetConfig->PlanetMass;
	TerrainConfig = PlanetConfig->TerrainConfig;
	TerrainSeed = PlanetConfig->TerrainConfig.Seed;

	// Apply atmosphere settings
	if (AtmosphereComponent && PlanetConfig->bHasAtmosphere)
	{
		AtmosphereComponent->ApplyAtmosphereSettings(PlanetConfig->AtmosphereConfig);
		AtmosphereComponent->PlanetRadius = PlanetRadius;
	}

	// Apply weather presets
	if (WeatherComponent)
	{
		WeatherComponent->WeatherPresets = PlanetConfig->WeatherPresets;
	}

	// Scale mesh based on radius (simplified - actual scale would be more complex)
	float ScaleFactor = PlanetRadius / EARTH_RADIUS_KM; // Normalize to Earth radius
	PlanetMesh->SetWorldScale3D(FVector(ScaleFactor * PlanetScale));
}

void APlanet::GeneratePlanetTerrain()
{
	UE_LOG(LogTemp, Log, TEXT("GeneratePlanetTerrain called for %s (Seed: %d, Radius: %.1f km)"),
		*GetName(), TerrainSeed, PlanetRadius);

	if (!BiomeManager || !PlanetConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot generate terrain - BiomeManager or PlanetConfig is null"));
		return;
	}

	// Configure noise parameters for terrain generation
	FNoiseConfig ContinentalNoise, MountainNoise, DetailNoise;
	ConfigureTerrainNoise(ContinentalNoise, MountainNoise, DetailNoise);

	// Log terrain generation configuration
	LogTerrainConfiguration(ContinentalNoise, MountainNoise, DetailNoise);

	// Generate sample terrain at a few test points to verify
	const int32 SampleCount = 8;
	for (int32 i = 0; i < SampleCount; ++i)
	{
		float Lat = FMath::RandRange(-90.0f, 90.0f);
		float Lon = FMath::RandRange(-180.0f, 180.0f);

		// Generate terrain height at this location
		float FinalHeight = GenerateTerrainHeightAtLocation(Lat, Lon, ContinentalNoise, MountainNoise, DetailNoise);

		UE_LOG(LogTemp, Log, TEXT("  Sample %d (Lat: %.1f, Lon: %.1f): Height=%.1fm"), i, Lat, Lon, FinalHeight);
	}

	UE_LOG(LogTemp, Log, TEXT("Terrain generation complete for %s"), *GetName());
}

float APlanet::GetTerrainHeightAtLocation(FVector2D Coordinates) const
{
	if (!BiomeManager || !PlanetConfig)
	{
		return 0.0f;
	}

	float Lat = Coordinates.X;
	float Lon = Coordinates.Y;

	// Configure noise (same as GeneratePlanetTerrain for consistency)
	// TODO: Add ContinentalFrequency, MaxElevation, MountainFrequency, DetailFrequency to FPlanetTerrainConfig
	// Using calculated values from existing TerrainConfig properties as placeholders
	float MaxElevation = TerrainConfig.Amplitude * 2.0f; // Derived from Amplitude
	float ContinentalFrequency = 1.0f / (TerrainConfig.Scale * 10.0f); // Derived from Scale
	float MountainFrequency = 1.0f / (TerrainConfig.Scale * 5.0f); // Derived from Scale
	float DetailFrequency = 1.0f / (TerrainConfig.Scale * 2.0f); // Derived from Scale

	FNoiseConfig ContinentalNoise;
	ContinentalNoise.Seed = TerrainSeed;
	ContinentalNoise.NoiseType = ENoiseType::Perlin;
	ContinentalNoise.Frequency = ContinentalFrequency;
	ContinentalNoise.Octaves = 4;
	ContinentalNoise.Lacunarity = 2.0f;
	ContinentalNoise.Persistence = 0.5f;
	ContinentalNoise.Amplitude = MaxElevation * 0.6f;

	FNoiseConfig MountainNoise;
	MountainNoise.Seed = TerrainSeed + 1000;
	MountainNoise.NoiseType = ENoiseType::RidgedMultifractal;
	MountainNoise.Frequency = MountainFrequency;
	MountainNoise.Octaves = 5;
	MountainNoise.Lacunarity = 2.2f;
	MountainNoise.Persistence = 0.6f;
	MountainNoise.Amplitude = MaxElevation;

	FNoiseConfig DetailNoise;
	DetailNoise.Seed = TerrainSeed + 2000;
	DetailNoise.NoiseType = ENoiseType::Perlin;
	DetailNoise.Frequency = DetailFrequency;
	DetailNoise.Octaves = 3;
	DetailNoise.Lacunarity = 2.0f;
	DetailNoise.Persistence = 0.4f;
	DetailNoise.Amplitude = MaxElevation * 0.1f;

	// Generate height layers
	float ContinentalHeight = UProceduralNoiseGenerator::FractalNoise2D(
		Lon, Lat, ContinentalNoise
	);

	float MountainHeight = 0.0f;
	if (ContinentalHeight > 0.0f)
	{
		MountainHeight = UProceduralNoiseGenerator::RidgedMultifractalNoise2D(
			Lon * 2.0f, Lat * 2.0f, MountainNoise
		) * ContinentalHeight;
	}

	float DetailHeight = UProceduralNoiseGenerator::FractalNoise2D(
		Lon * 4.0f, Lat * 4.0f, DetailNoise
	);

	// Get biome modifier
	FVector Position = UProceduralNoiseGenerator::SphericalToCartesian(Lat, Lon, PlanetRadius);
	int32 BiomeIndex = BiomeManager->GetDominantBiomeAtLocation(Position);
	float BiomeHeightModifier = 1.0f;

	if (BiomeIndex >= 0 && BiomeIndex < PlanetConfig->Biomes.Num())
	{
		const FBiomeDefinition& Biome = PlanetConfig->Biomes[BiomeIndex];

		switch (Biome.BiomeType)
		{
			case EBiomeType::Ocean:
				BiomeHeightModifier = -0.5f;
				break;
			case EBiomeType::Desert:
				BiomeHeightModifier = 0.3f;
				MountainHeight *= 0.5f;
				break;
			// TODO: Add Mountains and IceCaps to EBiomeType enum in PlanetConfiguration.h
			// Using Alpine as substitute for Mountains, Tundra as substitute for IceCaps
			case EBiomeType::Alpine:
				BiomeHeightModifier = 1.5f;
				MountainHeight *= 2.0f;
				break;
			case EBiomeType::Tundra:
				BiomeHeightModifier = 0.8f;
				DetailHeight *= 1.5f;
				break;
			case EBiomeType::Volcanic:
				MountainHeight *= 2.5f;
				DetailHeight *= 0.5f;
				break;
			default:
				BiomeHeightModifier = 1.0f;
				break;
		}
	}

	// Combine all layers
	return (ContinentalHeight + MountainHeight + DetailHeight) * BiomeHeightModifier;
}

void APlanet::DrawDebugVisualization()
{
	if (!GetWorld())
		return;

	FVector PlanetLocation = GetActorLocation();

	// Show atmosphere bounds
	if (bShowAtmosphereBounds && AtmosphereComponent)
	{
		float AtmosphereRadius = (PlanetRadius + AtmosphereComponent->AtmosphereSettings.AtmosphereHeight) * 100.0f; // Convert km to cm
		DrawDebugSphere(GetWorld(), PlanetLocation, AtmosphereRadius, 32, FColor::Cyan, false, -1.0f, 0, 2.0f);
	}

	// Show cloud bounds
	if (bShowCloudBounds && CloudComponent)
	{
		float CloudRadius = (PlanetRadius + CloudComponent->CloudLayerHeight) * 100.0f; // Convert km to cm
		DrawDebugSphere(GetWorld(), PlanetLocation, CloudRadius, 32, FColor::White, false, -1.0f, 0, 2.0f);
	}

	// Show terrain grid
	if (bShowTerrainGrid)
	{
		float SurfaceRadius = PlanetRadius * 100.0f; // Convert km to cm
		DrawDebugSphere(GetWorld(), PlanetLocation, SurfaceRadius, 16, FColor::Green, false, -1.0f, 0, 3.0f);
	}

	// Show debug text
	if (bShowDebugInfo)
	{
		FString DebugText = FString::Printf(TEXT("Planet: %s\nRadius: %.1f km\nWeather: %s"),
			PlanetConfig ? *PlanetConfig->PlanetName : TEXT("Unnamed"),
			PlanetRadius,
			WeatherComponent ? *UEnum::GetValueAsString(WeatherComponent->CurrentWeather) : TEXT("N/A")
		);

		DrawDebugString(GetWorld(), PlanetLocation + FVector(0, 0, 500.0f), DebugText, nullptr, FColor::Yellow, 0.0f, true);
	}
}

void APlanet::InitializeBiomeSystems()
{
	// Create biome manager
	if (!BiomeManager)
	{
		BiomeManager = NewObject<UBiomeManager>(this);
	}

	// Initialize with biomes from config or empty array
	TArray<FBiomeDefinition> Biomes;
	if (PlanetConfig)
	{
		Biomes = PlanetConfig->Biomes;
	}

	BiomeManager->Initialize(Biomes, PlanetRadius, TerrainSeed);

	// Create blending system
	if (!BiomeBlendingSystem)
	{
		BiomeBlendingSystem = NewObject<UBiomeBlendingSystem>(this);
	}
	BiomeBlendingSystem->Initialize(BiomeManager);

	// Create feature generator
	if (!BiomeFeatureGenerator)
	{
		BiomeFeatureGenerator = NewObject<UBiomeFeatureGenerator>(this);
	}
	BiomeFeatureGenerator->Initialize(BiomeManager, BiomeBlendingSystem, TerrainSeed);

	UE_LOG(LogTemp, Log, TEXT("Planet '%s' biome systems initialized with %d biomes"), 
		*GetName(), Biomes.Num());
}

int32 APlanet::GetBiomeAtLocation(FVector WorldLocation) const
{
	if (!BiomeManager)
	{
		return -1;
	}

	return BiomeManager->GetDominantBiomeAtLocation(WorldLocation);
}

FBlendedTerrainParameters APlanet::GetBlendedBiomeParameters(FVector WorldLocation) const
{
	FBlendedTerrainParameters DefaultParams;

	if (!BiomeBlendingSystem)
	{
		return DefaultParams;
	}

	return BiomeBlendingSystem->GetBlendedParameters(WorldLocation);
}

void APlanet::InitializeMaterialSystem()
{
	// Create material system
	if (!MaterialSystem)
	{
		MaterialSystem = NewObject<UTerrainMaterialSystem>(this);
	}

	// Initialize with master material if available
	if (TerrainMasterMaterial)
	{
		MaterialSystem->Initialize(TerrainMasterMaterial);
		UE_LOG(LogTemp, Log, TEXT("Planet '%s' material system initialized with master material"), *GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Planet '%s' has no master material assigned"), *GetName());
	}
}

void APlanet::UpdateTerrainMaterials(FVector ViewerLocation)
{
	if (!MaterialSystem || !PlanetMesh)
	{
		return;
	}

	// Get or create material instance for the planet mesh
	UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(PlanetMesh->GetMaterial(0));
	if (!MaterialInstance && TerrainMasterMaterial)
	{
		MaterialInstance = MaterialSystem->CreateTerrainMaterialInstance();
		if (MaterialInstance)
		{
			PlanetMesh->SetMaterial(0, MaterialInstance);
		}
	}

	if (!MaterialInstance)
	{
		return;
	}

	// Get blended biome parameters at viewer location
	FBlendedTerrainParameters BlendedParams = GetBlendedBiomeParameters(ViewerLocation);

	// Update biome blend weights
	MaterialSystem->SetBiomeBlendWeights(MaterialInstance, BlendedParams.BiomeWeights);

	// Calculate slope and altitude at viewer location
	FVector LocalPosition = ViewerLocation - GetActorLocation();
	float Distance = LocalPosition.Size();
	float Altitude = (Distance - (PlanetRadius * 100.0f)) / 100.0f; // Convert to meters

	// Calculate slope (simplified - would need actual terrain data)
	float SlopeAngle = 0.0f; // Placeholder

	// Update material blending
	MaterialSystem->UpdateSlopeBlending(MaterialInstance, SlopeAngle);
	MaterialSystem->UpdateAltitudeBlending(MaterialInstance, Altitude);

	// Update wetness based on weather
	if (WeatherComponent)
	{
		float WetnessAmount = WeatherComponent->GetWetnessAmount();
		MaterialSystem->UpdateWetness(MaterialInstance, WetnessAmount);
	}

	// Update tessellation based on distance
	float ViewDistance = FVector::Dist(ViewerLocation, GetActorLocation());
	MaterialSystem->UpdateTessellation(MaterialInstance, ViewDistance);
}

void APlanet::ApplyBiomeMaterials()
{
	if (!MaterialSystem || !BiomeManager || !PlanetMesh)
	{
		return;
	}

	// Create material instance if needed
	UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(PlanetMesh->GetMaterial(0));
	if (!MaterialInstance && TerrainMasterMaterial)
	{
		MaterialInstance = MaterialSystem->CreateTerrainMaterialInstance();
		if (MaterialInstance)
		{
			PlanetMesh->SetMaterial(0, MaterialInstance);
		}
	}

	if (!MaterialInstance)
	{
		return;
	}

	// Configure material layers based on biomes
	// TODO: Add GetAllBiomes() method to BiomeManager, or use PlanetConfig->Biomes directly
	// Using PlanetConfig->Biomes as workaround since BiomeManager doesn't expose GetAllBiomes()
	if (PlanetConfig)
	{
		const TArray<FBiomeDefinition>& Biomes = PlanetConfig->Biomes;

		for (int32 i = 0; i < FMath::Min(Biomes.Num(), 8); i++)
		{
			const FBiomeDefinition& Biome = Biomes[i];

			// Map biome to material slot (simplified mapping)
			ETerrainMaterialSlot Slot = static_cast<ETerrainMaterialSlot>(i);

			FTerrainMaterialLayerConfig LayerConfig;
			LayerConfig.LayerName = Biome.BiomeName;
			LayerConfig.Slot = Slot;

			// Configure based on biome properties
			if (Biome.MaterialLayers.Num() > 0)
			{
				const FTerrainMaterialLayer& BiomeLayer = Biome.MaterialLayers[0];
				// Set texture references from biome material layer
				// (In a real implementation, these would be properly mapped)
			}

			MaterialSystem->ConfigureLayer(Slot, LayerConfig);
		}
	}

	// Update textures in material instance
	MaterialSystem->UpdateLayerTextures(MaterialInstance);

	UE_LOG(LogTemp, Log, TEXT("Applied biome materials to planet '%s'"), *GetName());
}

void APlanet::ConfigureTerrainNoise(FNoiseConfig& ContinentalNoise, FNoiseConfig& MountainNoise, FNoiseConfig& DetailNoise)
{
	// Stub implementation - configure noise parameters for terrain generation
	ContinentalNoise.Frequency = 0.001f;
	ContinentalNoise.Amplitude = 1000.0f;
	ContinentalNoise.Octaves = 4;

	MountainNoise.Frequency = 0.01f;
	MountainNoise.Amplitude = 500.0f;
	MountainNoise.Octaves = 6;

	DetailNoise.Frequency = 0.1f;
	DetailNoise.Amplitude = 50.0f;
	DetailNoise.Octaves = 3;
}

void APlanet::LogTerrainConfiguration(const FNoiseConfig& ContinentalNoise, const FNoiseConfig& MountainNoise, const FNoiseConfig& DetailNoise)
{
	// Stub implementation - log terrain generation configuration
	UE_LOG(LogTemp, Log, TEXT("Terrain Configuration:"));
	UE_LOG(LogTemp, Log, TEXT("  Continental: Freq=%.6f, Amp=%.1f, Oct=%d"), ContinentalNoise.Frequency, ContinentalNoise.Amplitude, ContinentalNoise.Octaves);
	UE_LOG(LogTemp, Log, TEXT("  Mountain: Freq=%.6f, Amp=%.1f, Oct=%d"), MountainNoise.Frequency, MountainNoise.Amplitude, MountainNoise.Octaves);
	UE_LOG(LogTemp, Log, TEXT("  Detail: Freq=%.6f, Amp=%.1f, Oct=%d"), DetailNoise.Frequency, DetailNoise.Amplitude, DetailNoise.Octaves);
}

float APlanet::GenerateTerrainHeightAtLocation(float Latitude, float Longitude, const FNoiseConfig& ContinentalNoise, const FNoiseConfig& MountainNoise, const FNoiseConfig& DetailNoise)
{
	// Stub implementation - generate terrain height at location
	float ContinentalHeight = ContinentalNoise.Amplitude * FMath::PerlinNoise2D(FVector2D(Latitude * ContinentalNoise.Frequency, Longitude * ContinentalNoise.Frequency));
	float MountainHeight = MountainNoise.Amplitude * FMath::PerlinNoise2D(FVector2D(Latitude * MountainNoise.Frequency, Longitude * MountainNoise.Frequency));
	float DetailHeight = DetailNoise.Amplitude * FMath::PerlinNoise2D(FVector2D(Latitude * DetailNoise.Frequency, Longitude * DetailNoise.Frequency));

	return ContinentalHeight + MountainHeight + DetailHeight;
}
