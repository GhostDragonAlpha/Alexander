# C++ Blueprint Generator - Usage Guide

## Overview

This guide explains how to use the C++ Blueprint Generator system to programmatically create all Blueprints, materials, and level setup for the SolarSystem level.

**Why C++?** Full control, compile-time safety, version control friendly, and no manual clicking in the Editor.

---

## What Was Created

### C++ Editor Module: `AlexanderEditor`

**Location:** `Source/AlexanderEditor/`

**Files Created:**
```
Source/AlexanderEditor/
├── AlexanderEditor.Build.cs                          [Module build configuration]
├── Public/
│   ├── AlexanderEditor.h                             [Module header]
│   ├── SolarSystemBlueprintGenerator.h               [Main generator class]
│   └── SolarSystemGeneratorWidget.h                  [Editor UI widget]
└── Private/
    ├── AlexanderEditor.cpp                           [Module implementation]
    ├── SolarSystemBlueprintGenerator.cpp             [Generator implementation]
    └── SolarSystemGeneratorWidget.cpp                [Widget implementation]
```

### Key Classes

**`USolarSystemBlueprintGenerator`**
- Core Blueprint generation logic
- Creates all Blueprints programmatically
- Creates all materials programmatically
- Can setup level automatically

**`USolarSystemGeneratorWidget`**
- Editor UI for running the generator
- Provides buttons to run different generation phases
- Shows progress and error messages

---

## Step 1: Compile the Editor Module

### Prerequisites
- Visual Studio 2022 with C++ tools
- Unreal Engine 5.6
- Alexander project loaded

### Compile Steps

1. **Close Unreal Editor** (if open)

2. **Generate Visual Studio Project Files:**
   ```powershell
   cd "C:\Users\allen\Desktop\Alexander\Alexander"
   & "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -project="Alexander.uproject" -game -engine
   ```

3. **Open Solution in Visual Studio:**
   ```powershell
   start Alexander.sln
   ```

4. **Build the Project:**
   - Set Configuration to: `Development Editor`
   - Set Platform to: `Win64`
   - Right-click `Alexander` project → Build
   - Or press `Ctrl+Shift+B`

5. **Wait for Compilation:**
   - The new `AlexanderEditor` module will compile
   - You should see: "Build succeeded"

6. **Launch Unreal Editor:**
   - Open `Alexander.uproject`
   - Editor will detect the new module and load it

### Verify Module Loaded

In Unreal Editor:
1. Open Output Log (Window → Developer Tools → Output Log)
2. Look for: `LogTemp: AlexanderEditor module started successfully`

---

## Step 2: Run the Blueprint Generator

You have **three options** to run the generator:

### Option A: From C++ Code (Console Command)

1. Open Unreal Editor
2. Press `~` to open console
3. Run:
   ```
   py exec(open('run_generator.py').read())
   ```

Create `run_generator.py` in project root:
```python
import unreal

# Create generator
generator = unreal.new_object(unreal.SolarSystemBlueprintGenerator)

# Run generation
success = generator.generate_all_blueprints()

if success:
    unreal.log("✓ All Blueprints generated successfully!")
else:
    unreal.log_error("Some Blueprints failed. Check Output Log.")
```

### Option B: From Blueprint (Editor Utility Widget)

1. **Create Editor Utility Widget:**
   - Content Browser → Right-click → Editor Utilities → Editor Utility Widget
   - Parent Class: `SolarSystemGeneratorWidget`
   - Name: `EUW_SolarSystemGenerator`

2. **Design the Widget:**
   - Open `EUW_SolarSystemGenerator`
   - Add buttons:
     - "Generate All Blueprints"
     - "Generate Celestial Bodies Only"
     - "Generate Game Mode"
     - "Generate UI Widgets"
     - "Generate Materials"
     - "Setup Level"
   - Add text blocks for status messages

3. **Wire Up Buttons:**
   - "Generate All" button → On Clicked → Call `Generate All Blueprints`
   - "Generate Celestial Bodies" → Call `Generate Celestial Bodies`
   - etc.

4. **Implement Display Functions:**
   - Event Graph → Right-click → Implement Event → `Display Progress`
   - Create UI logic to show messages in text blocks

5. **Run the Widget:**
   - Content Browser → Right-click `EUW_SolarSystemGenerator` → Run Editor Utility Widget
   - Click "Generate All Blueprints" button

### Option C: From C++ Directly

Create a test function in your game code:

```cpp
// In any Actor or Component

void AMyTestActor::GenerateBlueprints()
{
    USolarSystemBlueprintGenerator* Generator = NewObject<USolarSystemBlueprintGenerator>();

    bool bSuccess = Generator->GenerateAllBlueprints();

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Generation successful!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ Generation failed!"));
    }
}
```

---

## Step 3: What Gets Generated

### Blueprints Created

**Celestial Bodies:**
```
/Game/Blueprints/CelestialBodies/
├── BP_CelestialBodyBase     [Parent class]
├── BP_Sun                    [Child: Sun with PointLight]
├── BP_Earth                  [Child: Earth]
├── BP_Moon                   [Child: Moon]
└── BP_Mars                   [Child: Mars]
```

**Game Mode:**
```
/Game/Blueprints/GameModes/
└── BP_SolarSystemGameMode    [Game mode with subsystem init]
```

**UI Widgets:**
```
/Game/UI/Widgets/
├── WBP_Chronometer           [Time display widget]
├── WBP_DebugOverlay          [Debug info widget]
└── WBP_TimeWarpWarning       [Time warp modal dialog]
```

### Materials Created

```
/Game/Materials/
├── M_Sun_Emissive            [Yellow emissive material]
├── M_Earth_Simple            [Blue lit material]
├── M_Moon_Simple             [Gray rough material]
└── M_Mars_Simple             [Red-orange material]
```

---

## Step 4: Verify Generated Assets

1. **Check Content Browser:**
   - Navigate to `/Game/Blueprints/CelestialBodies/`
   - You should see all 5 celestial body Blueprints

2. **Open BP_CelestialBodyBase:**
   - Should have 3 components:
     - CelestialBodyComponent
     - VisualMesh
     - GravityZone

3. **Check Compilation:**
   - All Blueprints should compile without errors
   - Green checkmark in Blueprint Editor

4. **Check Materials:**
   - Navigate to `/Game/Materials/`
   - Open each material
   - Verify node graphs are connected correctly

---

## Step 5: Next Steps (Manual Configuration)

The C++ generator creates the **structure** of all Blueprints. You still need to configure some details manually:

### BP_Sun (Manual Steps)
1. Open `BP_Sun`
2. Find PointLight component (already added by generator)
3. Set properties:
   - Intensity: `1000000.0`
   - Attenuation Radius: `100000000.0`
   - Light Color: `(255, 247, 230)`
4. Set VisualMesh material to `M_Sun_Emissive`
5. Compile and save

### BP_Earth (Manual Steps)
1. Open `BP_Earth`
2. Set VisualMesh material to `M_Earth_Simple`
3. Compile and save

### BP_Moon (Manual Steps)
1. Open `BP_Moon`
2. Set VisualMesh material to `M_Moon_Simple`
3. Compile and save

### BP_SolarSystemGameMode (Manual Steps)
1. Open `BP_SolarSystemGameMode`
2. Set Default Pawn Class: `BP_VRSpaceshipPlayer`
3. Add BeginPlay event graph logic (see specification docs)
4. Compile and save

### UI Widgets (Manual Steps)
1. Design widget layouts (text blocks, progress bars, etc.)
2. Add binding logic to RelativitySystem
3. Compile and save

**Note:** These manual steps are still required because Blueprint graph manipulation (adding nodes, connecting pins) is extremely complex in C++ and error-prone. The generator creates the base structure, and you complete the logic visually.

---

## Troubleshooting

### Module Won't Compile

**Error:** `Cannot find module 'AlexanderEditor'`

**Solution:**
1. Verify `AlexanderEditor.Build.cs` exists
2. Check `AlexanderEditor.Target.cs` includes `"AlexanderEditor"` in ExtraModuleNames
3. Regenerate project files
4. Clean and rebuild

### Blueprints Not Created

**Error:** `Failed to create Blueprint`

**Solution:**
1. Check Output Log for specific errors
2. Verify parent classes exist (for child Blueprints)
3. Ensure folders exist in Content Browser
4. Check write permissions

### Materials Appear Black

**Error:** Materials don't render correctly

**Solution:**
1. Open material in Material Editor
2. Verify nodes are connected to output pins
3. Click "Apply" button
4. Assign material to mesh manually if needed

### Compilation Errors in Generated Blueprints

**Error:** Red X in Blueprint Editor

**Solution:**
1. This is expected for initial generation
2. Complete manual configuration steps above
3. Add missing event graph logic
4. Compile again

---

## Advanced: Extending the Generator

### Add More Blueprints

Edit `SolarSystemBlueprintGenerator.cpp`:

```cpp
UBlueprint* USolarSystemBlueprintGenerator::CreateBP_Jupiter()
{
    // Load parent blueprint
    FString BaseClassPath = FString::Printf(TEXT("%s/BP_CelestialBodyBase"), *BaseBlueprintPath);
    UBlueprint* ParentBlueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BaseClassPath));

    if (!ParentBlueprint)
    {
        LogError(TEXT("Could not load BP_CelestialBodyBase for BP_Jupiter"));
        return nullptr;
    }

    // Create child
    UBlueprint* Blueprint = CreateBlueprintAsset(
        TEXT("BP_Jupiter"),
        BaseBlueprintPath,
        ParentBlueprint->GeneratedClass
    );

    // Compile
    CompileBlueprint(Blueprint);

    return Blueprint;
}
```

Then call it from `GenerateCelestialBodyBlueprints()`.

### Add Blueprint Graph Logic

This is complex but possible:

```cpp
// Example: Add BeginPlay event
UEdGraph* EventGraph = FBlueprintEditorUtils::FindEventGraph(Blueprint);
if (EventGraph)
{
    // Create BeginPlay node
    UK2Node_Event* BeginPlayNode = NewObject<UK2Node_Event>(EventGraph);
    BeginPlayNode->EventReference.SetExternalMember(TEXT("ReceiveBeginPlay"), AActor::StaticClass());
    EventGraph->AddNode(BeginPlayNode, false, false);

    // Add more nodes and connect them...
    // This gets very complex quickly!
}
```

---

## Summary

### What C++ Generator Does:
✅ Creates all Blueprint files automatically
✅ Creates all Material files automatically
✅ Sets up component hierarchies
✅ Configures material node graphs
✅ Compiles all assets
✅ Saves everything to Content Browser

### What You Still Do Manually:
⏳ Set Blueprint property defaults
⏳ Assign materials to meshes
⏳ Design UI widget layouts
⏳ Add Blueprint event graph logic
⏳ Place actors in level
⏳ Configure lighting

### Estimated Time Saved:
- **Manual implementation:** 3-4 hours
- **With C++ generator:** 1-2 hours (generation + manual configuration)
- **Time saved:** ~2 hours, plus guaranteed consistency

---

## Files Reference

### Documentation
- [Quick_Start_Implementation_Guide.md](Quick_Start_Implementation_Guide.md) - Manual implementation guide
- [Quick_Reference_Cards.md](Quick_Reference_Cards.md) - Property values and settings
- [Material_Setup_Guide.md](Material_Setup_Guide.md) - Material creation details

### C++ Source
- `Source/AlexanderEditor/Public/SolarSystemBlueprintGenerator.h` - Main generator class
- `Source/AlexanderEditor/Private/SolarSystemBlueprintGenerator.cpp` - Implementation
- `Source/AlexanderEditor/Public/SolarSystemGeneratorWidget.h` - Editor UI widget
- `Source/AlexanderEditor/Private/SolarSystemGeneratorWidget.cpp` - Widget implementation

### Build Configuration
- `Source/AlexanderEditor/AlexanderEditor.Build.cs` - Module dependencies
- `Source/AlexanderEditor.Target.cs` - Editor target configuration

---

**Created:** 2025-01-05
**Version:** 1.0
**Status:** Ready to compile and run
**Method:** C++ for full control and automation
