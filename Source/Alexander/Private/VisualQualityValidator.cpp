// Copyright Epic Games, Inc. All Rights Reserved.

#include "VisualQualityValidator.h"
#include "Planet.h"
#include "PlanetAtmosphereComponent.h"
#include "PlanetCloudComponent.h"
#include "BiomeManager.h"
#include "TerrainMaterialSystem.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"

AVisualQualityValidator::AVisualQualityValidator()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVisualQualityValidator::BeginPlay()
{
	Super::BeginPlay();
}

float AVisualQualityValidator::ValidateAtmosphericEffects(APlanet* Planet)
{
	if (!Planet)
	{
		UE_LOG(LogTemp, Warning, TEXT("ValidateAtmosphericEffects: Invalid planet"));
		return 0.0f;
	}

	float TotalScore = 0.0f;
	int32 ComponentCount = 0;

	// Validate atmospheric scattering
	UPlanetAtmosphereComponent* AtmosphereComponent = Planet->FindComponentByClass<UPlanetAtmosphereComponent>();
	if (AtmosphereComponent)
	{
		float ScatteringScore = ValidateAtmosphericScattering(AtmosphereComponent);
		TotalScore += ScatteringScore;
		ComponentCount++;

		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Log, TEXT("Atmospheric Scattering Score: %.2f"), ScatteringScore);
		}

		// Validate fog effects
		float FogScore = ValidateFogEffects(AtmosphereComponent);
		TotalScore += FogScore;
		ComponentCount++;

		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Log, TEXT("Fog Effects Score: %.2f"), FogScore);
		}
	}

	// Validate cloud quality
	UPlanetCloudComponent* CloudComponent = Planet->FindComponentByClass<UPlanetCloudComponent>();
	if (CloudComponent)
	{
		float CloudScore = ValidateCloudQuality(CloudComponent);
		TotalScore += CloudScore;
		ComponentCount++;

		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Log, TEXT("Cloud Quality Score: %.2f"), CloudScore);
		}
	}

	return ComponentCount > 0 ? TotalScore / ComponentCount : 0.0f;
}

float AVisualQualityValidator::ValidateAtmosphericScattering(UPlanetAtmosphereComponent* AtmosphereComponent)
{
	if (!AtmosphereComponent)
	{
		return 0.0f;
	}

	float Score = 1.0f;

	// Check Rayleigh scattering configuration
	FLinearColor RayleighCoeff = AtmosphereComponent->RayleighScatteringCoefficient;
	if (RayleighCoeff.R <= 0.0f || RayleighCoeff.G <= 0.0f || RayleighCoeff.B <= 0.0f)
	{
		Score -= 0.3f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Rayleigh scattering coefficients not properly configured"));
		}
	}

	// Check Mie scattering configuration
	FLinearColor MieCoeff = AtmosphereComponent->MieScatteringCoefficient;
	if (MieCoeff.R <= 0.0f || MieCoeff.G <= 0.0f || MieCoeff.B <= 0.0f)
	{
		Score -= 0.2f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Mie scattering coefficients not properly configured"));
		}
	}

	// Check scale heights
	if (AtmosphereComponent->RayleighScaleHeight <= 0.0f || AtmosphereComponent->MieScaleHeight <= 0.0f)
	{
		Score -= 0.2f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Scale heights not properly configured"));
		}
	}

	// Check atmosphere height
	if (AtmosphereComponent->AtmosphereHeight < 50.0f || AtmosphereComponent->AtmosphereHeight > 200.0f)
	{
		Score -= 0.15f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Atmosphere height outside realistic range (50-200km)"));
		}
	}

	// Check Mie anisotropy (should be between -1 and 1)
	if (AtmosphereComponent->MieAnisotropy < -1.0f || AtmosphereComponent->MieAnisotropy > 1.0f)
	{
		Score -= 0.15f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Mie anisotropy outside valid range"));
		}
	}

	return FMath::Max(0.0f, Score);
}

float AVisualQualityValidator::ValidateCloudQuality(UPlanetCloudComponent* CloudComponent)
{
	if (!CloudComponent)
	{
		return 0.0f;
	}

	float Score = 1.0f;

	// Check cloud layer height (should be realistic)
	if (CloudComponent->CloudLayerHeight < 1.0f || CloudComponent->CloudLayerHeight > 20.0f)
	{
		Score -= 0.2f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cloud layer height outside realistic range (1-20km)"));
		}
	}

	// Check cloud thickness
	if (CloudComponent->CloudLayerThickness < 0.5f || CloudComponent->CloudLayerThickness > 10.0f)
	{
		Score -= 0.15f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cloud thickness outside realistic range"));
		}
	}

	// Check cloud coverage (0-1 range)
	if (CloudComponent->CloudCoverage < 0.0f || CloudComponent->CloudCoverage > 1.0f)
	{
		Score -= 0.2f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cloud coverage outside valid range"));
		}
	}

	// Check cloud density
	if (CloudComponent->CloudDensity <= 0.0f)
	{
		Score -= 0.2f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cloud density not configured"));
		}
	}

	// Check wind speed (should be reasonable)
	if (CloudComponent->WindSpeed < 0.0f || CloudComponent->WindSpeed > 100.0f)
	{
		Score -= 0.15f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Wind speed outside realistic range"));
		}
	}

	// Compare against Star Citizen cloud detail standard
	if (Score < QualityStandards.MinCloudDetailLevel)
	{
		Score -= 0.1f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cloud detail below Star Citizen standards"));
		}
	}

	return FMath::Max(0.0f, Score);
}

float AVisualQualityValidator::ValidateFogEffects(UPlanetAtmosphereComponent* AtmosphereComponent)
{
	if (!AtmosphereComponent)
	{
		return 0.0f;
	}

	float Score = 1.0f;

	// Check fog density
	if (AtmosphereComponent->FogDensity < 0.0f)
	{
		Score -= 0.3f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Fog density is negative"));
		}
	}

	// Check fog height falloff
	if (AtmosphereComponent->FogHeightFalloff <= 0.0f)
	{
		Score -= 0.3f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Fog height falloff not properly configured"));
		}
	}

	// Validate fog density is reasonable (not too thick or too thin)
	if (AtmosphereComponent->FogDensity > 0.1f)
	{
		Score -= 0.2f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Fog density too high, may obscure visibility"));
		}
	}

	// Validate height falloff is realistic
	if (AtmosphereComponent->FogHeightFalloff > 1.0f)
	{
		Score -= 0.2f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Fog height falloff too steep"));
		}
	}

	return FMath::Max(0.0f, Score);
}

float AVisualQualityValidator::ValidateBiomeTransitions(APlanet* Planet, const TArray<FVector>& SampleLocations)
{
	if (!Planet || SampleLocations.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ValidateBiomeTransitions: Invalid parameters"));
		return 0.0f;
	}

	float Score = ValidateBiomeBlendSmoothness(SampleLocations, Planet);

	if (bEnableDetailedLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("Biome Transition Score: %.2f"), Score);
	}

	return Score;
}

float AVisualQualityValidator::ValidateBiomeBlendSmoothness(const TArray<FVector>& SampleLocations, APlanet* Planet)
{
	if (!Planet || SampleLocations.Num() < 2)
	{
		return 0.0f;
	}

	float Score = 1.0f;
	int32 TransitionIssues = 0;
	int32 TotalTransitions = 0;

	// Sample biome weights at each location
	for (int32 i = 0; i < SampleLocations.Num() - 1; ++i)
	{
		FVector Location1 = SampleLocations[i];
		FVector Location2 = SampleLocations[i + 1];

		// Calculate distance between samples
		float Distance = FVector::Dist(Location1, Location2);

		// For close samples, check if transition is smooth
		if (Distance < 100.0f) // Within 100m
		{
			TotalTransitions++;

			// Check if there's an abrupt change (this is a simplified check)
			// In a real implementation, you'd query the biome system
			float HeightDiff = FMath::Abs(Location1.Z - Location2.Z);
			if (HeightDiff > Distance * 0.5f) // More than 50% slope
			{
				TransitionIssues++;
			}
		}
	}

	if (TotalTransitions > 0)
	{
		float TransitionQuality = 1.0f - (static_cast<float>(TransitionIssues) / TotalTransitions);
		Score = TransitionQuality;

		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Log, TEXT("Biome transitions: %d issues out of %d transitions"), 
				TransitionIssues, TotalTransitions);
		}
	}

	// Compare against Star Citizen standard
	if (Score < QualityStandards.MinBiomeTransitionSmoothness)
	{
		Score *= 0.9f; // Penalty for not meeting standards
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Biome transition smoothness below Star Citizen standards"));
		}
	}

	return FMath::Max(0.0f, Score);
}

float AVisualQualityValidator::ValidateMaterialQuality(APlanet* Planet)
{
	if (!Planet)
	{
		UE_LOG(LogTemp, Warning, TEXT("ValidateMaterialQuality: Invalid planet"));
		return 0.0f;
	}

	float TotalScore = 0.0f;
	int32 MaterialCount = 0;

	// Get terrain material
	UMaterialInterface* TerrainMaterial = Planet->TerrainMasterMaterial;
	if (TerrainMaterial)
	{
		float TextureScore = ValidateMaterialTextureQuality(TerrainMaterial);
		float ParameterScore = ValidateMaterialParameters(TerrainMaterial);

		TotalScore += (TextureScore + ParameterScore) / 2.0f;
		MaterialCount++;

		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Log, TEXT("Terrain Material - Texture: %.2f, Parameters: %.2f"), 
				TextureScore, ParameterScore);
		}
	}

	// Validate LOD transitions
	float LODScore = ValidateLODTransitions(Planet);
	TotalScore += LODScore;
	MaterialCount++;

	if (bEnableDetailedLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("LOD Transition Score: %.2f"), LODScore);
	}

	return MaterialCount > 0 ? TotalScore / MaterialCount : 0.0f;
}

float AVisualQualityValidator::ValidateMaterialTextureQuality(UMaterialInterface* Material)
{
	if (!Material)
	{
		return 0.0f;
	}

	float Score = 1.0f;

	// Check if material has proper texture parameters
	// This is a simplified check - in production you'd inspect actual texture properties
	TArray<FMaterialParameterInfo> TextureParams;
	TArray<FGuid> TextureGuids;
	Material->GetAllTextureParameterInfo(TextureParams, TextureGuids);

	if (TextureParams.Num() == 0)
	{
		Score -= 0.4f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Material has no texture parameters"));
		}
	}

	// Check for essential PBR textures
	bool bHasBaseColor = false;
	bool bHasNormal = false;
	bool bHasRoughness = false;

	for (const FMaterialParameterInfo& Param : TextureParams)
	{
		FString ParamName = Param.Name.ToString().ToLower();
		if (ParamName.Contains(TEXT("basecolor")) || ParamName.Contains(TEXT("albedo")))
		{
			bHasBaseColor = true;
		}
		if (ParamName.Contains(TEXT("normal")))
		{
			bHasNormal = true;
		}
		if (ParamName.Contains(TEXT("roughness")))
		{
			bHasRoughness = true;
		}
	}

	if (!bHasBaseColor)
	{
		Score -= 0.2f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Material missing base color texture"));
		}
	}

	if (!bHasNormal)
	{
		Score -= 0.2f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Material missing normal map"));
		}
	}

	if (!bHasRoughness)
	{
		Score -= 0.2f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Material missing roughness map"));
		}
	}

	return FMath::Max(0.0f, Score);
}

float AVisualQualityValidator::ValidateMaterialParameters(UMaterialInterface* Material)
{
	if (!Material)
	{
		return 0.0f;
	}

	float Score = 1.0f;

	// Check scalar parameters
	TArray<FMaterialParameterInfo> ScalarParams;
	TArray<FGuid> ScalarGuids;
	Material->GetAllScalarParameterInfo(ScalarParams, ScalarGuids);

	if (ScalarParams.Num() == 0)
	{
		Score -= 0.3f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Material has no scalar parameters"));
		}
	}

	// Check vector parameters
	TArray<FMaterialParameterInfo> VectorParams;
	TArray<FGuid> VectorGuids;
	Material->GetAllVectorParameterInfo(VectorParams, VectorGuids);

	if (VectorParams.Num() == 0)
	{
		Score -= 0.2f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Material has no vector parameters"));
		}
	}

	// Compare against Star Citizen material detail standard
	if (Score < QualityStandards.MinMaterialDetailLevel)
	{
		Score *= 0.9f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Material detail below Star Citizen standards"));
		}
	}

	return FMath::Max(0.0f, Score);
}

float AVisualQualityValidator::ValidateLODTransitions(APlanet* Planet)
{
	if (!Planet)
	{
		return 0.0f;
	}

	float Score = 1.0f;

	// Check LOD configuration
	if (Planet->LODSettings.LODDistances.Num() < 4)
	{
		Score -= 0.3f;
		if (bEnableDetailedLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Insufficient LOD levels (minimum 4 recommended)"));
		}
	}

	// Validate LOD distances are reasonable
	for (int32 i = 0; i < Planet->LODSettings.LODDistances.Num() - 1; ++i)
	{
		float CurrentDist = Planet->LODSettings.LODDistances[i];
		float NextDist = Planet->LODSettings.LODDistances[i + 1];

		if (NextDist <= CurrentDist)
		{
			Score -= 0.2f;
			if (bEnableDetailedLogging)
			{
				UE_LOG(LogTemp, Warning, TEXT("LOD distances not properly ordered"));
			}
			break;
		}

		// Check if transition distance is too abrupt
		float TransitionRange = NextDist - CurrentDist;
		if (TransitionRange < QualityStandards.MaxLODTransitionDistance)
		{
			Score -= 0.1f;
			if (bEnableDetailedLogging)
			{
				UE_LOG(LogTemp, Warning, TEXT("LOD transition range too small at level %d"), i);
			}
		}
	}

	return FMath::Max(0.0f, Score);
}

FVisualQualityMetrics AVisualQualityValidator::RunCompleteValidation(APlanet* Planet)
{
	FVisualQualityMetrics Metrics;

	if (!Planet)
	{
		UE_LOG(LogTemp, Error, TEXT("RunCompleteValidation: Invalid planet"));
		Metrics.QualityIssues.Add(TEXT("Invalid planet reference"));
		return Metrics;
	}

	UE_LOG(LogTemp, Log, TEXT("=== Starting Visual Quality Validation ==="));

	// Validate atmospheric effects
	Metrics.AtmosphericQuality = ValidateAtmosphericEffects(Planet);
	if (Metrics.AtmosphericQuality < QualityStandards.MinAtmosphericScatteringQuality)
	{
		AddQualityIssue(Metrics.QualityIssues, 
			FString::Printf(TEXT("Atmospheric quality (%.2f) below standard (%.2f)"), 
				Metrics.AtmosphericQuality, QualityStandards.MinAtmosphericScatteringQuality), 
			1.0f - Metrics.AtmosphericQuality);
	}

	// Validate biome transitions (sample locations around planet)
	TArray<FVector> SampleLocations;
	for (int32 i = 0; i < 20; ++i)
	{
		float Angle = (i / 20.0f) * 2.0f * PI;
		float Radius = Planet->PlanetRadius * 100000.0f; // Convert km to cm
		SampleLocations.Add(FVector(
			FMath::Cos(Angle) * Radius,
			FMath::Sin(Angle) * Radius,
			0.0f
		));
	}

	Metrics.BiomeTransitionQuality = ValidateBiomeTransitions(Planet, SampleLocations);
	if (Metrics.BiomeTransitionQuality < QualityStandards.MinBiomeTransitionSmoothness)
	{
		AddQualityIssue(Metrics.QualityIssues,
			FString::Printf(TEXT("Biome transition quality (%.2f) below standard (%.2f)"),
				Metrics.BiomeTransitionQuality, QualityStandards.MinBiomeTransitionSmoothness),
			1.0f - Metrics.BiomeTransitionQuality);
	}

	// Validate material quality
	Metrics.MaterialQuality = ValidateMaterialQuality(Planet);
	if (Metrics.MaterialQuality < QualityStandards.MinMaterialDetailLevel)
	{
		AddQualityIssue(Metrics.QualityIssues,
			FString::Printf(TEXT("Material quality (%.2f) below standard (%.2f)"),
				Metrics.MaterialQuality, QualityStandards.MinMaterialDetailLevel),
			1.0f - Metrics.MaterialQuality);
	}

	// Calculate overall quality
	Metrics.OverallQuality = CalculateQualityScore(
		Metrics.AtmosphericQuality,
		Metrics.BiomeTransitionQuality,
		Metrics.MaterialQuality
	);

	// Compare to Star Citizen reference
	Metrics.StarCitizenComparison = CompareToStarCitizenReference(Metrics);

	UE_LOG(LogTemp, Log, TEXT("=== Validation Complete ==="));
	UE_LOG(LogTemp, Log, TEXT("Overall Quality: %.2f"), Metrics.OverallQuality);
	UE_LOG(LogTemp, Log, TEXT("Star Citizen Comparison: %.2f"), Metrics.StarCitizenComparison);

	return Metrics;
}

float AVisualQualityValidator::CalculateQualityScore(float AtmosphericScore, float BiomeScore, float MaterialScore)
{
	// Weighted average (atmospheric effects are most important for visual quality)
	const float AtmosphericWeight = 0.4f;
	const float BiomeWeight = 0.3f;
	const float MaterialWeight = 0.3f;

	return (AtmosphericScore * AtmosphericWeight) +
		   (BiomeScore * BiomeWeight) +
		   (MaterialScore * MaterialWeight);
}

float AVisualQualityValidator::CompareToStarCitizenReference(const FVisualQualityMetrics& Metrics)
{
	// Calculate how close we are to Star Citizen standards
	float AtmosphericComparison = Metrics.AtmosphericQuality / QualityStandards.MinAtmosphericScatteringQuality;
	float BiomeComparison = Metrics.BiomeTransitionQuality / QualityStandards.MinBiomeTransitionSmoothness;
	float MaterialComparison = Metrics.MaterialQuality / QualityStandards.MinMaterialDetailLevel;

	// Average the comparisons (clamped to 1.0 max)
	float AverageComparison = (
		FMath::Min(AtmosphericComparison, 1.0f) +
		FMath::Min(BiomeComparison, 1.0f) +
		FMath::Min(MaterialComparison, 1.0f)
	) / 3.0f;

	return AverageComparison;
}

FString AVisualQualityValidator::GenerateValidationReport(const FVisualQualityMetrics& Metrics)
{
	FString Report = TEXT("=== Visual Quality Validation Report ===\n\n");

	// Overall scores
	Report += FString::Printf(TEXT("Overall Quality Score: %.2f%%\n"), Metrics.OverallQuality * 100.0f);
	Report += FString::Printf(TEXT("Star Citizen Comparison: %.2f%%\n\n"), Metrics.StarCitizenComparison * 100.0f);

	// Component scores
	Report += TEXT("Component Scores:\n");
	Report += FString::Printf(TEXT("  Atmospheric Effects: %.2f%%\n"), Metrics.AtmosphericQuality * 100.0f);
	Report += FString::Printf(TEXT("  Biome Transitions: %.2f%%\n"), Metrics.BiomeTransitionQuality * 100.0f);
	Report += FString::Printf(TEXT("  Material Quality: %.2f%%\n\n"), Metrics.MaterialQuality * 100.0f);

	// Quality issues
	if (Metrics.QualityIssues.Num() > 0)
	{
		Report += TEXT("Quality Issues Found:\n");
		for (const FString& Issue : Metrics.QualityIssues)
		{
			Report += FString::Printf(TEXT("  - %s\n"), *Issue);
		}
	}
	else
	{
		Report += TEXT("No quality issues found. Excellent!\n");
	}

	// Recommendations
	Report += TEXT("\nRecommendations:\n");
	if (Metrics.AtmosphericQuality < 0.85f)
	{
		Report += TEXT("  - Review atmospheric scattering parameters\n");
		Report += TEXT("  - Adjust Rayleigh and Mie coefficients\n");
	}
	if (Metrics.BiomeTransitionQuality < 0.90f)
	{
		Report += TEXT("  - Increase biome transition zone size\n");
		Report += TEXT("  - Smooth terrain blending between biomes\n");
	}
	if (Metrics.MaterialQuality < 0.88f)
	{
		Report += TEXT("  - Add higher resolution textures\n");
		Report += TEXT("  - Implement additional PBR material layers\n");
	}

	Report += TEXT("\n=== End of Report ===\n");

	return Report;
}

void AVisualQualityValidator::AddQualityIssue(TArray<FString>& Issues, const FString& Issue, float Severity)
{
	FString SeverityLabel;
	if (Severity > 0.5f)
	{
		SeverityLabel = TEXT("[CRITICAL]");
	}
	else if (Severity > 0.25f)
	{
		SeverityLabel = TEXT("[WARNING]");
	}
	else
	{
		SeverityLabel = TEXT("[INFO]");
	}

	Issues.Add(FString::Printf(TEXT("%s %s"), *SeverityLabel, *Issue));
}
