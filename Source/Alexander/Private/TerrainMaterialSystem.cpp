// Copyright Epic Games, Inc. All Rights Reserved.

#include "TerrainMaterialSystem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector.h"

UTerrainMaterialSystem::UTerrainMaterialSystem()
{
	MasterMaterial = nullptr;
	GlobalTilingScale = 1.0f;
}

void UTerrainMaterialSystem::Initialize(UMaterialInterface* InMasterMaterial)
{
	MasterMaterial = InMasterMaterial;

	if (MasterMaterial)
	{
		UE_LOG(LogTemp, Log, TEXT("TerrainMaterialSystem initialized with master material: %s"), 
			*MasterMaterial->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TerrainMaterialSystem initialized with null master material"));
	}

	// Initialize default layer configurations if not already set
	if (LayerConfigs.Num() == 0)
	{
		// Set up default configurations for each slot
		for (int32 i = 0; i < 8; i++)
		{
			ETerrainMaterialSlot Slot = static_cast<ETerrainMaterialSlot>(i);
			FTerrainMaterialLayerConfig DefaultConfig;
			DefaultConfig.Slot = Slot;
			
			// Set default slope and altitude ranges based on slot type
			switch (Slot)
			{
			case ETerrainMaterialSlot::BaseRock:
				DefaultConfig.LayerName = TEXT("Base Rock");
				DefaultConfig.SlopeRange = FVector2D(0.0f, 90.0f);
				DefaultConfig.AltitudeRange = FVector2D(-1000.0f, 10000.0f);
				break;
			case ETerrainMaterialSlot::DirtSoil:
				DefaultConfig.LayerName = TEXT("Dirt/Soil");
				DefaultConfig.SlopeRange = FVector2D(0.0f, 30.0f);
				DefaultConfig.AltitudeRange = FVector2D(-100.0f, 2000.0f);
				break;
			case ETerrainMaterialSlot::Grass:
				DefaultConfig.LayerName = TEXT("Grass");
				DefaultConfig.SlopeRange = FVector2D(0.0f, 25.0f);
				DefaultConfig.AltitudeRange = FVector2D(0.0f, 1500.0f);
				break;
			case ETerrainMaterialSlot::Sand:
				DefaultConfig.LayerName = TEXT("Sand");
				DefaultConfig.SlopeRange = FVector2D(0.0f, 20.0f);
				DefaultConfig.AltitudeRange = FVector2D(-50.0f, 500.0f);
				break;
			case ETerrainMaterialSlot::Snow:
				DefaultConfig.LayerName = TEXT("Snow");
				DefaultConfig.SlopeRange = FVector2D(0.0f, 45.0f);
				DefaultConfig.AltitudeRange = FVector2D(2000.0f, 10000.0f);
				break;
			case ETerrainMaterialSlot::CliffRock:
				DefaultConfig.LayerName = TEXT("Cliff Rock");
				DefaultConfig.SlopeRange = FVector2D(45.0f, 90.0f);
				DefaultConfig.AltitudeRange = FVector2D(-1000.0f, 10000.0f);
				DefaultConfig.bUseTriplanarProjection = true;
				break;
			case ETerrainMaterialSlot::WetSurface:
				DefaultConfig.LayerName = TEXT("Wet Surface");
				DefaultConfig.SlopeRange = FVector2D(0.0f, 90.0f);
				DefaultConfig.AltitudeRange = FVector2D(-1000.0f, 10000.0f);
				break;
			case ETerrainMaterialSlot::Custom:
				DefaultConfig.LayerName = TEXT("Custom");
				DefaultConfig.SlopeRange = FVector2D(0.0f, 90.0f);
				DefaultConfig.AltitudeRange = FVector2D(-1000.0f, 10000.0f);
				break;
			}

			LayerConfigs.Add(Slot, DefaultConfig);
		}
	}
}

UMaterialInstanceDynamic* UTerrainMaterialSystem::CreateTerrainMaterialInstance()
{
	if (!MasterMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot create material instance: Master material is null"));
		return nullptr;
	}

	UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(MasterMaterial, this);

	if (MaterialInstance)
	{
		// Initialize with current layer configurations
		UpdateLayerTextures(MaterialInstance);

		// Apply advanced features
		MaterialInstance->SetScalarParameterValue(TEXT("EnableParallaxOcclusion"), 
			AdvancedFeatures.bEnableParallaxOcclusion ? 1.0f : 0.0f);
		MaterialInstance->SetScalarParameterValue(TEXT("ParallaxDepthScale"), 
			AdvancedFeatures.ParallaxDepthScale);
		MaterialInstance->SetScalarParameterValue(TEXT("EnableTessellation"), 
			AdvancedFeatures.bEnableTessellation ? 1.0f : 0.0f);
		MaterialInstance->SetScalarParameterValue(TEXT("TessellationMultiplier"), 
			AdvancedFeatures.TessellationMultiplier);
		MaterialInstance->SetScalarParameterValue(TEXT("GlobalTilingScale"), 
			GlobalTilingScale);

		UE_LOG(LogTemp, Log, TEXT("Created terrain material instance"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create material instance"));
	}

	return MaterialInstance;
}

void UTerrainMaterialSystem::ConfigureLayer(ETerrainMaterialSlot Slot, const FTerrainMaterialLayerConfig& LayerConfig)
{
	LayerConfigs.Add(Slot, LayerConfig);
	UE_LOG(LogTemp, Log, TEXT("Configured material layer: %s"), *LayerConfig.LayerName);
}

FTerrainMaterialLayerConfig UTerrainMaterialSystem::GetLayerConfig(ETerrainMaterialSlot Slot) const
{
	const FTerrainMaterialLayerConfig* Config = LayerConfigs.Find(Slot);
	if (Config)
	{
		return *Config;
	}

	// Return default config if not found
	FTerrainMaterialLayerConfig DefaultConfig;
	DefaultConfig.Slot = Slot;
	return DefaultConfig;
}

void UTerrainMaterialSystem::UpdateLayerTextures(UMaterialInstanceDynamic* MaterialInstance)
{
	if (!MaterialInstance)
	{
		return;
	}

	// Update textures for each layer
	for (const auto& Pair : LayerConfigs)
	{
		ETerrainMaterialSlot Slot = Pair.Key;
		const FTerrainMaterialLayerConfig& Config = Pair.Value;

		// Build parameter names based on slot index
		int32 SlotIndex = static_cast<int32>(Slot);
		FString SlotPrefix = FString::Printf(TEXT("Layer%d_"), SlotIndex);

		// Set PBR textures
		if (Config.TextureSet.BaseColor)
		{
			MaterialInstance->SetTextureParameterValue(
				FName(*(SlotPrefix + TEXT("BaseColor"))), 
				Config.TextureSet.BaseColor);
		}

		if (Config.TextureSet.Normal)
		{
			MaterialInstance->SetTextureParameterValue(
				FName(*(SlotPrefix + TEXT("Normal"))), 
				Config.TextureSet.Normal);
		}

		if (Config.TextureSet.Roughness)
		{
			MaterialInstance->SetTextureParameterValue(
				FName(*(SlotPrefix + TEXT("Roughness"))), 
				Config.TextureSet.Roughness);
		}

		if (Config.TextureSet.Metallic)
		{
			MaterialInstance->SetTextureParameterValue(
				FName(*(SlotPrefix + TEXT("Metallic"))), 
				Config.TextureSet.Metallic);
		}

		if (Config.TextureSet.AmbientOcclusion)
		{
			MaterialInstance->SetTextureParameterValue(
				FName(*(SlotPrefix + TEXT("AO"))), 
				Config.TextureSet.AmbientOcclusion);
		}

		if (Config.TextureSet.Height)
		{
			MaterialInstance->SetTextureParameterValue(
				FName(*(SlotPrefix + TEXT("Height"))), 
				Config.TextureSet.Height);
		}

		// Set tiling scale
		MaterialInstance->SetScalarParameterValue(
			FName(*(SlotPrefix + TEXT("TilingScale"))), 
			Config.TextureSet.TilingScale);

		// Set triplanar projection settings
		MaterialInstance->SetScalarParameterValue(
			FName(*(SlotPrefix + TEXT("UseTriplanar"))), 
			Config.bUseTriplanarProjection ? 1.0f : 0.0f);
		MaterialInstance->SetScalarParameterValue(
			FName(*(SlotPrefix + TEXT("TriplanarSharpness"))), 
			Config.TriplanarSharpness);

		// Set slope and altitude ranges
		MaterialInstance->SetVectorParameterValue(
			FName(*(SlotPrefix + TEXT("SlopeRange"))), 
			FLinearColor(Config.SlopeRange.X, Config.SlopeRange.Y, 0.0f, 0.0f));
		MaterialInstance->SetVectorParameterValue(
			FName(*(SlotPrefix + TEXT("AltitudeRange"))), 
			FLinearColor(Config.AltitudeRange.X, Config.AltitudeRange.Y, 0.0f, 0.0f));

		// Set macro texture blending
		MaterialInstance->SetScalarParameterValue(
			FName(*(SlotPrefix + TEXT("MacroBlendDistance"))), 
			Config.MacroBlendDistance);
	}
}

void UTerrainMaterialSystem::SetBiomeBlendWeights(UMaterialInstanceDynamic* MaterialInstance, 
	const TMap<int32, float>& BiomeWeights)
{
	if (!MaterialInstance)
	{
		return;
	}

	// Set up to 8 biome weights (matching our 8 material layers)
	TArray<int32> BiomeIndices;
	BiomeWeights.GetKeys(BiomeIndices);

	for (int32 i = 0; i < 8; i++)
	{
		float Weight = 0.0f;
		int32 BiomeIndex = -1;

		if (i < BiomeIndices.Num())
		{
			BiomeIndex = BiomeIndices[i];
			const float* WeightPtr = BiomeWeights.Find(BiomeIndex);
			if (WeightPtr)
			{
				Weight = *WeightPtr;
			}
		}

		// Set biome weight parameter
		FString ParamName = FString::Printf(TEXT("BiomeWeight%d"), i);
		MaterialInstance->SetScalarParameterValue(FName(*ParamName), Weight);

		// Set biome index parameter
		ParamName = FString::Printf(TEXT("BiomeIndex%d"), i);
		MaterialInstance->SetScalarParameterValue(FName(*ParamName), static_cast<float>(BiomeIndex));
	}
}

void UTerrainMaterialSystem::UpdateSlopeBlending(UMaterialInstanceDynamic* MaterialInstance, float SlopeAngle)
{
	if (!MaterialInstance)
	{
		return;
	}

	MaterialInstance->SetScalarParameterValue(TEXT("CurrentSlope"), SlopeAngle);

	// Calculate cliff material weight based on slope
	float CliffWeight = 0.0f;
	const FTerrainMaterialLayerConfig* CliffConfig = LayerConfigs.Find(ETerrainMaterialSlot::CliffRock);
	if (CliffConfig)
	{
		CliffWeight = CalculateLayerWeight(*CliffConfig, SlopeAngle, 0.0f);
	}

	MaterialInstance->SetScalarParameterValue(TEXT("CliffMaterialWeight"), CliffWeight);
}

void UTerrainMaterialSystem::UpdateAltitudeBlending(UMaterialInstanceDynamic* MaterialInstance, float Altitude)
{
	if (!MaterialInstance)
	{
		return;
	}

	MaterialInstance->SetScalarParameterValue(TEXT("CurrentAltitude"), Altitude);

	// Calculate snow material weight based on altitude
	float SnowWeight = 0.0f;
	const FTerrainMaterialLayerConfig* SnowConfig = LayerConfigs.Find(ETerrainMaterialSlot::Snow);
	if (SnowConfig)
	{
		SnowWeight = CalculateLayerWeight(*SnowConfig, 0.0f, Altitude);
	}

	MaterialInstance->SetScalarParameterValue(TEXT("SnowMaterialWeight"), SnowWeight);
}

void UTerrainMaterialSystem::UpdateMoistureBlending(UMaterialInstanceDynamic* MaterialInstance, float Moisture)
{
	if (!MaterialInstance)
	{
		return;
	}

	float ClampedMoisture = FMath::Clamp(Moisture, 0.0f, 1.0f);
	MaterialInstance->SetScalarParameterValue(TEXT("CurrentMoisture"), ClampedMoisture);

	// Calculate moisture-influenced material weights
	// Higher moisture favors grass, wet surfaces, and reduces sand
	float GrassBonus = ClampedMoisture * 0.3f; // Up to 30% bonus
	float SandPenalty = ClampedMoisture * 0.5f; // Up to 50% penalty
	float WetSurfaceWeight = ClampedMoisture > 0.7f ? (ClampedMoisture - 0.7f) / 0.3f : 0.0f;

	MaterialInstance->SetScalarParameterValue(TEXT("MoistureGrassBonus"), GrassBonus);
	MaterialInstance->SetScalarParameterValue(TEXT("MoistureSandPenalty"), SandPenalty);
	MaterialInstance->SetScalarParameterValue(TEXT("MoistureWetWeight"), WetSurfaceWeight);
}

void UTerrainMaterialSystem::UpdateTemperatureBlending(UMaterialInstanceDynamic* MaterialInstance, float Temperature)
{
	if (!MaterialInstance)
	{
		return;
	}

	MaterialInstance->SetScalarParameterValue(TEXT("CurrentTemperature"), Temperature);

	// Calculate temperature-influenced material weights
	// Cold temperatures favor snow and ice
	// Hot temperatures favor sand and dry materials
	float SnowWeight = 0.0f;
	float SandWeight = 0.0f;

	if (Temperature < 0.0f)
	{
		// Below freezing - increase snow
		SnowWeight = FMath::Clamp(-Temperature / 20.0f, 0.0f, 1.0f);
	}
	else if (Temperature > 30.0f)
	{
		// Hot - increase sand/desert materials
		SandWeight = FMath::Clamp((Temperature - 30.0f) / 30.0f, 0.0f, 1.0f);
	}

	MaterialInstance->SetScalarParameterValue(TEXT("TemperatureSnowWeight"), SnowWeight);
	MaterialInstance->SetScalarParameterValue(TEXT("TemperatureSandWeight"), SandWeight);
}

void UTerrainMaterialSystem::CalculateAndApplyBlendWeights(
	UMaterialInstanceDynamic* MaterialInstance,
	float SlopeAngle,
	float Altitude,
	const TMap<int32, float>& BiomeWeights,
	float Moisture,
	float Temperature)
{
	if (!MaterialInstance)
	{
		return;
	}

	// Update individual parameters
	UpdateSlopeBlending(MaterialInstance, SlopeAngle);
	UpdateAltitudeBlending(MaterialInstance, Altitude);
	UpdateMoistureBlending(MaterialInstance, Moisture);
	UpdateTemperatureBlending(MaterialInstance, Temperature);
	SetBiomeBlendWeights(MaterialInstance, BiomeWeights);

	// Calculate comprehensive layer weights
	TMap<ETerrainMaterialSlot, float> LayerWeights = CalculateLayerWeights(
		SlopeAngle, Altitude, Moisture, Temperature
	);

	// Apply layer weights to material
	for (const auto& Pair : LayerWeights)
	{
		ETerrainMaterialSlot Slot = Pair.Key;
		float Weight = Pair.Value;

		int32 SlotIndex = static_cast<int32>(Slot);
		FString ParamName = FString::Printf(TEXT("Layer%d_Weight"), SlotIndex);
		MaterialInstance->SetScalarParameterValue(FName(*ParamName), Weight);
	}

	// Log blending info for debugging
	UE_LOG(LogTemp, Verbose, TEXT("Material Blending - Slope: %.1f, Alt: %.1f, Moisture: %.2f, Temp: %.1f"),
		SlopeAngle, Altitude, Moisture, Temperature);
}

TMap<ETerrainMaterialSlot, float> UTerrainMaterialSystem::CalculateLayerWeights(
	float SlopeAngle,
	float Altitude,
	float Moisture,
	float Temperature) const
{
	TMap<ETerrainMaterialSlot, float> Weights;

	// Calculate weight for each layer based on all environmental factors
	for (const auto& Pair : LayerConfigs)
	{
		ETerrainMaterialSlot Slot = Pair.Key;
		const FTerrainMaterialLayerConfig& Config = Pair.Value;

		float Weight = CalculateLayerWeightAdvanced(Config, SlopeAngle, Altitude, Moisture, Temperature);
		Weights.Add(Slot, Weight);
	}

	// Normalize weights to sum to 1.0
	NormalizeLayerWeights(Weights);

	return Weights;
}

void UTerrainMaterialSystem::ConfigureAdvancedFeatures(const FAdvancedMaterialFeatures& Features)
{
	AdvancedFeatures = Features;
	UE_LOG(LogTemp, Log, TEXT("Configured advanced material features"));
}

void UTerrainMaterialSystem::UpdateWetness(UMaterialInstanceDynamic* MaterialInstance, float WetnessAmount)
{
	if (!MaterialInstance || !AdvancedFeatures.bEnableDynamicWetness)
	{
		return;
	}

	float ClampedWetness = FMath::Clamp(WetnessAmount, 0.0f, 1.0f);
	MaterialInstance->SetScalarParameterValue(TEXT("WetnessIntensity"), ClampedWetness);
}

void UTerrainMaterialSystem::UpdateTessellation(UMaterialInstanceDynamic* MaterialInstance, float ViewDistance)
{
	if (!MaterialInstance || !AdvancedFeatures.bEnableTessellation)
	{
		return;
	}

	// Calculate tessellation factor based on distance
	float TessellationFactor = 0.0f;
	if (ViewDistance < AdvancedFeatures.MaxTessellationDistance)
	{
		// Exponential falloff for smoother transition
		float DistanceFactor = 1.0f - (ViewDistance / AdvancedFeatures.MaxTessellationDistance);
		DistanceFactor = FMath::Clamp(DistanceFactor, 0.0f, 1.0f);
		
		// Apply power for exponential falloff
		DistanceFactor = FMath::Pow(DistanceFactor, 2.0f);
		
		TessellationFactor = DistanceFactor * AdvancedFeatures.TessellationMultiplier;
		TessellationFactor = FMath::Clamp(TessellationFactor, 0.0f, 4.0f);
	}

	MaterialInstance->SetScalarParameterValue(TEXT("TessellationFactor"), TessellationFactor);
	MaterialInstance->SetScalarParameterValue(TEXT("MaxTessellationDistance"), 
		AdvancedFeatures.MaxTessellationDistance);
}

void UTerrainMaterialSystem::UpdateParallaxOcclusion(UMaterialInstanceDynamic* MaterialInstance, float ViewDistance)
{
	if (!MaterialInstance || !AdvancedFeatures.bEnableParallaxOcclusion)
	{
		return;
	}

	// Disable parallax beyond 50 meters for performance
	const float MaxParallaxDistance = 50.0f;
	float ParallaxEnabled = ViewDistance < MaxParallaxDistance ? 1.0f : 0.0f;

	// Adjust parallax steps based on distance for quality/performance balance
	int32 ParallaxSteps = 16;
	if (ViewDistance > 25.0f)
	{
		ParallaxSteps = 8; // Reduce steps at medium distance
	}
	else if (ViewDistance < 10.0f)
	{
		ParallaxSteps = 32; // Increase steps for close viewing
	}

	MaterialInstance->SetScalarParameterValue(TEXT("EnableParallaxOcclusion"), ParallaxEnabled);
	MaterialInstance->SetScalarParameterValue(TEXT("ParallaxSteps"), static_cast<float>(ParallaxSteps));
}

void UTerrainMaterialSystem::UpdateMacroMicroBlending(UMaterialInstanceDynamic* MaterialInstance, float ViewDistance)
{
	if (!MaterialInstance)
	{
		return;
	}

	// Update macro/micro blending for each layer
	for (const auto& Pair : LayerConfigs)
	{
		ETerrainMaterialSlot Slot = Pair.Key;
		const FTerrainMaterialLayerConfig& Config = Pair.Value;

		int32 SlotIndex = static_cast<int32>(Slot);

		// Calculate blend factor for macro textures
		float BlendFactor = 0.0f;
		if (ViewDistance > Config.MacroBlendDistance)
		{
			// Start blending to macro textures
			float BlendRange = Config.MacroBlendDistance * 0.5f; // 50% range for smooth transition
			float DistanceOverThreshold = ViewDistance - Config.MacroBlendDistance;
			BlendFactor = FMath::Clamp(DistanceOverThreshold / BlendRange, 0.0f, 1.0f);
			
			// Use smoothstep for smoother transition
			BlendFactor = FMath::SmoothStep(0.0f, 1.0f, BlendFactor);
		}

		FString ParamName = FString::Printf(TEXT("Layer%d_MacroBlend"), SlotIndex);
		MaterialInstance->SetScalarParameterValue(FName(*ParamName), BlendFactor);
	}

	// Set global view distance for shader use
	MaterialInstance->SetScalarParameterValue(TEXT("ViewDistance"), ViewDistance);
}

void UTerrainMaterialSystem::UpdateAdvancedFeatures(UMaterialInstanceDynamic* MaterialInstance, float ViewDistance)
{
	if (!MaterialInstance)
	{
		return;
	}

	// Update all advanced features based on view distance
	UpdateParallaxOcclusion(MaterialInstance, ViewDistance);
	UpdateTessellation(MaterialInstance, ViewDistance);
	UpdateMacroMicroBlending(MaterialInstance, ViewDistance);

	// Log for debugging (verbose only)
	UE_LOG(LogTemp, VeryVerbose, TEXT("Updated advanced material features at distance: %.1fm"), ViewDistance);
}

float UTerrainMaterialSystem::CalculateLayerWeight(const FTerrainMaterialLayerConfig& LayerConfig, 
	float SlopeAngle, float Altitude) const
{
	// Calculate slope weight
	float SlopeWeight = 0.0f;
	if (SlopeAngle >= LayerConfig.SlopeRange.X && SlopeAngle <= LayerConfig.SlopeRange.Y)
	{
		// Within range, calculate smooth weight
		float SlopeRangeSize = LayerConfig.SlopeRange.Y - LayerConfig.SlopeRange.X;
		if (SlopeRangeSize > 0.0f)
		{
			float NormalizedSlope = (SlopeAngle - LayerConfig.SlopeRange.X) / SlopeRangeSize;
			// Use smoothstep for smooth transitions
			SlopeWeight = FMath::SmoothStep(0.0f, 1.0f, NormalizedSlope);
		}
		else
		{
			SlopeWeight = 1.0f;
		}
	}

	// Calculate altitude weight
	float AltitudeWeight = 0.0f;
	if (Altitude >= LayerConfig.AltitudeRange.X && Altitude <= LayerConfig.AltitudeRange.Y)
	{
		// Within range, calculate smooth weight
		float AltitudeRangeSize = LayerConfig.AltitudeRange.Y - LayerConfig.AltitudeRange.X;
		if (AltitudeRangeSize > 0.0f)
		{
			float NormalizedAltitude = (Altitude - LayerConfig.AltitudeRange.X) / AltitudeRangeSize;
			// Use smoothstep for smooth transitions
			AltitudeWeight = FMath::SmoothStep(0.0f, 1.0f, NormalizedAltitude);
		}
		else
		{
			AltitudeWeight = 1.0f;
		}
	}

	// Combine weights (both must be satisfied)
	return SlopeWeight * AltitudeWeight;
}

void UTerrainMaterialSystem::ApplyTriplanarProjection(UMaterialInstanceDynamic* MaterialInstance, 
	ETerrainMaterialSlot Slot)
{
	if (!MaterialInstance)
	{
		return;
	}

	const FTerrainMaterialLayerConfig* Config = LayerConfigs.Find(Slot);
	if (!Config)
	{
		return;
	}

	int32 SlotIndex = static_cast<int32>(Slot);
	FString ParamName = FString::Printf(TEXT("Layer%d_UseTriplanar"), SlotIndex);
	MaterialInstance->SetScalarParameterValue(FName(*ParamName), 
		Config->bUseTriplanarProjection ? 1.0f : 0.0f);

	ParamName = FString::Printf(TEXT("Layer%d_TriplanarSharpness"), SlotIndex);
	MaterialInstance->SetScalarParameterValue(FName(*ParamName), Config->TriplanarSharpness);
}

void UTerrainMaterialSystem::UpdateDistanceBlending(UMaterialInstanceDynamic* MaterialInstance, float ViewDistance)
{
	if (!MaterialInstance)
	{
		return;
	}

	// Update distance-based blending for all layers
	for (const auto& Pair : LayerConfigs)
	{
		ETerrainMaterialSlot Slot = Pair.Key;
		const FTerrainMaterialLayerConfig& Config = Pair.Value;

		int32 SlotIndex = static_cast<int32>(Slot);

		// Calculate blend factor for macro textures
		float BlendFactor = 0.0f;
		if (ViewDistance > Config.MacroBlendDistance)
		{
			// Start blending to macro textures
			float BlendRange = Config.MacroBlendDistance * 0.5f; // 50% range for smooth transition
			float DistanceOverThreshold = ViewDistance - Config.MacroBlendDistance;
			BlendFactor = FMath::Clamp(DistanceOverThreshold / BlendRange, 0.0f, 1.0f);
		}

		FString ParamName = FString::Printf(TEXT("Layer%d_MacroBlend"), SlotIndex);
		MaterialInstance->SetScalarParameterValue(FName(*ParamName), BlendFactor);
	}
}

float UTerrainMaterialSystem::CalculateLayerWeightAdvanced(
	const FTerrainMaterialLayerConfig& LayerConfig,
	float SlopeAngle,
	float Altitude,
	float Moisture,
	float Temperature) const
{
	// Calculate individual influences
	float SlopeInfluence = CalculateSlopeInfluence(SlopeAngle, LayerConfig.SlopeRange);
	float AltitudeInfluence = CalculateAltitudeInfluence(Altitude, LayerConfig.AltitudeRange);
	float MoistureInfluence = CalculateMoistureInfluence(Moisture, LayerConfig.Slot);
	float TemperatureInfluence = CalculateTemperatureInfluence(Temperature, LayerConfig.Slot);

	// Combine influences multiplicatively (all must be satisfied)
	float BaseWeight = SlopeInfluence * AltitudeInfluence;

	// Apply environmental modifiers
	float EnvironmentalModifier = (MoistureInfluence + TemperatureInfluence) * 0.5f;
	
	// Blend base weight with environmental influence
	float FinalWeight = BaseWeight * (0.7f + EnvironmentalModifier * 0.3f);

	return FMath::Clamp(FinalWeight, 0.0f, 1.0f);
}

float UTerrainMaterialSystem::CalculateSlopeInfluence(float SlopeAngle, const FVector2D& SlopeRange) const
{
	if (SlopeAngle < SlopeRange.X || SlopeAngle > SlopeRange.Y)
	{
		return 0.0f;
	}

	float RangeSize = SlopeRange.Y - SlopeRange.X;
	if (RangeSize <= 0.0f)
	{
		return 1.0f;
	}

	// Calculate position within range
	float NormalizedPosition = (SlopeAngle - SlopeRange.X) / RangeSize;

	// Use smoothstep for smooth transitions at edges
	// Peak weight in the middle of the range
	float EdgeBlend = 0.2f; // 20% of range for edge blending

	if (NormalizedPosition < EdgeBlend)
	{
		// Fade in from lower edge
		return FMath::SmoothStep(0.0f, 1.0f, NormalizedPosition / EdgeBlend);
	}
	else if (NormalizedPosition > (1.0f - EdgeBlend))
	{
		// Fade out to upper edge
		return FMath::SmoothStep(0.0f, 1.0f, (1.0f - NormalizedPosition) / EdgeBlend);
	}

	return 1.0f; // Full weight in middle of range
}

float UTerrainMaterialSystem::CalculateAltitudeInfluence(float Altitude, const FVector2D& AltitudeRange) const
{
	if (Altitude < AltitudeRange.X || Altitude > AltitudeRange.Y)
	{
		return 0.0f;
	}

	float RangeSize = AltitudeRange.Y - AltitudeRange.X;
	if (RangeSize <= 0.0f)
	{
		return 1.0f;
	}

	// Calculate position within range
	float NormalizedPosition = (Altitude - AltitudeRange.X) / RangeSize;

	// Use smoothstep for smooth transitions at edges
	float EdgeBlend = 0.15f; // 15% of range for edge blending

	if (NormalizedPosition < EdgeBlend)
	{
		// Fade in from lower edge
		return FMath::SmoothStep(0.0f, 1.0f, NormalizedPosition / EdgeBlend);
	}
	else if (NormalizedPosition > (1.0f - EdgeBlend))
	{
		// Fade out to upper edge
		return FMath::SmoothStep(0.0f, 1.0f, (1.0f - NormalizedPosition) / EdgeBlend);
	}

	return 1.0f; // Full weight in middle of range
}

float UTerrainMaterialSystem::CalculateMoistureInfluence(float Moisture, ETerrainMaterialSlot Slot) const
{
	float ClampedMoisture = FMath::Clamp(Moisture, 0.0f, 1.0f);

	switch (Slot)
	{
	case ETerrainMaterialSlot::Grass:
		// Grass prefers moderate to high moisture (0.4-0.8 optimal)
		if (ClampedMoisture < 0.4f)
		{
			return 0.5f + ClampedMoisture * 0.5f; // 0.5-0.7 for low moisture
		}
		else if (ClampedMoisture > 0.8f)
		{
			return 1.0f - (ClampedMoisture - 0.8f) * 0.5f; // 1.0-0.9 for high moisture
		}
		return 1.0f; // Optimal range

	case ETerrainMaterialSlot::Sand:
		// Sand prefers low moisture (0.0-0.3 optimal)
		return 1.0f - FMath::Clamp(ClampedMoisture / 0.5f, 0.0f, 1.0f);

	case ETerrainMaterialSlot::WetSurface:
		// Wet surface appears with high moisture (>0.7)
		return ClampedMoisture > 0.7f ? (ClampedMoisture - 0.7f) / 0.3f : 0.0f;

	case ETerrainMaterialSlot::DirtSoil:
		// Dirt/soil is neutral but slightly prefers moderate moisture
		return 0.8f + FMath::Abs(ClampedMoisture - 0.5f) * 0.4f;

	case ETerrainMaterialSlot::Snow:
		// Snow is less affected by moisture but slightly reduced in very wet conditions
		return ClampedMoisture > 0.8f ? 0.9f : 1.0f;

	default:
		// Other materials are neutral to moisture
		return 1.0f;
	}
}

float UTerrainMaterialSystem::CalculateTemperatureInfluence(float Temperature, ETerrainMaterialSlot Slot) const
{
	switch (Slot)
	{
	case ETerrainMaterialSlot::Snow:
		// Snow appears below 0°C, strongest below -10°C
		if (Temperature > 5.0f)
		{
			return 0.0f; // No snow above 5°C
		}
		else if (Temperature > 0.0f)
		{
			// Fade out between 0-5°C
			return 1.0f - (Temperature / 5.0f);
		}
		else if (Temperature < -10.0f)
		{
			return 1.0f; // Full snow below -10°C
		}
		// Blend between 0 and -10°C
		return 0.8f + FMath::Abs(Temperature / 10.0f) * 0.2f;

	case ETerrainMaterialSlot::Sand:
		// Sand prefers hot temperatures (>25°C optimal)
		if (Temperature < 15.0f)
		{
			return 0.6f; // Reduced in cold
		}
		else if (Temperature > 35.0f)
		{
			return 1.0f; // Maximum in very hot
		}
		// Linear increase from 15-35°C
		return 0.6f + ((Temperature - 15.0f) / 20.0f) * 0.4f;

	case ETerrainMaterialSlot::Grass:
		// Grass prefers moderate temperatures (10-30°C)
		if (Temperature < 0.0f || Temperature > 40.0f)
		{
			return 0.3f; // Minimal in extreme temperatures
		}
		else if (Temperature < 10.0f)
		{
			// Fade in from 0-10°C
			return 0.3f + (Temperature / 10.0f) * 0.7f;
		}
		else if (Temperature > 30.0f)
		{
			// Fade out from 30-40°C
			return 1.0f - ((Temperature - 30.0f) / 10.0f) * 0.7f;
		}
		return 1.0f; // Optimal range

	case ETerrainMaterialSlot::BaseRock:
	case ETerrainMaterialSlot::CliffRock:
		// Rock is largely unaffected by temperature
		return 1.0f;

	case ETerrainMaterialSlot::DirtSoil:
		// Soil slightly prefers moderate temperatures
		if (Temperature < -5.0f || Temperature > 35.0f)
		{
			return 0.8f; // Slightly reduced in extremes
		}
		return 1.0f;

	default:
		// Other materials are neutral to temperature
		return 1.0f;
	}
}

void UTerrainMaterialSystem::NormalizeLayerWeights(TMap<ETerrainMaterialSlot, float>& Weights) const
{
	// Calculate sum of all weights
	float TotalWeight = 0.0f;
	for (const auto& Pair : Weights)
	{
		TotalWeight += Pair.Value;
	}

	// Normalize if total is non-zero
	if (TotalWeight > 0.0f)
	{
		for (auto& Pair : Weights)
		{
			Pair.Value /= TotalWeight;
		}
	}
	else
	{
		// If all weights are zero, distribute evenly
		float EvenWeight = 1.0f / FMath::Max(Weights.Num(), 1);
		for (auto& Pair : Weights)
		{
			Pair.Value = EvenWeight;
		}
	}
}
