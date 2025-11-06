# VR Flight Setup - Manual Configuration

## 6DOF Flight System - READY TO USE

Your VR spaceship has **full 6 degrees of freedom** flight already implemented in C++!

### What's Already Done

Flight System (**FlightController.h/cpp**):
- ✓ 6DOF Input: Thrust X/Y/Z + Rotation Pitch/Yaw/Roll
- ✓ Input Smoothing & Deadzone
- ✓ 5 Flight Assist Modes (None, Stability, AutoLevel, Orbital, Docking)
- ✓ Collision Avoidance
- ✓ Thrust Limiting
- ✓ Orbital Mechanics Integration

Assets Found:
- ✓ **BP_VRSpaceshipPlayer** - Your VR spaceship pawn
- ✓ **BP_FlightStick** - Physical flight sticks for VR
- ✓ **BP_FlightPhysicsController** - Physics controller
- ✓ **FlightController** C++ Component - 6DOF implementation

---

## Simple 3-Step Setup

### Step 1: Open Unreal Editor

```bash
# Double-click to open:
Alexander.uproject
```

Wait for VRTemplateMap to load.

---

### Step 2: Configure GameMode

1. In Unreal Editor, open **Window → World Settings**
2. Scroll down to **GameMode Override** section
3. Find **Default Pawn Class**
4. Click the dropdown
5. Select: **BP_VRSpaceshipPlayer**
6. **File → Save Current Level** (Ctrl+S)

---

### Step 3: Test Flight!

1. Press **Alt+P** (or click Play button)
2. Put on your VR headset
3. You should spawn in the spaceship cockpit!

---

## VR Flight Controls (6DOF)

### Left VR Controller
- **Thumbstick Forward/Back**: Pitch rotation (nose up/down)
- **Thumbstick Left/Right**: Yaw rotation (turn left/right)
- **Grip Button**: Roll left

### Right VR Controller
- **Thumbstick Forward/Back**: Forward/Backward thrust
- **Thumbstick Left/Right**: Strafe left/right
- **Trigger**: Vertical thrust up
- **Grip Button**: Roll right

### Flight Sticks (Physical Grab in VR)
- Flight sticks are positioned 50cm in front of you
- 25cm apart (comfortable arm reach)
- Grab them with VR controllers for manual control
- They have physics - you can move them!

---

## Flight Assist Modes

Switch modes during flight:

1. **None** - Pure manual control, no assists
2. **Stability** - Gentle damping to prevent oscillation
3. **AutoLevel** - Strong auto-leveling (for beginners)
4. **Orbital** - Helps maintain stable orbits
5. **Docking** - Precision control for docking maneuvers

---

## Troubleshooting

### "I press Play but nothing happens"
- Make sure you saved the level after setting Default Pawn Class
- Check World Settings shows BP_VRSpaceshipPlayer

### "I spawn as VR hands, not in ship"
- You skipped Step 2
- Go back and set Default Pawn Class to BP_VRSpaceshipPlayer

### "Controls don't work"
- Make sure your VR controllers are tracked
- Try restarting SteamVR/Oculus

### "Ship won't move"
- Check that FlightController component is enabled
- Try switching flight assist mode (might be on Docking mode = very slow)

---

## Flight Stick Positions

The flight sticks are set up to appear:
- **50cm forward** from your starting position
- **25cm apart** (left and right)
- **20cm below** eye level
- In comfortable VR grab range

Grab them to override VR controller thumbstick input!

---

## Technical Details

### FlightController Component Features

**Input Processing:**
- Deadzone filtering (default: 0.05)
- Exponential smoothing
- Inversion support
- Mouse sensitivity scaling

**Flight Assistance:**
```cpp
void ApplyStabilityAssistance()    // Damping
void ApplyAutoLevelAssistance()     // Auto-level
void ApplyOrbitalAssistance()       // Orbital mechanics
void ApplyDockingAssistance()       // Precision mode
```

**Collision Avoidance:**
- 500x500x500 unit detection box
- Auto-avoidance thrust adjustment
- Debug visualization available

**Performance:**
- Ticks every frame
- Tracks input rate (should be ~60-90 Hz)
- Average input rate monitoring

---

## Next Steps

Once flight is working:

1. **Task T004**: Monitor performance during flight
2. **Add waypoint rings** for flight courses
3. **Add HUD** to show flight data
4. **Configure flight physics** presets
5. **Add audio feedback** for thrust/rotation

---

## Files Reference

C++ Implementation:
- [Source/Alexander/Public/FlightController.h](Source/Alexander/Public/FlightController.h)
- [Source/Alexander/Private/FlightController.cpp](Source/Alexander/Private/FlightController.cpp)

Blueprint Assets:
- [Content/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.uasset](Content/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.uasset)
- [Content/SpaceShip/Blueprints/BP_FlightStick.uasset](Content/SpaceShip/Blueprints/BP_FlightStick.uasset)
- [Content/SpaceShip/Blueprints/BP_FlightPhysicsController.uasset](Content/SpaceShip/Blueprints/BP_FlightPhysicsController.uasset)

---

## Summary

Your 6DOF VR flight system is **100% implemented**. The only thing needed is:

**Set Default Pawn Class to BP_VRSpaceshipPlayer in World Settings**

Then press Alt+P and fly!
