// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerSystem.h"
#include "IrrigationSystem.generated.h"

// Forward declarations
class AFarmPlot;
class UNiagaraComponent;
class UNiagaraSystem;
class USphereComponent;
class UStaticMeshComponent;

/**
 * Irrigation system status
 */
UENUM(BlueprintType)
enum class EIrrigationStatus : uint8
{
	Inactive,
	Active,
	OutOfWater,
	Malfunction
};

/**
 * Automated irrigation system actor
 * Automatically waters crops within a 10m radius
 * Consumes 100L/hour of water
 * Requires 500W of power to operate
 */
UCLASS()
class ALEXANDER_API AIrrigationSystem : public AActor, public IPowerConsumer
{
	GENERATED_BODY()

public:
	AIrrigationSystem();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ============================================================================
	// COMPONENTS
	// ============================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* WateringRadius;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* WaterSprayEffect;

	// ============================================================================
	// IRRIGATION PROPERTIES
	// ============================================================================

	/** Current status of the irrigation system */
	UPROPERTY(BlueprintReadOnly, Category = "Irrigation")
	EIrrigationStatus Status;

	/** Whether the system is currently active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Irrigation")
	bool bIsActive;

	/** Radius of automatic watering (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Irrigation")
	float WateringRadiusCm;

	/** Water consumption rate (liters per hour) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Irrigation")
	float WaterConsumptionRate;

	/** Current water level in tank (liters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Irrigation")
	float CurrentWaterLevel;

	/** Maximum water tank capacity (liters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Irrigation")
	float MaxWaterCapacity;

	/** Water amount to apply per watering cycle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Irrigation")
	float WaterAmountPerCycle;

	/** Time between watering cycles (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Irrigation")
	float WateringInterval;

	/** Time since last watering cycle */
	UPROPERTY(BlueprintReadOnly, Category = "Irrigation")
	float TimeSinceLastWatering;

	// ============================================================================
	// VISUAL EFFECTS
	// ============================================================================

	/** Niagara system for water spray effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* WaterSpraySystem;

	/** Number of spray nozzles to simulate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	int32 NumSprayNozzles;

	/** Spray particle spawn rate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	float SpraySpawnRate;

	/** Spray particle size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	float SprayParticleSize;

	// ============================================================================
	// POWER SYSTEM
	// ============================================================================

	/** Power requirement in watts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float PowerRequirement;

	/** Whether system is currently receiving power */
	UPROPERTY(BlueprintReadOnly, Category = "Power")
	bool bIsPowered;

	// ============================================================================
	// IRRIGATION OPERATIONS
	// ============================================================================

	/**
	 * Activate the irrigation system
	 */
	UFUNCTION(BlueprintCallable, Category = "Irrigation")
	void Activate();

	/**
	 * Deactivate the irrigation system
	 */
	UFUNCTION(BlueprintCallable, Category = "Irrigation")
	void Deactivate();

	/**
	 * Toggle irrigation system on/off
	 */
	UFUNCTION(BlueprintCallable, Category = "Irrigation")
	void Toggle();

	/**
	 * Refill water tank
	 * @param Amount Amount of water to add (liters)
	 */
	UFUNCTION(BlueprintCallable, Category = "Irrigation")
	void RefillWater(float Amount);

	/**
	 * Get current water level percentage
	 */
	UFUNCTION(BlueprintCallable, Category = "Irrigation")
	float GetWaterLevelPercent() const;

	/**
	 * Get estimated time until water runs out (seconds)
	 */
	UFUNCTION(BlueprintCallable, Category = "Irrigation")
	float GetTimeUntilEmpty() const;

	/**
	 * Check if system has enough water to operate
	 */
	UFUNCTION(BlueprintCallable, Category = "Irrigation")
	bool HasSufficientWater() const;

	/**
	 * Get all farm plots within watering radius
	 */
	UFUNCTION(BlueprintCallable, Category = "Irrigation")
	TArray<AFarmPlot*> GetFarmPlotsInRange() const;

	/**
	 * Water all crops within radius
	 */
	UFUNCTION(BlueprintCallable, Category = "Irrigation")
	void WaterCropsInRadius();

	// ============================================================================
	// IPowerConsumer Interface
	// ============================================================================

	virtual float GetPowerRequirement_Implementation() const override;
	virtual void SetPowered_Implementation(bool bInIsPowered) override;
	virtual bool IsPowered_Implementation() const override;

protected:
	/**
	 * Update water consumption
	 */
	void UpdateWaterConsumption(float DeltaTime);

	/**
	 * Update visual effects based on status
	 */
	void UpdateVisualEffects();

	/**
	 * Create water spray effect
	 */
	void CreateWaterSprayEffect();

	/**
	 * Update spray effect parameters
	 */
	void UpdateSprayEffect();

	/**
	 * Check system status and update accordingly
	 */
	void UpdateSystemStatus();
};
