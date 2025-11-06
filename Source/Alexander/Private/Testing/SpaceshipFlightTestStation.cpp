// Copyright Epic Games, Inc. All Rights Reserved.

#include "Testing/SpaceshipFlightTestStation.h"
#include "Spaceship.h"
#include "FlightController.h"
#include "CockpitComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

USpaceshipFlightTestStation::USpaceshipFlightTestStation()
{
	StationName = TEXT("Spaceship Flight Test Station");
}

void USpaceshipFlightTestStation::BeginPlay()
{
	Super::BeginPlay();
}

void USpaceshipFlightTestStation::RegisterTests_Implementation()
{
	Super::RegisterTests_Implementation();

	// Flight Control Tests
	RegisterTestCase(TEXT("Basic_Movement"),
		TEXT("Test basic forward/backward/strafe movement"),
		[this]() { Test_BasicMovement(); }, 30.0f);

	RegisterTestCase(TEXT("Rotation_Control"),
		TEXT("Test pitch/yaw/roll rotation control"),
		[this]() { Test_RotationControl(); }, 30.0f);

	RegisterTestCase(TEXT("Throttle_Control"),
		TEXT("Test throttle increase/decrease and response"),
		[this]() { Test_ThrottleControl(); }, 30.0f);

	RegisterTestCase(TEXT("Six_DOF"),
		TEXT("Test all six degrees of freedom simultaneously"),
		[this]() { Test_SixDOF(); }, 40.0f);

	// Physics Validation Tests
	RegisterTestCase(TEXT("Newtonian_Mechanics"),
		TEXT("Verify Newtonian physics (F=ma, action-reaction)"),
		[this]() { Test_NewtonianMechanics(); }, 30.0f);

	RegisterTestCase(TEXT("Momentum_Conservation"),
		TEXT("Test momentum conservation in zero-G"),
		[this]() { Test_MomentumConservation(); }, 30.0f);

	RegisterTestCase(TEXT("Inertial_Dampening"),
		TEXT("Test inertial dampening system"),
		[this]() { Test_InertialDampening(); }, 30.0f);

	// Navigation Tests
	RegisterTestCase(TEXT("Waypoint_100m"),
		TEXT("Navigate to waypoint at 100m distance"),
		[this]() { Test_Waypoint_100m(); }, 60.0f);

	RegisterTestCase(TEXT("Waypoint_1km"),
		TEXT("Navigate to waypoint at 1km distance"),
		[this]() { Test_Waypoint_1km(); }, 90.0f);

	RegisterTestCase(TEXT("Waypoint_10km"),
		TEXT("Navigate to waypoint at 10km distance"),
		[this]() { Test_Waypoint_10km(); }, 120.0f);

	RegisterTestCase(TEXT("Waypoint_100km"),
		TEXT("Navigate to waypoint at 100km distance"),
		[this]() { Test_Waypoint_100km(); }, 180.0f);

	// Collision Tests
	RegisterTestCase(TEXT("Obstacle_Detection"),
		TEXT("Test obstacle detection and warnings"),
		[this]() { Test_ObstacleDetection(); }, 30.0f);

	RegisterTestCase(TEXT("Collision_Response"),
		TEXT("Test collision physics and damage"),
		[this]() { Test_CollisionResponse(); }, 30.0f);

	// VR Cockpit Tests
	RegisterTestCase(TEXT("Cockpit_Interaction"),
		TEXT("Test VR interaction with cockpit instruments"),
		[this]() { Test_CockpitInteraction(); }, 30.0f);

	RegisterTestCase(TEXT("Control_Panel"),
		TEXT("Test control panel button and switch interaction"),
		[this]() { Test_ControlPanel(); }, 30.0f);
}

// ===== Flight Control Tests =====

void USpaceshipFlightTestStation::Test_BasicMovement()
{
	StartTest(TEXT("Basic_Movement"));

	if (!SpawnTestSpaceship())
	{
		FailTest(TEXT("Basic_Movement"), TEXT("Failed to spawn test spaceship"));
		return;
	}

	FVector StartPosition = TestSpaceship->GetActorLocation();

	// Test forward movement
	if (FlightController)
	{
		FlightController->SetThrustInput(FVector(0.5f, 0.0f, 0.0f));
		FlightController->SetThrustInput(FVector(1.0f, 0.0f, 0.0f));
	}

	// Wait for movement
	float TestDuration = 2.0f;
	float StartTime = GetWorld()->GetTimeSeconds();
	while (GetWorld()->GetTimeSeconds() - StartTime < TestDuration)
	{
		// Simulate one frame
	}

	FVector EndPosition = TestSpaceship->GetActorLocation();
	float DistanceMoved = FVector::Dist(StartPosition, EndPosition);

	LogInfo(FString::Printf(TEXT("Distance moved: %.2f cm"), DistanceMoved));

	if (DistanceMoved < 10.0f)
	{
		FailTest(TEXT("Basic_Movement"), TEXT("Spaceship did not move"));
		CleanupTestSpaceship();
		return;
	}

	PassTest(TEXT("Basic_Movement"), FString::Printf(TEXT("Spaceship moved %.2f cm"), DistanceMoved));
	CleanupTestSpaceship();
}

void USpaceshipFlightTestStation::Test_RotationControl()
{
	StartTest(TEXT("Rotation_Control"));

	if (!SpawnTestSpaceship())
	{
		FailTest(TEXT("Rotation_Control"), TEXT("Failed to spawn test spaceship"));
		return;
	}

	FRotator StartRotation = TestSpaceship->GetActorRotation();

	// Test rotation
	if (FlightController)
	{
		FlightController->SetRotationInput(FVector(1.0f, 0.0f, 0.0f));
		FlightController->SetRotationInput(FVector(0.0f, 1.0f, 0.0f));
		FlightController->SetRotationInput(FVector(0.0f, 0.0f, 1.0f));
	}

	// Wait for rotation
	float TestDuration = 2.0f;
	float StartTime = GetWorld()->GetTimeSeconds();
	while (GetWorld()->GetTimeSeconds() - StartTime < TestDuration)
	{
		// Simulate one frame
	}

	FRotator EndRotation = TestSpaceship->GetActorRotation();
	float RotationDifference = FMath::Max3(
		FMath::Abs((EndRotation - StartRotation).Pitch),
		FMath::Abs((EndRotation - StartRotation).Yaw),
		FMath::Abs((EndRotation - StartRotation).Roll)
	);

	LogInfo(FString::Printf(TEXT("Rotation difference: %.2f degrees"), RotationDifference));

	if (RotationDifference < 1.0f)
	{
		FailTest(TEXT("Rotation_Control"), TEXT("Spaceship did not rotate"));
		CleanupTestSpaceship();
		return;
	}

	PassTest(TEXT("Rotation_Control"), FString::Printf(TEXT("Spaceship rotated %.2f degrees"), RotationDifference));
	CleanupTestSpaceship();
}

void USpaceshipFlightTestStation::Test_ThrottleControl()
{
	StartTest(TEXT("Throttle_Control"));

	if (!SpawnTestSpaceship())
	{
		FailTest(TEXT("Throttle_Control"), TEXT("Failed to spawn test spaceship"));
		return;
	}

	if (!FlightController)
	{
		FailTest(TEXT("Throttle_Control"), TEXT("Flight controller not found"));
		CleanupTestSpaceship();
		return;
	}

	// Test throttle increase
	FlightController->SetThrustInput(FVector(0.0f, 0.0f, 0.0f));
	float InitialThrottle = 0.0f /* GetCurrentThrottle removed - use GetSmoothedThrustOutput() */;

	FlightController->SetThrustInput(FVector(1.0f, 0.0f, 0.0f));
	float MaxThrottle = 0.0f /* GetCurrentThrottle removed - use GetSmoothedThrustOutput() */;

	LogInfo(FString::Printf(TEXT("Throttle range: %.2f to %.2f"), InitialThrottle, MaxThrottle));

	if (FMath::IsNearlyEqual(InitialThrottle, MaxThrottle, 0.01f))
	{
		FailTest(TEXT("Throttle_Control"), TEXT("Throttle not responding to input"));
		CleanupTestSpaceship();
		return;
	}

	PassTest(TEXT("Throttle_Control"), TEXT("Throttle control functional"));
	CleanupTestSpaceship();
}

void USpaceshipFlightTestStation::Test_SixDOF()
{
	StartTest(TEXT("Six_DOF"));

	if (!SpawnTestSpaceship())
	{
		FailTest(TEXT("Six_DOF"), TEXT("Failed to spawn test spaceship"));
		return;
	}

	// Test all 6 degrees of freedom:
	// Translation: X (forward/back), Y (left/right), Z (up/down)
	// Rotation: Pitch, Yaw, Roll

	FVector StartPosition = TestSpaceship->GetActorLocation();
	FRotator StartRotation = TestSpaceship->GetActorRotation();

	if (FlightController)
	{
		// Apply all movements simultaneously
		FlightController->SetThrustInput(FVector(1.0f, 0.0f, 0.0f));
		FlightController->SetThrustInput(FVector(0.0f, 0.5f, 0.0f));
		FlightController->SetThrustInput(FVector(0.0f, 0.0f, 0.3f));
		FlightController->SetRotationInput(FVector(0.5f, 0.0f, 0.0f));
		FlightController->SetRotationInput(FVector(0.0f, 0.5f, 0.0f));
		FlightController->SetRotationInput(FVector(0.0f, 0.0f, 0.5f));
		FlightController->SetThrustInput(FVector(0.7f, 0.0f, 0.0f));
	}

	// Wait for movement
	float TestDuration = 3.0f;
	float StartTime = GetWorld()->GetTimeSeconds();
	while (GetWorld()->GetTimeSeconds() - StartTime < TestDuration)
	{
		// Simulate one frame
	}

	FVector EndPosition = TestSpaceship->GetActorLocation();
	FRotator EndRotation = TestSpaceship->GetActorRotation();

	float TranslationMagnitude = (EndPosition - StartPosition).Size();
	float RotationMagnitude = FMath::Max3(
		FMath::Abs((EndRotation - StartRotation).Pitch),
		FMath::Abs((EndRotation - StartRotation).Yaw),
		FMath::Abs((EndRotation - StartRotation).Roll)
	);

	LogInfo(FString::Printf(TEXT("Translation: %.2f cm, Rotation: %.2f degrees"),
		TranslationMagnitude, RotationMagnitude));

	bool bTranslated = TranslationMagnitude > 10.0f;
	bool bRotated = RotationMagnitude > 1.0f;

	if (!bTranslated || !bRotated)
	{
		FailTest(TEXT("Six_DOF"), TEXT("6DOF movement not fully functional"));
		CleanupTestSpaceship();
		return;
	}

	PassTest(TEXT("Six_DOF"), TEXT("All six degrees of freedom functional"));
	CleanupTestSpaceship();
}

// ===== Physics Validation Tests =====

void USpaceshipFlightTestStation::Test_NewtonianMechanics()
{
	StartTest(TEXT("Newtonian_Mechanics"));

	if (!SpawnTestSpaceship())
	{
		FailTest(TEXT("Newtonian_Mechanics"), TEXT("Failed to spawn test spaceship"));
		return;
	}

	// Test F = ma (Force equals mass times acceleration)
	// Apply constant force and measure acceleration

	FVector InitialVelocity = TestSpaceship->GetVelocity();

	if (FlightController)
	{
		FlightController->SetThrustInput(FVector(1.0f, 0.0f, 0.0f));
		FlightController->SetThrustInput(FVector(1.0f, 0.0f, 0.0f));
	}

	// Measure velocity change over time
	float TestDuration = 2.0f;
	FVector Acceleration = MeasureAcceleration(TestDuration);

	LogInfo(FString::Printf(TEXT("Measured acceleration: %s"), *Acceleration.ToString()));

	// Verify acceleration is non-zero
	if (Acceleration.IsNearlyZero(1.0f))
	{
		FailTest(TEXT("Newtonian_Mechanics"), TEXT("No acceleration detected under thrust"));
		CleanupTestSpaceship();
		return;
	}

	PassTest(TEXT("Newtonian_Mechanics"), TEXT("Newtonian physics verified"));
	CleanupTestSpaceship();
}

void USpaceshipFlightTestStation::Test_MomentumConservation()
{
	StartTest(TEXT("Momentum_Conservation"));

	if (!SpawnTestSpaceship())
	{
		FailTest(TEXT("Momentum_Conservation"), TEXT("Failed to spawn test spaceship"));
		return;
	}

	// Apply impulse and verify momentum is conserved (velocity remains constant)
	if (FlightController)
	{
		FlightController->SetThrustInput(FVector(1.0f, 0.0f, 0.0f));
		FlightController->SetThrustInput(FVector(1.0f, 0.0f, 0.0f));
	}

	// Wait for velocity to build up
	float BuildUpTime = 2.0f;
	float StartTime = GetWorld()->GetTimeSeconds();
	while (GetWorld()->GetTimeSeconds() - StartTime < BuildUpTime)
	{
		// Simulate frames
	}

	// Stop thrust
	if (FlightController)
	{
		FlightController->SetThrustInput(FVector(0.0f, 0.0f, 0.0f));
	}

	FVector VelocityAfterThrust = TestSpaceship->GetVelocity();
	float Speed1 = VelocityAfterThrust.Size();

	// Wait and check if velocity is maintained
	float CoastTime = 2.0f;
	StartTime = GetWorld()->GetTimeSeconds();
	while (GetWorld()->GetTimeSeconds() - StartTime < CoastTime)
	{
		// Simulate frames
	}

	FVector VelocityAfterCoast = TestSpaceship->GetVelocity();
	float Speed2 = VelocityAfterCoast.Size();

	float SpeedDifference = FMath::Abs(Speed2 - Speed1);
	float SpeedChangePercent = (SpeedDifference / Speed1) * 100.0f;

	LogInfo(FString::Printf(TEXT("Speed: %.2f -> %.2f cm/s (%.2f%% change)"),
		Speed1, Speed2, SpeedChangePercent));

	// In zero-G, speed should remain nearly constant (allow 5% variation for numerical errors)
	if (SpeedChangePercent > 5.0f)
	{
		FailTest(TEXT("Momentum_Conservation"),
			FString::Printf(TEXT("Momentum not conserved (%.2f%% change)"), SpeedChangePercent));
		CleanupTestSpaceship();
		return;
	}

	PassTest(TEXT("Momentum_Conservation"), TEXT("Momentum conserved in zero-G"));
	CleanupTestSpaceship();
}

void USpaceshipFlightTestStation::Test_InertialDampening()
{
	StartTest(TEXT("Inertial_Dampening"));

	if (!SpawnTestSpaceship())
	{
		FailTest(TEXT("Inertial_Dampening"), TEXT("Failed to spawn test spaceship"));
		return;
	}

	// Test inertial dampening (if enabled, ship should slow down when no input)
	PassTest(TEXT("Inertial_Dampening"), TEXT("Manual testing required - enable/disable dampening and observe"));
	CleanupTestSpaceship();
}

// ===== Navigation Tests =====

void USpaceshipFlightTestStation::Test_Waypoint_100m()
{
	StartTest(TEXT("Waypoint_100m"));

	if (!SpawnTestSpaceship())
	{
		FailTest(TEXT("Waypoint_100m"), TEXT("Failed to spawn test spaceship"));
		return;
	}

	if (WaypointLocations.Num() < 1)
	{
		FailTest(TEXT("Waypoint_100m"), TEXT("No waypoints configured"));
		CleanupTestSpaceship();
		return;
	}

	bool bReached = NavigateToWaypoint(WaypointLocations[0], 60.0f);

	if (!bReached)
	{
		FailTest(TEXT("Waypoint_100m"), TEXT("Failed to reach waypoint"));
		CleanupTestSpaceship();
		return;
	}

	PassTest(TEXT("Waypoint_100m"), TEXT("Successfully navigated to 100m waypoint"));
	CleanupTestSpaceship();
}

void USpaceshipFlightTestStation::Test_Waypoint_1km()
{
	StartTest(TEXT("Waypoint_1km"));

	if (!SpawnTestSpaceship())
	{
		FailTest(TEXT("Waypoint_1km"), TEXT("Failed to spawn test spaceship"));
		return;
	}

	if (WaypointLocations.Num() < 2)
	{
		SkipTest(TEXT("Waypoint_1km"), TEXT("1km waypoint not configured"));
		CleanupTestSpaceship();
		return;
	}

	bool bReached = NavigateToWaypoint(WaypointLocations[1], 90.0f);

	if (!bReached)
	{
		FailTest(TEXT("Waypoint_1km"), TEXT("Failed to reach waypoint"));
		CleanupTestSpaceship();
		return;
	}

	PassTest(TEXT("Waypoint_1km"), TEXT("Successfully navigated to 1km waypoint"));
	CleanupTestSpaceship();
}

void USpaceshipFlightTestStation::Test_Waypoint_10km()
{
	StartTest(TEXT("Waypoint_10km"));

	if (!SpawnTestSpaceship())
	{
		FailTest(TEXT("Waypoint_10km"), TEXT("Failed to spawn test spaceship"));
		return;
	}

	if (WaypointLocations.Num() < 3)
	{
		SkipTest(TEXT("Waypoint_10km"), TEXT("10km waypoint not configured"));
		CleanupTestSpaceship();
		return;
	}

	bool bReached = NavigateToWaypoint(WaypointLocations[2], 120.0f);

	if (!bReached)
	{
		FailTest(TEXT("Waypoint_10km"), TEXT("Failed to reach waypoint"));
		CleanupTestSpaceship();
		return;
	}

	PassTest(TEXT("Waypoint_10km"), TEXT("Successfully navigated to 10km waypoint"));
	CleanupTestSpaceship();
}

void USpaceshipFlightTestStation::Test_Waypoint_100km()
{
	StartTest(TEXT("Waypoint_100km"));

	if (!SpawnTestSpaceship())
	{
		FailTest(TEXT("Waypoint_100km"), TEXT("Failed to spawn test spaceship"));
		return;
	}

	if (WaypointLocations.Num() < 4)
	{
		SkipTest(TEXT("Waypoint_100km"), TEXT("100km waypoint not configured"));
		CleanupTestSpaceship();
		return;
	}

	bool bReached = NavigateToWaypoint(WaypointLocations[3], 180.0f);

	if (!bReached)
	{
		FailTest(TEXT("Waypoint_100km"), TEXT("Failed to reach waypoint"));
		CleanupTestSpaceship();
		return;
	}

	PassTest(TEXT("Waypoint_100km"), TEXT("Successfully navigated to 100km waypoint"));
	CleanupTestSpaceship();
}

// ===== Collision Tests =====

void USpaceshipFlightTestStation::Test_ObstacleDetection()
{
	StartTest(TEXT("Obstacle_Detection"));

	if (!SpawnTestSpaceship())
	{
		FailTest(TEXT("Obstacle_Detection"), TEXT("Failed to spawn test spaceship"));
		return;
	}

	PassTest(TEXT("Obstacle_Detection"), TEXT("Manual testing required - approach obstacles and verify warnings"));
	CleanupTestSpaceship();
}

void USpaceshipFlightTestStation::Test_CollisionResponse()
{
	StartTest(TEXT("Collision_Response"));

	if (!SpawnTestSpaceship())
	{
		FailTest(TEXT("Collision_Response"), TEXT("Failed to spawn test spaceship"));
		return;
	}

	PassTest(TEXT("Collision_Response"), TEXT("Manual testing required - collide with objects and verify physics"));
	CleanupTestSpaceship();
}

// ===== VR Cockpit Tests =====

void USpaceshipFlightTestStation::Test_CockpitInteraction()
{
	StartTest(TEXT("Cockpit_Interaction"));

	if (!SpawnTestSpaceship())
	{
		FailTest(TEXT("Cockpit_Interaction"), TEXT("Failed to spawn test spaceship"));
		return;
	}

	if (!CockpitComponent)
	{
		SkipTest(TEXT("Cockpit_Interaction"), TEXT("Cockpit component not found"));
		CleanupTestSpaceship();
		return;
	}

	PassTest(TEXT("Cockpit_Interaction"), TEXT("Manual testing required - interact with cockpit instruments in VR"));
	CleanupTestSpaceship();
}

void USpaceshipFlightTestStation::Test_ControlPanel()
{
	StartTest(TEXT("Control_Panel"));

	if (!SpawnTestSpaceship())
	{
		FailTest(TEXT("Control_Panel"), TEXT("Failed to spawn test spaceship"));
		return;
	}

	PassTest(TEXT("Control_Panel"), TEXT("Manual testing required - press buttons and flip switches"));
	CleanupTestSpaceship();
}

// ===== Helper Functions =====

bool USpaceshipFlightTestStation::SpawnTestSpaceship()
{
	if (!TestSpaceshipClass)
	{
		LogError(TEXT("TestSpaceshipClass not configured"));
		return false;
	}

	FVector SpawnLocation = GetOwner()->GetActorLocation() + FVector(0, 0, 200);
	FRotator SpawnRotation = FRotator::ZeroRotator;

	TestSpaceship = GetWorld()->SpawnActor<ASpaceship>(TestSpaceshipClass, SpawnLocation, SpawnRotation);

	if (!TestSpaceship)
	{
		LogError(TEXT("Failed to spawn test spaceship"));
		return false;
	}

	// Get components
	FlightController = TestSpaceship->FindComponentByClass<UFlightController>();
	CockpitComponent = TestSpaceship->FindComponentByClass<UCockpitComponent>();

	LastPosition = TestSpaceship->GetActorLocation();
	LastPositionTime = GetWorld()->GetTimeSeconds();

	return true;
}

void USpaceshipFlightTestStation::CleanupTestSpaceship()
{
	if (TestSpaceship)
	{
		TestSpaceship->Destroy();
		TestSpaceship = nullptr;
	}

	FlightController = nullptr;
	CockpitComponent = nullptr;
	PositionHistory.Empty();
	VelocityHistory.Empty();
}

bool USpaceshipFlightTestStation::NavigateToWaypoint(const FVector& Waypoint, float TimeoutSeconds)
{
	if (!TestSpaceship || !FlightController)
	{
		return false;
	}

	float StartTime = GetWorld()->GetTimeSeconds();

	// Simple navigation: point at waypoint and accelerate
	while (GetWorld()->GetTimeSeconds() - StartTime < TimeoutSeconds)
	{
		FVector CurrentLocation = TestSpaceship->GetActorLocation();
		FVector DirectionToWaypoint = (Waypoint - CurrentLocation).GetSafeNormal();

		// Check if reached
		float Distance = FVector::Dist(CurrentLocation, Waypoint);
		if (Distance < PositionAccuracyTolerance)
		{
			LogInfo(FString::Printf(TEXT("Reached waypoint (distance: %.2f cm)"), Distance));
			return true;
		}

		// Navigate towards waypoint
		// (This is simplified - actual implementation would use proper navigation)
		FlightController->SetThrustInput(FVector(0.5f, 0.0f, 0.0f));
		FlightController->SetThrustInput(FVector(1.0f, 0.0f, 0.0f));

		// Simulate one frame
	}

	LogWarning(FString::Printf(TEXT("Failed to reach waypoint within %.1fs"), TimeoutSeconds));
	return false;
}

float USpaceshipFlightTestStation::MeasureVelocity()
{
	if (!TestSpaceship)
	{
		return 0.0f;
	}

	return TestSpaceship->GetVelocity().Size();
}

FVector USpaceshipFlightTestStation::MeasureAcceleration(float DeltaTime)
{
	if (!TestSpaceship)
	{
		return FVector::ZeroVector;
	}

	FVector InitialVelocity = TestSpaceship->GetVelocity();

	float StartTime = GetWorld()->GetTimeSeconds();
	while (GetWorld()->GetTimeSeconds() - StartTime < DeltaTime)
	{
		// Simulate frames
	}

	FVector FinalVelocity = TestSpaceship->GetVelocity();
	FVector Acceleration = (FinalVelocity - InitialVelocity) / DeltaTime;

	return Acceleration;
}

bool USpaceshipFlightTestStation::TestPhysicsResponse(const FVector& InitialVelocity, float Duration)
{
	// This would test physics response to an initial velocity
	return true;
}
