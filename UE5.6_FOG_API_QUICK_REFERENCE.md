# UE5.6 Exponential Height Fog API Quick Reference

## Migration Guide: Property Access → Setter Methods

### Quick Comparison Table

| Old API (UE4/Early UE5) | New API (UE5.6) | Notes |
|-------------------------|------------------|-------|
| `Component->FogInscatteringColor = Color;` | `Component->SetFogInscatteringColor(Color);` | Direct setter |
| `Component->DirectionalInscatteringExponent = Value;` | `Component->SetDirectionalInscatteringExponent(Value);` | Direct setter |
| `Component->DirectionalInscatteringIntensity = Value;` | Scale color by intensity | No direct setter |
| `Component->DirectionalInscatteringColor = Color;` | `Component->SetDirectionalInscatteringColor(Color);` | Direct setter |
| `Component->VolumetricFogAlbedo = Color;` | `Component->SetVolumetricFogAlbedo(Color);` | Direct setter |
| N/A | `Component->SetDirectionalInscatteringStartDistance(0.0f);` | New in UE5.6 |

---

## Code Examples

### 1. Setting Fog Inscattering Color
```cpp
// UE5.6 Method
HeightFogComponent->SetFogInscatteringColor(FLinearColor(0.8f, 0.9f, 1.0f));

// Alternative: Dynamic based on altitude
FLinearColor FogColor = GetFogColorAtAltitude(ViewerAltitude);
HeightFogComponent->SetFogInscatteringColor(FogColor);
```

### 2. Setting Directional Inscattering (Sun Glow)
```cpp
// UE5.6 Method - Complete setup
HeightFogComponent->SetDirectionalInscatteringExponent(4.0f);
HeightFogComponent->SetDirectionalInscatteringStartDistance(0.0f);

// Apply intensity by scaling color (replaces DirectionalInscatteringIntensity)
float Intensity = 0.5f;
FLinearColor BaseColor = FLinearColor(0.8f, 0.9f, 1.0f);
FLinearColor ScaledColor = BaseColor * Intensity;
HeightFogComponent->SetDirectionalInscatteringColor(ScaledColor);
```

### 3. Setting Volumetric Fog Albedo
```cpp
// UE5.6 Method
HeightFogComponent->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.9f, 0.9f));

// Common values:
// Water particles: FLinearColor(0.95f, 0.95f, 0.95f) - bright, reflective
// Dust/Smoke: FLinearColor(0.7f, 0.7f, 0.7f) - darker, less reflective
```

---

## Complete Fog Setup Example

```cpp
void SetupAtmosphericFog(UExponentialHeightFogComponent* FogComponent)
{
    if (!FogComponent)
        return;

    // Basic fog properties (still use direct access)
    FogComponent->FogDensity = 0.02f;
    FogComponent->FogHeightFalloff = 0.2f;
    FogComponent->StartDistance = 0.0f;
    FogComponent->FogCutoffDistance = 0.0f;

    // Fog inscattering color (UE5.6 setter)
    FogComponent->SetFogInscatteringColor(FLinearColor(0.7f, 0.8f, 0.9f));

    // Directional inscattering (sun glow) - UE5.6 setters
    FogComponent->SetDirectionalInscatteringExponent(4.0f);
    FogComponent->SetDirectionalInscatteringStartDistance(0.0f);

    // Apply intensity through color scaling
    float Intensity = 0.5f;
    FLinearColor SunColor = FLinearColor(0.8f, 0.9f, 1.0f);
    FogComponent->SetDirectionalInscatteringColor(SunColor * Intensity);

    // Volumetric fog (UE5.6 setter for albedo)
    FogComponent->bEnableVolumetricFog = true;
    FogComponent->VolumetricFogScatteringDistribution = 0.2f;
    FogComponent->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.9f, 0.9f));
    FogComponent->VolumetricFogExtinctionScale = 1.0f;
    FogComponent->VolumetricFogDistance = 50000.0f;

    // Update render state
    FogComponent->MarkRenderStateDirty();
}
```

---

## Migration Patterns

### Pattern 1: Simple Property Replacement
```cpp
// Before
Component->PropertyName = Value;

// After
Component->SetPropertyName(Value);
```

### Pattern 2: Intensity Property Elimination
```cpp
// Before (UE4/Early UE5)
Component->DirectionalInscatteringIntensity = 0.5f;
Component->DirectionalInscatteringColor = FLinearColor(1.0f, 1.0f, 1.0f);

// After (UE5.6) - Merge into one call
FLinearColor Color = FLinearColor(1.0f, 1.0f, 1.0f) * 0.5f;
Component->SetDirectionalInscatteringColor(Color);
```

### Pattern 3: New Required Properties
```cpp
// UE5.6 requires explicit start distance
Component->SetDirectionalInscatteringStartDistance(0.0f);
```

---

## Properties That Still Use Direct Access

Some properties still use direct access in UE5.6:

```cpp
// These are still valid in UE5.6
FogComponent->FogDensity = 0.02f;
FogComponent->FogHeightFalloff = 0.2f;
FogComponent->StartDistance = 0.0f;
FogComponent->FogCutoffDistance = 0.0f;
FogComponent->bEnableVolumetricFog = true;
FogComponent->VolumetricFogScatteringDistribution = 0.2f;
FogComponent->VolumetricFogExtinctionScale = 1.0f;
FogComponent->VolumetricFogDistance = 50000.0f;
```

**Rule of Thumb:** Color properties and directional inscattering properties use setters; numeric/boolean properties still use direct access.

---

## Common Mistakes

### ❌ Mistake 1: Using Old Property Names
```cpp
// WRONG - Compilation error in UE5.6
Component->FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f);
```

### ✅ Fix
```cpp
// CORRECT
Component->SetFogInscatteringColor(FLinearColor(0.8f, 0.9f, 1.0f));
```

---

### ❌ Mistake 2: Not Scaling Intensity
```cpp
// WRONG - Intensity property doesn't exist in UE5.6
Component->DirectionalInscatteringIntensity = 0.5f;
```

### ✅ Fix
```cpp
// CORRECT - Scale the color
FLinearColor Color = BaseColor * 0.5f;
Component->SetDirectionalInscatteringColor(Color);
```

---

### ❌ Mistake 3: Forgetting Start Distance
```cpp
// INCOMPLETE - Should set start distance
Component->SetDirectionalInscatteringExponent(4.0f);
Component->SetDirectionalInscatteringColor(Color);
```

### ✅ Fix
```cpp
// CORRECT - Always set start distance
Component->SetDirectionalInscatteringExponent(4.0f);
Component->SetDirectionalInscatteringStartDistance(0.0f); // Add this
Component->SetDirectionalInscatteringColor(Color);
```

---

## Testing Checklist

After migrating to UE5.6 API:

- [ ] Project compiles without errors
- [ ] Fog renders at runtime
- [ ] Fog density changes with altitude
- [ ] Sun glow visible through fog
- [ ] Fog color matches expected values
- [ ] Volumetric fog renders (if enabled)
- [ ] Lumen integration works
- [ ] No performance regression
- [ ] No console warnings/errors

---

## Reference

**Official Documentation:**
- [Exponential Height Fog - UE5.6](https://dev.epicgames.com/documentation/en-us/unreal-engine/exponential-height-fog-in-unreal-engine)
- [Volumetric Fog - UE5.6](https://dev.epicgames.com/documentation/en-us/unreal-engine/volumetric-fog-in-unreal-engine)

**Related Components:**
- `UExponentialHeightFogComponent` - Main fog component
- `AExponentialHeightFog` - Actor wrapper
- `USkyAtmosphereComponent` - Atmosphere integration

**Modified in Alexander Project:**
- `Source/Alexander/Private/AlexanderAtmosphericFogComponent.cpp` - Main implementation
- `Source/Alexander/Public/AlexanderAtmosphericFogComponent.h` - Component header

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| UE 4.x | - | Direct property access for all fog properties |
| UE 5.0-5.5 | - | Transition period, both methods supported |
| UE 5.6 | 2025 | Setter methods required for color properties |
| Alexander Fix | 2025-11-04 | Migrated all 6 properties to UE5.6 API |

---

## Quick Reference Card

**Save this for quick lookup:**

```cpp
// === FOG INSCATTERING ===
SetFogInscatteringColor(FLinearColor)

// === DIRECTIONAL INSCATTERING (SUN GLOW) ===
SetDirectionalInscatteringExponent(float)
SetDirectionalInscatteringStartDistance(float)
SetDirectionalInscatteringColor(FLinearColor)

// === VOLUMETRIC FOG ===
SetVolumetricFogAlbedo(FLinearColor)

// === REMEMBER ===
// 1. Use setters for color properties
// 2. Scale color instead of setting intensity
// 3. Always call MarkRenderStateDirty() after changes
```
