# IMMEDIATE ACTION PLAN - What to Build Right Now

## 🎯 PRIORITY 1: VR Spaceship Cockpit (Week 1-2)

### Why Start Here?
- Most impressive for demos/testing
- Validates VR comfort and controls
- Foundation for all other systems
- Fun to iterate on immediately

### Step-by-Step Build Guide

#### **Day 1-2: Basic Cockpit Setup**

**In Unreal Editor:**

1. **Create New Map**
   - File → New Level → Empty Level
   - Save as: `Content/SpaceGame/Maps/SpaceTestLevel`

2. **Import/Create Cockpit Mesh**
   - Option A: Use free asset from Quixel/Marketplace
   - Option B: Create basic geometry with BSP brushes
   - Place at 0,0,0 in world

3. **Create Player Ship Blueprint**
   ```
   Content Browser → Right Click → Blueprint Class → Pawn
   Name: BP_PlayerShip
   Location: Content/SpaceGame/Blueprints/SpaceShip/
   ```

4. **Add Components to BP_PlayerShip**
   - Static Mesh (ship exterior)
   - Camera Component (VR camera inside cockpit)
   - FloatingPawnMovement component (for physics)
   - Scene Capture 2D components (for cockpit screens)

#### **Day 3-4: VR Hand Interactions**

5. **Create Interactive Controls**
   ```
   Create child blueprints from BP_GrabComponent (from VR Template)
   
   Blueprints to create:
   - BP_VR_Joystick (flight stick)
   - BP_VR_ThrottleHandle (throttle control)
   - BP_VR_Button (clickable buttons)
   - BP_VR_Switch (toggle switches)
   - BP_VR_Lever (landing gear, etc.)
   ```

6. **Grab System Logic** (Blueprint nodes)
   ```
   On Grab (from motion controller):
   1. Attach component to motion controller
   2. Disable physics on grabbed object
   3. Set "IsGrabbed" = True
   
   On Release:
   1. Detach from motion controller
   2. Enable physics
   3. Set "IsGrabbed" = False
   ```

#### **Day 5-7: Flight Physics**

7. **Implement 6DOF Movement** (Star Citizen style)
   ```
   In BP_PlayerShip Event Graph:
   
   Every Tick:
   1. Get Joystick Position (X, Y)
   2. Calculate Pitch/Yaw from joystick
   3. Get Throttle Position (0-1)
   4. Calculate forward velocity
   5. Add rotation: AddActorLocalRotation(Pitch, Yaw, Roll)
   6. Add movement: AddActorLocalOffset(Forward * Throttle)
   7. Apply drag/friction for stability
   ```

8. **Newtonian Physics Toggle**
   ```
   IFCS Mode (Coupled - Star Citizen default):
   - Auto-stabilization when joystick centered
   - Velocity matching to throttle setting
   
   Decoupled Mode:
   - No auto-stabilization
   - Maintain velocity after thrust
   - Manual counter-thrust needed
   ```

9. **Flight Assist System**
   ```
   Variables:
   - Target Velocity (Vector)
   - Current Velocity (Vector)
   - Max Speed (Float): 1000 units/sec
   - Acceleration (Float): 500 units/sec²
   - Rotation Speed (Float): 90 degrees/sec
   
   Logic:
   If (Flight Assist ON):
       Lerp current velocity toward target velocity
       Lerp rotation toward input rotation
   Else:
       Apply raw physics forces
   ```

#### **Day 8-10: HUD & Feedback**

10. **Create Holographic HUD**
    ```
    Create Widget Blueprint: WB_CockpitHUD
    Add to viewport or attach to world space
    
    Display:
    - Speed (m/s)
    - Altitude (when near planet)
    - Heading
    - Target reticle
    - System status (fuel, power, shields)
    ```

11. **Audio Feedback**
    ```
    Add sound cues:
    - Engine hum (loops, pitch varies with throttle)
    - Button clicks
    - Thrust bursts
    - Warning alarms
    ```

12. **Haptic Feedback**
    ```
    On joystick movement: Small rumble
    On collision: Strong rumble
    On button press: Quick pulse
    ```

---

## 📝 BLUEPRINT PSEUDOCODE

### BP_PlayerShip (Main Ship Actor)

```blueprint
// Variables
- Ship Velocity (Vector)
- Ship Angular Velocity (Rotator)
- Max Speed (Float) = 1000
- Acceleration Rate (Float) = 500
- Flight Assist Enabled (Bool) = True
- Is In Atmosphere (Bool) = False

// Components
- Ship Mesh (Static Mesh)
- VR Camera (Camera Component)
- Collision Sphere (Sphere Collision)
- Joystick Reference (BP_VR_Joystick)
- Throttle Reference (BP_VR_ThrottleHandle)

// Functions
Function: CalculateShipMovement()
    Input Pitch = Joystick.GetPitchInput() // -1 to 1
    Input Yaw = Joystick.GetYawInput() // -1 to 1
    Input Roll = Get gamepad axis or button
    Throttle Value = Throttle.GetPosition() // 0 to 1
    
    If Flight Assist Enabled:
        Target Velocity = Forward Vector * Throttle Value * Max Speed
        Ship Velocity = Lerp(Ship Velocity, Target Velocity, Delta Time * 2.0)
    Else:
        Acceleration = Forward Vector * Throttle Value * Acceleration Rate
        Ship Velocity += Acceleration * Delta Time
    
    Apply Rotation:
        New Rotation = Make Rotator(Input Pitch, Input Yaw, Input Roll) * Rotation Speed * Delta Time
        Add Actor Local Rotation(New Rotation)
    
    Apply Movement:
        Add Actor Local Offset(Ship Velocity * Delta Time)
    
    If Is In Atmosphere:
        Apply Drag(Ship Velocity * 0.98)

Function: HandleCollision()
    On Hit:
        Play Impact Sound
        Play Haptic Feedback
        Reduce Ship Health
        If (Health <= 0): Explode Ship

// Events
Event BeginPlay:
    Find and store references to Joystick and Throttle
    Enable VR mode
    Set up camera for VR

Event Tick:
    CalculateShipMovement()
    Update HUD
    Check for atmosphere proximity
```

### BP_VR_Joystick (Interactive Joystick)

```blueprint
// Variables
- Rest Position (Vector)
- Max Deflection (Float) = 10.0 cm
- Current Position (Vector)
- Is Grabbed (Bool) = False
- Smooth Return Speed (Float) = 5.0

// Functions
Function: GetPitchInput()
    Offset = Current Position - Rest Position
    Pitch = Offset.Y / Max Deflection
    Return Clamp(Pitch, -1.0, 1.0)

Function: GetYawInput()
    Offset = Current Position - Rest Position
    Yaw = Offset.X / Max Deflection
    Return Clamp(Yaw, -1.0, 1.0)

// Events
Event OnGrab(Motion Controller):
    Is Grabbed = True
    Play Haptic Feedback
    Attach to Motion Controller

Event OnRelease():
    Is Grabbed = False
    Detach from Motion Controller

Event Tick:
    If NOT Is Grabbed:
        Current Position = Lerp(Current Position, Rest Position, Delta Time * Smooth Return Speed)
    Else:
        Constrain position to sphere around Rest Position (Max Deflection radius)
```

### BP_VR_ThrottleHandle (Throttle Lever)

```blueprint
// Variables
- Min Position (Float) = 0.0
- Max Position (Float) = 30.0 cm
- Current Position (Float) = 0.0
- Is Grabbed (Bool) = False

// Functions
Function: GetPosition()
    Return Current Position / Max Position // Returns 0.0 to 1.0

// Events
Event OnGrab(Motion Controller):
    Is Grabbed = True
    Attach to Motion Controller
    Constrain to local Y axis only

Event OnRelease():
    Is Grabbed = False
    Detach from Motion Controller

Event Tick:
    If Is Grabbed:
        Clamp position between Min Position and Max Position
        Update visual position
```

---

## 🎨 VISUAL SETUP IN UNREAL

### Cockpit Scene Setup
```
Hierarchy:
└── BP_PlayerShip (Pawn)
    ├── ShipExterior (Static Mesh) - Collision enabled
    ├── CockpitInterior (Static Mesh) - No collision
    ├── VRCamera (Camera Component)
    │   └── Position: Center of cockpit, player seated position
    ├── MotionController_Left (Motion Controller Component)
    │   └── Hand Mesh (Skeletal Mesh)
    ├── MotionController_Right (Motion Controller Component)
    │   └── Hand Mesh (Skeletal Mesh)
    ├── JoystickRoot (Scene Component)
    │   └── BP_VR_Joystick (Child Actor Component)
    ├── ThrottleRoot (Scene Component)
    │   └── BP_VR_ThrottleHandle (Child Actor Component)
    └── ControlPanels (Scene Component)
        ├── BP_VR_Button x10
        └── BP_VR_Switch x5
```

---

## 🧪 TESTING CHECKLIST

**After implementing above:**

- [ ] Can put on VR headset and see cockpit
- [ ] Can see VR hands in correct position
- [ ] Can grab joystick and move it
- [ ] Ship pitches/yaws when joystick moved
- [ ] Can grab throttle and slide it
- [ ] Ship accelerates with throttle forward
- [ ] Ship has proper inertia/momentum
- [ ] HUD displays current speed
- [ ] Audio plays when moving controls
- [ ] No VR motion sickness (stable frame rate)

---

## 🚀 WHAT THIS ACHIEVES

After Week 1-2, you'll have:
- ✅ Fully functional VR cockpit
- ✅ Realistic space flight
- ✅ Foundation for all ship systems
- ✅ Something impressive to show/test
- ✅ Core tech for the entire game

**This is your MVP - everything else builds on this!**

---

## NEXT: Week 3-4 Options

Once cockpit works, choose one:

**A) Add planetary landing** → Transition from space to ground  
**B) Add farming mechanics** → Exit ship, plant crops  
**C) Add ship combat** → Weapons, shields, damage  
**D) Add AI NPC** → First LLM-powered character

Which direction excites you most?

---

**Ready to start building in Unreal Engine? Open the editor and let's go!** 🚀
