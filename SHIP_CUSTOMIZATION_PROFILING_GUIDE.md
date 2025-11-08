# Ship Customization Performance Profiling Guide

## Overview

This guide covers the comprehensive performance profiling system for the Ship Customization Component. The profiling system tracks CPU timing, memory usage, network bandwidth, and frame time impact to ensure optimal performance.

## Table of Contents

1. [System Architecture](#system-architecture)
2. [Enabling Profiling](#enabling-profiling)
3. [Profiling Components](#profiling-components)
4. [Reading Profiling Metrics](#reading-profiling-metrics)
5. [Performance Budgets](#performance-budgets)
6. [Optimization Recommendations](#optimization-recommendations)
7. [Advanced Usage](#advanced-usage)
8. [Troubleshooting](#troubleshooting)

---

## System Architecture

The profiling system consists of three main components:

### 1. ShipCustomizationProfiling.h
- Macro-based profiling using Unreal's STAT system
- Zero overhead in shipping builds
- Integrates with Unreal Insights and stat commands

### 2. UShipCustomizationProfiler Component
- Real-time performance monitoring
- Session-based profiling with CSV export
- Automatic budget checking
- Memory, CPU, and network tracking

### 3. Integration Points in ShipCustomizationComponent
- SCOPE_CYCLE_COUNTER macros for automatic timing
- Manual timing for detailed profiler integration
- Network replication tracking hooks

---

## Enabling Profiling

### Method 1: Compile-Time Profiling (STAT System)

The STAT system is enabled by default in non-shipping builds. To use it:

**In C++ (already integrated):**
```cpp
#include "ShipCustomizationProfiling.h"

void MyFunction()
{
    SCOPE_CYCLE_COUNTER(STAT_ShipCustomization_CalculateStats);
    // Your code here
}
```

**In Console:**
```
stat ShipCustomization
```

This displays real-time stats overlay showing:
- CalculateTotalStats execution time
- EquipPart execution time
- ApplyStatsToFlightController execution time
- UpdateShipVisuals execution time
- And more...

### Method 2: Runtime Profiling (Profiler Component)

Add the `UShipCustomizationProfiler` component to your ship actor:

**Blueprint:**
1. Open your ship Blueprint
2. Add Component → Ship Customization Profiler
3. In Details panel, set:
   - Profiling Enabled: ✓
   - Detailed Logging: ✓ (for verbose output)
   - Real Time Display: ✓ (for periodic logging)

**C++:**
```cpp
UShipCustomizationProfiler* Profiler = CreateDefaultSubobject<UShipCustomizationProfiler>(TEXT("CustomizationProfiler"));
Profiler->bProfilingEnabled = true;
Profiler->bDetailedLogging = true;
```

---

## Profiling Components

### CPU Timing Metrics

Tracks execution time of major operations:

| Metric | Description | Target Budget |
|--------|-------------|---------------|
| **CalculateStatsTime** | Time to calculate total ship stats from equipped parts | < 0.1 ms |
| **ApplyStatsTime** | Time to apply stats to FlightController and OrbitalBody | < 0.2 ms |
| **EquipPartTime** | Total time to equip a part (includes recalc + apply) | < 0.5 ms |
| **LoadLoadoutTime** | Time to load a complete loadout | < 1.0 ms |
| **UpdateVisualsTime** | Time to update ship visual appearance | < 0.3 ms |
| **TotalFrameTime** | Sum of all customization operations per frame | < 0.5 ms |

### Memory Metrics

Tracks memory usage across data structures:

| Metric | Description | Target Budget |
|--------|-------------|---------------|
| **LoadoutMemoryUsage** | Memory used by CurrentLoadout (TMap + FText) | Variable |
| **UnlockedPartsMemoryUsage** | Memory for unlocked parts/skins arrays | Variable |
| **DataTableMemoryUsage** | Memory for ship parts and skins data tables | Variable |
| **TotalMemoryUsage** | Sum of all customization memory | < 1 MB |

### Network Metrics

Tracks replication bandwidth:

| Metric | Description | Target Budget |
|--------|-------------|---------------|
| **LoadoutReplicationBytes** | Bytes sent for CurrentLoadout replication | N/A |
| **NetworkUpdateCount** | Number of network replication events | N/A |
| **AverageReplicationRate** | Average bytes per second for replication | < 10 KB/s |

### Frame Impact Metrics

| Metric | Description | Target |
|--------|-------------|--------|
| **FrameTimeImpact** | Percentage of 16.67ms frame budget used | < 3% (0.5ms) |

---

## Reading Profiling Metrics

### Console Commands

**View Real-Time Stats:**
```
stat ShipCustomization
```

**View Detailed Stats:**
```
stat ShipCustomization -verbose
```

**Reset Stats:**
```
stat ShipCustomization -reset
```

**Export to CSV (Unreal Insights):**
```
trace.start
trace.stop
```

### Blueprint/C++ API

**Get Current Metrics:**
```cpp
UShipCustomizationProfiler* Profiler = ShipActor->FindComponentByClass<UShipCustomizationProfiler>();
FCustomizationProfilingMetrics CurrentMetrics = Profiler->CurrentMetrics;

UE_LOG(LogTemp, Log, TEXT("Total Frame Time: %.3f ms"), CurrentMetrics.TotalFrameTime);
UE_LOG(LogTemp, Log, TEXT("Memory Usage: %lld bytes"), CurrentMetrics.TotalMemoryUsage);
```

**Get Profiling Summary:**
```cpp
FString Summary = Profiler->GetProfilingSummary();
UE_LOG(LogTemp, Log, TEXT("%s"), *Summary);
```

**Example Output:**
```
Ship Customization Profiling Summary
=====================================

CPU Timing:
  Total Frame Time: 0.234 ms (1.40% of frame budget)
  CalculateStats: 0.042 ms
  ApplyStats: 0.087 ms
  EquipPart: 0.234 ms
  LoadLoadout: 0.000 ms
  UpdateVisuals: 0.056 ms

Memory Usage:
  Total: 4352 bytes (4.25 KB)
  Loadout: 512 bytes
  Unlocked Parts: 2048 bytes
  Data Tables: 1792 bytes

Network:
  Replication Rate: 234.56 bytes/s
  Update Count: 12
  Total Bytes: 2815

Performance Budgets:
  CPU: PASS (0.234 / 0.500 ms)
  Memory: PASS (4352 / 1048576 bytes)
  Network: PASS (234.56 / 10240 bytes/s)
```

### Session-Based Profiling

**Start a Profiling Session:**
```cpp
Profiler->StartProfilingSession(TEXT("LoadoutTestSession"));

// ... perform operations to profile ...

Profiler->EndProfilingSession();
```

**Export Session to CSV:**
```cpp
Profiler->ExportSessionToCSV(TEXT("LoadoutTestSession.csv"));
// Saves to: <Project>/Saved/Profiling/LoadoutTestSession.csv
```

**CSV Format:**
```csv
Sample,CalculateStatsTime,ApplyStatsTime,EquipPartTime,LoadLoadoutTime,UpdateVisualsTime,TotalFrameTime,LoadoutMemory,UnlockedPartsMemory,DataTableMemory,TotalMemory,NetworkBytes,NetworkUpdateCount,ReplicationRate,FrameImpact,WithinCPUBudget,WithinMemoryBudget,WithinNetworkBudget
0,0.042,0.087,0.234,0.000,0.056,0.234,512,2048,1792,4352,128,1,128.00,1.40,1,1,1
1,0.038,0.082,0.000,0.000,0.000,0.082,512,2048,1792,4352,0,0,64.00,0.49,1,1,1
...
```

---

## Performance Budgets

### Default Budgets

The profiler comes with these default performance budgets:

```cpp
struct FPerformanceBudgets
{
    float MaxCPUTimePerFrame = 0.5f;        // 0.5ms (3% of 16.67ms frame)
    int64 MaxMemoryUsage = 1048576;         // 1 MB
    int32 MaxNetworkBytesPerSecond = 10240; // 10 KB/s
};
```

### Customizing Budgets

**In Blueprint:**
1. Select ShipCustomizationProfiler component
2. Expand "Performance Budgets"
3. Adjust values:
   - Max CPU Time Per Frame (ms)
   - Max Memory Usage (bytes)
   - Max Network Bytes Per Second

**In C++:**
```cpp
Profiler->PerformanceBudgets.MaxCPUTimePerFrame = 0.3f; // Stricter budget
Profiler->PerformanceBudgets.MaxMemoryUsage = 512000;   // 500 KB
```

### Budget Warnings

When budgets are exceeded, the profiler logs warnings:

```
LogTemp: Warning: ShipCustomizationProfiler: CPU budget exceeded! 0.623 ms / 0.500 ms
LogTemp: Warning: ShipCustomizationProfiler: Memory budget exceeded! 1234567 bytes / 1048576 bytes
```

---

## Optimization Recommendations

### CPU Optimization

#### 1. CalculateTotalStats Optimization

**Current Implementation:**
```cpp
for (const TPair<EShipPartCategory, FName>& EquippedPart : CurrentLoadout.EquippedParts)
{
    FShipPartData PartData = GetPartData(EquippedPart.Value, bFound);
    if (bFound)
    {
        TotalStats = TotalStats + PartData.StatModifiers;
    }
}
```

**Optimization Strategies:**
- **Cache Part Data**: Store FShipPartData* pointers instead of FName lookups
- **Incremental Updates**: Only recalculate when parts change, not every frame
- **SIMD Operations**: Use vectorized math for stat calculations

**Example Optimized Version:**
```cpp
// Cache part data on equip
TMap<EShipPartCategory, const FShipPartData*> CachedPartData;

// Incremental stat calculation
void IncrementalAddPartStats(const FShipPartData& PartData)
{
    CurrentLoadout.TotalStats = CurrentLoadout.TotalStats + PartData.StatModifiers;
}

void IncrementalRemovePartStats(const FShipPartData& PartData)
{
    CurrentLoadout.TotalStats = CurrentLoadout.TotalStats - PartData.StatModifiers;
}
```

#### 2. ApplyStatsToFlightController Optimization

**Bottlenecks:**
- Component lookups (GetOrbitalBody, GetFlightController)
- Network RPC calls on clients

**Optimizations:**
- Cache component references (already implemented)
- Batch RPC calls if applying multiple stats
- Use RepNotify for automatic replication

#### 3. Data Table Lookup Optimization

**Current:**
```cpp
FShipPartData* PartData = ShipPartsDataTable->FindRow<FShipPartData>(PartID, TEXT(""));
```

**Optimization:**
```cpp
// Build lookup cache on BeginPlay
TMap<FName, FShipPartData*> PartDataCache;

void BuildPartDataCache()
{
    TArray<FName> RowNames = ShipPartsDataTable->GetRowNames();
    for (FName RowName : RowNames)
    {
        FShipPartData* PartData = ShipPartsDataTable->FindRow<FShipPartData>(RowName, TEXT(""));
        if (PartData)
        {
            PartDataCache.Add(RowName, PartData);
        }
    }
}

FShipPartData* GetPartDataFast(FName PartID)
{
    FShipPartData** FoundData = PartDataCache.Find(PartID);
    return FoundData ? *FoundData : nullptr;
}
```

### Memory Optimization

#### 1. TMap Memory Overhead

**Current Issue:**
```cpp
TMap<EShipPartCategory, FName> EquippedParts; // ~8 categories max
```

**Optimization:**
```cpp
// Use fixed array for predictable memory usage
struct FFixedLoadout
{
    FName Parts[8]; // One per EShipPartCategory

    int64 GetMemoryUsage() const { return sizeof(FFixedLoadout); }
};
```

#### 2. FText Memory Overhead

**Current:**
```cpp
FText LoadoutName; // Can allocate 100+ bytes
```

**Optimization:**
```cpp
// Use FName for internal storage
FName LoadoutNameID;

// Generate FText on demand
FText GetLoadoutName() const
{
    return FText::FromName(LoadoutNameID);
}
```

#### 3. TArray Slack Optimization

**Current:**
```cpp
TArray<FName> UnlockedParts; // May have slack space
```

**Optimization:**
```cpp
UnlockedParts.Shrink(); // Remove slack after loading
```

### Network Optimization

#### 1. Reduce Replication Frequency

**Current:** CurrentLoadout replicates on every change

**Optimization:**
```cpp
// Batch multiple part changes before replicating
UPROPERTY(ReplicatedUsing=OnRep_LoadoutChanged)
FShipLoadout CurrentLoadout;

bool bPendingReplication = false;

void BatchEquipPart(EShipPartCategory Category, FName PartID)
{
    CurrentLoadout.EquippedParts.Add(Category, PartID);
    bPendingReplication = true;
    // Don't recalculate yet
}

void FlushBatchedChanges()
{
    if (bPendingReplication)
    {
        CalculateTotalStats();
        ApplyStatsToFlightController();
        bPendingReplication = false;
    }
}
```

#### 2. Custom Replication Struct

**Optimization:**
```cpp
// Only replicate essential data
USTRUCT()
struct FCompactLoadout
{
    GENERATED_BODY()

    // Bit-packed part IDs (8 parts x 8 bits = 64 bits)
    UPROPERTY()
    uint64 PackedPartIDs;

    // Serialize to minimal bytes
    bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};
```

---

## Advanced Usage

### Custom Profiling Scopes

**Manual Timing:**
```cpp
Profiler->BeginTiming(TEXT("CustomOperation"));

// Your code here
PerformExpensiveOperation();

Profiler->EndTiming(TEXT("CustomOperation"));
```

**Scoped Timing with Budget Warning:**
```cpp
{
    FShipCustomizationProfileScope Scope(TEXT("LoadAllParts"), 5.0f); // 5ms budget

    // This will log a warning if it exceeds 5ms
    LoadAllShipPartsFromDisk();

} // Logs execution time on scope exit
```

### Profiling Network Replication

**Track Custom Replication:**
```cpp
// In your OnRep function
void OnRep_CustomData()
{
    int32 EstimatedBytes = sizeof(FCustomData);

    UShipCustomizationProfiler* Profiler = FindComponentByClass<UShipCustomizationProfiler>();
    if (Profiler)
    {
        Profiler->RecordNetworkReplication(EstimatedBytes);
    }
}
```

### Memory Profiling Best Practices

**Periodic Memory Snapshots:**
```cpp
// In Tick or timer
void PeriodicMemorySnapshot()
{
    Profiler->UpdateMemoryMetrics();

    int64 CurrentMemory = Profiler->CurrentMetrics.TotalMemoryUsage;
    int64 PeakMemory = Profiler->PeakMetrics.TotalMemoryUsage;

    if (CurrentMemory > PeakMemory * 0.9f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Approaching peak memory usage!"));
    }
}
```

### Integration with Unreal Insights

**Capture Trace:**
```
trace.start
// Perform operations
trace.stop
```

**View in Unreal Insights:**
1. Open Unreal Insights (UnrealInsights.exe)
2. Load trace file
3. Navigate to "Timing Insights"
4. Filter by "ShipCustomization" stats
5. Analyze CPU usage, call counts, and frame time impact

---

## Troubleshooting

### Issue: Stats Not Appearing

**Symptoms:** `stat ShipCustomization` shows nothing

**Solutions:**
1. Verify you're in a non-shipping build
2. Check that `ENABLE_SHIP_CUSTOMIZATION_PROFILING` is defined
3. Ensure operations are actually being called
4. Try `stat ShipCustomization -reset`

### Issue: High CPU Time

**Symptoms:** EquipPart consistently exceeds 1ms

**Diagnosis:**
```cpp
// Add detailed timing breakdown
double T1 = FPlatformTime::Seconds();
GetPartData(PartID, bFound);
double T2 = FPlatformTime::Seconds();
CalculateTotalStats();
double T3 = FPlatformTime::Seconds();
ApplyStatsToFlightController();
double T4 = FPlatformTime::Seconds();

UE_LOG(LogTemp, Log, TEXT("GetPartData: %.3f ms"), (T2-T1)*1000.0);
UE_LOG(LogTemp, Log, TEXT("CalculateStats: %.3f ms"), (T3-T2)*1000.0);
UE_LOG(LogTemp, Log, TEXT("ApplyStats: %.3f ms"), (T4-T3)*1000.0);
```

**Solutions:**
- Implement part data caching
- Reduce data table lookups
- Profile with Unreal Insights for detailed breakdown

### Issue: Memory Leaks

**Symptoms:** TotalMemoryUsage continuously grows

**Diagnosis:**
```cpp
// Track memory over time
TArray<int64> MemorySamples;

void SampleMemory()
{
    MemorySamples.Add(Profiler->CurrentMetrics.TotalMemoryUsage);

    if (MemorySamples.Num() > 100)
    {
        // Analyze trend
        int64 FirstSample = MemorySamples[0];
        int64 LastSample = MemorySamples.Last();

        if (LastSample > FirstSample * 1.5f)
        {
            UE_LOG(LogTemp, Error, TEXT("Possible memory leak detected!"));
        }
    }
}
```

**Solutions:**
- Check for TArray growth without Shrink()
- Verify SavedLoadouts isn't accumulating duplicates
- Look for FText allocations in loops

### Issue: Network Bandwidth Spike

**Symptoms:** AverageReplicationRate > 100 KB/s

**Diagnosis:**
```cpp
// Log each replication event
void RecordNetworkReplication(int32 BytesSent)
{
    UE_LOG(LogTemp, Warning, TEXT("Network replication: %d bytes"), BytesSent);

    // Track source
    FString CallStack = FPlatformStackWalk::GetCallStack();
    UE_LOG(LogTemp, Log, TEXT("Called from: %s"), *CallStack);
}
```

**Solutions:**
- Reduce replication frequency (use RepNotify)
- Compress loadout data
- Only replicate on server authority

---

## Performance Baseline Measurements

### Expected Performance Targets

Based on typical usage patterns:

| Operation | Target | Acceptable | Warning |
|-----------|--------|------------|---------|
| EquipPart (cold cache) | 0.3 ms | 0.5 ms | 1.0 ms |
| EquipPart (warm cache) | 0.1 ms | 0.2 ms | 0.5 ms |
| CalculateTotalStats | 0.05 ms | 0.1 ms | 0.2 ms |
| ApplyStatsToFlightController | 0.08 ms | 0.15 ms | 0.3 ms |
| LoadLoadout | 0.5 ms | 1.0 ms | 2.0 ms |
| UpdateShipVisuals | 0.2 ms | 0.4 ms | 0.8 ms |

### Typical Memory Footprint

| Data | Expected Size |
|------|---------------|
| Single Loadout | 200-500 bytes |
| 50 Unlocked Parts | ~2 KB |
| 10 Saved Loadouts | ~5 KB |
| Full Data Tables | ~10-50 KB |
| **Total** | **< 100 KB** |

### Network Bandwidth

| Scenario | Expected Bandwidth |
|----------|-------------------|
| Idle (no changes) | 0 bytes/s |
| Single part equip | ~200 bytes |
| Full loadout change | ~500 bytes |
| Continuous changes (stress test) | < 5 KB/s |

---

## Console Command Reference

### Stat Commands
```
stat ShipCustomization           # Show real-time stats
stat ShipCustomization -verbose  # Show detailed stats
stat ShipCustomization -reset    # Reset counters
stat none                        # Hide all stats
```

### Profiler Commands (Blueprint/C++)
```cpp
// Start session
CustomizationProfiler.StartProfilingSession("MySession")

// End session
CustomizationProfiler.EndProfilingSession()

// Export to CSV
CustomizationProfiler.ExportSessionToCSV("Results.csv")

// Get summary
FString Summary = CustomizationProfiler.GetProfilingSummary()

// Get detailed report
FString Report = CustomizationProfiler.GetDetailedReport()
```

---

## Best Practices Summary

1. **Always profile before optimizing** - Use profiler to identify actual bottlenecks
2. **Set realistic budgets** - Base on target frame rate and platform
3. **Cache aggressively** - Component refs, part data, calculated stats
4. **Batch operations** - Combine multiple part changes before recalculating
5. **Use incremental updates** - Don't recalculate everything on small changes
6. **Monitor in PIE and shipped builds** - Performance can differ significantly
7. **Export sessions for analysis** - CSV data helps identify trends
8. **Integrate with CI/CD** - Automated performance regression testing

---

## Appendix: Profiler API Reference

### UShipCustomizationProfiler

#### Properties
- `bProfilingEnabled` - Master switch for profiling
- `bDetailedLogging` - Enable verbose logging
- `bRealTimeDisplay` - Log metrics periodically
- `PerformanceBudgets` - Budget thresholds
- `CurrentMetrics` - Latest frame metrics
- `AverageMetrics` - Running average
- `PeakMetrics` - Maximum values recorded

#### Methods
- `StartProfilingSession(SessionName)` - Begin new session
- `EndProfilingSession()` - End current session
- `ExportSessionToCSV(FilePath)` - Export to CSV
- `BeginTiming(OperationName)` - Start manual timer
- `EndTiming(OperationName)` - Stop manual timer
- `RecordCalculateStatsTime(TimeMS)` - Record calc time
- `RecordApplyStatsTime(TimeMS)` - Record apply time
- `RecordEquipPartTime(TimeMS)` - Record equip time
- `RecordLoadLoadoutTime(TimeMS)` - Record load time
- `RecordUpdateVisualsTime(TimeMS)` - Record visual update time
- `UpdateMemoryMetrics()` - Refresh memory measurements
- `RecordNetworkReplication(BytesSent)` - Track network event
- `GetProfilingSummary()` - Get formatted summary
- `GetDetailedReport()` - Get full report
- `LogCurrentMetrics()` - Print current metrics

---

## Support

For issues, questions, or feature requests related to the profiling system, please refer to:
- SHIP_CUSTOMIZATION_COMPONENT_GUIDE.md (main customization documentation)
- Unreal Engine documentation on Performance Profiling
- Unreal Insights documentation

---

**Document Version:** 1.0
**Last Updated:** 2025-01-07
**Maintained By:** Ship Customization Development Team
