// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FarmAlertSystem.generated.h"

// Forward declarations
class AFarmPlot;
class USoundBase;
class UTexture2D;

/**
 * Alert type enumeration
 */
UENUM(BlueprintType)
enum class EFarmAlertType : uint8
{
	WaterNeeded,          // Crops need watering
	LowSoilQuality,       // Soil quality below threshold
	LowFertility,         // Fertility below threshold
	LowNutrients,         // NPK levels below threshold
	CropUnhealthy,        // Crop health below threshold
	MaintenanceRequired,  // Automation device needs maintenance
	PowerLow,             // Power system running low
	HarvestReady,         // Crops ready to harvest
	SystemMalfunction     // Automation system malfunction
};

/**
 * Alert priority levels
 */
UENUM(BlueprintType)
enum class EAlertPriority : uint8
{
	Low,       // Informational, no immediate action needed
	Medium,    // Should be addressed soon
	High,      // Requires attention
	Critical   // Immediate action required
};

/**
 * Alert notification data
 */
USTRUCT(BlueprintType)
struct FFarmAlert
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Alert")
	EFarmAlertType AlertType;

	UPROPERTY(BlueprintReadOnly, Category = "Alert")
	EAlertPriority Priority;

	UPROPERTY(BlueprintReadOnly, Category = "Alert")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "Alert")
	FString DetailedMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Alert")
	AActor* SourceActor;

	UPROPERTY(BlueprintReadOnly, Category = "Alert")
	FVector Location;

	UPROPERTY(BlueprintReadOnly, Category = "Alert")
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly, Category = "Alert")
	bool bAcknowledged;

	UPROPERTY(BlueprintReadOnly, Category = "Alert")
	bool bResolved;

	UPROPERTY(BlueprintReadOnly, Category = "Alert")
	int32 AlertID;

	FFarmAlert()
		: AlertType(EFarmAlertType::WaterNeeded)
		, Priority(EAlertPriority::Medium)
		, SourceActor(nullptr)
		, Location(FVector::ZeroVector)
		, Timestamp(FDateTime::Now())
		, bAcknowledged(false)
		, bResolved(false)
		, AlertID(0)
	{
	}
};

/**
 * Alert configuration for each alert type
 */
USTRUCT(BlueprintType)
struct FAlertConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert Config")
	EFarmAlertType AlertType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert Config")
	EAlertPriority DefaultPriority;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert Config")
	bool bEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert Config")
	bool bPlaySound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert Config")
	bool bShowVisualNotification;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert Config")
	USoundBase* AlertSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert Config")
	UTexture2D* AlertIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert Config")
	FLinearColor AlertColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert Config")
	float CooldownSeconds;

	FAlertConfiguration()
		: AlertType(EFarmAlertType::WaterNeeded)
		, DefaultPriority(EAlertPriority::Medium)
		, bEnabled(true)
		, bPlaySound(true)
		, bShowVisualNotification(true)
		, AlertSound(nullptr)
		, AlertIcon(nullptr)
		, AlertColor(FLinearColor::Yellow)
		, CooldownSeconds(60.0f)
	{
	}
};

/**
 * Alert statistics
 */
USTRUCT(BlueprintType)
struct FAlertStatistics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 TotalAlerts;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 ActiveAlerts;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 CriticalAlerts;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 HighPriorityAlerts;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 UnacknowledgedAlerts;

	FAlertStatistics()
		: TotalAlerts(0)
		, ActiveAlerts(0)
		, CriticalAlerts(0)
		, HighPriorityAlerts(0)
		, UnacknowledgedAlerts(0)
	{
	}
};

/**
 * Farm Alert System
 * Manages notifications for farm conditions requiring player attention
 * Implements water need alerts, visual notifications, audio alerts, and priority system
 */
UCLASS()
class ALEXANDER_API UFarmAlertSystem : public UWorldSubsystem
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
	// ALERT MANAGEMENT
	// ============================================================================

	/**
	 * Create and trigger a new alert
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts")
	int32 TriggerAlert(
		EFarmAlertType AlertType,
		AActor* SourceActor,
		const FString& Message,
		const FString& DetailedMessage = TEXT(""),
		EAlertPriority OverridePriority = EAlertPriority::Medium,
		bool bUseOverridePriority = false
	);

	/**
	 * Acknowledge an alert (mark as seen)
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts")
	bool AcknowledgeAlert(int32 AlertID);

	/**
	 * Resolve an alert (mark as fixed)
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts")
	bool ResolveAlert(int32 AlertID);

	/**
	 * Dismiss an alert (remove from active list)
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts")
	bool DismissAlert(int32 AlertID);

	/**
	 * Clear all resolved alerts
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts")
	void ClearResolvedAlerts();

	/**
	 * Clear all alerts
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts")
	void ClearAllAlerts();

	// ============================================================================
	// ALERT QUERIES
	// ============================================================================

	/**
	 * Get all active alerts
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts")
	TArray<FFarmAlert> GetActiveAlerts() const;

	/**
	 * Get alerts by priority
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts")
	TArray<FFarmAlert> GetAlertsByPriority(EAlertPriority Priority) const;

	/**
	 * Get alerts by type
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts")
	TArray<FFarmAlert> GetAlertsByType(EFarmAlertType AlertType) const;

	/**
	 * Get alerts for specific actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts")
	TArray<FFarmAlert> GetAlertsForActor(AActor* Actor) const;

	/**
	 * Get unacknowledged alerts
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts")
	TArray<FFarmAlert> GetUnacknowledgedAlerts() const;

	/**
	 * Get alert by ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts")
	FFarmAlert GetAlert(int32 AlertID) const;

	/**
	 * Check if alert exists
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts")
	bool HasAlert(int32 AlertID) const;

	/**
	 * Get alert statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts")
	FAlertStatistics GetAlertStatistics() const;

	// ============================================================================
	// ALERT CONFIGURATION
	// ============================================================================

	/**
	 * Get configuration for alert type
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts|Config")
	FAlertConfiguration GetAlertConfiguration(EFarmAlertType AlertType) const;

	/**
	 * Set configuration for alert type
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts|Config")
	void SetAlertConfiguration(EFarmAlertType AlertType, const FAlertConfiguration& Config);

	/**
	 * Enable/disable alert type
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts|Config")
	void SetAlertEnabled(EFarmAlertType AlertType, bool bEnabled);

	/**
	 * Enable/disable sound for alert type
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts|Config")
	void SetAlertSoundEnabled(EFarmAlertType AlertType, bool bEnabled);

	/**
	 * Set master volume for alert sounds
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts|Config")
	void SetMasterAlertVolume(float Volume);

	// ============================================================================
	// FARM MONITORING
	// ============================================================================

	/**
	 * Register a farm plot for monitoring
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts|Monitoring")
	void RegisterFarmPlot(AFarmPlot* FarmPlot);

	/**
	 * Unregister a farm plot from monitoring
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts|Monitoring")
	void UnregisterFarmPlot(AFarmPlot* FarmPlot);

	/**
	 * Check all registered farms for alert conditions
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts|Monitoring")
	void CheckAllFarms();

	/**
	 * Check specific farm for alert conditions
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Alerts|Monitoring")
	void CheckFarm(AFarmPlot* FarmPlot);

	// ============================================================================
	// DELEGATES
	// ============================================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAlertTriggered, const FFarmAlert&, Alert);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAlertAcknowledged, int32, AlertID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAlertResolved, int32, AlertID);

	UPROPERTY(BlueprintAssignable, Category = "Farm Alerts|Events")
	FOnAlertTriggered OnAlertTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Farm Alerts|Events")
	FOnAlertAcknowledged OnAlertAcknowledged;

	UPROPERTY(BlueprintAssignable, Category = "Farm Alerts|Events")
	FOnAlertResolved OnAlertResolved;

protected:
	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	// Active alerts
	UPROPERTY()
	TArray<FFarmAlert> ActiveAlerts;

	// Alert configurations
	UPROPERTY()
	TMap<EFarmAlertType, FAlertConfiguration> AlertConfigurations;

	// Registered farm plots
	UPROPERTY()
	TArray<AFarmPlot*> RegisteredFarms;

	// Alert cooldowns (prevents spam)
	TMap<FString, FDateTime> AlertCooldowns;

	// Next alert ID
	int32 NextAlertID;

	// Master alert volume
	float MasterAlertVolume;

	// Update timer
	FTimerHandle MonitoringTimerHandle;

	// Monitoring interval
	float MonitoringInterval;

	// ============================================================================
	// INTERNAL METHODS
	// ============================================================================

	/**
	 * Initialize default alert configurations
	 */
	void InitializeAlertConfigurations();

	/**
	 * Check if alert is on cooldown
	 */
	bool IsAlertOnCooldown(EFarmAlertType AlertType, AActor* SourceActor) const;

	/**
	 * Set alert cooldown
	 */
	void SetAlertCooldown(EFarmAlertType AlertType, AActor* SourceActor, float CooldownSeconds);

	/**
	 * Get cooldown key for alert
	 */
	FString GetCooldownKey(EFarmAlertType AlertType, AActor* SourceActor) const;

	/**
	 * Play alert sound
	 */
	void PlayAlertSound(const FAlertConfiguration& Config, const FVector& Location);

	/**
	 * Show visual notification
	 */
	void ShowVisualNotification(const FFarmAlert& Alert, const FAlertConfiguration& Config);

	/**
	 * Check farm for water needs
	 */
	void CheckFarmWaterNeeds(AFarmPlot* FarmPlot);

	/**
	 * Check farm for soil quality
	 */
	void CheckFarmSoilQuality(AFarmPlot* FarmPlot);

	/**
	 * Check farm for fertility
	 */
	void CheckFarmFertility(AFarmPlot* FarmPlot);

	/**
	 * Check farm for nutrient levels
	 */
	void CheckFarmNutrients(AFarmPlot* FarmPlot);

	/**
	 * Check farm for crop health
	 */
	void CheckFarmCropHealth(AFarmPlot* FarmPlot);

	/**
	 * Check farm for harvest ready crops
	 */
	void CheckFarmHarvestReady(AFarmPlot* FarmPlot);

	/**
	 * Get priority color
	 */
	FLinearColor GetPriorityColor(EAlertPriority Priority) const;

	/**
	 * Get alert type name
	 */
	FString GetAlertTypeName(EFarmAlertType AlertType) const;
};
