// Copyright Epic Games, Inc. All Rights Reserved.

#include "VRPerformanceOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "GameFramework/WorldSettings.h"
#include "RenderingThread.h"
#include "RHICommandList.h"

UVRPerformanceOptimizer::UVRPerformanceOptimizer()
	: CurrentResolutionScale(1.0f)
	, TargetResolutionScale(1.0f)
	, ResolutionScaleVelocity(0.0f)
	, TimeSinceLastOptimization(0.0f)
	, OptimizationInterval(0.5f)
	, LastDrawCallCount(0)
	, FrameHistoryIndex(0)
{
	FrameTimeHistory.SetNum(FrameHistorySize);
	DrawCallHistory.SetNum(30);
	
	for (int32 i = 0; i < FrameHistorySize; ++i)
	{
		FrameTimeHistory[i] = 11.1f; // Initialize to target frame time
	}
	
	for (int32 i = 0; i < DrawCallHistory.Num(); ++i)
	{
		DrawCallHistory[i] = 0;
	}
}

void UVRPerformanceOptimizer::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("VRPerformanceOptimizer: Initializing VR performance optimization system"));
	
	// Initialize with default config
	Config = FVRPerformanceConfig();
	CurrentResolutionScale = 1.0f;
	TargetResolutionScale = 1.0f;
}

void UVRPerformanceOptimizer::Deinitialize()
{
	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PerformanceTickHandle);
	}
	
	Super::Deinitialize();
}

void UVRPerformanceOptimizer::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	UE_LOG(LogTemp, Log, TEXT("VRPerformanceOptimizer: Starting performance optimization"));
	
	// Apply initial console commands for VR optimization
	ApplyConsoleCommands();
	
	// Start performance monitoring tick
	InWorld.GetTimerManager().SetTimer(
		PerformanceTickHandle,
		FTimerDelegate::CreateUObject(this, &UVRPerformanceOptimizer::TickPerformanceOptimization, 0.016f),
		0.016f, // Tick every frame (~60Hz, will adapt)
		true
	);
	
	// Apply initial optimizations
	OptimizeLODSettings();
	OptimizeCullingSettings();
}

void UVRPerformanceOptimizer::SetPerformanceConfig(const FVRPerformanceConfig& NewConfig)
{
	Config = NewConfig;
	
	UE_LOG(LogTemp, Log, TEXT("VRPerformanceOptimizer: Updated configuration - Target FPS: %.1f, Max Draw Calls: %d"),
		Config.TargetFPS, Config.TargetMaxDrawCalls);
	
	// Reapply optimizations with new config
	OptimizeLODSettings();
	OptimizeCullingSettings();
	ApplyConsoleCommands();
}

void UVRPerformanceOptimizer::SetDynamicResolutionEnabled(bool bEnabled)
{
	Config.bEnableDynamicResolution = bEnabled;
	
	if (!bEnabled)
	{
		// Reset to 100% resolution
		SetResolutionScale(1.0f);
	}
	
	UE_LOG(LogTemp, Log, TEXT("VRPerformanceOptimizer: Dynamic resolution %s"),
		bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UVRPerformanceOptimizer::SetResolutionScale(float Scale)
{
	Scale = FMath::Clamp(Scale, Config.MinResolutionScale, Config.MaxResolutionScale);
	CurrentResolutionScale = Scale;
	TargetResolutionScale = Scale;
	
	// Apply resolution scale via console command
	if (GEngine)
	{
		GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.ScreenPercentage %f"), Scale * 100.0f));
	}
	
	CurrentMetrics.CurrentResolutionScale = Scale;
	
	UE_LOG(LogTemp, Log, TEXT("VRPerformanceOptimizer: Resolution scale set to %.2f%%"), Scale * 100.0f);
}

void UVRPerformanceOptimizer::OptimizeLODDistances(float DistanceScale)
{
	Config.LODDistanceScale = FMath::Clamp(DistanceScale, 0.5f, 2.0f);
	UpdateLODDistanceScale();
}

void UVRPerformanceOptimizer::ForceGarbageCollection()
{
	if (GEngine)
	{
		GEngine->ForceGarbageCollection(true);
		UE_LOG(LogTemp, Log, TEXT("VRPerformanceOptimizer: Forced garbage collection"));
	}
}

void UVRPerformanceOptimizer::TickPerformanceOptimization(float DeltaTime)
{
	// Update performance metrics
	UpdatePerformanceMetrics();
	CalculateFrameStats(DeltaTime);
	
	// Update dynamic resolution if enabled
	if (Config.bEnableDynamicResolution)
	{
		UpdateDynamicResolution(DeltaTime);
	}
	
	// Periodic optimization updates
	TimeSinceLastOptimization += DeltaTime;
	if (TimeSinceLastOptimization >= OptimizationInterval)
	{
		TimeSinceLastOptimization = 0.0f;
		
		OptimizeDrawCalls();
		
		// Update LOD settings if performance is poor
		if (!CurrentMetrics.bMeetingPerformanceTarget && Config.bEnableAggressiveLOD)
		{
			UpdateLODDistanceScale();
		}
	}
}

void UVRPerformanceOptimizer::UpdatePerformanceMetrics()
{
	// Calculate current FPS
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	if (DeltaTime > 0.0f)
	{
		CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
	}
	
	// Store frame time in history
	FrameTimeHistory[FrameHistoryIndex] = DeltaTime * 1000.0f; // Convert to milliseconds
	FrameHistoryIndex = (FrameHistoryIndex + 1) % FrameHistorySize;
	
	// Calculate average and max frame times
	CurrentMetrics.AverageFrameTime = CalculateAverageFrameTime();
	CurrentMetrics.MaxFrameTime = CalculateMaxFrameTime();
	
	// Get draw call count (approximation via stats)
	// In a real implementation, you'd query RHI stats
	CurrentMetrics.DrawCalls = LastDrawCallCount;
	
	// Determine if meeting performance target
	CurrentMetrics.bMeetingPerformanceTarget = 
		CurrentMetrics.CurrentFPS >= Config.TargetFPS &&
		CurrentMetrics.AverageFrameTime <= Config.MaxFrameTimeMS &&
		CurrentMetrics.DrawCalls <= Config.TargetMaxDrawCalls;
}

void UVRPerformanceOptimizer::CalculateFrameStats(float DeltaTime)
{
	// Track draw calls over time
	static int32 DrawCallHistoryIndex = 0;
	DrawCallHistory[DrawCallHistoryIndex] = CurrentMetrics.DrawCalls;
	DrawCallHistoryIndex = (DrawCallHistoryIndex + 1) % DrawCallHistory.Num();
}

void UVRPerformanceOptimizer::UpdateDynamicResolution(float DeltaTime)
{
	// Determine target resolution based on performance
	float PerformanceRatio = CurrentMetrics.CurrentFPS / Config.TargetFPS;
	
	if (PerformanceRatio < 0.95f) // Below target, reduce resolution
	{
		// Calculate how much to reduce
		float ReductionFactor = FMath::Clamp(1.0f - (0.95f - PerformanceRatio) * 2.0f, 0.0f, 1.0f);
		TargetResolutionScale = CurrentResolutionScale * ReductionFactor;
		TargetResolutionScale = FMath::Max(TargetResolutionScale, Config.MinResolutionScale);
	}
	else if (PerformanceRatio > 1.05f) // Above target, can increase resolution
	{
		// Gradually increase resolution
		float IncreaseFactor = FMath::Clamp((PerformanceRatio - 1.05f) * 0.5f, 0.0f, 0.1f);
		TargetResolutionScale = CurrentResolutionScale + IncreaseFactor;
		TargetResolutionScale = FMath::Min(TargetResolutionScale, Config.MaxResolutionScale);
	}
	
	// Smoothly adjust to target resolution
	AdjustResolutionScale(TargetResolutionScale, DeltaTime);
}

void UVRPerformanceOptimizer::AdjustResolutionScale(float TargetScale, float DeltaTime)
{
	// Smooth interpolation to target scale
	float ScaleDifference = TargetScale - CurrentResolutionScale;
	
	if (FMath::Abs(ScaleDifference) > 0.01f)
	{
		// Use spring-damper for smooth transitions
		float SpringForce = ScaleDifference * Config.ResolutionAdjustmentSpeed;
		float DampingForce = -ResolutionScaleVelocity * 0.5f;
		
		ResolutionScaleVelocity += (SpringForce + DampingForce) * DeltaTime;
		CurrentResolutionScale += ResolutionScaleVelocity * DeltaTime;
		
		// Clamp to valid range
		CurrentResolutionScale = FMath::Clamp(CurrentResolutionScale, Config.MinResolutionScale, Config.MaxResolutionScale);
		
		// Apply the new resolution scale
		SetResolutionScale(CurrentResolutionScale);
	}
}

void UVRPerformanceOptimizer::OptimizeLODSettings()
{
	if (!Config.bEnableAggressiveLOD)
	{
		return;
	}
	
	UpdateLODDistanceScale();
	
	UE_LOG(LogTemp, Log, TEXT("VRPerformanceOptimizer: LOD settings optimized"));
}

void UVRPerformanceOptimizer::UpdateLODDistanceScale()
{
	if (GEngine)
	{
		// Adjust LOD distance scale
		GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.LODDistanceScale %f"), Config.LODDistanceScale));
		
		// Enable aggressive LOD for VR
		GEngine->Exec(GetWorld(), TEXT("r.StaticMeshLODDistanceScale 0.8"));
		GEngine->Exec(GetWorld(), TEXT("r.SkeletalMeshLODBias 1"));
	}
}

void UVRPerformanceOptimizer::OptimizeCullingSettings()
{
	if (!GEngine)
	{
		return;
	}
	
	// Enable occlusion culling
	if (Config.bEnableOcclusionCulling)
	{
		GEngine->Exec(GetWorld(), TEXT("r.AllowOcclusionQueries 1"));
		GEngine->Exec(GetWorld(), TEXT("r.HZBOcclusion 1"));
	}
	
	// Set view distance culling
	if (Config.bEnableDistanceCulling)
	{
		if (AWorldSettings* WorldSettings = GetWorld()->GetWorldSettings())
		{
			// This would typically be set per-actor, but we can set a global hint
			GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.ViewDistanceScale %f"), 
				Config.MaxViewDistance / 50000.0f));
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("VRPerformanceOptimizer: Culling settings optimized"));
}

void UVRPerformanceOptimizer::UpdateViewDistanceCulling()
{
	// Update view distance based on performance
	if (Config.bEnableDistanceCulling && GEngine)
	{
		float DistanceScale = CurrentMetrics.bMeetingPerformanceTarget ? 1.0f : 0.7f;
		GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.ViewDistanceScale %f"), DistanceScale));
	}
}

void UVRPerformanceOptimizer::OptimizeDrawCalls()
{
	AnalyzeDrawCallBudget();
	
	// If draw calls are too high, apply additional optimizations
	if (CurrentMetrics.DrawCalls > Config.TargetMaxDrawCalls)
	{
		UE_LOG(LogTemp, Warning, TEXT("VRPerformanceOptimizer: Draw calls (%d) exceed target (%d)"),
			CurrentMetrics.DrawCalls, Config.TargetMaxDrawCalls);
		
		// Increase LOD bias to reduce complexity
		if (Config.bEnableAggressiveLOD)
		{
			Config.LODDistanceScale = FMath::Max(Config.LODDistanceScale - 0.1f, 0.5f);
			UpdateLODDistanceScale();
		}
	}
}

void UVRPerformanceOptimizer::AnalyzeDrawCallBudget()
{
	// Calculate average draw calls
	int32 TotalDrawCalls = 0;
	for (int32 DrawCalls : DrawCallHistory)
	{
		TotalDrawCalls += DrawCalls;
	}
	
	int32 AverageDrawCalls = TotalDrawCalls / FMath::Max(1, DrawCallHistory.Num());
	LastDrawCallCount = AverageDrawCalls;
}

float UVRPerformanceOptimizer::CalculateAverageFrameTime() const
{
	float Total = 0.0f;
	for (float FrameTime : FrameTimeHistory)
	{
		Total += FrameTime;
	}
	return Total / FrameHistorySize;
}

float UVRPerformanceOptimizer::CalculateMaxFrameTime() const
{
	float MaxTime = 0.0f;
	for (float FrameTime : FrameTimeHistory)
	{
		MaxTime = FMath::Max(MaxTime, FrameTime);
	}
	return MaxTime;
}

bool UVRPerformanceOptimizer::IsPerformanceBelowTarget() const
{
	return CurrentMetrics.CurrentFPS < Config.TargetFPS ||
		   CurrentMetrics.AverageFrameTime > Config.MaxFrameTimeMS;
}

void UVRPerformanceOptimizer::ApplyConsoleCommands()
{
	if (!GEngine)
	{
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("VRPerformanceOptimizer: Applying VR optimization console commands"));
	
	// VR-specific optimizations
	GEngine->Exec(GetWorld(), TEXT("vr.InstancedStereo 1"));
	GEngine->Exec(GetWorld(), TEXT("vr.MobileMultiView 1"));
	GEngine->Exec(GetWorld(), TEXT("vr.RoundRobinOcclusion 1"));
	
	// Frame timing consistency
	GEngine->Exec(GetWorld(), TEXT("r.VSync 0")); // VR handles its own vsync
	GEngine->Exec(GetWorld(), TEXT("r.FinishCurrentFrame 1"));
	
	// Shadow optimizations
	GEngine->Exec(GetWorld(), TEXT("r.Shadow.MaxResolution 1024"));
	GEngine->Exec(GetWorld(), TEXT("r.Shadow.DistanceScale 0.6"));
	GEngine->Exec(GetWorld(), TEXT("r.Shadow.RadiusThreshold 0.03"));
	
	// Texture streaming
	GEngine->Exec(GetWorld(), TEXT("r.Streaming.PoolSize 2000"));
	GEngine->Exec(GetWorld(), TEXT("r.Streaming.MaxEffectiveScreenSize 0"));
	
	// Post-processing optimizations
	GEngine->Exec(GetWorld(), TEXT("r.MotionBlurQuality 0"));
	GEngine->Exec(GetWorld(), TEXT("r.DepthOfFieldQuality 0"));
	GEngine->Exec(GetWorld(), TEXT("r.BloomQuality 3"));
	
	// Particle optimizations
	GEngine->Exec(GetWorld(), TEXT("fx.MaxCPUParticlesPerEmitter 500"));
	GEngine->Exec(GetWorld(), TEXT("fx.MaxGPUParticlesSpawnedPerFrame 32768"));
	
	// Mesh optimizations
	GEngine->Exec(GetWorld(), TEXT("r.MeshDrawCommands.ParallelPassSetup 1"));
	GEngine->Exec(GetWorld(), TEXT("r.RHICmdBypass 0"));
	
	// Culling optimizations
	GEngine->Exec(GetWorld(), TEXT("r.AllowOcclusionQueries 1"));
	GEngine->Exec(GetWorld(), TEXT("r.HZBOcclusion 1"));
	GEngine->Exec(GetWorld(), TEXT("r.HZBOcclusion.HardwareDepthFetch 1"));
	
	// LOD settings
	GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.LODDistanceScale %f"), Config.LODDistanceScale));
	
	// Dynamic resolution
	if (Config.bEnableDynamicResolution)
	{
		GEngine->Exec(GetWorld(), TEXT("r.DynamicRes.OperationMode 2")); // 2 = Enabled
		GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.DynamicRes.MinScreenPercentage %f"), 
			Config.MinResolutionScale * 100.0f));
		GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.DynamicRes.MaxScreenPercentage %f"), 
			Config.MaxResolutionScale * 100.0f));
	}
	
	UE_LOG(LogTemp, Log, TEXT("VRPerformanceOptimizer: Console commands applied successfully"));
}
