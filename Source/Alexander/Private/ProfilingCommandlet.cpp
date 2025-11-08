// ProfilingCommandlet.cpp
#include "ProfilingCommandlet.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Spaceship.h"
#include "OrbitalMechanics.h"
#include "FlightController.h"
#include "StarSystemManager.h"
#include "PhysicsConsensus.h"
#include "PerformanceProfilerSubsystem.h"

UProfilingCommandlet::UProfilingCommandlet()
{
	IsClient = false;
	IsEditor = true;  // Must be true to create worlds
	IsServer = false;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 UProfilingCommandlet::Main(const FString& Params)
{
	UE_LOG(LogTemp, Display, TEXT("======================================================================="));
	UE_LOG(LogTemp, Display, TEXT("AUTONOMOUS PROFILING COMMANDLET - Phase 9 Performance Optimization"));
	UE_LOG(LogTemp, Display, TEXT("======================================================================="));
	
	// Get editor world
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get editor world!"));
		return 1;
	}
	
	// Verify profiler subsystem exists
	UPerformanceProfilerSubsystem* Profiler = World->GetSubsystem<UPerformanceProfilerSubsystem>();
	if (!Profiler)
	{
		UE_LOG(LogTemp, Error, TEXT("PerformanceProfilerSubsystem not found!"));
		return 1;
	}
	
	UE_LOG(LogTemp, Display, TEXT("✓ Using editor world"));
	UE_LOG(LogTemp, Display, TEXT("✓ Profiler subsystem initialized"));
	UE_LOG(LogTemp, Display, TEXT(""));
	
	// Spawn test actors
	SpawnTestActors(World);
	
	// Run profiling test (tick for 3 minutes = 10800 frames at 60fps)
	UE_LOG(LogTemp, Display, TEXT("Running profiling test for 180 seconds (10800 frames)..."));
	TickWorld(World, 10800);
	
	UE_LOG(LogTemp, Display, TEXT("✓ Profiling complete"));
	UE_LOG(LogTemp, Display, TEXT(""));
	UE_LOG(LogTemp, Display, TEXT("======================================================================="));
	UE_LOG(LogTemp, Display, TEXT("✓ PROFILING COMMANDLET COMPLETE"));
	UE_LOG(LogTemp, Display, TEXT("✓ Profiler Deinitialize() will auto-export JSON on editor shutdown"));
	UE_LOG(LogTemp, Display, TEXT("======================================================================="));
	
	return 0;
}

void UProfilingCommandlet::SpawnTestActors(UWorld* World)
{
	UE_LOG(LogTemp, Display, TEXT("Spawning test actors with instrumented components..."));
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	// Spawn 5 Spaceships (tests Spaceship::Tick and FlightController)
	for (int32 i = 0; i < 5; i++)
	{
		FVector Location(i * 1000.0f, 0.0f, 0.0f);
		ASpaceship* Ship = World->SpawnActor<ASpaceship>(ASpaceship::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
		if (Ship)
		{
			// Initialize actor lifecycle (components now properly created in constructor)
			Ship->DispatchBeginPlay();

			// Verify FlightController exists and is active
			UFlightController* FlightCtrl = Ship->FindComponentByClass<UFlightController>();
			if (FlightCtrl)
			{
				FlightCtrl->SetComponentTickEnabled(true);
				UE_LOG(LogTemp, Display, TEXT("  ✓ Spawned Spaceship_%d with FlightController (active: %s)"),
					i, FlightCtrl->IsControllerActive() ? TEXT("YES") : TEXT("NO"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("  ✗ Spaceship_%d missing FlightController component"), i);
			}
		}
	}
	
	// Spawn actor with OrbitalMechanics
	{
		AActor* OrbitalActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(5000.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		if (OrbitalActor)
		{
			UOrbitalMechanics* OrbitalComp = NewObject<UOrbitalMechanics>(OrbitalActor);
			OrbitalComp->RegisterComponent();
			OrbitalComp->SetComponentTickEnabled(true);
			UE_LOG(LogTemp, Display, TEXT("  ✓ Spawned OrbitalBody with OrbitalMechanics"));
		}
	}
	
	// Spawn actor with StarSystemManager
	{
		AActor* StarActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(10000.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		if (StarActor)
		{
			UStarSystemManager* StarComp = NewObject<UStarSystemManager>(StarActor);
			StarComp->RegisterComponent();
			StarComp->SetComponentTickEnabled(true);
			UE_LOG(LogTemp, Display, TEXT("  ✓ Spawned StarSystem with StarSystemManager"));
		}
	}
	
	UE_LOG(LogTemp, Display, TEXT("✓ All test actors spawned"));
	UE_LOG(LogTemp, Display, TEXT(""));
}

void UProfilingCommandlet::TickWorld(UWorld* World, int32 NumFrames)
{
	const float DeltaTime = 1.0f / 60.0f; // 60 FPS
	
	// Get all actors to tick manually
	TArray<AActor*> AllActors;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AllActors.Add(*It);
	}
	
	UE_LOG(LogTemp, Display, TEXT("  Found %d actors to tick"), AllActors.Num());
	
	// Progress reporting every 1800 frames (30 seconds at 60fps)
	const int32 ReportInterval = 1800;
	
	for (int32 Frame = 0; Frame < NumFrames; Frame++)
	{
		// Manually tick each actor
		for (AActor* Actor : AllActors)
		{
			if (Actor && !Actor->IsPendingKillPending())
			{
				Actor->Tick(DeltaTime);
				
				// Tick all components
				for (UActorComponent* Comp : Actor->GetComponents())
				{
					if (Comp && Comp->IsRegistered() && Comp->IsComponentTickEnabled())
					{
						Comp->TickComponent(DeltaTime, LEVELTICK_All, nullptr);
					}
				}
			}
		}
		
		// Progress report
		if ((Frame + 1) % ReportInterval == 0)
		{
			int32 SecondsElapsed = (Frame + 1) / 60;
			int32 SecondsRemaining = (NumFrames - Frame - 1) / 60;
			UE_LOG(LogTemp, Display, TEXT("  [%ds elapsed, %ds remaining] Frame %d/%d"), 
				SecondsElapsed, SecondsRemaining, Frame + 1, NumFrames);
		}
	}
	
	UE_LOG(LogTemp, Display, TEXT("  Ticked %d actors for %d frames"), AllActors.Num(), NumFrames);
}
