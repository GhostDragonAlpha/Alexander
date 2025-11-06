# VR Desktop Simulation System

## Problem
- Need to test VR interactions (hand tracking, grabbing, controller input) without VR headset
- Multi-client testing requires 3+ instances but we only have 1 headset
- Automated testing can't use physical VR hardware

## Solution: VR Input Simulator

Create a C++ component that simulates VR controller inputs using keyboard/mouse when running in desktop mode.

---

## Implementation Plan

### 1. VRInputSimulator Component (C++)

**File**: `Source/Alexander/Public/VRInputSimulator.h`

```cpp
// Simulates VR controller inputs for desktop testing
class ALEXANDER_API UVRInputSimulator : public UActorComponent
{
    GENERATED_BODY()

public:
    // Enable/disable simulation (auto-detects if VR headset present)
    UPROPERTY(EditAnywhere, Category = "VR Simulation")
    bool bAutoEnableInDesktopMode = true;

    // Simulated controller positions (relative to camera)
    FVector LeftControllerOffset = FVector(-30, -20, -10);   // Left hand position
    FVector RightControllerOffset = FVector(-30, 20, -10);   // Right hand position

    // Keyboard mappings
    // WASD - Move left controller
    // Arrow Keys - Move right controller
    // Q/E - Rotate left controller
    // Z/C - Trigger left/right
    // Left Shift/Right Shift - Grip left/right

    // Returns simulated controller positions in world space
    FVector GetLeftControllerPosition();
    FVector GetRightControllerPosition();

    // Returns button states
    bool IsLeftTriggerPressed();
    bool IsRightTriggerPressed();
    bool IsLeftGripPressed();
    bool IsRightGripPressed();

private:
    bool bIsActive = false;
    void UpdateSimulatedControllers(float DeltaTime);
};
```

### 2. Modified BP_VRSpaceshipPlayer

**Add VRInputSimulator component** to the VR pawn blueprint:
- Automatically activates when running with `-nohmd`
- Falls back to real VR input when headset detected
- Transparent to existing flight controller code

### 3. Keyboard Controls for Simulated VR

```
Left Controller (Flight Stick):
  W/A/S/D       - Move controller position
  Q/E           - Rotate controller
  Left Shift    - Grip (grab flight stick)
  Z             - Trigger

Right Controller (Flight Stick):
  Arrow Keys    - Move controller position
  [/]           - Rotate controller
  Right Shift   - Grip (grab flight stick)
  C             - Trigger

Camera:
  Mouse         - Look around (replaces head tracking)
  Mouse Wheel   - Adjust controller distance
```

### 4. Launch Command for Desktop VR Simulation

```bash
UnrealEditor.exe Alexander.uproject VRTemplateMap?game=/Script/Alexander.AutomationGameMode \
  -game \
  -windowed \
  -nohmd \
  -SimulateVR \  # Custom flag to enable VR simulation
  -HTTPPort=8080
```

---

## Benefits

✓ **Multi-client testing**: Run 3+ VR clients without headsets
✓ **Automated testing**: Script VR interactions via HTTP API
✓ **Consistent behavior**: Same code path as real VR
✓ **Fast iteration**: No need to put on/take off headset
✓ **CI/CD compatible**: Can run in headless environments

---

## Testing Strategy

### Automated Testing (Desktop Mode)
```python
# HTTP API calls to simulate VR actions
POST /simulate_vr_input
{
    "left_controller": {"x": 10, "y": 20, "z": 30},
    "right_controller": {"x": 10, "y": -20, "z": 30},
    "left_grip": true,
    "right_trigger": true
}
```

### Manual VR Testing (Real Headset)
```bash
# Remove -nohmd flag, VR simulator auto-disables
UnrealEditor.exe Alexander.uproject -game
```

---

## Implementation Steps

1. **Create VRInputSimulator.h/.cpp** (C++ component)
2. **Add to BP_VRSpaceshipPlayer** blueprint
3. **Modify FlightController** to use simulated input when active
4. **Add HTTP API endpoints** for simulating VR input remotely
5. **Update launch scripts** to use `-nohmd -SimulateVR`
6. **Test multi-client** with 3 simulated VR instances

---

## Alternative: Mock VR Plugin

If Unreal's VR system is too tightly coupled, create a mock VR plugin:

**MockVRPlugin** - Pretends to be a VR headset but uses keyboard input
- Implements IXRTrackingSystem interface
- Returns fake tracking data based on keyboard state
- Unreal Engine thinks real VR is connected

This is more complex but provides perfect compatibility.

---

## Decision Point

**Quick Solution** (Recommended for now):
- Use `-nohmd` flag
- Add VRInputSimulator component
- Map keyboard to controller positions
- Good enough for multi-client consensus testing

**Complete Solution** (If VR interactions are critical):
- Implement MockVRPlugin
- Full VR system emulation
- Perfect compatibility with all VR features

---

## Next Actions

1. Should we implement VRInputSimulator for basic testing?
2. Or do you need full VR plugin emulation?
3. Or can we test physics consensus with non-VR ships first, then add VR later?

For **physics consensus testing**, we don't actually need VR at all - we can use `PhysicsTestShip` (non-VR) for the consensus validation, then layer VR on top later once consensus works.

**Recommendation**:
- Phase 1: Test consensus with non-VR PhysicsTestShip (simple, fast)
- Phase 2: Add VRInputSimulator for VR feature testing
- Phase 3: Real VR testing with headset for final validation

This way we can validate the core consensus system immediately without VR complexity.
