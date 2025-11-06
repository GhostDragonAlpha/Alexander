// Copyright Epic Games, Inc. All Rights Reserved.

#include "EngineComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"

UEngineComponent::UEngineComponent()
{
	PrimaryComponentTick.TickInterval = 0.016f; // ~60 FPS
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = false;
	
	// Create audio component
	EngineAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
	if (EngineAudioComponent)
	{
		EngineAudioComponent->bAutoActivate = false;
		EngineAudioComponent->SetVolumeMultiplier(0.0f);
	}
}

void UEngineComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeEngineAudio();
	CreateThrusterEffects();
}

void UEngineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (CurrentState == EEngineState::Running || CurrentState == EEngineState::Overload)
	{
		UpdateEngineState(DeltaTime);
		UpdateTemperature(DeltaTime);
		UpdateFuel(DeltaTime);
		CheckEngineWarnings();
		UpdateEngineEffects();
		UpdateEngineAudio();
	}
}

bool UEngineComponent::StartEngine()
{
	if (!CanStart())
	{
		return false;
	}
	
	if (CurrentState == EEngineState::Offline || CurrentState == EEngineState::Idle)
	{
		EEngineState OldState = CurrentState;
		CurrentState = EEngineState::Running;
		OnEngineStateChanged.Broadcast(OldState, CurrentState);
		return true;
	}
	
	return false;
}

void UEngineComponent::StopEngine()
{
	EEngineState OldState = CurrentState;
	CurrentState = EEngineState::Idle;
	CurrentThrustLevel = 0.0f;
	OnEngineStateChanged.Broadcast(OldState, CurrentState);
}

void UEngineComponent::SetThrustLevel(float ThrustLevel)
{
	CurrentThrustLevel = FMath::Clamp(ThrustLevel, 0.0f, 1.0f);
	
	if (CurrentState != EEngineState::Running && CurrentState != EEngineState::Overload)
	{
		CurrentThrustLevel = 0.0f;
	}
}

void UEngineComponent::EmergencyThrust(float OverrideLevel)
{
	if (CurrentState != EEngineState::Offline)
	{
		CurrentThrustLevel = FMath::Clamp(OverrideLevel, 0.0f, 1.5f);
		if (CurrentState == EEngineState::Idle)
		{
			StartEngine();
		}
		CurrentState = EEngineState::Emergency;
	}
}

void UEngineComponent::RepairEngine(float RepairAmount)
{
	EngineHealth = FMath::Clamp(EngineHealth + RepairAmount, 0.0f, 1.0f);
	
	if (EngineHealth > 0.7f && CurrentState == EEngineState::Damaged)
	{
		CurrentState = EEngineState::Idle;
	}
}

void UEngineComponent::Refuel(float FuelAmount)
{
	FuelLevel = FMath::Clamp(FuelLevel + FuelAmount, 0.0f, 1.0f);
}

float UEngineComponent::GetActualThrust() const
{
	if (CurrentState == EEngineState::Offline || CurrentState == EEngineState::Idle)
	{
		return 0.0f;
	}
	
	float HealthFactor = EngineHealth;
	float EfficiencyFactor = EngineEfficiency;
	float ActualThrust = MaxThrust * CurrentThrustLevel * HealthFactor * EfficiencyFactor;
	
	return ActualThrust;
}

float UEngineComponent::GetCurrentFuelConsumption() const
{
	if (CurrentState == EEngineState::Offline || CurrentState == EEngineState::Idle)
	{
		return 0.0f;
	}
	
	return FuelConsumptionRate * CurrentThrustLevel;
}

float UEngineComponent::GetCurrentHeatGeneration() const
{
	if (CurrentState == EEngineState::Offline || CurrentState == EEngineState::Idle)
	{
		return 0.0f;
	}
	
	return HeatGenerationRate * CurrentThrustLevel;
}

bool UEngineComponent::IsOperational() const
{
	return CurrentState != EEngineState::Offline && CurrentState != EEngineState::Damaged && EngineHealth > 0.3f;
}

bool UEngineComponent::CanStart() const
{
	return FuelLevel > 0.1f && EngineHealth > 0.3f && CurrentState != EEngineState::Damaged;
}

FString UEngineComponent::GetEngineStatus() const
{
	FString StatusText;
	
	switch (CurrentState)
	{
		case EEngineState::Offline:
			StatusText = TEXT("OFFLINE");
			break;
		case EEngineState::Idle:
			StatusText = TEXT("IDLE");
			break;
		case EEngineState::Starting:
			StatusText = TEXT("STARTING");
			break;
		case EEngineState::Running:
			StatusText = FString::Printf(TEXT("RUNNING (%.0f%%)"), CurrentThrustLevel * 100.0f);
			break;
		case EEngineState::Overload:
			StatusText = TEXT("OVERLOAD!");
			break;
		case EEngineState::Damaged:
			StatusText = TEXT("DAMAGED");
			break;
		case EEngineState::Emergency:
			StatusText = TEXT("EMERGENCY!");
			break;
		default:
			StatusText = TEXT("UNKNOWN");
			break;
	}
	
	return StatusText;
}

void UEngineComponent::UpdateEngineEffects()
{
	if (CurrentState == EEngineState::Running || CurrentState == EEngineState::Overload || CurrentState == EEngineState::Emergency)
	{
		UpdateThrusterIntensity(CurrentThrustLevel);
		UpdateEngineGlow(CurrentThrustLevel);
	}
	
	if (CurrentState == EEngineState::Damaged || EngineHealth < 0.5f)
	{
		UpdateDamageEffects();
	}
}

void UEngineComponent::UpdateEngineAudio()
{
	if (!EngineAudioComponent)
	{
		return;
	}
	
	if (CurrentState == EEngineState::Running || CurrentState == EEngineState::Overload || CurrentState == EEngineState::Emergency)
	{
		if (!EngineAudioComponent->IsPlaying() && ThrustSound)
		{
			EngineAudioComponent->SetSound(ThrustSound);
			EngineAudioComponent->Play();
		}
		
		EngineAudioComponent->SetVolumeMultiplier(CurrentThrustLevel);
	}
	else if (CurrentState == EEngineState::Idle)
	{
		if (!EngineAudioComponent->IsPlaying() && IdleSound)
		{
			EngineAudioComponent->SetSound(IdleSound);
			EngineAudioComponent->Play();
		}
		
		EngineAudioComponent->SetVolumeMultiplier(0.3f);
	}
	else
	{
		EngineAudioComponent->Stop();
	}
}

void UEngineComponent::UpdateDamageEffects()
{
	// Placeholder for damage effect updates
	// Could spawn particle effects, adjust material colors, etc.
}

void UEngineComponent::UpdateEngineState(float DeltaTime)
{
	// Check for temperature-based state changes
	if (CurrentTemperature > MaxTemperature * 0.9f && CurrentState == EEngineState::Running)
	{
		CurrentState = EEngineState::Overload;
	}
	else if (CurrentTemperature < MaxTemperature * 0.7f && CurrentState == EEngineState::Overload)
	{
		CurrentState = EEngineState::Running;
	}
}

void UEngineComponent::UpdateTemperature(float DeltaTime)
{
	float HeatGeneration = GetCurrentHeatGeneration();
	float CoolingRate = MaxTemperature * 0.1f; // Cool down when not running
	
	if (CurrentState == EEngineState::Running || CurrentState == EEngineState::Overload || CurrentState == EEngineState::Emergency)
	{
		CurrentTemperature += HeatGeneration * DeltaTime;
	}
	else
	{
		CurrentTemperature -= CoolingRate * DeltaTime;
	}
	
	CurrentTemperature = FMath::Clamp(CurrentTemperature, 0.0f, MaxTemperature * 1.2f);
}

void UEngineComponent::UpdateFuel(float DeltaTime)
{
	float FuelConsumption = GetCurrentFuelConsumption();
	FuelLevel -= (FuelConsumption * DeltaTime) / 100.0f; // Normalize consumption
	
	if (FuelLevel <= 0.0f)
	{
		FuelLevel = 0.0f;
		StopEngine();
		OnFuelEmpty.Broadcast();
	}
}

void UEngineComponent::CheckEngineWarnings()
{
	// Check overheating
	if (CurrentTemperature > MaxTemperature * 0.8f && !bOverheatWarning)
	{
		bOverheatWarning = true;
		OnEngineOverheat.Broadcast(CurrentTemperature);
	}
	else if (CurrentTemperature < MaxTemperature * 0.6f)
	{
		bOverheatWarning = false;
	}
	
	// Check fuel
	if (FuelLevel < 0.2f && !bLowFuelWarning)
	{
		bLowFuelWarning = true;
	}
	else if (FuelLevel > 0.4f)
	{
		bLowFuelWarning = false;
	}
	
	// Check engine damage
	if (EngineHealth < 0.3f && !bDamageWarning)
	{
		bDamageWarning = true;
		CurrentState = EEngineState::Damaged;
		OnEngineDamaged.Broadcast(1.0f - EngineHealth);
	}
	else if (EngineHealth > 0.7f)
	{
		bDamageWarning = false;
	}
}

void UEngineComponent::CreateThrusterEffects()
{
	// Placeholder for thruster effect setup
}

void UEngineComponent::UpdateThrusterIntensity(float Intensity)
{
	// Update particle system intensity
	for (UParticleSystemComponent* ParticleComp : ThrusterComponents)
	{
		if (ParticleComp)
		{
			ParticleComp->SetFloatParameter(TEXT("Intensity"), Intensity);
		}
	}
}

void UEngineComponent::UpdateEngineGlow(float Intensity)
{
	// Update engine glow material parameter
	for (UMaterialInstanceDynamic* MatInstance : EngineGlowMaterials)
	{
		if (MatInstance)
		{
			MatInstance->SetScalarParameterValue(TEXT("GlowIntensity"), Intensity);
		}
	}
}

void UEngineComponent::InitializeEngineAudio()
{
	// Audio component already created in constructor
}
