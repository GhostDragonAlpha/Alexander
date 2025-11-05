// Copyright Epic Games, Inc. All Rights Reserved.

#include "AlexanderAtmosphericFogComponent.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "PlanetAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"             // For UCameraComponent
#include "Camera/PlayerCameraManager.h"         // For APlayerCameraManager

UAlexanderAtmosphericFogComponent::UAlexanderAtmosphericFogComponent()
	: HeightFogComponent(nullptr)
	, AtmosphereComponent(nullptr)
	, bUseExponentialHeightFog(true)
	, bAutoUpdateFog(true)
	, UpdateFrequency(0.1f)
	, UpdateTimer(0.0f)
	, CachedFogDensity(0.0f)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAlexanderAtmosphericFogComponent::BeginPlay()
{
	Super::BeginPlay();

	// Create Exponential Height Fog component if enabled
	if (bUseExponentialHeightFog && !HeightFogComponent)
	{
		HeightFogComponent = NewObject<UExponentialHeightFogComponent>(GetOwner(), TEXT("ExponentialHeightFog"));
		if (HeightFogComponent)
		{
			HeightFogComponent->RegisterComponent();
			HeightFogComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
			UE_LOG(LogTemp, Log, TEXT("Exponential Height Fog component created and attached"));
		}
	}

	// Find atmosphere component if not set
	if (!AtmosphereComponent && GetOwner())
	{
		AtmosphereComponent = GetOwner()->FindComponentByClass<UPlanetAtmosphereComponent>();
		if (AtmosphereComponent)
		{
			UE_LOG(LogTemp, Log, TEXT("Found PlanetAtmosphereComponent for integration"));
		}
	}

	// Sync with Height Fog
	if (bUseExponentialHeightFog && HeightFogComponent)
	{
		SyncWithHeightFog();
	}

	// Apply Lumen fog parameters
	ApplyLumenFogParameters();
}

void UAlexanderAtmosphericFogComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Auto-update fog based on viewer position
	if (bAutoUpdateFog)
	{
		UpdateTimer += DeltaTime;
		if (UpdateTimer >= UpdateFrequency)
		{
			UpdateTimer = 0.0f;

			// Get player camera position
			APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
			if (CameraManager)
			{
				FVector ViewerPosition = CameraManager->GetCameraLocation();
				UpdateFogForViewerPosition(ViewerPosition);
			}
		}
	}

	// Update Lumen fog contribution
	UpdateLumenFogContribution(DeltaTime);
}

void UAlexanderAtmosphericFogComponent::InitializeFog(float InPlanetRadius, const FAtmosphericFogConfig& Config)
{
	PlanetRadius = InPlanetRadius;
	FogSettings = Config;
	
	UE_LOG(LogTemp, Log, TEXT("Atmospheric fog initialized: Radius=%.1f km, BaseDensity=%.3f, HeightFalloff=%.3f"), 
		PlanetRadius, FogSettings.BaseFogDensity, FogSettings.HeightFalloff);

	// Sync with Height Fog if available
	if (bUseExponentialHeightFog && HeightFogComponent)
	{
		SyncWithHeightFog();
	}

	// Apply Lumen parameters
	ApplyLumenFogParameters();
}

void UAlexanderAtmosphericFogComponent::ApplyFogSettings(const FAtmosphericFogConfig& NewSettings)
{
	FogSettings = NewSettings;
	
	UE_LOG(LogTemp, Log, TEXT("Fog settings applied: BaseDensity=%.3f, HeightFalloff=%.3f"), 
		FogSettings.BaseFogDensity, FogSettings.HeightFalloff);

	// Sync with Height Fog
	if (bUseExponentialHeightFog && HeightFogComponent)
	{
		SyncWithHeightFog();
	}

	// Apply Lumen parameters
	ApplyLumenFogParameters();
}

float UAlexanderAtmosphericFogComponent::GetFogDensityAtAltitude(float AltitudeKm) const
{
	return CalculateExponentialDensity(AltitudeKm);
}

FLinearColor UAlexanderAtmosphericFogComponent::GetFogColorAtAltitude(float AltitudeKm) const
{
	// Get sun direction (simplified - assumes directional light is the sun)
	FVector SunDirection = FVector(0.0f, 0.707f, 0.707f); // Default 45-degree angle
	
	// Try to find directional light in the world
	if (GetWorld())
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
		if (FoundActors.Num() > 0)
		{
			ADirectionalLight* DirectionalLight = Cast<ADirectionalLight>(FoundActors[0]);
			if (DirectionalLight)
			{
				SunDirection = DirectionalLight->GetActorForwardVector();
			}
		}
	}

	return CalculateFogColor(AltitudeKm, SunDirection);
}

FLinearColor UAlexanderAtmosphericFogComponent::CalculateAtmosphericPerspective(FVector ViewPosition, FVector TargetPosition, FLinearColor BaseColor) const
{
	// Calculate distance between viewer and target
	float Distance = (TargetPosition - ViewPosition).Size() / 100000.0f; // Convert cm to km
	
	// Calculate perspective blend factor based on distance
	float PerspectiveFactor = FMath::Clamp(Distance / FogSettings.PerspectiveDistance, 0.0f, 1.0f);
	PerspectiveFactor = FMath::Pow(PerspectiveFactor, 2.0f); // Quadratic falloff for smoother transition
	
	// Apply atmospheric perspective intensity
	PerspectiveFactor *= FogSettings.AtmosphericPerspectiveIntensity;
	
	// Get altitude for fog color calculation
	float ViewerAltitude = GetViewerAltitude(ViewPosition);
	FLinearColor FogColor = GetFogColorAtAltitude(ViewerAltitude);
	
	// Blend base color with fog color based on distance
	FLinearColor Result = FMath::Lerp(BaseColor, FogColor, PerspectiveFactor);
	
	// Desaturate distant objects slightly
	float Desaturation = PerspectiveFactor * 0.3f;
	float Luminance = Result.R * 0.299f + Result.G * 0.587f + Result.B * 0.114f;
	Result = FMath::Lerp(Result, FLinearColor(Luminance, Luminance, Luminance, Result.A), Desaturation);
	
	return Result;
}

void UAlexanderAtmosphericFogComponent::UpdateFogForViewerPosition(FVector ViewerPosition)
{
	// Cache viewer position
	CachedViewerPosition = ViewerPosition;
	
	// Calculate viewer altitude
	float ViewerAltitude = GetViewerAltitude(ViewerPosition);
	
	// Calculate fog density at viewer altitude
	CachedFogDensity = GetFogDensityAtAltitude(ViewerAltitude);
	
	// Update height fog component if available
	if (HeightFogComponent)
	{
		// Adjust fog density based on altitude
		HeightFogComponent->FogDensity = FogSettings.BaseFogDensity * CachedFogDensity;
		
		// Adjust fog color based on altitude
		FLinearColor FogColor = GetFogColorAtAltitude(ViewerAltitude);
		// UE5.6: Use setter method instead of direct property access
		HeightFogComponent->SetFogInscatteringColor(FogColor);

		// Update fog height to follow viewer
		FVector PlanetCenter = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
		float SurfaceHeight = PlanetCenter.Z + PlanetRadius * 100000.0f; // Convert km to cm
		HeightFogComponent->SetWorldLocation(FVector(PlanetCenter.X, PlanetCenter.Y, SurfaceHeight));
		
		// Mark for update
		HeightFogComponent->MarkRenderStateDirty();
	}
}

// ============================================================================
// EXPONENTIAL HEIGHT FOG INTEGRATION
// ============================================================================

void UAlexanderAtmosphericFogComponent::SyncWithHeightFog()
{
	if (!HeightFogComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot sync: Height Fog component not available"));
		return;
	}

	// Set fog density
	HeightFogComponent->FogDensity = FogSettings.BaseFogDensity;
	
	// Set fog height falloff
	HeightFogComponent->FogHeightFalloff = FogSettings.HeightFalloff;

	// Set fog inscattering color - UE5.6: Use setter method instead of direct property access
	HeightFogComponent->SetFogInscatteringColor(FogSettings.InscatteringColor);

	// Set directional inscattering - UE5.6: Use setter methods instead of direct property access
	// Note: DirectionalInscatteringIntensity is now controlled through the color's brightness and start distance
	HeightFogComponent->SetDirectionalInscatteringExponent(FogSettings.DirectionalInscatteringExponent);
	HeightFogComponent->SetDirectionalInscatteringStartDistance(0.0f);
	// Apply intensity by scaling the inscattering color
	FLinearColor ScaledInscatteringColor = FogSettings.InscatteringColor * FogSettings.DirectionalInscatteringIntensity;
	HeightFogComponent->SetDirectionalInscatteringColor(ScaledInscatteringColor);

	// Set fog start distance
	HeightFogComponent->StartDistance = FogSettings.FogStartDistance;
	
	// Set fog cutoff distance
	HeightFogComponent->FogCutoffDistance = FogSettings.FogCutoffDistance;
	
	// Apply volumetric fog settings
	if (FogSettings.bEnableVolumetricFog)
	{
		ApplyVolumetricFogSettings();
	}
	
	// Position fog at planet surface
	if (GetOwner())
	{
		FVector PlanetCenter = GetOwner()->GetActorLocation();
		float SurfaceHeight = PlanetCenter.Z + PlanetRadius * 100000.0f; // Convert km to cm
		HeightFogComponent->SetWorldLocation(FVector(PlanetCenter.X, PlanetCenter.Y, SurfaceHeight));
	}
	
	// Mark for update
	HeightFogComponent->MarkRenderStateDirty();
	
	UE_LOG(LogTemp, Log, TEXT("Synced fog settings with Exponential Height Fog component"));
}

void UAlexanderAtmosphericFogComponent::ApplyVolumetricFogSettings()
{
	if (!HeightFogComponent)
	{
		return;
	}

	// Enable volumetric fog
	HeightFogComponent->bEnableVolumetricFog = FogSettings.bEnableVolumetricFog;
	
	if (FogSettings.bEnableVolumetricFog)
	{
		// Set volumetric fog scattering distribution
		HeightFogComponent->VolumetricFogScatteringDistribution = FogSettings.VolumetricFogScatteringDistribution;

		// Set volumetric fog albedo - UE5.6: Use setter method instead of direct property access
		HeightFogComponent->SetVolumetricFogAlbedo(FogSettings.VolumetricFogAlbedo.ToFColor(true));

		// Set volumetric fog extinction scale
		HeightFogComponent->VolumetricFogExtinctionScale = FogSettings.VolumetricFogExtinctionScale;
		
		// Set volumetric fog distance (use perspective distance)
		HeightFogComponent->VolumetricFogDistance = FogSettings.PerspectiveDistance * 1000.0f; // Convert km to meters
		
		UE_LOG(LogTemp, Log, TEXT("Volumetric fog enabled: ScatteringDist=%.2f, ExtinctionScale=%.2f"),
			FogSettings.VolumetricFogScatteringDistribution, FogSettings.VolumetricFogExtinctionScale);
	}
}

// ============================================================================
// LUMEN INTEGRATION
// ============================================================================

void UAlexanderAtmosphericFogComponent::ApplyLumenFogParameters()
{
	// Lumen fog integration is handled through the exponential height fog component
	// and material parameter collections
	
	if (!HeightFogComponent)
	{
		return;
	}

	// Ensure volumetric fog is enabled for Lumen integration
	if (FogSettings.bEnableVolumetricFog)
	{
		// Lumen uses volumetric fog for atmospheric scattering
		// The height fog component automatically integrates with Lumen
		
		// Set fog to affect Lumen global illumination
		// This is done through the volumetric fog settings
		HeightFogComponent->bEnableVolumetricFog = true;
		
		UE_LOG(LogTemp, Log, TEXT("Lumen fog parameters applied"));
	}
}

void UAlexanderAtmosphericFogComponent::UpdateLumenFogContribution(float DeltaTime)
{
	// Update fog contribution to Lumen lighting
	// This is automatically handled by Unreal's rendering system
	// when volumetric fog is enabled
	
	// We can adjust fog parameters dynamically based on atmospheric conditions
	if (AtmosphereComponent && HeightFogComponent && FogSettings.bEnableVolumetricFog)
	{
		// Get current viewer altitude
		float ViewerAltitude = GetViewerAltitude(CachedViewerPosition);
		
		// Adjust volumetric fog density based on altitude
		float AltitudeFactor = CalculateExponentialDensity(ViewerAltitude);
		
		// Modulate fog extinction based on altitude
		float BaseExtinction = FogSettings.VolumetricFogExtinctionScale;
		HeightFogComponent->VolumetricFogExtinctionScale = BaseExtinction * AltitudeFactor;
	}
}

// ============================================================================
// PRIVATE METHODS
// ============================================================================

float UAlexanderAtmosphericFogComponent::CalculateExponentialDensity(float AltitudeKm) const
{
	// Exponential fog density falloff with altitude
	// Density = BaseDensity * exp(-Altitude * HeightFalloff)
	
	// Clamp altitude to valid range
	float ClampedAltitude = FMath::Clamp(AltitudeKm, 0.0f, FogSettings.MaxFogAltitude);
	
	// Calculate exponential falloff
	float Density = FMath::Exp(-ClampedAltitude * FogSettings.HeightFalloff);
	
	// Return clamped density
	return FMath::Clamp(Density, 0.0f, 1.0f);
}

FLinearColor UAlexanderAtmosphericFogComponent::CalculateFogColor(float AltitudeKm, FVector SunDirection) const
{
	// Interpolate between ground and high altitude fog colors based on altitude
	float AltitudeFactor = FMath::Clamp(AltitudeKm / FogSettings.MaxFogAltitude, 0.0f, 1.0f);
	FLinearColor BaseColor = FMath::Lerp(FogSettings.GroundFogColor, FogSettings.HighAltitudeFogColor, AltitudeFactor);
	
	// Add sun influence to fog color
	// When looking toward the sun, fog appears brighter and warmer
	float SunElevation = FMath::Max(0.0f, SunDirection.Z);
	FLinearColor SunInfluence = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f) * SunElevation * 0.3f;
	
	// Combine base color with sun influence
	FLinearColor FinalColor = BaseColor + SunInfluence;
	
	// Clamp to valid color range
	FinalColor.R = FMath::Clamp(FinalColor.R, 0.0f, 1.0f);
	FinalColor.G = FMath::Clamp(FinalColor.G, 0.0f, 1.0f);
	FinalColor.B = FMath::Clamp(FinalColor.B, 0.0f, 1.0f);
	FinalColor.A = 1.0f;
	
	return FinalColor;
}

float UAlexanderAtmosphericFogComponent::GetViewerAltitude(FVector ViewerPosition) const
{
	// Calculate altitude relative to planet surface
	FVector PlanetCenter = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	float DistanceFromCenter = (ViewerPosition - PlanetCenter).Size() / 100000.0f; // Convert cm to km
	float Altitude = DistanceFromCenter - PlanetRadius;
	
	return FMath::Max(0.0f, Altitude);
}
