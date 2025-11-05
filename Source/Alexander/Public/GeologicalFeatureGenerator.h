// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GeologicalFeatureGenerator.generated.h"

/**
 * Geological feature type enumeration
 */
UENUM(BlueprintType)
enum class EGeologicalFeatureType : uint8
{
	Mountain UMETA(DisplayName = "Mountain"),
	Valley UMETA(DisplayName = "Valley"),
	Canyon UMETA(DisplayName = "Canyon"),
	Crater UMETA(DisplayName = "Crater"),
	Plateau UMETA(DisplayName = "Plateau"),
	Ridge UMETA(DisplayName = "Ridge")
};

/**
 * Mountain range configuration
 */
USTRUCT(BlueprintType)
struct FMountainRangeConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
	FVector2D Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
	float Length = 5000.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
	float Width = 2000.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
	float Height = 2000.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
	float Rotation = 0.0f; // degrees

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
	float Roughness = 0.7f; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
	int32 PeakCount = 5;
};

/**
 * Valley configuration
 */
USTRUCT(BlueprintType)
struct FValleyConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Valley")
	FVector2D StartPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Valley")
	FVector2D EndPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Valley")
	float Width = 500.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Valley")
	float Depth = 200.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Valley")
	float Smoothness = 0.5f; // 0-1
};

/**
 * Canyon configuration
 */
USTRUCT(BlueprintType)
struct FCanyonConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canyon")
	TArray<FVector2D> PathPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canyon")
	float Width = 300.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canyon")
	float Depth = 500.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canyon")
	float Steepness = 0.8f; // 0-1, higher = steeper walls
};

/**
 * Crater configuration
 */
USTRUCT(BlueprintType)
struct FCraterConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
	FVector2D Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
	float Radius = 500.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
	float Depth = 100.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
	float RimHeight = 50.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
	float RimWidth = 100.0f; // meters
};

/**
 * Plateau configuration
 */
USTRUCT(BlueprintType)
struct FPlateauConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plateau")
	FVector2D Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plateau")
	float Radius = 2000.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plateau")
	float Height = 500.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plateau")
	float EdgeSteepness = 0.7f; // 0-1
};

/**
 * Geological feature generator for terrain
 */
UCLASS(BlueprintType)
class ALEXANDER_API UGeologicalFeatureGenerator : public UObject
{
	GENERATED_BODY()

public:
	// ============================================================================
	// MOUNTAIN GENERATION
	// ============================================================================

	/**
	 * Generate mountain range height contribution
	 * @param X World X coordinate
	 * @param Y World Y coordinate
	 * @param Config Mountain range configuration
	 * @param Seed Random seed
	 * @return Height contribution in meters
	 */
	UFUNCTION(BlueprintCallable, Category = "Geological Features")
	static float GenerateMountainRange(float X, float Y, const FMountainRangeConfig& Config, int32 Seed);

	/**
	 * Generate multiple mountain ranges
	 * @param X World X coordinate
	 * @param Y World Y coordinate
	 * @param Ranges Array of mountain range configurations
	 * @param Seed Random seed
	 * @return Combined height contribution
	 */
	UFUNCTION(BlueprintCallable, Category = "Geological Features")
	static float GenerateMountainRanges(float X, float Y, const TArray<FMountainRangeConfig>& Ranges, int32 Seed);

	// ============================================================================
	// VALLEY GENERATION
	// ============================================================================

	/**
	 * Generate valley height contribution
	 * @param X World X coordinate
	 * @param Y World Y coordinate
	 * @param Config Valley configuration
	 * @return Height contribution (negative for depression)
	 */
	UFUNCTION(BlueprintCallable, Category = "Geological Features")
	static float GenerateValley(float X, float Y, const FValleyConfig& Config);

	/**
	 * Generate valley system with multiple valleys
	 * @param X World X coordinate
	 * @param Y World Y coordinate
	 * @param Valleys Array of valley configurations
	 * @return Combined height contribution
	 */
	UFUNCTION(BlueprintCallable, Category = "Geological Features")
	static float GenerateValleySystem(float X, float Y, const TArray<FValleyConfig>& Valleys);

	// ============================================================================
	// CANYON GENERATION
	// ============================================================================

	/**
	 * Generate canyon height contribution
	 * @param X World X coordinate
	 * @param Y World Y coordinate
	 * @param Config Canyon configuration
	 * @return Height contribution (negative for depression)
	 */
	UFUNCTION(BlueprintCallable, Category = "Geological Features")
	static float GenerateCanyon(float X, float Y, const FCanyonConfig& Config);

	/**
	 * Generate canyon system with multiple canyons
	 * @param X World X coordinate
	 * @param Y World Y coordinate
	 * @param Canyons Array of canyon configurations
	 * @return Combined height contribution
	 */
	UFUNCTION(BlueprintCallable, Category = "Geological Features")
	static float GenerateCanyonSystem(float X, float Y, const TArray<FCanyonConfig>& Canyons);

	// ============================================================================
	// CRATER GENERATION
	// ============================================================================

	/**
	 * Generate crater height contribution
	 * @param X World X coordinate
	 * @param Y World Y coordinate
	 * @param Config Crater configuration
	 * @return Height contribution
	 */
	UFUNCTION(BlueprintCallable, Category = "Geological Features")
	static float GenerateCrater(float X, float Y, const FCraterConfig& Config);

	/**
	 * Generate multiple craters
	 * @param X World X coordinate
	 * @param Y World Y coordinate
	 * @param Craters Array of crater configurations
	 * @return Combined height contribution
	 */
	UFUNCTION(BlueprintCallable, Category = "Geological Features")
	static float GenerateCraters(float X, float Y, const TArray<FCraterConfig>& Craters);

	/**
	 * Generate procedural crater field using Voronoi
	 * @param X World X coordinate
	 * @param Y World Y coordinate
	 * @param Seed Random seed
	 * @param Density Crater density (craters per km²)
	 * @param MinRadius Minimum crater radius
	 * @param MaxRadius Maximum crater radius
	 * @return Height contribution
	 */
	UFUNCTION(BlueprintCallable, Category = "Geological Features")
	static float GenerateCraterField(float X, float Y, int32 Seed, float Density, float MinRadius, float MaxRadius);

	// ============================================================================
	// PLATEAU GENERATION
	// ============================================================================

	/**
	 * Generate plateau height contribution
	 * @param X World X coordinate
	 * @param Y World Y coordinate
	 * @param Config Plateau configuration
	 * @return Height contribution
	 */
	UFUNCTION(BlueprintCallable, Category = "Geological Features")
	static float GeneratePlateau(float X, float Y, const FPlateauConfig& Config);

	/**
	 * Generate multiple plateaus
	 * @param X World X coordinate
	 * @param Y World Y coordinate
	 * @param Plateaus Array of plateau configurations
	 * @return Combined height contribution
	 */
	UFUNCTION(BlueprintCallable, Category = "Geological Features")
	static float GeneratePlateaus(float X, float Y, const TArray<FPlateauConfig>& Plateaus);

private:
	// ============================================================================
	// HELPER FUNCTIONS
	// ============================================================================

	// Smooth step function for gradual transitions
	static float SmoothStep(float Edge0, float Edge1, float X);

	// Distance from point to line segment
	static float DistanceToLineSegment(FVector2D Point, FVector2D LineStart, FVector2D LineEnd);

	// Distance from point to polyline
	static float DistanceToPolyline(FVector2D Point, const TArray<FVector2D>& PathPoints);

	// Rotate point around origin
	static FVector2D RotatePoint(FVector2D Point, float AngleDegrees);

	// Falloff function for feature blending
	static float FalloffCurve(float Distance, float Radius, float Steepness);
};
