# Niagara VFX Creation Guide - Alexander Autonomous Baseline

**Total Systems:** 16 VFX Systems
**Estimated Total Time:** 4-6 hours (15-25 minutes per system)
**Difficulty:** Beginner to Intermediate
**Prerequisites:** Unreal Engine 5.6, Basic Editor Navigation

---

## Table of Contents
1. [Overview](#overview)
2. [Getting Started](#getting-started)
3. [Category 1: Ship Engines (5 systems)](#category-1-ship-engines)
4. [Category 2: Weapons (6 systems)](#category-2-weapons)
5. [Category 3: Environment (4 systems)](#category-3-environment)
6. [Category 4: UI Effects (2 systems)](#category-4-ui-effects)
7. [Troubleshooting](#troubleshooting)
8. [Validation Checklist](#validation-checklist)
9. [Optimization Tips](#optimization-tips)

---

## Overview

This guide provides step-by-step instructions for creating 16 Niagara VFX systems required for the Alexander autonomous baseline. Each system uses Unreal Engine's built-in Niagara templates as a starting point.

### VFX System Breakdown
- **Ship Engines:** 5 systems (engine trails for different ship types)
- **Weapons:** 6 systems (lasers, projectiles, explosions)
- **Environment:** 4 systems (space effects, nebulae, storms)
- **UI:** 2 systems (damage indicators, shield effects)

### Key Concepts
- **Niagara System:** The main VFX asset containing emitters
- **Emitter:** Individual particle effect within a system
- **Module:** Building blocks that control particle behavior
- **Parameters:** Values you can adjust (color, size, lifetime, etc.)

---

## Getting Started

### Step 1: Create Content Folder Structure

1. **Open Content Browser** (Ctrl+Space)
2. Navigate to `/Game/VFX/`
3. **Right-click** → **New Folder** → Name it `Generated`
4. Inside `Generated`, create 4 subfolders:
   - `ship_engines`
   - `weapons`
   - `environment`
   - `ui`

**Keyboard Shortcuts:**
- `Ctrl+Space` - Focus Content Browser
- `Ctrl+S` - Save selected asset
- `Ctrl+Shift+S` - Save all

### Step 2: Enable Niagara Plugin (if not already enabled)

1. **Edit** → **Plugins**
2. Search for "Niagara"
3. Ensure **Niagara VFX System** is checked
4. Restart editor if prompted

### Step 3: Locate Niagara Templates

Unreal Engine 5.6 includes starter templates at:
- **Content Browser** → **View Options** → Check "Show Engine Content"
- Navigate to: `/Engine/Niagara/Templates/`

---

## Category 1: Ship Engines

Engine trails provide visual feedback for ship movement and type. All use variations of smoke/trail effects.

### System 1: EngineTrail_Small
**Template:** NS_SmokeTrail (Smoke Trail)
**Category:** ship_engines
**Time:** 15-20 minutes
**Description:** Blue trail for small fighter ships

#### Step-by-Step Instructions:

1. **Navigate to:** `/Game/VFX/Generated/ship_engines/`

2. **Create Niagara System:**
   - Right-click in Content Browser
   - Select **FX** → **Niagara System**
   - Choose **New system from selected emitters**
   - In the template browser, select **Smoke Trail** (or `NS_SmokeTrail`)
   - Click **+** to add it
   - Click **Finish**
   - Name it: `NS_EngineTrail_Small`

3. **Open the System:**
   - Double-click `NS_EngineTrail_Small` to open Niagara Editor

4. **Modify Color Settings:**
   - In the **Emitter** panel (left side), expand the emitter
   - Find **Color** module (usually in "Particle Spawn" or "Particle Update")
   - Click on the **Color** module
   - In **Details** panel (right):
     - Set **Color Mode** to **Direct Set** (if not already)
     - Set **Color** to RGB: `(0.2, 0.5, 1.0)` (Blue)
     - To enter RGB values: Click color picker → Select RGB tab → Enter values
     - Alternatively, set HSV: H=210, S=80%, V=100%

5. **Adjust Intensity/Brightness:**
   - Still in **Color** module
   - Adjust **Alpha** (A) to `0.7`
   - Or find **Spawn Rate** module
   - Set **Spawn Rate** to `30-50` particles/second

6. **Adjust Size (Small):**
   - Find **Sprite Size** or **Scale** module
   - Set **Size** to: `X=8.0, Y=8.0, Z=8.0`
   - Or **Uniform Scale** to `0.5` (for smaller particles)

7. **Adjust Trail Length:**
   - Find **Lifetime** module
   - Set **Lifetime** to `1.0-2.0` seconds (shorter for small ship)

8. **Test the Effect:**
   - Click **Compile** button (top toolbar)
   - In the **Preview** viewport (center), you should see a blue smoke trail
   - Use mouse to rotate camera (Right Mouse Button + drag)
   - Use scroll wheel to zoom

9. **Save:**
   - Press `Ctrl+S`
   - Close Niagara Editor

**Expected Result:** A short, blue glowing trail suitable for small fighter ships.

---

### System 2: EngineTrail_Medium
**Template:** NS_SmokeTrail
**Category:** ship_engines
**Time:** 10-15 minutes
**Description:** Medium blue trail for transport ships

#### Quick Instructions:

1. **Duplicate Previous System:**
   - Right-click `NS_EngineTrail_Small`
   - Select **Duplicate**
   - Rename to: `NS_EngineTrail_Medium`

2. **Modify Settings:**
   - Open `NS_EngineTrail_Medium`
   - **Color:** RGB `(0.3, 0.6, 1.0)` (Lighter Blue)
   - **Alpha:** `0.8`
   - **Size:** `X=12.0, Y=12.0, Z=12.0` (or Uniform Scale `0.75`)
   - **Lifetime:** `2.0-3.0` seconds
   - **Spawn Rate:** `40-60` particles/second

3. **Compile and Save**

**Expected Result:** A medium-length, brighter blue trail with more particles.

---

### System 3: EngineTrail_Large
**Template:** NS_SmokeTrail
**Category:** ship_engines
**Time:** 10-15 minutes
**Description:** Large bright blue trail for freighters

#### Quick Instructions:

1. **Duplicate Medium System:**
   - Right-click `NS_EngineTrail_Medium` → Duplicate
   - Rename to: `NS_EngineTrail_Large`

2. **Modify Settings:**
   - **Color:** RGB `(0.4, 0.7, 1.0)` (Even Lighter Blue)
   - **Alpha:** `1.0` (Full intensity)
   - **Size:** `X=20.0, Y=20.0, Z=20.0` (or Uniform Scale `1.2`)
   - **Lifetime:** `3.0-5.0` seconds (long trail)
   - **Spawn Rate:** `60-100` particles/second

3. **Compile and Save**

**Expected Result:** A long, dense, bright blue trail for large ships.

---

### System 4: EngineTrail_Military
**Template:** NS_Fire (Fire/Flame effect)
**Category:** ship_engines
**Time:** 15-20 minutes
**Description:** Orange-red military engine trail

#### Step-by-Step Instructions:

1. **Create New System:**
   - Navigate to: `/Game/VFX/Generated/ship_engines/`
   - Right-click → **FX** → **Niagara System**
   - Select template: **Fire** or **Flame** (look for orange/fire effect)
   - Name: `NS_EngineTrail_Military`

2. **Modify Color:**
   - Open system
   - Find **Color** module
   - Set RGB to `(1.0, 0.4, 0.1)` (Orange-Red)
     - HSV equivalent: H=20, S=90%, V=100%
   - Set **Alpha** to `0.9`

3. **Adjust Size (Medium):**
   - **Size:** `X=15.0, Y=15.0, Z=15.0`
   - Or **Uniform Scale:** `0.9`

4. **Optional - Add Glow:**
   - Find **Material** module
   - Check if material has **Emissive** properties
   - Increase **Emissive Boost** to `2.0-3.0` for extra glow

5. **Compile and Save**

**Expected Result:** An aggressive orange-red flame trail suitable for military combat ships.

---

### System 5: EngineTrail_Racing
**Template:** NS_Electric (Electric/Energy effect)
**Category:** ship_engines
**Time:** 15-20 minutes
**Description:** Bright blue plasma trail for racing ships

#### Step-by-Step Instructions:

1. **Create New System:**
   - Navigate to: `/Game/VFX/Generated/ship_engines/`
   - Right-click → **FX** → **Niagara System**
   - Select template: **Electric** or **Energy Beam** (look for electrical arcs)
   - If not available, use **Smoke Trail** and modify heavily
   - Name: `NS_EngineTrail_Racing`

2. **Modify Color:**
   - RGB: `(0.1, 0.8, 1.0)` (Bright Cyan/Electric Blue)
     - HSV: H=190, S=90%, V=100%
   - **Alpha:** `1.0` (Full brightness)

3. **Increase Intensity:**
   - **Spawn Rate:** `80-120` particles/second
   - **Emissive Boost:** `3.0-5.0` (very bright)

4. **Adjust Size (Small but Bright):**
   - **Size:** `X=10.0, Y=10.0, Z=10.0`

5. **Add Energy Feel:**
   - Find **Velocity** module
   - Add slight **Curl Noise** or **Turbulence** (if available)
   - Set **Noise Strength** to `5-10`

6. **Compile and Save**

**Expected Result:** A bright, energetic electric blue trail with slight turbulence.

---

## Category 2: Weapons

Weapon effects include beams, trails, impacts, and explosions.

### System 6: Weapon_LaserBeam
**Template:** NS_BeamTrail (Beam effect)
**Category:** weapons
**Time:** 20-25 minutes
**Description:** Continuous red laser beam

#### Step-by-Step Instructions:

1. **Create New System:**
   - Navigate to: `/Game/VFX/Generated/weapons/`
   - Right-click → **FX** → **Niagara System**
   - Select template: **Beam** or **Laser Beam**
   - Name: `NS_Weapon_LaserBeam`

2. **Modify Beam Color:**
   - Open system
   - Find **Beam Emitter**
   - Locate **Color** module
   - Set RGB to `(1.0, 0.2, 0.2)` (Bright Red)
     - HSV: H=0, S=80%, V=100%

3. **Configure Beam Settings:**
   - Find **Beam** module or **Beam Setup**
   - **Beam Length:** Set to `1000.0` units (or desired range)
   - **Beam Width:** `2.0-5.0` units

4. **Add Glow Effect:**
   - **Emissive Boost:** `4.0-6.0` (very bright for laser)
   - Consider adding a second emitter for glow/halo effect

5. **Set to Continuous:**
   - Find **Emitter State** or **Loop Behavior**
   - Set to **Infinite** or **Loop**
   - **Duration:** Infinite

6. **Test Beam:**
   - In preview, ensure beam extends from origin
   - Beam should be solid and bright red

7. **Compile and Save**

**Expected Result:** A bright red continuous laser beam that can be aimed.

**Advanced Tip:** Add a second "core" emitter with white color for center of beam, red for outer glow.

---

### System 7: Weapon_LaserImpact
**Template:** NS_Sparks (Spark burst)
**Category:** weapons
**Time:** 15-20 minutes
**Description:** Yellow-orange sparks when laser hits

#### Step-by-Step Instructions:

1. **Create New System:**
   - Navigate to: `/Game/VFX/Generated/weapons/`
   - Right-click → **FX** → **Niagara System**
   - Select template: **Sparks** or **Hit Effect**
   - Name: `NS_Weapon_LaserImpact`

2. **Modify Color:**
   - RGB: `(1.0, 0.8, 0.3)` (Yellow-Orange)
     - HSV: H=45, S=70%, V=100%

3. **Configure Burst Behavior:**
   - Find **Spawn Burst Instantaneous** module
   - Set **Spawn Count:** `20-50` particles (one-time burst)
   - Find **Emitter State**
   - Set **Loop Behavior:** **Once** (not looping)
   - Set **Duration:** `0.5-1.0` seconds

4. **Adjust Particle Properties:**
   - **Size:** `X=3.0, Y=3.0, Z=3.0` (small sparks)
   - **Lifetime:** `0.2-0.5` seconds (quick fade)
   - **Velocity:** Add outward velocity `100-200` units/sec
   - **Gravity:** Add slight gravity `-980` (standard gravity)

5. **Test Burst:**
   - Compile and watch preview
   - Should see a quick burst of sparks that fall and fade

6. **Save**

**Expected Result:** A brief burst of yellow-orange sparks radiating outward.

---

### System 8: Weapon_ProjectileTrail
**Template:** NS_SmokeTrail
**Category:** weapons
**Time:** 10-15 minutes
**Description:** Orange smoke trail for projectiles

#### Quick Instructions:

1. **Create from Template:**
   - Navigate to: `/Game/VFX/Generated/weapons/`
   - Create Niagara System from **Smoke Trail** template
   - Name: `NS_Weapon_ProjectileTrail`

2. **Modify Settings:**
   - **Color:** RGB `(1.0, 0.5, 0.0)` (Orange)
   - **Size:** `X=6.0, Y=6.0, Z=6.0` (small trail)
   - **Lifetime:** `0.5-1.0` seconds (short trail)
   - **Spawn Rate:** `30-50` particles/second
   - **Alpha:** `0.7`

3. **Compile and Save**

**Expected Result:** A short orange smoke trail following projectiles.

---

### System 9: Weapon_ProjectileExplosion
**Template:** NS_Explosion
**Category:** weapons
**Time:** 20-25 minutes
**Description:** Red-orange explosion for projectile hits

#### Step-by-Step Instructions:

1. **Create New System:**
   - Navigate to: `/Game/VFX/Generated/weapons/`
   - Right-click → **FX** → **Niagara System**
   - Select template: **Explosion**
   - Name: `NS_Weapon_ProjectileExplosion`

2. **Modify Explosion Color:**
   - RGB: `(1.0, 0.3, 0.0)` (Red-Orange)
     - HSV: H=18, S=100%, V=100%
   - **Alpha:** Full brightness

3. **Configure Burst:**
   - **Loop Behavior:** **Once**
   - **Duration:** `1.0-2.0` seconds total
   - **Spawn Burst:** `50-100` particles

4. **Adjust Explosion Size:**
   - **Initial Size:** `X=20.0, Y=20.0, Z=20.0`
   - Find **Scale Over Life** module (if available)
   - Set **Start Scale:** `0.5`
   - Set **End Scale:** `2.0` (expands over time)

5. **Add Shockwave (Optional):**
   - Some explosion templates have multiple emitters
   - Look for "Shockwave" or "Ring" emitter
   - Adjust its **Color** to match (lighter orange)
   - Set **Scale** larger for visible shockwave

6. **Test Explosion:**
   - Should see rapid expansion and fade
   - Bright at start, dimmer at end

7. **Compile and Save**

**Expected Result:** A dramatic red-orange explosion with expanding fireball.

---

### System 10: Weapon_MissileTrail
**Template:** NS_SmokeTrail
**Category:** weapons
**Time:** 10-15 minutes
**Description:** Gray smoke trail for missiles

#### Quick Instructions:

1. **Create from Template:**
   - Navigate to: `/Game/VFX/Generated/weapons/`
   - Create Niagara System from **Smoke Trail**
   - Name: `NS_Weapon_MissileTrail`

2. **Modify Settings:**
   - **Color:** RGB `(0.8, 0.8, 0.8)` (Gray/White smoke)
   - **Size:** `X=15.0, Y=15.0, Z=15.0` (thick smoke)
   - **Lifetime:** `3.0-5.0` seconds (long-lasting trail)
   - **Spawn Rate:** `50-80` particles/second
   - **Alpha:** `0.6-0.8`

3. **Add Turbulence:**
   - Find **Curl Noise** or **Turbulence** module
   - Set **Strength:** `10-20` (for realistic smoke movement)

4. **Compile and Save**

**Expected Result:** A thick, realistic gray smoke trail with turbulence.

---

### System 11: Weapon_MissileExplosion
**Template:** NS_Explosion
**Category:** weapons
**Time:** 20-25 minutes
**Description:** Large orange explosion for missiles

#### Quick Instructions:

1. **Duplicate ProjectileExplosion:**
   - Navigate to: `/Game/VFX/Generated/weapons/`
   - Right-click `NS_Weapon_ProjectileExplosion` → Duplicate
   - Rename to: `NS_Weapon_MissileExplosion`

2. **Scale Up:**
   - **Color:** RGB `(1.0, 0.4, 0.0)` (Bright Orange)
   - **Spawn Burst:** `100-200` particles (larger explosion)
   - **Initial Size:** `X=40.0, Y=40.0, Z=40.0` (double size)
   - **Scale Over Life End:** `3.0-4.0` (even larger expansion)
   - **Duration:** `2.0-3.0` seconds

3. **Add Secondary Effects (Optional):**
   - Add second emitter for debris
   - Add third emitter for smoke cloud

4. **Compile and Save**

**Expected Result:** A massive orange explosion, significantly larger than projectile explosion.

---

## Category 3: Environment

Environmental effects add atmosphere to space scenes.

### System 12: Env_SolarWind
**Template:** NS_ParticleFlow (Directional particles)
**Category:** environment
**Time:** 20-25 minutes
**Description:** Pale yellow solar wind particles

#### Step-by-Step Instructions:

1. **Create New System:**
   - Navigate to: `/Game/VFX/Generated/environment/`
   - Right-click → **FX** → **Niagara System**
   - Look for **Particle Flow**, **Directional Particles**, or **Stream**
   - If not available, use **Simple Sprite Burst** and modify
   - Name: `NS_Env_SolarWind`

2. **Configure Continuous Flow:**
   - **Loop Behavior:** **Infinite**
   - **Spawn Rate:** `100-200` particles/second (dense flow)

3. **Set Color:**
   - RGB: `(1.0, 0.9, 0.7)` (Pale Yellow/Cream)
   - **Alpha:** `0.3-0.5` (subtle, not overpowering)

4. **Configure Movement:**
   - Find **Velocity** module
   - Set **Velocity:** `(500, 0, 0)` (flowing in one direction)
   - Or use **Point Attraction** to simulate solar gravity

5. **Adjust Particle Appearance:**
   - **Size:** `X=2.0, Y=2.0, Z=2.0` (small particles)
   - **Lifetime:** `5.0-10.0` seconds
   - Consider adding **Rotation** for visual variety

6. **Set Large Spawn Volume:**
   - Find **Spawn Location** module
   - Set to **Box** or **Sphere**
   - Set **Box Dimensions:** `X=1000, Y=1000, Z=1000` (large area)

7. **Test:**
   - Should see continuous stream of pale particles
   - Flowing in consistent direction

8. **Compile and Save**

**Expected Result:** Subtle pale yellow particles flowing directionally, like solar wind.

---

### System 13: Env_AsteroidDebris
**Template:** NS_Dust (Dust/particle cloud)
**Category:** environment
**Time:** 15-20 minutes
**Description:** Gray debris field

#### Step-by-Step Instructions:

1. **Create New System:**
   - Navigate to: `/Game/VFX/Generated/environment/`
   - Create from **Dust** or **Fog** template
   - Name: `NS_Env_AsteroidDebris`

2. **Set Color:**
   - RGB: `(0.5, 0.5, 0.5)` (Medium Gray)
   - **Alpha:** `0.4-0.6`

3. **Configure as Dense Cloud:**
   - **Spawn Rate:** `50-100` particles/second
   - **Spawn Volume:** Large box `(2000, 2000, 2000)`
   - **Lifetime:** `10.0-20.0` seconds (persistent)

4. **Add Varied Sizes:**
   - Find **Size** module
   - Set **Size Range:** Min `5.0`, Max `20.0`
   - This creates varied debris sizes

5. **Add Slow Rotation:**
   - Find **Mesh Rotation** or **Sprite Rotation**
   - Set **Rotation Rate:** `10-30` degrees/second

6. **Optional - Use Mesh:**
   - If you want 3D debris instead of sprites
   - Add **Mesh Renderer** module
   - Assign simple rock mesh

7. **Compile and Save**

**Expected Result:** A field of gray particles/debris floating in space.

---

### System 14: Env_NebulaFog
**Template:** NS_Fog (Volumetric fog)
**Category:** environment
**Time:** 15-20 minutes
**Description:** Purple nebula clouds

#### Step-by-Step Instructions:

1. **Create New System:**
   - Navigate to: `/Game/VFX/Generated/environment/`
   - Create from **Fog** or **Cloud** template
   - Name: `NS_Env_NebulaFog`

2. **Set Color:**
   - RGB: `(0.6, 0.3, 0.8)` (Purple/Violet)
     - HSV: H=270, S=62%, V=80%
   - **Alpha:** `0.3-0.5` (translucent fog)

3. **Configure Large Volume:**
   - **Spawn Volume:** Very large box `(5000, 5000, 5000)`
   - **Spawn Rate:** `20-50` particles/second (sparse but large)

4. **Adjust Fog Properties:**
   - **Size:** Very large `X=100.0, Y=100.0, Z=100.0`
   - **Lifetime:** `30.0-60.0` seconds (very persistent)

5. **Add Slow Movement:**
   - **Velocity:** Very slow `(10, 5, 0)`
   - Add **Curl Noise** with low strength `5.0`

6. **Set Material:**
   - Ensure using **SubUV** or **Fog** material
   - Should have soft edges, not hard sprites

7. **Compile and Save**

**Expected Result:** Large, slow-moving purple fog clouds creating nebula atmosphere.

---

### System 15: Env_ElectricalStorm
**Template:** NS_Lightning (Lightning arcs)
**Category:** environment
**Time:** 25-30 minutes
**Description:** Electric blue lightning arcs

#### Step-by-Step Instructions:

1. **Create New System:**
   - Navigate to: `/Game/VFX/Generated/environment/`
   - Create from **Lightning** or **Electric Arc** template
   - If not available, use **Beam** and modify for lightning
   - Name: `NS_Env_ElectricalStorm`

2. **Set Color:**
   - RGB: `(0.3, 0.7, 1.0)` (Electric Blue)
   - **Emissive Boost:** `5.0-8.0` (very bright)

3. **Configure Lightning Behavior:**
   - **Loop Behavior:** Infinite
   - **Spawn Burst:** Random bursts
   - Find **Spawn Rate** or **Burst**
   - Set to spawn intermittently (e.g., every 2-5 seconds)

4. **Lightning Arc Settings:**
   - **Beam Length:** `500-2000` units (varied)
   - **Beam Width:** `5.0-10.0` units
   - Add **Noise** to beam path for jagged lightning effect
   - **Noise Strength:** `50-100`

5. **Add Flickering:**
   - Find **Color Over Life** or **Alpha Over Life**
   - Create curve that flashes bright then fades
   - Or use **Scale Over Life** to pulse

6. **Add Secondary Glow:**
   - Consider adding second emitter for glow/bloom
   - White or light blue color
   - Larger size, lower alpha

7. **Test:**
   - Should see intermittent bright blue arcs
   - Jagged, lightning-like appearance

8. **Compile and Save**

**Expected Result:** Dramatic electric blue lightning arcs that flash intermittently.

**Advanced Tip:** Add sound cue trigger for thunder sound effect.

---

## Category 4: UI Effects

UI effects are typically screen-space or attached to UI elements.

### System 16: UI_DamageIndicator
**Template:** NS_Sparks
**Category:** ui
**Time:** 15-20 minutes
**Description:** Red damage indicator particles

#### Step-by-Step Instructions:

1. **Create New System:**
   - Navigate to: `/Game/VFX/Generated/ui/`
   - Create from **Sparks** template
   - Name: `NS_UI_DamageIndicator`

2. **Set Color:**
   - RGB: `(1.0, 0.0, 0.0)` (Pure Red)
   - **Alpha:** `0.8-1.0`

3. **Configure for UI:**
   - **Spawn Burst:** `10-20` particles
   - **Loop:** Once (triggered on damage)
   - **Duration:** `0.3-0.5` seconds (quick flash)

4. **Adjust for Screen Space:**
   - **Size:** `X=5.0, Y=5.0, Z=5.0` (visible but not huge)
   - **Velocity:** Outward from center `50-100` units/sec
   - **Lifetime:** `0.2-0.4` seconds

5. **Add Alpha Fade:**
   - Find **Alpha Over Life** module
   - Create curve: Start `1.0`, End `0.0`
   - Quick fade out

6. **Optional - Screen Alignment:**
   - In **Renderer** settings
   - Set **Alignment:** **Screen** or **Camera-Facing**
   - This keeps particles facing camera

7. **Compile and Save**

**Expected Result:** A quick burst of red particles indicating damage taken.

**Integration Note:** This will typically be spawned programmatically when player takes damage, positioned at hit location or screen edge indicating damage direction.

---

### System 17: UI_ShieldHit
**Template:** NS_Ripple (Ripple/wave effect)
**Category:** ui
**Time:** 20-25 minutes
**Description:** Blue shield ripple effect

#### Step-by-Step Instructions:

1. **Create New System:**
   - Navigate to: `/Game/VFX/Generated/ui/`
   - Create from **Ripple**, **Shockwave**, or **Impact Ring** template
   - If not available, create custom with expanding ring
   - Name: `NS_UI_ShieldHit`

2. **Set Color:**
   - RGB: `(0.0, 0.5, 1.0)` (Blue)
   - **Alpha:** Start `0.8`, fade to `0.0`

3. **Configure Ripple Behavior:**
   - **Spawn Burst:** `1-3` particles (ripple waves)
   - **Loop:** Once
   - **Duration:** `0.5-1.0` seconds

4. **Set Expansion:**
   - Find **Scale Over Life** module
   - **Start Scale:** `0.5`
   - **End Scale:** `3.0-5.0` (expands outward)
   - Create curve for smooth expansion

5. **Configure Alpha Fade:**
   - **Alpha Over Life:** Start `1.0`, End `0.0`
   - Fade as it expands

6. **Shield Surface Effect:**
   - **Shape:** Ring or Sphere
   - **Thickness:** `2.0-5.0` units
   - **Material:** Should have some transparency

7. **Optional - Add Hexagon Pattern:**
   - If using custom material
   - Add hexagonal grid texture
   - Gives "energy shield" appearance

8. **Test:**
   - Should see expanding blue ring/ripple
   - Fades as it grows

9. **Compile and Save**

**Expected Result:** An expanding blue ripple effect indicating shield impact.

**Integration Note:** Spawn at hit location on shield surface, oriented to face attacker.

---

## Troubleshooting

### Problem: Can't Find Templates

**Solution:**
1. Enable "Show Engine Content" in Content Browser
2. Navigate to `/Engine/Niagara/Templates/`
3. If templates don't exist, use **Niagara System** wizard
4. Select **Create Empty System** and build from scratch
5. Or search Unreal Marketplace for "Niagara Starter Pack"

### Problem: Colors Not Showing Correctly

**Solutions:**
- Check material has **Emissive** input connected
- Increase **Emissive Boost** multiplier
- Verify **Color Mode** is set correctly (not multiply)
- Check **Alpha** is not 0
- Ensure **Particle Color** module exists

### Problem: Particles Too Small/Large

**Solutions:**
- Adjust **Sprite Size** or **Scale** module
- Check **Scale Over Life** curve
- Verify **Uniform Scale** multiplier
- Camera distance affects perceived size - zoom in/out

### Problem: Effect Not Looping

**Solutions:**
- Check **Emitter State** → **Loop Behavior** = **Infinite**
- Set **Loop Duration** to appropriate value
- Ensure **Spawn Rate** is continuous (not burst only)

### Problem: Effect Looks Pixelated

**Solutions:**
- Increase particle count (**Spawn Rate**)
- Increase particle size overlap
- Check material texture resolution
- Enable **Motion Blur** in renderer settings

### Problem: Performance Issues / Lag

**Solutions:**
- Reduce **Spawn Rate**
- Decrease **Max Particle Count**
- Lower **Lifetime**
- Simplify material (fewer instructions)
- Use GPU simulation instead of CPU
- Enable **LOD** (Level of Detail) settings

### Problem: Can't See Effect in Preview

**Solutions:**
- Click **Compile** button
- Check **Preview** viewport is active
- Adjust camera position/zoom
- Check **Background Color** isn't hiding effect
- Verify emitter is enabled (not disabled)

### Problem: Effect Stops After Brief Time

**Solutions:**
- Set **Loop Behavior** to **Infinite** or **Loop**
- Increase **Emitter Duration**
- Check **Spawn** modules are active throughout duration

---

## Validation Checklist

Use this checklist to verify each VFX system is working correctly.

### Per-System Checklist

For each VFX system, verify:

- [ ] **Compiles without errors** - No red error messages
- [ ] **Visible in preview** - Can see effect in Niagara Editor
- [ ] **Correct color** - Matches RGB specification
- [ ] **Appropriate size** - Small/medium/large as specified
- [ ] **Proper behavior** - Loops, bursts, or beams as intended
- [ ] **Saved in correct folder** - Organized by category
- [ ] **Named correctly** - Follows naming convention `NS_[Category]_[Name]`

### Category Validation

#### Ship Engines (5 systems)
- [ ] All 5 engine trails created
- [ ] Color progression visible: Small (darker blue) → Medium → Large (lighter blue)
- [ ] Military trail is orange/red, not blue
- [ ] Racing trail is bright electric blue
- [ ] All trails follow movement smoothly

#### Weapons (6 systems)
- [ ] Laser beam is continuous (not burst)
- [ ] Impact effects are bursts (not continuous)
- [ ] Explosions expand and fade
- [ ] Missile explosion noticeably larger than projectile explosion
- [ ] Trail effects follow projectile movement

#### Environment (4 systems)
- [ ] Solar wind flows in consistent direction
- [ ] Debris field is static/slow-moving
- [ ] Nebula fog is large and translucent
- [ ] Lightning arcs are intermittent, not constant

#### UI (2 systems)
- [ ] Damage indicator bursts quickly
- [ ] Shield hit expands outward as ripple
- [ ] Both are camera-facing

### In-Game Testing

After creating all systems, test in actual game:

1. **Place in Test Level:**
   - Create test map
   - Drag each Niagara System into level
   - Verify visibility at different distances

2. **Attach to Objects:**
   - Attach engine trails to ships
   - Test weapon effects on projectile actors
   - Verify effects move with parent objects

3. **Performance Check:**
   - Monitor FPS with all effects active
   - Use `stat particles` console command
   - Verify particle count is reasonable

4. **Visual Polish:**
   - Check effects blend well with environment
   - Verify colors are distinct (no confusion)
   - Test in different lighting conditions

---

## Optimization Tips

### Performance Best Practices

1. **Use GPU Simulation:**
   - In Emitter properties, set **Simulation Target** to **GPU Compute**
   - Much faster for high particle counts

2. **Limit Max Particles:**
   - Set **Max Particle Count** to reasonable value (e.g., 500-1000)
   - Prevents unlimited spawning

3. **Use LOD (Level of Detail):**
   - Add LOD Distance settings
   - Reduce particle count at far distances
   - Disable effects beyond certain range

4. **Optimize Materials:**
   - Use simple materials (fewer texture lookups)
   - Avoid complex math in material graph
   - Use **Unlit** shader model if possible

5. **Batch Similar Effects:**
   - Group similar particle systems together
   - Reduces draw calls

### Visual Quality Tips

1. **Layer Effects:**
   - Combine multiple emitters for richer effects
   - Example: Core + Glow + Sparks for explosions

2. **Use Color Gradients:**
   - **Color Over Life** creates dynamic appearance
   - Example: Orange → Red → Black for fire

3. **Scale Over Lifetime:**
   - Particles that grow/shrink look more dynamic
   - Good for explosions, impacts

4. **Add Rotation:**
   - Rotating particles add visual interest
   - Especially for debris, smoke

5. **Velocity Inheritance:**
   - Particles inherit parent object velocity
   - Creates realistic motion for trails

### Reusability

1. **Create Base Templates:**
   - Make generic versions (e.g., "BaseExplosion")
   - Expose key parameters
   - Derive specific versions from base

2. **Use Parameters:**
   - Expose **Color**, **Size**, **Intensity** as user parameters
   - Allows runtime modification without duplicating systems

3. **Material Instances:**
   - Create master materials for particles
   - Use material instances to vary colors
   - More efficient than unique materials

---

## Quick Reference Tables

### Color Reference (RGB)

| VFX System | RGB Values | Description |
|------------|------------|-------------|
| EngineTrail_Small | (0.2, 0.5, 1.0) | Medium Blue |
| EngineTrail_Medium | (0.3, 0.6, 1.0) | Light Blue |
| EngineTrail_Large | (0.4, 0.7, 1.0) | Lighter Blue |
| EngineTrail_Military | (1.0, 0.4, 0.1) | Orange-Red |
| EngineTrail_Racing | (0.1, 0.8, 1.0) | Bright Cyan |
| Weapon_LaserBeam | (1.0, 0.2, 0.2) | Red |
| Weapon_LaserImpact | (1.0, 0.8, 0.3) | Yellow-Orange |
| Weapon_ProjectileTrail | (1.0, 0.5, 0.0) | Orange |
| Weapon_ProjectileExplosion | (1.0, 0.3, 0.0) | Red-Orange |
| Weapon_MissileTrail | (0.8, 0.8, 0.8) | Gray |
| Weapon_MissileExplosion | (1.0, 0.4, 0.0) | Bright Orange |
| Env_SolarWind | (1.0, 0.9, 0.7) | Pale Yellow |
| Env_AsteroidDebris | (0.5, 0.5, 0.5) | Gray |
| Env_NebulaFog | (0.6, 0.3, 0.8) | Purple |
| Env_ElectricalStorm | (0.3, 0.7, 1.0) | Electric Blue |
| UI_DamageIndicator | (1.0, 0.0, 0.0) | Pure Red |
| UI_ShieldHit | (0.0, 0.5, 1.0) | Blue |

### Template Mapping

| Template Name | Used For | Count |
|---------------|----------|-------|
| NS_SmokeTrail | Engine trails, projectile trails, missile trail | 7 |
| NS_Fire | Military engine | 1 |
| NS_Electric | Racing engine, electrical storm | 2 |
| NS_BeamTrail | Laser beam | 1 |
| NS_Sparks | Laser impact, damage indicator | 2 |
| NS_Explosion | Projectile/missile explosions | 2 |
| NS_ParticleFlow | Solar wind | 1 |
| NS_Dust | Asteroid debris | 1 |
| NS_Fog | Nebula fog | 1 |
| NS_Lightning | Electrical storm | 1 |
| NS_Ripple | Shield hit | 1 |

### Time Estimates

| Category | Systems | Estimated Time |
|----------|---------|----------------|
| Ship Engines | 5 | 60-80 minutes |
| Weapons | 6 | 90-120 minutes |
| Environment | 4 | 75-95 minutes |
| UI | 2 | 35-45 minutes |
| **TOTAL** | **16** | **4-6 hours** |

Note: Times include creation, customization, and testing. Experienced users may be faster.

---

## Next Steps

After creating all 16 VFX systems:

1. **Export VFX Config:**
   - Run Python script: `autonomous_vfx_generator.export_vfx_config()`
   - Verify `vfx_config.json` is created

2. **Integration:**
   - Attach engine trails to ship Blueprints
   - Hook weapon effects to combat system
   - Add environment effects to space levels
   - Integrate UI effects with HUD system

3. **Testing:**
   - Create test level with all effects
   - Performance profiling (`stat fps`, `stat particles`)
   - Visual verification across all lighting conditions

4. **Documentation:**
   - Take screenshots of each effect
   - Document any custom modifications
   - Note performance metrics

5. **Version Control:**
   - Commit all Niagara assets to source control
   - Tag as "VFX_Baseline_v1.0"

---

## Keyboard Shortcuts Reference

| Shortcut | Action |
|----------|--------|
| `Ctrl+Space` | Focus Content Browser |
| `Ctrl+S` | Save current asset |
| `Ctrl+Shift+S` | Save all |
| `Ctrl+D` | Duplicate selected asset |
| `F2` | Rename selected asset |
| `Right Mouse + Drag` | Rotate preview camera |
| `Middle Mouse + Drag` | Pan preview camera |
| `Mouse Wheel` | Zoom preview camera |
| `Spacebar` | Play/Pause preview |
| `R` | Reset simulation |
| `Alt+P` | Open project settings |

---

## Support Resources

- **Official Niagara Documentation:** [Unreal Engine Niagara VFX](https://docs.unrealengine.com/en-US/niagara-visual-effects-in-unreal-engine/)
- **Video Tutorials:** Search "Unreal Engine Niagara Tutorial" on YouTube
- **Community:** Unreal Engine Forums - VFX Section
- **Marketplace:** Free Niagara starter packs available

---

**Document Version:** 1.0
**Last Updated:** 2025-11-08
**Created For:** Alexander Autonomous Baseline Phase 9

---

## Summary

This guide provides complete instructions for creating 16 Niagara VFX systems organized into 4 categories: Ship Engines (5), Weapons (6), Environment (4), and UI (2). Each system includes step-by-step creation instructions, exact parameter values, and validation criteria. Estimated total creation time is 4-6 hours for someone with basic Unreal Engine knowledge.

Key features:
- Detailed step-by-step instructions with exact menu paths
- RGB color specifications for each system
- Template recommendations for each effect type
- Troubleshooting section for common issues
- Validation checklist to ensure quality
- Optimization tips for performance
- Quick reference tables for colors and templates
- Keyboard shortcuts for efficiency

All systems follow Unreal Engine best practices and are designed for the Alexander autonomous baseline requirements.
