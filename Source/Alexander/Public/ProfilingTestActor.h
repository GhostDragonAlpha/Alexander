// Copyright Alexander Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProfilingTestActor.generated.h"

/**
 * Test actor that automatically runs profiling test on BeginPlay
 * Spawns automatically in editor, runs test, saves results, and self-destructs
 */
UCLASS()
class ALEXANDER_API AProfilingTestActor : public AActor
{
	GENERATED_BODY()

public:
	AProfilingTestActor();

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Test configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling Test")
	float TestDurationSeconds = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling Test")
	bool bQuitEditorWhenDone = true;

private:
	class UPerformanceProfilerSubsystem* Profiler = nullptr;
	float TestStartTime = 0.0f;
	float LastSampleTime = 0.0f;
	bool bTestRunning = false;
	int32 SampleCount = 0;

	void StartProfilingTest();
	void RecordTestSamples();
	void StopProfilingTest();
};
