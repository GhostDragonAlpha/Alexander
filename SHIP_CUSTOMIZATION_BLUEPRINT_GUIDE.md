# Ship Customization Blueprint Guide

## Overview

This guide covers how to use the Ship Customization system in Blueprints. The C++ backend is fully implemented, and this documentation provides designer-friendly workflows for creating ship customization features in your VR flight game.

## Table of Contents

1. [Quick Start](#quick-start)
2. [Component Setup](#component-setup)
3. [Data Asset Creation](#data-asset-creation)
4. [Blueprint Function Library](#blueprint-function-library)
5. [Common Blueprint Patterns](#common-blueprint-patterns)
6. [UI/HUD Integration](#uihud-integration)
7. [Physics Configuration](#physics-configuration)
8. [Troubleshooting](#troubleshooting)

---

## Quick Start

### 5-Minute Setup

1. **Add Component to Ship Blueprint**
   - Open your ship Blueprint (e.g., `BP_PlayerShip`)
   - Add Component > Search "Ship Customization Component"
   - Add the component to your ship

2. **Create Data Tables**
   - Right-click in Content Browser > Miscellaneous > Data Table
   - Choose `FShipPartData` as Row Structure
   - Name it `DT_ShipParts`
   - Repeat for `FShipSkinData` (name it `DT_ShipSkins`)

3. **Assign Data Tables**
   - Select the Ship Customization Component
   - In Details panel, assign:
     - Ship Parts Data Table: `DT_ShipParts`
     - Ship Skins Data Table: `DT_ShipSkins`

4. **Create Physics Config**
   - Right-click > Miscellaneous > Data Asset
   - Choose `ShipPhysicsConfig`
   - Name it `DA_LightFighter`
   - Configure physics values (see [Physics Configuration](#physics-configuration))

5. **Apply Physics Config**
   - In Event Graph:
   ```
   Event BeginPlay
   -> Get Ship Customization Component (Self)
   -> Apply Physics Config (DA_LightFighter)
   ```

---

## Component Setup

### Adding to Player Ship

The Ship Customization Component should be added to your player ship Blueprint:

**Blueprint Path**: `Content/Blueprints/Ships/BP_PlayerShip`

**Steps**:
1. Open BP_PlayerShip
2. Components panel > Add Component > Ship Customization Component
3. Name it "ShipCustomization"
4. In Details panel:
   - Ship Parts Data Table: `DT_ShipParts`
   - Ship Skins Data Table: `DT_ShipSkins`
   - Physics Config: `DA_DefaultShip` (optional preset)

### Component References

To get the component in Blueprints:

**Method 1: Direct Component Reference (Same Actor)**
```
Get Ship Customization Component (Target: Self)
```

**Method 2: From Any Actor (Blueprint Function Library)**
```
Get Ship Customization Component (Ship: TargetActor)
-> Returns: Ship Customization Component (or None if not found)
```

---

## Data Asset Creation

### Ship Parts Data Table

**Create a Data Table**:
1. Right-click in Content Browser
2. Miscellaneous > Data Table
3. Choose `FShipPartData` as Row Structure
4. Name: `DT_ShipParts`

**Example Part Entry**:

| Field | Example Value | Description |
|-------|--------------|-------------|
| Row Name | `engine_starter` | Unique ID for this part |
| Display Name | "Basic Impulse Engine" | User-facing name |
| Description | "Standard engine for beginners" | Part description |
| Category | Engine | Part category enum |
| Rarity | Common | Rarity level |
| Level Requirement | 1 | Required player level |
| Unlock Cost | 0 | Credits to unlock (0 = free) |
| Unlocked By Default | True | Start with this part unlocked? |
| Stat Modifiers | (See below) | Performance bonuses |
| Part Mesh | None | Optional 3D mesh |
| Icon | T_Engine_Starter | UI icon texture |

**Stat Modifiers Example (Basic Engine)**:
- ThrustPower: 1.0 (baseline)
- MaxVelocity: 1.0 (baseline)
- RotationSpeed: 1.0
- Acceleration: 1.0
- Hull Integrity: 0.0 (engines don't affect health)
- Shield Strength: 0.0
- Weapon Damage: 0.0
- Energy Capacity: 0.0
- Energy Regen Rate: 0.0
- Mass: 100.0

**Advanced Engine Example (Legendary)**:
- ThrustPower: 2.5 (+150% thrust)
- MaxVelocity: 1.8 (+80% max speed)
- Acceleration: 2.0 (+100% acceleration)
- Mass: 150.0 (heavier)

### Ship Skins Data Table

**Create a Data Table**:
1. Right-click in Content Browser
2. Miscellaneous > Data Table
3. Choose `FShipSkinData` as Row Structure
4. Name: `DT_ShipSkins`

**Example Skin Entry**:

| Field | Example Value | Description |
|-------|--------------|-------------|
| Row Name | `skin_default` | Unique skin ID |
| Display Name | "Standard Paint" | User-facing name |
| Description | "Classic ship paint" | Skin description |
| Rarity | Common | Rarity level |
| Skin Material | M_Ship_Default | Material Instance |
| Icon | T_Skin_Default | UI icon texture |
| Unlock Cost | 0 | Credits to unlock |
| Unlocked By Default | True | Start unlocked? |

### Physics Configuration Data Asset

**Create Physics Config**:
1. Right-click in Content Browser
2. Miscellaneous > Data Asset
3. Choose `ShipPhysicsConfig`
4. Name: `DA_LightFighter`

**Example Values** (see [SHIP_PHYSICS_PRESETS_EXAMPLES.md](SHIP_PHYSICS_PRESETS_EXAMPLES.md) for complete presets):

| Parameter | Value | Description |
|-----------|-------|-------------|
| Config Name | "Light Fighter" | Preset name |
| Mass | 500.0 kg | Low mass for agility |
| Thrust Power | 150000.0 N | High thrust |
| Max Velocity | 1500.0 m/s | Very fast |
| Angular Thrust Power | 75000.0 N | High rotation |
| Acceleration Multiplier | 1.5 | Quick response |
| Rotation Speed Multiplier | 1.3 | Agile turning |

---

## Blueprint Function Library

All customization functions are available through the **Ship Customization Blueprint Library**. These are static functions you can call from any Blueprint.

### Core Functions

#### Get Ship Customization Component
```
Input: Ship (Actor)
Output: Ship Customization Component (or None)
```
Gets the customization component from any actor.

#### Apply Physics Config
```
Input: Component, Physics Config (Data Asset)
Output: Boolean (success)
```
Applies a physics preset to the ship.

#### Equip Part by ID
```
Input: Component, Category (Enum), Part ID (Name)
Output: Boolean (equipped successfully)
```
Equips a specific part. Example:
```
Category: Engine
Part ID: "engine_advanced"
```

#### Get Current Stats
```
Input: Component
Output: Ship Stats (struct)
```
Returns total stats from all equipped parts.

### Example: Equip Engine Blueprint

**Event Graph Example**:
```
[Button Clicked Event]
-> Get Ship Customization Component (Self)
-> Equip Part by ID
   - Category: Engine
   - Part ID: "engine_advanced"
-> Branch (if successful)
   -> True: Update Ship Visuals
   -> False: Show Error Message
```

### Example: Display Current Stats

```
[Update Stats UI Event]
-> Get Ship Customization Component (PlayerShipRef)
-> Get Current Stats
-> Format Stats for Display (Blueprint Library)
-> Set Text (StatsTextBlock)
```

---

## Common Blueprint Patterns

### Pattern 1: Part Selection UI

**Use Case**: Display all available parts in a category for player to choose

```
[Widget Blueprint: WBP_PartSelector]

Event Construct:
-> Get Ship Customization Component (OwningPlayerPawn)
-> Get Unlocked Parts in Category (Category: Engine)
-> ForEach Loop
   -> Create Widget (WBP_PartButton)
   -> Set Part Data (Icon, Name, Stats)
   -> Add to Scroll Box
```

### Pattern 2: Loadout Saver

**Use Case**: Save current ship configuration

```
[Widget: WBP_LoadoutManager]

On Save Button Clicked:
-> Get Ship Customization Component (PlayerShip)
-> Save Loadout (Loadout Name: TextBox.GetText())
-> Show Success Message
```

### Pattern 3: Part Unlock System

**Use Case**: Unlock part when player earns enough credits

```
On Purchase Button Clicked:
-> Get Ship Customization Component (PlayerShip)
-> Get Credits (returns current credits)
-> Branch (Credits >= Part Cost?)
   -> True:
      -> Unlock Part (Part ID)
      -> Add Credits (-PartCost)
      -> Refresh UI
   -> False:
      -> Show "Insufficient Credits" message
```

### Pattern 4: Physics Preset Switcher

**Use Case**: Let player switch between flight profiles

```
On Preset Selected:
-> Get Ship Customization Component (Self)
-> Select (on PresetType enum)
   -> LightFighter: Apply Physics Config (DA_LightFighter)
   -> HeavyFreighter: Apply Physics Config (DA_HeavyFreighter)
   -> Balanced: Apply Physics Config (DA_BalancedScout)
```

### Pattern 5: Live Stats Comparison

**Use Case**: Show stat differences when hovering over a new part

```
On Part Button Hovered:
-> Get Ship Customization Component (PlayerShip)
-> Get Current Stats (save to variable: CurrentStats)
-> Get Part Data (Hovered Part ID)
-> Compare Ship Stats (CurrentStats, NewPartStats)
-> Update Comparison UI (show green/red arrows for improvements/decreases)
```

---

## UI/HUD Integration

### HUD Widget Setup

**Example Widget Hierarchy**:
```
WBP_ShipCustomizationMenu (User Widget)
|
+-- Canvas Panel
    |
    +-- Part Categories (Horizontal Box)
    |   |-- Engine Button
    |   |-- Thruster Button
    |   |-- Hull Button
    |   |-- Weapon Button
    |
    +-- Part List (Scroll Box)
    |   |-- WBP_PartButton (x N) [Dynamic]
    |
    +-- Stats Display (Vertical Box)
    |   |-- Thrust Power Text
    |   |-- Max Velocity Text
    |   |-- etc...
    |
    +-- Preview Viewport (3D ship preview)
    |
    +-- Loadout Manager
        |-- Save Button
        |-- Load Dropdown
```

### Binding to Events

The Ship Customization Component fires events when changes occur. Bind to these in your UI:

**OnLoadoutChanged Event**:
```
Event Graph (WBP_ShipCustomizationMenu):

Event Construct:
-> Get Ship Customization Component (PlayerShip)
-> Bind Event to OnLoadoutChanged
   -> Custom Event: RefreshStatsDisplay
```

**OnPartEquipped Event**:
```
Bind Event to OnPartEquipped:
-> Custom Event: UpdatePartVisualFeedback
   - Highlight equipped part
   - Refresh stats
   - Play equip sound
```

**OnPlayerLevelUp Event**:
```
Bind Event to OnPlayerLevelUp:
-> Custom Event: ShowLevelUpNotification
   -> Unlock new parts based on level
   -> Refresh part list
```

### Example: Part Button Widget

**WBP_PartButton Blueprint**:

**Variables**:
- PartData (FShipPartData)
- bIsUnlocked (Boolean)
- bIsEquipped (Boolean)

**Designer Tab**:
- Icon Image
- Part Name Text
- Rarity Border (colored by rarity)
- Locked Overlay (visible when !bIsUnlocked)
- Equipped Indicator (visible when bIsEquipped)
- Stats Preview (tooltip)

**Event Graph**:
```
On Button Clicked:
-> Branch (bIsUnlocked)
   -> True:
      -> Get Ship Customization Component
      -> Equip Part by ID (PartData.PartID)
   -> False:
      -> Show Unlock Dialog
```

---

## Physics Configuration

### Understanding Physics Parameters

The physics system controls how your ship flies. Each parameter affects flight feel:

| Parameter | Effect | Low Value | High Value |
|-----------|--------|-----------|------------|
| **Mass** | Inertia and momentum | Light, agile, affected by gravity more | Heavy, sluggish, stable |
| **Thrust Power** | Acceleration force | Slow acceleration | Fast acceleration |
| **Max Velocity** | Top speed limit | Slower max speed | Faster max speed |
| **Angular Thrust** | Rotation force | Slow turning | Fast turning |
| **Acceleration Multiplier** | Input responsiveness | Floaty, gradual | Snappy, instant |
| **Rotation Speed Multiplier** | Turning speed | Gentle turns | Sharp turns |
| **Stability Assist** | Auto-stabilization | Manual flight control | Auto-leveling |

### Creating Custom Presets

**Step-by-Step**:

1. **Create Data Asset**:
   - Right-click > Miscellaneous > Data Asset
   - Choose `ShipPhysicsConfig`
   - Name: `DA_CustomPreset`

2. **Configure Values**:
   - Config Name: "My Custom Ship"
   - Description: "Fast and agile fighter"
   - Set physics parameters (see examples below)

3. **Test In-Game**:
   ```
   Event BeginPlay (or Debug Key):
   -> Get Ship Customization Component (Self)
   -> Apply Physics Config (DA_CustomPreset)
   ```

4. **Iterate**:
   - Fly the ship
   - Adjust values in Data Asset
   - Press "Hot Reload" or restart PIE session

### Example Preset: Racing Ship

```
Config Name: "Speed Demon"
Description: "Maximum velocity, minimum everything else"

Mass: 400.0 kg (very light)
Thrust Power: 200000.0 N (very high)
Max Velocity: 2000.0 m/s (extremely fast)
Angular Thrust Power: 40000.0 N (moderate turning)
Acceleration Multiplier: 2.0 (instant response)
Rotation Speed Multiplier: 1.0 (normal turning)
Stability Assist: 0.2 (minimal - player must control)
```

**Flight Feel**: Blazing fast, requires skilled piloting, minimal assistance.

### Example Preset: Tank Ship

```
Config Name: "Juggernaut"
Description: "Heavy and powerful, slow but unstoppable"

Mass: 2000.0 kg (very heavy)
Thrust Power: 250000.0 N (powerful engines needed)
Max Velocity: 500.0 m/s (slow top speed)
Angular Thrust Power: 100000.0 N (needs power to turn)
Acceleration Multiplier: 0.5 (slow response)
Rotation Speed Multiplier: 0.7 (sluggish turning)
Stability Assist: 0.8 (lots of help needed)
```

**Flight Feel**: Like piloting a battleship. Momentum-based, heavy, powerful.

---

## Advanced Patterns

### Dynamic Loadout System

**Use Case**: Switch between combat/exploration loadouts on-the-fly

```
[Player Controller Blueprint]

Input Action: Switch Loadout (Tab Key)
-> Get Ship Customization Component (Pawn)
-> Get Saved Loadouts
-> Increment CurrentLoadoutIndex (wrap around)
-> Load Loadout (CurrentLoadoutIndex)
-> Show Loadout Name notification
```

### Part Upgrade Tree

**Use Case**: Unlock advanced parts by upgrading base parts

```
On Upgrade Button Clicked:
-> Get Part Data (CurrentPartID)
-> Branch (Can Be Upgraded == True)
   -> True:
      -> Find Upgraded Part (UpgradeLevel + 1)
      -> Check Credits and Level Requirement
      -> Unlock Part (UpgradePartID)
      -> Auto-Equip upgraded part
```

### Performance-Based Unlocks

**Use Case**: Unlock parts based on race performance

```
On Race Completed:
-> Calculate Score (Time, Rings Hit, etc.)
-> Branch (Score >= Gold Threshold)
   -> True:
      -> Award XP (500)
      -> Award Credits (1000)
      -> Unlock Part ("engine_legendary")
      -> Show Unlock Notification
```

---

## Troubleshooting

### Common Issues

#### Issue: "Component not found"
**Symptom**: `Get Ship Customization Component` returns None

**Solutions**:
1. Verify component is added to the actor
2. Check spelling of component name (case-sensitive)
3. Ensure you're calling on the correct actor reference
4. Use `Get Ship Customization Component` (Blueprint Library) instead of direct component access

#### Issue: "Parts not showing in UI"
**Symptom**: Part list is empty

**Solutions**:
1. Verify Data Table is assigned to component
2. Check Data Table has rows populated
3. Ensure Row Names are unique
4. Verify `bUnlockedByDefault` is True for starter parts
5. Check category filter matches parts in table

#### Issue: "Physics config not applying"
**Symptom**: Ship still flies the same after applying preset

**Solutions**:
1. Verify FlightController component exists on ship
2. Check Physics Config Data Asset is valid (not null)
3. Call `Apply Stats to Flight Controller` after equipping parts
4. Ensure values in Data Asset are reasonable (not 0 or negative)

#### Issue: "Stats not calculating correctly"
**Symptom**: Total stats don't match equipped parts

**Solutions**:
1. Call `Calculate Total Stats` after equipping parts
2. Verify all equipped parts have valid Stat Modifiers
3. Check for negative stat values causing issues
4. Ensure Data Table Row References are correct

#### Issue: "Visuals not updating"
**Symptom**: Ship appearance doesn't change when equipping parts/skins

**Solutions**:
1. Call `Update Ship Visuals` after equipping
2. Verify Part Mesh soft references are valid
3. Check Skin Material Instance paths
4. Ensure ship has a Static Mesh Component to apply materials to

---

## Best Practices

### Performance

1. **Cache Component References**: Store the Ship Customization Component as a variable instead of calling `Get Component` every frame
2. **Update Stats Only When Changed**: Don't recalculate stats every tick, only when loadout changes
3. **Async Load Assets**: Use soft object references for meshes/materials to avoid loading all assets at startup
4. **Limit UI Refreshes**: Only update UI when data actually changes (use event bindings)

### Design

1. **Start Simple**: Begin with 3-5 parts per category, expand later
2. **Clear Stat Differences**: Make stat changes noticeable (minimum 10% difference)
3. **Visual Feedback**: Always show stat comparisons before equipping
4. **Unlock Progression**: Gate powerful parts behind level requirements
5. **Loadout Slots**: Provide 3-5 loadout save slots for variety

### Testing

1. **Test All Combinations**: Verify extreme stat combinations work (all lightweight parts, all heavy parts)
2. **Validate Data**: Check for missing textures, invalid references in Data Tables
3. **Balance Testing**: Playtest each preset for 10+ minutes to verify fun factor
4. **Multiplayer**: If applicable, verify stats replicate correctly

---

## Additional Resources

- **Physics Presets Guide**: [SHIP_PHYSICS_PRESETS_EXAMPLES.md](SHIP_PHYSICS_PRESETS_EXAMPLES.md)
- **C++ API Reference**: See `ShipCustomizationComponent.h` for all available functions
- **Flight Controller Tuning**: See `FLIGHTCONTROLLER_PHYSICS_TUNING_GUIDE.md`

---

## Quick Reference: All Blueprint Library Functions

### Component Access
- `Get Ship Customization Component(Actor)` -> Component

### Parts
- `Equip Part by ID(Component, Category, PartID)` -> Bool
- `Unequip Part(Component, Category)` -> Bool
- `Get Equipped Part(Component, Category)` -> PartData, bFound

### Skins
- `Equip Skin by ID(Component, SkinID)` -> Bool
- `Get Equipped Skin(Component)` -> SkinData, bFound

### Stats
- `Get Current Stats(Component)` -> ShipStats
- `Apply Stats to Flight Controller(Component)` -> Bool
- `Format Stats for Display(ShipStats)` -> Text
- `Compare Ship Stats(StatsA, StatsB)` -> StatsDifference

### Loadouts
- `Save Loadout(Component, LoadoutName)` -> Bool
- `Load Loadout(Component, LoadoutIndex)` -> Bool
- `Delete Loadout(Component, LoadoutIndex)` -> Bool
- `Get Saved Loadouts(Component)` -> Array<Loadout>

### Progression
- `Unlock Part(Component, PartID)` -> Bool
- `Unlock Skin(Component, SkinID)` -> Bool
- `Is Part Unlocked(Component, PartID)` -> Bool
- `Is Skin Unlocked(Component, SkinID)` -> Bool
- `Add XP(Component, Amount)`
- `Add Credits(Component, Amount)`
- `Get Player Level(Component)` -> Int
- `Get Credits(Component)` -> Int

### Data Queries
- `Get Parts in Category(Component, Category)` -> Array<PartData>
- `Get Unlocked Parts in Category(Component, Category)` -> Array<PartData>
- `Get All Skins(Component)` -> Array<SkinData>
- `Get Unlocked Skins(Component)` -> Array<SkinData>
- `Get Part Data(Component, PartID)` -> PartData, bFound
- `Get Skin Data(Component, SkinID)` -> SkinData, bFound

### Visuals
- `Update Ship Visuals(Component)` -> Bool
- `Apply Skin Material(Component)` -> Bool

### Utility
- `Get Rarity Color(Rarity)` -> LinearColor

### Save/Load
- `Save Customization Data(Component)` -> Bool
- `Load Customization Data(Component)` -> Bool

### Physics
- `Apply Physics Config(Component, PhysicsConfig)` -> Bool

---

**End of Guide**
