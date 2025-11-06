// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Testing/TestStation.h"
#include "AIVisionTestStation.generated.h"

// Forward declarations
// class AAIScreenshotCapture; // TODO: Create this class
class UTexture2D;

/**
 * AI Vision Test Station
 *
 * Tests AI-powered vision system for automated screenshot capture,
 * scene analysis, code generation, and iterative development.
 *
 * Test Cases:
 * 1. Screenshot Capture - Test automated screenshot system
 * 2. Scene Analysis - Test AI vision analysis backend
 * 3. Code Generation - Test Blueprint/C++/material generation
 * 4. Iteration Testing - Test feedback loop and convergence
 * 5. Multiple Scenes - Test various test scenes with validation markers
 * 6. Quality Validation - Verify AI suggestions quality
 * 7. Integration Test - Full capture-analyze-generate cycle
 *
 * Requirements: 6.1, 6.2, 6.3, 6.4, 6.5
 */
UCLASS(ClassGroup=(Testing), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UAIVisionTestStation : public UTestStation
{
	GENERATED_BODY()

public:
	UAIVisionTestStation();

protected:
	virtual void BeginPlay() override;
	virtual void RegisterTests_Implementation() override;

public:
	// Test configuration
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Vision Testing|Config")
	// TSubclassOf<AAIScreenshotCapture> ScreenshotCaptureClass; // TODO: Create AAIScreenshotCapture class

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Vision Testing|Config")
	FString AIBackendURL = TEXT("http://localhost:8000");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Vision Testing|Config")
	FString ScreenshotOutputDirectory = TEXT("Saved/Screenshots/AIVision");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Vision Testing|Config")
	int32 ScreenshotWidth = 1920;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Vision Testing|Config")
	int32 ScreenshotHeight = 1080;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Vision Testing|Config")
	float AnalysisTimeout = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Vision Testing|Config")
	int32 MaxIterations = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Vision Testing|Config")
	float IterationDelay = 2.0f;

	// Test scene markers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Vision Testing|Scenes")
	TArray<FString> TestSceneNames = {
		TEXT("SpaceFarming"),
		TEXT("VRCockpit"),
		TEXT("PlanetarySurface"),
		TEXT("OrbitalStation"),
		TEXT("AsteroidField")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Vision Testing|Scenes")
	TArray<FVector> ValidationMarkerLocations;

protected:
	// Test case implementations

	/** Test 1: Screenshot Capture - Automated Screenshot System */
	void Test_ScreenshotCapture();

	/** Test 2: Screenshot Capture - Metadata Collection */
	void Test_MetadataCollection();

	/** Test 3: Screenshot Capture - Multi-Scene Capture */
	void Test_MultiSceneCapture();

	/** Test 4: Scene Analysis - Backend Connection */
	void Test_BackendConnection();

	/** Test 5: Scene Analysis - Image Upload */
	void Test_ImageUpload();

	/** Test 6: Scene Analysis - AI Response Parsing */
	void Test_AIResponseParsing();

	/** Test 7: Scene Analysis - Quality Assessment */
	void Test_QualityAssessment();

	/** Test 8: Code Generation - Blueprint Generation */
	void Test_BlueprintGeneration();

	/** Test 9: Code Generation - C++ Code Generation */
	void Test_CppCodeGeneration();

	/** Test 10: Code Generation - Material Generation */
	void Test_MaterialGeneration();

	/** Test 11: Code Generation - Code Validation */
	void Test_CodeValidation();

	/** Test 12: Iteration Testing - Feedback Loop */
	void Test_FeedbackLoop();

	/** Test 13: Iteration Testing - Convergence Detection */
	void Test_ConvergenceDetection();

	/** Test 14: Iteration Testing - Improvement Tracking */
	void Test_ImprovementTracking();

	/** Test 15: Multiple Scenes - Scene Switching */
	void Test_SceneSwitching();

	/** Test 16: Multiple Scenes - Marker Validation */
	void Test_MarkerValidation();

	/** Test 17: Multiple Scenes - Cross-Scene Consistency */
	void Test_CrossSceneConsistency();

	/** Test 18: Quality Validation - Suggestion Accuracy */
	void Test_SuggestionAccuracy();

	/** Test 19: Quality Validation - Code Quality Metrics */
	void Test_CodeQualityMetrics();

	/** Test 20: Integration Test - Full Pipeline */
	void Test_FullPipeline();

	// Helper functions
	bool InitializeScreenshotCapture();
	void CleanupScreenshotCapture();
	bool CaptureScreenshot(const FString& Filename);
	bool SendToAIBackend(const FString& ImagePath, const FString& Metadata);
	bool WaitForAIResponse(float TimeoutSeconds);
	FString ParseAIResponse(const FString& Response);
	bool ValidateGeneratedCode(const FString& Code);
	bool ValidateMarkerVisibility(const FVector& MarkerLocation);
	float CalculateQualityScore(const FString& Suggestion);
	bool LoadTestScene(const FString& SceneName);
	void PlaceValidationMarkers();
	void ClearValidationMarkers();
	int32 CountVisibleMarkers();
	float MeasureIterationImprovement(int32 IterationIndex);

private:
	// References
	// UPROPERTY() // Commented out because AAIScreenshotCapture doesn't exist yet
	// AAIScreenshotCapture* ScreenshotCapture = nullptr; // TODO: Create AAIScreenshotCapture class

	// Test state
	TArray<FString> CapturedScreenshots;
	TArray<FString> AIResponses;
	TMap<FString, float> QualityScores;
	TArray<float> IterationImprovements;
	FString CurrentTestScene;
	int32 CurrentIteration = 0;
	bool bBackendAvailable = false;
	bool bAIResponseReceived = false;
	FString LastAIResponse;

	// Spawned validation markers
	UPROPERTY()
	TArray<AActor*> ValidationMarkers;

	// Timing data
	TArray<float> ResponseTimes;
	float LastRequestTime = 0.0f;

	// Statistics
	int32 SuccessfulCaptures = 0;
	int32 FailedCaptures = 0;
	int32 SuccessfulAnalyses = 0;
	int32 FailedAnalyses = 0;
};
