// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Texture2D.h"
#include "AtmosphereLookupTable.generated.h"

/**
 * Pre-computed lookup tables for atmospheric scattering
 * Improves performance by pre-calculating expensive scattering integrals
 */
UCLASS()
class ALEXANDER_API UAtmosphereLookupTable : public UObject
{
	GENERATED_BODY()

public:
	UAtmosphereLookupTable();

	// ============================================================================
	// LOOKUP TABLE GENERATION
	// ============================================================================

	/**
	 * Generate transmittance lookup table
	 * Stores atmospheric transmittance as function of view angle and altitude
	 * @param Resolution - Texture resolution (recommended: 256x64)
	 * @param PlanetRadius - Planet radius in km
	 * @param AtmosphereHeight - Atmosphere thickness in km
	 * @param RayleighScaleHeight - Rayleigh scale height in km
	 * @param MieScaleHeight - Mie scale height in km
	 * @return Generated texture
	 */
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	UTexture2D* GenerateTransmittanceLUT(
		int32 Resolution = 256,
		float PlanetRadius = 6371.0f,
		float AtmosphereHeight = 100.0f,
		float RayleighScaleHeight = 8.0f,
		float MieScaleHeight = 1.2f);

	/**
	 * Generate single scattering lookup table
	 * Stores single scattering contribution for different view/sun angles
	 * @param Resolution - Texture resolution (recommended: 256x256)
	 * @param PlanetRadius - Planet radius in km
	 * @param AtmosphereHeight - Atmosphere thickness in km
	 * @param RayleighCoefficient - Rayleigh scattering coefficient
	 * @param MieCoefficient - Mie scattering coefficient
	 * @return Generated texture
	 */
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	UTexture2D* GenerateSingleScatteringLUT(
		int32 Resolution = 256,
		float PlanetRadius = 6371.0f,
		float AtmosphereHeight = 100.0f,
		FLinearColor RayleighCoefficient = FLinearColor(0.0058f, 0.0135f, 0.0331f),
		FLinearColor MieCoefficient = FLinearColor(0.021f, 0.021f, 0.021f));

	/**
	 * Generate multiple scattering lookup table
	 * Stores multiple scattering approximation
	 * @param Resolution - Texture resolution (recommended: 64x64)
	 * @param PlanetRadius - Planet radius in km
	 * @param AtmosphereHeight - Atmosphere thickness in km
	 * @return Generated texture
	 */
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	UTexture2D* GenerateMultipleScatteringLUT(
		int32 Resolution = 64,
		float PlanetRadius = 6371.0f,
		float AtmosphereHeight = 100.0f);

	// ============================================================================
	// LOOKUP TABLE ACCESS
	// ============================================================================

	/** Get transmittance lookup table */
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	UTexture2D* GetTransmittanceLUT() const { return TransmittanceLUT; }

	/** Get single scattering lookup table */
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	UTexture2D* GetSingleScatteringLUT() const { return SingleScatteringLUT; }

	/** Get multiple scattering lookup table */
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	UTexture2D* GetMultipleScatteringLUT() const { return MultipleScatteringLUT; }

	// ============================================================================
	// UTILITY FUNCTIONS
	// ============================================================================

	/**
	 * Sample transmittance from lookup table
	 * @param ViewZenithAngle - Angle from zenith (0 = up, PI = down)
	 * @param ViewHeight - Height above surface in km
	 * @return Transmittance value (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	float SampleTransmittance(float ViewZenithAngle, float ViewHeight) const;

	/**
	 * Sample scattering from lookup table
	 * @param ViewZenithAngle - View angle from zenith
	 * @param SunZenithAngle - Sun angle from zenith
	 * @param ViewHeight - Height above surface in km
	 * @return Scattered light color
	 */
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	FLinearColor SampleScattering(float ViewZenithAngle, float SunZenithAngle, float ViewHeight) const;

protected:
	// ============================================================================
	// INTERNAL CALCULATION FUNCTIONS
	// ============================================================================

	/** Calculate optical depth along ray */
	float CalculateOpticalDepth(
		FVector RayStart,
		FVector RayDir,
		float RayLength,
		FVector PlanetCenter,
		float PlanetRadius,
		float ScaleHeight,
		int32 SampleCount = 32) const;

	/** Ray-sphere intersection test */
	bool RaySphereIntersection(
		FVector RayOrigin,
		FVector RayDir,
		FVector SphereCenter,
		float SphereRadius,
		float& T0,
		float& T1) const;

	/** Calculate transmittance for specific parameters */
	FLinearColor CalculateTransmittance(
		float ViewZenithAngle,
		float ViewHeight,
		float PlanetRadius,
		float AtmosphereHeight,
		float RayleighScaleHeight,
		float MieScaleHeight) const;

	/** Calculate single scattering contribution */
	FLinearColor CalculateSingleScattering(
		float ViewZenithAngle,
		float SunZenithAngle,
		float ViewHeight,
		float PlanetRadius,
		float AtmosphereHeight,
		FLinearColor RayleighCoefficient,
		FLinearColor MieCoefficient) const;

	/** Create texture from color data */
	UTexture2D* CreateTextureFromData(
		const TArray<FColor>& ColorData,
		int32 Width,
		int32 Height,
		FName TextureName) const;

private:
	// ============================================================================
	// LOOKUP TABLES
	// ============================================================================

	/** Transmittance lookup table */
	UPROPERTY()
	UTexture2D* TransmittanceLUT;

	/** Single scattering lookup table */
	UPROPERTY()
	UTexture2D* SingleScatteringLUT;

	/** Multiple scattering lookup table */
	UPROPERTY()
	UTexture2D* MultipleScatteringLUT;

	// ============================================================================
	// CACHED PARAMETERS
	// ============================================================================

	float CachedPlanetRadius;
	float CachedAtmosphereHeight;
	float CachedRayleighScaleHeight;
	float CachedMieScaleHeight;
};
