// Copyright Epic Games, Inc. All Rights Reserved.

#include "HarvestingMachine.h"
#include "FarmPlot.h"
#include "CropDefinition.h"
#include "FarmingSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AHarvestingMachine::AHarvestingMachine()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootComp;

	// Create machine mesh
	MachineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MachineMesh"));
	MachineMesh->SetupAttachment(RootComp);

	// Create harvest zone
	HarvestZone = CreateDefaultSubobject<UBoxComponent>(TEXT("HarvestZone"));
	HarvestZone->SetupAttachment(RootComp);
	HarvestZone->SetBoxExtent(FVector(1000.0f, 1000.0f, 200.0f)); // 10m radius default
	HarvestZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HarvestZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	HarvestZone->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);

	// Initialize state
	bIsActive = false;
	bHasPower = true;
	TimeSinceLastHarvest = 0.0f;
	CurrentStorageCount = 0;
	TotalCropsHarvested = 0;
	TotalValueHarvested = 0.0f;
	TotalPowerConsumed = 0.0f;

	// Default configuration
	Config.HarvestRadius = 10.0f;
	Config.HarvestInterval = 5.0f;
	Config.MaxStorageCapacity = 1000;
	Config.bAutoHarvest = true;
	Config.bRequiresPower = true;
	Config.PowerConsumption = 500.0f;
}

void AHarvestingMachine::BeginPlay()
{
	Super::BeginPlay();

	// Update harvest zone size based on config
	UpdateHarvestZoneSize();

	// Detect farm plots in range
	DetectFarmPlots();

	// Start harvesting if auto-harvest is enabled
	if (Config.bAutoHarvest)
	{
		StartHarvesting();
	}
}

void AHarvestingMachine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Process automatic harvesting
	if (bIsActive)
	{
		ProcessAutoHarvest(DeltaTime);
	}

	// Update power consumption
	if (bIsActive && Config.bRequiresPower)
	{
		UpdatePowerConsumption(DeltaTime);
	}
}

// ============================================================================
// HARVESTING OPERATIONS
// ============================================================================

void AHarvestingMachine::StartHarvesting()
{
	if (!bIsActive)
	{
		bIsActive = true;
		TimeSinceLastHarvest = 0.0f;

		// Update managed plots
		UpdateManagedPlots();

		UE_LOG(LogTemp, Log, TEXT("HarvestingMachine: Started automatic harvesting"));
	}
}

void AHarvestingMachine::StopHarvesting()
{
	if (bIsActive)
	{
		bIsActive = false;
		UE_LOG(LogTemp, Log, TEXT("HarvestingMachine: Stopped automatic harvesting"));
	}
}

int32 AHarvestingMachine::PerformHarvestCycle()
{
	// Check if we have power
	if (Config.bRequiresPower && !HasSufficientPower())
	{
		UE_LOG(LogTemp, Warning, TEXT("HarvestingMachine: Insufficient power for harvest cycle"));
		return 0;
	}

	// Check if storage is full
	if (IsStorageFull())
	{
		UE_LOG(LogTemp, Warning, TEXT("HarvestingMachine: Storage is full, cannot harvest"));
		return 0;
	}

	int32 HarvestedCount = 0;

	// Iterate through all managed plots
	for (AFarmPlot* Plot : ManagedPlots)
	{
		if (!Plot)
		{
			continue;
		}

		// Get cells in harvest zone for this plot
		TArray<FIntPoint>* CellsPtr = PlotCellMap.Find(Plot);
		if (!CellsPtr)
		{
			continue;
		}

		// Harvest ready crops in this plot
		for (const FIntPoint& GridPos : *CellsPtr)
		{
			if (IsCropReadyForHarvest(Plot, GridPos))
			{
				if (HarvestCropCell(Plot, GridPos))
				{
					HarvestedCount++;

					// Stop if storage is full
					if (IsStorageFull())
					{
						UE_LOG(LogTemp, Log, TEXT("HarvestingMachine: Storage full, stopping harvest cycle"));
						return HarvestedCount;
					}
				}
			}
		}
	}

	if (HarvestedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("HarvestingMachine: Harvested %d crops"), HarvestedCount);
	}

	return HarvestedCount;
}

bool AHarvestingMachine::HarvestCropCell(AFarmPlot* Plot, FIntPoint GridPosition)
{
	if (!Plot)
	{
		return false;
	}

	// Harvest the crop from the plot
	FHarvestResult Result = Plot->HarvestCrop(GridPosition);

	if (Result.bSuccess)
	{
		// Add to storage
		bool bAdded = AddToStorage(Result.ItemName, Result.YieldAmount, Result.YieldAmount * Result.Quality);

		if (bAdded)
		{
			// Update statistics
			TotalCropsHarvested += Result.YieldAmount;
			TotalValueHarvested += Result.YieldAmount * Result.Quality;

			return true;
		}
	}

	return false;
}

bool AHarvestingMachine::IsCropReadyForHarvest(AFarmPlot* Plot, FIntPoint GridPosition) const
{
	if (!Plot)
	{
		return false;
	}

	// Check if grid position is valid
	if (GridPosition.X < 0 || GridPosition.Y < 0 || 
		GridPosition.X >= Plot->GridResolution || GridPosition.Y >= Plot->GridResolution)
	{
		return false;
	}

	// Get the crop cell
	int32 CellIndex = GridPosition.Y * Plot->GridResolution + GridPosition.X;
	if (CellIndex < 0 || CellIndex >= Plot->CropGrid.Num())
	{
		return false;
	}

	const FCropCell& Cell = Plot->CropGrid[CellIndex];

	// Check if there's a crop and it's fully grown
	return Cell.CropType != nullptr && Cell.GrowthProgress >= 1.0f && Cell.Health > 0.0f;
}

// ============================================================================
// PLOT BOUNDARY DETECTION
// ============================================================================

void AHarvestingMachine::DetectFarmPlots()
{
	ManagedPlots.Empty();

	// Get farming subsystem
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UFarmingSubsystem* FarmingSubsystem = World->GetSubsystem<UFarmingSubsystem>();
	if (!FarmingSubsystem)
	{
		return;
	}

	// Get all active farms
	const TArray<AFarmPlot*>& AllFarms = FarmingSubsystem->GetActiveFarms();

	// Check which farms are within harvest radius
	FVector MachineLocation = GetActorLocation();

	for (AFarmPlot* Farm : AllFarms)
	{
		if (!Farm)
		{
			continue;
		}

		FVector FarmLocation = Farm->GetActorLocation();
		float Distance = FVector::Dist2D(MachineLocation, FarmLocation);

		// Check if farm is within harvest radius (with some buffer for plot size)
		float MaxDistance = Config.HarvestRadius + FMath::Max(Farm->PlotSize.X, Farm->PlotSize.Y);

		if (Distance <= MaxDistance)
		{
			ManagedPlots.Add(Farm);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("HarvestingMachine: Detected %d farm plots in range"), ManagedPlots.Num());

	// Update cell mapping
	UpdateManagedPlots();
}

bool AHarvestingMachine::IsLocationInHarvestZone(FVector Location) const
{
	FVector MachineLocation = GetActorLocation();
	float Distance = FVector::Dist2D(MachineLocation, Location);
	return Distance <= Config.HarvestRadius;
}

TArray<FIntPoint> AHarvestingMachine::GetCropCellsInZone(AFarmPlot* Plot) const
{
	TArray<FIntPoint> CellsInZone;

	if (!Plot)
	{
		return CellsInZone;
	}

	// Check each cell in the plot
	for (int32 Y = 0; Y < Plot->GridResolution; ++Y)
	{
		for (int32 X = 0; X < Plot->GridResolution; ++X)
		{
			FIntPoint GridPos(X, Y);
			FVector CellWorldPos = GetCropCellWorldPosition(Plot, GridPos);

			if (IsLocationInHarvestZone(CellWorldPos))
			{
				CellsInZone.Add(GridPos);
			}
		}
	}

	return CellsInZone;
}

void AHarvestingMachine::UpdateManagedPlots()
{
	PlotCellMap.Empty();

	// Build cell map for each managed plot
	for (AFarmPlot* Plot : ManagedPlots)
	{
		if (!Plot)
		{
			continue;
		}

		TArray<FIntPoint> CellsInZone = GetCropCellsInZone(Plot);
		PlotCellMap.Add(Plot, CellsInZone);

		UE_LOG(LogTemp, Log, TEXT("HarvestingMachine: Plot has %d cells in harvest zone"), CellsInZone.Num());
	}
}

// ============================================================================
// STORAGE SYSTEM
// ============================================================================

bool AHarvestingMachine::AddToStorage(const FString& CropID, int32 Quantity, float Value)
{
	// Check if storage is full
	if (CurrentStorageCount + Quantity > Config.MaxStorageCapacity)
	{
		UE_LOG(LogTemp, Warning, TEXT("HarvestingMachine: Cannot add %d items, storage full"), Quantity);
		return false;
	}

	// Find existing entry or create new one
	FHarvestedCropEntry* Entry = FindStorageEntry(CropID);

	if (Entry)
	{
		Entry->Quantity += Quantity;
		Entry->TotalValue += Value;
	}
	else
	{
		FHarvestedCropEntry NewEntry;
		NewEntry.CropID = CropID;
		NewEntry.Quantity = Quantity;
		NewEntry.TotalValue = Value;
		StoredCrops.Add(NewEntry);
	}

	CurrentStorageCount += Quantity;

	return true;
}

bool AHarvestingMachine::RemoveFromStorage(const FString& CropID, int32 Quantity)
{
	FHarvestedCropEntry* Entry = FindStorageEntry(CropID);

	if (!Entry || Entry->Quantity < Quantity)
	{
		return false;
	}

	Entry->Quantity -= Quantity;
	CurrentStorageCount -= Quantity;

	// Remove entry if empty
	if (Entry->Quantity <= 0)
	{
		StoredCrops.RemoveAll([&CropID](const FHarvestedCropEntry& E) {
			return E.CropID == CropID;
		});
	}

	return true;
}

int32 AHarvestingMachine::GetStoredQuantity(const FString& CropID) const
{
	const FHarvestedCropEntry* Entry = StoredCrops.FindByPredicate([&CropID](const FHarvestedCropEntry& E) {
		return E.CropID == CropID;
	});

	return Entry ? Entry->Quantity : 0;
}

float AHarvestingMachine::GetTotalStoredValue() const
{
	float TotalValue = 0.0f;

	for (const FHarvestedCropEntry& Entry : StoredCrops)
	{
		TotalValue += Entry.TotalValue;
	}

	return TotalValue;
}

bool AHarvestingMachine::IsStorageFull() const
{
	return CurrentStorageCount >= Config.MaxStorageCapacity;
}

float AHarvestingMachine::GetStorageFillPercentage() const
{
	if (Config.MaxStorageCapacity <= 0)
	{
		return 0.0f;
	}

	return static_cast<float>(CurrentStorageCount) / static_cast<float>(Config.MaxStorageCapacity);
}

void AHarvestingMachine::EmptyStorage()
{
	StoredCrops.Empty();
	CurrentStorageCount = 0;

	UE_LOG(LogTemp, Log, TEXT("HarvestingMachine: Storage emptied"));
}

// ============================================================================
// POWER SYSTEM
// ============================================================================

void AHarvestingMachine::SetPowerState(bool bPowered)
{
	bHasPower = bPowered;

	if (!bHasPower && bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("HarvestingMachine: Lost power, stopping harvesting"));
		StopHarvesting();
	}
}

float AHarvestingMachine::GetCurrentPowerConsumption() const
{
	if (bIsActive && Config.bRequiresPower)
	{
		return Config.PowerConsumption;
	}

	return 0.0f;
}

bool AHarvestingMachine::HasSufficientPower() const
{
	if (!Config.bRequiresPower)
	{
		return true;
	}

	return bHasPower;
}

// ============================================================================
// CONFIGURATION
// ============================================================================

void AHarvestingMachine::SetHarvestRadius(float Radius)
{
	Config.HarvestRadius = FMath::Max(1.0f, Radius);
	UpdateHarvestZoneSize();
	UpdateManagedPlots();
}

void AHarvestingMachine::SetHarvestInterval(float Interval)
{
	Config.HarvestInterval = FMath::Max(0.1f, Interval);
}

void AHarvestingMachine::SetAutoHarvest(bool bAuto)
{
	Config.bAutoHarvest = bAuto;

	if (bAuto && !bIsActive)
	{
		StartHarvesting();
	}
	else if (!bAuto && bIsActive)
	{
		StopHarvesting();
	}
}

// ============================================================================
// STATISTICS
// ============================================================================

void AHarvestingMachine::GetHarvestStatistics(int32& OutTotalHarvested, float& OutTotalValue, float& OutPowerUsed) const
{
	OutTotalHarvested = TotalCropsHarvested;
	OutTotalValue = TotalValueHarvested;
	OutPowerUsed = TotalPowerConsumed;
}

void AHarvestingMachine::ResetStatistics()
{
	TotalCropsHarvested = 0;
	TotalValueHarvested = 0.0f;
	TotalPowerConsumed = 0.0f;
}

// ============================================================================
// PROTECTED METHODS
// ============================================================================

void AHarvestingMachine::UpdateHarvestZoneSize()
{
	if (HarvestZone)
	{
		// Convert radius to box extent (radius * 100 for cm, height 200cm)
		FVector BoxExtent(Config.HarvestRadius * 100.0f, Config.HarvestRadius * 100.0f, 200.0f);
		HarvestZone->SetBoxExtent(BoxExtent);
	}
}

void AHarvestingMachine::ProcessAutoHarvest(float DeltaTime)
{
	TimeSinceLastHarvest += DeltaTime;

	// Check if it's time for a harvest cycle
	if (TimeSinceLastHarvest >= Config.HarvestInterval)
	{
		PerformHarvestCycle();
		TimeSinceLastHarvest = 0.0f;
	}
}

void AHarvestingMachine::UpdatePowerConsumption(float DeltaTime)
{
	if (bHasPower)
	{
		// Convert watts to kWh (watts * hours / 1000)
		float PowerUsedKWh = (Config.PowerConsumption * DeltaTime) / 3600000.0f;
		TotalPowerConsumed += PowerUsedKWh;
	}
}

FHarvestedCropEntry* AHarvestingMachine::FindStorageEntry(const FString& CropID)
{
	return StoredCrops.FindByPredicate([&CropID](const FHarvestedCropEntry& E) {
		return E.CropID == CropID;
	});
}

FVector AHarvestingMachine::GetCropCellWorldPosition(AFarmPlot* Plot, FIntPoint GridPosition) const
{
	if (!Plot)
	{
		return FVector::ZeroVector;
	}

	// Calculate cell size
	FVector2D CellSize = Plot->PlotSize / static_cast<float>(Plot->GridResolution);

	// Calculate local position within plot
	FVector2D LocalPos;
	LocalPos.X = (GridPosition.X + 0.5f) * CellSize.X - (Plot->PlotSize.X * 0.5f);
	LocalPos.Y = (GridPosition.Y + 0.5f) * CellSize.Y - (Plot->PlotSize.Y * 0.5f);

	// Convert to world position
	FVector PlotLocation = Plot->GetActorLocation();
	FRotator PlotRotation = Plot->GetActorRotation();

	FVector LocalPos3D(LocalPos.X, LocalPos.Y, 0.0f);
	FVector WorldPos = PlotLocation + PlotRotation.RotateVector(LocalPos3D);

	return WorldPos;
}

// ============================================================================
// IPowerConsumer Interface Implementation
// ============================================================================

float AHarvestingMachine::GetPowerRequirement_Implementation() const
{
	return Config.PowerConsumption;
}

void AHarvestingMachine::SetPowered_Implementation(bool bInIsPowered)
{
	bool bWasPowered = bHasPower;
	bHasPower = bInIsPowered;

	// If power is lost while active, stop harvesting
	if (!bHasPower && bIsActive && Config.bRequiresPower)
	{
		UE_LOG(LogTemp, Warning, TEXT("HarvestingMachine: Power lost - stopping harvesting"));
		StopHarvesting();
	}

	// Log power state changes
	if (bWasPowered != bHasPower)
	{
		UE_LOG(LogTemp, Log, TEXT("HarvestingMachine: Power %s"), bHasPower ? TEXT("restored") : TEXT("lost"));
	}
}

bool AHarvestingMachine::IsPowered_Implementation() const
{
	return bHasPower;
}
