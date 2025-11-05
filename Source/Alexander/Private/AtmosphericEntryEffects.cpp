// Copyright Epic Games, Inc. All Rights Reserved.

#include "AtmosphericEntryEffects.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "NiagaraFunctionLibrary.h"

UAtmosphericEntryEffects::UAtmosphericEntryEffects()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	// Initialize state
	bEffectsEnabled = true;
	bEffectsActive = false;
	CurrentHeatShimmerIntensity = 0.0f;
	CurrentPlasmaIntensity = 0.0f;
	CurrentSoundVolume = 0.0f;
	CurrentShakeIntensity = 0.0f;
	TimeSinceLastShake = 0.0f;
	HeatShimmerTimeAccumulator = 0.0f;
	PlasmaTimeAccumulator = 0.0f;

	// Initialize default configurations
	HeatShimmerConfig.DistortionIntensity = 0.5f;
	HeatShimmerConfig.AnimationSpeed = 2.0f;
	HeatShimmerConfig.DistortionScale = 1.0f;
	HeatShimmerConfig.StartAltitude = 80.0f;
	HeatShimmerConfig.PeakAltitude = 60.0f;
	HeatShimmerConfig.EndAltitude = 40.0f;

	PlasmaConfig.PlasmaColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
	PlasmaConfig.GlowIntensity = 0.8f;
	PlasmaConfig.ParticleSize = 50.0f;
	PlasmaConfig.SpawnRate = 100.0f;
	PlasmaConfig.StartAltitude = 90.0f;
	PlasmaConfig.PeakAltitude = 70.0f;
	PlasmaConfig.EndAltitude = 50.0f;

	SoundConfig.VolumeMultiplier = 1.0f;
	SoundConfig.PitchRange = FVector2D(0.8f, 1.2f);

	CameraShakeConfig.BaseIntensity = 0.5f;
	CameraShakeConfig.MaxIntensity = 1.5f;
	CameraShakeConfig.ShakeFrequency = 2.0f;
}

void UAtmosphericEntryEffects::BeginPlay()
{
	Super::BeginPlay();

	// Create audio components
	if (GetOwner())
	{
		WindAudioComponent = NewObject<UAudioComponent>(GetOwner(), TEXT("WindAudioComponent"));
		if (WindAudioComponent)
		{
			WindAudioComponent->RegisterComponent();
			WindAudioComponent->SetSound(SoundConfig.WindSound);
			WindAudioComponent->bAutoActivate = false;
		}

		PlasmaAudioComponent = NewObject<UAudioComponent>(GetOwner(), TEXT("PlasmaAudioComponent"));
		if (PlasmaAudioComponent)
		{
			PlasmaAudioComponent->RegisterComponent();
			PlasmaAudioComponent->SetSound(SoundConfig.PlasmaSound);
			PlasmaAudioComponent->bAutoActivate = false;
		}

		TurbulenceAudioComponent = NewObject<UAudioComponent>(GetOwner(), TEXT("TurbulenceAudioComponent"));
		if (TurbulenceAudioComponent)
		{
			TurbulenceAudioComponent->RegisterComponent();
			TurbulenceAudioComponent->SetSound(SoundConfig.TurbulenceSound);
			TurbulenceAudioComponent->bAutoActivate = false;
		}
	}

	// Create plasma effect component if not already created
	if (!PlasmaEffectComponent && GetOwner())
	{
		PlasmaEffectComponent = NewObject<UNiagaraComponent>(GetOwner(), TEXT("PlasmaEffectComponent"));
		if (PlasmaEffectComponent)
		{
			PlasmaEffectComponent->RegisterComponent();
			PlasmaEffectComponent->SetAutoActivate(false);
			PlasmaEffectComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
}

void UAtmosphericEntryEffects::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEffectsEnabled || !TransitionController)
	{
		return;
	}

	// Check if we're in atmospheric entry phase
	bool bShouldBeActive = TransitionController->IsInAtmosphere() && 
	                       TransitionController->GetAltitude() < HeatShimmerConfig.StartAltitude;

	if (bShouldBeActive != bEffectsActive)
	{
		bEffectsActive = bShouldBeActive;
		
		if (!bEffectsActive)
		{
			// Fade out all effects
			CurrentHeatShimmerIntensity = 0.0f;
			CurrentPlasmaIntensity = 0.0f;
			CurrentSoundVolume = 0.0f;
			CurrentShakeIntensity = 0.0f;
		}
	}

	if (bEffectsActive)
	{
		// Update all effect systems
		UpdateHeatShimmer(DeltaTime);
		UpdatePlasmaEffects(DeltaTime);
		UpdateEntrySounds(DeltaTime);
		UpdateCameraShake(DeltaTime);
	}
}

void UAtmosphericEntryEffects::Initialize(UTransitionController* InTransitionController)
{
	TransitionController = InTransitionController;
	
	if (TransitionController)
	{
		UE_LOG(LogTemp, Log, TEXT("AtmosphericEntryEffects initialized"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AtmosphericEntryEffects initialized with null TransitionController"));
	}
}

void UAtmosphericEntryEffects::SetEffectsEnabled(bool bEnabled)
{
	bEffectsEnabled = bEnabled;
	
	if (!bEnabled)
	{
		bEffectsActive = false;
		CurrentHeatShimmerIntensity = 0.0f;
		CurrentPlasmaIntensity = 0.0f;
		CurrentSoundVolume = 0.0f;
		CurrentShakeIntensity = 0.0f;

		// Stop all audio
		if (WindAudioComponent && WindAudioComponent->IsPlaying())
		{
			WindAudioComponent->Stop();
		}
		if (PlasmaAudioComponent && PlasmaAudioComponent->IsPlaying())
		{
			PlasmaAudioComponent->Stop();
		}
		if (TurbulenceAudioComponent && TurbulenceAudioComponent->IsPlaying())
		{
			TurbulenceAudioComponent->Stop();
		}

		// Deactivate plasma effects
		if (PlasmaEffectComponent && PlasmaEffectComponent->IsActive())
		{
			PlasmaEffectComponent->Deactivate();
		}
	}
}

// ============================================================================
// HEAT SHIMMER
// ============================================================================

void UAtmosphericEntryEffects::UpdateHeatShimmer(float DeltaTime)
{
	// Calculate target intensity
	float TargetIntensity = CalculateHeatShimmerIntensity();
	
	// Smooth interpolation
	float InterpSpeed = 2.0f;
	CurrentHeatShimmerIntensity = FMath::FInterpTo(CurrentHeatShimmerIntensity, TargetIntensity, DeltaTime, InterpSpeed);

	// Update time accumulator for animation
	HeatShimmerTimeAccumulator += DeltaTime * HeatShimmerConfig.AnimationSpeed;

	// Apply effect
	if (CurrentHeatShimmerIntensity > 0.01f)
	{
		ApplyHeatShimmerEffect(CurrentHeatShimmerIntensity);
	}
}

float UAtmosphericEntryEffects::CalculateHeatShimmerIntensity() const
{
	if (!TransitionController)
	{
		return 0.0f;
	}

	float Altitude = TransitionController->GetAltitude();

	// Heat shimmer is strongest during high-speed atmospheric entry
	// Intensity curve: starts at StartAltitude, peaks at PeakAltitude, ends at EndAltitude
	
	if (Altitude >= HeatShimmerConfig.StartAltitude)
	{
		return 0.0f;
	}
	else if (Altitude >= HeatShimmerConfig.PeakAltitude)
	{
		// Ramping up from start to peak
		float Progress = 1.0f - ((Altitude - HeatShimmerConfig.PeakAltitude) / 
		                         (HeatShimmerConfig.StartAltitude - HeatShimmerConfig.PeakAltitude));
		return FMath::Clamp(Progress, 0.0f, 1.0f);
	}
	else if (Altitude >= HeatShimmerConfig.EndAltitude)
	{
		// At peak or ramping down
		float Progress = (Altitude - HeatShimmerConfig.EndAltitude) / 
		                 (HeatShimmerConfig.PeakAltitude - HeatShimmerConfig.EndAltitude);
		return FMath::Clamp(Progress, 0.0f, 1.0f);
	}
	else
	{
		// Below end altitude, no heat shimmer
		return 0.0f;
	}
}

void UAtmosphericEntryEffects::ApplyHeatShimmerEffect(float Intensity)
{
	// Heat shimmer is typically implemented as a post-process material effect
	// This would be applied to the camera's post-process settings
	// For now, we'll log the intensity for Blueprint implementation
	
	// In a full implementation, you would:
	// 1. Get the player camera manager
	// 2. Access post-process settings
	// 3. Set material parameter for heat distortion
	
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
		{
			// Set post-process material parameters
			// This requires a post-process material with heat distortion
			// CameraManager->PostProcessSettings.WeightedBlendables.Array[0].Weight = Intensity;
		}
	}
}

// ============================================================================
// PLASMA EFFECTS
// ============================================================================

void UAtmosphericEntryEffects::UpdatePlasmaEffects(float DeltaTime)
{
	// Calculate target intensity
	float TargetIntensity = CalculatePlasmaIntensity();
	
	// Smooth interpolation
	float InterpSpeed = 3.0f;
	CurrentPlasmaIntensity = FMath::FInterpTo(CurrentPlasmaIntensity, TargetIntensity, DeltaTime, InterpSpeed);

	// Update time accumulator for animation
	PlasmaTimeAccumulator += DeltaTime;

	// Update particle effects
	UpdatePlasmaParticles(CurrentPlasmaIntensity);
}

float UAtmosphericEntryEffects::CalculatePlasmaIntensity() const
{
	if (!TransitionController)
	{
		return 0.0f;
	}

	float Altitude = TransitionController->GetAltitude();

	// Plasma effects occur at higher altitudes than heat shimmer
	// They represent ionization of air molecules during high-speed entry
	
	if (Altitude >= PlasmaConfig.StartAltitude)
	{
		return 0.0f;
	}
	else if (Altitude >= PlasmaConfig.PeakAltitude)
	{
		// Ramping up from start to peak
		float Progress = 1.0f - ((Altitude - PlasmaConfig.PeakAltitude) / 
		                         (PlasmaConfig.StartAltitude - PlasmaConfig.PeakAltitude));
		return FMath::Clamp(Progress, 0.0f, 1.0f);
	}
	else if (Altitude >= PlasmaConfig.EndAltitude)
	{
		// At peak or ramping down
		float Progress = (Altitude - PlasmaConfig.EndAltitude) / 
		                 (PlasmaConfig.PeakAltitude - PlasmaConfig.EndAltitude);
		return FMath::Clamp(Progress, 0.0f, 1.0f);
	}
	else
	{
		// Below end altitude, no plasma
		return 0.0f;
	}
}

void UAtmosphericEntryEffects::UpdatePlasmaParticles(float Intensity)
{
	if (!PlasmaEffectComponent)
	{
		return;
	}

	if (Intensity > 0.01f)
	{
		// Activate plasma effect if not already active
		if (!PlasmaEffectComponent->IsActive())
		{
			PlasmaEffectComponent->Activate();
		}

		// Update Niagara parameters
		PlasmaEffectComponent->SetFloatParameter(TEXT("Intensity"), Intensity);
		PlasmaEffectComponent->SetFloatParameter(TEXT("SpawnRate"), PlasmaConfig.SpawnRate * Intensity);
		PlasmaEffectComponent->SetFloatParameter(TEXT("ParticleSize"), PlasmaConfig.ParticleSize);
		PlasmaEffectComponent->SetColorParameter(TEXT("PlasmaColor"), PlasmaConfig.PlasmaColor);
		PlasmaEffectComponent->SetFloatParameter(TEXT("GlowIntensity"), PlasmaConfig.GlowIntensity * Intensity);
	}
	else
	{
		// Deactivate plasma effect when intensity is too low
		if (PlasmaEffectComponent->IsActive())
		{
			PlasmaEffectComponent->Deactivate();
		}
	}
}

// ============================================================================
// ENTRY SOUNDS
// ============================================================================

void UAtmosphericEntryEffects::UpdateEntrySounds(float DeltaTime)
{
	// Calculate target volume
	float TargetVolume = CalculateEntrySoundVolume();
	
	// Smooth interpolation
	float InterpSpeed = 2.0f;
	CurrentSoundVolume = FMath::FInterpTo(CurrentSoundVolume, TargetVolume, DeltaTime, InterpSpeed);

	// Play or update sounds
	PlayEntrySounds(CurrentSoundVolume);
}

float UAtmosphericEntryEffects::CalculateEntrySoundVolume() const
{
	if (!TransitionController)
	{
		return 0.0f;
	}

	// Sound volume is based on atmospheric density and velocity
	float AtmosphericDensity = TransitionController->GetAtmosphericDensity();
	
	// Volume increases with atmospheric density
	float Volume = AtmosphericDensity * SoundConfig.VolumeMultiplier;

	return FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAtmosphericEntryEffects::PlayEntrySounds(float Volume)
{
	if (!TransitionController)
	{
		return;
	}

	// Wind sound - plays throughout atmospheric entry
	if (WindAudioComponent && SoundConfig.WindSound)
	{
		if (Volume > 0.01f)
		{
			if (!WindAudioComponent->IsPlaying())
			{
				WindAudioComponent->Play();
			}
			WindAudioComponent->SetVolumeMultiplier(Volume);
			
			// Vary pitch based on velocity
			float PitchVariation = FMath::GetMappedRangeValueClamped(
				FVector2D(0.0f, 1.0f),
				SoundConfig.PitchRange,
				Volume
			);
			WindAudioComponent->SetPitchMultiplier(PitchVariation);
		}
		else if (WindAudioComponent->IsPlaying())
		{
			WindAudioComponent->Stop();
		}
	}

	// Plasma sound - plays during high-altitude entry
	if (PlasmaAudioComponent && SoundConfig.PlasmaSound)
	{
		float PlasmaVolume = CurrentPlasmaIntensity * Volume;
		
		if (PlasmaVolume > 0.01f)
		{
			if (!PlasmaAudioComponent->IsPlaying())
			{
				PlasmaAudioComponent->Play();
			}
			PlasmaAudioComponent->SetVolumeMultiplier(PlasmaVolume);
		}
		else if (PlasmaAudioComponent->IsPlaying())
		{
			PlasmaAudioComponent->Stop();
		}
	}

	// Turbulence sound - plays based on turbulence intensity
	if (TurbulenceAudioComponent && SoundConfig.TurbulenceSound)
	{
		float TurbulenceIntensity = TransitionController->GetTurbulenceIntensity();
		float TurbulenceVolume = TurbulenceIntensity * Volume;
		
		if (TurbulenceVolume > 0.01f)
		{
			if (!TurbulenceAudioComponent->IsPlaying())
			{
				TurbulenceAudioComponent->Play();
			}
			TurbulenceAudioComponent->SetVolumeMultiplier(TurbulenceVolume);
		}
		else if (TurbulenceAudioComponent->IsPlaying())
		{
			TurbulenceAudioComponent->Stop();
		}
	}
}

// ============================================================================
// CAMERA SHAKE
// ============================================================================

void UAtmosphericEntryEffects::UpdateCameraShake(float DeltaTime)
{
	// Calculate target shake intensity
	float TargetIntensity = CalculateCameraShakeIntensity();
	
	// Smooth interpolation
	float InterpSpeed = 4.0f;
	CurrentShakeIntensity = FMath::FInterpTo(CurrentShakeIntensity, TargetIntensity, DeltaTime, InterpSpeed);

	// Update shake timing
	TimeSinceLastShake += DeltaTime;

	// Trigger shake at specified frequency
	float ShakeInterval = 1.0f / CameraShakeConfig.ShakeFrequency;
	if (CurrentShakeIntensity > 0.01f && TimeSinceLastShake >= ShakeInterval)
	{
		TriggerCameraShake(CurrentShakeIntensity);
		TimeSinceLastShake = 0.0f;
	}
}

float UAtmosphericEntryEffects::CalculateCameraShakeIntensity() const
{
	if (!TransitionController)
	{
		return 0.0f;
	}

	// Camera shake intensity is based on turbulence
	float TurbulenceIntensity = TransitionController->GetTurbulenceIntensity();
	
	// Map turbulence to shake intensity range
	float ShakeIntensity = FMath::GetMappedRangeValueClamped(
		FVector2D(0.0f, 1.0f),
		FVector2D(CameraShakeConfig.BaseIntensity, CameraShakeConfig.MaxIntensity),
		TurbulenceIntensity
	);

	return ShakeIntensity;
}

void UAtmosphericEntryEffects::TriggerCameraShake(float Intensity)
{
	if (!CameraShakeConfig.ShakeClass)
	{
		return;
	}

	// Get player controller
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return;
	}

	// Trigger camera shake
	PC->ClientStartCameraShake(CameraShakeConfig.ShakeClass, Intensity);
}
