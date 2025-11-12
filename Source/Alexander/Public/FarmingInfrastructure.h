// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "CropGrowthSystem.h"
#include "FarmingInfrastructure.generated.h"

// Forward declarations
class AFarmPlot;
class UCropGrowthSystem;

// Hydroponics bay - indoor controlled environment farming
UCLASS()
class ALEXANDER_API AHydroponicsBay : public AActor
{
	GENERATED_BODY()

public:
	AHydroponicsBay();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Initialize hydroponics bay
	UFUNCTION(BlueprintCallable, Category = "Hydroponics")
	void InitializeBay(int32 MaxPlants, float WaterCapacity, float NutrientCapacity);

	// Add plant to bay
	UFUNCTION(BlueprintCallable, Category = "Hydroponics")
	bool AddPlant(FActiveCrop& Crop);

	// Remove plant from bay
	UFUNCTION(BlueprintCallable, Category = "Hydroponics")
	bool RemovePlant(int32 PlantIndex);

	// Water all plants
	UFUNCTION(BlueprintCallable, Category = "Hydroponics")
	void WaterAllPlants(float WaterAmount);

	// Add nutrients
	UFUNCTION(BlueprintCallable, Category = "Hydroponics")
	void AddNutrients(float NutrientAmount, EFertilizerType FertilizerType);

	// Set environmental conditions
	UFUNCTION(BlueprintCallable, Category = "Hydroponics")
	void SetEnvironment(float Temperature, float Humidity, float LightLevel);

	// Get bay status
	UFUNCTION(BlueprintCallable, Category = "Hydroponics")
	bool IsBayOperational() const { return bIsOperational; }

	// Get number of plants
	UFUNCTION(BlueprintCallable, Category = "Hydroponics")
	int32 GetPlantCount() const { return Plants.Num(); }

	// Get water level
	UFUNCTION(BlueprintCallable, Category = "Hydroponics")
	float GetWaterLevel() const { return CurrentWater / MaxWaterCapacity; }

	// Get nutrient level
	UFUNCTION(BlueprintCallable, Category = "Hydroponics")
	float GetNutrientLevel() const { return CurrentNutrients / MaxNutrientCapacity; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BayMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* InteractionVolume;

	// Plants in this bay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plants")
	TArray<FActiveCrop> Plants;

	// Maximum number of plants
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 MaxPlants;

	// Water capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float MaxWaterCapacity;

	// Current water level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float CurrentWater;

	// Nutrient capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float MaxNutrientCapacity;

	// Current nutrient level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float CurrentNutrients;

	// Current fertilizer type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	EFertilizerType CurrentFertilizerType;

	// Environmental controls
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float CurrentTemperature;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float CurrentHumidity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float CurrentLightLevel;

	// Operational status
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsOperational;

	// Power consumption
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float PowerConsumption;

private:
	// Update plant growth
	void UpdatePlantGrowth(float DeltaTime);

	// Consume water and nutrients
	void ConsumeResources(float DeltaTime);
};

// Greenhouse - protected outdoor farming
UCLASS()
class ALEXANDER_API AGreenhouse : public AActor
{
	GENERATED_BODY()

public:
	AGreenhouse();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Initialize greenhouse
	UFUNCTION(BlueprintCallable, Category = "Greenhouse")
	void InitializeGreenhouse(FVector2D Dimensions, int32 MaxRows, int32 MaxColumns);

	// Add farm plot to greenhouse
	UFUNCTION(BlueprintCallable, Category = "Greenhouse")
	bool AddFarmPlot(AFarmPlot* Plot);

	// Remove farm plot from greenhouse
	UFUNCTION(BlueprintCallable, Category = "Greenhouse")
	bool RemoveFarmPlot(const FGuid& PlotID);

	// Control temperature
	UFUNCTION(BlueprintCallable, Category = "Greenhouse")
	void SetTemperature(float Temperature);

	// Control humidity
	UFUNCTION(BlueprintCallable, Category = "Greenhouse")
	void SetHumidity(float Humidity);

	// Control ventilation
	UFUNCTION(BlueprintCallable, Category = "Greenhouse")
	void SetVentilation(bool bEnable);

	// Get greenhouse status
	UFUNCTION(BlueprintCallable, Category = "Greenhouse")
	bool IsGreenhouseOperational() const { return bIsOperational; }

	// Get number of plots
	UFUNCTION(BlueprintCallable, Category = "Greenhouse")
	int32 GetPlotCount() const { return FarmPlots.Num(); }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* GreenhouseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* GreenhouseVolume;

	// Farm plots in this greenhouse
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plots")
	TArray<AFarmPlot*> FarmPlots;

	// Greenhouse dimensions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FVector2D Dimensions;

	// Maximum rows and columns
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 MaxRows;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 MaxColumns;

	// Environmental controls
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float CurrentTemperature;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float CurrentHumidity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	bool bVentilationEnabled;

	// Structural integrity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float StructuralIntegrity;

	// Operational status
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsOperational;

	// Power consumption
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float PowerConsumption;

private:
	// Update greenhouse environment
	void UpdateEnvironment(float DeltaTime);

	// Update structural integrity
	void UpdateStructuralIntegrity(float DeltaTime);
};

// Forward declaration for irrigation system
class AAlexanderIrrigationSystem;

// Fertilizer injector - automated nutrient delivery
UCLASS()
class ALEXANDER_API AFertilizerInjector : public AActor
{
	GENERATED_BODY()

public:
	AFertilizerInjector();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Initialize injector
	UFUNCTION(BlueprintCallable, Category = "Fertilizer")
	void InitializeInjector(float TankCapacity, float InjectionRate);

	// Add farm plot to injection coverage
	UFUNCTION(BlueprintCallable, Category = "Fertilizer")
	bool AddPlotToCoverage(AFarmPlot* Plot);

	// Remove farm plot from coverage
	UFUNCTION(BlueprintCallable, Category = "Fertilizer")
	bool RemovePlotFromCoverage(const FGuid& PlotID);

	// Inject fertilizer to all covered plots
	UFUNCTION(BlueprintCallable, Category = "Fertilizer")
	void InjectFertilizerToAll(EFertilizerType FertilizerType, float Amount);

	// Set injection schedule
	UFUNCTION(BlueprintCallable, Category = "Fertilizer")
	void SetInjectionSchedule(float Interval, float Amount);

	// Refill fertilizer tank
	UFUNCTION(BlueprintCallable, Category = "Fertilizer")
	void RefillTank(EFertilizerType FertilizerType, float Amount);

	// Get system status
	UFUNCTION(BlueprintCallable, Category = "Fertilizer")
	bool IsInjectorOperational() const { return bIsOperational; }

	// Get fertilizer level
	UFUNCTION(BlueprintCallable, Category = "Fertilizer")
	float GetFertilizerLevel(EFertilizerType Type) const;

	// Get number of covered plots
	UFUNCTION(BlueprintCallable, Category = "Fertilizer")
	int32 GetCoveredPlotCount() const { return CoveredPlots.Num(); }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* InjectorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CoverageVolume;

	// Covered farm plots
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plots")
	TArray<AFarmPlot*> CoveredPlots;

	// Fertilizer tanks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tanks")
	TMap<EFertilizerType, float> FertilizerTanks;

	// Tank capacity per fertilizer type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float TankCapacity;

	// Injection rate (kg per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float InjectionRate;

	// Injection schedule
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	float InjectionInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	float InjectionAmount;

	// Last injection time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float LastInjectionTime;

	// Operational status
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsOperational;

	// Power consumption
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float PowerConsumption;

private:
	// Check injection schedule
	void CheckInjectionSchedule(float DeltaTime);

	// Consume fertilizer
	void ConsumeFertilizer(EFertilizerType Type, float Amount);
};

// Harvesting drone - automated crop collection
UCLASS()
class ALEXANDER_API AHarvestingDrone : public AActor
{
	GENERATED_BODY()

public:
	AHarvestingDrone();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Initialize drone
	UFUNCTION(BlueprintCallable, Category = "Drone")
	void InitializeDrone(float BatteryCapacity, float HarvestSpeed, float MovementSpeed);

	// Assign farm plot to drone
	UFUNCTION(BlueprintCallable, Category = "Drone")
	bool AssignPlot(AFarmPlot* Plot);

	// Start harvesting
	UFUNCTION(BlueprintCallable, Category = "Drone")
	void StartHarvesting();

	// Stop harvesting
	UFUNCTION(BlueprintCallable, Category = "Drone")
	void StopHarvesting();

	// Return to charging station
	UFUNCTION(BlueprintCallable, Category = "Drone")
	void ReturnToChargingStation();

	// Charge battery
	UFUNCTION(BlueprintCallable, Category = "Drone")
	void ChargeBattery(float Amount);

	// Get drone status
	UFUNCTION(BlueprintCallable, Category = "Drone")
	bool IsDroneOperational() const { return bIsOperational; }

	// Get battery level
	UFUNCTION(BlueprintCallable, Category = "Drone")
	float GetBatteryLevel() const { return CurrentBattery / MaxBatteryCapacity; }

	// Get current task
	UFUNCTION(BlueprintCallable, Category = "Drone")
	FString GetCurrentTask() const;

	// Get harvested amount
	UFUNCTION(BlueprintCallable, Category = "Drone")
	int32 GetHarvestedAmount() const { return TotalHarvested; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DroneMesh;

	// Current target plot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	AFarmPlot* CurrentPlot;

	// Battery capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battery")
	float MaxBatteryCapacity;

	// Current battery level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battery")
	float CurrentBattery;

	// Harvesting speed (plants per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float HarvestSpeed;

	// Movement speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float MovementSpeed;

	// Current task
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FString CurrentTask;

	// Is harvesting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsHarvesting;

	// Total harvested
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 TotalHarvested;

	// Operational status
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsOperational;

	// Power consumption while active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float ActivePowerConsumption;

	// Power consumption while idle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float IdlePowerConsumption;

private:
	// Harvest crops
	void PerformHarvesting(float DeltaTime);

	// Consume battery
	void ConsumeBattery(float DeltaTime);

	// Move to target
	void MoveToTarget(float DeltaTime);
};