# Terrain Rendering Optimization - Quick Start Guide

## Overview

The Terrain Rendering Optimizer implements advanced rendering techniques to achieve 90+ FPS in VR:

- **Nanite**: Virtualized geometry for terrain meshes
- **GPU Instancing**: Efficient rendering of vegetation and detail objects
- **Frustum Culling**: Only render objects in camera view
- **Draw Call Batching**: Reduce CPU overhead

## Quick Setup

### 1. Enable Nanite for Terrain

```cpp
// In your terrain actor or component
UTerrainRenderingOptimizer* Optimizer = NewObject<UTerrainRenderingOptimizer>(this);
Optimizer->Initialize(this);

// Enable Nanite for a terrain mesh component
UStaticMeshComponent* TerrainMesh = GetTerrainMeshComponent();
Optimizer->EnableNaniteForMesh(TerrainMesh);
```

### 2. Add GPU Instancing for Vegetation

```cpp
// Create instances for vegetation
TArray<FTransform> TreeTransforms;
// ... populate transforms ...

Optimizer->AddInstances(
    TreeMesh,
    TreeMaterial,
    TreeTransforms,
    0 // LOD level
);

// Update all instance components
Optimizer->UpdateInstanceComponents();
```

### 3. Enable Frustum Culling

```cpp
// In your Tick function
void ATerrain::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Get camera info
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && Optimizer)
    {
        FVector CameraLocation;
        FRotator CameraRotation;
        PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

        // Update culling
        Optimizer->UpdateFrustumCulling(CameraLocation, CameraRotation, 90.0f);
    }
}
```

### 4. Optimize Draw Calls

```cpp
// After adding all instances
Optimizer->OptimizeDrawCalls();

// Check performance
int32 DrawCalls, Triangles, Instances, Culled;
Optimizer->GetPerformanceStats(DrawCalls, Triangles, Instances, Culled);

UE_LOG(LogTemp, Log, TEXT("Draw Calls: %d, Triangles: %d"), DrawCalls, Triangles);
```

## Configuration

### Nanite Settings

```cpp
FNaniteTerrainConfig NaniteConfig;
NaniteConfig.bEnableNanite = true;
NaniteConfig.PositionPrecision = 0.01f; // cm
NaniteConfig.bPreserveArea = true;

Optimizer->NaniteConfig = NaniteConfig;
```

### Instancing Settings

```cpp
FGPUInstancingConfig InstancingConfig;
InstancingConfig.bEnableInstancing = true;
InstancingConfig.bUseHierarchicalInstancing = true;
InstancingConfig.MaxInstancesPerComponent = 10000;
InstancingConfig.CullDistance = 1000000.0f; // 10km

Optimizer->InstancingConfig = InstancingConfig;
```

### Culling Settings

```cpp
FFrustumCullingConfig CullingConfig;
CullingConfig.bEnableFrustumCulling = true;
CullingConfig.bEnableOcclusionCulling = true;
CullingConfig.MaxRenderDistance = 5000000.0f; // 50km

Optimizer->CullingConfig = CullingConfig;
```

## Integration with QuadTreeTileManager

The optimizer is automatically integrated with the QuadTree system:

```cpp
// In QuadTreeTileManager initialization
UQuadTreeTileManager* TileManager = NewObject<UQuadTreeTileManager>(this);
TileManager->OwnerActor = this;
TileManager->Initialize(RootPosition, RootSize, LODConfig);

// Optimizer is created automatically
// Access it via TileManager->RenderingOptimizer
```

## Performance Monitoring

```cpp
// Log performance stats
Optimizer->LogPerformanceStats();

// Output:
// === Terrain Rendering Performance Stats ===
//   Draw Calls: 45
//   Triangles: 2,450,000
//   Instances: 15,000
//   Culled Objects: 120
//   Instance Batches: 8
//   Tracked Components: 165
// ==========================================
```

## Best Practices

### 1. Nanite Usage

- Enable Nanite for all terrain meshes
- Use high-poly source meshes (Nanite handles LOD automatically)
- Disable manual LOD generation for Nanite meshes

### 2. Instancing

- Batch similar objects together (same mesh + material)
- Use hierarchical instancing for better culling
- Keep instance counts under 10,000 per component

### 3. Culling

- Update culling at 10 Hz (every 0.1 seconds)
- Use occlusion culling for dense environments
- Set appropriate max render distances per object type

### 4. Draw Call Optimization

- Group objects by material
- Use material instances instead of unique materials
- Enable static mesh merging for static objects

## Troubleshooting

### Nanite Not Working

- Check if platform supports Nanite (DX12/Vulkan required)
- Verify mesh has enough triangles (minimum ~1000)
- Check console for Nanite warnings

### Low Performance

- Reduce max render distance
- Increase culling update frequency
- Lower instance counts per component
- Check draw call count (should be < 5000)

### Popping/Flickering

- Increase culling margin
- Smooth LOD transitions
- Check frustum calculation accuracy

## Example: Complete Setup

```cpp
void APlanet::InitializeTerrainRendering()
{
    // Create optimizer
    RenderingOptimizer = NewObject<UTerrainRenderingOptimizer>(this);
    RenderingOptimizer->Initialize(this);

    // Configure Nanite
    RenderingOptimizer->NaniteConfig.bEnableNanite = true;
    RenderingOptimizer->NaniteConfig.PositionPrecision = 0.01f;

    // Configure instancing
    RenderingOptimizer->InstancingConfig.MaxInstancesPerComponent = 10000;
    RenderingOptimizer->InstancingConfig.CullDistance = 1000000.0f;

    // Configure culling
    RenderingOptimizer->CullingConfig.MaxRenderDistance = 5000000.0f;
    RenderingOptimizer->CullingConfig.UpdateFrequency = 0.1f;

    // Enable Nanite for terrain tiles
    for (UStaticMeshComponent* TileMesh : TerrainTileMeshes)
    {
        RenderingOptimizer->EnableNaniteForMesh(TileMesh);
    }

    // Add vegetation instances
    TArray<FTransform> VegetationTransforms = GenerateVegetationPlacements();
    RenderingOptimizer->AddInstances(
        VegetationMesh,
        VegetationMaterial,
        VegetationTransforms,
        0
    );

    // Update components
    RenderingOptimizer->UpdateInstanceComponents();
    RenderingOptimizer->OptimizeDrawCalls();

    UE_LOG(LogTemp, Log, TEXT("Terrain rendering optimization initialized"));
}

void APlanet::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update culling
    if (RenderingOptimizer)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            FVector CameraLocation;
            FRotator CameraRotation;
            PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

            RenderingOptimizer->UpdateFrustumCulling(
                CameraLocation,
                CameraRotation,
                90.0f
            );
        }
    }
}
```

## Performance Targets

With proper optimization, you should achieve:

- **Frame Rate**: 90+ FPS in VR
- **Draw Calls**: < 5,000 per frame
- **Visible Triangles**: < 10 million
- **Instance Count**: 100,000+ with GPU instancing
- **Culling Efficiency**: 60-80% objects culled when not in view

## Next Steps

1. Profile your scene with Unreal Insights
2. Adjust culling distances based on performance
3. Fine-tune Nanite settings for quality/performance balance
4. Implement dynamic LOD for non-Nanite objects
5. Add occlusion culling for indoor areas
