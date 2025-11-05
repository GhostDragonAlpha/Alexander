# VR Performance Optimization - Quick Start Guide

## Overview

The VR Performance Optimization system ensures smooth 90 FPS minimum performance in VR through dynamic resolution scaling, LOD optimization, draw call management, and frame timing consistency.

## Key Features

- **90 FPS Target**: Maintains minimum 90 FPS for VR comfort
- **Dynamic Resolution**: Automatically adjusts resolution to maintain frame rate
- **Draw Call Optimization**: Keeps draw calls below 5000 per frame
- **Frame Timing**: Ensures consistent frame delivery
- **LOD Management**: Optimizes level of detail based on performance
- **Occlusion Culling**: Reduces rendering overhead

## Quick Setup

### 1. System Initialization

The VR Performance Optimizer is a World Subsystem that initializes automatically:

```cpp
// Get the optimizer subsystem
UVRPerformanceOptimizer* Optimizer = GetWorld()->GetSubsystem<UVRPerformanceOptimizer>();

// Check if meeting performance targets
bool bGoodPerformance = Optimizer->IsMeetingPerformanceTarget();

// Get current metrics
FVRPerformanceMetrics Metrics = Optimizer->GetCurrentMetrics();
UE_LOG(LogTemp, Log, TEXT("FPS: %.1f, Frame Time: %.2fms, Draw Calls: %d"),
    Metrics.CurrentFPS, Metrics.AverageFrameTime, Metrics.DrawCalls);
```

### 2. Configuration

Configure performance targets and optimization settings:

```cpp
FVRPerformanceConfig Config;
Config.TargetFPS = 90.0f;
Config.MaxFrameTimeMS = 11.1f;
Config.TargetMaxDrawCalls = 5000;
Config.bEnableDynamicResolution = true;
Config.MinResolutionScale = 0.5f;
Config.MaxResolutionScale = 1.5f;
Config.bEnableAggressiveLOD = true;
Config.LODDistanceScale = 1.0f;

Optimizer->SetPerformanceConfig(Config);
```

### 3. Dynamic Resolution Control

```cpp
// Enable/disable dynamic resolution
Optimizer->SetDynamicResolutionEnabled(true);

// Manually set resolution scale (50% to 150%)
Optimizer->SetResolutionScale(0.8f); // 80% resolution

// Adjust LOD distances
Optimizer->OptimizeLODDistances(0.8f); // More aggressive LOD
```

### 4. Performance Monitoring

```cpp
// Get current performance metrics
FVRPerformanceMetrics Metrics = Optimizer->GetCurrentMetrics();

if (!Metrics.bMeetingPerformanceTarget)
{
    UE_LOG(LogTemp, Warning, TEXT("Performance below target!"));
    UE_LOG(LogTemp, Warning, TEXT("  FPS: %.1f (target: 90)"), Metrics.CurrentFPS);
    UE_LOG(LogTemp, Warning, TEXT("  Frame Time: %.2fms (target: 11.1ms)"), Metrics.AverageFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("  Draw Calls: %d (target: 5000)"), Metrics.DrawCalls);
}
```

## Blueprint Usage

### Get Performance Metrics

```
Get World Subsystem (VRPerformanceOptimizer)
  -> Get Current Metrics
    -> Print String (FPS: {CurrentFPS})
```

### Enable Dynamic Resolution

```
Get World Subsystem (VRPerformanceOptimizer)
  -> Set Dynamic Resolution Enabled (true)
```

### Check Performance Status

```
Get World Subsystem (VRPerformanceOptimizer)
  -> Is Meeting Performance Target
    -> Branch
      True: Display "Performance Good"
      False: Display "Performance Warning"
```

## Automatic Optimizations

The system automatically applies these optimizations:

### VR-Specific Settings

- Instanced stereo rendering
- Mobile multi-view
- Round-robin occlusion culling

### Frame Timing

- VSync disabled (VR runtime handles sync)
- Finish current frame enabled
- Consistent frame delivery

### Shadow Optimizations

- Max shadow resolution: 1024
- Shadow distance scale: 0.6
- Shadow radius threshold: 0.03

### LOD Optimizations

- Static mesh LOD distance scale: 0.8
- Skeletal mesh LOD bias: 1
- Dynamic LOD adjustment based on performance

### Culling Optimizations

- Hardware occlusion queries enabled
- Hierarchical Z-buffer occlusion
- View distance culling

## Performance Targets

| Metric           | Target | Critical    |
| ---------------- | ------ | ----------- |
| Frame Rate       | 90 FPS | 72 FPS      |
| Frame Time       | 11.1ms | 13.9ms      |
| Draw Calls       | 5000   | 7000        |
| Resolution Scale | 100%   | 50% minimum |

## Troubleshooting

### Low Frame Rate

If FPS drops below 90:

1. Dynamic resolution will automatically reduce render resolution
2. LOD distances will be adjusted to show lower detail sooner
3. View distance culling will be more aggressive

### High Draw Calls

If draw calls exceed 5000:

1. Check for excessive small meshes (use instancing)
2. Combine static meshes where possible
3. Reduce particle emitter counts
4. Use LOD groups for vegetation

### Inconsistent Frame Timing

If frame times vary significantly:

1. Check for expensive Tick functions
2. Reduce physics complexity
3. Optimize Blueprint execution
4. Use async loading for assets

## Console Commands

Manually adjust settings via console:

```
# View current performance stats
stat fps
stat unit
stat rhi

# Adjust resolution
r.ScreenPercentage 80

# Adjust LOD
r.LODDistanceScale 0.8

# View draw calls
stat scenerendering

# Force garbage collection
obj gc
```

## Integration with Other Systems

### Terrain System Integration

```cpp
// Adjust terrain LOD based on performance
if (!Optimizer->IsMeetingPerformanceTarget())
{
    TerrainSystem->SetLODDistanceScale(0.7f);
    TerrainSystem->ReduceDetailLevel();
}
```

### Farming System Integration

```cpp
// Reduce crop instance counts if performance is poor
if (Optimizer->GetCurrentMetrics().CurrentFPS < 80.0f)
{
    FarmingSystem->SetInstanceCullingDistance(5000.0f);
    FarmingSystem->ReduceVisualComplexity();
}
```

## Best Practices

1. **Monitor Continuously**: Check performance metrics every frame
2. **Gradual Adjustments**: Don't make sudden quality changes
3. **User Feedback**: Show performance warnings to players
4. **Profile Regularly**: Use Unreal Insights to find bottlenecks
5. **Test on Target Hardware**: VR performance varies by headset

## Performance Budget

Recommended frame time budget (11.1ms total):

- Game Logic: 2ms
- Physics: 1ms
- Animation: 1ms
- Rendering: 6ms
- GPU Wait: 1ms
- Margin: 0.1ms

## Example: Complete Setup

```cpp
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Get VR performance optimizer
    UVRPerformanceOptimizer* Optimizer = GetWorld()->GetSubsystem<UVRPerformanceOptimizer>();

    if (Optimizer)
    {
        // Configure for high-quality VR
        FVRPerformanceConfig Config;
        Config.TargetFPS = 90.0f;
        Config.bEnableDynamicResolution = true;
        Config.MinResolutionScale = 0.6f;
        Config.MaxResolutionScale = 1.2f;
        Config.bEnableAggressiveLOD = true;

        Optimizer->SetPerformanceConfig(Config);

        UE_LOG(LogTemp, Log, TEXT("VR Performance Optimization Enabled"));
    }
}

void AMyGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Monitor performance
    UVRPerformanceOptimizer* Optimizer = GetWorld()->GetSubsystem<UVRPerformanceOptimizer>();
    if (Optimizer)
    {
        FVRPerformanceMetrics Metrics = Optimizer->GetCurrentMetrics();

        // Show warning if performance is poor
        if (!Metrics.bMeetingPerformanceTarget)
        {
            ShowPerformanceWarning(Metrics);
        }
    }
}
```

## Next Steps

1. Test in VR headset with target hardware
2. Profile with Unreal Insights
3. Optimize specific bottlenecks
4. Adjust configuration based on testing
5. Implement quality presets for different hardware tiers
