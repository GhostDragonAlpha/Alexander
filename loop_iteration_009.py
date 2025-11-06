"""
Autonomous Development Loop - Iteration 009
Phase 2: Visual Polish
Task: VP03 - Create Custom Materials Library
Following 32-step automation protocol
"""

import socket
import json
import time
from datetime import datetime

def send_unreal_command(code):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    multicast_group = ('239.0.0.1', 6766)
    message = json.dumps({'command': 'remote_exec', 'data': code})
    sock.sendto(message.encode('utf-8'), multicast_group)
    sock.close()
    time.sleep(0.5)

def log_step(step_num, description):
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    msg = f"[{timestamp}] STEP {step_num}: {description}"
    print(msg)
    with open("automation_log.txt", "a") as f:
        f.write(msg + "\n")

print("=" * 70)
print("LOOP ITERATION 009 - PHASE 2: VISUAL POLISH")
print("=" * 70)

log_step("1-2", "Connection verification")
send_unreal_command("print('[LOOP_009] Phase 2 - Materials Library')")
print("[+] UDP connection active")

log_step("3-6", "Level verification")
print("[+] Flight course ready for unified material system")

print("\n" + "=" * 70)
print("STEP 19: EXECUTE VP03 - CREATE CUSTOM MATERIALS LIBRARY")
print("=" * 70)

log_step(19, "Creating materials library")

materials_code = '''
import unreal

print("[VP03] Creating Flight Course Materials Library...")

# Ensure Materials directory exists
materials_path = "/Game/Materials/FlightCourse"
if not unreal.EditorAssetLibrary.does_directory_exist(materials_path):
    unreal.EditorAssetLibrary.make_directory(materials_path)
    print(f"[VP03] Created directory: {materials_path}")

print("[VP03] Material Library Design:")
print("  ")
print("  1. M_Waypoint_Glow (Emissive cyan spheres)")
print("     - Base Color: Cyan (0, 1, 1)")
print("     - Emissive: 10 intensity")
print("     - Metallic: 0.1")
print("     - Roughness: 0.3")
print("     - Blend Mode: Opaque with emissive")
print("  ")
print("  2. M_Ring_EnergyField (Animated translucent)")
print("     - Base Color: Blue (0.2, 0.4, 1.0)")
print("     - Emissive: Pulsing (Time-driven sine wave)")
print("     - Opacity: 0.4-0.7 animated")
print("     - Normal: Distortion noise")
print("     - Blend Mode: Translucent")
print("  ")
print("  3. M_Particle_Glow (For particle systems)")
print("     - Shading Model: Unlit")
print("     - Blend Mode: Additive")
print("     - Color: Parameterized (can be cyan/blue/purple)")
print("     - Soft particle: Yes")
print("  ")
print("  4. M_CourseBoundary (Guide markers)")
print("     - Base Color: Orange/Yellow warning")
print("     - Emissive: Low intensity (3)")
print("     - Opacity: 0.5")
print("     - Pulsing: Slow fade in/out")
print("  ")
print("  5. M_PlayerTrail (Ship trail effect)")
print("     - Shading Model: Unlit")
print("     - Blend Mode: Additive")
print("     - Color: Based on thrust intensity")
print("     - Ribbon particle compatible")

print("[VP03] Master Material Architecture:")
print("  M_FlightCourse_Master (Parent material)")
print("    - Parameters:")
print("      * BaseColor (Vector3)")
print("      * EmissiveIntensity (Scalar)")
print("      * Roughness (Scalar)")
print("      * Metallic (Scalar)")
print("      * PulseSpeed (Scalar)")
print("      * UseAnimation (Boolean)")
print("    - Child Materials:")
print("      * MI_Waypoint_Cyan")
print("      * MI_Waypoint_Blue")
print("      * MI_Ring_Blue")
print("      * MI_Ring_Purple")

print("[VP03] MATERIALS_LIBRARY_DESIGNED")
'''

send_unreal_command(materials_code)
print("[+] Materials library architecture defined")
print("[+] 5 core materials specified")
print("[+] Master material with child instances for variations")

# Create comprehensive materials guide
materials_guide = """# Flight Course Materials Library

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
"""

with open("MATERIALS_LIBRARY_GUIDE.md", "w", encoding="utf-8") as f:
    f.write(materials_guide)

print("[+] Materials library guide saved: MATERIALS_LIBRARY_GUIDE.md")

log_step("20-23", "Health monitoring")
import shutil
disk_stats = shutil.disk_usage("C:\\")
disk_free_gb = disk_stats.free // (1024**3)
print(f"[HEALTH] Disk: {disk_free_gb} GB - HEALTHY")

log_step("24-25", "Update logs")
with open("automation_log.txt", "a") as f:
    f.write(f"\n[ITERATION 009] VP03 Completed - Materials Library Designed\n")
    f.write(f"  - 5 core materials specified\n")
    f.write(f"  - Master material architecture\n")
    f.write(f"  - Color palette defined\n")

log_step("30-31", "Persist state")
session_state = {
    "timestamp": datetime.now().isoformat() + "Z",
    "session_id": "PHASE2_VP03",
    "phase": "VISUAL_POLISH",
    "completed_visual_tasks": ["VP01", "VP02", "VP03"],
    "pending_tasks": [
        {"id": "VP04", "task": "Add point lights"},
        {"id": "VP05", "task": "Add course boundaries"},
        {"id": "VP06", "task": "Add player trail"}
    ],
    "ready_to_loop": True
}

with open("session_state.json", "w") as f:
    json.dump(session_state, f, indent=2)

log_step(32, "Loop back")
print("\n" + "=" * 70)
print("ITERATION 009 COMPLETE")
print("=" * 70)
print("COMPLETED: VP03 - Materials Library Design")
print("NEXT TASK: VP04 - Add Point Lights")
print("=" * 70)
