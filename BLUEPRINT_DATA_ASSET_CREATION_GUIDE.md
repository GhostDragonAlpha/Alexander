# Blueprint Data Asset Creation Guide
## Ship Customization Data Assets in Unreal Editor

**Version:** 1.0
**Last Updated:** November 2025
**Audience:** Level Designers, Game Designers, Content Creators

---

## Table of Contents

1. [Quick Start Overview](#quick-start-overview)
2. [Section 1: Creating Ship Parts Data Tables (30 Steps)](#section-1-creating-ship-parts-data-tables-30-steps)
3. [Section 2: Creating Ship Skins Data Tables (20 Steps)](#section-2-creating-ship-skins-data-tables-20-steps)
4. [Section 3: Creating Ship Physics Configs (25 Steps)](#section-3-creating-ship-physics-configs-25-steps)
5. [Section 4: Setting Up Ship Blueprint (20 Steps)](#section-4-setting-up-ship-blueprint-20-steps)
6. [Section 5: Testing in PIE (15 Steps)](#section-5-testing-in-pie-15-steps)
7. [Section 6: Common Issues & Solutions](#section-6-common-issues--solutions)
8. [Section 7: Content Organization Best Practices](#section-7-content-organization-best-practices)
9. [Section 8: Part Balancing Guidelines](#section-8-part-balancing-guidelines)
10. [Section 9: Example Preset Parts](#section-9-example-preset-parts)
11. [Section 10: Video Tutorial Script](#section-10-video-tutorial-script)

---

## Quick Start Overview

Ship customization in Alexander requires creating data assets that define:
- **Ship Parts** (engines, thrusters, hulls, weapons, shields, etc.)
- **Ship Skins** (visual customization and materials)
- **Ship Physics Configs** (preset flight characteristics)

These assets are managed by the `ShipCustomizationComponent`, which:
- Loads data from data tables
- Applies physics configurations to the flight controller
- Manages player progression (levels, credits, unlocked parts)
- Handles part equipping/unequipping and stat calculations
- Persists customization data via SaveGame

---

## Section 1: Creating Ship Parts Data Tables (30 Steps)

### Understanding FShipPartData Structure

Before creating data tables, understand what each field represents:

| Field | Type | Purpose |
|-------|------|---------|
| **PartID** | FName | Unique identifier (e.g., "Engine_Basic") |
| **DisplayName** | FText | Player-facing name (e.g., "Basic Engine") |
| **Description** | FText | Full description of the part |
| **Category** | EShipPartCategory | Engine, Thrusters, Hull, Wings, Cockpit, Weapon, Shield, Utility |
| **Rarity** | EShipPartRarity | Common, Uncommon, Rare, Epic, Legendary |
| **LevelRequirement** | int32 | Player level needed to equip |
| **UnlockCost** | int32 | Credits needed to unlock (0 = free) |
| **bUnlockedByDefault** | bool | Available to player from start |
| **StatModifiers** | FShipStats | Performance changes (see FShipStats structure) |
| **PartMesh** | TSoftObjectPtr<UStaticMesh> | 3D model of the part (optional) |
| **Icon** | TSoftObjectPtr<UTexture2D> | UI icon (128x128 recommended) |
| **ParentPartID** | FName | Part this upgrades from (for upgrade chains) |
| **bCanBeUpgraded** | bool | Whether this part can be upgraded |
| **UpgradeLevel** | int32 | 0 = base, 1-5 = upgrade levels |

### FShipStats Structure

Each part modifies ship performance through stat multipliers:

| Stat | Default | Purpose |
|------|---------|---------|
| **ThrustPower** | 1.0 | Engine acceleration multiplier |
| **MaxVelocity** | 1.0 | Maximum speed multiplier |
| **RotationSpeed** | 1.0 | Turn rate multiplier |
| **Acceleration** | 1.0 | Acceleration rate multiplier |
| **HullIntegrity** | 100.0 | Ship health/durability |
| **ShieldStrength** | 0.0 | Shield capacity (if equipped) |
| **WeaponDamage** | 1.0 | Weapon output multiplier |
| **EnergyCapacity** | 100.0 | Energy reserves |
| **EnergyRegenRate** | 10.0 | Energy regeneration per second |
| **Mass** | 1000.0 | Ship weight (affects handling) |

### Step-by-Step: Creating Engine Parts Data Table

#### Steps 1-5: Create and Name Data Table

**Step 1:** Launch Unreal Editor and navigate to your project

**Step 2:** In the Content Browser (bottom left panel), navigate to: `Content/Data/ShipParts/`
- If folders don't exist, right-click in Content root → New Folder → "Data"
- Then create "ShipParts" inside Data folder

**Step 3:** Right-click in the Content Browser → Miscellaneous → Data Table
- Screenshot would show: Content Browser context menu with "Miscellaneous" expanded, "Data Table" option highlighted

**Step 4:** A popup appears asking to select row structure
- Find and select: **FShipPartData**
- Screenshot would show: Popup dialog with search box, FShipPartData selected with blue highlight

**Step 5:** Name the new data table: `DT_ShipParts_Engines`
- Click outside to confirm name
- Screenshot would show: Content Browser with renamed asset "DT_ShipParts_Engines"

#### Steps 6-15: Add Basic Engine Part

**Step 6:** Double-click `DT_ShipParts_Engines` to open it
- Screenshot would show: Data Table editor window opening, empty grid view

**Step 7:** Click "Add Row" button (top toolbar)
- Screenshot would show: Empty data table with "Add Row" button highlighted

**Step 8:** Name the row: `Engine_Basic`
- Screenshot would show: New row added with name "Engine_Basic" selected in left panel

**Step 9:** Fill in Part Information:
- **PartID:** Engine_Basic
- **DisplayName:** "Basic Engine"
- **Description:** "Standard propulsion system for light spacecraft"

**Step 10:** Set Part Category and Rarity:
- **Category:** Engine (dropdown)
- **Rarity:** Common (dropdown)

**Step 11:** Set Progression Parameters:
- **LevelRequirement:** 1
- **UnlockCost:** 0
- **bUnlockedByDefault:** TRUE (checkbox)

**Step 12:** Set Stat Modifiers to baseline (1.0 multipliers):
- All stat multipliers should be 1.0 or close to it for basic engine
- **ThrustPower:** 1.0
- **MaxVelocity:** 1.0
- **RotationSpeed:** 1.0
- **Acceleration:** 1.0
- **HullIntegrity:** 100.0
- **ShieldStrength:** 0.0
- **WeaponDamage:** 1.0
- **EnergyCapacity:** 100.0
- **EnergyRegenRate:** 10.0
- **Mass:** 1000.0

**Step 13:** Set Visual and Progression:
- **PartMesh:** Leave blank or assign Engine_Basic mesh if available
- **Icon:** Assign a 128x128 texture for the engine icon
- **ParentPartID:** Leave blank (no parent)

**Step 14:** Set Upgrade Properties:
- **bCanBeUpgraded:** TRUE
- **UpgradeLevel:** 0 (this is the base version)

**Step 15:** Press Ctrl+S to save the data table

#### Steps 16-20: Add Advanced Engine Part

**Step 16:** Click "Add Row" button again

**Step 17:** Name the row: `Engine_Advanced`

**Step 18:** Copy settings from Engine_Basic, but modify:
- **PartID:** Engine_Advanced
- **DisplayName:** "Advanced Engine"
- **Description:** "High-performance engine with improved thrust"
- **LevelRequirement:** 5
- **UnlockCost:** 500
- **bUnlockedByDefault:** FALSE

**Step 19:** Modify Stats for improved performance:
- **ThrustPower:** 1.5 (50% more thrust)
- **MaxVelocity:** 1.2
- **RotationSpeed:** 1.0
- **Acceleration:** 1.3
- **HullIntegrity:** 100.0
- **Mass:** 1100.0 (slightly heavier due to more powerful components)

**Step 20:** Set:
- **ParentPartID:** Engine_Basic (upgrades from basic engine)
- **UpgradeLevel:** 1
- Save table

#### Steps 21-30: Add Racing Engine and Complete Table

**Step 21:** Click "Add Row" for racing engine

**Step 22:** Name: `Engine_Racing`
- **DisplayName:** "Racing Engine"
- **Description:** "Lightweight engine optimized for speed and agility"
- **Rarity:** Rare
- **LevelRequirement:** 10
- **UnlockCost:** 1500
- **bUnlockedByDefault:** FALSE

**Step 23:** Set Racing Engine stats:
- **ThrustPower:** 1.8 (high acceleration)
- **MaxVelocity:** 1.6 (maximum speed)
- **RotationSpeed:** 1.4 (very agile)
- **Acceleration:** 1.6
- **HullIntegrity:** 80.0 (less durability, lighter weight)
- **Mass:** 800.0 (much lighter)
- **ParentPartID:** Engine_Advanced
- **UpgradeLevel:** 2

**Step 24-25:** Add two more engine variants (Heavy Freighter Engine, Military Engine)

**Step 26-27:** Add Heavy Freighter Engine:
- **PartID:** Engine_Freighter
- **DisplayName:** "Freighter Engine"
- **Category:** Engine
- **Rarity:** Common
- **LevelRequirement:** 3
- **UnlockCost:** 300
- **ThrustPower:** 0.8 (lower thrust, higher cargo capacity)
- **Acceleration:** 0.7
- **Mass:** 2000.0 (very heavy)
- **HullIntegrity:** 200.0 (very strong)

**Step 28:** Add Military Engine:
- **PartID:** Engine_Military
- **DisplayName:** "Military Engine"
- **Category:** Engine
- **Rarity:** Epic
- **LevelRequirement:** 15
- **UnlockCost:** 3000
- **ThrustPower:** 2.0
- **MaxVelocity:** 1.4
- **RotationSpeed:** 1.2
- **Acceleration:** 1.8
- **Mass:** 1200.0

**Step 29:** Add one more: Stable Engine
- **PartID:** Engine_Stable
- **DisplayName:** "Stable Engine"
- **Category:** Engine
- **Rarity:** Uncommon
- **LevelRequirement:** 2
- **UnlockCost:** 100
- **ThrustPower:** 1.1
- **MaxVelocity:** 1.0
- **Acceleration:** 1.0
- **Mass:** 950.0

**Step 30:** Press Ctrl+S to save the complete Engines data table

### Repeat for Other Categories

Follow the same process for:

#### DT_ShipParts_Thrusters
Create 5-8 thruster variants:
- **Thruster_Basic** - Standard rotation (1.0x)
- **Thruster_Agile** - High rotation speed (1.5x)
- **Thruster_Heavy** - Strong but slower (0.8x rotation, heavy)
- **Thruster_Responsive** - Quick reaction (1.3x)
- **Thruster_Balanced** - Steady performance (1.1x)
- **Thruster_Military** - Combat optimized (1.4x rotation, high energy drain)

#### DT_ShipParts_Hulls
Create 5-8 hull variants focused on durability and mass:
- **Hull_Light** - Minimal durability (50 HP), minimal mass (500 kg)
- **Hull_Standard** - Default (100 HP), baseline mass
- **Hull_Reinforced** - High durability (150 HP), heavy (1500 kg)
- **Hull_Combat** - Very high HP (200 HP), heavy armor (2000 kg)
- **Hull_Racing** - Minimal protective (40 HP), very light (400 kg)

Key concept: Hulls primarily affect HullIntegrity and Mass stats.

#### DT_ShipParts_Wings
Create 5-6 wing variants:
- **Wings_Standard** - Baseline
- **Wings_Aerodynamic** - Better max velocity (1.3x)
- **Wings_Heavy** - More lift (1.2x max velocity, heavier)
- **Wings_Racing** - Minimal weight (1.5x velocity, light)
- **Wings_Combat** - Stabilization assistance (1.0x velocity, good handling)

#### DT_ShipParts_Weapons
Create 5-8 weapon variants:
- **Weapon_Pulse** - Basic weapon (1.0x damage)
- **Weapon_Laser** - Mid-range (1.3x damage, high energy)
- **Weapon_Missile** - Heavy (1.8x damage, slow fire rate)
- **Weapon_Railgun** - High velocity (2.0x damage, requires max velocity boost)
- **Weapon_Plasma** - Balanced (1.5x damage)

#### DT_ShipParts_Shields
Create 4-5 shield variants:
- **Shield_Basic** - Standard shields (50 strength)
- **Shield_Advanced** - Stronger (100 strength, more energy drain)
- **Shield_Reactive** - Adaptive (80 strength, regenerates)
- **Shield_Military** - Combat (150 strength, high energy)

#### DT_ShipParts_Utility
Create 5-6 utility variants:
- **Utility_Scanner** - Detection system (small CPU bonus, minimal weight)
- **Utility_Cooler** - Better thermal management (improves energy regen)
- **Utility_Stabilizer** - Flight assist (improves rotation speed)
- **Utility_Booster** - Extra power (temporary thrust boost, limited)

#### DT_ShipParts_Cockpit
Create 3-4 cockpit variants:
- **Cockpit_Standard** - Basic controls
- **Cockpit_Advanced** - Better feedback and control (improves rotation)
- **Cockpit_Combat** - Optimized for combat (improves all flight stats)

---

## Section 2: Creating Ship Skins Data Tables (20 Steps)

### Understanding FShipSkinData Structure

| Field | Type | Purpose |
|-------|------|---------|
| **SkinID** | FName | Unique identifier (e.g., "Skin_Default") |
| **DisplayName** | FText | Player-facing name |
| **Description** | FText | Visual style description |
| **Rarity** | EShipPartRarity | Affects visual distinctiveness |
| **SkinMaterial** | TSoftObjectPtr<UMaterialInstance> | Material applied to ship mesh |
| **Icon** | TSoftObjectPtr<UTexture2D> | UI preview icon |
| **UnlockCost** | int32 | Credits to unlock |
| **bUnlockedByDefault** | bool | Available from start |

### Step-by-Step: Creating Ship Skins Data Table

**Steps 1-3: Create Data Table**

**Step 1:** Navigate to Content/Data/ShipSkins/
- Create folder if it doesn't exist

**Step 2:** Right-click → Miscellaneous → Data Table → Select **FShipSkinData**

**Step 3:** Name it: `DT_ShipSkins_Default`

**Steps 4-8: Add Default Skin**

**Step 4:** Double-click to open the data table

**Step 5:** Click "Add Row" → Name: `Skin_Default`

**Step 6:** Fill in basic info:
- **SkinID:** Skin_Default
- **DisplayName:** "Standard White"
- **Description:** "Classic white and blue colorway"
- **Rarity:** Common
- **bUnlockedByDefault:** TRUE

**Step 7:** Set visual assets:
- **SkinMaterial:** Assign M_ShipSkin_White or default material
- **Icon:** Assign 128x128 icon showing white skin preview

**Step 8:** Leave UnlockCost at 0, save

**Steps 9-12: Add Racing Skin**

**Step 9:** Click "Add Row" → Name: `Skin_Racing`

**Step 10:** Fill in:
- **DisplayName:** "Racing Stripe"
- **Description:** "Sleek black with neon racing stripes"
- **Rarity:** Uncommon
- **UnlockCost:** 200
- **bUnlockedByDefault:** FALSE

**Step 11:** Assign racing materials and icon

**Step 12:** Save

**Steps 13-16: Add Combat Skin**

**Step 13:** Click "Add Row" → Name: `Skin_Combat`

**Step 14:** Fill in:
- **DisplayName:** "Combat Black"
- **Description:** "Military-grade black coating with tactical markings"
- **Rarity:** Rare
- **UnlockCost:** 500
- **bUnlockedByDefault:** FALSE

**Step 15:** Assign combat materials

**Step 16:** Save

**Steps 17-20: Add Premium Skins**

**Step 17:** Add `Skin_Chrome`:
- **DisplayName:** "Chrome Luxury"
- **Rarity:** Epic
- **UnlockCost:** 1500

**Step 18:** Add `Skin_Neon`:
- **DisplayName:** "Neon Cyberpunk"
- **Rarity:** Legendary
- **UnlockCost:** 3000

**Step 19:** Add `Skin_Forest`:
- **DisplayName:** "Forest Camo"
- **Rarity:** Rare
- **UnlockCost:** 400

**Step 20:** Save final table

---

## Section 3: Creating Ship Physics Configs (25 Steps)

### Understanding UShipPhysicsConfig

This data asset class defines preset physics parameters that can be applied to the ship.

**Key Fields:**
- **ConfigName:** Display name of configuration
- **Description:** What this config is designed for
- **Mass:** Ship weight (affects inertia)
- **ThrustPower:** Base thrust in Newtons
- **MaxVelocity:** Speed limit
- **AngularThrustPower:** Rotation capability
- **AccelerationMultiplier:** How quickly thrust builds
- **RotationSpeedMultiplier:** Turn rate
- **StabilityAssistStrength:** Auto-stabilization (0-1)

### Step-by-Step: Creating Physics Configs

**Steps 1-5: Create Light Fighter Config**

**Step 1:** Navigate to Content/Data/PhysicsConfigs/

**Step 2:** Right-click → Miscellaneous → Data Asset → Select **UShipPhysicsConfig**

**Step 3:** Name it: `DA_LightFighter`
- Screenshot would show: Asset created in Content Browser

**Step 4:** Double-click to open Properties panel

**Step 5:** Fill in Configuration info:
- **ConfigName:** "Light Fighter"
- **Description:** "Fast and agile combat vessel optimized for dogfighting"

**Steps 6-10: Set Physics for Light Fighter**

**Step 6:** Set basic physics:
- **Mass:** 500 kg (very light)
- **ThrustPower:** 150000 N (very high relative to mass)
- **MaxVelocity:** 1500 m/s (very fast)

**Step 7:** Set angular properties:
- **AngularThrustPower:** 80000 N (highly maneuverable)
- **FuelCapacity:** 1000 L
- **FuelConsumptionRate:** 2.5 (consumes fuel quickly)

**Step 8:** Set flight characteristics:
- **AccelerationMultiplier:** 1.3 (quick response)
- **RotationSpeedMultiplier:** 1.5 (very agile)

**Step 9:** Set stability:
- **StabilityAssistStrength:** 0.3 (minimal assist for responsive controls)

**Step 10:** Click "Save" (top right)

**Steps 11-15: Create Heavy Freighter Config**

**Step 11:** Right-click → Data Asset → UShipPhysicsConfig
- Name: `DA_HeavyFreighter`

**Step 12:** Fill in:
- **ConfigName:** "Heavy Freighter"
- **Description:** "Cargo vessel designed for hauling large quantities of material"

**Step 13:** Set physics (opposite of fighter):
- **Mass:** 5000 kg (very heavy)
- **ThrustPower:** 100000 N (lower thrust)
- **MaxVelocity:** 800 m/s (slower top speed)

**Step 14:** Set for cargo hauling:
- **AngularThrustPower:** 40000 N (slow turning)
- **FuelCapacity:** 5000 L (large tank)
- **FuelConsumptionRate:** 5.0 (high consumption)

**Step 15:** Set flight:
- **AccelerationMultiplier:** 0.7 (sluggish)
- **RotationSpeedMultiplier:** 0.6
- **StabilityAssistStrength:** 0.9 (high assist for stability)

**Steps 16-20: Create Balanced Scout Config**

**Step 16:** Create `DA_BalancedScout`
- **ConfigName:** "Balanced Scout"
- **Description:** "Multipurpose exploration vessel with good balance of speed and cargo"

**Step 17:** Set balanced physics:
- **Mass:** 1200 kg
- **ThrustPower:** 120000 N
- **MaxVelocity:** 1200 m/s

**Step 18:** Set balanced characteristics:
- **AngularThrustPower:** 60000 N
- **FuelCapacity:** 2500 L
- **FuelConsumptionRate:** 3.0

**Step 19:** Set flight:
- **AccelerationMultiplier:** 1.0
- **RotationSpeedMultiplier:** 1.0
- **StabilityAssistStrength:** 0.5

**Step 20:** Save

**Steps 21-25: Create Racing Ship Config**

**Step 21:** Create `DA_RacingShip`
- **ConfigName:** "Racing Ship"
- **Description:** "Purpose-built racer designed for speed and agility"

**Step 22:** Set racing physics:
- **Mass:** 400 kg (extremely light)
- **ThrustPower:** 180000 N (extremely high)
- **MaxVelocity:** 2000 m/s (fastest possible)

**Step 23:** Set racing characteristics:
- **AngularThrustPower:** 100000 N (maximum maneuverability)
- **FuelCapacity:** 500 L (minimal cargo)
- **FuelConsumptionRate:** 4.0 (high consumption at high speeds)

**Step 24:** Set racing flight:
- **AccelerationMultiplier:** 1.5 (instantaneous response)
- **RotationSpeedMultiplier:** 1.8 (extremely agile)
- **StabilityAssistStrength:** 0.1 (minimal assist for precise control)

**Step 25:** Save and close all configs

---

## Section 4: Setting Up Ship Blueprint (20 Steps)

### Understanding Component Setup

The `ShipCustomizationComponent` must be added to your ship blueprint and configured with references to your data tables and physics configs.

### Step-by-Step: Configure Ship Blueprint

**Steps 1-5: Open Ship Blueprint**

**Step 1:** Navigate to Content/Blueprints/Ships/

**Step 2:** Find and open `BP_VRSpaceshipPlayer` (or your main ship blueprint)
- Screenshot would show: Blueprint editor opening with ship actor selected

**Step 3:** In the Details panel (right side), find "Components" section

**Step 4:** Verify `ShipCustomizationComponent` exists in the component list
- If not, click "Add Component" → Search "ShipCustomizationComponent" → Click to add

**Step 5:** Click on `ShipCustomizationComponent` to select it
- Screenshot would show: Component highlighted in component tree

**Steps 6-10: Assign Ship Parts Data Table**

**Step 6:** With ShipCustomizationComponent selected, find Details panel

**Step 7:** Scroll down to find "Data" section

**Step 8:** Click on "Ship Parts Data Table" field
- Screenshot would show: Details panel with "Data" category expanded

**Step 9:** Click the dropdown arrow to select asset
- Popup shows available data tables

**Step 10:** Select `DT_ShipParts_Engines`
- Or select all engine parts from the data table reference
- Note: You can have multiple data tables for different part categories, or combine all parts in one table

**Steps 11-15: Assign Ship Skins Data Table**

**Step 11:** Find "Ship Skins Data Table" field in Details

**Step 12:** Click the dropdown arrow

**Step 13:** Select `DT_ShipSkins_Default`
- Screenshot would show: Data table selector dropdown

**Step 14:** Verify the assignment appears in the Details panel

**Step 15:** Repeat for all other part category tables if using separate tables

**Steps 16-20: Configure Physics Config**

**Step 16:** Find "Configuration" section in Details panel

**Step 17:** Look for "Physics Config" field

**Step 18:** Click the dropdown arrow
- Screenshot would show: Configuration section with Physics Config field

**Step 19:** Select `DA_LightFighter` as the default configuration
- This can be changed at runtime or by player preference

**Step 20:** Click "Compile" (top toolbar) then "Save"
- Screenshot would show: Compile and Save buttons in toolbar, no errors in Message Log

### Optional: Configure Additional Settings

**Enable Replication:**
- If using multiplayer, ensure "Replication" is enabled on the component
- Set "Initially Replicated" to TRUE

**Configure Events:**
- You can bind to the component's events in the Blueprint Details:
  - `OnPartEquipped`
  - `OnSkinEquipped`
  - `OnPartUnlocked`
  - `OnPlayerLevelUp`
  - `OnLoadoutChanged`

**Add Visual Blueprint Logic:**
- In the Blueprint Event Graph, bind to `OnLoadoutChanged` event to update HUD displays
- Bind to `OnPartEquipped` to play equip animations or sounds

---

## Section 5: Testing in PIE (15 Steps)

### Setting Up Test Level

**Steps 1-3: Prepare Test Level**

**Step 1:** Open or create a test level (or use your main level)
- Screenshot would show: Level editor with a space scene and spaceship

**Step 2:** Verify the ship actor has the ShipCustomizationComponent with data tables assigned

**Step 3:** Save the level (Ctrl+S)

### Testing Data Table Loading

**Steps 4-6: Test in Play in Editor (PIE)**

**Step 4:** Click "Play" button (or Alt+P) to start PIE
- Screenshot would show: Game running in editor viewport, spaceship visible

**Step 5:** Open the Output Log (Window → Developer Tools → Output Log)
- Screenshot would show: Output log panel appearing at bottom

**Step 6:** Look for initialization messages:
- Should see: `ShipCustomizationComponent initialized - Level 1, X parts equipped`
- If data tables fail to load, you'll see warning messages here

### Testing Stats Application

**Steps 7-9: Verify Stats Are Applied**

**Step 7:** Open the Console (press ~ tilde key)
- Screenshot would show: Console input box appearing at top of screen

**Step 8:** Type: `ShowDebug ShipCustomization`
- Console should display current ship stats on screen overlay

**Step 9:** Verify displayed stats match your configured values:
- Thrust Power value
- Max Velocity value
- Current Mass
- Hull Integrity
- Other configured stats

### Testing Part Equipping (Blueprint)

**Steps 10-12: Test Part Equipping**

**Step 10:** Open the Blueprint implementation of your ship (right-click actor → Edit Blueprint)

**Step 11:** In the Event Graph, add a test node to equip a part:
- Add Event: `Event Begin Play`
- Add Node: `Equip Part` (from ShipCustomizationComponent)
- Set Category: Engine
- Set PartID: Engine_Advanced

**Step 12:** Recompile and Play again
- Open Output Log to verify part equipped successfully
- Stats should update to reflect the new part

### Testing Data Persistence

**Steps 13-15: Test Save/Load**

**Step 13:** In game, use Blueprint nodes to add credits and XP:
- Call `Add Credits` with value 500
- Call `Add XP` with value 1000

**Step 14:** Check Output Log:
- Should see player level increases if XP threshold crossed
- Credits should update

**Step 15:** Stop PIE and restart:
- With proper SaveGame implementation, data should persist
- Open Output Log to verify data was loaded from save

---

## Section 6: Common Issues & Solutions

### Issue 1: Data Table Won't Open

**Symptom:** Double-clicking data table does nothing or crashes editor

**Solutions:**
1. Verify row structure is correctly set to `FShipPartData` or `FShipSkinData`
2. Delete and recreate the data table, ensuring correct struct selected
3. Check editor logs for struct-related errors
4. Verify struct is properly exposed to the data table system (USTRUCT(BlueprintType))

### Issue 2: Data Table Shows Empty Rows

**Symptom:** Data table opens but no columns visible, or data appears blank

**Solutions:**
1. Verify the row structure has USTRUCT(BlueprintType) and UPROPERTY macros
2. Regenerate Visual Studio project files: File → Generate Visual Studio Project Files
3. Recompile C++ code and refresh editor
4. Create a new data table and manually re-enter data

### Issue 3: Parts Don't Show Up in Game

**Symptom:** Data table created and populated, but parts aren't available in game

**Solutions:**
1. Verify `ShipPartsDataTable` is assigned in component Details
2. Check that at least one part has `bUnlockedByDefault = TRUE`
3. Open Output Log and look for data table loading errors
4. Verify player level meets part's `LevelRequirement`
5. Check that PartID matches exactly (case-sensitive)

### Issue 4: Stats Don't Apply to Flight

**Symptom:** Parts equip but flight controller stats don't change

**Solutions:**
1. Verify `ApplyStatsToFlightController()` is being called
   - Should be called automatically in BeginPlay
   - Can manually call from Blueprint if needed
2. Check that FlightController component exists on the same actor
3. Verify stat values are set correctly (not all zeros)
4. Check Output Log for "Cannot find FlightController" warnings
5. Ensure mass values are reasonable (not 0 or negative)

### Issue 5: Network Replication Issues

**Symptom:** Multiplayer - part changes don't sync to other players

**Solutions:**
1. Verify `ShipCustomizationComponent` has `bReplicates = true`
2. Check that `CurrentLoadout` is replicated via `GetLifetimeReplicatedProps()`
3. Ensure `EquipPart()` is marked as `Replicated` or called on server
4. Verify data tables are replicated from server (may need custom replication)
5. Check server logs for replication errors

### Issue 6: Icon/Material References Not Loading

**Symptom:** Parts have icons but they don't display in UI

**Solutions:**
1. Verify soft object pointers are correctly assigned: `TSoftObjectPtr<UTexture2D>`
2. Check that referenced assets still exist in project
3. Use hard references temporarily for testing: `UTexture2D*` instead of `TSoftObjectPtr`
4. Verify asset paths are correct (should auto-populate when dragging into field)
5. Check that UI widget is correctly loading icons from part data

### Issue 7: Data Table Corruption

**Symptom:** Data table was working but now shows errors or missing rows

**Solutions:**
1. Make a backup before attempting fixes
2. Export data table to CSV: Right-click → Export to CSV
3. Create new data table and re-import CSV data
4. Check asset integrity: Right-click → Asset Actions → Fix Up Redirects
5. Revert from version control if available

### Issue 8: Performance Issues With Large Data Tables

**Symptom:** Editor/game lags when working with large data tables (100+ rows)

**Solutions:**
1. Split into multiple data tables by category instead of one monolithic table
2. Use soft object pointers for meshes/textures (already done with TSoftObjectPtr)
3. Lazy load data only when needed
4. Consider database approach if exceeding 1000 parts
5. Profile with Unreal Insights to identify bottleneck

### Issue 9: Player Progression Not Saving

**Symptom:** XP, Credits, or unlocked parts reset when closing/reopening game

**Solutions:**
1. Verify `SaveCustomizationData()` is being called
2. Check that SaveGame class is properly created and stored
3. Verify save game slot name is consistent
4. Check hard drive storage permissions
5. Look for SaveGame file in: `Saved/SaveGames/` directory

### Issue 10: PartID Case Sensitivity Issues

**Symptom:** Can't find parts even though they exist in table

**Solutions:**
1. Remember that FName comparisons are case-insensitive
2. Double-check spelling of PartID exactly matches row name
3. Avoid special characters in PartID
4. Use consistent naming convention (e.g., all lowercase with underscores)
5. Log the actual PartID being searched: `UE_LOG(LogTemp, Warning, TEXT("Looking for: %s"), *PartID.ToString())`

---

## Section 7: Content Organization Best Practices

### Recommended Folder Structure

```
Content/
├── Data/
│   ├── ShipParts/
│   │   ├── DT_ShipParts_Engines.uasset
│   │   ├── DT_ShipParts_Thrusters.uasset
│   │   ├── DT_ShipParts_Hulls.uasset
│   │   ├── DT_ShipParts_Wings.uasset
│   │   ├── DT_ShipParts_Cockpits.uasset
│   │   ├── DT_ShipParts_Weapons.uasset
│   │   ├── DT_ShipParts_Shields.uasset
│   │   └── DT_ShipParts_Utility.uasset
│   ├── ShipSkins/
│   │   ├── DT_ShipSkins_Default.uasset
│   │   ├── DT_ShipSkins_Premium.uasset
│   │   └── DT_ShipSkins_Limited.uasset
│   └── PhysicsConfigs/
│       ├── DA_LightFighter.uasset
│       ├── DA_HeavyFreighter.uasset
│       ├── DA_BalancedScout.uasset
│       ├── DA_RacingShip.uasset
│       └── DA_Starter.uasset
├── Materials/
│   └── ShipSkins/
│       ├── M_ShipSkin_White.uasset
│       ├── M_ShipSkin_Racing.uasset
│       └── M_ShipSkin_Combat.uasset
├── Meshes/
│   └── ShipParts/
│       ├── Engine_Basic.uasset
│       ├── Engine_Advanced.uasset
│       ├── Thruster_Standard.uasset
│       └── ... (other part meshes)
└── Blueprints/
    └── Ships/
        ├── BP_VRSpaceshipPlayer.uasset
        └── BP_ShipCustomizationUI.uasset
```

### Naming Conventions

**Data Tables:**
- Prefix with `DT_` to identify as Data Table
- Format: `DT_ShipParts_[Category]`
- Example: `DT_ShipParts_Engines`, `DT_ShipParts_Weapons`

**Data Assets:**
- Prefix with `DA_` to identify as Data Asset
- Format: `DA_[ConfigType][Variant]`
- Example: `DA_LightFighter`, `DA_HeavyFreighter`

**Part IDs (Row Names):**
- Format: `[PartType]_[Variant]`
- Use CamelCase or snake_case consistently
- Examples: `Engine_Basic`, `Thruster_Agile`, `Hull_Reinforced`

**Skin IDs (Row Names):**
- Format: `Skin_[Description]`
- Examples: `Skin_Default`, `Skin_Racing`, `Skin_Chrome`

### Documentation Best Practices

**Create a Design Document for Your Parts:**

```
ShipParts_DesignDoc.txt

ENGINES:
- Engine_Basic: Multiplier 1.0, Cost 0, Level 1
- Engine_Advanced: Multiplier 1.5, Cost 500, Level 5
- Engine_Racing: Multiplier 1.8, Cost 1500, Level 10
...

THRUSTERS:
- Thruster_Basic: Rotation 1.0, Cost 0, Level 1
...

HULLS:
...
```

**Add Comments in Data:**
- Use Description field for gameplay balance notes
- Example: "Aggressive thrust - requires pilot skill" or "Beginner-friendly part"

**Track Progression Gates:**
- Document which parts unlock at which levels
- Identify progression curves to ensure balanced advancement
- Verify no early parts are overpowered

### Version Control Recommendations

**Treat data tables like source code:**
1. Commit all DT_* and DA_* assets to version control
2. Create branches for major balance changes
3. Use descriptive commit messages: "Add advanced engine variants for level 5+"
4. Tag versions: "v1.0_launch_content", "v1.1_balance_patch"

**Backup strategy:**
1. Export important data tables to CSV monthly
2. Keep offline backup of all ship data definitions
3. Document all balance changes in changelog

---

## Section 8: Part Balancing Guidelines

### Balance Philosophy

Ship customization should provide meaningful choices without breaking game balance:
- **No single best build** - every loadout should have pros/cons
- **Trade-offs required** - faster ships are lighter and more fragile
- **Skill-based gameplay** - powerful parts require player skill to use effectively
- **Progression feel** - each new part should feel like an upgrade, not just a sidegrade

### Stat Modifier Guidelines

#### Thrust Power Multipliers
| Multiplier | Use Case | Example |
|------------|----------|---------|
| 0.5 - 0.7 | Cargo/Freighter | Heavy/slow engine |
| 0.8 - 0.9 | Stability focused | Balanced but sluggish |
| 1.0 | Baseline | Starting engine |
| 1.1 - 1.3 | Good upgrade | Advanced engine |
| 1.4 - 1.6 | Powerful | Rare/Epic engine |
| 1.7 - 2.0 | Extreme | Legendary/Racing |

**Balancing Tip:** Higher thrust multipliers should require:
- Higher player level (5+)
- Higher unlock cost (500+ credits)
- Higher rarity (Rare or higher)
- Trade-off with mass or energy (not pure upgrade)

#### Max Velocity Multipliers
Similar to thrust power:
| Multiplier | Context |
|------------|---------|
| 0.8 | Freighter, heavy |
| 1.0 | Baseline |
| 1.2 - 1.4 | Scout/fast |
| 1.5 - 1.8 | Racing |

**Balancing Tip:**
- Racing engines should have lower hull integrity (fragile)
- High velocity parts should have higher fuel consumption
- Very fast ships should be harder to control (lower rotation speed)

#### Mass Additions/Subtractions
| Delta | Type | Example |
|-------|------|---------|
| -500 to -200 | Lightweight | Racing engine, minimal armor |
| -100 to 0 | Standard | Normal parts |
| +100 to +500 | Heavy | Reinforced armor, military grade |
| +500 to +2000 | Very Heavy | Massive cargo capacity, shields |

**Balancing Tip:**
- Every 100 kg added should reduce acceleration by ~1-2%
- Light parts should have lower durability (fragile)
- Heavy parts should provide defensive benefits

#### Hull Integrity Values
| Value | Use Case |
|-------|----------|
| 50 | Racing ship (expendable) |
| 100 | Standard |
| 150 - 200 | Combat equipped |
| 250+ | Heavy armor/freighter |

**Balancing Tip:**
- Hull integrity is additive, not multiplicative
- Tank builds should sacrifice speed
- Glass cannons (high damage, low HP) need skilled players

#### Shield Strength
| Value | Type | Cost |
|-------|------|------|
| 0 | No shield | Free |
| 50 - 100 | Basic | Cheap |
| 100 - 150 | Advanced | Expensive |
| 150+ | Military | Very expensive |

**Balancing Tip:**
- Shields should consume significant energy when active
- Shields should NOT stack - only one can be equipped
- Very strong shields should have high level requirements

#### Weapon Damage Multipliers
| Multiplier | Context | Cost |
|------------|---------|------|
| 1.0 | Baseline | Free |
| 1.3 - 1.5 | Improved | Medium cost |
| 1.6 - 1.8 | Advanced | High cost |
| 1.9 - 2.2 | Extreme | Very high cost |

**Balancing Tip:**
- High damage weapons should have drawbacks:
  - High energy consumption
  - Low fire rate
  - Heavy weight
  - Requires high player level
- Weapon balance is about DPS, not raw damage

### Rarity Pricing Guide

| Rarity | Level Req | Cost | Multiplier Range |
|--------|-----------|------|------------------|
| Common | 1-3 | 0-200 | 0.9 - 1.1 |
| Uncommon | 3-6 | 100-400 | 1.0 - 1.3 |
| Rare | 6-12 | 400-1000 | 1.2 - 1.6 |
| Epic | 12-18 | 1000-3000 | 1.5 - 1.9 |
| Legendary | 18-20 | 3000-5000 | 1.8 - 2.0 |

**Pricing Formula Example:**
```
Base Cost = 100
Cost = Base Cost × Rarity Multiplier × Level Multiplier
  where Rarity Multiplier: Common=1, Uncommon=2, Rare=4, Epic=8, Legendary=15
  where Level Multiplier: 1 + (LevelRequirement / 20)

Example: Rare engine, level 15
Cost = 100 × 4 × (1 + 15/20) = 100 × 4 × 1.75 = 700 credits
```

### Progression Pacing

**Level 1-3 (Starter):**
- Only Common parts
- All unlock costs 0-200
- Multipliers 0.9 - 1.1
- Focus on learning mechanics

**Level 4-8 (Intermediate):**
- Uncommon parts available
- Unlock costs 200-500
- Multipliers 1.0 - 1.4
- Introduce build diversity

**Level 9-15 (Advanced):**
- Rare parts available
- Unlock costs 500-1500
- Multipliers 1.3 - 1.7
- Allow specialized builds

**Level 16-20 (Endgame):**
- Epic and Legendary parts
- Unlock costs 1500-5000
- Multipliers 1.6 - 2.0
- Peak specialization

**Balancing Tip:** Space out new parts across levels - don't unlock everything at once.

### Build Archetypes

**Fighter (High Damage, Medium Speed):**
- Engine: Advanced (1.5x thrust)
- Weapon: Laser or Railgun (1.5x damage)
- Thruster: Responsive (1.3x rotation)
- Hull: Standard (100 HP)
- Total Cost: ~2000 credits, Level 8+

**Scout (High Speed, Light)**
- Engine: Racing (1.8x thrust)
- Thruster: Agile (1.5x rotation)
- Hull: Light (50 HP)
- Weapon: Pulse (1.0x, low weight)
- Total Cost: ~1500 credits, Level 7+

**Tank (High Defense, Slow)**
- Engine: Freighter (0.8x thrust)
- Hull: Combat (200 HP)
- Shield: Military (150 strength)
- Weapon: Missile (1.8x damage)
- Total Cost: ~3000 credits, Level 12+

**Racer (Maximum Speed)**
- Engine: Racing (1.8x thrust)
- Hull: Racing (40 HP)
- Thruster: Racing (1.8x rotation)
- Weapon: None or minimal
- Total Cost: ~1500 credits, Level 10+

### Testing Builds

Before releasing parts, test these scenarios:
1. **Starter only:** Can new players succeed with all Common parts?
2. **Single archetype:** Does one build dominate all others?
3. **Counter builds:** Can high-skill pilots beat high-damage builds?
4. **Progression feel:** Does each level feel like meaningful upgrades?
5. **Economy balance:** Can players afford parts without grinding excessively?

---

## Section 9: Example Preset Parts

### Complete Engine Parts Library

#### Basic Tier
```
Row: Engine_Basic
PartID: Engine_Basic
DisplayName: Basic Engine
Description: Standard propulsion system for light spacecraft. Reliable and affordable.
Category: Engine
Rarity: Common
LevelRequirement: 1
UnlockCost: 0
bUnlockedByDefault: TRUE
StatModifiers:
  ThrustPower: 1.0
  MaxVelocity: 1.0
  RotationSpeed: 1.0
  Acceleration: 1.0
  HullIntegrity: 100.0
  ShieldStrength: 0.0
  WeaponDamage: 1.0
  EnergyCapacity: 100.0
  EnergyRegenRate: 10.0
  Mass: 1000.0
bCanBeUpgraded: TRUE
UpgradeLevel: 0
```

#### Intermediate Tier
```
Row: Engine_Advanced
PartID: Engine_Advanced
DisplayName: Advanced Engine
Description: High-performance engine with improved thrust and reliability. 50% more power than basic.
Category: Engine
Rarity: Uncommon
LevelRequirement: 5
UnlockCost: 500
bUnlockedByDefault: FALSE
StatModifiers:
  ThrustPower: 1.5
  MaxVelocity: 1.2
  RotationSpeed: 1.0
  Acceleration: 1.3
  HullIntegrity: 100.0
  ShieldStrength: 0.0
  WeaponDamage: 1.0
  EnergyCapacity: 100.0
  EnergyRegenRate: 10.0
  Mass: 1100.0
ParentPartID: Engine_Basic
bCanBeUpgraded: TRUE
UpgradeLevel: 1
```

#### Racing Tier
```
Row: Engine_Racing
PartID: Engine_Racing
DisplayName: Racing Engine
Description: Lightweight engine optimized for extreme speed. 80% thrust increase but 20% less durable.
Category: Engine
Rarity: Rare
LevelRequirement: 10
UnlockCost: 1500
bUnlockedByDefault: FALSE
StatModifiers:
  ThrustPower: 1.8
  MaxVelocity: 1.6
  RotationSpeed: 1.4
  Acceleration: 1.6
  HullIntegrity: 80.0
  ShieldStrength: 0.0
  WeaponDamage: 1.0
  EnergyCapacity: 100.0
  EnergyRegenRate: 10.0
  Mass: 800.0
ParentPartID: Engine_Advanced
bCanBeUpgraded: TRUE
UpgradeLevel: 2
```

#### Freighter Tier
```
Row: Engine_Freighter
PartID: Engine_Freighter
DisplayName: Freighter Engine
Description: Heavy-duty engine designed for hauling cargo. Lower speed, much heavier.
Category: Engine
Rarity: Common
LevelRequirement: 3
UnlockCost: 300
bUnlockedByDefault: FALSE
StatModifiers:
  ThrustPower: 0.8
  MaxVelocity: 0.9
  RotationSpeed: 0.8
  Acceleration: 0.7
  HullIntegrity: 200.0
  ShieldStrength: 0.0
  WeaponDamage: 1.0
  EnergyCapacity: 150.0
  EnergyRegenRate: 15.0
  Mass: 2000.0
bCanBeUpgraded: FALSE
UpgradeLevel: 0
```

#### Military Tier
```
Row: Engine_Military
PartID: Engine_Military
DisplayName: Military Engine
Description: Combat-grade engine with high reliability and power delivery.
Category: Engine
Rarity: Epic
LevelRequirement: 15
UnlockCost: 3000
bUnlockedByDefault: FALSE
StatModifiers:
  ThrustPower: 2.0
  MaxVelocity: 1.4
  RotationSpeed: 1.2
  Acceleration: 1.8
  HullIntegrity: 100.0
  ShieldStrength: 0.0
  WeaponDamage: 1.0
  EnergyCapacity: 120.0
  EnergyRegenRate: 20.0
  Mass: 1200.0
bCanBeUpgraded: TRUE
UpgradeLevel: 1
```

### Complete Thruster Parts Library

```
Row: Thruster_Basic
PartID: Thruster_Basic
DisplayName: Basic Thruster
Description: Standard maneuvering thrusters
Category: Thrusters
Rarity: Common
LevelRequirement: 1
UnlockCost: 0
bUnlockedByDefault: TRUE
StatModifiers:
  ThrustPower: 1.0
  RotationSpeed: 1.0
  Mass: 500.0

Row: Thruster_Agile
PartID: Thruster_Agile
DisplayName: Agile Thruster
Description: High-response maneuvering system for combat and racing
Category: Thrusters
Rarity: Uncommon
LevelRequirement: 6
UnlockCost: 600
bUnlockedByDefault: FALSE
StatModifiers:
  ThrustPower: 1.0
  RotationSpeed: 1.5
  Mass: 600.0

Row: Thruster_Heavy
PartID: Thruster_Heavy
DisplayName: Heavy Thruster
Description: Stable maneuvering for large cargo vessels
Category: Thrusters
Rarity: Common
LevelRequirement: 4
UnlockCost: 400
bUnlockedByDefault: FALSE
StatModifiers:
  ThrustPower: 1.0
  RotationSpeed: 0.8
  Mass: 1000.0

Row: Thruster_Military
PartID: Thruster_Military
DisplayName: Military Thruster
Description: Combat-optimized with rapid response
Category: Thrusters
Rarity: Epic
LevelRequirement: 16
UnlockCost: 2500
bUnlockedByDefault: FALSE
StatModifiers:
  ThrustPower: 1.2
  RotationSpeed: 1.4
  Mass: 700.0
```

### Complete Hull Parts Library

```
Row: Hull_Light
PartID: Hull_Light
DisplayName: Light Hull
Description: Racing hull - minimal armor, maximum speed potential
Category: Hull
Rarity: Rare
LevelRequirement: 9
UnlockCost: 800
bUnlockedByDefault: FALSE
StatModifiers:
  HullIntegrity: 40.0
  Mass: 400.0

Row: Hull_Standard
PartID: Hull_Standard
DisplayName: Standard Hull
Description: Balanced protection and weight
Category: Hull
Rarity: Common
LevelRequirement: 1
UnlockCost: 0
bUnlockedByDefault: TRUE
StatModifiers:
  HullIntegrity: 100.0
  Mass: 1000.0

Row: Hull_Reinforced
PartID: Hull_Reinforced
DisplayName: Reinforced Hull
Description: Heavy armor plating for protection
Category: Hull
Rarity: Uncommon
LevelRequirement: 7
UnlockCost: 700
bUnlockedByDefault: FALSE
StatModifiers:
  HullIntegrity: 150.0
  Mass: 1500.0

Row: Hull_Combat
PartID: Hull_Combat
DisplayName: Combat Hull
Description: Military-grade armor designed to withstand intense combat
Category: Hull
Rarity: Epic
LevelRequirement: 14
UnlockCost: 2000
bUnlockedByDefault: FALSE
StatModifiers:
  HullIntegrity: 200.0
  Mass: 2000.0
```

### Complete Weapon Parts Library

```
Row: Weapon_Pulse
PartID: Weapon_Pulse
DisplayName: Pulse Cannon
Description: Standard energy weapon with moderate damage and rate of fire
Category: Weapon
Rarity: Common
LevelRequirement: 1
UnlockCost: 0
bUnlockedByDefault: TRUE
StatModifiers:
  WeaponDamage: 1.0
  EnergyCapacity: 100.0
  Mass: 300.0

Row: Weapon_Laser
PartID: Weapon_Laser
DisplayName: Laser Cannon
Description: Precise mid-range weapon with good damage output
Category: Weapon
Rarity: Uncommon
LevelRequirement: 8
UnlockCost: 1000
bUnlockedByDefault: FALSE
StatModifiers:
  WeaponDamage: 1.3
  EnergyCapacity: 120.0
  Mass: 400.0

Row: Weapon_Missile
PartID: Weapon_Missile
DisplayName: Missile Launcher
Description: Heavy weapon with devastating area damage
Category: Weapon
Rarity: Rare
LevelRequirement: 11
UnlockCost: 1800
bUnlockedByDefault: FALSE
StatModifiers:
  WeaponDamage: 1.8
  EnergyCapacity: 100.0
  Mass: 600.0

Row: Weapon_Plasma
PartID: Weapon_Plasma
DisplayName: Plasma Cannon
Description: Balanced weapon with good all-around performance
Category: Weapon
Rarity: Uncommon
LevelRequirement: 9
UnlockCost: 1200
bUnlockedByDefault: FALSE
StatModifiers:
  WeaponDamage: 1.5
  EnergyCapacity: 110.0
  Mass: 450.0
```

### Complete Shield Parts Library

```
Row: Shield_Basic
PartID: Shield_Basic
DisplayName: Basic Shield
Description: Standard energy shield for light protection
Category: Shield
Rarity: Common
LevelRequirement: 5
UnlockCost: 300
bUnlockedByDefault: FALSE
StatModifiers:
  ShieldStrength: 50.0
  EnergyCapacity: 50.0
  Mass: 200.0

Row: Shield_Advanced
PartID: Shield_Advanced
DisplayName: Advanced Shield
Description: Stronger shields with better energy efficiency
Category: Shield
Rarity: Uncommon
LevelRequirement: 10
UnlockCost: 800
bUnlockedByDefault: FALSE
StatModifiers:
  ShieldStrength: 100.0
  EnergyCapacity: 100.0
  Mass: 300.0

Row: Shield_Military
PartID: Shield_Military
DisplayName: Military Shield
Description: Heavy-duty combat shields for frontline fighters
Category: Shield
Rarity: Epic
LevelRequirement: 17
UnlockCost: 2500
bUnlockedByDefault: FALSE
StatModifiers:
  ShieldStrength: 150.0
  EnergyCapacity: 150.0
  Mass: 500.0
```

### Complete Utility Parts Library

```
Row: Utility_Scanner
PartID: Utility_Scanner
DisplayName: Scanner
Description: Detection system for improved awareness
Category: Utility
Rarity: Common
LevelRequirement: 3
UnlockCost: 200
bUnlockedByDefault: FALSE
StatModifiers:
  EnergyCapacity: 50.0
  Mass: 100.0

Row: Utility_Stabilizer
PartID: Utility_Stabilizer
DisplayName: Flight Stabilizer
Description: Assists flight control and improves handling
Category: Utility
Rarity: Uncommon
LevelRequirement: 6
UnlockCost: 500
bUnlockedByDefault: FALSE
StatModifiers:
  RotationSpeed: 1.1
  Mass: 150.0

Row: Utility_EnergyBooster
PartID: Utility_EnergyBooster
DisplayName: Energy Booster
Description: Increases energy capacity and regeneration rate
Category: Utility
Rarity: Uncommon
LevelRequirement: 8
UnlockCost: 600
bUnlockedByDefault: FALSE
StatModifiers:
  EnergyCapacity: 100.0
  EnergyRegenRate: 15.0
  Mass: 200.0
```

### Complete Skin Library

```
Row: Skin_Default
PartID: Skin_Default
DisplayName: Standard White
Description: Classic white and blue colorway
Category: Skin
Rarity: Common
UnlockCost: 0
bUnlockedByDefault: TRUE

Row: Skin_Racing
PartID: Skin_Racing
DisplayName: Racing Stripe
Description: Sleek black with neon racing stripes
Category: Skin
Rarity: Uncommon
UnlockCost: 200
bUnlockedByDefault: FALSE

Row: Skin_Combat
PartID: Skin_Combat
DisplayName: Combat Black
Description: Military-grade black coating with tactical markings
Category: Skin
Rarity: Rare
UnlockCost: 500
bUnlockedByDefault: FALSE

Row: Skin_Chrome
PartID: Skin_Chrome
DisplayName: Chrome Luxury
Description: Polished chrome with elegant design
Category: Skin
Rarity: Epic
UnlockCost: 1500
bUnlockedByDefault: FALSE

Row: Skin_Neon
PartID: Skin_Neon
DisplayName: Neon Cyberpunk
Description: Futuristic neon colors with glowing accents
Category: Skin
Rarity: Legendary
UnlockCost: 3000
bUnlockedByDefault: FALSE
```

---

## Section 10: Video Tutorial Script

### 10-Minute Tutorial: Creating Ship Parts and Physics Configs

#### Opening (0:00 - 0:15)

**Script:**
"Welcome to the Alexander ship customization guide. In this video, I'll show you how to create ship parts, skins, and physics configurations in Unreal Editor. By the end of this tutorial, you'll have a complete understanding of how to define ship equipment and flight characteristics that power Alexander's customization system."

**Visual:**
- Title card: "Ship Customization Guide"
- Brief clips of ship flying with different parts equipped

#### Part 1: Understanding Data Structures (0:15 - 1:30)

**Script:**
"Before we create anything, let's understand the core data structures. Ship customization uses three main components. First, FShipPartData defines individual components like engines, weapons, and thrusters. Each part has a unique ID, rarity, level requirement, unlock cost, and stat modifiers that affect how the ship flies.

Second, FShipSkinData defines visual appearance and materials. Skins are purely cosmetic but can have rarities and unlock costs.

Third, UShipPhysicsConfig is a preset configuration that defines physics parameters like mass, thrust, and max velocity. This allows you to create different 'ship classes' with different flight characteristics.

The ShipCustomizationComponent brings it all together. It loads data from tables, manages player progression, and applies stats to the flight controller."

**Visuals:**
- Screen showing ShipCustomizationComponent.h
- Highlight FShipPartData struct
- Highlight FShipSkinData struct
- Highlight UShipPhysicsConfig class

#### Part 2: Creating Your First Data Table (1:30 - 3:00)

**Script:**
"Now let's create our first data table. Open the Content Browser and navigate to the Data folder. If it doesn't exist, create it. Right-click in Content Browser and select Miscellaneous > Data Table.

A dialog appears asking you to select the row structure. This is important - we need to select FShipPartData. Search for it, find it in the list, and click to select.

Now name your table. Let's call it DT_ShipParts_Engines. Click outside to confirm the name. Great - you've created your first data table!"

**Visuals:**
- Content Browser navigation
- Right-click menu showing Miscellaneous → Data Table
- Struct selection dialog with FShipPartData highlighted
- New data table created in Content Browser

#### Part 3: Adding Your First Part (3:00 - 5:30)

**Script:**
"Double-click the data table to open it. The editor shows a grid where we add rows. Each row represents one part. Click 'Add Row' to add our first engine.

Name this row Engine_Basic. Now we'll fill in the fields. First, set the PartID to match the row name - Engine_Basic. Then enter the display name: 'Basic Engine'. Add a description like 'Standard propulsion system'.

For Category, select Engine from the dropdown. For Rarity, choose Common - this is a starting part. Set the level requirement to 1 so anyone can use it. UnlockCost should be 0 since it's available by default. Check the bUnlockedByDefault checkbox.

Now for stats. These multipliers define how this part affects the ship. For a basic engine, we want all multipliers at 1.0 - the baseline. Leave them as default values.

Skip the visual assets for now - we can add those later. Leave ParentPartID empty since this is a base part. Set CanBeUpgraded to true and UpgradeLevel to 0.

Finally, press Ctrl+S to save. You've successfully created your first part!"

**Visuals:**
- Data table editor opening
- "Add Row" button being clicked
- Fields being filled in for Engine_Basic
- Stat modifiers section
- Save operation

#### Part 4: Adding More Engine Variants (5:30 - 6:45)

**Script:**
"Let's add a second engine to show how part progression works. Click Add Row again and name it Engine_Advanced. This engine will be better but require the player to level up first.

Set the PartID to Engine_Advanced and DisplayName to 'Advanced Engine'. Set the level requirement to 5 - players can't use this until they reach level 5. Set UnlockCost to 500 credits. Uncheck bUnlockedByDefault.

For stats, this engine should be more powerful. Set ThrustPower to 1.5 - that's 50% more thrust than basic. Set MaxVelocity to 1.2. Set Acceleration to 1.3. Since this engine is more powerful, it's also heavier - set Mass to 1100.

Set ParentPartID to Engine_Basic to show this is an upgrade path. This creates a progression system where players advance from basic to advanced engines."

**Visuals:**
- Adding second row
- Filling in Engine_Advanced stats
- Stats showing 1.5x thrust, 1.2x max velocity
- Save

#### Part 5: Creating Physics Configurations (6:45 - 8:00)

**Script:**
"Now let's create a physics configuration. These define overall ship class characteristics. Go to the PhysicsConfigs folder and right-click in Content Browser.

This time select Miscellaneous > Data Asset. When prompted, choose UShipPhysicsConfig as the class. Name it DA_LightFighter.

Double-click to open. In the Details panel, set the config name to 'Light Fighter' and description to 'Fast and agile combat vessel'.

Now set the physics parameters. For a light fighter, we want high power-to-weight ratio. Set Mass to 500 kg - very light. Set ThrustPower to 150,000 Newtons - very high. Set MaxVelocity to 1500 m/s.

For maneuverability, set AngularThrustPower to 80,000. Set RotationSpeedMultiplier to 1.5 so it turns quickly. Set StabilityAssistStrength to 0.3 - minimal assist so skilled pilots can control it precisely.

Save this config. Repeat for different ship types: heavy freighter, balanced scout, racing ship. Each configuration creates a different flying experience."

**Visuals:**
- Right-click menu showing Data Asset option
- UShipPhysicsConfig selection
- Details panel with physics parameters
- Settings for light fighter
- Save operation

#### Part 6: Setting Up the Ship Blueprint (8:00 - 9:00)

**Script:**
"Now we need to connect these data assets to your ship blueprint. Open your ship blueprint - in this case BP_VRSpaceshipPlayer.

In the Details panel, find the ShipCustomizationComponent. Click to select it. Now scroll down to the Data section. You'll see Ship Parts Data Table and Ship Skins Data Table fields.

Click the dropdown for Ship Parts Data Table and select DT_ShipParts_Engines. Repeat for the other part categories. For Ship Skins Data Table, select DT_ShipSkins_Default.

In the Configuration section, set Physics Config to DA_LightFighter as the default. Now click Compile and Save. Your ship is ready!"

**Visuals:**
- Blueprint editor opening ship
- Component selection
- Details panel with data table fields
- Dropdowns for selecting tables
- Compile and Save buttons
- Success message

#### Part 7: Testing Your Creation (9:00 - 9:45)

**Script:**
"Let's test everything works. Press Play to start the game in the editor. Once loaded, press the tilde key to open the console.

Type ShowDebug ShipCustomization. You'll see your current ship stats displayed on screen - thrust power, max velocity, mass, etc.

If everything loads correctly, you should see your equipped parts in the debug display. If nothing appears, check your data table references in the component.

In the Blueprint's Event Graph, you can add test nodes to equip different parts. Call EquipPart with different PartIDs to see stats change in real-time. This confirms your customization system is working!"

**Visuals:**
- Play button being clicked
- Game running in editor
- Console opening with ~ key
- ShowDebug command
- Debug stats displayed on screen
- Stats changing as different parts are equipped

#### Part 8: Best Practices and Balancing (9:45 - 10:00)

**Script:**
"Remember a few key things when creating parts. Use consistent naming conventions - DT_ for data tables, DA_ for data assets, and meaningful part IDs.

Organize your folder structure with separate folders for parts, skins, and configs. This keeps your project organized as you add hundreds of parts.

Most importantly, think about balance. More powerful parts should cost more and require higher levels. High-damage weapons should be heavier or consume more energy. Create meaningful trade-offs so players make interesting choices.

That's it! You now know how to create ship parts, skins, and physics configurations. Go create an amazing ship customization system!"

**Visuals:**
- Content Browser showing organized folder structure
- DT_ShipParts_Engines highlighted
- Balance scale icon
- Title card: "Create Your Custom Ships!"
- Credits

---

## Appendix A: CSV Import Format

### How to Bulk Import Parts from CSV

If you prefer creating parts in a spreadsheet, you can export/import as CSV.

**Export CSV:**
1. Right-click data table → Asset Actions → Export to CSV
2. Open in Excel or Google Sheets

**CSV Format for FShipPartData:**
```
Row Name,PartID,DisplayName,Description,Category,Rarity,LevelRequirement,UnlockCost,bUnlockedByDefault,ThrustPower,MaxVelocity,RotationSpeed,Acceleration,HullIntegrity,ShieldStrength,WeaponDamage,EnergyCapacity,EnergyRegenRate,Mass,ParentPartID,bCanBeUpgraded,UpgradeLevel
Engine_Basic,Engine_Basic,Basic Engine,Standard propulsion,Engine,Common,1,0,TRUE,1.0,1.0,1.0,1.0,100.0,0.0,1.0,100.0,10.0,1000.0,,TRUE,0
Engine_Advanced,Engine_Advanced,Advanced Engine,High-performance engine,Engine,Uncommon,5,500,FALSE,1.5,1.2,1.0,1.3,100.0,0.0,1.0,100.0,10.0,1100.0,Engine_Basic,TRUE,1
```

**Import CSV:**
1. Create new data table with FShipPartData
2. Right-click → Asset Actions → Import from CSV
3. Select your CSV file

---

## Appendix B: Recommended Initial Content Package

Start with these 15 parts minimum:

### Engines (3):
- Engine_Basic
- Engine_Advanced
- Engine_Racing

### Thrusters (2):
- Thruster_Basic
- Thruster_Agile

### Hulls (3):
- Hull_Light
- Hull_Standard
- Hull_Reinforced

### Weapons (3):
- Weapon_Pulse
- Weapon_Laser
- Weapon_Missile

### Shields (2):
- Shield_Basic
- Shield_Advanced

### Skins (3):
- Skin_Default
- Skin_Racing
- Skin_Chrome

### Physics Configs (2):
- DA_LightFighter
- DA_HeavyFreighter

This gives players meaningful choices while keeping data manageable for your first iteration.

---

## Appendix C: Advanced Topics

### Creating Part Upgrade Chains

Set ParentPartID to create progression:
```
Engine_Basic (UpgradeLevel: 0)
  ↓ (ParentPartID: Engine_Basic)
Engine_Advanced (UpgradeLevel: 1)
  ↓ (ParentPartID: Engine_Advanced)
Engine_Racing (UpgradeLevel: 2)
```

This visually shows upgrade paths in UI and prevents players from having circular upgrade chains.

### Synergy Bonuses

While not built-in, you can create synergies by naming related parts. For example, all Combat parts might have a naming prefix:
- Combat_Engine
- Combat_Thruster
- Combat_Weapon
- Combat_Hull

Then in code, detect these prefixes and apply bonus multipliers when a full Combat loadout is equipped.

### Special Events and Limited Skins

Create seasonal or event-specific skins with extremely high rarity and limited availability:
```
Skin_HalloweenSpecial (Legendary, cost: 5000, level: 20)
Skin_ViceAdmiral (Legendary, cost: 0, level: 1, unlocked by specific achievement)
```

### Custom Stat Modifiers

If you need stats beyond the 10 defined ones, consider extending FShipStats. However, this requires C++ changes. For designers working purely in data, use existing stats creatively:
- Use ThrustPower for general power
- Use Mass for weight/defense
- Use EnergyCapacity for energy/shields
- Use EnergyRegenRate for efficiency

---

## Appendix D: Performance Optimization

### Data Table Size Limits

- **Optimal:** 20-50 rows per data table
- **Acceptable:** 50-200 rows per data table
- **Needs Optimization:** 200+ rows in single table

### Soft References for Assets

Always use TSoftObjectPtr for large assets:
```cpp
TSoftObjectPtr<UStaticMesh> PartMesh;      // Good - loads on demand
UStaticMesh* PartMesh;                      // Bad - always loaded
```

### Caching Loaded Data

The ShipCustomizationComponent caches FlightController and mesh references. If you notice slowness:
1. Profile with Unreal Insights
2. Check that data tables aren't being reloaded constantly
3. Verify soft object pointers aren't forcing too many loads

---

## Appendix E: Troubleshooting Checklist

Before asking for help, verify:

- [ ] Data table was created with correct row structure (FShipPartData)
- [ ] All required fields are filled (no empty PartID, DisplayName, etc.)
- [ ] Part IDs are case-consistent (Engine_Basic vs engine_basic)
- [ ] Physics config was created as Data Asset (not Data Table)
- [ ] Ship blueprint component has data table references assigned
- [ ] Level requirement is reasonable (not 0 or 99)
- [ ] Unlock costs match rarity (Common < Uncommon < Rare < Epic < Legendary)
- [ ] Stat modifiers are positive numbers (no negatives for multiplicative stats)
- [ ] Part mesh and icon references are valid (if assigned)
- [ ] SaveGame slot name is consistent between saves

---

## Quick Reference: Step-by-Step Checklist

### Creating a New Part (Any Category)

- [ ] Open or create DT_ShipParts_[Category] data table
- [ ] Click Add Row and name it [Type]_[Variant]
- [ ] Fill in PartID, DisplayName, Description
- [ ] Set Category and Rarity
- [ ] Set LevelRequirement and UnlockCost
- [ ] Set bUnlockedByDefault (usually FALSE for upgrades)
- [ ] Fill in StatModifiers (or copy from similar part)
- [ ] Optionally set PartMesh and Icon references
- [ ] Set ParentPartID if this is an upgrade
- [ ] Set UpgradeLevel (0 for base, 1-5 for upgrades)
- [ ] Press Ctrl+S to save

### Creating a New Physics Config

- [ ] Right-click in Content Browser
- [ ] Miscellaneous → Data Asset → UShipPhysicsConfig
- [ ] Name it DA_[ConfigName]
- [ ] Fill in ConfigName and Description
- [ ] Set Mass, ThrustPower, MaxVelocity
- [ ] Set AngularThrustPower and RotationSpeedMultiplier
- [ ] Set StabilityAssistStrength (0-1)
- [ ] Press Ctrl+S to save

### Adding to Ship Blueprint

- [ ] Open BP_VRSpaceshipPlayer
- [ ] Find ShipCustomizationComponent in components list
- [ ] Assign ShipPartsDataTable
- [ ] Assign ShipSkinsDataTable
- [ ] Assign PhysicsConfig
- [ ] Press Compile and Save

---

## Conclusion

You now have a complete guide for creating ship customization assets in Unreal Engine. The system is flexible and extensible - start with the basic 15-part content package, test thoroughly, then expand based on player feedback and game balance data.

Remember: great game balance comes from iteration. Create your initial parts, test them extensively, gather data on which builds players use, then adjust based on what you learn.

Happy designing!

---

**Document Version History:**
- v1.0 (November 2025): Initial comprehensive guide

**Additional Resources:**
- Unreal Engine Data Tables Documentation
- ShipCustomizationComponent.h (source code reference)
- Alexander Game Design Document

**Contact:** For questions about this guide or ship customization, contact the design team.
