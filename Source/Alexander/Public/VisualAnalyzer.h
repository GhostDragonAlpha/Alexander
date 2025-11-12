// Copyright (c) 2025 Alexander Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VisualAnalyzer.generated.h"

// Object detection result
USTRUCT(BlueprintType)
struct FObjectDetectionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Detection")
	FString ObjectName;

	UPROPERTY(BlueprintReadOnly, Category = "Detection")
	FBox2D BoundingBox;

	UPROPERTY(BlueprintReadOnly, Category = "Detection")
	float Confidence;

	UPROPERTY(BlueprintReadOnly, Category = "Detection")
	bool bDetected;

	FObjectDetectionResult()
	{
		Confidence = 0.0f;
		bDetected = false;
	}
};

// Screenshot capture settings
USTRUCT(BlueprintType)
struct FScreenshotSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FString OutputDirectory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FString FilenamePrefix;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 Quality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bIncludeHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bIncludeFPS;

	FScreenshotSettings()
	{
		OutputDirectory = TEXT("Screenshots/Verification");
		FilenamePrefix = TEXT("verification");
		Quality = 90;
		bIncludeHUD = true;
		bIncludeFPS = true;
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UVisualAnalyzer : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UVisualAnalyzer();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Initialize the analyzer
	UFUNCTION(BlueprintCallable, Category = "Visual Analysis")
	void Initialize();

	// Capture screenshot
	UFUNCTION(BlueprintCallable, Category = "Visual Analysis")
	bool CaptureScreenshot(const FString& ScreenshotName);

	// Capture screenshot with custom settings
	UFUNCTION(BlueprintCallable, Category = "Visual Analysis")
	bool CaptureScreenshotWithSettings(const FString& ScreenshotName, const FScreenshotSettings& Settings);

	// Detect object in last captured screenshot
	UFUNCTION(BlueprintCallable, Category = "Visual Analysis")
	FObjectDetectionResult DetectObject(const FString& ObjectName);

	// Detect multiple objects
	UFUNCTION(BlueprintCallable, Category = "Visual Analysis")
	TArray<FObjectDetectionResult> DetectObjects(const TArray<FString>& ObjectNames);

	// Compare two screenshots
	UFUNCTION(BlueprintCallable, Category = "Visual Analysis")
	float CompareScreenshots(const FString& ScreenshotPath1, const FString& ScreenshotPath2);

	// Verify UI element is visible
	UFUNCTION(BlueprintCallable, Category = "Visual Analysis")
	bool VerifyUIElementVisible(const FString& ElementName);

	// Verify game object is visible
	UFUNCTION(BlueprintCallable, Category = "Visual Analysis")
	bool VerifyGameObjectVisible(const FString& ObjectName);

	// Get FPS from screenshot (if FPS counter visible)
	UFUNCTION(BlueprintCallable, Category = "Visual Analysis")
	float GetFPSFromScreenshot(const FString& ScreenshotPath);

	// Get last captured screenshot path
	UFUNCTION(BlueprintCallable, Category = "Visual Analysis")
	FString GetLastScreenshotPath() const { return LastScreenshotPath; }

	// Start continuous capture
	UFUNCTION(BlueprintCallable, Category = "Visual Analysis")
	void StartContinuousCapture(float Interval);

	// Stop continuous capture
	UFUNCTION(BlueprintCallable, Category = "Visual Analysis")
	void StopContinuousCapture();

	// Get capture statistics
	UFUNCTION(BlueprintCallable, Category = "Visual Analysis")
	void GetCaptureStats(int32& TotalCaptures, float& AverageInterval);

protected:
	// Capture screenshot using Unreal's screenshot system
	bool CaptureScreenshotInternal(const FString& FullPath);

	// Perform object detection using template matching
	FObjectDetectionResult PerformObjectDetection(const FString& ImagePath, const FString& ObjectName);

	// Load template image for object detection
	UTexture2D* LoadTemplateImage(const FString& ObjectName);

	// Compare images using SSIM or similar algorithm
	float CompareImages(const FString& ImagePath1, const FString& ImagePath2);

	// Extract FPS from image using OCR
	float ExtractFPSFromImage(const FString& ImagePath);

	// Continuous capture tick
	void ContinuousCaptureTick();

private:
	// Screenshot settings
	UPROPERTY(EditAnywhere, Category = "Settings")
	FScreenshotSettings ScreenshotSettings;

	// Last captured screenshot path
	UPROPERTY( Category = "State")
	FString LastScreenshotPath;

	// Capture statistics
	UPROPERTY( Category = "Statistics")
	int32 TotalCaptures;

	UPROPERTY( Category = "Statistics")
	float TotalCaptureTime;

	// Continuous capture active
	UPROPERTY( Category = "State")
	bool bContinuousCaptureActive;

	// Capture interval
	UPROPERTY(EditAnywhere, Category = "Settings")
	float CaptureInterval;

	// Timer handle for continuous capture
	FTimerHandle ContinuousCaptureTimer;

	// Template images for object detection
	UPROPERTY()
	TMap<FString, UTexture2D*> TemplateImages;

	// Capture timestamps for statistics
	TArray<float> CaptureTimestamps;
};