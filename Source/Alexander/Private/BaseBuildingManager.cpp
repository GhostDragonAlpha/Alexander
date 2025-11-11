// Copyright Epic Games, Inc. All Rights Reserved.

#include "BaseBuildingManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Math/UnrealMathUtility.h"

ABaseBuildingManager::ABaseBuildingManager()
{
	PrimaryActorTick.bCanEverTick = true;

	// Default configuration
	GridCellSize = 100.0f;
	GridDimensions = FIntPoint(100, 100);
	CurrentPower = 0.0f;
	MaxPowerCapacity = 1000.0f;
	OxygenLevel = 0.0f;
	Pressure = 0.0f;
	MaxPressure = 1.0f;
	CurrentPopulation = 0;
	MaxQueueSize = 20;
	ConstructionSpeedMultiplier = 1.0f;
}

void ABaseBuildingManager::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("BaseBuildingManager initialized: %dx%d grid"), GridDimensions.X, GridDimensions.Y);
}

void ABaseBuildingManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update construction
	UpdateConstruction(DeltaTime);

	// Update power grid
	UpdatePowerGrid();

	// Update atmosphere system
	UpdateAtmosphereSystem();

	// Update module connections
	UpdateModuleConnections();

	// Update statistics periodically
	static float StatsAccumulator = 0.0f;
	StatsAccumulator += DeltaTime;

	if (StatsAccumulator >= 5.0f) // Update every 5 seconds
	{
		StatsAccumulator = 0.0f;
		FBaseStatistics Stats = GetBaseStatistics();
		OnBaseStatisticsUpdated.Broadcast(Stats);
	}
}

void ABaseBuildingManager::Initialize(ALandClaimManager* LandManager)
{
	if (!LandManager)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid land claim manager"));
		return;
	}

	LandClaimManager = LandManager;
	UE_LOG(LogTemp, Log, TEXT("BaseBuildingManager initialized with LandClaimManager"));
}

FGuid ABaseBuildingManager::CreateModule(EBaseModuleType ModuleType, FIntPoint GridPosition, const FGuid& ClaimID)
{
	// Validate claim
	if (LandClaimManager.IsValid())
	{
		FLandClaim Claim = LandClaimManager->GetClaimByID(ClaimID);
		if (!Claim.ClaimID.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid claim ID for module creation"));
			return FGuid();
		}

		// Check if grid position is within claim bounds
		FVector WorldLocation = FVector(GridPosition.X * GridCellSize, GridPosition.Y * GridCellSize, 0.0f);
		if (!Claim.Bounds.IsInside(WorldLocation))
		{
			UE_LOG(LogTemp, Warning, TEXT("Grid position outside claim bounds"));
			return FGuid();
		}
	}

	// Check if position is valid
	EModuleSize Size = GetModuleSize(ModuleType);
	if (!IsGridPositionValid(GridPosition, Size))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid grid position for module"));
		return FGuid();
	}

	// Create module data
	FBaseModuleData NewModule;
	NewModule.ModuleType = ModuleType;
	NewModule.GridPosition = GridPosition;
	NewModule.Location = FVector(GridPosition.X * GridCellSize, GridPosition.Y * GridCellSize, 0.0f);
	NewModule.ModuleSize = Size;
	NewModule.ClaimID = ClaimID;

	// Initialize module data
	InitializeModuleData(NewModule);

	// Add to construction queue
	if (QueueModuleConstruction(NewModule))
	{
		UE_LOG(LogTemp, Log, TEXT("Module queued for construction: %s at (%d, %d)"), 
			*UEnum::GetDisplayValueAsText(ModuleType).ToString(), GridPosition.X, GridPosition.Y);
		return NewModule.ModuleID;
	}

	return FGuid();
}

bool ABaseBuildingManager::DestroyModule(const FGuid& ModuleID)
{
	int32 ModuleIndex = FindModuleIndex(ModuleID);
	if (ModuleIndex == INDEX_NONE)
	{
		return false;
	}

	FBaseModuleData ModuleData = Modules[ModuleIndex];

	// Remove from grid
	Modules.RemoveAt(ModuleIndex);
	ModuleIndexMap.Remove(ModuleID);

	// Fire event
	OnModuleDestroyed.Broadcast(ModuleData);

	UE_LOG(LogTemp, Log, TEXT("Module destroyed: %s"), *ModuleData.ModuleName);
	return true;
}

bool ABaseBuildingManager::UpgradeModule(const FGuid& ModuleID)
{
	int32 ModuleIndex = FindModuleIndex(ModuleID);
	if (ModuleIndex == INDEX_NONE)
	{
		return false;
	}

	FBaseModuleData& ModuleData = Modules[ModuleIndex];

	if (ModuleData.UpgradeLevel >= ModuleData.MaxUpgradeLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("Module already at max upgrade level"));
		return false;
	}

	// Check resources for upgrade
	TMap<FName, int32> UpgradeCost = CalculateModuleCost(ModuleData.ModuleType);
	for (const auto& ResourcePair : UpgradeCost)
	{
		int32* StoredAmount = ResourceStorage.Find(ResourcePair.Key);
		if (!StoredAmount || *StoredAmount < ResourcePair.Value)
		{
			UE_LOG(LogTemp, Warning, TEXT("Insufficient resources for upgrade"));
			return false;
		}
	}

	// Deduct resources
	for (const auto& ResourcePair : UpgradeCost)
	{
		ResourceStorage[ResourcePair.Key] -= ResourcePair.Value;
	}

	// Apply upgrade
	ModuleData.UpgradeLevel++;
	ModuleData.Efficiency = FMath::Min(ModuleData.Efficiency + 0.2f, 2.0f);
	ModuleData.MaxHealth = ModuleData.MaxHealth * 1.2f;
	ModuleData.Health = ModuleData.MaxHealth;

	// Update stats based on module type
	switch (ModuleData.ModuleType)
	{
		case EBaseModuleType::SolarPanel:
		case EBaseModuleType::FusionReactor:
		case EBaseModuleType::GeothermalGenerator:
			ModuleData.PowerGeneration *= 1.5f;
			break;

		case EBaseModuleType::HabitatQuarters:
			ModuleData.PopulationCapacity = FMath::RoundToInt(ModuleData.PopulationCapacity * 1.3f);
			break;

		case EBaseModuleType::RawMaterialsStorage:
		case EBaseModuleType::ProcessedGoodsStorage:
		case EBaseModuleType::ColdStorage:
		case EBaseModuleType::SecureStorage:
			ModuleData.StorageCapacity = FMath::RoundToInt(ModuleData.StorageCapacity * 1.5f);
			break;

		case EBaseModuleType::Turret:
		case EBaseModuleType::ShieldGenerator:
			ModuleData.DefenseRating *= 1.4f;
			break;
	}

	// Fire event
	OnModuleUpgraded.Broadcast(ModuleData);

	UE_LOG(LogTemp, Log, TEXT("Module upgraded to level %d: %s"), ModuleData.UpgradeLevel, *ModuleData.ModuleName);
	return true;
}

bool ABaseBuildingManager::RepairModule(const FGuid& ModuleID, float RepairAmount)
{
	int32 ModuleIndex = FindModuleIndex(ModuleID);
	if (ModuleIndex == INDEX_NONE)
	{
		return false;
	}

	FBaseModuleData& ModuleData = Modules[ModuleIndex];

	float OldHealth = ModuleData.Health;
	ModuleData.Health = FMath::Clamp(ModuleData.Health + RepairAmount, 0.0f, ModuleData.MaxHealth);
	float ActualRepair = ModuleData.Health - OldHealth;

	if (ActualRepair > 0.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("Module repaired: %s (+%f health)"), *ModuleData.ModuleName, ActualRepair);
		return true;
	}

	return false;
}

void ABaseBuildingManager::DamageModule(const FGuid& ModuleID, float DamageAmount)
{
	int32 ModuleIndex = FindModuleIndex(ModuleID);
	if (ModuleIndex == INDEX_NONE)
	{
		return;
	}

	FBaseModuleData& ModuleData = Modules[ModuleIndex];

	float OldHealth = ModuleData.Health;
	ModuleData.Health = FMath::Max(0.0f, ModuleData.Health - DamageAmount);

	// Update status if destroyed
	if (ModuleData.Health <= 0.0f)
	{
		ModuleData.Status = EModuleStatus::Damaged;
	}

	// Fire event
	OnModuleDamaged.Broadcast(ModuleData);

	UE_LOG(LogTemp, Warning, TEXT("Module damaged: %s (-%f health)"), *ModuleData.ModuleName, DamageAmount);
}

bool ABaseBuildingManager::ConnectModules(const FGuid& ModuleID1, const FGuid& ModuleID2, EConnectionType ConnectionType)
{
	int32 Index1 = FindModuleIndex(ModuleID1);
	int32 Index2 = FindModuleIndex(ModuleID2);

	if (Index1 == INDEX_NONE || Index2 == INDEX_NONE)
	{
		return false;
	}

	FBaseModuleData& Module1 = Modules[Index1];
	FBaseModuleData& Module2 = Modules[Index2];

	// Create connection for module 1
	FModuleConnection Connection1;
	Connection1.ConnectionType = ConnectionType;
	Connection1.ConnectedModuleID = ModuleID2;
	Connection1.bIsActive = true;
	Connection1.ConnectionStrength = 1.0f;

	// Create connection for module 2
	FModuleConnection Connection2;
	Connection2.ConnectionType = ConnectionType;
	Connection2.ConnectedModuleID = ModuleID1;
	Connection2.bIsActive = true;
	Connection2.ConnectionStrength = 1.0f;

	// Add connections
	Module1.Connections.Add(Connection1);
	Module2.Connections.Add(Connection2);

	UE_LOG(LogTemp, Log, TEXT("Modules connected: %s <-> %s (%s)"), 
		*Module1.ModuleName, *Module2.ModuleName, *UEnum::GetDisplayValueAsText(ConnectionType).ToString());
	return true;
}

bool ABaseBuildingManager::DisconnectModules(const FGuid& ModuleID1, const FGuid& ModuleID2)
{
	int32 Index1 = FindModuleIndex(ModuleID1);
	int32 Index2 = FindModuleIndex(ModuleID2);

	if (Index1 == INDEX_NONE || Index2 == INDEX_NONE)
	{
		return false;
	}

	FBaseModuleData& Module1 = Modules[Index1];
	FBaseModuleData& Module2 = Modules[Index2];

	// Remove connections
	Module1.Connections.RemoveAll([&](const FModuleConnection& Connection) {
		return Connection.ConnectedModuleID == ModuleID2;
	});

	Module2.Connections.RemoveAll([&](const FModuleConnection& Connection) {
		return Connection.ConnectedModuleID == ModuleID1;
	});

	UE_LOG(LogTemp, Log, TEXT("Modules disconnected: %s <-> %s"), *Module1.ModuleName, *Module2.ModuleName);
	return true;
}

FBaseModuleData ABaseBuildingManager::GetModule(const FGuid& ModuleID) const
{
	const int32* IndexPtr = ModuleIndexMap.Find(ModuleID);
	if (IndexPtr)
	{
		return Modules[*IndexPtr];
	}
	return FBaseModuleData();
}

TArray<FBaseModuleData> ABaseBuildingManager::GetAllModules() const
{
	return Modules;
}

TArray<FBaseModuleData> ABaseBuildingManager::GetModulesByType(EBaseModuleType ModuleType) const
{
	TArray<FBaseModuleData> Result;
	for (const FBaseModuleData& Module : Modules)
	{
		if (Module.ModuleType == ModuleType)
		{
			Result.Add(Module);
		}
	}
	return Result;
}

FBaseStatistics ABaseBuildingManager::GetBaseStatistics() const
{
	FBaseStatistics Stats;
	Stats.TotalModules = Modules.Num();

	for (const FBaseModuleData& Module : Modules)
	{
		switch (Module.Status)
		{
			case EModuleStatus::Operational:
				Stats.OperationalModules++;
				break;
			case EModuleStatus::UnderConstruction:
				Stats.ModulesUnderConstruction++;
				break;
		}

		// Power calculations
		Stats.TotalPowerConsumption += Module.PowerConsumption;
		Stats.TotalPowerGeneration += Module.PowerGeneration;

		// Storage calculations
		Stats.TotalStorageCapacity += Module.StorageCapacity;

		// Population calculations
		Stats.TotalPopulationCapacity += Module.PopulationCapacity;

		// Defense calculations
		Stats.TotalDefenseRating += Module.DefenseRating;

		// Base value (simplified)
		Stats.BaseValue += CalculateModuleValue(Module);
	}

	Stats.NetPower = Stats.TotalPowerGeneration - Stats.TotalPowerConsumption;
	Stats.CurrentPopulation = CurrentPopulation;
	Stats.MonthlyMaintenanceCost = CalculateMaintenanceCost();

	return Stats;
}

TArray<FConstructionQueueItem> ABaseBuildingManager::GetConstructionQueue() const
{
	return ConstructionQueue;
}

bool ABaseBuildingManager::QueueModuleConstruction(const FBaseModuleData& ModuleData, int32 Priority)
{
	if (ConstructionQueue.Num() >= MaxQueueSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("Construction queue is full"));
		return false;
	}

	FConstructionQueueItem QueueItem;
	QueueItem.ModuleData = ModuleData;
	QueueItem.StartTime = GetWorld()->GetTimeSeconds();
	QueueItem.Priority = Priority;
	QueueItem.bIsPaused = false;

	// Insert based on priority (higher priority first)
	int32 InsertIndex = 0;
	for (; InsertIndex < ConstructionQueue.Num(); InsertIndex++)
	{
		if (Priority > ConstructionQueue[InsertIndex].Priority)
		{
			break;
		}
	}

	ConstructionQueue.Insert(QueueItem, InsertIndex);
	UE_LOG(LogTemp, Log, TEXT("Module added to construction queue: %s (Priority: %d)"), *ModuleData.ModuleName, Priority);
	return true;
}

bool ABaseBuildingManager::RemoveFromConstructionQueue(const FGuid& ModuleID)
{
	for (int32 i = 0; i < ConstructionQueue.Num(); i++)
	{
		if (ConstructionQueue[i].ModuleData.ModuleID == ModuleID)
		{
			ConstructionQueue.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("Module removed from construction queue: %s"), *ModuleID.ToString());
			return true;
		}
	}
	return false;
}

bool ABaseBuildingManager::PauseConstruction(const FGuid& ModuleID, bool bPause)
{
	for (FConstructionQueueItem& QueueItem : ConstructionQueue)
	{
		if (QueueItem.ModuleData.ModuleID == ModuleID)
		{
			QueueItem.bIsPaused = bPause;
			UE_LOG(LogTemp, Log, TEXT("Construction %s for module: %s"), 
				bPause ? TEXT("paused") : TEXT("resumed"), *ModuleID.ToString());
			return true;
		}
	}
	return false;
}

bool ABaseBuildingManager::IsGridPositionValid(FIntPoint GridPosition, EModuleSize Size) const
{
	// Check grid bounds
	int32 SizeInCells = GetSizeInCells(Size);
	if (GridPosition.X < 0 || GridPosition.Y < 0 || 
		GridPosition.X + SizeInCells > GridDimensions.X || 
		GridPosition.Y + SizeInCells > GridDimensions.Y)
	{
		return false;
	}

	// Check for overlapping modules
	return !CanPlaceModuleAt(GridPosition, Size);
}

TArray<FIntPoint> ABaseBuildingManager::FindValidGridPositions(EModuleSize Size) const
{
	TArray<FIntPoint> ValidPositions;
	int32 SizeInCells = GetSizeInCells(Size);

	for (int32 X = 0; X <= GridDimensions.X - SizeInCells; X++)
	{
		for (int32 Y = 0; Y <= GridDimensions.Y - SizeInCells; Y++)
		{
			FIntPoint Position(X, Y);
			if (IsGridPositionValid(Position, Size))
			{
				ValidPositions.Add(Position);
			}
		}
	}

	return ValidPositions;
}

FBaseModuleData ABaseBuildingManager::GetModuleAtGridPosition(FIntPoint GridPosition) const
{
	for (const FBaseModuleData& Module : Modules)
	{
		if (Module.GridPosition == GridPosition)
		{
			return Module;
		}
	}
	return FBaseModuleData();
}

void ABaseBuildingManager::InitializeModuleData(FBaseModuleData& ModuleData)
{
	// Set module name based on type
	ModuleData.ModuleName = UEnum::GetDisplayValueAsText(ModuleData.ModuleType).ToString();

	// Set default values based on module type
	switch (ModuleData.ModuleType)
	{
		case EBaseModuleType::HabitatQuarters:
			ModuleData.MaxHealth = 150.0f;
			ModuleData.PowerConsumption = 2.0f;
			ModuleData.PopulationCapacity = 4;
			break;

		case EBaseModuleType::LifeSupport:
			ModuleData.MaxHealth = 200.0f;
			ModuleData.PowerConsumption = 5.0f;
			break;

		case EBaseModuleType::SolarPanel:
			ModuleData.MaxHealth = 100.0f;
			ModuleData.PowerConsumption = 0.0f;
			ModuleData.PowerGeneration = 10.0f;
			break;

		case EBaseModuleType::FusionReactor:
			ModuleData.MaxHealth = 300.0f;
			ModuleData.PowerConsumption = 2.0f;
			ModuleData.PowerGeneration = 100.0f;
			break;

		case EBaseModuleType::RawMaterialsStorage:
			ModuleData.MaxHealth = 120.0f;
			ModuleData.PowerConsumption = 0.5f;
			ModuleData.StorageCapacity = 1000;
			break;

		case EBaseModuleType::Turret:
			ModuleData.MaxHealth = 180.0f;
			ModuleData.PowerConsumption = 3.0f;
			ModuleData.DefenseRating = 25.0f;
			break;

		case EBaseModuleType::ShieldGenerator:
			ModuleData.MaxHealth = 250.0f;
			ModuleData.PowerConsumption = 8.0f;
			ModuleData.DefenseRating = 50.0f;
			break;

		default:
			ModuleData.MaxHealth = 100.0f;
			ModuleData.PowerConsumption = 1.0f;
			break;
	}

	ModuleData.Health = ModuleData.MaxHealth;
	ModuleData.ConstructionTime = CalculateConstructionTime(ModuleData.ModuleType);
	ModuleData.RequiredResources = CalculateModuleCost(ModuleData.ModuleType);
}

TMap<FName, int32> ABaseBuildingManager::CalculateModuleCost(EBaseModuleType ModuleType) const
{
	TMap<FName, int32> Cost;

	switch (ModuleType)
	{
		case EBaseModuleType::HabitatQuarters:
			Cost.Add(FName("Steel"), 50);
			Cost.Add(FName("Electronics"), 10);
			break;

		case EBaseModuleType::SolarPanel:
			Cost.Add(FName("Steel"), 20);
			Cost.Add(FName("Silicon"), 30);
			Cost.Add(FName("Copper"), 10);
			break;

		case EBaseModuleType::FusionReactor:
			Cost.Add(FName("Steel"), 200);
			Cost.Add(FName("Electronics"), 100);
			Cost.Add(FName("Titanium"), 50);
			break;

		case EBaseModuleType::RawMaterialsStorage:
			Cost.Add(FName("Steel"), 30);
			Cost.Add(FName("Concrete"), 20);
			break;

		case EBaseModuleType::Turret:
			Cost.Add(FName("Steel"), 40);
			Cost.Add(FName("Electronics"), 15);
			Cost.Add(FName("Ammunition"), 20);
			break;

		default:
			Cost.Add(FName("Steel"), 25);
			Cost.Add(FName("Electronics"), 5);
			break;
	}

	return Cost;
}

float ABaseBuildingManager::CalculateConstructionTime(EBaseModuleType ModuleType) const
{
	switch (ModuleType)
	{
		case EBaseModuleType::HabitatQuarters:
			return 60.0f; // 1 minute

		case EBaseModuleType::SolarPanel:
			return 30.0f; // 30 seconds

		case EBaseModuleType::FusionReactor:
			return 300.0f; // 5 minutes

		case EBaseModuleType::RawMaterialsStorage:
			return 45.0f; // 45 seconds

		case EBaseModuleType::Turret:
			return 50.0f; // 50 seconds

		default:
			return 60.0f; // 1 minute
	}
}

void ABaseBuildingManager::UpdateConstruction(float DeltaTime)
{
	for (int32 i = ConstructionQueue.Num() - 1; i >= 0; i--)
	{
		FConstructionQueueItem& QueueItem = ConstructionQueue[i];

		if (QueueItem.bIsPaused)
		{
			continue;
		}

		// Update construction progress
		float ProgressDelta = (DeltaTime / QueueItem.ModuleData.ConstructionTime) * ConstructionSpeedMultiplier;
		QueueItem.ModuleData.ConstructionProgress = FMath::Clamp(QueueItem.ModuleData.ConstructionProgress + ProgressDelta, 0.0f, 1.0f);

		// Check if construction is complete
		if (QueueItem.ModuleData.ConstructionProgress >= 1.0f)
		{
			// Add module to active modules
			QueueItem.ModuleData.Status = EModuleStatus::Operational;
			QueueItem.ModuleData.bIsPowered = true;
			Modules.Add(QueueItem.ModuleData);
			ModuleIndexMap.Add(QueueItem.ModuleData.ModuleID, Modules.Num() - 1);

			// Fire event
			OnModuleConstructed.Broadcast(QueueItem.ModuleData);

			// Remove from queue
			ConstructionQueue.RemoveAt(i);

			UE_LOG(LogTemp, Log, TEXT("Module construction completed: %s"), *QueueItem.ModuleData.ModuleName);
		}
	}
}

void ABaseBuildingManager::UpdatePowerGrid()
{
	float TotalConsumption = 0.0f;
	float TotalGeneration = 0.0f;

	for (const FBaseModuleData& Module : Modules)
	{
		if (Module.Status == EModuleStatus::Operational)
		{
			TotalConsumption += Module.PowerConsumption;
			TotalGeneration += Module.PowerGeneration;
		}
	}

	CurrentPower = TotalGeneration - TotalConsumption;

	// Update module power status
	for (FBaseModuleData& Module : Modules)
	{
		if (Module.Status == EModuleStatus::Operational)
		{
			Module.bIsPowered = CurrentPower >= 0.0f;
		}
	}

	// Fire event
	OnPowerGridUpdated.Broadcast(CurrentPower, MaxPowerCapacity);
}

void ABaseBuildingManager::UpdateAtmosphereSystem()
{
	float TotalOxygenProduction = 0.0f;
	float TotalOxygenConsumption = 0.0f;

	for (const FBaseModuleData& Module : Modules)
	{
		if (Module.Status == EModuleStatus::Operational && Module.bIsPowered)
		{
			// Life support modules produce oxygen
			if (Module.ModuleType == EBaseModuleType::LifeSupport)
			{
				TotalOxygenProduction += Module.Efficiency * 10.0f;
			}

			// Habitat modules consume oxygen
			if (Module.ModuleType == EBaseModuleType::HabitatQuarters)
			{
				TotalOxygenConsumption += Module.PopulationCapacity * 0.5f;
			}
		}
	}

	// Update oxygen level
	float OxygenDelta = (TotalOxygenProduction - TotalOxygenConsumption) * 0.01f;
	OxygenLevel = FMath::Clamp(OxygenLevel + OxygenDelta, 0.0f, 1.0f);

	// Update pressure
	Pressure = FMath::Clamp(OxygenLevel * MaxPressure, 0.0f, MaxPressure);

	// Update module atmosphere status
	for (FBaseModuleData& Module : Modules)
	{
		if (Module.Status == EModuleStatus::Operational)
		{
			Module.bHasAtmosphere = Pressure > 0.5f;
		}
	}

	// Fire event
	OnAtmosphereSystemUpdated.Broadcast(OxygenLevel, Pressure);
}

void ABaseBuildingManager::UpdateModuleConnections()
{
	// In a real implementation, this would update power grid connections,
	// atmosphere connections, data networks, etc.
	// For now, just ensure connected modules share status
}

int32 ABaseBuildingManager::FindModuleIndex(const FGuid& ModuleID) const
{
	const int32* IndexPtr = ModuleIndexMap.Find(ModuleID);
	return IndexPtr ? *IndexPtr : INDEX_NONE;
}

bool ABaseBuildingManager::CanPlaceModuleAt(FIntPoint GridPosition, EModuleSize Size, const FGuid& ExcludeModuleID) const
{
	int32 SizeInCells = GetSizeInCells(Size);

	for (const FBaseModuleData& Module : Modules)
	{
		if (Module.ModuleID == ExcludeModuleID)
		{
			continue;
		}

		int32 ModuleSizeInCells = GetSizeInCells(Module.ModuleSize);

		// Check for overlap
		if (GridPosition.X < Module.GridPosition.X + ModuleSizeInCells &&
			GridPosition.X + SizeInCells > Module.GridPosition.X &&
			GridPosition.Y < Module.GridPosition.Y + ModuleSizeInCells &&
			GridPosition.Y + SizeInCells > Module.GridPosition.Y)
		{
			return false;
		}
	}

	return true;
}

TArray<FBaseModuleData> ABaseBuildingManager::GetModulesInRange(FVector Location, float Range) const
{
	TArray<FBaseModuleData> Result;
	for (const FBaseModuleData& Module : Modules)
	{
		float Distance = FVector::Dist(Location, Module.Location);
		if (Distance <= Range)
		{
			Result.Add(Module);
		}
	}
	return Result;
}

float ABaseBuildingManager::CalculateGridDistance(FIntPoint Pos1, FIntPoint Pos2) const
{
	return FVector2D(Pos1 - Pos2).Size();
}

EModuleSize ABaseBuildingManager::GetModuleSize(EBaseModuleType ModuleType) const
{
	// Most modules are 1x1, some are larger
	switch (ModuleType)
	{
		case EBaseModuleType::FusionReactor:
		case EBaseModuleType::ShieldGenerator:
			return EModuleSize::Medium2x2;

		case EBaseModuleType::CommandCenter:
			return EModuleSize::Large3x3;

		default:
			return EModuleSize::Small1x1;
	}
}

int32 ABaseBuildingManager::GetSizeInCells(EModuleSize Size) const
{
	switch (Size)
	{
		case EModuleSize::Small1x1:
			return 1;
		case EModuleSize::Medium2x2:
			return 2;
		case EModuleSize::Large3x3:
			return 3;
		case EModuleSize::Huge4x4:
			return 4;
		default:
			return 1;
	}
}

float ABaseBuildingManager::CalculateModuleValue(const FBaseModuleData& Module) const
{
	float BaseValue = 1000.0f; // Base value for any module

	// Adjust based on type
	switch (Module.ModuleType)
	{
		case EBaseModuleType::FusionReactor:
			BaseValue += 5000.0f;
			break;
		case EBaseModuleType::ShieldGenerator:
			BaseValue += 3000.0f;
			break;
		case EBaseModuleType::CommandCenter:
			BaseValue += 4000.0f;
			break;
		default:
			BaseValue += 500.0f;
			break;
	}

	// Adjust based on upgrade level
	BaseValue *= (1.0f + (Module.UpgradeLevel - 1) * 0.3f);

	// Adjust based on efficiency
	BaseValue *= Module.Efficiency;

	return BaseValue;
}

float ABaseBuildingManager::CalculateMaintenanceCost() const
{
	float TotalCost = 0.0f;

	for (const FBaseModuleData& Module : Modules)
	{
		if (Module.Status == EModuleStatus::Operational)
		{
			// Base maintenance cost per module
			float ModuleCost = 10.0f;

			// Adjust based on type
			switch (Module.ModuleType)
			{
				case EBaseModuleType::FusionReactor:
					ModuleCost = 100.0f;
					break;
				case EBaseModuleType::LifeSupport:
					ModuleCost = 50.0f;
					break;
				case EBaseModuleType::ShieldGenerator:
					ModuleCost = 75.0f;
					break;
				default:
					ModuleCost = 10.0f;
					break;
			}

			// Adjust based on upgrade level
			ModuleCost *= (1.0f + (Module.UpgradeLevel - 1) * 0.2f);

			TotalCost += ModuleCost;
		}
	}

	return TotalCost;
}