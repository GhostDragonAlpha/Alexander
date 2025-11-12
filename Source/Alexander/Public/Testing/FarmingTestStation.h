// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Testing/TestStation.h"
#include "CropSystem.h"
#include "FarmingTestStation.generated.h"

// Forward declarations
class AFarmPlot;
class UCropDefinition;
class UVRSeedItem;
class UVRPlantingInteractionManager;
class UVRWateringCan;
class UVRHarvestInteractionManager;
class UFarmingSubsystem;
class USoilSystem;
class UIrrigationSystem;
class UBiomeCompatibilitySystem;
class UCropGrowthStageManager;
class UCropHealthSystem;

/**
 * Farming Test Station
 *
 * Tests:
 * 1. Planting System - VR seed placement, soil interaction
 * 2. Growth System - Crop growth stages (0-100%), progression rates
 * 3. Watering System - VR watering can, moisture levels
 * 4. Harvesting System - VR harvest interaction, yield calculation
 * 5. Crop Health - Health system, environmental factors
 * 6. Biome Compatibility - Crops in different biomes
 * 7. Soil Quality - Soil system integration
 * 8. Accelerated Growth - Fast-forward for testing
 * 9. Multiple Crop Types - All 10 crop types
 * 10. VR Interaction - All VR farming interactions
 *
 * Requirements: 5.1, 5.2, 5.3, 5.4, 5.5
 */
UCLASS(ClassGroup=(Testing), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UFarmingTestStation : public UTestStation
{
	GENERATED_BODY()

public:
	UFarmingTestStation();

protected:
	virtual void BeginPlay() override;
	virtual void RegisterTests_Implementation() override;

public:
	// Test configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Testing|Config")
	TSubclassOf<AFarmPlot> TestFarmPlotClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Testing|Config")
	TArray<ECropTypeExtended> CropTypesToTest = {
		ECropTypeExtended::Wheat,
		ECropTypeExtended::Corn,
		ECropTypeExtended::Tomatoes,
		ECropTypeExtended::Potatoes,
		ECropTypeExtended::Carrots,
		ECropTypeExtended::Lettuce,
		ECropTypeExtended::Strawberries,
		ECropTypeExtended::SpaceWeed,
		ECropTypeExtended::Rice,
		ECropTypeExtended::QuantumPlants
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Testing|Config")
	float GrowthAccelerationFactor = 100.0f; // 100x speed for testing

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Testing|Config")
	float GrowthTestDuration = 10.0f; // seconds (simulates much longer with acceleration)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Testing|Config")
	float MinHealthThreshold = 0.7f; // Minimum acceptable crop health

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Testing|Config")
	FVector2D FarmPlotSize = FVector2D(500.0f, 500.0f); // 5m x 5m

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Testing|Config")
	int32 GridResolution = 8; // 8x8 grid

protected:
	// Test case implementations

	/** Test 1.1: Planting System - VR Seed Placement */
	void Test_VRSeedPlacement();

	/** Test 1.2: Planting System - Soil Interaction */
	void Test_SoilInteraction();

	/** Test 2.1: Growth System - Basic Growth Progression */
	void Test_BasicGrowthProgression();

	/** Test 2.2: Growth System - Growth Stages (0-100%) */
	void Test_GrowthStages();

	/** Test 2.3: Growth System - Growth Rate Calculation */
	void Test_GrowthRateCalculation();

	/** Test 3.1: Watering System - VR Watering Can */
	void Test_VRWateringCan();

	/** Test 3.2: Watering System - Moisture Levels */
	void Test_MoistureLevels();

	/** Test 3.3: Watering System - Water Effect on Growth */
	void Test_WaterEffectOnGrowth();

	/** Test 4.1: Harvesting System - VR Harvest Interaction */
	void Test_VRHarvestInteraction();

	/** Test 4.2: Harvesting System - Yield Calculation */
	void Test_YieldCalculation();

	/** Test 4.3: Harvesting System - Harvest Timing */
	void Test_HarvestTiming();

	/** Test 5.1: Crop Health - Health System */
	void Test_HealthSystem();

	/** Test 5.2: Crop Health - Environmental Factors */
	void Test_EnvironmentalFactors();

	/** Test 5.3: Crop Health - Health Recovery */
	void Test_HealthRecovery();

	/** Test 6.1: Biome Compatibility - Crop Biome Matching */
	void Test_BiomeCompatibility();

	/** Test 6.2: Biome Compatibility - Temperature Effects */
	void Test_TemperatureEffects();

	/** Test 7.1: Soil Quality - Soil System Integration */
	void Test_SoilSystemIntegration();

	/** Test 7.2: Soil Quality - Nutrient Depletion */
	void Test_NutrientDepletion();

	/** Test 7.3: Soil Quality - Fertilization */
	void Test_Fertilization();

	/** Test 8.1: Accelerated Growth - Fast-Forward Testing */
	void Test_AcceleratedGrowth();

	/** Test 8.2: Accelerated Growth - Full Lifecycle */
	void Test_FullLifecycle();

	/** Test 9.1: Multiple Crop Types - Wheat */
	void Test_WheatCrop();

	/** Test 9.2: Multiple Crop Types - Corn */
	void Test_CornCrop();

	/** Test 9.3: Multiple Crop Types - Tomatoes */
	void Test_TomatoesCrop();

	/** Test 9.4: Multiple Crop Types - All Crop Types */
	void Test_AllCropTypes();

	/** Test 10.1: VR Interaction - Plant Gesture */
	void Test_PlantingGesture();

	/** Test 10.2: VR Interaction - Watering Gesture */
	void Test_WateringGesture();

	/** Test 10.3: VR Interaction - Harvesting Gesture */
	void Test_HarvestingGesture();

	// Helper functions

	/**
	 * Create a test farm plot
	 */
	bool CreateTestFarmPlot();

	/**
	 * Cleanup test farm plot
	 */
	void CleanupTestFarmPlot();

	/**
	 * Plant a crop at a specific grid position
	 */
	bool PlantCropAtPosition(ECropTypeExtended CropType, FIntPoint GridPosition);

	/**
	 * Get crop definition for a crop type
	 */
	UCropDefinition* GetCropDefinition(ECropTypeExtended CropType);

	/**
	 * Accelerate growth by time factor
	 */
	void AccelerateGrowth(float SimulatedSeconds);

	/**
	 * Water crops in the test plot
	 */
	void WaterCrops(float WaterAmount = 1.0f);

	/**
	 * Harvest crop at grid position
	 */
	bool HarvestCropAtPosition(FIntPoint GridPosition, int32& OutYield, float& OutQuality);

	/**
	 * Get crop growth progress at position
	 */
	float GetCropGrowthProgress(FIntPoint GridPosition);

	/**
	 * Get crop health at position
	 */
	float GetCropHealth(FIntPoint GridPosition);

	/**
	 * Check if crop is ready for harvest
	 */
	bool IsCropReadyForHarvest(FIntPoint GridPosition);

	/**
	 * Set soil quality
	 */
	void SetSoilQuality(float Quality);

	/**
	 * Set environmental temperature
	 */
	void SetEnvironmentTemperature(float Temperature);

	/**
	 * Apply fertilizer to plot
	 */
	void ApplyFertilizer(float Amount);

	/**
	 * Measure growth rate over time period
	 */
	float MeasureGrowthRate(FIntPoint GridPosition, float Duration);

	/**
	 * Verify VR interaction manager is available
	 */
	bool IsVRInteractionAvailable();

private:
	// References
	UPROPERTY()
	AFarmPlot* TestFarmPlot = nullptr;

	UPROPERTY()
	UFarmingSubsystem* FarmingSubsystem = nullptr;

	UPROPERTY()
	UVRPlantingInteractionManager* PlantingManager = nullptr;

	UPROPERTY()
	UVRHarvestInteractionManager* HarvestManager = nullptr;

	// Test state
	TMap<ECropTypeExtended, UCropDefinition*> CropDefinitionCache;
	FIntPoint LastPlantedPosition = FIntPoint(-1, -1);
	float InitialGrowthProgress = 0.0f;
	float TestStartTime = 0.0f;
};
