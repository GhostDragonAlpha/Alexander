// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameSystemsTest.generated.h"

/**
 * Test function delegate
 */
DECLARE_DELEGATE_RetVal(bool, FTestFunction);

/**
 * Test definition structure
 */
USTRUCT(BlueprintType)
struct FTestDefinition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	FString TestName;

	// Test function (C++ only)
	TFunction<void()> TestFunction;

	FTestDefinition()
	{
		TestName = TEXT("Unnamed Test");
	}

	FTestDefinition(const FString& InTestName, FTestFunction InTestFunction)
		: TestName(InTestName), TestFunction(InTestFunction)
	{
	}
};

/**
 * Comprehensive game systems test suite
 * Validates all major systems in the Alexander project
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API AGameSystemsTest : public AActor
{
	GENERATED_BODY()

public:
	AGameSystemsTest();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	// Start the complete test suite
	UFUNCTION(BlueprintCallable, Category = "Test")
	void StartTestSuite();

	// Run a specific test by name
	UFUNCTION(BlueprintCallable, Category = "Test")
	bool RunSpecificTest(const FString& TestName);

	// Get current test results
	UFUNCTION(BlueprintCallable, Category = "Test")
	FString GetTestResults() const;

	// Are tests currently running?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test")
	bool bIsTestRunning;

	// Current test index
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test")
	int32 CurrentTestIndex;

	// Number of tests passed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test")
	int32 TestsPassed;

	// Number of tests failed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test")
	int32 TestsFailed;

protected:
	// Initialize all tests
	void InitializeTests();

	// Run the next test in the suite
	void RunNextTest();

	// Handle test completion
	void OnTestCompleted(bool bPassed, const FString& Message);

	// Finish the test suite and display results
	void FinishTestSuite();

	// Update test progress (for animated tests)
	void UpdateTestProgress(float DeltaTime);

private:
	// Array of all tests to run
	TArray<FTestDefinition> Tests;

	// Test timing
	FDateTime CurrentTestStartTime;
	bool bTestTimedOut;

	// ============================================================================
	// INDIVIDUAL TEST FUNCTIONS
	// ============================================================================

	// Orbital Physics Tests
	bool TestCircularOrbit();
	bool TestBallisticTrajectory();
	bool TestEscapeVelocity();

	// Planet Systems Tests
	bool TestPlanetComponentIntegration();
	bool TestBiomeGeneration();

	// Spaceship Systems Tests
	bool TestSpaceshipFlightControls();
	bool TestVRIntegration();

	// Terrain Generation Tests
	bool TestProceduralNoise();
	bool TestAdvancedTerrainFeatures();

	// AI Vision System Tests
	bool TestAIVisionScreenshot();
	bool TestAIVisionBackend();
};