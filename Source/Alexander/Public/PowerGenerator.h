// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerSystem.h"
#include "PowerGenerator.generated.h"

// Forward declarations
class UStaticMeshComponent;
class UAudioComponent;
class UNiagaraComponent;
class UNiagaraSystem;

/**
 * Generator fuel type
 */
UENUM(BlueprintType)
enum class EGeneratorFuelType : uint8
{
	Diesel,
	Biofuel,
	Nuclear,
	Fusion
};

/**
 * Generator status
 */
UENUM(BlueprintType)
enum class EGeneratorStatus : uint8
{
	Off,
	Starting,
	Running,
	OutOfFuel,
	Malfunction
};

/**
 * Power generator actor - generates power from fuel
 * Provides consistent power output regardless of time of day
 */
UCLASS()
class ALEXANDER_API APowerGenerator : public AActor, public IPowerProvider
{
	GENERATED_BODY()

public:
	APowerGenerator();

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
	UStaticMeshComponent* GeneratorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* EngineSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* ExhaustEffect;

	// ============================================================================
	// GENERATOR PROPERTIES
	// ============================================================================

	/** Generator status */
	UPROPERTY(BlueprintReadOnly, Category = "Generator")
	EGeneratorStatus Status;

	/** Fuel type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	EGeneratorFuelType FuelType;

	/** Maximum power output in watts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	float MaxPowerOutput;

	/** Current power output in watts */
	UPROPERTY(BlueprintReadOnly, Category = "Generator")
	float CurrentPowerOutput;

	/** Fuel consumption rate (liters per hour at max output) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	float FuelConsumptionRate;

	/** Current fuel level (liters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	float CurrentFuelLevel;

	/** Maximum fuel tank capacity (liters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	float MaxFuelCapacity;

	/** Generator efficiency (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	float Efficiency;

	/** Whether generator is currently running */
	UPROPERTY(BlueprintReadOnly, Category = "Generator")
	bool bIsRunning;

	/** Time to start up (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	float StartupTime;

	/** Current startup progress (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Generator")
	float StartupProgress;

	// ============================================================================
	// VISUAL EFFECTS
	// ============================================================================

	/** Exhaust particle system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* ExhaustSystem;

	/** Engine running sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundBase* EngineRunningSound;

	/** Engine startup sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundBase* EngineStartupSound;

	/** Engine shutdown sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundBase* EngineShutdownSound;

	// ============================================================================
	// GENERATOR OPERATIONS
	// ============================================================================

	/**
	 * Start the generator
	 */
	UFUNCTION(BlueprintCallable, Category = "Generator")
	void StartGenerator();

	/**
	 * Stop the generator
	 */
	UFUNCTION(BlueprintCallable, Category = "Generator")
	void StopGenerator();

	/**
	 * Toggle generator on/off
	 */
	UFUNCTION(BlueprintCallable, Category = "Generator")
	void ToggleGenerator();

	/**
	 * Refuel the generator
	 * @param Amount Amount of fuel to add (liters)
	 */
	UFUNCTION(BlueprintCallable, Category = "Generator")
	void Refuel(float Amount);

	/**
	 * Get fuel level percentage
	 */
	UFUNCTION(BlueprintCallable, Category = "Generator")
	float GetFuelLevelPercent() const;

	/**
	 * Get estimated runtime remaining (hours)
	 */
	UFUNCTION(BlueprintCallable, Category = "Generator")
	float GetEstimatedRuntime() const;

	/**
	 * Check if generator has sufficient fuel
	 */
	UFUNCTION(BlueprintCallable, Category = "Generator")
	bool HasSufficientFuel() const;

	/**
	 * Get power output percentage (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Generator")
	float GetPowerOutputPercent() const;

	// ============================================================================
	// IPowerProvider Interface
	// ============================================================================

	virtual float GetAvailablePower_Implementation() const override;
	virtual float GetMaxPowerCapacity_Implementation() const override;
	virtual EPowerSourceType GetPowerSourceType_Implementation() const override;

protected:
	/**
	 * Update power output based on fuel and status
	 */
	void UpdatePowerOutput(float DeltaTime);

	/**
	 * Consume fuel based on power output
	 */
	void ConsumeFuel(float DeltaTime);

	/**
	 * Update startup progress
	 */
	void UpdateStartup(float DeltaTime);

	/**
	 * Update visual and audio effects
	 */
	void UpdateEffects();

	/**
	 * Update generator status
	 */
	void UpdateStatus();
};
