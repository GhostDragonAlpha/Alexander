// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CropTypes.h"
#include "CropGrowthSystem.generated.h"

// Fertilizer types
UENUM(BlueprintType)
enum class EFertilizerType : uint8
{
	None UMETA(DisplayName = "No Fertilizer"),
	Basic UMETA(DisplayName = "Basic Fertilizer"),
	Premium UMETA(DisplayName = "Premium Fertilizer"),
	Specialized UMETA(DisplayName = "Specialized Fertilizer"),
	Organic UMETA(DisplayName = "Organic Fertilizer"),
	Synthetic UMETA(DisplayName = "Synthetic Fertilizer")
};

// Pest types
UENUM(BlueprintType)
enum class EPestType : uint8
{
	None UMETA(DisplayName = "None"),
	Aphids UMETA(DisplayName = "Aphids"),
	SpiderMites UMETA(DisplayName = "Spider Mites"),
	Caterpillars UMETA(DisplayName = "Caterpillars"),
	RootRot UMETA(DisplayName = "Root Rot"),
	Mildew UMETA(DisplayName = "Mildew"),
	Rodents UMETA(DisplayName = "Rodents")
};

// Disease types
UENUM(BlueprintType)
enum class EDiseaseType : uint8
{
	None UMETA(DisplayName = "None"),
	Blight UMETA(DisplayName = "Blight"),
	Rust UMETA(DisplayName = "Rust"),
	Wilt UMETA(DisplayName = "Wilt"),
	Mold UMETA(DisplayName = "Mold"),
	Virus UMETA(DisplayName = "Virus")
};

// Crop growth stages
UENUM(BlueprintType)
enum class ECropGrowthStage : uint8
{
	Seed		UMETA(DisplayName = "Seed"),
	Sprout		UMETA(DisplayName = "Sprout"),
	Vegetative	UMETA(DisplayName = "Vegetative"),
	Flowering	UMETA(DisplayName = "Flowering"),
	Fruiting	UMETA(DisplayName = "Fruiting"),
	Growing		UMETA(DisplayName = "Growing"),
	Mature		UMETA(DisplayName = "Mature"),
	Harvestable	UMETA(DisplayName = "Harvestable")
};

// Crop growth data with extended information
USTRUCT(BlueprintType)
struct FCropGrowthData
{
	GENERATED_BODY()

	// Crop type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	ECropTypeExtended CropType;

	// Crop name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FString CropName;

	// Base growth time in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
	float BaseGrowthTime;

	// Water needs (0-1 scale)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
	float WaterNeed;

	// Nutrient needs (0-1 scale)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
	float NutrientNeed;

	// Optimal temperature (Celsius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
	float OptimalTemp;

	// Temperature tolerance (+- degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
	float TempTolerance;

	// Optimal humidity (0-1 scale)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
	float OptimalHumidity;

	// Light requirements (0-1 scale)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
	float LightRequirement;

	// Base yield per plot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Yield")
	int32 BaseYield;

	// Market value per unit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Yield")
	float MarketValue;

	// Crop category
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FString Category; // Food, Medical, Industrial, Illegal

	// Growth difficulty (0-1, higher is harder)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
	float GrowthDifficulty;

	// Pest resistance (0-1, higher is more resistant)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pest Resistance")
	float PestResistance;

	// Disease resistance (0-1, higher is more resistant)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Disease Resistance")
	float DiseaseResistance;

	// Season preferences (0-1, where 0=winter, 1=summer)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Season")
	float PreferredSeason;

	// Soil quality requirement (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float SoilQualityRequirement;

	// Fertilizer type preference
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	EFertilizerType PreferredFertilizer;

	// Growth stage durations (percentage of total growth time)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth Stages")
	float SeedStageDuration;    // 0-0.15
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth Stages")
	float SproutStageDuration;  // 0.15-0.30
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth Stages")
	float GrowingStageDuration; // 0.30-0.70
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth Stages")
	float MatureStageDuration;  // 0.70-0.90
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth Stages")
	float HarvestableStageDuration; // 0.90-1.00

	// Icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UTexture2D* CropIcon;

	FCropGrowthData()
	{
		CropType = ECropTypeExtended::Wheat;
		BaseGrowthTime = 60.0f;
		WaterNeed = 0.5f;
		NutrientNeed = 0.4f;
		OptimalTemp = 20.0f;
		TempTolerance = 10.0f;
		OptimalHumidity = 0.6f;
		LightRequirement = 0.7f;
		BaseYield = 15;
		MarketValue = 10.0f;
		GrowthDifficulty = 0.5f;
		PestResistance = 0.7f;
		DiseaseResistance = 0.7f;
		PreferredSeason = 0.5f;
		SoilQualityRequirement = 0.3f;
		PreferredFertilizer = EFertilizerType::Basic;
		SeedStageDuration = 0.1f;
		SproutStageDuration = 0.15f;
		GrowingStageDuration = 0.5f;
		MatureStageDuration = 0.15f;
		HarvestableStageDuration = 0.1f;
	}
};

// Active crop instance
USTRUCT(BlueprintType)
struct FActiveCrop
{
	GENERATED_BODY()

	// Crop type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	ECropTypeExtended CropType;

	// Current growth progress (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
	float GrowthProgress;

	// Current growth stage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
	ECropGrowthStage CurrentStage;

	// Current health (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float Health;

	// Water level (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	float WaterLevel;

	// Nutrient level (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrients")
	float NutrientLevel;

	// Pest infestation level (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pests")
	float PestLevel;

	// Current pest type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pests")
	EPestType CurrentPest;

	// Disease level (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Disease")
	float DiseaseLevel;

	// Current disease type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Disease")
	EDiseaseType CurrentDisease;

	// Quality modifier (affected by care)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
	float QualityModifier;

	// Preferred fertilizer type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	EFertilizerType PreferredFertilizer;

	// Time planted
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	FDateTime PlantTime;

	// Expected harvest time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	FDateTime ExpectedHarvestTime;

	// Actual yield (calculated at harvest)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Yield")
	int32 ActualYield;

	FActiveCrop()
	{
		CropType = ECropTypeExtended::Wheat;
		GrowthProgress = 0.0f;
		CurrentStage = ECropGrowthStage::Seed;
		Health = 1.0f;
		WaterLevel = 0.5f;
		NutrientLevel = 0.5f;
		PestLevel = 0.0f;
		CurrentPest = EPestType::None;
		DiseaseLevel = 0.0f;
		CurrentDisease = EDiseaseType::None;
		QualityModifier = 1.0f;
		PreferredFertilizer = EFertilizerType::None;
		ActualYield = 0;
	}
};

// Crop growth system - manages detailed crop growth mechanics
UCLASS(BlueprintType, Blueprintable)
class ALEXANDER_API UCropGrowthSystem : public UObject
{
	GENERATED_BODY()

public:
	UCropGrowthSystem();

	// Crop database with 20 crop types
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop Database")
	TArray<FCropGrowthData> CropDatabase;

	// Get crop data
	UFUNCTION(BlueprintCallable, Category = "Crop System")
	FCropGrowthData GetCropGrowthData(ECropTypeExtended CropType) const;

	// Get crop market value
	UFUNCTION(BlueprintCallable, Category = "Crop System")
	float GetCropMarketValue(ECropTypeExtended CropType) const;

	// Get all available crops
	UFUNCTION(BlueprintCallable, Category = "Crop System")
	TArray<ECropTypeExtended> GetAllAvailableCrops() const;

	// Get crops by category
	UFUNCTION(BlueprintCallable, Category = "Crop System")
	TArray<ECropTypeExtended> GetCropsByCategory(const FString& Category) const;

	// Get crops suitable for environment
	UFUNCTION(BlueprintCallable, Category = "Crop System")
	TArray<ECropTypeExtended> GetCropsForEnvironment(float Temperature, float Humidity, float LightLevel) const;

	// Calculate growth rate based on conditions
	UFUNCTION(BlueprintCallable, Category = "Growth Calculation")
	float CalculateGrowthRate(ECropTypeExtended CropType, float Temperature, float Humidity, float SoilQuality, float LightLevel, EFertilizerType FertilizerType) const;

	// Calculate water needs
	UFUNCTION(BlueprintCallable, Category = "Growth Calculation")
	float CalculateWaterNeeds(ECropTypeExtended CropType, float Temperature, float Humidity, float GrowthStage) const;

	// Calculate nutrient needs
	UFUNCTION(BlueprintCallable, Category = "Growth Calculation")
	float CalculateNutrientNeeds(ECropTypeExtended CropType, float GrowthStage) const;

	// Update crop growth
	UFUNCTION(BlueprintCallable, Category = "Growth")
	void UpdateCropGrowth(FActiveCrop& Crop, float DeltaTime, float Temperature, float Humidity, float SoilQuality, float LightLevel);

	// Apply water to crop
	UFUNCTION(BlueprintCallable, Category = "Crop Care")
	void WaterCrop(FActiveCrop& Crop, float WaterAmount);

	// Apply fertilizer to crop
	UFUNCTION(BlueprintCallable, Category = "Crop Care")
	void FertilizeCrop(FActiveCrop& Crop, float FertilizerAmount, EFertilizerType FertilizerType);

	// Apply pesticide
	UFUNCTION(BlueprintCallable, Category = "Crop Care")
	void ApplyPesticide(FActiveCrop& Crop, float Effectiveness);

	// Apply fungicide
	UFUNCTION(BlueprintCallable, Category = "Crop Care")
	void ApplyFungicide(FActiveCrop& Crop, float Effectiveness);

	// Check for pest infestation
	UFUNCTION(BlueprintCallable, Category = "Pest Management")
	void CheckForPests(FActiveCrop& Crop, float DeltaTime, float Temperature, float Humidity);

	// Check for disease
	UFUNCTION(BlueprintCallable, Category = "Disease Management")
	void CheckForDisease(FActiveCrop& Crop, float DeltaTime, float Temperature, float Humidity);

	// Get growth stage from progress
	UFUNCTION(BlueprintCallable, Category = "Growth")
	ECropGrowthStage GetGrowthStageFromProgress(ECropTypeExtended CropType, float Progress) const;

	// Calculate final yield
	UFUNCTION(BlueprintCallable, Category = "Yield")
	int32 CalculateFinalYield(const FActiveCrop& Crop, const FCropGrowthData& CropData) const;

	// Calculate crop quality
	UFUNCTION(BlueprintCallable, Category = "Quality")
	float CalculateCropQuality(const FActiveCrop& Crop) const;

	// Get fertilizer effectiveness
	UFUNCTION(BlueprintCallable, Category = "Fertilizer")
	float GetFertilizerEffectiveness(EFertilizerType FertilizerType, ECropTypeExtended CropType) const;

	// Get pest damage rate
	UFUNCTION(BlueprintCallable, Category = "Pest")
	float GetPestDamageRate(EPestType PestType) const;

	// Get disease damage rate
	UFUNCTION(BlueprintCallable, Category = "Disease")
	float GetDiseaseDamageRate(EDiseaseType DiseaseType) const;

protected:
	void InitializeCropDatabase();

private:
	TMap<ECropTypeExtended, FCropGrowthData> CropGrowthDataMap;

	// Growth stage thresholds
	float GetStageThreshold(ECropGrowthStage Stage, const FCropGrowthData& CropData) const;

	// Environmental stress calculation
	float CalculateEnvironmentalStress(const FActiveCrop& Crop, float Temperature, float Humidity, float SoilQuality, float LightLevel) const;

	// Pest/disease probability calculation
	float CalculatePestProbability(float Temperature, float Humidity, float PestResistance) const;
	float CalculateDiseaseProbability(float Temperature, float Humidity, float DiseaseResistance) const;
};