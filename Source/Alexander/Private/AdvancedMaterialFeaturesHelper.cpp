// Copyright Epic Games, Inc. All Rights Reserved.

#include "AdvancedMaterialFeaturesHelper.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Kismet/KismetMathLibrary.h"

FAdvancedMaterialFeatures UAdvancedMaterialFeaturesHelper::CreateDefaultAdvancedFeatures()
{
	FAdvancedMaterialFeatures Features;
	
	// Parallax Occlusion
	Features.bEnableParallaxOcclusion = true;
	Features.ParallaxDepthScale = 0.05f;
	
	// Tessellation
	Features.bEnableTessellation = true;
	Features.TessellationMultiplier = 2.0f;
	Features.MaxTessellationDistance = 50.0f;
	
	// Wetness
	Features.bEnableDynamicWetness = true;
	Features.WetnessIntensity = 0.0f;
	
	// Deformation (future feature)
	Features.bEnableDeformation = false;
	
	return Features;
}

FAdvancedMaterialFeatures UAdvancedMaterialFeaturesHelper::CreateVROptimizedFeatures()
{
	FAdvancedMaterialFeatures Features;
	
	// Parallax Occlusion - Reduced for VR performance
	Features.bEnableParallaxOcclusion = true;
	Features.ParallaxDepthScale = 0.03f; // Reduced depth
	
	// Tessellation - Conservative for VR
	Features.bEnableTessellation = true;
	Features.TessellationMultiplier = 1.5f; // Lower multiplier
	Features.MaxTessellationDistance = 30.0f; // Shorter distance
	
	// Wetness - Full feature, low cost
	Features.bEnableDynamicWetness = true;
	Features.WetnessIntensity = 0.0f;
	
	// Deformation - Disabled for VR
	Features.bEnableDeformation = false;
	
	UE_LOG(LogTemp, Log, TEXT("Created VR-optimized material features"));
	
	return Features;
}

FAdvancedMaterialFeatures UAdvancedMaterialFeaturesHelper::CreateHighQualityFeatures()
{
	FAdvancedMaterialFeatures Features;
	
	// Parallax Occlusion - Maximum quality
	Features.bEnableParallaxOcclusion = true;
	Features.ParallaxDepthScale = 0.08f; // Increased depth
	
	// Tessellation - High quality
	Features.bEnableTessellation = true;
	Features.TessellationMultiplier = 3.0f; // High multiplier
	Features.MaxTessellationDistance = 75.0f; // Extended distance
	
	// Wetness - Full feature
	Features.bEnableDynamicWetness = true;
	Features.WetnessIntensity = 0.0f;
	
	// Deformation - Enabled for high quality
	Features.bEnableDeformation = true;
	
	UE_LOG(LogTemp, Log, TEXT("Created high-quality material features"));
	
	return Features;
}

void UAdvancedMaterialFeaturesHelper::ApplyQualityPreset(FAdvancedMaterialFeatures& Features, int32 QualityLevel)
{
	switch (QualityLevel)
	{
	case 0: // Low
		Features.bEnableParallaxOcclusion = false;
		Features.bEnableTessellation = false;
		Features.bEnableDynamicWetness = true; // Keep wetness, it's cheap
		Features.bEnableDeformation = false;
		UE_LOG(LogTemp, Log, TEXT("Applied Low quality preset"));
		break;

	case 1: // Medium
		Features.bEnableParallaxOcclusion = true;
		Features.ParallaxDepthScale = 0.03f;
		Features.bEnableTessellation = true;
		Features.TessellationMultiplier = 1.0f;
		Features.MaxTessellationDistance = 30.0f;
		Features.bEnableDynamicWetness = true;
		Features.bEnableDeformation = false;
		UE_LOG(LogTemp, Log, TEXT("Applied Medium quality preset"));
		break;

	case 2: // High
		Features.bEnableParallaxOcclusion = true;
		Features.ParallaxDepthScale = 0.05f;
		Features.bEnableTessellation = true;
		Features.TessellationMultiplier = 2.0f;
		Features.MaxTessellationDistance = 50.0f;
		Features.bEnableDynamicWetness = true;
		Features.bEnableDeformation = false;
		UE_LOG(LogTemp, Log, TEXT("Applied High quality preset"));
		break;

	case 3: // Epic
		Features.bEnableParallaxOcclusion = true;
		Features.ParallaxDepthScale = 0.08f;
		Features.bEnableTessellation = true;
		Features.TessellationMultiplier = 3.0f;
		Features.MaxTessellationDistance = 75.0f;
		Features.bEnableDynamicWetness = true;
		Features.bEnableDeformation = true;
		UE_LOG(LogTemp, Log, TEXT("Applied Epic quality preset"));
		break;

	default:
		UE_LOG(LogTemp, Warning, TEXT("Invalid quality level: %d, using Medium"), QualityLevel);
		ApplyQualityPreset(Features, 1);
		break;
	}
}

float UAdvancedMaterialFeaturesHelper::GetRecommendedTessellationDistance(int32 QualityLevel)
{
	switch (QualityLevel)
	{
	case 0: return 0.0f;    // Low - Disabled
	case 1: return 30.0f;   // Medium
	case 2: return 50.0f;   // High
	case 3: return 75.0f;   // Epic
	default: return 50.0f;  // Default to High
	}
}

float UAdvancedMaterialFeaturesHelper::GetRecommendedTessellationMultiplier(int32 QualityLevel)
{
	switch (QualityLevel)
	{
	case 0: return 0.0f;    // Low - Disabled
	case 1: return 1.0f;    // Medium
	case 2: return 2.0f;    // High
	case 3: return 3.0f;    // Epic
	default: return 2.0f;   // Default to High
	}
}

int32 UAdvancedMaterialFeaturesHelper::CalculateParallaxSteps(float ViewDistance)
{
	if (ViewDistance < 10.0f)
	{
		return 32; // High quality for close viewing
	}
	else if (ViewDistance < 25.0f)
	{
		return 16; // Medium quality
	}
	else if (ViewDistance < 50.0f)
	{
		return 8; // Low quality
	}
	else
	{
		return 0; // Disabled beyond 50m
	}
}

float UAdvancedMaterialFeaturesHelper::CalculateTessellationFactor(float ViewDistance, float MaxDistance, float Multiplier)
{
	if (ViewDistance >= MaxDistance)
	{
		return 0.0f;
	}

	// Exponential falloff
	float DistanceFactor = 1.0f - (ViewDistance / MaxDistance);
	DistanceFactor = FMath::Clamp(DistanceFactor, 0.0f, 1.0f);
	DistanceFactor = FMath::Pow(DistanceFactor, 2.0f);

	float TessellationFactor = DistanceFactor * Multiplier;
	return FMath::Clamp(TessellationFactor, 0.0f, 4.0f);
}

float UAdvancedMaterialFeaturesHelper::CalculateMacroMicroBlend(float ViewDistance, float BlendDistance, float BlendRange)
{
	if (ViewDistance <= BlendDistance)
	{
		return 0.0f; // Pure micro
	}

	float DistanceOverThreshold = ViewDistance - BlendDistance;
	float BlendFactor = FMath::Clamp(DistanceOverThreshold / BlendRange, 0.0f, 1.0f);

	// Use smoothstep for smoother transition
	return UKismetMathLibrary::FInterpEaseInOut(0.0f, 1.0f, BlendFactor, 2.0f);
}

bool UAdvancedMaterialFeaturesHelper::ShouldEnableParallax(float ViewDistance)
{
	const float MaxParallaxDistance = 50.0f;
	return ViewDistance < MaxParallaxDistance;
}

bool UAdvancedMaterialFeaturesHelper::ShouldEnableTessellation(float ViewDistance, float MaxDistance)
{
	return ViewDistance < MaxDistance;
}

float UAdvancedMaterialFeaturesHelper::CalculateWetnessFromWeather(const FString& WeatherType)
{
	if (WeatherType.Equals(TEXT("Clear"), ESearchCase::IgnoreCase))
	{
		return 0.0f;
	}
	else if (WeatherType.Equals(TEXT("PartlyCloudy"), ESearchCase::IgnoreCase))
	{
		return 0.0f;
	}
	else if (WeatherType.Equals(TEXT("Overcast"), ESearchCase::IgnoreCase))
	{
		return 0.1f;
	}
	else if (WeatherType.Equals(TEXT("LightRain"), ESearchCase::IgnoreCase))
	{
		return 0.4f;
	}
	else if (WeatherType.Equals(TEXT("HeavyRain"), ESearchCase::IgnoreCase))
	{
		return 0.9f;
	}
	else if (WeatherType.Equals(TEXT("Thunderstorm"), ESearchCase::IgnoreCase))
	{
		return 1.0f;
	}
	else if (WeatherType.Equals(TEXT("LightSnow"), ESearchCase::IgnoreCase))
	{
		return 0.3f;
	}
	else if (WeatherType.Equals(TEXT("HeavySnow"), ESearchCase::IgnoreCase))
	{
		return 0.6f;
	}
	else if (WeatherType.Equals(TEXT("Sandstorm"), ESearchCase::IgnoreCase))
	{
		return 0.0f; // Sandstorms don't make things wet
	}
	else if (WeatherType.Equals(TEXT("Fog"), ESearchCase::IgnoreCase))
	{
		return 0.2f;
	}

	return 0.0f; // Default to dry
}

float UAdvancedMaterialFeaturesHelper::CalculateRainAccumulation(FVector WorldNormal)
{
	// Upward-facing surfaces accumulate more rain
	float UpwardFactor = FMath::Clamp(WorldNormal.Z, 0.0f, 1.0f);
	
	// Apply power to sharpen the effect
	UpwardFactor = FMath::Pow(UpwardFactor, 2.0f);
	
	return UpwardFactor;
}

float UAdvancedMaterialFeaturesHelper::InterpolateWetness(float CurrentWetness, float TargetWetness, float DeltaTime, float TransitionSpeed)
{
	// Faster transition when getting wet, slower when drying
	float Speed = (TargetWetness > CurrentWetness) ? TransitionSpeed * 2.0f : TransitionSpeed;
	
	return FMath::FInterpTo(CurrentWetness, TargetWetness, DeltaTime, Speed);
}

bool UAdvancedMaterialFeaturesHelper::IsVRFrameRateAcceptable(float CurrentFPS)
{
	const float MinVRFPS = 90.0f;
	return CurrentFPS >= MinVRFPS;
}

float UAdvancedMaterialFeaturesHelper::GetPerformanceAdjustmentFactor(float CurrentFPS, float TargetFPS)
{
	if (CurrentFPS >= TargetFPS)
	{
		return 1.0f; // No adjustment needed
	}

	// Calculate how far below target we are
	float Ratio = CurrentFPS / TargetFPS;
	
	// Clamp to reasonable range (0.5 - 1.0)
	return FMath::Clamp(Ratio, 0.5f, 1.0f);
}

void UAdvancedMaterialFeaturesHelper::AdjustFeaturesForPerformance(FAdvancedMaterialFeatures& Features, float AdjustmentFactor)
{
	if (AdjustmentFactor >= 1.0f)
	{
		return; // No adjustment needed
	}

	UE_LOG(LogTemp, Warning, TEXT("Adjusting material features for performance (factor: %.2f)"), AdjustmentFactor);

	// Reduce tessellation distance
	Features.MaxTessellationDistance *= AdjustmentFactor;

	// Reduce tessellation multiplier
	Features.TessellationMultiplier *= AdjustmentFactor;

	// Reduce parallax depth
	Features.ParallaxDepthScale *= AdjustmentFactor;

	// Disable features if adjustment is severe
	if (AdjustmentFactor < 0.7f)
	{
		Features.bEnableParallaxOcclusion = false;
		UE_LOG(LogTemp, Warning, TEXT("Disabled parallax occlusion for performance"));
	}

	if (AdjustmentFactor < 0.6f)
	{
		Features.bEnableTessellation = false;
		UE_LOG(LogTemp, Warning, TEXT("Disabled tessellation for performance"));
	}
}

FString UAdvancedMaterialFeaturesHelper::GetAdvancedFeaturesDebugString(const FAdvancedMaterialFeatures& Features)
{
	FString DebugString = TEXT("Advanced Material Features:\n");
	
	DebugString += FString::Printf(TEXT("  Parallax Occlusion: %s (Depth: %.3f)\n"),
		Features.bEnableParallaxOcclusion ? TEXT("Enabled") : TEXT("Disabled"),
		Features.ParallaxDepthScale);
	
	DebugString += FString::Printf(TEXT("  Tessellation: %s (Mult: %.1f, MaxDist: %.1fm)\n"),
		Features.bEnableTessellation ? TEXT("Enabled") : TEXT("Disabled"),
		Features.TessellationMultiplier,
		Features.MaxTessellationDistance);
	
	DebugString += FString::Printf(TEXT("  Dynamic Wetness: %s (Intensity: %.2f)\n"),
		Features.bEnableDynamicWetness ? TEXT("Enabled") : TEXT("Disabled"),
		Features.WetnessIntensity);
	
	DebugString += FString::Printf(TEXT("  Deformation: %s\n"),
		Features.bEnableDeformation ? TEXT("Enabled") : TEXT("Disabled"));
	
	return DebugString;
}

void UAdvancedMaterialFeaturesHelper::LogAdvancedFeatures(const FAdvancedMaterialFeatures& Features, const FString& Prefix)
{
	FString DebugString = GetAdvancedFeaturesDebugString(Features);
	
	if (!Prefix.IsEmpty())
	{
		DebugString = Prefix + TEXT("\n") + DebugString;
	}
	
	UE_LOG(LogTemp, Log, TEXT("%s"), *DebugString);
}
