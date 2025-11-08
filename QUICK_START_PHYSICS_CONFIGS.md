# Quick Start: Physics Configuration Data Assets

**GOAL**: Create 10 ship physics configuration data assets in Unreal Editor

---

## Method 1: Automated (5 minutes)

### Prerequisites
- Unreal Editor 5.0+ with Python enabled
- Project compiled successfully

### Steps

1. **Open Unreal Editor** with your Alexander project

2. **Open Python Console**:
   - Menu: Window > Developer Tools > Python Console
   - Or press: `Ctrl+Shift+P` (if mapped)

3. **Navigate to project directory** in Python console:
   ```python
   import os
   os.chdir(r'C:\Users\allen\Desktop\Alexander\Alexander')
   ```

4. **Run the generator script**:
   ```python
   exec(open('generated_create_physics_assets.py').read())
   ```

5. **Wait for completion**. You should see:
   ```
   Creating Ship Physics Config Data Assets...
   Creating DA_LightFighter...
     ✓ Created: DA_LightFighter
   Creating DA_HeavyFreighter...
     ✓ Created: DA_HeavyFreighter
   ...
   Created 10 / 10 assets
   ```

6. **Verify assets created**:
   - Open Content Browser
   - Navigate to: `Content/Data/PhysicsConfigs/`
   - You should see 10 data assets:
     - DA_LightFighter
     - DA_HeavyFreighter
     - DA_BalancedScout
     - DA_SpeedDemon
     - DA_Juggernaut
     - DA_Interceptor
     - DA_StealthScout
     - DA_AssaultBomber
     - DA_CivilianShuttle
     - DA_SupportShip

7. **Done!** Proceed to testing.

---

## Method 2: Manual (30 minutes)

If the automated script doesn't work, create assets manually:

### For Each of the 10 Configs

1. **Right-click** in `Content/Data/PhysicsConfigs/` folder
2. Select: **Miscellaneous > Data Asset**
3. Choose asset class: **ShipPhysicsConfig**
4. Name it (see below for names)
5. **Double-click** to open
6. **Copy values** from the tables below
7. **Save** (Ctrl+S)

### Config 1: Light Fighter

**Asset Name**: `DA_LightFighter`

**Config Settings**:
- Config Name: `Light Fighter`
- Description: `Fast and agile combat ship with excellent maneuverability`

**Physics Parameters**:
```
Mass: 500.0
Thrust Power: 150000.0
Max Velocity: 1500.0
Angular Thrust Power: 75000.0
Fuel Capacity: 800.0
Fuel Consumption Rate: 1.2
```

**Flight Characteristics**:
```
Acceleration Multiplier: 1.5
Rotation Speed Multiplier: 1.3
Stability Assist Strength: 0.4
```

---

### Config 2: Heavy Freighter

**Asset Name**: `DA_HeavyFreighter`

**Config Settings**:
- Config Name: `Heavy Freighter`
- Description: `Massive cargo hauler with powerful thrust and high stability`

**Physics Parameters**:
```
Mass: 3000.0
Thrust Power: 200000.0
Max Velocity: 600.0
Angular Thrust Power: 60000.0
Fuel Capacity: 2000.0
Fuel Consumption Rate: 2.5
```

**Flight Characteristics**:
```
Acceleration Multiplier: 0.6
Rotation Speed Multiplier: 0.5
Stability Assist Strength: 0.8
```

---

### Config 3: Balanced Scout

**Asset Name**: `DA_BalancedScout`

**Config Settings**:
- Config Name: `Balanced Scout`
- Description: `Well-rounded ship suitable for all mission types`

**Physics Parameters**:
```
Mass: 1000.0
Thrust Power: 120000.0
Max Velocity: 1000.0
Angular Thrust Power: 60000.0
Fuel Capacity: 1200.0
Fuel Consumption Rate: 1.5
```

**Flight Characteristics**:
```
Acceleration Multiplier: 1.0
Rotation Speed Multiplier: 1.0
Stability Assist Strength: 0.5
```

---

### Config 4: Speed Demon

**Asset Name**: `DA_SpeedDemon`

**Config Settings**:
- Config Name: `Speed Demon`
- Description: `Maximum velocity racing ship with minimal flight assistance`

**Physics Parameters**:
```
Mass: 350.0
Thrust Power: 180000.0
Max Velocity: 2000.0
Angular Thrust Power: 50000.0
Fuel Capacity: 600.0
Fuel Consumption Rate: 2.0
```

**Flight Characteristics**:
```
Acceleration Multiplier: 2.0
Rotation Speed Multiplier: 0.9
Stability Assist Strength: 0.2
```

---

### Config 5: Juggernaut

**Asset Name**: `DA_Juggernaut`

**Config Settings**:
- Config Name: `Juggernaut`
- Description: `Heavily armored assault ship with devastating firepower`

**Physics Parameters**:
```
Mass: 4000.0
Thrust Power: 300000.0
Max Velocity: 400.0
Angular Thrust Power: 120000.0
Fuel Capacity: 2500.0
Fuel Consumption Rate: 3.5
```

**Flight Characteristics**:
```
Acceleration Multiplier: 0.4
Rotation Speed Multiplier: 0.4
Stability Assist Strength: 0.9
```

---

### Config 6: Interceptor

**Asset Name**: `DA_Interceptor`

**Config Settings**:
- Config Name: `Interceptor`
- Description: `Fast-response combat ship designed for pursuit missions`

**Physics Parameters**:
```
Mass: 600.0
Thrust Power: 140000.0
Max Velocity: 1400.0
Angular Thrust Power: 70000.0
Fuel Capacity: 900.0
Fuel Consumption Rate: 1.3
```

**Flight Characteristics**:
```
Acceleration Multiplier: 1.4
Rotation Speed Multiplier: 1.4
Stability Assist Strength: 0.3
```

---

### Config 7: Stealth Scout

**Asset Name**: `DA_StealthScout`

**Config Settings**:
- Config Name: `Stealth Scout`
- Description: `Reconnaissance ship with low signature and precise handling`

**Physics Parameters**:
```
Mass: 700.0
Thrust Power: 100000.0
Max Velocity: 1200.0
Angular Thrust Power: 55000.0
Fuel Capacity: 1000.0
Fuel Consumption Rate: 1.1
```

**Flight Characteristics**:
```
Acceleration Multiplier: 1.1
Rotation Speed Multiplier: 1.2
Stability Assist Strength: 0.6
```

---

### Config 8: Assault Bomber

**Asset Name**: `DA_AssaultBomber`

**Config Settings**:
- Config Name: `Assault Bomber`
- Description: `Heavy weapons platform designed for anti-capital ship combat`

**Physics Parameters**:
```
Mass: 2500.0
Thrust Power: 180000.0
Max Velocity: 700.0
Angular Thrust Power: 80000.0
Fuel Capacity: 1800.0
Fuel Consumption Rate: 2.2
```

**Flight Characteristics**:
```
Acceleration Multiplier: 0.7
Rotation Speed Multiplier: 0.6
Stability Assist Strength: 0.7
```

---

### Config 9: Civilian Shuttle

**Asset Name**: `DA_CivilianShuttle`

**Config Settings**:
- Config Name: `Civilian Shuttle`
- Description: `Safe and stable passenger transport with maximum comfort`

**Physics Parameters**:
```
Mass: 1500.0
Thrust Power: 110000.0
Max Velocity: 800.0
Angular Thrust Power: 50000.0
Fuel Capacity: 1400.0
Fuel Consumption Rate: 1.6
```

**Flight Characteristics**:
```
Acceleration Multiplier: 0.8
Rotation Speed Multiplier: 0.8
Stability Assist Strength: 0.9
```

---

### Config 10: Support Ship

**Asset Name**: `DA_SupportShip`

**Config Settings**:
- Config Name: `Support Ship`
- Description: `Reliable utility workhorse for repair and resupply missions`

**Physics Parameters**:
```
Mass: 1800.0
Thrust Power: 130000.0
Max Velocity: 900.0
Angular Thrust Power: 65000.0
Fuel Capacity: 1600.0
Fuel Consumption Rate: 1.8
```

**Flight Characteristics**:
```
Acceleration Multiplier: 0.9
Rotation Speed Multiplier: 0.9
Stability Assist Strength: 0.8
```

---

## Testing Your Configs

### Quick PIE Test

1. **Open your ship Blueprint** (e.g., BP_PlayerShip)

2. **Add test code to BeginPlay**:
   ```
   Event BeginPlay
   -> Load Asset (DA_LightFighter)
   -> Cast to ShipPhysicsConfig
   -> Print String (Config Name)
   ```

3. **Launch PIE** (Alt+P)

4. **Verify** config loaded correctly

### Full Validation

1. **Place PhysicsConfigUtility actor** in level

2. **Create validation Blueprint**:
   ```
   Get All Actors of Class (PhysicsConfigUtility)
   -> Get Array Element [0]
   -> Validate Physics Config (DA_LightFighter)
   -> Print (Is Valid, T/W Ratio, Warnings)
   ```

3. **Run for each config** to verify all valid

---

## Troubleshooting

### "ShipPhysicsConfig class not found"

**Solution**: Compile C++ code first
```bash
# Close Unreal Editor
# In project root:
./Build.bat
# Reopen Unreal Editor
```

### "Cannot create asset in Python"

**Solution**: Use manual creation method instead. Unreal Python asset creation can be finicky.

### "Config not applying to ship"

**Solution**:
1. Verify config exists in Content Browser
2. Check ShipCustomizationComponent exists on ship
3. Ensure ApplyPhysicsConfig is being called
4. Add debug logging to verify values

---

## Next Steps After Creation

1. **Compile Project**: Ensure PhysicsConfigUtility C++ code compiles
2. **Test in PIE**: Apply each config and test flight feel
3. **Tune if Needed**: Adjust values based on gameplay feel
4. **Integrate with Systems**: Connect to ship customization and flight controller

---

**For detailed information, see**:
- `PHYSICS_CONFIG_CREATION_GUIDE.md` - Comprehensive guide
- `PHASE2_PHYSICS_CONFIG_SYSTEM_SUMMARY.md` - System overview
- `Content/Data/PhysicsConfigs.json` - Source data
