// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataAsset.h"
#include "FarmingSubsystem.generated.h"

// Forward declarations
class AFarmPlot;
class UCropDefinition;
class UBiomeCompatibilitySystem;
struct FBiomeDefinition;

/**
 * Crop database entry
 */
USTRUCT(BlueprintType)
struct FCropDatabaseEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FString CropID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	UCropDefinition* CropDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	TArray<FString> SuitableBiomes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float MinSoilQuality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float OptimalTemperature;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float OptimalHumidity;

	FCropDatabaseEntry()
		: MinSoilQuality(0.3f)
		, OptimalTemperature(20.0f)
		, OptimalHumidity(0.6f)
	{
	}
};

/**
 * Farming location validation result
 */
USTRUCT(BlueprintType)
struct FFarmingLocationValidation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	bool bIsValid;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString FailureReason;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	float SlopeAngle;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	float SoilQuality;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString BiomeName;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	float WaterAvailability;

	FFarmingLocationValidation()
		: bIsValid(false)
		, SlopeAngle(0.0f)
		, SoilQuality(0.0f)
		, WaterAvailability(0.0f)
	{
	}
};

/**
 * World subsystem for managing farming operations across all planets
 */
UCLASS()
class ALEXANDER_API UFarmingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFarmingSubsystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tick function
	virtual void Tick(float DeltaTime);
	virtual bool IsTickable() const { return true; }

	// ============================================================================
	// FARM PLOT MANAGEMENT
	// ============================================================================

	/**
	 * Create a new farm plot at the specified location
	 * @param Location World location for the farm plot
	 * @param Size Size of the farm plot in meters (X, Y)
	 * @param PlanetActor The planet actor this farm belongs to
	 * @return Newly created farm plot, or nullptr if creation failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	AFarmPlot* CreateFarmPlot(FVector Location, FVector2D Size, AActor* PlanetActor);

	/**
	 * Remove a farm plot from management
	 * @param FarmPlot The farm plot to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void RemoveFarmPlot(AFarmPlot* FarmPlot);

	/**
	 * Get all active farm plots
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	const TArray<AFarmPlot*>& GetActiveFarms() const { return ActiveFarms; }

	/**
	 * Get farm plots on a specific planet
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	TArray<AFarmPlot*> GetFarmsOnPlanet(AActor* PlanetActor) const;

	// ============================================================================
	// CROP DATABASE
	// ============================================================================

	/**
	 * Register a crop definition in the database
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Database")
	void RegisterCrop(const FString& CropID, UCropDefinition* CropDef, const TArray<FString>& SuitableBiomes);

	/**
	 * Get crop definition by ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Database")
	UCropDefinition* GetCropDefinition(const FString& CropID) const;

	/**
	 * Get all crops suitable for a specific biome
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Database")
	TArray<UCropDefinition*> GetCropsForBiome(const FString& BiomeName) const;

	/**
	 * Get all registered crop IDs
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Database")
	TArray<FString> GetAllCropIDs() const;

	// ============================================================================
	// LOCATION VALIDATION
	// ============================================================================

	/**
	 * Validate if a location is suitable for farming
	 * @param Location World location to validate
	 * @param PlanetActor The planet actor to check against
	 * @return Validation result with details
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Validation")
	FFarmingLocationValidation ValidateFarmingLocation(FVector Location, AActor* PlanetActor);

	/**
	 * Check if slope is suitable for farming (< 15 degrees)
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Validation")
	bool IsSlopeSuitable(FVector Location, AActor* PlanetActor, float& OutSlopeAngle);

	/**
	 * Get soil quality at location based on biome
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Validation")
	float GetSoilQualityAtLocation(FVector Location, AActor* PlanetActor);

	/**
	 * Get biome name at location
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Validation")
	FString GetBiomeAtLocation(FVector Location, AActor* PlanetActor);

	/**
	 * Check water availability at location
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Validation")
	float GetWaterAvailability(FVector Location, AActor* PlanetActor);

	// ============================================================================
	// BIOME COMPATIBILITY
	// ============================================================================

	/**
	 * Get the biome compatibility system
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Compatibility")
	UBiomeCompatibilitySystem* GetBiomeCompatibilitySystem() const { return BiomeCompatibilitySystem; }

	/**
	 * Get crops suitable for a specific biome type
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Compatibility")
	TArray<UCropDefinition*> GetCompatibleCropsForBiome(EBiomeType BiomeType) const;

	/**
	 * Calculate crop compatibility score at location
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Compatibility")
	float CalculateCropCompatibility(UCropDefinition* CropDef, FVector Location, AActor* PlanetActor);

	/**
	 * Get recommended crops for a location
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Compatibility")
	TArray<UCropDefinition*> GetRecommendedCropsForLocation(FVector Location, AActor* PlanetActor, int32 MaxResults = 5);

protected:
	// Active farm plots being managed
	UPROPERTY()
	TArray<AFarmPlot*> ActiveFarms;

	// Crop database
	UPROPERTY()
	TMap<FString, FCropDatabaseEntry> CropDatabase;

	// Maximum slope angle for farming (degrees)
	UPROPERTY(EditDefaultsOnly, Category = "Farming|Validation")
	float MaxFarmingSlope;

	// Minimum soil quality for farming
	UPROPERTY(EditDefaultsOnly, Category = "Farming|Validation")
	float MinSoilQuality;

	// Minimum water availability for farming
	UPROPERTY(EditDefaultsOnly, Category = "Farming|Validation")
	float MinWaterAvailability;

	// Biome compatibility system
	UPROPERTY()
	UBiomeCompatibilitySystem* BiomeCompatibilitySystem;

	// Biome soil system (Task 14.2)
	UPROPERTY()
	class UBiomeSoilSystem* BiomeSoilSystem;

	// Farming optimization system (Task 18.2)
	UPROPERTY()
	class UFarmingOptimizationSystem* OptimizationSystem;

public:
	/**
	 * Get the biome soil system
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Soil")
	UBiomeSoilSystem* GetBiomeSoilSystem() const { return BiomeSoilSystem; }

	/**
	 * Get the farming optimization system
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	UFarmingOptimizationSystem* GetOptimizationSystem() const { return OptimizationSystem; }

	/**
	 * Get soil variation at a specific location
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Soil")
	struct FSoilVariation GetSoilVariationAtLocation(FVector Location, AActor* PlanetActor);

private:
	// Helper to calculate slope at location
	float CalculateSlope(FVector Location, AActor* PlanetActor);

	// Helper to get biome definition at location
	const FBiomeDefinition* GetBiomeDefinitionAtLocation(FVector Location, AActor* PlanetActor);
};
