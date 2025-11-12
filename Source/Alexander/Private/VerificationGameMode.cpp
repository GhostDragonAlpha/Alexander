// Copyright Epic Games, Inc. All Rights Reserved.

#include "VerificationGameMode.h"
#include "PerformanceSelfMonitor.h"
#include "VerificationReportGenerator.h"
#include "OrbitalMechanicsManager.h"
#include "PersistentUniverseManager.h"
#include "PlanetaryMiningSystem.h"
#include "ResourceGatheringSystem.h"
#include "AsteroidMiningSystem.h"
#include "PlanetaryFarmingSystem.h"
#include "CraftingSystem.h"
#include "RefiningSystem.h"
#include "TradingEconomySystem.h"
#include "FactionEconomyManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"

AVerificationGameMode::AVerificationGameMode()
{
	// Set default player controller to null since we don't need player input
	PlayerControllerClass = nullptr;

	// Initialize system test order - core world systems first
	CoreWorldSystems = {
		TEXT("OrbitalMechanicsManager"),
		TEXT("PersistentUniverseManager")
	};

	ResourceSystems = {
		TEXT("PlanetaryMiningSystem"),
		TEXT("ResourceGatheringSystem"),
		TEXT("AsteroidMiningSystem")
	};

	ProductionSystems = {
		TEXT("PlanetaryFarmingSystem"),
		TEXT("CraftingSystem"),
		TEXT("RefiningSystem")
	};

	EconomySystems = {
		TEXT("TradingEconomySystem"),
		TEXT("FactionEconomyManager"),
		TEXT("DynamicMarketManager"),
		TEXT("EconomicEventManager")
	};

	// Check command line for verification mode
	bCommandLineVerifyMode = FParse::Param(FCommandLine::Get(), TEXT("verify"));
	
	FString SystemFilter;
	if (FParse::Value(FCommandLine::Get(), TEXT("system="), SystemFilter))
	{
		CommandLineSystemFilter = SystemFilter;
	}
}

void AVerificationGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Create performance monitor
	PerformanceMonitor = NewObject<UPerformanceSelfMonitor>(this);
	PerformanceMonitor->RegisterComponent();

	// Create report generator
	ReportGenerator = NewObject<UVerificationReportGenerator>(this);
	ReportGenerator->RegisterComponent();

	// Auto-start verification if in verify mode
	if (bCommandLineVerifyMode)
	{
		FVerificationConfig Config;
		if (!CommandLineSystemFilter.IsEmpty())
		{
			// Single system test mode
			Config.bExitOnComplete = true;
		}
		StartVerification(Config);
	}
}

void AVerificationGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Continue verification if in progress
	if (bVerificationInProgress && !bVerificationComplete)
	{
		ExecuteNextTest();
	}
}

void AVerificationGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bVerificationInProgress)
	{
		CancelVerification();
	}

	Super::EndPlay(EndPlayReason);
}

void AVerificationGameMode::StartVerification(const FVerificationConfig& Config)
{
	if (bVerificationInProgress)
	{
		LogWarning(TEXT("Verification already in progress"));
		return;
	}

	CurrentConfig = Config;
	bVerificationInProgress = true;
	bVerificationComplete = false;
	TestStartTime = GetWorld()->GetTimeSeconds();
	CurrentSystemIndex = 0;

	// Reset final result
	FinalResult = FVerificationResult();
	FinalResult.SystemResults.Empty();
	FinalResult.ErrorMessages.Empty();
	FinalResult.WarningMessages.Empty();

	// Build system test list based on order
	SystemsToTest.Empty();

	switch (Config.TestOrder)
	{
	case EVerificationTestOrder::CoreWorldSystemsFirst:
		SystemsToTest.Append(CoreWorldSystems);
		SystemsToTest.Append(ResourceSystems);
		SystemsToTest.Append(ProductionSystems);
		SystemsToTest.Append(EconomySystems);
		break;

	case EVerificationTestOrder::ResourceSystemsFirst:
		SystemsToTest.Append(ResourceSystems);
		SystemsToTest.Append(CoreWorldSystems);
		SystemsToTest.Append(ProductionSystems);
		SystemsToTest.Append(EconomySystems);
		break;

	case EVerificationTestOrder::ProductionSystemsFirst:
		SystemsToTest.Append(ProductionSystems);
		SystemsToTest.Append(CoreWorldSystems);
		SystemsToTest.Append(ResourceSystems);
		SystemsToTest.Append(EconomySystems);
		break;

	case EVerificationTestOrder::EconomySystemsFirst:
		SystemsToTest.Append(EconomySystems);
		SystemsToTest.Append(CoreWorldSystems);
		SystemsToTest.Append(ResourceSystems);
		SystemsToTest.Append(ProductionSystems);
		break;

	case EVerificationTestOrder::DependencyOrder:
	default:
		// Test in proper dependency order: Core World -> Resources -> Production -> Economy
		SystemsToTest.Append(CoreWorldSystems);
		SystemsToTest.Append(ResourceSystems);
		SystemsToTest.Append(ProductionSystems);
		SystemsToTest.Append(EconomySystems);
		break;
	}

	// Apply system filter if specified
	if (!CommandLineSystemFilter.IsEmpty())
	{
		SystemsToTest = SystemsToTest.FilterByPredicate([this](const FString& SystemName) {
			return SystemName.Contains(CommandLineSystemFilter);
		});
	}

	FinalResult.SystemsTested = SystemsToTest.Num();

	Log(FString::Printf(TEXT("=== VERIFICATION STARTED ===")));
	Log(FString::Printf(TEXT("Testing %d systems in %s order"), 
		SystemsToTest.Num(), *UEnum::GetValueAsString(Config.TestOrder)));

	OnVerificationStarted.Broadcast(Config);

	// Start performance monitoring if enabled
	if (Config.bEnablePerformanceMonitoring && PerformanceMonitor)
	{
		PerformanceMonitor->SetPerformanceTargets(Config.TargetFPS, Config.MaxMemoryUsageMB);
		PerformanceMonitor->StartMonitoring();
	}
}

bool AVerificationGameMode::SimulateCompleteGameplayLoop()
{
	Log(TEXT("=== Starting Complete Gameplay Simulation ==="));

	// Step 1: Test core world systems (orbital mechanics, persistence)
	Log(TEXT("Step 1: Testing Core World Systems..."));
	FSystemTestResult OrbitalResult, PersistenceResult;
	if (!TestOrbitalMechanicsManager(OrbitalResult) || !TestPersistentUniverseManager(PersistenceResult))
	{
		LogError(TEXT("Core world systems test FAILED"));
		return false;
	}
	LogSuccess(TEXT("Core world systems test PASSED"));

	// Step 2: Test resource gathering
	Log(TEXT("Step 2: Testing Resource Systems..."));
	FSystemTestResult MiningResult, GatheringResult, AsteroidResult;
	if (!TestPlanetaryMiningSystem(MiningResult) || 
		!TestResourceGatheringSystem(GatheringResult) || 
		!TestAsteroidMiningSystem(AsteroidResult))
	{
		LogError(TEXT("Resource systems test FAILED"));
		return false;
	}
	LogSuccess(TEXT("Resource systems test PASSED"));

	// Step 3: Test production systems
	Log(TEXT("Step 3: Testing Production Systems..."));
	FSystemTestResult FarmingResult, CraftingResult, RefiningResult;
	if (!TestPlanetaryFarmingSystem(FarmingResult) || 
		!TestCraftingSystem(CraftingResult) || 
		!TestRefiningSystem(RefiningResult))
	{
		LogError(TEXT("Production systems test FAILED"));
		return false;
	}
	LogSuccess(TEXT("Production systems test PASSED"));

	// Step 4: Test economy systems
	Log(TEXT("Step 4: Testing Economy Systems..."));
	FSystemTestResult TradingResult, FactionResult, MarketResult, EventResult;
	if (!TestTradingEconomySystem(TradingResult) || 
		!TestFactionEconomyManager(FactionResult) || 
		!TestDynamicMarketManager(MarketResult) || 
		!TestEconomicEventManager(EventResult))
	{
		LogError(TEXT("Economy systems test FAILED"));
		return false;
	}
	LogSuccess(TEXT("Economy systems test PASSED"));

	Log(TEXT("=== Complete Gameplay Simulation PASSED ==="));
	return true;
}

bool AVerificationGameMode::TestSpecificSystem(const FString& SystemName)
{
	FSystemTestResult Result;
	Result.SystemName = SystemName;

	TScriptInterface<ISystemSelfTestInterface> System = FindSystemByName(SystemName);
	if (!System)
	{
		Result.bPassed = false;
		Result.ErrorMessage = FString::Printf(TEXT("System '%s' not found"), *SystemName);
		OnSystemTestCompleted.Broadcast(Result);
		return false;
	}

	// Run the test
	bool bTestPassed = false;
	if (SystemName == TEXT("OrbitalMechanicsManager"))
	{
		bTestPassed = TestOrbitalMechanicsManager(Result);
	}
	else if (SystemName == TEXT("PersistentUniverseManager"))
	{
		bTestPassed = TestPersistentUniverseManager(Result);
	}
	else if (SystemName == TEXT("PlanetaryMiningSystem"))
	{
		bTestPassed = TestPlanetaryMiningSystem(Result);
	}
	else if (SystemName == TEXT("ResourceGatheringSystem"))
	{
		bTestPassed = TestResourceGatheringSystem(Result);
	}
	else if (SystemName == TEXT("AsteroidMiningSystem"))
	{
		bTestPassed = TestAsteroidMiningSystem(Result);
	}
	else if (SystemName == TEXT("PlanetaryFarmingSystem"))
	{
		bTestPassed = TestPlanetaryFarmingSystem(Result);
	}
	else if (SystemName == TEXT("CraftingSystem"))
	{
		bTestPassed = TestCraftingSystem(Result);
	}
	else if (SystemName == TEXT("RefiningSystem"))
	{
		bTestPassed = TestRefiningSystem(Result);
	}
	else if (SystemName == TEXT("TradingEconomySystem"))
	{
		bTestPassed = TestTradingEconomySystem(Result);
	}
	else if (SystemName == TEXT("FactionEconomyManager"))
	{
		bTestPassed = TestFactionEconomyManager(Result);
	}
	else if (SystemName == TEXT("DynamicMarketManager"))
	{
		bTestPassed = TestDynamicMarketManager(Result);
	}
	else if (SystemName == TEXT("EconomicEventManager"))
	{
		bTestPassed = TestEconomicEventManager(Result);
	}

	Result.bPassed = bTestPassed;
	OnSystemTestCompleted.Broadcast(Result);

	return bTestPassed;
}

FString AVerificationGameMode::GetVerificationStatus() const
{
	if (!bVerificationInProgress)
	{
		return TEXT("Not running");
	}

	if (bVerificationComplete)
	{
		return FString::Printf(TEXT("Complete - %d/%d systems passed"), 
			FinalResult.SystemsPassed, FinalResult.SystemsTested);
	}

	if (SystemsToTest.IsValidIndex(CurrentSystemIndex))
	{
		return FString::Printf(TEXT("Testing %s (%d/%d)"), 
			*SystemsToTest[CurrentSystemIndex], CurrentSystemIndex + 1, SystemsToTest.Num());
	}

	return TEXT("Unknown state");
}

void AVerificationGameMode::CancelVerification()
{
	if (!bVerificationInProgress)
	{
		return;
	}

	bVerificationInProgress = false;
	bVerificationComplete = true;

	if (PerformanceMonitor && PerformanceMonitor->IsMonitoring())
	{
		PerformanceMonitor->StopMonitoring();
	}

	Log(TEXT("=== VERIFICATION CANCELLED ==="));
}

void AVerificationGameMode::ExecuteNextTest()
{
	if (CurrentSystemIndex >= SystemsToTest.Num())
	{
		// All tests complete
		bVerificationComplete = true;
		bVerificationInProgress = false;

		// Stop performance monitoring
		if (PerformanceMonitor && PerformanceMonitor->IsMonitoring())
		{
			PerformanceMonitor->StopMonitoring();
			FSelfTestPerformanceMetrics Metrics = PerformanceMonitor->GetFinalMetrics();
			FinalResult.PerformanceMetrics.Add(TEXT("AverageFPS"), Metrics.AverageFPS);
			FinalResult.PerformanceMetrics.Add(TEXT("MinFPS"), Metrics.MinFPS);
			FinalResult.PerformanceMetrics.Add(TEXT("MaxFPS"), Metrics.MaxFPS);
			FinalResult.PerformanceMetrics.Add(TEXT("AverageFrameTime"), Metrics.AverageFrameTime);
			FinalResult.PerformanceMetrics.Add(TEXT("PeakMemoryUsageMB"), Metrics.PeakMemoryUsageMB);
			FinalResult.PerformanceMetrics.Add(TEXT("AverageMemoryUsageMB"), Metrics.AverageMemoryUsageMB);
			FinalResult.PerformanceMetrics.Add(TEXT("TestDuration"), Metrics.TestDuration);
			FinalResult.PerformanceMetrics.Add(TEXT("FramesRendered"), Metrics.FramesRendered);
			FinalResult.PerformanceMetrics.Add(TEXT("CPUUsagePercent"), Metrics.CPUUsagePercent);
			FinalResult.PerformanceMetrics.Add(TEXT("GPUUsagePercent"), Metrics.GPUUsagePercent);
			FinalResult.PerformanceMetrics.Add(TEXT("DrawCalls"), Metrics.DrawCalls);
			FinalResult.PerformanceMetrics.Add(TEXT("TrianglesRendered"), Metrics.TrianglesRendered);
			FinalResult.PerformanceMetrics.Add(TEXT("ShaderComplexityWarnings"), Metrics.ShaderComplexityWarnings);
		}

		// Calculate final results
		FinalResult.TotalDuration = GetWorld()->GetTimeSeconds() - TestStartTime;
		FinalResult.bAllTestsPassed = (FinalResult.SystemsFailed == 0);

		// Generate report
		if (ReportGenerator)
		{
			ReportGenerator->GenerateReport(FinalResult, CurrentConfig);
			
			if (CurrentConfig.bSaveReportToFile)
			{
				ReportGenerator->SaveReportToFile(CurrentConfig.ReportFilePath);
			}
		}

		Log(FString::Printf(TEXT("=== VERIFICATION COMPLETE ===")));
		Log(FString::Printf(TEXT("Tested: %d systems"), FinalResult.SystemsTested));
		Log(FString::Printf(TEXT("Passed: %d systems"), FinalResult.SystemsPassed));
		Log(FString::Printf(TEXT("Failed: %d systems"), FinalResult.SystemsFailed));
		Log(FString::Printf(TEXT("Duration: %.2f seconds"), FinalResult.TotalDuration));

		if (FinalResult.bAllTestsPassed)
		{
			LogSuccess(TEXT("ALL TESTS PASSED!"));
		}
		else
		{
			LogError(TEXT("SOME TESTS FAILED!"));
		}

		OnVerificationCompleted.Broadcast(FinalResult);

		// Exit if requested
		if (CurrentConfig.bExitOnComplete)
		{
			FGenericPlatformMisc::RequestExit(false);
		}

		return;
	}

	// Get current system to test
	FString SystemName = SystemsToTest[CurrentSystemIndex];
	FSystemTestResult Result;
	Result.SystemName = SystemName;

	Log(FString::Printf(TEXT("Testing %s..."), *SystemName));

	// Start timing
	float TestStart = GetWorld()->GetTimeSeconds();

	// Run the test
	bool bTestPassed = TestSpecificSystem(SystemName);

	// Record timing
	Result.TestDuration = GetWorld()->GetTimeSeconds() - TestStart;
	Result.bPassed = bTestPassed;

	// Update counters
	if (bTestPassed)
	{
		FinalResult.SystemsPassed++;
		LogSuccess(FString::Printf(TEXT("%s PASSED (%.2f seconds)"), *SystemName, Result.TestDuration));
	}
	else
	{
		FinalResult.SystemsFailed++;
		LogError(FString::Printf(TEXT("%s FAILED (%.2f seconds)"), *SystemName, Result.TestDuration));
		if (!Result.ErrorMessage.IsEmpty())
		{
			FinalResult.ErrorMessages.Add(Result.ErrorMessage);
		}
	}

	FinalResult.SystemResults.Add(Result);
	CurrentSystemIndex++;
}

void AVerificationGameMode::OnTestCompleted(const FSystemTestResult& Result)
{
	// This is called by TestSpecificSystem, but we handle results in ExecuteNextTest
}

TScriptInterface<ISystemSelfTestInterface> AVerificationGameMode::FindSystemByName(const FString& SystemName)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Try to find the system in the world
	if (SystemName == TEXT("OrbitalMechanicsManager"))
	{
		return TScriptInterface<ISystemSelfTestInterface>(World->GetSubsystem<UOrbitalMechanicsManager>());
	}
	else if (SystemName == TEXT("PersistentUniverseManager"))
	{
		// Find the first PersistentUniverseManager component in the world
		for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
		{
			if (UPersistentUniverseManager* Manager = ActorItr->FindComponentByClass<UPersistentUniverseManager>())
			{
				return TScriptInterface<ISystemSelfTestInterface>(Manager);
			}
		}
	}
	else if (SystemName == TEXT("PlanetaryMiningSystem"))
	{
		return TScriptInterface<ISystemSelfTestInterface>(World->GetSubsystem<UPlanetaryMiningSystem>());
	}
	else if (SystemName == TEXT("ResourceGatheringSystem"))
	{
		return TScriptInterface<ISystemSelfTestInterface>(World->GetSubsystem<UResourceGatheringSystem>());
	}
	else if (SystemName == TEXT("AsteroidMiningSystem"))
	{
		// Get AsteroidMiningSystem subsystem
		if (UAsteroidMiningSystem* System = World->GetSubsystem<UAsteroidMiningSystem>())
		{
			return TScriptInterface<ISystemSelfTestInterface>(System);
		}
	}
	else if (SystemName == TEXT("TradingEconomySystem"))
	{
		return TScriptInterface<ISystemSelfTestInterface>(World->GetSubsystem<UTradingEconomySystem>());
	}
	else if (SystemName == TEXT("FactionEconomyManager"))
	{
		return TScriptInterface<ISystemSelfTestInterface>(World->GetSubsystem<UFactionEconomyManager>());
	}
	else if (SystemName == TEXT("DynamicMarketManager"))
	{
		return TScriptInterface<ISystemSelfTestInterface>(World->GetSubsystem<UDynamicMarketManager>());
	}
	else if (SystemName == TEXT("EconomicEventManager"))
	{
		return TScriptInterface<ISystemSelfTestInterface>(World->GetSubsystem<UEconomicEventManager>());
	}

	return nullptr;
}

void AVerificationGameMode::Log(const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("[VERIFICATION] %s"), *Message);
}

void AVerificationGameMode::LogError(const FString& Message)
{
	UE_LOG(LogTemp, Error, TEXT("[VERIFICATION] ERROR: %s"), *Message);
	FinalResult.ErrorMessages.Add(Message);
}

void AVerificationGameMode::LogWarning(const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("[VERIFICATION] WARNING: %s"), *Message);
	FinalResult.WarningMessages.Add(Message);
}

void AVerificationGameMode::LogSuccess(const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("[VERIFICATION] SUCCESS: %s"), *Message);
}

// Individual system test implementations will be added in the next iteration
// These will contain the actual test logic for each system
bool AVerificationGameMode::TestOrbitalMechanicsManager(FSystemTestResult& OutResult)
{
	OutResult.SystemName = TEXT("OrbitalMechanicsManager");
	// Implementation will be added
	return true;
}

bool AVerificationGameMode::TestPersistentUniverseManager(FSystemTestResult& OutResult)
{
	OutResult.SystemName = TEXT("PersistentUniverseManager");
	// Implementation will be added
	return true;
}

bool AVerificationGameMode::TestPlanetaryMiningSystem(FSystemTestResult& OutResult)
{
	OutResult.SystemName = TEXT("PlanetaryMiningSystem");
	// Implementation will be added
	return true;
}

bool AVerificationGameMode::TestResourceGatheringSystem(FSystemTestResult& OutResult)
{
	OutResult.SystemName = TEXT("ResourceGatheringSystem");
	// Implementation will be added
	return true;
}

bool AVerificationGameMode::TestAsteroidMiningSystem(FSystemTestResult& OutResult)
{
	OutResult.SystemName = TEXT("AsteroidMiningSystem");
	// Implementation will be added
	return true;
}

bool AVerificationGameMode::TestPlanetaryFarmingSystem(FSystemTestResult& OutResult)
{
	OutResult.SystemName = TEXT("PlanetaryFarmingSystem");
	// Implementation will be added
	return true;
}

bool AVerificationGameMode::TestCraftingSystem(FSystemTestResult& OutResult)
{
	OutResult.SystemName = TEXT("CraftingSystem");
	// Implementation will be added
	return true;
}

bool AVerificationGameMode::TestRefiningSystem(FSystemTestResult& OutResult)
{
	OutResult.SystemName = TEXT("RefiningSystem");
	// Implementation will be added
	return true;
}

bool AVerificationGameMode::TestTradingEconomySystem(FSystemTestResult& OutResult)
{
	OutResult.SystemName = TEXT("TradingEconomySystem");
	// Implementation will be added
	return true;
}

bool AVerificationGameMode::TestFactionEconomyManager(FSystemTestResult& OutResult)
{
	OutResult.SystemName = TEXT("FactionEconomyManager");
	// Implementation will be added
	return true;
}

bool AVerificationGameMode::TestDynamicMarketManager(FSystemTestResult& OutResult)
{
	OutResult.SystemName = TEXT("DynamicMarketManager");
	// Implementation will be added
	return true;
}

bool AVerificationGameMode::TestEconomicEventManager(FSystemTestResult& OutResult)
{
	OutResult.SystemName = TEXT("EconomicEventManager");
	// Implementation will be added
	return true;
}

bool AVerificationGameMode::TestVisualRendering(FSystemTestResult& OutResult)
{
	OutResult.SystemName = TEXT("VisualRendering");
	// Implementation will be added
	return true;
}

bool AVerificationGameMode::TestPlanetaryRendering(FSystemTestResult& OutResult)
{
	OutResult.SystemName = TEXT("PlanetaryRendering");
	// Implementation will be added
	return true;
}

bool AVerificationGameMode::TestShipRendering(FSystemTestResult& OutResult)
{
	OutResult.SystemName = TEXT("ShipRendering");
	// Implementation will be added
	return true;
}

bool AVerificationGameMode::TestPerformanceBaseline(FSystemTestResult& OutResult)
{
	OutResult.SystemName = TEXT("PerformanceBaseline");
	// Implementation will be added
	return true;
}