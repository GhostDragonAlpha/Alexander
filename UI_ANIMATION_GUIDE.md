# UI Animation Guide

## Overview

This document provides comprehensive specifications for all UI animations in the ship customization system, including timing, easing functions, performance considerations, and VR-specific requirements.

---

## Animation Principles

### Core Principles

1. **Purposeful Motion:** Every animation should serve a purpose (feedback, guidance, or delight)
2. **Consistency:** Similar interactions should have similar animations
3. **Performance First:** Smooth 60fps (90fps in VR) is mandatory
4. **Accessibility:** Respect reduced motion preferences
5. **Physicality:** Animations should feel natural and obey physics

### Timing Guidelines

- **Instant:** <50ms - Perceived as immediate
- **Fast:** 50-150ms - Quick feedback
- **Normal:** 150-300ms - Standard transitions
- **Slow:** 300-500ms - Important state changes
- **Dramatic:** 500-2000ms - Celebratory moments

### Easing Functions

**Standard Easings (UE5 Curve Assets):**

```
EaseLinear: Linear interpolation (rarely used)
EaseInQuad: Slow start, accelerates (t²)
EaseOutQuad: Fast start, decelerates (1 - (1-t)²)
EaseInOutQuad: Slow start and end, fast middle
EaseInCubic: Slower start than quad (t³)
EaseOutCubic: Smoother deceleration (1 - (1-t)³)
EaseInOutCubic: Balanced cubic curve
EaseOutBounce: Bounces at end (spring effect)
EaseOutElastic: Overshoots and oscillates back
EaseOutBack: Slight overshoot, settles (scale = 1.1)
```

**Custom Curves:**
- Create curve assets in Content/UI/Curves/
- Use tangent handles for fine control
- Test curves at different frame rates

---

## Animation Catalog

### 1. Menu Transitions

#### Menu Open (300ms)

**Use Cases:** Opening any menu panel, sub-menu, or modal

**Timeline:**
```
0ms   - Initial state (hidden)
        Scale: 0.95
        Opacity: 0.0
        Position: Center

100ms - Fade in begins
        Opacity: 0.0 → 1.0 (ease-out-cubic)

0-300ms - Scale animation
        Scale: 0.95 → 1.05 → 1.0 (ease-out-back)

300ms - Final state (visible)
        Scale: 1.0
        Opacity: 1.0
```

**UMG Animation Track Setup:**
```
Track 1: Opacity
  - Keyframe 0.0s: Alpha = 0.0
  - Keyframe 0.1s: Alpha = 1.0
  - Curve: EaseOutCubic

Track 2: Render Transform - Scale
  - Keyframe 0.0s: Scale = 0.95
  - Keyframe 0.15s: Scale = 1.05
  - Keyframe 0.3s: Scale = 1.0
  - Curve: EaseOutBack (overshoot: 1.1)

Track 3: Render Transform - Translation (optional)
  - Keyframe 0.0s: Y = 20px
  - Keyframe 0.3s: Y = 0px
  - Curve: EaseOutCubic
```

**VR Adjustments:**
- Remove scale animation (use fade only)
- Increase duration to 400ms
- Add subtle Z-translation: -10cm → 0cm

#### Menu Close (200ms)

**Use Cases:** Closing menus, dismissing dialogs

**Timeline:**
```
0ms   - Initial state (visible)
        Scale: 1.0
        Opacity: 1.0

0-200ms - Fade and scale out
        Opacity: 1.0 → 0.0 (ease-in-cubic)
        Scale: 1.0 → 0.95 (ease-in-cubic)

200ms - Final state (hidden)
        Remove from viewport
```

**UMG Animation Track Setup:**
```
Track 1: Opacity
  - Keyframe 0.0s: Alpha = 1.0
  - Keyframe 0.2s: Alpha = 0.0
  - Curve: EaseInCubic

Track 2: Render Transform - Scale
  - Keyframe 0.0s: Scale = 1.0
  - Keyframe 0.2s: Scale = 0.95
  - Curve: EaseInCubic
```

### 2. Tab Switching (200ms)

**Use Cases:** Switching between customization categories (Hull, Weapons, Engines)

**Timeline:**
```
// Previous Tab (Out)
0ms   - Begin slide out
        Position: X = 0
        Opacity: 1.0

0-200ms - Slide left and fade
        Position: X = 0 → -100px (ease-in-cubic)
        Opacity: 1.0 → 0.0 (ease-in-cubic)

// New Tab (In)
0ms   - Initial state (hidden right)
        Position: X = 100px
        Opacity: 0.0

0-200ms - Slide in and fade
        Position: X = 100px → 0 (ease-out-cubic)
        Opacity: 0.0 → 1.0 (ease-out-cubic)
```

**Implementation:**
```cpp
void UCustomizationMenu::SwitchTab(int32 NewTabIndex)
{
    // Animate current tab out
    if (CurrentTab)
    {
        PlayAnimation(TabSlideOut, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
    }

    // Animate new tab in (delayed by 50ms for overlap)
    FTimerHandle DelayHandle;
    GetWorld()->GetTimerManager().SetTimer(DelayHandle, [this, NewTabIndex]()
    {
        CurrentTab = TabWidgets[NewTabIndex];
        PlayAnimation(TabSlideIn, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
    }, 0.05f, false);
}
```

**VR Adjustments:**
- Use cross-fade instead of slide (no lateral motion)
- Duration: 250ms

### 3. Part Selection (100ms)

**Use Cases:** Highlighting selected part in grid, hover feedback

**Timeline:**
```
// Hover Enter
0ms   - Default state
        Border: 2px, #424242
        Scale: 1.0

0-100ms - Highlight animation
        Border: 2px → 3px, #1E88E5 (ease-out-quad)
        Scale: 1.0 → 1.05 (ease-out-quad)
        Glow: 0 → 8px blur, #1E88E5

// Hover Exit
0ms   - Highlighted state
        Border: 3px, #1E88E5
        Scale: 1.05

0-100ms - Dehighlight animation
        Border: 3px → 2px, #424242 (ease-in-quad)
        Scale: 1.05 → 1.0 (ease-in-quad)
        Glow: 8px → 0

// Click/Selection
0ms   - Click registered
        Scale: 1.05

0-50ms - Press down
        Scale: 1.05 → 0.98 (ease-in-quad)

50-200ms - Release and highlight
        Scale: 0.98 → 1.0 (ease-out-bounce)
        Border: #1E88E5 → #43A047 (ease-out-quad)
        Glow: Pulse effect (2 cycles, 1000ms)
```

**Material Animation:**
```
// Use material instance parameter animation for glow
FLinearColor GlowColor = FLinearColor(0.117f, 0.533f, 0.898f, 0.0f); // #1E88E5
float GlowIntensity = 0.0f;

// Animate using timeline
Timeline->AddInterpFloat(GlowCurve, [this](float Value)
{
    GlowIntensity = Value;
    MaterialInstance->SetScalarParameterValue(TEXT("GlowIntensity"), GlowIntensity);
});
```

### 4. Button Interactions

#### Standard Button (150ms total)

**Timeline:**
```
// Hover
0ms   - Normal state
        Background: #424242
        Scale: 1.0

0-100ms - Hover animation
        Background: #424242 → #616161 (ease-out-quad)
        Scale: 1.0 → 1.05 (ease-out-quad)
        Glow: 0 → 4px blur

// Press
0ms   - Hovered state
        Scale: 1.05

0-50ms - Press down
        Scale: 1.05 → 0.95 (ease-in-quad)
        Background: #616161 → #212121

// Release
0ms   - Pressed state
        Scale: 0.95

0-150ms - Return to hover
        Scale: 0.95 → 1.05 (ease-out-bounce)
        Background: #212121 → #616161
```

#### Primary Action Button (200ms)

**Additional Effects:**
- Pulsing glow when enabled (1500ms cycle)
- Particle shimmer on hover
- Success flash on click (#43A047, 300ms)

#### Disabled Button

**State:**
- No animations
- Opacity: 0.5
- Cursor: Not allowed
- Optional tooltip on hover explaining why disabled

### 5. Stat Value Changes (500ms)

**Use Cases:** Updating ship stats, displaying new values

**Count-Up Animation:**
```cpp
void UStatDisplay::AnimateValueChange(int32 OldValue, int32 NewValue)
{
    float Duration = 0.5f;
    float ElapsedTime = 0.0f;

    // Use FMath::InterpEaseOut for smooth deceleration
    while (ElapsedTime < Duration)
    {
        float Alpha = ElapsedTime / Duration;
        float EasedAlpha = FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 2.0f); // Quadratic

        int32 CurrentValue = FMath::Lerp(OldValue, NewValue, EasedAlpha);
        UpdateDisplayText(CurrentValue);

        ElapsedTime += DeltaTime;
    }

    // Ensure final value is exact
    UpdateDisplayText(NewValue);
}
```

**Timeline:**
```
0ms   - Old value displayed
        Text: "350"
        Color: #FFFFFF

0-500ms - Count-up animation
        Text: Increments from 350 → 425
        Easing: EaseOutQuad

100ms - Color flash
        Color: #FFFFFF → #43A047 (if increase) or #E53935 (if decrease)

100-400ms - Color fade back
        Color: #43A047 → #FFFFFF

300ms - Scale pulse
        Scale: 1.0 → 1.15 → 1.0 (ease-out-back)

500ms - Final state
        Text: "425"
        Color: #FFFFFF
        Scale: 1.0
```

**VR Optimization:**
- Reduce duration to 300ms
- Skip color flash (too distracting)
- Keep scale pulse

### 6. XP Bar Fill (1000ms)

**Use Cases:** Filling XP progress bar on XP gain

**Timeline:**
```
0ms   - Current progress
        Fill: 35%
        Glow: None

0-1000ms - Fill animation
        Fill: 35% → 72% (ease-out-cubic)
        Glow: Travels along fill edge (speed: 200px/s)

0-1000ms - Background pulse
        Background brightness: 0% → 20% → 0% (sine wave)

// If bar fills to 100%, trigger level-up
1000ms - Check if full
        If Fill == 100%:
          - Trigger level-up animation
          - Reset bar to overflow amount
```

**Glow Effect Implementation:**
```cpp
// Material: M_ProgressBarFill
// Parameters:
//   - FillPercent (0.0 - 1.0)
//   - GlowPosition (0.0 - 1.0)
//   - GlowIntensity (0.0 - 2.0)

void UXPProgressBar::AnimateFill(float TargetPercent)
{
    FTimeline FillTimeline;
    FillTimeline.SetPlayRate(1.0f / FillDuration); // 1.0 seconds

    FOnTimelineFloat FillProgressUpdate;
    FillProgressUpdate.BindDynamic(this, &UXPProgressBar::UpdateFillProgress);
    FillTimeline.AddInterpFloat(FillCurve, FillProgressUpdate);

    FillTimeline.PlayFromStart();
}

void UXPProgressBar::UpdateFillProgress(float Alpha)
{
    float CurrentFill = FMath::Lerp(StartPercent, TargetPercent, Alpha);
    ProgressBarMaterial->SetScalarParameterValue(TEXT("FillPercent"), CurrentFill);

    // Glow follows fill edge
    float GlowPos = CurrentFill;
    ProgressBarMaterial->SetScalarParameterValue(TEXT("GlowPosition"), GlowPos);
    ProgressBarMaterial->SetScalarParameterValue(TEXT("GlowIntensity"), 1.5f);
}
```

### 7. Level-Up Animation (2000ms)

**Use Cases:** Player levels up

**Timeline:**
```
// Phase 1: Recognition (0-500ms)
0ms   - XP bar reaches 100%
        Bar fill: 100%

0-200ms - Bar flash
        Bar color: #43A047 → #FFFFFF → #43A047

200ms - Sound effect
        Play: "level_up_chime.wav"

// Phase 2: Visual Burst (500-1200ms)
500ms - Particle burst
        Emitter: Gold sparkles, radial burst
        Count: 50 particles
        Velocity: 200-400 units/s
        Lifetime: 1.5s

500-1100ms - Level indicator scale
        Scale: 1.0 → 1.5 → 1.0 (ease-out-elastic)

500-1100ms - Level indicator glow
        Glow: 0 → 16px → 8px blur, #FDD835

600-1100ms - Level number count-up
        Text: "12" → "13" (instant transition at 600ms)
        Additional: Number "spin" effect

// Phase 3: Celebration (1200-2000ms)
1200ms - Notification toast appears
        WBP_NotificationToast: "LEVEL UP! Now Level 13"

1200-1700ms - XP bar reset
        Fill: 100% → 0% (fade out, 200ms)
        Fill: 0% → overflow% (fade in, 300ms)
        Example: 2150 / 2000 XP → 150 / 2200 XP

1700-2000ms - Glow fade out
        Glow: 8px → 0px

2000ms - Complete
        Return to normal state
```

**VR Enhancements:**
- Add haptic feedback pulse (500ms, medium intensity)
- Spatial audio for level-up chime
- Particle effect at 1.5m distance, facing player

### 8. Unlock Toast Notification (4400ms total)

**Timeline:**
```
// Slide In (0-400ms)
0ms   - Initial state
        Position: Y = -100px (above screen)
        Opacity: 0.0
        Scale: 0.95

0-400ms - Slide down
        Position: Y = -100px → 20px (ease-out-cubic)
        Opacity: 0.0 → 1.0
        Scale: 0.95 → 1.0

200-700ms - Icon entrance (starts at 200ms)
        Icon scale: 0.0 → 1.2 → 1.0 (ease-out-bounce)
        Icon rotation: -15° → 0°

// Display (400-4000ms)
400ms - Fully visible
3600ms - Begin exit preparation

// Slide Out (4000-4400ms)
4000-4400ms - Slide up
        Position: Y = 20px → -100px (ease-in-cubic)
        Opacity: 1.0 → 0.0
        Scale: 1.0 → 0.95

4400ms - Remove from viewport
```

**Stacking Behavior:**
```cpp
// When multiple toasts appear, stack vertically
void UToastManager::ShowToast(UWBPNotificationToast* NewToast)
{
    // Calculate Y offset based on existing toasts
    float YOffset = 20.0f; // Base offset from top
    for (auto ExistingToast : ActiveToasts)
    {
        YOffset += ExistingToast->GetDesiredSize().Y + 12.0f; // 12px gap
    }

    NewToast->SetTargetYPosition(YOffset);
    ActiveToasts.Add(NewToast);

    // When toast is removed, shift others up
    NewToast->OnToastDismissed.AddDynamic(this, &UToastManager::ShiftToastsUp);
}
```

### 9. Unlock Dialog Animations

#### Dialog Open (300ms)

```
// Modal Overlay
0-200ms - Fade in
        Opacity: 0.0 → 0.8 (ease-out-quad)
        Blur: 0px → 4px (if enabled)

// Dialog Container
0-300ms - Scale in
        Scale: 0.8 → 1.05 → 1.0 (ease-out-back)
        Opacity: 0.0 → 1.0

// Content Stagger (150-350ms)
150ms - Part icon appears
        Opacity: 0.0 → 1.0 (ease-out-quad)
        Scale: 0.0 → 1.2 → 1.0 (ease-out-bounce)

200ms - Part name appears
        Opacity: 0.0 → 1.0
        Position: Y = -10px → 0px

250ms - Cost section appears
        Opacity: 0.0 → 1.0
        Position: Y = -10px → 0px

300ms - Buttons appear
        Opacity: 0.0 → 1.0
        Position: Y = -10px → 0px
```

#### Success Animation (1500ms)

```
0ms   - User clicks Confirm
        Button state: Hovered

0-300ms - Confirm button success flash
        Background: #1E88E5 → #43A047 (ease-out-quad)
        Glow: Intensifies to 16px

300ms - Checkmark icon appears on button
        Icon scale: 0.0 → 1.2 → 1.0 (ease-out-bounce)
        Icon color: #FFFFFF

300-800ms - Part icon celebration
        Scale: 1.0 → 1.3 → 1.0 (ease-out-elastic)
        Glow: 0 → 20px → 10px, rarity color
        Rotation: 0° → 360° (one full spin)

500-1000ms - Particle burst
        Gold sparkles from part icon
        Count: 30 particles
        Spread: 360°

1000ms - Sound effect
        Play: "unlock_success.wav"

1200-1500ms - Dialog fade out
        Opacity: 1.0 → 0.0 (ease-in-quad)
        Scale: 1.0 → 0.9

1500ms - Close dialog and update UI
```

#### Insufficient Funds Warning (Looping)

```
// Plays when user tries to unlock without enough credits
0ms   - Warning section visible
        Background: #E53935, 20% opacity

0-500ms - Pulse animation
        Background opacity: 20% → 40% → 20% (sine wave)

0-200ms - Balance text shake
        Position X: 0 → -5px → 5px → -3px → 3px → 0
        Easing: Linear (for shake effect)

200ms - Sound effect
        Play: "error_beep.wav"

// Loop continues until user closes dialog
```

---

## Advanced Animation Techniques

### 1. Material-Based Animations

**Progress Bar Fill with Gradient:**
```cpp
// Material: M_ProgressBarFill
// Use material parameters for GPU-accelerated animation

Material Graph:
  - TexCoord → Multiply by FillPercent parameter
  - Compare against UV.X
  - If UV.X <= FillPercent: Show fill gradient
  - Gradient: Lerp between PrimaryColor and SecondaryColor
  - Add glow at fill edge using smoothstep
```

**Glow Effect:**
```cpp
// Material: M_GlowBorder
// Animated glow using emissive channel

Material Graph:
  - Border mask (distance field from edge)
  - Multiply by GlowIntensity parameter
  - Add pulsing using Time node + Sine
  - Feed into Emissive Color
  - Base Color uses GlowColor parameter
```

### 2. Particle Systems for UI

**Level-Up Burst:**
```
Particle System: PS_LevelUpBurst
Emitter:
  - Spawn: Burst, 50 particles
  - Lifetime: 1.0 - 1.5 seconds
  - Initial Velocity: Radial, 200-400 units/s
  - Color Over Life: #FDD835 → #FFFFFF → transparent
  - Size Over Life: 4px → 8px → 0px
  - Material: M_SparkleParticle (additive blend)
```

**Unlock Sparkles:**
```
Particle System: PS_UnlockSparkles
Emitter:
  - Spawn: Burst, 30 particles
  - Lifetime: 0.8 - 1.2 seconds
  - Initial Velocity: Random cone, 150-300 units/s
  - Gravity: -100 units/s² (slight upward bias)
  - Color: Matches part rarity
  - Material: M_StarParticle
```

### 3. Sequencer Animations

**Complex Multi-Track Animations:**
```
// For unlock success animation
Level Sequence: LS_UnlockSuccess

Tracks:
  1. Dialog_Container (Transform)
     - 0.0s: Scale = 1.0
     - 1.0s: Scale = 0.9
     - Curve: EaseInQuad

  2. PartIcon (Transform)
     - 0.3s: Scale = 1.0, Rotation = 0°
     - 0.8s: Scale = 1.3, Rotation = 360°
     - 1.2s: Scale = 1.0, Rotation = 360°
     - Curve: EaseOutElastic

  3. ParticleBurst (Event)
     - 0.5s: Trigger event "SpawnParticles"

  4. Audio (Sound)
     - 1.0s: Play "unlock_success.wav"

  5. ModalOverlay (Opacity)
     - 1.2s: Alpha = 0.8
     - 1.5s: Alpha = 0.0
```

---

## Performance Considerations

### Frame Rate Targets

**Desktop:**
- Minimum: 60fps
- Target: 144fps
- Cap: Match display refresh rate

**VR:**
- Minimum: 90fps (critical)
- Target: 120fps (Index, Quest 3)
- Never drop below 90fps (motion sickness risk)

### Optimization Strategies

#### 1. Widget Invalidation
```cpp
// Only invalidate when values actually change
void UStatDisplay::UpdateValue(int32 NewValue)
{
    if (NewValue == CachedValue)
        return; // Skip if unchanged

    CachedValue = NewValue;
    Invalidate(EInvalidateWidget::Paint); // Minimal invalidation
}
```

#### 2. Animation Pooling
```cpp
// Reuse animation instances
TArray<UWidgetAnimation*> AnimationPool;

UWidgetAnimation* GetPooledAnimation(UClass* AnimClass)
{
    for (auto Anim : AnimationPool)
    {
        if (!Anim->IsPlaying() && Anim->IsA(AnimClass))
            return Anim; // Reuse stopped animation
    }

    // Create new if none available
    UWidgetAnimation* NewAnim = NewObject<UWidgetAnimation>(this, AnimClass);
    AnimationPool.Add(NewAnim);
    return NewAnim;
}
```

#### 3. LOD for Animations
```cpp
// Reduce animation quality based on distance (VR)
float DistanceToPlayer = FVector::Dist(WidgetLocation, PlayerLocation);

if (DistanceToPlayer > 300.0f)
{
    // Far: Skip particle effects, reduce animation steps
    AnimationQuality = EAnimQuality::Low;
}
else if (DistanceToPlayer > 150.0f)
{
    // Medium: Reduce particle count
    AnimationQuality = EAnimQuality::Medium;
}
else
{
    // Close: Full quality
    AnimationQuality = EAnimQuality::High;
}
```

#### 4. Batch Updates
```cpp
// Update multiple stats in one frame
void UShipStatsPanel::UpdateAllStats(const FShipStats& NewStats)
{
    // Collect all changes
    TArray<FStatChange> Changes;
    if (NewStats.Speed != CachedStats.Speed)
        Changes.Add({TEXT("Speed"), CachedStats.Speed, NewStats.Speed});
    if (NewStats.Armor != CachedStats.Armor)
        Changes.Add({TEXT("Armor"), CachedStats.Armor, NewStats.Armor});
    // ... etc

    // Animate all at once
    for (const FStatChange& Change : Changes)
    {
        AnimateStatChange(Change);
    }

    CachedStats = NewStats;
}
```

### Memory Budget

**Widget Animation Memory:**
- Single animation track: ~50KB
- Complex multi-track animation: ~200KB
- Total budget for all UI animations: 5MB

**Texture Memory:**
- Icon atlas: 2048×2048, BC7 compressed (~5MB)
- Particle textures: 512×512, BC7 compressed (~350KB each)
- UI backgrounds: Reuse materials with parameters

### GPU Considerations

**Draw Calls:**
- Batch UI elements using same material
- Max draw calls per frame: 50 (UI only)
- Use texture atlasing to reduce material count

**Shader Complexity:**
- Keep UI materials under 100 instructions
- Use simple math for animations (avoid complex noise)
- Profile with "stat RHI" and "profilegpu"

---

## VR Animation Considerations

### Motion Comfort

**Avoid:**
- Rapid lateral sliding (instant teleport instead)
- Fast rotations (>60°/second)
- Pulsing/flashing at 1-20Hz (seizure risk)
- Depth changes >50cm

**Prefer:**
- Fade transitions
- Scale animations (subtle, <30% change)
- Color transitions
- Opacity changes

### Head-Locked vs World-Space

**Head-Locked:**
- Use for: Critical notifications, health/ammo displays
- Max distance: 1.5 meters
- Angle offset: 15-30° from center (avoid blocking view)
- Example: WBP_ProgressionDisplay on wrist

**World-Space:**
- Use for: Menus, dialogs, contextual UI
- Distance: 1.5-3.0 meters
- Always face player
- Example: WBP_UnlockDialog

### Input Latency

**Critical Timing:**
- Button press to visual feedback: <50ms
- Total interaction latency: <100ms
- Use predictive animations (start on input, not confirmation)

**Motion Controller Support:**
```cpp
// Immediate visual feedback on trigger press
void UVRButton::OnMotionControllerTriggerPressed()
{
    // Instant visual change (don't wait for click event)
    SetRenderScale(FVector2D(0.95f, 0.95f));

    // Schedule release animation
    FTimerHandle Handle;
    GetWorld()->GetTimerManager().SetTimer(Handle, [this]()
    {
        SetRenderScale(FVector2D(1.0f, 1.0f));
    }, 0.15f, false);
}
```

---

## Accessibility

### Reduced Motion

**Respect User Preferences:**
```cpp
bool bReducedMotion = false; // From game settings

void PlayAnimation(UWidgetAnimation* Anim)
{
    if (bReducedMotion)
    {
        // Skip to end state instantly
        Anim->JumpToEnd();
    }
    else
    {
        // Play normally
        PlayAnimationForward(Anim);
    }
}
```

**Alternative Animations:**
- Replace slide with fade
- Replace scale with opacity
- Reduce duration by 50%
- Remove bounces and elasticity

### High Contrast Mode

**Animation Adjustments:**
- Increase border thickness (2px → 4px)
- Higher opacity for overlays (80% → 90%)
- Slower animations (+100ms duration)
- More pronounced state changes

### Screen Reader Support

**Announce Animation Events:**
```cpp
void ULevelUpWidget::PlayLevelUpAnimation()
{
    // Visual animation
    PlayAnimation(LevelUpAnim);

    // Screen reader announcement
    if (bScreenReaderEnabled)
    {
        AnnounceToScreenReader(FText::FromString(
            FString::Printf(TEXT("Level up! You are now level %d"), NewLevel)
        ));
    }
}
```

---

## Testing and Debugging

### Animation Testing Checklist

- [ ] All animations complete within specified duration
- [ ] Easing curves feel natural and smooth
- [ ] No visual popping or jarring transitions
- [ ] Animations maintain 60fps (90fps in VR)
- [ ] Reduced motion mode works correctly
- [ ] Animations stack/queue properly (multiple toasts)
- [ ] Cancel/interrupt behavior is smooth
- [ ] VR animations don't cause discomfort

### Debug Commands

```cpp
// In development build, enable animation debugging
ConsoleCommands:
  - ui.animation.debug 1        // Show animation timings
  - ui.animation.slow 0.5       // Slow all animations to 50% speed
  - ui.animation.disable 1      // Disable all animations (jump to end)
  - stat UI                     // Show UI performance stats
  - widget.invalidate.debug 1   // Visualize widget invalidation
```

### Profiling

**Key Metrics:**
- Animation update time: <2ms per frame
- Draw call count: <50 for all UI
- Tick time for all widgets: <5ms
- Memory allocation: 0 during animation (pre-allocated)

**Tools:**
- Unreal Insights: Track animation timelines
- GPU Visualizer: Check shader cost
- Widget Reflector: Inspect animation state

---

## Implementation Examples

### Example 1: Custom Easing Function

```cpp
// Custom bounce easing with configurable intensity
float EaseOutBounce(float t, float Intensity = 1.0f)
{
    const float n1 = 7.5625f * Intensity;
    const float d1 = 2.75f;

    if (t < 1.0f / d1)
    {
        return n1 * t * t;
    }
    else if (t < 2.0f / d1)
    {
        t -= 1.5f / d1;
        return n1 * t * t + 0.75f;
    }
    else if (t < 2.5f / d1)
    {
        t -= 2.25f / d1;
        return n1 * t * t + 0.9375f;
    }
    else
    {
        t -= 2.625f / d1;
        return n1 * t * t + 0.984375f;
    }
}
```

### Example 2: Chained Animations

```cpp
// Play animations in sequence with callbacks
void UUnlockDialog::PlayUnlockSequence()
{
    // Step 1: Button flash (300ms)
    PlayAnimation(ButtonFlash, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f,
        FOnWidgetAnimationPlaybackEvent::CreateUObject(this, &UUnlockDialog::OnButtonFlashComplete));
}

void UUnlockDialog::OnButtonFlashComplete()
{
    // Step 2: Icon celebration (800ms)
    PlayAnimation(IconCelebration, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f,
        FOnWidgetAnimationPlaybackEvent::CreateUObject(this, &UUnlockDialog::OnIconCelebrationComplete));
}

void UUnlockDialog::OnIconCelebrationComplete()
{
    // Step 3: Particle burst
    SpawnParticleBurst();

    // Step 4: Close after delay
    FTimerHandle CloseTimer;
    GetWorld()->GetTimerManager().SetTimer(CloseTimer, [this]()
    {
        PlayAnimation(DialogClose);
    }, 0.5f, false);
}
```

### Example 3: Smooth Value Interpolation

```cpp
// Smooth stat value update with automatic timing
class UStatValueInterpolator
{
private:
    float CurrentValue;
    float TargetValue;
    float InterpolationSpeed;
    UCurveFloat* EasingCurve;

public:
    void Tick(float DeltaTime)
    {
        if (FMath::IsNearlyEqual(CurrentValue, TargetValue, 0.1f))
        {
            CurrentValue = TargetValue;
            return;
        }

        // Calculate interpolation alpha based on curve
        float TimePassed = GetTimeSinceStarted();
        float Duration = 0.5f; // 500ms
        float Alpha = FMath::Clamp(TimePassed / Duration, 0.0f, 1.0f);

        // Apply easing curve
        float EasedAlpha = EasingCurve->GetFloatValue(Alpha);

        // Interpolate value
        CurrentValue = FMath::Lerp(StartValue, TargetValue, EasedAlpha);

        // Update UI
        UpdateDisplayValue(FMath::RoundToInt(CurrentValue));
    }
};
```

---

## Best Practices Summary

### Do's
- Use consistent timing for similar interactions
- Provide immediate feedback (<50ms)
- Test animations at various frame rates
- Respect user accessibility preferences
- Pre-allocate animation resources
- Use GPU for heavy animations (materials)
- Profile regularly during development

### Don'ts
- Don't animate too many elements simultaneously
- Don't use linear easing (feels robotic)
- Don't block user input during animations
- Don't create animations >2 seconds (user impatience)
- Don't allocate memory during animation playback
- Don't use complex shaders on mobile/VR UI
- Don't forget VR motion comfort guidelines

---

**Document Version:** 1.0
**Last Updated:** 2025-11-07
**Author:** Ship Customization System - Phase 3, Subagent 4
