// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Math/UnrealMathUtility.h"
#include "VRHandSimulator.generated.h"

/**
 * Motion arc type for hand movement simulation
 */
UENUM(BlueprintType)
enum class EMotionArcType : uint8
{
	Linear UMETA(DisplayName = "Linear"),
	Bezier UMETA(DisplayName = "Bezier Curve"),
	Parabolic UMETA(DisplayName = "Parabolic Arc"),
	Circular UMETA(DisplayName = "Circular Arc"),
	Natural UMETA(DisplayName = "Natural Human Motion")
};

/**
 * Motion profile parameters
 */
USTRUCT(BlueprintType)
struct FMotionProfile
{
	GENERATED_BODY()

	// Starting position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector StartPosition = FVector::ZeroVector;

	// Target position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetPosition = FVector::ZeroVector;

	// Motion duration (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 1.0f;

	// Arc height (for parabolic/natural motion)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ArcHeight = 10.0f;

	// Motion type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMotionArcType ArcType = EMotionArcType::Natural;

	// Ease in/out factor (0 = constant velocity, 1 = smooth acceleration)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EaseFactor = 0.5f;
};

/**
 * VR Hand Simulator
 * Simulates realistic VR hand movements using calculus-based motion arcs
 * Perfect for automated testing without VR hardware
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UVRHandSimulator : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRHandSimulator();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ============================================================================
	// HAND SIMULATION
	// ============================================================================

	// Start moving hand from current position to target
	UFUNCTION(BlueprintCallable, Category = "VR Hand Simulation")
	void MoveHandTo(const FVector& TargetPosition, float Duration = 1.0f, EMotionArcType ArcType = EMotionArcType::Natural);

	// Start grabbing motion towards object
	UFUNCTION(BlueprintCallable, Category = "VR Hand Simulation")
	void SimulateGrab(AActor* TargetObject, float ReachDuration = 0.5f);

	// Release object and return hand to rest position
	UFUNCTION(BlueprintCallable, Category = "VR Hand Simulation")
	void SimulateRelease(const FVector& RestPosition, float ReturnDuration = 0.5f);

	// Simulate holding stick and applying input
	UFUNCTION(BlueprintCallable, Category = "VR Hand Simulation")
	void SimulateStickInput(const FVector& InputDirection, float InputMagnitude, float Duration = 2.0f);

	// Stop current motion
	UFUNCTION(BlueprintCallable, Category = "VR Hand Simulation")
	void StopMotion();

	// Get current hand position
	UFUNCTION(BlueprintCallable, Category = "VR Hand Simulation")
	FVector GetHandPosition() const { return CurrentHandPosition; }

	// Is hand currently moving?
	UFUNCTION(BlueprintCallable, Category = "VR Hand Simulation")
	bool IsMoving() const { return bIsMoving; }

	// ============================================================================
	// MOTION CALCULATION (Calculus-based)
	// ============================================================================

	// Calculate position along motion arc using parametric equations
	UFUNCTION(BlueprintCallable, Category = "Motion Calculation")
	FVector CalculateArcPosition(float NormalizedTime, const FMotionProfile& Profile);

	// Calculate velocity vector at given time (derivative of position)
	UFUNCTION(BlueprintCallable, Category = "Motion Calculation")
	FVector CalculateVelocity(float NormalizedTime, const FMotionProfile& Profile);

	// Calculate acceleration vector (second derivative)
	UFUNCTION(BlueprintCallable, Category = "Motion Calculation")
	FVector CalculateAcceleration(float NormalizedTime, const FMotionProfile& Profile);

	// Apply easing function (for smooth acceleration/deceleration)
	UFUNCTION(BlueprintCallable, Category = "Motion Calculation")
	float ApplyEasing(float T, float EaseFactor);

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Default hand rest position (relative to HMD)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Hand Config")
	FVector DefaultRestPosition = FVector(30.0f, -20.0f, -15.0f);

	// Maximum reach distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Hand Config")
	float MaxReachDistance = 80.0f;

	// Natural motion arc height factor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Hand Config")
	float NaturalArcHeight = 5.0f;

	// Enable motion smoothing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Hand Config")
	bool bSmoothMotion = true;

	// Enable debug visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Hand Config")
	bool bDebugVisualization = true;

protected:
	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	// Current hand position
	UPROPERTY()
	FVector CurrentHandPosition;

	// Current motion profile
	UPROPERTY()
	FMotionProfile ActiveMotionProfile;

	// Is currently moving?
	bool bIsMoving = false;

	// Current motion elapsed time
	float MotionElapsedTime = 0.0f;

	// Currently held object
	UPROPERTY()
	AActor* HeldObject = nullptr;

	// ============================================================================
	// MOTION CALCULATION HELPERS
	// ============================================================================

	// Linear interpolation
	FVector CalculateLinearMotion(float T, const FVector& Start, const FVector& End);

	// Bezier curve (cubic Bezier with automatic control points)
	FVector CalculateBezierMotion(float T, const FVector& Start, const FVector& End, float ArcHeight);

	// Parabolic arc (physics-based ballistic trajectory)
	FVector CalculateParabolicMotion(float T, const FVector& Start, const FVector& End, float ArcHeight);

	// Circular arc (arc along circle segment)
	FVector CalculateCircularMotion(float T, const FVector& Start, const FVector& End, float ArcHeight);

	// Natural human motion (combination of bezier + ease + slight upward bias)
	FVector CalculateNaturalMotion(float T, const FVector& Start, const FVector& End, float ArcHeight);

	// Ease-in-out function (smooth start and stop)
	float EaseInOutCubic(float T);

	// Ease-in-out quintic (smoother)
	float EaseInOutQuintic(float T);

	// Update debug visualization
	void DrawDebugMotionPath();
};
