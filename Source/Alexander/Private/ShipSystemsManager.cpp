// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShipSystemsManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

UShipSystemsManager::UShipSystemsManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// Initialize power systems
	TotalPowerAvailable = 1000.0f;
	CurrentPowerUsage = 0.0f;
	PowerReserve = 500.0f;
	
	// Initialize internal state
	bDiagnosticRunning = false;
	LastDiagnosticTime = 0.0f;
	SystemUpdateCount = 0;
	AverageUpdateTime = 0.0f;
}

void UShipSystemsManager::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize Performance Profiler
	PerformanceProfiler = NewObject<UPerformanceProfiler>(this);
	PerformanceProfiler->RegisterSystem(TEXT("ShipSystemsManager"), EPerformanceCategory::Gameplay);
	PerformanceProfiler->SetProfilingLevel(EProfilingLevel::Detailed);
	
	// Initialize all ship systems
	InitializeSystems();
	
	UE_LOG(LogTemp, Log, TEXT("ShipSystemsManager initialized with %d systems"), ShipSystems.Num());
}

void UShipSystemsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Begin performance profiling
	if (PerformanceProfiler)
	{
		PerformanceProfiler->BeginSystemTick(TEXT("ShipSystemsManager"));
	}
	
	// Update performance tracking
	SystemUpdateCount++;
	AverageUpdateTime = (AverageUpdateTime * (SystemUpdateCount - 1) + DeltaTime) / SystemUpdateCount;
	
	// Update all systems
	UpdateEngineSystems(DeltaTime);
	UpdateLifeSupportSystems(DeltaTime);
	UpdateShieldSystems(DeltaTime);
	UpdateWeaponSystems(DeltaTime);
	UpdateSensorSystems(DeltaTime);
	UpdateCommunicationSystems(DeltaTime);
	UpdateNavigationSystems(DeltaTime);
	UpdatePowerSystems(DeltaTime);
	UpdateCoolingSystems(DeltaTime);
	
	// Update system interactions
	UpdateSystemInteractions();
	
	// Check for critical failures
	CheckForCriticalFailures();
	
	// Update system temperatures
	UpdateSystemTemperatures(DeltaTime);
	
	// End performance profiling
	if (PerformanceProfiler)
	{
		PerformanceProfiler->EndSystemTick(TEXT("ShipSystemsManager"));
	}
}

void UShipSystemsManager::InitializeSystems()
{
	ShipSystems.Empty();
	SystemPriorities.Empty();
	SystemTemperatures.Empty();
	
	// Initialize default ship systems
	InitializeDefaultSystems();
	
	// Set initial power priorities
	SetPowerPriority(EShipSystemType::LifeSupport, 1); // Highest priority
	SetPowerPriority(EShipSystemType::Power, 2);
	SetPowerPriority(EShipSystemType::Cooling, 3);
	SetPowerPriority(EShipSystemType::Navigation, 4);
	SetPowerPriority(EShipSystemType::Sensors, 5);
	SetPowerPriority(EShipSystemType::Communications, 6);
	SetPowerPriority(EShipSystemType::Shields, 7);
	SetPowerPriority(EShipSystemType::Weapons, 8);
	SetPowerPriority(EShipSystemType::Engines, 9); // Lowest priority
	
	// Initialize all systems to online status
	for (FShipSystemData& System : ShipSystems)
	{
		System.Status = EShipSystemStatus::Online;
		System.Health = 100.0f;
		System.Efficiency = 1.0f;
		System.Temperature = 20.0f;
		System.bIsCritical = IsSystemCritical(System.SystemType);
		UpdateSystemStatusMessage(System);
	}
	
	// Calculate initial power usage
	CurrentPowerUsage = CalculateTotalPowerUsage();
}

FShipSystemData* UShipSystemsManager::GetSystem(EShipSystemType SystemType)
{
	for (FShipSystemData& System : ShipSystems)
	{
		if (System.SystemType == SystemType)
		{
			return &System;
		}
	}
	return nullptr;
}

void UShipSystemsManager::SetSystemStatus(EShipSystemType SystemType, EShipSystemStatus NewStatus)
{
	FShipSystemData* System = GetSystem(SystemType);
	if (!System)
		return;
		
	EShipSystemStatus OldStatus = System->Status;
	System->Status = NewStatus;
	UpdateSystemStatusMessage(*System);
	
	// Broadcast status change
	OnSystemStatusChanged.Broadcast(SystemType, NewStatus);
	
	UE_LOG(LogTemp, Log, TEXT("System %s status changed to %s"), 
		*System->SystemName, *UEnum::GetValueAsString(NewStatus));
}

void UShipSystemsManager::DamageSystem(EShipSystemType SystemType, float DamageAmount)
{
	FShipSystemData* System = GetSystem(SystemType);
	if (!System)
		return;
		
	System->Health = FMath::Max(0.0f, System->Health - DamageAmount);
	
	// Update status based on health
	if (System->Health <= 0.0f)
	{
		SetSystemStatus(SystemType, EShipSystemStatus::Offline);
	}
	else if (System->Health < 25.0f)
	{
		SetSystemStatus(SystemType, EShipSystemStatus::Critical);
	}
	else if (System->Health < 50.0f)
	{
		SetSystemStatus(SystemType, EShipSystemStatus::Failing);
	}
	else if (System->Health < 75.0f)
	{
		SetSystemStatus(SystemType, EShipSystemStatus::Damaged);
	}
	else
	{
		SetSystemStatus(SystemType, EShipSystemStatus::Online);
	}
	
	// Update efficiency based on health
	System->Efficiency = System->Health / 100.0f;
	
	// Broadcast damage event
	OnSystemDamaged.Broadcast(SystemType, DamageAmount);
	
	UE_LOG(LogTemp, Warning, TEXT("System %s took %.1f damage, health: %.1f%%"), 
		*System->SystemName, DamageAmount, System->Health);
}

void UShipSystemsManager::RepairSystem(EShipSystemType SystemType, float RepairAmount)
{
	FShipSystemData* System = GetSystem(SystemType);
	if (!System)
		return;
		
	float OldHealth = System->Health;
	System->Health = FMath::Min(100.0f, System->Health + RepairAmount);
	
	// Update status based on new health
	if (System->Health >= 100.0f)
	{
		SetSystemStatus(SystemType, EShipSystemStatus::Online);
	}
	else if (System->Health >= 75.0f)
	{
		SetSystemStatus(SystemType, EShipSystemStatus::Online);
	}
	else if (System->Health >= 50.0f)
	{
		SetSystemStatus(SystemType, EShipSystemStatus::Damaged);
	}
	else if (System->Health >= 25.0f)
	{
		SetSystemStatus(SystemType, EShipSystemStatus::Failing);
	}
	
	// Update efficiency
	System->Efficiency = System->Health / 100.0f;
	
	// Broadcast repair event
	OnSystemRepaired.Broadcast(SystemType);
	
	UE_LOG(LogTemp, Log, TEXT("System %s repaired %.1f, health: %.1f%%"), 
		*System->SystemName, RepairAmount, System->Health);
}

float UShipSystemsManager::CalculateTotalPowerUsage()
{
	float TotalUsage = 0.0f;
	
	for (const FShipSystemData& System : ShipSystems)
	{
		if (System.Status == EShipSystemStatus::Online)
		{
			TotalUsage += System.PowerUsage * System.Efficiency;
		}
		else if (System.Status == EShipSystemStatus::Damaged || 
				 System.Status == EShipSystemStatus::Failing)
		{
			// Damaged systems use more power
			TotalUsage += System.PowerUsage * System.Efficiency * 1.5f;
		}
	}
	
	return TotalUsage;
}

void UShipSystemsManager::RedistributePower()
{
	// Prioritize critical systems
	TArray<FShipSystemData*> CriticalSystems;
	TArray<FShipSystemData*> NonCriticalSystems;
	
	for (FShipSystemData& System : ShipSystems)
	{
		if (System.bIsCritical && System.Status != EShipSystemStatus::Offline)
		{
			CriticalSystems.Add(&System);
		}
		else if (System.Status != EShipSystemStatus::Offline)
		{
			NonCriticalSystems.Add(&System);
		}
	}
	
	// Calculate available power for non-critical systems
	float CriticalPowerUsage = 0.0f;
	for (FShipSystemData* System : CriticalSystems)
	{
		CriticalPowerUsage += System->PowerUsage * System->Efficiency;
	}
	
	float AvailablePower = TotalPowerAvailable - CriticalPowerUsage;
	
	// Distribute remaining power to non-critical systems
	if (NonCriticalSystems.Num() > 0)
	{
		float PowerPerSystem = AvailablePower / NonCriticalSystems.Num();
		
		for (FShipSystemData* System : NonCriticalSystems)
		{
			float RequiredPower = System->PowerUsage * System->Efficiency;
			
			if (RequiredPower > PowerPerSystem)
			{
				// Reduce system efficiency to match available power
				System->Efficiency = PowerPerSystem / System->PowerUsage;
			}
			else
			{
				// Restore full efficiency
				System->Efficiency = System->Health / 100.0f;
			}
		}
	}
}

float UShipSystemsManager::GetPowerEfficiency() const
{
	if (TotalPowerAvailable <= 0.0f)
		return 0.0f;
		
	return FMath::Clamp(CurrentPowerUsage / TotalPowerAvailable, 0.0f, 1.0f);
}

bool UShipSystemsManager::RunSystemDiagnostic()
{
	if (bDiagnosticRunning)
	{
		UE_LOG(LogTemp, Warning, TEXT("Diagnostic already running"));
		return false;
	}
	
	bDiagnosticRunning = true;
	LastDiagnosticTime = GetWorld()->GetTimeSeconds();
	
	UE_LOG(LogTemp, Log, TEXT("Starting comprehensive system diagnostic..."));
	
	// Test each system
	bool AllSystemsHealthy = true;
	for (FShipSystemData& System : ShipSystems)
	{
		// Simulate diagnostic test
		float DiagnosticResult = FMath::RandRange(0.7f, 1.0f) * System.Efficiency;
		
		if (DiagnosticResult < 0.5f)
		{
			SetSystemStatus(System.SystemType, EShipSystemStatus::Critical);
			AllSystemsHealthy = false;
		}
		else if (DiagnosticResult < 0.8f)
		{
			SetSystemStatus(System.SystemType, EShipSystemStatus::Damaged);
			AllSystemsHealthy = false;
		}
		else
		{
			SetSystemStatus(System.SystemType, EShipSystemStatus::Online);
		}
	}
	
	bDiagnosticRunning = false;
	
	UE_LOG(LogTemp, Log, TEXT("System diagnostic complete. All systems healthy: %s"), 
		AllSystemsHealthy ? TEXT("Yes") : TEXT("No"));
	
	return AllSystemsHealthy;
}

FString UShipSystemsManager::GetSystemHealthReport()
{
	FString Report = TEXT("=== SHIP SYSTEMS HEALTH REPORT ===\n\n");
	
	for (const FShipSystemData& System : ShipSystems)
	{
		Report += FString::Printf(TEXT("%s:\n"), *System.SystemName);
		Report += FString::Printf(TEXT("  Status: %s\n"), *UEnum::GetValueAsString(System.Status));
		Report += FString::Printf(TEXT("  Health: %.1f%%\n"), System.Health);
		Report += FString::Printf(TEXT("  Efficiency: %.1f%%\n"), System.Efficiency * 100.0f);
		Report += FString::Printf(TEXT("  Temperature: %.1f°C\n"), System.Temperature);
		Report += FString::Printf(TEXT("  Power Usage: %.1f MW\n"), System.PowerUsage);
		Report += FString::Printf(TEXT("  Critical: %s\n\n"), System.bIsCritical ? TEXT("Yes") : TEXT("No"));
	}
	
	Report += FString::Printf(TEXT("Total Power Usage: %.1f / %.1f MW (%.1f%%)\n"), 
		CurrentPowerUsage, TotalPowerAvailable, GetPowerEfficiency() * 100.0f);
	
	return Report;
}

TArray<FString> UShipSystemsManager::GetCriticalAlerts()
{
	TArray<FString> Alerts;
	
	for (const FShipSystemData& System : ShipSystems)
	{
		if (System.Status == EShipSystemStatus::Critical || System.Status == EShipSystemStatus::Failing)
		{
			FString Alert = FString::Printf(TEXT("CRITICAL: %s - %s"), 
				*System.SystemName, *System.StatusMessage);
			Alerts.Add(Alert);
		}
		
		if (System.Temperature > 80.0f)
		{
			FString Alert = FString::Printf(TEXT("WARNING: %s - Overheating (%.1f°C)"), 
				*System.SystemName, System.Temperature);
			Alerts.Add(Alert);
		}
	}
	
	return Alerts;
}

FString UShipSystemsManager::GetStatusSummary()
{
	int32 OnlineSystems = 0;
	int32 DamagedSystems = 0;
	int32 CriticalSystems = 0;
	int32 OfflineSystems = 0;
	
	for (const FShipSystemData& System : ShipSystems)
	{
		switch (System.Status)
		{
		case EShipSystemStatus::Online:
			OnlineSystems++;
			break;
		case EShipSystemStatus::Damaged:
		case EShipSystemStatus::Failing:
			DamagedSystems++;
			break;
		case EShipSystemStatus::Critical:
			CriticalSystems++;
			break;
		case EShipSystemStatus::Offline:
			OfflineSystems++;
			break;
		}
	}
	
	FString Summary = FString::Printf(TEXT("Ship Status Summary:\n"));
	Summary += FString::Printf(TEXT("Online: %d\n"), OnlineSystems);
	Summary += FString::Printf(TEXT("Damaged: %d\n"), DamagedSystems);
	Summary += FString::Printf(TEXT("Critical: %d\n"), CriticalSystems);
	Summary += FString::Printf(TEXT("Offline: %d\n"), OfflineSystems);
	Summary += FString::Printf(TEXT("Power Efficiency: %.1f%%\n"), GetPowerEfficiency() * 100.0f);
	Summary += FString::Printf(TEXT("Average Temperature: %.1f°C\n"), CalculateAverageTemperature());
	
	return Summary;
}

void UShipSystemsManager::UpdateEngineSystems(float DeltaTime)
{
	FShipSystemData* EngineSystem = GetSystem(EShipSystemType::Engines);
	if (!EngineSystem)
		return;
		
	// Engines generate heat based on usage
	float HeatGeneration = EngineSystem->PowerUsage * EngineSystem->Efficiency * 0.1f;
	EngineSystem->Temperature += HeatGeneration * DeltaTime;
	
	// Update power usage based on throttle (would get from FlightController)
	// This is a simplified simulation
	EngineSystem->PowerUsage = 200.0f * EngineSystem->Efficiency; // Base 200 MW
}

void UShipSystemsManager::UpdateLifeSupportSystems(float DeltaTime)
{
	FShipSystemData* LifeSupport = GetSystem(EShipSystemType::LifeSupport);
	if (!LifeSupport)
		return;
		
	// Life support is always critical and uses constant power
	LifeSupport->PowerUsage = 50.0f; // 50 MW constant
	LifeSupport->Temperature = 22.0f; // Stable temperature
	
	// Life support efficiency affects crew survival
	if (LifeSupport->Efficiency < 0.5f)
	{
		SetSystemStatus(EShipSystemType::LifeSupport, EShipSystemStatus::Critical);
	}
}

void UShipSystemsManager::UpdateShieldSystems(float DeltaTime)
{
	FShipSystemData* Shields = GetSystem(EShipSystemType::Shields);
	if (!Shields)
		return;
		
	// Shield power usage varies with status
	if (Shields->Status == EShipSystemStatus::Online)
	{
		Shields->PowerUsage = 150.0f * Shields->Efficiency; // 150 MW when active
	}
	else
	{
		Shields->PowerUsage = 5.0f; // 5 MW standby
	}
}

void UShipSystemsManager::UpdateWeaponSystems(float DeltaTime)
{
	FShipSystemData* Weapons = GetSystem(EShipSystemType::Weapons);
	if (!Weapons)
		return;
		
	// Weapons use minimal power when not firing
	Weapons->PowerUsage = 10.0f * Weapons->Efficiency; // 10 MW standby
}

void UShipSystemsManager::UpdateSensorSystems(float DeltaTime)
{
	FShipSystemData* Sensors = GetSystem(EShipSystemType::Sensors);
	if (!Sensors)
		return;
		
	// Sensors use constant power
	Sensors->PowerUsage = 30.0f * Sensors->Efficiency; // 30 MW
}

void UShipSystemsManager::UpdateCommunicationSystems(float DeltaTime)
{
	FShipSystemData* Comms = GetSystem(EShipSystemType::Communications);
	if (!Comms)
		return;
		
	// Communications use variable power
	Comms->PowerUsage = 20.0f * Comms->Efficiency; // 20 MW
}

void UShipSystemsManager::UpdateNavigationSystems(float DeltaTime)
{
	FShipSystemData* Navigation = GetSystem(EShipSystemType::Navigation);
	if (!Navigation)
		return;
		
	// Navigation uses constant power
	Navigation->PowerUsage = 25.0f * Navigation->Efficiency; // 25 MW
}

void UShipSystemsManager::UpdatePowerSystems(float DeltaTime)
{
	FShipSystemData* Power = GetSystem(EShipSystemType::Power);
	if (!Power)
		return;
		
	// Power system manages itself
	Power->PowerUsage = 5.0f; // Minimal self-usage
	Power->Efficiency = 1.0f; // Always 100% efficient
}

void UShipSystemsManager::UpdateCoolingSystems(float DeltaTime)
{
	FShipSystemData* Cooling = GetSystem(EShipSystemType::Cooling);
	if (!Cooling)
		return;
		
	// Cooling power usage scales with system temperatures
	float AverageTemp = CalculateAverageTemperature();
	float CoolingPower = FMath::Clamp((AverageTemp - 20.0f) * 10.0f, 0.0f, 100.0f);
	Cooling->PowerUsage = CoolingPower * Cooling->Efficiency;
}

float UShipSystemsManager::CalculateAverageTemperature() const
{
	if (ShipSystems.Num() == 0)
		return 20.0f;
		
	float TotalTemp = 0.0f;
	for (const FShipSystemData& System : ShipSystems)
	{
		TotalTemp += System.Temperature;
	}
	
	return TotalTemp / ShipSystems.Num();
}

void UShipSystemsManager::UpdateSystemInteractions()
{
	// Check for cascading effects
	for (FShipSystemData& System : ShipSystems)
	{
		if (System.Status == EShipSystemStatus::Critical)
		{
			ApplyCascadingEffects(System.SystemType, System.Status);
		}
	}
	
	// Update total power usage
	CurrentPowerUsage = CalculateTotalPowerUsage();
	
	// Handle power issues
	if (CurrentPowerUsage > TotalPowerAvailable)
	{
		HandlePowerShortage();
	}
	else if (CurrentPowerUsage < TotalPowerAvailable * 0.5f)
	{
		// Excess power - could recharge reserves
		float ExcessPower = TotalPowerAvailable - CurrentPowerUsage;
		PowerReserve = FMath::Min(PowerReserve + ExcessPower * 0.1f, 1000.0f);
	}
}

void UShipSystemsManager::HandleSystemFailure(EShipSystemType FailedSystem)
{
	// Broadcast critical failure
	OnCriticalFailure.Broadcast(FailedSystem);
	
	// Apply cascading effects
	ApplyCascadingEffects(FailedSystem, EShipSystemStatus::Offline);
	
	UE_LOG(LogTemp, Error, TEXT("CRITICAL: System %s has failed!"), 
		*UEnum::GetValueAsString(FailedSystem));
}

bool UShipSystemsManager::CheckForCriticalFailures()
{
	bool bCriticalFailure = false;
	
	for (const FShipSystemData& System : ShipSystems)
	{
		if (System.bIsCritical && System.Status == EShipSystemStatus::Offline)
		{
			HandleSystemFailure(System.SystemType);
			bCriticalFailure = true;
		}
		
		if (System.Temperature > 100.0f)
		{
			HandleOverheating(System.SystemType);
			bCriticalFailure = true;
		}
	}
	
	return bCriticalFailure;
}

void UShipSystemsManager::UpdateSystemTemperatures(float DeltaTime)
{
	for (FShipSystemData& System : ShipSystems)
	{
		// Temperature changes based on power usage and cooling
		float HeatGeneration = System.PowerUsage * 0.05f; // Simplified heat model
		float CoolingRate = (System.Temperature - 20.0f) * 0.1f; // Cooling to ambient
		
		System.Temperature += (HeatGeneration - CoolingRate) * DeltaTime;
		SystemTemperatures[System.SystemType] = System.Temperature;
		
		// Check for overheating
		if (System.Temperature > 90.0f)
		{
			HandleOverheating(System.SystemType);
		}
	}
}

void UShipSystemsManager::HandleOverheating(EShipSystemType SystemType)
{
	FShipSystemData* System = GetSystem(SystemType);
	if (!System)
		return;
		
	// Reduce efficiency to prevent damage
	System->Efficiency *= 0.5f;
	
	// Log warning
	UE_LOG(LogTemp, Warning, TEXT("WARNING: System %s is overheating (%.1f°C)"), 
		*System->SystemName, System->Temperature);
}

void UShipSystemsManager::ApplyCascadingEffects(EShipSystemType PrimarySystem, EShipSystemStatus NewStatus)
{
	// Define system dependencies
	switch (PrimarySystem)
	{
	case EShipSystemType::Power:
		// Power failure affects everything
		if (NewStatus == EShipSystemStatus::Offline)
		{
			for (FShipSystemData& System : ShipSystems)
			{
				if (System.SystemType != EShipSystemType::Power)
				{
					SetSystemStatus(System.SystemType, EShipSystemStatus::Offline);
				}
			}
		}
		break;
		
	case EShipSystemType::Cooling:
		// Cooling failure causes overheating
		if (NewStatus == EShipSystemStatus::Offline)
		{
			for (FShipSystemData& System : ShipSystems)
			{
				if (System.SystemType != EShipSystemType::Cooling)
				{
					System.Temperature = FMath::Min(System.Temperature + 20.0f, 120.0f);
				}
			}
		}
		break;
		
	case EShipSystemType::LifeSupport:
		// Life support failure is critical but doesn't cascade
		break;
		
	default:
		// Other systems have minimal cascading effects
		break;
	}
}

void UShipSystemsManager::DistributePower()
{
	// Simple power distribution - could be more complex
	RedistributePower();
}

void UShipSystemsManager::HandlePowerShortage()
{
	// Reduce power to non-critical systems
	RedistributePower();
	
	UE_LOG(LogTemp, Warning, TEXT("Power shortage detected. Redistributing power."));
}

void UShipSystemsManager::HandlePowerSurge()
{
	// Power surge could damage systems
	for (FShipSystemData& System : ShipSystems)
	{
		if (FMath::RandRange(0.0f, 1.0f) < 0.1f) // 10% chance of damage
		{
			DamageSystem(System.SystemType, FMath::RandRange(5.0f, 20.0f));
		}
	}
}

void UShipSystemsManager::InitializeDefaultSystems()
{
	// Create essential ship systems
	ShipSystems.Add(CreateSystem(EShipSystemType::Engines, TEXT("Main Engines"), true));
	ShipSystems.Add(CreateSystem(EShipSystemType::LifeSupport, TEXT("Life Support"), true));
	ShipSystems.Add(CreateSystem(EShipSystemType::Shields, TEXT("Shield Generators"), false));
	ShipSystems.Add(CreateSystem(EShipSystemType::Weapons, TEXT("Weapon Systems"), false));
	ShipSystems.Add(CreateSystem(EShipSystemType::Sensors, TEXT("Sensor Array"), false));
	ShipSystems.Add(CreateSystem(EShipSystemType::Communications, TEXT("Communications"), false));
	ShipSystems.Add(CreateSystem(EShipSystemType::Navigation, TEXT("Navigation Computer"), true));
	ShipSystems.Add(CreateSystem(EShipSystemType::Power, TEXT("Power Core"), true));
	ShipSystems.Add(CreateSystem(EShipSystemType::Cooling, TEXT("Cooling Systems"), true));
}

FShipSystemData UShipSystemsManager::CreateSystem(EShipSystemType Type, const FString& Name, bool bCritical)
{
	FShipSystemData System;
	System.SystemType = Type;
	System.SystemName = Name;
	System.Status = EShipSystemStatus::Offline;
	System.Health = 100.0f;
	System.PowerUsage = 0.0f;
	System.Efficiency = 1.0f;
	System.Temperature = 20.0f;
	System.bIsCritical = bCritical;
	UpdateSystemStatusMessage(System);
	
	return System;
}

void UShipSystemsManager::UpdateSystemStatusMessage(FShipSystemData& System)
{
	switch (System.Status)
	{
	case EShipSystemStatus::Online:
		System.StatusMessage = TEXT("System Online");
		break;
	case EShipSystemStatus::Damaged:
		System.StatusMessage = TEXT("System Damaged");
		break;
	case EShipSystemStatus::Critical:
		System.StatusMessage = TEXT("System Critical");
		break;
	case EShipSystemStatus::Failing:
		System.StatusMessage = TEXT("System Failing");
		break;
	case EShipSystemStatus::Offline:
		System.StatusMessage = TEXT("System Offline");
		break;
	}
}

bool UShipSystemsManager::IsSystemCritical(EShipSystemType SystemType) const
{
	return SystemType == EShipSystemType::LifeSupport || 
		   SystemType == EShipSystemType::Power || 
		   SystemType == EShipSystemType::Cooling ||
		   SystemType == EShipSystemType::Navigation;
}

void UShipSystemsManager::SetPowerPriority(EShipSystemType SystemType, int32 Priority)
{
	// Clamp priority to valid range (0 = lowest, 100 = highest)
	int32 ClampedPriority = FMath::Clamp(Priority, 0, 100);
	
	// Find the system and update its priority
	for (FShipSystemData& System : ShipSystems)
	{
		if (System.SystemType == SystemType)
		{
			// Store priority (could be in FShipSystemData if needed)
			// For now, adjust power allocation based on priority
			float PriorityMultiplier = (ClampedPriority / 100.0f) * 2.0f; // 0.0 to 2.0
			System.PowerUsage *= PriorityMultiplier;
			
			UE_LOG(LogTemp, Log, TEXT("Set power priority for %s to %d"), *System.SystemName, ClampedPriority);
			break;
		}
	}
}