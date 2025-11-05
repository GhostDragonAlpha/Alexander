# Example: Creating a Crop with Growth Stages

## Overview

This guide shows how to create a crop definition with multiple growth stages in Unreal Engine.

## Step 1: Create Static Meshes for Each Stage

Create 4-6 static meshes representing different growth stages:

### Example: Wheat Crop

1. **SM_Wheat_Seed** (Stage 1)

   - Small brown seed mesh
   - Scale: 5cm x 5cm x 2cm
   - Very simple geometry

2. **SM_Wheat_Sprout** (Stage 2)

   - Tiny green shoots
   - Scale: 10cm x 10cm x 8cm
   - 2-3 small leaves

3. **SM_Wheat_Seedling** (Stage 3)

   - Young wheat plant
   - Scale: 20cm x 20cm x 25cm
   - Multiple leaves, thin stalk

4. **SM_Wheat_Vegetative** (Stage 4)

   - Growing wheat
   - Scale: 30cm x 30cm x 50cm
   - Full foliage, thicker stalk

5. **SM_Wheat_Heading** (Stage 5)

   - Wheat head forming
   - Scale: 35cm x 35cm x 70cm
   - Visible grain head

6. **SM_Wheat_Mature** (Stage 6)
   - Ready for harvest
   - Scale: 40cm x 40cm x 80cm
   - Golden grain head, full size

## Step 2: Create Crop Definition Data Asset

1. In Unreal Editor, right-click in Content Browser
2. Select **Miscellaneous → Data Asset**
3. Choose **CropDefinition** as the class
4. Name it `DA_Crop_Wheat`

## Step 3: Configure Crop Properties

### Identity

```
Crop Name: "Wheat"
Crop Description: "A staple grain crop that grows well in temperate climates"
Crop Icon: T_Icon_Wheat
```

### Visual Representation

```
Crop Mesh: SM_Wheat_Mature (fallback mesh)
Growth Stage Meshes:
  [0] SM_Wheat_Seed
  [1] SM_Wheat_Sprout
  [2] SM_Wheat_Seedling
  [3] SM_Wheat_Vegetative
  [4] SM_Wheat_Heading
  [5] SM_Wheat_Mature
Crop Material: M_Wheat
```

### Growth Requirements

```
Growth Duration: 600.0 (10 minutes real-time)
Optimal Temperature: Min: 15.0, Max: 25.0 (Celsius)
Optimal Humidity: Min: 0.4, Max: 0.7
Water Consumption: 15.0 (liters per day)
Soil Quality Requirement: 0.6
Sunlight Requirement: 0.8 (needs full sun)
```

### Biome Compatibility

```
Suitable Biomes:
  - Grassland
  - Temperate
Biome Adaptability: 0.6
```

### Harvest

```
Yield Amount: Min: 8, Max: 15
Harvest Item ID: "wheat_grain"
Sell Price: 5.0
Regrowth Chance: 0.0 (doesn't regrow)
```

### Special Properties

```
Requires Fertilizer: false
Requires Irrigation: true
Resistant to Pests: false
Resistant to Drought: false
Fertility Depletion: 0.08
```

## Step 4: Test in Game

### Blueprint Testing

```cpp
// Get crop definition
UCropDefinition* WheatCrop = LoadObject<UCropDefinition>(nullptr,
    TEXT("/Game/Farming/Crops/DA_Crop_Wheat"));

// Plant in farm plot
AFarmPlot* FarmPlot = GetFarmPlot();
FarmPlot->PlantCrop(FIntPoint(5, 5), WheatCrop);

// Monitor growth stages
float Progress = 0.0f;
while (Progress < 1.0f)
{
    // Get current stage
    int32 Stage = WheatCrop->GetGrowthStageIndex(Progress);
    FString StageName = WheatCrop->GetGrowthStageName(Progress);

    UE_LOG(LogTemp, Log, TEXT("Wheat at %.1f%% - Stage %d: %s"),
        Progress * 100.0f, Stage, *StageName);

    Progress += 0.1f; // Simulate growth
}
```

## Step 5: Visual Verification

### In-Editor Checklist

1. ✅ All stage meshes load correctly
2. ✅ Meshes have consistent pivot points (bottom center)
3. ✅ Scale progression looks natural
4. ✅ Materials apply correctly to all stages
5. ✅ Collision is appropriate for each stage
6. ✅ LODs are set up for performance

### In-Game Checklist

1. ✅ Crop plants successfully
2. ✅ Growth progresses through all stages
3. ✅ Stage transitions are smooth
4. ✅ Meshes swap at correct progress points
5. ✅ Scale increases naturally
6. ✅ Colors transition appropriately
7. ✅ Harvest works at 100% growth

## Example: Tomato Crop (4 Stages)

### Simplified Configuration

```
Growth Stage Meshes:
  [0] SM_Tomato_Seed       (0-25%)
  [1] SM_Tomato_Seedling   (25-50%)
  [2] SM_Tomato_Flowering  (50-75%)
  [3] SM_Tomato_Fruiting   (75-100%)

Growth Duration: 480.0 (8 minutes)
Optimal Temperature: Min: 18.0, Max: 28.0
Optimal Humidity: Min: 0.5, Max: 0.8
Water Consumption: 20.0
Soil Quality Requirement: 0.7
Sunlight Requirement: 0.9
```

## Example: Potato Crop (5 Stages)

### Underground Growth

```
Growth Stage Meshes:
  [0] SM_Potato_Seed       (0-20%)   - Just planted
  [1] SM_Potato_Sprout     (20-40%)  - Shoots emerging
  [2] SM_Potato_Vegetative (40-60%)  - Leafy growth
  [3] SM_Potato_Flowering  (60-80%)  - Flowers blooming
  [4] SM_Potato_Mature     (80-100%) - Ready to dig

Growth Duration: 720.0 (12 minutes)
Optimal Temperature: Min: 12.0, Max: 22.0
Optimal Humidity: Min: 0.5, Max: 0.7
Water Consumption: 18.0
Soil Quality Requirement: 0.65
Sunlight Requirement: 0.7
```

## Best Practices

### Mesh Creation

1. **Consistent Pivot**: All stage meshes should have pivot at ground level, centered
2. **Scale Progression**: Each stage should be noticeably larger than previous
3. **Polygon Count**: Keep early stages low-poly, increase detail for later stages
4. **Materials**: Use same material setup across all stages for consistency
5. **Collision**: Simple collision for early stages, more detailed for mature

### Stage Count Guidelines

- **Fast Crops** (< 5 min): 4 stages
- **Medium Crops** (5-15 min): 5-6 stages
- **Slow Crops** (> 15 min): 6-8 stages

### Visual Design

1. **Color Progression**:

   - Early: Pale/light colors
   - Mid: Vibrant greens
   - Late: Rich, saturated colors

2. **Size Progression**:

   - Stage 1: 10-15% of final size
   - Stage 2: 25-35% of final size
   - Stage 3: 50-60% of final size
   - Stage 4: 75-85% of final size
   - Stage 5+: 90-100% of final size

3. **Detail Progression**:
   - Early: Simple shapes
   - Mid: More detail, leaves
   - Late: Full detail, flowers/fruit

## Troubleshooting

### Meshes Not Appearing

**Problem**: Crops don't show up in farm plot
**Solution**:

- Check that GrowthStageMeshes array is populated
- Verify meshes are valid static mesh assets
- Ensure CropMesh fallback is set

### Stage Transitions Too Abrupt

**Problem**: Sudden size changes between stages
**Solution**:

- Ensure stage meshes have similar proportions
- Check that scale multipliers are set correctly
- Verify smooth interpolation is enabled

### Wrong Mesh Displaying

**Problem**: Incorrect mesh for growth progress
**Solution**:

- Verify stage ranges (0-1) are correct
- Check GetGrowthStageIndex() logic
- Ensure meshes are in correct array order

### Performance Issues

**Problem**: Frame rate drops with many crops
**Solution**:

- Use LODs on stage meshes
- Reduce polygon count on early stages
- Enable instanced static mesh rendering
- Limit visual update frequency

## Advanced: Custom Stage Logic

### Override Stage Selection

```cpp
// In custom crop class
UStaticMesh* UCustomCrop::GetGrowthStageMesh(float GrowthProgress) const
{
    // Custom logic for stage selection
    if (GrowthProgress < 0.1f)
        return SeedMesh;
    else if (GrowthProgress < 0.5f)
        return GrowingMesh;
    else if (GrowthProgress < 0.9f)
        return FloweringMesh;
    else
        return MatureMesh;
}
```

### Dynamic Stage Count

```cpp
// Adjust stages based on environment
int32 NumStages = 4;
if (bRichSoil)
    NumStages = 6; // More detailed growth in good conditions

TArray<FGrowthStage> Stages = UCropGrowthStageManager::GenerateDefaultStages(
    CropDef,
    NumStages
);
```

## Summary

Creating crops with growth stages:

1. ✅ Create 4-6 static meshes per crop
2. ✅ Configure CropDefinition data asset
3. ✅ Set growth parameters
4. ✅ Test stage progression in-game
5. ✅ Verify visual quality
6. ✅ Optimize for performance

The growth stage system automatically handles:

- Stage detection and transitions
- Visual updates and mesh swapping
- Progress tracking
- Event notifications
- Performance optimization
