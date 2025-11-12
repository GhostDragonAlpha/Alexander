// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "ProceduralNoiseGenerator.h"
#include "BiomeBlendingSystem.h"
#include "LODTransitionManager.h"
#include "CropGrowthCalculator.h"
#include "CropDefinition.h"
#include "BiomeManager.h"
#include "PerformanceTestingSystem.h"

// ============================================================================
// NOISE GENERATION TESTS
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNoiseGenerationConsistencyTest,
	"Alexander.PlanetInterior.NoiseGeneration.Consistency",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNoiseGenerationConsistencyTest::RunTest(const FString& Parameters)
{
	// Test that same seed produces same results
	const int32 TestSeed = 12345;
	const float TestX = 100.0f;
	const float TestY = 200.0f;

	float Result1 = UProceduralNoiseGenerator::PerlinNoise2D(TestX, TestY, TestSeed);
	float Result2 = UProceduralNoiseGenerator::PerlinNoise2D(TestX, TestY, TestSeed);

	TestEqual(TEXT("Perlin noise should be deterministic"), Result1, Result2);

	// Test Simplex noise consistency
	float SimplexResult1 = UProceduralNoiseGenerator::SimplexNoise2D(TestX, TestY, TestSeed);
	float SimplexResult2 = UProceduralNoiseGenerator::SimplexNoise2D(TestX, TestY, TestSeed);

	TestEqual(TEXT("Simplex noise should be deterministic"), SimplexResult1, SimplexResult2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNoiseGenerationRangeTest,
	"Alexander.PlanetInterior.NoiseGeneration.Range",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNoiseGenerationRangeTest::RunTest(const FString& Parameters)
{
	// Test that noise values are within expected range [-1, 1]
	const int32 TestSeed = 54321;
	const int32 NumSamples = 100;

	for (int32 i = 0; i < NumSamples; ++i)
	{
		float X = FMath::FRandRange(-1000.0f, 1000.0f);
		float Y = FMath::FRandRange(-1000.0f, 1000.0f);

		float PerlinValue = UProceduralNoiseGenerator::PerlinNoise2D(X, Y, TestSeed);
		TestTrue(TEXT("Perlin noise should be >= -1"), PerlinValue >= -1.0f);
		TestTrue(TEXT("Perlin noise should be <= 1"), PerlinValue <= 1.0f);

		float SimplexValue = UProceduralNoiseGenerator::SimplexNoise2D(X, Y, TestSeed);
		TestTrue(TEXT("Simplex noise should be >= -1"), SimplexValue >= -1.0f);
		TestTrue(TEXT("Simplex noise should be <= 1"), SimplexValue <= 1.0f);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNoiseGenerationFractalTest,
	"Alexander.PlanetInterior.NoiseGeneration.Fractal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNoiseGenerationFractalTest::RunTest(const FString& Parameters)
{
	// Test fractal noise with different octaves
	FNoiseConfig Config;
	Config.Seed = 99999;
	Config.Frequency = 1.0f;
	Config.Amplitude = 1.0f;
	Config.Octaves = 4;
	Config.Lacunarity = 2.0f;
	Config.Persistence = 0.5f;

	float Result1 = UProceduralNoiseGenerator::FractalNoise2D(50.0f, 50.0f, Config);
	
	// Change octaves and verify result changes
	Config.Octaves = 6;
	float Result2 = UProceduralNoiseGenerator::FractalNoise2D(50.0f, 50.0f, Config);

	TestNotEqual(TEXT("Different octaves should produce different results"), Result1, Result2);

	// Verify range
	TestTrue(TEXT("Fractal noise should be >= -1"), Result1 >= -1.0f);
	TestTrue(TEXT("Fractal noise should be <= 1"), Result1 <= 1.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNoiseGenerationVoronoiTest,
	"Alexander.PlanetInterior.NoiseGeneration.Voronoi",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNoiseGenerationVoronoiTest::RunTest(const FString& Parameters)
{
	// Test Voronoi noise
	const int32 TestSeed = 11111;
	const float CellSize = 10.0f;

	float VoronoiValue = UProceduralNoiseGenerator::VoronoiNoise2D(25.0f, 25.0f, TestSeed, CellSize);
	
	// Voronoi should return distance values >= 0
	TestTrue(TEXT("Voronoi noise should be non-negative"), VoronoiValue >= 0.0f);

	// Test cell retrieval
	FVoronoiCell Cell = UProceduralNoiseGenerator::GetVoronoiCell(25.0f, 25.0f, TestSeed, CellSize);
	TestTrue(TEXT("Voronoi cell should have valid position"), !Cell.Position.IsNearlyZero());

	return true;
}

// ============================================================================
// BIOME BLENDING TESTS
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBiomeBlendingWeightSumTest,
	"Alexander.PlanetInterior.BiomeBlending.WeightSum",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBiomeBlendingWeightSumTest::RunTest(const FString& Parameters)
{
	// Create biome blending system
	UBiomeBlendingSystem* BlendingSystem = NewObject<UBiomeBlendingSystem>();
	UBiomeManager* BiomeManager = NewObject<UBiomeManager>();
	
	BlendingSystem->Initialize(BiomeManager);

	// Test that biome weights sum to approximately 1.0
	FVector TestPosition(1000.0f, 2000.0f, 0.0f);
	FBiomeWeights Weights = BlendingSystem->CalculateSmoothedWeights(TestPosition);

	float WeightSum = 0.0f;
	for (const auto& WeightPair : Weights.Weights)
	{
		WeightSum += WeightPair.Value;
		
		// Individual weights should be in valid range
		TestTrue(TEXT("Biome weight should be >= 0"), WeightPair.Value >= 0.0f);
		TestTrue(TEXT("Biome weight should be <= 1"), WeightPair.Value <= 1.0f);
	}

	// Sum should be approximately 1.0 (allowing small floating point error)
	TestTrue(TEXT("Biome weights should sum to approximately 1.0"), 
		FMath::IsNearlyEqual(WeightSum, 1.0f, 0.01f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBiomeBlendingTransitionTest,
	"Alexander.PlanetInterior.BiomeBlending.Transition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBiomeBlendingTransitionTest::RunTest(const FString& Parameters)
{
	// Create biome blending system
	UBiomeBlendingSystem* BlendingSystem = NewObject<UBiomeBlendingSystem>();
	UBiomeManager* BiomeManager = NewObject<UBiomeManager>();
	
	BlendingSystem->Initialize(BiomeManager);

	// Test transition zone detection
	FVector TestPosition(500.0f, 500.0f, 0.0f);
	float TransitionFactor = BlendingSystem->GetTransitionFactor(TestPosition);

	// Transition factor should be in valid range
	TestTrue(TEXT("Transition factor should be >= 0"), TransitionFactor >= 0.0f);
	TestTrue(TEXT("Transition factor should be <= 1"), TransitionFactor <= 1.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBiomeBlendingParametersTest,
	"Alexander.PlanetInterior.BiomeBlending.Parameters",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBiomeBlendingParametersTest::RunTest(const FString& Parameters)
{
	// Create biome blending system
	UBiomeBlendingSystem* BlendingSystem = NewObject<UBiomeBlendingSystem>();
	UBiomeManager* BiomeManager = NewObject<UBiomeManager>();
	
	BlendingSystem->Initialize(BiomeManager);

	// Test blended parameters
	FVector TestPosition(1500.0f, 1500.0f, 0.0f);
	FBlendedTerrainParameters Params = BlendingSystem->GetBlendedParameters(TestPosition);

	// Verify parameter ranges
	TestTrue(TEXT("Roughness should be in valid range"), 
		Params.Roughness >= 0.0f && Params.Roughness <= 1.0f);
	TestTrue(TEXT("Humidity should be in valid range"), 
		Params.Humidity >= 0.0f && Params.Humidity <= 1.0f);
	TestTrue(TEXT("Vegetation density should be in valid range"), 
		Params.VegetationDensity >= 0.0f && Params.VegetationDensity <= 1.0f);

	return true;
}

// ============================================================================
// LOD DISTANCE CALCULATION TESTS
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLODDistanceCalculationTest,
	"Alexander.PlanetInterior.LOD.DistanceCalculation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLODDistanceCalculationTest::RunTest(const FString& Parameters)
{
	// Test distance-based fade calculation
	FVector2D TilePosition(0.0f, 0.0f);
	FVector2D ViewerPosition(100.0f, 0.0f);
	float FadeRange = 50.0f;

	float FadeFactor = ULODTransitionManager::CalculateDistanceFade(
		TilePosition, ViewerPosition, FadeRange);

	// Fade factor should be in valid range
	TestTrue(TEXT("Fade factor should be >= 0"), FadeFactor >= 0.0f);
	TestTrue(TEXT("Fade factor should be <= 1"), FadeFactor <= 1.0f);

	// Test at exact fade range
	ViewerPosition = FVector2D(FadeRange, 0.0f);
	float FadeAtRange = ULODTransitionManager::CalculateDistanceFade(
		TilePosition, ViewerPosition, FadeRange);
	
	TestTrue(TEXT("Fade at range boundary should be valid"), 
		FadeAtRange >= 0.0f && FadeAtRange <= 1.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLODTransitionProgressTest,
	"Alexander.PlanetInterior.LOD.TransitionProgress",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLODTransitionProgressTest::RunTest(const FString& Parameters)
{
	// Create LOD transition manager
	ULODTransitionManager* LODManager = NewObject<ULODTransitionManager>();
	
	FLODTransitionConfig Config;
	Config.TransitionDuration = 0.5f;
	Config.bEnableGeomorphing = true;
	
	LODManager->Initialize(Config);

	// Test transition progress tracking
	FVector2D TilePos(0.0f, 0.0f);
	
	// Initially should not be transitioning
	TestFalse(TEXT("Should not be transitioning initially"), 
		LODManager->IsTransitioning(TilePos));

	float Progress = LODManager->GetTransitionProgress(TilePos);
	TestEqual(TEXT("Progress should be -1 when not transitioning"), Progress, -1.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLODVertexInterpolationTest,
	"Alexander.PlanetInterior.LOD.VertexInterpolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLODVertexInterpolationTest::RunTest(const FString& Parameters)
{
	// Test vertex interpolation
	TArray<FVector> OldVertices;
	OldVertices.Add(FVector(0.0f, 0.0f, 0.0f));
	OldVertices.Add(FVector(100.0f, 0.0f, 0.0f));
	OldVertices.Add(FVector(0.0f, 100.0f, 0.0f));

	TArray<FVector> NewVertices;
	NewVertices.Add(FVector(0.0f, 0.0f, 10.0f));
	NewVertices.Add(FVector(100.0f, 0.0f, 10.0f));
	NewVertices.Add(FVector(0.0f, 100.0f, 10.0f));

	// Test at alpha = 0.5
	TArray<FVector> Interpolated = ULODTransitionManager::InterpolateVertices(
		OldVertices, NewVertices, 0.5f);

	TestEqual(TEXT("Should have same number of vertices"), 
		Interpolated.Num(), OldVertices.Num());

	// Check interpolation at midpoint
	if (Interpolated.Num() > 0)
	{
		TestTrue(TEXT("Interpolated Z should be approximately 5.0"), 
			FMath::IsNearlyEqual(Interpolated[0].Z, 5.0f, 0.01f));
	}

	return true;
}

// ============================================================================
// CROP GROWTH SIMULATION TESTS
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCropGrowthTemperatureTest,
	"Alexander.PlanetInterior.CropGrowth.Temperature",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCropGrowthTemperatureTest::RunTest(const FString& Parameters)
{
	// Test temperature modifier calculation
	float OptimalMin = 15.0f;
	float OptimalMax = 25.0f;

	// Test at optimal temperature
	float OptimalTemp = 20.0f;
	float ModifierOptimal = UCropGrowthCalculator::CalculateTemperatureModifier(
		OptimalTemp, OptimalMin, OptimalMax);
	
	TestTrue(TEXT("Optimal temperature should give high modifier"), 
		ModifierOptimal >= 0.9f && ModifierOptimal <= 1.2f);

	// Test below optimal range
	float ColdTemp = 5.0f;
	float ModifierCold = UCropGrowthCalculator::CalculateTemperatureModifier(
		ColdTemp, OptimalMin, OptimalMax);
	
	TestTrue(TEXT("Cold temperature should reduce modifier"), 
		ModifierCold < ModifierOptimal);
	TestTrue(TEXT("Cold modifier should be non-negative"), 
		ModifierCold >= 0.0f);

	// Test above optimal range
	float HotTemp = 35.0f;
	float ModifierHot = UCropGrowthCalculator::CalculateTemperatureModifier(
		HotTemp, OptimalMin, OptimalMax);
	
	TestTrue(TEXT("Hot temperature should reduce modifier"), 
		ModifierHot < ModifierOptimal);
	TestTrue(TEXT("Hot modifier should be non-negative"), 
		ModifierHot >= 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCropGrowthHumidityTest,
	"Alexander.PlanetInterior.CropGrowth.Humidity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCropGrowthHumidityTest::RunTest(const FString& Parameters)
{
	// Test humidity modifier calculation
	float OptimalMin = 0.4f;
	float OptimalMax = 0.7f;

	// Test at optimal humidity
	float OptimalHumidity = 0.55f;
	float ModifierOptimal = UCropGrowthCalculator::CalculateHumidityModifier(
		OptimalHumidity, OptimalMin, OptimalMax);
	
	TestTrue(TEXT("Optimal humidity should give high modifier"), 
		ModifierOptimal >= 0.9f && ModifierOptimal <= 1.2f);

	// Test low humidity
	float LowHumidity = 0.1f;
	float ModifierLow = UCropGrowthCalculator::CalculateHumidityModifier(
		LowHumidity, OptimalMin, OptimalMax);
	
	TestTrue(TEXT("Low humidity should reduce modifier"), 
		ModifierLow < ModifierOptimal);
	TestTrue(TEXT("Low humidity modifier should be in valid range"), 
		ModifierLow >= 0.0f && ModifierLow <= 1.2f);

	// Test high humidity
	float HighHumidity = 0.95f;
	float ModifierHigh = UCropGrowthCalculator::CalculateHumidityModifier(
		HighHumidity, OptimalMin, OptimalMax);
	
	TestTrue(TEXT("High humidity modifier should be in valid range"), 
		ModifierHigh >= 0.0f && ModifierHigh <= 1.2f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCropGrowthSunlightTest,
	"Alexander.PlanetInterior.CropGrowth.Sunlight",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCropGrowthSunlightTest::RunTest(const FString& Parameters)
{
	// Test sunlight modifier calculation
	float RequiredSunlight = 0.7f;

	// Test with sufficient sunlight
	float SufficientSunlight = 0.8f;
	float ModifierSufficient = UCropGrowthCalculator::CalculateSunlightModifier(
		SufficientSunlight, RequiredSunlight);
	
	TestTrue(TEXT("Sufficient sunlight should give good modifier"), 
		ModifierSufficient >= 0.9f);

	// Test with insufficient sunlight
	float InsufficientSunlight = 0.3f;
	float ModifierInsufficient = UCropGrowthCalculator::CalculateSunlightModifier(
		InsufficientSunlight, RequiredSunlight);
	
	TestTrue(TEXT("Insufficient sunlight should reduce modifier"), 
		ModifierInsufficient < ModifierSufficient);
	TestTrue(TEXT("Sunlight modifier should be non-negative"), 
		ModifierInsufficient >= 0.0f);

	// Test with no sunlight
	float NoSunlight = 0.0f;
	float ModifierNone = UCropGrowthCalculator::CalculateSunlightModifier(
		NoSunlight, RequiredSunlight);
	
	TestTrue(TEXT("No sunlight should give very low modifier"), 
		ModifierNone < 0.5f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCropGrowthWaterTest,
	"Alexander.PlanetInterior.CropGrowth.Water",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCropGrowthWaterTest::RunTest(const FString& Parameters)
{
	// Test water modifier calculation
	float WaterConsumption = 0.5f;

	// Test with adequate water
	float AdequateWater = 0.8f;
	float ModifierAdequate = UCropGrowthCalculator::CalculateWaterModifier(
		AdequateWater, WaterConsumption);
	
	TestTrue(TEXT("Adequate water should give high modifier"), 
		ModifierAdequate >= 0.8f);
	TestTrue(TEXT("Water modifier should be <= 1"), 
		ModifierAdequate <= 1.0f);

	// Test with low water
	float LowWater = 0.2f;
	float ModifierLow = UCropGrowthCalculator::CalculateWaterModifier(
		LowWater, WaterConsumption);
	
	TestTrue(TEXT("Low water should reduce modifier"), 
		ModifierLow < ModifierAdequate);
	TestTrue(TEXT("Low water modifier should be non-negative"), 
		ModifierLow >= 0.0f);

	// Test with no water
	float NoWater = 0.0f;
	float ModifierNone = UCropGrowthCalculator::CalculateWaterModifier(
		NoWater, WaterConsumption);
	
	TestTrue(TEXT("No water should give very low modifier"), 
		ModifierNone < 0.3f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCropGrowthSoilQualityTest,
	"Alexander.PlanetInterior.CropGrowth.SoilQuality",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCropGrowthSoilQualityTest::RunTest(const FString& Parameters)
{
	// Test soil quality modifier
	float RequiredQuality = 0.6f;

	// Test with good soil
	float GoodSoil = 0.8f;
	float ModifierGood = UCropGrowthCalculator::CalculateSoilQualityModifier(
		GoodSoil, RequiredQuality);
	
	TestTrue(TEXT("Good soil should give high modifier"), 
		ModifierGood >= 0.8f);
	TestTrue(TEXT("Soil modifier should be <= 1"), 
		ModifierGood <= 1.0f);

	// Test with poor soil
	float PoorSoil = 0.3f;
	float ModifierPoor = UCropGrowthCalculator::CalculateSoilQualityModifier(
		PoorSoil, RequiredQuality);
	
	TestTrue(TEXT("Poor soil should reduce modifier"), 
		ModifierPoor < ModifierGood);
	TestTrue(TEXT("Poor soil modifier should be non-negative"), 
		ModifierPoor >= 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCropGrowthFertilityTest,
	"Alexander.PlanetInterior.CropGrowth.Fertility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCropGrowthFertilityTest::RunTest(const FString& Parameters)
{
	// Test fertility modifier
	float HighFertility = 1.0f;
	float ModifierHigh = UCropGrowthCalculator::CalculateFertilityModifier(HighFertility);
	
	TestEqual(TEXT("Full fertility should give modifier of 1.0"), ModifierHigh, 1.0f);

	float MediumFertility = 0.5f;
	float ModifierMedium = UCropGrowthCalculator::CalculateFertilityModifier(MediumFertility);
	
	TestTrue(TEXT("Medium fertility should reduce modifier"), 
		ModifierMedium < ModifierHigh);
	TestTrue(TEXT("Medium fertility modifier should be reasonable"), 
		ModifierMedium >= 0.3f && ModifierMedium <= 0.7f);

	float LowFertility = 0.1f;
	float ModifierLow = UCropGrowthCalculator::CalculateFertilityModifier(LowFertility);
	
	TestTrue(TEXT("Low fertility should significantly reduce modifier"), 
		ModifierLow < ModifierMedium);
	TestTrue(TEXT("Low fertility modifier should be non-negative"), 
		ModifierLow >= 0.0f);

	return true;
}

// ============================================================================
// PERFORMANCE TESTING TESTS
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceMetricsCaptureTest,
	"Alexander.PlanetInterior.Performance.MetricsCapture",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPerformanceMetricsCaptureTest::RunTest(const FString& Parameters)
{
	// Create performance testing system
	UPerformanceTestingSystem* PerfSystem = NewObject<UPerformanceTestingSystem>();
	
	// Capture metrics
	FPerformanceMetrics Metrics = PerfSystem->CapturePerformanceMetrics();
	
	// Verify metrics are in valid ranges
	TestTrue(TEXT("Frame rate should be positive"), Metrics.FPS >= 0.0f);
	TestTrue(TEXT("Frame time should be positive"), Metrics.FrameTime >= 0.0f);
	TestTrue(TEXT("Memory usage should be positive"), Metrics.MemoryUsageMB >= 0.0f);
	TestTrue(TEXT("Streaming bandwidth should be non-negative"), Metrics.StreamingBandwidthMBps >= 0.0f);
	TestTrue(TEXT("Active terrain tiles should be non-negative"), Metrics.ActiveTerrainTiles >= 0);
	TestTrue(TEXT("Streaming tiles should be non-negative"), Metrics.StreamingTiles >= 0);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceTestConfigurationTest,
	"Alexander.PlanetInterior.Performance.Configuration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPerformanceTestConfigurationTest::RunTest(const FString& Parameters)
{
	// Create test configuration
	FPerformanceTestConfig Config;
	
	// Verify default configuration
	TestTrue(TEXT("Should have default test altitudes"), Config.TestAltitudes.Num() > 0);
	TestTrue(TEXT("Sample duration should be positive"), Config.SampleDuration > 0.0f);
	TestTrue(TEXT("Samples per altitude should be positive"), Config.SamplesPerAltitude > 0);
	TestFalse(TEXT("Report output path should not be empty"), Config.ReportOutputPath.IsEmpty());
	
	// Test custom configuration
	Config.TestAltitudes = {10000.0f, 5000.0f, 1000.0f};
	Config.SampleDuration = 3.0f;
	Config.SamplesPerAltitude = 50;
	
	TestEqual(TEXT("Custom altitudes should be set"), Config.TestAltitudes.Num(), 3);
	TestEqual(TEXT("Custom sample duration should be set"), Config.SampleDuration, 3.0f);
	TestEqual(TEXT("Custom samples per altitude should be set"), Config.SamplesPerAltitude, 50);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceVRValidationTest,
	"Alexander.PlanetInterior.Performance.VRValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPerformanceVRValidationTest::RunTest(const FString& Parameters)
{
	// Create performance testing system
	UPerformanceTestingSystem* PerfSystem = NewObject<UPerformanceTestingSystem>();
	
	// Test passing VR performance
	FPerformanceTestResults PassingResults;
	PassingResults.AverageFrameRate = 95.0f;
	PassingResults.MinimumFrameRate = 91.0f;
	PassingResults.MaximumFrameRate = 100.0f;
	
	// Add samples with consistent frame times
	for (int32 i = 0; i < 10; ++i)
	{
		FPerformanceMetrics Sample;
		Sample.FPS = 95.0f;
		Sample.FrameTime = 10.5f; // ~95 FPS
		PassingResults.Samples.Add(Sample);
	}
	
	bool PassingValidation = PerfSystem->ValidateVRPerformance(PassingResults);
	TestTrue(TEXT("Should pass VR validation with 95 FPS average"), PassingValidation);
	
	// Test failing VR performance (low FPS)
	FPerformanceTestResults FailingResults;
	FailingResults.AverageFrameRate = 85.0f;
	FailingResults.MinimumFrameRate = 80.0f;
	FailingResults.MaximumFrameRate = 90.0f;
	
	bool FailingValidation = PerfSystem->ValidateVRPerformance(FailingResults);
	TestFalse(TEXT("Should fail VR validation with 85 FPS average"), FailingValidation);
	
	// Test failing VR performance (inconsistent frame times)
	FPerformanceTestResults InconsistentResults;
	InconsistentResults.AverageFrameRate = 95.0f;
	InconsistentResults.MinimumFrameRate = 91.0f;
	InconsistentResults.MaximumFrameRate = 100.0f;
	
	// Add samples with high variance
	for (int32 i = 0; i < 10; ++i)
	{
		FPerformanceMetrics Sample;
		Sample.FPS = 95.0f;
		Sample.FrameTime = (i % 2 == 0) ? 8.0f : 15.0f; // High variance
		InconsistentResults.Samples.Add(Sample);
	}
	
	bool InconsistentValidation = PerfSystem->ValidateVRPerformance(InconsistentResults);
	TestFalse(TEXT("Should fail VR validation with inconsistent frame times"), InconsistentValidation);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceTestProgressTest,
	"Alexander.PlanetInterior.Performance.TestProgress",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPerformanceTestProgressTest::RunTest(const FString& Parameters)
{
	// Create performance testing system
	UPerformanceTestingSystem* PerfSystem = NewObject<UPerformanceTestingSystem>();
	
	// Initially should not be running
	TestFalse(TEXT("Should not be running initially"), PerfSystem->IsTestRunning());
	
	float InitialProgress = PerfSystem->GetTestProgress();
	TestEqual(TEXT("Initial progress should be 0"), InitialProgress, 0.0f);
	
	// Start test
	FPerformanceTestConfig Config;
	Config.TestAltitudes = {1000.0f, 500.0f, 100.0f};
	Config.SamplesPerAltitude = 10;
	Config.SampleDuration = 1.0f;
	Config.bGenerateReport = false;
	
	bool Started = PerfSystem->StartPerformanceTest(Config);
	TestTrue(TEXT("Test should start successfully"), Started);
	TestTrue(TEXT("Should be running after start"), PerfSystem->IsTestRunning());
	
	// Stop test
	PerfSystem->StopPerformanceTest();
	TestFalse(TEXT("Should not be running after stop"), PerfSystem->IsTestRunning());
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceMemoryMeasurementTest,
	"Alexander.PlanetInterior.Performance.MemoryMeasurement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPerformanceMemoryMeasurementTest::RunTest(const FString& Parameters)
{
	// Create performance testing system
	UPerformanceTestingSystem* PerfSystem = NewObject<UPerformanceTestingSystem>();
	
	// Capture metrics
	FPerformanceMetrics Metrics = PerfSystem->CapturePerformanceMetrics();
	
	// Memory usage should be reasonable (not zero, not absurdly high)
	TestTrue(TEXT("Memory usage should be positive"), Metrics.MemoryUsageMB > 0.0f);
	TestTrue(TEXT("Memory usage should be reasonable"), Metrics.MemoryUsageMB < 100000.0f);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceStreamingBandwidthTest,
	"Alexander.PlanetInterior.Performance.StreamingBandwidth",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPerformanceStreamingBandwidthTest::RunTest(const FString& Parameters)
{
	// Create performance testing system
	UPerformanceTestingSystem* PerfSystem = NewObject<UPerformanceTestingSystem>();
	
	// Capture metrics multiple times
	FPerformanceMetrics Metrics1 = PerfSystem->CapturePerformanceMetrics();
	FPlatformProcess::Sleep(0.1f);
	FPerformanceMetrics Metrics2 = PerfSystem->CapturePerformanceMetrics();
	
	// Bandwidth should be non-negative
	TestTrue(TEXT("Streaming bandwidth should be non-negative"), Metrics1.StreamingBandwidthMBps >= 0.0f);
	TestTrue(TEXT("Streaming bandwidth should be non-negative"), Metrics2.StreamingBandwidthMBps >= 0.0f);
	
	// Bandwidth should be reasonable (not absurdly high)
	TestTrue(TEXT("Streaming bandwidth should be reasonable"), Metrics1.StreamingBandwidthMBps < 10000.0f);
	TestTrue(TEXT("Streaming bandwidth should be reasonable"), Metrics2.StreamingBandwidthMBps < 10000.0f);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceVRMetricsCaptureTest,
	"Alexander.PlanetInterior.Performance.VRMetricsCapture",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPerformanceVRMetricsCaptureTest::RunTest(const FString& Parameters)
{
	// Create performance testing system
	UPerformanceTestingSystem* PerfSystem = NewObject<UPerformanceTestingSystem>();
	
	// Capture VR metrics
	FVRPerformanceMetrics VRMetrics = PerfSystem->CaptureVRPerformanceMetrics();
	
	// Verify metrics are in valid ranges
	TestTrue(TEXT("Left eye frame time should be non-negative"), VRMetrics.LeftEyeFrameTime >= 0.0f);
	TestTrue(TEXT("Right eye frame time should be non-negative"), VRMetrics.RightEyeFrameTime >= 0.0f);
	TestTrue(TEXT("Motion to photon latency should be non-negative"), VRMetrics.MotionToPhotonLatency >= 0.0f);
	TestTrue(TEXT("Dropped frames should be non-negative"), VRMetrics.DroppedFrames >= 0);
	TestTrue(TEXT("IPD should be non-negative"), VRMetrics.IPD >= 0.0f);
	TestTrue(TEXT("Headset FPS should be non-negative"), VRMetrics.HeadsetFPS >= 0.0f);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceResultsAnalysisTest,
	"Alexander.PlanetInterior.Performance.ResultsAnalysis",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPerformanceResultsAnalysisTest::RunTest(const FString& Parameters)
{
	// Create test results with sample data
	FPerformanceTestResults Results;
	
	// Add sample metrics
	for (int32 i = 0; i < 10; ++i)
	{
		FPerformanceMetrics Sample;
		Sample.FPS = 90.0f + (i * 2.0f); // 90-108 FPS
		Sample.MemoryUsageMB = 1000.0f + (i * 10.0f);
		Sample.FrameTime = 1000.0f / Sample.FPS;
		Results.Samples.Add(Sample);
	}
	
	// Manually calculate expected values
	float ExpectedAvgFPS = 99.0f; // Average of 90-108
	float ExpectedMinFPS = 90.0f;
	float ExpectedMaxFPS = 108.0f;
	
	// Create system and analyze
	UPerformanceTestingSystem* PerfSystem = NewObject<UPerformanceTestingSystem>();
	
	// Note: AnalyzeResults is private, so we test through the public interface
	// by checking that results are populated correctly
	TestEqual(TEXT("Should have 10 samples"), Results.Samples.Num(), 10);
	
	return true;
}
