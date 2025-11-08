// ProfilingTestLevel.cpp
#include "ProfilingTestLevel.h"
#include "Spaceship.h"
#include "OrbitalMechanics.h"
#include "FlightController.h"
#include "StarSystemManager.h"
#include "PhysicsConsensus.h"
#include "PerformanceProfilerSubsystem.h"

AProfilingTestLevel::AProfilingTestLevel()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AProfilingTestLevel::StartPlay()
{
	Super::StartPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("ProfilingTestLevel: No world!"));
		return;
	}

	// Verify profiler subsystem exists
	UPerformanceProfilerSubsystem* Profiler = World->GetSubsystem<UPerformanceProfilerSubsystem>();
	if (!Profiler)
	{
		UE_LOG(LogTemp, Error, TEXT("ProfilingTestLevel: PerformanceProfilerSubsystem not found!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("================================================================="));
	UE_LOG(LogTemp, Warning, TEXT("PROFILING TEST LEVEL - Spawning test actors with instrumented components"));
	UE_LOG(LogTemp, Warning, TEXT("================================================================="));

	// Spawn 5 Spaceships (will exercise Spaceship, FlightController, PhysicsConsensus)
	for (int32 i = 0; i < 5; i++)
	{
		FVector Location(i * 1000.0f, 0.0f, 500.0f);
		FRotator Rotation = FRotator::ZeroRotator;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = FName(*FString::Printf(TEXT("TestSpaceship_%d"), i));
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ASpaceship* Ship = World->SpawnActor<ASpaceship>(ASpaceship::StaticClass(), Location, Rotation, SpawnParams);
		if (Ship)
		{
			TestActors.Add(Ship);
			UE_LOG(LogTemp, Log, TEXT("  Spawned %s at %s"), *Ship->GetName(), *Location.ToString());

			// Ensure FlightController component exists and is active
			UFlightController* FlightCtrl = Ship->FindComponentByClass<UFlightController>();
			if (FlightCtrl)
			{
				FlightCtrl->SetComponentTickEnabled(true);
				UE_LOG(LogTemp, Log, TEXT("    FlightController enabled for %s"), *Ship->GetName());
			}
		}
	}

	// Spawn actor with OrbitalMechanics component
	{
		FVector Location(5000.0f, 0.0f, 0.0f);
		FRotator Rotation = FRotator::ZeroRotator;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = FName(TEXT("TestOrbitalBody"));
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* OrbitalActor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation, SpawnParams);
		if (OrbitalActor)
		{
			// Add OrbitalMechanics component
			UOrbitalMechanics* OrbitalComp = NewObject<UOrbitalMechanics>(OrbitalActor, UOrbitalMechanics::StaticClass(), TEXT("OrbitalMechanics"));
			if (OrbitalComp)
			{
				OrbitalComp->RegisterComponent();
				OrbitalComp->SetComponentTickEnabled(true);
				TestActors.Add(OrbitalActor);
				UE_LOG(LogTemp, Log, TEXT("  Spawned %s with OrbitalMechanics at %s"), *OrbitalActor->GetName(), *Location.ToString());
			}
		}
	}

	// Spawn actor with StarSystemManager component
	{
		FVector Location(10000.0f, 0.0f, 0.0f);
		FRotator Rotation = FRotator::ZeroRotator;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = FName(TEXT("TestStarSystemManager"));
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* StarActor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation, SpawnParams);
		if (StarActor)
		{
			// Add StarSystemManager component
			UStarSystemManager* StarComp = NewObject<UStarSystemManager>(StarActor, UStarSystemManager::StaticClass(), TEXT("StarSystemManager"));
			if (StarComp)
			{
				StarComp->RegisterComponent();
				StarComp->SetComponentTickEnabled(true);
				TestActors.Add(StarActor);
				UE_LOG(LogTemp, Log, TEXT("  Spawned %s with StarSystemManager at %s"), *StarActor->GetName(), *Location.ToString());
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("================================================================="));
	UE_LOG(LogTemp, Warning, TEXT("PROFILING TEST LEVEL - Spawned %d test actors"), TestActors.Num());
	UE_LOG(LogTemp, Warning, TEXT("All instrumented systems now ticking - profiling data being collected"));
	UE_LOG(LogTemp, Warning, TEXT("================================================================="));
}
