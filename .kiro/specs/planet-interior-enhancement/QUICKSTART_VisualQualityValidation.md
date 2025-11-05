# Quick Start: Visual Quality Validation

## Overview

The Visual Quality Validator compares your planet's visual quality against Star Citizen reference standards. It validates atmospheric effects, biome transitions, and material quality to ensure AAA-level visual fidelity.

## Quick Usage

### In Blueprint

1. Add a `VisualQualityValidator` actor to your level
2. Get reference to your planet
3. Call `RunCompleteValidation`
4. Review the quality metrics

```cpp
// Blueprint pseudocode
VisualQualityValidator->RunCompleteValidation(MyPlanet)
Print(Metrics.OverallQuality)
Print(Metrics.StarCitizenComparison)
```

### In C++

```cpp
#include "VisualQualityValidator.h"

// Create validator
AVisualQualityValidator* Validator = GetWorld()->SpawnActor<AVisualQualityValidator>();

// Run validation
FVisualQualityMetrics Metrics = Validator->RunCompleteValidation(MyPlanet);

// Check results
if (Metrics.OverallQuality >= 0.85f)
{
    UE_LOG(LogTemp, Log, TEXT("Visual quality meets Star Citizen standards!"));
}

// Generate report
FString Report = Validator->GenerateValidationReport(Metrics);
UE_LOG(LogTemp, Log, TEXT("%s"), *Report);
```

## What Gets Validated

### 1. Atmospheric Effects (40% weight)

- **Rayleigh Scattering**: Blue sky color from molecular scattering
- **Mie Scattering**: Haze and sunset colors from particle scattering
- **Scale Heights**: Atmospheric density falloff
- **Fog Effects**: Distance fog and height-based density
- **Cloud Quality**: Volumetric cloud detail and realism

**Star Citizen Standard**: 85% minimum quality

### 2. Biome Transitions (30% weight)

- **Blend Smoothness**: Gradual transitions between biomes
- **Transition Zones**: 50-100m smooth blending areas
- **Terrain Continuity**: No abrupt height changes
- **Material Blending**: Seamless texture transitions

**Star Citizen Standard**: 90% minimum smoothness

### 3. Material Quality (30% weight)

- **Texture Resolution**: Minimum 2048x2048 for terrain
- **PBR Completeness**: Base color, normal, roughness, metallic, AO
- **LOD Transitions**: Smooth detail level changes
- **Parameter Configuration**: Proper material setup

**Star Citizen Standard**: 88% minimum detail level

## Quality Metrics Explained

### Overall Quality Score (0-1)

Weighted average of all components:

- 0.90-1.00: Excellent (Star Citizen level)
- 0.80-0.89: Good (AAA quality)
- 0.70-0.79: Acceptable (AA quality)
- Below 0.70: Needs improvement

### Star Citizen Comparison (0-1)

How close you are to Star Citizen reference standards:

- 1.00: Matches or exceeds Star Citizen
- 0.90-0.99: Very close to Star Citizen
- 0.80-0.89: Good but below Star Citizen
- Below 0.80: Significant gap from Star Citizen

## Validation Report

The validator generates a detailed report including:

```
=== Visual Quality Validation Report ===

Overall Quality Score: 87.50%
Star Citizen Comparison: 92.30%

Component Scores:
  Atmospheric Effects: 89.00%
  Biome Transitions: 91.00%
  Material Quality: 83.00%

Quality Issues Found:
  - [WARNING] Material quality (0.83) below standard (0.88)
  - [INFO] LOD transition range too small at level 2

Recommendations:
  - Add higher resolution textures
  - Implement additional PBR material layers

=== End of Report ===
```

## Customizing Standards

You can adjust the quality standards to match your target:

```cpp
// In Blueprint or C++
Validator->QualityStandards.MinAtmosphericScatteringQuality = 0.90f; // Stricter
Validator->QualityStandards.MinBiomeTransitionSmoothness = 0.95f;
Validator->QualityStandards.MinMaterialDetailLevel = 0.92f;
Validator->QualityStandards.MinTextureResolution = 4096; // 4K textures
```

## Individual Component Validation

You can validate specific components separately:

```cpp
// Validate only atmospheric effects
float AtmosphericScore = Validator->ValidateAtmosphericEffects(MyPlanet);

// Validate only biome transitions
TArray<FVector> SamplePoints = GetBiomeBoundaryPoints();
float BiomeScore = Validator->ValidateBiomeTransitions(MyPlanet, SamplePoints);

// Validate only materials
float MaterialScore = Validator->ValidateMaterialQuality(MyPlanet);
```

## Common Issues and Fixes

### Low Atmospheric Quality

**Issue**: Atmospheric scattering not configured properly

**Fix**:

```cpp
AtmosphereComponent->RayleighScatteringCoefficient = FLinearColor(0.0058f, 0.0135f, 0.0331f);
AtmosphereComponent->MieScatteringCoefficient = FLinearColor(0.004f, 0.004f, 0.004f);
AtmosphereComponent->RayleighScaleHeight = 8.0f; // km
AtmosphereComponent->MieScaleHeight = 1.2f; // km
```

### Low Biome Transition Quality

**Issue**: Abrupt changes between biomes

**Fix**:

- Increase transition zone size to 50-100m
- Use smooth interpolation functions (smoothstep)
- Blend terrain parameters gradually
- Check for terrain height discontinuities

### Low Material Quality

**Issue**: Missing PBR textures or low resolution

**Fix**:

- Add all PBR texture maps (base color, normal, roughness, metallic, AO)
- Use minimum 2048x2048 resolution
- Implement proper triplanar projection
- Add distance-based detail textures

## Performance Considerations

The validator is designed for development/testing, not runtime:

- Run during development to check quality
- Use in editor tools and automated tests
- Don't run every frame in shipping builds
- Sample locations can be adjusted for performance

## Integration with Testing

Add to your automated test suite:

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVisualQualityTest,
    "Alexander.PlanetInterior.VisualQuality",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVisualQualityTest::RunTest(const FString& Parameters)
{
    // Create validator
    AVisualQualityValidator* Validator = /* ... */;

    // Run validation
    FVisualQualityMetrics Metrics = Validator->RunCompleteValidation(TestPlanet);

    // Assert quality standards
    TestTrue("Overall quality meets standards", Metrics.OverallQuality >= 0.85f);
    TestTrue("Atmospheric quality sufficient", Metrics.AtmosphericQuality >= 0.85f);
    TestTrue("Biome transitions smooth", Metrics.BiomeTransitionQuality >= 0.90f);
    TestTrue("Material quality adequate", Metrics.MaterialQuality >= 0.88f);

    return true;
}
```

## Logging

Enable detailed logging for debugging:

```cpp
Validator->bEnableDetailedLogging = true;
```

This will output detailed information about each validation check:

```
LogTemp: Atmospheric Scattering Score: 0.89
LogTemp: Fog Effects Score: 0.92
LogTemp: Cloud Quality Score: 0.85
LogTemp: Biome transitions: 2 issues out of 20 transitions
LogTemp: Terrain Material - Texture: 0.88, Parameters: 0.85
LogTemp: LOD Transition Score: 0.90
```

## Best Practices

1. **Run Early and Often**: Validate quality throughout development
2. **Set Realistic Targets**: Star Citizen standards are very high
3. **Focus on Weakest Areas**: Address lowest-scoring components first
4. **Iterate**: Use validation feedback to guide improvements
5. **Document Changes**: Track quality improvements over time

## Requirements Validated

This validator addresses these requirements:

- **1.1**: High-fidelity planetary surface rendering
- **2.1**: Realistic atmospheric system with scattering
- **3.2**: Smooth biome transitions
- **6.1**: Advanced material system with PBR

## Next Steps

After validation:

1. Review the generated report
2. Address critical issues first
3. Iterate on atmospheric parameters
4. Improve biome blending
5. Enhance material quality
6. Re-validate to confirm improvements

## Support

For issues or questions:

- Check the detailed log output
- Review component-specific validation methods
- Adjust quality standards if needed
- Consult Star Citizen reference materials
