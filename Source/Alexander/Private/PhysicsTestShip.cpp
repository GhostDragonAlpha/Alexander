// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysicsTestShip.h"
#include "FlightController.h"
#include "OrbitalMechanics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"

APhysicsTestShip::APhysicsTestShip()
{
	// Parent constructor (AOrbitalBody) already creates:
	// - RootSceneComponent
	// - MeshComponent (physics-enabled)
	// - OrbitalMechanics

	PrimaryActorTick.bCanEverTick = true;

	// Configure as spacecraft (not a planet)
	Mass = 50000.0f; // 50 tons (typical small spacecraft)
	Radius = 5.0f; // 5 meter radius
	OrbitMode = EOrbitMode::Thrust; // Using thrust, not orbital motion

	// Enable physics on mesh
	bEnablePhysics = true;
	bEnableGravity = true; // Custom gravity via OrbitalMechanics
	bEnableCollision = true;

	// Configure mesh component for space flight
	if (MeshComponent)
	{
		MeshComponent->SetSimulatePhysics(false); // Will enable in BeginPlay
		MeshComponent->SetEnableGravity(false); // Custom gravity
		MeshComponent->SetLinearDamping(0.1f); // Low damping for space
		MeshComponent->SetAngularDamping(0.5f); // Moderate angular damping
		MeshComponent->SetMassOverrideInKg(NAME_None, Mass);
		MeshComponent->SetCollisionProfileName(TEXT("Pawn"));
	}

	// Create FlightController component (optional)
	bEnableFlightController = true;
	FlightController = CreateDefaultSubobject<UFlightController>(TEXT("FlightController"));
	if (FlightController)
	{
		FlightController->AssistMode = EFlightAssistMode::Stability;
	}

	// Configure force/torque magnitudes
	ThrustForceMagnitude = 100000.0f; // 100 kN per unit input
	TorqueMagnitude = 50000.0f; // 50 kN·m per unit input
}

void APhysicsTestShip::BeginPlay()
{
	Super::BeginPlay();

	ConfigurePhysics();

	UE_LOG(LogTemp, Log, TEXT("PhysicsTestShip: Spawned at %s"), *GetActorLocation().ToString());
	UE_LOG(LogTemp, Log, TEXT("PhysicsTestShip: Mass=%f kg, FlightController=%s"),
		Mass, FlightController ? TEXT("YES") : TEXT("NO"));
}

void APhysicsTestShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Parent AOrbitalBody::Tick handles:
	// - Custom gravity from OrbitalMechanics
	// - Orbital physics
	// - Scale factor based on distance
}

void APhysicsTestShip::ConfigurePhysics()
{
	// Enable physics on mesh component
	if (MeshComponent)
	{
		MeshComponent->SetSimulatePhysics(true);
		MeshComponent->SetEnableGravity(false); // Custom gravity via OrbitalMechanics
		MeshComponent->SetMassOverrideInKg(NAME_None, Mass);

		UE_LOG(LogTemp, Log, TEXT("PhysicsTestShip: Physics enabled on MeshComponent"));
		UE_LOG(LogTemp, Log, TEXT("PhysicsTestShip: IsSimulatingPhysics=%s"),
			MeshComponent->IsSimulatingPhysics() ? TEXT("YES") : TEXT("NO"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PhysicsTestShip: No MeshComponent found!"));
	}
}

void APhysicsTestShip::ApplyThrust(FVector ThrustInput)
{
	if (!MeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("PhysicsTestShip: No MeshComponent for thrust"));
		return;
	}

	// Check if FlightController is available
	if (FlightController)
	{
		// Use FlightController for advanced flight assists
		FlightController->SetThrustInput(ThrustInput);
		UE_LOG(LogTemp, Verbose, TEXT("PhysicsTestShip: Thrust via FlightController: %s"),
			*ThrustInput.ToString());
	}
	else if (MeshComponent->IsSimulatingPhysics())
	{
		// Direct physics force application (fallback)
		// Convert local space thrust to world space
		FVector WorldForce = GetActorRotation().RotateVector(ThrustInput) * ThrustForceMagnitude;
		MeshComponent->AddForce(WorldForce, NAME_None, false);

		UE_LOG(LogTemp, Verbose, TEXT("PhysicsTestShip: Thrust via physics: Input=%s, Force=%s"),
			*ThrustInput.ToString(), *WorldForce.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PhysicsTestShip: Physics not simulating, cannot apply thrust"));
	}
}

void APhysicsTestShip::ApplyTorque(FVector TorqueInput)
{
	if (!MeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("PhysicsTestShip: No MeshComponent for torque"));
		return;
	}

	// Check if FlightController is available
	if (FlightController)
	{
		// Use FlightController for advanced flight assists
		FlightController->SetRotationInput(TorqueInput);
		UE_LOG(LogTemp, Verbose, TEXT("PhysicsTestShip: Torque via FlightController: %s"),
			*TorqueInput.ToString());
	}
	else if (MeshComponent->IsSimulatingPhysics())
	{
		// Direct physics torque application (fallback)
		// Convert local space torque to world space
		FVector WorldTorque = GetActorRotation().RotateVector(TorqueInput) * TorqueMagnitude;
		MeshComponent->AddTorqueInRadians(WorldTorque, NAME_None, false);

		UE_LOG(LogTemp, Verbose, TEXT("PhysicsTestShip: Torque via physics: Input=%s, Torque=%s"),
			*TorqueInput.ToString(), *WorldTorque.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PhysicsTestShip: Physics not simulating, cannot apply torque"));
	}
}

FVector APhysicsTestShip::GetCurrentVelocity() const
{
	// Return velocity from OrbitalBody parent class
	return Velocity;
}

float APhysicsTestShip::GetCurrentSpeed() const
{
	return Velocity.Size();
}
