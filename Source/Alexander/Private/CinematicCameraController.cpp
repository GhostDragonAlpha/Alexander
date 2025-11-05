// Copyright Epic Games, Inc. All Rights Reserved.

#include "CinematicCameraController.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Camera/CameraComponent.h"           // For UCameraComponent
#include "Kismet/KismetMathLibrary.h"

ACinematicCameraController::ACinematicCameraController()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create camera component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	RootComponent = CameraComponent;

	// Default camera settings
	CameraComponent->SetFieldOfView(90.0f);
}

void ACinematicCameraController::BeginPlay()
{
	Super::BeginPlay();
}

void ACinematicCameraController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update based on current mode
	switch (CurrentMode)
	{
	case ECinematicCameraMode::OrbitPlanet:
		UpdateOrbitCamera(DeltaTime);
		break;

	case ECinematicCameraMode::LandingApproach:
		UpdateLandingApproachCamera(DeltaTime);
		break;

	case ECinematicCameraMode::FarmOverview:
		UpdateFarmOverviewCamera(DeltaTime);
		break;

	case ECinematicCameraMode::BiomeShowcase:
		UpdateBiomeShowcaseCamera(DeltaTime);
		break;

	case ECinematicCameraMode::AtmosphericEntry:
		// Handled by transition system
		break;

	default:
		break;
	}

	// Update camera shake if enabled
	if (bCameraShakeEnabled)
	{
		UpdateCameraShake(DeltaTime);
	}

	// Update smooth transition if active
	if (bTransitioning)
	{
		TransitionElapsedTime += DeltaTime;
		float Alpha = FMath::Clamp(TransitionElapsedTime / TransitionDuration, 0.0f, 1.0f);
		
		// Smooth interpolation
		Alpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

		// Interpolate location and rotation
		FVector NewLocation = FMath::Lerp(TransitionStartLocation, TransitionTargetLocation, Alpha);
		FRotator NewRotation = FMath::Lerp(TransitionStartRotation, TransitionTargetRotation, Alpha);

		SetActorLocation(NewLocation);
		SetActorRotation(NewRotation);

		// End transition
		if (Alpha >= 1.0f)
		{
			bTransitioning = false;
		}
	}
}

void ACinematicCameraController::SetCameraMode(ECinematicCameraMode Mode)
{
	CurrentMode = Mode;
	UE_LOG(LogTemp, Log, TEXT("Cinematic camera mode set to: %d"), static_cast<int32>(Mode));
}

void ACinematicCameraController::SetOrbitTarget(AActor* Target, float Distance, float Height)
{
	OrbitTarget = Target;
	OrbitDistance = Distance;
	OrbitHeight = Height;
	OrbitAngle = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Orbit camera configured: Distance=%.1f, Height=%.1f"), Distance, Height);
}

void ACinematicCameraController::SetupLandingApproach(AActor* Ship, AActor* LandingPad)
{
	FollowShip = Ship;
	TargetLandingPad = LandingPad;

	UE_LOG(LogTemp, Log, TEXT("Landing approach camera configured"));
}

void ACinematicCameraController::SetupFarmOverview(FVector FarmCenterLocation, float ViewRadius)
{
	FarmCenter = FarmCenterLocation;
	FarmViewRadius = ViewRadius;
	FarmViewAngle = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Farm overview camera configured at: %s"), *FarmCenter.ToString());
}

void ACinematicCameraController::SetupBiomeShowcase(FVector BiomeCenterLocation, float SweepRadius)
{
	BiomeCenter = BiomeCenterLocation;
	BiomeSweepRadius = SweepRadius;
	BiomeSweepAngle = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Biome showcase camera configured at: %s"), *BiomeCenter.ToString());
}

void ACinematicCameraController::EnableCameraShake(const FCameraShakeSettings& Settings)
{
	bCameraShakeEnabled = true;
	ShakeSettings = Settings;
	ShakeTime = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Camera shake enabled: Intensity=%.2f"), Settings.Intensity);
}

void ACinematicCameraController::DisableCameraShake()
{
	bCameraShakeEnabled = false;
	UE_LOG(LogTemp, Log, TEXT("Camera shake disabled"));
}

void ACinematicCameraController::SetFieldOfView(float FOV)
{
	if (CameraComponent)
	{
		CameraComponent->SetFieldOfView(FOV);
	}
}

void ACinematicCameraController::SetFocusDistance(float Distance)
{
	// Set focus distance for depth of field
	// This would be implemented with post-process settings
	UE_LOG(LogTemp, Log, TEXT("Focus distance set to: %.1f"), Distance);
}

void ACinematicCameraController::SmoothTransitionTo(FVector TargetLocation, FRotator TargetRotation, float Duration)
{
	TransitionStartLocation = GetActorLocation();
	TransitionStartRotation = GetActorRotation();
	TransitionTargetLocation = TargetLocation;
	TransitionTargetRotation = TargetRotation;
	TransitionElapsedTime = 0.0f;
	TransitionDuration = Duration;
	bTransitioning = true;

	UE_LOG(LogTemp, Log, TEXT("Starting smooth transition over %.2f seconds"), Duration);
}

void ACinematicCameraController::UpdateOrbitCamera(float DeltaTime)
{
	if (!OrbitTarget)
	{
		return;
	}

	// Update orbit angle
	OrbitAngle += OrbitSpeed * DeltaTime;
	if (OrbitAngle >= 360.0f)
	{
		OrbitAngle -= 360.0f;
	}

	// Calculate orbit position
	FVector TargetLocation = OrbitTarget->GetActorLocation();
	float RadAngle = FMath::DegreesToRadians(OrbitAngle);
	
	FVector Offset;
	Offset.X = FMath::Cos(RadAngle) * OrbitDistance;
	Offset.Y = FMath::Sin(RadAngle) * OrbitDistance;
	Offset.Z = OrbitHeight;

	FVector NewLocation = TargetLocation + Offset;
	SetActorLocation(NewLocation);

	// Look at target
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(NewLocation, TargetLocation);
	SetActorRotation(LookAtRotation);
}

void ACinematicCameraController::UpdateLandingApproachCamera(float DeltaTime)
{
	if (!FollowShip || !TargetLandingPad)
	{
		return;
	}

	// Position camera behind and above ship
	FVector ShipLocation = FollowShip->GetActorLocation();
	FVector ShipForward = FollowShip->GetActorForwardVector();
	FVector ShipUp = FollowShip->GetActorUpVector();

	FVector CameraOffset = -ShipForward * 500.0f + ShipUp * 200.0f;
	FVector NewLocation = ShipLocation + CameraOffset;

	// Smooth camera movement
	FVector CurrentLocation = GetActorLocation();
	FVector SmoothedLocation = FMath::VInterpTo(CurrentLocation, NewLocation, DeltaTime, 2.0f);
	SetActorLocation(SmoothedLocation);

	// Look at landing pad
	FVector LandingPadLocation = TargetLandingPad->GetActorLocation();
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(SmoothedLocation, LandingPadLocation);
	
	FRotator CurrentRotation = GetActorRotation();
	FRotator SmoothedRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, DeltaTime, 2.0f);
	SetActorRotation(SmoothedRotation);
}

void ACinematicCameraController::UpdateFarmOverviewCamera(float DeltaTime)
{
	// Slowly orbit around farm
	FarmViewAngle += 15.0f * DeltaTime; // 15 degrees per second
	if (FarmViewAngle >= 360.0f)
	{
		FarmViewAngle -= 360.0f;
	}

	// Calculate camera position
	float RadAngle = FMath::DegreesToRadians(FarmViewAngle);
	
	FVector Offset;
	Offset.X = FMath::Cos(RadAngle) * FarmViewRadius;
	Offset.Y = FMath::Sin(RadAngle) * FarmViewRadius;
	Offset.Z = FarmViewRadius * 0.5f; // Elevated view

	FVector NewLocation = FarmCenter + Offset;
	SetActorLocation(NewLocation);

	// Look at farm center
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(NewLocation, FarmCenter);
	SetActorRotation(LookAtRotation);
}

void ACinematicCameraController::UpdateBiomeShowcaseCamera(float DeltaTime)
{
	// Sweep across biome
	BiomeSweepAngle += 10.0f * DeltaTime; // 10 degrees per second
	if (BiomeSweepAngle >= 180.0f)
	{
		BiomeSweepAngle = 0.0f; // Reset for continuous sweep
	}

	// Calculate camera position with varying height
	float RadAngle = FMath::DegreesToRadians(BiomeSweepAngle);
	float HeightVariation = FMath::Sin(RadAngle * 2.0f) * 500.0f;
	
	FVector Offset;
	Offset.X = FMath::Cos(RadAngle) * BiomeSweepRadius;
	Offset.Y = FMath::Sin(RadAngle) * BiomeSweepRadius;
	Offset.Z = BiomeSweepRadius * 0.3f + HeightVariation;

	FVector NewLocation = BiomeCenter + Offset;
	SetActorLocation(NewLocation);

	// Look at biome center with slight downward angle
	FVector LookAtTarget = BiomeCenter + FVector(0, 0, -200.0f);
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(NewLocation, LookAtTarget);
	SetActorRotation(LookAtRotation);
}

void ACinematicCameraController::UpdateCameraShake(float DeltaTime)
{
	ShakeTime += DeltaTime;

	// Calculate shake offset
	FVector ShakeOffset;
	ShakeOffset.X = FMath::Sin(ShakeTime * ShakeSettings.Frequency) * ShakeSettings.Intensity;
	ShakeOffset.Y = FMath::Cos(ShakeTime * ShakeSettings.Frequency * 1.3f) * ShakeSettings.Intensity;
	ShakeOffset.Z = FMath::Sin(ShakeTime * ShakeSettings.Frequency * 0.7f) * ShakeSettings.Intensity * 0.5f;

	// Apply shake to camera
	if (CameraComponent)
	{
		CameraComponent->SetRelativeLocation(ShakeOffset);

		// Add rotational shake if enabled
		if (ShakeSettings.bRotational)
		{
			FRotator ShakeRotation;
			ShakeRotation.Pitch = FMath::Sin(ShakeTime * ShakeSettings.Frequency * 1.5f) * ShakeSettings.Intensity * 0.5f;
			ShakeRotation.Yaw = FMath::Cos(ShakeTime * ShakeSettings.Frequency * 1.2f) * ShakeSettings.Intensity * 0.5f;
			ShakeRotation.Roll = FMath::Sin(ShakeTime * ShakeSettings.Frequency * 0.8f) * ShakeSettings.Intensity * 0.3f;
			
			CameraComponent->SetRelativeRotation(ShakeRotation);
		}
	}
}
