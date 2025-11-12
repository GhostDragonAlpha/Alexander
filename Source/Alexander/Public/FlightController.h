// Copyright (c) 2025 Alexander Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/SystemModuleBase.h"
#include "FlightController.generated.h"

/**
 * FlightController - 6-DOF spaceship flight system
 * 
 * This system handles all physics-based flight mechanics for the spaceship.
 * It processes input, applies forces, and manages the ship's movement through space.
 * 
 * Implements the 5-method LEGO pattern from SystemModuleBase:
 * - InitializeModule(): Sets up physics and input bindings
 * - ShutdownModule(): Cleans up resources
 * - UpdateModule(): Processes input and applies forces each frame
 * - GetSystemName(): Returns "FlightController"
 * - IsSystemHealthy(): Validates physics state and input systems
 * 
 * Communication via EventBus:
 * - Subscribes to: "InputMove", "InputLook", "InputThrust"
 * - Publishes: "ThrustApplied", "ShipMoved", "ShipRotated"
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API UFlightController : public USystemModuleBase
{
	GENERATED_BODY()
	
public:
	// Implement the 5 LEGO methods
	virtual void InitializeModule() override;
	virtual void ShutdownModule() override;
	virtual void UpdateModule(float DeltaTime) override;
	virtual FString GetSystemName() const override;
	virtual bool IsSystemHealthy() const override;
	
	// Flight-specific methods
	UFUNCTION(BlueprintCallable, Category = "Flight")
	void SetShipActor(AActor* Ship);
	
	UFUNCTION(BlueprintCallable, Category = "Flight")
	AActor* GetShipActor() const { return ControlledShip; }
	
	UFUNCTION(BlueprintCallable, Category = "Flight")
	void SetThrustPower(float Power) { ThrustPower = FMath::Clamp(Power, 0.0f, 1000.0f); }
	
	UFUNCTION(BlueprintCallable, Category = "Flight")
	float GetThrustPower() const { return ThrustPower; }
	
protected:
	// Input handling
	void HandleInputMove(const FSystemEvent& Event);
	void HandleInputLook(const FSystemEvent& Event);
	void HandleInputThrust(const FSystemEvent& Event);
	
	// Physics application
	void ApplyThrust(float DeltaTime);
	void ApplyRotation(float DeltaTime);
	void ApplyDamping(float DeltaTime);
	
	// Ship reference
	UPROPERTY()
	AActor* ControlledShip = nullptr;
	
	UPROPERTY()
	UPrimitiveComponent* ShipPhysicsComponent = nullptr;
	
	// Flight parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Settings")
	float ThrustPower = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Settings")
	float RotationSpeed = 45.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Settings")
	float DampingFactor = 0.95f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Settings")
	float MaxVelocity = 5000.0f;
	
	// Input state
	FVector CurrentThrustInput;
	FVector CurrentRotationInput;
	bool bIsThrusting = false;
};