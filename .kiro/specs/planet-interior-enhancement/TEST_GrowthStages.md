# Growth Stage System - Testing Guide

## Overview

This document provides comprehensive testing procedures for the crop growth stage system.

## Test Environment Setup

### Prerequisites

1. Unreal Engine 5.6 project loaded
2. Alexander game module compiled
3. At least one crop definition with growth stage meshes
4. Farm plot actor placed in level
5. Planet actor with weather system

### Test Level Setup

```cpp
// Create test farm plot
AFarmPlot* TestPlot = World->SpawnActor<AFarmPlot>(FVector(0, 0, 0), FRotator::ZeroRotator);
TestPlot->SetPlotSize(FVector2D(1000.0f, 1000.0f));
TestPlot->SetInitialSoilQuality(0.8f);

// Create test crop definition
UCropDefinition* TestCrop = NewObject<UCropDefinition>();
TestCrop->CropName = TEXT("Test Crop");
TestCrop->GrowthDuration = 60.0f; // 1 minute for testing
```

## Unit Tests

### Test 1: Stage Generation

**Purpose**: Verify default stage generation works correctly

```cpp
void TestStageGeneration()
{
    UCropDefinition* Crop = CreateTestCrop();

    // Test with 4 stages
    TArray<FGrowthStage> Stages4 = UCropGrowthStageManager::GenerateDefaultStages(Crop, 4);
    check(Stages4.Num() == 4);
    check(Stages4[0].MinProgress == 0.0f);
    check(Stages4[3].MaxProgress == 1.0f);

    // Test with 6 stages
    TArray<FGrowthStage> Stages6 = UCropGrowthStageManager::GenerateDefaultStages(Crop, 6);
    check(Stages6.Num() == 6);

    // Test with 8 stages
    TArray<FGrowthStage> Stages8 = UCropGrowthStageManager::GenerateDefaultStages(Crop, 8);
    check(Stages8.Num() == 8);

    UE_LOG(LogTemp, Log, TEXT("✅ Stage generation test passed"));
}
```

**Expected Results**:

- ✅ Correct number of stages generated
- ✅ Stages cover full 0-1 range
- ✅ No gaps or overlaps
- ✅ Stage names are assigned

### Test 2: Stage Index Calculation

**Purpose**: Verify stage index is calculated correctly for any progress value

```cpp
void TestStageIndexCalculation()
{
    TArray<FGrowthStage> Stages = GenerateTestStages(4);

    // Test boundaries
    check(UCropGrowthStageManager::GetCurrentStageIndex(0.0f, Stages) == 0);
    check(UCropGrowthStageManager::GetCurrentStageIndex(0.24f, Stages) == 0);
    check(UCropGrowthStageManager::GetCurrentStageIndex(0.25f, Stages) == 1);
    check(UCropGrowthStageManager::GetCurrentStageIndex(0.49f, Stages) == 1);
    check(UCropGrowthStageManager::GetCurrentStageIndex(0.50f, Stages) == 2);
    check(UCropGrowthStageManager::GetCurrentStageIndex(0.74f, Stages) == 2);
    check(UCropGrowthStageManager::GetCurrentStageIndex(0.75f, Stages) == 3);
    check(UCropGrowthStageManager::GetCurrentStageIndex(1.0f, Stages) == 3);

    UE_LOG(LogTemp, Log, TEXT("✅ Stage index calculation test passed"));
}
```

**Expected Results**:

- ✅ Correct stage for all progress values
- ✅ Boundary conditions handled correctly
- ✅ No off-by-one errors

### Test 3: Stage Change Detection

**Purpose**: Verify stage changes are detected correctly

```cpp
void TestStageChangeDetection()
{
    TArray<FGrowthStage> Stages = GenerateTestStages(4);

    // No change within same stage
    check(!UCropGrowthStageManager::HasStageChanged(0.1f, 0.2f, Stages));
    check(!UCropGrowthStageManager::HasStageChanged(0.3f, 0.4f, Stages));

    // Change detected at boundaries
    check(UCropGrowthStageManager::HasStageChanged(0.24f, 0.26f, Stages));
    check(UCropGrowthStageManager::HasStageChanged(0.49f, 0.51f, Stages));
    check(UCropGrowthStageManager::HasStageChanged(0.74f, 0.76f, Stages));

    UE_LOG(LogTemp, Log, TEXT("✅ Stage change detection test passed"));
}
```

**Expected Results**:

- ✅ Changes detected at stage boundaries
- ✅ No false positives within stages
- ✅ Works for all stage transitions

### Test 4: Visual Property Interpolation

**Purpose**: Verify smooth interpolation of scale and color

```cpp
void TestVisualInterpolation()
{
    TArray<FGrowthStage> Stages = GenerateTestStages(4);

    // Test scale interpolation
    FVector Scale0 = UCropGrowthStageManager::GetStageScale(0.0f, Stages);
    FVector Scale25 = UCropGrowthStageManager::GetStageScale(0.25f, Stages);
    FVector Scale50 = UCropGrowthStageManager::GetStageScale(0.5f, Stages);
    FVector Scale100 = UCropGrowthStageManager::GetStageScale(1.0f, Stages);

    check(Scale0.X < Scale25.X);
    check(Scale25.X < Scale50.X);
    check(Scale50.X < Scale100.X);

    // Test color interpolation
    FLinearColor Color0 = UCropGrowthStageManager::GetStageColor(0.0f, Stages);
    FLinearColor Color100 = UCropGrowthStageManager::GetStageColor(1.0f, Stages);

    check(Color0 != Color100); // Colors should differ

    UE_LOG(LogTemp, Log, TEXT("✅ Visual interpolation test passed"));
}
```

**Expected Results**:

- ✅ Scale increases with progress
- ✅ Smooth interpolation between stages
- ✅ Colors transition naturally

### Test 5: Stage Validation

**Purpose**: Verify stage configuration validation works

```cpp
void TestStageValidation()
{
    // Valid stages
    TArray<FGrowthStage> ValidStages = GenerateTestStages(4);
    FString ErrorMsg;
    check(UCropGrowthStageManager::ValidateStages(ValidStages, ErrorMsg));

    // Invalid stages (gap)
    TArray<FGrowthStage> InvalidStages;
    InvalidStages.Add(FGrowthStage(TEXT("Stage1"), 0.0f, 0.4f));
    InvalidStages.Add(FGrowthStage(TEXT("Stage2"), 0.5f, 1.0f)); // Gap at 0.4-0.5
    check(!UCropGrowthStageManager::ValidateStages(InvalidStages, ErrorMsg));

    // Fix invalid stages
    TArray<FGrowthStage> FixedStages = UCropGrowthStageManager::FixStageRanges(InvalidStages);
    check(UCropGrowthStageManager::ValidateStages(FixedStages, ErrorMsg));

    UE_LOG(LogTemp, Log, TEXT("✅ Stage validation test passed"));
}
```

**Expected Results**:

- ✅ Valid stages pass validation
- ✅ Invalid stages are detected
- ✅ Auto-fix corrects issues

## Integration Tests

### Test 6: Farm Plot Integration

**Purpose**: Verify growth stages work in farm plot

```cpp
void TestFarmPlotIntegration()
{
    AFarmPlot* Plot = CreateTestPlot();
    UCropDefinition* Crop = CreateTestCrop();

    // Plant crop
    bool bPlanted = Plot->PlantCrop(FIntPoint(0, 0), Crop);
    check(bPlanted);

    // Verify initial stage
    int32 InitialStage = Plot->GetCropGrowthStage(FIntPoint(0, 0));
    check(InitialStage == 0);

    // Simulate growth
    float DeltaTime = 0.1f;
    for (int32 i = 0; i < 100; ++i)
    {
        Plot->UpdateCropGrowth(DeltaTime);
    }

    // Verify stage progressed
    int32 FinalStage = Plot->GetCropGrowthStage(FIntPoint(0, 0));
    check(FinalStage > InitialStage);

    UE_LOG(LogTemp, Log, TEXT("✅ Farm plot integration test passed"));
}
```

**Expected Results**:

- ✅ Crops plant successfully
- ✅ Initial stage is 0
- ✅ Stage progresses with growth
- ✅ Stage tracking is accurate

### Test 7: Stage Change Events

**Purpose**: Verify stage change events fire correctly

```cpp
void TestStageChangeEvents()
{
    AFarmPlot* Plot = CreateTestPlot();
    UCropDefinition* Crop = CreateTestCrop();
    Crop->GrowthDuration = 10.0f; // Fast growth

    Plot->PlantCrop(FIntPoint(0, 0), Crop);

    int32 StageChangeCount = 0;
    int32 LastStage = -1;

    // Monitor stage changes
    for (float Time = 0.0f; Time < 12.0f; Time += 0.1f)
    {
        Plot->UpdateCropGrowth(0.1f);

        int32 CurrentStage = Plot->GetCropGrowthStage(FIntPoint(0, 0));
        if (CurrentStage != LastStage && LastStage >= 0)
        {
            StageChangeCount++;
            UE_LOG(LogTemp, Log, TEXT("Stage changed: %d -> %d"), LastStage, CurrentStage);
        }
        LastStage = CurrentStage;
    }

    check(StageChangeCount >= 3); // Should have multiple stage changes

    UE_LOG(LogTemp, Log, TEXT("✅ Stage change events test passed (%d changes)"), StageChangeCount);
}
```

**Expected Results**:

- ✅ Multiple stage changes occur
- ✅ Events fire at correct times
- ✅ Stage progression is sequential

### Test 8: Visual Update Performance

**Purpose**: Verify visual updates don't cause performance issues

```cpp
void TestVisualUpdatePerformance()
{
    AFarmPlot* Plot = CreateTestPlot();
    Plot->SetPlotSize(FVector2D(5000.0f, 5000.0f)); // Large plot
    Plot->GridResolution = 20; // 20x20 = 400 cells

    UCropDefinition* Crop = CreateTestCrop();

    // Plant many crops
    for (int32 Y = 0; Y < 20; ++Y)
    {
        for (int32 X = 0; X < 20; ++X)
        {
            Plot->PlantCrop(FIntPoint(X, Y), Crop);
        }
    }

    // Measure update time
    double StartTime = FPlatformTime::Seconds();
    Plot->UpdateCropGrowth(0.1f);
    double EndTime = FPlatformTime::Seconds();

    double UpdateTime = (EndTime - StartTime) * 1000.0; // Convert to ms
    check(UpdateTime < 16.0); // Should be under 16ms for 60 FPS

    UE_LOG(LogTemp, Log, TEXT("✅ Visual update performance test passed (%.2f ms)"), UpdateTime);
}
```

**Expected Results**:

- ✅ Update time under 16ms
- ✅ No frame drops
- ✅ Smooth visual updates

## Visual Tests

### Test 9: Stage Mesh Swapping

**Manual Test Procedure**:

1. Place farm plot in level
2. Plant crop with 4+ stage meshes
3. Use console command to accelerate time:
   ```
   SetTimeScale 10.0
   ```
4. Observe crop growth

**Verification Checklist**:

- ✅ Mesh changes at each stage
- ✅ No visual popping
- ✅ Meshes are correct for each stage
- ✅ Transitions are smooth
- ✅ Scale progression looks natural

### Test 10: Color Transitions

**Manual Test Procedure**:

1. Plant crop in farm plot
2. Monitor color changes during growth
3. Reduce crop health (stop watering)
4. Observe color changes

**Verification Checklist**:

- ✅ Color transitions smoothly
- ✅ Early stages are lighter
- ✅ Mature stages are vibrant
- ✅ Unhealthy crops turn brown
- ✅ Health affects color appropriately

### Test 11: Scale Progression

**Manual Test Procedure**:

1. Plant multiple crops at different growth stages
2. Compare sizes visually
3. Verify smooth scaling

**Verification Checklist**:

- ✅ Size increases with each stage
- ✅ No sudden size jumps
- ✅ Micro-scaling within stages visible
- ✅ Final size matches expectations
- ✅ Proportions remain consistent

## Stress Tests

### Test 12: Many Crops

**Purpose**: Test system with large number of crops

```cpp
void TestManyCrops()
{
    AFarmPlot* Plot = CreateTestPlot();
    Plot->GridResolution = 50; // 50x50 = 2500 crops

    UCropDefinition* Crop = CreateTestCrop();

    // Plant all cells
    for (int32 Y = 0; Y < 50; ++Y)
    {
        for (int32 X = 0; X < 50; ++X)
        {
            Plot->PlantCrop(FIntPoint(X, Y), Crop);
        }
    }

    // Update multiple times
    for (int32 i = 0; i < 100; ++i)
    {
        Plot->UpdateCropGrowth(0.1f);
    }

    UE_LOG(LogTemp, Log, TEXT("✅ Many crops stress test passed (2500 crops)"));
}
```

**Expected Results**:

- ✅ All crops plant successfully
- ✅ Updates complete without crashes
- ✅ Performance remains acceptable
- ✅ Memory usage is reasonable

### Test 13: Rapid Growth

**Purpose**: Test with very fast growth rates

```cpp
void TestRapidGrowth()
{
    AFarmPlot* Plot = CreateTestPlot();
    UCropDefinition* Crop = CreateTestCrop();
    Crop->GrowthDuration = 1.0f; // 1 second growth

    Plot->PlantCrop(FIntPoint(0, 0), Crop);

    // Rapid updates
    for (int32 i = 0; i < 100; ++i)
    {
        Plot->UpdateCropGrowth(0.01f);
    }

    // Verify reached maturity
    int32 FinalStage = Plot->GetCropGrowthStage(FIntPoint(0, 0));
    check(FinalStage == Crop->GetNumGrowthStages() - 1);

    UE_LOG(LogTemp, Log, TEXT("✅ Rapid growth test passed"));
}
```

**Expected Results**:

- ✅ All stages transition correctly
- ✅ No stages skipped
- ✅ Final stage reached
- ✅ No visual glitches

## Edge Case Tests

### Test 14: No Stage Meshes

**Purpose**: Test behavior when crop has no stage meshes

```cpp
void TestNoStageMeshes()
{
    AFarmPlot* Plot = CreateTestPlot();
    UCropDefinition* Crop = CreateTestCrop();
    Crop->GrowthStageMeshes.Empty(); // Remove all stage meshes
    Crop->CropMesh = LoadDefaultMesh(); // Only fallback mesh

    bool bPlanted = Plot->PlantCrop(FIntPoint(0, 0), Crop);
    check(bPlanted);

    // Should still work with fallback mesh
    Plot->UpdateCropGrowth(0.5f);

    UE_LOG(LogTemp, Log, TEXT("✅ No stage meshes test passed"));
}
```

**Expected Results**:

- ✅ Crop plants successfully
- ✅ Falls back to default mesh
- ✅ No crashes or errors
- ✅ Growth still progresses

### Test 15: Single Stage

**Purpose**: Test with only one growth stage

```cpp
void TestSingleStage()
{
    UCropDefinition* Crop = CreateTestCrop();
    TArray<FGrowthStage> Stages = UCropGrowthStageManager::GenerateDefaultStages(Crop, 1);

    check(Stages.Num() == 1);
    check(Stages[0].MinProgress == 0.0f);
    check(Stages[0].MaxProgress == 1.0f);

    // All progress values should return stage 0
    check(UCropGrowthStageManager::GetCurrentStageIndex(0.0f, Stages) == 0);
    check(UCropGrowthStageManager::GetCurrentStageIndex(0.5f, Stages) == 0);
    check(UCropGrowthStageManager::GetCurrentStageIndex(1.0f, Stages) == 0);

    UE_LOG(LogTemp, Log, TEXT("✅ Single stage test passed"));
}
```

**Expected Results**:

- ✅ Single stage created correctly
- ✅ All progress maps to stage 0
- ✅ No stage changes occur
- ✅ System handles gracefully

## Regression Tests

### Test 16: Backward Compatibility

**Purpose**: Ensure existing crops without stages still work

```cpp
void TestBackwardCompatibility()
{
    // Old crop definition without growth stages
    UCropDefinition* OldCrop = CreateLegacyCrop();
    check(OldCrop->GrowthStageMeshes.Num() == 0);

    AFarmPlot* Plot = CreateTestPlot();
    bool bPlanted = Plot->PlantCrop(FIntPoint(0, 0), OldCrop);
    check(bPlanted);

    // Should still grow normally
    Plot->UpdateCropGrowth(0.5f);

    UE_LOG(LogTemp, Log, TEXT("✅ Backward compatibility test passed"));
}
```

**Expected Results**:

- ✅ Old crops still work
- ✅ No breaking changes
- ✅ Graceful degradation
- ✅ No errors or warnings

## Test Execution

### Run All Tests

```cpp
void RunAllGrowthStageTests()
{
    UE_LOG(LogTemp, Log, TEXT("=== Running Growth Stage Tests ==="));

    // Unit tests
    TestStageGeneration();
    TestStageIndexCalculation();
    TestStageChangeDetection();
    TestVisualInterpolation();
    TestStageValidation();

    // Integration tests
    TestFarmPlotIntegration();
    TestStageChangeEvents();
    TestVisualUpdatePerformance();

    // Stress tests
    TestManyCrops();
    TestRapidGrowth();

    // Edge cases
    TestNoStageMeshes();
    TestSingleStage();

    // Regression
    TestBackwardCompatibility();

    UE_LOG(LogTemp, Log, TEXT("=== All Tests Passed ✅ ==="));
}
```

### Console Commands

Add these for manual testing:

```cpp
// Accelerate crop growth
SetCropGrowthRate 10.0

// Jump to specific stage
SetCropStage 0 0 3  // Plot 0, Cell (0,0), Stage 3

// Show stage debug info
ShowCropStageInfo

// Toggle stage visualization
ToggleStageDebugDraw
```

## Success Criteria

All tests must pass:

- ✅ 16/16 automated tests pass
- ✅ No crashes or errors
- ✅ Performance within targets
- ✅ Visual quality acceptable
- ✅ No regressions

## Reporting Issues

If tests fail, report:

1. Test name and number
2. Expected vs actual results
3. Console log output
4. Screenshots (for visual tests)
5. Reproduction steps
6. System specifications

## Summary

Comprehensive testing ensures:

- Correct stage generation and tracking
- Smooth visual transitions
- Good performance
- Edge case handling
- Backward compatibility
- Production readiness
