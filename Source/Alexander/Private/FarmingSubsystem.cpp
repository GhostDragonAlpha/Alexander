// Copyright Epic Games, Inc. All Rights Reserved.

#include "FarmingSubsystem.h"
#include "FarmPlot.h"
#include "CropDefinition.h"
#include "Planet.h"
#include "BiomeManager.h"
#include "BiomeCompatibilitySystem.h"
#include "BiomeSoilSystem.h"
#include "FarmingOptimizationSystem.h"
#include "PlanetConfiguration.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UFarmingSubsystem::UFarmingSubsystem()
	: MaxFarmingSlope(15.0f)
	, MinSoilQuality(0.2f)
	, MinWaterAvailability(0.1f)
	, BiomeCompatibilitySystem(nullptr)
	, BiomeSoilSystem(nullptr)
	, OptimizationSystem(nullptr)
{
}

void UFarmingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("FarmingSubsystem initialized"));

	// Initialize default crop database
	// This would typically be loaded from data assets
	ActiveFarms.Empty();
	CropDatabase.Empty();

	// Create and initialize biome compatibility system
	BiomeCompatibilitySystem = NewObject<UBiomeCompatibilitySystem>(this);
	if (BiomeCompatibilitySystem)
	{
		BiomeCompatibilitySystem->Initialize();
		UE_LOG(LogTemp, Log, TEXT("FarmingSubsystem: BiomeCompatibilitySystem initialized"));
	}

	// Create biome soil system (Task 14.2)
	BiomeSoilSystem = NewObject<UBiomeSoilSystem>(this);
	if (BiomeSoilSystem)
	{
		UE_LOG(LogTemp, Log, TEXT("FarmingSubsystem: BiomeSoilSystem created"));
	}

	// Create farming optimization system (Task 18.2)
	OptimizationSystem = NewObject<UFarmingOptimizationSystem>(this);
	if (OptimizationSystem)
	{
		OptimizationSystem->Initialize(GetWorld());
		UE_LOG(LogTemp, Log, TEXT("FarmingSubsystem: OptimizationSystem initialized"));
	}
}

void UFarmingSubsystem::Deinitialize()
{
	// Shutdown optimization system
	if (OptimizationSystem)
	{
		OptimizationSystem->Shutdown();
	}

	// Clean up all farm plots
	for (AFarmPlot* Farm : ActiveFarms)
	{
		if (Farm && !Farm->IsPendingKill())
		{
			Farm->Destroy();
		}
	}
	ActiveFarms.Empty();
	CropDatabase.Empty();

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("FarmingSubsystem deinitialized"));
}

void UFarmingSubsystem::Tick(float DeltaTime)
{
	// Get player location for optimization
	FVector PlayerLocation = FVector::ZeroVector;
	if (GetWorld() && GetWorld()->GetFirstPlayerController())
	{
		APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
		if (PlayerPawn)
		{
			PlayerLocation = PlayerPawn->GetActorLocation();
		}
	}

	// Update optimization system
	if (OptimizationSystem)
	{
		OptimizationSystem->Update(DeltaTime, PlayerLocation);

		// Get farms that need updating this frame
		TArray<AFarmPlot*> FarmsToUpdate = OptimizationSystem->GetFarmsNeedingUpdate();

		// Only update farms that need it (optimized update frequency)
		for (AFarmPlot* Farm : FarmsToUpdate)
		{
if (Farm && Farm->IsValid())
			{
				// Farm will update its crops in its own Tick
				// This just ensures we're managing update frequency
			}
		}
	}

	// Update all active farms (cleanup pass)
	for (int32 i = ActiveFarms.Num() - 1; i >= 0; --i)
	{
		AFarmPlot* Farm = ActiveFarms[i];
		
		// Remove invalid or destroyed farms
		if (!Farm || !Farm->IsValid())
		{
			if (OptimizationSystem)
			{
				OptimizationSystem->UnregisterFarmPlot(Farm);
			}
			ActiveFarms.RemoveAt(i);
			continue;
		}

		// Farm plots update themselves via their Tick function
		// This is just for subsystem-level management
	}
}

TStatId UFarmingSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UFarmingSubsystem, STATGROUP_Tickables);
}

// ============================================================================
// FARM PLOT MANAGEMENT
// ============================================================================

AFarmPlot* UFarmingSubsystem::CreateFarmPlot(FVector Location, FVector2D Size, AActor* PlanetActor)
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("FarmingSubsystem: Cannot create farm plot - no world"));
		return nullptr;
	}

	// Validate location
	FFarmingLocationValidation Validation = ValidateFarmingLocation(Location, PlanetActor);
	if (!Validation.bIsValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("FarmingSubsystem: Location not suitable for farming - %s"), 
			*Validation.FailureReason);
		return nullptr;
	}

	// Spawn farm plot actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AFarmPlot* NewFarm = GetWorld()->SpawnActor<AFarmPlot>(AFarmPlot::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
	
	if (NewFarm)
	{
		// Initialize farm plot
		NewFarm->SetPlotSize(Size);
		NewFarm->SetPlanetActor(PlanetActor);
		NewFarm->SetInitialSoilQuality(Validation.SoilQuality);
		
		// Apply biome soil variation (Task 14.2)
		FSoilVariation SoilVar = GetSoilVariationAtLocation(Location, PlanetActor);
		NewFarm->ApplyBiomeSoilVariation(SoilVar);
		
		// Add to active farms
		ActiveFarms.Add(NewFarm);

		// Register with optimization system (Task 18.2)
		if (OptimizationSystem)
		{
			OptimizationSystem->RegisterFarmPlot(NewFarm);
		}

		UE_LOG(LogTemp, Log, TEXT("FarmingSubsystem: Created farm plot at %s (Size: %.1f x %.1f, Soil: %.2f, Biome: %s)"),
			*Location.ToString(), Size.X, Size.Y, Validation.SoilQuality, *Validation.BiomeName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FarmingSubsystem: Failed to spawn farm plot actor"));
	}

	return NewFarm;
}

void UFarmingSubsystem::RemoveFarmPlot(AFarmPlot* FarmPlot)
{
	if (!FarmPlot)
	{
		return;
	}

	// Unregister from optimization system
	if (OptimizationSystem)
	{
		OptimizationSystem->UnregisterFarmPlot(FarmPlot);
	}

	ActiveFarms.Remove(FarmPlot);
	
	if (FarmPlot->IsValid())
	{
		FarmPlot->Destroy();
	}

	UE_LOG(LogTemp, Log, TEXT("FarmingSubsystem: Removed farm plot"));
}

TArray<AFarmPlot*> UFarmingSubsystem::GetFarmsOnPlanet(AActor* PlanetActor) const
{
	TArray<AFarmPlot*> PlanetFarms;

	if (!PlanetActor)
	{
		return PlanetFarms;
	}

	for (AFarmPlot* Farm : ActiveFarms)
	{
		if (Farm && Farm->GetPlanetActor() == PlanetActor)
		{
			PlanetFarms.Add(Farm);
		}
	}

	return PlanetFarms;
}

// ============================================================================
// CROP DATABASE
// ============================================================================

void UFarmingSubsystem::RegisterCrop(const FString& CropID, UCropDefinition* CropDef, const TArray<FString>& SuitableBiomes)
{
	if (CropID.IsEmpty() || !CropDef)
	{
		UE_LOG(LogTemp, Warning, TEXT("FarmingSubsystem: Cannot register crop - invalid parameters"));
		return;
	}

	FCropDatabaseEntry Entry;
	Entry.CropID = CropID;
	Entry.CropDefinition = CropDef;
	Entry.SuitableBiomes = SuitableBiomes;
	Entry.MinSoilQuality = CropDef->SoilQualityRequirement;
	Entry.OptimalTemperature = (CropDef->OptimalTemperature.Min + CropDef->OptimalTemperature.Max) * 0.5f;
	Entry.OptimalHumidity = (CropDef->OptimalHumidity.Min + CropDef->OptimalHumidity.Max) * 0.5f;

	CropDatabase.Add(CropID, Entry);

	UE_LOG(LogTemp, Log, TEXT("FarmingSubsystem: Registered crop '%s' for biomes: %s"),
		*CropID, *FString::Join(SuitableBiomes, TEXT(", ")));
}

UCropDefinition* UFarmingSubsystem::GetCropDefinition(const FString& CropID) const
{
	const FCropDatabaseEntry* Entry = CropDatabase.Find(CropID);
	return Entry ? Entry->CropDefinition : nullptr;
}

TArray<UCropDefinition*> UFarmingSubsystem::GetCropsForBiome(const FString& BiomeName) const
{
	TArray<UCropDefinition*> SuitableCrops;

	for (const auto& Pair : CropDatabase)
	{
		const FCropDatabaseEntry& Entry = Pair.Value;
		if (Entry.SuitableBiomes.Contains(BiomeName))
		{
			SuitableCrops.Add(Entry.CropDefinition);
		}
	}

	return SuitableCrops;
}

TArray<FString> UFarmingSubsystem::GetAllCropIDs() const
{
	TArray<FString> CropIDs;
	CropDatabase.GetKeys(CropIDs);
	return CropIDs;
}

// ============================================================================
// LOCATION VALIDATION
// ============================================================================

FFarmingLocationValidation UFarmingSubsystem::ValidateFarmingLocation(FVector Location, AActor* PlanetActor)
{
	FFarmingLocationValidation Result;
	Result.bIsValid = false;

	if (!PlanetActor)
	{
		Result.FailureReason = TEXT("No planet actor provided");
		return Result;
	}

	// Check slope
	float SlopeAngle = 0.0f;
	if (!IsSlopeSuitable(Location, PlanetActor, SlopeAngle))
	{
		Result.FailureReason = FString::Printf(TEXT("Slope too steep (%.1f degrees, max %.1f)"), 
			SlopeAngle, MaxFarmingSlope);
		Result.SlopeAngle = SlopeAngle;
		return Result;
	}
	Result.SlopeAngle = SlopeAngle;

	// Check soil quality
	float SoilQuality = GetSoilQualityAtLocation(Location, PlanetActor);
	if (SoilQuality < MinSoilQuality)
	{
		Result.FailureReason = FString::Printf(TEXT("Soil quality too low (%.2f, min %.2f)"), 
			SoilQuality, MinSoilQuality);
		Result.SoilQuality = SoilQuality;
		return Result;
	}
	Result.SoilQuality = SoilQuality;

	// Check water availability
	float WaterAvailability = GetWaterAvailability(Location, PlanetActor);
	if (WaterAvailability < MinWaterAvailability)
	{
		Result.FailureReason = FString::Printf(TEXT("Insufficient water availability (%.2f, min %.2f)"), 
			WaterAvailability, MinWaterAvailability);
		Result.WaterAvailability = WaterAvailability;
		return Result;
	}
	Result.WaterAvailability = WaterAvailability;

	// Get biome information
	Result.BiomeName = GetBiomeAtLocation(Location, PlanetActor);

	// All checks passed
	Result.bIsValid = true;
	return Result;
}

bool UFarmingSubsystem::IsSlopeSuitable(FVector Location, AActor* PlanetActor, float& OutSlopeAngle)
{
	OutSlopeAngle = CalculateSlope(Location, PlanetActor);
	return OutSlopeAngle <= MaxFarmingSlope;
}

float UFarmingSubsystem::GetSoilQualityAtLocation(FVector Location, AActor* PlanetActor)
{
	// Get biome definition to determine base soil quality
	const FBiomeDefinition* BiomeDef = GetBiomeDefinitionAtLocation(Location, PlanetActor);
	
	if (BiomeDef)
	{
		// Base soil quality from biome
		float BaseSoilQuality = BiomeDef->BaseSoilQuality;
		
		// Add some variation based on location (using simple noise)
		FVector LocalPos = Location - PlanetActor->GetActorLocation();
		float NoiseValue = FMath::PerlinNoise2D(FVector2D(LocalPos.X * 0.001f, LocalPos.Y * 0.001f));
		float Variation = NoiseValue * 0.2f; // +/- 0.1 variation
		
		float FinalQuality = FMath::Clamp(BaseSoilQuality + Variation, 0.0f, 1.0f);
		return FinalQuality;
	}

	// Default soil quality if no biome found
	return 0.5f;
}

FString UFarmingSubsystem::GetBiomeAtLocation(FVector Location, AActor* PlanetActor)
{
	const FBiomeDefinition* BiomeDef = GetBiomeDefinitionAtLocation(Location, PlanetActor);
	return BiomeDef ? BiomeDef->BiomeName : TEXT("Unknown");
}

float UFarmingSubsystem::GetWaterAvailability(FVector Location, AActor* PlanetActor)
{
	// Get biome to determine base water availability
	const FBiomeDefinition* BiomeDef = GetBiomeDefinitionAtLocation(Location, PlanetActor);
	
	if (BiomeDef)
	{
		// Base water availability from biome humidity
		float BaseWater = BiomeDef->Humidity;
		
		// Adjust based on altitude (higher = less water)
		FVector LocalPos = Location - PlanetActor->GetActorLocation();
		float Distance = LocalPos.Size();
		
		APlanet* Planet = Cast<APlanet>(PlanetActor);
		if (Planet)
		{
			float PlanetRadiusCm = Planet->PlanetRadius * 100000.0f; // km to cm
			float Altitude = (Distance - PlanetRadiusCm) / 100.0f; // cm to meters
			
			// Reduce water availability by 10% per 1000m altitude
			float AltitudeFactor = FMath::Max(0.0f, 1.0f - (Altitude / 10000.0f));
			BaseWater *= AltitudeFactor;
		}
		
		return FMath::Clamp(BaseWater, 0.0f, 1.0f);
	}

	// Default water availability
	return 0.5f;
}

// ============================================================================
// PRIVATE HELPERS
// ============================================================================

float UFarmingSubsystem::CalculateSlope(FVector Location, AActor* PlanetActor)
{
	if (!PlanetActor || !GetWorld())
	{
		return 0.0f;
	}

	// Calculate slope by sampling nearby terrain heights
	// For a sphere-based planet, we calculate the angle from the planet center
	
	FVector PlanetCenter = PlanetActor->GetActorLocation();
	FVector ToPlanet = (Location - PlanetCenter).GetSafeNormal();
	
	// Sample points around the location
	float SampleDistance = 100.0f; // 1 meter
	FVector Right = FVector::CrossProduct(ToPlanet, FVector::UpVector).GetSafeNormal();
	FVector Forward = FVector::CrossProduct(Right, ToPlanet).GetSafeNormal();
	
	// Sample 4 points around the location
	TArray<FVector> SamplePoints;
	SamplePoints.Add(Location + Forward * SampleDistance);
	SamplePoints.Add(Location - Forward * SampleDistance);
	SamplePoints.Add(Location + Right * SampleDistance);
	SamplePoints.Add(Location - Right * SampleDistance);
	
	// Perform line traces to get terrain heights
	float MaxHeightDiff = 0.0f;
	
	for (const FVector& SamplePoint : SamplePoints)
	{
		FHitResult HitResult;
		FVector TraceStart = SamplePoint + ToPlanet * 1000.0f;
		FVector TraceEnd = SamplePoint - ToPlanet * 1000.0f;
		
		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
		{
			float HeightDiff = FMath::Abs((HitResult.Location - Location).Size());
			MaxHeightDiff = FMath::Max(MaxHeightDiff, HeightDiff);
		}
	}
	
	// Calculate slope angle from height difference
	float SlopeAngle = FMath::RadiansToDegrees(FMath::Atan2(MaxHeightDiff, SampleDistance));
	
	return SlopeAngle;
}

const FBiomeDefinition* UFarmingSubsystem::GetBiomeDefinitionAtLocation(FVector Location, AActor* PlanetActor)
{
	APlanet* Planet = Cast<APlanet>(PlanetActor);
	if (!Planet || !Planet->BiomeManager)
	{
		return nullptr;
	}

	// Get biome index at location
	int32 BiomeIndex = Planet->GetBiomeAtLocation(Location);
	if (BiomeIndex < 0)
	{
		return nullptr;
	}

	// Get biome definition from manager
	const FBiomeDefinition& BiomeDef = Planet->BiomeManager->GetBiomeDefinition(BiomeIndex);
	return &BiomeDef;
}

// ============================================================================
// BIOME COMPATIBILITY
// ============================================================================

TArray<UCropDefinition*> UFarmingSubsystem::GetCompatibleCropsForBiome(EBiomeType BiomeType) const
{
	if (!BiomeCompatibilitySystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("FarmingSubsystem: BiomeCompatibilitySystem not initialized"));
		return TArray<UCropDefinition*>();
	}

	return BiomeCompatibilitySystem->GetCropsForBiome(BiomeType);
}

float UFarmingSubsystem::CalculateCropCompatibility(UCropDefinition* CropDef, FVector Location, AActor* PlanetActor)
{
	if (!BiomeCompatibilitySystem || !CropDef || !PlanetActor)
	{
		return 0.0f;
	}

	// Get environmental conditions at location
	const FBiomeDefinition* BiomeDef = GetBiomeDefinitionAtLocation(Location, PlanetActor);
	if (!BiomeDef)
	{
		return 0.0f;
	}

	float Temperature = BiomeDef->Temperature;
	float Humidity = BiomeDef->Humidity;
	float SoilQuality = GetSoilQualityAtLocation(Location, PlanetActor);

	// Calculate compatibility score
	FBiomeCompatibilityScore Score = BiomeCompatibilitySystem->CalculateCompatibilityScore(
		CropDef, BiomeDef->BiomeType, Temperature, Humidity, SoilQuality);

	return Score.CompatibilityScore;
}

TArray<UCropDefinition*> UFarmingSubsystem::GetRecommendedCropsForLocation(FVector Location, AActor* PlanetActor, int32 MaxResults)
{
	if (!BiomeCompatibilitySystem || !PlanetActor)
	{
		return TArray<UCropDefinition*>();
	}

	// Get environmental conditions at location
	const FBiomeDefinition* BiomeDef = GetBiomeDefinitionAtLocation(Location, PlanetActor);
	if (!BiomeDef)
	{
		return TArray<UCropDefinition*>();
	}

	float Temperature = BiomeDef->Temperature;
	float Humidity = BiomeDef->Humidity;
	float SoilQuality = GetSoilQualityAtLocation(Location, PlanetActor);

	// Get recommended crops from compatibility system
	return BiomeCompatibilitySystem->GetRecommendedCrops(
		BiomeDef->BiomeType, Temperature, Humidity, SoilQuality, MaxResults);
}

// ============================================================================
// SOIL SYSTEM (Task 14.2)
// ============================================================================

FSoilVariation UFarmingSubsystem::GetSoilVariationAtLocation(FVector Location, AActor* PlanetActor)
{
	FSoilVariation DefaultVariation;

	if (!BiomeSoilSystem || !PlanetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("FarmingSubsystem: Cannot get soil variation - missing BiomeSoilSystem or PlanetActor"));
		return DefaultVariation;
	}

	// Get biome definition at location
	const FBiomeDefinition* BiomeDef = GetBiomeDefinitionAtLocation(Location, PlanetActor);
	if (!BiomeDef)
	{
		UE_LOG(LogTemp, Warning, TEXT("FarmingSubsystem: No biome found at location"));
		return DefaultVariation;
	}

	// Initialize soil system with planet's biomes if not already done
	APlanet* Planet = Cast<APlanet>(PlanetActor);
	if (Planet && Planet->Configuration)
	{
		BiomeSoilSystem->Initialize(Planet->Configuration->Biomes);
	}

	// Get soil variation from biome definition
	FSoilVariation SoilVar = BiomeSoilSystem->GetSoilVariationFromDefinition(*BiomeDef);

	// Add local variation based on location
	FVector LocalPos = Location - PlanetActor->GetActorLocation();
	int32 LocationSeed = FMath::FloorToInt(LocalPos.X + LocalPos.Y * 1000.0f + LocalPos.Z * 1000000.0f);
	SoilVar.SoilQuality = BiomeSoilSystem->CalculateSoilQualityWithVariation(SoilVar.SoilQuality, LocationSeed);

	return SoilVar;
}
