// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TestDataStructures.h"
#include "TestLevelGameMode.generated.h"

// Forward declarations
class UTestStation;
class UPerformanceProfiler;
class UMemoryOptimizationManager;
class UTickOptimizationManager;

/**
 * Test station location for navigation and teleportation
 */
USTRUCT(BlueprintType)
struct FTestStationLocation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Testing")
	FString StationName;

	UPROPERTY(BlueprintReadWrite, Category = "Testing")
	FVector TeleportLocation;

	UPROPERTY(BlueprintReadWrite, Category = "Testing")
	FRotator TeleportRotation;

	UPROPERTY(BlueprintReadWrite, Category = "Testing")
	UTestStation* StationReference = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Testing")
	FString Description;

	UPROPERTY(BlueprintReadWrite, Category = "Testing")
	bool bEnabled = true;

	FTestStationLocation()
		: StationName(TEXT("")), TeleportLocation(FVector::ZeroVector),
		  TeleportRotation(FRotator::ZeroRotator), StationReference(nullptr),
		  Description(TEXT("")), bEnabled(true)
	{}
};

/**
 * Delegates for test level events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTestLevelEvent, const FString&, EventMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAllStationsCompleted, const FTestResults&, Results);

/**
 * Game mode for the comprehensive test level
 *
 * Responsibilities:
 * - Register and manage all test stations
 * - Provide centralized test execution (run all, run single station, run single test)
 * - Handle teleportation between test stations
 * - Aggregate results from all stations
 * - Export results (CSV, JSON)
 * - Integrate with performance profiling systems
 */
UCLASS()
class ALEXANDER_API ATestLevelGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATestLevelGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	// Test station registration
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RegisterTestStation(UTestStation* Station, const FVector& TeleportLocation, const FRotator& TeleportRotation, const FString& Description = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Testing")
	void UnregisterTestStation(UTestStation* Station);

	UFUNCTION(BlueprintPure, Category = "Testing")
	TArray<FTestStationLocation> GetAllStations() const { return StationLocations; }

	UFUNCTION(BlueprintPure, Category = "Testing")
	int32 GetStationCount() const { return StationLocations.Num(); }

	// Test execution
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RunAllTests();

	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RunSingleStation(const FString& StationName);

	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RunSingleTest(const FString& StationName, const FString& TestName);

	UFUNCTION(BlueprintCallable, Category = "Testing")
	void StopAllTests();

	UFUNCTION(BlueprintCallable, Category = "Testing")
	void ResetAllTests();

	// Navigation/teleportation
	UFUNCTION(BlueprintCallable, Category = "Testing|Navigation")
	bool TeleportPlayerToStation(const FString& StationName);

	UFUNCTION(BlueprintCallable, Category = "Testing|Navigation")
	bool TeleportPlayerToHub();

	UFUNCTION(BlueprintCallable, Category = "Testing|Navigation")
	void SetHubLocation(const FVector& Location, const FRotator& Rotation);

	// Results access
	UFUNCTION(BlueprintPure, Category = "Testing|Results")
	FTestResults GetOverallResults() const { return OverallResults; }

	UFUNCTION(BlueprintPure, Category = "Testing|Results")
	bool AreTestsRunning() const { return bTestsRunning; }

	UFUNCTION(BlueprintPure, Category = "Testing|Results")
	float GetOverallProgress() const;

	UFUNCTION(BlueprintPure, Category = "Testing|Results")
	FString GetCurrentStationName() const;

	// Results export
	UFUNCTION(BlueprintCallable, Category = "Testing|Export")
	bool ExportResultsToJSON(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "Testing|Export")
	bool ExportResultsToCSV(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "Testing|Export")
	FString GetResultsJSON() const;

	UFUNCTION(BlueprintCallable, Category = "Testing|Export")
	FString GetResultsCSV() const;

	// Performance profiling integration
	UFUNCTION(BlueprintCallable, Category = "Testing|Performance")
	void EnablePerformanceProfiling(bool bEnable);

	UFUNCTION(BlueprintPure, Category = "Testing|Performance")
	UPerformanceProfiler* GetPerformanceProfiler() const { return PerformanceProfiler; }

	UFUNCTION(BlueprintPure, Category = "Testing|Performance")
	UMemoryOptimizationManager* GetMemoryOptimizationManager() const { return MemoryOptimizationManager; }

	UFUNCTION(BlueprintPure, Category = "Testing|Performance")
	UTickOptimizationManager* GetTickOptimizationManager() const { return TickOptimizationManager; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Testing|Events")
	FOnTestLevelEvent OnTestsStarted;

	UPROPERTY(BlueprintAssignable, Category = "Testing|Events")
	FOnAllStationsCompleted OnAllTestsCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Testing|Events")
	FOnTestLevelEvent OnStationChanged;

	UPROPERTY(BlueprintAssignable, Category = "Testing|Events")
	FOnTestLevelEvent OnTestError;

protected:
	// Internal test execution
	void ExecuteNextStation();
	void OnStationCompleted(const FTestStationResults& Results);
	void CompleteAllTests();

	// Performance profiling
	void InitializePerformanceProfiling();
	void UpdatePerformanceMetrics();

	// Logging
	void LogInfo(const FString& Message);
	void LogWarning(const FString& Message);
	void LogError(const FString& Message);

protected:
	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing|Config")
	bool bAutoStartTests = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing|Config")
	bool bStopOnFirstFailure = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing|Config")
	bool bEnablePerformanceProfiling = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing|Config")
	bool bVerboseLogging = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing|Config")
	FString DefaultExportDirectory = TEXT("Saved/TestResults/");

	// Hub location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing|Navigation")
	FVector HubLocation = FVector(0.0f, 0.0f, 200.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing|Navigation")
	FRotator HubRotation = FRotator::ZeroRotator;

private:
	// Test station management
	UPROPERTY()
	TArray<FTestStationLocation> StationLocations;

	UPROPERTY()
	TArray<UTestStation*> RegisteredStations;

	// Test execution state
	FTestResults OverallResults;
	int32 CurrentStationIndex = -1;
	bool bTestsRunning = false;
	bool bSingleStationMode = false;

	// Performance profiling
	UPROPERTY()
	UPerformanceProfiler* PerformanceProfiler = nullptr;

	UPROPERTY()
	UMemoryOptimizationManager* MemoryOptimizationManager = nullptr;

	UPROPERTY()
	UTickOptimizationManager* TickOptimizationManager = nullptr;

	// Platform info
	FString PlatformName;
	FString BuildConfiguration;
	FString EngineVersion;
};
