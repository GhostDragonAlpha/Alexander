// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/AssetManager.h"
#include "SpaceAssetManager.generated.h"

// Forward declarations
class APlanet;
class ASpaceship;
class AAsteroid;
class ASun;
class AMoon;
class UMaterialInterface;
class UTexture2D;
class UStaticMesh;

/**
 * Space asset categories
 */
UENUM(BlueprintType)
enum class ESpaceAssetCategory : uint8
{
	Planetary UMETA(DisplayName = "Planetary"),
	Ship UMETA(DisplayName = "Ship"),
	Station UMETA(DisplayName = "Station"),
	Asteroid UMETA(DisplayName = "Asteroid"),
	Nebula UMETA(DisplayName = "Nebula"),
	Starfield UMETA(DisplayName = "Starfield"),
	Effect UMETA(DisplayName = "Effect"),
	UI UMETA(DisplayName = "UI")
};

/**
 * Planet asset data
 */
USTRUCT(BlueprintType)
struct FPlanetAssetData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	UTexture2D* DiffuseTexture;

	UPROPERTY(BlueprintReadOnly)
	UTexture2D* NormalTexture;

	UPROPERTY(BlueprintReadOnly)
	UTexture2D* SpecularTexture;

	UPROPERTY(BlueprintReadOnly)
	UMaterialInterface* PlanetMaterial;

	UPROPERTY(BlueprintReadOnly)
	UStaticMesh* PlanetMesh;

	UPROPERTY(BlueprintReadOnly)
	FString PlanetName;

	UPROPERTY(BlueprintReadOnly)
	FString PlanetType;

	UPROPERTY(BlueprintReadOnly)
	float PlanetRadius;

	UPROPERTY(BlueprintReadOnly)
	FAssetMetadata Metadata;

	FPlanetAssetData()
	{
		DiffuseTexture = nullptr;
		NormalTexture = nullptr;
		SpecularTexture = nullptr;
		PlanetMaterial = nullptr;
		PlanetMesh = nullptr;
		PlanetName = TEXT("");
		PlanetType = TEXT("Terrestrial");
		PlanetRadius = 6371.0f; // Earth radius in km
	}
};

/**
 * Spaceship asset data
 */
USTRUCT(BlueprintType)
struct FSpaceshipAssetData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	UStaticMesh* ShipMesh;

	UPROPERTY(BlueprintReadOnly)
	UMaterialInterface* ShipMaterial;

	UPROPERTY(BlueprintReadOnly)
	UTexture2D* ShipTexture;

	UPROPERTY(BlueprintReadOnly)
	TArray<UStaticMesh*> ComponentMeshes;

	UPROPERTY(BlueprintReadOnly)
	FString ShipName;

	UPROPERTY(BlueprintReadOnly)
	FString ShipClass;

	UPROPERTY(BlueprintReadOnly)
	float ShipLength;

	UPROPERTY(BlueprintReadOnly)
	float ShipMass;

	UPROPERTY(BlueprintReadOnly)
	FAssetMetadata Metadata;

	FSpaceshipAssetData()
	{
		ShipMesh = nullptr;
		ShipMaterial = nullptr;
		ShipTexture = nullptr;
		ComponentMeshes.Empty();
		ShipName = TEXT("");
		ShipClass = TEXT("Fighter");
		ShipLength = 50.0f;
		ShipMass = 1000.0f;
	}
};

/**
 * Wrapper for planet assets by type
 */
USTRUCT()
struct FPlanetAssetsByType
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FPlanetAssetData> Planets;
};

/**
 * Wrapper for spaceship assets by class
 */
USTRUCT()
struct FSpaceshipAssetsByClass
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FSpaceshipAssetData> Spaceships;
};

/**
 * Space Asset Manager - Specialized asset manager for space simulation
 * Integrates free assets with planetary and ship systems
 */
UCLASS(BlueprintType, Blueprintable)
class ALEXANDER_API USpaceAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	USpaceAssetManager();

protected:
	virtual void BeginDestroy() override;

public:
	// Planet assets
	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void LoadPlanetAssets();

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	FPlanetAssetData GetPlanetAsset(const FString& PlanetName);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	TArray<FPlanetAssetData> GetPlanetsByType(const FString& PlanetType);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void RegisterPlanetAsset(const FPlanetAssetData& PlanetData);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void ApplyPlanetAsset(APlanet* Planet, const FPlanetAssetData& AssetData);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void GeneratePlanetVariations(const FString& BasePlanetName, int32 VariationCount);

	// Spaceship assets
	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void LoadSpaceshipAssets();

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	FSpaceshipAssetData GetSpaceshipAsset(const FString& ShipName);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	TArray<FSpaceshipAssetData> GetSpaceshipsByClass(const FString& ShipClass);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void RegisterSpaceshipAsset(const FSpaceshipAssetData& ShipData);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void ApplySpaceshipAsset(ASpaceship* Spaceship, const FSpaceshipAssetData& AssetData);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void CustomizeSpaceship(ASpaceship* Spaceship, const TArray<FString>& CustomComponents);

	// Asteroid assets
	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void LoadAsteroidAssets();

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	UStaticMesh* GetRandomAsteroidMesh();

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	UMaterialInterface* GetAsteroidMaterial();

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void ApplyAsteroidAsset(AAsteroid* Asteroid, UStaticMesh* Mesh);

	// Space station assets
	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void LoadSpaceStationAssets();

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	UStaticMesh* GetSpaceStationMesh(const FString& StationType);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	TArray<UMaterialInterface*> GetStationMaterials();

	// Environmental assets
	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void LoadEnvironmentalAssets();

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	UTexture2D* GetNebulaTexture(const FString& NebulaType);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	UParticleSystem* GetStarfieldEffect();

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	UParticleSystem* GetEngineEffect(const FString& EngineType);

	// Audio assets
	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void LoadSpaceAudioAssets();

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	USoundBase* GetEngineSound(const FString& EngineType);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	USoundBase* GetWeaponSound(const FString& WeaponType);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	USoundBase* GetAmbientSound(const FString& EnvironmentType);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	USoundBase* GetUISound(const FString& UIAction);

	// UI assets
	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void LoadSpaceUIAssets();

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	UTexture2D* GetUIIcon(const FString& IconName);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	UMaterialInterface* GetUIMaterial(const FString& UIElementType);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	UStaticMesh* GetUIElement(const FString& ElementName);

	// Asset integration with existing systems
	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void IntegrateWithPlanetSystem();

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void IntegrateWithShipSystem();

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void IntegrateWithAsteroidSystem();

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void IntegrateWithAudioSystem();

	// Procedural asset generation
	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void GenerateProceduralPlanet(const FString& PlanetName, const FPlanetAssetData& BaseData);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void GenerateProceduralAsteroid(int32 Seed);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void GenerateProceduralNebula(const FString& NebulaName, int32 Seed);

	// Asset optimization for space
	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void OptimizeForSpaceRendering();

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void OptimizeForLargeDistances();

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void OptimizeForVR();

	// Asset streaming for space
	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void StreamPlanetAssets(const FVector& PlayerLocation, float StreamRadius);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void StreamShipAssets(const TArray<ASpaceship*>& VisibleShips);

	UFUNCTION(BlueprintCallable, Category = "Space Asset Manager")
	void StreamEnvironmentalAssets(const FVector& ViewLocation, const FRotator& ViewRotation);

protected:
	// Planet asset registry
	UPROPERTY()
	TMap<FString, FPlanetAssetData> PlanetAssets;

	UPROPERTY()
	TMap<FString, FPlanetAssetsByType> PlanetsByType;

	// Spaceship asset registry
	UPROPERTY()
	TMap<FString, FSpaceshipAssetData> SpaceshipAssets;

	UPROPERTY()
	TMap<FString, FSpaceshipAssetsByClass> SpaceshipsByClass;

	// Asteroid assets
	UPROPERTY()
	TArray<UStaticMesh*> AsteroidMeshes;

	UPROPERTY()
	UMaterialInterface* AsteroidMaterial;

	// Space station assets
	UPROPERTY()
	TMap<FString, UStaticMesh*> StationMeshes;

	UPROPERTY()
	TArray<UMaterialInterface*> StationMaterials;

	// Environmental assets
	UPROPERTY()
	TMap<FString, UTexture2D*> NebulaTextures;

	UPROPERTY()
	UParticleSystem* StarfieldEffect;

	UPROPERTY()
	TMap<FString, UParticleSystem*> EngineEffects;

	// Audio assets
	UPROPERTY()
	TMap<FString, USoundBase*> EngineSounds;

	UPROPERTY()
	TMap<FString, USoundBase*> WeaponSounds;

	UPROPERTY()
	TMap<FString, USoundBase*> AmbientSounds;

	UPROPERTY()
	TMap<FString, USoundBase*> UISounds;

	// UI assets
	UPROPERTY()
	TMap<FString, UTexture2D*> UIIcons;

	UPROPERTY()
	TMap<FString, UMaterialInterface*> UIMaterials;

	UPROPERTY()
	TMap<FString, UStaticMesh*> UIElements;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space Asset Manager")
	bool bEnableProceduralGeneration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space Asset Manager")
	bool bEnableDistanceOptimization;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space Asset Manager")
	bool bEnableVROptimization;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space Asset Manager")
	float MaxAssetStreamingDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space Asset Manager")
	int32 MaxProceduralVariations;

private:
	void InitializePlanetAssets();
	void InitializeSpaceshipAssets();
	void InitializeAsteroidAssets();
	void InitializeEnvironmentalAssets();
	void InitializeAudioAssets();
	void InitializeUIAssets();
	void CreateProceduralTexture(UTexture2D* Texture, int32 Width, int32 Height, int32 Seed);
	void CreateProceduralMesh(UStaticMesh* Mesh, int32 Complexity, int32 Seed);
};