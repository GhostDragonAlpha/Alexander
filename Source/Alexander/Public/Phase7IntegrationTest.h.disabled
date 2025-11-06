#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "GameSystemCoordinator.h"
#include "Phase7IntegrationTest.generated.h"

// Test Result Structure
USTRUCT(BlueprintType)
struct FTestResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPassed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Details;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;
};

/**
 * Phase 7 Integration Test Suite
 * Comprehensive testing for all Phase 7 systems integration
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Testing))
class ALEXANDER_API UPhase7IntegrationTest : public UObject
{
    GENERATED_BODY()

public:
    UPhase7IntegrationTest();

    // Test Execution
    UFUNCTION(BlueprintCallable, Category = "Integration Test")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "Integration Test")
    bool RunIndividualTest(const FString& TestName);

    // Individual System Tests
    UFUNCTION(BlueprintCallable, Category = "Integration Test")
    bool TestAISystem();

    UFUNCTION(BlueprintCallable, Category = "Integration Test")
    bool TestVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration Test")
    bool TestAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration Test")
    bool TestTutorialSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration Test")
    bool TestCrossSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration Test")
    bool TestPerformanceOptimization();

    // Test Results
    UFUNCTION(BlueprintPure, Category = "Integration Test")
    TArray<FTestResult> GetTestResults() const;

    UFUNCTION(BlueprintPure, Category = "Integration Test")
    bool IsTestRunning() const;

    UFUNCTION(BlueprintPure, Category = "Integration Test")
    float GetTestProgress() const;

protected:
    // Test Data
    UPROPERTY(BlueprintReadOnly, Category = "Integration Test")
    TArray<FTestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Test")
    bool bTestRunning;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Test")
    int32 CurrentTestIndex;

private:
    void LogTestResult(const FString& TestName, bool bPassed, const FString& Details);
    void GenerateTestReport();
};