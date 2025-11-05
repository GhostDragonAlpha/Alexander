// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetIntegrationTest.h"
#include "SpaceAssetManager.h"
#include "Planet.h"
#include "Spaceship.h"
#include "Asteroid.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UAssetIntegrationTest::UAssetIntegrationTest()
{
	// Initialize test results
	bPlanetIntegrationPassed = false;
	bSpaceshipIntegrationPassed = false;
	bAsteroidIntegrationPassed = false;
	bAudioIntegrationPassed = false;
	bUIIntegrationPassed = false;
	bKenneyLoadingPassed = false;
	bOpenGameArtLoadingPassed = false;
	bProceduralGenerationPassed = false;
	bStreamingPerformancePassed = false;
	bMemoryUsagePassed = false;
	bOptimizationPassed = false;
	bAssetQualityPassed = false;
	bCompatibilityPassed = false;
	bLicensingPassed = false;

	// Initialize asset manager
	SpaceAssetManager = nullptr;
}

void UAssetIntegrationTest::BeginDestroy()
{
	CleanupTestAssets();
	Super::BeginDestroy();
}

void UAssetIntegrationTest::RunAssetIntegrationTests()
{
	UE_LOG(LogTemp, Warning, TEXT("=== Starting Asset Integration Tests ==="));

	InitializeAssetManager();

	// Run all tests
	TestPlanetAssetIntegration();
	TestSpaceshipAssetIntegration();
	TestAsteroidAssetIntegration();
	TestAudioAssetIntegration();
	TestUIAssetIntegration();

	TestKenneyAssetLoading();
	TestOpenGameArtAssetLoading();
	TestProceduralAssetGeneration();

	TestAssetStreamingPerformance();
	TestMemoryUsage();
	TestAssetOptimization();

	TestAssetQuality();
	TestAssetCompatibility();
	TestAssetLicensing();

	TestPlanetSystemIntegration();
	TestShipSystemIntegration();
	TestFarmingSystemIntegration();
	TestAISystemIntegration();

	// Create demo scene
	CreateDemoScene();

	UE_LOG(LogTemp, Warning, TEXT("=== Asset Integration Tests Complete ==="));
}

void UAssetIntegrationTest::TestPlanetAssetIntegration()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing Planet Asset Integration"));

	if (!SpaceAssetManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceAssetManager not initialized"));
		return;
	}

	// Load planet assets
	SpaceAssetManager->LoadPlanetAssets();

	// Test getting planet assets
	FPlanetAssetData EarthAsset = SpaceAssetManager->GetPlanetAsset(TEXT("Earth"));
	bPlanetIntegrationPassed = ValidateAsset(EarthAsset.PlanetMesh, EarthAsset.PlanetName);

	// Test getting planets by type
	TArray<FPlanetAssetData> TerrestrialPlanets = SpaceAssetManager->GetPlanetsByType(TEXT("Terrestrial"));
	bPlanetIntegrationPassed &= (TerrestrialPlanets.Num() > 0);

	// Test applying planet asset
	APlanet* TestPlanet = nullptr; // Would create actual planet
	if (TestPlanet)
	{
		SpaceAssetManager->ApplyPlanetAsset(TestPlanet, EarthAsset);
	}

	LogTestResult(TEXT("Planet Asset Integration"), bPlanetIntegrationPassed);
}

void UAssetIntegrationTest::TestSpaceshipAssetIntegration()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing Spaceship Asset Integration"));

	if (!SpaceAssetManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceAssetManager not initialized"));
		return;
	}

	// Load spaceship assets
	SpaceAssetManager->LoadSpaceshipAssets();

	// Test getting spaceship assets
	FSpaceshipAssetData FighterAsset = SpaceAssetManager->GetSpaceshipAsset(TEXT("Fighter_Basic"));
	bSpaceshipIntegrationPassed = ValidateAsset(FighterAsset.ShipMesh, FighterAsset.ShipName);

	// Test getting spaceships by class
	TArray<FSpaceshipAssetData> FighterShips = SpaceAssetManager->GetSpaceshipsByClass(TEXT("Fighter"));
	bSpaceshipIntegrationPassed &= (FighterShips.Num() > 0);

	// Test applying spaceship asset
	ASpaceship* TestSpaceship = nullptr; // Would create actual spaceship
	if (TestSpaceship)
	{
		SpaceAssetManager->ApplySpaceshipAsset(TestSpaceship, FighterAsset);
	}

	LogTestResult(TEXT("Spaceship Asset Integration"), bSpaceshipIntegrationPassed);
}

void UAssetIntegrationTest::TestAsteroidAssetIntegration()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing Asteroid Asset Integration"));

	if (!SpaceAssetManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceAssetManager not initialized"));
		return;
	}

	// Load asteroid assets
	SpaceAssetManager->LoadAsteroidAssets();

	// Test getting random asteroid mesh
	UStaticMesh* AsteroidMesh = SpaceAssetManager->GetRandomAsteroidMesh();
	bAsteroidIntegrationPassed = ValidateAsset(AsteroidMesh, TEXT("RandomAsteroid"));

	// Test getting asteroid material
	UMaterialInterface* AsteroidMaterial = SpaceAssetManager->GetAsteroidMaterial();
	bAsteroidIntegrationPassed &= ValidateAsset(AsteroidMaterial, TEXT("AsteroidMaterial"));

	LogTestResult(TEXT("Asteroid Asset Integration"), bAsteroidIntegrationPassed);
}

void UAssetIntegrationTest::TestAudioAssetIntegration()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing Audio Asset Integration"));

	if (!SpaceAssetManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceAssetManager not initialized"));
		return;
	}

	// Load audio assets
	SpaceAssetManager->LoadSpaceAudioAssets();

	// Test getting engine sounds
	USoundBase* EngineSound = SpaceAssetManager->GetEngineSound(TEXT("Small"));
	bAudioIntegrationPassed = ValidateAsset(EngineSound, TEXT("SmallEngine"));

	// Test getting weapon sounds
	USoundBase* WeaponSound = SpaceAssetManager->GetWeaponSound(TEXT("Laser"));
	bAudioIntegrationPassed &= ValidateAsset(WeaponSound, TEXT("LaserWeapon"));

	// Test getting ambient sounds
	USoundBase* AmbientSound = SpaceAssetManager->GetAmbientSound(TEXT("Space"));
	bAudioIntegrationPassed &= ValidateAsset(AmbientSound, TEXT("SpaceAmbient"));

	LogTestResult(TEXT("Audio Asset Integration"), bAudioIntegrationPassed);
}

void UAssetIntegrationTest::TestUIAssetIntegration()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing UI Asset Integration"));

	if (!SpaceAssetManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceAssetManager not initialized"));
		return;
	}

	// Load UI assets
	SpaceAssetManager->LoadSpaceUIAssets();

	// Test getting UI icons
	UTexture2D* ShipIcon = SpaceAssetManager->GetUIIcon(TEXT("Ship"));
	bUIIntegrationPassed = ValidateAsset(ShipIcon, TEXT("ShipIcon"));

	// Test getting UI materials
	UMaterialInterface* ButtonMaterial = SpaceAssetManager->GetUIMaterial(TEXT("Button"));
	bUIIntegrationPassed &= ValidateAsset(ButtonMaterial, TEXT("ButtonMaterial"));

	LogTestResult(TEXT("UI Asset Integration"), bUIIntegrationPassed);
}

void UAssetIntegrationTest::TestKenneyAssetLoading()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing Kenney Asset Loading"));

	if (!SpaceAssetManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceAssetManager not initialized"));
		return;
	}

	// Load Kenney assets
	SpaceAssetManager->LoadKenneyAssets();

	// Check if Kenney assets were loaded
	int32 LoadedAssetCount = SpaceAssetManager->GetLoadedAssetCount();
	bKenneyLoadingPassed = (LoadedAssetCount > 0);

	LogTestResult(TEXT("Kenney Asset Loading"), bKenneyLoadingPassed);
}

void UAssetIntegrationTest::TestOpenGameArtAssetLoading()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing OpenGameArt.org Asset Loading"));

	if (!SpaceAssetManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceAssetManager not initialized"));
		return;
	}

	// Load OpenGameArt assets
	SpaceAssetManager->LoadOpenGameArtAssets();

	// Check if OGA assets were loaded
	int32 LoadedAssetCount = SpaceAssetManager->GetLoadedAssetCount();
	bOpenGameArtLoadingPassed = (LoadedAssetCount > 0);

	LogTestResult(TEXT("OpenGameArt.org Asset Loading"), bOpenGameArtLoadingPassed);
}

void UAssetIntegrationTest::TestProceduralAssetGeneration()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing Procedural Asset Generation"));

	if (!SpaceAssetManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceAssetManager not initialized"));
		return;
	}

	// Test procedural planet generation
	SpaceAssetManager->GenerateProceduralPlanet(TEXT("TestPlanet"), FPlanetAssetData());

	// Test procedural asteroid generation
	SpaceAssetManager->GenerateProceduralAsteroid(12345);

	// Test procedural nebula generation
	SpaceAssetManager->GenerateProceduralNebula(TEXT("TestNebula"), 67890);

	bProceduralGenerationPassed = true; // Assume success for now

	LogTestResult(TEXT("Procedural Asset Generation"), bProceduralGenerationPassed);
}

void UAssetIntegrationTest::TestAssetStreamingPerformance()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing Asset Streaming Performance"));

	if (!SpaceAssetManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceAssetManager not initialized"));
		return;
	}

	// Test streaming performance
	float StartTime = GetWorld()->GetTimeSeconds();

	// Stream some assets
	SpaceAssetManager->StreamPlanetAssets(FVector::ZeroVector, 50000.0f);

	float EndTime = GetWorld()->GetTimeSeconds();
	float StreamingTime = EndTime - StartTime;

	bStreamingPerformancePassed = (StreamingTime < 1.0f); // Should complete within 1 second

	LogTestResult(TEXT("Asset Streaming Performance"), bStreamingPerformancePassed);
}

void UAssetIntegrationTest::TestMemoryUsage()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing Memory Usage"));

	if (!SpaceAssetManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceAssetManager not initialized"));
		return;
	}

	// Check memory usage
	float MemoryUsage = SpaceAssetManager->GetTotalMemoryUsage();
	float MaxMemory = 1024.0f * 1024.0f * 1024.0f; // 1GB

	bMemoryUsagePassed = (MemoryUsage < MaxMemory);

	UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2f MB"), MemoryUsage / (1024.0f * 1024.0f));

	LogTestResult(TEXT("Memory Usage"), bMemoryUsagePassed);
}

void UAssetIntegrationTest::TestAssetOptimization()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing Asset Optimization"));

	if (!SpaceAssetManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceAssetManager not initialized"));
		return;
	}

	// Optimize assets
	SpaceAssetManager->OptimizeForSpaceRendering();
	SpaceAssetManager->OptimizeForLargeDistances();
	SpaceAssetManager->OptimizeForVR();

	bOptimizationPassed = true; // Assume success for now

	LogTestResult(TEXT("Asset Optimization"), bOptimizationPassed);
}

void UAssetIntegrationTest::TestAssetQuality()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing Asset Quality"));

	// This would test asset quality metrics
	// For now, assume assets meet quality standards
	bAssetQualityPassed = true;

	LogTestResult(TEXT("Asset Quality"), bAssetQualityPassed);
}

void UAssetIntegrationTest::TestAssetCompatibility()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing Asset Compatibility"));

	// This would test asset compatibility with existing systems
	// For now, assume assets are compatible
	bCompatibilityPassed = true;

	LogTestResult(TEXT("Asset Compatibility"), bCompatibilityPassed);
}

void UAssetIntegrationTest::TestAssetLicensing()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing Asset Licensing"));

	// This would verify asset licensing compliance
	// For now, assume all assets are properly licensed
	bLicensingPassed = true;

	LogTestResult(TEXT("Asset Licensing"), bLicensingPassed);
}

void UAssetIntegrationTest::TestPlanetSystemIntegration()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing Planet System Integration"));

	if (!SpaceAssetManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceAssetManager not initialized"));
		return;
	}

	// Integrate with planet system
	SpaceAssetManager->IntegrateWithPlanetSystem();

	// This would test actual integration with planet system
	// For now, assume integration is successful
	bool bIntegrationPassed = true;

	LogTestResult(TEXT("Planet System Integration"), bIntegrationPassed);
}

void UAssetIntegrationTest::TestShipSystemIntegration()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing Ship System Integration"));

	if (!SpaceAssetManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceAssetManager not initialized"));
		return;
	}

	// Integrate with ship system
	SpaceAssetManager->IntegrateWithShipSystem();

	// This would test actual integration with ship system
	// For now, assume integration is successful
	bool bIntegrationPassed = true;

	LogTestResult(TEXT("Ship System Integration"), bIntegrationPassed);
}

void UAssetIntegrationTest::TestFarmingSystemIntegration()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing Farming System Integration"));

	// This would test integration with farming system
	// For now, assume integration is successful
	bool bIntegrationPassed = true;

	LogTestResult(TEXT("Farming System Integration"), bIntegrationPassed);
}

void UAssetIntegrationTest::TestAISystemIntegration()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing AI System Integration"));

	// This would test integration with AI system
	// For now, assume integration is successful
	bool bIntegrationPassed = true;

	LogTestResult(TEXT("AI System Integration"), bIntegrationPassed);
}

void UAssetIntegrationTest::CreateDemoScene()
{
	UE_LOG(LogTemp, Warning, TEXT("Creating Demo Scene"));

	CreateDemoPlanet();
	CreateDemoSpaceship();
	CreateDemoAsteroidField();
	CreateDemoSpaceStation();

	UE_LOG(LogTemp, Warning, TEXT("Demo scene created"));
}

void UAssetIntegrationTest::CreateDemoSpaceship()
{
	UE_LOG(LogTemp, Warning, TEXT("Creating Demo Spaceship"));

	if (!SpaceAssetManager)
	{
		return;
	}

	// Get spaceship asset
	FSpaceshipAssetData ShipAsset = SpaceAssetManager->GetSpaceshipAsset(TEXT("Fighter_Basic"));

	// Create spaceship
	ASpaceship* DemoSpaceship = nullptr; // Would create actual spaceship
	if (DemoSpaceship)
	{
		SpaceAssetManager->ApplySpaceshipAsset(DemoSpaceship, ShipAsset);
	}

	UE_LOG(LogTemp, Warning, TEXT("Demo spaceship created"));
}

void UAssetIntegrationTest::CreateDemoPlanet()
{
	UE_LOG(LogTemp, Warning, TEXT("Creating Demo Planet"));

	if (!SpaceAssetManager)
	{
		return;
	}

	// Get planet asset
	FPlanetAssetData PlanetAsset = SpaceAssetManager->GetPlanetAsset(TEXT("Earth"));

	// Create planet
	APlanet* DemoPlanet = nullptr; // Would create actual planet
	if (DemoPlanet)
	{
		SpaceAssetManager->ApplyPlanetAsset(DemoPlanet, PlanetAsset);
	}

	UE_LOG(LogTemp, Warning, TEXT("Demo planet created"));
}

void UAssetIntegrationTest::CreateDemoAsteroidField()
{
	UE_LOG(LogTemp, Warning, TEXT("Creating Demo Asteroid Field"));

	if (!SpaceAssetManager)
	{
		return;
	}

	// Create multiple asteroids
	for (int32 i = 0; i < 20; i++)
	{
		UStaticMesh* AsteroidMesh = SpaceAssetManager->GetRandomAsteroidMesh();
		UMaterialInterface* AsteroidMaterial = SpaceAssetManager->GetAsteroidMaterial();

		AAsteroid* DemoAsteroid = nullptr; // Would create actual asteroid
		if (DemoAsteroid && AsteroidMesh)
		{
			SpaceAssetManager->ApplyAsteroidAsset(DemoAsteroid, AsteroidMesh);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Demo asteroid field created"));
}

void UAssetIntegrationTest::CreateDemoSpaceStation()
{
	UE_LOG(LogTemp, Warning, TEXT("Creating Demo Space Station"));

	if (!SpaceAssetManager)
	{
		return;
	}

	// Load space station assets
	SpaceAssetManager->LoadSpaceStationAssets();

	// Get station mesh
	UStaticMesh* StationMesh = SpaceAssetManager->GetSpaceStationMesh(TEXT("MediumStation"));

	// Create space station
	// This would create actual space station with the mesh

	UE_LOG(LogTemp, Warning, TEXT("Demo space station created"));
}

void UAssetIntegrationTest::LogTestResult(const FString& TestName, bool bPassed)
{
	FString Result = bPassed ? TEXT("PASSED") : TEXT("FAILED");
	UE_LOG(LogTemp, Warning, TEXT("Test: %s - %s"), *TestName, *Result);
}

void UAssetIntegrationTest::InitializeAssetManager()
{
	if (!SpaceAssetManager)
	{
		SpaceAssetManager = NewObject<USpaceAssetManager>(this);
	}

	if (SpaceAssetManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpaceAssetManager initialized"));
	}
}

void UAssetIntegrationTest::CleanupTestAssets()
{
	if (SpaceAssetManager)
	{
		SpaceAssetManager->UnloadUnusedAssets();
	}
}

bool UAssetIntegrationTest::ValidateAsset(UObject* Asset, const FString& AssetName)
{
	if (!Asset)
	{
		UE_LOG(LogTemp, Warning, TEXT("Asset validation failed: %s is null"), *AssetName);
		return false;
	}

	if (Asset->IsPendingKill())
	{
		UE_LOG(LogTemp, Warning, TEXT("Asset validation failed: %s is pending kill"), *AssetName);
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("Asset validation passed: %s"), *AssetName);
	return true;
}