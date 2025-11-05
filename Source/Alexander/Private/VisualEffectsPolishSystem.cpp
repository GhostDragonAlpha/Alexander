// Copyright Epic Games, Inc. All Rights Reserved.

#include "VisualEffectsPolishSystem.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"

void UVisualEffectsPolishSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Set default quality based on platform
	#if PLATFORM_DESKTOP
		CurrentQuality = EVisualEffectsQuality::High;
	#else
		CurrentQuality = EVisualEffectsQuality::Medium;
	#endif

	ApplyQualitySettings();

	// Start update timer for ambient details
	GetWorld()->GetTimerManager().SetTimer(
		UpdateTimerHandle,
		[this]() { UpdateAmbientDetails(0.1f); },
		0.1f,
		true
	);

	UE_LOG(LogTemp, Log, TEXT("Visual Effects Polish System initialized"));
}

void UVisualEffectsPolishSystem::Deinitialize()
{
	CleanupAmbientDetails();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}

	Super::Deinitialize();
}

void UVisualEffectsPolishSystem::SetEffectsQuality(EVisualEffectsQuality Quality)
{
	CurrentQuality = Quality;
	ApplyQualitySettings();

	UE_LOG(LogTemp, Log, TEXT("Visual effects quality set to: %d"), static_cast<int32>(Quality));
}

void UVisualEffectsPolishSystem::EnhanceParticleEffect(UNiagaraComponent* ParticleComponent)
{
	if (!ParticleComponent)
	{
		return;
	}

	// Apply quality-based enhancements
	switch (CurrentQuality)
	{
	case EVisualEffectsQuality::Low:
		ParticleComponent->SetFloatParameter("ParticleCount", 100.0f);
		ParticleComponent->SetFloatParameter("DetailLevel", 0.5f);
		break;

	case EVisualEffectsQuality::Medium:
		ParticleComponent->SetFloatParameter("ParticleCount", 500.0f);
		ParticleComponent->SetFloatParameter("DetailLevel", 0.7f);
		break;

	case EVisualEffectsQuality::High:
		ParticleComponent->SetFloatParameter("ParticleCount", 1000.0f);
		ParticleComponent->SetFloatParameter("DetailLevel", 0.9f);
		break;

	case EVisualEffectsQuality::Ultra:
		ParticleComponent->SetFloatParameter("ParticleCount", 2000.0f);
		ParticleComponent->SetFloatParameter("DetailLevel", 1.0f);
		break;

	case EVisualEffectsQuality::Cinematic:
		ParticleComponent->SetFloatParameter("ParticleCount", 5000.0f);
		ParticleComponent->SetFloatParameter("DetailLevel", 1.0f);
		ParticleComponent->SetFloatParameter("EnableSubEmitters", 1.0f);
		break;
	}

	// Enable advanced features for high quality
	if (CurrentQuality >= EVisualEffectsQuality::High)
	{
		ParticleComponent->SetFloatParameter("EnableLighting", 1.0f);
		ParticleComponent->SetFloatParameter("EnableShadows", 1.0f);
		ParticleComponent->SetFloatParameter("EnableCollision", 1.0f);
	}
}

UNiagaraComponent* UVisualEffectsPolishSystem::SpawnEnhancedParticleEffect(
	UNiagaraSystem* System,
	FVector Location,
	FRotator Rotation,
	FVector Scale)
{
	if (!System || !GetWorld())
	{
		return nullptr;
	}

	UNiagaraComponent* ParticleComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		System,
		Location,
		Rotation,
		Scale,
		true,
		true,
		ENCPoolMethod::AutoRelease
	);

	if (ParticleComponent)
	{
		EnhanceParticleEffect(ParticleComponent);
	}

	return ParticleComponent;
}

void UVisualEffectsPolishSystem::TransitionMaterial(
	UMeshComponent* MeshComponent,
	UMaterialInterface* TargetMaterial,
	float Duration)
{
	if (!MeshComponent || !TargetMaterial)
	{
		return;
	}

	// Create transition entry
	FMaterialTransition Transition;
	Transition.MeshComponent = MeshComponent;
	Transition.TargetMaterial = TargetMaterial;
	Transition.ElapsedTime = 0.0f;
	Transition.Duration = Duration;

	ActiveTransitions.Add(Transition);

	UE_LOG(LogTemp, Log, TEXT("Started material transition over %.2f seconds"), Duration);
}

void UVisualEffectsPolishSystem::TransitionMaterialParameter(
	UMaterialInstanceDynamic* Material,
	FName ParameterName,
	float TargetValue,
	float Duration)
{
	if (!Material)
	{
		return;
	}

	// Get current value
	float CurrentValue = 0.0f;
	Material->GetScalarParameterValue(ParameterName, CurrentValue);

	// Create smooth transition using timer
	float StartValue = CurrentValue;
	float ElapsedTime = 0.0f;

	FTimerHandle TransitionTimer;
	GetWorld()->GetTimerManager().SetTimer(
		TransitionTimer,
		[Material, ParameterName, StartValue, TargetValue, Duration, &ElapsedTime]() mutable
		{
			ElapsedTime += 0.016f; // ~60 FPS
			float Alpha = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);
			
			// Smooth interpolation
			Alpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);
			
			float NewValue = FMath::Lerp(StartValue, TargetValue, Alpha);
			Material->SetScalarParameterValue(ParameterName, NewValue);
		},
		0.016f,
		true
	);
}

void UVisualEffectsPolishSystem::SpawnAmbientDetails(FVector Location, float Radius)
{
	if (!GetWorld())
	{
		return;
	}

	// Clean up old ambient details if too many
	if (AmbientParticles.Num() > 50)
	{
		CleanupAmbientDetails();
	}

	// Spawn dust particles
	if (AmbientConfig.bEnableDustParticles)
	{
		// Load dust particle system (would be actual asset in production)
		// UNiagaraSystem* DustSystem = LoadObject<UNiagaraSystem>(...);
		// UNiagaraComponent* Dust = SpawnEnhancedParticleEffect(DustSystem, Location, FRotator::ZeroRotator);
		// if (Dust) AmbientParticles.Add(Dust);
	}

	// Spawn insects
	if (AmbientConfig.bEnableInsects && CurrentQuality >= EVisualEffectsQuality::Medium)
	{
		// Spawn insect particles
	}

	// Spawn leaves
	if (AmbientConfig.bEnableLeaves && CurrentQuality >= EVisualEffectsQuality::High)
	{
		// Spawn falling leaves
	}

	// Spawn pollen
	if (AmbientConfig.bEnablePollen && CurrentQuality >= EVisualEffectsQuality::High)
	{
		// Spawn pollen particles
	}

	UE_LOG(LogTemp, Verbose, TEXT("Spawned ambient details at location: %s"), *Location.ToString());
}

void UVisualEffectsPolishSystem::SetAmbientDetailConfig(const FAmbientDetailConfig& Config)
{
	AmbientConfig = Config;
	
	// Update existing ambient details
	for (UNiagaraComponent* Particle : AmbientParticles)
	{
		if (Particle)
		{
			Particle->SetFloatParameter("Density", Config.ParticleDensity);
		}
	}
}

void UVisualEffectsPolishSystem::EnableCinematicMode(bool bEnable)
{
	bCinematicModeEnabled = bEnable;

	if (bEnable)
	{
		// Apply cinematic quality settings
		SetEffectsQuality(EVisualEffectsQuality::Cinematic);
		
		// Enable cinematic post-process effects
		SetDepthOfFieldSettings(500.0f, 2.8f);
		SetMotionBlurAmount(0.5f);
		SetVignetteIntensity(0.3f);
		SetBloomIntensity(0.8f);
	}
	else
	{
		// Restore normal settings
		SetEffectsQuality(EVisualEffectsQuality::High);
		SetMotionBlurAmount(0.2f);
		SetVignetteIntensity(0.0f);
	}

	UE_LOG(LogTemp, Log, TEXT("Cinematic mode: %s"), bEnable ? TEXT("Enabled") : TEXT("Disabled"));
}

void UVisualEffectsPolishSystem::SetDepthOfFieldSettings(float FocalDistance, float Aperture)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return;
	}

	// Apply to camera post-process settings
	// This would be implemented with actual camera component access
	UE_LOG(LogTemp, Log, TEXT("Depth of field: Distance=%.1f, Aperture=%.1f"), FocalDistance, Aperture);
}

void UVisualEffectsPolishSystem::SetMotionBlurAmount(float Amount)
{
	// Apply motion blur settings to post-process volume
	// This would be implemented with actual post-process volume access
	UE_LOG(LogTemp, Log, TEXT("Motion blur amount: %.2f"), Amount);
}

void UVisualEffectsPolishSystem::SetVignetteIntensity(float Intensity)
{
	// Apply vignette settings to post-process volume
	UE_LOG(LogTemp, Log, TEXT("Vignette intensity: %.2f"), Intensity);
}

void UVisualEffectsPolishSystem::SetBloomIntensity(float Intensity)
{
	// Apply bloom settings to post-process volume
	UE_LOG(LogTemp, Log, TEXT("Bloom intensity: %.2f"), Intensity);
}

void UVisualEffectsPolishSystem::SetColorGrading(FLinearColor Tint, float Saturation, float Contrast)
{
	// Apply color grading to post-process volume
	UE_LOG(LogTemp, Log, TEXT("Color grading: Tint=%s, Saturation=%.2f, Contrast=%.2f"), 
		*Tint.ToString(), Saturation, Contrast);
}

void UVisualEffectsPolishSystem::UpdateAmbientDetails(float DeltaTime)
{
	// Update material transitions
	for (int32 i = ActiveTransitions.Num() - 1; i >= 0; --i)
	{
		FMaterialTransition& Transition = ActiveTransitions[i];
		Transition.ElapsedTime += DeltaTime;

		if (Transition.ElapsedTime >= Transition.Duration)
		{
			// Complete transition
			if (Transition.MeshComponent)
			{
				Transition.MeshComponent->SetMaterial(0, Transition.TargetMaterial);
			}
			ActiveTransitions.RemoveAt(i);
		}
		else
		{
			// Update transition progress
			float Alpha = Transition.ElapsedTime / Transition.Duration;
			Alpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);
			
			// Apply blended material (would need custom material setup)
		}
	}

	// Clean up destroyed ambient particles
	for (int32 i = AmbientParticles.Num() - 1; i >= 0; --i)
	{
		if (!AmbientParticles[i] || !AmbientParticles[i]->IsActive())
		{
			AmbientParticles.RemoveAt(i);
		}
	}
}

void UVisualEffectsPolishSystem::ApplyQualitySettings()
{
	// Apply quality-specific settings
	switch (CurrentQuality)
	{
	case EVisualEffectsQuality::Low:
		AmbientConfig.ParticleDensity = 0.3f;
		AmbientConfig.bEnableInsects = false;
		AmbientConfig.bEnableLeaves = false;
		AmbientConfig.bEnablePollen = false;
		break;

	case EVisualEffectsQuality::Medium:
		AmbientConfig.ParticleDensity = 0.6f;
		AmbientConfig.bEnableInsects = true;
		AmbientConfig.bEnableLeaves = false;
		AmbientConfig.bEnablePollen = false;
		break;

	case EVisualEffectsQuality::High:
		AmbientConfig.ParticleDensity = 0.9f;
		AmbientConfig.bEnableInsects = true;
		AmbientConfig.bEnableLeaves = true;
		AmbientConfig.bEnablePollen = true;
		break;

	case EVisualEffectsQuality::Ultra:
	case EVisualEffectsQuality::Cinematic:
		AmbientConfig.ParticleDensity = 1.0f;
		AmbientConfig.bEnableInsects = true;
		AmbientConfig.bEnableLeaves = true;
		AmbientConfig.bEnablePollen = true;
		AmbientConfig.SpawnRadius = 100.0f;
		break;
	}

	// Update existing ambient particles
	for (UNiagaraComponent* Particle : AmbientParticles)
	{
		if (Particle)
		{
			EnhanceParticleEffect(Particle);
		}
	}
}

void UVisualEffectsPolishSystem::CleanupAmbientDetails()
{
	// Destroy all ambient particles
	for (UNiagaraComponent* Particle : AmbientParticles)
	{
		if (Particle)
		{
			Particle->DestroyComponent();
		}
	}

	AmbientParticles.Empty();
}
