# Planet Interior Enhancement - Testing Guide

## Overview

Automated tests have been created for the core functionality of the planet interior enhancement system. These tests validate noise generation, biome blending, LOD calculations, and crop growth simulation.

## Test File Location

`Source/Alexander/Private/PlanetInteriorTests.cpp`

## Test Categories

### 1. Noise Generation Tests

**Test: NoiseGeneration.Consistency**

- Validates that noise functions produce deterministic results with the same seed
- Tests both Perlin and Simplex noise algorithms

**Test: NoiseGeneration.Range**

- Verifies noise values stay within expected range [-1, 1]
- Samples 100 random positions to ensure consistency

**Test: NoiseGeneration.Fractal**

- Tests multi-octave fractal noise generation
- Validates that different octave counts produce different results

**Test: NoiseGeneration.Voronoi**

- Tests Voronoi noise for crater generation
- Validates cell retrieval and distance calculations

### 2. Biome Blending Tests

**Test: BiomeBlending.WeightSum**

- Validates that biome weights sum to approximately 1.0
- Ensures individual weights are in valid range [0, 1]

**Test: BiomeBlending.Transition**

- Tests transition zone detection
- Validates transition factor calculations

**Test: BiomeBlending.Parameters**

- Tests blended terrain parameter calculations
- Validates parameter ranges (roughness, humidity, vegetation density)

### 3. LOD Distance Calculation Tests

**Test: LOD.DistanceCalculation**

- Tests distance-based fade factor calculations
- Validates fade factors are in valid range [0, 1]

**Test: LOD.TransitionProgress**

- Tests LOD transition progress tracking
- Validates initial state and progress values

**Test: LOD.VertexInterpolation**

- Tests vertex interpolation between LOD levels
- Validates smooth transitions at alpha = 0.5

### 4. Crop Growth Simulation Tests

**Test: CropGrowth.Temperature**

- Tests temperature modifier calculations
- Validates optimal, cold, and hot temperature scenarios

**Test: CropGrowth.Humidity**

- Tests humidity modifier calculations
- Validates optimal, low, and high humidity scenarios

**Test: CropGrowth.Sunlight**

- Tests sunlight exposure modifier
- Validates sufficient, insufficient, and no sunlight scenarios

**Test: CropGrowth.Water**

- Tests water availability modifier
- Validates adequate, low, and no water scenarios

**Test: CropGrowth.SoilQuality**

- Tests soil quality modifier calculations
- Validates good and poor soil scenarios

**Test: CropGrowth.Fertility**

- Tests fertility modifier calculations
- Validates high, medium, and low fertility scenarios

## Running Tests

### From Unreal Editor

1. Open the Unreal Editor
2. Go to **Window > Test Automation**
3. In the Automation tab, expand **Alexander > PlanetInterior**
4. Select the tests you want to run
5. Click **Start Tests**

### From Command Line

```powershell
# Run all planet interior tests
& "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
  "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" `
  -ExecCmds="Automation RunTests Alexander.PlanetInterior" `
  -TestExit="Automation Test Queue Empty" `
  -ReportOutputPath="C:\Users\allen\Desktop\Alexander\Alexander\Saved\Automation\Reports" `
  -Log

# Run specific test category
& "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
  "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" `
  -ExecCmds="Automation RunTests Alexander.PlanetInterior.NoiseGeneration" `
  -TestExit="Automation Test Queue Empty" `
  -Log
```

### From Python (Automation Script)

```python
import subprocess
import os

def run_planet_tests():
    """Run all planet interior enhancement tests"""
    ue_editor = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
    project_file = r"C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"

    cmd = [
        ue_editor,
        project_file,
        "-ExecCmds=Automation RunTests Alexander.PlanetInterior",
        "-TestExit=Automation Test Queue Empty",
        "-ReportOutputPath=Saved/Automation/Reports",
        "-Log",
        "-Unattended",
        "-NullRHI"
    ]

    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.returncode == 0

if __name__ == "__main__":
    success = run_planet_tests()
    print(f"Tests {'passed' if success else 'failed'}")
```

## Test Coverage

### Core Functionality Tested

✅ **Noise Generation**

- Perlin noise determinism and range
- Simplex noise determinism and range
- Fractal/multi-octave noise
- Voronoi noise for craters

✅ **Biome Blending**

- Weight normalization (sum to 1.0)
- Transition zone detection
- Parameter blending (elevation, roughness, humidity, etc.)

✅ **LOD System**

- Distance-based fade calculations
- Transition progress tracking
- Vertex interpolation between LOD levels

✅ **Crop Growth**

- Temperature effects on growth
- Humidity effects on growth
- Sunlight requirements
- Water availability
- Soil quality impact
- Fertility depletion

### Requirements Coverage

These tests validate the following requirements from the design document:

- **Requirement 4**: Procedural Terrain Generation (noise generation tests)
- **Requirement 3**: Multiple Biome Support (biome blending tests)
- **Requirement 8**: Performance Optimization (LOD calculation tests)
- **Requirement 15**: Crop Growth Simulation (crop growth tests)

## Expected Results

All tests should pass with the following characteristics:

- **Noise tests**: Deterministic results, values in range [-1, 1]
- **Biome tests**: Weights sum to ~1.0, smooth transitions
- **LOD tests**: Smooth interpolation, valid fade factors
- **Crop tests**: Realistic growth modifiers based on environmental factors

## Troubleshooting

### Tests Not Appearing

If tests don't appear in the Test Automation window:

1. Ensure the project has been compiled successfully
2. Restart the Unreal Editor
3. Check that the test file is included in the build

### Tests Failing

If tests fail:

1. Check the test output for specific failure messages
2. Verify that all required classes are properly initialized
3. Ensure no breaking changes were made to the tested classes

### Compilation Errors

If the test file doesn't compile:

1. Ensure all header files are properly included
2. Check that all tested classes exist and are accessible
3. Verify that the Unreal Engine automation framework is available

## Future Test Additions

Consider adding tests for:

- Weather system transitions
- Atmospheric scattering calculations
- Cave generation algorithms
- Terrain erosion simulation
- VR farming interactions
- Performance benchmarks

## Notes

- Tests are designed to be fast and focused on core logic
- No mocks or fake data are used - tests validate real functionality
- Tests follow Unreal Engine's automation test framework conventions
- All tests are categorized under `Alexander.PlanetInterior` for easy filtering
