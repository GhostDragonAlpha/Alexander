# Farm Alert System - Quick Start Guide

## Overview

The Farm Alert System provides automated notifications for farm conditions requiring player attention. It monitors water needs, soil quality, crop health, and automation systems, triggering visual and audio alerts based on configurable priority levels.

## Key Components

### 1. UFarmAlertSystem (World Subsystem)

- Manages all farm alerts
- Monitors registered farm plots
- Triggers notifications based on conditions
- Handles alert lifecycle (create, acknowledge, resolve, dismiss)

### 2. Alert Types

- **WaterNeeded**: Crops need watering or water level low
- **LowSoilQuality**: Soil quality below threshold
- **LowFertility**: Fertility depleted
- **LowNutrients**: NPK levels low
- **CropUnhealthy**: Crop health below threshold
- **MaintenanceRequired**: Automation device needs maintenance
- **PowerLow**: Power system running low
- **HarvestReady**: Crops ready to harvest
- **SystemMalfunction**: Automation system malfunction

### 3. Priority Levels

- **Critical**: Immediate action required (red)
- **High**: Requires attention (orange)
- **Medium**: Should be addressed soon (yellow)
- **Low**: Informational (light blue)

## Basic Usage

### Register a Farm Plot for Monitoring

```cpp
// In your farm setup code
UFarmAlertSystem* AlertSystem = GetWorld()->GetSubsystem<UFarmAlertSystem>();
if (AlertSystem)
{
    AlertSystem->RegisterFarmPlot(MyFarmPlot);
}
```

### Manually Trigger an Alert

```cpp
UFarmAlertSystem* AlertSystem = GetWorld()->GetSubsystem<UFarmAlertSystem>();
if (AlertSystem)
{
    int32 AlertID = AlertSystem->TriggerAlert(
        EFarmAlertType::WaterNeeded,
        FarmPlotActor,
        TEXT("5 crops need watering"),
        TEXT("Crops are running low on water and need irrigation.")
    );
}
```

### Query Active Alerts

```cpp
// Get all active alerts
TArray<FFarmAlert> ActiveAlerts = AlertSystem->GetActiveAlerts();

// Get alerts by priority
TArray<FFarmAlert> CriticalAlerts = AlertSystem->GetAlertsByPriority(EAlertPriority::Critical);

// Get alerts for specific farm
TArray<FFarmAlert> FarmAlerts = AlertSystem->GetAlertsForActor(MyFarmPlot);

// Get unacknowledged alerts
TArray<FFarmAlert> NewAlerts = AlertSystem->GetUnacknowledgedAlerts();
```

### Acknowledge and Resolve Alerts

```cpp
// Acknowledge an alert (mark as seen)
AlertSystem->AcknowledgeAlert(AlertID);

// Resolve an alert (mark as fixed)
AlertSystem->ResolveAlert(AlertID);

// Dismiss an alert (remove from list)
AlertSystem->DismissAlert(AlertID);

// Clear all resolved alerts
AlertSystem->ClearResolvedAlerts();
```

## Alert Configuration

### Customize Alert Behavior

```cpp
// Get current configuration
FAlertConfiguration Config = AlertSystem->GetAlertConfiguration(EFarmAlertType::WaterNeeded);

// Modify configuration
Config.bPlaySound = false; // Disable sound
Config.CooldownSeconds = 300.0f; // 5 minute cooldown
Config.AlertColor = FLinearColor::Blue;

// Apply configuration
AlertSystem->SetAlertConfiguration(EFarmAlertType::WaterNeeded, Config);
```

### Enable/Disable Alert Types

```cpp
// Disable water alerts
AlertSystem->SetAlertEnabled(EFarmAlertType::WaterNeeded, false);

// Disable sound for specific alert type
AlertSystem->SetAlertSoundEnabled(EFarmAlertType::HarvestReady, false);

// Set master volume for all alerts
AlertSystem->SetMasterAlertVolume(0.5f); // 50% volume
```

## Automatic Monitoring

The alert system automatically checks registered farms every 5 seconds for:

### Water Needs

- Triggers when 50%+ of crops need water
- Triggers when farm water level < 20%
- Priority: High
- Cooldown: 2 minutes

### Soil Quality

- Triggers when soil quality < 30%
- Priority: Medium
- Cooldown: 5 minutes

### Fertility

- Triggers when fertility < 30%
- Priority: Medium
- Cooldown: 5 minutes

### Nutrients

- Triggers when N, P, or K < 30%
- Priority: Medium
- Cooldown: 5 minutes

### Crop Health

- Triggers when 30%+ of crops have health < 50%
- Priority: High
- Cooldown: 3 minutes

### Harvest Ready

- Triggers when 5+ crops are mature
- Priority: Low
- Cooldown: 10 minutes

## UI Integration

### Display Alerts in Widget

```cpp
// In your widget class
void UMyFarmWidget::UpdateAlerts()
{
    UFarmAlertSystem* AlertSystem = GetWorld()->GetSubsystem<UFarmAlertSystem>();
    if (!AlertSystem || !FarmPlot)
    {
        return;
    }

    // Get alerts for this farm
    TArray<FFarmAlert> Alerts = AlertSystem->GetAlertsForActor(FarmPlot);

    // Display alerts in UI
    for (const FFarmAlert& Alert : Alerts)
    {
        // Create UI element for alert
        AddAlertToUI(Alert);
    }
}
```

### Handle Alert Events

```cpp
// Bind to alert events
void UMyFarmWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UFarmAlertSystem* AlertSystem = GetWorld()->GetSubsystem<UFarmAlertSystem>();
    if (AlertSystem)
    {
        AlertSystem->OnAlertTriggered.AddDynamic(this, &UMyFarmWidget::OnAlertTriggered);
        AlertSystem->OnAlertAcknowledged.AddDynamic(this, &UMyFarmWidget::OnAlertAcknowledged);
        AlertSystem->OnAlertResolved.AddDynamic(this, &UMyFarmWidget::OnAlertResolved);
    }
}

void UMyFarmWidget::OnAlertTriggered(const FFarmAlert& Alert)
{
    // Handle new alert
    UE_LOG(LogTemp, Warning, TEXT("New alert: %s"), *Alert.Message);
}
```

## Alert Statistics

```cpp
// Get alert statistics
FAlertStatistics Stats = AlertSystem->GetAlertStatistics();

UE_LOG(LogTemp, Log, TEXT("Total Alerts: %d"), Stats.TotalAlerts);
UE_LOG(LogTemp, Log, TEXT("Active Alerts: %d"), Stats.ActiveAlerts);
UE_LOG(LogTemp, Log, TEXT("Critical Alerts: %d"), Stats.CriticalAlerts);
UE_LOG(LogTemp, Log, TEXT("Unacknowledged: %d"), Stats.UnacknowledgedAlerts);
```

## Blueprint Usage

All alert system functions are Blueprint-callable:

```
// Get alert system
Get World Subsystem -> Farm Alert System

// Register farm
Register Farm Plot (Farm Plot)

// Get active alerts
Get Active Alerts -> For Each Loop

// Acknowledge alert
Acknowledge Alert (Alert ID)
```

## Best Practices

1. **Register farms early**: Register farm plots in BeginPlay or after creation
2. **Use cooldowns**: Prevent alert spam by using appropriate cooldown times
3. **Prioritize correctly**: Use Critical only for urgent issues
4. **Acknowledge alerts**: Mark alerts as acknowledged when player sees them
5. **Resolve alerts**: Mark alerts as resolved when issue is fixed
6. **Clear periodically**: Call ClearResolvedAlerts() to clean up old alerts
7. **Customize sounds**: Assign appropriate sound assets for each alert type
8. **Test thresholds**: Adjust monitoring thresholds based on gameplay balance

## Performance Considerations

- Monitoring runs every 5 seconds (configurable)
- Cooldowns prevent duplicate alerts
- Resolved alerts are auto-cleared
- Maximum of 1000 active alerts (soft limit)
- Alert checks are lightweight (< 1ms per farm)

## Troubleshooting

### Alerts Not Triggering

- Verify farm is registered: `AlertSystem->RegisterFarmPlot(Farm)`
- Check alert is enabled: `AlertSystem->GetAlertConfiguration(Type).bEnabled`
- Check cooldown hasn't expired yet
- Verify monitoring conditions are met

### Too Many Alerts

- Increase cooldown times
- Adjust threshold values
- Disable less important alert types
- Implement alert grouping in UI

### Sounds Not Playing

- Verify sound asset is assigned in configuration
- Check master volume: `AlertSystem->SetMasterAlertVolume(1.0f)`
- Verify sound is enabled: `Config.bPlaySound = true`
- Check audio settings in project

## Example: Complete Alert Setup

```cpp
void AMyFarmManager::SetupAlertSystem()
{
    UFarmAlertSystem* AlertSystem = GetWorld()->GetSubsystem<UFarmAlertSystem>();
    if (!AlertSystem)
    {
        return;
    }

    // Register all farms
    for (AFarmPlot* Farm : MyFarms)
    {
        AlertSystem->RegisterFarmPlot(Farm);
    }

    // Customize water alert
    FAlertConfiguration WaterConfig = AlertSystem->GetAlertConfiguration(EFarmAlertType::WaterNeeded);
    WaterConfig.AlertSound = WaterAlertSound;
    WaterConfig.AlertIcon = WaterAlertIcon;
    WaterConfig.CooldownSeconds = 180.0f; // 3 minutes
    AlertSystem->SetAlertConfiguration(EFarmAlertType::WaterNeeded, WaterConfig);

    // Bind to events
    AlertSystem->OnAlertTriggered.AddDynamic(this, &AMyFarmManager::HandleNewAlert);

    // Set master volume
    AlertSystem->SetMasterAlertVolume(0.7f);
}

void AMyFarmManager::HandleNewAlert(const FFarmAlert& Alert)
{
    // Show notification to player
    if (Alert.Priority == EAlertPriority::Critical)
    {
        ShowCriticalAlertUI(Alert);
    }
    else
    {
        AddAlertToQueue(Alert);
    }
}
```

## Related Systems

- **FarmStatusWidget**: Displays alerts in farm UI
- **AutomationEfficiencySystem**: Triggers maintenance alerts
- **PowerSystem**: Triggers power low alerts
- **FarmPlot**: Source of crop and soil condition alerts

## Next Steps

1. Create custom alert sounds for each type
2. Design alert icons for visual notifications
3. Implement alert notification widget
4. Add alert history/log system
5. Create alert filtering and sorting UI
6. Implement alert priority escalation
7. Add alert grouping for multiple farms
