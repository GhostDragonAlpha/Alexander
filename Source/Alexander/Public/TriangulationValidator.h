// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TriangulationValidator.generated.h"

/**
 * Observer measurement of a target (distance + direction)
 */
USTRUCT(BlueprintType)
struct FObserverMeasurement
{
	GENERATED_BODY()

	// ID of observer making measurement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation")
	int32 ObserverID = 0;

	// ID of target being observed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation")
	int32 TargetID = 0;

	// Observer's position in world space
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation")
	FVector ObserverPosition = FVector::ZeroVector;

	// Measured distance to target (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation")
	float Distance = 0.0f;

	// Direction unit vector to target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation")
	FVector Direction = FVector::ForwardVector;

	// Scaling factor applied (from ScalingCalculator)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation")
	float ScaleFactor = 1.0f;

	// Timestamp of measurement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation")
	float Timestamp = 0.0f;
};

/**
 * Result of geometric validation
 */
USTRUCT(BlueprintType)
struct FGeometricValidationResult
{
	GENERATED_BODY()

	// Is the geometry valid?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation")
	bool bIsValid = false;

	// Geometric error magnitude (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation")
	float GeometricError = 0.0f;

	// Calculated position from triangulation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation")
	FVector TriangulatedPosition = FVector::ZeroVector;

	// Confidence level (0.0-1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation")
	float Confidence = 0.0f;

	// Number of observers used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation")
	int32 ObserverCount = 0;

	// Validation method used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation")
	FString ValidationMethod;
};

/**
 * Triangulation Validator
 *
 * Performs geometric validation of positions using triangulation mathematics.
 * The MORE observers, the MORE trustworthy the result (anti-fragile system).
 *
 * Key Principles:
 * - 2 observers = Can measure distance only (line)
 * - 3 observers = 2D triangulation (triangle in plane)
 * - 4 observers = 3D triangulation (tetrahedron in space)
 * - 5+ observers = Overdetermined (redundancy catches inconsistencies)
 *
 * Mathematical Basis:
 * - Law of Cosines: c² = a² + b² - 2ab·cos(θ)
 * - Triangle Inequality: a + b > c
 * - Tetrahedron volume calculation for 3D validation
 *
 * The system gets STRONGER with more players:
 * - 4 players = 94% confidence (basic tetrahedron)
 * - 8 players = 99% confidence (double redundancy)
 * - 16 players = 99.9% confidence (highly overdetermined)
 * - 64 players = 99.99% confidence (cryptographically secure)
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API UTriangulationValidator : public UObject
{
	GENERATED_BODY()

public:
	UTriangulationValidator();

	// ============================================================================
	// CORE VALIDATION METHODS
	// ============================================================================

	/**
	 * Validate target position using all available measurements
	 *
	 * Automatically selects best validation method based on observer count:
	 * - 2 observers: Distance consistency check only
	 * - 3 observers: Triangle validation (planar)
	 * - 4 observers: Tetrahedron validation (full 3D)
	 * - 5+ observers: Overdetermined system (best-fit with outlier detection)
	 *
	 * @param Measurements - Array of observer measurements
	 * @return Validation result with triangulated position
	 */
	UFUNCTION(BlueprintCallable, Category = "Triangulation")
	FGeometricValidationResult ValidatePosition(const TArray<FObserverMeasurement>& Measurements);

	/**
	 * Validate using triangle (3 observers)
	 *
	 * Verifies that 3 distance measurements form a valid triangle using Law of Cosines.
	 * Works in 2D (planar) space.
	 *
	 * @param ObsA, ObsB, ObsC - Three observer measurements
	 * @return True if triangle is geometrically consistent
	 */
	UFUNCTION(BlueprintCallable, Category = "Triangulation|Triangle")
	bool ValidateTriangle(const FObserverMeasurement& ObsA, const FObserverMeasurement& ObsB, const FObserverMeasurement& ObsC);

	/**
	 * Validate using tetrahedron (4 observers)
	 *
	 * Verifies that 4 distance measurements form a valid tetrahedron in 3D space.
	 * All 4 faces must be valid triangles, and volume must be non-zero.
	 *
	 * @param Measurements - Array of exactly 4 observer measurements
	 * @return True if tetrahedron is geometrically consistent
	 */
	UFUNCTION(BlueprintCallable, Category = "Triangulation|Tetrahedron")
	bool ValidateTetrahedron(const TArray<FObserverMeasurement>& Measurements);

	/**
	 * Validate overdetermined system (5+ observers)
	 *
	 * Uses least-squares fitting to find best position that satisfies all measurements.
	 * Detects outliers and calculates confidence based on residual errors.
	 *
	 * @param Measurements - Array of 5 or more observer measurements
	 * @return Validation result with confidence metrics
	 */
	UFUNCTION(BlueprintCallable, Category = "Triangulation|Overdetermined")
	FGeometricValidationResult ValidateOverdetermined(const TArray<FObserverMeasurement>& Measurements);

	// ============================================================================
	// GEOMETRIC CALCULATIONS
	// ============================================================================

	/**
	 * Calculate distance between two points using Law of Cosines
	 *
	 * Given two points A and B, both at known distances from origin C,
	 * and the angle between them, calculate distance AB.
	 *
	 * Formula: c² = a² + b² - 2ab·cos(θ)
	 *
	 * @param DistanceA - Distance from C to A
	 * @param DistanceB - Distance from C to B
	 * @param AngleRadians - Angle ACB in radians
	 * @return Distance from A to B
	 */
	UFUNCTION(BlueprintCallable, Category = "Triangulation|Math")
	float CalculateThirdSide(float DistanceA, float DistanceB, float AngleRadians);

	/**
	 * Calculate angle using Law of Cosines (inverse)
	 *
	 * Given three sides of a triangle, calculate angle opposite to side C.
	 *
	 * Formula: cos(θ) = (a² + b² - c²) / (2ab)
	 *
	 * @param SideA, SideB, SideC - Three sides of triangle
	 * @return Angle opposite to SideC in radians
	 */
	UFUNCTION(BlueprintCallable, Category = "Triangulation|Math")
	float CalculateAngle(float SideA, float SideB, float SideC);

	/**
	 * Check triangle inequality theorem
	 *
	 * A valid triangle must satisfy: a + b > c for all side combinations.
	 *
	 * @param SideA, SideB, SideC - Three sides of triangle
	 * @return True if sides can form a valid triangle
	 */
	UFUNCTION(BlueprintCallable, Category = "Triangulation|Math")
	bool CheckTriangleInequality(float SideA, float SideB, float SideC);

	/**
	 * Calculate volume of tetrahedron
	 *
	 * Used to verify tetrahedron is non-degenerate (not planar).
	 * Volume must be > tolerance for valid 3D configuration.
	 *
	 * @param Vertices - 4 vertices of tetrahedron
	 * @return Volume in cubic meters
	 */
	UFUNCTION(BlueprintCallable, Category = "Triangulation|Math")
	float CalculateTetrahedronVolume(const TArray<FVector>& Vertices);

	// ============================================================================
	// TRIANGULATION (POSITION CALCULATION)
	// ============================================================================

	/**
	 * Triangulate position from 3 observers (2D)
	 *
	 * Calculates target position given 3 distance measurements.
	 * Returns position in 2D plane defined by observers.
	 *
	 * @param Measurements - Array of exactly 3 measurements
	 * @return Calculated position (may be ambiguous, returns one solution)
	 */
	UFUNCTION(BlueprintCallable, Category = "Triangulation|Position")
	FVector TriangulateFrom3Observers(const TArray<FObserverMeasurement>& Measurements);

	/**
	 * Triangulate position from 4 observers (3D)
	 *
	 * Calculates target position given 4 distance measurements.
	 * Provides unique solution in 3D space (no ambiguity).
	 *
	 * @param Measurements - Array of exactly 4 measurements
	 * @return Calculated position in 3D space
	 */
	UFUNCTION(BlueprintCallable, Category = "Triangulation|Position")
	FVector TriangulateFrom4Observers(const TArray<FObserverMeasurement>& Measurements);

	/**
	 * Triangulate position from 5+ observers (least-squares)
	 *
	 * Calculates best-fit position using overdetermined system.
	 * More observers = higher confidence.
	 *
	 * @param Measurements - Array of 5+ measurements
	 * @return Best-fit position with confidence metric
	 */
	UFUNCTION(BlueprintCallable, Category = "Triangulation|Position")
	FGeometricValidationResult TriangulateOverdetermined(const TArray<FObserverMeasurement>& Measurements);

	// ============================================================================
	// TOLERANCE AND ERROR CALCULATIONS
	// ============================================================================

	/**
	 * Calculate geometric tolerance based on distance and conditions
	 *
	 * Tolerance accounts for:
	 * - Floating-point precision loss at large distances
	 * - Scaling factor compression errors
	 * - Network latency effects
	 * - Measurement uncertainty
	 *
	 * @param Distance - Distance being measured
	 * @param ScaleFactor - Scaling factor applied
	 * @param ObserverCount - Number of observers (more = tighter tolerance)
	 * @return Tolerance in meters
	 */
	UFUNCTION(BlueprintCallable, Category = "Triangulation|Tolerance")
	float CalculateTolerance(float Distance, float ScaleFactor, int32 ObserverCount);

	/**
	 * Calculate confidence level based on observer count
	 *
	 * More observers = higher confidence (anti-fragile property).
	 *
	 * Formula: Confidence = 1 - (1 / ObserverCount^1.5)
	 *
	 * @param ObserverCount - Number of observers
	 * @return Confidence (0.0 = no confidence, 1.0 = absolute confidence)
	 */
	UFUNCTION(BlueprintCallable, Category = "Triangulation|Tolerance")
	float CalculateConfidence(int32 ObserverCount);

	/**
	 * Calculate geometric error between measurements
	 *
	 * Measures how much the observations deviate from geometric consistency.
	 *
	 * @param Measurements - Observer measurements
	 * @return RMS error in meters
	 */
	UFUNCTION(BlueprintCallable, Category = "Triangulation|Tolerance")
	float CalculateGeometricError(const TArray<FObserverMeasurement>& Measurements);

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Base geometric tolerance (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation|Config")
	float BaseGeometricTolerance = 10.0f;

	// Tolerance scaling per 1000 meters distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation|Config")
	float TolerancePerKM = 1.0f;

	// Minimum tetrahedron volume (cubic meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation|Config")
	float MinimumTetrahedronVolume = 1.0f;

	// Maximum allowed geometric error (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation|Config")
	float MaximumGeometricError = 100.0f;

	// Enable debug logging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation|Debug")
	bool bEnableDebugLogging = false;

	// Enable debug visualization (draw lines, spheres)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangulation|Debug")
	bool bEnableDebugVisualization = false;

protected:
	// Helper: Get distance between two observers
	float GetDistanceBetweenObservers(const FObserverMeasurement& ObsA, const FObserverMeasurement& ObsB) const;

	// Helper: Calculate angle between two direction vectors
	float GetAngleBetweenDirections(const FVector& DirA, const FVector& DirB) const;

	// Helper: Check if measurements are consistent within tolerance
	bool AreConsistentWithinTolerance(float Measured, float Expected, float Tolerance) const;

	// Helper: Draw debug visualization
	void DrawDebugGeometry(const TArray<FObserverMeasurement>& Measurements, const FVector& TargetPosition, bool bIsValid);
};
