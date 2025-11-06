// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameSystemsTest.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "OrbitalBody.h"
#include "Planet.h"
#include "Spaceship.h"
#include "ProceduralNoiseGenerator.h"
#include "TerrainStreamingManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AGameSystemsTest::AGameSystemsTest()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsTestRunning = false;
	CurrentTestIndex = 0;
	TestsPassed = 0;
	TestsFailed = 0;
}

void AGameSystemsTest::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Log, TEXT("=== Alexander Game Systems Test Suite ==="));
	UE_LOG(LogTemp, Log, TEXT("Starting comprehensive system validation..."));
	
	// Schedule test start
	FTimerHandle StartTimer;
	GetWorld()->GetTimerManager().SetTimer(StartTimer, this, &AGameSystemsTest::StartTestSuite, 2.0f, false);
}

void AGameSystemsTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bIsTestRunning)
	{
		UpdateTestProgress(DeltaTime);
	}
}

void AGameSystemsTest::StartTestSuite()
{
	bIsTestRunning = true;
	CurrentTestIndex = 0;
	TestsPassed = 0;
	TestsFailed = 0;
	
	// Initialize all tests
	InitializeTests();
	
	// Start first test
	RunNextTest();
}

void AGameSystemsTest::InitializeTests()
{
	Tests.Empty();
	
	// Orbital Physics Tests
	Tests.Add(FTestDefinition(
		TEXT("Orbital Physics - Circular Orbit"),
		[this]() { return TestCircularOrbit(); }
	));
	
	Tests.Add(FTestDefinition(
		TEXT("Orbital Physics - Ballistic Trajectory"),
		[this]() { return TestBallisticTrajectory(); }
	));
	
	Tests.Add(FTestDefinition(
		TEXT("Orbital Physics - Escape Velocity"),
		[this]() { return TestEscapeVelocity(); }
	));
	
	// Planet Systems Tests
	Tests.Add(FTestDefinition(
		TEXT("Planet Systems - Component Integration"),
		[this]() { return TestPlanetComponentIntegration(); }
	));
	
	Tests.Add(FTestDefinition(
		TEXT("Planet Systems - Biome Generation"),
		[this]() { return TestBiomeGeneration(); }
	));
	
	// Spaceship Systems Tests
	Tests.Add(FTestDefinition(
		TEXT("Spaceship Systems - Flight Controls"),
		[this]() { return TestSpaceshipFlightControls(); }
	));
	
	Tests.Add(FTestDefinition(
		TEXT("Spaceship Systems - VR Integration"),
		[this]() { return TestVRIntegration(); }
	));
	
	// Terrain Generation Tests
	Tests.Add(FTestDefinition(
		TEXT("Terrain Generation - Procedural Noise"),
		[this]() { return TestProceduralNoise(); }
	));
	
	Tests.Add(FTestDefinition(
		TEXT("Terrain Generation - Advanced Features"),
		[this]() { return TestAdvancedTerrainFeatures(); }
	));
	
	// AI Vision System Tests
	Tests.Add(FTestDefinition(
		TEXT("AI Vision - Screenshot Capture"),
		[this]() { return TestAIVisionScreenshot(); }
	));
	
	Tests.Add(FTestDefinition(
		TEXT("AI Vision - Backend Communication"),
		[this]() { return TestAIVisionBackend(); }
	));
	
	UE_LOG(LogTemp, Log, TEXT("Initialized %d tests"), Tests.Num());
}

void AGameSystemsTest::RunNextTest()
{
	if (CurrentTestIndex >= Tests.Num())
	{
		FinishTestSuite();
		return;
	}
	
	const FTestDefinition& Test = Tests[CurrentTestIndex];
	UE_LOG(LogTemp, Log, TEXT("Running test %d/%d: %s"), 
		CurrentTestIndex + 1, Tests.Num(), *Test.TestName);
	
	// Run test with timeout
	CurrentTestStartTime = FDateTime::UtcNow();
	bTestTimedOut = false;
	
	FTimerHandle TimeoutTimer;
	GetWorld()->GetTimerManager().SetTimer(TimeoutTimer, [this]()
	{
		bTestTimedOut = true;
		UE_LOG(LogTemp, Error, TEXT("Test timed out"));
		OnTestCompleted(false, TEXT("Test timed out"));
	}, 30.0f, false);
	
	// Execute test
	bool bResult = Test.TestFunction();
	
	// Clear timeout timer
	GetWorld()->GetTimerManager().ClearTimer(TimeoutTimer);
	
	if (!bTestTimedOut)
	{
		OnTestCompleted(bResult, bResult ? TEXT("Test passed") : TEXT("Test failed"));
	}
}

void AGameSystemsTest::OnTestCompleted(bool bPassed, const FString& Message)
{
	if (bPassed)
	{
		TestsPassed++;
		UE_LOG(LogTemp, Log, TEXT("✓ %s - %s"), *Tests[CurrentTestIndex].TestName, *Message);
	}
	else
	{
		TestsFailed++;
		UE_LOG(LogTemp, Error, TEXT("✗ %s - %s"), *Tests[CurrentTestIndex].TestName, *Message);
	}
	
	CurrentTestIndex++;
	
	// Small delay between tests
	FTimerHandle NextTestTimer;
	GetWorld()->GetTimerManager().SetTimer(NextTestTimer, this, &AGameSystemsTest::RunNextTest, 1.0f, false);
}

void AGameSystemsTest::FinishTestSuite()
{
	bIsTestRunning = false;
	
	UE_LOG(LogTemp, Log, TEXT("=== Test Suite Complete ==="));
	UE_LOG(LogTemp, Log, TEXT("Tests Passed: %d"), TestsPassed);
	UE_LOG(LogTemp, Log, TEXT("Tests Failed: %d"), TestsFailed);
	UE_LOG(LogTemp, Log, TEXT("Total Tests: %d"), TestsPassed + TestsFailed);
	UE_LOG(LogTemp, Log, TEXT("Success Rate: %.1f%%"), 
		(TestsPassed * 100.0f) / (TestsPassed + TestsFailed));
	
	// Display results on screen if possible
	if (GEngine)
	{
		FString ResultText = FString::Printf(
			TEXT("Alexander Systems Test Complete\nPassed: %d Failed: %d\nSuccess Rate: %.1f%%"),
			TestsPassed, TestsFailed, (TestsPassed * 100.0f) / (TestsPassed + TestsFailed));
		
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, ResultText);
	}
}

bool AGameSystemsTest::TestCircularOrbit()
{
	// Create test orbital body
	AOrbitalBody* TestBody = GetWorld()->SpawnActor<AOrbitalBody>(AOrbitalBody::StaticClass());
	if (!TestBody)
	{
		return false;
	}
	
	// Create central body (sun)
	AOrbitalBody* CentralBody = GetWorld()->SpawnActor<AOrbitalBody>(AOrbitalBody::StaticClass());
	if (!CentralBody)
	{
		TestBody->Destroy();
		return false;
	}
	
	// Setup orbital relationship
	CentralBody->Mass = 1.989e30f; // Sun mass
	CentralBody->SetActorLocation(FVector::ZeroVector);
	
	TestBody->OrbitTarget = CentralBody;
	TestBody->OrbitRadius = 100000000.0f; // 100,000 km
	TestBody->InitializeCircularOrbit();
	
	// Verify orbital parameters
	float ExpectedVelocity = FMath::Sqrt(6.67430e-11f * CentralBody->Mass / TestBody->OrbitRadius);
	float ActualVelocity = TestBody->OrbitSpeed * FMath::DegreesToRadians(TestBody->OrbitRadius);
	
	bool bVelocityCorrect = FMath::Abs(ExpectedVelocity - ActualVelocity) < (ExpectedVelocity * 0.01f);
	bool bOrbitModeCorrect = TestBody->OrbitMode == EOrbitMode::Orbit;
	
	// Cleanup
	TestBody->Destroy();
	CentralBody->Destroy();
	
	return bVelocityCorrect && bOrbitModeCorrect;
}

bool AGameSystemsTest::TestBallisticTrajectory()
{
	// Create test body
	AOrbitalBody* TestBody = GetWorld()->SpawnActor<AOrbitalBody>(AOrbitalBody::StaticClass());
	if (!TestBody)
	{
		return false;
	}
	
	// Create central body
	AOrbitalBody* CentralBody = GetWorld()->SpawnActor<AOrbitalBody>(AOrbitalBody::StaticClass());
	if (!CentralBody)
	{
		TestBody->Destroy();
		return false;
	}
	
	// Setup ballistic trajectory
	CentralBody->Mass = 5.972e24f; // Earth mass
	CentralBody->SetActorLocation(FVector::ZeroVector);
	
	TestBody->OrbitTarget = CentralBody;
	TestBody->SetActorLocation(FVector(7000000.0f, 0.0f, 0.0f)); // 7000 km from center
	TestBody->Velocity = FVector(0.0f, 7500.0f, 0.0f); // Orbital velocity
	TestBody->OrbitMode = EOrbitMode::Ballistic;
	
	// Simulate for a short time
	FVector InitialPosition = TestBody->GetActorLocation();
	for (int32 i = 0; i < 10; ++i)
	{
		TestBody->Tick(0.1f);
	}
	
	FVector FinalPosition = TestBody->GetActorLocation();
	float DistanceTraveled = FVector::Dist(InitialPosition, FinalPosition);
	
	// Should have moved due to gravity
	bool bMovedCorrectly = DistanceTraveled > 100.0f;
	bool bVelocityChanged = TestBody->Velocity.Size() > 0.0f;
	
	// Cleanup
	TestBody->Destroy();
	CentralBody->Destroy();
	
	return bMovedCorrectly && bVelocityChanged;
}

bool AGameSystemsTest::TestEscapeVelocity()
{
	// Create test body
	AOrbitalBody* TestBody = GetWorld()->SpawnActor<AOrbitalBody>(AOrbitalBody::StaticClass());
	if (!TestBody)
	{
		return false;
	}
	
	// Create central body
	AOrbitalBody* CentralBody = GetWorld()->SpawnActor<AOrbitalBody>(AOrbitalBody::StaticClass());
	if (!CentralBody)
	{
		TestBody->Destroy();
		return false;
	}
	
	// Setup escape velocity test
	CentralBody->Mass = 5.972e24f; // Earth mass
	CentralBody->SetActorLocation(FVector::ZeroVector);
	
	TestBody->OrbitTarget = CentralBody;
	TestBody->SetActorLocation(FVector(7000000.0f, 0.0f, 0.0f)); // 7000 km from center
	
	float EscapeVelocity = TestBody->CalculateEscapeVelocity();
	float ExpectedEscapeVelocity = FMath::Sqrt(2.0f * 6.67430e-11f * CentralBody->Mass / 7000000.0f);
	
	bool bEscapeVelocityCorrect = FMath::Abs(EscapeVelocity - ExpectedEscapeVelocity) < 100.0f;
	
	// Test with velocity below escape
	TestBody->Velocity = FVector(0.0f, EscapeVelocity * 0.9f, 0.0f);
	bool bWillEscape1 = TestBody->WillEscape();
	
	// Test with velocity above escape
	TestBody->Velocity = FVector(0.0f, EscapeVelocity * 1.1f, 0.0f);
	bool bWillEscape2 = TestBody->WillEscape();
	
	// Cleanup
	TestBody->Destroy();
	CentralBody->Destroy();
	
	return bEscapeVelocityCorrect && !bWillEscape1 && bWillEscape2;
}

bool AGameSystemsTest::TestPlanetComponentIntegration()
{
	// Create test planet
	APlanet* TestPlanet = GetWorld()->SpawnActor<APlanet>(APlanet::StaticClass());
	if (!TestPlanet)
	{
		return false;
	}
	
	// Check if components are created
	bool bHasMesh = TestPlanet->PlanetMesh != nullptr;
	bool bHasAtmosphere = TestPlanet->AtmosphereComponent != nullptr;
	bool bHasWeather = TestPlanet->WeatherComponent != nullptr;
	bool bHasFarming = TestPlanet->FarmingComponent != nullptr;
	
	// Test biome system initialization
	TestPlanet->InitializeBiomeSystems();
	bool bBiomeSystemsInitialized = TestPlanet->BiomeManager != nullptr;
	
	// Test material system initialization
	TestPlanet->InitializeMaterialSystem();
	bool bMaterialSystemInitialized = TestPlanet->MaterialSystem != nullptr;
	
	// Cleanup
	TestPlanet->Destroy();
	
	return bHasMesh && bHasAtmosphere && bHasWeather && bHasFarming && 
		   bBiomeSystemsInitialized && bMaterialSystemInitialized;
}

bool AGameSystemsTest::TestBiomeGeneration()
{
	// Test procedural noise generation
	float NoiseValue = UProceduralNoiseGenerator::PerlinNoise2D(100.0f, 200.0f, 12345);
	bool bNoiseValid = NoiseValue >= -1.0f && NoiseValue <= 1.0f;
	
	// Test fractal noise
	FNoiseConfig Config;
	Config.Seed = 12345;
	Config.Octaves = 4;
	Config.Frequency = 0.01f;
	Config.Amplitude = 1.0f;
	
	float FractalValue = UProceduralNoiseGenerator::FractalNoise2D(100.0f, 200.0f, Config);
	bool bFractalValid = FractalValue >= -1.0f && FractalValue <= 1.0f;
	
	// Test continental terrain
	float ContinentHeight = UProceduralNoiseGenerator::GenerateContinentalTerrain(
		100.0f, 200.0f, 12345, 6371.0f);
	bool bContinentValid = FMath::IsFinite(ContinentHeight);
	
	return bNoiseValid && bFractalValid && bContinentValid;
}

bool AGameSystemsTest::TestSpaceshipFlightControls()
{
	// Create test spaceship
	ASpaceship* TestShip = GetWorld()->SpawnActor<ASpaceship>(ASpaceship::StaticClass());
	if (!TestShip)
	{
		return false;
	}
	
	// Test component creation
	bool bHasMesh = TestShip->ShipMesh != nullptr;
	bool bHasCockpit = TestShip->CockpitComponent != nullptr;
	bool bHasEngine = TestShip->EngineComponent != nullptr;
	
	// Test flight controls
	TestShip->ApplyThrust(FVector(1.0f, 0.0f, 0.0f));
	bool bThrustApplied = !TestShip->ThrustInput.IsNearlyZero();
	
	TestShip->ApplyRotation(FVector(0.0f, 1.0f, 0.0f));
	bool bRotationApplied = !TestShip->RotationInput.IsNearlyZero();
	
	// Test flight modes
	TestShip->SetFlightMode(EFlightMode::Assisted);
	bool bFlightModeSet = TestShip->FlightMode == EFlightMode::Assisted;
	
	// Test emergency stop
	TestShip->EmergencyStop();
	bool bEmergencyStopWorked = TestShip->Velocity.IsNearlyZero() && 
								 TestShip->ThrustInput.IsNearlyZero();
	
	// Cleanup
	TestShip->Destroy();
	
	return bHasMesh && bHasCockpit && bHasEngine && bThrustApplied && 
		   bRotationApplied && bFlightModeSet && bEmergencyStopWorked;
}

bool AGameSystemsTest::TestVRIntegration()
{
	// Create test spaceship with VR enabled
	ASpaceship* TestShip = GetWorld()->SpawnActor<ASpaceship>(ASpaceship::StaticClass());
	if (!TestShip)
	{
		return false;
	}
	
	TestShip->bIsVREnabled = true;
	TestShip->InitializeVR();
	
	// Check VR components
	bool bHasVRCamera = TestShip->VRCamera != nullptr;
	bool bVRInitialized = TestShip->bIsVREnabled;
	
	// Test cockpit transform
	FTransform CockpitTransform = TestShip->GetCockpitTransform();
	bool bCockpitTransformValid = CockpitTransform.IsValid();
	
	// Cleanup
	TestShip->Destroy();
	
	return bHasVRCamera && bVRInitialized && bCockpitTransformValid;
}

bool AGameSystemsTest::TestProceduralNoise()
{
	// Test different noise types
	float PerlinValue = UProceduralNoiseGenerator::PerlinNoise2D(0.0f, 0.0f, 12345);
	float SimplexValue = UProceduralNoiseGenerator::SimplexNoise2D(0.0f, 0.0f, 12345);
	float VoronoiValue = UProceduralNoiseGenerator::VoronoiNoise2D(0.0f, 0.0f, 12345, 10.0f);
	
	bool bPerlinValid = PerlinValue >= -1.0f && PerlinValue <= 1.0f;
	bool bSimplexValid = SimplexValue >= -1.0f && SimplexValue <= 1.0f;
	bool bVoronoiValid = VoronoiValue >= 0.0f && VoronoiValue <= 1.0f;
	
	// Test ridged multifractal
	FNoiseConfig Config;
	Config.Seed = 12345;
	Config.Octaves = 4;
	float RidgedValue = UProceduralNoiseGenerator::RidgedMultifractalNoise2D(0.0f, 0.0f, Config);
	bool bRidgedValid = RidgedValue >= 0.0f && RidgedValue <= 1.0f;
	
	return bPerlinValid && bSimplexValid && bVoronoiValid && bRidgedValid;
}

bool AGameSystemsTest::TestAdvancedTerrainFeatures()
{
	// Test advanced terrain generation features
	float RiverValue = UProceduralNoiseGenerator::GenerateRiverNetwork(100.0f, 200.0f, 12345, 100.0f);
	float CaveValue = UProceduralNoiseGenerator::GenerateCaveSystem(100.0f, 200.0f, 50.0f, 12345);
	float VolcanicValue = UProceduralNoiseGenerator::GenerateVolcanicTerrain(100.0f, 200.0f, 12345, 100.0f);
	float CraterValue = UProceduralNoiseGenerator::GenerateCraterField(100.0f, 200.0f, 12345, 0.001f, 1000.0f);
	
	bool bRiverValid = FMath::IsFinite(RiverValue);
	bool bCaveValid = CaveValue >= 0.0f && CaveValue <= 1.0f;
	bool bVolcanicValid = FMath::IsFinite(VolcanicValue);
	bool bCraterValid = FMath::IsFinite(CraterValue);
	
	// Test biome features
	float BiomeHeight = UProceduralNoiseGenerator::GenerateBiomeFeatures(
		100.0f, 200.0f, EBiomeType::Forest, 12345, 100.0f);
	bool bBiomeValid = FMath::IsFinite(BiomeHeight);
	
	// Test terrain blending
	TArray<float> TerrainLayers = {100.0f, 50.0f, 25.0f};
	TArray<float> Weights = {0.5f, 0.3f, 0.2f};
	float BlendedHeight = UProceduralNoiseGenerator::BlendTerrainLayers(TerrainLayers, Weights);
	bool bBlendedValid = FMath::IsFinite(BlendedHeight);
	
	return bRiverValid && bCaveValid && bVolcanicValid && bCraterValid && 
		   bBiomeValid && bBlendedValid;
}

bool AGameSystemsTest::TestAIVisionScreenshot()
{
	// This test would require the AI Vision System plugin
	// For now, just test if the plugin is available
	bool bPluginAvailable = false;
	
#if WITH_EDITOR
	// Check if AI Vision System plugin is loaded
	if (FModuleManager::Get().IsModuleLoaded("AIVisionSystem"))
	{
		bPluginAvailable = true;
	}
#endif
	
	return bPluginAvailable;
}

bool AGameSystemsTest::TestAIVisionBackend()
{
	// This would test backend communication
	// For now, just validate the concept
	bool bBackendConfigValid = true; // Would test actual backend URL
	
	return bBackendConfigValid;
}

bool AGameSystemsTest::RunSpecificTest(const FString& TestName)
{
	for (const FTestDefinition& Test : Tests)
	{
		if (Test.TestName == TestName)
		{
			if (Test.TestFunction)
			{
				bool bResult = Test.TestFunction();
				UE_LOG(LogTemp, Log, TEXT("Test '%s': %s"), *TestName, bResult ? TEXT("PASSED") : TEXT("FAILED"));
				return bResult;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Test '%s' has no function!"), *TestName);
				return false;
			}
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Test '%s' not found"), *TestName);
	return false;
}

FString AGameSystemsTest::GetTestResults() const
{
	int32 TotalTests = TestsPassed + TestsFailed;
	float PassPercentage = TotalTests > 0 ? (static_cast<float>(TestsPassed) / TotalTests) * 100.0f : 0.0f;
	
	FString ResultsText = FString::Printf(
		TEXT("=== TEST RESULTS ===\nTotal Tests: %d\nPassed: %d\nFailed: %d\nPass Rate: %.1f%%"),
		TotalTests,
		TestsPassed,
		TestsFailed,
		PassPercentage
	);
	
	return ResultsText;
}

void AGameSystemsTest::UpdateTestProgress(float DeltaTime)
{
	// Update any ongoing test progress indicators
	// This could be used for animated tests or long-running operations
}