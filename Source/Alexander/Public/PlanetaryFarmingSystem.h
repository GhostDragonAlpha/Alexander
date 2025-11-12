// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CropSystem.h"
#include "LandClaimManager.h"
#include "Containers/Map.h"
#include "Containers/Array.h"
#include "Engine/EngineTypes.h"
#include "PlanetaryFarmingSystem.generated.h"

// Forward declarations
class AFarmPlot;
class AHydroponicsBay;
class AGreenhouse;
class AIrrigationSystem;
class AFertilizerInjector;
class AHarvestingDrone;
class APlanet;
class UCropGrowthStageManager;

// Farm plot status
UENUM(BlueprintType)
enum class EFarmPlotStatus : uint8
{
	Empty UMETA(DisplayName = "Empty"),
	Planted UMETA(DisplayName = "Planted"),
	Growing UMETA(DisplayName = "Growing"),
	ReadyToHarvest UMETA(DisplayName = "Ready to Harvest"),
	Withered UMETA(DisplayName = "Withered"),
	Fallow UMETA(DisplayName = "Fallow")
};

// Farming infrastructure type
UENUM(BlueprintType)
enum class EFarmingInfrastructureType : uint8
{
	HydroponicsBay UMETA(DisplayName = "Hydroponics Bay"),
	Greenhouse UMETA(DisplayName = "Greenhouse"),
	IrrigationSystem UMETA(DisplayName = "Irrigation System"),
	FertilizerInjector UMETA(DisplayName = "Fertilizer Injector"),
	HarvestingDrone UMETA(DisplayName = "Harvesting Drone"),
	ProcessingFacility UMETA(DisplayName = "Processing Facility"),
	StorageSilo UMETA(DisplayName = "Storage Silo")
};

// Crop growth stage (planetary-specific)
UENUM(BlueprintType)
enum class EPlanetaryCropGrowthStage : uint8
{
	Seed UMETA(DisplayName = "Seed"),
	Sprout UMETA(DisplayName = "Sprout"),
	Vegetative UMETA(DisplayName = "Vegetative"),
	Flowering UMETA(DisplayName = "Flowering"),
	Fruiting UMETA(DisplayName = "Fruiting"),
	Mature UMETA(DisplayName = "Mature")
};

// Farm statistics
USTRUCT(BlueprintType)
struct FFarmStatistics
{
	GENERATED_BODY()

	// Total number of farm plots
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 TotalPlots;

	// Number of active plots (with crops)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 ActivePlots;

	// Number of plots ready to harvest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 ReadyToHarvest;

	// Total crops planted
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 TotalCropsPlanted;

	// Total crops harvested
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 TotalCropsHarvested;

	// Current crop yield (units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 CurrentYield;

	// Total farm value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	float TotalFarmValue;

	// Daily revenue
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	float DailyRevenue;

	// Daily expenses
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	float DailyExpenses;

	// Net profit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	float NetProfit;

	// Crop diversity (number of different crop types)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 CropDiversity;

	// Average growth time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	float AverageGrowthTime;

	// Water usage (liters per day)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	float WaterUsage;

	// Fertilizer usage (kg per day)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	float FertilizerUsage;

	FFarmStatistics()
	{
		TotalPlots = 0;
		ActivePlots = 0;
		ReadyToHarvest = 0;
		TotalCropsPlanted = 0;
		TotalCropsHarvested = 0;
		CurrentYield = 0;
		TotalFarmValue = 0.0f;
		DailyRevenue = 0.0f;
		DailyExpenses = 0.0f;
		NetProfit = 0.0f;
		CropDiversity = 0;
		AverageGrowthTime = 0.0f;
		WaterUsage = 0.0f;
		FertilizerUsage = 0.0f;
	}
};

// Crop planting info
USTRUCT(BlueprintType)
struct FCropPlantingInfo
{
	GENERATED_BODY()

	// Crop type to plant
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planting")
	ECropType CropType;

	// Plot location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planting")
	FVector PlotLocation;

	// Planting density (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planting")
	float PlantingDensity;

	// Expected harvest time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planting")
	float ExpectedHarvestTime;

	// Expected yield
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planting")
	int32 ExpectedYield;

	FCropPlantingInfo()
	{
		CropType = ECropType::Wheat;
		PlantingDensity = 1.0f;
		ExpectedHarvestTime = 0.0f;
		ExpectedYield = 0;
	}
};

// Harvest result
USTRUCT(BlueprintType)
struct FAlexanderHarvestResult
{
	GENERATED_BODY()

	// Was harvest successful?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	bool bSuccess;

	// Crop type harvested
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	ECropType CropType;

	// Quantity harvested
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	int32 Quantity;

	// Quality rating (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	float Quality;

	// Market value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	float MarketValue;

	// Harvest location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	FVector Location;

	FAlexanderHarvestResult()
	{
		bSuccess = false;
		CropType = ECropType::Wheat;
		Quantity = 0;
		Quality = 0.0f;
		MarketValue = 0.0f;
	}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCropPlanted, const FCropPlantingInfo&, PlantingInfo, const FGuid&, PlotID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCropHarvested, const FHarvestResult&, HarvestResult, const FGuid&, PlotID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFarmPlotCreated, const FGuid&, PlotID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFarmPlotDestroyed, const FGuid&, PlotID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInfrastructureBuilt, EFarmingInfrastructureType, Type, const FVector&, Location);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFarmStatisticsUpdated, const FFarmStatistics&, Statistics);

/**
 * Planetary Farming System - Core manager for planetary farming operations
 */
UCLASS()
class ALEXANDER_API APlanetaryFarmingSystem : public AActor
{
	GENERATED_BODY()

public:
	APlanetaryFarmingSystem();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Initialize the farming system for a planet
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	void InitializeForPlanet(APlanet* Planet, ALandClaimManager* LandManager);

	// Create a new farm plot
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	FGuid CreateFarmPlot(FVector Location, const FGuid& ClaimID, FVector2D Size = FVector2D(100.0f, 100.0f));

	// Plant crops in a plot
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	bool PlantCrops(const FGuid& PlotID, ECropType CropType, float PlantingDensity = 1.0f);

	// Harvest crops from a plot
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	FAlexanderHarvestResult HarvestCrops(const FGuid& PlotID);

	// Water a farm plot
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	bool WaterPlot(const FGuid& PlotID, float WaterAmount);

	// Fertilize a farm plot
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	bool FertilizePlot(const FGuid& PlotID, float FertilizerAmount, FName FertilizerType);

	// Build farming infrastructure
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	FGuid BuildInfrastructure(EFarmingInfrastructureType Type, FVector Location, const FGuid& ClaimID);

	// Get farm plot by ID
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	AFarmPlot* GetFarmPlot(const FGuid& PlotID) const;

	// Get all farm plots
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	TArray<AFarmPlot*> GetAllFarmPlots() const;

	// Get plots ready for harvest
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	TArray<FGuid> GetPlotsReadyForHarvest() const;

	// Get farm statistics
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	FFarmStatistics GetFarmStatistics() const;

	// Get crop system
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	UCropSystem* GetCropSystem() const { return CropSystem; }

	// Get all infrastructure of a type
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	TArray<AActor*> GetInfrastructureByType(EFarmingInfrastructureType Type) const;

	// Find best crop for location
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	ECropType FindBestCropForLocation(FVector Location) const;

	// Get crop growth progress
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	float GetCropGrowthProgress(const FGuid& PlotID) const;

	// Get crop growth stage
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	EPlanetaryCropGrowthStage GetCropGrowthStage(const FGuid& PlotID) const;

	// Auto-manage farming (AI helper)
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	void EnableAutoManagement(bool bEnable);

	// Process all farm plots
	UFUNCTION(BlueprintCallable, Category = "Farming System")
	void ProcessAllPlots(float DeltaTime);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Farming Events")
	FOnCropPlanted OnCropPlanted;

	UPROPERTY(BlueprintAssignable, Category = "Farming Events")
	FOnCropHarvested OnCropHarvested;

	UPROPERTY(BlueprintAssignable, Category = "Farming Events")
	FOnFarmPlotCreated OnFarmPlotCreated;

	UPROPERTY(BlueprintAssignable, Category = "Farming Events")
	FOnFarmPlotDestroyed OnFarmPlotDestroyed;

	UPROPERTY(BlueprintAssignable, Category = "Farming Events")
	FOnInfrastructureBuilt OnInfrastructureBuilt;

	UPROPERTY(BlueprintAssignable, Category = "Farming Events")
	FOnFarmStatisticsUpdated OnFarmStatisticsUpdated;

protected:
	// Planet this system manages
	UPROPERTY()
	TWeakObjectPtr<APlanet> TargetPlanet;

	// Land claim manager
	UPROPERTY()
	TWeakObjectPtr<ALandClaimManager> LandClaimManager;

	// Crop system for crop data
	UPROPERTY()
	UCropSystem* CropSystem;

	// All farm plots
	UPROPERTY()
	TArray<AFarmPlot*> FarmPlots;

	// Farm plot lookup by ID
	UPROPERTY()
	TMap<FGuid, int32> FarmPlotIndexMap;

	// Farming infrastructure
	UPROPERTY()
	TArray<AActor*> FarmingInfrastructure;

	// Infrastructure by type
	UPROPERTY()
	TMap<EFarmingInfrastructureType, TArray<AActor*>> InfrastructureByType;

	// Auto-management enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bAutoManagementEnabled;

	// Maximum number of farm plots
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 MaxFarmPlots;

	// Water cost per liter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	float WaterCostPerLiter;

	// Fertilizer cost per kg
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	float FertilizerCostPerKg;

	// Labor cost per hour
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	float LaborCostPerHour;

private:
	// Initialize crop system
	void InitializeCropSystem();

	// Update farm statistics
	void UpdateFarmStatistics();

	// Process auto-management
	void ProcessAutoManagement();

	// Find optimal crop for location
	ECropType FindOptimalCrop(FVector Location) const;

	// Calculate expected yield
	int32 CalculateExpectedYield(ECropType CropType, float SoilQuality, float PlantingDensity) const;

	// Get plot index by ID
	int32 GetPlotIndex(const FGuid& PlotID) const;
};