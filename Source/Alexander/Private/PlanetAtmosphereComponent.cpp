// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlanetAtmosphereComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "AtmosphereLookupTable.h"

UPlanetAtmosphereComponent::UPlanetAtmosphereComponent()
	: SkyAtmosphereComponent(nullptr)
	, LookupTables(nullptr)
	, bUseSkyAtmosphere(true)
	, bUseLookupTables(true)
	, bUseRayMarching(false)
	, RayMarchingSamples(16)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlanetAtmosphereComponent::BeginPlay()
{
	Super::BeginPlay();

	// Create Sky Atmosphere component if enabled
	if (bUseSkyAtmosphere && !SkyAtmosphereComponent)
	{
		SkyAtmosphereComponent = NewObject<USkyAtmosphereComponent>(GetOwner(), TEXT("SkyAtmosphere"));
		if (SkyAtmosphereComponent)
		{
			SkyAtmosphereComponent->RegisterComponent();
			SkyAtmosphereComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
			UE_LOG(LogTemp, Log, TEXT("Sky Atmosphere component created and attached"));
		}
	}

	// Create lookup tables if enabled
	if (bUseLookupTables && !LookupTables)
	{
		LookupTables = NewObject<UAtmosphereLookupTable>(this);
		GenerateLookupTables();
	}

	// Sync with Sky Atmosphere
	if (bUseSkyAtmosphere && SkyAtmosphereComponent)
	{
		SyncWithSkyAtmosphere();
	}
}

void UPlanetAtmosphereComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update custom shader parameters if needed
	if (bUseSkyAtmosphere && SkyAtmosphereComponent)
	{
		ApplyCustomShaderParameters();
	}
}

void UPlanetAtmosphereComponent::InitializeAtmosphere(float InPlanetRadius, const FAtmosphereConfig& Config)
{
	PlanetRadius = InPlanetRadius;
	AtmosphereSettings = Config;
	
	UE_LOG(LogTemp, Log, TEXT("Atmosphere initialized: Radius=%.1f km, AtmosphereHeight=%.1f km"), 
		PlanetRadius, AtmosphereSettings.AtmosphereHeight);

	// Sync with Sky Atmosphere if available
	if (bUseSkyAtmosphere && SkyAtmosphereComponent)
	{
		SyncWithSkyAtmosphere();
	}

	// Regenerate lookup tables with new parameters
	if (bUseLookupTables && LookupTables)
	{
		GenerateLookupTables();
	}
}

FLinearColor UPlanetAtmosphereComponent::CalculateAtmosphericColor(FVector ViewDirection, FVector SunDirection)
{
	// Normalize input vectors
	ViewDirection.Normalize();
	SunDirection.Normalize();

	// Get viewer position relative to planet center
	FVector PlanetCenter = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	FVector ViewerPosition = GetComponentLocation();
	FVector RelativePosition = ViewerPosition - PlanetCenter;
	float ViewHeight = RelativePosition.Size() / 100000.0f; // Convert cm to km

	// Calculate Rayleigh and Mie scattering contributions
	FLinearColor RayleighColor = CalculateRayleighScattering(ViewDirection, SunDirection, ViewHeight);
	FLinearColor MieColor = CalculateMieScattering(ViewDirection, SunDirection, ViewHeight);

	// Combine scattering contributions
	FLinearColor FinalColor = RayleighColor + MieColor;

	// Apply ground albedo contribution if looking down
	float GroundDot = FVector::DotProduct(ViewDirection, -RelativePosition.GetSafeNormal());
	if (GroundDot > 0.0f && ViewHeight <= PlanetRadius + AtmosphereSettings.AtmosphereHeight)
	{
		FLinearColor GroundContribution = AtmosphereSettings.GroundAlbedo * GroundDot * 0.3f;
		FinalColor += GroundContribution;
	}

	// Clamp to valid color range
	FinalColor.R = FMath::Clamp(FinalColor.R, 0.0f, 1.0f);
	FinalColor.G = FMath::Clamp(FinalColor.G, 0.0f, 1.0f);
	FinalColor.B = FMath::Clamp(FinalColor.B, 0.0f, 1.0f);
	FinalColor.A = 1.0f;

	return FinalColor;
}

FLinearColor UPlanetAtmosphereComponent::CalculateRayleighScattering(FVector ViewDirection, FVector SunDirection, float ViewHeight)
{
	// Rayleigh scattering is responsible for the blue color of the sky
	// It scatters shorter wavelengths (blue) more than longer wavelengths (red)

	// Calculate phase function
	float CosTheta = FVector::DotProduct(ViewDirection, SunDirection);
	float Phase = RayleighPhase(CosTheta);

	// Calculate optical depth based on view height
	float HeightFactor = FMath::Exp(-(ViewHeight - PlanetRadius) / AtmosphereSettings.RayleighScaleHeight);
	
	// Apply scattering coefficient (wavelength dependent)
	FLinearColor ScatteredLight = AtmosphereSettings.RayleighScatteringCoefficient * Phase * HeightFactor;

	// Attenuate based on sun angle (more scattering at sunset/sunrise)
	float SunElevation = FMath::Max(0.0f, SunDirection.Z);
	float Attenuation = FMath::Lerp(0.3f, 1.0f, SunElevation);
	ScatteredLight *= Attenuation;

	return ScatteredLight;
}

FLinearColor UPlanetAtmosphereComponent::CalculateMieScattering(FVector ViewDirection, FVector SunDirection, float ViewHeight)
{
	// Mie scattering is responsible for halos around the sun and hazy appearance
	// It's less wavelength dependent than Rayleigh scattering

	// Calculate phase function with anisotropy
	float CosTheta = FVector::DotProduct(ViewDirection, SunDirection);
	float Phase = MiePhase(CosTheta, AtmosphereSettings.MieAnisotropy);

	// Calculate optical depth based on view height
	float HeightFactor = FMath::Exp(-(ViewHeight - PlanetRadius) / AtmosphereSettings.MieScaleHeight);
	
	// Apply scattering coefficient
	FLinearColor ScatteredLight = AtmosphereSettings.MieScatteringCoefficient * Phase * HeightFactor;

	// Mie scattering is stronger near the sun
	float SunProximity = FMath::Max(0.0f, CosTheta);
	ScatteredLight *= FMath::Pow(SunProximity, 2.0f);

	return ScatteredLight;
}

float UPlanetAtmosphereComponent::RayleighPhase(float CosTheta)
{
	// Rayleigh phase function: (3/16π)(1 + cos²θ)
	// Simplified for real-time use
	const float ThreeOver16Pi = 3.0f / (16.0f * PI);
	return ThreeOver16Pi * (1.0f + CosTheta * CosTheta);
}

float UPlanetAtmosphereComponent::MiePhase(float CosTheta, float G)
{
	// Henyey-Greenstein phase function for Mie scattering
	// This creates the forward-scattering halo around the sun
	float G2 = G * G;
	float Numerator = 1.0f - G2;
	float Denominator = FMath::Pow(1.0f + G2 - 2.0f * G * CosTheta, 1.5f);
	
	// Avoid division by zero
	if (FMath::Abs(Denominator) < SMALL_NUMBER)
	{
		return 0.0f;
	}

	return (1.0f / (4.0f * PI)) * (Numerator / Denominator);
}

float UPlanetAtmosphereComponent::CalculateOpticalDepth(FVector RayOrigin, FVector RayDirection, float RayLength, float ScaleHeight)
{
	// Calculate optical depth along a ray through the atmosphere
	// This determines how much light is scattered/absorbed
	
	const int32 SampleCount = 8; // Number of samples along the ray
	float StepSize = RayLength / SampleCount;
	float OpticalDepth = 0.0f;

	FVector PlanetCenter = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;

	for (int32 i = 0; i < SampleCount; ++i)
	{
		// Sample position along the ray
		FVector SamplePos = RayOrigin + RayDirection * (i + 0.5f) * StepSize;
		
		// Calculate height at sample position
		float SampleHeight = (SamplePos - PlanetCenter).Size() / 100000.0f; // Convert cm to km
		
		// Calculate density at this height using exponential falloff
		float Density = FMath::Exp(-(SampleHeight - PlanetRadius) / ScaleHeight);
		
		// Accumulate optical depth
		OpticalDepth += Density * StepSize;
	}

	return OpticalDepth;
}

bool UPlanetAtmosphereComponent::RaySphereIntersection(FVector RayOrigin, FVector RayDirection, FVector SphereCenter, float SphereRadius, float& T0, float& T1)
{
	// Ray-sphere intersection for atmosphere boundary calculations
	FVector L = SphereCenter - RayOrigin;
	float Tca = FVector::DotProduct(L, RayDirection);
	float D2 = FVector::DotProduct(L, L) - Tca * Tca;
	float Radius2 = SphereRadius * SphereRadius;

	if (D2 > Radius2)
	{
		return false; // Ray misses sphere
	}

	float Thc = FMath::Sqrt(Radius2 - D2);
	T0 = Tca - Thc;
	T1 = Tca + Thc;

	return true;
}

float UPlanetAtmosphereComponent::GetFogDensityAtAltitude(float Altitude)
{
	// Exponential fog density falloff with altitude
	float NormalizedAltitude = Altitude / AtmosphereSettings.AtmosphereHeight;
	float Density = AtmosphereSettings.FogDensity * FMath::Exp(-NormalizedAltitude * AtmosphereSettings.FogHeightFalloff);
	return FMath::Clamp(Density, 0.0f, 1.0f);
}

void UPlanetAtmosphereComponent::ApplyAtmosphereSettings(const FAtmosphereConfig& NewSettings)
{
	AtmosphereSettings = NewSettings;
	UE_LOG(LogTemp, Log, TEXT("Atmosphere settings applied: Rayleigh Scale=%.1f km, Mie Scale=%.1f km"), 
		AtmosphereSettings.RayleighScaleHeight, AtmosphereSettings.MieScaleHeight);
}


// ============================================================================
// RAY MARCHING IMPLEMENTATION
// ============================================================================

FLinearColor UPlanetAtmosphereComponent::CalculateAtmosphericColorRayMarched(FVector ViewDirection, FVector SunDirection, FVector ViewPosition)
{
	// Normalize input vectors
	ViewDirection.Normalize();
	SunDirection.Normalize();

	// Get planet center
	FVector PlanetCenter = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	
	// Calculate ray parameters
	FVector RayOrigin = ViewPosition;
	FVector RayDir = ViewDirection;
	
	// Find intersection with atmosphere
	float AtmosphereRadius = (PlanetRadius + AtmosphereSettings.AtmosphereHeight) * 100000.0f; // Convert km to cm
	float PlanetRadiusCm = PlanetRadius * 100000.0f;
	
	float T0, T1;
	if (!RaySphereIntersection(RayOrigin, RayDir, PlanetCenter, AtmosphereRadius, T0, T1))
	{
		return FLinearColor::Black; // No intersection with atmosphere
	}
	
	// Clamp ray to atmosphere bounds
	T0 = FMath::Max(T0, 0.0f);
	float RayLength = T1 - T0;
	
	// Check if ray hits planet surface
	float SurfaceT0, SurfaceT1;
	bool HitsSurface = RaySphereIntersection(RayOrigin, RayDir, PlanetCenter, PlanetRadiusCm, SurfaceT0, SurfaceT1);
	if (HitsSurface && SurfaceT0 > 0.0f)
	{
		RayLength = FMath::Min(RayLength, SurfaceT0 - T0);
	}
	
	// Ray marching through atmosphere
	float StepSize = RayLength / float(RayMarchingSamples);
	
	FLinearColor RayleighAccum = FLinearColor::Black;
	FLinearColor MieAccum = FLinearColor::Black;
	
	float CosTheta = FVector::DotProduct(RayDir, SunDirection);
	float RayleighPhaseValue = RayleighPhase(CosTheta);
	float MiePhaseValue = MiePhase(CosTheta, AtmosphereSettings.MieAnisotropy);
	
	for (int32 i = 0; i < RayMarchingSamples; i++)
	{
		FVector SamplePos = RayOrigin + RayDir * (T0 + (float(i) + 0.5f) * StepSize);
		float SampleHeight = ((SamplePos - PlanetCenter).Size() - PlanetRadiusCm) / 100000.0f; // Convert to km
		
		// Calculate density at sample point
		float RayleighDensity = FMath::Exp(-SampleHeight / AtmosphereSettings.RayleighScaleHeight);
		float MieDensity = FMath::Exp(-SampleHeight / AtmosphereSettings.MieScaleHeight);
		
		// Calculate optical depth to sun (simplified)
		float SunOpticalDepth = CalculateOpticalDepth(
			SamplePos, 
			SunDirection, 
			AtmosphereRadius - (SamplePos - PlanetCenter).Size(),
			PlanetCenter, 
			PlanetRadius, 
			AtmosphereSettings.RayleighScaleHeight);
		
		// Calculate attenuation
		FLinearColor Attenuation = FLinearColor(
			FMath::Exp(-SunOpticalDepth * AtmosphereSettings.RayleighScatteringCoefficient.R),
			FMath::Exp(-SunOpticalDepth * AtmosphereSettings.RayleighScatteringCoefficient.G),
			FMath::Exp(-SunOpticalDepth * AtmosphereSettings.RayleighScatteringCoefficient.B));
		
		// Accumulate scattering
		RayleighAccum += AtmosphereSettings.RayleighScatteringCoefficient * RayleighDensity * Attenuation * StepSize;
		MieAccum += AtmosphereSettings.MieScatteringCoefficient * MieDensity * Attenuation * StepSize;
	}
	
	// Apply phase functions
	FLinearColor RayleighScattering = RayleighAccum * RayleighPhaseValue;
	FLinearColor MieScattering = MieAccum * MiePhaseValue;
	
	// Combine scattering contributions
	FLinearColor FinalColor = RayleighScattering + MieScattering;
	
	// Clamp to valid color range
	FinalColor.R = FMath::Clamp(FinalColor.R, 0.0f, 1.0f);
	FinalColor.G = FMath::Clamp(FinalColor.G, 0.0f, 1.0f);
	FinalColor.B = FMath::Clamp(FinalColor.B, 0.0f, 1.0f);
	FinalColor.A = 1.0f;
	
	return FinalColor;
}

// ============================================================================
// SKY ATMOSPHERE INTEGRATION
// ============================================================================

void UPlanetAtmosphereComponent::SyncWithSkyAtmosphere()
{
	if (!SkyAtmosphereComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot sync: Sky Atmosphere component not available"));
		return;
	}

	// Convert our atmosphere settings to Sky Atmosphere parameters
	// Note: Sky Atmosphere uses different units and parameterization
	
	// Bottom radius (planet surface)
	SkyAtmosphereComponent->BottomRadius = PlanetRadius * 10.0f; // Convert km to Sky Atmosphere units
	
	// Atmosphere height
	SkyAtmosphereComponent->AtmosphereHeight = AtmosphereSettings.AtmosphereHeight * 10.0f;
	
	// Rayleigh scattering
	FLinearColor RayleighScattering = AtmosphereSettings.RayleighScatteringCoefficient * 100.0f;
	SkyAtmosphereComponent->RayleighScattering = FLinearColor(
		RayleighScattering.R,
		RayleighScattering.G,
		RayleighScattering.B);
	SkyAtmosphereComponent->RayleighScaleHeight = AtmosphereSettings.RayleighScaleHeight;
	
	// Mie scattering
	FLinearColor MieScattering = AtmosphereSettings.MieScatteringCoefficient * 100.0f;
	SkyAtmosphereComponent->MieScattering = FLinearColor(
		MieScattering.R,
		MieScattering.G,
		MieScattering.B);
	SkyAtmosphereComponent->MieScaleHeight = AtmosphereSettings.MieScaleHeight;
	SkyAtmosphereComponent->MieAnisotropy = AtmosphereSettings.MieAnisotropy;
	
	// Ground albedo
	SkyAtmosphereComponent->GroundAlbedo = AtmosphereSettings.GroundAlbedo;
	
	// Mark for update
	SkyAtmosphereComponent->MarkRenderStateDirty();
	
	UE_LOG(LogTemp, Log, TEXT("Synced atmosphere settings with Sky Atmosphere component"));
}

void UPlanetAtmosphereComponent::ApplyCustomShaderParameters()
{
	if (!SkyAtmosphereComponent)
	{
		return;
	}

	// Apply any custom shader parameters here
	// This can be used to pass additional data to custom atmospheric shaders
	
	// Example: Set fog parameters based on atmosphere
	float FogDensity = AtmosphereSettings.FogDensity;
	float FogHeightFalloff = AtmosphereSettings.FogHeightFalloff;
	
	// These would be applied to material parameter collections or post-process volumes
	// For now, just log that we're updating
	// UE_LOG(LogTemp, VeryVerbose, TEXT("Applying custom shader parameters"));
}

// ============================================================================
// LOOKUP TABLE MANAGEMENT
// ============================================================================

void UPlanetAtmosphereComponent::GenerateLookupTables()
{
	if (!LookupTables)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot generate lookup tables: LookupTables object not created"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Generating atmospheric lookup tables..."));

	// Generate transmittance LUT
	LookupTables->GenerateTransmittanceLUT(
		256, // Resolution
		PlanetRadius,
		AtmosphereSettings.AtmosphereHeight,
		AtmosphereSettings.RayleighScaleHeight,
		AtmosphereSettings.MieScaleHeight);

	// Generate single scattering LUT
	LookupTables->GenerateSingleScatteringLUT(
		256, // Resolution
		PlanetRadius,
		AtmosphereSettings.AtmosphereHeight,
		AtmosphereSettings.RayleighScatteringCoefficient,
		AtmosphereSettings.MieScatteringCoefficient);

	// Generate multiple scattering LUT
	LookupTables->GenerateMultipleScatteringLUT(
		64, // Resolution
		PlanetRadius,
		AtmosphereSettings.AtmosphereHeight);

	UE_LOG(LogTemp, Log, TEXT("Atmospheric lookup tables generated successfully"));
}

float UPlanetAtmosphereComponent::GetTransmittance(float ViewZenithAngle, float ViewHeight) const
{
	if (!LookupTables)
	{
		return 1.0f;
	}

	return LookupTables->SampleTransmittance(ViewZenithAngle, ViewHeight);
}

FLinearColor UPlanetAtmosphereComponent::GetScatteringFromLUT(float ViewZenithAngle, float SunZenithAngle, float ViewHeight) const
{
	if (!LookupTables)
	{
		return FLinearColor::Black;
	}

	return LookupTables->SampleScattering(ViewZenithAngle, SunZenithAngle, ViewHeight);
}
