// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceAssetManager.h"
#include "Planet.h"
#include "Spaceship.h"
#include "Asteroid.h"
#include "Sun.h"
#include "Moon.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
#include "ProceduralNoiseGenerator.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpaceAssetManager, Log, All);

USpaceAssetManager::USpaceAssetManager()
{
// Configuration defaults
bEnableProceduralGeneration = true;
bEnableDistanceOptimization = true;
bEnableVROptimization = true;
MaxAssetStreamingDistance = 100000.0f; // 100km
MaxProceduralVariations = 10;

// Initialize empty registries
PlanetAssets.Empty();
PlanetsByType.Empty();
SpaceshipAssets.Empty();
SpaceshipsByClass.Empty();
AsteroidMeshes.Empty();
StationMeshes.Empty();
NebulaTextures.Empty();
EngineEffects.Empty();
EngineSounds.Empty();
WeaponSounds.Empty();
AmbientSounds.Empty();
UISounds.Empty();
UIIcons.Empty();
UIMaterials.Empty();
UIElements.Empty();

AsteroidMaterial = nullptr;
StarfieldEffect = nullptr;
}

void USpaceAssetManager::BeginDestroy()
{
// Cleanup loaded assets
PlanetAssets.Empty();
SpaceshipAssets.Empty();
AsteroidMeshes.Empty();
StationMeshes.Empty();

Super::BeginDestroy();
}

// ============================================================================
// PLANET ASSETS
// ============================================================================

void USpaceAssetManager::LoadPlanetAssets()
{
UE_LOG(LogSpaceAssetManager, Log, TEXT("Loading planet assets..."));

InitializePlanetAssets();

UE_LOG(LogSpaceAssetManager, Log, TEXT("Loaded %d planet assets"), PlanetAssets.Num());
}

FPlanetAssetData USpaceAssetManager::GetPlanetAsset(const FString& PlanetName)
{
if (PlanetAssets.Contains(PlanetName))
{
return PlanetAssets[PlanetName];
}

UE_LOG(LogSpaceAssetManager, Warning, TEXT("Planet asset not found: %s"), *PlanetName);
return FPlanetAssetData();
}

TArray<FPlanetAssetData> USpaceAssetManager::GetPlanetsByType(const FString& PlanetType)
{
if (PlanetsByType.Contains(PlanetType))
{
return PlanetsByType[PlanetType];
}

return TArray<FPlanetAssetData>();
}

void USpaceAssetManager::RegisterPlanetAsset(const FPlanetAssetData& PlanetData)
{
PlanetAssets.Add(PlanetData.PlanetName, PlanetData);

// Add to type registry
if (!PlanetsByType.Contains(PlanetData.PlanetType))
{
    PlanetsByType.Add(PlanetData.PlanetType, FPlanetAssetsByType());
}
PlanetsByType[PlanetData.PlanetType].Planets.Add(PlanetData);

UE_LOG(LogSpaceAssetManager, Log, TEXT("Registered planet asset: %s (Type: %s)"), 
*PlanetData.PlanetName, *PlanetData.PlanetType);
}

void USpaceAssetManager::ApplyPlanetAsset(APlanet* Planet, const FPlanetAssetData& AssetData)
{
if (!Planet)
{
UE_LOG(LogSpaceAssetManager, Error, TEXT("Cannot apply asset to null planet"));
return;
}

// Apply mesh if available
if (AssetData.PlanetMesh)
{
UStaticMeshComponent* MeshComp = Planet->FindComponentByClass<UStaticMeshComponent>();
if (MeshComp)
{
MeshComp->SetStaticMesh(AssetData.PlanetMesh);
}
}

// Apply material if available
if (AssetData.PlanetMaterial)
{
UStaticMeshComponent* MeshComp = Planet->FindComponentByClass<UStaticMeshComponent>();
if (MeshComp)
{
MeshComp->SetMaterial(0, AssetData.PlanetMaterial);
}
}

UE_LOG(LogSpaceAssetManager, Log, TEXT("Applied planet asset to: %s"), *Planet->GetName());
}

void USpaceAssetManager::GeneratePlanetVariations(const FString& BasePlanetName, int32 VariationCount)
{
FPlanetAssetData BaseAsset = GetPlanetAsset(BasePlanetName);
if (BaseAsset.PlanetName.IsEmpty())
{
UE_LOG(LogSpaceAssetManager, Warning, TEXT("Base planet not found: %s"), *BasePlanetName);
return;
}

for (int32 i = 0; i < FMath::Min(VariationCount, MaxProceduralVariations); ++i)
{
FPlanetAssetData Variation = BaseAsset;
Variation.PlanetName = FString::Printf(TEXT("%s_Var%d"), *BasePlanetName, i + 1);
Variation.PlanetRadius *= FMath::RandRange(0.8f, 1.2f);
RegisterPlanetAsset(Variation);
}

UE_LOG(LogSpaceAssetManager, Log, TEXT("Generated %d variations of %s"), VariationCount, *BasePlanetName);
}

// ============================================================================
// SPACESHIP ASSETS
// ============================================================================

void USpaceAssetManager::LoadSpaceshipAssets()
{
UE_LOG(LogSpaceAssetManager, Log, TEXT("Loading spaceship assets..."));

InitializeSpaceshipAssets();

UE_LOG(LogSpaceAssetManager, Log, TEXT("Loaded %d spaceship assets"), SpaceshipAssets.Num());
}

FSpaceshipAssetData USpaceAssetManager::GetSpaceshipAsset(const FString& ShipName)
{
if (SpaceshipAssets.Contains(ShipName))
{
return SpaceshipAssets[ShipName];
}

UE_LOG(LogSpaceAssetManager, Warning, TEXT("Spaceship asset not found: %s"), *ShipName);
return FSpaceshipAssetData();
}

TArray<FSpaceshipAssetData> USpaceAssetManager::GetSpaceshipsByClass(const FString& ShipClass)
{
    if (SpaceshipsByClass.Contains(ShipClass))
    {
        return SpaceshipsByClass[ShipClass].Spaceships;
    }

    return TArray<FSpaceshipAssetData>();
}

void USpaceAssetManager::RegisterSpaceshipAsset(const FSpaceshipAssetData& ShipData)
{
SpaceshipAssets.Add(ShipData.ShipName, ShipData);

// Add to class registry
if (!SpaceshipsByClass.Contains(ShipData.ShipClass))
{
    SpaceshipsByClass.Add(ShipData.ShipClass, FSpaceshipAssetsByClass());
}
SpaceshipsByClass[ShipData.ShipClass].Spaceships.Add(ShipData);

UE_LOG(LogSpaceAssetManager, Log, TEXT("Registered spaceship asset: %s (Class: %s)"), 
*ShipData.ShipName, *ShipData.ShipClass);
}

void USpaceAssetManager::ApplySpaceshipAsset(ASpaceship* Spaceship, const FSpaceshipAssetData& AssetData)
{
if (!Spaceship)
{
UE_LOG(LogSpaceAssetManager, Error, TEXT("Cannot apply asset to null spaceship"));
return;
}

// Apply mesh if available
if (AssetData.ShipMesh)
{
UStaticMeshComponent* MeshComp = Spaceship->FindComponentByClass<UStaticMeshComponent>();
if (MeshComp)
{
MeshComp->SetStaticMesh(AssetData.ShipMesh);
}
}

// Apply material if available
if (AssetData.ShipMaterial)
{
UStaticMeshComponent* MeshComp = Spaceship->FindComponentByClass<UStaticMeshComponent>();
if (MeshComp)
{
MeshComp->SetMaterial(0, AssetData.ShipMaterial);
}
}

UE_LOG(LogSpaceAssetManager, Log, TEXT("Applied spaceship asset to: %s"), *Spaceship->GetName());
}

void USpaceAssetManager::CustomizeSpaceship(ASpaceship* Spaceship, const TArray<FString>& CustomComponents)
{
if (!Spaceship)
{
return;
}

UE_LOG(LogSpaceAssetManager, Log, TEXT("Customizing spaceship with %d components"), CustomComponents.Num());

// Apply custom components (placeholder for future implementation)
for (const FString& ComponentName : CustomComponents)
{
UE_LOG(LogSpaceAssetManager, Log, TEXT("  - Adding component: %s"), *ComponentName);
}
}

// ============================================================================
// ASTEROID ASSETS
// ============================================================================

void USpaceAssetManager::LoadAsteroidAssets()
{
UE_LOG(LogSpaceAssetManager, Log, TEXT("Loading asteroid assets..."));

InitializeAsteroidAssets();

UE_LOG(LogSpaceAssetManager, Log, TEXT("Loaded %d asteroid mesh variations"), AsteroidMeshes.Num());
}

UStaticMesh* USpaceAssetManager::GetRandomAsteroidMesh()
{
if (AsteroidMeshes.Num() == 0)
{
UE_LOG(LogSpaceAssetManager, Warning, TEXT("No asteroid meshes loaded"));
return nullptr;
}

int32 RandomIndex = FMath::RandRange(0, AsteroidMeshes.Num() - 1);
return AsteroidMeshes[RandomIndex];
}

UMaterialInterface* USpaceAssetManager::GetAsteroidMaterial()
{
return AsteroidMaterial;
}

void USpaceAssetManager::ApplyAsteroidAsset(AAsteroid* Asteroid, UStaticMesh* Mesh)
{
if (!Asteroid || !Mesh)
{
return;
}

UStaticMeshComponent* MeshComp = Asteroid->FindComponentByClass<UStaticMeshComponent>();
if (MeshComp)
{
MeshComp->SetStaticMesh(Mesh);
if (AsteroidMaterial)
{
MeshComp->SetMaterial(0, AsteroidMaterial);
}
}
}

// ============================================================================
// SPACE STATION ASSETS
// ============================================================================

void USpaceAssetManager::LoadSpaceStationAssets()
{
UE_LOG(LogSpaceAssetManager, Log, TEXT("Loading space station assets..."));

// Placeholder for space station asset loading
StationMeshes.Add(TEXT("BasicStation"), nullptr);
StationMeshes.Add(TEXT("TradingHub"), nullptr);
StationMeshes.Add(TEXT("MilitaryOutpost"), nullptr);

UE_LOG(LogSpaceAssetManager, Log, TEXT("Loaded %d station types"), StationMeshes.Num());
}

UStaticMesh* USpaceAssetManager::GetSpaceStationMesh(const FString& StationType)
{
if (StationMeshes.Contains(StationType))
{
return StationMeshes[StationType];
}

return nullptr;
}

TArray<UMaterialInterface*> USpaceAssetManager::GetStationMaterials()
{
return StationMaterials;
}

// ============================================================================
// ENVIRONMENTAL ASSETS
// ============================================================================

void USpaceAssetManager::LoadEnvironmentalAssets()
{
UE_LOG(LogSpaceAssetManager, Log, TEXT("Loading environmental assets..."));

InitializeEnvironmentalAssets();

UE_LOG(LogSpaceAssetManager, Log, TEXT("Loaded environmental assets"));
}

UTexture2D* USpaceAssetManager::GetNebulaTexture(const FString& NebulaType)
{
if (NebulaTextures.Contains(NebulaType))
{
return NebulaTextures[NebulaType];
}

return nullptr;
}

UParticleSystem* USpaceAssetManager::GetStarfieldEffect()
{
return StarfieldEffect;
}

UParticleSystem* USpaceAssetManager::GetEngineEffect(const FString& EngineType)
{
if (EngineEffects.Contains(EngineType))
{
return EngineEffects[EngineType];
}

return nullptr;
}

// ============================================================================
// AUDIO ASSETS
// ============================================================================

void USpaceAssetManager::LoadSpaceAudioAssets()
{
UE_LOG(LogSpaceAssetManager, Log, TEXT("Loading space audio assets..."));

InitializeAudioAssets();

UE_LOG(LogSpaceAssetManager, Log, TEXT("Loaded audio assets"));
}

USoundBase* USpaceAssetManager::GetEngineSound(const FString& EngineType)
{
if (EngineSounds.Contains(EngineType))
{
return EngineSounds[EngineType];
}

return nullptr;
}

USoundBase* USpaceAssetManager::GetWeaponSound(const FString& WeaponType)
{
if (WeaponSounds.Contains(WeaponType))
{
return WeaponSounds[WeaponType];
}

return nullptr;
}

USoundBase* USpaceAssetManager::GetAmbientSound(const FString& EnvironmentType)
{
if (AmbientSounds.Contains(EnvironmentType))
{
return AmbientSounds[EnvironmentType];
}

return nullptr;
}

USoundBase* USpaceAssetManager::GetUISound(const FString& UIAction)
{
if (UISounds.Contains(UIAction))
{
return UISounds[UIAction];
}

return nullptr;
}

// ============================================================================
// UI ASSETS
// ============================================================================

void USpaceAssetManager::LoadSpaceUIAssets()
{
UE_LOG(LogSpaceAssetManager, Log, TEXT("Loading space UI assets..."));

InitializeUIAssets();

UE_LOG(LogSpaceAssetManager, Log, TEXT("Loaded UI assets"));
}

UTexture2D* USpaceAssetManager::GetUIIcon(const FString& IconName)
{
if (UIIcons.Contains(IconName))
{
return UIIcons[IconName];
}

return nullptr;
}

UMaterialInterface* USpaceAssetManager::GetUIMaterial(const FString& UIElementType)
{
if (UIMaterials.Contains(UIElementType))
{
return UIMaterials[UIElementType];
}

return nullptr;
}

UStaticMesh* USpaceAssetManager::GetUIElement(const FString& ElementName)
{
if (UIElements.Contains(ElementName))
{
return UIElements[ElementName];
}

return nullptr;
}

// ============================================================================
// SYSTEM INTEGRATION
// ============================================================================

void USpaceAssetManager::IntegrateWithPlanetSystem()
{
UE_LOG(LogSpaceAssetManager, Log, TEXT("Integrating with planet system..."));
// Placeholder for planet system integration
}

void USpaceAssetManager::IntegrateWithShipSystem()
{
UE_LOG(LogSpaceAssetManager, Log, TEXT("Integrating with ship system..."));
// Placeholder for ship system integration
}

void USpaceAssetManager::IntegrateWithAsteroidSystem()
{
UE_LOG(LogSpaceAssetManager, Log, TEXT("Integrating with asteroid system..."));
// Placeholder for asteroid system integration
}

void USpaceAssetManager::IntegrateWithAudioSystem()
{
UE_LOG(LogSpaceAssetManager, Log, TEXT("Integrating with audio system..."));
// Placeholder for audio system integration
}

// ============================================================================
// PROCEDURAL GENERATION
// ============================================================================

void USpaceAssetManager::GenerateProceduralPlanet(const FString& PlanetName, const FPlanetAssetData& BaseData)
{
if (!bEnableProceduralGeneration)
{
return;
}

FPlanetAssetData ProceduralPlanet = BaseData;
ProceduralPlanet.PlanetName = PlanetName;
ProceduralPlanet.PlanetRadius *= FMath::RandRange(0.5f, 2.0f);

// Generate procedural textures for the planet
int32 PlanetSeed = GetTypeHash(PlanetName);

// Create diffuse texture
UTexture2D* DiffuseTexture = UTexture2D::CreateTransient(512, 512, PF_B8G8R8A8);
if (DiffuseTexture)
{
	CreateProceduralTexture(DiffuseTexture, 512, 512, PlanetSeed);
	ProceduralPlanet.DiffuseTexture = DiffuseTexture;
}

// Create normal texture
UTexture2D* NormalTexture = UTexture2D::CreateTransient(512, 512, PF_B8G8R8A8);
if (NormalTexture)
{
	CreateProceduralTexture(NormalTexture, 512, 512, PlanetSeed + 1000);
	ProceduralPlanet.NormalTexture = NormalTexture;
}

// Create specular/roughness texture
UTexture2D* SpecularTexture = UTexture2D::CreateTransient(512, 512, PF_B8G8R8A8);
if (SpecularTexture)
{
	CreateProceduralTexture(SpecularTexture, 512, 512, PlanetSeed + 2000);
	ProceduralPlanet.SpecularTexture = SpecularTexture;
}

RegisterPlanetAsset(ProceduralPlanet);
UE_LOG(LogSpaceAssetManager, Log, TEXT("Generated procedural planet: %s with textures"), *PlanetName);
}

void USpaceAssetManager::GenerateProceduralAsteroid(int32 Seed)
{
	UE_LOG(LogSpaceAssetManager, Log, TEXT("Generating procedural asteroid with seed: %d"), Seed);

	// Use ProceduralMeshComponent approach
	// Note: This requires ProceduralMeshComponent plugin
	// For now, we'll create a static mesh placeholder and add it to the asteroid meshes

	FRandomStream Random(Seed);

	// Generate asteroid parameters
	float AsteroidSize = Random.FRandRange(50.0f, 500.0f);
	int32 Complexity = Random.RandRange(3, 8);

	// Create procedural mesh (simplified - actual implementation would use ProceduralMeshComponent)
	// Generate vertices using noise displacement
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FColor> VertexColors;

	// Generate icosphere base
	int32 SubdivisionLevel = Complexity;
	float Radius = AsteroidSize;

	// Start with icosahedron
	const float GoldenRatio = (1.0f + FMath::Sqrt(5.0f)) / 2.0f;
	const float Scale = Radius / FMath::Sqrt(1.0f + GoldenRatio * GoldenRatio);

	// Base icosahedron vertices (12 vertices)
	TArray<FVector> BaseVertices = {
		FVector(-1, GoldenRatio, 0) * Scale,
		FVector(1, GoldenRatio, 0) * Scale,
		FVector(-1, -GoldenRatio, 0) * Scale,
		FVector(1, -GoldenRatio, 0) * Scale,
		FVector(0, -1, GoldenRatio) * Scale,
		FVector(0, 1, GoldenRatio) * Scale,
		FVector(0, -1, -GoldenRatio) * Scale,
		FVector(0, 1, -GoldenRatio) * Scale,
		FVector(GoldenRatio, 0, -1) * Scale,
		FVector(GoldenRatio, 0, 1) * Scale,
		FVector(-GoldenRatio, 0, -1) * Scale,
		FVector(-GoldenRatio, 0, 1) * Scale
	};

	// Apply noise-based displacement to vertices for irregular asteroid shape
	for (int32 i = 0; i < BaseVertices.Num(); ++i)
	{
		FVector Vertex = BaseVertices[i];
		FVector Normalized = Vertex.GetSafeNormal();

		// Use multiple octaves of noise for realistic asteroid surface
		float NoiseValue = 0.0f;
		float Amplitude = 1.0f;
		float Frequency = 0.01f;
		int32 Octaves = 4;

		for (int32 Octave = 0; Octave < Octaves; ++Octave)
		{
			NoiseValue += UProceduralNoiseGenerator::PerlinNoise3D(
				Normalized.X * Frequency,
				Normalized.Y * Frequency,
				Normalized.Z * Frequency,
				Seed + Octave
			) * Amplitude;

			Amplitude *= 0.5f;
			Frequency *= 2.0f;
		}

		// Displace vertex along normal
		float Displacement = NoiseValue * Radius * 0.3f;
		Vertices.Add(Vertex + Normalized * Displacement);
	}

	// Generate triangles for icosahedron
	TArray<int32> BaseTriangles = {
		0, 11, 5,  0, 5, 1,   0, 1, 7,   0, 7, 10,  0, 10, 11,
		1, 5, 9,   5, 11, 4,  11, 10, 2, 10, 7, 6,  7, 1, 8,
		3, 9, 4,   3, 4, 2,   3, 2, 6,   3, 6, 8,   3, 8, 9,
		4, 9, 5,   2, 4, 11,  6, 2, 10,  8, 6, 7,   9, 8, 1
	};

	Triangles = BaseTriangles;

	// Generate normals
	for (int32 i = 0; i < Vertices.Num(); ++i)
	{
		Normals.Add(Vertices[i].GetSafeNormal());
	}

	// Generate UVs (simple spherical mapping)
	for (int32 i = 0; i < Vertices.Num(); ++i)
	{
		FVector Normalized = Vertices[i].GetSafeNormal();
		float U = 0.5f + FMath::Atan2(Normalized.Y, Normalized.X) / (2.0f * PI);
		float V = 0.5f - FMath::Asin(Normalized.Z) / PI;
		UVs.Add(FVector2D(U, V));
	}

	// Generate vertex colors based on composition
	FLinearColor AsteroidColor = FLinearColor(
		Random.FRandRange(0.3f, 0.5f),
		Random.FRandRange(0.3f, 0.5f),
		Random.FRandRange(0.3f, 0.5f)
	);

	for (int32 i = 0; i < Vertices.Num(); ++i)
	{
		VertexColors.Add(AsteroidColor.ToFColor(true));
	}

	// Create material instance for asteroid
	if (AsteroidMaterial)
	{
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(AsteroidMaterial, this);
		if (DynamicMaterial)
		{
			// Set procedural material parameters
			DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), AsteroidColor);
			DynamicMaterial->SetScalarParameterValue(FName("Roughness"), Random.FRandRange(0.7f, 0.95f));
			DynamicMaterial->SetScalarParameterValue(FName("Metallic"), Random.FRandRange(0.1f, 0.3f));
		}
	}

	UE_LOG(LogSpaceAssetManager, Log, TEXT("Generated procedural asteroid with %d vertices, size %.1f"),
		Vertices.Num(), AsteroidSize);
}

void USpaceAssetManager::GenerateProceduralNebula(const FString& NebulaName, int32 Seed)
{
	UE_LOG(LogSpaceAssetManager, Log, TEXT("Generating procedural nebula: %s (seed: %d)"), *NebulaName, Seed);

	FRandomStream Random(Seed);

	// Generate nebula parameters
	FLinearColor NebulaColor1 = FLinearColor(
		Random.FRandRange(0.5f, 1.0f),
		Random.FRandRange(0.2f, 0.8f),
		Random.FRandRange(0.3f, 0.9f)
	);

	FLinearColor NebulaColor2 = FLinearColor(
		Random.FRandRange(0.3f, 0.7f),
		Random.FRandRange(0.4f, 0.9f),
		Random.FRandRange(0.5f, 1.0f)
	);

	// Create 3D volumetric texture for nebula
	const int32 TextureSize = 128;
	UTexture2D* NebulaTexture = UTexture2D::CreateTransient(TextureSize, TextureSize, PF_B8G8R8A8);

	if (NebulaTexture)
	{
		// Lock texture for editing
		FTexture2DMipMap& Mip = NebulaTexture->GetPlatformData()->Mips[0];
		void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
		FColor* ColorData = static_cast<FColor*>(Data);

		// Generate nebula using 3D Perlin-Worley noise for realistic cloud-like structure
		for (int32 Y = 0; Y < TextureSize; ++Y)
		{
			for (int32 X = 0; X < TextureSize; ++X)
			{
				// Normalize coordinates
				float NormX = X / static_cast<float>(TextureSize);
				float NormY = Y / static_cast<float>(TextureSize);

				// Use multiple layers of noise for depth
				float BaseNoise = UProceduralNoiseGenerator::FractalPerlinWorleyNoise3D(
					NormX * 4.0f,
					NormY * 4.0f,
					0.5f,
					Seed,
					4,         // Octaves
					1.0f,      // Frequency
					2.0f,      // Lacunarity
					0.5f,      // Persistence
					0.6f       // Worley weight for cellular structure
				);

				// Add detail noise
				float DetailNoise = UProceduralNoiseGenerator::FractalNoise2D(
					NormX * 8.0f,
					NormY * 8.0f,
					FNoiseConfig()
				);

				// Combine noise layers
				float CombinedNoise = BaseNoise * 0.7f + DetailNoise * 0.3f;

				// Apply falloff from center for spherical nebula appearance
				FVector2D Center(0.5f, 0.5f);
				float DistFromCenter = FVector2D::Distance(FVector2D(NormX, NormY), Center);
				float Falloff = FMath::Clamp(1.0f - (DistFromCenter / 0.5f), 0.0f, 1.0f);
				Falloff = FMath::Pow(Falloff, 2.0f);

				// Apply falloff to noise
				CombinedNoise *= Falloff;

				// Create color gradient based on noise value
				FLinearColor FinalColor = FLinearColor::LerpUsingHSV(NebulaColor1, NebulaColor2, CombinedNoise);

				// Set alpha based on density
				float Alpha = FMath::Clamp(CombinedNoise * 1.2f, 0.0f, 1.0f);
				FinalColor.A = Alpha;

				// Write to texture
				int32 Index = Y * TextureSize + X;
				ColorData[Index] = FinalColor.ToFColor(true);
			}
		}

		// Unlock texture
		Mip.BulkData.Unlock();
		NebulaTexture->UpdateResource();

		// Store nebula texture
		NebulaTextures.Add(NebulaName, NebulaTexture);

		UE_LOG(LogSpaceAssetManager, Log, TEXT("Generated procedural nebula texture: %s (%dx%d)"),
			*NebulaName, TextureSize, TextureSize);
	}

	// Note: For actual volumetric nebula rendering in-game, you would use:
	// - Niagara particle systems with sprite renderers
	// - Volume textures (3D textures) for true volumetric fog
	// - Multiple layers of translucent billboards
	// - Custom volumetric materials with depth-based blending
}

// ============================================================================
// OPTIMIZATION
// ============================================================================

void USpaceAssetManager::OptimizeForSpaceRendering()
{
UE_LOG(LogSpaceAssetManager, Log, TEXT("Optimizing assets for space rendering..."));
// Placeholder for space-specific optimizations
}

void USpaceAssetManager::OptimizeForLargeDistances()
{
if (!bEnableDistanceOptimization)
{
return;
}

UE_LOG(LogSpaceAssetManager, Log, TEXT("Optimizing assets for large distances..."));
// Placeholder for distance-based optimizations
}

void USpaceAssetManager::OptimizeForVR()
{
if (!bEnableVROptimization)
{
return;
}

UE_LOG(LogSpaceAssetManager, Log, TEXT("Optimizing assets for VR..."));
// Placeholder for VR-specific optimizations
}

// ============================================================================
// STREAMING
// ============================================================================

void USpaceAssetManager::StreamPlanetAssets(const FVector& PlayerLocation, float StreamRadius)
{
	if (!bEnableDistanceOptimization)
	{
		return;
	}

	// Use StreamRadius if provided, otherwise use default
	float ActiveStreamRadius = StreamRadius > 0.0f ? StreamRadius : MaxAssetStreamingDistance;

	// Track loaded and unloaded assets
	TArray<FString> AssetsToLoad;
	TArray<FString> AssetsToUnload;

	// Iterate through all planet assets
	for (const auto& PlanetPair : PlanetAssets)
	{
		const FPlanetAssetData& PlanetData = PlanetPair.Value;

		// Calculate distance from player to planet
		// Note: In actual implementation, you would get planet world location from level
		// For now, we use a placeholder distance calculation
		float Distance = 0.0f; // Placeholder

		// Determine if asset should be loaded based on distance
		if (Distance <= ActiveStreamRadius)
		{
			// Asset should be loaded
			if (!PlanetData.PlanetMesh || !PlanetData.PlanetMesh->IsValidLowLevel())
			{
				AssetsToLoad.Add(PlanetPair.Key);
			}
		}
		else
		{
			// Asset should be unloaded to save memory
			if (PlanetData.PlanetMesh && PlanetData.PlanetMesh->IsValidLowLevel())
			{
				AssetsToUnload.Add(PlanetPair.Key);
			}
		}
	}

	// Load assets that are in range
	for (const FString& AssetName : AssetsToLoad)
	{
		UE_LOG(LogSpaceAssetManager, Log, TEXT("Streaming in planet asset: %s"), *AssetName);
		// Async load asset here
		// This would use FStreamableManager in a real implementation
	}

	// Unload assets that are out of range
	for (const FString& AssetName : AssetsToUnload)
	{
		UE_LOG(LogSpaceAssetManager, Log, TEXT("Streaming out planet asset: %s"), *AssetName);
		// Unload asset to free memory
		if (PlanetAssets.Contains(AssetName))
		{
			FPlanetAssetData& PlanetData = PlanetAssets[AssetName];
			// Clear texture references to allow garbage collection
			PlanetData.DiffuseTexture = nullptr;
			PlanetData.NormalTexture = nullptr;
			PlanetData.SpecularTexture = nullptr;
			PlanetData.PlanetMesh = nullptr;
		}
	}

	UE_LOG(LogSpaceAssetManager, Log, TEXT("Planet asset streaming: %d loaded, %d unloaded"),
		AssetsToLoad.Num(), AssetsToUnload.Num());
}

void USpaceAssetManager::StreamShipAssets(const TArray<ASpaceship*>& VisibleShips)
{
	if (!bEnableDistanceOptimization)
	{
		return;
	}

	// Track which ship assets are currently needed
	TSet<FString> NeededShipAssets;

	// Collect all ship classes that are visible
	for (const ASpaceship* Ship : VisibleShips)
	{
		if (Ship)
		{
			// Get ship class/name
			// This would be based on actual ship properties
			FString ShipClass = TEXT("Fighter"); // Placeholder
			NeededShipAssets.Add(ShipClass);
		}
	}

	// Stream in assets for visible ships
	for (const FString& ShipClass : NeededShipAssets)
	{
		if (SpaceshipAssets.Contains(ShipClass))
		{
			const FSpaceshipAssetData& ShipData = SpaceshipAssets[ShipClass];

			// Ensure ship mesh is loaded
			if (!ShipData.ShipMesh || !ShipData.ShipMesh->IsValidLowLevel())
			{
				UE_LOG(LogSpaceAssetManager, Log, TEXT("Streaming in ship asset: %s"), *ShipClass);
				// Async load ship assets
			}
		}
	}

	// Unload assets for ships that are no longer visible
	// This would be based on usage tracking in a real implementation
	TArray<FString> UnusedAssets;
	for (const auto& ShipPair : SpaceshipAssets)
	{
		if (!NeededShipAssets.Contains(ShipPair.Key))
		{
			UnusedAssets.Add(ShipPair.Key);
		}
	}

	// Only unload if we have too many loaded
	const int32 MaxLoadedShipTypes = 20;
	if (SpaceshipAssets.Num() > MaxLoadedShipTypes && UnusedAssets.Num() > 0)
	{
		// Unload oldest unused assets
		int32 ToUnload = SpaceshipAssets.Num() - MaxLoadedShipTypes;
		for (int32 i = 0; i < FMath::Min(ToUnload, UnusedAssets.Num()); ++i)
		{
			UE_LOG(LogSpaceAssetManager, Log, TEXT("Streaming out ship asset: %s"), *UnusedAssets[i]);
			// Clear references to allow garbage collection
			if (SpaceshipAssets.Contains(UnusedAssets[i]))
			{
				FSpaceshipAssetData& ShipData = SpaceshipAssets[UnusedAssets[i]];
				ShipData.ShipMesh = nullptr;
				ShipData.ShipMaterial = nullptr;
				ShipData.ShipTexture = nullptr;
				ShipData.ComponentMeshes.Empty();
			}
		}
	}

	UE_LOG(LogSpaceAssetManager, Log, TEXT("Ship asset streaming: %d visible ships, %d unique types"),
		VisibleShips.Num(), NeededShipAssets.Num());
}

void USpaceAssetManager::StreamEnvironmentalAssets(const FVector& ViewLocation, const FRotator& ViewRotation)
{
	if (!bEnableDistanceOptimization)
	{
		return;
	}

	// Stream environmental assets based on view frustum
	// This includes nebulas, starfields, and effects

	// Calculate view direction
	FVector ViewDirection = ViewRotation.Vector();

	// Determine which environmental effects are in view
	TArray<FString> AssetsToStream;

	// Check nebulas
	for (const auto& NebulaPair : NebulaTextures)
	{
		// In a real implementation, you would check if nebula is in view frustum
		// For now, we'll use a simplified distance check

		bool bInView = true; // Placeholder

		if (bInView && !NebulaPair.Value)
		{
			AssetsToStream.Add(NebulaPair.Key);
		}
	}

	// Stream in needed environmental assets
	for (const FString& AssetName : AssetsToStream)
	{
		UE_LOG(LogSpaceAssetManager, Log, TEXT("Streaming in environmental asset: %s"), *AssetName);
		// Async load environmental asset
	}

	// Environmental assets are typically small, so we keep them loaded
	// unless memory budget is exceeded

	UE_LOG(LogSpaceAssetManager, Log, TEXT("Environmental asset streaming: %d assets to stream"),
		AssetsToStream.Num());
}

// ============================================================================
// INITIALIZATION HELPERS
// ============================================================================

void USpaceAssetManager::InitializePlanetAssets()
{
// Register default planet types
FPlanetAssetData EarthLike;
EarthLike.PlanetName = TEXT("EarthLike");
EarthLike.PlanetType = TEXT("Terrestrial");
EarthLike.PlanetRadius = 6371.0f;
RegisterPlanetAsset(EarthLike);

FPlanetAssetData GasGiant;
GasGiant.PlanetName = TEXT("GasGiant");
GasGiant.PlanetType = TEXT("Gas");
GasGiant.PlanetRadius = 69911.0f;
RegisterPlanetAsset(GasGiant);

FPlanetAssetData IceWorld;
IceWorld.PlanetName = TEXT("IceWorld");
IceWorld.PlanetType = TEXT("Ice");
IceWorld.PlanetRadius = 3000.0f;
RegisterPlanetAsset(IceWorld);
}

void USpaceAssetManager::InitializeSpaceshipAssets()
{
// Register default ship classes
FSpaceshipAssetData Fighter;
Fighter.ShipName = TEXT("Fighter");
Fighter.ShipClass = TEXT("Fighter");
Fighter.ShipLength = 15.0f;
Fighter.ShipMass = 5000.0f;
RegisterSpaceshipAsset(Fighter);

FSpaceshipAssetData Freighter;
Freighter.ShipName = TEXT("Freighter");
Freighter.ShipClass = TEXT("Transport");
Freighter.ShipLength = 50.0f;
Freighter.ShipMass = 50000.0f;
RegisterSpaceshipAsset(Freighter);
}

void USpaceAssetManager::InitializeAsteroidAssets()
{
// Placeholder for asteroid asset initialization
}

void USpaceAssetManager::InitializeEnvironmentalAssets()
{
// Placeholder for environmental asset initialization
}

void USpaceAssetManager::InitializeAudioAssets()
{
// Placeholder for audio asset initialization
}

void USpaceAssetManager::InitializeUIAssets()
{
// Placeholder for UI asset initialization
}

void USpaceAssetManager::CreateProceduralTexture(UTexture2D* Texture, int32 Width, int32 Height, int32 Seed)
{
	if (!Texture)
	{
		return;
	}

	// Ensure texture has platform data
	if (!Texture->GetPlatformData())
	{
		UE_LOG(LogSpaceAssetManager, Warning, TEXT("Texture has no platform data, cannot generate procedural texture"));
		return;
	}

	// Lock the texture for editing
	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FColor* ColorData = static_cast<FColor*>(Data);

	FRandomStream Random(Seed);

	// Generate base color variation
	FLinearColor BaseColor = FLinearColor(
		Random.FRandRange(0.4f, 0.8f),
		Random.FRandRange(0.4f, 0.8f),
		Random.FRandRange(0.4f, 0.8f)
	);

	// Configure noise for texture generation
	FNoiseConfig NoiseConfig;
	NoiseConfig.Seed = Seed;
	NoiseConfig.Frequency = 0.01f;
	NoiseConfig.Octaves = 5;
	NoiseConfig.Lacunarity = 2.0f;
	NoiseConfig.Persistence = 0.5f;
	NoiseConfig.NoiseType = ENoiseType::Perlin;

	// Generate texture data
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			// Normalize coordinates
			float NormX = X / static_cast<float>(Width);
			float NormY = Y / static_cast<float>(Height);

			// Generate multi-octave noise
			float NoiseValue = UProceduralNoiseGenerator::FractalNoise2D(
				NormX * Width,
				NormY * Height,
				NoiseConfig
			);

			// Map noise from [-1, 1] to [0, 1]
			NoiseValue = (NoiseValue + 1.0f) * 0.5f;

			// Add detail noise layer
			float DetailNoise = UProceduralNoiseGenerator::PerlinNoise2D(
				NormX * Width * 4.0f,
				NormY * Height * 4.0f,
				Seed + 100
			);
			DetailNoise = (DetailNoise + 1.0f) * 0.5f;

			// Combine noise layers
			float FinalNoise = NoiseValue * 0.7f + DetailNoise * 0.3f;

			// Apply to base color
			FLinearColor FinalColor = BaseColor * FinalNoise;
			FinalColor.A = 1.0f;

			// Write to texture
			int32 Index = Y * Width + X;
			ColorData[Index] = FinalColor.ToFColor(true);
		}
	}

	// Unlock and update texture
	Mip.BulkData.Unlock();
	Texture->UpdateResource();

	UE_LOG(LogSpaceAssetManager, Log, TEXT("Generated procedural texture %dx%d with seed %d"),
		Width, Height, Seed);
}

void USpaceAssetManager::CreateProceduralMesh(UStaticMesh* Mesh, int32 Complexity, int32 Seed)
{
	if (!Mesh)
	{
		return;
	}

	// Note: Creating a static mesh at runtime requires significantly more setup
	// This is a simplified placeholder implementation
	// In a full implementation, you would need to:
	// 1. Create FStaticMeshSourceModel
	// 2. Create FMeshDescription
	// 3. Populate vertex, triangle, and UV data
	// 4. Build the mesh using FStaticMeshBuilder
	// 5. Create collision geometry
	// 6. Update render data

	FRandomStream Random(Seed);

	// Generate mesh parameters
	int32 SegmentCount = FMath::Clamp(Complexity * 4, 8, 64);
	float Radius = Random.FRandRange(100.0f, 500.0f);

	// Log mesh generation
	UE_LOG(LogSpaceAssetManager, Log, TEXT("Procedural mesh generation initiated: Complexity=%d, Seed=%d, Segments=%d, Radius=%.1f"),
		Complexity, Seed, SegmentCount, Radius);

	// Note: Actual mesh creation would happen here using StaticMesh APIs
	// This requires engine-level access to mesh building systems
	// For runtime mesh generation, ProceduralMeshComponent is recommended instead

	UE_LOG(LogSpaceAssetManager, Warning, TEXT("Full static mesh generation not implemented - use ProceduralMeshComponent for runtime mesh generation"));
}
