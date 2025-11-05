// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VRPerformanceOptimizer.generated.h"

/**
 * Performance metrics tracked per frame
 */
USTRUCT(BlueprintType)
struct FVRPerformanceMetrics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float CurrentFPS;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float AverageFrameTime;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float MaxFrameTime;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	int32 DrawCalls;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	int32 VisibleTriangles;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float CurrentResolutionScale;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	bool bMeetingPerformanceTarget;

	FVRPerformanceMetrics()
		: CurrentFPS(90.0f)
		, AverageFrameTime(11.1f)
		, MaxFrameTime(11.1f)
		, DrawCalls(0)
		, VisibleTriangles(0)
		, CurrentResolutionScale(1.0f)
		, bMeetingPerformanceTarget(true)
	{}
};

/**
 * Configuration for VR performance optimization
 */
USTRUCT(BlueprintType)
struct FVRPerformanceConfig
{
	GENERATED_BODY()

	// Target frame rate for VR (typically 90 or 120 FPS)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float TargetFPS;

	// Maximum allowed frame time in milliseconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float MaxFrameTimeMS;

	// Target maximum draw calls per frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 TargetMaxDrawCalls;

	// Enable dynamic resolution scaling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resolution")
	bool bEnableDynamicResolution;

	// Minimum resolution scale (0.5 = 50%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resolution", meta = (ClampMin = "0.3", ClampMax = "1.0"))
	float MinResolutionScale;

	// Maximum resolution scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resolution", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float MaxResolutionScale;

	// How quickly to adjust resolution (higher = faster)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resolution", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float ResolutionAdjustmentSpeed;

	// Enable aggressive LOD optimization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	bool bEnableAggressiveLOD;

	// LOD distance scale multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float LODDistanceScale;

	// Enable occlusion culling optimizations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
	bool bEnableOcclusionCulling;

	// Enable distance culling for far objects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
	bool bEnableDistanceCulling;

	// Maximum view distance in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
	float MaxViewDistance;

	FVRPerformanceConfig()
		: TargetFPS(90.0f)
		, MaxFrameTimeMS(11.1f)
		, TargetMaxDrawCalls(5000)
		, bEnableDynamicResolution(true)
		, MinResolutionScale(0.5f)
		, MaxResolutionScale(1.5f)
		, ResolutionAdjustmentSpeed(0.3f)
		, bEnableAggressiveLOD(true)
		, LODDistanceScale(1.0f)
		, bEnableOcclusionCulling(true)
		, bEnableDistanceCulling(true)
		, MaxViewDistance(50000.0f)
	{}
};

/**
 * World subsystem that manages VR performance optimization
 * Ensures 90 FPS minimum, optimizes frame timing, reduces draw calls, and implements dynamic resolution
 */
UCLASS()
class ALEXANDER_API UVRPerformanceOptimizer : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UVRPerformanceOptimizer();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// Performance monitoring
	UFUNCTION(BlueprintCallable, Category = "VR Performance")
	FVRPerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

	UFUNCTION(BlueprintCallable, Category = "VR Performance")
	bool IsMeetingPerformanceTarget() const { return CurrentMetrics.bMeetingPerformanceTarget; }

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "VR Performance")
	void SetPerformanceConfig(const FVRPerformanceConfig& NewConfig);

	UFUNCTION(BlueprintCallable, Category = "VR Performance")
	FVRPerformanceConfig GetPerformanceConfig() const { return Config; }

	// Manual optimization controls
	UFUNCTION(BlueprintCallable, Category = "VR Performance")
	void SetDynamicResolutionEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "VR Performance")
	void SetResolutionScale(float Scale);

	UFUNCTION(BlueprintCallable, Category = "VR Performance")
	void OptimizeLODDistances(float DistanceScale);

	UFUNCTION(BlueprintCallable, Category = "VR Performance")
	void ForceGarbageCollection();

protected:
	// Core update function
	void TickPerformanceOptimization(float DeltaTime);

	// Performance monitoring
	void UpdatePerformanceMetrics();
	void CalculateFrameStats(float DeltaTime);

	// Dynamic resolution scaling
	void UpdateDynamicResolution(float DeltaTime);
	void AdjustResolutionScale(float TargetScale, float DeltaTime);

	// LOD optimization
	void OptimizeLODSettings();
	void UpdateLODDistanceScale();

	// Culling optimization
	void OptimizeCullingSettings();
	void UpdateViewDistanceCulling();

	// Draw call optimization
	void OptimizeDrawCalls();
	void AnalyzeDrawCallBudget();

private:
	UPROPERTY()
	FVRPerformanceConfig Config;

	UPROPERTY()
	FVRPerformanceMetrics CurrentMetrics;

	// Frame timing tracking
	TArray<float> FrameTimeHistory;
	static constexpr int32 FrameHistorySize = 60;
	int32 FrameHistoryIndex;

	// Resolution scaling state
	float CurrentResolutionScale;
	float TargetResolutionScale;
	float ResolutionScaleVelocity;

	// Performance tracking
	float TimeSinceLastOptimization;
	float OptimizationInterval;

	// Draw call tracking
	int32 LastDrawCallCount;
	TArray<int32> DrawCallHistory;

	// Timer handle for tick
	FTimerHandle PerformanceTickHandle;

	// Helper functions
	float CalculateAverageFrameTime() const;
	float CalculateMaxFrameTime() const;
	bool IsPerformanceBelowTarget() const;
	void ApplyConsoleCommands();
};
