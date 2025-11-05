# Quick Start: Biome Soil Variations (Task 14.2)

## Overview

The Biome Soil Variations system provides biome-specific soil properties that affect farming gameplay. Each biome has unique soil characteristics including quality, color, texture, moisture, and water table depth.

## Key Components

### 1. BiomeSoilSystem (UBiomeSoilSystem)

Central system for managing biome-specific soil variations.

**Location**: `Source/Alexander/Public/BiomeSoilSystem.h`

**Key Methods**:

- `Initialize(BiomeDefinitions)` - Initialize with planet's biomes
- `GetSoilVariationForBiome(BiomeType)` - Get soil data for a biome type
- `GetBlendedSoilVariation(BiomeWeights, BiomeDefinitions)` - Blend multiple biomes
- `CalculateSoilQualityWithVariation(BaseSoilQuality, Seed)` - Add local variation
- `GetModifiedSoilColor(BaseColor, Moisture, Health)` - Calculate visual color

### 2. FSoilVariation Structure

Contains all soil properties for a location.

**Properties**:

- `SoilQuality` (0.3-1.0) - Base farming quality per Requirement 12.2
- `SoilColor` - Visual color for soil rendering
- `Roughness` (0-1) - Texture appearance (sandy vs clay)
- `Moisture` (0-1) - Base moisture level
- `Drainage` (0-1) - Water drainage rate
- `WaterTableDepth` (meters) - Depth to groundwater per Requirement 12.5
- `BiomeType` - Associated biome type
- `BiomeName` - Human-readable biome name

### 3. Biome Definition Extensions

Added to `FBiomeDefinition` in `PlanetConfiguration.h`:

```cpp
UPROPERTY(EditAnywhere, Category = "Farming")
float BaseSoilQuality = 0.5f; // 0.3-1.0 range

UPROPERTY(EditAnywhere, Category = "Farming")
float WaterTableDepth = 10.0f; // meters

UPROPERTY(EditAnywhere, Category = "Farming")
FLinearColor SoilColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);

UPROPERTY(EditAnywhere, Category = "Farming")
float SoilRoughness = 0.8f; // 0-1

UPROPERTY(EditAnywhere, Category = "Farming")
float SoilMoisture = 0.5f; // 0-1

UPROPERTY(EditAnywhere, Category = "Farming")
float SoilDrainage = 0.5f; // 0-1
```

## Default Soil Properties by Biome

### Desert

- **Quality**: 0.3 (Poor)
- **Color**: Sandy tan (0.8, 0.7, 0.5)
- **Roughness**: 0.9 (Very sandy)
- **Moisture**: 0.1 (Very dry)
- **Drainage**: 0.9 (Fast)
- **Water Table**: 50m (Deep)

### Grassland

- **Quality**: 0.7 (Good)
- **Color**: Rich brown (0.4, 0.3, 0.2)
- **Roughness**: 0.5 (Loamy)
- **Moisture**: 0.5 (Moderate)
- **Drainage**: 0.5 (Moderate)
- **Water Table**: 10m (Moderate)

### Forest

- **Quality**: 0.8 (High)
- **Color**: Dark brown (0.3, 0.25, 0.15)
- **Roughness**: 0.4 (Rich loam)
- **Moisture**: 0.7 (Moist)
- **Drainage**: 0.4 (Slow)
- **Water Table**: 5m (Shallow)

### Tundra

- **Quality**: 0.4 (Poor)
- **Color**: Gray (0.5, 0.5, 0.5)
- **Roughness**: 0.6 (Rocky)
- **Moisture**: 0.6 (Moist from permafrost)
- **Drainage**: 0.2 (Very slow)
- **Water Table**: 2m (Shallow/permafrost)

### Volcanic

- **Quality**: 0.9 (Excellent)
- **Color**: Very dark brown/black (0.2, 0.15, 0.1)
- **Roughness**: 0.7 (Ashy/rocky)
- **Moisture**: 0.4 (Moderate)
- **Drainage**: 0.7 (Good)
- **Water Table**: 15m (Variable)

### Savanna

- **Quality**: 0.6 (Moderate)
- **Color**: Reddish brown (0.6, 0.5, 0.3)
- **Roughness**: 0.7 (Sandy loam)
- **Moisture**: 0.3 (Dry)
- **Drainage**: 0.6 (Good)
- **Water Table**: 20m (Deep)

### Rainforest

- **Quality**: 0.7 (Good)
- **Color**: Rich dark brown (0.35, 0.25, 0.15)
- **Roughness**: 0.3 (Clay-rich)
- **Moisture**: 0.9 (Very moist)
- **Drainage**: 0.3 (Poor)
- **Water Table**: 3m (Very shallow)

### Alpine

- **Quality**: 0.4 (Poor)
- **Color**: Light brown/gray (0.5, 0.45, 0.4)
- **Roughness**: 0.8 (Rocky)
- **Moisture**: 0.5 (Moderate)
- **Drainage**: 0.8 (Fast/rocky)
- **Water Table**: 30m (Deep/mountainous)

### Swamp

- **Quality**: 0.5 (Moderate)
- **Color**: Dark muddy brown (0.25, 0.25, 0.2)
- **Roughness**: 0.2 (Very fine/muddy)
- **Moisture**: 1.0 (Saturated)
- **Drainage**: 0.1 (Very poor)
- **Water Table**: 0.5m (At/near surface)

## Usage Examples

### Getting Soil Variation at Location

```cpp
// In FarmingSubsystem or similar
UFarmingSubsystem* FarmingSystem = GetWorld()->GetSubsystem<UFarmingSubsystem>();
FSoilVariation SoilVar = FarmingSystem->GetSoilVariationAtLocation(Location, PlanetActor);

UE_LOG(LogTemp, Log, TEXT("Soil Quality: %.2f, Biome: %s"),
    SoilVar.SoilQuality, *SoilVar.BiomeName);
```

### Applying Soil Variation to Farm Plot

```cpp
// When creating a farm plot
AFarmPlot* FarmPlot = FarmingSystem->CreateFarmPlot(Location, Size, PlanetActor);
// Soil variation is automatically applied during creation

// Or manually apply
FSoilVariation SoilVar = FarmingSystem->GetSoilVariationAtLocation(Location, PlanetActor);
FarmPlot->ApplyBiomeSoilVariation(SoilVar);
```

### Getting Soil Texture Description

```cpp
UBiomeSoilSystem* SoilSystem = FarmingSystem->GetBiomeSoilSystem();
FString TextureDesc = SoilSystem->GetSoilTextureDescription(SoilVar);
// Returns: "Sandy, Dry" or "Loamy" or "Clay, Wet" etc.
```

### Calculating Water Availability

```cpp
UBiomeSoilSystem* SoilSystem = FarmingSystem->GetBiomeSoilSystem();
float WaterAvail = SoilSystem->CalculateWaterAvailability(
    SoilVar.WaterTableDepth,
    RecentRainfall
);
// Returns 0-1 based on water table depth and recent rain
```

## Integration with Farm Plots

When a farm plot is created, it automatically receives biome soil variations:

1. **Soil Quality**: Sets base farming productivity (0.3-1.0)
2. **Soil Color**: Used for visual rendering of soil mesh
3. **Compaction**: Calculated from roughness (affects root growth)
4. **Water Level**: Initialized from moisture
5. **Drainage**: Affects evaporation rate
6. **Nutrient Levels**: Scaled by soil quality
   - Nitrogen: Quality × 0.9
   - Phosphorus: Quality × 0.85
   - Potassium: Quality × 0.8
7. **Organic Matter**: Quality × 0.7

## Visual Feedback

Soil color changes based on:

- **Base Color**: From biome definition
- **Moisture**: Darkens soil when wet (up to 30% darker)
- **Health**: Desaturates to gray when unhealthy (up to 50%)

```cpp
// Get modified color for rendering
FLinearColor VisualColor = SoilSystem->GetModifiedSoilColor(
    BiomeSoilColor,
    CurrentMoisture,
    SoilHealth
);
```

## Requirements Satisfied

✅ **Requirement 12.2**: Soil quality based on biome type (0.3-1.0 range)
✅ **Requirement 12.5**: Visual feedback through soil color and texture variations
✅ **Water table depth per biome**: Implemented in FSoilVariation
✅ **Soil texture variations**: Roughness, drainage, and moisture properties

## Testing

### Blueprint Testing

1. Create a test map with multiple biomes
2. Place farm plots in different biomes
3. Check soil quality values (should be 0.3-1.0)
4. Verify soil colors match biome types
5. Test water availability in different biomes

### C++ Testing

```cpp
// Test soil quality validation
UBiomeSoilSystem* SoilSystem = NewObject<UBiomeSoilSystem>();
float ValidQuality = SoilSystem->ValidateSoilQuality(1.5f); // Should clamp to 1.0
float InvalidQuality = SoilSystem->ValidateSoilQuality(0.1f); // Should clamp to 0.3

// Test biome defaults
FSoilVariation DesertSoil = SoilSystem->GetSoilVariationForBiome(EBiomeType::Desert);
check(DesertSoil.SoilQuality == 0.3f); // Desert has poor soil
check(DesertSoil.WaterTableDepth == 50.0f); // Deep water table
```

## Future Enhancements

- **Soil pH variation by biome**: Affects nutrient availability
- **Seasonal soil changes**: Temperature and moisture variations
- **Soil erosion**: Quality degradation over time
- **Soil improvement**: Player actions to enhance soil quality
- **Detailed texture rendering**: Use roughness for material parameters

## Related Systems

- **BiomeCompatibilitySystem**: Uses soil quality for crop compatibility
- **CropGrowthCalculator**: Soil quality affects growth rates
- **FarmPlot**: Applies and uses soil variations
- **FarmingSubsystem**: Manages soil system initialization

## Files Modified

- `Source/Alexander/Public/PlanetConfiguration.h` - Added soil properties to FBiomeDefinition
- `Source/Alexander/Public/BiomeSoilSystem.h` - New soil system class
- `Source/Alexander/Private/BiomeSoilSystem.cpp` - Soil system implementation
- `Source/Alexander/Public/FarmingSubsystem.h` - Added soil system integration
- `Source/Alexander/Private/FarmingSubsystem.cpp` - Soil variation queries
- `Source/Alexander/Public/FarmPlot.h` - Added soil variation application
- `Source/Alexander/Private/FarmPlot.cpp` - Apply biome soil properties
