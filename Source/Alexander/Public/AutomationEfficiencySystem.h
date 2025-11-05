// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AutomationEfficiencySystem.generated.h"

// Forward declarations
class AIrrigationSystem;
class AHarvestingMachine;

/**
 * Automation device type
 */
UENUM(BlueprintType)
enum class EAutomationDeviceType : uint8
{
	IrrigationSystem,
	HarvestingMachine,
	PowerGenerator,
	SolarPanel
};

/**
 * Automation device status
 */
UENUM(BlueprintType)
enum class EAutomationDeviceStatus : uint8
{
	Operational,      // Working normally
	Degraded,         // Working but needs maintenance
	Malfunction,      // Not working, needs repair
	Offline           // Powered off or no power
};

/**
 * Upgrade type for automation devices
 */
UENUM(BlueprintType)
enum class EAutomationUpgradeType : uint8
{
	Efficiency,       // Reduces resource consumption
	Speed,            // Increases operation speed
	Capacity,         // Increases storage/tank capacity
	Range,            // Increases operational range
	Durability        // Reduces maintenance frequency
};

/**
 * Automation device statistics
 */
USTRUCT(BlueprintType)
struct FAutomationDeviceStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	EAutomationDeviceType DeviceType;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	EAutomationDeviceStatus Status;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float EfficiencyPercent; // 0-100

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float OperationalHours;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float MaintenanceProgress; // 0-1, 1 = needs maintenance

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 UpgradeLevel;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float ManualLaborReduction; // 0-1, percentage of manual labor saved

	FAutomationDeviceStats()
		: DeviceType(EAutomationDeviceType::IrrigationSystem)
		, Status(EAutomationDeviceStatus::Operational)
		, EfficiencyPercent(100.0f)
		, OperationalHours(0.0f)
		, MaintenanceProgress(0.0f)
		, UpgradeLevel(0)
		, ManualLaborReduction(0.8f)
	{
	}
};

/**
 * Upgrade definition
 */
USTRUCT(BlueprintType)
struct FAutomationUpgrade
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	EAutomationUpgradeType UpgradeType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	int32 Level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	FString UpgradeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	float EffectMultiplier; // Multiplier applied to base value

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	float Cost;

	FAutomationUpgrade()
		: UpgradeType(EAutomationUpgradeType::Efficiency)
		, Level(1)
		, EffectMultiplier(1.1f)
		, Cost(1000.0f)
	{
	}
};

/**
 * Maintenance requirement
 */
USTRUCT(BlueprintType)
struct FMaintenanceRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maintenance")
	FString RequirementName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maintenance")
	FString ItemRequired;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maintenance")
	int32 QuantityRequired;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maintenance")
	float TimeRequired; // seconds

	FMaintenanceRequirement()
		: QuantityRequired(1)
		, TimeRequired(30.0f)
	{
	}
};

/**
 * Automation efficiency system
 * Tracks automation device performance, maintenance, and upgrades
 * Implements 80% manual labor reduction through automation
 */
UCLASS()
class ALEXANDER_API UAutomationEfficiencySystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// ============================================================================
	// SUBSYSTEM LIFECYCLE
	// ============================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// ============================================================================
	// DEVICE TRACKING
	// ============================================================================

	/** Registered automation devices */
	UPROPERTY(BlueprintReadOnly, Category = "Automation")
	TMap<AActor*, FAutomationDeviceStats> RegisteredDevices;

	/**
	 * Register an automation device
	 */
	UFUNCTION(BlueprintCallable, Category = "Automation")
	void RegisterDevice(AActor* Device, EAutomationDeviceType DeviceType);

	/**
	 * Unregister an automation device
	 */
	UFUNCTION(BlueprintCallable, Category = "Automation")
	void UnregisterDevice(AActor* Device);

	/**
	 * Get device statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Automation")
	FAutomationDeviceStats GetDeviceStats(AActor* Device) const;

	/**
	 * Update device statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Automation")
	void UpdateDeviceStats(AActor* Device, const FAutomationDeviceStats& Stats);

	// ============================================================================
	// EFFICIENCY TRACKING
	// ============================================================================

	/**
	 * Calculate overall automation efficiency (0-100)
	 */
	UFUNCTION(BlueprintCallable, Category = "Automation")
	float GetOverallEfficiency() const;

	/**
	 * Calculate manual labor reduction percentage (0-100)
	 */
	UFUNCTION(BlueprintCallable, Category = "Automation")
	float GetManualLaborReduction() const;

	/**
	 * Get total operational hours across all devices
	 */
	UFUNCTION(BlueprintCallable, Category = "Automation")
	float GetTotalOperationalHours() const;

	/**
	 * Get number of devices by status
	 */
	UFUNCTION(BlueprintCallable, Category = "Automation")
	void GetDeviceStatusCounts(int32& Operational, int32& Degraded, int32& Malfunction, int32& Offline) const;

	// ============================================================================
	// MAINTENANCE SYSTEM
	// ============================================================================

	/** Maintenance requirements by device type */
// Maintenance requirements (C++ only due to TArray in TMap limitation)
	TMap<EAutomationDeviceType, TArray<FMaintenanceRequirement>> MaintenanceRequirements;

	/** Hours between maintenance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maintenance")
	float MaintenanceIntervalHours;

	/**
	 * Check if device needs maintenance
	 */
	UFUNCTION(BlueprintCallable, Category = "Maintenance")
	bool NeedsMaintenance(AActor* Device) const;

	/**
	 * Perform maintenance on device
	 */
	UFUNCTION(BlueprintCallable, Category = "Maintenance")
	bool PerformMaintenance(AActor* Device);

	/**
	 * Get maintenance requirements for device
	 */
	UFUNCTION(BlueprintCallable, Category = "Maintenance")
	TArray<FMaintenanceRequirement> GetMaintenanceRequirements(AActor* Device) const;

	/**
	 * Get devices needing maintenance
	 */
	UFUNCTION(BlueprintCallable, Category = "Maintenance")
	TArray<AActor*> GetDevicesNeedingMaintenance() const;

	// ============================================================================
	// UPGRADE SYSTEM
	// ============================================================================

	/** Available upgrades by device type */
// Available upgrades (C++ only due to TArray in TMap limitation)
	TMap<EAutomationDeviceType, TArray<FAutomationUpgrade>> AvailableUpgrades;

	/** Applied upgrades per device */
	// Applied upgrades (C++ only due to TArray in TMap limitation)
	TMap<AActor*, TArray<FAutomationUpgrade>> AppliedUpgrades;

	/**
	 * Get available upgrades for device
	 */
	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	TArray<FAutomationUpgrade> GetAvailableUpgrades(AActor* Device) const;

	/**
	 * Apply upgrade to device
	 */
	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	bool ApplyUpgrade(AActor* Device, const FAutomationUpgrade& Upgrade);

	/**
	 * Get applied upgrades for device
	 */
	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	TArray<FAutomationUpgrade> GetAppliedUpgrades(AActor* Device) const;

	/**
	 * Calculate upgrade effect multiplier
	 */
	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	float GetUpgradeMultiplier(AActor* Device, EAutomationUpgradeType UpgradeType) const;

	// ============================================================================
	// STATUS MONITORING
	// ============================================================================

	/**
	 * Update all device statuses
	 */
	UFUNCTION(BlueprintCallable, Category = "Monitoring")
	void UpdateAllDeviceStatuses();

	/**
	 * Update single device status
	 */
	UFUNCTION(BlueprintCallable, Category = "Monitoring")
	void UpdateDeviceStatus(AActor* Device, float DeltaTime);

	/**
	 * Get automation summary
	 */
	UFUNCTION(BlueprintCallable, Category = "Monitoring")
	void GetAutomationSummary(int32& TotalDevices, float& AvgEfficiency, float& LaborReduction, int32& NeedsMaintenance) const;

protected:
	// Update timer handle
	FTimerHandle UpdateTimerHandle;

	// Update interval (seconds)
	float UpdateInterval;

	// Initialize default maintenance requirements
	void InitializeMaintenanceRequirements();

	// Initialize default upgrades
	void InitializeUpgrades();

	// Calculate device efficiency based on operational hours and maintenance
	float CalculateDeviceEfficiency(const FAutomationDeviceStats& Stats) const;

	// Degrade device efficiency over time
	void DegradeDeviceEfficiency(AActor* Device, float DeltaTime);
};
