// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ErosionSimulator.generated.h"

/**
 * Erosion type enumeration
 */
UENUM(BlueprintType)
enum class EErosionType : uint8
{
	Hydraulic UMETA(DisplayName = "Hydraulic (Water)"),
	Thermal UMETA(DisplayName = "Thermal (Gravity)"),
	Wind UMETA(DisplayName = "Wind"),
	Combined UMETA(DisplayName = "Combined")
};

/**
 * Hydraulic erosion configuration
 */
USTRUCT(BlueprintType)
struct FHydraulicErosionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic Erosion")
	int32 Iterations = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic Erosion")
	float ErosionStrength = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic Erosion")
	float DepositionStrength = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic Erosion")
	float EvaporationRate = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic Erosion")
	float SedimentCapacity = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic Erosion")
	float MinSlope = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic Erosion")
	float Gravity = 9.81f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic Erosion")
	int32 MaxDropletLifetime = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic Erosion")
	float Inertia = 0.05f; // Higher = droplet changes direction less
};

/**
 * Thermal erosion configuration
 */
USTRUCT(BlueprintType)
struct FThermalErosionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal Erosion")
	int32 Iterations = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal Erosion")
	float TalusAngle = 45.0f; // degrees - angle of repose

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal Erosion")
	float ErosionRate = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal Erosion")
	float MinHeightDifference = 0.1f; // meters
};

/**
 * Wind erosion configuration
 */
USTRUCT(BlueprintType)
struct FWindErosionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Erosion")
	int32 Iterations = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Erosion")
	FVector2D WindDirection = FVector2D(1.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Erosion")
	float WindStrength = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Erosion")
	float AbrasionRate = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Erosion")
	float DepositionRate = 0.05f;
};

/**
 * Drainage pattern data
 */
USTRUCT(BlueprintType)
struct FDrainagePattern
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Drainage")
	TArray<FVector2D> StreamPath;

	UPROPERTY(BlueprintReadWrite, Category = "Drainage")
	float FlowStrength;

	UPROPERTY(BlueprintReadWrite, Category = "Drainage")
	int32 StreamOrder; // Strahler stream order
};

/**
 * Erosion simulator for realistic terrain weathering
 */
UCLASS(BlueprintType)
class ALEXANDER_API UErosionSimulator : public UObject
{
	GENERATED_BODY()

public:
	// ============================================================================
	// HYDRAULIC EROSION
	// ============================================================================

	/**
	 * Apply hydraulic erosion to heightmap
	 * @param HeightData Height values (will be modified)
	 * @param Resolution Grid resolution
	 * @param TileSize Size of tile in meters
	 * @param Config Erosion configuration
	 * @param Seed Random seed for droplet placement
	 */
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void ApplyHydraulicErosion(
		UPARAM(ref) TArray<float>& HeightData,
		int32 Resolution,
		float TileSize,
		const FHydraulicErosionConfig& Config,
		int32 Seed
	);

	/**
	 * Simulate single water droplet erosion
	 * @param HeightData Height values (will be modified)
	 * @param Resolution Grid resolution
	 * @param TileSize Size of tile in meters
	 * @param StartX Starting X position (0-1)
	 * @param StartY Starting Y position (0-1)
	 * @param Config Erosion configuration
	 * @return Path taken by droplet
	 */
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static TArray<FVector2D> SimulateWaterDroplet(
		UPARAM(ref) TArray<float>& HeightData,
		int32 Resolution,
		float TileSize,
		float StartX,
		float StartY,
		const FHydraulicErosionConfig& Config
	);

	// ============================================================================
	// THERMAL EROSION
	// ============================================================================

	/**
	 * Apply thermal erosion to heightmap (gravity-based slope erosion)
	 * @param HeightData Height values (will be modified)
	 * @param Resolution Grid resolution
	 * @param TileSize Size of tile in meters
	 * @param Config Erosion configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void ApplyThermalErosion(
		UPARAM(ref) TArray<float>& HeightData,
		int32 Resolution,
		float TileSize,
		const FThermalErosionConfig& Config
	);

	// ============================================================================
	// WIND EROSION
	// ============================================================================

	/**
	 * Apply wind erosion to heightmap
	 * @param HeightData Height values (will be modified)
	 * @param Resolution Grid resolution
	 * @param TileSize Size of tile in meters
	 * @param Config Erosion configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void ApplyWindErosion(
		UPARAM(ref) TArray<float>& HeightData,
		int32 Resolution,
		float TileSize,
		const FWindErosionConfig& Config
	);

	// ============================================================================
	// DRAINAGE PATTERNS
	// ============================================================================

	/**
	 * Generate drainage patterns (river networks)
	 * @param HeightData Height values
	 * @param Resolution Grid resolution
	 * @param TileSize Size of tile in meters
	 * @param MinStreamLength Minimum stream length to include
	 * @return Array of drainage patterns
	 */
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static TArray<FDrainagePattern> GenerateDrainagePatterns(
		const TArray<float>& HeightData,
		int32 Resolution,
		float TileSize,
		float MinStreamLength
	);

	/**
	 * Calculate flow accumulation map
	 * @param HeightData Height values
	 * @param Resolution Grid resolution
	 * @return Flow accumulation values (higher = more water flow)
	 */
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static TArray<float> CalculateFlowAccumulation(
		const TArray<float>& HeightData,
		int32 Resolution
	);

	// ============================================================================
	// SEDIMENT TRANSPORT
	// ============================================================================

	/**
	 * Simulate sediment transport and deposition
	 * @param HeightData Height values (will be modified)
	 * @param Resolution Grid resolution
	 * @param FlowAccumulation Flow accumulation map
	 * @param TransportCapacity Sediment transport capacity
	 * @param DepositionRate Rate of sediment deposition
	 */
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void SimulateSedimentTransport(
		UPARAM(ref) TArray<float>& HeightData,
		int32 Resolution,
		const TArray<float>& FlowAccumulation,
		float TransportCapacity,
		float DepositionRate
	);

private:
	// ============================================================================
	// HELPER FUNCTIONS
	// ============================================================================

	// Get height at position with bilinear interpolation
	static float GetHeightInterpolated(
		const TArray<float>& HeightData,
		int32 Resolution,
		float X,
		float Y
	);

	// Set height at position with bilinear distribution
	static void SetHeightInterpolated(
		TArray<float>& HeightData,
		int32 Resolution,
		float X,
		float Y,
		float DeltaHeight
	);

	// Calculate gradient at position
	static FVector2D CalculateGradient(
		const TArray<float>& HeightData,
		int32 Resolution,
		float TileSize,
		float X,
		float Y
	);

	// Get neighboring cell indices
	static void GetNeighbors(
		int32 Index,
		int32 Resolution,
		TArray<int32>& OutNeighbors
	);

	// Calculate slope angle between two heights
	static float CalculateSlopeAngle(float Height1, float Height2, float Distance);

	// Check if position is within bounds
	static bool IsInBounds(float X, float Y, int32 Resolution);
};
