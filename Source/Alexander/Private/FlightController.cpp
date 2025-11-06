// Copyright Epic Games, Inc. All Rights Reserved.

#include "FlightController.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "OrbitalBody.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "Net/UnrealNetwork.h"

UFlightController::UFlightController()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// Initialize input state
	RawThrustInput = FVector::ZeroVector;
	RawRotationInput = FVector::ZeroVector;
	SmoothedThrustInput = FVector::ZeroVector;
	SmoothedRotationInput = FVector::ZeroVector;
	
	// Initialize internal state
	bIsControllerActive = false;
	LastUpdateTime = 0.0f;
	PreviousThrustInput = FVector::ZeroVector;
	PreviousRotationInput = FVector::ZeroVector;
	
	// Default smoothing config
	SmoothingConfig = FInputSmoothingConfig();
	
	// Create collision detection box
	CollisionDetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionDetectionBox"));
	CollisionDetectionBox->SetBoxExtent(FVector(500.0f, 500.0f, 500.0f));
	
	// Performance tracking
	InputUpdateCount = 0;
	AverageInputRate = 0.0f;
}

void UFlightController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate input state
	DOREPLIFETIME(UFlightController, RawThrustInput);
	DOREPLIFETIME(UFlightController, RawRotationInput);
	DOREPLIFETIME(UFlightController, SmoothedThrustInput);
	DOREPLIFETIME(UFlightController, SmoothedRotationInput);

	// Replicate flight assist mode
	DOREPLIFETIME(UFlightController, AssistMode);
}

void UFlightController::BeginPlay()
{
	Super::BeginPlay();

	bIsControllerActive = true;
	UE_LOG(LogTemp, Log, TEXT("FlightController initialized with assist mode: %s"),
		*UEnum::GetValueAsString(AssistMode));
}

void UFlightController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!bIsControllerActive)
		return;
		
	// Update performance tracking
	InputUpdateCount++;
	LastUpdateTime = DeltaTime;
	
	// Smooth raw inputs
	SmoothInputs(DeltaTime);
	
	// Update performance metrics
	if (InputUpdateCount % 60 == 0) // Every 60 frames
	{
		AverageInputRate = 60.0f / (DeltaTime * 60.0f);
	}
}

void UFlightController::SetThrustInput(FVector Input)
{
	RawThrustInput = Input;
	
	// Apply deadzone
	ApplyDeadzone(RawThrustInput);
	
	// Apply inversion if needed
	ApplyInversion(RawThrustInput);
}

void UFlightController::SetRotationInput(FVector Input)
{
	RawRotationInput = Input;
	
	// Apply deadzone
	ApplyDeadzone(RawRotationInput);
	
	// Apply inversion if needed
	ApplyInversion(RawRotationInput);
}

FVector UFlightController::GetSmoothedThrustOutput() const
{
	return SmoothedThrustInput;
}

FVector UFlightController::GetSmoothedRotationOutput() const
{
	return SmoothedRotationInput;
}

void UFlightController::ApplyFlightAssistance(FVector& OutThrust, FVector& OutRotation, float DeltaTime)
{
	switch (AssistMode)
	{
	case EFlightAssistMode::Stability:
		ApplyStabilityAssistance(OutThrust, OutRotation, DeltaTime);
		break;
		
	case EFlightAssistMode::AutoLevel:
		ApplyAutoLevelAssistance(OutThrust, OutRotation, DeltaTime);
		break;
		
	case EFlightAssistMode::Orbital:
		ApplyOrbitalAssistance(OutThrust, OutRotation, DeltaTime);
		break;
		
	case EFlightAssistMode::Docking:
		ApplyDockingAssistance(OutThrust, OutRotation, DeltaTime);
		break;
		
	case EFlightAssistMode::None:
	default:
		// No assistance
		break;
	}
	
	// Apply collision avoidance if enabled
	if (bCollisionAvoidance)
	{
		FVector AvoidanceVector = GetCollisionAvoidanceVector(
			GetOwner()->GetActorLocation(), 
			GetOwner()->GetVelocity()
		);
		OutThrust += AvoidanceVector;
	}
	
	// Apply thrust limiting if enabled
	if (bThrustLimiting)
	{
		float CurrentSpeed = GetOwner()->GetVelocity().Size();
		if (CurrentSpeed > MaxSafeVelocity)
		{
			OutThrust *= 0.1f; // Reduce thrust significantly
		}
	}
}

void UFlightController::ApplyStabilityAssistance(FVector& OutThrust, FVector& OutRotation, float DeltaTime)
{
	// Apply gentle damping to prevent oscillations
	float DampingFactor = 0.95f;
	OutThrust *= DampingFactor;
	OutRotation *= DampingFactor;
	
	// Counteract unwanted rotation
	FRotator CurrentRotation = GetOwner()->GetActorRotation();
	FRotator TargetRotation = FRotator::ZeroRotator; // Level flight
	
	FRotator RotationError = TargetRotation - CurrentRotation;
	OutRotation += FVector(
		RotationError.Pitch * 0.1f,
		RotationError.Yaw * 0.1f,
		RotationError.Roll * 0.1f
	);
}

void UFlightController::ApplyAutoLevelAssistance(FVector& OutThrust, FVector& OutRotation, float DeltaTime)
{
	// Strong auto-leveling for beginners
	FRotator CurrentRotation = GetOwner()->GetActorRotation();
	
	// Calculate level flight target (pitch and roll = 0)
	FRotator TargetRotation = FRotator(0.0f, CurrentRotation.Yaw, 0.0f);
	
	// Apply strong correction
	FRotator RotationError = TargetRotation - CurrentRotation;
	OutRotation += FVector(
		RotationError.Pitch * 0.5f,
		0.0f, // Don't auto-correct yaw
		RotationError.Roll * 0.5f
	);
	
	// Reduce thrust during aggressive maneuvers
	if (RotationError.Euler().Size() > 45.0f)
	{
		OutThrust *= 0.7f;
	}
}

void UFlightController::ApplyOrbitalAssistance(FVector& OutThrust, FVector& OutRotation, float DeltaTime)
{
	// Orbital assistance helps maintain stable orbits
	// This would integrate with the OrbitalBody system
	AOrbitalBody* OrbitalBody = Cast<AOrbitalBody>(GetOwner());
	if (!OrbitalBody || !OrbitalBody->OrbitTarget.IsValid())
		return;
		
	// Calculate prograde/retrograde vectors
	FVector ToTarget = OrbitalBody->OrbitTarget->GetActorLocation() - GetOwner()->GetActorLocation();
	FVector Prograde = OrbitalBody->Velocity.GetSafeNormal();
	FVector RadialIn = ToTarget.GetSafeNormal();
	FVector Normal = FVector::CrossProduct(Prograde, RadialIn).GetSafeNormal();
	
	// Provide assistance based on input direction
	FVector LocalThrust = GetOwner()->GetActorTransform().InverseTransformVectorNoScale(OutThrust);
	
	// Enhance prograde/retrograde thrust
	float ProgradeComponent = FVector::DotProduct(LocalThrust, Prograde);
	if (FMath::Abs(ProgradeComponent) > 0.1f)
	{
		// Boost prograde/retrograde efficiency
		OutThrust = Prograde * ProgradeComponent * 1.2f;
	}
	
	// Provide radial assistance for station keeping
	float RadialComponent = FVector::DotProduct(LocalThrust, RadialIn);
	if (FMath::Abs(RadialComponent) < 0.1f)
	{
		// Auto-correct radial drift
		float RadialVelocity = FVector::DotProduct(OrbitalBody->Velocity, RadialIn);
		OutThrust -= RadialIn * RadialVelocity * 0.1f;
	}
}

void UFlightController::ApplyDockingAssistance(FVector& OutThrust, FVector& OutRotation, float DeltaTime)
{
	// Precision control for docking maneuvers
	OutThrust *= 0.3f; // Reduce thrust for precision
	OutRotation *= 0.5f; // Reduce rotation for precision
	
	// Add fine-tuning assistance
	FVector LocalThrust = GetOwner()->GetActorTransform().InverseTransformVectorNoScale(OutThrust);
	
	// Enhance small movements
	if (LocalThrust.Size() < 0.3f)
	{
		OutThrust *= 1.5f; // Boost small movements
	}
	
	// Reduce large movements
	if (LocalThrust.Size() > 0.7f)
	{
		OutThrust *= 0.5f; // Dampen large movements
	}
}

bool UFlightController::CheckForCollisions(const FVector& ProposedPosition, const FVector& CurrentVelocity)
{
	if (!CollisionDetectionBox)
		return false;
		
	// Update collision box position
	CollisionDetectionBox->SetWorldLocation(ProposedPosition);
	
	// Perform overlap test
	TArray<FHitResult> HitResults;
	FCollisionShape BoxShape = CollisionDetectionBox->GetCollisionShape();
	
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		CollisionDetectionBox->GetComponentLocation(),
		CollisionDetectionBox->GetComponentLocation() + CollisionDetectionBox->GetForwardVector() * 100.0f,
		CollisionDetectionBox->GetComponentQuat(),
		CollisionDetectionBox->GetCollisionObjectType(),
		BoxShape
	);
	
	// Draw debug visualization
	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			DrawDebugSphere(GetWorld(), Hit.Location, 50.0f, 16, FColor::Red, false, 2.0f);
		}
	}
	
	return bHit;
}

FVector UFlightController::GetCollisionAvoidanceVector(const FVector& CurrentPosition, const FVector& CurrentVelocity)
{
	FVector AvoidanceVector = FVector::ZeroVector;
	
	// Check for obstacles in front
	FVector ForwardVector = CurrentVelocity.GetSafeNormal();
	FVector CheckPosition = CurrentPosition + ForwardVector * 1000.0f;
	
	if (CheckForCollisions(CheckPosition, CurrentVelocity))
	{
		// Calculate avoidance vector
		FVector AvoidanceDirection = FVector::CrossProduct(ForwardVector, FVector::UpVector);
		AvoidanceVector = AvoidanceDirection.GetSafeNormal() * 500.0f;
	}
	
	return AvoidanceVector;
}

void UFlightController::ResetInputs()
{
	RawThrustInput = FVector::ZeroVector;
	RawRotationInput = FVector::ZeroVector;
	SmoothedThrustInput = FVector::ZeroVector;
	SmoothedRotationInput = FVector::ZeroVector;
	PreviousThrustInput = FVector::ZeroVector;
	PreviousRotationInput = FVector::ZeroVector;
}

void UFlightController::SetAssistMode(EFlightAssistMode NewMode)
{
	if (AssistMode != NewMode)
	{
		AssistMode = NewMode;
		UE_LOG(LogTemp, Log, TEXT("Flight assist mode changed to: %s"), 
			*UEnum::GetValueAsString(NewMode));
	}
}

FString UFlightController::GetControllerStatus() const
{
	FString Status = TEXT("Flight Controller Status:\n");
	Status += FString::Printf(TEXT("Active: %s\n"), bIsControllerActive ? TEXT("Yes") : TEXT("No"));
	Status += FString::Printf(TEXT("Assist Mode: %s\n"), *UEnum::GetValueAsString(AssistMode));
	Status += FString::Printf(TEXT("Thrust Input: (%.2f, %.2f, %.2f)\n"), 
		SmoothedThrustInput.X, SmoothedThrustInput.Y, SmoothedThrustInput.Z);
	Status += FString::Printf(TEXT("Rotation Input: (%.2f, %.2f, %.2f)\n"), 
		SmoothedRotationInput.X, SmoothedRotationInput.Y, SmoothedRotationInput.Z);
	Status += FString::Printf(TEXT("Input Rate: %.1f Hz\n"), AverageInputRate);
	
	return Status;
}

void UFlightController::SmoothInputs(float DeltaTime)
{
	// Smooth thrust input
	SmoothedThrustInput = SmoothVector(
		PreviousThrustInput,
		RawThrustInput,
		SmoothingConfig.ThrottleSmoothing,
		DeltaTime
	);
	
	// Smooth rotation input
	SmoothedRotationInput = SmoothVector(
		PreviousRotationInput,
		RawRotationInput,
		SmoothingConfig.RotationSmoothing,
		DeltaTime
	);
	
	// Update previous values
	PreviousThrustInput = SmoothedThrustInput;
	PreviousRotationInput = SmoothedRotationInput;
}

void UFlightController::ApplyDeadzone(FVector& Input)
{
	if (Input.Size() < SmoothingConfig.Deadzone)
	{
		Input = FVector::ZeroVector;
	}
	else
	{
		// Scale input to remove deadzone discontinuity
		float Scale = (Input.Size() - SmoothingConfig.Deadzone) / (1.0f - SmoothingConfig.Deadzone);
		Input = Input.GetSafeNormal() * Scale;
	}
}

void UFlightController::ApplyInversion(FVector& Input)
{
	if (bInvertPitch)
	{
		Input.X *= -1.0f;
	}
	if (bInvertYaw)
	{
		Input.Y *= -1.0f;
	}
}

float UFlightController::SmoothValue(float Current, float Target, float SmoothingFactor, float DeltaTime)
{
	// Exponential smoothing
	float Alpha = 1.0f - FMath::Exp(-SmoothingFactor * DeltaTime);
	return FMath::Lerp(Current, Target, Alpha);
}

FVector UFlightController::SmoothVector(const FVector& Current, const FVector& Target, float SmoothingFactor, float DeltaTime)
{
	return FVector(
		SmoothValue(Current.X, Target.X, SmoothingFactor, DeltaTime),
		SmoothValue(Current.Y, Target.Y, SmoothingFactor, DeltaTime),
		SmoothValue(Current.Z, Target.Z, SmoothingFactor, DeltaTime)
	);
}

bool UFlightController::IsWithinDeadzone(const FVector& Input) const
{
	return Input.Size() < SmoothingConfig.Deadzone;
}