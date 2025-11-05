#include "Sun.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProfilingDebugging/CpuProfilerTrace.h" // For performance profiling
#include "HAL/PlatformTime.h" // For performance timing

ASun::ASun()
{
	PrimaryActorTick.bCanEverTick = true;

	// Configure as root of solar system orbital hierarchy
	OrbitMode = EOrbitMode::Root;
	OrbitTarget = nullptr;
	Mass = 1.989e30f; // Mass of our Sun in kg (will be overridden by config if set)

	// Create root scene component
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootComp;

	// Create and setup visual mesh component
	SunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SunMesh"));
	SunMesh->SetupAttachment(RootComponent);

	// Load basic sphere mesh for sun visualization
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		SunMesh->SetStaticMesh(SphereMesh.Object);
	}

	// === SPECTACULAR MULTI-LAYER LIGHTING SETUP ===
	// Uses three light sources for realistic and dramatic illumination

	// Main sun light (primary illumination for entire solar system)
	SunLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("SunLight"));
	SunLight->SetupAttachment(RootComponent);
	SunLight->SetIntensity(50000.0f); // Much brighter than default!
	SunLight->SetAttenuationRadius(200000.0f); // Illuminate entire solar system
	SunLight->SetSourceRadius(500.0f); // Large soft shadows
	SunLight->SetSoftSourceRadius(800.0f);
	SunLight->bUseInverseSquaredFalloff = false; // Better control
	SunLight->SetLightFalloffExponent(2.0f);

	// Core light (intense inner core for close-range drama)
	CoreLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CoreLight"));
	CoreLight->SetupAttachment(RootComponent);
	CoreLight->SetIntensity(100000.0f); // Extremely intense
	CoreLight->SetAttenuationRadius(50000.0f);
	CoreLight->SetSourceRadius(200.0f);
	CoreLight->SetLightColor(FLinearColor(1.0f, 0.9f, 0.7f)); // Bright white-yellow

	// Corona light (outer glow)
	CoronaLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CoronaLight"));
	CoronaLight->SetupAttachment(RootComponent);
	CoronaLight->SetIntensity(25000.0f);
	CoronaLight->SetAttenuationRadius(150000.0f);
	CoronaLight->SetSourceRadius(1000.0f); // Very soft
	CoronaLight->SetSoftSourceRadius(1500.0f);
	CoronaLight->SetLightColor(FLinearColor(1.0f, 0.6f, 0.2f)); // Orange corona

	// === NIAGARA EFFECTS ===

	// Corona effect (always visible glow)
	CoronaEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("CoronaEffect"));
	CoronaEffect->SetupAttachment(RootComponent);
	CoronaEffect->SetAutoActivate(true);

	// Solar flares (bursts from surface)
	SolarFlares = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SolarFlares"));
	SolarFlares->SetupAttachment(RootComponent);
	SolarFlares->SetAutoActivate(true);

	// Electromagnetic storms (intense electrical activity)
	ElectromagneticStorm = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ElectromagneticStorm"));
	ElectromagneticStorm->SetupAttachment(RootComponent);
	ElectromagneticStorm->SetAutoActivate(true);

	// Storm management component (handles electromagnetic storm logic)
	StormComponent = CreateDefaultSubobject<USunStormComponent>(TEXT("StormComponent"));

	// Default properties
	SunScale = 3.0f; // Larger for more dramatic effect
	SunColor = FLinearColor(1.0f, 0.85f, 0.4f, 1.0f); // Warm golden
	EmissiveIntensity = 1000.0f; // Very bright emissive

	// Lighting intensities
	MainLightIntensity = 50000.0f;
	CoreLightIntensity = 100000.0f;
	CoronaLightIntensity = 25000.0f;
	LightAttenuationRadius = 200000.0f;

	// Storm properties
	bEnableElectromagneticStorms = true;
	StormIntensity = 1.0f;
	StormFrequency = 2.0f; // Storms every 2 seconds
	FlareIntensity = 1.5f;

	// Initialize timers
	StormTimer = 0.0f;
	FlickerTimer = 0.0f;
	RotationAngle = 0.0f;

	// Initialize performance tracking
	TotalTickTime = 0.0f;
	TickCount = 0;
}

void ASun::BeginPlay()
{
	Super::BeginPlay();

	// Apply configuration values if available, otherwise use defaults
	if (SunConfig)
	{
		// Apply physical properties
		Mass = SunConfig->Mass;

		// Apply visual properties
		SunScale = SunConfig->SunScale;
		SunColor = SunConfig->SunColor;
		EmissiveIntensity = SunConfig->EmissiveIntensity;

		// Apply lighting properties
		MainLightIntensity = SunConfig->MainLightIntensity;
		CoreLightIntensity = SunConfig->CoreLightIntensity;
		CoronaLightIntensity = SunConfig->CoronaLightIntensity;
		LightAttenuationRadius = SunConfig->LightAttenuationRadius;

		// Apply storm properties
		bEnableElectromagneticStorms = SunConfig->bEnableElectromagneticStorms;
		StormIntensity = SunConfig->StormIntensity;
		StormFrequency = SunConfig->StormFrequency;
		FlareIntensity = SunConfig->FlareIntensity;
	}

	// Apply scale
	SunMesh->SetWorldScale3D(FVector(SunScale));

	// === APPLY SPECTACULAR LIGHTING ===

	// Main light
	SunLight->SetIntensity(MainLightIntensity);
	SunLight->SetLightColor(SunColor);
	SunLight->SetAttenuationRadius(LightAttenuationRadius);

	// Core light (intense white-yellow from config)
	CoreLight->SetIntensity(CoreLightIntensity);
	CoreLight->SetLightColor(SunConfig ? SunConfig->CoreLightColor : FLinearColor(1.0f, 0.95f, 0.8f));

	// Corona light (soft orange glow from config)
	CoronaLight->SetIntensity(CoronaLightIntensity);
	CoronaLight->SetLightColor(SunConfig ? SunConfig->CoronaLightColor : SunColor * FLinearColor(1.0f, 0.7f, 0.3f));

	// Create and apply emissive material
	if (SunMesh->GetStaticMesh())
	{
		UMaterialInterface* BaseMaterial = SunMesh->GetMaterial(0);
		if (BaseMaterial)
		{
			UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			if (DynMaterial)
			{
				DynMaterial->SetVectorParameterValue(FName("EmissiveColor"), SunColor * EmissiveIntensity);
				DynMaterial->SetScalarParameterValue(FName("EmissiveIntensity"), EmissiveIntensity);
				SunMesh->SetMaterial(0, DynMaterial);
			}
		}
	}

	// Set Niagara parameters (if systems are assigned in editor)
	if (CoronaEffect && CoronaEffect->GetAsset())
	{
		CoronaEffect->SetFloatParameter(FName("Intensity"), 1.0f);
		CoronaEffect->SetColorParameter(FName("Color"), SunColor);
	}

	if (SolarFlares && SolarFlares->GetAsset())
	{
		SolarFlares->SetFloatParameter(FName("FlareIntensity"), FlareIntensity);
		SolarFlares->SetColorParameter(FName("FlareColor"), SunColor);
	}

	if (ElectromagneticStorm && ElectromagneticStorm->GetAsset())
	{
		ElectromagneticStorm->SetFloatParameter(FName("StormIntensity"), StormIntensity);
		ElectromagneticStorm->SetFloatParameter(FName("StormFrequency"), StormFrequency);
	}

	// Initialize storm component with references and configuration
	if (StormComponent)
	{
		StormComponent->InitializeStorm(this, ElectromagneticStorm, CoreLight, SolarFlares, bEnableElectromagneticStorms, StormIntensity, StormFrequency);
	}
}

void ASun::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ASun::Tick);

	// Performance tracking
	float StartTime = FPlatformTime::Seconds();

	Super::Tick(DeltaTime);

	// Rotate the sun slowly for visual interest
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(SunRotation);
		float RotationSpeed = SunConfig ? SunConfig->RotationSpeed : 5.0f;
		RotationAngle += DeltaTime * RotationSpeed;
		FRotator NewRotation = GetActorRotation();
		NewRotation.Yaw = RotationAngle;
		SetActorRotation(NewRotation);
	}

	// Update electromagnetic storms if enabled
	if (bEnableElectromagneticStorms && StormComponent)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(StormUpdate);
		StormComponent->UpdateStorm(DeltaTime);
	}

	// Always update subtle light flicker for realism
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(LightFlicker);
		UpdateLightFlicker(DeltaTime);
	}

	// Update performance stats
	float EndTime = FPlatformTime::Seconds();
	float TickDuration = EndTime - StartTime;
	TotalTickTime += TickDuration;
	TickCount++;

	// Log average performance every 100 ticks
	if (TickCount % 100 == 0 && TickCount > 0)
	{
		float AverageTickTime = TotalTickTime / TickCount;
		UE_LOG(LogTemp, Log, TEXT("Sun Tick Performance - Average: %.6f ms, Last: %.6f ms"), AverageTickTime * 1000.0f, TickDuration * 1000.0f);
	}
}



void ASun::UpdateLightFlicker(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ASun::UpdateLightFlicker);

	// Increment flicker timer for animation
	FlickerTimer += DeltaTime * 10.0f;

	// Create organic flicker using multiple sine waves at different frequencies
	// This simulates the natural variation of solar surface activity
	float Flicker1 = FMath::Sin(FlickerTimer * 1.3f) * 0.02f;  // Primary flicker
	float Flicker2 = FMath::Sin(FlickerTimer * 2.7f) * 0.015f; // Secondary variation
	float Flicker3 = FMath::Sin(FlickerTimer * 4.1f) * 0.01f;  // Subtle high-frequency noise
	float TotalFlicker = 1.0f + Flicker1 + Flicker2 + Flicker3;

	// Apply flicker to main sun light
	if (SunLight)
	{
		SunLight->SetIntensity(MainLightIntensity * TotalFlicker);
	}

	// Apply enhanced flicker to corona light for more visible effect
	if (CoronaLight)
	{
		CoronaLight->SetIntensity(CoronaLightIntensity * (1.0f + Flicker1 * 2.0f));
	}

	// Occasional subtle color temperature shifts for realism
	if (FMath::Fmod(FlickerTimer, 2.0f) < 0.01f)
	{
		FLinearColor TempShift = SunColor;
		TempShift.R += FMath::RandRange(-0.02f, 0.02f); // Slight red variation
		TempShift.G += FMath::RandRange(-0.015f, 0.015f); // Subtle green shift

		if (SunLight)
		{
			SunLight->SetLightColor(TempShift);
		}
	}
}
