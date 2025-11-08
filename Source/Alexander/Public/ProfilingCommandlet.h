// ProfilingCommandlet.h - Headless commandlet for autonomous profiling
#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "ProfilingCommandlet.generated.h"

/**
 * Commandlet that runs profiling tests in headless mode
 * Usage: UnrealEditor-Cmd.exe Project.uproject -run=Profiling
 */
UCLASS()
class ALEXANDER_API UProfilingCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UProfilingCommandlet();

	virtual int32 Main(const FString& Params) override;

private:
	void RunProfilingTest(UWorld* World);
	void SpawnTestActors(UWorld* World);
	void TickWorld(UWorld* World, int32 NumFrames);
};
