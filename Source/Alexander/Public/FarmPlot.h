// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CropGrowthSystem.h"
#include "FarmPlot.generated.h"

// Forward declarations
class UCropDefinition;
class UInstancedStaticMeshComponent;
class UProceduralMeshComponent;
struct FGrowthCalculationResult;
struct FSoilVariation;

/**
 * Individual crop cell in the farm grid
 */
USTRUCT(BlueprintType)
struct FCropCell
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Crop")
	UCropDefinition* CropType;

	UPROPERTY(BlueprintReadWrite, Category = "Crop")
	float GrowthProgress; // 0-1

	UPROPERTY(BlueprintReadWrite, Category = "Crop")
	float Health; // 0-1

	UPROPERTY(BlueprintReadWrite, Category = "Crop")
	bool bNeedsWater;

	UPROPERTY(BlueprintReadWrite, Category = "Crop")
	bool bHasPests;

	UPROPERTY(BlueprintReadWrite, Category = "Crop")
	FDateTime PlantedTime;

	UPROPERTY(BlueprintReadWrite, Category = "Crop")
	int32 CurrentGrowthStage; // Current stage index

	UPROPERTY(BlueprintReadWrite, Category = "Crop")
	float LastGrowthProgress; // For detecting stage changes

	UPROPERTY(BlueprintReadWrite, Category = "Crop")
	float TimeSinceLastWater; // Seconds since last watering

	UPROPERTY(BlueprintReadWrite, Category = "Crop")
	FDateTime LastWateredTime; // When crop was last watered

	FCropCell()
		: CropType(nullptr)
		, GrowthProgress(0.0f)
		, Health(1.0f)
		, bNeedsWater(false)
		, bHasPests(false)
		, CurrentGrowthStage(0)
		, LastGrowthProgress(0.0f)
		, TimeSinceLastWater(0.0f)
	{
	}
};

/**
 * Farm environment conditions
 */
USTRUCT(BlueprintType)
struct FFarmEnvironment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Environment")
	float Temperature; // Celsius

	UPROPERTY(BlueprintReadWrite, Category = "Environment")
	float Humidity; // 0-1

	UPROPERTY(BlueprintReadWrite, Category = "Environment")
	float Rainfall; // mm per hour

	UPROPERTY(BlueprintReadWrite, Category = "Environment")
	float Sunlight; // 0-1

	FFarmEnvironment()
		: Temperature(20.0f)
		, Humidity(0.6f)
		, Rainfall(0.0f)
		, Sunlight(1.0f)
	{
	}
};

/**
 * Harvest result information
 */
USTRUCT(BlueprintType)
struct FHarvestResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	bool bSuccess;

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	int32 YieldAmount;

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	int32 Quantity; // Added for interface compatibility

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	FString ItemName;

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	float Quality; // 0-1

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	ECropTypeExtended CropType; // Added for interface compatibility

	FHarvestResult()
		: bSuccess(false)
		, YieldAmount(0)
		, Quantity(0)
		, Quality(0.0f)
		, CropType(ECropTypeExtended::Wheat)
	{
	}
};

/**
 * Harvest record for historical tracking (Task 17.4)
 */
USTRUCT(BlueprintType)
struct FHarvestRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	FDateTime HarvestTime;

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	FString CropName;

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	UCropDefinition* CropType;

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	int32 YieldAmount;

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	float Quality; // 0-1

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	float EconomicValue; // Calculated sell price

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	FIntPoint GridPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	float GrowthDuration; // Actual time from plant to harvest (seconds)

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	float AverageSoilQuality; // Soil quality during growth

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	float AverageFertility; // Fertility during growth

	FHarvestRecord()
		: CropType(nullptr)
		, YieldAmount(0)
		, Quality(0.0f)
		, EconomicValue(0.0f)
		, GridPosition(FIntPoint::ZeroValue)
		, GrowthDuration(0.0f)
		, AverageSoilQuality(0.0f)
		, AverageFertility(0.0f)
	{
	}
};

/**
 * Harvest statistics for a specific crop type (Task 17.4)
 */
USTRUCT(BlueprintType)
struct FHarvestStatistics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	FString CropName;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	UCropDefinition* CropType;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 TotalHarvests;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 TotalYield;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float AverageYield;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float AverageQuality;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float TotalEconomicValue;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float AverageEconomicValue;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float AverageGrowthDuration; // Average time from plant to harvest

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float BestQuality;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 BestYield;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	FDateTime FirstHarvest;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	FDateTime LastHarvest;

	FHarvestStatistics()
		: CropType(nullptr)
		, TotalHarvests(0)
		, TotalYield(0)
		, AverageYield(0.0f)
		, AverageQuality(0.0f)
		, TotalEconomicValue(0.0f)
		, AverageEconomicValue(0.0f)
		, AverageGrowthDuration(0.0f)
		, BestQuality(0.0f)
		, BestYield(0)
	{
	}
};

/**
 * Farm plot actor - represents a farmable area on a planet surface
 */
UCLASS()
class ALEXANDER_API AFarmPlot : public AActor
{
	GENERATED_BODY()

public:
	AFarmPlot();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ============================================================================
	// COMPONENTS
	// ============================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInstancedStaticMeshComponent* CropInstances;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProceduralMeshComponent* SoilMesh;

	// ============================================================================
	// PLOT PROPERTIES
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm")
	FGuid PlotID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm")
	FVector2D PlotSize; // meters (X, Y)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm")
	int32 GridResolution; // cells per side

	UPROPERTY(BlueprintReadOnly, Category = "Farm")
	TArray<FCropCell> CropGrid;

	// ============================================================================
	// SOIL SYSTEM
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float SoilQuality; // 0-1, base quality from biome

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float SoilNutrients; // 0-1, overall nutrient level

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float WaterLevel; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float Fertility; // 0-1, depletes with use

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float NitrogenLevel; // 0-1, depletes with crop growth

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float PhosphorusLevel; // 0-1, depletes with crop growth

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float PotassiumLevel; // 0-1, depletes with crop growth

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float OrganicMatter; // 0-1, affects fertility

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float SoilPH; // 4.0-9.0, affects nutrient availability

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float Compaction; // 0-1, affects root growth

	// Biome soil properties (Task 14.2)
	UPROPERTY(BlueprintReadOnly, Category = "Soil")
	FLinearColor BiomeSoilColor; // Base color from biome

	UPROPERTY(BlueprintReadOnly, Category = "Soil")
	float SoilRoughness; // Visual texture roughness

	UPROPERTY(BlueprintReadOnly, Category = "Soil")
	float SoilDrainage; // Water drainage rate

	// ============================================================================
	// PLANET REFERENCE
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Farm")
	AActor* PlanetActor;

	// ============================================================================
	// WEATHER INTEGRATION PROPERTIES
	// ============================================================================

	// Reference to planet's weather component
	UPROPERTY()
	class UPlanetWeatherComponent* WeatherComponent;

	// Accumulated rain water (mm)
	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	float AccumulatedRainfall;

	// Time since last storm damage check
	UPROPERTY()
	float TimeSinceLastStormCheck;

	// Storm damage check interval (seconds)
	UPROPERTY(EditAnywhere, Category = "Weather")
	float StormCheckInterval;

	// Temperature damage threshold (degrees outside optimal)
	UPROPERTY(EditAnywhere, Category = "Weather")
	float TemperatureDamageThreshold;

	// Temperature damage rate per second
	UPROPERTY(EditAnywhere, Category = "Weather")
	float TemperatureDamageRate;

	// ============================================================================
	// HARVEST TRACKING (Task 17.4)
	// ============================================================================

	// Historical harvest records
	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	TArray<FHarvestRecord> HarvestHistory;

	// Maximum number of harvest records to keep
	UPROPERTY(EditAnywhere, Category = "Harvest")
	int32 MaxHarvestRecords;

	// Current crop being grown (for single crop plots)
	UPROPERTY(BlueprintReadOnly, Category = "Farming")
	UCropDefinition* CurrentCrop;

	// Statistics tracking
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 CropsPlanted;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 CropsHarvested;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float CurrentYield;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float WaterUsed;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float FertilizerUsed;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float SoilMoisture;

	// Track soil quality over time for averaging
	UPROPERTY()
	float AccumulatedSoilQuality;

	// Track fertility over time for averaging
	UPROPERTY()
	float AccumulatedFertility;

	// Number of samples for averaging
	UPROPERTY()
	int32 QualitySampleCount;

	// ============================================================================
	// WEED MANAGEMENT
	// ============================================================================

	// Weed growth level (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weeds")
	float WeedGrowth;

	// Is the plot ready for harvest
	UPROPERTY(BlueprintReadOnly, Category = "Farming")
	bool bIsHarvestable;

	// ============================================================================
	// FARMING OPERATIONS
	// ============================================================================

	/**
	 * Plant a crop at the specified grid position
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	bool PlantCrop(FIntPoint GridPosition, UCropDefinition* CropType);

	/**
	 * Harvest crop at the specified grid position
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	FHarvestResult HarvestCrop(FIntPoint GridPosition);

	/**
	 * Harvest all crops in the plot
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	FHarvestResult HarvestAllCrops();

	/**
	 * Water the entire plot
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	bool WaterPlot(float WaterAmount);

	/**
	 * Fertilize the plot to restore fertility
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	bool FertilizePlot(float FertilizerAmount, EFertilizerType FertilizerType = EFertilizerType::Basic);

	/**
	 * Apply specific fertilizer type (NPK)
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void ApplyFertilizer(float Nitrogen, float Phosphorus, float Potassium);

	/**
	 * Add organic matter to improve soil structure
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void AddOrganicMatter(float Amount);

	/**
	 * Calculate overall soil quality based on all factors
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	float CalculateSoilQuality() const;

	/**
	 * Calculate water needed for optimal growth
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	float CalculateWaterNeeded() const;

	/**
	 * Calculate fertilizer needed for optimal growth
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	float CalculateFertilizerNeeded() const;

	/**
	 * Get soil nutrient levels as a vector (N, P, K)
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	FVector GetNutrientLevels() const;

	/**
	 * Update crop growth for all planted crops
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void UpdateCropGrowth(float DeltaTime);

	/**
	 * Get current environmental conditions
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	FFarmEnvironment GetEnvironmentalConditions() const;

	/**
	 * Check if plot has weeds
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	bool HasWeeds() const { return WeedGrowth > 0.5f; }

	/**
	 * Remove weeds from the plot
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void RemoveWeeds() { WeedGrowth = 0.0f; }

	/**
	 * Get expected yield for the plot
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	int32 GetExpectedYield() const;

	// ============================================================================
	// WEATHER INTEGRATION (Task 13.4)
	// ============================================================================

	/**
	 * Apply rain watering to all crops
	 * @param PrecipitationRate Rain rate in mm per hour
	 * @param DeltaTime Time elapsed in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Weather")
	void ApplyRainWatering(float PrecipitationRate, float DeltaTime);

	/**
	 * Apply storm damage to crops
	 * @param StormIntensity Storm intensity (0-1)
	 * @param DeltaTime Time elapsed in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Weather")
	void ApplyStormDamage(float StormIntensity, float DeltaTime);

	/**
	 * Apply temperature effects to crops
	 * @param Temperature Current temperature in Celsius
	 * @param DeltaTime Time elapsed in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Weather")
	void ApplyTemperatureEffects(float Temperature, float DeltaTime);

	/**
	 * Get seasonal growth modifier based on time of year
	 * @param DayOfYear Current day of year (0-365)
	 * @return Seasonal modifier (0.5-1.5)
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Weather")
	float GetSeasonalGrowthModifier(int32 DayOfYear) const;

	/**
	 * Update weather-based effects on crops
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Weather")
	void UpdateWeatherEffects(float DeltaTime);

	/**
	 * Get detailed growth calculation for a specific crop cell
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	FGrowthCalculationResult GetCropGrowthDetails(FIntPoint GridPosition) const;

	/**
	 * Get estimated time until crop is ready for harvest
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	float GetTimeToHarvest(FIntPoint GridPosition) const;

	/**
	 * Get current growth stage for a crop
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	int32 GetCropGrowthStage(FIntPoint GridPosition) const;

	/**
	 * Get growth stage name for a crop
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	FString GetCropGrowthStageName(FIntPoint GridPosition) const;

	/**
	 * Get progress within current growth stage (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	float GetProgressWithinGrowthStage(FIntPoint GridPosition) const;

	// ============================================================================
	// HARVEST TRACKING (Task 17.4)
	// ============================================================================

	/**
	 * Record a harvest in the historical data
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Harvest")
	void RecordHarvest(const FHarvestResult& HarvestResult, FIntPoint GridPosition, const FCropCell& CropCell);

	/**
	 * Get all harvest records
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Harvest")
	TArray<FHarvestRecord> GetHarvestHistory() const;

	/**
	 * Get harvest records for a specific crop type
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Harvest")
	TArray<FHarvestRecord> GetHarvestHistoryForCrop(UCropDefinition* CropType) const;

	/**
	 * Get harvest statistics for a specific crop type
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Harvest")
	FHarvestStatistics GetHarvestStatistics(UCropDefinition* CropType) const;

	/**
	 * Get harvest statistics for all crops
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Harvest")
	TArray<FHarvestStatistics> GetAllHarvestStatistics() const;

	/**
	 * Get total economic value of all harvests
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Harvest")
	float GetTotalEconomicValue() const;

	/**
	 * Get economic value for a specific crop type
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Harvest")
	float GetEconomicValueForCrop(UCropDefinition* CropType) const;

	/**
	 * Clear harvest history (keep only recent N records)
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Harvest")
	void TrimHarvestHistory(int32 MaxRecords = 100);

	/**
	 * Calculate economic value for a harvest
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Harvest")
	float CalculateEconomicValue(UCropDefinition* CropType, int32 YieldAmount, float Quality) const;

	// ============================================================================
	// SETUP FUNCTIONS
	// ============================================================================

	void SetPlotSize(FVector2D Size);
	void SetPlanetActor(AActor* Planet);
	void SetInitialSoilQuality(float Quality);

	/**
	 * Apply biome soil variation to this farm plot (Task 14.2)
	 * @param SoilVar Soil variation data from biome
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Soil")
	void ApplyBiomeSoilVariation(const struct FSoilVariation& SoilVar);

	AActor* GetPlanetActor() const { return PlanetActor; }

	// Check if grid position is valid
	bool IsValidGridPosition(FIntPoint GridPosition) const;

protected:
	// Initialize the crop grid
	void InitializeCropGrid();

	// Update visual representation of crops
	void UpdateCropVisuals();

	// Update soil mesh appearance
	void UpdateSoilVisuals();

	// Get grid cell index from grid position
	int32 GetCellIndex(FIntPoint GridPosition) const;

	// Calculate growth rate modifier based on environment
	float CalculateGrowthRateModifier(const FCropCell& Cell, const FFarmEnvironment& Environment) const;

	// Deplete soil nutrients based on crop consumption
	void DepleteSoilNutrients(const FCropCell& Cell, float DeltaTime);

	// Natural soil regeneration over time
	void RegenerateSoil(float DeltaTime);

	// Calculate soil color based on quality and nutrients
	FLinearColor CalculateSoilColor() const;

	// Update growth stage for a crop cell
	void UpdateCropGrowthStage(FCropCell& Cell);

	// Handle stage transition event
	void OnCropStageChanged(FCropCell& Cell, int32 OldStage, int32 NewStage);
};
