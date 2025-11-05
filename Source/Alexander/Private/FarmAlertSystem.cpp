// Copyright Epic Games, Inc. All Rights Reserved.

#include "FarmAlertSystem.h"
#include "FarmPlot.h"
#include "AutomationEfficiencySystem.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

// ============================================================================
// SUBSYSTEM LIFECYCLE
// ============================================================================

void UFarmAlertSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	NextAlertID = 1;
	MasterAlertVolume = 1.0f;
	MonitoringInterval = 5.0f; // Check every 5 seconds

	InitializeAlertConfigurations();

	UE_LOG(LogTemp, Log, TEXT("FarmAlertSystem: Initialized"));
}

void UFarmAlertSystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
	}

	ActiveAlerts.Empty();
	RegisteredFarms.Empty();
	AlertCooldowns.Empty();

	Super::Deinitialize();
}

void UFarmAlertSystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Start periodic monitoring
	InWorld.GetTimerManager().SetTimer(
		MonitoringTimerHandle,
		[this]()
		{
			CheckAllFarms();
		},
		MonitoringInterval,
		true // Loop
	);

	UE_LOG(LogTemp, Log, TEXT("FarmAlertSystem: Started monitoring"));
}

// ============================================================================
// ALERT MANAGEMENT
// ============================================================================

int32 UFarmAlertSystem::TriggerAlert(
	EFarmAlertType AlertType,
	AActor* SourceActor,
	const FString& Message,
	const FString& DetailedMessage,
	EAlertPriority OverridePriority,
	bool bUseOverridePriority)
{
	// Get configuration
	FAlertConfiguration Config = GetAlertConfiguration(AlertType);

	// Check if alert type is enabled
	if (!Config.bEnabled)
	{
		return -1;
	}

	// Check cooldown
	if (IsAlertOnCooldown(AlertType, SourceActor))
	{
		return -1;
	}

	// Create alert
	FFarmAlert Alert;
	Alert.AlertID = NextAlertID++;
	Alert.AlertType = AlertType;
	Alert.Priority = bUseOverridePriority ? OverridePriority : Config.DefaultPriority;
	Alert.Message = Message;
	Alert.DetailedMessage = DetailedMessage.IsEmpty() ? Message : DetailedMessage;
	Alert.SourceActor = SourceActor;
	Alert.Location = SourceActor ? SourceActor->GetActorLocation() : FVector::ZeroVector;
	Alert.Timestamp = FDateTime::Now();
	Alert.bAcknowledged = false;
	Alert.bResolved = false;

	// Add to active alerts
	ActiveAlerts.Add(Alert);

	// Set cooldown
	SetAlertCooldown(AlertType, SourceActor, Config.CooldownSeconds);

	// Play sound
	if (Config.bPlaySound && Config.AlertSound)
	{
		PlayAlertSound(Config, Alert.Location);
	}

	// Show visual notification
	if (Config.bShowVisualNotification)
	{
		ShowVisualNotification(Alert, Config);
	}

	// Broadcast event
	OnAlertTriggered.Broadcast(Alert);

	UE_LOG(LogTemp, Log, TEXT("FarmAlertSystem: Triggered alert %d - %s"), Alert.AlertID, *Message);

	return Alert.AlertID;
}

bool UFarmAlertSystem::AcknowledgeAlert(int32 AlertID)
{
	for (FFarmAlert& Alert : ActiveAlerts)
	{
		if (Alert.AlertID == AlertID)
		{
			Alert.bAcknowledged = true;
			OnAlertAcknowledged.Broadcast(AlertID);
			UE_LOG(LogTemp, Log, TEXT("FarmAlertSystem: Acknowledged alert %d"), AlertID);
			return true;
		}
	}

	return false;
}

bool UFarmAlertSystem::ResolveAlert(int32 AlertID)
{
	for (FFarmAlert& Alert : ActiveAlerts)
	{
		if (Alert.AlertID == AlertID)
		{
			Alert.bResolved = true;
			Alert.bAcknowledged = true;
			OnAlertResolved.Broadcast(AlertID);
			UE_LOG(LogTemp, Log, TEXT("FarmAlertSystem: Resolved alert %d"), AlertID);
			return true;
		}
	}

	return false;
}

bool UFarmAlertSystem::DismissAlert(int32 AlertID)
{
	for (int32 i = ActiveAlerts.Num() - 1; i >= 0; --i)
	{
		if (ActiveAlerts[i].AlertID == AlertID)
		{
			ActiveAlerts.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("FarmAlertSystem: Dismissed alert %d"), AlertID);
			return true;
		}
	}

	return false;
}

void UFarmAlertSystem::ClearResolvedAlerts()
{
	int32 RemovedCount = 0;
	for (int32 i = ActiveAlerts.Num() - 1; i >= 0; --i)
	{
		if (ActiveAlerts[i].bResolved)
		{
			ActiveAlerts.RemoveAt(i);
			RemovedCount++;
		}
	}

	if (RemovedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("FarmAlertSystem: Cleared %d resolved alerts"), RemovedCount);
	}
}

void UFarmAlertSystem::ClearAllAlerts()
{
	int32 Count = ActiveAlerts.Num();
	ActiveAlerts.Empty();
	UE_LOG(LogTemp, Log, TEXT("FarmAlertSystem: Cleared all %d alerts"), Count);
}

// ============================================================================
// ALERT QUERIES
// ============================================================================

TArray<FFarmAlert> UFarmAlertSystem::GetActiveAlerts() const
{
	TArray<FFarmAlert> Result;
	for (const FFarmAlert& Alert : ActiveAlerts)
	{
		if (!Alert.bResolved)
		{
			Result.Add(Alert);
		}
	}
	return Result;
}

TArray<FFarmAlert> UFarmAlertSystem::GetAlertsByPriority(EAlertPriority Priority) const
{
	TArray<FFarmAlert> Result;
	for (const FFarmAlert& Alert : ActiveAlerts)
	{
		if (Alert.Priority == Priority && !Alert.bResolved)
		{
			Result.Add(Alert);
		}
	}
	return Result;
}

TArray<FFarmAlert> UFarmAlertSystem::GetAlertsByType(EFarmAlertType AlertType) const
{
	TArray<FFarmAlert> Result;
	for (const FFarmAlert& Alert : ActiveAlerts)
	{
		if (Alert.AlertType == AlertType && !Alert.bResolved)
		{
			Result.Add(Alert);
		}
	}
	return Result;
}

TArray<FFarmAlert> UFarmAlertSystem::GetAlertsForActor(AActor* Actor) const
{
	TArray<FFarmAlert> Result;
	if (!Actor)
	{
		return Result;
	}

	for (const FFarmAlert& Alert : ActiveAlerts)
	{
		if (Alert.SourceActor == Actor && !Alert.bResolved)
		{
			Result.Add(Alert);
		}
	}
	return Result;
}

TArray<FFarmAlert> UFarmAlertSystem::GetUnacknowledgedAlerts() const
{
	TArray<FFarmAlert> Result;
	for (const FFarmAlert& Alert : ActiveAlerts)
	{
		if (!Alert.bAcknowledged && !Alert.bResolved)
		{
			Result.Add(Alert);
		}
	}
	return Result;
}

FFarmAlert UFarmAlertSystem::GetAlert(int32 AlertID) const
{
	for (const FFarmAlert& Alert : ActiveAlerts)
	{
		if (Alert.AlertID == AlertID)
		{
			return Alert;
		}
	}

	return FFarmAlert();
}

bool UFarmAlertSystem::HasAlert(int32 AlertID) const
{
	for (const FFarmAlert& Alert : ActiveAlerts)
	{
		if (Alert.AlertID == AlertID)
		{
			return true;
		}
	}

	return false;
}

FAlertStatistics UFarmAlertSystem::GetAlertStatistics() const
{
	FAlertStatistics Stats;
	Stats.TotalAlerts = ActiveAlerts.Num();

	for (const FFarmAlert& Alert : ActiveAlerts)
	{
		if (!Alert.bResolved)
		{
			Stats.ActiveAlerts++;

			if (!Alert.bAcknowledged)
			{
				Stats.UnacknowledgedAlerts++;
			}

			if (Alert.Priority == EAlertPriority::Critical)
			{
				Stats.CriticalAlerts++;
			}
			else if (Alert.Priority == EAlertPriority::High)
			{
				Stats.HighPriorityAlerts++;
			}
		}
	}

	return Stats;
}

// ============================================================================
// ALERT CONFIGURATION
// ============================================================================

FAlertConfiguration UFarmAlertSystem::GetAlertConfiguration(EFarmAlertType AlertType) const
{
	const FAlertConfiguration* Config = AlertConfigurations.Find(AlertType);
	if (Config)
	{
		return *Config;
	}

	// Return default configuration
	FAlertConfiguration DefaultConfig;
	DefaultConfig.AlertType = AlertType;
	return DefaultConfig;
}

void UFarmAlertSystem::SetAlertConfiguration(EFarmAlertType AlertType, const FAlertConfiguration& Config)
{
	AlertConfigurations.Add(AlertType, Config);
	UE_LOG(LogTemp, Log, TEXT("FarmAlertSystem: Updated configuration for alert type %d"), (int32)AlertType);
}

void UFarmAlertSystem::SetAlertEnabled(EFarmAlertType AlertType, bool bEnabled)
{
	FAlertConfiguration Config = GetAlertConfiguration(AlertType);
	Config.bEnabled = bEnabled;
	SetAlertConfiguration(AlertType, Config);
}

void UFarmAlertSystem::SetAlertSoundEnabled(EFarmAlertType AlertType, bool bEnabled)
{
	FAlertConfiguration Config = GetAlertConfiguration(AlertType);
	Config.bPlaySound = bEnabled;
	SetAlertConfiguration(AlertType, Config);
}

void UFarmAlertSystem::SetMasterAlertVolume(float Volume)
{
	MasterAlertVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	UE_LOG(LogTemp, Log, TEXT("FarmAlertSystem: Set master volume to %.2f"), MasterAlertVolume);
}

// ============================================================================
// FARM MONITORING
// ============================================================================

void UFarmAlertSystem::RegisterFarmPlot(AFarmPlot* FarmPlot)
{
	if (!FarmPlot)
	{
		return;
	}

	if (!RegisteredFarms.Contains(FarmPlot))
	{
		RegisteredFarms.Add(FarmPlot);
		UE_LOG(LogTemp, Log, TEXT("FarmAlertSystem: Registered farm plot %s"), *FarmPlot->GetName());
	}
}

void UFarmAlertSystem::UnregisterFarmPlot(AFarmPlot* FarmPlot)
{
	if (!FarmPlot)
	{
		return;
	}

	if (RegisteredFarms.Remove(FarmPlot) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("FarmAlertSystem: Unregistered farm plot %s"), *FarmPlot->GetName());
	}
}

void UFarmAlertSystem::CheckAllFarms()
{
	for (AFarmPlot* Farm : RegisteredFarms)
	{
		if (Farm)
		{
			CheckFarm(Farm);
		}
	}

	// Auto-clear resolved alerts periodically
	ClearResolvedAlerts();
}

void UFarmAlertSystem::CheckFarm(AFarmPlot* FarmPlot)
{
	if (!FarmPlot)
	{
		return;
	}

	CheckFarmWaterNeeds(FarmPlot);
	CheckFarmSoilQuality(FarmPlot);
	CheckFarmFertility(FarmPlot);
	CheckFarmNutrients(FarmPlot);
	CheckFarmCropHealth(FarmPlot);
	CheckFarmHarvestReady(FarmPlot);
}

// ============================================================================
// INTERNAL METHODS
// ============================================================================

void UFarmAlertSystem::InitializeAlertConfigurations()
{
	// Water Needed Alert
	{
		FAlertConfiguration Config;
		Config.AlertType = EFarmAlertType::WaterNeeded;
		Config.DefaultPriority = EAlertPriority::High;
		Config.bEnabled = true;
		Config.bPlaySound = true;
		Config.bShowVisualNotification = true;
		Config.AlertColor = FLinearColor(0.2f, 0.5f, 1.0f); // Blue
		Config.CooldownSeconds = 120.0f; // 2 minutes
		AlertConfigurations.Add(Config.AlertType, Config);
	}

	// Low Soil Quality Alert
	{
		FAlertConfiguration Config;
		Config.AlertType = EFarmAlertType::LowSoilQuality;
		Config.DefaultPriority = EAlertPriority::Medium;
		Config.bEnabled = true;
		Config.bPlaySound = true;
		Config.bShowVisualNotification = true;
		Config.AlertColor = FLinearColor(0.6f, 0.4f, 0.2f); // Brown
		Config.CooldownSeconds = 300.0f; // 5 minutes
		AlertConfigurations.Add(Config.AlertType, Config);
	}

	// Low Fertility Alert
	{
		FAlertConfiguration Config;
		Config.AlertType = EFarmAlertType::LowFertility;
		Config.DefaultPriority = EAlertPriority::Medium;
		Config.bEnabled = true;
		Config.bPlaySound = true;
		Config.bShowVisualNotification = true;
		Config.AlertColor = FLinearColor(0.8f, 0.6f, 0.2f); // Orange
		Config.CooldownSeconds = 300.0f; // 5 minutes
		AlertConfigurations.Add(Config.AlertType, Config);
	}

	// Low Nutrients Alert
	{
		FAlertConfiguration Config;
		Config.AlertType = EFarmAlertType::LowNutrients;
		Config.DefaultPriority = EAlertPriority::Medium;
		Config.bEnabled = true;
		Config.bPlaySound = true;
		Config.bShowVisualNotification = true;
		Config.AlertColor = FLinearColor(0.9f, 0.7f, 0.3f); // Yellow-Orange
		Config.CooldownSeconds = 300.0f; // 5 minutes
		AlertConfigurations.Add(Config.AlertType, Config);
	}

	// Crop Unhealthy Alert
	{
		FAlertConfiguration Config;
		Config.AlertType = EFarmAlertType::CropUnhealthy;
		Config.DefaultPriority = EAlertPriority::High;
		Config.bEnabled = true;
		Config.bPlaySound = true;
		Config.bShowVisualNotification = true;
		Config.AlertColor = FLinearColor(1.0f, 0.3f, 0.3f); // Red
		Config.CooldownSeconds = 180.0f; // 3 minutes
		AlertConfigurations.Add(Config.AlertType, Config);
	}

	// Maintenance Required Alert
	{
		FAlertConfiguration Config;
		Config.AlertType = EFarmAlertType::MaintenanceRequired;
		Config.DefaultPriority = EAlertPriority::Medium;
		Config.bEnabled = true;
		Config.bPlaySound = true;
		Config.bShowVisualNotification = true;
		Config.AlertColor = FLinearColor(1.0f, 0.8f, 0.0f); // Yellow
		Config.CooldownSeconds = 600.0f; // 10 minutes
		AlertConfigurations.Add(Config.AlertType, Config);
	}

	// Power Low Alert
	{
		FAlertConfiguration Config;
		Config.AlertType = EFarmAlertType::PowerLow;
		Config.DefaultPriority = EAlertPriority::High;
		Config.bEnabled = true;
		Config.bPlaySound = true;
		Config.bShowVisualNotification = true;
		Config.AlertColor = FLinearColor(1.0f, 0.5f, 0.0f); // Orange
		Config.CooldownSeconds = 180.0f; // 3 minutes
		AlertConfigurations.Add(Config.AlertType, Config);
	}

	// Harvest Ready Alert
	{
		FAlertConfiguration Config;
		Config.AlertType = EFarmAlertType::HarvestReady;
		Config.DefaultPriority = EAlertPriority::Low;
		Config.bEnabled = true;
		Config.bPlaySound = true;
		Config.bShowVisualNotification = true;
		Config.AlertColor = FLinearColor(0.0f, 1.0f, 0.5f); // Green
		Config.CooldownSeconds = 600.0f; // 10 minutes
		AlertConfigurations.Add(Config.AlertType, Config);
	}

	// System Malfunction Alert
	{
		FAlertConfiguration Config;
		Config.AlertType = EFarmAlertType::SystemMalfunction;
		Config.DefaultPriority = EAlertPriority::Critical;
		Config.bEnabled = true;
		Config.bPlaySound = true;
		Config.bShowVisualNotification = true;
		Config.AlertColor = FLinearColor(1.0f, 0.0f, 0.0f); // Bright Red
		Config.CooldownSeconds = 60.0f; // 1 minute
		AlertConfigurations.Add(Config.AlertType, Config);
	}
}

bool UFarmAlertSystem::IsAlertOnCooldown(EFarmAlertType AlertType, AActor* SourceActor) const
{
	FString Key = GetCooldownKey(AlertType, SourceActor);
	const FDateTime* CooldownEnd = AlertCooldowns.Find(Key);

	if (CooldownEnd)
	{
		return FDateTime::Now() < *CooldownEnd;
	}

	return false;
}

void UFarmAlertSystem::SetAlertCooldown(EFarmAlertType AlertType, AActor* SourceActor, float CooldownSeconds)
{
	FString Key = GetCooldownKey(AlertType, SourceActor);
	FDateTime CooldownEnd = FDateTime::Now() + FTimespan::FromSeconds(CooldownSeconds);
	AlertCooldowns.Add(Key, CooldownEnd);
}

FString UFarmAlertSystem::GetCooldownKey(EFarmAlertType AlertType, AActor* SourceActor) const
{
	FString ActorName = SourceActor ? SourceActor->GetName() : TEXT("Global");
	return FString::Printf(TEXT("%d_%s"), (int32)AlertType, *ActorName);
}

void UFarmAlertSystem::PlayAlertSound(const FAlertConfiguration& Config, const FVector& Location)
{
	if (!Config.AlertSound)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Play sound at location with master volume
	UGameplayStatics::PlaySoundAtLocation(
		World,
		Config.AlertSound,
		Location,
		MasterAlertVolume,
		1.0f, // Pitch
		0.0f, // Start time
		nullptr, // Attenuation
		nullptr, // Concurrency
		nullptr  // Owner
	);
}

void UFarmAlertSystem::ShowVisualNotification(const FFarmAlert& Alert, const FAlertConfiguration& Config)
{
	// Visual notification will be handled by UI widgets
	// This is a placeholder for future implementation
	UE_LOG(LogTemp, Display, TEXT("FarmAlertSystem: Visual notification - %s"), *Alert.Message);
}

void UFarmAlertSystem::CheckFarmWaterNeeds(AFarmPlot* FarmPlot)
{
	if (!FarmPlot)
	{
		return;
	}

	// Count crops needing water
	int32 CropsNeedingWater = 0;
	int32 TotalCrops = 0;

	for (const FCropCell& Cell : FarmPlot->CropGrid)
	{
		if (Cell.CropType)
		{
			TotalCrops++;
			if (Cell.bNeedsWater)
			{
				CropsNeedingWater++;
			}
		}
	}

	// Trigger alert if significant portion needs water
	if (TotalCrops > 0)
	{
		float WaterNeedPercentage = (float)CropsNeedingWater / TotalCrops;

		if (WaterNeedPercentage >= 0.5f) // 50% or more need water
		{
			FString Message = FString::Printf(
				TEXT("%d crops need watering (%d%%)"),
				CropsNeedingWater,
				FMath::RoundToInt(WaterNeedPercentage * 100.0f)
			);

			TriggerAlert(
				EFarmAlertType::WaterNeeded,
				FarmPlot,
				Message,
				TEXT("Crops are running low on water and need irrigation.")
			);
		}
	}

	// Also check water level
	if (FarmPlot->WaterLevel < 0.2f) // Below 20%
	{
		FString Message = FString::Printf(
			TEXT("Farm water level low: %d%%"),
			FMath::RoundToInt(FarmPlot->WaterLevel * 100.0f)
		);

		TriggerAlert(
			EFarmAlertType::WaterNeeded,
			FarmPlot,
			Message,
			TEXT("Farm plot water reserves are running low.")
		);
	}
}

void UFarmAlertSystem::CheckFarmSoilQuality(AFarmPlot* FarmPlot)
{
	if (!FarmPlot)
	{
		return;
	}

	float SoilQuality = FarmPlot->CalculateSoilQuality();

	if (SoilQuality < 0.3f) // Below 30%
	{
		FString Message = FString::Printf(
			TEXT("Soil quality low: %d%%"),
			FMath::RoundToInt(SoilQuality * 100.0f)
		);

		TriggerAlert(
			EFarmAlertType::LowSoilQuality,
			FarmPlot,
			Message,
			TEXT("Soil quality has degraded and may affect crop growth.")
		);
	}
}

void UFarmAlertSystem::CheckFarmFertility(AFarmPlot* FarmPlot)
{
	if (!FarmPlot)
	{
		return;
	}

	if (FarmPlot->Fertility < 0.3f) // Below 30%
	{
		FString Message = FString::Printf(
			TEXT("Fertility low: %d%%"),
			FMath::RoundToInt(FarmPlot->Fertility * 100.0f)
		);

		TriggerAlert(
			EFarmAlertType::LowFertility,
			FarmPlot,
			Message,
			TEXT("Soil fertility is depleted. Consider fertilizing the plot.")
		);
	}
}

void UFarmAlertSystem::CheckFarmNutrients(AFarmPlot* FarmPlot)
{
	if (!FarmPlot)
	{
		return;
	}

	// Check each nutrient
	TArray<FString> LowNutrients;

	if (FarmPlot->NitrogenLevel < 0.3f)
	{
		LowNutrients.Add(FString::Printf(TEXT("Nitrogen: %d%%"), FMath::RoundToInt(FarmPlot->NitrogenLevel * 100.0f)));
	}

	if (FarmPlot->PhosphorusLevel < 0.3f)
	{
		LowNutrients.Add(FString::Printf(TEXT("Phosphorus: %d%%"), FMath::RoundToInt(FarmPlot->PhosphorusLevel * 100.0f)));
	}

	if (FarmPlot->PotassiumLevel < 0.3f)
	{
		LowNutrients.Add(FString::Printf(TEXT("Potassium: %d%%"), FMath::RoundToInt(FarmPlot->PotassiumLevel * 100.0f)));
	}

	if (LowNutrients.Num() > 0)
	{
		FString Message = FString::Printf(TEXT("Low nutrients: %s"), *FString::Join(LowNutrients, TEXT(", ")));

		TriggerAlert(
			EFarmAlertType::LowNutrients,
			FarmPlot,
			Message,
			TEXT("Nutrient levels are low. Consider applying fertilizer.")
		);
	}
}

void UFarmAlertSystem::CheckFarmCropHealth(AFarmPlot* FarmPlot)
{
	if (!FarmPlot)
	{
		return;
	}

	// Count unhealthy crops
	int32 UnhealthyCrops = 0;
	int32 TotalCrops = 0;

	for (const FCropCell& Cell : FarmPlot->CropGrid)
	{
		if (Cell.CropType)
		{
			TotalCrops++;
			if (Cell.Health < 0.5f) // Below 50% health
			{
				UnhealthyCrops++;
			}
		}
	}

	if (TotalCrops > 0 && UnhealthyCrops > 0)
	{
		float UnhealthyPercentage = (float)UnhealthyCrops / TotalCrops;

		if (UnhealthyPercentage >= 0.3f) // 30% or more unhealthy
		{
			FString Message = FString::Printf(
				TEXT("%d crops unhealthy (%d%%)"),
				UnhealthyCrops,
				FMath::RoundToInt(UnhealthyPercentage * 100.0f)
			);

			TriggerAlert(
				EFarmAlertType::CropUnhealthy,
				FarmPlot,
				Message,
				TEXT("Multiple crops are in poor health. Check water and nutrient levels.")
			);
		}
	}
}

void UFarmAlertSystem::CheckFarmHarvestReady(AFarmPlot* FarmPlot)
{
	if (!FarmPlot)
	{
		return;
	}

	// Count mature crops
	int32 MatureCrops = 0;

	for (const FCropCell& Cell : FarmPlot->CropGrid)
	{
		if (Cell.CropType && Cell.GrowthProgress >= 1.0f)
		{
			MatureCrops++;
		}
	}

	if (MatureCrops >= 5) // At least 5 crops ready
	{
		FString Message = FString::Printf(
			TEXT("%d crops ready to harvest"),
			MatureCrops
		);

		TriggerAlert(
			EFarmAlertType::HarvestReady,
			FarmPlot,
			Message,
			TEXT("Crops have reached maturity and are ready for harvest.")
		);
	}
}

FLinearColor UFarmAlertSystem::GetPriorityColor(EAlertPriority Priority) const
{
	switch (Priority)
	{
	case EAlertPriority::Low:
		return FLinearColor(0.5f, 0.5f, 1.0f); // Light Blue
	case EAlertPriority::Medium:
		return FLinearColor(1.0f, 1.0f, 0.0f); // Yellow
	case EAlertPriority::High:
		return FLinearColor(1.0f, 0.5f, 0.0f); // Orange
	case EAlertPriority::Critical:
		return FLinearColor(1.0f, 0.0f, 0.0f); // Red
	default:
		return FLinearColor::White;
	}
}

FString UFarmAlertSystem::GetAlertTypeName(EFarmAlertType AlertType) const
{
	switch (AlertType)
	{
	case EFarmAlertType::WaterNeeded:
		return TEXT("Water Needed");
	case EFarmAlertType::LowSoilQuality:
		return TEXT("Low Soil Quality");
	case EFarmAlertType::LowFertility:
		return TEXT("Low Fertility");
	case EFarmAlertType::LowNutrients:
		return TEXT("Low Nutrients");
	case EFarmAlertType::CropUnhealthy:
		return TEXT("Crop Unhealthy");
	case EFarmAlertType::MaintenanceRequired:
		return TEXT("Maintenance Required");
	case EFarmAlertType::PowerLow:
		return TEXT("Power Low");
	case EFarmAlertType::HarvestReady:
		return TEXT("Harvest Ready");
	case EFarmAlertType::SystemMalfunction:
		return TEXT("System Malfunction");
	default:
		return TEXT("Unknown");
	}
}
