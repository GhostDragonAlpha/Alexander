// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FarmPlot.h"
#include "CropTypes.h"
#include "CropSystem.generated.h"

// ECropType enum removed - use ECropTypeExtended from CropGrowthSystem.h instead

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
	ECropTypeExtended CropType = ECropTypeExtended::Wheat;

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
	FCropData GetCropData(ECropTypeExtended CropType) const;

	UFUNCTION(BlueprintCallable, Category = "Crop System")
	TArray<ECropTypeExtended> GetAvailableCrops() const;

	UFUNCTION(BlueprintCallable, Category = "Crop System")
	TArray<ECropTypeExtended> GetCropsForClimate(float Temperature, float Humidity) const;

	UFUNCTION(BlueprintCallable, Category = "Crop System")
	float GetCropMarketValue(ECropTypeExtended CropType) const;

	UFUNCTION(BlueprintCallable, Category = "Crop System")
	bool IsCropInSeason(ECropTypeExtended CropType, float SeasonFactor) const;

	// Crop breeding
	UFUNCTION(BlueprintCallable, Category = "Crop System")
	ECropTypeExtended BreedCrops(ECropTypeExtended Crop1, ECropTypeExtended Crop2);

	UFUNCTION(BlueprintCallable, Category = "Crop System")
	bool CanBreedCrops(ECropTypeExtended Crop1, ECropTypeExtended Crop2) const;

	// Crop quality assessment
	UFUNCTION(BlueprintCallable, Category = "Crop System")
	float CalculateCropQuality(ECropTypeExtended CropType, ESoilQuality Soil, float GrowthTime) const;

	UFUNCTION(BlueprintCallable, Category = "Crop System")
	FString GetQualityDescription(float Quality) const;

protected:
	void InitializeCropDatabase();

private:
	TMap<ECropTypeExtended, FCropData> CropDataMap;
};