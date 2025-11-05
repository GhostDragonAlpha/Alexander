// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/CriticalSection.h"
#include "ScalingCalculator.generated.h"

/**
 * Scaling method for distance-based calculations
 */
UENUM(BlueprintType)
enum class EScalingMethod : uint8
{
	/** Linear scaling based on distance */
	Linear UMETA(DisplayName = "Linear"),

	/** Inverse square law scaling (realistic) */
	InverseSquare UMETA(DisplayName = "Inverse Square"),

	/** Logarithmic scaling for large distances */
	Logarithmic UMETA(DisplayName = "Logarithmic"),

	/** Custom user-defined curve */
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Game instance subsystem for calculating celestial body scaling
 * Implements inverse-square law and other scaling algorithms
 * Thread-safe with caching for performance optimization
 */
UCLASS()
class ALEXANDER_API UScalingCalculator : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ========== Subsystem Lifecycle ==========

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== Scale Calculation ==========

	/**
	 * Calculate scale factor based on distance from reference point
	 * Uses the configured scaling method (default: inverse square)
	 * @param Distance - Distance from reference point in Unreal units
	 * @param ReferenceDistance - Distance at which scale factor is 1.0
	 * @return Scale factor to apply (1.0 = normal size)
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Scaling")
	double CalculateScaleFactor(double Distance, double ReferenceDistance) const;

	/**
	 * Apply inverse square law scaling with clamping
	 * Scale = (ReferenceDistance / Distance)²
	 * @param Distance - Current distance from reference point
	 * @return Clamped scale factor
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Scaling")
	double ApplyInverseSquareLaw(double Distance) const;

	/**
	 * Apply linear scaling with clamping
	 * Scale = ReferenceDistance / Distance
	 * @param Distance - Current distance from reference point
	 * @return Clamped scale factor
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Scaling")
	double ApplyLinearScaling(double Distance) const;

	/**
	 * Apply logarithmic scaling for extreme distances
	 * Scale = Log(ReferenceDistance) / Log(Distance)
	 * @param Distance - Current distance from reference point
	 * @return Clamped scale factor
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Scaling")
	double ApplyLogarithmicScaling(double Distance) const;

	// ========== Smooth Transitions ==========

	/**
	 * Smoothly transition from current scale to target scale
	 * Uses interpolation for smooth visual transitions
	 * @param CurrentScale - Current scale value
	 * @param TargetScale - Target scale value
	 * @param DeltaTime - Time since last update
	 * @param TransitionSpeed - Speed of transition (higher = faster)
	 * @return Interpolated scale value
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Scaling")
	double SmoothScaleTransition(double CurrentScale, double TargetScale, float DeltaTime, float TransitionSpeed) const;

	/**
	 * Clamp scale factor to configured min/max values
	 * @param ScaleFactor - Raw scale factor to clamp
	 * @param MinScale - Minimum allowed scale (optional override)
	 * @param MaxScale - Maximum allowed scale (optional override)
	 * @return Clamped scale factor
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Scaling")
	double ClampScaleFactor(double ScaleFactor, double MinScale = 0.0, double MaxScale = 0.0) const;

	// ========== Optimization ==========

	/**
	 * Check if scale should be updated based on distance change threshold
	 * Prevents unnecessary updates for small distance changes
	 * @param OldDistance - Previous distance
	 * @param NewDistance - Current distance
	 * @param Threshold - Minimum distance change to trigger update (as fraction)
	 * @return True if scale should be updated
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Scaling")
	bool ShouldUpdateScale(double OldDistance, double NewDistance, double Threshold = 0.05) const;

	/**
	 * Get cached scale factor for a specific distance
	 * Uses thread-safe caching for performance
	 * @param Distance - Distance to query
	 * @param OutScaleFactor - Output scale factor
	 * @return True if found in cache
	 */
	bool GetCachedScaleFactor(int32 DistanceKey, double& OutScaleFactor) const;

	/**
	 * Cache a scale factor for future lookups
	 * Thread-safe caching
	 * @param DistanceKey - Quantized distance key
	 * @param ScaleFactor - Scale factor to cache
	 */
	void CacheScaleFactor(int32 DistanceKey, double ScaleFactor);

	/**
	 * Clear the scale factor cache
	 * Use when changing scaling parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Scaling")
	void ClearCache();

	/**
	 * Convert distance to cache key (quantized for efficient lookup)
	 * @param Distance - Distance to quantize
	 * @return Integer cache key
	 */
	int32 DistanceToCacheKey(double Distance) const;

	// ========== Configuration ==========

	/**
	 * Set the scaling method to use
	 * @param Method - Scaling method to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Scaling")
	void SetScalingMethod(EScalingMethod Method);

	/**
	 * Get the current scaling method
	 * @return Active scaling method
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Scaling")
	EScalingMethod GetScalingMethod() const { return CurrentScalingMethod; }

	/**
	 * Set the reference distance for scaling calculations
	 * @param Distance - Reference distance in Unreal units
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Scaling")
	void SetReferenceDistance(double Distance);

	/**
	 * Get the reference distance
	 * @return Reference distance in Unreal units
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Scaling")
	double GetReferenceDistance() const { return ReferenceDistance; }

	/**
	 * Set scale factor limits
	 * @param Min - Minimum scale factor
	 * @param Max - Maximum scale factor
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Scaling")
	void SetScaleLimits(double Min, double Max);

	// ========== Network Sync ==========

	/**
	 * Server-side calculation of scale factors
	 * Broadcasts results to clients for synchronization
	 * @param Distances - Array of distances to calculate scales for
	 * @return Array of calculated scale factors
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Scaling|Network")
	TArray<double> CalculateScaleFactorsForDistances(const TArray<double>& Distances);

	// ========== Debug ==========

	/**
	 * Get statistics about the scale cache
	 * @param OutCacheSize - Number of cached entries
	 * @param OutHitRate - Cache hit rate (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Scaling|Debug")
	void GetCacheStatistics(int32& OutCacheSize, float& OutHitRate) const;

	/**
	 * Enable or disable debug logging
	 * @param bEnabled - Whether to enable debug logging
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Scaling|Debug")
	void SetDebugLogging(bool bEnabled) { bEnableDebugLogging = bEnabled; }

protected:
	// ========== Configuration Properties ==========

	/** Current scaling method being used */
	UPROPERTY(EditDefaultsOnly, Category = "Scaling")
	EScalingMethod CurrentScalingMethod;

	/** Reference distance where scale factor = 1.0 (in cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Scaling")
	double ReferenceDistance;

	/** Minimum allowed scale factor */
	UPROPERTY(EditDefaultsOnly, Category = "Scaling")
	double MinScaleFactor;

	/** Maximum allowed scale factor */
	UPROPERTY(EditDefaultsOnly, Category = "Scaling")
	double MaxScaleFactor;

	/** Exponent for inverse square law (default: 2.0) */
	UPROPERTY(EditDefaultsOnly, Category = "Scaling|InverseSquare")
	double InverseSquareExponent;

	/** Base for logarithmic scaling (default: 10.0) */
	UPROPERTY(EditDefaultsOnly, Category = "Scaling|Logarithmic")
	double LogarithmicBase;

	/** Default transition speed for smooth scaling */
	UPROPERTY(EditDefaultsOnly, Category = "Scaling|Transitions")
	float DefaultTransitionSpeed;

	// ========== Caching ==========

	/** Scale factor cache (distance key -> scale factor) */
	TMap<int32, double> ScaleFactorCache;

	/** Thread safety lock for cache access */
	mutable FCriticalSection CacheLock;

	/** Cache quantization factor (distance units per cache key) */
	UPROPERTY(EditDefaultsOnly, Category = "Scaling|Cache")
	double CacheQuantizationFactor;

	/** Maximum cache size before cleanup */
	UPROPERTY(EditDefaultsOnly, Category = "Scaling|Cache")
	int32 MaxCacheSize;

	// ========== Statistics ==========

	/** Total cache lookups */
	mutable int32 CacheLookups;

	/** Cache hits */
	mutable int32 CacheHits;

	// ========== Debug ==========

	/** Enable debug logging */
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bEnableDebugLogging;

	// ========== Internal Methods ==========

	/** Perform cache cleanup when size exceeds limit */
	void CleanupCache();

	/** Calculate scale using the selected method */
	double CalculateScaleInternal(double Distance, EScalingMethod Method) const;
};
