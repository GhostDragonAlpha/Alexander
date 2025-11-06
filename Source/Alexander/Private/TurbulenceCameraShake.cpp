// Copyright Epic Games, Inc. All Rights Reserved.

#include "TurbulenceCameraShake.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Kismet/KismetMathLibrary.h"

// Constructor removed - UCameraShakeBase doesn't have default constructor in UE 5.6

void UTurbulenceCameraShake::StartShakePatternImpl(const FCameraShakePatternStartParams& Params)
{
	CurrentTime = 0.0f;
	CurrentBlendWeight = 0.0f;
	bIsActive = true;
}

void UTurbulenceCameraShake::UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params, FCameraShakePatternUpdateResult& OutResult)
{
	if (!bIsActive)
	{
		return;
	}

	// Update time
	CurrentTime += Params.DeltaTime;

	// Calculate blend weight
	if (CurrentTime < BlendInTime)
	{
		// Blending in
		CurrentBlendWeight = CurrentTime / BlendInTime;
	}
	else if (CurrentTime > Duration - BlendOutTime)
	{
		// Blending out
		float BlendOutProgress = (Duration - CurrentTime) / BlendOutTime;
		CurrentBlendWeight = FMath::Clamp(BlendOutProgress, 0.0f, 1.0f);
	}
	else
	{
		// Full intensity
		CurrentBlendWeight = 1.0f;
	}

	// Apply shake scale from parameters
	float ShakeScale = Params.GetTotalScale();
	float FinalBlendWeight = CurrentBlendWeight * ShakeScale;

	// Calculate rotation shake using sine waves
	FRotator RotationShake;
	RotationShake.Pitch = FMath::Sin(CurrentTime * RotationFrequency.Pitch) * RotationAmplitude.Pitch * FinalBlendWeight;
	RotationShake.Yaw = FMath::Sin(CurrentTime * RotationFrequency.Yaw) * RotationAmplitude.Yaw * FinalBlendWeight;
	RotationShake.Roll = FMath::Sin(CurrentTime * RotationFrequency.Roll) * RotationAmplitude.Roll * FinalBlendWeight;

	// Calculate location shake using sine waves with phase offset
	FVector LocationShake;
	LocationShake.X = FMath::Sin(CurrentTime * LocationFrequency.X) * LocationAmplitude.X * FinalBlendWeight;
	LocationShake.Y = FMath::Sin(CurrentTime * LocationFrequency.Y + 1.0f) * LocationAmplitude.Y * FinalBlendWeight;
	LocationShake.Z = FMath::Sin(CurrentTime * LocationFrequency.Z + 2.0f) * LocationAmplitude.Z * FinalBlendWeight;

	// Apply shake to output
	OutResult.Location = LocationShake;
	OutResult.Rotation = RotationShake;

	// Check if shake is complete
	if (CurrentTime >= Duration)
	{
		FCameraShakePatternStopParams StopParams;
		StopParams.bImmediately = false;
		StopShakePatternImpl(StopParams);
	}
}

void UTurbulenceCameraShake::StopShakePatternImpl(const FCameraShakePatternStopParams& Params)
{
	bIsActive = false;
	CurrentBlendWeight = 0.0f;

	if (Params.bImmediately)
	{
		CurrentTime = Duration;
	}
}
