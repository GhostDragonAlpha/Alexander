// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrbitalBody.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
// #include "Components/VRPawn.h" // Removed - VRPawn not available in UE5.6
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Spaceship.generated.h"

// Forward declarations
class UCockpitComponent;
class UEngineComponent;
class UFlightController;
class UShipSystemsManager;
class AVRCharacter;

/**
 * Spaceship flight modes
 */
UENUM(BlueprintType)
enum class EFlightMode : uint8
{
	// Standard flight mode with full control
	Normal UMETA(DisplayName = "Normal"),
	
	// Assisted flight with stability
	Assisted UMETA(DisplayName = "Assisted"),
	
	// Newtonian physics only
	Newtonian UMETA(DisplayName = "Newtonian"),
	
	// Docking mode with precision control
	Docking UMETA(DisplayName = "Docking"),
	
	// Combat mode with enhanced targeting
	Combat UMETA(DisplayName = "Combat")
};

/**
 * Ship damage states
 */
UENUM(BlueprintType)
enum class EShipDamageState : uint8
{
	Pristine UMETA(DisplayName = "Pristine"),
	MinorDamage UMETA(DisplayName = "Minor Damage"),
	ModerateDamage UMETA(DisplayName = "Moderate Damage"),
	HeavyDamage UMETA(DisplayName = "Heavy Damage"),
	Critical UMETA(DisplayName = "Critical"),
	Destroyed UMETA(DisplayName = "Destroyed")
};

/**
 * Base Spaceship class - inherits from OrbitalBody for realistic space physics
 * Supports VR interaction and 6DOF movement
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API ASpaceship : public AOrbitalBody
{
	GENERATED_BODY()

public:
	ASpaceship();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);

public:
	// ============================================================================
	// CORE COMPONENTS
	// ============================================================================

	// Root component for ship
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* ShipRoot;

	// Main ship mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ShipMesh;

	// VR cockpit component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCockpitComponent* CockpitComponent;

	// Engine component for thrust
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UEngineComponent* EngineComponent;

	// Flight controller for input handling
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UFlightController* FlightController;

	// Ship systems manager
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UShipSystemsManager* SystemsManager;

	// ============================================================================
	// VR INTEGRATION
	// ============================================================================

	// VR camera component (will be attached to cockpit)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	UCameraComponent* VRCamera;

	// VR pawn reference (for hand tracking, etc.)
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	// AVRCharacter* VRPawn;

	// Is this a VR-enabled ship?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR")
	bool bIsVREnabled = true;

	// ============================================================================
	// FLIGHT PROPERTIES
	// ============================================================================

	// Current flight mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
	EFlightMode FlightMode = EFlightMode::Assisted;

	// Maximum thrust force (Newtons)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
	float MaxThrust = 100000.0f;

	// Maximum angular velocity (degrees/second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
	float MaxAngularVelocity = 90.0f;

	// Thrust sensitivity (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
	float ThrustSensitivity = 0.5f;

	// Rotation sensitivity (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
	float RotationSensitivity = 0.5f;

	// Current damage state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship")
	EShipDamageState DamageState = EShipDamageState::Pristine;

	// Hull integrity (0-100)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship")
	float HullIntegrity = 100.0f;

	// ============================================================================
	// INPUT HANDLING
	// ============================================================================

	// Thrust input (-1 to 1)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	FVector ThrustInput = FVector::ZeroVector;

	// Rotation input (-1 to 1)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	FVector RotationInput = FVector::ZeroVector;

	// ============================================================================
	// FLIGHT CONTROL
	// ============================================================================

	// Apply thrust in local space
	UFUNCTION(BlueprintCallable, Category = "Flight")
	void ApplyThrust(FVector LocalThrust);

	// Apply rotation
	UFUNCTION(BlueprintCallable, Category = "Flight")
	void ApplyRotation(FVector LocalRotation);

	// Set flight mode
	UFUNCTION(BlueprintCallable, Category = "Flight")
	void SetFlightMode(EFlightMode NewMode);

	// Toggle between normal and assisted flight
	UFUNCTION(BlueprintCallable, Category = "Flight")
	void ToggleAssistedFlight();

	// Emergency stop (kill all velocity)
	UFUNCTION(BlueprintCallable, Category = "Flight")
	void EmergencyStop();

	// ============================================================================
	// SHIP SYSTEMS
	// ============================================================================

	// Take damage
	UFUNCTION(BlueprintCallable, Category = "Ship")
	void TakeDamage(float DamageAmount, FVector HitLocation, FVector HitNormal);

	// Repair ship systems
	UFUNCTION(BlueprintCallable, Category = "Ship")
	void RepairSystems(float RepairAmount);

	// Check if ship is operational
	UFUNCTION(BlueprintCallable, Category = "Ship")
	bool IsOperational() const;

	// Get ship status as text
	UFUNCTION(BlueprintCallable, Category = "Ship")
	FString GetShipStatus() const;

	// ============================================================================
	// VR INTERACTION
	// ============================================================================

	// Initialize VR systems
	UFUNCTION(BlueprintCallable, Category = "VR")
	void InitializeVR();

	// Get cockpit transform for VR positioning
	UFUNCTION(BlueprintCallable, Category = "VR")
	FTransform GetCockpitTransform() const;

	// Handle VR controller interaction
	UFUNCTION(BlueprintCallable, Category = "VR")
	void HandleVRControllerInteraction(class UMotionControllerComponent* Controller, bool bIsRightHand);

	// ============================================================================
	// DEBUG & UTILITIES
	// ============================================================================

	// Show flight debug info
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowFlightDebug = false;

	// Show ship status in HUD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowShipStatus = true;

protected:
	// Update flight physics
	void UpdateFlightPhysics(float DeltaTime);

	// Update ship systems
	void UpdateShipSystems(float DeltaTime);

	// Handle input processing
	void ProcessFlightInput(float DeltaTime);

	// Apply flight mode specific behavior
	void ApplyFlightModeBehavior(float DeltaTime);

	// Update damage state based on hull integrity
	void UpdateDamageState();

	// Draw debug information
	void DrawDebugInfo();

private:
	// Internal state
	bool bIsThrusting = false;
	bool bIsRotating = false;
	float CurrentThrustLevel = 0.0f;
	FVector CurrentAngularVelocity = FVector::ZeroVector;

	// Performance tracking
	float LastPhysicsUpdateTime = 0.0f;
	int32 PhysicsUpdateCount = 0;
};