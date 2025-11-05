// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlanetCloudComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProceduralNoiseGenerator.h"
#include "DayNightCycleComponent.h"

UPlanetCloudComponent::UPlanetCloudComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	// Create volumetric cloud component
	VolumetricCloudComponent = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricCloudComponent"));
	if (VolumetricCloudComponent)
	{
		VolumetricCloudComponent->SetupAttachment(this);
	}
}

void UPlanetCloudComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize targets
	TargetCoverage = CloudCoverage;
	TargetDensity = CloudDensity;
	BaseWindDirection = WindDirection.GetSafeNormal();
	
	// Try to find day-night cycle component on owner
	if (AActor* Owner = GetOwner())
	{
		DayNightCycleComponent = Owner->FindComponentByClass<UDayNightCycleComponent>();
		if (DayNightCycleComponent)
		{
			UE_LOG(LogTemp, Log, TEXT("PlanetCloudComponent: Found DayNightCycleComponent for sun direction"));
		}
	}
	
	// Initialize volumetric cloud rendering
	InitializeVolumetricClouds();
}

void UPlanetCloudComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Update cloud movement based on wind
	UpdateCloudMovement(DeltaTime);
	
	// Update wind variation
	UpdateWindVariation(DeltaTime);
	
	// Smooth transitions for coverage and density
	UpdateTransitions(DeltaTime);
	
	// Update material parameters
	UpdateCloudMaterialParameters();
}

void UPlanetCloudComponent::InitializeVolumetricClouds()
{
	if (!VolumetricCloudComponent || !bEnableVolumetricClouds)
	{
		return;
	}

	// Configure volumetric cloud component
	VolumetricCloudComponent->SetLayerBottomAltitude(CloudLayerBottomAltitude);
	VolumetricCloudComponent->SetLayerHeight(CloudLayerThickness);
	
	// Set rendering quality
	VolumetricCloudComponent->SetViewSampleCountScale(ViewSampleScale);
	VolumetricCloudComponent->SetShadowViewSampleCountScale(ShadowSampleScale);
	VolumetricCloudComponent->SetShadowTracingDistance(ShadowTracingDistance);

	// Create dynamic material instance if cloud material exists
	if (VolumetricCloudComponent->GetMaterial())
	{
		CloudMaterialInstance = VolumetricCloudComponent->CreateDynamicMaterialInstance();
		if (CloudMaterialInstance)
		{
			UpdateCloudMaterialParameters();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("PlanetCloudComponent: Volumetric clouds initialized at altitude %.2f km with thickness %.2f km"), 
		CloudLayerBottomAltitude, CloudLayerThickness);
}

void UPlanetCloudComponent::UpdateCloudMovement(float DeltaTime)
{
	if (!bEnableVolumetricClouds)
	{
		return;
	}

	// Calculate wind offset based on wind speed and direction
	// Convert wind speed from m/s to km per frame
	float WindSpeedKmPerSecond = WindSpeed * 0.001f; // m/s to km/s
	FVector2D WindVelocity = WindDirection * WindSpeedKmPerSecond;
	
	// Accumulate wind offset
	CurrentWindOffset += WindVelocity * DeltaTime;
	
	// Wrap offset to prevent floating point precision issues
	// Wrap at 1000 km to maintain precision
	const float WrapDistance = 1000.0f;
	if (CurrentWindOffset.X > WrapDistance) CurrentWindOffset.X -= WrapDistance;
	if (CurrentWindOffset.X < -WrapDistance) CurrentWindOffset.X += WrapDistance;
	if (CurrentWindOffset.Y > WrapDistance) CurrentWindOffset.Y -= WrapDistance;
	if (CurrentWindOffset.Y < -WrapDistance) CurrentWindOffset.Y += WrapDistance;
}

void UPlanetCloudComponent::UpdateWindVariation(float DeltaTime)
{
	if (WindVariation <= 0.0f || WindVariationFrequency <= 0.0f)
	{
		WindDirection = BaseWindDirection;
		return;
	}

	// Update variation time
	WindVariationTime += DeltaTime * WindVariationFrequency;
	
	// Calculate wind variation using sine waves for smooth changes
	float AngleVariation = FMath::Sin(WindVariationTime) * WindVariation * PI * 0.25f; // Max 45 degree variation
	float SpeedVariation = FMath::Cos(WindVariationTime * 1.3f) * WindVariation; // Different frequency for speed
	
	// Apply angle variation to wind direction
	float BaseAngle = FMath::Atan2(BaseWindDirection.Y, BaseWindDirection.X);
	float NewAngle = BaseAngle + AngleVariation;
	WindDirection = FVector2D(FMath::Cos(NewAngle), FMath::Sin(NewAngle));
	
	// Apply speed variation (keep wind speed positive)
	float VariedSpeed = WindSpeed * (1.0f + SpeedVariation * 0.5f);
	WindSpeed = FMath::Max(0.1f, VariedSpeed);
}

void UPlanetCloudComponent::UpdateTransitions(float DeltaTime)
{
	// Smooth coverage transition
	if (FMath::Abs(CloudCoverage - TargetCoverage) > 0.001f)
	{
		if (CoverageTransitionSpeed > 0.0f)
		{
			CloudCoverage = FMath::FInterpTo(CloudCoverage, TargetCoverage, DeltaTime, CoverageTransitionSpeed);
		}
		else
		{
			CloudCoverage = TargetCoverage;
		}
	}
	
	// Smooth density transition
	if (FMath::Abs(CloudDensity - TargetDensity) > 0.001f)
	{
		if (DensityTransitionSpeed > 0.0f)
		{
			CloudDensity = FMath::FInterpTo(CloudDensity, TargetDensity, DeltaTime, DensityTransitionSpeed);
		}
		else
		{
			CloudDensity = TargetDensity;
		}
	}
}

void UPlanetCloudComponent::UpdateCloudMaterialParameters()
{
	if (!CloudMaterialInstance || !bEnableVolumetricClouds)
	{
		return;
	}

	// Update cloud appearance parameters
	CloudMaterialInstance->SetScalarParameterValue(FName("CloudCoverage"), CloudCoverage);
	CloudMaterialInstance->SetScalarParameterValue(FName("CloudDensity"), CloudDensity);
	CloudMaterialInstance->SetScalarParameterValue(FName("CloudExtinction"), CloudExtinction);
	
	// Update wind offset for noise scrolling
	CloudMaterialInstance->SetVectorParameterValue(FName("WindOffset"), 
		FLinearColor(CurrentWindOffset.X, CurrentWindOffset.Y, 0.0f, 0.0f));
	
	// Update noise parameters
	CloudMaterialInstance->SetScalarParameterValue(FName("NoiseScale"), NoiseScale);
	CloudMaterialInstance->SetScalarParameterValue(FName("DetailNoiseScale"), DetailNoiseScale);
	CloudMaterialInstance->SetScalarParameterValue(FName("DetailStrength"), DetailStrength);
	CloudMaterialInstance->SetScalarParameterValue(FName("CloudSeed"), static_cast<float>(CloudSeed));
	
	// Update lighting parameters
	CloudMaterialInstance->SetScalarParameterValue(FName("ScatteringIntensity"), ScatteringIntensity);
	CloudMaterialInstance->SetScalarParameterValue(FName("PhaseG"), PhaseG);
	CloudMaterialInstance->SetScalarParameterValue(FName("MultiScatteringContribution"), MultiScatteringContribution);
	CloudMaterialInstance->SetScalarParameterValue(FName("MultiScatteringOcclusion"), MultiScatteringOcclusion);
	
	// Update shadow parameters (Task 7.3)
	CloudMaterialInstance->SetScalarParameterValue(FName("LightRayMarchSteps"), static_cast<float>(LightRayMarchSteps));
	CloudMaterialInstance->SetScalarParameterValue(FName("ShadowStepSize"), ShadowStepSize);
	CloudMaterialInstance->SetScalarParameterValue(FName("ShadowDensityMultiplier"), ShadowDensityMultiplier);
	CloudMaterialInstance->SetScalarParameterValue(FName("AmbientLightIntensity"), AmbientLightIntensity);
	CloudMaterialInstance->SetScalarParameterValue(FName("TerrainShadowIntensity"), TerrainShadowIntensity);
	CloudMaterialInstance->SetScalarParameterValue(FName("SelfShadowIntensity"), SelfShadowIntensity);
	CloudMaterialInstance->SetVectorParameterValue(FName("AmbientLightColor"), AmbientLightColor);
	
	// Update sun direction if available
	FVector SunDir = GetSunDirection();
	CloudMaterialInstance->SetVectorParameterValue(FName("SunDirection"), 
		FLinearColor(SunDir.X, SunDir.Y, SunDir.Z, 0.0f));
	
	// Update cloud color
	CloudMaterialInstance->SetVectorParameterValue(FName("CloudAlbedo"), CloudAlbedo);
}

void UPlanetCloudComponent::SetWeatherCoverage(float Coverage, float TransitionTime)
{
	TargetCoverage = FMath::Clamp(Coverage, 0.0f, 1.0f);
	
	if (TransitionTime > 0.0f)
	{
		CoverageTransitionSpeed = 1.0f / TransitionTime;
	}
	else
	{
		CloudCoverage = TargetCoverage;
		CoverageTransitionSpeed = 0.0f;
	}

	UE_LOG(LogTemp, Log, TEXT("PlanetCloudComponent: Transitioning cloud coverage to %.2f over %.2f seconds"), 
		TargetCoverage, TransitionTime);
}

void UPlanetCloudComponent::SetCloudDensity(float Density, float TransitionTime)
{
	TargetDensity = FMath::Clamp(Density, 0.0f, 1.0f);
	
	if (TransitionTime > 0.0f)
	{
		DensityTransitionSpeed = 1.0f / TransitionTime;
	}
	else
	{
		CloudDensity = TargetDensity;
		DensityTransitionSpeed = 0.0f;
	}

	UE_LOG(LogTemp, Log, TEXT("PlanetCloudComponent: Transitioning cloud density to %.2f over %.2f seconds"), 
		TargetDensity, TransitionTime);
}

void UPlanetCloudComponent::SetWindParameters(FVector2D Direction, float Speed)
{
	BaseWindDirection = Direction.GetSafeNormal();
	WindDirection = BaseWindDirection;
	WindSpeed = FMath::Max(0.0f, Speed);

	UE_LOG(LogTemp, Log, TEXT("PlanetCloudComponent: Wind set to direction (%.2f, %.2f) at %.2f m/s"), 
		WindDirection.X, WindDirection.Y, WindSpeed);
}

// ============================================================================
// CLOUD SHAPE GENERATION (Task 7.2)
// ============================================================================

float UPlanetCloudComponent::GenerateBaseCloudShape(FVector Position)
{
	// Apply wind offset to position for cloud movement
	FVector AnimatedPosition = Position;
	AnimatedPosition.X += CurrentWindOffset.X;
	AnimatedPosition.Y += CurrentWindOffset.Y;

	// Generate base cloud shape using multi-octave Perlin-Worley noise
	// This creates the large-scale cloud formations
	float BaseShape = UProceduralNoiseGenerator::FractalPerlinWorleyNoise3D(
		AnimatedPosition.X,
		AnimatedPosition.Y,
		AnimatedPosition.Z,
		CloudSeed,
		BaseShapeOctaves,
		BaseShapeFrequency * NoiseScale,
		BaseShapeLacunarity,
		BaseShapePersistence,
		WorleyWeight
	);

	return BaseShape;
}

float UPlanetCloudComponent::GenerateCloudDetail(FVector Position)
{
	// Apply wind offset with slightly different speed for detail layer
	FVector AnimatedPosition = Position;
	AnimatedPosition.X += CurrentWindOffset.X * 1.2f; // Detail moves slightly faster
	AnimatedPosition.Y += CurrentWindOffset.Y * 1.2f;

	// Generate detail noise using higher frequency Perlin-Worley
	// This adds fine details and wispy edges to clouds
	float Detail = UProceduralNoiseGenerator::FractalPerlinWorleyNoise3D(
		AnimatedPosition.X,
		AnimatedPosition.Y,
		AnimatedPosition.Z,
		CloudSeed + 5000, // Different seed for detail
		DetailOctaves,
		DetailFrequency * DetailNoiseScale,
		DetailLacunarity,
		DetailPersistence,
		WorleyWeight * 0.7f // Less Worley influence for detail
	);

	return Detail;
}

float UPlanetCloudComponent::CalculateCloudDensity(FVector Position, float HeightInLayer)
{
	// Generate base cloud shape
	float BaseShape = GenerateBaseCloudShape(Position);

	// Generate detail noise
	float Detail = GenerateCloudDetail(Position);

	// Combine base shape with detail
	// Detail erodes the base shape, creating wispy edges
	float CombinedShape = BaseShape - (Detail * DetailStrength);

	// Apply coverage pattern
	FVector2D CoveragePos(Position.X, Position.Y);
	float Coverage = GenerateCoveragePattern(CoveragePos);
	
	// Modulate density by coverage
	float ModulatedDensity = CombinedShape * FMath::Lerp(0.5f, 1.0f, Coverage);

	// Apply height gradient if enabled
	if (bUseHeightGradient)
	{
		// Create gradient that fades clouds at top and bottom of layer
		float HeightGradient = 1.0f;
		
		// Fade at bottom
		if (HeightInLayer < 0.2f)
		{
			HeightGradient *= FMath::Lerp(HeightGradientBottom, 1.0f, HeightInLayer / 0.2f);
		}
		
		// Fade at top
		if (HeightInLayer > 0.8f)
		{
			HeightGradient *= FMath::Lerp(1.0f, HeightGradientTop, (HeightInLayer - 0.8f) / 0.2f);
		}
		
		ModulatedDensity *= HeightGradient;
	}

	// Apply density threshold and multiplier
	float FinalDensity = (ModulatedDensity - DensityThreshold) * DensityMultiplier;
	
	// Apply global cloud density parameter
	FinalDensity *= CloudDensity;

	// Clamp to valid range
	return FMath::Clamp(FinalDensity, 0.0f, 1.0f);
}

float UPlanetCloudComponent::GenerateCoveragePattern(FVector2D Position)
{
	// Generate large-scale coverage patterns using fractal noise
	// This creates areas of more/less cloud coverage
	float CoverageNoise = 0.0f;
	float Amplitude = 1.0f;
	float Frequency = CoverageFrequency;
	float MaxValue = 0.0f;

	for (int32 Octave = 0; Octave < CoverageOctaves; ++Octave)
	{
		float NoiseValue = UProceduralNoiseGenerator::PerlinNoise2D(
			Position.X * Frequency,
			Position.Y * Frequency,
			CloudSeed + 10000 + Octave
		);

		// Normalize from [-1, 1] to [0, 1]
		NoiseValue = (NoiseValue + 1.0f) * 0.5f;

		CoverageNoise += NoiseValue * Amplitude;
		MaxValue += Amplitude;

		Amplitude *= 0.5f;
		Frequency *= 2.0f;
	}

	CoverageNoise /= MaxValue;

	// Blend with global coverage parameter
	// CoverageVariation controls how much the pattern affects coverage
	float FinalCoverage = FMath::Lerp(
		CloudCoverage,
		CoverageNoise,
		CoverageVariation
	);

	return FMath::Clamp(FinalCoverage, 0.0f, 1.0f);
}

float UPlanetCloudComponent::SampleCloudDensityAtPosition(FVector WorldPosition)
{
	// Convert world position to cloud layer space
	FVector ComponentLocation = GetComponentLocation();
	FVector RelativePosition = WorldPosition - ComponentLocation;
	
	// Calculate altitude above surface (assuming component is at planet center)
	float Altitude = RelativePosition.Size() * 0.001f; // Convert to km
	
	// Check if position is within cloud layer
	float CloudBottom = CloudLayerBottomAltitude;
	float CloudTop = CloudLayerBottomAltitude + CloudLayerThickness;
	
	if (Altitude < CloudBottom || Altitude > CloudTop)
	{
		return 0.0f; // Outside cloud layer
	}

	// Calculate normalized height in cloud layer [0, 1]
	float HeightInLayer = (Altitude - CloudBottom) / CloudLayerThickness;

	// Convert to cloud space coordinates (scale to reasonable noise space)
	FVector CloudSpacePosition = RelativePosition * 0.001f; // Convert to km

	// Calculate cloud density
	return CalculateCloudDensity(CloudSpacePosition, HeightInLayer);
}

// ============================================================================
// CLOUD LIGHTING AND SHADOWS (Task 7.3)
// ============================================================================

float UPlanetCloudComponent::CalculateCloudLighting(FVector Position, float HeightInLayer, FVector SunDirection)
{
	if (!bEnableVolumetricClouds)
	{
		return 1.0f;
	}

	// Normalize sun direction
	SunDirection.Normalize();

	// Calculate base cloud density at this position
	float CloudDensity = CalculateCloudDensity(Position, HeightInLayer);
	
	if (CloudDensity < 0.01f)
	{
		return 1.0f; // No cloud, fully lit
	}

	// Ray march toward sun to calculate shadow
	float ShadowFactor = RayMarchShadow(Position, SunDirection, ShadowTracingDistance);

	// Apply self-shadow intensity
	ShadowFactor = FMath::Lerp(1.0f, ShadowFactor, SelfShadowIntensity);

	// Add ambient light contribution
	float AmbientContribution = AmbientLightIntensity;
	
	// Combine direct and ambient lighting
	float TotalLight = FMath::Max(ShadowFactor, AmbientContribution);

	// Apply phase function for forward/back scattering
	// Assume view direction is roughly opposite to sun for this calculation
	float CosAngle = -1.0f; // Looking toward sun
	float PhaseValue = HenyeyGreensteinPhase(CosAngle, PhaseG);
	
	// Modulate by phase function
	TotalLight *= FMath::Lerp(1.0f, PhaseValue, 0.5f);

	// Add multi-scattering approximation
	// Multi-scattering brightens dense clouds
	float MultiScatter = MultiScatteringContribution * (1.0f - FMath::Exp(-CloudDensity * 2.0f));
	MultiScatter *= (1.0f - MultiScatteringOcclusion * (1.0f - ShadowFactor));
	TotalLight += MultiScatter;

	return FMath::Clamp(TotalLight, 0.0f, 1.0f);
}

float UPlanetCloudComponent::RayMarchShadow(FVector StartPosition, FVector SunDirection, float MaxDistance)
{
	if (LightRayMarchSteps <= 0)
	{
		return 1.0f;
	}

	// Normalize sun direction
	SunDirection.Normalize();

	// Calculate step size
	float StepSize = ShadowStepSize;
	int32 NumSteps = FMath::Min(LightRayMarchSteps, FMath::CeilToInt(MaxDistance / StepSize));
	
	if (NumSteps <= 0)
	{
		return 1.0f;
	}

	// Accumulate optical depth (density along ray)
	float OpticalDepth = 0.0f;
	FVector CurrentPosition = StartPosition;

	// March toward sun
	for (int32 Step = 0; Step < NumSteps; ++Step)
	{
		// Move along ray
		CurrentPosition += SunDirection * StepSize;

		// Calculate height in cloud layer for this position
		// Approximate altitude based on Z coordinate
		float Altitude = CurrentPosition.Z; // Already in km
		float CloudBottom = CloudLayerBottomAltitude;
		float CloudTop = CloudLayerBottomAltitude + CloudLayerThickness;

		// Check if still in cloud layer
		if (Altitude < CloudBottom || Altitude > CloudTop)
		{
			break; // Exited cloud layer, no more shadowing
		}

		// Calculate normalized height in layer
		float HeightInLayer = (Altitude - CloudBottom) / CloudLayerThickness;

		// Sample cloud density at this position
		float Density = CalculateCloudDensity(CurrentPosition, HeightInLayer);

		// Accumulate optical depth
		OpticalDepth += Density * StepSize;
	}

	// Convert optical depth to transmittance using Beer-Lambert law
	// Transmittance = exp(-optical_depth * extinction)
	float Extinction = CloudExtinction * ShadowDensityMultiplier;
	float Transmittance = FMath::Exp(-OpticalDepth * Extinction);

	// Transmittance is the shadow factor (1 = no shadow, 0 = full shadow)
	return FMath::Clamp(Transmittance, 0.0f, 1.0f);
}

float UPlanetCloudComponent::HenyeyGreensteinPhase(float CosAngle, float G)
{
	// Henyey-Greenstein phase function for anisotropic scattering
	// P(cos θ) = (1 - g²) / (4π * (1 + g² - 2g*cos θ)^(3/2))
	// Simplified without the 4π normalization factor
	
	float G2 = G * G;
	float Denominator = 1.0f + G2 - 2.0f * G * CosAngle;
	Denominator = FMath::Pow(FMath::Max(Denominator, 0.001f), 1.5f);
	
	float Phase = (1.0f - G2) / Denominator;
	
	return Phase;
}

float UPlanetCloudComponent::CalculateTerrainShadow(FVector TerrainPosition, FVector SunDirection)
{
	if (!bEnableVolumetricClouds || TerrainShadowIntensity <= 0.0f)
	{
		return 0.0f; // No shadow
	}

	// Normalize sun direction
	SunDirection.Normalize();

	// Check if sun is below horizon
	if (SunDirection.Z <= 0.0f)
	{
		return 0.0f; // Sun below horizon, no cloud shadows
	}

	// Convert terrain position to cloud space
	FVector CloudSpacePosition = WorldToCloudSpace(TerrainPosition);

	// Calculate the intersection point with cloud layer
	// Ray from terrain upward toward sun
	float TerrainAltitude = CloudSpacePosition.Z;
	float CloudBottom = CloudLayerBottomAltitude;
	float CloudTop = CloudLayerBottomAltitude + CloudLayerThickness;

	// If terrain is above clouds, no shadow
	if (TerrainAltitude > CloudTop)
	{
		return 0.0f;
	}

	// Calculate distance to cloud layer bottom
	float DistanceToCloudBottom = (CloudBottom - TerrainAltitude) / FMath::Max(SunDirection.Z, 0.001f);
	
	// Calculate entry point into cloud layer
	FVector CloudEntryPoint = CloudSpacePosition + SunDirection * DistanceToCloudBottom;

	// Ray march through cloud layer
	float OpticalDepth = 0.0f;
	FVector CurrentPosition = CloudEntryPoint;
	float StepSize = ShadowStepSize;
	int32 NumSteps = FMath::CeilToInt(CloudLayerThickness / StepSize);

	for (int32 Step = 0; Step < NumSteps; ++Step)
	{
		float Altitude = CurrentPosition.Z;
		
		// Check if still in cloud layer
		if (Altitude > CloudTop)
		{
			break;
		}

		// Calculate normalized height in layer
		float HeightInLayer = (Altitude - CloudBottom) / CloudLayerThickness;

		// Sample cloud density
		float Density = CalculateCloudDensity(CurrentPosition, HeightInLayer);

		// Accumulate optical depth
		OpticalDepth += Density * StepSize;

		// Move to next position
		CurrentPosition += SunDirection * StepSize;
	}

	// Convert optical depth to shadow intensity
	float Extinction = CloudExtinction * ShadowDensityMultiplier;
	float Transmittance = FMath::Exp(-OpticalDepth * Extinction);
	
	// Shadow intensity is inverse of transmittance
	float ShadowIntensity = (1.0f - Transmittance) * TerrainShadowIntensity;

	return FMath::Clamp(ShadowIntensity, 0.0f, 1.0f);
}

FVector UPlanetCloudComponent::GetSunDirection()
{
	if (DayNightCycleComponent)
	{
		return DayNightCycleComponent->CalculateSunDirection();
	}

	// Default sun direction (noon, pointing down)
	return FVector(0.0f, 0.0f, -1.0f);
}

void UPlanetCloudComponent::SetDayNightCycleComponent(UDayNightCycleComponent* DayNightCycle)
{
	DayNightCycleComponent = DayNightCycle;
	
	if (DayNightCycleComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("PlanetCloudComponent: Day-night cycle component set for sun direction"));
	}
}

FVector UPlanetCloudComponent::WorldToCloudSpace(FVector WorldPosition) const
{
	// Convert world position to cloud space (relative to component, in km)
	FVector ComponentLocation = GetComponentLocation();
	FVector RelativePosition = WorldPosition - ComponentLocation;
	
	// Convert to km
	return RelativePosition * 0.001f;
}

bool UPlanetCloudComponent::IsInCloudLayer(FVector WorldPosition, float& OutHeightInLayer) const
{
	FVector CloudSpacePosition = WorldToCloudSpace(WorldPosition);
	
	// Calculate altitude
	float Altitude = CloudSpacePosition.Size();
	
	// Check if in cloud layer
	float CloudBottom = CloudLayerBottomAltitude;
	float CloudTop = CloudLayerBottomAltitude + CloudLayerThickness;
	
	if (Altitude < CloudBottom || Altitude > CloudTop)
	{
		OutHeightInLayer = 0.0f;
		return false;
	}

	// Calculate normalized height
	OutHeightInLayer = (Altitude - CloudBottom) / CloudLayerThickness;
	return true;
}
