# Quick Start: Automation Efficiency System

## Overview

The Automation Efficiency System tracks and manages the performance of automated farming equipment, implementing an 80% manual labor reduction through automation. It provides status monitoring, maintenance requirements, and an upgrade system for all automation devices.

## Key Features

- **80% Manual Labor Reduction**: Automated systems reduce manual farming work by 80%
- **Status Monitoring**: Real-time tracking of device operational status
- **Maintenance System**: Periodic maintenance requirements to maintain efficiency
- **Upgrade System**: Improve device performance through upgrades
- **Efficiency Tracking**: Monitor overall automation efficiency

## Quick Setup

### 1. Register Automation Devices

```cpp
// Get the automation efficiency system
UAutomationEfficiencySystem* AutoSystem = GetWorld()->GetSubsystem<UAutomationEfficiencySystem>();

// Register an irrigation system
AIrrigationSystem* Irrigation = GetWorld()->SpawnActor<AIrrigationSystem>(...);
AutoSystem->RegisterDevice(Irrigation, EAutomationDeviceType::IrrigationSystem);

// Register a harvesting machine
AHarvestingMachine* Harvester = GetWorld()->SpawnActor<AHarvestingMachine>(...);
AutoSystem->RegisterDevice(Harvester, EAutomationDeviceType::HarvestingMachine);
```

### 2. Monitor Device Status

```cpp
// Get device statistics
FAutomationDeviceStats Stats = AutoSystem->GetDeviceStats(Irrigation);

UE_LOG(LogTemp, Log, TEXT("Efficiency: %.1f%%"), Stats.EfficiencyPercent);
UE_LOG(LogTemp, Log, TEXT("Operational Hours: %.1f"), Stats.OperationalHours);
UE_LOG(LogTemp, Log, TEXT("Maintenance Progress: %.1f%%"), Stats.MaintenanceProgress * 100.0f);

// Check if maintenance is needed
if (AutoSystem->NeedsMaintenance(Irrigation))
{
    UE_LOG(LogTemp, Warning, TEXT("Device needs maintenance!"));
}
```

### 3. Perform Maintenance

```cpp
// Get maintenance requirements
TArray<FMaintenanceRequirement> Requirements = AutoSystem->GetMaintenanceRequirements(Irrigation);

for (const FMaintenanceRequirement& Req : Requirements)
{
    UE_LOG(LogTemp, Log, TEXT("Required: %s x%d (%s)"),
        *Req.ItemRequired, Req.QuantityRequired, *Req.RequirementName);
}

// Perform maintenance (after gathering required items)
if (AutoSystem->PerformMaintenance(Irrigation))
{
    UE_LOG(LogTemp, Log, TEXT("Maintenance completed!"));
}
```

### 4. Apply Upgrades

```cpp
// Get available upgrades
TArray<FAutomationUpgrade> Upgrades = AutoSystem->GetAvailableUpgrades(Irrigation);

for (const FAutomationUpgrade& Upgrade : Upgrades)
{
    UE_LOG(LogTemp, Log, TEXT("Upgrade: %s - %s (Cost: %.0f)"),
        *Upgrade.UpgradeName, *Upgrade.Description, Upgrade.Cost);
}

// Apply an upgrade
if (Upgrades.Num() > 0)
{
    AutoSystem->ApplyUpgrade(Irrigation, Upgrades[0]);
}
```

## Device Types

### Irrigation System

- **Base Labor Reduction**: 80%
- **Maintenance Interval**: 100 hours
- **Available Upgrades**:
  - Water Efficiency I: -10% water consumption
  - Extended Range I: +20% watering radius
  - Large Tank I: +50% water capacity

### Harvesting Machine

- **Base Labor Reduction**: 80%
- **Maintenance Interval**: 100 hours
- **Available Upgrades**:
  - Fast Harvesting I: -25% harvest interval
  - Large Storage I: +100% storage capacity
  - Extended Reach I: +30% harvest radius

### Power Generator

- **Maintenance Interval**: 100 hours
- **Available Upgrades**:
  - Fuel Efficiency I: -15% fuel consumption
  - Large Fuel Tank I: +50% fuel capacity

### Solar Panel

- **Maintenance Interval**: 100 hours
- **Available Upgrades**:
  - High-Efficiency Cells I: +25% power output
  - Weather Resistant I: -50% maintenance frequency

## Status Types

### Operational

- Device is working normally
- Full efficiency (100%)
- No immediate action required

### Degraded

- Device needs maintenance soon
- Reduced efficiency (70%)
- Maintenance recommended

### Malfunction

- Device requires immediate maintenance
- No efficiency (0%)
- Device not functioning

### Offline

- Device has no power
- No efficiency (0%)
- Check power supply

## Monitoring Overall Automation

```cpp
// Get automation summary
int32 TotalDevices;
float AvgEfficiency;
float LaborReduction;
int32 NeedsMaintenance;

AutoSystem->GetAutomationSummary(TotalDevices, AvgEfficiency, LaborReduction, NeedsMaintenance);

UE_LOG(LogTemp, Log, TEXT("Total Devices: %d"), TotalDevices);
UE_LOG(LogTemp, Log, TEXT("Average Efficiency: %.1f%%"), AvgEfficiency);
UE_LOG(LogTemp, Log, TEXT("Manual Labor Reduction: %.1f%%"), LaborReduction);
UE_LOG(LogTemp, Log, TEXT("Devices Needing Maintenance: %d"), NeedsMaintenance);

// Get device status counts
int32 Operational, Degraded, Malfunction, Offline;
AutoSystem->GetDeviceStatusCounts(Operational, Degraded, Malfunction, Offline);

UE_LOG(LogTemp, Log, TEXT("Operational: %d, Degraded: %d, Malfunction: %d, Offline: %d"),
    Operational, Degraded, Malfunction, Offline);
```

## Maintenance Requirements

### Irrigation System

1. **Filter Replacement**

   - Item: Water Filter x1
   - Time: 30 seconds

2. **Nozzle Cleaning**
   - Item: Cleaning Solution x1
   - Time: 20 seconds

### Harvesting Machine

1. **Blade Sharpening**

   - Item: Sharpening Stone x1
   - Time: 45 seconds

2. **Lubrication**
   - Item: Machine Oil x2
   - Time: 15 seconds

### Power Generator

1. **Oil Change**

   - Item: Engine Oil x5
   - Time: 60 seconds

2. **Spark Plug Replacement**
   - Item: Spark Plug x4
   - Time: 30 seconds

### Solar Panel

1. **Panel Cleaning**
   - Item: Cleaning Cloth x1
   - Time: 15 seconds

## Upgrade Effects

### Efficiency Upgrades

- Reduce resource consumption
- Improve power output (solar panels)
- Applied as multiplier to consumption rates

### Speed Upgrades

- Reduce operation intervals
- Faster harvesting cycles
- Applied as multiplier to interval times

### Capacity Upgrades

- Increase storage capacity
- Larger fuel/water tanks
- Applied as multiplier to capacity values

### Range Upgrades

- Increase operational radius
- Larger coverage area
- Applied as multiplier to range values

### Durability Upgrades

- Reduce maintenance frequency
- Slower efficiency degradation
- Applied as multiplier to maintenance progress

## Blueprint Usage

### Register Device (Blueprint)

```
Get World Subsystem (Automation Efficiency System)
  → Register Device
    Device: [Irrigation System Reference]
    Device Type: Irrigation System
```

### Check Maintenance (Blueprint)

```
Get World Subsystem (Automation Efficiency System)
  → Needs Maintenance
    Device: [Device Reference]
  → Branch
    True: Show Maintenance Warning
    False: Continue Normal Operation
```

### Apply Upgrade (Blueprint)

```
Get World Subsystem (Automation Efficiency System)
  → Get Available Upgrades
    Device: [Device Reference]
  → For Each Loop
    → Apply Upgrade
      Device: [Device Reference]
      Upgrade: [Selected Upgrade]
```

## Performance Considerations

- **Update Interval**: 5 seconds (configurable)
- **Maintenance Interval**: 100 hours (configurable)
- **Efficiency Calculation**: Smooth interpolation over time
- **Status Updates**: Automatic via timer

## Best Practices

1. **Register devices immediately after spawning**
2. **Monitor maintenance progress regularly**
3. **Perform maintenance before devices malfunction**
4. **Apply upgrades strategically based on needs**
5. **Check power status for offline devices**
6. **Track overall efficiency for farm optimization**

## Common Issues

### Device Not Updating

- Ensure device is registered with the system
- Check that the world subsystem is initialized
- Verify update timer is running

### Efficiency Always 0%

- Check device power status (may be offline)
- Verify device hasn't malfunctioned
- Perform maintenance if needed

### Upgrades Not Working

- Ensure upgrade is applied successfully
- Check that device type matches upgrade type
- Verify upgrade multiplier is being used in device logic

## Integration with Existing Systems

### Power System

- Devices automatically marked offline when unpowered
- Efficiency drops to 0% without power
- Status restored when power returns

### Farming System

- Tracks irrigation and harvesting automation
- Calculates manual labor reduction
- Monitors farm automation efficiency

## Example: Complete Automation Setup

```cpp
void SetupAutomatedFarm()
{
    UAutomationEfficiencySystem* AutoSystem = GetWorld()->GetSubsystem<UAutomationEfficiencySystem>();

    // Create and register irrigation
    AIrrigationSystem* Irrigation = GetWorld()->SpawnActor<AIrrigationSystem>(FarmLocation);
    AutoSystem->RegisterDevice(Irrigation, EAutomationDeviceType::IrrigationSystem);
    Irrigation->Activate();

    // Create and register harvester
    AHarvestingMachine* Harvester = GetWorld()->SpawnActor<AHarvestingMachine>(FarmLocation);
    AutoSystem->RegisterDevice(Harvester, EAutomationDeviceType::HarvestingMachine);
    Harvester->StartHarvesting();

    // Create power system
    ASolarPanel* Solar = GetWorld()->SpawnActor<ASolarPanel>(PowerLocation);
    AutoSystem->RegisterDevice(Solar, EAutomationDeviceType::SolarPanel);

    APowerNetworkNode* PowerNode = GetWorld()->SpawnActor<APowerNetworkNode>(FarmLocation);

    // Apply initial upgrades
    TArray<FAutomationUpgrade> IrrigationUpgrades = AutoSystem->GetAvailableUpgrades(Irrigation);
    if (IrrigationUpgrades.Num() > 0)
    {
        AutoSystem->ApplyUpgrade(Irrigation, IrrigationUpgrades[0]); // Water efficiency
    }

    // Monitor automation
    float LaborReduction = AutoSystem->GetManualLaborReduction();
    UE_LOG(LogTemp, Log, TEXT("Farm automation reduces manual labor by %.1f%%"), LaborReduction);
}
```

## See Also

- [Irrigation System Guide](QUICKSTART_IrrigationSystem.md)
- [Harvesting Automation Guide](QUICKSTART_HarvestingAutomation.md)
- [Power System Guide](QUICKSTART_PowerSystem.md)
