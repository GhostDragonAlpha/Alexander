// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "EngineComponent.generated.h"

/**
 * Engine type enumeration
 */
UENUM(BlueprintType)
enum class EEngineType : uint8
{
	Chemical UMETA(DisplayName = "Chemical"),
	Ion UMETA(DisplayName = "Ion"),
	Fusion UMETA(DisplayName = "Fusion"),
	Quantum UMETA(DisplayName = "Quantum"),
	Warp UMETA(DisplayName = "Warp")
};

/**
 * Engine state
 */
UENUM(BlueprintType)
enum class EEngineState : uint8
{
	Offline UMETA(DisplayName = "Offline"),
	Idle UMETA(DisplayName = "Idle"),
	Starting UMETA(DisplayName = "Starting"),
	Running UMETA(DisplayName = "Running"),
	Overload UMETA(DisplayName = "Overload"),
	Damaged UMETA(DisplayName = "Damaged"),
	Emergency UMETA(DisplayName = "Emergency")
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEngineStateChanged, EEngineState, OldState, EEngineState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEngineDamaged, float, DamageAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEngineOverheat, float, Temperature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFuelEmpty);

/**
 * Engine Component - handles thrust, visual effects, and audio
 * Provides realistic space engine behavior with multiple engine types
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UEngineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEngineComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ============================================================================
	// ENGINE CONFIGURATION
	// ============================================================================

	// Type of engine
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	EEngineType EngineType = EEngineType::Chemical;

	// Maximum thrust output (Newtons)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float MaxThrust = 100000.0f;

	// Fuel consumption rate (units per second at max thrust)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float FuelConsumptionRate = 10.0f;

	// Heat generation rate (degrees per second at max thrust)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float HeatGenerationRate = 50.0f;

	// Engine efficiency (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float EngineEfficiency = 0.8f;

	// ============================================================================
	// ENGINE STATE
	// ============================================================================

	// Current engine state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EEngineState CurrentState = EEngineState::Offline;

	// Current thrust level (0-1)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float CurrentThrustLevel = 0.0f;

	// Current temperature (degrees Celsius)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float CurrentTemperature = 20.0f;

	// Maximum safe temperature
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float MaxTemperature = 500.0f;

	// Current fuel level (0-1)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float FuelLevel = 1.0f;

	// Engine health (0-1)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float EngineHealth = 1.0f;

	// ============================================================================
	// VISUAL EFFECTS
	// ============================================================================

	// Thruster particle system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	class UParticleSystem* ThrusterEffect = nullptr;

	// Engine glow material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	class UMaterialInterface* EngineGlowMaterial = nullptr;

	// Damage particle system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	class UParticleSystem* DamageEffect = nullptr;

	// Engine mesh components for visual feedback
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TArray<class UStaticMeshComponent*> EngineMeshes;

	// ============================================================================
	// AUDIO
	// ============================================================================

	// Engine idle sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	class USoundBase* IdleSound = nullptr;

	// Engine thrust sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	class USoundBase* ThrustSound = nullptr;

	// Engine damage sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	class USoundBase* DamageSound = nullptr;

	// Audio component for engine sounds
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAudioComponent* EngineAudioComponent;

	// ============================================================================
	// ENGINE CONTROL
	// ============================================================================

	// Start the engine
	UFUNCTION(BlueprintCallable, Category = "Engine")
	bool StartEngine();

	// Stop the engine
	UFUNCTION(BlueprintCallable, Category = "Engine")
	void StopEngine();

	// Set thrust level (0-1)
	UFUNCTION(BlueprintCallable, Category = "Engine")
	void SetThrustLevel(float ThrustLevel);

	// Apply emergency thrust override
	UFUNCTION(BlueprintCallable, Category = "Engine")
	void EmergencyThrust(float OverrideLevel);

	// Repair engine systems
	UFUNCTION(BlueprintCallable, Category = "Engine")
	void RepairEngine(float RepairAmount);

	// Refuel engine
	UFUNCTION(BlueprintCallable, Category = "Engine")
	void Refuel(float FuelAmount);

	// ============================================================================
	// ENGINE OPERATIONS
	// ============================================================================

	// Get actual thrust output considering damage and efficiency
	UFUNCTION(BlueprintCallable, Category = "Engine")
	float GetActualThrust() const;

	// Get fuel consumption at current thrust level
	UFUNCTION(BlueprintCallable, Category = "Engine")
	float GetCurrentFuelConsumption() const;

	// Get heat generation at current thrust level
	UFUNCTION(BlueprintCallable, Category = "Engine")
	float GetCurrentHeatGeneration() const;

	// Is engine operational?
	UFUNCTION(BlueprintCallable, Category = "Engine")
	bool IsOperational() const;

	// Can engine start?
	UFUNCTION(BlueprintCallable, Category = "Engine")
	bool CanStart() const;

	// Get engine status as text
	UFUNCTION(BlueprintCallable, Category = "Engine")
	FString GetEngineStatus() const;

	// ============================================================================
	// VISUAL & AUDIO UPDATES
	// ============================================================================

	// Update engine visual effects
	UFUNCTION(BlueprintCallable, Category = "Effects")
	void UpdateEngineEffects();

	// Update engine audio
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void UpdateEngineAudio();

	// Update damage effects
	UFUNCTION(BlueprintCallable, Category = "Effects")
	void UpdateDamageEffects();

	// ============================================================================
	// EVENTS
	// ============================================================================

	// Called when engine state changes
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEngineStateChanged OnEngineStateChanged;

	// Called when engine takes damage
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEngineDamaged OnEngineDamaged;

	// Called when engine overheats
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEngineOverheat OnEngineOverheat;

	// Called when fuel runs out
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFuelEmpty OnFuelEmpty;

protected:
	// Internal state management
	void UpdateEngineState(float DeltaTime);
	void UpdateTemperature(float DeltaTime);
	void UpdateFuel(float DeltaTime);
	void CheckEngineWarnings();

	// Visual effect helpers
	void CreateThrusterEffects();
	void UpdateThrusterIntensity(float Intensity);
	void UpdateEngineGlow(float Intensity);

	// Audio helpers
	void InitializeEngineAudio();
	void UpdateEngineSoundParameters();

private:
	// Internal particle system components
	UPROPERTY()
	TArray<class UParticleSystemComponent*> ThrusterComponents;

	// Internal material instances for engine glow
	UPROPERTY()
	TArray<class UMaterialInstanceDynamic*> EngineGlowMaterials;

	// Performance tracking
	float LastThrustUpdate = 0.0f;
	float LastTemperatureUpdate = 0.0f;
	float LastFuelUpdate = 0.0f;

	// Warning thresholds
	bool bOverheatWarning = false;
	bool bLowFuelWarning = false;
	bool bDamageWarning = false;

	// Engine startup sequence
	float StartupProgress = 0.0f;
	bool bIsStarting = false;
};