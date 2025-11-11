// Copyright Epic Games, Inc. All Rights Reserved.

#include "FarmingInfrastructure.h"
#include "FarmPlot.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"

// Hydroponics Bay Implementation
AHydroponicsBay::AHydroponicsBay()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

	BayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BayMesh"));
	BayMesh->SetupAttachment(RootComponent);

	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(RootComponent);

	// Default configuration
	MaxPlants = 50;
	MaxWaterCapacity = 1000.0f;
	CurrentWater = 800.0f;
	MaxNutrientCapacity = 500.0f;
	CurrentNutrients = 400.0f;
	CurrentFertilizerType = EFertilizerType::Basic;
	CurrentTemperature = 20.0f;
	CurrentHumidity = 0.6f;
	CurrentLightLevel = 0.8f;
	bIsOperational = false;
	PowerConsumption = 5.0f;
}

void AHydroponicsBay::BeginPlay()
{
	Super::BeginPlay();
	bIsOperational = true;
	UE_LOG(LogTemp, Log, TEXT("Hydroponics bay initialized with capacity for %d plants"), MaxPlants);
}

void AHydroponicsBay::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsOperational)
	{
		UpdatePlantGrowth(DeltaTime);
		ConsumeResources(DeltaTime);
	}
}

void AHydroponicsBay::InitializeBay(int32 InMaxPlants, float WaterCapacity, float NutrientCapacity)
{
	MaxPlants = InMaxPlants;
	MaxWaterCapacity = WaterCapacity;
	MaxNutrientCapacity = NutrientCapacity;
	CurrentWater = WaterCapacity * 0.8f; // Start at 80%
	CurrentNutrients = NutrientCapacity * 0.8f; // Start at 80%
}

bool AHydroponicsBay::AddPlant(FActiveCrop& Crop)
{
	if (Plants.Num() >= MaxPlants)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hydroponics bay is full"));
		return false;
	}

	Plants.Add(Crop);
	UE_LOG(LogTemp, Log, TEXT("Plant added to hydroponics bay. Total plants: %d"), Plants.Num());
	return true;
}

bool AHydroponicsBay::RemovePlant(int32 PlantIndex)
{
	if (PlantIndex < 0 || PlantIndex >= Plants.Num())
	{
		return false;
	}

	Plants.RemoveAt(PlantIndex);
	UE_LOG(LogTemp, Log, TEXT("Plant removed from hydroponics bay. Total plants: %d"), Plants.Num());
	return true;
}

void AHydroponicsBay::WaterAllPlants(float WaterAmount)
{
	if (CurrentWater >= WaterAmount)
	{
		CurrentWater -= WaterAmount;
		
		// Distribute water to all plants
		float WaterPerPlant = WaterAmount / FMath::Max(1, Plants.Num());
		for (FActiveCrop& Plant : Plants)
		{
			Plant.WaterLevel = FMath::Clamp(Plant.WaterLevel + WaterPerPlant, 0.0f, 1.0f);
		}
		
		UE_LOG(LogTemp, Log, TEXT("Watered all plants in hydroponics bay: %f liters"), WaterAmount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Insufficient water in hydroponics bay"));
	}
}

void AHydroponicsBay::AddNutrients(float NutrientAmount, EFertilizerType FertilizerType)
{
	if (CurrentNutrients + NutrientAmount <= MaxNutrientCapacity)
	{
		CurrentNutrients += NutrientAmount;
		CurrentFertilizerType = FertilizerType;
		UE_LOG(LogTemp, Log, TEXT("Added %f kg of nutrients to hydroponics bay"), NutrientAmount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Nutrient tank would overflow"));
	}
}

void AHydroponicsBay::SetEnvironment(float Temperature, float Humidity, float LightLevel)
{
	CurrentTemperature = Temperature;
	CurrentHumidity = Humidity;
	CurrentLightLevel = LightLevel;
	UE_LOG(LogTemp, Log, TEXT("Hydroponics bay environment set: Temp=%.1f°C, Humidity=%.1f%%, Light=%.1f%%"), 
		Temperature, Humidity * 100.0f, LightLevel * 100.0f);
}

void AHydroponicsBay::UpdatePlantGrowth(float DeltaTime)
{
	for (FActiveCrop& Plant : Plants)
	{
		// Update crop growth using ideal conditions
		// In a real implementation, this would use UCropGrowthSystem
		Plant.GrowthProgress = FMath::Clamp(Plant.GrowthProgress + DeltaTime * 0.01f, 0.0f, 1.0f);
		
		// Keep water and nutrient levels optimal
		Plant.WaterLevel = FMath::Clamp(Plant.WaterLevel + DeltaTime * 0.1f, 0.7f, 1.0f);
		Plant.NutrientLevel = FMath::Clamp(Plant.NutrientLevel + DeltaTime * 0.05f, 0.7f, 1.0f);
	}
}

void AHydroponicsBay::ConsumeResources(float DeltaTime)
{
	// Water consumption based on number of plants
	float WaterConsumption = Plants.Num() * DeltaTime * 0.5f;
	CurrentWater = FMath::Max(0.0f, CurrentWater - WaterConsumption);

	// Nutrient consumption
	float NutrientConsumption = Plants.Num() * DeltaTime * 0.1f;
	CurrentNutrients = FMath::Max(0.0f, CurrentNutrients - NutrientConsumption);

	// Check for low resources
	if (CurrentWater < MaxWaterCapacity * 0.1f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hydroponics bay water level low: %f%%"), (CurrentWater / MaxWaterCapacity) * 100.0f);
	}

	if (CurrentNutrients < MaxNutrientCapacity * 0.1f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hydroponics bay nutrients low: %f%%"), (CurrentNutrients / MaxNutrientCapacity) * 100.0f);
	}
}

// Greenhouse Implementation
AGreenhouse::AGreenhouse()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

	GreenhouseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GreenhouseMesh"));
	GreenhouseMesh->SetupAttachment(RootComponent);

	GreenhouseVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("GreenhouseVolume"));
	GreenhouseVolume->SetupAttachment(RootComponent);

	// Default configuration
	Dimensions = FVector2D(1000.0f, 1000.0f);
	MaxRows = 5;
	MaxColumns = 5;
	CurrentTemperature = 20.0f;
	CurrentHumidity = 0.6f;
	bVentilationEnabled = true;
	StructuralIntegrity = 100.0f;
	bIsOperational = false;
	PowerConsumption = 3.0f;
}

void AGreenhouse::BeginPlay()
{
	Super::BeginPlay();
	bIsOperational = true;
	UE_LOG(LogTemp, Log, TEXT("Greenhouse initialized: %.1fx%.1f meters"), Dimensions.X, Dimensions.Y);
}

void AGreenhouse::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsOperational)
	{
		UpdateEnvironment(DeltaTime);
		UpdateStructuralIntegrity(DeltaTime);
	}
}

void AGreenhouse::InitializeGreenhouse(FVector2D InDimensions, int32 InMaxRows, int32 InMaxColumns)
{
	Dimensions = InDimensions;
	MaxRows = InMaxRows;
	MaxColumns = InMaxColumns;
	UE_LOG(LogTemp, Log, TEXT("Greenhouse configured for %d rows x %d columns"), MaxRows, MaxColumns);
}

bool AGreenhouse::AddFarmPlot(AFarmPlot* Plot)
{
	if (!Plot || FarmPlots.Num() >= MaxRows * MaxColumns)
	{
		return false;
	}

	FarmPlots.Add(Plot);
	UE_LOG(LogTemp, Log, TEXT("Farm plot added to greenhouse. Total plots: %d"), FarmPlots.Num());
	return true;
}

bool AGreenhouse::RemoveFarmPlot(const FGuid& PlotID)
{
	for (int32 i = 0; i < FarmPlots.Num(); i++)
	{
		if (FarmPlots[i] && FarmPlots[i]->PlotID == PlotID)
		{
			FarmPlots.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("Farm plot removed from greenhouse. Total plots: %d"), FarmPlots.Num());
			return true;
		}
	}
	return false;
}

void AGreenhouse::SetTemperature(float Temperature)
{
	CurrentTemperature = Temperature;
	UE_LOG(LogTemp, Log, TEXT("Greenhouse temperature set to: %.1f°C"), Temperature);
}

void AGreenhouse::SetHumidity(float Humidity)
{
	CurrentHumidity = FMath::Clamp(Humidity, 0.0f, 1.0f);
	UE_LOG(LogTemp, Log, TEXT("Greenhouse humidity set to: %.1f%%"), Humidity * 100.0f);
}

void AGreenhouse::SetVentilation(bool bEnable)
{
	bVentilationEnabled = bEnable;
	UE_LOG(LogTemp, Log, TEXT("Greenhouse ventilation %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void AGreenhouse::UpdateEnvironment(float DeltaTime)
{
	// In a real implementation, this would adjust the environment
	// based on external conditions and greenhouse settings
	// For now, maintain stable conditions
}

void AGreenhouse::UpdateStructuralIntegrity(float DeltaTime)
{
	// Slowly degrade over time (would be affected by weather, etc.)
	StructuralIntegrity = FMath::Max(0.0f, StructuralIntegrity - DeltaTime * 0.001f);

	if (StructuralIntegrity < 20.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Greenhouse structural integrity critical: %f%%"), StructuralIntegrity);
	}
}

// Irrigation System Implementation
AIrrigationSystem::AIrrigationSystem()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

	PumpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PumpMesh"));
	PumpMesh->SetupAttachment(RootComponent);

	CoverageVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("CoverageVolume"));
	CoverageVolume->SetupAttachment(RootComponent);

	// Default configuration
	MaxWaterCapacity = 5000.0f;
	CurrentWater = 4000.0f;
	FlowRate = 10.0f; // 10 liters per second
	CoverageRadius = 500.0f;
	WateringInterval = 3600.0f; // 1 hour
	WateringDuration = 300.0f; // 5 minutes
	LastWateringTime = 0.0f;
	bIsOperational = false;
	PowerConsumption = 2.0f;
}

void AIrrigationSystem::BeginPlay()
{
	Super::BeginPlay();
	bIsOperational = true;
	UE_LOG(LogTemp, Log, TEXT("Irrigation system initialized: %.1fL capacity, %.1fm radius"), MaxWaterCapacity, CoverageRadius);
}

void AIrrigationSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsOperational)
	{
		CheckWateringSchedule(DeltaTime);
	}
}

void AIrrigationSystem::InitializeSystem(float WaterCapacity, float InFlowRate, float InCoverageRadius)
{
	MaxWaterCapacity = WaterCapacity;
	CurrentWater = WaterCapacity * 0.8f;
	FlowRate = InFlowRate;
	CoverageRadius = InCoverageRadius;
}

bool AIrrigationSystem::AddPlotToCoverage(AFarmPlot* Plot)
{
	if (!Plot)
	{
		return false;
	}

	CoveredPlots.Add(Plot);
	UE_LOG(LogTemp, Log, TEXT("Farm plot added to irrigation coverage. Total plots: %d"), CoveredPlots.Num());
	return true;
}

bool AIrrigationSystem::RemovePlotFromCoverage(const FGuid& PlotID)
{
	for (int32 i = 0; i < CoveredPlots.Num(); i++)
	{
		if (CoveredPlots[i] && CoveredPlots[i]->PlotID == PlotID)
		{
			CoveredPlots.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("Farm plot removed from irrigation coverage. Total plots: %d"), CoveredPlots.Num());
			return true;
		}
	}
	return false;
}

void AIrrigationSystem::WaterAllCoveredPlots()
{
	if (CurrentWater <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Irrigation system has no water"));
		return;
	}

	float WaterPerPlot = FMath::Min(FlowRate, CurrentWater) / FMath::Max(1, CoveredPlots.Num());

	for (AFarmPlot* Plot : CoveredPlots)
	{
		if (Plot)
		{
			Plot->WaterPlot(WaterPerPlot);
			ConsumeWater(WaterPerPlot);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Irrigation system watered %d plots"), CoveredPlots.Num());
}

void AIrrigationSystem::SetWateringSchedule(float Interval, float Duration)
{
	WateringInterval = Interval;
	WateringDuration = Duration;
	UE_LOG(LogTemp, Log, TEXT("Irrigation schedule set: Interval=%.1fs, Duration=%.1fs"), Interval, Duration);
}

void AIrrigationSystem::RefillWaterTank(float Amount)
{
	float OldWater = CurrentWater;
	CurrentWater = FMath::Clamp(CurrentWater + Amount, 0.0f, MaxWaterCapacity);
	float ActualAdded = CurrentWater - OldWater;
	UE_LOG(LogTemp, Log, TEXT("Water tank refilled: %f liters (current: %f/%f)"), ActualAdded, CurrentWater, MaxWaterCapacity);
}

void AIrrigationSystem::CheckWateringSchedule(float DeltaTime)
{
	LastWateringTime += DeltaTime;

	if (LastWateringTime >= WateringInterval)
	{
		WaterAllCoveredPlots();
		LastWateringTime = 0.0f;
	}
}

void AIrrigationSystem::ConsumeWater(float Amount)
{
	CurrentWater = FMath::Max(0.0f, CurrentWater - Amount);
}

// Fertilizer Injector Implementation
AFertilizerInjector::AFertilizerInjector()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

	InjectorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InjectorMesh"));
	InjectorMesh->SetupAttachment(RootComponent);

	CoverageVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("CoverageVolume"));
	CoverageVolume->SetupAttachment(RootComponent);

	// Default configuration
	TankCapacity = 1000.0f;
	InjectionRate = 1.0f; // 1 kg per second
	InjectionInterval = 7200.0f; // 2 hours
	InjectionAmount = 5.0f; // 5 kg per injection
	LastInjectionTime = 0.0f;
	bIsOperational = false;
	PowerConsumption = 1.5f;

	// Initialize fertilizer tanks
	FertilizerTanks.Add(EFertilizerType::Basic, 800.0f);
	FertilizerTanks.Add(EFertilizerType::Premium, 600.0f);
	FertilizerTanks.Add(EFertilizerType::Specialized, 400.0f);
	FertilizerTanks.Add(EFertilizerType::Organic, 700.0f);
	FertilizerTanks.Add(EFertilizerType::Synthetic, 500.0f);
}

void AFertilizerInjector::BeginPlay()
{
	Super::BeginPlay();
	bIsOperational = true;
	UE_LOG(LogTemp, Log, TEXT("Fertilizer injector initialized"));
}

void AFertilizerInjector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsOperational)
	{
		CheckInjectionSchedule(DeltaTime);
	}
}

void AFertilizerInjector::InitializeInjector(float InTankCapacity, float InInjectionRate)
{
	TankCapacity = InTankCapacity;
	InjectionRate = InInjectionRate;
}

bool AFertilizerInjector::AddPlotToCoverage(AFarmPlot* Plot)
{
	if (!Plot)
	{
		return false;
	}

	CoveredPlots.Add(Plot);
	UE_LOG(LogTemp, Log, TEXT("Farm plot added to fertilizer coverage. Total plots: %d"), CoveredPlots.Num());
	return true;
}

bool AFertilizerInjector::RemovePlotFromCoverage(const FGuid& PlotID)
{
	for (int32 i = 0; i < CoveredPlots.Num(); i++)
	{
		if (CoveredPlots[i] && CoveredPlots[i]->PlotID == PlotID)
		{
			CoveredPlots.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("Farm plot removed from fertilizer coverage. Total plots: %d"), CoveredPlots.Num());
			return true;
		}
	}
	return false;
}

void AFertilizerInjector::InjectFertilizerToAll(EFertilizerType FertilizerType, float Amount)
{
	float* TankLevel = FertilizerTanks.Find(FertilizerType);
	if (!TankLevel || *TankLevel < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("Insufficient fertilizer in tank"));
		return;
	}

	float FertilizerPerPlot = Amount / FMath::Max(1, CoveredPlots.Num());

	for (AFarmPlot* Plot : CoveredPlots)
	{
		if (Plot)
		{
			Plot->FertilizePlot(FertilizerPerPlot, FertilizerType);
			ConsumeFertilizer(FertilizerType, FertilizerPerPlot);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Injected %f kg of fertilizer to %d plots"), Amount, CoveredPlots.Num());
}

void AFertilizerInjector::SetInjectionSchedule(float Interval, float Amount)
{
	InjectionInterval = Interval;
	InjectionAmount = Amount;
	UE_LOG(LogTemp, Log, TEXT("Fertilizer injection schedule set: Interval=%.1fs, Amount=%.1fkg"), Interval, Amount);
}

void AFertilizerInjector::RefillTank(EFertilizerType FertilizerType, float Amount)
{
	float* TankLevel = FertilizerTanks.Find(FertilizerType);
	if (TankLevel)
	{
		float OldLevel = *TankLevel;
		*TankLevel = FMath::Clamp(*TankLevel + Amount, 0.0f, TankCapacity);
		float ActualAdded = *TankLevel - OldLevel;
		UE_LOG(LogTemp, Log, TEXT("Fertilizer tank refilled: %f kg of %s (current: %f/%f)"), 
			ActualAdded, *UEnum::GetDisplayValueAsText(FertilizerType).ToString(), *TankLevel, TankCapacity);
	}
}

float AFertilizerInjector::GetFertilizerLevel(EFertilizerType Type) const
{
	const float* Level = FertilizerTanks.Find(Type);
	return Level ? *Level / TankCapacity : 0.0f;
}

void AFertilizerInjector::CheckInjectionSchedule(float DeltaTime)
{
	LastInjectionTime += DeltaTime;

	if (LastInjectionTime >= InjectionInterval)
	{
		// Use basic fertilizer by default for scheduled injections
		InjectFertilizerToAll(EFertilizerType::Basic, InjectionAmount);
		LastInjectionTime = 0.0f;
	}
}

void AFertilizerInjector::ConsumeFertilizer(EFertilizerType Type, float Amount)
{
	float* TankLevel = FertilizerTanks.Find(Type);
	if (TankLevel)
	{
		*TankLevel = FMath::Max(0.0f, *TankLevel - Amount);
	}
}

// Harvesting Drone Implementation
AHarvestingDrone::AHarvestingDrone()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

	DroneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DroneMesh"));
	DroneMesh->SetupAttachment(RootComponent);

	// Default configuration
	MaxBatteryCapacity = 1000.0f;
	CurrentBattery = 800.0f;
	HarvestSpeed = 2.0f; // 2 plants per second
	MovementSpeed = 500.0f; // 5 meters per second
	CurrentTask = TEXT("Idle");
	bIsHarvesting = false;
	TotalHarvested = 0;
	bIsOperational = false;
	ActivePowerConsumption = 3.0f;
	IdlePowerConsumption = 0.5f;
}

void AHarvestingDrone::BeginPlay()
{
	Super::BeginPlay();
	bIsOperational = true;
	UE_LOG(LogTemp, Log, TEXT("Harvesting drone initialized"));
}

void AHarvestingDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsOperational)
	{
		if (bIsHarvesting && CurrentPlot)
		{
			PerformHarvesting(DeltaTime);
		}

		ConsumeBattery(DeltaTime);
	}
}

void AHarvestingDrone::InitializeDrone(float BatteryCapacity, float InHarvestSpeed, float InMovementSpeed)
{
	MaxBatteryCapacity = BatteryCapacity;
	CurrentBattery = BatteryCapacity * 0.8f;
	HarvestSpeed = InHarvestSpeed;
	MovementSpeed = InMovementSpeed;
}

bool AHarvestingDrone::AssignPlot(AFarmPlot* Plot)
{
	if (!Plot)
	{
		return false;
	}

	CurrentPlot = Plot;
	CurrentTask = TEXT("Assigned to plot");
	UE_LOG(LogTemp, Log, TEXT("Drone assigned to farm plot: %s"), *Plot->PlotID.ToString());
	return true;
}

void AHarvestingDrone::StartHarvesting()
{
	if (!CurrentPlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("No plot assigned to drone"));
		return;
	}

	bIsHarvesting = true;
	CurrentTask = TEXT("Harvesting");
	UE_LOG(LogTemp, Log, TEXT("Drone started harvesting"));
}

void AHarvestingDrone::StopHarvesting()
{
	bIsHarvesting = false;
	CurrentTask = TEXT("Idle");
	UE_LOG(LogTemp, Log, TEXT("Drone stopped harvesting"));
}

void AHarvestingDrone::ReturnToChargingStation()
{
	StopHarvesting();
	CurrentTask = TEXT("Returning to charger");
	UE_LOG(LogTemp, Log, TEXT("Drone returning to charging station"));
}

void AHarvestingDrone::ChargeBattery(float Amount)
{
	float OldBattery = CurrentBattery;
	CurrentBattery = FMath::Clamp(CurrentBattery + Amount, 0.0f, MaxBatteryCapacity);
	float ActualCharge = CurrentBattery - OldBattery;
	UE_LOG(LogTemp, Log, TEXT("Drone battery charged: %f units (current: %f/%f)"), ActualCharge, CurrentBattery, MaxBatteryCapacity);
}

FString AHarvestingDrone::GetCurrentTask() const
{
	return CurrentTask;
}

void AHarvestingDrone::PerformHarvesting(float DeltaTime)
{
	if (!CurrentPlot || !CurrentPlot->bIsHarvestable)
	{
		StopHarvesting();
		return;
	}

	// Simulate harvesting
	float HarvestAmount = HarvestSpeed * DeltaTime;
	int32 HarvestedThisFrame = FMath::FloorToInt(HarvestAmount);

	if (HarvestedThisFrame > 0)
	{
		FHarvestResult Result = CurrentPlot->HarvestAllCrops();
		TotalHarvested += Result.Quantity;
		UE_LOG(LogTemp, Log, TEXT("Drone harvested %d crops. Total: %d"), Result.Quantity, TotalHarvested);
	}

	// Check if plot is empty
	if (CurrentPlot->CurrentCrop == nullptr)
	{
		StopHarvesting();
	}
}

void AHarvestingDrone::ConsumeBattery(float DeltaTime)
{
	float ConsumptionRate = bIsHarvesting ? ActivePowerConsumption : IdlePowerConsumption;
	CurrentBattery = FMath::Max(0.0f, CurrentBattery - ConsumptionRate * DeltaTime);

	if (CurrentBattery < MaxBatteryCapacity * 0.1f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Drone battery low: %f%%"), (CurrentBattery / MaxBatteryCapacity) * 100.0f);
		
		if (CurrentBattery <= 0.0f)
		{
			StopHarvesting();
			ReturnToChargingStation();
		}
	}
}