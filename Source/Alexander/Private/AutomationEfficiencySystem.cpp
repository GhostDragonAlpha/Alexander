// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutomationEfficiencySystem.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "IrrigationSystem.h"
#include "HarvestingMachine.h"
#include "PowerGenerator.h"
#include "SolarPanel.h"
#include "TimerManager.h"
#include "Engine/World.h"

// ============================================================================
// SUBSYSTEM LIFECYCLE
// ============================================================================

void UAutomationEfficiencySystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UpdateInterval = 5.0f; // Update every 5 seconds
	MaintenanceIntervalHours = 100.0f; // Maintenance every 100 hours

	InitializeMaintenanceRequirements();
	InitializeUpgrades();

	UE_LOG(LogTemp, Log, TEXT("AutomationEfficiencySystem: Initialized"));
}

void UAutomationEfficiencySystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}

	RegisteredDevices.Empty();
	AppliedUpgrades.Empty();

	Super::Deinitialize();
}

void UAutomationEfficiencySystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Start periodic update timer
	InWorld.GetTimerManager().SetTimer(
		UpdateTimerHandle,
		[this]()
		{
			UpdateAllDeviceStatuses();
		},
		UpdateInterval,
		true // Loop
	);

	UE_LOG(LogTemp, Log, TEXT("AutomationEfficiencySystem: Started monitoring"));
}

// ============================================================================
// DEVICE TRACKING
// ============================================================================

void UAutomationEfficiencySystem::RegisterDevice(AActor* Device, EAutomationDeviceType DeviceType)
{
	if (!Device)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutomationEfficiencySystem: Cannot register null device"));
		return;
	}

	if (RegisteredDevices.Contains(Device))
	{
		UE_LOG(LogTemp, Warning, TEXT("AutomationEfficiencySystem: Device already registered: %s"), *Device->GetName());
		return;
	}

	FAutomationDeviceStats Stats;
	Stats.DeviceType = DeviceType;
	Stats.Status = EAutomationDeviceStatus::Operational;
	Stats.EfficiencyPercent = 100.0f;
	Stats.OperationalHours = 0.0f;
	Stats.MaintenanceProgress = 0.0f;
	Stats.UpgradeLevel = 0;
	Stats.ManualLaborReduction = 0.8f; // 80% reduction

	RegisteredDevices.Add(Device, Stats);

	UE_LOG(LogTemp, Log, TEXT("AutomationEfficiencySystem: Registered device: %s (Type: %d)"),
		*Device->GetName(), (int32)DeviceType);
}

void UAutomationEfficiencySystem::UnregisterDevice(AActor* Device)
{
	if (!Device)
	{
		return;
	}

	if (RegisteredDevices.Remove(Device) > 0)
	{
		AppliedUpgrades.Remove(Device);
		UE_LOG(LogTemp, Log, TEXT("AutomationEfficiencySystem: Unregistered device: %s"), *Device->GetName());
	}
}

FAutomationDeviceStats UAutomationEfficiencySystem::GetDeviceStats(AActor* Device) const
{
	if (const FAutomationDeviceStats* Stats = RegisteredDevices.Find(Device))
	{
		return *Stats;
	}

	return FAutomationDeviceStats();
}

void UAutomationEfficiencySystem::UpdateDeviceStats(AActor* Device, const FAutomationDeviceStats& Stats)
{
	if (RegisteredDevices.Contains(Device))
	{
		RegisteredDevices[Device] = Stats;
	}
}

// ============================================================================
// EFFICIENCY TRACKING
// ============================================================================

float UAutomationEfficiencySystem::GetOverallEfficiency() const
{
	if (RegisteredDevices.Num() == 0)
	{
		return 0.0f;
	}

	float TotalEfficiency = 0.0f;
	for (const auto& Pair : RegisteredDevices)
	{
		TotalEfficiency += Pair.Value.EfficiencyPercent;
	}

	return TotalEfficiency / RegisteredDevices.Num();
}

float UAutomationEfficiencySystem::GetManualLaborReduction() const
{
	if (RegisteredDevices.Num() == 0)
	{
		return 0.0f;
	}

	float TotalReduction = 0.0f;
	int32 OperationalCount = 0;

	for (const auto& Pair : RegisteredDevices)
	{
		if (Pair.Value.Status == EAutomationDeviceStatus::Operational ||
			Pair.Value.Status == EAutomationDeviceStatus::Degraded)
		{
			TotalReduction += Pair.Value.ManualLaborReduction;
			OperationalCount++;
		}
	}

	if (OperationalCount == 0)
	{
		return 0.0f;
	}

	// Return average reduction as percentage
	return (TotalReduction / OperationalCount) * 100.0f;
}

float UAutomationEfficiencySystem::GetTotalOperationalHours() const
{
	float TotalHours = 0.0f;
	for (const auto& Pair : RegisteredDevices)
	{
		TotalHours += Pair.Value.OperationalHours;
	}
	return TotalHours;
}

void UAutomationEfficiencySystem::GetDeviceStatusCounts(int32& Operational, int32& Degraded, int32& Malfunction, int32& Offline) const
{
	Operational = 0;
	Degraded = 0;
	Malfunction = 0;
	Offline = 0;

	for (const auto& Pair : RegisteredDevices)
	{
		switch (Pair.Value.Status)
		{
		case EAutomationDeviceStatus::Operational:
			Operational++;
			break;
		case EAutomationDeviceStatus::Degraded:
			Degraded++;
			break;
		case EAutomationDeviceStatus::Malfunction:
			Malfunction++;
			break;
		case EAutomationDeviceStatus::Offline:
			Offline++;
			break;
		}
	}
}

// ============================================================================
// MAINTENANCE SYSTEM
// ============================================================================

void UAutomationEfficiencySystem::InitializeMaintenanceRequirements()
{
	// Irrigation System maintenance
	FMaintenanceRequirementArray IrrigationMaintenance;
	{
		FMaintenanceRequirement Req;
		Req.RequirementName = TEXT("Filter Replacement");
		Req.ItemRequired = TEXT("Water Filter");
		Req.QuantityRequired = 1;
		Req.TimeRequired = 30.0f;
		IrrigationMaintenance.Requirements.Add(Req);
	}
	{
		FMaintenanceRequirement Req;
		Req.RequirementName = TEXT("Nozzle Cleaning");
		Req.ItemRequired = TEXT("Cleaning Solution");
		Req.QuantityRequired = 1;
		Req.TimeRequired = 20.0f;
		IrrigationMaintenance.Requirements.Add(Req);
	}
	MaintenanceRequirements.Add(EAutomationDeviceType::IrrigationSystem, IrrigationMaintenance);

	// Harvesting Machine maintenance
	FMaintenanceRequirementArray HarvesterMaintenance;
	{
		FMaintenanceRequirement Req;
		Req.RequirementName = TEXT("Blade Sharpening");
		Req.ItemRequired = TEXT("Sharpening Stone");
		Req.QuantityRequired = 1;
		Req.TimeRequired = 45.0f;
		HarvesterMaintenance.Requirements.Add(Req);
	}
	{
		FMaintenanceRequirement Req;
		Req.RequirementName = TEXT("Lubrication");
		Req.ItemRequired = TEXT("Machine Oil");
		Req.QuantityRequired = 2;
		Req.TimeRequired = 15.0f;
		HarvesterMaintenance.Requirements.Add(Req);
	}
	MaintenanceRequirements.Add(EAutomationDeviceType::HarvestingMachine, HarvesterMaintenance);

	// Power Generator maintenance
	FMaintenanceRequirementArray GeneratorMaintenance;
	{
		FMaintenanceRequirement Req;
		Req.RequirementName = TEXT("Oil Change");
		Req.ItemRequired = TEXT("Engine Oil");
		Req.QuantityRequired = 5;
		Req.TimeRequired = 60.0f;
		GeneratorMaintenance.Requirements.Add(Req);
	}
	{
		FMaintenanceRequirement Req;
		Req.RequirementName = TEXT("Spark Plug Replacement");
		Req.ItemRequired = TEXT("Spark Plug");
		Req.QuantityRequired = 4;
		Req.TimeRequired = 30.0f;
		GeneratorMaintenance.Requirements.Add(Req);
	}
	MaintenanceRequirements.Add(EAutomationDeviceType::PowerGenerator, GeneratorMaintenance);

	// Solar Panel maintenance
	FMaintenanceRequirementArray SolarMaintenance;
	{
		FMaintenanceRequirement Req;
		Req.RequirementName = TEXT("Panel Cleaning");
		Req.ItemRequired = TEXT("Cleaning Cloth");
		Req.QuantityRequired = 1;
		Req.TimeRequired = 15.0f;
		SolarMaintenance.Requirements.Add(Req);
	}
	MaintenanceRequirements.Add(EAutomationDeviceType::SolarPanel, SolarMaintenance);
}

bool UAutomationEfficiencySystem::NeedsMaintenance(AActor* Device) const
{
	const FAutomationDeviceStats* Stats = RegisteredDevices.Find(Device);
	if (!Stats)
	{
		return false;
	}

	return Stats->MaintenanceProgress >= 0.8f; // Needs maintenance at 80%
}

bool UAutomationEfficiencySystem::PerformMaintenance(AActor* Device)
{
	FAutomationDeviceStats* Stats = RegisteredDevices.Find(Device);
	if (!Stats)
	{
		return false;
	}

	// Reset maintenance progress
	Stats->MaintenanceProgress = 0.0f;

	// Restore efficiency
	Stats->EfficiencyPercent = 100.0f;

	// Update status
	if (Stats->Status == EAutomationDeviceStatus::Degraded)
	{
		Stats->Status = EAutomationDeviceStatus::Operational;
	}

	UE_LOG(LogTemp, Log, TEXT("AutomationEfficiencySystem: Performed maintenance on %s"), *Device->GetName());

	return true;
}

TArray<FMaintenanceRequirement> UAutomationEfficiencySystem::GetMaintenanceRequirements(AActor* Device) const
{
	const FAutomationDeviceStats* Stats = RegisteredDevices.Find(Device);
	if (!Stats)
	{
		return TArray<FMaintenanceRequirement>();
	}

	const FMaintenanceRequirementArray* RequirementArray = MaintenanceRequirements.Find(Stats->DeviceType);
	if (RequirementArray)
	{
		return RequirementArray->Requirements;
	}

	return TArray<FMaintenanceRequirement>();
}

TArray<AActor*> UAutomationEfficiencySystem::GetDevicesNeedingMaintenance() const
{
	TArray<AActor*> DevicesNeedingMaintenance;

	for (const auto& Pair : RegisteredDevices)
	{
		if (NeedsMaintenance(Pair.Key))
		{
			DevicesNeedingMaintenance.Add(Pair.Key);
		}
	}

	return DevicesNeedingMaintenance;
}

// ============================================================================
// UPGRADE SYSTEM
// ============================================================================

void UAutomationEfficiencySystem::InitializeUpgrades()
{
	// Irrigation System upgrades
	FAutomationUpgradeArray IrrigationUpgrades;
	{
		FAutomationUpgrade Upgrade;
		Upgrade.UpgradeType = EAutomationUpgradeType::Efficiency;
		Upgrade.Level = 1;
		Upgrade.UpgradeName = TEXT("Water Efficiency I");
		Upgrade.Description = TEXT("Reduces water consumption by 10%");
		Upgrade.EffectMultiplier = 0.9f; // 10% reduction
		Upgrade.Cost = 500.0f;
		IrrigationUpgrades.Upgrades.Add(Upgrade);
	}
	{
		FAutomationUpgrade Upgrade;
		Upgrade.UpgradeType = EAutomationUpgradeType::Range;
		Upgrade.Level = 1;
		Upgrade.UpgradeName = TEXT("Extended Range I");
		Upgrade.Description = TEXT("Increases watering radius by 20%");
		Upgrade.EffectMultiplier = 1.2f; // 20% increase
		Upgrade.Cost = 750.0f;
		IrrigationUpgrades.Upgrades.Add(Upgrade);
	}
	{
		FAutomationUpgrade Upgrade;
		Upgrade.UpgradeType = EAutomationUpgradeType::Capacity;
		Upgrade.Level = 1;
		Upgrade.UpgradeName = TEXT("Large Tank I");
		Upgrade.Description = TEXT("Increases water capacity by 50%");
		Upgrade.EffectMultiplier = 1.5f; // 50% increase
		Upgrade.Cost = 1000.0f;
		IrrigationUpgrades.Upgrades.Add(Upgrade);
	}
	AvailableUpgrades.Add(EAutomationDeviceType::IrrigationSystem, IrrigationUpgrades);

	// Harvesting Machine upgrades
	FAutomationUpgradeArray HarvesterUpgrades;
	{
		FAutomationUpgrade Upgrade;
		Upgrade.UpgradeType = EAutomationUpgradeType::Speed;
		Upgrade.Level = 1;
		Upgrade.UpgradeName = TEXT("Fast Harvesting I");
		Upgrade.Description = TEXT("Reduces harvest interval by 25%");
		Upgrade.EffectMultiplier = 0.75f; // 25% faster
		Upgrade.Cost = 800.0f;
		HarvesterUpgrades.Upgrades.Add(Upgrade);
	}
	{
		FAutomationUpgrade Upgrade;
		Upgrade.UpgradeType = EAutomationUpgradeType::Capacity;
		Upgrade.Level = 1;
		Upgrade.UpgradeName = TEXT("Large Storage I");
		Upgrade.Description = TEXT("Increases storage capacity by 100%");
		Upgrade.EffectMultiplier = 2.0f; // Double capacity
		Upgrade.Cost = 1200.0f;
		HarvesterUpgrades.Upgrades.Add(Upgrade);
	}
	{
		FAutomationUpgrade Upgrade;
		Upgrade.UpgradeType = EAutomationUpgradeType::Range;
		Upgrade.Level = 1;
		Upgrade.UpgradeName = TEXT("Extended Reach I");
		Upgrade.Description = TEXT("Increases harvest radius by 30%");
		Upgrade.EffectMultiplier = 1.3f; // 30% increase
		Upgrade.Cost = 900.0f;
		HarvesterUpgrades.Upgrades.Add(Upgrade);
	}
	AvailableUpgrades.Add(EAutomationDeviceType::HarvestingMachine, HarvesterUpgrades);

	// Power Generator upgrades
	FAutomationUpgradeArray GeneratorUpgrades;
	{
		FAutomationUpgrade Upgrade;
		Upgrade.UpgradeType = EAutomationUpgradeType::Efficiency;
		Upgrade.Level = 1;
		Upgrade.UpgradeName = TEXT("Fuel Efficiency I");
		Upgrade.Description = TEXT("Reduces fuel consumption by 15%");
		Upgrade.EffectMultiplier = 0.85f; // 15% reduction
		Upgrade.Cost = 1500.0f;
		GeneratorUpgrades.Upgrades.Add(Upgrade);
	}
	{
		FAutomationUpgrade Upgrade;
		Upgrade.UpgradeType = EAutomationUpgradeType::Capacity;
		Upgrade.Level = 1;
		Upgrade.UpgradeName = TEXT("Large Fuel Tank I");
		Upgrade.Description = TEXT("Increases fuel capacity by 50%");
		Upgrade.EffectMultiplier = 1.5f; // 50% increase
		Upgrade.Cost = 1000.0f;
		GeneratorUpgrades.Upgrades.Add(Upgrade);
	}
	AvailableUpgrades.Add(EAutomationDeviceType::PowerGenerator, GeneratorUpgrades);

	// Solar Panel upgrades
	FAutomationUpgradeArray SolarUpgrades;
	{
		FAutomationUpgrade Upgrade;
		Upgrade.UpgradeType = EAutomationUpgradeType::Efficiency;
		Upgrade.Level = 1;
		Upgrade.UpgradeName = TEXT("High-Efficiency Cells I");
		Upgrade.Description = TEXT("Increases power output by 25%");
		Upgrade.EffectMultiplier = 1.25f; // 25% increase
		Upgrade.Cost = 2000.0f;
		SolarUpgrades.Upgrades.Add(Upgrade);
	}
	{
		FAutomationUpgrade Upgrade;
		Upgrade.UpgradeType = EAutomationUpgradeType::Durability;
		Upgrade.Level = 1;
		Upgrade.UpgradeName = TEXT("Weather Resistant I");
		Upgrade.Description = TEXT("Reduces maintenance frequency by 50%");
		Upgrade.EffectMultiplier = 0.5f; // 50% slower degradation
		Upgrade.Cost = 1500.0f;
		SolarUpgrades.Upgrades.Add(Upgrade);
	}
	AvailableUpgrades.Add(EAutomationDeviceType::SolarPanel, SolarUpgrades);
}

TArray<FAutomationUpgrade> UAutomationEfficiencySystem::GetAvailableUpgrades(AActor* Device) const
{
	const FAutomationDeviceStats* Stats = RegisteredDevices.Find(Device);
	if (!Stats)
	{
		return TArray<FAutomationUpgrade>();
	}

	const FAutomationUpgradeArray* UpgradeArray = AvailableUpgrades.Find(Stats->DeviceType);
	if (UpgradeArray)
	{
		return UpgradeArray->Upgrades;
	}

	return TArray<FAutomationUpgrade>();
}

bool UAutomationEfficiencySystem::ApplyUpgrade(AActor* Device, const FAutomationUpgrade& Upgrade)
{
	FAutomationDeviceStats* Stats = RegisteredDevices.Find(Device);
	if (!Stats)
	{
		return false;
	}

	// Add to applied upgrades
	FAutomationUpgradeArray& DeviceUpgrades = AppliedUpgrades.FindOrAdd(Device);
	DeviceUpgrades.Upgrades.Add(Upgrade);

	// Increment upgrade level
	Stats->UpgradeLevel++;

	// Apply upgrade effects based on type
	switch (Upgrade.UpgradeType)
	{
	case EAutomationUpgradeType::Efficiency:
		// Efficiency upgrades improve overall efficiency
		Stats->EfficiencyPercent = FMath::Min(Stats->EfficiencyPercent * Upgrade.EffectMultiplier, 120.0f);
		break;

	case EAutomationUpgradeType::Durability:
		// Durability upgrades slow maintenance progress
		Stats->MaintenanceProgress *= Upgrade.EffectMultiplier;
		break;

	default:
		// Other upgrades are applied directly to device properties
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("AutomationEfficiencySystem: Applied upgrade '%s' to %s"),
		*Upgrade.UpgradeName, *Device->GetName());

	return true;
}

TArray<FAutomationUpgrade> UAutomationEfficiencySystem::GetAppliedUpgrades(AActor* Device) const
{
	const FAutomationUpgradeArray* UpgradeArray = AppliedUpgrades.Find(Device);
	if (UpgradeArray)
	{
		return UpgradeArray->Upgrades;
	}

	return TArray<FAutomationUpgrade>();
}

float UAutomationEfficiencySystem::GetUpgradeMultiplier(AActor* Device, EAutomationUpgradeType UpgradeType) const
{
	const FAutomationUpgradeArray* UpgradeArray = AppliedUpgrades.Find(Device);
	if (!UpgradeArray)
	{
		return 1.0f;
	}

	float Multiplier = 1.0f;
	for (const FAutomationUpgrade& Upgrade : UpgradeArray->Upgrades)
	{
		if (Upgrade.UpgradeType == UpgradeType)
		{
			Multiplier *= Upgrade.EffectMultiplier;
		}
	}

	return Multiplier;
}

// ============================================================================
// STATUS MONITORING
// ============================================================================

void UAutomationEfficiencySystem::UpdateAllDeviceStatuses()
{
	for (auto& Pair : RegisteredDevices)
	{
		UpdateDeviceStatus(Pair.Key, UpdateInterval);
	}
}

void UAutomationEfficiencySystem::UpdateDeviceStatus(AActor* Device, float DeltaTime)
{
	FAutomationDeviceStats* Stats = RegisteredDevices.Find(Device);
	if (!Stats)
	{
		return;
	}

	// Check if device is powered (if it implements IPowerConsumer)
	if (Device->Implements<UPowerConsumer>())
	{
		IPowerConsumer* Consumer = Cast<IPowerConsumer>(Device);
		if (Consumer && !Consumer->Execute_IsPowered(Device))
		{
			Stats->Status = EAutomationDeviceStatus::Offline;
			return;
		}
	}

	// Update operational hours
	float HoursElapsed = DeltaTime / 3600.0f;
	Stats->OperationalHours += HoursElapsed;

	// Degrade efficiency over time
	DegradeDeviceEfficiency(Device, DeltaTime);

	// Update maintenance progress
	float DurabilityMultiplier = GetUpgradeMultiplier(Device, EAutomationUpgradeType::Durability);
	Stats->MaintenanceProgress += (HoursElapsed / MaintenanceIntervalHours) * DurabilityMultiplier;
	Stats->MaintenanceProgress = FMath::Clamp(Stats->MaintenanceProgress, 0.0f, 1.0f);

	// Update status based on maintenance progress
	if (Stats->MaintenanceProgress >= 1.0f)
	{
		Stats->Status = EAutomationDeviceStatus::Malfunction;
		Stats->EfficiencyPercent = 0.0f;
	}
	else if (Stats->MaintenanceProgress >= 0.8f)
	{
		Stats->Status = EAutomationDeviceStatus::Degraded;
	}
	else if (Stats->Status != EAutomationDeviceStatus::Offline)
	{
		Stats->Status = EAutomationDeviceStatus::Operational;
	}
}

void UAutomationEfficiencySystem::GetAutomationSummary(int32& TotalDevices, float& AvgEfficiency, float& LaborReduction, int32& NeedsMaintenance) const
{
	TotalDevices = RegisteredDevices.Num();
	AvgEfficiency = GetOverallEfficiency();
	LaborReduction = GetManualLaborReduction();
	NeedsMaintenance = GetDevicesNeedingMaintenance().Num();
}

// ============================================================================
// PROTECTED METHODS
// ============================================================================

float UAutomationEfficiencySystem::CalculateDeviceEfficiency(const FAutomationDeviceStats& Stats) const
{
	// Base efficiency
	float Efficiency = 100.0f;

	// Reduce efficiency based on maintenance progress
	if (Stats.MaintenanceProgress > 0.5f)
	{
		float MaintenancePenalty = (Stats.MaintenanceProgress - 0.5f) * 2.0f; // 0-1 range
		Efficiency -= MaintenancePenalty * 30.0f; // Up to 30% penalty
	}

	// Status affects efficiency
	switch (Stats.Status)
	{
	case EAutomationDeviceStatus::Degraded:
		Efficiency *= 0.7f; // 70% efficiency when degraded
		break;
	case EAutomationDeviceStatus::Malfunction:
		Efficiency = 0.0f; // No efficiency when malfunctioning
		break;
	case EAutomationDeviceStatus::Offline:
		Efficiency = 0.0f; // No efficiency when offline
		break;
	default:
		break;
	}

	return FMath::Clamp(Efficiency, 0.0f, 120.0f);
}

void UAutomationEfficiencySystem::DegradeDeviceEfficiency(AActor* Device, float DeltaTime)
{
	FAutomationDeviceStats* Stats = RegisteredDevices.Find(Device);
	if (!Stats)
	{
		return;
	}

	// Calculate efficiency based on current state
	float TargetEfficiency = CalculateDeviceEfficiency(*Stats);

	// Smoothly interpolate to target efficiency
	Stats->EfficiencyPercent = FMath::FInterpTo(
		Stats->EfficiencyPercent,
		TargetEfficiency,
		DeltaTime,
		0.5f // Interpolation speed
	);
}
