# Flight Course Materials Library

## Overview
Unified material system for consistent visual appearance across all flight course objects. Uses master material with instances for easy customization.

## Material Architecture

### Master Material: M_FlightCourse_Master

**Purpose**: Base material with parameters for creating all flight course materials

**Parameters:**
- BaseColor (Vector3, Default: 0.5, 0.5, 1.0)
- EmissiveIntensity (Scalar, Default: 5.0, Range: 0-20)
- Roughness (Scalar, Default: 0.3, Range: 0-1)
- Metallic (Scalar, Default: 0.1, Range: 0-1)
- PulseSpeed (Scalar, Default: 1.0, Range: 0-5)
- UseAnimation (Boolean, Default: True)
- OpacityAmount (Scalar, Default: 1.0, Range: 0-1)

**Material Graph:**
```
BaseColor Parameter -> Base Color pin
BaseColor * EmissiveIntensity -> Emissive Color pin
Roughness Parameter -> Roughness pin
Metallic Parameter -> Metallic pin

If UseAnimation:
    Time * PulseSpeed -> Sine -> * 0.3 + 0.7 -> Multiply with Emissive

OpacityAmount -> Opacity pin (if translucent)
```

## Individual Materials

### 1. M_Waypoint_Glow
**Type**: Opaque with strong emissive
**Use**: Waypoint sphere meshes

**Properties:**
- Base Color: Cyan RGB(0, 255, 255)
- Emissive: 10 intensity
- Metallic: 0.1
- Roughness: 0.3
- Shading Model: Default Lit
- Two Sided: No

**Material Instance (MI_Waypoint):**
- Parent: M_FlightCourse_Master
- BaseColor: (0, 1, 1)
- EmissiveIntensity: 10
- PulseSpeed: 0 (static glow)
- UseAnimation: False

**Color Variations:**
- MI_Waypoint_Cyan: Default
- MI_Waypoint_Blue: (0.2, 0.4, 1.0)
- MI_Waypoint_Green: (0, 1, 0.5)
- MI_Waypoint_Purple: (0.7, 0.3, 1.0)

### 2. M_Ring_EnergyField
**Type**: Translucent with animation
**Use**: Ring energy portal effect

**Properties:**
- Base Color: Electric Blue RGB(51, 102, 255)
- Emissive: Pulsing 5-8 intensity
- Opacity: 0.4-0.7 (animated)
- Blend Mode: Translucent
- Shading Model: Default Lit
- Two Sided: Yes
- Fresnel: Edge glow effect

**Material Graph Details:**
```
1. Fresnel Node:
   - Exponent: 3.0
   - Base Reflect Fraction: 0.1
   - Output -> Multiply with BaseColor -> Emissive

2. Time Animation:
   - Time node -> Sine (Period = 2 * PI / PulseSpeed)
   - Sine output * 0.3 + 0.7 -> Modulates opacity
   - Sine output * 3 + 5 -> Modulates emissive intensity

3. Distortion (Optional):
   - Noise Texture (Panning UVs)
   - Speed: (0.1, 0.1)
   - Output -> Normal Map
   - Strength: 0.3
```

**Material Instance (MI_Ring_Energy):**
- Parent: Custom graph (not master due to complexity)
- BaseColor: (0.2, 0.4, 1.0)
- EmissiveIntensity: 6 (average)
- PulseSpeed: 2.0 (0.5 second cycle)
- UseAnimation: True

### 3. M_Particle_Glow
**Type**: Unlit additive for particles
**Use**: All particle systems (waypoints, rings)

**Properties:**
- Shading Model: Unlit
- Blend Mode: Additive
- Two Sided: Yes
- Disable Depth Test: No

**Material Graph:**
```
Particle Color node -> Multiply by EmissiveIntensity -> Emissive Color
Particle Alpha -> Opacity

Soft Particle Setup:
- Scene Depth - Pixel Depth -> Divide by SoftParticleDistance
- Saturate -> Multiply with Particle Alpha -> Opacity
```

**Parameters:**
- EmissiveIntensity: 2.0
- SoftParticleDistance: 50 units

**Material Instances:**
- MI_Particle_Cyan: For waypoints
- MI_Particle_Blue: For rings
- MI_Particle_Purple: For rings (alternate)

### 4. M_CourseBoundary
**Type**: Semi-transparent warning marker
**Use**: Course boundary/guide lights

**Properties:**
- Base Color: Orange-Yellow RGB(255, 200, 0)
- Emissive: 3 intensity
- Opacity: 0.5
- Blend Mode: Translucent
- Two Sided: Yes
- Animation: Slow pulse (2-second cycle)

**Material Instance (MI_Boundary):**
- Parent: M_FlightCourse_Master
- BaseColor: (1, 0.8, 0)
- EmissiveIntensity: 3
- PulseSpeed: 0.5
- OpacityAmount: 0.5
- UseAnimation: True

### 5. M_PlayerTrail
**Type**: Unlit additive for ribbon trails
**Use**: Player ship engine trail

**Properties:**
- Shading Model: Unlit
- Blend Mode: Additive
- Material Domain: Surface
- Blend Mode: Additive

**Material Graph:**
```
Trail Color (Parameter) -> Emissive
Trail Alpha (Parameter) -> Multiply by VertexColor Alpha -> Opacity

Dynamic Color:
- Input: Thrust Intensity (0-1)
- Low thrust: Blue (0.2, 0.4, 1.0)
- High thrust: White-hot (1.0, 0.9, 0.8)
```

**Material Instance (MI_PlayerTrail):**
- TrailColor: (0.5, 0.7, 1.0)
- TrailIntensity: 3.0
- FadeSpeed: 2.0

## Implementation Workflow

### Phase 1: Create Master Material
1. Content Browser -> /Game/Materials/FlightCourse/
2. Right-click -> Material -> Name: M_FlightCourse_Master
3. Add all parameters listed above
4. Build material graph
5. Save and compile

### Phase 2: Create Specialized Materials
1. M_Ring_EnergyField (custom graph for advanced effects)
2. M_Particle_Glow (unlit additive)
3. M_CourseBoundary (simple translucent)
4. M_PlayerTrail (ribbon compatible)

### Phase 3: Create Material Instances
1. Right-click M_FlightCourse_Master -> Create Material Instance
2. Name: MI_Waypoint_Cyan
3. Set parameters:
   - BaseColor: (0, 1, 1)
   - EmissiveIntensity: 10
4. Repeat for other variations

### Phase 4: Apply to Assets
**Waypoints (5 actors):**
```python
import unreal
waypoint = # get waypoint actor
material = unreal.load_asset('/Game/Materials/FlightCourse/MI_Waypoint_Cyan')
waypoint.static_mesh_component.set_material(0, material)
```

**Rings (4 actors):**
```python
ring = # get ring actor
material = unreal.load_asset('/Game/Materials/FlightCourse/MI_Ring_Energy')
ring.static_mesh_component.set_material(0, material)
```

**Particle Systems:**
- Open PS_Waypoint_Shimmer
- Required module -> Material: MI_Particle_Cyan
- Save

## Material Performance

**Shader Complexity:**
- Waypoint materials: LOW (simple emissive)
- Ring materials: MEDIUM (animated translucent)
- Particle materials: LOW (unlit additive)

**Optimization Tips:**
- Use material instances (cheap to render)
- Avoid complex noise in materials (pre-bake if possible)
- Translucent materials are more expensive than opaque
- Limit instruction count to <100 per material

**Target Performance:**
- All materials: <100 shader instructions
- Translucent overdraw: Minimal (rings are thin)
- Material instances: Nearly free (same shader, different params)

## Color Palette

**Primary Colors:**
- Waypoint Cyan: RGB(0, 255, 255) / HSV(180°, 100%, 100%)
- Ring Blue: RGB(51, 102, 255) / HSV(220°, 80%, 100%)
- Ring Purple: RGB(178, 76, 255) / HSV(270°, 70%, 100%)

**Accent Colors:**
- Boundary Orange: RGB(255, 200, 0) / HSV(47°, 100%, 100%)
- Trail Blue-White: RGB(127, 178, 255) / HSV(212°, 50%, 100%)

**Color Theory:**
- Cyan waypoints: High visibility, goal-oriented
- Blue/Purple rings: Energy field aesthetic
- Orange boundaries: Warning/attention
- Complementary color scheme: Blue vs Orange

## Testing Checklist
- [ ] M_FlightCourse_Master compiles without errors
- [ ] All material instances created
- [ ] Materials applied to waypoints (visible glow)
- [ ] Materials applied to rings (translucency works)
- [ ] Particle materials in particle systems
- [ ] Animation working (pulsing effect)
- [ ] Performance acceptable (60+ FPS)
- [ ] Colors match design specification
- [ ] Materials look good in different lighting conditions

## Next Steps
After materials complete:
- VP04: Add point lights matching material colors
- VP05: Create boundary markers with M_CourseBoundary
- VP06: Apply M_PlayerTrail to player ship
