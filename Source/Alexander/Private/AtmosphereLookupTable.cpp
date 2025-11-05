// Copyright Epic Games, Inc. All Rights Reserved.

#include "AtmosphereLookupTable.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Engine/Texture2D.h"

UAtmosphereLookupTable::UAtmosphereLookupTable()
	: TransmittanceLUT(nullptr)
	, SingleScatteringLUT(nullptr)
	, MultipleScatteringLUT(nullptr)
	, CachedPlanetRadius(6371.0f)
	, CachedAtmosphereHeight(100.0f)
	, CachedRayleighScaleHeight(8.0f)
	, CachedMieScaleHeight(1.2f)
{
}

UTexture2D* UAtmosphereLookupTable::GenerateTransmittanceLUT(
	int32 Resolution,
	float PlanetRadius,
	float AtmosphereHeight,
	float RayleighScaleHeight,
	float MieScaleHeight)
{
	UE_LOG(LogTemp, Log, TEXT("Generating Transmittance LUT: %dx%d"), Resolution, Resolution / 4);

	// Cache parameters
	CachedPlanetRadius = PlanetRadius;
	CachedAtmosphereHeight = AtmosphereHeight;
	CachedRayleighScaleHeight = RayleighScaleHeight;
	CachedMieScaleHeight = MieScaleHeight;

	int32 Width = Resolution;
	int32 Height = Resolution / 4; // Aspect ratio 4:1 is typical for transmittance LUT

	TArray<FColor> ColorData;
	ColorData.Reserve(Width * Height);

	// Generate lookup table
	for (int32 Y = 0; Y < Height; Y++)
	{
		// Map Y to view height (0 = surface, 1 = top of atmosphere)
		float NormalizedHeight = float(Y) / float(Height - 1);
		float ViewHeight = NormalizedHeight * AtmosphereHeight;

		for (int32 X = 0; X < Width; X++)
		{
			// Map X to view zenith angle (0 = horizon, 1 = zenith)
			float NormalizedAngle = float(X) / float(Width - 1);
			float ViewZenithAngle = NormalizedAngle * PI;

			// Calculate transmittance
			FLinearColor Transmittance = CalculateTransmittance(
				ViewZenithAngle,
				ViewHeight,
				PlanetRadius,
				AtmosphereHeight,
				RayleighScaleHeight,
				MieScaleHeight);

			// Convert to color
			FColor PixelColor = Transmittance.ToFColor(false);
			ColorData.Add(PixelColor);
		}
	}

	// Create texture
	TransmittanceLUT = CreateTextureFromData(ColorData, Width, Height, TEXT("TransmittanceLUT"));

	UE_LOG(LogTemp, Log, TEXT("Transmittance LUT generated successfully"));
	return TransmittanceLUT;
}

UTexture2D* UAtmosphereLookupTable::GenerateSingleScatteringLUT(
	int32 Resolution,
	float PlanetRadius,
	float AtmosphereHeight,
	FLinearColor RayleighCoefficient,
	FLinearColor MieCoefficient)
{
	UE_LOG(LogTemp, Log, TEXT("Generating Single Scattering LUT: %dx%d"), Resolution, Resolution);

	TArray<FColor> ColorData;
	ColorData.Reserve(Resolution * Resolution);

	// Generate lookup table
	for (int32 Y = 0; Y < Resolution; Y++)
	{
		// Map Y to sun zenith angle
		float SunZenithAngle = (float(Y) / float(Resolution - 1)) * PI;

		for (int32 X = 0; X < Resolution; X++)
		{
			// Map X to view zenith angle
			float ViewZenithAngle = (float(X) / float(Resolution - 1)) * PI;

			// Calculate at mid-atmosphere height for average case
			float ViewHeight = AtmosphereHeight * 0.5f;

			// Calculate single scattering
			FLinearColor Scattering = CalculateSingleScattering(
				ViewZenithAngle,
				SunZenithAngle,
				ViewHeight,
				PlanetRadius,
				AtmosphereHeight,
				RayleighCoefficient,
				MieCoefficient);

			// Convert to color
			FColor PixelColor = Scattering.ToFColor(false);
			ColorData.Add(PixelColor);
		}
	}

	// Create texture
	SingleScatteringLUT = CreateTextureFromData(ColorData, Resolution, Resolution, TEXT("SingleScatteringLUT"));

	UE_LOG(LogTemp, Log, TEXT("Single Scattering LUT generated successfully"));
	return SingleScatteringLUT;
}

UTexture2D* UAtmosphereLookupTable::GenerateMultipleScatteringLUT(
	int32 Resolution,
	float PlanetRadius,
	float AtmosphereHeight)
{
	UE_LOG(LogTemp, Log, TEXT("Generating Multiple Scattering LUT: %dx%d"), Resolution, Resolution);

	TArray<FColor> ColorData;
	ColorData.Reserve(Resolution * Resolution);

	// Generate simplified multiple scattering approximation
	for (int32 Y = 0; Y < Resolution; Y++)
	{
		float NormalizedHeight = float(Y) / float(Resolution - 1);

		for (int32 X = 0; X < Resolution; X++)
		{
			float NormalizedAngle = float(X) / float(Resolution - 1);

			// Simplified multiple scattering approximation
			// In reality, this would require iterative calculation
			float MultipleScatteringFactor = 0.1f * (1.0f - NormalizedHeight) * (1.0f - NormalizedAngle);
			FLinearColor MultipleScattering = FLinearColor(
				MultipleScatteringFactor,
				MultipleScatteringFactor,
				MultipleScatteringFactor);

			FColor PixelColor = MultipleScattering.ToFColor(false);
			ColorData.Add(PixelColor);
		}
	}

	// Create texture
	MultipleScatteringLUT = CreateTextureFromData(ColorData, Resolution, Resolution, TEXT("MultipleScatteringLUT"));

	UE_LOG(LogTemp, Log, TEXT("Multiple Scattering LUT generated successfully"));
	return MultipleScatteringLUT;
}

float UAtmosphereLookupTable::SampleTransmittance(float ViewZenithAngle, float ViewHeight) const
{
	if (!TransmittanceLUT)
	{
		return 1.0f;
	}

	// Map parameters to UV coordinates
	float U = ViewZenithAngle / PI;
	float V = ViewHeight / CachedAtmosphereHeight;

	// In a real implementation, this would sample the texture
	// For now, return a placeholder value
	return FMath::Clamp(1.0f - V * 0.5f, 0.0f, 1.0f);
}

FLinearColor UAtmosphereLookupTable::SampleScattering(float ViewZenithAngle, float SunZenithAngle, float ViewHeight) const
{
	if (!SingleScatteringLUT)
	{
		return FLinearColor::Black;
	}

	// Map parameters to UV coordinates
	float U = ViewZenithAngle / PI;
	float V = SunZenithAngle / PI;

	// In a real implementation, this would sample the texture
	// For now, return a placeholder value
	return FLinearColor(0.3f, 0.5f, 0.8f) * (1.0f - ViewHeight / CachedAtmosphereHeight);
}

float UAtmosphereLookupTable::CalculateOpticalDepth(
	FVector RayStart,
	FVector RayDir,
	float RayLength,
	FVector PlanetCenter,
	float PlanetRadius,
	float ScaleHeight,
	int32 SampleCount) const
{
	float StepSize = RayLength / float(SampleCount);
	float OpticalDepth = 0.0f;

	// Convert km to cm for Unreal units
	float PlanetRadiusCm = PlanetRadius * 100000.0f;

	for (int32 i = 0; i < SampleCount; i++)
	{
		FVector SamplePos = RayStart + RayDir * (float(i) + 0.5f) * StepSize;
		float SampleHeight = (SamplePos - PlanetCenter).Size();
		float HeightAboveSurface = (SampleHeight - PlanetRadiusCm) / 100000.0f; // Convert back to km

		float Density = FMath::Exp(-HeightAboveSurface / ScaleHeight);
		OpticalDepth += Density * StepSize;
	}

	return OpticalDepth;
}

bool UAtmosphereLookupTable::RaySphereIntersection(
	FVector RayOrigin,
	FVector RayDir,
	FVector SphereCenter,
	float SphereRadius,
	float& T0,
	float& T1) const
{
	FVector L = SphereCenter - RayOrigin;
	float Tca = FVector::DotProduct(L, RayDir);
	float D2 = FVector::DotProduct(L, L) - Tca * Tca;
	float Radius2 = SphereRadius * SphereRadius;

	if (D2 > Radius2)
	{
		return false;
	}

	float Thc = FMath::Sqrt(Radius2 - D2);
	T0 = Tca - Thc;
	T1 = Tca + Thc;

	return true;
}

FLinearColor UAtmosphereLookupTable::CalculateTransmittance(
	float ViewZenithAngle,
	float ViewHeight,
	float PlanetRadius,
	float AtmosphereHeight,
	float RayleighScaleHeight,
	float MieScaleHeight) const
{
	// Simplified transmittance calculation
	// In a full implementation, this would integrate optical depth along the ray

	float CosAngle = FMath::Cos(ViewZenithAngle);
	float PathLength = AtmosphereHeight / FMath::Max(CosAngle, 0.01f);

	// Calculate optical depth using exponential density model
	float RayleighOpticalDepth = (RayleighScaleHeight / PathLength) * FMath::Exp(-ViewHeight / RayleighScaleHeight);
	float MieOpticalDepth = (MieScaleHeight / PathLength) * FMath::Exp(-ViewHeight / MieScaleHeight);

	// Calculate transmittance (Beer-Lambert law)
	float RayleighTransmittance = FMath::Exp(-RayleighOpticalDepth);
	float MieTransmittance = FMath::Exp(-MieOpticalDepth);

	// Combine transmittances
	float Transmittance = RayleighTransmittance * MieTransmittance;

	return FLinearColor(Transmittance, Transmittance, Transmittance);
}

FLinearColor UAtmosphereLookupTable::CalculateSingleScattering(
	float ViewZenithAngle,
	float SunZenithAngle,
	float ViewHeight,
	float PlanetRadius,
	float AtmosphereHeight,
	FLinearColor RayleighCoefficient,
	FLinearColor MieCoefficient) const
{
	// Simplified single scattering calculation
	// In a full implementation, this would integrate scattering along the view ray

	float CosViewAngle = FMath::Cos(ViewZenithAngle);
	float CosSunAngle = FMath::Cos(SunZenithAngle);

	// Phase function approximation
	float CosTheta = CosViewAngle * CosSunAngle;
	float RayleighPhase = 0.75f * (1.0f + CosTheta * CosTheta);
	float MiePhase = 0.5f * (1.0f + CosTheta);

	// Height-based density
	float Density = FMath::Exp(-ViewHeight / 8.0f); // Using typical scale height

	// Calculate scattering
	FLinearColor RayleighScattering = RayleighCoefficient * RayleighPhase * Density;
	FLinearColor MieScattering = MieCoefficient * MiePhase * Density * 0.1f;

	return RayleighScattering + MieScattering;
}

UTexture2D* UAtmosphereLookupTable::CreateTextureFromData(
	const TArray<FColor>& ColorData,
	int32 Width,
	int32 Height,
	FName TextureName) const
{
	// Create texture
	UTexture2D* NewTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!NewTexture)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create texture: %s"), *TextureName.ToString());
		return nullptr;
	}

#if WITH_EDITORONLY_DATA
	NewTexture->MipGenSettings = TMGS_NoMipmaps;
#endif
	NewTexture->SRGB = false;
	NewTexture->Filter = TF_Bilinear;
	NewTexture->AddressX = TA_Clamp;
	NewTexture->AddressY = TA_Clamp;

	// Lock texture for writing
	FTexture2DMipMap& Mip = NewTexture->GetPlatformData()->Mips[0];
	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);

	// Copy color data
	FMemory::Memcpy(TextureData, ColorData.GetData(), Width * Height * sizeof(FColor));

	// Unlock and update
	Mip.BulkData.Unlock();
	NewTexture->UpdateResource();

	UE_LOG(LogTemp, Log, TEXT("Created texture: %s (%dx%d)"), *TextureName.ToString(), Width, Height);
	return NewTexture;
}
