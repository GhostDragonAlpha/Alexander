// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseBuildingManager.h"
#include "PlanetaryFarmingSystem.h"
#include "ProductionChainManager.generated.h"

// Forward declarations
class AProductionModule;
class AConveyorBelt;
class AStorageSilo;

// Production tier
UENUM(BlueprintType)
enum class EProductionTier : uint8
{
	Tier1_RawProcessing UMETA(DisplayName = "Tier 1 - Raw Processing"),
	Tier2_ComponentManufacturing UMETA(DisplayName = "Tier 2 - Component Manufacturing"),
	Tier3_AdvancedProducts UMETA(DisplayName = "Tier 3 - Advanced Products"),
	Tier4_Specialized UMETA(DisplayName = "Tier 4 - Specialized")
};

// Production chain type
UENUM(BlueprintType)
enum class EProductionChainType : uint8
{
	FoodProcessing UMETA(DisplayName = "Food Processing"),
	MedicineProduction UMETA(DisplayName = "Medicine Production"),
	MaterialRefining UMETA(DisplayName = "Material Refining"),
	ComponentManufacturing UMETA(DisplayName = "Component Manufacturing"),
	WeaponProduction UMETA(DisplayName = "Weapon Production"),
	ShipPartProduction UMETA(DisplayName = "Ship Part Production"),
	ElectronicsManufacturing UMETA(DisplayName = "Electronics Manufacturing"),
	ResearchDevelopment UMETA(DisplayName = "Research & Development")
};

// Production recipe
USTRUCT(BlueprintType)
struct FProductionRecipe
{
	GENERATED_BODY()

	// Recipe name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FString RecipeName;

	// Input resources
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inputs")
	TMap<FName, int32> InputResources;

	// Output resources
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outputs")
	TMap<FName, int32> OutputResources;

	// Production time (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	float ProductionTime;

	// Required tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	EProductionTier RequiredTier;

	// Required module type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	EBaseModuleType RequiredModuleType;

	// Power consumption per production cycle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float PowerConsumption;

	// Worker requirement (number of workers)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Labor")
	int32 WorkerRequirement;

	// Recipe efficiency (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	float Efficiency;

	// Byproducts (if any)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	TMap<FName, int32> Byproducts;

	FProductionRecipe()
	{
		ProductionTime = 60.0f;
		RequiredTier = EProductionTier::Tier1_RawProcessing;
		RequiredModuleType = EBaseModuleType::Refinery;
		PowerConsumption = 10.0f;
		WorkerRequirement = 0;
		Efficiency = 1.0f;
	}
};

// Production module data
USTRUCT(BlueprintType)
struct FProductionModuleData
{
	GENERATED_BODY()

	// Module ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	FGuid ModuleID;

	// Module type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	EBaseModuleType ModuleType;

	// Current recipe being produced
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	FProductionRecipe CurrentRecipe;

	// Production progress (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	float ProductionProgress;

	// Is currently producing?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	bool bIsProducing;

	// Production rate multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	float ProductionRate;

	// Input storage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage")
	TMap<FName, int32> InputStorage;

	// Output storage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage")
	TMap<FName, int32> OutputStorage;

	// Maximum storage capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage")
	int32 MaxStorageCapacity;

	// Connected input modules
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connections")
	TArray<FGuid> InputModuleIDs;

	// Connected output modules
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connections")
	TArray<FGuid> OutputModuleIDs;

	// Is module operational?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsOperational;

	// Efficiency (affected by upgrades, power, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float Efficiency;

	FProductionModuleData()
	{
		ProductionProgress = 0.0f;
		bIsProducing = false;
		ProductionRate = 1.0f;
		MaxStorageCapacity = 1000;
		bIsOperational = false;
		Efficiency = 1.0f;
	}
};

// Production statistics
USTRUCT(BlueprintType)
struct FProductionStatistics
{
	GENERATED_BODY()

	// Number of active production modules
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 ActiveModules;

	// Number of modules currently producing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 ProducingModules;

	// Total production cycles completed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 TotalCyclesCompleted;

	// Current production rate (items per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	float CurrentProductionRate;

	// Total input resources consumed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	TMap<FName, int32> TotalInputsConsumed;

	// Total output resources produced
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	TMap<FName, int32> TotalOutputsProduced;

	// Efficiency rating (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float EfficiencyRating;

	// Bottleneck resources (what's limiting production)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bottlenecks")
	TMap<FName, int32> BottleneckResources;

	// Production value per hour
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	float ProductionValuePerHour;

	FProductionStatistics()
	{
		ActiveModules = 0;
		ProducingModules = 0;
		TotalCyclesCompleted = 0;
		CurrentProductionRate = 0.0f;
		EfficiencyRating = 0.0f;
		ProductionValuePerHour = 0.0f;
	}
};

// Resource routing path
USTRUCT(BlueprintType)
struct FResourceRoutingPath
{
	GENERATED_BODY()

	// Source module ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routing")
	FGuid SourceModuleID;

	// Destination module ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routing")
	FGuid DestinationModuleID;

	// Resource type being transported
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routing")
	FName ResourceType;

	// Transport rate (units per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routing")
	float TransportRate;

	// Is route active?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routing")
	bool bIsActive;

	FResourceRoutingPath()
	{
		TransportRate = 10.0f;
		bIsActive = false;
	}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProductionStarted, const FProductionModuleData&, ModuleData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProductionCompleted, const FProductionModuleData&, ModuleData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProductionChainCreated, EProductionChainType, ChainType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResourceRouted, const FResourceRoutingPath&, RoutingPath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProductionStatisticsUpdated, const FProductionStatistics&, Statistics);

/**
 * Production Chain Manager - Handles automated production chains and resource routing
 */
UCLASS()
class ALEXANDER_API AProductionChainManager : public AActor
{
	GENERATED_BODY()

public:
	AProductionChainManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Initialize production chain manager
	UFUNCTION(BlueprintCallable, Category = "Production")
	void Initialize(ABaseBuildingManager* BuildingManager, APlanetaryFarmingSystem* FarmingSystem);

	// Create production chain
	UFUNCTION(BlueprintCallable, Category = "Production")
	bool CreateProductionChain(EProductionChainType ChainType, const FGuid& StartModuleID);

	// Add production module
	UFUNCTION(BlueprintCallable, Category = "Production")
	FGuid AddProductionModule(EBaseModuleType ModuleType, FVector Location, const FGuid& ClaimID);

	// Set module recipe
	UFUNCTION(BlueprintCallable, Category = "Production")
	bool SetModuleRecipe(const FGuid& ModuleID, const FProductionRecipe& Recipe);

	// Start production
	UFUNCTION(BlueprintCallable, Category = "Production")
	bool StartProduction(const FGuid& ModuleID);

	// Stop production
	UFUNCTION(BlueprintCallable, Category = "Production")
	bool StopProduction(const FGuid& ModuleID);

	// Route resources between modules
	UFUNCTION(BlueprintCallable, Category = "Production")
	bool RouteResources(const FGuid& SourceModuleID, const FGuid& DestinationModuleID, FName ResourceType, float TransportRate);

	// Auto-route resources for a production chain
	UFUNCTION(BlueprintCallable, Category = "Production")
	bool AutoRouteResources(const FGuid& StartModuleID);

	// Get production module data
	UFUNCTION(BlueprintCallable, Category = "Production")
	FProductionModuleData GetProductionModule(const FGuid& ModuleID) const;

	// Get all production modules
	UFUNCTION(BlueprintCallable, Category = "Production")
	TArray<FProductionModuleData> GetAllProductionModules() const;

	// Get production statistics
	UFUNCTION(BlueprintCallable, Category = "Production")
	FProductionStatistics GetProductionStatistics() const;

	// Find optimal recipe for resource
	UFUNCTION(BlueprintCallable, Category = "Production")
	FProductionRecipe FindOptimalRecipe(FName DesiredOutput) const;

	// Check resource availability
	UFUNCTION(BlueprintCallable, Category = "Production")
	bool CheckResourceAvailability(const FProductionRecipe& Recipe) const;

	// Get bottleneck analysis
	UFUNCTION(BlueprintCallable, Category = "Production")
	TMap<FName, int32> GetBottleneckAnalysis() const;

	// Optimize production chain
	UFUNCTION(BlueprintCallable, Category = "Production")
	bool OptimizeProductionChain(const FGuid& StartModuleID);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Production Events")
	FOnProductionStarted OnProductionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Production Events")
	FOnProductionCompleted OnProductionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Production Events")
	FOnProductionChainCreated OnProductionChainCreated;

	UPROPERTY(BlueprintAssignable, Category = "Production Events")
	FOnResourceRouted OnResourceRouted;

	UPROPERTY(BlueprintAssignable, Category = "Production Events")
	FOnProductionStatisticsUpdated OnProductionStatisticsUpdated;

protected:
	// Base building manager
	UPROPERTY()
	TWeakObjectPtr<ABaseBuildingManager> BaseBuildingManager;

	// Farming system
	UPROPERTY()
	TWeakObjectPtr<APlanetaryFarmingSystem> FarmingSystem;

	// Production modules
	UPROPERTY()
	TArray<FProductionModuleData> ProductionModules;

	// Production module lookup by ID
	UPROPERTY()
	TMap<FGuid, int32> ProductionModuleIndexMap;

	// Resource routing paths
	UPROPERTY()
	TArray<FResourceRoutingPath> RoutingPaths;

	// Production recipes database
	UPROPERTY()
	TArray<FProductionRecipe> RecipeDatabase;

	// Production statistics
	UPROPERTY()
	FProductionStatistics ProductionStats;

	// Auto-routing enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bAutoRoutingEnabled;

	// Maximum number of production modules
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 MaxProductionModules;

	// Resource value mapping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	TMap<FName, float> ResourceValues;

private:
	// Initialize recipe database
	void InitializeRecipeDatabase();

	// Update production
	void UpdateProduction(float DeltaTime);

	// Update resource routing
	void UpdateResourceRouting(float DeltaTime);

	// Find production module index
	int32 FindProductionModuleIndex(const FGuid& ModuleID) const;

	// Can module produce recipe?
	bool CanModuleProduceRecipe(const FProductionModuleData& Module, const FProductionRecipe& Recipe) const;

	// Transfer resources between modules
	bool TransferResources(const FGuid& SourceID, const FGuid& DestID, FName ResourceType, int32 Amount);

	// Find storage for resource
	FGuid FindStorageForResource(FName ResourceType) const;

	// Calculate recipe value
	float CalculateRecipeValue(const FProductionRecipe& Recipe) const;

	// Get module efficiency
	float GetModuleEfficiency(const FProductionModuleData& Module) const;
};