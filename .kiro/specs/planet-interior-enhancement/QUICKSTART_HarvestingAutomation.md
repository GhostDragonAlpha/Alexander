# Quickstart Guide: Automated Harvesting System

## Overview

The Automated Harvesting System provides machinery that automatically collects mature crops from farm plots within a designated area. This system reduces manual labor and enables large-scale farming operations.

## Core Components

### AHarvestingMachine

The main actor that performs automatic crop harvesting.

**Key Features:**

- Automatic crop collection within configurable radius
- Plot boundary detection and tracking
- Harvest storage system with capacity limits
- Power consumption simulation
- Configurable harvest intervals
- Statistics tracking

## Quick Setup

### 1. Place a Harvesting Machine

```cpp
// In Blueprint or C++
AHarvestingMachine* Machine = GetWorld()->SpawnActor<AHarvestingMachine>(
    AHarvestingMachine::StaticClass(),
    FarmLocation,
    FRotator::ZeroRotator
);

// Configure the machine
FHarvestingMachineConfig Config;
Config.HarvestRadius = 15.0f;  // 15 meter radius
Config.HarvestInterval = 10.0f; // Harvest every 10 seconds
Config.MaxStorageCapacity = 500; // Store up to 500 items
Config.bAutoHarvest = true;
Config.bRequiresPower = true;
Config.PowerConsumption = 500.0f; // 500 watts

Machine->Config = Config;
```

### 2. Start Automatic Harvesting

```cpp
// Start the machine
Machine->StartHarvesting();

// The machine will automatically:
// - Detect farm plots within radius
// - Track crop cells in harvest zone
// - Harvest mature crops at intervals
// - Store harvested crops
```

### 3. Monitor Harvesting

```cpp
// Check storage status
float FillPercentage = Machine->GetStorageFillPercentage();
int32 StoredCount = Machine->CurrentStorageCount;
bool bFull = Machine->IsStorageFull();

// Get statistics
int32 TotalHarvested;
float TotalValue;
float PowerUsed;
Machine->GetHarvestStatistics(TotalHarvested, TotalValue, PowerUsed);

// Check specific crop storage
int32 WheatCount = Machine->GetStoredQuantity("Wheat");
```

## Configuration Options

### Harvest Radius

Controls the area the machine can harvest from:

```cpp
Machine->SetHarvestRadius(20.0f); // 20 meter radius
```

### Harvest Interval

Time between automatic harvest cycles:

```cpp
Machine->SetHarvestInterval(5.0f); // Harvest every 5 seconds
```

### Auto-Harvest Mode

Enable or disable automatic harvesting:

```cpp
Machine->SetAutoHarvest(true); // Enable auto-harvest
Machine->SetAutoHarvest(false); // Disable (manual only)
```

### Storage Capacity

Maximum number of items the machine can store:

```cpp
Machine->Config.MaxStorageCapacity = 1000;
```

## Plot Boundary Detection

The machine automatically detects farm plots within its harvest radius:

```cpp
// Manually trigger plot detection
Machine->DetectFarmPlots();

// Check managed plots
TArray<AFarmPlot*> ManagedPlots = Machine->ManagedPlots;

// Get cells in harvest zone for a specific plot
TArray<FIntPoint> Cells = Machine->GetCropCellsInZone(FarmPlot);

// Update plot tracking
Machine->UpdateManagedPlots();
```

## Manual Harvesting

Perform manual harvest operations:

```cpp
// Perform a single harvest cycle
int32 HarvestedCount = Machine->PerformHarvestCycle();

// Harvest a specific crop cell
bool bSuccess = Machine->HarvestCropCell(FarmPlot, FIntPoint(5, 3));

// Check if a crop is ready
bool bReady = Machine->IsCropReadyForHarvest(FarmPlot, FIntPoint(5, 3));
```

## Storage Management

### Adding to Storage

```cpp
// Add harvested crops (done automatically during harvest)
bool bAdded = Machine->AddToStorage("Wheat", 10, 50.0f);
```

### Removing from Storage

```cpp
// Remove crops (for processing or selling)
bool bRemoved = Machine->RemoveFromStorage("Wheat", 5);
```

### Emptying Storage

```cpp
// Empty all storage
Machine->EmptyStorage();
```

### Storage Queries

```cpp
// Get total stored value
float TotalValue = Machine->GetTotalStoredValue();

// Get stored quantity of specific crop
int32 Quantity = Machine->GetStoredQuantity("Tomato");

// Check if storage is full
if (Machine->IsStorageFull())
{
    // Handle full storage (stop harvesting, transfer to warehouse, etc.)
}
```

## Power System

### Power Management

```cpp
// Set power state
Machine->SetPowerState(true);  // Power on
Machine->SetPowerState(false); // Power off

// Check power status
bool bHasPower = Machine->HasSufficientPower();

// Get current power consumption
float PowerWatts = Machine->GetCurrentPowerConsumption();

// Get total power consumed
float TotalKWh = Machine->TotalPowerConsumed;
```

### Power Requirements

```cpp
// Configure power requirements
Machine->Config.bRequiresPower = true;
Machine->Config.PowerConsumption = 750.0f; // 750 watts
```

## Statistics Tracking

### Harvest Statistics

```cpp
// Get comprehensive statistics
int32 TotalHarvested;
float TotalValue;
float PowerUsed;
Machine->GetHarvestStatistics(TotalHarvested, TotalValue, PowerUsed);

// Access individual stats
int32 CropsHarvested = Machine->TotalCropsHarvested;
float ValueHarvested = Machine->TotalValueHarvested;
float PowerConsumed = Machine->TotalPowerConsumed;

// Reset statistics
Machine->ResetStatistics();
```

## Blueprint Usage

### Creating in Blueprint

1. Drag `AHarvestingMachine` into your level
2. Set configuration in Details panel:
   - Harvest Radius
   - Harvest Interval
   - Max Storage Capacity
   - Auto Harvest enabled
   - Power settings

### Blueprint Events

```cpp
// Start/Stop harvesting
UFUNCTION(BlueprintCallable)
void StartHarvesting();

UFUNCTION(BlueprintCallable)
void StopHarvesting();

// Perform manual harvest
UFUNCTION(BlueprintCallable)
int32 PerformHarvestCycle();

// Storage management
UFUNCTION(BlueprintCallable)
bool IsStorageFull();

UFUNCTION(BlueprintCallable)
float GetStorageFillPercentage();

UFUNCTION(BlueprintCallable)
void EmptyStorage();
```

## Integration with Farm Plots

The harvesting machine integrates seamlessly with farm plots:

```cpp
// Machine automatically:
// 1. Detects farm plots within radius
// 2. Identifies crop cells in harvest zone
// 3. Checks crop maturity (GrowthProgress >= 1.0)
// 4. Harvests mature crops
// 5. Stores harvested items
// 6. Updates farm plot state
```

## Example: Complete Setup

```cpp
void AMyGameMode::SetupAutomatedFarm()
{
    // Create farm plot
    AFarmPlot* FarmPlot = FarmingSubsystem->CreateFarmPlot(
        FarmLocation,
        FVector2D(20.0f, 20.0f),
        PlanetActor
    );

    // Plant crops
    for (int32 Y = 0; Y < FarmPlot->GridResolution; ++Y)
    {
        for (int32 X = 0; X < FarmPlot->GridResolution; ++X)
        {
            FarmPlot->PlantCrop(FIntPoint(X, Y), WheatCropDef);
        }
    }

    // Create harvesting machine
    FVector MachineLocation = FarmLocation + FVector(0, 0, 100);
    AHarvestingMachine* Machine = GetWorld()->SpawnActor<AHarvestingMachine>(
        AHarvestingMachine::StaticClass(),
        MachineLocation,
        FRotator::ZeroRotator
    );

    // Configure machine
    Machine->SetHarvestRadius(15.0f);
    Machine->SetHarvestInterval(10.0f);
    Machine->Config.MaxStorageCapacity = 1000;
    Machine->Config.bAutoHarvest = true;

    // Start automatic harvesting
    Machine->StartHarvesting();

    // Monitor in game loop
    GetWorld()->GetTimerManager().SetTimer(
        MonitorTimer,
        [Machine]()
        {
            if (Machine->IsStorageFull())
            {
                // Transfer to warehouse
                Machine->EmptyStorage();
            }
        },
        30.0f, // Check every 30 seconds
        true
    );
}
```

## Performance Considerations

### Optimization Tips

1. **Harvest Interval**: Longer intervals reduce CPU usage

   ```cpp
   Machine->SetHarvestInterval(15.0f); // Less frequent checks
   ```

2. **Harvest Radius**: Smaller radius = fewer plots to check

   ```cpp
   Machine->SetHarvestRadius(10.0f); // Smaller area
   ```

3. **Plot Updates**: Only update when necessary

   ```cpp
   // Update plots when configuration changes
   Machine->UpdateManagedPlots();
   ```

4. **Storage Management**: Empty storage regularly to prevent overflow checks
   ```cpp
   if (Machine->GetStorageFillPercentage() > 0.8f)
   {
       // Transfer to warehouse before full
   }
   ```

## Troubleshooting

### Machine Not Harvesting

```cpp
// Check if machine is active
if (!Machine->bIsActive)
{
    Machine->StartHarvesting();
}

// Check power state
if (!Machine->HasSufficientPower())
{
    Machine->SetPowerState(true);
}

// Check storage
if (Machine->IsStorageFull())
{
    Machine->EmptyStorage();
}

// Check if plots are detected
if (Machine->ManagedPlots.Num() == 0)
{
    Machine->DetectFarmPlots();
}
```

### Crops Not Being Detected

```cpp
// Verify crop is ready
bool bReady = Machine->IsCropReadyForHarvest(Plot, GridPos);

// Check if cell is in harvest zone
TArray<FIntPoint> Cells = Machine->GetCropCellsInZone(Plot);
bool bInZone = Cells.Contains(GridPos);

// Update plot tracking
Machine->UpdateManagedPlots();
```

### Storage Issues

```cpp
// Check storage capacity
UE_LOG(LogTemp, Log, TEXT("Storage: %d / %d (%.1f%%)"),
    Machine->CurrentStorageCount,
    Machine->Config.MaxStorageCapacity,
    Machine->GetStorageFillPercentage() * 100.0f
);

// List stored crops
for (const FHarvestedCropEntry& Entry : Machine->StoredCrops)
{
    UE_LOG(LogTemp, Log, TEXT("Crop: %s, Quantity: %d, Value: %.2f"),
        *Entry.CropID, Entry.Quantity, Entry.TotalValue);
}
```

## Related Systems

- **FarmPlot**: Provides crops to harvest
- **FarmingSubsystem**: Manages farm plot registry
- **CropDefinition**: Defines crop properties
- **IrrigationSystem**: Automated watering (Task 16.1)

## Next Steps

1. Implement power generation systems (solar panels, generators)
2. Create warehouse storage for harvested crops
3. Add conveyor systems for crop transport
4. Implement crop processing machinery
5. Create economic system for selling harvested crops
