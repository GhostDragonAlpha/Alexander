// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SystemSelfTestInterface.generated.h"

// Test result item structure
USTRUCT(BlueprintType)
struct FTestResultItem
{
	GENERATED_BODY()

	// Did this specific test pass?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	bool bPassed;

	// Name of the test
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FString TestName;

	// Description of what was tested
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FString Description;

	// Test duration in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	float Duration;

	FTestResultItem()
		: bPassed(false)
		, Duration(0.0f)
	{
	}

	FTestResultItem(bool InPassed, const FString& InTestName, const FString& InDescription, float InDuration)
		: bPassed(InPassed)
		, TestName(InTestName)
		, Description(InDescription)
		, Duration(InDuration)
	{
	}
};

// Test result structure
USTRUCT(BlueprintType)
struct FSystemTestResult
{
	GENERATED_BODY()

	// System name that was tested
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FString SystemName;

	// Did the test pass?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	bool bPassed;

	// Test duration in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	float TestDuration;

	// Error message if test failed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FString ErrorMessage;

	// Error messages (multiple)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	TArray<FString> ErrorMessages;

	// Warning messages
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	TArray<FString> WarningMessages;

	// Detailed test results
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	TArray<FTestResultItem> TestResults;

	// Performance metrics collected during test
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	TMap<FString, float> PerformanceMetrics;

	FSystemTestResult()
	{
		bPassed = false;
		TestDuration = 0.0f;
	}
};

// Interface for systems that support self-testing
UINTERFACE(MinimalAPI, Blueprintable)
class USystemSelfTestInterface : public UInterface
{
	GENERATED_BODY()
};

class ISystemSelfTestInterface
{
	GENERATED_BODY()

public:
	// Run self-test on this system
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Self Test")
	bool RunSelfTest(FSystemTestResult& OutResult);

	// Get the name of this system for reporting
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Self Test")
	FString GetSystemName() const;

	// Get test description
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Self Test")
	FString GetTestDescription() const;

	// Check if system is ready for testing
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Self Test")
	bool IsReadyForTesting() const;
};