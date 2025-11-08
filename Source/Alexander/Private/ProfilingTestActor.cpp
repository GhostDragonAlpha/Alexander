// Copyright Alexander Project. All Rights Reserved.

#include "ProfilingTestActor.h"
#include "PerformanceProfilerSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AProfilingTestActor::AProfilingTestActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AProfilingTestActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Start test immediately when spawned (works in editor)
	UE_LOG(LogTemp, Log, TEXT("======================================================================"));
	UE_LOG(LogTemp, Log, TEXT("PROFILING TEST ACTOR - STARTING AUTONOMOUS TEST"));
	UE_LOG(LogTemp, Log, TEXT("======================================================================"));

	StartProfilingTest();
}

void AProfilingTestActor::BeginPlay()
{
	Super::BeginPlay();
	
	// If not started yet (e.g., placed in level), start now
	if (!bTestRunning)
	{
		StartProfilingTest();
	}
}

void AProfilingTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bTestRunning)
	{
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	float ElapsedTime = CurrentTime - TestStartTime;

	// Record samples every second
	if (CurrentTime - LastSampleTime >= 1.0f)
	{
		RecordTestSamples();
		LastSampleTime = CurrentTime;
		SampleCount++;

		// Log progress every 15 seconds
		if (SampleCount % 15 == 0)
		{
			UE_LOG(LogTemp, Log, TEXT("Test Progress: %d/%d seconds"), 
				static_cast<int32>(ElapsedTime), static_cast<int32>(TestDurationSeconds));
		}
	}

	// Check if test is complete
	if (ElapsedTime >= TestDurationSeconds)
	{
		StopProfilingTest();
	}
}

void AProfilingTestActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bTestRunning)
	{
		StopProfilingTest();
	}

	Super::EndPlay(EndPlayReason);
}

void AProfilingTestActor::StartProfilingTest()
{
	// Get profiler subsystem
	Profiler = GetWorld()->GetSubsystem<UPerformanceProfilerSubsystem>();

	if (!Profiler)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get PerformanceProfilerSubsystem!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Profiler subsystem found"));
	UE_LOG(LogTemp, Log, TEXT("Profiling enabled: %s"), Profiler->IsProfilingEnabled() ? TEXT("true") : TEXT("false"));

	// Start profiling test systems
	Profiler->StartProfiling(FName("TestSystem1"));
	Profiler->StartProfiling(FName("TestSystem2"));
	Profiler->StartProfiling(FName("TestSystem3"));

	bTestRunning = true;
	TestStartTime = GetWorld()->GetTimeSeconds();
	LastSampleTime = TestStartTime;
	SampleCount = 0;

	UE_LOG(LogTemp, Log, TEXT("Test systems started - Running for %.0f seconds..."), TestDurationSeconds);
}

void AProfilingTestActor::RecordTestSamples()
{
	if (!Profiler)
	{
		return;
	}

	// Generate random sample data for testing
	float Sample1 = FMath::RandRange(1.0f, 5.0f);
	float Sample2 = FMath::RandRange(5.0f, 10.0f);
	float Sample3 = FMath::RandRange(0.5f, 2.0f);

	Profiler->RecordSample(FName("TestSystem1"), Sample1);
	Profiler->RecordSample(FName("TestSystem2"), Sample2);
	Profiler->RecordSample(FName("TestSystem3"), Sample3);
}

void AProfilingTestActor::StopProfilingTest()
{
	if (!Profiler || !bTestRunning)
	{
		return;
	}

	bTestRunning = false;

	UE_LOG(LogTemp, Log, TEXT("Test duration complete - Stopping profiling..."));

	// Stop profiling
	Profiler->StopProfiling(FName("TestSystem1"));
	Profiler->StopProfiling(FName("TestSystem2"));
	Profiler->StopProfiling(FName("TestSystem3"));

	// Get results
	TArray<FProfileData> Bottlenecks = Profiler->GetTopBottlenecks(10);
	int64 CurrentMem = Profiler->GetCurrentMemoryUsageMB();
	int64 PeakMem = Profiler->GetPeakMemoryUsageMB();

	// Log results
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("======================================================================"));
	UE_LOG(LogTemp, Log, TEXT("PROFILING RESULTS"));
	UE_LOG(LogTemp, Log, TEXT("======================================================================"));
	UE_LOG(LogTemp, Log, TEXT("Memory: Current=%lld MB, Peak=%lld MB"), CurrentMem, PeakMem);
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("Top Bottlenecks:"));

	for (int32 i = 0; i < Bottlenecks.Num(); ++i)
	{
		const FProfileData& Data = Bottlenecks[i];
		FString Status = Data.AverageTimeMs > 8.0 ? TEXT("CRITICAL") :
			Data.AverageTimeMs > 5.0 ? TEXT("HIGH") : TEXT("OK");

		UE_LOG(LogTemp, Log, TEXT("  %d. %s: %.2f ms avg [%s]"),
			i + 1, *Data.SystemName.ToString(), Data.AverageTimeMs, *Status);
	}

	// Export to JSON
	FString ExportPath = FPaths::ProjectSavedDir() / TEXT("Profiling") / TEXT("test_run.json");
	Profiler->ExportToJSON(ExportPath);

	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("======================================================================"));
	UE_LOG(LogTemp, Log, TEXT("PROFILING TEST COMPLETE"));
	UE_LOG(LogTemp, Log, TEXT("Results exported to: %s"), *ExportPath);
	UE_LOG(LogTemp, Log, TEXT("======================================================================"));

	// Quit editor if requested
	if (bQuitEditorWhenDone)
	{
		UE_LOG(LogTemp, Log, TEXT("Quitting editor in 3 seconds..."));

		FTimerHandle QuitTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(QuitTimerHandle, []()
		{
			FPlatformMisc::RequestExit(false);
		}, 3.0f, false);
	}
	else
	{
		// Just destroy this actor
		Destroy();
	}
}
