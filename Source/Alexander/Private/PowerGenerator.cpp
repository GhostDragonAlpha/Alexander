// Copyright Epic Games, Inc. All Rights Reserved.

#include "PowerGenerator.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

APowerGenerator::APowerGenerator()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	RootComponent = RootComp;

	// Create generator mesh
	GeneratorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GeneratorMesh"));
	GeneratorMesh->SetupAttachment(RootComp);

	// Create audio component
	EngineSound = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineSound"));
	EngineSound->SetupAttachment(GeneratorMesh);
	EngineSound->bAutoActivate = false;

	// Create exhaust effect
	ExhaustEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ExhaustEffect"));
	ExhaustEffect->SetupAttachment(GeneratorMesh);
	ExhaustEffect->bAutoActivate = false;

	// Default properties
	Status = EGeneratorStatus::Off;
	FuelType = EGeneratorFuelType::Diesel;
	MaxPowerOutput = 5000.0f; // 5 kW
	CurrentPowerOutput = 0.0f;
	FuelConsumptionRate = 2.0f; // 2 liters per hour at max output
	CurrentFuelLevel = 50.0f; // Start with 50 liters
	MaxFuelCapacity = 100.0f; // 100 liter tank
	Efficiency = 0.30f; // 30% efficiency (typical for diesel generator)
	bIsRunning = false;
	StartupTime = 3.0f; // 3 seconds to start
	StartupProgress = 0.0f;
}

void APowerGenerator::BeginPlay()
{
	Super::BeginPlay();

	UpdateStatus();
	UpdateEffects();
}

void APowerGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateStatus();
	UpdateStartup(DeltaTime);
	UpdatePowerOutput(DeltaTime);
	ConsumeFuel(DeltaTime);
	UpdateEffects();
}

void APowerGenerator::StartGenerator()
{
	if (Status == EGeneratorStatus::Running || Status == EGeneratorStatus::Starting)
	{
		return;
	}

	if (!HasSufficientFuel())
	{
		Status = EGeneratorStatus::OutOfFuel;
		return;
	}

	Status = EGeneratorStatus::Starting;
	StartupProgress = 0.0f;

	// Play startup sound
	if (EngineStartupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EngineStartupSound, GetActorLocation());
	}
}

void APowerGenerator::StopGenerator()
{
	if (Status == EGeneratorStatus::Off)
	{
		return;
	}

	Status = EGeneratorStatus::Off;
	bIsRunning = false;
	CurrentPowerOutput = 0.0f;
	StartupProgress = 0.0f;

	// Play shutdown sound
	if (EngineShutdownSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EngineShutdownSound, GetActorLocation());
	}
}

void APowerGenerator::ToggleGenerator()
{
	if (bIsRunning)
	{
		StopGenerator();
	}
	else
	{
		StartGenerator();
	}
}

void APowerGenerator::Refuel(float Amount)
{
	CurrentFuelLevel = FMath::Clamp(CurrentFuelLevel + Amount, 0.0f, MaxFuelCapacity);

	// If we were out of fuel and now have fuel, allow restart
	if (Status == EGeneratorStatus::OutOfFuel && CurrentFuelLevel > 0.0f)
	{
		Status = EGeneratorStatus::Off;
	}
}

float APowerGenerator::GetFuelLevelPercent() const
{
	if (MaxFuelCapacity <= 0.0f)
	{
		return 0.0f;
	}

	return CurrentFuelLevel / MaxFuelCapacity;
}

float APowerGenerator::GetEstimatedRuntime() const
{
	if (FuelConsumptionRate <= 0.0f || CurrentFuelLevel <= 0.0f)
	{
		return 0.0f;
	}

	return CurrentFuelLevel / FuelConsumptionRate;
}

bool APowerGenerator::HasSufficientFuel() const
{
	return CurrentFuelLevel > 0.0f;
}

float APowerGenerator::GetPowerOutputPercent() const
{
	if (MaxPowerOutput <= 0.0f)
	{
		return 0.0f;
	}

	return CurrentPowerOutput / MaxPowerOutput;
}

float APowerGenerator::GetAvailablePower_Implementation() const
{
	return CurrentPowerOutput;
}

float APowerGenerator::GetMaxPowerCapacity_Implementation() const
{
	return MaxPowerOutput;
}

EPowerSourceType APowerGenerator::GetPowerSourceType_Implementation() const
{
	return EPowerSourceType::Generator;
}

void APowerGenerator::UpdatePowerOutput(float DeltaTime)
{
	if (Status == EGeneratorStatus::Running)
	{
		// Generator provides full power when running
		CurrentPowerOutput = MaxPowerOutput * Efficiency;
	}
	else if (Status == EGeneratorStatus::Starting)
	{
		// Ramp up power during startup
		CurrentPowerOutput = MaxPowerOutput * Efficiency * StartupProgress;
	}
	else
	{
		// No power when off or out of fuel
		CurrentPowerOutput = 0.0f;
	}
}

void APowerGenerator::ConsumeFuel(float DeltaTime)
{
	if (Status != EGeneratorStatus::Running)
	{
		return;
	}

	// Calculate fuel consumption based on power output
	float PowerPercent = GetPowerOutputPercent();
	float FuelConsumed = FuelConsumptionRate * PowerPercent * (DeltaTime / 3600.0f); // Convert to hours

	CurrentFuelLevel = FMath::Max(0.0f, CurrentFuelLevel - FuelConsumed);

	// Check if we ran out of fuel
	if (CurrentFuelLevel <= 0.0f)
	{
		Status = EGeneratorStatus::OutOfFuel;
		bIsRunning = false;
	}
}

void APowerGenerator::UpdateStartup(float DeltaTime)
{
	if (Status != EGeneratorStatus::Starting)
	{
		return;
	}

	StartupProgress += DeltaTime / StartupTime;

	if (StartupProgress >= 1.0f)
	{
		StartupProgress = 1.0f;
		Status = EGeneratorStatus::Running;
		bIsRunning = true;
	}
}

void APowerGenerator::UpdateEffects()
{
	bool bShouldPlayEffects = (Status == EGeneratorStatus::Running || Status == EGeneratorStatus::Starting);

	// Update audio
	if (EngineSound)
	{
		if (bShouldPlayEffects && !EngineSound->IsPlaying())
		{
			if (EngineRunningSound)
			{
				EngineSound->SetSound(EngineRunningSound);
				EngineSound->Play();
			}
		}
		else if (!bShouldPlayEffects && EngineSound->IsPlaying())
		{
			EngineSound->Stop();
		}

		// Adjust volume based on power output
		if (EngineSound->IsPlaying())
		{
			float VolumeMultiplier = FMath::Lerp(0.5f, 1.0f, GetPowerOutputPercent());
			EngineSound->SetVolumeMultiplier(VolumeMultiplier);
		}
	}

	// Update exhaust effect
	if (ExhaustEffect)
	{
		if (bShouldPlayEffects && !ExhaustEffect->IsActive())
		{
			ExhaustEffect->Activate();
		}
		else if (!bShouldPlayEffects && ExhaustEffect->IsActive())
		{
			ExhaustEffect->Deactivate();
		}

		// Adjust spawn rate based on power output
		if (ExhaustEffect->IsActive())
		{
			float SpawnRate = FMath::Lerp(10.0f, 50.0f, GetPowerOutputPercent());
			ExhaustEffect->SetFloatParameter(FName("SpawnRate"), SpawnRate);
		}
	}
}

void APowerGenerator::UpdateStatus()
{
	// Check for out of fuel condition
	if (bIsRunning && CurrentFuelLevel <= 0.0f)
	{
		Status = EGeneratorStatus::OutOfFuel;
		bIsRunning = false;
		return;
	}

	// Status is managed by Start/Stop functions and startup logic
}
