// Copyright Epic Games, Inc. All Rights Reserved.

#include "PerformanceTestingSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformMemory.h"
#include "Stats/StatsData.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"
#include "IXRTrackingSystem.h"
#include "IHeadMountedDisplay.h"

void UPerformanceTestingSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("PerformanceTestingSystem initialized"));
}

void UPerformanceTestingSystem::Deinitialize()
{
	if (bIsTestRunning)
	{
		StopPerformanceTest();
	}
	
	Super::Deinitialize();
}

bool UPerformanceTestingSystem::StartPerformanceTest(const FPerformanceTestConfig& Config)
{
	if (bIsTestRunning)
	{
		UE_LOG(LogTemp, Warning, TEXT("Performance test already running"));
		return false;
	}

	if (Config.TestAltitudes.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No test altitudes specified"));
		return false;
	}

	// Initialize test state
	CurrentConfig = Config;
	TestResults = FPerformanceTestResults();
	TestResults.TestStartTime = FDateTime::Now();
	CurrentAltitudeIndex = 0;
	CurrentSampleIndex = 0;
	SampleTimer = 0.0f;
	LastStreamingBytes = 0.0f;
	LastStreamingCheckTime = FDateTime::Now();
	bIsTestRunning = true;

	// Reserve space for samples
	TestResults.Samples.Reserve(Config.TestAltitudes.Num() * Config.SamplesPerAltitude);
	if (Config.bTestVRPerformance)
	{
		TestResults.VRSamples.Reserve(Config.TestAltitudes.Num() * Config.SamplesPerAltitude);
	}

	UE_LOG(LogTemp, Log, TEXT("Performance test started with %d altitudes, %d samples per altitude"),
		Config.TestAltitudes.Num(), Config.SamplesPerAltitude);

	// Start first altitude test
	StartAltitudeTest(Config.TestAltitudes[0]);

	return true;
}

void UPerformanceTestingSystem::StopPerformanceTest()
{
	if (!bIsTestRunning)
	{
		return;
	}

	CompleteTest();
	bIsTestRunning = false;

	UE_LOG(LogTemp, Log, TEXT("Performance test stopped"));
}

float UPerformanceTestingSystem::GetTestProgress() const
{
	if (!bIsTestRunning || CurrentConfig.TestAltitudes.Num() == 0)
	{
		return 0.0f;
	}

	float AltitudeProgress = static_cast<float>(CurrentAltitudeIndex) / CurrentConfig.TestAltitudes.Num();
	float SampleProgress = static_cast<float>(CurrentSampleIndex) / CurrentConfig.SamplesPerAltitude;
	float CurrentAltitudeWeight = 1.0f / CurrentConfig.TestAltitudes.Num();

	return AltitudeProgress + (SampleProgress * CurrentAltitudeWeight);
}

FPerformanceMetrics UPerformanceTestingSystem::CapturePerformanceMetrics()
{
	FPerformanceMetrics Metrics;

	// Get player altitude
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				Metrics.Altitude = Pawn->GetActorLocation().Z;
			}
		}
	}

	// Capture performance metrics
	Metrics.FrameRate = MeasureFrameRate();
	Metrics.FrameTime = MeasureFrameTime();
	Metrics.GameThreadTime = MeasureGameThreadTime();
	Metrics.RenderThreadTime = MeasureRenderThreadTime();
	Metrics.GPUTime = MeasureGPUTime();
	Metrics.DrawCalls = MeasureDrawCalls();
	Metrics.VisibleTriangles = MeasureVisibleTriangles();
	Metrics.MemoryUsageMB = MeasureMemoryUsage();
	Metrics.StreamingBandwidthMBps = MeasureStreamingBandwidth();
	Metrics.ActiveTerrainTiles = CountActiveTerrainTiles();
	Metrics.StreamingTiles = CountStreamingTiles();
	Metrics.Timestamp = FDateTime::Now();

	return Metrics;
}

FVRPerformanceMetrics UPerformanceTestingSystem::CaptureVRPerformanceMetrics()
{
	FVRPerformanceMetrics Metrics;

	// Check if VR is active
	if (GEngine && GEngine->XRSystem.IsValid())
	{
		IXRTrackingSystem* XRSystem = GEngine->XRSystem.Get();
		
		if (XRSystem && XRSystem->IsHeadTrackingAllowed())
		{
			// Get VR-specific metrics
			Metrics.MotionToPhotonLatency = MeasureMotionToPhotonLatency();
			Metrics.DroppedFrames = CountDroppedFrames();
			Metrics.bIsReprojecting = IsReprojecting();
			Metrics.HeadsetFPS = GetHeadsetFPS();

			// Get IPD
			if (IHeadMountedDisplay* HMD = XRSystem->GetHMDDevice())
			{
				Metrics.IPD = HMD->GetInterpupillaryDistance();
			}

			// Estimate per-eye frame times (approximate)
			float TotalFrameTime = MeasureFrameTime();
			Metrics.LeftEyeFrameTime = TotalFrameTime * 0.5f;
			Metrics.RightEyeFrameTime = TotalFrameTime * 0.5f;
		}
	}

	return Metrics;
}

bool UPerformanceTestingSystem::GeneratePerformanceReport(const FString& OutputPath)
{
	if (TestResults.Samples.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No performance data to report"));
		return false;
	}

	// Create output directory
	FString ReportDir = OutputPath.IsEmpty() ? CurrentConfig.ReportOutputPath : OutputPath;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*ReportDir))
	{
		PlatformFile.CreateDirectoryTree(*ReportDir);
	}

	// Generate timestamp for filenames
	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));

	// Generate text report
	FString TextReport = GenerateReportText();
	FString TextPath = ReportDir / FString::Printf(TEXT("PerformanceReport_%s.txt"), *Timestamp);
	if (!FFileHelper::SaveStringToFile(TextReport, *TextPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save text report to %s"), *TextPath);
		return false;
	}

	// Generate CSV report
	FString CSVReport = GenerateCSVReport();
	FString CSVPath = ReportDir / FString::Printf(TEXT("PerformanceReport_%s.csv"), *Timestamp);
	if (!FFileHelper::SaveStringToFile(CSVReport, *CSVPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save CSV report to %s"), *CSVPath);
		return false;
	}

	// Generate JSON report
	FString JSONReport = GenerateJSONReport();
	FString JSONPath = ReportDir / FString::Printf(TEXT("PerformanceReport_%s.json"), *Timestamp);
	if (!FFileHelper::SaveStringToFile(JSONReport, *JSONPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save JSON report to %s"), *JSONPath);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Performance reports generated:"));
	UE_LOG(LogTemp, Log, TEXT("  Text: %s"), *TextPath);
	UE_LOG(LogTemp, Log, TEXT("  CSV: %s"), *CSVPath);
	UE_LOG(LogTemp, Log, TEXT("  JSON: %s"), *JSONPath);

	return true;
}

bool UPerformanceTestingSystem::ValidateVRPerformance(const FPerformanceTestResults& Results) const
{
	// VR requirement: minimum 90 FPS (requirement 8.1)
	const float MinVRFrameRate = 90.0f;
	
	if (Results.MinimumFrameRate < MinVRFrameRate)
	{
		UE_LOG(LogTemp, Warning, TEXT("VR performance validation failed: Minimum FPS %.2f < %.2f"),
			Results.MinimumFrameRate, MinVRFrameRate);
		return false;
	}

	if (Results.AverageFrameRate < MinVRFrameRate)
	{
		UE_LOG(LogTemp, Warning, TEXT("VR performance validation failed: Average FPS %.2f < %.2f"),
			Results.AverageFrameRate, MinVRFrameRate);
		return false;
	}

	// Check frame time consistency (< 2ms variance for comfort)
	float MaxFrameTimeVariance = 2.0f;
	float FrameTimeVariance = 0.0f;
	
	if (Results.Samples.Num() > 1)
	{
		float MinFrameTime = FLT_MAX;
		float MaxFrameTime = 0.0f;
		
		for (const FPerformanceMetrics& Sample : Results.Samples)
		{
			MinFrameTime = FMath::Min(MinFrameTime, Sample.FrameTime);
			MaxFrameTime = FMath::Max(MaxFrameTime, Sample.FrameTime);
		}
		
		FrameTimeVariance = MaxFrameTime - MinFrameTime;
		
		if (FrameTimeVariance > MaxFrameTimeVariance)
		{
			UE_LOG(LogTemp, Warning, TEXT("VR performance validation failed: Frame time variance %.2fms > %.2fms"),
				FrameTimeVariance, MaxFrameTimeVariance);
			return false;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("VR performance validation passed"));
	return true;
}

void UPerformanceTestingSystem::TickTest(float DeltaTime)
{
	if (!bIsTestRunning)
	{
		return;
	}

	SampleTimer += DeltaTime;

	// Check if it's time to capture a sample
	float SampleInterval = CurrentConfig.SampleDuration / CurrentConfig.SamplesPerAltitude;
	
	if (SampleTimer >= SampleInterval)
	{
		// Capture metrics
		FPerformanceMetrics Metrics = CapturePerformanceMetrics();
		TestResults.Samples.Add(Metrics);

		// Capture VR metrics if enabled
		if (CurrentConfig.bTestVRPerformance)
		{
			FVRPerformanceMetrics VRMetrics = CaptureVRPerformanceMetrics();
			TestResults.VRSamples.Add(VRMetrics);
		}

		CurrentSampleIndex++;
		SampleTimer = 0.0f;

		// Check if altitude test is complete
		if (CurrentSampleIndex >= CurrentConfig.SamplesPerAltitude)
		{
			CompleteAltitudeTest();
			
			// Move to next altitude
			CurrentAltitudeIndex++;
			
			if (CurrentAltitudeIndex < CurrentConfig.TestAltitudes.Num())
			{
				StartAltitudeTest(CurrentConfig.TestAltitudes[CurrentAltitudeIndex]);
			}
			else
			{
				// All tests complete
				CompleteTest();
			}
		}
	}
}

void UPerformanceTestingSystem::StartAltitudeTest(float Altitude)
{
	CurrentSampleIndex = 0;
	SampleTimer = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Starting performance test at altitude: %.2f meters"), Altitude);

	// TODO: Move player/camera to specified altitude
	// This would require integration with the player controller or camera system
}

void UPerformanceTestingSystem::CompleteAltitudeTest()
{
	float Altitude = CurrentConfig.TestAltitudes[CurrentAltitudeIndex];
	UE_LOG(LogTemp, Log, TEXT("Completed performance test at altitude: %.2f meters"), Altitude);
}

void UPerformanceTestingSystem::CompleteTest()
{
	TestResults.TestEndTime = FDateTime::Now();
	bIsTestRunning = false;

	// Analyze results
	AnalyzeResults();

	// Generate report if configured
	if (CurrentConfig.bGenerateReport)
	{
		GeneratePerformanceReport(CurrentConfig.ReportOutputPath);
	}

	UE_LOG(LogTemp, Log, TEXT("Performance test completed"));
	UE_LOG(LogTemp, Log, TEXT("  Samples: %d"), TestResults.Samples.Num());
	UE_LOG(LogTemp, Log, TEXT("  Average FPS: %.2f"), TestResults.AverageFrameRate);
	UE_LOG(LogTemp, Log, TEXT("  Min FPS: %.2f"), TestResults.MinimumFrameRate);
	UE_LOG(LogTemp, Log, TEXT("  Max FPS: %.2f"), TestResults.MaximumFrameRate);
	UE_LOG(LogTemp, Log, TEXT("  VR Requirements Met: %s"), TestResults.bMeetsVRRequirements ? TEXT("Yes") : TEXT("No"));
}

// Metrics measurement implementations
float UPerformanceTestingSystem::MeasureFrameRate() const
{
	if (GEngine)
	{
		return 1.0f / GEngine->GetMaxTickRate(0.0f, false);
	}
	return 0.0f;
}

float UPerformanceTestingSystem::MeasureFrameTime() const
{
	return FPlatformTime::ToMilliseconds(GFrameTime);
}

float UPerformanceTestingSystem::MeasureGameThreadTime() const
{
	return FPlatformTime::ToMilliseconds(GGameThreadTime);
}

float UPerformanceTestingSystem::MeasureRenderThreadTime() const
{
	return FPlatformTime::ToMilliseconds(GRenderThreadTime);
}

float UPerformanceTestingSystem::MeasureGPUTime() const
{
	return FPlatformTime::ToMilliseconds(GGPUFrameTime);
}

int32 UPerformanceTestingSystem::MeasureDrawCalls() const
{
	// This would require access to rendering stats
	// Placeholder implementation
	return 0;
}

int32 UPerformanceTestingSystem::MeasureVisibleTriangles() const
{
	// This would require access to rendering stats
	// Placeholder implementation
	return 0;
}

float UPerformanceTestingSystem::MeasureMemoryUsage() const
{
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	return static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);
}

float UPerformanceTestingSystem::MeasureStreamingBandwidth()
{
	// Calculate bandwidth based on memory change over time
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	float CurrentBytes = static_cast<float>(MemStats.UsedPhysical);
	
	FDateTime CurrentTime = FDateTime::Now();
	FTimespan TimeDelta = CurrentTime - LastStreamingCheckTime;
	float DeltaSeconds = static_cast<float>(TimeDelta.GetTotalSeconds());
	
	float Bandwidth = 0.0f;
	if (DeltaSeconds > 0.0f && LastStreamingBytes > 0.0f)
	{
		float BytesDelta = FMath::Abs(CurrentBytes - LastStreamingBytes);
		Bandwidth = (BytesDelta / (1024.0f * 1024.0f)) / DeltaSeconds; // MB/s
	}
	
	LastStreamingBytes = CurrentBytes;
	LastStreamingCheckTime = CurrentTime;
	
	return Bandwidth;
}

int32 UPerformanceTestingSystem::CountActiveTerrainTiles() const
{
	// This would require integration with terrain streaming system
	// Placeholder implementation
	return 0;
}

int32 UPerformanceTestingSystem::CountStreamingTiles() const
{
	// This would require integration with terrain streaming system
	// Placeholder implementation
	return 0;
}

float UPerformanceTestingSystem::MeasureMotionToPhotonLatency() const
{
	// This would require VR system integration
	// Placeholder implementation
	return 0.0f;
}

int32 UPerformanceTestingSystem::CountDroppedFrames() const
{
	// This would require VR system integration
	// Placeholder implementation
	return 0;
}

bool UPerformanceTestingSystem::IsReprojecting() const
{
	// This would require VR system integration
	// Placeholder implementation
	return false;
}

float UPerformanceTestingSystem::GetHeadsetFPS() const
{
	if (GEngine && GEngine->XRSystem.IsValid())
	{
		// Most VR headsets run at 90 or 120 Hz
		return 90.0f; // Placeholder
	}
	return 0.0f;
}

void UPerformanceTestingSystem::AnalyzeResults()
{
	if (TestResults.Samples.Num() == 0)
	{
		return;
	}

	// Calculate frame rate statistics
	float TotalFPS = 0.0f;
	float MinFPS = FLT_MAX;
	float MaxFPS = 0.0f;
	
	float TotalMemory = 0.0f;
	float PeakMemory = 0.0f;
	
	float TotalBandwidth = 0.0f;
	float PeakBandwidth = 0.0f;

	for (const FPerformanceMetrics& Sample : TestResults.Samples)
	{
		TotalFPS += Sample.FrameRate;
		MinFPS = FMath::Min(MinFPS, Sample.FrameRate);
		MaxFPS = FMath::Max(MaxFPS, Sample.FrameRate);
		
		TotalMemory += Sample.MemoryUsageMB;
		PeakMemory = FMath::Max(PeakMemory, Sample.MemoryUsageMB);
		
		TotalBandwidth += Sample.StreamingBandwidthMBps;
		PeakBandwidth = FMath::Max(PeakBandwidth, Sample.StreamingBandwidthMBps);
	}

	int32 NumSamples = TestResults.Samples.Num();
	TestResults.AverageFrameRate = TotalFPS / NumSamples;
	TestResults.MinimumFrameRate = MinFPS;
	TestResults.MaximumFrameRate = MaxFPS;
	TestResults.AverageMemoryUsageMB = TotalMemory / NumSamples;
	TestResults.PeakMemoryUsageMB = PeakMemory;
	TestResults.AverageStreamingBandwidthMBps = TotalBandwidth / NumSamples;
	TestResults.PeakStreamingBandwidthMBps = PeakBandwidth;

	// Validate VR performance
	TestResults.bMeetsVRRequirements = ValidateVRPerformance(TestResults);
}

FString UPerformanceTestingSystem::GenerateReportText() const
{
	FString Report;
	
	Report += TEXT("=================================================================\n");
	Report += TEXT("           PLANET INTERIOR PERFORMANCE TEST REPORT\n");
	Report += TEXT("=================================================================\n\n");
	
	Report += FString::Printf(TEXT("Test Start: %s\n"), *TestResults.TestStartTime.ToString());
	Report += FString::Printf(TEXT("Test End: %s\n"), *TestResults.TestEndTime.ToString());
	Report += FString::Printf(TEXT("Duration: %.2f seconds\n\n"), 
		(TestResults.TestEndTime - TestResults.TestStartTime).GetTotalSeconds());
	
	Report += TEXT("-----------------------------------------------------------------\n");
	Report += TEXT("SUMMARY\n");
	Report += TEXT("-----------------------------------------------------------------\n");
	Report += FString::Printf(TEXT("Total Samples: %d\n"), TestResults.Samples.Num());
	Report += FString::Printf(TEXT("Average Frame Rate: %.2f FPS\n"), TestResults.AverageFrameRate);
	Report += FString::Printf(TEXT("Minimum Frame Rate: %.2f FPS\n"), TestResults.MinimumFrameRate);
	Report += FString::Printf(TEXT("Maximum Frame Rate: %.2f FPS\n"), TestResults.MaximumFrameRate);
	Report += FString::Printf(TEXT("Average Memory Usage: %.2f MB\n"), TestResults.AverageMemoryUsageMB);
	Report += FString::Printf(TEXT("Peak Memory Usage: %.2f MB\n"), TestResults.PeakMemoryUsageMB);
	Report += FString::Printf(TEXT("Average Streaming Bandwidth: %.2f MB/s\n"), TestResults.AverageStreamingBandwidthMBps);
	Report += FString::Printf(TEXT("Peak Streaming Bandwidth: %.2f MB/s\n"), TestResults.PeakStreamingBandwidthMBps);
	Report += FString::Printf(TEXT("VR Requirements Met: %s\n\n"), 
		TestResults.bMeetsVRRequirements ? TEXT("YES") : TEXT("NO"));
	
	Report += TEXT("-----------------------------------------------------------------\n");
	Report += TEXT("REQUIREMENTS VALIDATION\n");
	Report += TEXT("-----------------------------------------------------------------\n");
	Report += FString::Printf(TEXT("8.1 - VR Frame Rate (90 FPS min): %s\n"), 
		TestResults.MinimumFrameRate >= 90.0f ? TEXT("PASS") : TEXT("FAIL"));
	Report += FString::Printf(TEXT("8.2 - LOD System (6+ levels): %s\n"), TEXT("N/A"));
	Report += FString::Printf(TEXT("8.3 - Frustum Culling: %s\n"), TEXT("N/A"));
	Report += FString::Printf(TEXT("8.4 - GPU Instancing (100k instances): %s\n"), TEXT("N/A"));
	Report += FString::Printf(TEXT("8.5 - Streaming Performance (5ms max): %s\n\n"), TEXT("N/A"));
	
	Report += TEXT("-----------------------------------------------------------------\n");
	Report += TEXT("DETAILED SAMPLES BY ALTITUDE\n");
	Report += TEXT("-----------------------------------------------------------------\n");
	
	// Group samples by altitude
	TMap<float, TArray<FPerformanceMetrics>> SamplesByAltitude;
	for (const FPerformanceMetrics& Sample : TestResults.Samples)
	{
		float RoundedAltitude = FMath::RoundToFloat(Sample.Altitude / 100.0f) * 100.0f;
		SamplesByAltitude.FindOrAdd(RoundedAltitude).Add(Sample);
	}
	
	// Sort altitudes
	TArray<float> Altitudes;
	SamplesByAltitude.GetKeys(Altitudes);
	Altitudes.Sort([](float A, float B) { return A > B; });
	
	for (float Altitude : Altitudes)
	{
		const TArray<FPerformanceMetrics>& Samples = SamplesByAltitude[Altitude];
		
		float AvgFPS = 0.0f;
		float AvgMemory = 0.0f;
		float AvgBandwidth = 0.0f;
		
		for (const FPerformanceMetrics& Sample : Samples)
		{
			AvgFPS += Sample.FrameRate;
			AvgMemory += Sample.MemoryUsageMB;
			AvgBandwidth += Sample.StreamingBandwidthMBps;
		}
		
		int32 NumSamples = Samples.Num();
		AvgFPS /= NumSamples;
		AvgMemory /= NumSamples;
		AvgBandwidth /= NumSamples;
		
		Report += FString::Printf(TEXT("\nAltitude: %.0f meters (%d samples)\n"), Altitude, NumSamples);
		Report += FString::Printf(TEXT("  Average FPS: %.2f\n"), AvgFPS);
		Report += FString::Printf(TEXT("  Average Memory: %.2f MB\n"), AvgMemory);
		Report += FString::Printf(TEXT("  Average Bandwidth: %.2f MB/s\n"), AvgBandwidth);
	}
	
	Report += TEXT("\n=================================================================\n");
	
	return Report;
}

FString UPerformanceTestingSystem::GenerateCSVReport() const
{
	FString CSV;
	
	// Header
	CSV += TEXT("Timestamp,Altitude,FrameRate,FrameTime,GameThreadTime,RenderThreadTime,GPUTime,");
	CSV += TEXT("DrawCalls,VisibleTriangles,MemoryUsageMB,StreamingBandwidthMBps,");
	CSV += TEXT("ActiveTerrainTiles,StreamingTiles\n");
	
	// Data rows
	for (const FPerformanceMetrics& Sample : TestResults.Samples)
	{
		CSV += FString::Printf(TEXT("%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%.2f,%.2f,%d,%d\n"),
			*Sample.Timestamp.ToString(),
			Sample.Altitude,
			Sample.FrameRate,
			Sample.FrameTime,
			Sample.GameThreadTime,
			Sample.RenderThreadTime,
			Sample.GPUTime,
			Sample.DrawCalls,
			Sample.VisibleTriangles,
			Sample.MemoryUsageMB,
			Sample.StreamingBandwidthMBps,
			Sample.ActiveTerrainTiles,
			Sample.StreamingTiles);
	}
	
	return CSV;
}

FString UPerformanceTestingSystem::GenerateJSONReport() const
{
	FString JSON;
	
	JSON += TEXT("{\n");
	JSON += FString::Printf(TEXT("  \"testStartTime\": \"%s\",\n"), *TestResults.TestStartTime.ToString());
	JSON += FString::Printf(TEXT("  \"testEndTime\": \"%s\",\n"), *TestResults.TestEndTime.ToString());
	JSON += FString::Printf(TEXT("  \"totalSamples\": %d,\n"), TestResults.Samples.Num());
	JSON += FString::Printf(TEXT("  \"averageFrameRate\": %.2f,\n"), TestResults.AverageFrameRate);
	JSON += FString::Printf(TEXT("  \"minimumFrameRate\": %.2f,\n"), TestResults.MinimumFrameRate);
	JSON += FString::Printf(TEXT("  \"maximumFrameRate\": %.2f,\n"), TestResults.MaximumFrameRate);
	JSON += FString::Printf(TEXT("  \"averageMemoryUsageMB\": %.2f,\n"), TestResults.AverageMemoryUsageMB);
	JSON += FString::Printf(TEXT("  \"peakMemoryUsageMB\": %.2f,\n"), TestResults.PeakMemoryUsageMB);
	JSON += FString::Printf(TEXT("  \"averageStreamingBandwidthMBps\": %.2f,\n"), TestResults.AverageStreamingBandwidthMBps);
	JSON += FString::Printf(TEXT("  \"peakStreamingBandwidthMBps\": %.2f,\n"), TestResults.PeakStreamingBandwidthMBps);
	JSON += FString::Printf(TEXT("  \"meetsVRRequirements\": %s,\n"), 
		TestResults.bMeetsVRRequirements ? TEXT("true") : TEXT("false"));
	JSON += TEXT("  \"samples\": [\n");
	
	for (int32 i = 0; i < TestResults.Samples.Num(); ++i)
	{
		const FPerformanceMetrics& Sample = TestResults.Samples[i];
		JSON += TEXT("    {\n");
		JSON += FString::Printf(TEXT("      \"timestamp\": \"%s\",\n"), *Sample.Timestamp.ToString());
		JSON += FString::Printf(TEXT("      \"altitude\": %.2f,\n"), Sample.Altitude);
		JSON += FString::Printf(TEXT("      \"frameRate\": %.2f,\n"), Sample.FrameRate);
		JSON += FString::Printf(TEXT("      \"frameTime\": %.2f,\n"), Sample.FrameTime);
		JSON += FString::Printf(TEXT("      \"memoryUsageMB\": %.2f,\n"), Sample.MemoryUsageMB);
		JSON += FString::Printf(TEXT("      \"streamingBandwidthMBps\": %.2f\n"), Sample.StreamingBandwidthMBps);
		JSON += TEXT("    }");
		if (i < TestResults.Samples.Num() - 1)
		{
			JSON += TEXT(",");
		}
		JSON += TEXT("\n");
	}
	
	JSON += TEXT("  ]\n");
	JSON += TEXT("}\n");
	
	return JSON;
}

FString UPerformanceTestingSystem::FormatMetrics(const FPerformanceMetrics& Metrics) const
{
	return FString::Printf(
		TEXT("Altitude: %.2fm | FPS: %.2f | Frame: %.2fms | Memory: %.2fMB | Bandwidth: %.2fMB/s"),
		Metrics.Altitude,
		Metrics.FrameRate,
		Metrics.FrameTime,
		Metrics.MemoryUsageMB,
		Metrics.StreamingBandwidthMBps);
}

FString UPerformanceTestingSystem::FormatVRMetrics(const FVRPerformanceMetrics& Metrics) const
{
	return FString::Printf(
		TEXT("Latency: %.2fms | Dropped: %d | Reprojecting: %s | HMD FPS: %.2f"),
		Metrics.MotionToPhotonLatency,
		Metrics.DroppedFrames,
		Metrics.bIsReprojecting ? TEXT("Yes") : TEXT("No"),
		Metrics.HeadsetFPS);
}
