# VR Menu Interaction Guide

## Overview

This guide covers the implementation and usage of the VR UI interaction system for the ship customization menu in Alexander. The system provides raycast-based widget interaction optimized for VR motion controllers with haptic feedback and visual pointer indicators.

## Table of Contents

1. [VR Setup](#vr-setup)
2. [Motion Controller Configuration](#motion-controller-configuration)
3. [Widget Setup](#widget-setup)
4. [Interaction System](#interaction-system)
5. [Haptic Feedback](#haptic-feedback)
6. [Performance Optimization](#performance-optimization)
7. [Common Pitfalls](#common-pitfalls)

---

## VR Setup

### Initial VR Configuration

1. **Enable VR Plugin**
   - Project Settings > Plugins > Virtual Reality
   - Enable OpenXR or your target VR platform plugin
   - Restart Unreal Editor

2. **Configure VR Project Settings**
   ```
   Project Settings > Engine > Rendering
   - Forward Shading: Enabled (recommended for VR)
   - MSAA: 4x (minimum for VR readability)
   - Instanced Stereo: Enabled

   Project Settings > Engine > VR
   - Start in VR: Enabled (for testing)
   - Target Frame Rate: 90 FPS (minimum)
   ```

3. **VR Pawn Setup**
   - Create a VR pawn with camera component at eye height
   - Add motion controller components for left and right hands
   - Attach VRUIInteractionComponent to each motion controller

### VR Camera Configuration

```cpp
// Example VR Pawn Camera Setup
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
UCameraComponent* VRCamera;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
UMotionControllerComponent* LeftController;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
UMotionControllerComponent* RightController;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
UVRUIInteractionComponent* LeftUIInteraction;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
UVRUIInteractionComponent* RightUIInteraction;
```

---

## Motion Controller Configuration

### Setting Up Motion Controllers

1. **Add Motion Controllers to Pawn**
   ```cpp
   // In constructor
   LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftController"));
   LeftController->SetupAttachment(VRCamera);
   LeftController->MotionSource = FName("Left");

   RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightController"));
   RightController->SetupAttachment(VRCamera);
   RightController->MotionSource = FName("Right");
   ```

2. **Add VR UI Interaction Components**
   ```cpp
   // Create interaction components
   LeftUIInteraction = CreateDefaultSubobject<UVRUIInteractionComponent>(TEXT("LeftUIInteraction"));
   LeftUIInteraction->SetInteractionHand(EVRInteractionHand::Left);
   LeftUIInteraction->SetMotionController(LeftController);

   RightUIInteraction = CreateDefaultSubobject<UVRUIInteractionComponent>(TEXT("RightUIInteraction"));
   RightUIInteraction->SetInteractionHand(EVRInteractionHand::Right);
   RightUIInteraction->SetMotionController(RightController);
   ```

3. **Configure Interaction Distance**
   ```cpp
   // Set maximum interaction distance (200cm = 2 meters)
   LeftUIInteraction->MaxInteractionDistance = 200.0f;
   RightUIInteraction->MaxInteractionDistance = 200.0f;
   ```

### Input Binding

Bind motion controller buttons to interaction functions:

```cpp
// In SetupPlayerInputComponent
PlayerInputComponent->BindAction("VR_TriggerLeft", IE_Pressed, this, &AYourVRPawn::LeftTriggerPressed);
PlayerInputComponent->BindAction("VR_TriggerLeft", IE_Released, this, &AYourVRPawn::LeftTriggerReleased);
PlayerInputComponent->BindAction("VR_TriggerRight", IE_Pressed, this, &AYourVRPawn::RightTriggerPressed);
PlayerInputComponent->BindAction("VR_TriggerRight", IE_Released, this, &AYourVRPawn::RightTriggerReleased);
```

```cpp
void AYourVRPawn::LeftTriggerPressed()
{
    if (LeftUIInteraction)
    {
        LeftUIInteraction->HandleButtonPress();
    }
}

void AYourVRPawn::LeftTriggerReleased()
{
    if (LeftUIInteraction)
    {
        LeftUIInteraction->HandleButtonRelease();
    }
}
```

---

## Widget Setup

### Creating VR-Compatible Widgets

1. **Use Widget Components (Not Screen Space)**
   ```cpp
   // Create widget component in world space
   UWidgetComponent* MenuWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("MenuWidget"));
   MenuWidget->SetupAttachment(RootComponent);
   MenuWidget->SetWidgetSpace(EWidgetSpace::World); // CRITICAL for VR
   MenuWidget->SetDrawSize(FVector2D(1920, 1080)); // Physical size in pixels
   MenuWidget->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
   ```

2. **Position Widget for Comfortable Viewing**
   ```cpp
   // Position 50-80cm from player face (arm's length)
   MenuWidget->SetRelativeLocation(FVector(60.0f, 0.0f, 0.0f)); // 60cm forward
   MenuWidget->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f)); // Face player

   // Scale to physical size (1 unit = 1cm in UE5)
   // For a 40cm wide menu:
   float PhysicalWidth = 40.0f; // cm
   float PixelWidth = 1920.0f; // pixels
   float Scale = PhysicalWidth / PixelWidth;
   MenuWidget->SetRelativeScale3D(FVector(Scale, Scale, Scale));
   ```

3. **Enable Widget Interaction**
   ```cpp
   // Enable hit testing
   MenuWidget->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
   MenuWidget->SetGenerateOverlapEvents(true);
   ```

### Hit Box Sizing Guidelines

**Minimum Interactive Element Size: 2cm × 2cm**

This ensures comfortable interaction in VR without precision issues.

```cpp
// Example button sizing in UMG
// For a 2cm × 2cm button at 40cm menu width (1920px):
// 2cm / 40cm = 5% of menu width
// 5% of 1920px = 96px minimum button size

// In UMG Designer:
Button Size: 96×96 pixels minimum
Padding: 8-16 pixels between elements
```

### Distance and Angle Considerations

1. **Optimal Viewing Distance**: 50-80cm from eyes
2. **Maximum Angle**: ±45° from forward (avoid extreme angles)
3. **Menu Curvature**: Consider curved menus for wide interfaces

```cpp
// Example: Create curved menu panel
void CreateCurvedMenu(float CurveRadius = 60.0f)
{
    // Position widgets along arc
    for (int i = 0; i < NumMenuPanels; i++)
    {
        float Angle = FMath::DegreesToRadians(i * 15.0f - 30.0f);
        FVector Position(
            CurveRadius * FMath::Cos(Angle),
            CurveRadius * FMath::Sin(Angle),
            0.0f
        );
        MenuPanels[i]->SetRelativeLocation(Position);
        MenuPanels[i]->SetRelativeRotation(FRotator(0.0f, -FMath::RadiansToDegrees(Angle), 0.0f));
    }
}
```

---

## Interaction System

### Raycast Interaction Flow

1. **Every Frame** (or every N frames for performance):
   - Cast ray from motion controller forward
   - Check for widget component intersection
   - Update hover state
   - Update laser pointer visual

2. **On Hover Enter**:
   - Play subtle haptic feedback (0.3 intensity, 0.1s duration)
   - Change laser pointer color to green
   - Highlight UI element (handled by widget)

3. **On Button Press**:
   - Play strong haptic feedback (0.7 intensity, 0.15s duration)
   - Change laser pointer color to yellow/orange
   - Simulate mouse press on widget

4. **On Button Release**:
   - Play medium haptic feedback (0.5 intensity, 0.1s duration)
   - Change laser pointer color back to green (if still hovering)
   - Simulate mouse release on widget

### Usage Example

```cpp
// In your VR pawn or game mode
void ACustomizationMenu::SetupVRInteraction()
{
    // Get VR UI interaction component
    UVRUIInteractionComponent* RightHand = FindComponentByClass<UVRUIInteractionComponent>();

    // Configure interaction
    RightHand->MaxInteractionDistance = 200.0f; // 2 meters
    RightHand->bEnableHapticFeedback = true;
    RightHand->bShowDebugRays = false; // Disable for release

    // Customize haptic patterns
    RightHand->HoverHapticPattern.Intensity = 0.3f;
    RightHand->PressHapticPattern.Intensity = 0.7f;

    // Customize pointer colors
    RightHand->PointerColorIdle = FLinearColor(0.3f, 0.5f, 1.0f, 0.6f); // Blue
    RightHand->PointerColorHover = FLinearColor(0.0f, 1.0f, 0.3f, 0.8f); // Green
    RightHand->PointerColorPressed = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f); // Orange
}
```

### Switching Interaction Hand

```cpp
// Allow player to switch which hand controls UI
void SwitchInteractionHand()
{
    if (CurrentInteractionHand == LeftUIInteraction)
    {
        LeftUIInteraction->HidePointer();
        RightUIInteraction->ShowPointer();
        CurrentInteractionHand = RightUIInteraction;
    }
    else
    {
        RightUIInteraction->HidePointer();
        LeftUIInteraction->ShowPointer();
        CurrentInteractionHand = LeftUIInteraction;
    }
}
```

---

## Haptic Feedback

### Haptic Feedback Patterns

The system includes three default haptic patterns:

1. **Hover Haptic** (Subtle notification)
   - Intensity: 0.3
   - Duration: 0.1s
   - Use: When cursor enters interactive element

2. **Press Haptic** (Confirmation)
   - Intensity: 0.7
   - Duration: 0.15s
   - Use: When button is pressed

3. **Release Haptic** (Acknowledgment)
   - Intensity: 0.5
   - Duration: 0.1s
   - Use: When button is released

### Custom Haptic Patterns

```cpp
// Create custom haptic pattern
FVRHapticPattern CustomPattern;
CustomPattern.Intensity = 0.6f;
CustomPattern.Duration = 0.25f;
CustomPattern.bLoop = false;

// Load custom haptic effect asset
CustomPattern.HapticEffect = LoadObject<UHapticFeedbackEffect_Base>(
    nullptr,
    TEXT("/Game/VR/Haptics/CustomButtonPress.CustomButtonPress")
);

// Apply to interaction component
RightUIInteraction->PressHapticPattern = CustomPattern;
```

### Haptic Design Best Practices

1. **Intensity Guidelines**:
   - Subtle notification: 0.2 - 0.4
   - Medium feedback: 0.4 - 0.6
   - Strong confirmation: 0.6 - 0.8
   - Warning/Error: 0.8 - 1.0

2. **Duration Guidelines**:
   - Quick tap: 0.05 - 0.1s
   - Standard feedback: 0.1 - 0.2s
   - Emphasis: 0.2 - 0.5s
   - Continuous: Use looping patterns

3. **Frequency**:
   - Don't overuse haptics
   - Minimum 100ms between haptic events
   - Reserve strong haptics for important actions

---

## Performance Optimization

### Target: 90 FPS Minimum

VR requires consistent, high frame rates. The VRUIInteractionComponent includes several optimization features.

### Frame Skipping

```cpp
// Skip raycast every N frames to save performance
RightUIInteraction->RaycastSkipFrames = 1; // Raycast every other frame
// 0 = every frame (default, best responsiveness)
// 1 = every other frame (45Hz raycast at 90 FPS)
// 2 = every third frame (30Hz raycast at 90 FPS)
```

**Recommendation**: Use 0 for primary hand, 1-2 for secondary hand

### Simplified Collision

```cpp
// Use simplified collision for raycasts
RightUIInteraction->bUseSimplifiedCollision = true;
```

### Result Caching

```cpp
// Cache widget interaction results
RightUIInteraction->bCacheInteractionResults = true;
```

### Widget Optimization

1. **Reduce Widget Complexity**:
   - Minimize number of widgets in customization menu
   - Use texture atlases for icons
   - Avoid complex materials on widgets

2. **Tick Optimization**:
   ```cpp
   // Disable tick on inactive widgets
   MenuWidget->SetComponentTickEnabled(false);

   // Enable only when menu is visible
   void ShowMenu()
   {
       MenuWidget->SetVisibility(true);
       MenuWidget->SetComponentTickEnabled(true);
   }
   ```

3. **LOD for Widget Details**:
   - Simplify widget appearance when far from player
   - Reduce texture resolution at distance

### Profiling VR Performance

```cpp
// Enable VR stats in console
stat unit          // Overall frame time
stat gpu           // GPU performance
stat scenerendering // Rendering stats
vr.PixelDensity    // Adjust for performance (default 1.0)
```

**Performance Budget** (90 FPS = 11.1ms per frame):
- Game Logic: 3ms
- Rendering: 6ms
- VR Overhead: 2ms

---

## Common Pitfalls

### 1. Widget Not Interactable

**Problem**: Raycast passes through widget

**Solution**:
```cpp
// Ensure widget is in World space
MenuWidget->SetWidgetSpace(EWidgetSpace::World);

// Enable collision
MenuWidget->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
MenuWidget->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

// Check trace channel matches
WidgetInteraction->TraceChannel = ECC_Visibility;
```

### 2. Buttons Too Small to Click

**Problem**: Player struggles to accurately click buttons

**Solution**:
- Increase button size to minimum 2cm × 2cm physical size
- Add padding between buttons (minimum 0.5cm)
- Use hover highlighting to show active element

```cpp
// Calculate physical button size
float MenuWidthCm = 40.0f;
float MenuWidthPx = 1920.0f;
float MinButtonSizeCm = 2.0f;
float MinButtonSizePx = (MinButtonSizeCm / MenuWidthCm) * MenuWidthPx; // 96px
```

### 3. Haptics Not Working

**Problem**: No haptic feedback on interaction

**Solution**:
```cpp
// Verify haptic feedback is enabled
RightUIInteraction->bEnableHapticFeedback = true;

// Check motion controller is valid
if (RightUIInteraction->GetMotionController() == nullptr)
{
    RightUIInteraction->SetMotionController(RightController);
}

// Verify controller hand is set correctly
RightUIInteraction->SetInteractionHand(EVRInteractionHand::Right);
```

### 4. Laser Pointer Not Visible

**Problem**: Laser pointer doesn't appear

**Solution**:
```cpp
// Assign Niagara system for laser
RightUIInteraction->LaserPointerEffect = LoadObject<UNiagaraSystem>(
    nullptr,
    TEXT("/Game/VR/Effects/NS_LaserPointer.NS_LaserPointer")
);

// Ensure pointer is shown
RightUIInteraction->ShowPointer();

// Check visibility
if (!RightUIInteraction->IsPointerVisible())
{
    RightUIInteraction->ShowPointer();
}
```

### 5. Text Unreadable in VR

**Problem**: Font too small or blurry in VR

**Solution**:
- Increase font size to minimum 18pt effective size
- Use high-contrast colors (light text on dark background)
- Enable MSAA 4x in project settings
- Use SDF (Signed Distance Field) fonts for clarity

```cpp
// In UMG widget styling
Font Size: 24pt minimum (for 40cm viewing distance)
Font: Use SDF font for crisp rendering
Outline: Add subtle outline for readability
Color: High contrast (white on dark gray/black)
```

### 6. Performance Drops in VR

**Problem**: Frame rate drops below 90 FPS

**Solution**:
```cpp
// Enable frame skipping
RightUIInteraction->RaycastSkipFrames = 1;

// Reduce draw calls
MenuWidget->SetComponentTickInterval(0.033f); // 30Hz update

// Simplify materials
// Use unlit materials for UI elements

// Profile and optimize
// Use "stat fps" and "stat unit" to identify bottlenecks
```

### 7. Widgets at Wrong Distance

**Problem**: Menus too close or too far

**Solution**:
- Position widgets 50-80cm from player eyes
- For ship customization: 60cm is ideal
- Test with real VR headset for comfort

```cpp
// Comfortable viewing distance
const float ComfortableDistance = 60.0f; // cm
MenuWidget->SetRelativeLocation(FVector(ComfortableDistance, 0.0f, 0.0f));
```

### 8. Input Lag

**Problem**: Noticeable delay between trigger press and UI response

**Solution**:
```cpp
// Ensure raycast happens every frame
RightUIInteraction->RaycastSkipFrames = 0;

// Disable result caching if causing lag
RightUIInteraction->bCacheInteractionResults = false;

// Check input binding is direct (not using tick)
// Bind input action directly to HandleButtonPress/Release
```

---

## Testing Checklist

### VR Interaction Testing

- [ ] Both hands can interact with UI independently
- [ ] Laser pointer is visible and tracks motion controller
- [ ] Laser pointer changes color on hover/press
- [ ] Haptic feedback triggers on hover/press/release
- [ ] All buttons are at least 2cm × 2cm
- [ ] Text is readable at arm's length (50-80cm)
- [ ] Frame rate stays at 90 FPS
- [ ] No judder or stuttering during interaction
- [ ] Widgets positioned at comfortable viewing distance
- [ ] UI responds immediately to input (no lag)

### Accessibility Testing

- [ ] Buttons large enough for users with tremors
- [ ] High contrast for users with color blindness
- [ ] Haptics can be disabled for users sensitive to vibration
- [ ] Font size adjustable for users with visual impairment
- [ ] Voice commands available as alternative (future feature)

---

## Additional Resources

### Unreal Engine VR Documentation
- [VR Development in Unreal Engine](https://docs.unrealengine.com/5.3/en-US/vr-development-in-unreal-engine/)
- [Motion Controllers in Unreal Engine](https://docs.unrealengine.com/5.3/en-US/motion-controllers-in-unreal-engine/)
- [UMG UI Designer](https://docs.unrealengine.com/5.3/en-US/umg-ui-designer-for-unreal-engine/)

### VR Design Best Practices
- [Oculus VR Best Practices](https://developer.oculus.com/resources/bp-overview/)
- [SteamVR Interaction System](https://valvesoftware.github.io/steamvr_unity_plugin/)

### Performance Optimization
- [Unreal Engine VR Performance](https://docs.unrealengine.com/5.3/en-US/vr-performance-optimization-in-unreal-engine/)
- [Mobile VR Performance Tips](https://docs.unrealengine.com/5.3/en-US/mobile-performance-guidelines-for-unreal-engine/)

---

## Support

For issues or questions:
1. Check Common Pitfalls section above
2. Review VR_WIDGET_SPECIFICATIONS.md for widget-specific guidance
3. Enable debug visualization: `RightUIInteraction->bShowDebugRays = true`
4. Check console for warnings/errors
5. Profile performance with `stat unit` and `stat fps`

---

**Last Updated**: 2025-11-07
**Engine Version**: Unreal Engine 5.6
**Author**: Claude (Subagent 3 - Ship Customization System Phase 3)
