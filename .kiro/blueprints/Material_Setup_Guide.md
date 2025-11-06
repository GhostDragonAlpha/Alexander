# Material Setup Guide - Celestial Bodies

## Overview
Step-by-step guide for creating the three required materials in Unreal Engine Material Editor.

---

## Material 1: M_Sun_Emissive

### Purpose
Bright, glowing yellow material for the Sun with high emissive strength.

### Creation Steps

1. **Create Material:**
   - Content Browser → Navigate to `Content/Materials/`
   - Right-click → Material
   - Name: `M_Sun_Emissive`
   - Double-click to open Material Editor

2. **Material Properties:**
   - Details Panel → Material:
     - Material Domain: `Surface`
     - Blend Mode: `Opaque`
     - Shading Model: `Unlit` (or `Default Lit` with emissive)
     - Two Sided: `false`

3. **Node Graph:**

```
┌─────────────────┐
│ Constant3Vector │ RGB (1.0, 0.9, 0.6) - Warm Yellow
│   (Yellow)      │
└────────┬────────┘
         │
         ├──────────────→ Base Color (Material Output)
         │
         └─────┬──────→ [×] Multiply ──→ Emissive Color
               │              ↑
               │              │
               │        ┌─────┴──────┐
               │        │ Constant   │
               │        │   100.0    │
               │        └────────────┘
               │
               └──────────────────────→ (Optional) Opacity (if translucent)
```

4. **Detailed Node Setup:**

   **Node 1: Constant3Vector (Color)**
   - Right-click → Constant3Vector
   - R: `1.0`
   - G: `0.9`
   - B: `0.6`
   - Connect to: Base Color

   **Node 2: Constant (Emissive Multiplier)**
   - Right-click → Constant
   - Value: `100.0`

   **Node 3: Multiply**
   - Right-click → Multiply
   - Input A: Constant3Vector (color)
   - Input B: Constant (100.0)
   - Output → Emissive Color

5. **Alternative: Texture-Based Sun**

   If you have a sun texture:

```
┌─────────────────┐
│ TextureSample   │
│  (T_Sun_Diff)   │
└────────┬────────┘
         │
         ├──────────────→ Base Color
         │
         └─────┬──────→ [×] Multiply ──→ Emissive Color
               │              ↑
               │              │
               │        ┌─────┴──────┐
               │        │ Constant   │
               │        │   50.0     │
               │        └────────────┘
```

6. **Apply and Save:**
   - Click Apply
   - Click Save
   - Close Material Editor

7. **Usage:**
   - In BP_Sun → VisualMesh → Details → Materials
   - Element 0: `M_Sun_Emissive`

---

## Material 2: M_Earth_Simple

### Purpose
Blue planet material representing Earth (placeholder before NASA textures).

### Creation Steps

1. **Create Material:**
   - Content Browser → `Content/Materials/`
   - Right-click → Material
   - Name: `M_Earth_Simple`
   - Double-click to open

2. **Material Properties:**
   - Material Domain: `Surface`
   - Blend Mode: `Opaque`
   - Shading Model: `Default Lit`
   - Two Sided: `false`

3. **Node Graph:**

```
┌─────────────────┐
│ Constant3Vector │ RGB (0.1, 0.3, 0.8) - Blue
│    (Blue)       │
└────────┬────────┘
         └──────────────→ Base Color

┌─────────────────┐
│   Constant      │ 0.4
│  (Roughness)    │
└────────┬────────┘
         └──────────────→ Roughness

┌─────────────────┐
│   Constant      │ 0.0
│  (Metallic)     │
└────────┬────────┘
         └──────────────→ Metallic

┌─────────────────┐
│   Constant      │ 0.5
│  (Specular)     │
└────────┬────────┘
         └──────────────→ Specular
```

4. **Detailed Node Setup:**

   **Node 1: Base Color**
   - Constant3Vector
   - R: `0.1` (10%)
   - G: `0.3` (30%)
   - B: `0.8` (80%)
   - Connect to: Base Color

   **Node 2: Roughness**
   - Constant
   - Value: `0.4`
   - Connect to: Roughness

   **Node 3: Metallic**
   - Constant
   - Value: `0.0`
   - Connect to: Metallic

   **Node 4: Specular**
   - Constant
   - Value: `0.5`
   - Connect to: Specular

5. **Advanced: With NASA Texture**

   If you have Earth diffuse texture:

```
┌─────────────────────┐
│   TextureSample     │
│  (T_Earth_Diffuse)  │
└──────────┬──────────┘
           └─────────────→ Base Color

┌─────────────────────┐
│   TextureSample     │
│  (T_Earth_Normal)   │
└──────────┬──────────┘
           └─────────────→ Normal

┌─────────────────────┐
│   TextureSample     │
│ (T_Earth_Roughness) │
└──────────┬──────────┘
           └─────────────→ Roughness

┌─────────────────────┐
│   TextureSample     │
│  (T_Earth_Lights)   │ (City lights for night side)
└──────────┬──────────┘
           └─────────────→ Emissive Color
```

6. **Apply and Save:**
   - Click Apply
   - Click Save

7. **Usage:**
   - In BP_Earth → VisualMesh → Materials
   - Element 0: `M_Earth_Simple`

---

## Material 3: M_Moon_Simple

### Purpose
Gray, dusty material for the Moon with high roughness.

### Creation Steps

1. **Create Material:**
   - Content Browser → `Content/Materials/`
   - Right-click → Material
   - Name: `M_Moon_Simple`
   - Double-click to open

2. **Material Properties:**
   - Material Domain: `Surface`
   - Blend Mode: `Opaque`
   - Shading Model: `Default Lit`
   - Two Sided: `false`

3. **Node Graph:**

```
┌─────────────────┐
│ Constant3Vector │ RGB (0.4, 0.4, 0.4) - Gray
│     (Gray)      │
└────────┬────────┘
         └──────────────→ Base Color

┌─────────────────┐
│   Constant      │ 0.9
│  (Roughness)    │
└────────┬────────┘
         └──────────────→ Roughness

┌─────────────────┐
│   Constant      │ 0.0
│  (Metallic)     │
└────────┬────────┘
         └──────────────→ Metallic
```

4. **Detailed Node Setup:**

   **Node 1: Base Color**
   - Constant3Vector
   - R: `0.4` (40% gray)
   - G: `0.4`
   - B: `0.4`
   - Connect to: Base Color

   **Node 2: Roughness**
   - Constant
   - Value: `0.9` (very rough, dusty regolith)
   - Connect to: Roughness

   **Node 3: Metallic**
   - Constant
   - Value: `0.0` (non-metallic rock)
   - Connect to: Metallic

5. **Advanced: With Crater Normal Map**

   If you have a crater normal map:

```
┌─────────────────────┐
│   Constant3Vector   │ RGB (0.4, 0.4, 0.4)
│       (Gray)        │
└──────────┬──────────┘
           └─────────────→ Base Color

┌─────────────────────┐
│   TextureSample     │
│  (T_Moon_Normal)    │ (Crater details)
└──────────┬──────────┘
           └─────────────→ Normal

┌─────────────────────┐
│     Constant        │ 0.9
└──────────┬──────────┘
           └─────────────→ Roughness

┌─────────────────────┐
│     Constant        │ 0.0
└──────────┬──────────┘
           └─────────────→ Metallic
```

6. **Apply and Save:**
   - Click Apply
   - Click Save

7. **Usage:**
   - In BP_Moon → VisualMesh → Materials
   - Element 0: `M_Moon_Simple`

---

## Optional Material 4: M_Mars_Simple

### Purpose
Red-orange material for Mars.

### Quick Setup

```
Base Color: Constant3Vector (0.8, 0.4, 0.2) - Red-orange
Roughness: Constant (0.85) - Dusty
Metallic: Constant (0.0)
```

---

## Material Instances (Recommended)

For easier tweaking, create Material Instances:

### Create Master Material: M_Planet_Master

1. **Create Material:**
   - Name: `M_Planet_Master`

2. **Add Parameters:**

```
Parameters (convert constants to parameters):

┌──────────────────────┐
│ VectorParameter      │ "PlanetColor"
│  Default: (0.5, 0.5, │
│            0.5)      │
└─────────┬────────────┘
          └────────────────→ Base Color

┌──────────────────────┐
│ ScalarParameter      │ "Roughness"
│  Default: 0.5        │
└─────────┬────────────┘
          └────────────────→ Roughness

┌──────────────────────┐
│ ScalarParameter      │ "Metallic"
│  Default: 0.0        │
└─────────┬────────────┘
          └────────────────→ Metallic

┌──────────────────────┐
│ ScalarParameter      │ "EmissiveStrength"
│  Default: 0.0        │
└─────────┬────────────┘
          └─────┬─────────→ [×] Multiply ──→ Emissive Color
                │                  ↑
                │                  │
        ┌───────┴──────┐    ┌──────┴─────────┐
        │ PlanetColor  │    │ (color input)  │
        └──────────────┘    └────────────────┘
```

3. **Create Instances:**

   **MI_Earth:**
   - Right-click M_Planet_Master → Create Material Instance
   - Name: `MI_Earth`
   - PlanetColor: (0.1, 0.3, 0.8)
   - Roughness: 0.4
   - EmissiveStrength: 0.0

   **MI_Moon:**
   - Name: `MI_Moon`
   - PlanetColor: (0.4, 0.4, 0.4)
   - Roughness: 0.9
   - EmissiveStrength: 0.0

   **MI_Mars:**
   - Name: `MI_Mars`
   - PlanetColor: (0.8, 0.4, 0.2)
   - Roughness: 0.85
   - EmissiveStrength: 0.0

   **MI_Sun:**
   - Name: `MI_Sun`
   - PlanetColor: (1.0, 0.9, 0.6)
   - Roughness: 0.0
   - EmissiveStrength: 100.0

---

## Textures from NASA (Optional Enhancement)

### Download Free Textures

**Earth:**
- Source: https://visibleearth.nasa.gov/
- Recommended: "Blue Marble" 8K
- Files needed:
  - Base color: `T_Earth_Diffuse`
  - Normal map: `T_Earth_Normal`
  - Night lights: `T_Earth_Lights`

**Moon:**
- Source: https://svs.gsfc.nasa.gov/cgi-bin/details.cgi?aid=4720
- Files needed:
  - Base color: `T_Moon_Diffuse`
  - Normal map: `T_Moon_Normal`

**Mars:**
- Source: https://www.nasa.gov/mars-texture-maps
- Files needed:
  - Base color: `T_Mars_Diffuse`
  - Normal map: `T_Mars_Normal`

### Import to Unreal

1. **Download textures** (usually .jpg or .png)

2. **Import:**
   - Content Browser → `Content/Textures/Planets/`
   - Drag and drop texture files
   - Or: Right-click → Import to...

3. **Texture Settings:**
   - For diffuse/color maps:
     - sRGB: `true`
     - Compression: `Default (DXT1/5)`
   - For normal maps:
     - sRGB: `false`
     - Compression: `Normalmap`
     - Flip Green Channel: `false` (usually)

4. **Use in Materials:**
   - Open material (e.g., M_Earth_Simple)
   - Add node: TextureSample
   - Texture: Select imported texture
   - Connect RGB → Base Color
   - For normal maps: Connect RGB → Normal

---

## Material Testing Checklist

### M_Sun_Emissive
- [ ] Appears bright yellow/white
- [ ] Glows visibly (emissive)
- [ ] No shadows cast on itself
- [ ] Visible from far distances

### M_Earth_Simple
- [ ] Appears blue
- [ ] Responds to lighting (not too shiny)
- [ ] Looks like a planet from distance
- [ ] Roughness makes it look matte

### M_Moon_Simple
- [ ] Appears gray
- [ ] Very matte/dusty appearance
- [ ] Darker than Earth
- [ ] No shine or reflections

---

## Common Material Issues

### Issue: Material Appears Black

**Causes:**
- No lights in level
- Material is Unlit but has no emissive
- Textures failed to import

**Solutions:**
- Add DirectionalLight to level
- Check emissive color is set
- Verify textures imported correctly

### Issue: Material Too Shiny

**Causes:**
- Roughness too low
- Specular too high

**Solutions:**
- Increase Roughness (0.7-0.9 for planets)
- Decrease Specular (0.3-0.5)

### Issue: Sun Not Bright Enough

**Causes:**
- Emissive strength too low
- Post-processing tone mapping down

**Solutions:**
- Increase emissive multiplier (100+)
- Adjust PostProcessVolume exposure
- Enable Bloom in post-processing

### Issue: Performance Problems

**Causes:**
- Complex material with many texture samples
- High-resolution textures (8K+)

**Solutions:**
- Reduce texture resolution (2K or 4K)
- Use texture streaming
- Simplify material (fewer nodes)

---

## Quick Material Creation Summary

| Material | Base Color | Roughness | Metallic | Emissive | Shading |
|----------|------------|-----------|----------|----------|---------|
| **M_Sun_Emissive** | (1.0, 0.9, 0.6) | 0.0 | 0.0 | 100.0 | Unlit |
| **M_Earth_Simple** | (0.1, 0.3, 0.8) | 0.4 | 0.0 | 0.0 | Lit |
| **M_Moon_Simple** | (0.4, 0.4, 0.4) | 0.9 | 0.0 | 0.0 | Lit |
| **M_Mars_Simple** | (0.8, 0.4, 0.2) | 0.85 | 0.0 | 0.0 | Lit |

---

**Created:** 2025-01-05
**Version:** 1.0
**Estimated Time:** 15-20 minutes for all three materials
