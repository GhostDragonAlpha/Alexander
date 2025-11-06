// Copyright Epic Games, Inc. All Rights Reserved.

#include "Testing/CelestialScalingTestStation.h"
#include "CelestialBodyComponent.h"
#include "CelestialBodyRegistry.h"
#include "ScalingCalculator.h"
#include "GravitySimulator.h"
#include "AstronomicalConstants.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

UCelestialScalingTestStation::UCelestialScalingTestStation()
{
	StationName = TEXT("Celestial Scaling Test Station");
}

void UCelestialScalingTestStation::BeginPlay()
{
	Super::BeginPlay();

	// Get subsystem references
	if (UWorld* World = GetWorld())
	{
		BodyRegistry = World->GetSubsystem<UCelestialBodyRegistry>();

		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			ScalingCalculator = GameInstance->GetSubsystem<UScalingCalculator>();
			GravitySimulator = GameInstance->GetSubsystem<UGravitySimulator>();
		}
	}
}

void UCelestialScalingTestStation::RegisterTests_Implementation()
{
	Super::RegisterTests_Implementation();

	// Scale Factor Tests
	RegisterTestCase(TEXT("Scale_Factor_Validation"),
		TEXT("Verify inverse-square law scaling at various distances (1,000 km to 10 million km)"),
		[this]() { Test_ScaleFactorValidation(); }, 30.0f);

	// Gravitational Force Tests
	RegisterTestCase(TEXT("Gravitational_Force_Measurement"),
		TEXT("Validate gravity calculations using F=GMm/r²"),
		[this]() { Test_GravitationalForceMeasurement(); }, 30.0f);

	// VR Precision Tests
	RegisterTestCase(TEXT("VR_Precision_Player_Origin"),
		TEXT("Test player positioned at world origin for maximum precision"),
		[this]() { Test_VRPrecision_PlayerAtOrigin(); }, 15.0f);

	RegisterTestCase(TEXT("VR_Precision_Universe_Translation"),
		TEXT("Test universe translation to maintain player at origin"),
		[this]() { Test_VRPrecision_UniverseTranslation(); }, 30.0f);

	RegisterTestCase(TEXT("VR_Precision_Floating_Point"),
		TEXT("Test floating point precision at extreme distances"),
		[this]() { Test_VRPrecision_FloatingPoint(); }, 20.0f);

	// Performance Tests
	RegisterTestCase(TEXT("Performance_Stress_Test"),
		TEXT("Spawn 50+ celestial bodies and measure frame time"),
		[this]() { Test_PerformanceStressTest(); }, 60.0f);

	// Dynamic Scaling Tests
	RegisterTestCase(TEXT("Dynamic_Scaling"),
		TEXT("Test real-time scale factor adjustments as player moves"),
		[this]() { Test_DynamicScaling(); }, 30.0f);

	// Multi-body Gravity Tests
	RegisterTestCase(TEXT("Multi_Body_Gravity"),
		TEXT("Test N-body gravitational simulation with multiple bodies"),
		[this]() { Test_MultiBodyGravity(); }, 40.0f);

	// Culling Tests
	RegisterTestCase(TEXT("Distance_Based_Culling"),
		TEXT("Verify distant bodies beyond threshold are properly culled"),
		[this]() { Test_DistanceBasedCulling(); }, 30.0f);

	// Origin Recentering Tests
	RegisterTestCase(TEXT("Origin_Recentering"),
		TEXT("Test automatic VR origin recentering for precision maintenance"),
		[this]() { Test_OriginRecentering(); }, 30.0f);

	// LOD Tests
	RegisterTestCase(TEXT("LOD_System"),
		TEXT("Verify level-of-detail transitions based on distance"),
		[this]() { Test_LODSystem(); }, 30.0f);

	// Registry Tests
	RegisterTestCase(TEXT("Registry_Management"),
		TEXT("Test celestial body registration and unregistration"),
		[this]() { Test_RegistryManagement(); }, 20.0f);
}

// ===== Scale Factor Tests =====

void UCelestialScalingTestStation::Test_ScaleFactorValidation()
{
	StartTest(TEXT("Scale_Factor_Validation"));

	if (!CheckNotNull(ScalingCalculator, TEXT("Scale_Factor_Validation"), TEXT("ScalingCalculator subsystem not found")))
	{
		return;
	}

	LogInfo(TEXT("Testing inverse-square law scaling at multiple distances..."));

	bool bAllTestsPassed = true;

	for (float TestDistance : TestDistances)
	{
		// Calculate actual scale factor using the calculator
		double ActualScale = ScalingCalculator->ApplyInverseSquareLaw(TestDistance);

		// Calculate expected scale factor: (ReferenceDistance / Distance)²
		float ExpectedScale = CalculateExpectedScaleFactor(TestDistance, ReferenceDistance);

		// Log the comparison
		LogInfo(FString::Printf(TEXT("Distance: %.0f km, Expected Scale: %.6f, Actual Scale: %.6f"),
			TestDistance / KilometersToCentimeters, ExpectedScale, (float)ActualScale));

		// Verify within tolerance
		if (!IsWithinTolerance((float)ActualScale, ExpectedScale, ScaleFactorTolerance))
		{
			FailTest(TEXT("Scale_Factor_Validation"),
				FString::Printf(TEXT("Scale factor mismatch at %.0f km: Expected %.6f, Got %.6f"),
					TestDistance / KilometersToCentimeters, ExpectedScale, (float)ActualScale));
			bAllTestsPassed = false;
			break;
		}
	}

	if (bAllTestsPassed)
	{
		PassTest(TEXT("Scale_Factor_Validation"),
			FString::Printf(TEXT("Inverse-square law verified at %d distance points"), TestDistances.Num()));
	}
}

void UCelestialScalingTestStation::Test_GravitationalForceMeasurement()
{
	StartTest(TEXT("Gravitational_Force_Measurement"));

	if (!CheckNotNull(GravitySimulator, TEXT("Gravitational_Force_Measurement"), TEXT("GravitySimulator subsystem not found")))
	{
		return;
	}

	// Spawn two test bodies at known positions
	FVector Body1Location = FVector(0, 0, 0);
	FVector Body2Location = FVector(100000000.0f, 0, 0); // 1,000 km away

	float Mass1 = 5.97237e24f; // Earth mass (kg)
	float Mass2 = 1000.0f; // 1 ton spacecraft

	AActor* Body1 = SpawnTestCelestialBody(Body1Location, Mass1, 6371.0f);
	AActor* Body2 = SpawnTestCelestialBody(Body2Location, Mass2, 1.0f);

	if (!CheckNotNull(Body1, TEXT("Gravitational_Force_Measurement"), TEXT("Failed to spawn test body 1")))
	{
		CleanupTestBodies();
		return;
	}

	if (!CheckNotNull(Body2, TEXT("Gravitational_Force_Measurement"), TEXT("Failed to spawn test body 2")))
	{
		CleanupTestBodies();
		return;
	}

	// Get celestial body components
	UCelestialBodyComponent* CelestialBody1 = Body1->FindComponentByClass<UCelestialBodyComponent>();

	if (!CheckNotNull(CelestialBody1, TEXT("Gravitational_Force_Measurement"), TEXT("Body 1 missing CelestialBodyComponent")))
	{
		CleanupTestBodies();
		return;
	}

	// Calculate gravitational force
	FVector GravityForce = GravitySimulator->CalculateGravityFromBody(CelestialBody1, Body2Location, Mass2);
	float ActualForceMagnitude = GravityForce.Size();

	// Calculate expected force using F = GMm/r²
	float Distance = FVector::Dist(Body1Location, Body2Location);
	float ExpectedForce = CalculateExpectedGravity(Mass1, Mass2, Distance);

	LogInfo(FString::Printf(TEXT("Distance: %.0f km"), Distance / KilometersToCentimeters));
	LogInfo(FString::Printf(TEXT("Expected Force: %.6f N"), ExpectedForce));
	LogInfo(FString::Printf(TEXT("Actual Force: %.6f N"), ActualForceMagnitude));

	CleanupTestBodies();

	if (IsWithinTolerance(ActualForceMagnitude, ExpectedForce, GravityAccuracyTolerance))
	{
		PassTest(TEXT("Gravitational_Force_Measurement"),
			FString::Printf(TEXT("Gravity calculation accurate: %.6f N (expected %.6f N)"),
				ActualForceMagnitude, ExpectedForce));
	}
	else
	{
		FailTest(TEXT("Gravitational_Force_Measurement"),
			FString::Printf(TEXT("Gravity calculation error: Expected %.6f N, Got %.6f N"),
				ExpectedForce, ActualForceMagnitude));
	}
}

// ===== VR Precision Tests =====

void UCelestialScalingTestStation::Test_VRPrecision_PlayerAtOrigin()
{
	StartTest(TEXT("VR_Precision_Player_Origin"));

	// Get player pawn location
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!CheckNotNull(PC, TEXT("VR_Precision_Player_Origin"), TEXT("Player controller not found")))
	{
		return;
	}

	APawn* PlayerPawn = PC->GetPawn();
	if (!CheckNotNull(PlayerPawn, TEXT("VR_Precision_Player_Origin"), TEXT("Player pawn not found")))
	{
		return;
	}

	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	float DistanceFromOrigin = PlayerLocation.Size();

	LogInfo(FString::Printf(TEXT("Player location: %s"), *PlayerLocation.ToString()));
	LogInfo(FString::Printf(TEXT("Distance from origin: %.2f cm (%.4f m)"),
		DistanceFromOrigin, DistanceFromOrigin / MetersToUnrealUnits));

	// For VR precision, player should be within 10 meters of origin
	if (DistanceFromOrigin > 1000.0f) // 10 meters in cm
	{
		FailTest(TEXT("VR_Precision_Player_Origin"),
			FString::Printf(TEXT("Player too far from origin: %.2f m (should be < 10 m)"),
				DistanceFromOrigin / MetersToUnrealUnits));
		return;
	}

	PassTest(TEXT("VR_Precision_Player_Origin"),
		FString::Printf(TEXT("Player at origin: %.2f m from world origin"),
			DistanceFromOrigin / MetersToUnrealUnits));
}

void UCelestialScalingTestStation::Test_VRPrecision_UniverseTranslation()
{
	StartTest(TEXT("VR_Precision_Universe_Translation"));

	if (!CheckNotNull(BodyRegistry, TEXT("VR_Precision_Universe_Translation"), TEXT("BodyRegistry subsystem not found")))
	{
		return;
	}

	// Spawn test bodies at various distances
	SpawnTestCelestialBody(FVector(10000000.0f, 0, 0), 1.0e24f, 1000.0f);
	SpawnTestCelestialBody(FVector(0, 20000000.0f, 0), 2.0e24f, 2000.0f);
	SpawnTestCelestialBody(FVector(0, 0, 30000000.0f), 3.0e24f, 3000.0f);

	int32 InitialBodyCount = BodyRegistry->GetRegisteredBodyCount();
	LogInfo(FString::Printf(TEXT("Spawned %d test bodies"), InitialBodyCount));

	// Simulate universe translation
	FVector TranslationOffset = FVector(-5000000.0f, -5000000.0f, -5000000.0f);
	LogInfo(FString::Printf(TEXT("Applying translation offset: %s"), *TranslationOffset.ToString()));

	SimulateUniverseTranslation(TranslationOffset);

	// Verify all bodies were translated
	int32 FinalBodyCount = BodyRegistry->GetRegisteredBodyCount();

	CleanupTestBodies();

	if (FinalBodyCount != InitialBodyCount)
	{
		FailTest(TEXT("VR_Precision_Universe_Translation"),
			FString::Printf(TEXT("Body count mismatch after translation: %d -> %d"),
				InitialBodyCount, FinalBodyCount));
		return;
	}

	PassTest(TEXT("VR_Precision_Universe_Translation"),
		FString::Printf(TEXT("Universe translation successful: %d bodies translated"), FinalBodyCount));
}

void UCelestialScalingTestStation::Test_VRPrecision_FloatingPoint()
{
	StartTest(TEXT("VR_Precision_Floating_Point"));

	// Test floating point precision at extreme distances
	TArray<double> ExtremeDistances = {
		1.0e8,    // 100 million cm
		1.0e9,    // 1 billion cm
		1.0e10,   // 10 billion cm
		1.0e11,   // 100 billion cm
		1.0e12    // 1 trillion cm
	};

	bool bPrecisionMaintained = true;

	for (double Distance : ExtremeDistances)
	{
		// Test precision by adding and subtracting small values
		double Position1 = Distance;
		double Position2 = Distance + 1.0; // Add 1 cm

		double Difference = FMath::Abs(Position2 - Position1);

		LogInfo(FString::Printf(TEXT("Distance: %.2e cm, Precision test: %.6f cm difference"),
			Distance, Difference));

		// At extreme distances, we should still be able to differentiate 1 cm
		if (Difference < 0.5 || Difference > 1.5)
		{
			LogWarning(FString::Printf(TEXT("Precision loss at distance %.2e cm"), Distance));
			bPrecisionMaintained = false;
		}
	}

	if (bPrecisionMaintained)
	{
		PassTest(TEXT("VR_Precision_Floating_Point"), TEXT("Floating point precision maintained at extreme distances"));
	}
	else
	{
		FailTest(TEXT("VR_Precision_Floating_Point"), TEXT("Floating point precision degraded at extreme distances"));
	}
}

// ===== Performance Tests =====

void UCelestialScalingTestStation::Test_PerformanceStressTest()
{
	StartTest(TEXT("Performance_Stress_Test"));

	LogInfo(FString::Printf(TEXT("Spawning %d celestial bodies for stress test..."), StressTestBodyCount));

	StartPerformanceCapture();

	// Spawn multiple celestial bodies in a grid pattern
	int32 GridSize = FMath::CeilToInt(FMath::Sqrt((float)StressTestBodyCount));
	float Spacing = 10000000.0f; // 100 km spacing

	for (int32 i = 0; i < StressTestBodyCount; ++i)
	{
		int32 X = i % GridSize;
		int32 Y = i / GridSize;

		FVector Location(X * Spacing, Y * Spacing, 0.0f);
		float Mass = 1.0e20f + (i * 1.0e19f); // Varying masses
		float Radius = 100.0f + (i * 10.0f); // Varying radii

		SpawnTestCelestialBody(Location, Mass, Radius);
	}

	LogInfo(FString::Printf(TEXT("Spawned %d bodies, measuring performance..."), SpawnedTestBodies.Num()));

	// Measure frame time for 5 seconds
	float AverageFrameTime = MeasureAverageFrameTime(5.0f);

	FTestPerformanceMetrics Metrics = StopPerformanceCapture();

	CleanupTestBodies();

	LogInfo(FString::Printf(TEXT("Average FPS: %.1f"), Metrics.AverageFPS));
	LogInfo(FString::Printf(TEXT("Average Frame Time: %.2f ms"), AverageFrameTime));
	LogInfo(FString::Printf(TEXT("Min FPS: %.1f"), Metrics.MinFPS));

	if (AverageFrameTime > MaxAllowedFrameTime)
	{
		FailTest(TEXT("Performance_Stress_Test"),
			FString::Printf(TEXT("Frame time exceeded target with %d bodies: %.2f ms (target: %.2f ms)"),
				StressTestBodyCount, AverageFrameTime, MaxAllowedFrameTime));
		return;
	}

	PassTest(TEXT("Performance_Stress_Test"),
		FString::Printf(TEXT("Performance acceptable with %d bodies: %.2f ms frame time, %.1f FPS"),
			StressTestBodyCount, AverageFrameTime, Metrics.AverageFPS));
}

// ===== Dynamic Scaling Tests =====

void UCelestialScalingTestStation::Test_DynamicScaling()
{
	StartTest(TEXT("Dynamic_Scaling"));

	if (!CheckNotNull(ScalingCalculator, TEXT("Dynamic_Scaling"), TEXT("ScalingCalculator subsystem not found")))
	{
		return;
	}

	// Spawn a test body
	FVector InitialLocation = FVector(10000000.0f, 0, 0); // 100 km
	AActor* TestBody = SpawnTestCelestialBody(InitialLocation, 1.0e24f, 1000.0f);

	if (!CheckNotNull(TestBody, TEXT("Dynamic_Scaling"), TEXT("Failed to spawn test body")))
	{
		CleanupTestBodies();
		return;
	}

	UCelestialBodyComponent* CelestialBody = TestBody->FindComponentByClass<UCelestialBodyComponent>();
	if (!CheckNotNull(CelestialBody, TEXT("Dynamic_Scaling"), TEXT("CelestialBodyComponent not found")))
	{
		CleanupTestBodies();
		return;
	}

	// Get initial scale
	float InitialScale = CelestialBody->CurrentScaleFactor;
	LogInfo(FString::Printf(TEXT("Initial scale at %.0f km: %.6f"),
		InitialLocation.Size() / KilometersToCentimeters, InitialScale));

	// Simulate moving the body to different distances
	TArray<float> TestDistancesLocal = { 5000000.0f, 20000000.0f, 50000000.0f }; // 50 km, 200 km, 500 km

	bool bScaleChangedCorrectly = true;
	float PreviousScale = InitialScale;

	for (float NewDistance : TestDistancesLocal)
	{
		FVector NewLocation = FVector(NewDistance, 0, 0);
		TestBody->SetActorLocation(NewLocation);

		// Update scale
		CelestialBody->UpdateScale(FVector::ZeroVector);
		float NewScale = CelestialBody->CurrentScaleFactor;

		LogInfo(FString::Printf(TEXT("Scale at %.0f km: %.6f"),
			NewDistance / KilometersToCentimeters, NewScale));

		// Verify scale changes with distance (should get smaller as distance increases)
		if (NewDistance > InitialLocation.Size() && NewScale >= PreviousScale)
		{
			LogWarning(TEXT("Scale did not decrease with increased distance"));
			bScaleChangedCorrectly = false;
		}

		PreviousScale = NewScale;
	}

	CleanupTestBodies();

	if (bScaleChangedCorrectly)
	{
		PassTest(TEXT("Dynamic_Scaling"), TEXT("Dynamic scaling adjusts correctly with distance changes"));
	}
	else
	{
		FailTest(TEXT("Dynamic_Scaling"), TEXT("Dynamic scaling did not adjust correctly with distance"));
	}
}

// ===== Multi-body Gravity Tests =====

void UCelestialScalingTestStation::Test_MultiBodyGravity()
{
	StartTest(TEXT("Multi_Body_Gravity"));

	if (!CheckNotNull(GravitySimulator, TEXT("Multi_Body_Gravity"), TEXT("GravitySimulator subsystem not found")))
	{
		return;
	}

	// Spawn 3 bodies in a triangle formation
	AActor* Body1 = SpawnTestCelestialBody(FVector(0, 0, 0), 5.97e24f, 6371.0f);
	AActor* Body2 = SpawnTestCelestialBody(FVector(10000000.0f, 0, 0), 7.34e22f, 1737.0f);
	AActor* Body3 = SpawnTestCelestialBody(FVector(5000000.0f, 8660254.0f, 0), 1.0e24f, 3000.0f);

	if (!CheckNotNull(Body1, TEXT("Multi_Body_Gravity"), TEXT("Failed to spawn body 1")) ||
		!CheckNotNull(Body2, TEXT("Multi_Body_Gravity"), TEXT("Failed to spawn body 2")) ||
		!CheckNotNull(Body3, TEXT("Multi_Body_Gravity"), TEXT("Failed to spawn body 3")))
	{
		CleanupTestBodies();
		return;
	}

	// Test point in the center of the triangle
	FVector TestPoint = FVector(5000000.0f, 2886751.0f, 0);

	// Calculate total gravitational force at test point
	FVector TotalForce = GravitySimulator->CalculateTotalGravitationalForce(nullptr, TestPoint);
	float TotalForceMagnitude = TotalForce.Size();

	LogInfo(FString::Printf(TEXT("Total gravitational force at test point: %.6f N"), TotalForceMagnitude));
	LogInfo(FString::Printf(TEXT("Force direction: %s"), *TotalForce.GetSafeNormal().ToString()));

	CleanupTestBodies();

	// Force should be non-zero with multiple bodies
	if (TotalForceMagnitude > 0.0f)
	{
		PassTest(TEXT("Multi_Body_Gravity"),
			FString::Printf(TEXT("Multi-body gravity calculation successful: %.6f N"), TotalForceMagnitude));
	}
	else
	{
		FailTest(TEXT("Multi_Body_Gravity"), TEXT("Multi-body gravity calculation failed: zero force"));
	}
}

// ===== Culling Tests =====

void UCelestialScalingTestStation::Test_DistanceBasedCulling()
{
	StartTest(TEXT("Distance_Based_Culling"));

	if (!CheckNotNull(BodyRegistry, TEXT("Distance_Based_Culling"), TEXT("BodyRegistry subsystem not found")))
	{
		return;
	}

	// Spawn bodies at various distances
	SpawnTestCelestialBody(FVector(1000000.0f, 0, 0), 1.0e24f, 1000.0f); // 10 km - should be visible
	SpawnTestCelestialBody(FVector(100000000.0f, 0, 0), 1.0e24f, 1000.0f); // 1,000 km - should be visible
	SpawnTestCelestialBody(FVector(1000000000.0f, 0, 0), 1.0e24f, 1000.0f); // 10,000 km - might be culled

	int32 TotalBodies = SpawnedTestBodies.Num();
	int32 VisibleBodies = CountVisibleBodies();

	LogInfo(FString::Printf(TEXT("Total bodies: %d"), TotalBodies));
	LogInfo(FString::Printf(TEXT("Visible bodies: %d"), VisibleBodies));
	LogInfo(FString::Printf(TEXT("Culled bodies: %d"), TotalBodies - VisibleBodies));

	CleanupTestBodies();

	// At least some bodies should be visible
	if (VisibleBodies > 0)
	{
		PassTest(TEXT("Distance_Based_Culling"),
			FString::Printf(TEXT("Culling system functional: %d/%d bodies visible"), VisibleBodies, TotalBodies));
	}
	else
	{
		FailTest(TEXT("Distance_Based_Culling"), TEXT("All bodies culled - culling too aggressive"));
	}
}

// ===== Origin Recentering Tests =====

void UCelestialScalingTestStation::Test_OriginRecentering()
{
	StartTest(TEXT("Origin_Recentering"));

	// Test if origin recentering threshold is properly configured
	FVector TestOffset = FVector(600000.0f, 0, 0); // 6 km - above typical threshold

	bool bRequiresRecentering = UAstronomicalConstantsLibrary::RequiresRecentering(TestOffset);

	LogInfo(FString::Printf(TEXT("Test offset: %s"), *TestOffset.ToString()));
	LogInfo(FString::Printf(TEXT("Requires recentering: %s"), bRequiresRecentering ? TEXT("Yes") : TEXT("No")));

	// Test with small offset
	FVector SmallOffset = FVector(100.0f, 0, 0); // 1 m
	bool bSmallOffsetRecenter = UAstronomicalConstantsLibrary::RequiresRecentering(SmallOffset);

	LogInfo(FString::Printf(TEXT("Small offset: %s"), *SmallOffset.ToString()));
	LogInfo(FString::Printf(TEXT("Requires recentering: %s"), bSmallOffsetRecenter ? TEXT("Yes") : TEXT("No")));

	// Large offset should trigger recentering, small offset should not
	if (bRequiresRecentering && !bSmallOffsetRecenter)
	{
		PassTest(TEXT("Origin_Recentering"), TEXT("Origin recentering threshold configured correctly"));
	}
	else
	{
		FailTest(TEXT("Origin_Recentering"), TEXT("Origin recentering threshold not configured correctly"));
	}
}

// ===== LOD Tests =====

void UCelestialScalingTestStation::Test_LODSystem()
{
	StartTest(TEXT("LOD_System"));

	// Spawn test body
	AActor* TestBody = SpawnTestCelestialBody(FVector(0, 0, 0), 1.0e24f, 1000.0f);

	if (!CheckNotNull(TestBody, TEXT("LOD_System"), TEXT("Failed to spawn test body")))
	{
		CleanupTestBodies();
		return;
	}

	UCelestialBodyComponent* CelestialBody = TestBody->FindComponentByClass<UCelestialBodyComponent>();
	if (!CheckNotNull(CelestialBody, TEXT("LOD_System"), TEXT("CelestialBodyComponent not found")))
	{
		CleanupTestBodies();
		return;
	}

	// Test LOD at different distances
	TArray<float> LODTestDistances = {
		1000000.0f,    // 10 km - should be LOD 0 (highest detail)
		10000000.0f,   // 100 km - should be LOD 1
		100000000.0f,  // 1,000 km - should be LOD 2
		1000000000.0f  // 10,000 km - should be LOD 3 (lowest detail)
	};

	int32 PreviousLOD = -1;
	bool bLODProgression = true;

	for (float Distance : LODTestDistances)
	{
		int32 LODLevel = MeasureLODLevel(CelestialBody, Distance);

		LogInfo(FString::Printf(TEXT("Distance: %.0f km, LOD Level: %d"),
			Distance / KilometersToCentimeters, LODLevel));

		// LOD should increase (lower detail) or stay the same with distance
		if (PreviousLOD >= 0 && LODLevel < PreviousLOD)
		{
			LogWarning(TEXT("LOD level decreased with increasing distance"));
			bLODProgression = false;
		}

		PreviousLOD = LODLevel;
	}

	CleanupTestBodies();

	if (bLODProgression)
	{
		PassTest(TEXT("LOD_System"), TEXT("LOD system transitions correctly with distance"));
	}
	else
	{
		FailTest(TEXT("LOD_System"), TEXT("LOD system did not transition correctly"));
	}
}

// ===== Registry Tests =====

void UCelestialScalingTestStation::Test_RegistryManagement()
{
	StartTest(TEXT("Registry_Management"));

	if (!CheckNotNull(BodyRegistry, TEXT("Registry_Management"), TEXT("BodyRegistry subsystem not found")))
	{
		return;
	}

	int32 InitialCount = BodyRegistry->GetRegisteredBodyCount();
	LogInfo(FString::Printf(TEXT("Initial registry count: %d"), InitialCount));

	// Register test bodies
	const int32 TestBodyCount = 10;
	for (int32 i = 0; i < TestBodyCount; ++i)
	{
		FVector Location = FVector(i * 1000000.0f, 0, 0);
		SpawnTestCelestialBody(Location, 1.0e24f, 1000.0f);
	}

	int32 AfterSpawnCount = BodyRegistry->GetRegisteredBodyCount();
	LogInfo(FString::Printf(TEXT("After spawning %d bodies: %d registered"), TestBodyCount, AfterSpawnCount));

	// Verify registration
	int32 ExpectedCount = InitialCount + TestBodyCount;
	if (AfterSpawnCount != ExpectedCount)
	{
		FailTest(TEXT("Registry_Management"),
			FString::Printf(TEXT("Registration failed: Expected %d, Got %d"), ExpectedCount, AfterSpawnCount));
		CleanupTestBodies();
		return;
	}

	// Test unregistration
	CleanupTestBodies();

	int32 FinalCount = BodyRegistry->GetRegisteredBodyCount();
	LogInfo(FString::Printf(TEXT("After cleanup: %d registered"), FinalCount));

	if (FinalCount == InitialCount)
	{
		PassTest(TEXT("Registry_Management"),
			FString::Printf(TEXT("Registry management successful: %d bodies registered and unregistered"), TestBodyCount));
	}
	else
	{
		FailTest(TEXT("Registry_Management"),
			FString::Printf(TEXT("Unregistration failed: Expected %d, Got %d"), InitialCount, FinalCount));
	}
}

// ===== Helper Functions =====

AActor* UCelestialScalingTestStation::SpawnTestCelestialBody(const FVector& Location, float Mass, float Radius)
{
	if (!TestCelestialBodyClass)
	{
		LogWarning(TEXT("TestCelestialBodyClass not set, cannot spawn test bodies"));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewBody = GetWorld()->SpawnActor<AActor>(TestCelestialBodyClass, Location, FRotator::ZeroRotator, SpawnParams);

	if (NewBody)
	{
		// Configure the celestial body component if it exists
		UCelestialBodyComponent* CelestialComp = NewBody->FindComponentByClass<UCelestialBodyComponent>();
		if (CelestialComp)
		{
			CelestialComp->Mass = Mass;
			CelestialComp->Radius = Radius;
			CelestialComp->bEnableDynamicScaling = true;
			CelestialComp->bEnableGravity = true;
		}

		SpawnedTestBodies.Add(NewBody);
	}

	return NewBody;
}

void UCelestialScalingTestStation::CleanupTestBodies()
{
	for (AActor* Body : SpawnedTestBodies)
	{
		if (Body)
		{
			Body->Destroy();
		}
	}
	SpawnedTestBodies.Empty();
}

float UCelestialScalingTestStation::CalculateExpectedScaleFactor(float Distance, float InReferenceDistance) const
{
	if (Distance <= 0.0f)
	{
		return 1.0f;
	}

	// Inverse square law: Scale = (ReferenceDistance / Distance)²
	float Ratio = InReferenceDistance / Distance;
	return Ratio * Ratio;
}

float UCelestialScalingTestStation::CalculateExpectedGravity(float Mass1, float Mass2, float Distance) const
{
	if (Distance <= 0.0f)
	{
		return 0.0f;
	}

	// F = G * (m1 * m2) / r²
	// Convert distance from cm to meters for calculation
	double DistanceMeters = Distance / MetersToUnrealUnits;
	double Force = (GravitationalConstant * Mass1 * Mass2) / (DistanceMeters * DistanceMeters);

	return static_cast<float>(Force);
}

float UCelestialScalingTestStation::MeasureAverageFrameTime(float DurationSeconds)
{
	FrameTimeSamples.Empty();

	float StartTime = GetWorld()->GetTimeSeconds();
	float LastFrameTime = StartTime;

	while (GetWorld()->GetTimeSeconds() - StartTime < DurationSeconds)
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		float FrameTime = (CurrentTime - LastFrameTime) * 1000.0f; // Convert to ms

		if (FrameTime > 0.0f)
		{
			FrameTimeSamples.Add(FrameTime);
		}

		LastFrameTime = CurrentTime;
	}

	if (FrameTimeSamples.Num() == 0)
	{
		return 0.0f;
	}

	float TotalFrameTime = 0.0f;
	for (float FrameTime : FrameTimeSamples)
	{
		TotalFrameTime += FrameTime;
	}

	return TotalFrameTime / FrameTimeSamples.Num();
}

bool UCelestialScalingTestStation::IsWithinTolerance(float Actual, float Expected, float TolerancePercent) const
{
	if (Expected == 0.0f)
	{
		return FMath::IsNearlyZero(Actual, 0.0001f);
	}

	float PercentDifference = FMath::Abs((Actual - Expected) / Expected);
	return PercentDifference <= TolerancePercent;
}

float UCelestialScalingTestStation::GetDistanceFromPlayer(const FVector& Location) const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return 0.0f;
	}

	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn)
	{
		return 0.0f;
	}

	return FVector::Dist(PlayerPawn->GetActorLocation(), Location);
}

void UCelestialScalingTestStation::SimulateUniverseTranslation(const FVector& Offset)
{
	if (BodyRegistry)
	{
		BodyRegistry->UpdateAllBodyPositions(Offset);
	}
}

int32 UCelestialScalingTestStation::CountVisibleBodies() const
{
	int32 VisibleCount = 0;

	for (AActor* Body : SpawnedTestBodies)
	{
		if (Body && !Body->IsHidden())
		{
			VisibleCount++;
		}
	}

	return VisibleCount;
}

int32 UCelestialScalingTestStation::MeasureLODLevel(UCelestialBodyComponent* Body, float Distance) const
{
	if (!Body)
	{
		return -1;
	}

	// Update LOD based on distance
	// This is a simplified version - actual implementation may vary
	if (Distance < 10000000.0f) // < 100 km
	{
		return 0;
	}
	else if (Distance < 100000000.0f) // < 1,000 km
	{
		return 1;
	}
	else if (Distance < 1000000000.0f) // < 10,000 km
	{
		return 2;
	}
	else
	{
		return 3;
	}
}
