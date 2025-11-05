// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FarmPlot.h"
#include "CropSystem.generated.h"

UENUM(BlueprintType)
enum class ECropType : uint8
{
	Wheat UMETA(DisplayName = "Wheat"),
	Corn UMETA(DisplayName = "Corn"),
	Tomatoes UMETA(DisplayName = "Tomatoes"),
	Potatoes UMETA(DisplayName = "Potatoes"),
	Carrots UMETA(DisplayName = "Carrots"),
	Lettuce UMETA(DisplayName = "Lettuce"),
	Strawberries UMETA(DisplayName = "Strawberries"),
	SpaceWeed UMETA(DisplayName = "Space Weed"),
	MoonMelons UMETA(DisplayName = "Moon Melons"),
	QuantumPotatoes UMETA(DisplayName = "Quantum Potatoes")
};

UENUM(BlueprintType)
enum class ESoilQuality : uint8
{
	Pristine UMETA(DisplayName = "Pristine"),
	Excellent UMETA(DisplayName = "Excellent"),
	Good UMETA(DisplayName = "Good"),
	Fair UMETA(DisplayName = "Fair"),
	Poor UMETA(DisplayName = "Poor")
};

/**
 * Crop data structure
 */
USTRUCT(BlueprintType)
struct FCropData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	ECropType CropType = ECropType::Wheat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FString CropName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float BaseGrowthTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float WaterNeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float NutrientNeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float OptimalTemp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float TempTolerance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 BaseYield = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float MarketValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	UTexture2D* CropIcon = nullptr;
};

/**
 * Crop System - Manages crop types, growth patterns, and crop data
 */
UCLASS(BlueprintType, Blueprintable)
class ALEXANDER_API UCropSystem : public UObject
{
	GENERATED_BODY()

public:
	UCropSystem();

	// Crop database
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop Database")
	TArray<FCropData> CropDatabase;

	// Crop operations
	UFUNCTION(BlueprintCallable, Category = "Crop System")
	FCropData GetCropData(ECropType CropType) const;

	UFUNCTION(BlueprintCallable, Category = "Crop System")
	TArray<ECropType> GetAvailableCrops() const;

	UFUNCTION(BlueprintCallable, Category = "Crop System")
	TArray<ECropType> GetCropsForClimate(float Temperature, float Humidity) const;

	UFUNCTION(BlueprintCallable, Category = "Crop System")
	float GetCropMarketValue(ECropType CropType) const;

	UFUNCTION(BlueprintCallable, Category = "Crop System")
	bool IsCropInSeason(ECropType CropType, float SeasonFactor) const;

	// Crop breeding
	UFUNCTION(BlueprintCallable, Category = "Crop System")
	ECropType BreedCrops(ECropType Crop1, ECropType Crop2);

	UFUNCTION(BlueprintCallable, Category = "Crop System")
	bool CanBreedCrops(ECropType Crop1, ECropType Crop2) const;

	// Crop quality assessment
	UFUNCTION(BlueprintCallable, Category = "Crop System")
	float CalculateCropQuality(ECropType CropType, ESoilQuality Soil, float GrowthTime) const;

	UFUNCTION(BlueprintCallable, Category = "Crop System")
	FString GetQualityDescription(float Quality) const;

protected:
	void InitializeCropDatabase();

private:
	TMap<ECropType, FCropData> CropDataMap;
};