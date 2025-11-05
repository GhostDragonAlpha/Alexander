// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FarmPlot.h"
#include "SoilSystem.generated.h"

/**
 * Soil composition data
 */
USTRUCT(BlueprintType)
struct FSoilComposition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float ClayContent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float SandContent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float SiltContent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float OrganicMatter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float pH;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float Nitrogen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float Phosphorus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float Potassium;
};

/**
 * Soil System - Manages soil quality, composition, and improvement
 */
UCLASS(BlueprintType, Blueprintable)
class ALEXANDER_API USoilSystem : public UObject
{
	GENERATED_BODY()

public:
	USoilSystem();

	// Soil analysis
	UFUNCTION(BlueprintCallable, Category = "Soil System")
	FSoilComposition AnalyzeSoil(FVector Location) const;

	UFUNCTION(BlueprintCallable, Category = "Soil System")
	ESoilQuality EvaluateSoilQuality(const FSoilComposition& Composition) const;

	UFUNCTION(BlueprintCallable, Category = "Soil System")
	float GetSoilFertility(const FSoilComposition& Composition) const;

	UFUNCTION(BlueprintCallable, Category = "Soil System")
	float GetWaterRetention(const FSoilComposition& Composition) const;

	// Soil improvement
	UFUNCTION(BlueprintCallable, Category = "Soil System")
	FSoilComposition ApplyFertilizer(const FSoilComposition& Soil, float Nitrogen, float Phosphorus, float Potassium);

	UFUNCTION(BlueprintCallable, Category = "Soil System")
	FSoilComposition ApplyCompost(const FSoilComposition& Soil, float Amount);

	UFUNCTION(BlueprintCallable, Category = "Soil System")
	FSoilComposition AdjustpH(const FSoilComposition& Soil, float TargetpH);

	UFUNCTION(BlueprintCallable, Category = "Soil System")
	FSoilComposition TillSoil(const FSoilComposition& Soil);

	// Soil degradation
	UFUNCTION(BlueprintCallable, Category = "Soil System")
	FSoilComposition SimulateErosion(const FSoilComposition& Soil, float RainfallIntensity, float Slope);

	UFUNCTION(BlueprintCallable, Category = "Soil System")
	FSoilComposition SimulateNutrientDepletion(const FSoilComposition& Soil, float CropConsumption);

	UFUNCTION(BlueprintCallable, Category = "Soil System")
	FSoilComposition SimulateCompaction(const FSoilComposition& Soil, float Traffic);

	// Soil recommendations
	UFUNCTION(BlueprintCallable, Category = "Soil System")
	TArray<FString> GetSoilRecommendations(const FSoilComposition& Soil, ECropType CropType);

	UFUNCTION(BlueprintCallable, Category = "Soil System")
	float GetCropSuitability(const FSoilComposition& Soil, ECropType CropType) const;

	// Soil types
	UFUNCTION(BlueprintCallable, Category = "Soil System")
	FString GetSoilType(const FSoilComposition& Composition) const;

	UFUNCTION(BlueprintCallable, Category = "Soil System")
	bool IsSuitableForFarming(const FSoilComposition& Composition) const;

protected:
	// Helper functions
	float CalculateSoilStructure(const FSoilComposition& Composition) const;
	float CalculateNutrientBalance(const FSoilComposition& Composition) const;
	float CalculateOptimalpH(const FSoilComposition& Composition) const;
};