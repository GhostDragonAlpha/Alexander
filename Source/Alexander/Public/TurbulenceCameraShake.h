// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "TurbulenceCameraShake.generated.h"

/**
 * Camera shake for atmospheric turbulence during entry
 * Provides realistic buffeting and vibration effects
 */
UCLASS()
class ALEXANDER_API UTurbulenceCameraShake : public UCameraShakePattern
{
	GENERATED_BODY()

public:
	// UCameraShakeBase doesn't have a default constructor in UE 5.6
	// Camera shakes are created through the camera shake system

	// Rotation shake amplitude
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	FRotator RotationAmplitude;

	// Rotation shake frequency
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	FRotator RotationFrequency;

	// Location shake amplitude
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	FVector LocationAmplitude;

	// Location shake frequency
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	FVector LocationFrequency;

	// Duration of shake
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	float Duration;

	// Blend in time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	float BlendInTime;

	// Blend out time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	float BlendOutTime;

protected:
	virtual void StartShakePatternImpl(const FCameraShakePatternStartParams& Params) override;
	virtual void UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params, FCameraShakePatternUpdateResult& OutResult) override;
	virtual void StopShakePatternImpl(const FCameraShakePatternStopParams& Params) override;

private:
	// Current shake time
	float CurrentTime;

	// Current blend weight
	float CurrentBlendWeight;

	// Whether shake is active
	bool bIsActive;
};
