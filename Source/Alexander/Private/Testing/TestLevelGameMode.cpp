// Copyright Epic Games, Inc. All Rights Reserved.

#include "Testing/TestLevelGameMode.h"
#include "Testing/TestStation.h"
#include "PerformanceProfiler.h"
#include "MemoryOptimizationManager.h"
#include "TickOptimizationManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

ATestLevelGameMode::ATestLevelGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Get platform information
	PlatformName = UGameplayStatics::GetPlatformName();

#if UE_BUILD_DEBUG
	BuildConfiguration = TEXT("Debug");
#elif UE_BUILD_DEVELOPMENT
	BuildConfiguration = TEXT("Development");
#elif UE_BUILD_TEST
	BuildConfiguration = TEXT("Test");
#elif UE_BUILD_SHIPPING
	BuildConfiguration = TEXT("Shipping");
#else
	BuildConfiguration = TEXT("Unknown");
#endif

	EngineVersion = FString::Printf(TEXT("%d.%d"), ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION);
}

void ATestLevelGameMode::BeginPlay()
{
	Super::BeginPlay();

	LogInfo(TEXT("Test Level Game Mode initialized"));

	// Initialize performance profiling
	if (bEnablePerformanceProfiling)
	{
		InitializePerformanceProfiling();
	}

	// Initialize overall results
	OverallResults = FTestResults();
	OverallResults.Platform = PlatformName;
	OverallResults.BuildConfiguration = BuildConfiguration;
	OverallResults.EngineVersion = EngineVersion;

	// Auto-start if configured
	if (bAutoStartTests)
	{
		// Delay start to allow all stations to register
		FTimerHandle DelayTimer;
		GetWorldTimerManager().SetTimer(DelayTimer, this, &ATestLevelGameMode::RunAllTests, 1.0f, false);
	}
}

void ATestLevelGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bTestsRunning && bEnablePerformanceProfiling)
	{
		UpdatePerformanceMetrics();
	}
}

void ATestLevelGameMode::RegisterTestStation(UTestStation* Station, const FVector& TeleportLocation, const FRotator& TeleportRotation, const FString& Description)
{
	if (!Station)
	{
		LogWarning(TEXT("Attempted to register null test station"));
		return;
	}

	// Check if already registered
	if (RegisteredStations.Contains(Station))
	{
		LogWarning(FString::Printf(TEXT("Station already registered: %s"), *Station->GetStationName()));
		return;
	}

	// Create station location
	FTestStationLocation Location;
	Location.StationName = Station->GetStationName();
	Location.TeleportLocation = TeleportLocation;
	Location.TeleportRotation = TeleportRotation;
	Location.StationReference = Station;
	Location.Description = Description.IsEmpty() ? Station->GetStationName() : Description;
	Location.bEnabled = true;

	StationLocations.Add(Location);
	RegisteredStations.Add(Station);

	// Subscribe to station completion event
	Station->OnAllTestsCompleted.AddDynamic(this, &ATestLevelGameMode::OnStationCompleted);

	LogInfo(FString::Printf(TEXT("Registered test station: %s at location %s"),
		*Station->GetStationName(), *TeleportLocation.ToString()));
}

void ATestLevelGameMode::UnregisterTestStation(UTestStation* Station)
{
	if (!Station)
	{
		return;
	}

	RegisteredStations.Remove(Station);
	StationLocations.RemoveAll([Station](const FTestStationLocation& Loc)
	{
		return Loc.StationReference == Station;
	});

	Station->OnAllTestsCompleted.RemoveDynamic(this, &ATestLevelGameMode::OnStationCompleted);

	LogInfo(FString::Printf(TEXT("Unregistered test station: %s"), *Station->GetStationName()));
}

void ATestLevelGameMode::RunAllTests()
{
	if (bTestsRunning)
	{
		LogWarning(TEXT("Tests are already running"));
		return;
	}

	if (RegisteredStations.Num() == 0)
	{
		LogWarning(TEXT("No test stations registered"));
		return;
	}

	LogInfo(FString::Printf(TEXT("Starting tests for %d stations"), RegisteredStations.Num()));

	// Reset state
	bTestsRunning = true;
	bSingleStationMode = false;
	CurrentStationIndex = -1;
	OverallResults = FTestResults();
	OverallResults.Platform = PlatformName;
	OverallResults.BuildConfiguration = BuildConfiguration;
	OverallResults.EngineVersion = EngineVersion;
	OverallResults.TestRunStartTime = FDateTime::Now();

	OnTestsStarted.Broadcast(TEXT("Starting all test stations"));

	// Start first station
	ExecuteNextStation();
}

void ATestLevelGameMode::RunSingleStation(const FString& StationName)
{
	if (bTestsRunning)
	{
		LogWarning(TEXT("Tests are already running"));
		return;
	}

	// Find station by name
	int32 StationIndex = RegisteredStations.IndexOfByPredicate([&StationName](UTestStation* Station)
	{
		return Station && Station->GetStationName().Equals(StationName, ESearchCase::IgnoreCase);
	});

	if (StationIndex == INDEX_NONE)
	{
		LogError(FString::Printf(TEXT("Station not found: %s"), *StationName));
		return;
	}

	LogInfo(FString::Printf(TEXT("Running single station: %s"), *StationName));

	// Run only this station
	bTestsRunning = true;
	bSingleStationMode = true;
	CurrentStationIndex = StationIndex - 1; // Will be incremented in ExecuteNextStation
	OverallResults = FTestResults();
	OverallResults.Platform = PlatformName;
	OverallResults.BuildConfiguration = BuildConfiguration;
	OverallResults.EngineVersion = EngineVersion;
	OverallResults.TestRunStartTime = FDateTime::Now();

	OnTestsStarted.Broadcast(FString::Printf(TEXT("Starting station: %s"), *StationName));

	ExecuteNextStation();
}

void ATestLevelGameMode::RunSingleTest(const FString& StationName, const FString& TestName)
{
	// Find station
	UTestStation* Station = nullptr;
	for (UTestStation* S : RegisteredStations)
	{
		if (S && S->GetStationName().Equals(StationName, ESearchCase::IgnoreCase))
		{
			Station = S;
			break;
		}
	}

	if (!Station)
	{
		LogError(FString::Printf(TEXT("Station not found: %s"), *StationName));
		return;
	}

	LogInfo(FString::Printf(TEXT("Running single test: %s.%s"), *StationName, *TestName));
	Station->RunSingleTest(TestName);
}

void ATestLevelGameMode::StopAllTests()
{
	if (!bTestsRunning)
	{
		return;
	}

	LogWarning(TEXT("Stopping all tests"));

	// Stop all stations
	for (UTestStation* Station : RegisteredStations)
	{
		if (Station)
		{
			Station->StopAllTests();
		}
	}

	bTestsRunning = false;
	CurrentStationIndex = -1;

	OverallResults.TestRunEndTime = FDateTime::Now();
	OverallResults.TotalExecutionTimeSeconds = (OverallResults.TestRunEndTime - OverallResults.TestRunStartTime).GetTotalSeconds();
}

void ATestLevelGameMode::ResetAllTests()
{
	StopAllTests();

	for (UTestStation* Station : RegisteredStations)
	{
		if (Station)
		{
			Station->ResetTests();
		}
	}

	OverallResults = FTestResults();
	OverallResults.Platform = PlatformName;
	OverallResults.BuildConfiguration = BuildConfiguration;
	OverallResults.EngineVersion = EngineVersion;

	LogInfo(TEXT("All tests reset"));
}

bool ATestLevelGameMode::TeleportPlayerToStation(const FString& StationName)
{
	// Find station location
	const FTestStationLocation* Location = StationLocations.FindByPredicate([&StationName](const FTestStationLocation& Loc)
	{
		return Loc.StationName.Equals(StationName, ESearchCase::IgnoreCase);
	});

	if (!Location)
	{
		LogError(FString::Printf(TEXT("Station location not found: %s"), *StationName));
		return false;
	}

	// Get player pawn
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		LogError(TEXT("Player controller not found"));
		return false;
	}

	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn)
	{
		LogError(TEXT("Player pawn not found"));
		return false;
	}

	// Teleport
	PlayerPawn->SetActorLocation(Location->TeleportLocation);
	PlayerPawn->SetActorRotation(Location->TeleportRotation);
	PC->SetControlRotation(Location->TeleportRotation);

	LogInfo(FString::Printf(TEXT("Teleported player to station: %s"), *StationName));
	OnStationChanged.Broadcast(StationName);

	return true;
}

bool ATestLevelGameMode::TeleportPlayerToHub()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		LogError(TEXT("Player controller not found"));
		return false;
	}

	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn)
	{
		LogError(TEXT("Player pawn not found"));
		return false;
	}

	PlayerPawn->SetActorLocation(HubLocation);
	PlayerPawn->SetActorRotation(HubRotation);
	PC->SetControlRotation(HubRotation);

	LogInfo(TEXT("Teleported player to hub"));
	OnStationChanged.Broadcast(TEXT("Hub"));

	return true;
}

void ATestLevelGameMode::SetHubLocation(const FVector& Location, const FRotator& Rotation)
{
	HubLocation = Location;
	HubRotation = Rotation;
	LogInfo(FString::Printf(TEXT("Hub location set to: %s"), *Location.ToString()));
}

float ATestLevelGameMode::GetOverallProgress() const
{
	if (RegisteredStations.Num() == 0)
	{
		return 0.0f;
	}

	int32 CompletedStations = OverallResults.StationResults.Num();
	float StationProgress = (float)CompletedStations / RegisteredStations.Num();

	// Add partial progress from current station if running
	if (bTestsRunning && CurrentStationIndex >= 0 && CurrentStationIndex < RegisteredStations.Num())
	{
		UTestStation* CurrentStation = RegisteredStations[CurrentStationIndex];
		if (CurrentStation)
		{
			float CurrentStationProgress = CurrentStation->GetTestProgress();
			StationProgress += CurrentStationProgress / RegisteredStations.Num();
		}
	}

	return FMath::Clamp(StationProgress, 0.0f, 1.0f);
}

FString ATestLevelGameMode::GetCurrentStationName() const
{
	if (bTestsRunning && CurrentStationIndex >= 0 && CurrentStationIndex < RegisteredStations.Num())
	{
		UTestStation* CurrentStation = RegisteredStations[CurrentStationIndex];
		if (CurrentStation)
		{
			return CurrentStation->GetStationName();
		}
	}
	return TEXT("");
}

bool ATestLevelGameMode::ExportResultsToJSON(const FString& FilePath)
{
	FString JSON = GetResultsJSON();
	FString FullPath = FPaths::ProjectDir() / DefaultExportDirectory / FilePath;

	if (FFileHelper::SaveStringToFile(JSON, *FullPath))
	{
		LogInfo(FString::Printf(TEXT("Exported results to JSON: %s"), *FullPath));
		return true;
	}
	else
	{
		LogError(FString::Printf(TEXT("Failed to export results to JSON: %s"), *FullPath));
		return false;
	}
}

bool ATestLevelGameMode::ExportResultsToCSV(const FString& FilePath)
{
	FString CSV = GetResultsCSV();
	FString FullPath = FPaths::ProjectDir() / DefaultExportDirectory / FilePath;

	if (FFileHelper::SaveStringToFile(CSV, *FullPath))
	{
		LogInfo(FString::Printf(TEXT("Exported results to CSV: %s"), *FullPath));
		return true;
	}
	else
	{
		LogError(FString::Printf(TEXT("Failed to export results to CSV: %s"), *FullPath));
		return false;
	}
}

FString ATestLevelGameMode::GetResultsJSON() const
{
	return OverallResults.ToJSON();
}

FString ATestLevelGameMode::GetResultsCSV() const
{
	return OverallResults.ToCSV();
}

void ATestLevelGameMode::EnablePerformanceProfiling(bool bEnable)
{
	bEnablePerformanceProfiling = bEnable;

	if (bEnable && !PerformanceProfiler)
	{
		InitializePerformanceProfiling();
	}

	LogInfo(FString::Printf(TEXT("Performance profiling: %s"), bEnable ? TEXT("Enabled") : TEXT("Disabled")));
}

void ATestLevelGameMode::ExecuteNextStation()
{
	// Check if we're in single station mode
	if (bSingleStationMode && CurrentStationIndex >= 0)
	{
		// Single station will complete via callback
		return;
	}

	// Move to next station
	CurrentStationIndex++;

	// Check if all stations complete
	if (CurrentStationIndex >= RegisteredStations.Num())
	{
		CompleteAllTests();
		return;
	}

	// Get current station
	UTestStation* CurrentStation = RegisteredStations[CurrentStationIndex];
	if (!CurrentStation)
	{
		LogError(TEXT("Current station is null"));
		ExecuteNextStation();
		return;
	}

	// Teleport player to station
	TeleportPlayerToStation(CurrentStation->GetStationName());

	// Run station tests
	LogInfo(FString::Printf(TEXT("Starting station %d/%d: %s"),
		CurrentStationIndex + 1, RegisteredStations.Num(), *CurrentStation->GetStationName()));

	CurrentStation->RunAllTests();
}

void ATestLevelGameMode::OnStationCompleted(const FTestStationResults& Results)
{
	LogInfo(FString::Printf(TEXT("Station completed: %s"), *Results.ToString()));

	// Add results
	OverallResults.AddStationResults(Results);

	// Check if we should stop on failure
	if (bStopOnFirstFailure && !Results.AllTestsPassed())
	{
		LogError(TEXT("Stopping tests due to station failure (bStopOnFirstFailure = true)"));
		CompleteAllTests();
		return;
	}

	// Execute next station
	if (bSingleStationMode)
	{
		CompleteAllTests();
	}
	else
	{
		ExecuteNextStation();
	}
}

void ATestLevelGameMode::CompleteAllTests()
{
	bTestsRunning = false;
	OverallResults.TestRunEndTime = FDateTime::Now();
	OverallResults.TotalExecutionTimeSeconds = (OverallResults.TestRunEndTime - OverallResults.TestRunStartTime).GetTotalSeconds();

	LogInfo(TEXT("=== ALL TESTS COMPLETED ==="));
	LogInfo(OverallResults.ToString());

	OnAllTestsCompleted.Broadcast(OverallResults);

	// Auto-export if configured
	if (bAutoStartTests)
	{
		FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
		ExportResultsToJSON(FString::Printf(TEXT("TestResults_%s.json"), *Timestamp));
		ExportResultsToCSV(FString::Printf(TEXT("TestResults_%s.csv"), *Timestamp));
	}

	// Teleport back to hub
	TeleportPlayerToHub();
}

void ATestLevelGameMode::InitializePerformanceProfiling()
{
	// Create performance profiler if not exists
	if (!PerformanceProfiler)
	{
		PerformanceProfiler = NewObject<UPerformanceProfiler>(this, UPerformanceProfiler::StaticClass());
		if (PerformanceProfiler)
		{
			PerformanceProfiler->RegisterComponent();
			LogInfo(TEXT("Performance profiler initialized"));
		}
	}

	// Create memory optimization manager if not exists
	if (!MemoryOptimizationManager)
	{
		MemoryOptimizationManager = NewObject<UMemoryOptimizationManager>(this, UMemoryOptimizationManager::StaticClass());
		if (MemoryOptimizationManager)
		{
			MemoryOptimizationManager->RegisterComponent();
			LogInfo(TEXT("Memory optimization manager initialized"));
		}
	}

	// Create tick optimization manager if not exists
	if (!TickOptimizationManager)
	{
		TickOptimizationManager = NewObject<UTickOptimizationManager>(this, UTickOptimizationManager::StaticClass());
		if (TickOptimizationManager)
		{
			TickOptimizationManager->RegisterComponent();
			LogInfo(TEXT("Tick optimization manager initialized"));
		}
	}
}

void ATestLevelGameMode::UpdatePerformanceMetrics()
{
	// Performance metrics are updated per-test in each TestStation
	// This method could aggregate metrics if needed
}

void ATestLevelGameMode::LogInfo(const FString& Message)
{
	if (bVerboseLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("[TestLevelGameMode] %s"), *Message);
	}
}

void ATestLevelGameMode::LogWarning(const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("[TestLevelGameMode] %s"), *Message);
}

void ATestLevelGameMode::LogError(const FString& Message)
{
	UE_LOG(LogTemp, Error, TEXT("[TestLevelGameMode] %s"), *Message);
	OnTestError.Broadcast(Message);
}
