// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "FlightController.generated.h"

/**
 * Flight assist modes
 */
UENUM(BlueprintType)
enum class EFlightAssistMode : uint8
{
	None UMETA(DisplayName = "None"),
	Stability UMETA(DisplayName = "Stability"),
	AutoLevel UMETA(DisplayName = "Auto Level"),
	Orbital UMETA(DisplayName = "Orbital"),
	Docking UMETA(DisplayName = "Docking")
};

/**
 * Input smoothing configuration
 */
USTRUCT(BlueprintType)
struct FInputSmoothingConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing")
	float ThrottleSmoothing = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing")
	float RotationSmoothing = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing")
	float TranslationSmoothing = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing")
	float Deadzone = 0.05f;
};

/**
 * Flight Controller - handles all spaceship input and flight assistance
 * Provides realistic 6DOF flight with multiple assist modes
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UFlightController : public UActorComponent
{
	GENERATED_BODY()

public:
	UFlightController();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// ============================================================================
	// INPUT CONFIGURATION
	// ============================================================================

	// Input smoothing settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FInputSmoothingConfig SmoothingConfig;

	// Invert Y axis for pitch
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bInvertPitch = false;

	// Invert X axis for yaw
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bInvertYaw = false;

	// Mouse sensitivity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float MouseSensitivity = 1.0f;

	// ============================================================================
	// FLIGHT ASSISTANCE
	// ============================================================================

	// Current flight assist mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Flight Assist")
	EFlightAssistMode AssistMode = EFlightAssistMode::Stability;

	// Enable automatic collision avoidance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Assist")
	bool bCollisionAvoidance = true;

	// Enable automatic thrust limiting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Assist")
	bool bThrustLimiting = true;

	// Maximum safe velocity (m/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Assist")
	float MaxSafeVelocity = 1000.0f;

	// ============================================================================
	// INPUT STATE
	// ============================================================================

	// Raw input values (-1 to 1)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Input State")
	FVector RawThrustInput = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Input State")
	FVector RawRotationInput = FVector::ZeroVector;

	// Smoothed input values
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Input State")
	FVector SmoothedThrustInput = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Input State")
	FVector SmoothedRotationInput = FVector::ZeroVector;

	// ============================================================================
	// FLIGHT CONTROL
	// ============================================================================

	// Set thrust input
	UFUNCTION(BlueprintCallable, Category = "Flight Control")
	void SetThrustInput(FVector Input);

	// Set rotation input
	UFUNCTION(BlueprintCallable, Category = "Flight Control")
	void SetRotationInput(FVector Input);

	// Get smoothed thrust output
	UFUNCTION(BlueprintCallable, Category = "Flight Control")
	FVector GetSmoothedThrustOutput() const;

	// Get smoothed rotation output
	UFUNCTION(BlueprintCallable, Category = "Flight Control")
	FVector GetSmoothedRotationOutput() const;

	// ============================================================================
	// FLIGHT ASSISTANCE FUNCTIONS
	// ============================================================================

	// Apply flight assistance based on current mode
	UFUNCTION(BlueprintCallable, Category = "Flight Assist")
	void ApplyFlightAssistance(FVector& OutThrust, FVector& OutRotation, float DeltaTime);

	// Apply stability assistance
	UFUNCTION(BlueprintCallable, Category = "Flight Assist")
	void ApplyStabilityAssistance(FVector& OutThrust, FVector& OutRotation, float DeltaTime);

	// Apply auto-level assistance
	UFUNCTION(BlueprintCallable, Category = "Flight Assist")
	void ApplyAutoLevelAssistance(FVector& OutThrust, FVector& OutRotation, float DeltaTime);

	// Apply orbital assistance
	UFUNCTION(BlueprintCallable, Category = "Flight Assist")
	void ApplyOrbitalAssistance(FVector& OutThrust, FVector& OutRotation, float DeltaTime);

	// Apply docking assistance
	UFUNCTION(BlueprintCallable, Category = "Flight Assist")
	void ApplyDockingAssistance(FVector& OutThrust, FVector& OutRotation, float DeltaTime);

	// ============================================================================
	// COLLISION AVOIDANCE
	// ============================================================================

	// Check for potential collisions
	UFUNCTION(BlueprintCallable, Category = "Collision Avoidance")
	bool CheckForCollisions(const FVector& ProposedPosition, const FVector& CurrentVelocity);

	// Get collision avoidance vector
	UFUNCTION(BlueprintCallable, Category = "Collision Avoidance")
	FVector GetCollisionAvoidanceVector(const FVector& CurrentPosition, const FVector& CurrentVelocity);

	// ============================================================================
	// UTILITY FUNCTIONS
	// ============================================================================

	// Reset all inputs
	UFUNCTION(BlueprintCallable, Category = "Input")
	void ResetInputs();

	// Set flight assist mode
	UFUNCTION(BlueprintCallable, Category = "Flight Assist")
	void SetAssistMode(EFlightAssistMode NewMode);

	// Get current assist mode
	UFUNCTION(BlueprintCallable, Category = "Flight Assist")
	EFlightAssistMode GetAssistMode() const { return AssistMode; }

	// Is controller active?
	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsControllerActive() const { return bIsControllerActive; }

	// Get flight controller status
	UFUNCTION(BlueprintCallable, Category = "State")
	FString GetControllerStatus() const;

protected:
	// Input smoothing
	void SmoothInputs(float DeltaTime);
	void ApplyDeadzone(FVector& Input);
	void ApplyInversion(FVector& Input);

	// Internal state
	bool bIsControllerActive = false;
	float LastUpdateTime = 0.0f;

	// Smoothing state
	FVector PreviousThrustInput = FVector::ZeroVector;
	FVector PreviousRotationInput = FVector::ZeroVector;

	// Collision detection
	UPROPERTY()
	class UBoxComponent* CollisionDetectionBox;

	// Performance tracking
	int32 InputUpdateCount = 0;
	float AverageInputRate = 0.0f;

private:
	// Helper functions
	float SmoothValue(float Current, float Target, float SmoothingFactor, float DeltaTime);
	FVector SmoothVector(const FVector& Current, const FVector& Target, float SmoothingFactor, float DeltaTime);
	bool IsWithinDeadzone(const FVector& Input) const;
};