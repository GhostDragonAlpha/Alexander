// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PerformanceProfiler.h"
#include "ShipSystemsManager.generated.h"

/**
 * Ship system status
 */
UENUM(BlueprintType)
enum class EShipSystemStatus : uint8
{
	Offline UMETA(DisplayName = "Offline"),
	Online UMETA(DisplayName = "Online"),
	Damaged UMETA(DisplayName = "Damaged"),
	Critical UMETA(DisplayName = "Critical"),
	Failing UMETA(DisplayName = "Failing")
};

/**
 * Ship system type
 */
UENUM(BlueprintType)
enum class EShipSystemType : uint8
{
	Engines UMETA(DisplayName = "Engines"),
	LifeSupport UMETA(DisplayName = "Life Support"),
	Shields UMETA(DisplayName = "Shields"),
	Weapons UMETA(DisplayName = "Weapons"),
	Sensors UMETA(DisplayName = "Sensors"),
	Communications UMETA(DisplayName = "Communications"),
	Navigation UMETA(DisplayName = "Navigation"),
	Power UMETA(DisplayName = "Power"),
	Cooling UMETA(DisplayName = "Cooling")
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSystemStatusChanged, EShipSystemType, SystemType, EShipSystemStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSystemDamaged, EShipSystemType, SystemType, float, DamageAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemRepaired, EShipSystemType, SystemType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCriticalFailure, EShipSystemType, SystemType);

/**
 * Ship system data
 */
USTRUCT(BlueprintType)
struct FShipSystemData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "System")
	EShipSystemType SystemType = EShipSystemType::Engines;

	UPROPERTY(BlueprintReadWrite, Category = "System")
	EShipSystemStatus Status = EShipSystemStatus::Offline;

	UPROPERTY(BlueprintReadWrite, Category = "System")
	float Health = 100.0f;

	UPROPERTY(BlueprintReadWrite, Category = "System")
	float PowerUsage = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "System")
	float Efficiency = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "System")
	float Temperature = 20.0f;

	UPROPERTY(BlueprintReadWrite, Category = "System")
	bool bIsCritical = false;

	UPROPERTY(BlueprintReadWrite, Category = "System")
	FString SystemName = TEXT("Unknown System");

	UPROPERTY(BlueprintReadWrite, Category = "System")
	FString StatusMessage = TEXT("System Offline");
};

/**
 * Ship Systems Manager - manages all ship subsystems
 * Handles power distribution, damage, repairs, and system interactions
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UShipSystemsManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UShipSystemsManager();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ============================================================================
	// SYSTEM MANAGEMENT
	// ============================================================================

	// All ship systems
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
	TArray<FShipSystemData> ShipSystems;

	// Total power available
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Power")
	float TotalPowerAvailable = 1000.0f;

	// Current power usage
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Power")
	float CurrentPowerUsage = 0.0f;

	// Power reserve capacity
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Power")
	float PowerReserve = 500.0f;

	// ============================================================================
	// SYSTEM CONTROL
	// ============================================================================

	// Initialize all ship systems
	UFUNCTION(BlueprintCallable, Category = "Systems")
	void InitializeSystems();

	// Get system by type
	FShipSystemData* GetSystem(EShipSystemType SystemType);

	// Set system status
	UFUNCTION(BlueprintCallable, Category = "Systems")
	void SetSystemStatus(EShipSystemType SystemType, EShipSystemStatus NewStatus);

	// Damage system
	UFUNCTION(BlueprintCallable, Category = "Systems")
	void DamageSystem(EShipSystemType SystemType, float DamageAmount);

	// Repair system
	UFUNCTION(BlueprintCallable, Category = "Systems")
	void RepairSystem(EShipSystemType SystemType, float RepairAmount);

	// ============================================================================
	// POWER MANAGEMENT
	// ============================================================================

	// Calculate total power usage
	UFUNCTION(BlueprintCallable, Category = "Power")
	float CalculateTotalPowerUsage();

	// Redistribute power to critical systems
	UFUNCTION(BlueprintCallable, Category = "Power")
	void RedistributePower();

	// Set power priority for system
	UFUNCTION(BlueprintCallable, Category = "Power")
	void SetPowerPriority(EShipSystemType SystemType, int32 Priority);

	// Get power efficiency
	UFUNCTION(BlueprintCallable, Category = "Power")
	float GetPowerEfficiency() const;

	// ============================================================================
	// SYSTEM INTERACTIONS
	// ============================================================================

	// Update system interactions (cascade effects)
	UFUNCTION(BlueprintCallable, Category = "Systems")
	void UpdateSystemInteractions();

	// Handle system failure
	UFUNCTION(BlueprintCallable, Category = "Systems")
	void HandleSystemFailure(EShipSystemType FailedSystem);

	// Check for critical failures
	UFUNCTION(BlueprintCallable, Category = "Systems")
	bool CheckForCriticalFailures();

	// ============================================================================
	// DIAGNOSTICS
	// ============================================================================

	// Run full system diagnostic
	UFUNCTION(BlueprintCallable, Category = "Diagnostics")
	bool RunSystemDiagnostic();

	// Get system health report
	UFUNCTION(BlueprintCallable, Category = "Diagnostics")
	FString GetSystemHealthReport();

	// Get critical system alerts
	UFUNCTION(BlueprintCallable, Category = "Diagnostics")
	TArray<FString> GetCriticalAlerts();

	// Get system status summary
	UFUNCTION(BlueprintCallable, Category = "Diagnostics")
	FString GetStatusSummary();

	// Calculate average temperature across all systems
	float CalculateAverageTemperature() const;

	// ============================================================================
	// EVENTS
	// ============================================================================

	// Called when system status changes
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSystemStatusChanged OnSystemStatusChanged;

	// Called when system takes damage
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSystemDamaged OnSystemDamaged;

	// Called when system is repaired
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSystemRepaired OnSystemRepaired;

	// Called when critical failure occurs
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCriticalFailure OnCriticalFailure;

protected:
	// System update functions
	void UpdateEngineSystems(float DeltaTime);
	void UpdateLifeSupportSystems(float DeltaTime);
	void UpdateShieldSystems(float DeltaTime);
	void UpdateWeaponSystems(float DeltaTime);
	void UpdateSensorSystems(float DeltaTime);
	void UpdateCommunicationSystems(float DeltaTime);
	void UpdateNavigationSystems(float DeltaTime);
	void UpdatePowerSystems(float DeltaTime);
	void UpdateCoolingSystems(float DeltaTime);

	// Power distribution
	void DistributePower();
	void HandlePowerShortage();
	void HandlePowerSurge();

	// Temperature management
	void UpdateSystemTemperatures(float DeltaTime);
	void HandleOverheating(EShipSystemType SystemType);

	// System cascading effects
	void ApplyCascadingEffects(EShipSystemType PrimarySystem, EShipSystemStatus NewStatus);

private:
	// Internal state
	TMap<EShipSystemType, int32> SystemPriorities;
	TMap<EShipSystemType, float> SystemTemperatures;
	bool bDiagnosticRunning = false;
	float LastDiagnosticTime = 0.0f;

	// Performance tracking
	int32 SystemUpdateCount = 0;
	float AverageUpdateTime = 0.0f;

	// Performance Profiler
	UPROPERTY()
	UPerformanceProfiler* PerformanceProfiler;

	// Helper functions
	void InitializeDefaultSystems();
	FShipSystemData CreateSystem(EShipSystemType Type, const FString& Name, bool bCritical = false);
	void UpdateSystemStatusMessage(FShipSystemData& System);
	bool IsSystemCritical(EShipSystemType SystemType) const;
};