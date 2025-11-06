// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Testing/TestStation.h"
#include "VRTestStation.generated.h"

// Forward declarations
class AVRSpaceshipPawn;
class UMotionControllerComponent;
class UVRPlantingInteractionManager;
class UVRHarvestInteractionManager;

/**
 * VR Test Station
 *
 * Tests:
 * 1. VR Calibration - Headset detection, tracking validation, origin setup
 * 2. Hand Tracking - Controller detection, position accuracy, rotation accuracy
 * 3. Interaction - Grab mechanics, throw physics, haptic feedback
 * 4. UI Interaction - Widget interaction, button presses, laser pointer
 * 5. Comfort - Frame rate (90+ FPS), judder detection, motion sickness prevention
 *
 * Requirements: 1.1, 1.2, 1.3, 1.4, 1.5
 */
UCLASS(ClassGroup=(Testing), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UVRTestStation : public UTestStation
{
	GENERATED_BODY()

public:
	UVRTestStation();

protected:
	virtual void BeginPlay() override;
	virtual void RegisterTests_Implementation() override;

public:
	// Test configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Testing|Config")
	int32 InteractionObjectCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Testing|Config")
	float TrackingAccuracyTolerance = 1.0f; // cm

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Testing|Config")
	float MinVRFPS = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Testing|Config")
	float TestDurationSeconds = 5.0f;

	// Test objects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Testing|Objects")
	TSubclassOf<AActor> InteractionObjectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Testing|Objects")
	TSubclassOf<AActor> UITestPanelClass;

protected:
	// Test case implementations

	/** Test 1.1: VR Calibration - Headset Detection */
	void Test_VRHeadsetDetection();

	/** Test 1.2: VR Calibration - Tracking Validation */
	void Test_VRTrackingValidation();

	/** Test 1.3: VR Calibration - Origin Setup */
	void Test_VROriginSetup();

	/** Test 2.1: Hand Tracking - Controller Detection */
	void Test_ControllerDetection();

	/** Test 2.2: Hand Tracking - Position Accuracy */
	void Test_HandPositionAccuracy();

	/** Test 2.3: Hand Tracking - Rotation Accuracy */
	void Test_HandRotationAccuracy();

	/** Test 3.1: Interaction - Grab Mechanics */
	void Test_GrabMechanics();

	/** Test 3.2: Interaction - Throw Physics */
	void Test_ThrowPhysics();

	/** Test 3.3: Interaction - Haptic Feedback */
	void Test_HapticFeedback();

	/** Test 4.1: UI Interaction - Widget Interaction */
	void Test_WidgetInteraction();

	/** Test 4.2: UI Interaction - Button Presses */
	void Test_ButtonPresses();

	/** Test 4.3: UI Interaction - Laser Pointer */
	void Test_LaserPointer();

	/** Test 5.1: Comfort - Frame Rate */
	void Test_VRFrameRate();

	/** Test 5.2: Comfort - Judder Detection */
	void Test_JudderDetection();

	/** Test 5.3: Comfort - Motion Smoothness */
	void Test_MotionSmoothness();

	// Helper functions
	bool IsVRSystemAvailable() const;
	bool IsControllerTracked(bool bLeftHand) const;
	FVector GetControllerPosition(bool bLeftHand) const;
	FRotator GetControllerRotation(bool bLeftHand) const;
	void SpawnInteractionObjects();
	void CleanupInteractionObjects();
	float MeasureFrameRate(float DurationSeconds);
	bool DetectJudder(float DurationSeconds);

private:
	// References
	UPROPERTY()
	AVRSpaceshipPawn* VRPawn = nullptr;

	UPROPERTY()
	UMotionControllerComponent* LeftController = nullptr;

	UPROPERTY()
	UMotionControllerComponent* RightController = nullptr;

	// Spawned test objects
	UPROPERTY()
	TArray<AActor*> SpawnedTestObjects;

	// Test tracking data
	TArray<FVector> LeftHandPositions;
	TArray<FVector> RightHandPositions;
	TArray<float> FrameRateSamples;
	TArray<float> FrameTimeSamples;

	// Test state
	bool bVRSystemChecked = false;
	bool bVRSystemAvailable = false;
};
