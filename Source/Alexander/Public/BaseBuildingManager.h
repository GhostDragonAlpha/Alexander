// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "LandClaimManager.h"
#include "BaseBuildingManager.generated.h"

// Forward declarations
class ABaseModule;
class APowerGrid;
class AAtmosphericSystem;

// Base module types
UENUM(BlueprintType)
enum class EBaseModuleType : uint8
{
	// Habitat modules
	HabitatQuarters UMETA(DisplayName = "Habitat Quarters"),
	LifeSupport UMETA(DisplayName = "Life Support"),
	MedicalBay UMETA(DisplayName = "Medical Bay"),
	Galley UMETA(DisplayName = "Galley"),

	// Storage modules
	RawMaterialsStorage UMETA(DisplayName = "Raw Materials Storage"),
	ProcessedGoodsStorage UMETA(DisplayName = "Processed Goods Storage"),
	ColdStorage UMETA(DisplayName = "Cold Storage"),
	SecureStorage UMETA(DisplayName = "Secure Storage"),

	// Processing modules
	Refinery UMETA(DisplayName = "Refinery"),
	Factory UMETA(DisplayName = "Factory"),
	Workshop UMETA(DisplayName = "Workshop"),
	Laboratory UMETA(DisplayName = "Laboratory"),

	// Defense modules
	Turret UMETA(DisplayName = "Turret"),
	ShieldGenerator UMETA(DisplayName = "Shield Generator"),
	SecurityStation UMETA(DisplayName = "Security Station"),
	PatrolDroneBay UMETA(DisplayName = "Patrol Drone Bay"),

	// Power modules
	SolarPanel UMETA(DisplayName = "Solar Panel"),
	FusionReactor UMETA(DisplayName = "Fusion Reactor"),
	GeothermalGenerator UMETA(DisplayName = "Geothermal Generator"),
	BatteryBank UMETA(DisplayName = "Battery Bank"),

	// Utility modules
	Airlock UMETA(DisplayName = "Airlock"),
	Communications UMETA(DisplayName = "Communications"),
	CommandCenter UMETA(DisplayName = "Command Center"),
	RepairBay UMETA(DisplayName = "Repair Bay")
};

// Module size
UENUM(BlueprintType)
enum class EModuleSize : uint8
{
	Small1x1 UMETA(DisplayName = "1x1 (Small)"),
	Medium2x2 UMETA(DisplayName = "2x2 (Medium)"),
	Large3x3 UMETA(DisplayName = "3x3 (Large)"),
	Huge4x4 UMETA(DisplayName = "4x4 (Huge)")
};

// Module status
UENUM(BlueprintType)
enum class EModuleStatus : uint8
{
	Planned UMETA(DisplayName = "Planned"),
	UnderConstruction UMETA(DisplayName = "Under Construction"),
	Operational UMETA(DisplayName = "Operational"),
	Damaged UMETA(DisplayName = "Damaged"),
	Offline UMETA(DisplayName = "Offline"),
	Upgrading UMETA(DisplayName = "Upgrading")
};

// Connection type
UENUM(BlueprintType)
enum class EConnectionType : uint8
{
	Power UMETA(DisplayName = "Power"),
	Atmosphere UMETA(DisplayName = "Atmosphere"),
	Data UMETA(DisplayName = "Data"),
	Conveyor UMETA(DisplayName = "Conveyor"),
	Passage UMETA(DisplayName = "Passage")
};

// Module connection
USTRUCT(BlueprintType)
struct FModuleConnection
{
	GENERATED_BODY()

	// Connection type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	EConnectionType ConnectionType;

	// Connected module ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	FGuid ConnectedModuleID;

	// Is connection active?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	bool bIsActive;

	// Connection strength (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	float ConnectionStrength;

	FModuleConnection()
	{
		ConnectionType = EConnectionType::Power;
		bIsActive = false;
		ConnectionStrength = 0.0f;
	}
};

// Base module data
USTRUCT(BlueprintType)
struct FBaseModuleData
{
	GENERATED_BODY()

	// Module ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	FGuid ModuleID;

	// Module type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	EBaseModuleType ModuleType;

	// Module name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	FString ModuleName;

	// Grid position
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
	FIntPoint GridPosition;

	// World location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
	FVector Location;

	// Rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
	FRotator Rotation;

	// Module size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	EModuleSize ModuleSize;

	// Module status
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	EModuleStatus Status;

	// Health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health;

	// Max health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth;

	// Power consumption
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float PowerConsumption;

	// Power generation (for generators)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float PowerGeneration;

	// Storage capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage")
	int32 StorageCapacity;

	// Population capacity (for habitat modules)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitat")
	int32 PopulationCapacity;

	// Defense rating
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	float DefenseRating;

	// Construction progress (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	float ConstructionProgress;

	// Construction time in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	float ConstructionTime;

	// Is module connected to power grid?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	bool bIsPowered;

	// Is module connected to atmosphere system?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	bool bHasAtmosphere;

	// Module connections
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connections")
	TArray<FModuleConnection> Connections;

	// Required resources for construction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	TMap<FName, int32> RequiredResources;

	// Current resources stored
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	TMap<FName, int32> StoredResources;

	// Module efficiency (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float Efficiency;

	// Upgrade level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades")
	int32 UpgradeLevel;

	// Max upgrade level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades")
	int32 MaxUpgradeLevel;

	// Land claim this module is on
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Claim")
	FGuid ClaimID;

	FBaseModuleData()
	{
		ModuleID = FGuid::NewGuid();
		Status = EModuleStatus::Planned;
		Health = 100.0f;
		MaxHealth = 100.0f;
		PowerConsumption = 0.0f;
		PowerGeneration = 0.0f;
		StorageCapacity = 0;
		PopulationCapacity = 0;
		DefenseRating = 0.0f;
		ConstructionProgress = 0.0f;
		ConstructionTime = 60.0f;
		bIsPowered = false;
		bHasAtmosphere = false;
		Efficiency = 1.0f;
		UpgradeLevel = 1;
		MaxUpgradeLevel = 3;
	}
};

// Base statistics
USTRUCT(BlueprintType)
struct FBaseStatistics
{
	GENERATED_BODY()

	// Total number of modules
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 TotalModules;

	// Number of operational modules
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 OperationalModules;

	// Number of modules under construction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 ModulesUnderConstruction;

	// Total power consumption
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float TotalPowerConsumption;

	// Total power generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float TotalPowerGeneration;

	// Net power (generation - consumption)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float NetPower;

	// Total storage capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage")
	int32 TotalStorageCapacity;

	// Current storage used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage")
	int32 CurrentStorageUsed;

	// Total population capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitat")
	int32 TotalPopulationCapacity;

	// Current population
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitat")
	int32 CurrentPopulation;

	// Total defense rating
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	float TotalDefenseRating;

	// Base value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	float BaseValue;

	// Monthly maintenance cost
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	float MonthlyMaintenanceCost;

	FBaseStatistics()
	{
		TotalModules = 0;
		OperationalModules = 0;
		ModulesUnderConstruction = 0;
		TotalPowerConsumption = 0.0f;
		TotalPowerGeneration = 0.0f;
		NetPower = 0.0f;
		TotalStorageCapacity = 0;
		CurrentStorageUsed = 0;
		TotalPopulationCapacity = 0;
		CurrentPopulation = 0;
		TotalDefenseRating = 0.0f;
		BaseValue = 0.0f;
		MonthlyMaintenanceCost = 0.0f;
	}
};

// Construction queue item
USTRUCT(BlueprintType)
struct FConstructionQueueItem
{
	GENERATED_BODY()

	// Module data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	FBaseModuleData ModuleData;

	// Construction start time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	float StartTime;

	// Is construction paused?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	bool bIsPaused;

	// Construction priority (higher = more important)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	int32 Priority;

	FConstructionQueueItem()
	{
		StartTime = 0.0f;
		bIsPaused = false;
		Priority = 0;
	}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModuleConstructed, const FBaseModuleData&, ModuleData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModuleDestroyed, const FBaseModuleData&, ModuleData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModuleUpgraded, const FBaseModuleData&, ModuleData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModuleDamaged, const FBaseModuleData&, ModuleData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBaseStatisticsUpdated, const FBaseStatistics&, Statistics);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPowerGridUpdated, float, CurrentPower, float, MaxPower);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAtmosphereSystemUpdated, float, OxygenLevel, float, Pressure);

/**
 * Base Building Manager - Handles modular base construction and management
 */
UCLASS()
class ALEXANDER_API ABaseBuildingManager : public AActor
{
	GENERATED_BODY()

public:
	ABaseBuildingManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Initialize base building manager
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	void Initialize(ALandClaimManager* LandManager);

	// Create a new base module
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	FGuid CreateModule(EBaseModuleType ModuleType, FIntPoint GridPosition, const FGuid& ClaimID);

	// Destroy a module
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	bool DestroyModule(const FGuid& ModuleID);

	// Upgrade a module
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	bool UpgradeModule(const FGuid& ModuleID);

	// Repair a module
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	bool RepairModule(const FGuid& ModuleID, float RepairAmount);

	// Damage a module
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	void DamageModule(const FGuid& ModuleID, float DamageAmount);

	// Connect two modules
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	bool ConnectModules(const FGuid& ModuleID1, const FGuid& ModuleID2, EConnectionType ConnectionType);

	// Disconnect two modules
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	bool DisconnectModules(const FGuid& ModuleID1, const FGuid& ModuleID2);

	// Get module by ID
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	FBaseModuleData GetModule(const FGuid& ModuleID) const;

	// Get all modules
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	TArray<FBaseModuleData> GetAllModules() const;

	// Get modules by type
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	TArray<FBaseModuleData> GetModulesByType(EBaseModuleType ModuleType) const;

	// Get base statistics
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	FBaseStatistics GetBaseStatistics() const;

	// Get construction queue
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	TArray<FConstructionQueueItem> GetConstructionQueue() const;

	// Add module to construction queue
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	bool QueueModuleConstruction(const FBaseModuleData& ModuleData, int32 Priority = 0);

	// Remove module from construction queue
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	bool RemoveFromConstructionQueue(const FGuid& ModuleID);

	// Pause/resume construction
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	bool PauseConstruction(const FGuid& ModuleID, bool bPause);

	// Check if grid position is valid
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	bool IsGridPositionValid(FIntPoint GridPosition, EModuleSize Size) const;

	// Find valid grid positions for module
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	TArray<FIntPoint> FindValidGridPositions(EModuleSize Size) const;

	// Get module at grid position
	UFUNCTION(BlueprintCallable, Category = "Base Building")
	FBaseModuleData GetModuleAtGridPosition(FIntPoint GridPosition) const;

	// Get power grid status
	UFUNCTION(BlueprintCallable, Category = "Power")
	float GetCurrentPower() const { return CurrentPower; }

	UFUNCTION(BlueprintCallable, Category = "Power")
	float GetMaxPower() const { return MaxPowerCapacity; }

	// Get atmosphere system status
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	float GetOxygenLevel() const { return OxygenLevel; }

	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	float GetPressure() const { return Pressure; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Base Building Events")
	FOnModuleConstructed OnModuleConstructed;

	UPROPERTY(BlueprintAssignable, Category = "Base Building Events")
	FOnModuleDestroyed OnModuleDestroyed;

	UPROPERTY(BlueprintAssignable, Category = "Base Building Events")
	FOnModuleUpgraded OnModuleUpgraded;

	UPROPERTY(BlueprintAssignable, Category = "Base Building Events")
	FOnModuleDamaged OnModuleDamaged;

	UPROPERTY(BlueprintAssignable, Category = "Base Building Events")
	FOnBaseStatisticsUpdated OnBaseStatisticsUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Base Building Events")
	FOnPowerGridUpdated OnPowerGridUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Base Building Events")
	FOnAtmosphereSystemUpdated OnAtmosphereSystemUpdated;

protected:
	// Land claim manager
	UPROPERTY()
	TWeakObjectPtr<ALandClaimManager> LandClaimManager;

	// All base modules
	UPROPERTY()
	TArray<FBaseModuleData> Modules;

	// Module lookup by ID
	UPROPERTY()
	TMap<FGuid, int32> ModuleIndexMap;

	// Construction queue
	UPROPERTY()
	TArray<FConstructionQueueItem> ConstructionQueue;

	// Grid size (meters per grid cell)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float GridCellSize;

	// Base grid dimensions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FIntPoint GridDimensions;

	// Power grid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float CurrentPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float MaxPowerCapacity;

	// Atmosphere system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	float OxygenLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	float Pressure;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	float MaxPressure;

	// Resource storage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	TMap<FName, int32> ResourceStorage;

	// Population
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
	int32 CurrentPopulation;

	// Maximum construction queue size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 MaxQueueSize;

	// Construction speed multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float ConstructionSpeedMultiplier;

private:
	// Initialize module data
	void InitializeModuleData(FBaseModuleData& ModuleData);

	// Calculate module cost
	TMap<FName, int32> CalculateModuleCost(EBaseModuleType ModuleType) const;

	// Calculate construction time
	float CalculateConstructionTime(EBaseModuleType ModuleType) const;

	// Update construction progress
	void UpdateConstruction(float DeltaTime);

	// Update power grid
	void UpdatePowerGrid();

	// Update atmosphere system
	void UpdateAtmosphereSystem();

	// Update module connections
	void UpdateModuleConnections();

	// Find module index by ID
	int32 FindModuleIndex(const FGuid& ModuleID) const;

	// Check if module can be placed at grid position
	bool CanPlaceModuleAt(FIntPoint GridPosition, EModuleSize Size, const FGuid& ExcludeModuleID = FGuid()) const;

	// Get modules in range
	TArray<FBaseModuleData> GetModulesInRange(FVector Location, float Range) const;

	// Calculate distance between grid positions
	float CalculateGridDistance(FIntPoint Pos1, FIntPoint Pos2) const;

	// Get module size for a given type
	EModuleSize GetModuleSize(EBaseModuleType ModuleType) const;

	// Get size in cells for a given module size
	int32 GetSizeInCells(EModuleSize Size) const;

	// Calculate module value
	float CalculateModuleValue(const FBaseModuleData& Module) const;

	// Calculate maintenance cost
	float CalculateMaintenanceCost() const;
};