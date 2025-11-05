// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "PlanetConfiguration.h"
#include "AlexanderAtmosphericFogComponent.generated.h"

/**
 * Atmospheric fog configuration structure
 */
USTRUCT(BlueprintType)
struct FAtmosphericFogConfig
{
	GENERATED_BODY()

	// Base fog density at sea level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	float BaseFogDensity = 0.02f;

	// How quickly fog density falls off with altitude (higher = faster falloff)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	float HeightFalloff = 0.2f;

	// Maximum altitude where fog is visible (km)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	float MaxFogAltitude = 10.0f;

	// Fog color at ground level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	FLinearColor GroundFogColor = FLinearColor(0.7f, 0.75f, 0.8f, 1.0f);

	// Fog color at high altitude
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	FLinearColor HighAltitudeFogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

	// Fog inscattering color (light scattered into view)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	FLinearColor InscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

	// Directional inscattering intensity (sun glow through fog)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	float DirectionalInscatteringIntensity = 0.5f;

	// Directional inscattering exponent (controls sun glow size)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	float DirectionalInscatteringExponent = 4.0f;

	// Start distance for fog (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	float FogStartDistance = 0.0f;

	// Fog cutoff distance (meters, 0 = infinite)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	float FogCutoffDistance = 0.0f;

	// Atmospheric perspective intensity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	float AtmosphericPerspectiveIntensity = 1.0f;

	// Distance at which atmospheric perspective is fully applied (km)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	float PerspectiveDistance = 50.0f;

	// Enable volumetric fog
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
	bool bEnableVolumetricFog = true;

	// Volumetric fog scattering distribution (0-1, higher = more forward scattering)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
	float VolumetricFogScatteringDistribution = 0.2f;

	// Volumetric fog albedo (color of scattered light)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
	FLinearColor VolumetricFogAlbedo = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);

	// Volumetric fog extinction scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
	float VolumetricFogExtinctionScale = 1.0f;
};

/**
 * Component for managing atmospheric fog effects
 * Provides altitude-based fog density, exponential height fog, and atmospheric perspective
 * Integrates with Unreal's Exponential Height Fog and Lumen lighting
 */
UCLASS(ClassGroup=(Planet), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UAlexanderAtmosphericFogComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAlexanderAtmosphericFogComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Fog configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	FAtmosphericFogConfig FogSettings;

	// Planet radius for altitude calculations (km)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	float PlanetRadius = 6371.0f;

	// Reference to atmosphere component for integration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog|Integration")
	class UPlanetAtmosphereComponent* AtmosphereComponent;

	// Use Unreal's Exponential Height Fog component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog|Integration")
	bool bUseExponentialHeightFog = true;

	// Auto-update fog based on viewer altitude
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog|Update")
	bool bAutoUpdateFog = true;

	// Update frequency (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog|Update")
	float UpdateFrequency = 0.1f;

	// ============================================================================
	// COMPONENTS
	// ============================================================================

	// Unreal's Exponential Height Fog component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fog|Components")
	UExponentialHeightFogComponent* HeightFogComponent;

	// ============================================================================
	// PUBLIC INTERFACE
	// ============================================================================

	// Initialize fog with planet parameters
	UFUNCTION(BlueprintCallable, Category = "Fog")
	void InitializeFog(float InPlanetRadius, const FAtmosphericFogConfig& Config);

	// Apply fog settings
	UFUNCTION(BlueprintCallable, Category = "Fog")
	void ApplyFogSettings(const FAtmosphericFogConfig& NewSettings);

	// Get fog density at specific altitude
	UFUNCTION(BlueprintCallable, Category = "Fog")
	float GetFogDensityAtAltitude(float AltitudeKm) const;

	// Get fog color at specific altitude
	UFUNCTION(BlueprintCallable, Category = "Fog")
	FLinearColor GetFogColorAtAltitude(float AltitudeKm) const;

	// Calculate atmospheric perspective color
	UFUNCTION(BlueprintCallable, Category = "Fog")
	FLinearColor CalculateAtmosphericPerspective(FVector ViewPosition, FVector TargetPosition, FLinearColor BaseColor) const;

	// Update fog based on viewer position
	UFUNCTION(BlueprintCallable, Category = "Fog")
	void UpdateFogForViewerPosition(FVector ViewerPosition);

	// ============================================================================
	// EXPONENTIAL HEIGHT FOG INTEGRATION
	// ============================================================================

	// Sync settings with Exponential Height Fog component
	UFUNCTION(BlueprintCallable, Category = "Fog|Integration")
	void SyncWithHeightFog();

	// Apply volumetric fog settings
	UFUNCTION(BlueprintCallable, Category = "Fog|Integration")
	void ApplyVolumetricFogSettings();

	// ============================================================================
	// LUMEN INTEGRATION
	// ============================================================================

	// Apply fog parameters for Lumen lighting
	UFUNCTION(BlueprintCallable, Category = "Fog|Lumen")
	void ApplyLumenFogParameters();

	// Update Lumen fog contribution
	UFUNCTION(BlueprintCallable, Category = "Fog|Lumen")
	void UpdateLumenFogContribution(float DeltaTime);

private:
	// Calculate exponential fog density
	float CalculateExponentialDensity(float AltitudeKm) const;

	// Calculate fog color based on altitude and sun direction
	FLinearColor CalculateFogColor(float AltitudeKm, FVector SunDirection) const;

	// Get viewer altitude relative to planet surface
	float GetViewerAltitude(FVector ViewerPosition) const;

	// Timer for periodic updates
	float UpdateTimer = 0.0f;

	// Cached viewer position for optimization
	FVector CachedViewerPosition = FVector::ZeroVector;

	// Cached fog density for optimization
	float CachedFogDensity = 0.0f;
};
