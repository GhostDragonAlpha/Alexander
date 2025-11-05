// Copyright Epic Games, Inc. All Rights Reserved.

#include "ScalingCalculator.h"
#include "AstronomicalConstants.h"
#include "Math/UnrealMathUtility.h"

void UScalingCalculator::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default values
	CurrentScalingMethod = EScalingMethod::InverseSquare;
	ReferenceDistance = 1000000.0; // 10 km in cm
	MinScaleFactor = 0.001; // 0.1% minimum size
	MaxScaleFactor = 100.0; // 10000% maximum size
	InverseSquareExponent = 2.0;
	LogarithmicBase = 10.0;
	DefaultTransitionSpeed = 5.0f;
	CacheQuantizationFactor = 10000.0; // Quantize to nearest 100 meters
	MaxCacheSize = 10000;
	bEnableDebugLogging = false;

	// Initialize statistics
	CacheLookups = 0;
	CacheHits = 0;

	// Clear cache
	ScaleFactorCache.Empty();

	UE_LOG(LogTemp, Log, TEXT("ScalingCalculator: Initialized with method %d, reference distance %.2f"),
		static_cast<int32>(CurrentScalingMethod), ReferenceDistance);
}

void UScalingCalculator::Deinitialize()
{
	// Log cache statistics before cleanup
	if (bEnableDebugLogging)
	{
		int32 CacheSize = 0;
		float HitRate = 0.0f;
		GetCacheStatistics(CacheSize, HitRate);
		UE_LOG(LogTemp, Log, TEXT("ScalingCalculator: Final cache statistics - Size: %d, Hit Rate: %.2f%%"),
			CacheSize, HitRate * 100.0f);
	}

	// Clear cache
	{
		FScopeLock Lock(&CacheLock);
		ScaleFactorCache.Empty();
	}

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("ScalingCalculator: Deinitialized"));
}

// ========== Scale Calculation ==========

double UScalingCalculator::CalculateScaleFactor(double Distance, double InReferenceDistance) const
{
	// Validate inputs
	if (Distance <= 0.0 || InReferenceDistance <= 0.0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScalingCalculator: Invalid distance parameters (Distance: %.2f, Reference: %.2f)"),
			Distance, InReferenceDistance);
		return 1.0;
	}

	// Check cache first
	int32 CacheKey = DistanceToCacheKey(Distance);
	double CachedScale = 0.0;

	{
		FScopeLock Lock(&CacheLock);
		CacheLookups++;

		if (ScaleFactorCache.Contains(CacheKey))
		{
			CacheHits++;
			CachedScale = ScaleFactorCache[CacheKey];

			if (bEnableDebugLogging)
			{
				UE_LOG(LogTemp, Verbose, TEXT("ScalingCalculator: Cache hit for distance %.2f (key %d) = %.6f"),
					Distance, CacheKey, CachedScale);
			}

			return CachedScale;
		}
	}

	// Calculate scale factor based on method
	double ScaleFactor = CalculateScaleInternal(Distance / InReferenceDistance, CurrentScalingMethod);

	// Clamp to configured limits
	ScaleFactor = ClampScaleFactor(ScaleFactor, MinScaleFactor, MaxScaleFactor);

	// Cache the result
	const_cast<UScalingCalculator*>(this)->CacheScaleFactor(CacheKey, ScaleFactor);

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Verbose, TEXT("ScalingCalculator: Calculated scale %.6f for distance %.2f (method: %d)"),
			ScaleFactor, Distance, static_cast<int32>(CurrentScalingMethod));
	}

	return ScaleFactor;
}

double UScalingCalculator::ApplyInverseSquareLaw(double Distance) const
{
	if (Distance <= 0.0)
	{
		return MaxScaleFactor;
	}

	double NormalizedDistance = Distance / ReferenceDistance;
	double ScaleFactor = FMath::Pow(1.0 / NormalizedDistance, InverseSquareExponent);

	return ClampScaleFactor(ScaleFactor, MinScaleFactor, MaxScaleFactor);
}

double UScalingCalculator::ApplyLinearScaling(double Distance) const
{
	if (Distance <= 0.0)
	{
		return MaxScaleFactor;
	}

	double NormalizedDistance = Distance / ReferenceDistance;
	double ScaleFactor = 1.0 / NormalizedDistance;

	return ClampScaleFactor(ScaleFactor, MinScaleFactor, MaxScaleFactor);
}

double UScalingCalculator::ApplyLogarithmicScaling(double Distance) const
{
	if (Distance <= 1.0)
	{
		return MaxScaleFactor;
	}

	double NormalizedDistance = Distance / ReferenceDistance;

	// Logarithmic scaling: Log(Reference) / Log(Distance)
	double LogReference = FMath::LogX(LogarithmicBase, FMath::Max(1.0, ReferenceDistance));
	double LogDistance = FMath::LogX(LogarithmicBase, FMath::Max(1.0, Distance));

	double ScaleFactor = LogReference / LogDistance;

	return ClampScaleFactor(ScaleFactor, MinScaleFactor, MaxScaleFactor);
}

// ========== Smooth Transitions ==========

double UScalingCalculator::SmoothScaleTransition(double CurrentScale, double TargetScale, float DeltaTime, float TransitionSpeed) const
{
	if (FMath::IsNearlyEqual(CurrentScale, TargetScale, 0.0001))
	{
		return TargetScale;
	}

	// Use exponential interpolation for smooth transitions
	float Alpha = FMath::Clamp(DeltaTime * TransitionSpeed, 0.0f, 1.0f);
	double NewScale = FMath::Lerp(CurrentScale, TargetScale, static_cast<double>(Alpha));

	return NewScale;
}

double UScalingCalculator::ClampScaleFactor(double ScaleFactor, double MinScale, double MaxScale) const
{
	// Use provided limits or default to configured limits
	double ActualMin = (MinScale > 0.0) ? MinScale : MinScaleFactor;
	double ActualMax = (MaxScale > 0.0) ? MaxScale : MaxScaleFactor;

	return FMath::Clamp(ScaleFactor, ActualMin, ActualMax);
}

// ========== Optimization ==========

bool UScalingCalculator::ShouldUpdateScale(double OldDistance, double NewDistance, double Threshold) const
{
	if (OldDistance <= 0.0 || NewDistance <= 0.0)
	{
		return true;
	}

	// Calculate relative change
	double RelativeChange = FMath::Abs(NewDistance - OldDistance) / OldDistance;

	return RelativeChange >= Threshold;
}

bool UScalingCalculator::GetCachedScaleFactor(int32 DistanceKey, double& OutScaleFactor) const
{
	FScopeLock Lock(&CacheLock);

	if (const double* CachedValue = ScaleFactorCache.Find(DistanceKey))
	{
		OutScaleFactor = *CachedValue;
		return true;
	}

	return false;
}

void UScalingCalculator::CacheScaleFactor(int32 DistanceKey, double ScaleFactor)
{
	FScopeLock Lock(&CacheLock);

	// Check if cache needs cleanup
	if (ScaleFactorCache.Num() >= MaxCacheSize)
	{
		CleanupCache();
	}

	ScaleFactorCache.Add(DistanceKey, ScaleFactor);

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Verbose, TEXT("ScalingCalculator: Cached scale %.6f for key %d (cache size: %d)"),
			ScaleFactor, DistanceKey, ScaleFactorCache.Num());
	}
}

void UScalingCalculator::ClearCache()
{
	FScopeLock Lock(&CacheLock);

	int32 OldSize = ScaleFactorCache.Num();
	ScaleFactorCache.Empty();
	CacheLookups = 0;
	CacheHits = 0;

	UE_LOG(LogTemp, Log, TEXT("ScalingCalculator: Cache cleared (%d entries removed)"), OldSize);
}

int32 UScalingCalculator::DistanceToCacheKey(double Distance) const
{
	// Quantize distance to reduce cache size
	return FMath::FloorToInt(Distance / CacheQuantizationFactor);
}

// ========== Configuration ==========

void UScalingCalculator::SetScalingMethod(EScalingMethod Method)
{
	if (CurrentScalingMethod != Method)
	{
		CurrentScalingMethod = Method;

		// Clear cache when method changes
		ClearCache();

		UE_LOG(LogTemp, Log, TEXT("ScalingCalculator: Scaling method changed to %d"), static_cast<int32>(Method));
	}
}

void UScalingCalculator::SetReferenceDistance(double Distance)
{
	if (!FMath::IsNearlyEqual(ReferenceDistance, Distance))
	{
		ReferenceDistance = FMath::Max(Distance, 1.0);

		// Clear cache when reference distance changes
		ClearCache();

		UE_LOG(LogTemp, Log, TEXT("ScalingCalculator: Reference distance changed to %.2f"), ReferenceDistance);
	}
}

void UScalingCalculator::SetScaleLimits(double Min, double Max)
{
	MinScaleFactor = FMath::Max(Min, 0.0001);
	MaxScaleFactor = FMath::Max(Max, MinScaleFactor);

	// Clear cache when limits change
	ClearCache();

	UE_LOG(LogTemp, Log, TEXT("ScalingCalculator: Scale limits set to [%.6f, %.2f]"), MinScaleFactor, MaxScaleFactor);
}

// ========== Network Sync ==========

TArray<double> UScalingCalculator::CalculateScaleFactorsForDistances(const TArray<double>& Distances)
{
	TArray<double> ScaleFactors;
	ScaleFactors.Reserve(Distances.Num());

	for (double Distance : Distances)
	{
		double ScaleFactor = CalculateScaleFactor(Distance, ReferenceDistance);
		ScaleFactors.Add(ScaleFactor);
	}

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Verbose, TEXT("ScalingCalculator: Calculated %d scale factors for network sync"),
			ScaleFactors.Num());
	}

	return ScaleFactors;
}

// ========== Debug ==========

void UScalingCalculator::GetCacheStatistics(int32& OutCacheSize, float& OutHitRate) const
{
	FScopeLock Lock(&CacheLock);

	OutCacheSize = ScaleFactorCache.Num();

	if (CacheLookups > 0)
	{
		OutHitRate = static_cast<float>(CacheHits) / static_cast<float>(CacheLookups);
	}
	else
	{
		OutHitRate = 0.0f;
	}
}

// ========== Internal Methods ==========

void UScalingCalculator::CleanupCache()
{
	// Remove oldest 25% of entries
	int32 NumToRemove = ScaleFactorCache.Num() / 4;

	if (NumToRemove > 0)
	{
		// Simple cleanup: remove arbitrary entries
		// In a more sophisticated implementation, could use LRU or other strategies
		TArray<int32> Keys;
		ScaleFactorCache.GetKeys(Keys);

		for (int32 i = 0; i < NumToRemove && i < Keys.Num(); ++i)
		{
			ScaleFactorCache.Remove(Keys[i]);
		}

		if (bEnableDebugLogging)
		{
			UE_LOG(LogTemp, Verbose, TEXT("ScalingCalculator: Cache cleanup removed %d entries (new size: %d)"),
				NumToRemove, ScaleFactorCache.Num());
		}
	}
}

double UScalingCalculator::CalculateScaleInternal(double NormalizedDistance, EScalingMethod Method) const
{
	switch (Method)
	{
	case EScalingMethod::Linear:
		return 1.0 / NormalizedDistance;

	case EScalingMethod::InverseSquare:
		return FMath::Pow(1.0 / NormalizedDistance, InverseSquareExponent);

	case EScalingMethod::Logarithmic:
	{
		double LogBase = FMath::LogX(LogarithmicBase, FMath::Max(1.0, 1.0));
		double LogDist = FMath::LogX(LogarithmicBase, FMath::Max(1.0, NormalizedDistance));
		return (LogDist > 0.0) ? (LogBase / LogDist) : 1.0;
	}

	case EScalingMethod::Custom:
		// Custom method could be implemented via curve or user callback
		return 1.0 / NormalizedDistance;

	default:
		return 1.0;
	}
}
