// Copyright Epic Games, Inc. All Rights Reserved.

#include "IrrigationSystem.h"
#include "FarmPlot.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AIrrigationSystem::AIrrigationSystem()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootComp;

	// Create base mesh component
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(RootComponent);

	// Create watering radius sphere
	WateringRadius = CreateDefaultSubobject<USphereComponent>(TEXT("WateringRadius"));
	WateringRadius->SetupAttachment(RootComponent);
	WateringRadius->SetSphereRadius(1000.0f); // 10m radius in cm
	WateringRadius->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WateringRadius->SetCollisionResponseToAllChannels(ECR_Ignore);
	WateringRadius->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Create water spray effect component
	WaterSprayEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WaterSprayEffect"));
	WaterSprayEffect->SetupAttachment(RootComponent);
	WaterSprayEffect->bAutoActivate = false;

	// Default values
	Status = EIrrigationStatus::Inactive;
	bIsActive = false;
	WateringRadiusCm = 1000.0f; // 10m in cm
	WaterConsumptionRate = 100.0f; // 100 liters per hour
	CurrentWaterLevel = 1000.0f; // Start with 1000 liters
	MaxWaterCapacity = 1000.0f;
	WaterAmountPerCycle = 0.2f; // 20% water level increase per cycle
	WateringInterval = 30.0f; // Water every 30 seconds
	TimeSinceLastWatering = 0.0f;

	// Visual effect defaults
	NumSprayNozzles = 8;
	SpraySpawnRate = 100.0f;
	SprayParticleSize = 5.0f;

	// Power system defaults
	PowerRequirement = 500.0f; // 500 watts
	bIsPowered = false;
}

void AIrrigationSystem::BeginPlay()
{
	Super::BeginPlay();

	// Set initial sphere radius
	if (WateringRadius)
	{
		WateringRadius->SetSphereRadius(WateringRadiusCm);
	}

	// Create water spray effect if we have a system
	if (WaterSpraySystem && WaterSprayEffect)
	{
		WaterSprayEffect->SetAsset(WaterSpraySystem);
		CreateWaterSprayEffect();
	}

	UE_LOG(LogTemp, Log, TEXT("IrrigationSystem initialized at %s (Radius: %.1fm, Capacity: %.1fL)"),
		*GetActorLocation().ToString(), WateringRadiusCm / 100.0f, MaxWaterCapacity);
}

void AIrrigationSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsActive)
	{
		return;
	}

	// Update water consumption
	UpdateWaterConsumption(DeltaTime);

	// Update system status
	UpdateSystemStatus();

	// Update watering cycle
	TimeSinceLastWatering += DeltaTime;
	if (TimeSinceLastWatering >= WateringInterval)
	{
		WaterCropsInRadius();
		TimeSinceLastWatering = 0.0f;
	}

	// Update visual effects
	UpdateVisualEffects();
}

// ============================================================================
// IRRIGATION OPERATIONS
// ============================================================================

void AIrrigationSystem::Activate()
{
	if (bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("IrrigationSystem: Already active"));
		return;
	}

	if (!HasSufficientWater())
	{
		UE_LOG(LogTemp, Warning, TEXT("IrrigationSystem: Cannot activate - insufficient water"));
		Status = EIrrigationStatus::OutOfWater;
		return;
	}

	if (!bIsPowered)
	{
		UE_LOG(LogTemp, Warning, TEXT("IrrigationSystem: Cannot activate - no power"));
		Status = EIrrigationStatus::Inactive;
		return;
	}

	bIsActive = true;
	Status = EIrrigationStatus::Active;
	TimeSinceLastWatering = 0.0f;

	// Activate water spray effect
	if (WaterSprayEffect)
	{
		WaterSprayEffect->Activate(true);
	}

	UE_LOG(LogTemp, Log, TEXT("IrrigationSystem: Activated (Water: %.1fL / %.1fL, Power: %s)"),
		CurrentWaterLevel, MaxWaterCapacity, bIsPowered ? TEXT("Yes") : TEXT("No"));
}

void AIrrigationSystem::Deactivate()
{
	if (!bIsActive)
	{
		return;
	}

	bIsActive = false;
	Status = EIrrigationStatus::Inactive;

	// Deactivate water spray effect
	if (WaterSprayEffect)
	{
		WaterSprayEffect->Deactivate();
	}

	UE_LOG(LogTemp, Log, TEXT("IrrigationSystem: Deactivated"));
}

void AIrrigationSystem::Toggle()
{
	if (bIsActive)
	{
		Deactivate();
	}
	else
	{
		Activate();
	}
}

void AIrrigationSystem::RefillWater(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	float OldLevel = CurrentWaterLevel;
	CurrentWaterLevel = FMath::Clamp(CurrentWaterLevel + Amount, 0.0f, MaxWaterCapacity);
	float ActualAdded = CurrentWaterLevel - OldLevel;

	UE_LOG(LogTemp, Log, TEXT("IrrigationSystem: Refilled %.1fL (%.1fL -> %.1fL)"),
		ActualAdded, OldLevel, CurrentWaterLevel);

	// If we were out of water and now have enough, update status
	if (Status == EIrrigationStatus::OutOfWater && HasSufficientWater())
	{
		if (bIsActive)
		{
			Status = EIrrigationStatus::Active;
		}
		else
		{
			Status = EIrrigationStatus::Inactive;
		}
	}
}

float AIrrigationSystem::GetWaterLevelPercent() const
{
	if (MaxWaterCapacity <= 0.0f)
	{
		return 0.0f;
	}

	return (CurrentWaterLevel / MaxWaterCapacity) * 100.0f;
}

float AIrrigationSystem::GetTimeUntilEmpty() const
{
	if (!bIsActive || WaterConsumptionRate <= 0.0f || CurrentWaterLevel <= 0.0f)
	{
		return 0.0f;
	}

	// Calculate hours until empty, then convert to seconds
	float HoursUntilEmpty = CurrentWaterLevel / WaterConsumptionRate;
	return HoursUntilEmpty * 3600.0f; // Convert to seconds
}

bool AIrrigationSystem::HasSufficientWater() const
{
	// Need at least 10% capacity to operate
	return CurrentWaterLevel >= (MaxWaterCapacity * 0.1f);
}

TArray<AFarmPlot*> AIrrigationSystem::GetFarmPlotsInRange() const
{
	TArray<AFarmPlot*> FarmPlots;

	if (!GetWorld())
	{
		return FarmPlots;
	}

	// Find all farm plots in the world
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFarmPlot::StaticClass(), FoundActors);

	// Filter by distance
	FVector SystemLocation = GetActorLocation();
	float RadiusSquared = WateringRadiusCm * WateringRadiusCm;

	for (AActor* Actor : FoundActors)
	{
		AFarmPlot* FarmPlot = Cast<AFarmPlot>(Actor);
		if (FarmPlot)
		{
			float DistanceSquared = FVector::DistSquared(SystemLocation, FarmPlot->GetActorLocation());
			if (DistanceSquared <= RadiusSquared)
			{
				FarmPlots.Add(FarmPlot);
			}
		}
	}

	return FarmPlots;
}

void AIrrigationSystem::WaterCropsInRadius()
{
	if (!HasSufficientWater())
	{
		UE_LOG(LogTemp, Warning, TEXT("IrrigationSystem: Cannot water - insufficient water"));
		Status = EIrrigationStatus::OutOfWater;
		Deactivate();
		return;
	}

	// Get all farm plots in range
	TArray<AFarmPlot*> FarmPlots = GetFarmPlotsInRange();

	if (FarmPlots.Num() == 0)
	{
		UE_LOG(LogTemp, Verbose, TEXT("IrrigationSystem: No farm plots in range"));
		return;
	}

	// Water each farm plot
	int32 WateredPlots = 0;
	for (AFarmPlot* FarmPlot : FarmPlots)
	{
		if (FarmPlot)
		{
			FarmPlot->WaterPlot(WaterAmountPerCycle);
			WateredPlots++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("IrrigationSystem: Watered %d farm plots (Water: %.1fL)"),
		WateredPlots, CurrentWaterLevel);
}

// ============================================================================
// PROTECTED HELPERS
// ============================================================================

void AIrrigationSystem::UpdateWaterConsumption(float DeltaTime)
{
	if (!bIsActive || CurrentWaterLevel <= 0.0f)
	{
		return;
	}

	// Calculate water consumed this frame
	// WaterConsumptionRate is in liters per hour, so convert to per second
	float WaterPerSecond = WaterConsumptionRate / 3600.0f;
	float WaterConsumed = WaterPerSecond * DeltaTime;

	// Deduct water
	CurrentWaterLevel = FMath::Max(0.0f, CurrentWaterLevel - WaterConsumed);

	// Log when water is running low
	float WaterPercent = GetWaterLevelPercent();
	if (WaterPercent <= 10.0f && WaterPercent > 0.0f)
	{
		static float LastWarningTime = 0.0f;
		float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		
		// Only log warning every 10 seconds to avoid spam
		if (CurrentTime - LastWarningTime >= 10.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("IrrigationSystem: Water level low (%.1f%%)"), WaterPercent);
			LastWarningTime = CurrentTime;
		}
	}

	// If water runs out, deactivate
	if (CurrentWaterLevel <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("IrrigationSystem: Out of water - deactivating"));
		Status = EIrrigationStatus::OutOfWater;
		Deactivate();
	}
}

void AIrrigationSystem::UpdateVisualEffects()
{
	if (!WaterSprayEffect)
	{
		return;
	}

	// Update spray effect based on status
	if (bIsActive && Status == EIrrigationStatus::Active)
	{
		if (!WaterSprayEffect->IsActive())
		{
			WaterSprayEffect->Activate(true);
		}
		UpdateSprayEffect();
	}
	else
	{
		if (WaterSprayEffect->IsActive())
		{
			WaterSprayEffect->Deactivate();
		}
	}
}

void AIrrigationSystem::CreateWaterSprayEffect()
{
	if (!WaterSprayEffect || !WaterSpraySystem)
	{
		return;
	}

	// Set up the Niagara system
	WaterSprayEffect->SetAsset(WaterSpraySystem);
	
	// Set initial parameters
	UpdateSprayEffect();

	UE_LOG(LogTemp, Log, TEXT("IrrigationSystem: Created water spray effect"));
}

void AIrrigationSystem::UpdateSprayEffect()
{
	if (!WaterSprayEffect)
	{
		return;
	}

	// Update spray parameters based on water level and status
	float WaterPercent = GetWaterLevelPercent() / 100.0f;
	
	// Reduce spray intensity when water is low
	float SprayIntensity = FMath::Lerp(0.3f, 1.0f, WaterPercent);
	
	// Set Niagara parameters
	WaterSprayEffect->SetFloatParameter(FName("SpawnRate"), SpraySpawnRate * SprayIntensity);
	WaterSprayEffect->SetFloatParameter(FName("ParticleSize"), SprayParticleSize);
	WaterSprayEffect->SetIntParameter(FName("NumNozzles"), NumSprayNozzles);
	WaterSprayEffect->SetFloatParameter(FName("SprayRadius"), WateringRadiusCm);
	
	// Set spray color based on water quality (could be extended)
	FLinearColor WaterColor = FLinearColor(0.3f, 0.6f, 0.9f, 0.5f); // Light blue
	WaterSprayEffect->SetColorParameter(FName("WaterColor"), WaterColor);
}

void AIrrigationSystem::UpdateSystemStatus()
{
	if (!bIsActive)
	{
		Status = EIrrigationStatus::Inactive;
		return;
	}

	if (!HasSufficientWater())
	{
		Status = EIrrigationStatus::OutOfWater;
		Deactivate();
		return;
	}

	if (!bIsPowered)
	{
		Status = EIrrigationStatus::Inactive;
		Deactivate();
		return;
	}

	// Check for malfunctions (could be extended with more conditions)
	// For now, just ensure we're active if we have water and power
	if (Status != EIrrigationStatus::Active)
	{
		Status = EIrrigationStatus::Active;
	}
}

// ============================================================================
// IPowerConsumer Interface Implementation
// ============================================================================

float AIrrigationSystem::GetPowerRequirement_Implementation() const
{
	return PowerRequirement;
}

void AIrrigationSystem::SetPowered_Implementation(bool bInIsPowered)
{
	bool bWasPowered = bIsPowered;
	bIsPowered = bInIsPowered;

	// If power is lost while active, deactivate
	if (!bIsPowered && bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("IrrigationSystem: Power lost - deactivating"));
		Deactivate();
	}

	// Log power state changes
	if (bWasPowered != bIsPowered)
	{
		UE_LOG(LogTemp, Log, TEXT("IrrigationSystem: Power %s"), bIsPowered ? TEXT("restored") : TEXT("lost"));
	}
}

bool AIrrigationSystem::IsPowered_Implementation() const
{
	return bIsPowered;
}
