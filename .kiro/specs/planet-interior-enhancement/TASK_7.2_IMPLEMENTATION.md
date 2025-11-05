# Task 7.2 Implementation: Generate Cloud Shapes

## Overview

This task implements realistic cloud shape generation using 3D Perlin-Worley noise, multiple noise octaves for detail, cloud density calculation, and cloud coverage patterns. The implementation creates volumetric clouds with realistic "cauliflower" appearance characteristic of cumulus clouds.

## Implementation Details

### 1. Worley Noise (Cellular Noise) - ProceduralNoiseGenerator

Added three new Worley noise functions to `ProceduralNoiseGenerator`:

#### WorleyNoise3D

```cpp
float WorleyNoise3D(float X, float Y, float Z, int32 Seed, float CellSize)
```

- Generates 3D cellular/Worley noise
- Returns distance to nearest cell center
- Checks 3x3x3 grid of neighboring cells for accuracy
- Used for creating cellular cloud structures

#### WorleyNoise3DMulti

```cpp
void WorleyNoise3DMulti(float X, float Y, float Z, int32 Seed, float CellSize, float& OutF1, float& OutF2)
```

- Returns both F1 (closest) and F2 (second closest) distances
- Useful for advanced cloud effects (F2-F1 creates cell edges)
- Provides more control over cloud appearance

#### GetRandomVector3D

```cpp
FVector GetRandomVector3D(int32 X, int32 Y, int32 Z, int32 Seed)
```

- Helper function for generating random 3D points in cells
- Uses hash function for deterministic randomness
- Ensures consistent results for same seed

### 2. Perlin-Worley Combination

#### PerlinWorleyNoise3D

```cpp
float PerlinWorleyNoise3D(float X, float Y, float Z, int32 Seed, float PerlinScale, float WorleyScale, float WorleyWeight)
```

- Combines Perlin noise (smooth gradients) with Worley noise (cellular structure)
- Perlin provides base cloud shapes
- Worley (inverted) creates cellular "cauliflower" structure
- WorleyWeight controls the blend (0.6 is typical for realistic clouds)
- Formula: `Result = Perlin - (InvertedWorley * Weight)`

#### FractalPerlinWorleyNoise3D

```cpp
float FractalPerlinWorleyNoise3D(float X, float Y, float Z, int32 Seed, int32 Octaves, float Frequency, float Lacunarity, float Persistence, float WorleyWeight)
```

- Multi-octave version for detailed clouds
- Each octave adds finer detail
- Lacunarity (2.0): Frequency multiplier per octave
- Persistence (0.5): Amplitude multiplier per octave
- Creates fractal detail at multiple scales

### 3. Cloud Shape Generation - PlanetCloudComponent

#### Configuration Properties

**Base Shape Parameters:**

- `BaseShapeOctaves` (4): Number of octaves for base cloud shape
- `BaseShapeFrequency` (0.5): Base frequency for large cloud formations
- `BaseShapeLacunarity` (2.0): Frequency multiplier per octave
- `BaseShapePersistence` (0.5): Amplitude multiplier per octave
- `WorleyWeight` (0.6): Weight of Worley noise in combination

**Detail Parameters:**

- `DetailOctaves` (3): Number of octaves for detail noise
- `DetailFrequency` (2.0): Higher frequency for fine details
- `DetailLacunarity` (2.0): Frequency multiplier for detail
- `DetailPersistence` (0.5): Amplitude multiplier for detail

**Density Parameters:**

- `DensityThreshold` (0.4): Minimum density for cloud formation
- `DensityMultiplier` (1.5): Multiplier for cloud density
- `bUseHeightGradient` (true): Apply height-based density gradient
- `HeightGradientBottom` (0.1): Density at bottom of cloud layer
- `HeightGradientTop` (0.1): Density at top of cloud layer

**Coverage Parameters:**

- `CoverageFrequency` (0.2): Frequency of coverage patterns
- `CoverageOctaves` (2): Octaves for coverage variation
- `CoverageVariation` (0.3): Amount of coverage variation across sky

#### GenerateBaseCloudShape

```cpp
float GenerateBaseCloudShape(FVector Position)
```

- Generates large-scale cloud formations
- Applies wind offset for cloud movement
- Uses multi-octave Perlin-Worley noise
- Returns density value [0, 1]

**Process:**

1. Apply wind offset to position for animation
2. Generate fractal Perlin-Worley noise with 4 octaves
3. Creates characteristic cumulus cloud shapes
4. Returns normalized density value

#### GenerateCloudDetail

```cpp
float GenerateCloudDetail(FVector Position)
```

- Generates fine details and wispy edges
- Uses higher frequency noise
- Detail layer moves slightly faster (1.2x) for parallax effect
- Less Worley influence (0.7x) for softer details

**Process:**

1. Apply wind offset with 1.2x multiplier
2. Generate fractal Perlin-Worley with 3 octaves
3. Higher frequency creates fine details
4. Returns detail noise value [0, 1]

#### CalculateCloudDensity

```cpp
float CalculateCloudDensity(FVector Position, float HeightInLayer)
```

- Combines all noise layers and modifiers
- Applies coverage patterns
- Applies height gradient
- Returns final cloud density

**Process:**

1. Generate base cloud shape
2. Generate detail noise
3. Combine: `BaseShape - (Detail * DetailStrength)`
   - Detail erodes base shape, creating wispy edges
4. Apply coverage pattern modulation
5. Apply height gradient (fade at top and bottom)
6. Apply density threshold and multiplier
7. Apply global cloud density parameter
8. Clamp to [0, 1]

**Height Gradient:**

- Bottom 20% of layer: Fade from `HeightGradientBottom` to 1.0
- Middle 60%: Full density
- Top 20% of layer: Fade from 1.0 to `HeightGradientTop`
- Creates realistic cloud layer boundaries

#### GenerateCoveragePattern

```cpp
float GenerateCoveragePattern(FVector2D Position)
```

- Creates large-scale coverage variation
- Uses 2D Perlin noise (horizontal patterns)
- Multiple octaves for natural variation
- Blends with global coverage parameter

**Process:**

1. Generate multi-octave 2D Perlin noise
2. Normalize to [0, 1]
3. Blend with global `CloudCoverage` using `CoverageVariation`
4. Creates areas of more/less cloud coverage
5. Returns coverage value [0, 1]

#### SampleCloudDensityAtPosition

```cpp
float SampleCloudDensityAtPosition(FVector WorldPosition)
```

- Samples cloud density at any world position
- Checks if position is within cloud layer
- Converts to cloud space coordinates
- Returns 0 if outside cloud layer

**Process:**

1. Convert world position to relative position
2. Calculate altitude in km
3. Check if within cloud layer bounds
4. Calculate normalized height in layer [0, 1]
5. Convert to cloud space (km scale)
6. Call `CalculateCloudDensity`
7. Return final density or 0 if outside layer

## Technical Details

### Noise Combination Strategy

The Perlin-Worley combination creates realistic clouds:

1. **Perlin Noise**: Provides smooth, flowing base shapes

   - Good for large-scale cloud formations
   - Creates natural-looking gradients

2. **Worley Noise (Inverted)**: Adds cellular structure

   - Creates "cauliflower" appearance
   - Inverted so cells are high density (clouds)
   - Edges are low density (gaps between clouds)

3. **Combination**: `Perlin - (InvertedWorley * Weight)`
   - Perlin forms the base
   - Worley subtracts to create cellular structure
   - Weight (0.6) balances the two

### Multi-Octave Detail

Using multiple octaves creates fractal detail:

- **Octave 0**: Large cloud formations (low frequency)
- **Octave 1**: Medium details (2x frequency)
- **Octave 2**: Fine details (4x frequency)
- **Octave 3**: Very fine details (8x frequency)

Each octave has half the amplitude of the previous (persistence = 0.5).

### Wind Animation

Clouds animate by offsetting noise sampling position:

- Base shape uses `CurrentWindOffset`
- Detail layer uses `CurrentWindOffset * 1.2` for parallax
- Wind offset accumulates over time based on wind speed
- Wraps at 1000 km to prevent precision issues

### Height Gradient

Clouds fade at layer boundaries for realism:

- **Bottom fade**: Simulates cloud base evaporation
- **Top fade**: Simulates cloud top dissipation
- **Middle**: Full density for main cloud body
- Configurable fade amounts for different cloud types

### Coverage Patterns

Large-scale coverage creates realistic sky:

- Low-frequency 2D noise for coverage variation
- Creates clear areas and cloudy areas
- Blends with global coverage parameter
- `CoverageVariation` controls pattern strength

## Performance Considerations

### Optimization Strategies

1. **Noise Caching**: Consider caching noise values in 3D texture
2. **LOD**: Use fewer octaves for distant clouds
3. **Spatial Hashing**: Only calculate density where needed
4. **GPU Computation**: Move noise generation to material/compute shader

### Current Performance

- CPU-based noise generation
- Suitable for material parameter updates
- For real-time ray marching, consider GPU implementation
- Noise functions are deterministic (same seed = same result)

## Material Integration

The generated cloud shapes should be used in cloud material:

```cpp
// In UpdateCloudMaterialParameters()
CloudMaterialInstance->SetScalarParameterValue("BaseShapeOctaves", BaseShapeOctaves);
CloudMaterialInstance->SetScalarParameterValue("BaseShapeFrequency", BaseShapeFrequency);
CloudMaterialInstance->SetScalarParameterValue("WorleyWeight", WorleyWeight);
CloudMaterialInstance->SetScalarParameterValue("DetailOctaves", DetailOctaves);
CloudMaterialInstance->SetScalarParameterValue("DetailFrequency", DetailFrequency);
CloudMaterialInstance->SetScalarParameterValue("DensityThreshold", DensityThreshold);
CloudMaterialInstance->SetScalarParameterValue("DensityMultiplier", DensityMultiplier);
```

The material shader should implement the same noise functions for GPU-side evaluation.

## Testing

### Test Cases

1. **Basic Shape Generation**

   - Verify base shape produces cloud-like patterns
   - Check detail adds fine structure
   - Confirm density calculation combines correctly

2. **Coverage Patterns**

   - Verify coverage creates variation
   - Check blending with global coverage
   - Confirm patterns are smooth and natural

3. **Height Gradient**

   - Verify fade at bottom of layer
   - Verify fade at top of layer
   - Check middle section has full density

4. **Wind Animation**

   - Verify clouds move with wind
   - Check detail moves faster for parallax
   - Confirm offset wrapping works

5. **Density Calculation**
   - Test threshold removes low-density areas
   - Verify multiplier scales density correctly
   - Check clamping to [0, 1]

### Visual Quality Checks

- Clouds should have "cauliflower" appearance
- Edges should be wispy and detailed
- Coverage should create natural patterns
- Height gradient should be smooth
- Animation should be smooth and continuous

## Requirements Satisfied

This implementation satisfies **Requirement 2.3** from the requirements document:

> "WHILE flying through the atmosphere, THE Planet System SHALL render volumetric clouds with realistic lighting, shadows, and parallax effects"

Specifically, this task implements:

- ✅ 3D Perlin-Worley noise for cloud shapes
- ✅ Multiple noise octaves for detail
- ✅ Cloud density calculation
- ✅ Cloud coverage patterns

## Next Steps

The next task (7.3) will implement cloud lighting and shadows, which will use the density values calculated here to determine light transmission and shadow casting.

## References

- **Perlin Noise**: Ken Perlin's improved noise function
- **Worley Noise**: Steven Worley's cellular texture basis function
- **Perlin-Worley**: Combination technique used in real-time cloud rendering
- **Fractal Brownian Motion**: Multi-octave noise for natural detail
- **GPU Gems 3**: Chapter on volumetric cloud rendering
