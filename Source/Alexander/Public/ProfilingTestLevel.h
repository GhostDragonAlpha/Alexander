// ProfilingTestLevel.h - GameMode that spawns actors with profiled components for testing
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ProfilingTestLevel.generated.h"

/**
 * GameMode for profiling testing
 * Spawns test actors with all the instrumented components
 */
UCLASS()
class ALEXANDER_API AProfilingTestLevel : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AProfilingTestLevel();

	virtual void StartPlay() override;

private:
	// Test actors spawned for profiling
	UPROPERTY()
	TArray<AActor*> TestActors;
};
