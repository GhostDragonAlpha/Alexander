// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlanetaryFarmingSystem.h"
#include "FarmPlot.h"
//#include "HydroponicsBay.h"
//#include "Greenhouse.h" // Temporarily disabled - file not found
#include "AlexanderIrrigationSystem.h"
//#include "FertilizerInjector.h" // Temporarily disabled - file not found
//#include "HarvestingDrone.h" // Temporarily disabled - file not found
#include "Planet.h"
#include "LandClaimManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"

APlanetaryFarmingSystem::APlanetaryFarmingSystem()
{
	PrimaryActorTick.bCanEverTick = true;

	// Default configuration
	bAutoManagementEnabled = false;
	MaxFarmPlots = 100;
	WaterCostPerLiter = 0.01f;
	FertilizerCostPerKg = 2.0f;
	LaborCostPerHour = 15.0f;
}

void APlanetaryFarmingSystem::BeginPlay()
{
	Super::BeginPlay();

	// Initialize crop system
	InitializeCropSystem();
}

void APlanetaryFarmingSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Process all farm plots
	ProcessAllPlots(DeltaTime);

	// Update statistics periodically
	static float StatsAccumulator = 0.0f;
	StatsAccumulator += DeltaTime;

	if (StatsAccumulator >= 5.0f) // Update every 5 seconds
	{
		StatsAccumulator = 0.0f;
		UpdateFarmStatistics();
	}

	// Process auto-management
	if (bAutoManagementEnabled)
	{
		ProcessAutoManagement();
	}
}

void APlanetaryFarmingSystem::InitializeForPlanet(APlanet* Planet, ALandClaimManager* LandManager)
{
	if (!Planet || !LandManager)
	{
		UE_LOG(LogTemp, Error, TEXT("PlanetaryFarmingSystem: Invalid planet or land manager"));
		return;
	}

	TargetPlanet = Planet;
	LandClaimManager = LandManager;

	UE_LOG(LogTemp, Log, TEXT("PlanetaryFarmingSystem initialized for planet: %s"), *Planet->GetName());

	// Find suitable farming locations and create initial farm plots
	TArray<FVector> SuitableLocations = LandManager->FindSuitableFarmingLocations(10);
	
	for (const FVector& Location : SuitableLocations)
	{
		// Get the claim at this location
		FLandClaim Claim = LandManager->GetClaimAtLocation(Location);
		if (Claim.ClaimID.IsValid())
		{
			CreateFarmPlot(Location, Claim.ClaimID);
		}
	}
}

void APlanetaryFarmingSystem::InitializeCropSystem()
{
	if (!CropSystem)
	{
		CropSystem = NewObject<UCropGrowthSystem>(this);
	}

	if (CropSystem)
	{
		UE_LOG(LogTemp, Log, TEXT("Crop growth system initialized with %d crop types"), CropSystem->GetAllAvailableCrops().Num());
	}
}

FGuid APlanetaryFarmingSystem::CreateFarmPlot(FVector Location, const FGuid& ClaimID, FVector2D Size)
{
	// Validate claim
	if (LandClaimManager.IsValid())
	{
		FLandClaim Claim = LandClaimManager->GetClaimByID(ClaimID);
		if (!Claim.ClaimID.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid claim ID for farm plot creation"));
			return FGuid();
		}
	}

	// Create farm plot actor
	AFarmPlot* NewPlot = GetWorld()->SpawnActor<AFarmPlot>(AFarmPlot::StaticClass(), Location, FRotator::ZeroRotator);
	
	if (!NewPlot)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create farm plot actor"));
		return FGuid();
	}

	// Configure farm plot
	NewPlot->PlotSize = Size;
	NewPlot->ClaimID = ClaimID;
	NewPlot->PlotID = FGuid::NewGuid();

	// Add to collection
	FarmPlots.Add(NewPlot);
	FarmPlotIndexMap.Add(NewPlot->PlotID, FarmPlots.Num() - 1);

	// Fire event
	OnFarmPlotCreated.Broadcast(NewPlot->PlotID);

	UE_LOG(LogTemp, Log, TEXT("Farm plot created at location: %s"), *Location.ToString());
	return NewPlot->PlotID;
}

bool APlanetaryFarmingSystem::PlantCrops(const FGuid& PlotID, ECropTypeExtended CropType, float PlantingDensity)
{
	int32 PlotIndex = GetPlotIndex(PlotID);
	if (PlotIndex == INDEX_NONE)
	{
		return false;
	}

	AFarmPlot* Plot = FarmPlots[PlotIndex];
	if (!Plot)
	{
		return false;
	}

	// Get crop data
	FCropData CropData = CropSystem->GetCropData(CropType);
	
	// Create crop definition
	UCropDefinition* CropDef = NewObject<UCropDefinition>(this);
	CropDef->CropType = CropType;
	CropDef->CropName = CropData.CropName;
	CropDef->GrowthDuration = CropData.BaseGrowthTime;
	CropDef->WaterConsumption = CropData.WaterNeed;
	CropDef->NutrientRequirement = CropData.NutrientNeed;
	CropDef->OptimalTemperature = CropData.OptimalTemp;
	CropDef->TemperatureTolerance = CropData.TempTolerance;
	CropDef->ExpectedYield = CropData.BaseYield;

	// Plant crops in plot
	bool bSuccess = Plot->PlantCrop(FIntPoint(0, 0), CropDef, PlantingDensity);
	
	if (bSuccess)
	{
		// Create planting info
		FCropPlantingInfo PlantingInfo;
		PlantingInfo.CropType = CropType;
		PlantingInfo.PlotLocation = Plot->GetActorLocation();
		PlantingInfo.PlantingDensity = PlantingDensity;
		PlantingInfo.ExpectedHarvestTime = CropData.BaseGrowthTime;
		PlantingInfo.ExpectedYield = CalculateExpectedYield(CropType, Plot->SoilQuality, PlantingDensity);

		// Fire event
		OnCropPlanted.Broadcast(PlantingInfo, PlotID);

		UE_LOG(LogTemp, Log, TEXT("Crops planted: %s in plot %s"), *CropData.CropName, *PlotID.ToString());
	}

	return bSuccess;
}

FAlexanderHarvestResult APlanetaryFarmingSystem::HarvestCrops(const FGuid& PlotID)
{
	FAlexanderHarvestResult Result;
	Result.bSuccess = false;

	int32 PlotIndex = GetPlotIndex(PlotID);
	if (PlotIndex == INDEX_NONE)
	{
		return Result;
	}

	AFarmPlot* Plot = FarmPlots[PlotIndex];
	if (!Plot)
	{
		return Result;
	}

	// Harvest crops
	Result = Plot->HarvestAllCrops();
	
	if (Result.bSuccess)
	{
		// Fire event
		OnCropHarvested.Broadcast(Result, PlotID);

		UE_LOG(LogTemp, Log, TEXT("Crops harvested: %d units of %s"), Result.Quantity, *UEnum::GetDisplayValueAsText(TEXT("ECropTypeExtended"), Result.CropType).ToString());
	}

	return Result;
}

bool APlanetaryFarmingSystem::WaterPlot(const FGuid& PlotID, float WaterAmount)
{
	int32 PlotIndex = GetPlotIndex(PlotID);
	if (PlotIndex == INDEX_NONE)
	{
		return false;
	}

	AFarmPlot* Plot = FarmPlots[PlotIndex];
	if (!Plot)
	{
		return false;
	}

	bool bSuccess = Plot->WaterPlot(WaterAmount);
	
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Plot watered: %f liters"), WaterAmount);
	}

	return bSuccess;
}

bool APlanetaryFarmingSystem::FertilizePlot(const FGuid& PlotID, float FertilizerAmount, FName FertilizerType)
{
	int32 PlotIndex = GetPlotIndex(PlotID);
	if (PlotIndex == INDEX_NONE)
	{
		return false;
	}

	AFarmPlot* Plot = FarmPlots[PlotIndex];
	if (!Plot)
	{
		return false;
	}

	bool bSuccess = Plot->FertilizePlot(FertilizerAmount);
	
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Plot fertilized: %f kg of %s"), FertilizerAmount, *FertilizerType.ToString());
	}

	return bSuccess;
}

FGuid APlanetaryFarmingSystem::BuildInfrastructure(EFarmingInfrastructureType Type, FVector Location, const FGuid& ClaimID)
{
	// Validate claim
	if (LandClaimManager.IsValid())
	{
		FLandClaim Claim = LandClaimManager->GetClaimByID(ClaimID);
		if (!Claim.ClaimID.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid claim ID for infrastructure building"));
			return FGuid();
		}

		// Check if location is within claim bounds
		if (!Claim.Bounds.IsInside(Location))
		{
			UE_LOG(LogTemp, Warning, TEXT("Infrastructure location outside claim bounds"));
			return FGuid();
		}
	}

	// Spawn appropriate infrastructure actor
	AActor* NewInfrastructure = nullptr;
	FGuid InfrastructureID = FGuid::NewGuid();

	switch (Type)
	{
		case EFarmingInfrastructureType::HydroponicsBay:
			NewInfrastructure = GetWorld()->SpawnActor<AHydroponicsBay>(AHydroponicsBay::StaticClass(), Location, FRotator::ZeroRotator);
			break;

		case EFarmingInfrastructureType::Greenhouse:
			NewInfrastructure = GetWorld()->SpawnActor<AGreenhouse>(AGreenhouse::StaticClass(), Location, FRotator::ZeroRotator);
			break;

		case EFarmingInfrastructureType::IrrigationSystem:
			NewInfrastructure = GetWorld()->SpawnActor<AIrrigationSystem>(AIrrigationSystem::StaticClass(), Location, FRotator::ZeroRotator);
			break;

		case EFarmingInfrastructureType::FertilizerInjector:
			NewInfrastructure = GetWorld()->SpawnActor<AFertilizerInjector>(AFertilizerInjector::StaticClass(), Location, FRotator::ZeroRotator);
			break;

		case EFarmingInfrastructureType::HarvestingDrone:
			NewInfrastructure = GetWorld()->SpawnActor<AHarvestingDrone>(AHarvestingDrone::StaticClass(), Location, FRotator::ZeroRotator);
			break;

		default:
			UE_LOG(LogTemp, Warning, TEXT("Unsupported infrastructure type"));
			return FGuid();
	}

	if (NewInfrastructure)
	{
		// Add to infrastructure collection
		FarmingInfrastructure.Add(NewInfrastructure);
		InfrastructureByType[Type].Actors.Add(NewInfrastructure);

		// Associate with claim
		if (LandClaimManager.IsValid())
		{
			LandClaimManager->AddBuildingToClaim(ClaimID, InfrastructureID);
		}

		// Fire event
		OnInfrastructureBuilt.Broadcast(Type, Location);

		UE_LOG(LogTemp, Log, TEXT("Infrastructure built: %s at %s"), *UEnum::GetDisplayValueAsText(Type).ToString(), *Location.ToString());
	}

	return InfrastructureID;
}

AFarmPlot* APlanetaryFarmingSystem::GetFarmPlot(const FGuid& PlotID) const
{
	const int32* IndexPtr = FarmPlotIndexMap.Find(PlotID);
	if (IndexPtr)
	{
		return FarmPlots[*IndexPtr];
	}
	return nullptr;
}

TArray<AFarmPlot*> APlanetaryFarmingSystem::GetAllFarmPlots() const
{
	return FarmPlots;
}

TArray<FGuid> APlanetaryFarmingSystem::GetPlotsReadyForHarvest() const
{
	TArray<FGuid> ReadyPlots;

	for (AFarmPlot* Plot : FarmPlots)
	{
		if (Plot && Plot->bIsHarvestable)
		{
			ReadyPlots.Add(Plot->PlotID);
		}
	}

	return ReadyPlots;
}

FFarmStatistics APlanetaryFarmingSystem::GetFarmStatistics() const
{
	FFarmStatistics Stats;
	Stats.TotalPlots = FarmPlots.Num();

	TSet<ECropTypeExtended> UniqueCropTypes;

	for (AFarmPlot* Plot : FarmPlots)
	{
		if (!Plot)
		{
			continue;
		}

		if (Plot->CurrentCrop != nullptr)
		{
			Stats.ActivePlots++;
			UniqueCropTypes.Add(Plot->CurrentCrop->CropType);
		}

		if (Plot->bIsHarvestable)
		{
			Stats.ReadyToHarvest++;
		}

		Stats.TotalCropsPlanted += Plot->CropsPlanted;
		Stats.TotalCropsHarvested += Plot->CropsHarvested;
		Stats.CurrentYield += Plot->CurrentYield;
		Stats.WaterUsage += Plot->WaterUsed;
		Stats.FertilizerUsage += Plot->FertilizerUsed;
	}

	Stats.CropDiversity = UniqueCropTypes.Num();

	// Calculate financials
	Stats.DailyExpenses = (Stats.WaterUsage * WaterCostPerLiter) + (Stats.FertilizerUsage * FertilizerCostPerKg);
	Stats.NetProfit = Stats.DailyRevenue - Stats.DailyExpenses;

	return Stats;
}

TArray<AActor*> APlanetaryFarmingSystem::GetInfrastructureByType(EFarmingInfrastructureType Type) const
{
	const FActorArray* InfrastructureArray = InfrastructureByType.Find(Type);
	if (InfrastructureArray)
	{
		return InfrastructureArray->Actors;
	}
	return TArray<AActor*>();
}

ECropTypeExtended APlanetaryFarmingSystem::FindBestCropForLocation(FVector Location) const
{
	if (!TargetPlanet.IsValid() || !CropSystem)
	{
		return ECropTypeExtended::Wheat;
	}

	// Get environmental conditions at location
	float Temperature = 20.0f; // Default
	float Humidity = 0.5f; // Default

	// Would get actual environmental data from planet systems
	if (TargetPlanet->WeatherComponent)
	{
		// Temperature = TargetPlanet->WeatherComponent->GetTemperatureAtLocation(Location);
		// Humidity = TargetPlanet->WeatherComponent->GetHumidityAtLocation(Location);
	}

	// Get suitable crops for these conditions
	TArray<ECropTypeExtended> SuitableCrops = CropSystem->GetCropsForEnvironment(Temperature, Humidity, 0.7f);

	// Return the most valuable suitable crop
	ECropTypeExtended BestCrop = ECropTypeExtended::Wheat;
	float BestValue = 0.0f;

	for (ECropTypeExtended Crop : SuitableCrops)
	{
		float Value = CropSystem->GetCropMarketValue(Crop);
		if (Value > BestValue)
		{
			BestValue = Value;
			BestCrop = Crop;
		}
	}

	return BestCrop;
}

float APlanetaryFarmingSystem::GetCropGrowthProgress(const FGuid& PlotID) const
{
	AFarmPlot* Plot = GetFarmPlot(PlotID);
	if (Plot)
	{
		float TotalProgress = 0.0f;
		int32 ActiveCrops = 0;
		
		for (const FCropCell& Cell : Plot->CropGrid)
		{
			if (Cell.CropType)
			{
				TotalProgress += Cell.GrowthProgress;
				ActiveCrops++;
			}
		}
		
		return ActiveCrops > 0 ? TotalProgress / ActiveCrops : 0.0f;
	}
	return 0.0f;
}

EPlanetaryCropGrowthStage APlanetaryFarmingSystem::GetCropGrowthStage(const FGuid& PlotID) const
{
	AFarmPlot* Plot = GetFarmPlot(PlotID);
	if (Plot)
	{
		float AvgProgress = GetCropGrowthProgress(PlotID);
		
		if (AvgProgress < 0.2f) return EPlanetaryCropGrowthStage::Seed;
		else if (AvgProgress < 0.4f) return EPlanetaryCropGrowthStage::Sprout;
		else if (AvgProgress < 0.6f) return EPlanetaryCropGrowthStage::Vegetative;
		else if (AvgProgress < 0.8f) return EPlanetaryCropGrowthStage::Flowering;
		else if (AvgProgress < 1.0f) return EPlanetaryCropGrowthStage::Fruiting;
		else return EPlanetaryCropGrowthStage::Mature;
	}
	return EPlanetaryCropGrowthStage::Seed;
}

void APlanetaryFarmingSystem::EnableAutoManagement(bool bEnable)
{
	bAutoManagementEnabled = bEnable;
	UE_LOG(LogTemp, Log, TEXT("Auto-management %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void APlanetaryFarmingSystem::ProcessAllPlots(float DeltaTime)
{
	for (AFarmPlot* Plot : FarmPlots)
	{
		if (Plot)
		{
			Plot->Tick(DeltaTime);
		}
	}
}

void APlanetaryFarmingSystem::UpdateFarmStatistics()
{
	FFarmStatistics Stats = GetFarmStatistics();
	OnFarmStatisticsUpdated.Broadcast(Stats);
}

void APlanetaryFarmingSystem::ProcessAutoManagement()
{
	// Auto-water plots that need it
	for (AFarmPlot* Plot : FarmPlots)
	{
		if (!Plot || !Plot->CurrentCrop)
		{
			continue;
		}

		// Check if plot needs water
		if (Plot->SoilMoisture < 0.3f)
		{
			float WaterNeeded = Plot->CalculateWaterNeeded();
			WaterPlot(Plot->PlotID, WaterNeeded);
		}

		// Check if plot needs fertilizer
		if (Plot->SoilNutrients < 0.2f)
		{
			float FertilizerNeeded = Plot->CalculateFertilizerNeeded();
			FertilizePlot(Plot->PlotID, FertilizerNeeded, TEXT("Basic"));
		}
	}

	// Auto-harvest ready plots
	TArray<FGuid> ReadyPlots = GetPlotsReadyForHarvest();
	for (const FGuid& PlotID : ReadyPlots)
	{
		HarvestCrops(PlotID);
	}
}

int32 APlanetaryFarmingSystem::CalculateExpectedYield(ECropTypeExtended CropType, float SoilQuality, float PlantingDensity) const
{
	FCropGrowthData CropData = CropSystem->GetCropGrowthData(CropType);
	
	// Base yield modified by soil quality and planting density
	float QualityModifier = FMath::Clamp(SoilQuality, 0.5f, 1.5f);
	float DensityModifier = FMath::Clamp(PlantingDensity, 0.5f, 1.0f);
	
	return FMath::RoundToInt(CropData.BaseYield * QualityModifier * DensityModifier);
}

int32 APlanetaryFarmingSystem::GetPlotIndex(const FGuid& PlotID) const
{
	const int32* IndexPtr = FarmPlotIndexMap.Find(PlotID);
	return IndexPtr ? *IndexPtr : INDEX_NONE;
}