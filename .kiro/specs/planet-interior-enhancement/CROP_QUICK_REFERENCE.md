# Crop System Quick Reference Card

## 12 Standard Crops

| #   | Crop              | Biomes                    | Time | Price | Special          |
| --- | ----------------- | ------------------------- | ---- | ----- | ---------------- |
| 1   | Space Wheat       | Grassland, Desert         | 10m  | 5     | Hardy            |
| 2   | Lunar Potato      | Grassland, Tundra, Desert | 15m  | 8     | Drought Resist   |
| 3   | Martian Tomato    | Grassland, Forest         | 12m  | 12    | Needs Irrigation |
| 4   | Titan Corn        | Grassland, Forest         | 20m  | 10    | High Yield       |
| 5   | Venusian Pepper   | Volcanic, Desert          | 14m  | 20    | Pest Resist      |
| 6   | Europa Kelp       | Ocean, Tundra             | 8m   | 6     | Aquatic          |
| 7   | Ganymede Berry    | Forest, Grassland         | 18m  | 18    | Regrows 30%      |
| 8   | Callisto Mushroom | Forest, Tundra, Volcanic  | 6m   | 15    | Low Light        |
| 9   | Io Cactus         | Desert, Volcanic          | 25m  | 25    | Both Resists     |
| 10  | Enceladus Rice    | Ocean, Grassland          | 16m  | 7     | High Yield       |
| 11  | Triton Herb       | Tundra, Grassland         | 9m   | 22    | Medicinal        |
| 12  | Pluto Flower      | Tundra, Forest            | 12m  | 30    | Regrows 20%      |

## Quick Code Snippets

### Create All Crops

```cpp
TArray<UCropDefinition*> Crops = UCropFactory::CreateAllStandardCrops();
```

### Register with System

```cpp
UFarmingSubsystem* FS = GetWorld()->GetSubsystem<UFarmingSubsystem>();
for (UCropDefinition* Crop : Crops)
{
    TArray<FString> Biomes;
    for (EBiomeType B : Crop->SuitableBiomes)
        Biomes.Add(UEnum::GetValueAsString(B));
    FS->RegisterCrop(Crop->HarvestItemID, Crop, Biomes);
}
```

### Get Crop

```cpp
UCropDefinition* Wheat = FS->GetCropDefinition("space_wheat");
```

### Check Suitability

```cpp
bool bOK = Crop->IsSuitableForBiome(EBiomeType::Grassland);
```

### Calculate Growth

```cpp
float Mod = Crop->CalculateGrowthModifier(20.0f, 0.5f, 0.8f, 0.6f);
```

### Get Stage Mesh

```cpp
UStaticMesh* Mesh = Crop->GetGrowthStageMesh(0.75f);
```

## Biome Recommendations

### Grassland → Space Wheat, Titan Corn, Ganymede Berry

### Desert → Lunar Potato, Io Cactus

### Forest → Callisto Mushroom, Martian Tomato

### Tundra → Triton Herb, Pluto Flower

### Volcanic → Venusian Pepper

### Ocean → Europa Kelp, Enceladus Rice

## Profit per Minute (Average)

1. **Triton Herb**: 24.4 credits/min
2. **Pluto Flower**: 17.5 credits/min
3. **Callisto Mushroom**: 16.5 credits/min
4. **Venusian Pepper**: 11.4 credits/min
5. **Ganymede Berry**: 8.0 credits/min (+ regrows)
6. **Martian Tomato**: 7.5 credits/min
7. **Lunar Potato**: 6.0 credits/min
8. **Enceladus Rice**: 5.5 credits/min
9. **Space Wheat**: 5.0 credits/min
10. **Titan Corn**: 5.0 credits/min
11. **Europa Kelp**: 4.5 credits/min
12. **Io Cactus**: 4.0 credits/min (but extreme conditions)

## Files

- `Source/Alexander/Public/CropDefinition.h`
- `Source/Alexander/Private/CropDefinition.cpp`
- `Source/Alexander/Public/CropFactory.h`
- `Source/Alexander/Private/CropFactory.cpp`
- `.kiro/specs/planet-interior-enhancement/TASK_12.3_IMPLEMENTATION.md`
- `.kiro/specs/planet-interior-enhancement/QUICKSTART_CropDefinitions.md`
- `.kiro/specs/planet-interior-enhancement/CROP_SYSTEM_SUMMARY.md`
