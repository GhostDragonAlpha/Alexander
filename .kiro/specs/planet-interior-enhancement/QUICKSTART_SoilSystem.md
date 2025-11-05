# Quick Start: Soil System

## Overview

The soil system simulates realistic soil properties including nutrients, fertility, and visual representation. This guide will help you quickly understand and use the soil system in your farming operations.

## Key Concepts

### Soil Properties

**Primary Properties**:

- **Soil Quality** (0-1): Overall soil health, calculated from all factors
- **Fertility** (0-1): General productivity, depletes with use
- **Water Level** (0-1): Moisture content, depletes over time

**Nutrients (NPK)**:

- **Nitrogen** (0-1): For leafy growth, depletes fastest
- **Phosphorus** (0-1): For root development
- **Potassium** (0-1): For fruiting and flowering

**Structure**:

- **Organic Matter** (0-1): Improves soil structure and water retention
- **Soil pH** (4.0-9.0): Affects nutrient availability (optimal: 6.5)
- **Compaction** (0-1): Affects root growth (lower is better)

## Quick Setup

### 1. Create a Farm Plot

```cpp
// Spawn a farm plot
AFarmPlot* FarmPlot = GetWorld()->SpawnActor<AFarmPlot>(Location, Rotation);

// Set plot size (in centimeters)
FarmPlot->SetPlotSize(FVector2D(1000.0f, 1000.0f)); // 10m x 10m

// Set initial soil quality based on biome
FarmPlot->SetInitialSoilQuality(0.7f); // Good quality soil
```

### 2. Check Soil Status

```cpp
// Get overall soil quality
float Quality = FarmPlot->CalculateSoilQuality();

// Get nutrient levels
FVector Nutrients = FarmPlot->GetNutrientLevels();
float Nitrogen = Nutrients.X;
float Phosphorus = Nutrients.Y;
float Potassium = Nutrients.Z;

// Check individual properties
float Fertility = FarmPlot->Fertility;
float WaterLevel = FarmPlot->WaterLevel;
float OrganicMatter = FarmPlot->OrganicMatter;
```

### 3. Apply Fertilizers

```cpp
// Generic fertilizer (restores all nutrients)
FarmPlot->FertilizePlot(0.3f); // Adds 30% to all nutrients

// Specific NPK fertilizer
FarmPlot->ApplyFertilizer(
    0.5f,  // Nitrogen
    0.2f,  // Phosphorus
    0.3f   // Potassium
);

// Add organic matter (improves structure)
FarmPlot->AddOrganicMatter(0.2f);
```

## Visual Indicators

### Soil Color Guide

**Dark Brown/Black**: Rich, healthy soil

- High organic matter
- Good nutrient levels
- Optimal quality

**Medium Brown**: Average soil

- Moderate nutrients
- Acceptable quality
- May need fertilization

**Light Tan/Gray**: Poor soil

- Low nutrients (especially nitrogen)
- High compaction
- Needs improvement

**Very Dark (Wet)**: High moisture

- Good water level
- Recently watered
- Optimal for growth

## Common Scenarios

### Scenario 1: Starting a New Farm

```cpp
// 1. Check initial soil quality
float Quality = FarmPlot->CalculateSoilQuality();

// 2. If quality is low, improve it
if (Quality < 0.5f)
{
    FarmPlot->FertilizePlot(0.4f);
    FarmPlot->AddOrganicMatter(0.3f);
}

// 3. Water the plot
FarmPlot->WaterPlot(0.5f);

// 4. Plant crops
FarmPlot->PlantCrop(FIntPoint(0, 0), CropDefinition);
```

### Scenario 2: Maintaining Soil Health

```cpp
// Monitor nutrients during growth
FVector Nutrients = FarmPlot->GetNutrientLevels();

// Apply targeted fertilizer if needed
if (Nutrients.X < 0.3f) // Low nitrogen
{
    FarmPlot->ApplyFertilizer(0.5f, 0.0f, 0.0f);
}

if (Nutrients.Y < 0.3f) // Low phosphorus
{
    FarmPlot->ApplyFertilizer(0.0f, 0.5f, 0.0f);
}

if (Nutrients.Z < 0.3f) // Low potassium
{
    FarmPlot->ApplyFertilizer(0.0f, 0.0f, 0.5f);
}
```

### Scenario 3: Post-Harvest Recovery

```cpp
// After harvesting, soil is depleted
FHarvestResult Result = FarmPlot->HarvestCrop(GridPosition);

// Restore soil for next planting
FarmPlot->FertilizePlot(0.3f);        // Restore nutrients
FarmPlot->AddOrganicMatter(0.2f);     // Improve structure
FarmPlot->WaterPlot(0.5f);            // Add water

// Or let it regenerate naturally (slower)
// Natural regeneration: ~0.6% per minute
```

### Scenario 4: Dealing with Compaction

```cpp
// Check compaction level
float Compaction = FarmPlot->Compaction;

if (Compaction > 0.5f)
{
    // Add organic matter to reduce compaction
    FarmPlot->AddOrganicMatter(0.4f);

    // Or leave field fallow to naturally recover
    // (compaction reduces when no crops are planted)
}
```

## Soil Management Tips

### Optimal Ranges

- **Soil Quality**: Keep above 0.6 for best results
- **Nitrogen**: 0.5-0.8 (depletes fastest)
- **Phosphorus**: 0.4-0.7
- **Potassium**: 0.4-0.7
- **Organic Matter**: 0.4-0.8
- **Compaction**: Below 0.4
- **Water Level**: 0.4-0.8

### Fertilization Strategy

**Early Game** (Limited Resources):

- Use generic fertilizer
- Focus on high-quality locations
- Allow natural regeneration

**Mid Game** (Better Resources):

- Use targeted NPK fertilizers
- Monitor specific nutrients
- Add organic matter regularly

**Late Game** (Automated):

- Automated fertilizer systems
- Continuous monitoring
- Optimal nutrient balance

### Crop Rotation Benefits

Different crops deplete different nutrients:

- **Leafy Crops**: High nitrogen use
- **Root Crops**: High phosphorus use
- **Fruiting Crops**: High potassium use

Rotate crops to balance nutrient depletion!

## Depletion Rates

### During Growth

- **Nitrogen**: 15% of growth duration
- **Phosphorus**: 10% of growth duration
- **Potassium**: 12% of growth duration
- **Organic Matter**: 5% of growth duration

### After Harvest

- **Nitrogen**: -0.1
- **Phosphorus**: -0.05
- **Potassium**: -0.05
- **Fertility**: -0.05
- **Compaction**: +0.02

### Natural Regeneration

- **Rate**: 0.01% per second (0.6% per minute)
- **Organic Bonus**: Up to +50% with high organic matter
- **Maximum Natural**: N=0.8, P=0.7, K=0.7

## Blueprint Usage

### Get Soil Information

```cpp
// In Blueprint, call these functions:
float Quality = FarmPlot->CalculateSoilQuality();
FVector Nutrients = FarmPlot->GetNutrientLevels();

// Access properties directly:
float Nitrogen = FarmPlot->NitrogenLevel;
float Phosphorus = FarmPlot->PhosphorusLevel;
float Potassium = FarmPlot->PotassiumLevel;
float Organic = FarmPlot->OrganicMatter;
float pH = FarmPlot->SoilPH;
```

### Apply Treatments

```cpp
// Generic fertilizer
FarmPlot->FertilizePlot(0.3);

// Specific NPK
FarmPlot->ApplyFertilizer(0.5, 0.2, 0.3);

// Organic matter
FarmPlot->AddOrganicMatter(0.2);

// Water
FarmPlot->WaterPlot(0.5);
```

## Troubleshooting

### Problem: Crops Growing Slowly

**Solution**: Check soil quality and nutrients

```cpp
float Quality = FarmPlot->CalculateSoilQuality();
if (Quality < 0.5f)
{
    FarmPlot->FertilizePlot(0.4f);
}
```

### Problem: Soil Color Looks Wrong

**Solution**: Soil color reflects actual conditions

- Light/gray = Low nutrients or high compaction
- Dark = High quality and organic matter
- Very dark = High moisture

### Problem: Nutrients Depleting Too Fast

**Solution**: This is normal! Options:

1. Apply fertilizer more frequently
2. Use crops with lower nutrient requirements
3. Add organic matter to slow depletion
4. Rotate crops to balance nutrient use

### Problem: Compaction Too High

**Solution**:

1. Add organic matter: `FarmPlot->AddOrganicMatter(0.4f)`
2. Leave field fallow (no crops) to naturally recover
3. Avoid excessive harvesting in same location

## Performance Notes

- Soil calculations are lightweight (simple arithmetic)
- Visual updates occur every 1 second (not every frame)
- Minimal memory overhead (~36 bytes per farm plot)
- No performance impact on gameplay

## Next Steps

1. **Integrate with Biomes**: Different biomes provide different base soil quality
2. **Add UI**: Display soil information to players
3. **Automate**: Create automated fertilizer systems
4. **Balance**: Tune depletion rates for gameplay

## Related Systems

- **Crop Growth System**: Affected by soil quality
- **Weather System**: Rain provides natural watering
- **Biome System**: Determines base soil properties
- **Farming Subsystem**: Manages multiple farm plots

## Summary

The soil system provides:
✓ Realistic nutrient tracking (NPK)
✓ Dynamic fertility depletion
✓ Multiple fertilization options
✓ Visual feedback through soil color
✓ Natural regeneration mechanics
✓ Integration with crop growth

Use this system to create engaging farming gameplay with strategic resource management!
