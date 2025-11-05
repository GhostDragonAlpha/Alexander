// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRPlantingGestureDetector.generated.h"

// Forward declarations
class AVRSeedItem;
class AFarmPlot;

/**
 * Planting gesture types
 */
UENUM(BlueprintType)
enum class EPlantingGesture : uint8
{
	None UMETA(DisplayName = "None"),
	DownwardMotion UMETA(DisplayName = "Downward Motion"),
	ReleaseNearGround UMETA(DisplayName = "Release Near Ground"),
	PressAndRelease UMETA(DisplayName = "Press and Release")
};

/**
 * Gesture detection result
 */
USTRUCT(BlueprintType)
struct FGestureDetectionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Gesture")
	bool bGestureDetected;

	UPROPERTY(BlueprintReadOnly, Category = "Gesture")
	EPlantingGesture GestureType;

	UPROPERTY(BlueprintReadOnly, Category = "Gesture")
	float Confidence; // 0-1

	UPROPERTY(BlueprintReadOnly, Category = "Gesture")
	FVector GestureLocation;

	FGestureDetectionResult()
		: bGestureDetected(false)
		, GestureType(EPlantingGesture::None)
		, Confidence(0.0f)
		, GestureLocation(FVector::ZeroVector)
	{
	}
};

/**
 * Component for detecting VR planting gestures
 * Analyzes hand motion to determine when player intends to plant a seed
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UVRPlantingGestureDetector : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRPlantingGestureDetector();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// GESTURE DETECTION
	// ============================================================================

	/**
	 * Start tracking gesture for a seed item
	 */
	UFUNCTION(BlueprintCallable, Category = "Gesture")
	void StartTracking(AVRSeedItem* SeedItem, AActor* HandActor);

	/**
	 * Stop tracking current gesture
	 */
	UFUNCTION(BlueprintCallable, Category = "Gesture")
	void StopTracking();

	/**
	 * Check if a planting gesture was detected
	 */
	UFUNCTION(BlueprintCallable, Category = "Gesture")
	FGestureDetectionResult DetectGesture();

	/**
	 * Reset gesture detection state
	 */
	UFUNCTION(BlueprintCallable, Category = "Gesture")
	void ResetGesture();

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Minimum downward velocity to trigger gesture (cm/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture|Detection")
	float MinDownwardVelocity;

	// Maximum height above ground for planting (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture|Detection")
	float MaxPlantingHeight;

	// Time window for gesture detection (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture|Detection")
	float GestureTimeWindow;

	// Minimum confidence threshold for gesture detection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture|Detection")
	float MinConfidenceThreshold;

	// Enable debug visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture|Debug")
	bool bDebugVisualization;

protected:
	// Currently tracked seed item
	UPROPERTY()
	AVRSeedItem* TrackedSeedItem;

	// Hand actor holding the seed
	UPROPERTY()
	AActor* TrackedHandActor;

	// Tracking state
	UPROPERTY()
	bool bIsTracking;

	// Motion history for gesture analysis
	TArray<FVector> PositionHistory;
	TArray<float> TimeHistory;

	// Maximum history size
	UPROPERTY(EditAnywhere, Category = "Gesture|Detection")
	int32 MaxHistorySize;

	// Current gesture state
	EPlantingGesture CurrentGesture;
	float GestureConfidence;
	float GestureStartTime;

	// ============================================================================
	// GESTURE ANALYSIS
	// ============================================================================

	// Update motion history
	void UpdateMotionHistory(FVector CurrentPosition, float CurrentTime);

	// Analyze motion for downward gesture
	bool DetectDownwardMotion(float& OutConfidence);

	// Check if hand is near ground
	bool IsNearGround(FVector Position, float& OutHeight);

	// Calculate velocity from motion history
	FVector CalculateVelocity();

	// Calculate average velocity over time window
	FVector CalculateAverageVelocity(float TimeWindow);

	// Draw debug visualization
	void DrawDebugInfo();
};
