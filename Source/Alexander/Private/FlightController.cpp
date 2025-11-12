// Copyright (c) 2025 Alexander Project. All rights reserved.

#include "FlightController.h"
#include "Core/EventBus.h"
#include "Core/SystemRegistry.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"

void UFlightController::InitializeModule()
{
	// Call base class initialization
	Super::InitializeModule();
	
	// Reset input state
	CurrentThrustInput = FVector::ZeroVector;
	CurrentRotationInput = FVector::ZeroVector;
	bIsThrusting = false;
	
	LogSystemMessage(TEXT("FlightController: Physics and input initialized"));
}

void UFlightController::ShutdownModule()
{
	// Clear ship reference
	ControlledShip = nullptr;
	ShipPhysicsComponent = nullptr;
	
	LogSystemMessage(TEXT("FlightController: Flight systems shut down"));
	
	// Call base class shutdown
	Super::ShutdownModule();
}

void UFlightController::UpdateModule(float DeltaTime)
{
	// LAW #3: Check health before doing work
	if (!IsSystemHealthy())
	{
		return;
	}
	
	// Only process if we have a ship to control
	if (!ControlledShip || !ShipPhysicsComponent)
	{
		return;
	}
	
	// Apply physics based on input state
	if (bIsThrusting)
	{
		ApplyThrust(DeltaTime);
	}
	
	ApplyRotation(DeltaTime);
	ApplyDamping(DeltaTime);
	
	// Publish ship movement event
	FSystemEvent MovedEvent;
	MovedEvent.EventType = TEXT("ShipMoved");
	MovedEvent.SourceSystem = GetSystemName();
	MovedEvent.EventData = FString::Printf(TEXT("{\"position\":\"%s\"}"), 
		*ControlledShip->GetActorLocation().ToString());
	PublishEvent(MovedEvent);
}

FString UFlightController::GetSystemName() const
{
	return TEXT("FlightController");
}

bool UFlightController::IsSystemHealthy() const
{
	// Check base class health
	if (!Super::IsSystemHealthy())
	{
		return false;
	}
	
	// Additional flight-specific health checks
	if (ControlledShip && !ControlledShip->IsValidLowLevel())
	{
		return false;
	}
	
	if (ShipPhysicsComponent && !ShipPhysicsComponent->IsValidLowLevel())
	{
		return false;
	}
	
	return true;
}

void UFlightController::SetShipActor(AActor* Ship)
{
	ControlledShip = Ship;
	
	if (ControlledShip)
	{
		// Try to find a physics component
		ShipPhysicsComponent = Cast<UPrimitiveComponent>(
			ControlledShip->GetRootComponent());
		
		if (ShipPhysicsComponent)
		{
			// Enable physics
			ShipPhysicsComponent->SetSimulatePhysics(true);
			ShipPhysicsComponent->SetEnableGravity(false); // Space flight - no gravity
			
			LogSystemMessage(TEXT("FlightController: Ship actor set with physics enabled"));
		}
		else
		{
			LogSystemMessage(TEXT("FlightController: Warning - Ship actor has no physics component"), true);
		}
	}
	else
	{
		ShipPhysicsComponent = nullptr;
	}
}

void UFlightController::HandleInputMove(const FSystemEvent& Event)
{
	// Parse event data (expects JSON with "direction" field)
	if (Event.EventData.Contains(TEXT("direction")))
	{
		// Simple parsing - in real implementation would use proper JSON parser
		CurrentThrustInput = FVector(1.0f, 0.0f, 0.0f); // Default forward
		bIsThrusting = true;
	}
}

void UFlightController::HandleInputLook(const FSystemEvent& Event)
{
	// Parse rotation input
	if (Event.EventData.Contains(TEXT("rotation")))
	{
		CurrentRotationInput = FVector(0.0f, 0.0f, 1.0f); // Default yaw
	}
}

void UFlightController::HandleInputThrust(const FSystemEvent& Event)
{
	// Parse thrust power
	bIsThrusting = Event.EventData.Contains(TEXT("thrust"));
}

void UFlightController::ApplyThrust(float DeltaTime)
{
	if (!ShipPhysicsComponent)
	{
		return;
	}
	
	// Calculate thrust force
	FVector ThrustForce = ControlledShip->GetActorForwardVector() * ThrustPower * 100.0f;
	
	// Apply force to physics component
	ShipPhysicsComponent->AddForce(ThrustForce);
	
	// Limit velocity
	FVector Velocity = ShipPhysicsComponent->GetPhysicsLinearVelocity();
	if (Velocity.Size() > MaxVelocity)
	{
		ShipPhysicsComponent->SetPhysicsLinearVelocity(
			Velocity.GetSafeNormal() * MaxVelocity);
	}
}

void UFlightController::ApplyRotation(float DeltaTime)
{
	if (!ShipPhysicsComponent)
	{
		return;
	}
	
	// Apply rotation based on input
	float YawRotation = CurrentRotationInput.Z * RotationSpeed * DeltaTime;
	
	if (FMath::Abs(YawRotation) > 0.01f)
	{
		FRotator CurrentRotation = ControlledShip->GetActorRotation();
		CurrentRotation.Yaw += YawRotation;
		ControlledShip->SetActorRotation(CurrentRotation);
	}
}

void UFlightController::ApplyDamping(float DeltaTime)
{
	if (!ShipPhysicsComponent)
	{
		return;
	}
	
	// Apply damping to simulate space friction (for better control)
	if (!bIsThrusting)
	{
		FVector Velocity = ShipPhysicsComponent->GetPhysicsLinearVelocity();
		FVector DampedVelocity = Velocity * DampingFactor;
		ShipPhysicsComponent->SetPhysicsLinearVelocity(DampedVelocity);
	}
}