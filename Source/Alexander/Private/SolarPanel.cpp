// Copyright Epic Games, Inc. All Rights Reserved.

#include "SolarPanel.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DayNightCycleComponent.h"

ASolarPanel::ASolarPanel()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	RootComponent = RootComp;

	// Create panel mesh
	PanelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PanelMesh"));
	PanelMesh->SetupAttachment(RootComp);

	// Create mount mesh
	MountMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MountMesh"));
	MountMesh->SetupAttachment(RootComp);

	// Create sun direction indicator
	SunDirectionIndicator = CreateDefaultSubobject<UArrowComponent>(TEXT("SunDirectionIndicator"));
	SunDirectionIndicator->SetupAttachment(PanelMesh);
	SunDirectionIndicator->SetHiddenInGame(false);

	// Default properties
	MaxPowerOutput = 1000.0f; // 1 kW
	CurrentPowerOutput = 0.0f;
	Efficiency = 0.20f; // 20% efficiency (typical for solar panels)
	PanelArea = 2.0f; // 2 square meters
	bSunTracking = false;
	TrackingSpeed = 15.0f; // 15 degrees per second
	MinSunAngle = 10.0f; // Minimum 10 degrees above horizon
	PlanetActor = nullptr;
}

void ASolarPanel::BeginPlay()
{
	Super::BeginPlay();

	FindPlanetActor();
	UpdatePowerOutput();
}

void ASolarPanel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bSunTracking)
	{
		TrackSun(DeltaTime);
	}

	UpdatePowerOutput();
	UpdateVisuals();
}

void ASolarPanel::UpdatePowerOutput()
{
	float SunAngle = CalculateSunAngle();

	if (SunAngle < MinSunAngle)
	{
		// No power generation when sun is below minimum angle
		CurrentPowerOutput = 0.0f;
		return;
	}

	// Calculate power based on sun angle and efficiency
	float AngleEfficiency = CalculateAngleEfficiency(SunAngle);
	CurrentPowerOutput = MaxPowerOutput * Efficiency * AngleEfficiency;
}

FVector ASolarPanel::GetSunDirection() const
{
	if (!PlanetActor)
	{
		// Default to overhead sun
		return FVector(0.0f, 0.0f, 1.0f);
	}

	// Try to get sun direction from planet's day-night cycle component
	UDayNightCycleComponent* DayNightCycle = PlanetActor->FindComponentByClass<UDayNightCycleComponent>();
	if (DayNightCycle)
	{
		return DayNightCycle->GetSunDirection();
	}

	// Fallback to default
	return FVector(0.0f, 0.0f, 1.0f);
}

float ASolarPanel::CalculateSunAngle() const
{
	FVector SunDir = GetSunDirection();
	FVector PanelNormal = PanelMesh->GetUpVector();

	// Calculate angle between panel normal and sun direction
	float DotProduct = FVector::DotProduct(PanelNormal, SunDir);
	float AngleRadians = FMath::Acos(DotProduct);
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	return AngleDegrees;
}

float ASolarPanel::CalculateAngleEfficiency(float SunAngle) const
{
	// Optimal efficiency at 0 degrees (perpendicular to sun)
	// Efficiency drops off with cosine of angle
	float AngleRadians = FMath::DegreesToRadians(SunAngle);
	float CosineEfficiency = FMath::Cos(AngleRadians);

	// Clamp to 0-1 range
	return FMath::Clamp(CosineEfficiency, 0.0f, 1.0f);
}

void ASolarPanel::TrackSun(float DeltaTime)
{
	FVector SunDir = GetSunDirection();

	// Calculate target rotation to face sun
	FRotator TargetRotation = SunDir.Rotation();

	// Get current rotation
	FRotator CurrentRotation = PanelMesh->GetComponentRotation();

	// Interpolate towards target rotation
	FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		TargetRotation,
		DeltaTime,
		TrackingSpeed
	);

	PanelMesh->SetWorldRotation(NewRotation);
}

float ASolarPanel::GetPowerOutputPercent() const
{
	if (MaxPowerOutput <= 0.0f)
	{
		return 0.0f;
	}

	return CurrentPowerOutput / MaxPowerOutput;
}

float ASolarPanel::GetAvailablePower_Implementation() const
{
	return CurrentPowerOutput;
}

float ASolarPanel::GetMaxPowerCapacity_Implementation() const
{
	return MaxPowerOutput;
}

EPowerSourceType ASolarPanel::GetPowerSourceType_Implementation() const
{
	return EPowerSourceType::Solar;
}

void ASolarPanel::FindPlanetActor()
{
	if (!GetWorld())
	{
		return;
	}

	// Find planet actor by class name
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Planet")))
		{
			PlanetActor = Actor;
			break;
		}
	}
}

void ASolarPanel::UpdateVisuals()
{
	// Update sun direction indicator
	if (SunDirectionIndicator)
	{
		FVector SunDir = GetSunDirection();
		SunDirectionIndicator->SetWorldRotation(SunDir.Rotation());
	}

	// Could add material parameter updates here for visual feedback
	// e.g., emissive glow based on power output
}
