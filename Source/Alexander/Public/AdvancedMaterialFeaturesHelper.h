// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TerrainMaterialSystem.h"
#include "AdvancedMaterialFeaturesHelper.generated.h"

/**
 * Blueprint function library for advanced material features
 * Provides convenient access to material system features from Blueprints
 */
UCLASS()
class ALEXANDER_API UAdvancedMaterialFeaturesHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ============================================================================
	// FEATURE CONFIGURATION
	// ============================================================================

	/**
	 * Create default advanced features configuration
	 * @return Default configuration with recommended settings
	 */
	UFUNCTION(BlueprintPure, Category = "Material Features")
	static FAdvancedMaterialFeatures CreateDefaultAdvancedFeatures();

	/**
	 * Create VR-optimized advanced features configuration
	 * @return VR-optimized configuration for 90 FPS target
	 */
	UFUNCTION(BlueprintPure, Category = "Material Features")
	static FAdvancedMaterialFeatures CreateVROptimizedFeatures();

	/**
	 * Create high-quality advanced features configuration
	 * @return High-quality configuration for desktop
	 */
	UFUNCTION(BlueprintPure, Category = "Material Features")
	static FAdvancedMaterialFeatures CreateHighQualityFeatures();

	// ============================================================================
	// QUALITY PRESETS
	// ============================================================================

	/**
	 * Apply quality preset to advanced features
	 * @param Features Features to modify
	 * @param QualityLevel Quality level (0=Low, 1=Medium, 2=High, 3=Epic)
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Features")
	static void ApplyQualityPreset(UPARAM(ref) FAdvancedMaterialFeatures& Features, int32 QualityLevel);

	/**
	 * Get recommended tessellation distance for quality level
	 * @param QualityLevel Quality level (0=Low, 1=Medium, 2=High, 3=Epic)
	 * @return Recommended max tessellation distance in meters
	 */
	UFUNCTION(BlueprintPure, Category = "Material Features")
	static float GetRecommendedTessellationDistance(int32 QualityLevel);

	/**
	 * Get recommended tessellation multiplier for quality level
	 * @param QualityLevel Quality level (0=Low, 1=Medium, 2=High, 3=Epic)
	 * @return Recommended tessellation multiplier
	 */
	UFUNCTION(BlueprintPure, Category = "Material Features")
	static float GetRecommendedTessellationMultiplier(int32 QualityLevel);

	// ============================================================================
	// DISTANCE CALCULATIONS
	// ============================================================================

	/**
	 * Calculate optimal parallax steps for distance
	 * @param ViewDistance Distance from camera in meters
	 * @return Recommended number of parallax steps
	 */
	UFUNCTION(BlueprintPure, Category = "Material Features")
	static int32 CalculateParallaxSteps(float ViewDistance);

	/**
	 * Calculate tessellation factor for distance
	 * @param ViewDistance Distance from camera in meters
	 * @param MaxDistance Maximum tessellation distance
	 * @param Multiplier Base tessellation multiplier
	 * @return Calculated tessellation factor
	 */
	UFUNCTION(BlueprintPure, Category = "Material Features")
	static float CalculateTessellationFactor(float ViewDistance, float MaxDistance, float Multiplier);

	/**
	 * Calculate macro/micro blend factor for distance
	 * @param ViewDistance Distance from camera in meters
	 * @param BlendDistance Distance at which blending starts
	 * @param BlendRange Distance range for smooth transition
	 * @return Blend factor (0=micro, 1=macro)
	 */
	UFUNCTION(BlueprintPure, Category = "Material Features")
	static float CalculateMacroMicroBlend(float ViewDistance, float BlendDistance, float BlendRange);

	// ============================================================================
	// FEATURE ENABLEMENT
	// ============================================================================

	/**
	 * Check if parallax should be enabled at distance
	 * @param ViewDistance Distance from camera in meters
	 * @return True if parallax should be enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Material Features")
	static bool ShouldEnableParallax(float ViewDistance);

	/**
	 * Check if tessellation should be enabled at distance
	 * @param ViewDistance Distance from camera in meters
	 * @param MaxDistance Maximum tessellation distance
	 * @return True if tessellation should be enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Material Features")
	static bool ShouldEnableTessellation(float ViewDistance, float MaxDistance);

	// ============================================================================
	// WETNESS CALCULATIONS
	// ============================================================================

	/**
	 * Calculate wetness amount from weather type
	 * @param WeatherType Current weather type name
	 * @return Wetness amount (0-1)
	 */
	UFUNCTION(BlueprintPure, Category = "Material Features")
	static float CalculateWetnessFromWeather(const FString& WeatherType);

	/**
	 * Calculate rain accumulation based on surface normal
	 * @param WorldNormal Surface normal in world space
	 * @return Rain accumulation factor (0-1)
	 */
	UFUNCTION(BlueprintPure, Category = "Material Features")
	static float CalculateRainAccumulation(FVector WorldNormal);

	/**
	 * Interpolate wetness over time
	 * @param CurrentWetness Current wetness value
	 * @param TargetWetness Target wetness value
	 * @param DeltaTime Time since last update
	 * @param TransitionSpeed Transition speed multiplier
	 * @return New wetness value
	 */
	UFUNCTION(BlueprintPure, Category = "Material Features")
	static float InterpolateWetness(float CurrentWetness, float TargetWetness, float DeltaTime, float TransitionSpeed);

	// ============================================================================
	// PERFORMANCE MONITORING
	// ============================================================================

	/**
	 * Check if current frame rate is acceptable for VR
	 * @param CurrentFPS Current frames per second
	 * @return True if frame rate is acceptable (>= 90 FPS)
	 */
	UFUNCTION(BlueprintPure, Category = "Material Features")
	static bool IsVRFrameRateAcceptable(float CurrentFPS);

	/**
	 * Get performance adjustment factor based on frame rate
	 * @param CurrentFPS Current frames per second
	 * @param TargetFPS Target frames per second
	 * @return Adjustment factor (0.5-1.0, lower = reduce quality)
	 */
	UFUNCTION(BlueprintPure, Category = "Material Features")
	static float GetPerformanceAdjustmentFactor(float CurrentFPS, float TargetFPS);

	/**
	 * Adjust features for performance
	 * @param Features Features to adjust
	 * @param AdjustmentFactor Performance adjustment factor (0.5-1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Features")
	static void AdjustFeaturesForPerformance(UPARAM(ref) FAdvancedMaterialFeatures& Features, float AdjustmentFactor);

	// ============================================================================
	// DEBUGGING
	// ============================================================================

	/**
	 * Get debug string for advanced features
	 * @param Features Features to describe
	 * @return Human-readable description
	 */
	UFUNCTION(BlueprintPure, Category = "Material Features")
	static FString GetAdvancedFeaturesDebugString(const FAdvancedMaterialFeatures& Features);

	/**
	 * Log advanced features configuration
	 * @param Features Features to log
	 * @param Prefix Optional prefix for log message
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Features")
	static void LogAdvancedFeatures(const FAdvancedMaterialFeatures& Features, const FString& Prefix = TEXT(""));
};
