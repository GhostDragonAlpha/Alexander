// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "CinematicCameraController.generated.h"

/**
 * Cinematic camera mode types
 */
UENUM(BlueprintType)
enum class ECinematicCameraMode : uint8
{
	None,
	OrbitPlanet,
	LandingApproach,
	FarmOverview,
	BiomeShowcase,
	AtmosphericEntry,
	Custom
};

/**
 * Camera shake settings
 */
USTRUCT(BlueprintType)
struct FCameraShakeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
	float Intensity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
	float Frequency = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
	bool bRotational = true;
};

/**
 * Controller for cinematic camera modes
 */
UCLASS()
class ALEXANDER_API ACinematicCameraController : public AActor
{
	GENERATED_BODY()

public:
	ACinematicCameraController();

	virtual void Tick(float DeltaTime) override;

	// Camera mode control
	UFUNCTION(BlueprintCallable, Category = "Cinematic Camera")
	void SetCameraMode(ECinematicCameraMode Mode);

	UFUNCTION(BlueprintPure, Category = "Cinematic Camera")
	ECinematicCameraMode GetCurrentMode() const { return CurrentMode; }

	// Orbit camera
	UFUNCTION(BlueprintCallable, Category = "Cinematic Camera")
	void SetOrbitTarget(AActor* Target, float Distance, float Height);

	UFUNCTION(BlueprintCallable, Category = "Cinematic Camera")
	void SetOrbitSpeed(float Speed) { OrbitSpeed = Speed; }

	// Landing approach camera
	UFUNCTION(BlueprintCallable, Category = "Cinematic Camera")
	void SetupLandingApproach(AActor* Ship, AActor* LandingPad);

	// Farm overview camera
	UFUNCTION(BlueprintCallable, Category = "Cinematic Camera")
	void SetupFarmOverview(FVector FarmCenter, float ViewRadius);

	// Biome showcase camera
	UFUNCTION(BlueprintCallable, Category = "Cinematic Camera")
	void SetupBiomeShowcase(FVector BiomeCenter, float SweepRadius);

	// Camera effects
	UFUNCTION(BlueprintCallable, Category = "Cinematic Camera")
	void EnableCameraShake(const FCameraShakeSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Cinematic Camera")
	void DisableCameraShake();

	UFUNCTION(BlueprintCallable, Category = "Cinematic Camera")
	void SetFieldOfView(float FOV);

	UFUNCTION(BlueprintCallable, Category = "Cinematic Camera")
	void SetFocusDistance(float Distance);

	// Smooth transitions
	UFUNCTION(BlueprintCallable, Category = "Cinematic Camera")
	void SmoothTransitionTo(FVector TargetLocation, FRotator TargetRotation, float Duration);

protected:
	virtual void BeginPlay() override;

	void UpdateOrbitCamera(float DeltaTime);
	void UpdateLandingApproachCamera(float DeltaTime);
	void UpdateFarmOverviewCamera(float DeltaTime);
	void UpdateBiomeShowcaseCamera(float DeltaTime);
	void UpdateCameraShake(float DeltaTime);

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UCameraComponent* CameraComponent;

	UPROPERTY()
	ECinematicCameraMode CurrentMode = ECinematicCameraMode::None;

	// Orbit mode
	UPROPERTY()
	AActor* OrbitTarget = nullptr;

	UPROPERTY()
	float OrbitDistance = 1000.0f;

	UPROPERTY()
	float OrbitHeight = 500.0f;

	UPROPERTY()
	float OrbitSpeed = 30.0f;

	UPROPERTY()
	float OrbitAngle = 0.0f;

	// Landing approach mode
	UPROPERTY()
	AActor* FollowShip = nullptr;

	UPROPERTY()
	AActor* TargetLandingPad = nullptr;

	// Farm overview mode
	UPROPERTY()
	FVector FarmCenter = FVector::ZeroVector;

	UPROPERTY()
	float FarmViewRadius = 500.0f;

	UPROPERTY()
	float FarmViewAngle = 0.0f;

	// Biome showcase mode
	UPROPERTY()
	FVector BiomeCenter = FVector::ZeroVector;

	UPROPERTY()
	float BiomeSweepRadius = 2000.0f;

	UPROPERTY()
	float BiomeSweepAngle = 0.0f;

	// Camera shake
	UPROPERTY()
	bool bCameraShakeEnabled = false;

	UPROPERTY()
	FCameraShakeSettings ShakeSettings;

	UPROPERTY()
	float ShakeTime = 0.0f;

	// Smooth transition
	UPROPERTY()
	bool bTransitioning = false;

	UPROPERTY()
	FVector TransitionStartLocation;

	UPROPERTY()
	FRotator TransitionStartRotation;

	UPROPERTY()
	FVector TransitionTargetLocation;

	UPROPERTY()
	FRotator TransitionTargetRotation;

	UPROPERTY()
	float TransitionElapsedTime = 0.0f;

	UPROPERTY()
	float TransitionDuration = 1.0f;
};
