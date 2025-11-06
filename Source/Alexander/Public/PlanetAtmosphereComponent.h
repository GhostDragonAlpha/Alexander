// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "PlanetConfiguration.h"
#include "AtmosphereLookupTable.h"
#include "PlanetAtmosphereComponent.generated.h"

/**
 * Component for managing planetary atmospheric effects
 * Integrates with Unreal's Sky Atmosphere system and provides custom atmospheric rendering
 */
UCLASS(ClassGroup=(Planet), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UPlanetAtmosphereComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPlanetAtmosphereComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Atmosphere Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	FAtmosphereConfig AtmosphereSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	float PlanetRadius = 6371.0f; // km

	// Use Unreal's Sky Atmosphere component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Integration")
	bool bUseSkyAtmosphere = true;

	// Use pre-computed lookup tables for performance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Performance")
	bool bUseLookupTables = true;

	// Use real-time ray marching for high quality
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Quality")
	bool bUseRayMarching = false;

	// Number of ray marching samples (higher = better quality, lower performance)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Quality", meta = (ClampMin = "4", ClampMax = "32"))
	int32 RayMarchingSamples = 16;

	// ============================================================================
	// COMPONENTS
	// ============================================================================

	// Unreal's Sky Atmosphere component for integration
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere|Components")
	USkyAtmosphereComponent* SkyAtmosphereComponent;

	// Pre-computed lookup tables
	UPROPERTY()
	UAtmosphereLookupTable* LookupTables;

	// ============================================================================
	// PUBLIC INTERFACE
	// ============================================================================

	// Initialize atmosphere with planet parameters
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	void InitializeAtmosphere(float InPlanetRadius, const FAtmosphereConfig& Config);

	// Apply atmosphere settings
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	void ApplyAtmosphereSettings(const FAtmosphereConfig& NewSettings);

	// Calculate atmospheric color at view direction
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	FLinearColor CalculateAtmosphericColor(FVector ViewDirection, FVector SunDirection);

	// Calculate atmospheric color using ray marching (high quality)
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	FLinearColor CalculateAtmosphericColorRayMarched(FVector ViewDirection, FVector SunDirection, FVector ViewPosition);

	// Get fog density at altitude
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	float GetFogDensityAtAltitude(float Altitude);

	// ============================================================================
	// SKY ATMOSPHERE INTEGRATION
	// ============================================================================

	// Sync settings with Unreal's Sky Atmosphere component
	UFUNCTION(BlueprintCallable, Category = "Atmosphere|Integration")
	void SyncWithSkyAtmosphere();

	// Apply custom atmospheric shader parameters
	UFUNCTION(BlueprintCallable, Category = "Atmosphere|Integration")
	void ApplyCustomShaderParameters();

	// ============================================================================
	// LOOKUP TABLE MANAGEMENT
	// ============================================================================

	// Generate pre-computed lookup tables
	UFUNCTION(BlueprintCallable, Category = "Atmosphere|Performance")
	void GenerateLookupTables();

	// Get transmittance at view angle and altitude
	UFUNCTION(BlueprintCallable, Category = "Atmosphere|Performance")
	float GetTransmittance(float ViewZenithAngle, float ViewHeight) const;

	// Get scattering color from lookup tables
	UFUNCTION(BlueprintCallable, Category = "Atmosphere|Performance")
	FLinearColor GetScatteringFromLUT(float ViewZenithAngle, float SunZenithAngle, float ViewHeight) const;

private:
	// Rayleigh scattering calculation
	FLinearColor CalculateRayleighScattering(FVector ViewDirection, FVector SunDirection, float ViewHeight);

	// Mie scattering calculation
	FLinearColor CalculateMieScattering(FVector ViewDirection, FVector SunDirection, float ViewHeight);

	// Phase function for Rayleigh scattering
	float RayleighPhase(float CosTheta);

	// Phase function for Mie scattering (Henyey-Greenstein)
	float MiePhase(float CosTheta, float MieG);

	// Calculate optical depth for ray through atmosphere
	float CalculateOpticalDepth(FVector RayOrigin, FVector RayDirection, float RayLength, float ScaleHeight);

	// Ray-sphere intersection test
	bool RaySphereIntersection(FVector RayOrigin, FVector RayDirection, FVector SphereCenter, float SphereRadius, float& T0, float& T1);
};
