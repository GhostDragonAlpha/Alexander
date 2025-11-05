# Irrigation System - Quick Start Guide

## Overview

The Irrigation System provides automated watering for crops within a 10-meter radius. It consumes 100 liters of water per hour and includes visual water spray effects.

## Key Features

- **Automatic Watering**: Waters all farm plots within 10m radius every 30 seconds
- **Water Management**: 1000L tank capacity with consumption tracking
- **Visual Effects**: Niagara-based water spray particles
- **Status Monitoring**: Active, Inactive, OutOfWater, and Malfunction states

## Quick Setup

### 1. Place Irrigation System

```cpp
// Spawn irrigation system in world
FActorSpawnParameters SpawnParams;
AIrrigationSystem* IrrigationSystem = GetWorld()->SpawnActor<AIrrigationSystem>(
    AIrrigationSystem::StaticClass(),
    FarmLocation,
    FRotator::ZeroRotator,
    SpawnParams
);
```

### 2. Configure System

```cpp
// Set water capacity
IrrigationSystem->MaxWaterCapacity = 1000.0f;
IrrigationSystem->CurrentWaterLevel = 1000.0f;

// Set watering parameters
IrrigationSystem->WateringInterval = 30.0f; // Water every 30 seconds
IrrigationSystem->WaterAmountPerCycle = 0.2f; // 20% water level per cycle
IrrigationSystem->WaterConsumptionRate = 100.0f; // 100L per hour
```

### 3. Activate System

```cpp
// Activate irrigation
IrrigationSystem->Activate();

// Or toggle on/off
IrrigationSystem->Toggle();
```

## Blueprint Usage

### Placing in Level

1. Open your planet level
2. Search for "IrrigationSystem" in Place Actors panel
3. Drag into level near farm plots
4. Position within 10m of crops you want to water

### Configuring in Editor

1. Select the irrigation system actor
2. In Details panel, set:
   - **Max Water Capacity**: 1000.0 (liters)
   - **Current Water Level**: 1000.0 (liters)
   - **Watering Radius Cm**: 1000.0 (10 meters)
   - **Water Consumption Rate**: 100.0 (L/hour)
   - **Watering Interval**: 30.0 (seconds)

### Blueprint Functions

```
// Activate/Deactivate
Call "Activate" or "Deactivate" on irrigation system

// Refill water
Call "Refill Water" with Amount (liters)

// Check status
Get "Water Level Percent" - returns 0-100%
Get "Time Until Empty" - returns seconds until water runs out
Get "Has Sufficient Water" - returns true if >10% capacity
```

## C++ Usage

### Basic Operations

```cpp
// Activate irrigation
IrrigationSystem->Activate();

// Deactivate irrigation
IrrigationSystem->Deactivate();

// Toggle on/off
IrrigationSystem->Toggle();

// Refill water tank
IrrigationSystem->RefillWater(500.0f); // Add 500 liters
```

### Monitoring

```cpp
// Check water level
float WaterPercent = IrrigationSystem->GetWaterLevelPercent();
if (WaterPercent < 20.0f)
{
    UE_LOG(LogTemp, Warning, TEXT("Water level low: %.1f%%"), WaterPercent);
}

// Check time until empty
float TimeLeft = IrrigationSystem->GetTimeUntilEmpty();
UE_LOG(LogTemp, Log, TEXT("Water will last %.1f seconds"), TimeLeft);

// Check if operational
if (!IrrigationSystem->HasSufficientWater())
{
    UE_LOG(LogTemp, Warning, TEXT("Insufficient water to operate"));
}
```

### Getting Farm Plots in Range

```cpp
// Get all farm plots within watering radius
TArray<AFarmPlot*> FarmPlots = IrrigationSystem->GetFarmPlotsInRange();

UE_LOG(LogTemp, Log, TEXT("Found %d farm plots in range"), FarmPlots.Num());

for (AFarmPlot* Plot : FarmPlots)
{
    // Do something with each plot
    float WaterLevel = Plot->WaterLevel;
}
```

### Manual Watering Trigger

```cpp
// Manually trigger watering cycle
IrrigationSystem->WaterCropsInRadius();
```

## System Behavior

### Automatic Watering Cycle

1. System checks for farm plots within 10m radius
2. Every 30 seconds (configurable), waters all plots in range
3. Each watering adds 20% to plot's water level
4. Consumes water from tank at 100L/hour

### Water Consumption

- **Active Consumption**: 100 liters per hour when active
- **Per-Frame Calculation**: `WaterPerSecond = 100.0f / 3600.0f`
- **Tank Capacity**: Default 1000 liters (10 hours of operation)

### Status States

- **Inactive**: System is off, no water consumption
- **Active**: System is running and watering crops
- **OutOfWater**: Water level below 10%, system auto-deactivates
- **Malfunction**: System error (extensible for future features)

## Visual Effects

### Water Spray Effect

The system uses Niagara particles to simulate water spray:

```cpp
// Configure spray effect
IrrigationSystem->NumSprayNozzles = 8; // Number of spray points
IrrigationSystem->SpraySpawnRate = 100.0f; // Particles per second
IrrigationSystem->SprayParticleSize = 5.0f; // Particle size in cm
```

### Effect Parameters

The Niagara system receives these parameters:

- **SpawnRate**: Adjusted based on water level (30%-100%)
- **ParticleSize**: Size of water droplets
- **NumNozzles**: Number of spray points around system
- **SprayRadius**: Visual radius of spray effect
- **WaterColor**: Light blue color (0.3, 0.6, 0.9)

## Integration with Farm Plots

### Automatic Integration

The irrigation system automatically finds and waters farm plots:

```cpp
// System automatically calls this every WateringInterval seconds
void AIrrigationSystem::WaterCropsInRadius()
{
    TArray<AFarmPlot*> FarmPlots = GetFarmPlotsInRange();

    for (AFarmPlot* FarmPlot : FarmPlots)
    {
        FarmPlot->WaterPlot(WaterAmountPerCycle);
    }
}
```

### Farm Plot Response

When watered by irrigation system:

- Water level increases by configured amount (default 20%)
- All crops reset their `bNeedsWater` flag
- Crops reset `TimeSinceLastWater` counter
- Crops update `LastWateredTime` timestamp

## Performance Considerations

### Optimization Tips

1. **Limit Active Systems**: Don't place too many active systems in one area
2. **Adjust Intervals**: Increase watering interval for better performance
3. **Particle Budget**: Reduce spray spawn rate if needed
4. **Range Checks**: System only checks for plots every watering cycle

### Recommended Settings

For optimal performance:

- **Max Active Systems per Area**: 5-10
- **Watering Interval**: 30-60 seconds
- **Spray Spawn Rate**: 50-100 particles/second
- **Update Frequency**: Tick every frame (required for water consumption)

## Troubleshooting

### System Won't Activate

**Problem**: Calling `Activate()` but system stays inactive

**Solutions**:

- Check water level: Must be >10% capacity
- Verify `CurrentWaterLevel > 0`
- Check status: Should not be `Malfunction`

```cpp
if (!IrrigationSystem->HasSufficientWater())
{
    IrrigationSystem->RefillWater(500.0f);
}
IrrigationSystem->Activate();
```

### No Crops Being Watered

**Problem**: System is active but crops aren't getting watered

**Solutions**:

- Verify farm plots are within 10m radius
- Check that farm plots exist in world
- Ensure watering interval has elapsed

```cpp
TArray<AFarmPlot*> Plots = IrrigationSystem->GetFarmPlotsInRange();
UE_LOG(LogTemp, Log, TEXT("Plots in range: %d"), Plots.Num());
```

### Water Depletes Too Fast

**Problem**: Water runs out too quickly

**Solutions**:

- Increase tank capacity: `MaxWaterCapacity = 2000.0f`
- Reduce consumption rate: `WaterConsumptionRate = 50.0f`
- Increase watering interval: `WateringInterval = 60.0f`

### Visual Effects Not Showing

**Problem**: No water spray particles visible

**Solutions**:

- Assign Niagara system: `WaterSpraySystem` must be set
- Check effect is active: System must be in Active status
- Verify Niagara component: Should auto-activate when system activates

```cpp
if (IrrigationSystem->WaterSprayEffect)
{
    IrrigationSystem->WaterSprayEffect->Activate(true);
}
```

## Example: Complete Setup

```cpp
// Spawn and configure irrigation system
AIrrigationSystem* Irrigation = GetWorld()->SpawnActor<AIrrigationSystem>(
    AIrrigationSystem::StaticClass(),
    FVector(0, 0, 100),
    FRotator::ZeroRotator
);

// Configure water capacity
Irrigation->MaxWaterCapacity = 1000.0f;
Irrigation->CurrentWaterLevel = 1000.0f;

// Configure watering behavior
Irrigation->WateringRadiusCm = 1000.0f; // 10m radius
Irrigation->WateringInterval = 30.0f; // Water every 30 seconds
Irrigation->WaterAmountPerCycle = 0.2f; // 20% water per cycle
Irrigation->WaterConsumptionRate = 100.0f; // 100L/hour

// Configure visual effects
Irrigation->NumSprayNozzles = 8;
Irrigation->SpraySpawnRate = 100.0f;
Irrigation->SprayParticleSize = 5.0f;

// Activate system
Irrigation->Activate();

// Monitor water level
float TimeLeft = Irrigation->GetTimeUntilEmpty();
UE_LOG(LogTemp, Log, TEXT("Irrigation will run for %.1f seconds"), TimeLeft);
```

## Requirements Met

This implementation satisfies:

- ✅ **Requirement 14.1**: Irrigation systems automatically water crops within 10m radius
- ✅ **Requirement 14.2**: Water consumption at 100L/hour
- ✅ Visual water spray effects using Niagara
- ✅ Status monitoring and management
- ✅ Integration with farm plot system

## Next Steps

1. Create Niagara water spray effect asset
2. Assign static mesh for irrigation system base
3. Test with multiple farm plots
4. Add power requirements (future enhancement)
5. Implement water source connections (future enhancement)
