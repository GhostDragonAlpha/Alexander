// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VisualQualityValidator.generated.h"

/**
 * Quality metrics for visual validation
 */
USTRUCT(BlueprintType)
struct FVisualQualityMetrics
{
	GENERATED_BODY()

	// Atmospheric quality (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "Quality")
	float AtmosphericQuality = 0.0f;

	// Biome transition quality (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "Quality")
	float BiomeTransitionQuality = 0.0f;

	// Material quality (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "Quality")
	float MaterialQuality = 0.0f;

	// Overall quality score (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "Quality")
	float OverallQuality = 0.0f;

	// Detailed feedback
	UPROPERTY(BlueprintReadOnly, Category = "Quality")
	TArray<FString> QualityIssues;

	// Star Citizen comparison score (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "Quality")
	float StarCitizenComparison = 0.0f;
};

/**
 * Reference standards for Star Citizen quality comparison
 */
USTRUCT(BlueprintType)
struct FStarCitizenQualityStandards
{
	GENERATED_BODY()

	// Atmospheric scattering quality threshold
	UPROPERTY(EditAnywhere, Category = "Standards")
	float MinAtmosphericScatteringQuality = 0.85f;

	// Biome transition smoothness threshold
	UPROPERTY(EditAnywhere, Category = "Standards")
	float MinBiomeTransitionSmoothness = 0.90f;

	// Material detail level threshold
	UPROPERTY(EditAnywhere, Category = "Standards")
	float MinMaterialDetailLevel = 0.88f;

	// Minimum texture resolution (pixels)
	UPROPERTY(EditAnywhere, Category = "Standards")
	int32 MinTextureResolution = 2048;

	// Maximum LOD transition distance (meters)
	UPROPERTY(EditAnywhere, Category = "Standards")
	float MaxLODTransitionDistance = 500.0f;

	// Minimum cloud detail level
	UPROPERTY(EditAnywhere, Category = "Standards")
	float MinCloudDetailLevel = 0.80f;
};

/**
 * Visual Quality Validator
 * Validates visual quality against Star Citizen reference standards
 */
UCLASS()
class ALEXANDER_API AVisualQualityValidator : public AActor
{
	GENERATED_BODY()
	
public:	
	AVisualQualityValidator();

	// Quality standards reference
	UPROPERTY(EditAnywhere, Category = "Validation")
	FStarCitizenQualityStandards QualityStandards;

	// Enable detailed logging
	UPROPERTY(EditAnywhere, Category = "Validation")
	bool bEnableDetailedLogging = true;

	// Validate atmospheric effects
	UFUNCTION(BlueprintCallable, Category = "Validation")
	float ValidateAtmosphericEffects(class APlanet* Planet);

	// Validate biome transitions
	UFUNCTION(BlueprintCallable, Category = "Validation")
	float ValidateBiomeTransitions(class APlanet* Planet, const TArray<FVector>& SampleLocations);

	// Validate material quality
	UFUNCTION(BlueprintCallable, Category = "Validation")
	float ValidateMaterialQuality(class APlanet* Planet);

	// Run complete visual quality validation
	UFUNCTION(BlueprintCallable, Category = "Validation")
	FVisualQualityMetrics RunCompleteValidation(class APlanet* Planet);

	// Compare against Star Citizen reference
	UFUNCTION(BlueprintCallable, Category = "Validation")
	float CompareToStarCitizenReference(const FVisualQualityMetrics& Metrics);

	// Generate validation report
	UFUNCTION(BlueprintCallable, Category = "Validation")
	FString GenerateValidationReport(const FVisualQualityMetrics& Metrics);

protected:
	virtual void BeginPlay() override;

private:
	// Validate atmospheric scattering
	float ValidateAtmosphericScattering(class UPlanetAtmosphereComponent* AtmosphereComponent);

	// Validate cloud quality
	float ValidateCloudQuality(class UPlanetCloudComponent* CloudComponent);

	// Validate fog effects
	float ValidateFogEffects(class UPlanetAtmosphereComponent* AtmosphereComponent);

	// Validate biome blend smoothness
	float ValidateBiomeBlendSmoothness(const TArray<FVector>& SampleLocations, class APlanet* Planet);

	// Validate material texture quality
	float ValidateMaterialTextureQuality(class UMaterialInterface* Material);

	// Validate material parameters
	float ValidateMaterialParameters(class UMaterialInterface* Material);

	// Validate LOD transitions
	float ValidateLODTransitions(class APlanet* Planet);

	// Calculate quality score from metrics
	float CalculateQualityScore(float AtmosphericScore, float BiomeScore, float MaterialScore);

	// Add quality issue to list
	void AddQualityIssue(TArray<FString>& Issues, const FString& Issue, float Severity);
};
