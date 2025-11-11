// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SystemSelfTestInterface.h"
#include "VerificationGameMode.generated.h"

// Forward declarations
class UPerformanceSelfMonitor;
class UVerificationReportGenerator;

// Test execution order - tests core systems first, then dependent systems
UENUM(BlueprintType)
enum class EVerificationTestOrder : uint8
{
	// Test core world systems first (orbital mechanics, persistence)
	CoreWorldSystemsFirst	UMETA(DisplayName = "Core World Systems First"),
	
	// Test resource systems after core world is verified
	ResourceSystemsFirst	UMETA(DisplayName = "Resource Systems First"),
	
	// Test production systems after resources are verified
	ProductionSystemsFirst	UMETA(DisplayName = "Production Systems First"),
	
	// Test economy systems last (depends on production)
	EconomySystemsFirst		UMETA(DisplayName = "Economy Systems First"),
	
	// Test all systems in dependency order
	DependencyOrder			UMETA(DisplayName = "Dependency Order"),
	
	// Test all systems simultaneously (fastest but less safe)
	Parallel				UMETA(DisplayName = "Parallel")
};

// Verification configuration
USTRUCT(BlueprintType)
struct FVerificationConfig
{
	GENERATED_BODY()

	// Test execution order
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	EVerificationTestOrder TestOrder;

	// Enable performance monitoring
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	bool bEnablePerformanceMonitoring;

	// Enable visual verification (rendering tests)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	bool bEnableVisualVerification;

	// Exit game when verification completes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	bool bExitOnComplete;

	// Save report to file
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	bool bSaveReportToFile;

	// Report file path
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	FString ReportFilePath;

	// Maximum test duration per system (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	float MaxTestDurationPerSystem;

	// Target FPS for performance tests
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	float TargetFPS;

	// Maximum memory usage in MB
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	float MaxMemoryUsageMB;

	FVerificationConfig()
	{
		TestOrder = EVerificationTestOrder::DependencyOrder;
		bEnablePerformanceMonitoring = true;
		bEnableVisualVerification = true;
		bExitOnComplete = true;
		bSaveReportToFile = true;
		ReportFilePath = "verification_report.json";
		MaxTestDurationPerSystem = 30.0f;
		TargetFPS = 90.0f;
		MaxMemoryUsageMB = 8192.0f;
	}
};

// Overall verification result
USTRUCT(BlueprintType)
struct FVerificationResult
{
	GENERATED_BODY()

	// Did all tests pass?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	bool bAllTestsPassed;

	// Total test duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	float TotalDuration;

	// Number of systems tested
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	int32 SystemsTested;

	// Number of systems that passed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	int32 SystemsPassed;

	// Number of systems that failed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	int32 SystemsFailed;

	// Individual system results
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	TArray<FSystemTestResult> SystemResults;

	// Performance metrics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	TMap<FString, float> PerformanceMetrics;

	// Error messages
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	TArray<FString> ErrorMessages;

	// Warning messages
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verification")
	TArray<FString> WarningMessages;

	FVerificationResult()
	{
		bAllTestsPassed = false;
		TotalDuration = 0.0f;
		SystemsTested = 0;
		SystemsPassed = 0;
		SystemsFailed = 0;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVerificationStarted, const FVerificationConfig&, Config);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemTestCompleted, const FSystemTestResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVerificationCompleted, const FVerificationResult&, Result);

/**
 * Verification Game Mode - Automated testing game mode that verifies all game systems
 * Tests systems in dependency order: Core World -> Resources -> Production -> Economy
 */
UCLASS()
class ALEXANDER_API AVerificationGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AVerificationGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Start verification process
	UFUNCTION(BlueprintCallable, Category = "Verification")
	void StartVerification(const FVerificationConfig& Config);

	// Run complete gameplay simulation
	UFUNCTION(BlueprintCallable, Category = "Verification")
	bool SimulateCompleteGameplayLoop();

	// Test specific system by name
	UFUNCTION(BlueprintCallable, Category = "Verification")
	bool TestSpecificSystem(const FString& SystemName);

	// Get current verification status
	UFUNCTION(BlueprintCallable, Category = "Verification")
	FString GetVerificationStatus() const;

	// Get final result
	UFUNCTION(BlueprintCallable, Category = "Verification")
	FVerificationResult GetVerificationResult() const { return FinalResult; }

	// Cancel verification
	UFUNCTION(BlueprintCallable, Category = "Verification")
	void CancelVerification();

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Verification Events")
	FOnVerificationStarted OnVerificationStarted;

	UPROPERTY(BlueprintAssignable, Category = "Verification Events")
	FOnSystemTestCompleted OnSystemTestCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Verification Events")
	FOnVerificationCompleted OnVerificationCompleted;

protected:
	// Core world systems - test these first (no dependencies)
	bool TestOrbitalMechanicsManager(FSystemTestResult& OutResult);
	bool TestPersistentUniverseManager(FSystemTestResult& OutResult);

	// Resource systems - depend on core world
	bool TestPlanetaryMiningSystem(FSystemTestResult& OutResult);
	bool TestResourceGatheringSystem(FSystemTestResult& OutResult);
	bool TestAsteroidMiningSystem(FSystemTestResult& OutResult);

	// Production systems - depend on resources
	bool TestPlanetaryFarmingSystem(FSystemTestResult& OutResult);
	bool TestCraftingSystem(FSystemTestResult& OutResult);
	bool TestRefiningSystem(FSystemTestResult& OutResult);

	// Economy systems - depend on production
	bool TestTradingEconomySystem(FSystemTestResult& OutResult);
	bool TestFactionEconomyManager(FSystemTestResult& OutResult);
	bool TestDynamicMarketManager(FSystemTestResult& OutResult);
	bool TestEconomicEventManager(FSystemTestResult& OutResult);

	// Visual verification tests
	bool TestVisualRendering(FSystemTestResult& OutResult);
	bool TestPlanetaryRendering(FSystemTestResult& OutResult);
	bool TestShipRendering(FSystemTestResult& OutResult);

	// Performance tests
	bool TestPerformanceBaseline(FSystemTestResult& OutResult);

	// Helper methods
	void Log(const FString& Message);
	void LogError(const FString& Message);
	void LogWarning(const FString& Message);
	void LogSuccess(const FString& Message);

	// Find system by name
	TScriptInterface<ISystemSelfTestInterface> FindSystemByName(const FString& SystemName);

	// Test execution
	void ExecuteNextTest();
	void OnTestCompleted(const FSystemTestResult& Result);

	// Configuration
	UPROPERTY()
	FVerificationConfig CurrentConfig;

	// Final result
	UPROPERTY()
	FVerificationResult FinalResult;

	// Performance monitor
	UPROPERTY()
	UPerformanceSelfMonitor* PerformanceMonitor;

	// Report generator
	UPROPERTY()
	UVerificationReportGenerator* ReportGenerator;

	// Test state
	UPROPERTY()
	bool bVerificationInProgress;

	UPROPERTY()
	bool bVerificationComplete;

	UPROPERTY()
	float TestStartTime;

	UPROPERTY()
	TArray<FString> SystemsToTest;

	UPROPERTY()
	int32 CurrentSystemIndex;

	UPROPERTY()
	TArray<FString> CoreWorldSystems;

	UPROPERTY()
	TArray<FString> ResourceSystems;

	UPROPERTY()
	TArray<FString> ProductionSystems;

	UPROPERTY()
	TArray<FString> EconomySystems;

	// Command line parameters
	UPROPERTY()
	FString CommandLineSystemFilter;

	UPROPERTY()
	bool bCommandLineVerifyMode;
};