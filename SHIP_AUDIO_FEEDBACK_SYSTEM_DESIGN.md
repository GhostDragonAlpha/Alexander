# Ship Audio Feedback System Design

## Overview

This document specifies a comprehensive audio feedback system for the VR space flight game, providing immersive spatial audio for ship customization, flight mechanics, and environmental interactions. The system integrates dynamic audio parameters tied to ship stats, creates spatial 3D audio experiences, and enhances VR presence through carefully designed sound effects and ambient audio layers.

---

## 1. Audio System Overview

### 1.1 Purpose and Goals

The audio feedback system serves multiple critical functions in the VR flight experience:

**1.1.1 Immersion Enhancement**
- Spatial audio positioning creates a 360-degree soundscape
- HRTF (Head-Related Transfer Function) provides directional cues
- Multiple overlapping audio layers create environmental depth
- Dynamic audio response to player actions provides real-time feedback

**1.1.2 Ship Status Communication**
- Audio parameters reflect current ship stats and status
- Engine sound pitch/frequency conveys thrust power level
- Shield strength indicated through hum intensity
- Damage state communicated through hull creaking and warnings
- Resource levels reflected in audio characteristics

**1.1.3 UI/UX Feedback**
- Confirmation sounds for menu interactions
- Audio notifications for events (purchase, unlock, upgrade)
- Error sounds for invalid actions
- Spatial audio cues for menu navigation

**1.1.4 Gameplay Enhancement**
- Thruster positions audibly locate control inputs
- Doppler effects indicate velocity changes
- Environmental audio creates context awareness
- Audio cues provide non-visual feedback in intense moments

### 1.2 Audio Categories

The system organizes audio into 8 primary categories:

**1. Engine Sounds** - Primary propulsion audio
**2. Thruster Sounds** - RCS (Reaction Control System) audio
**3. UI/Menu Sounds** - Ship customization interface feedback
**4. Hull/Damage Sounds** - Structural integrity feedback
**5. Shield Sounds** - Shield system audio
**6. Weapon Sounds** - Combat audio feedback (optional)
**7. Ambient Ship Sounds** - Background atmospheric layers
**8. Environmental Audio** - Space environment interactions

### 1.3 Technical Architecture Overview

```
Audio Manager
├── Master Volume Control
├── Sound Class Router
│   ├── Engine (40% of mix)
│   ├── UI (20% of mix)
│   ├── Effects (20% of mix)
│   ├── Ambient (10% of mix)
│   ├── Music (10% of mix)
│   └── Voice (if applicable)
├── Audio Component Manager
│   ├── Engine Audio Component
│   ├── Thruster Audio Components (6+)
│   ├── Hull Audio Component
│   ├── Shield Audio Component
│   ├── UI Audio Source
│   └── Ambient Audio Component
├── Parameter Modulator
│   ├── Thrust Input → Frequency/Pitch
│   ├── Ship Stats → Audio Characteristics
│   ├── Damage State → Audio Indicators
│   └── Shield State → Audio Feedback
└── Spatial Audio Manager
    ├── HRTF Processing
    ├── Attenuation Calculator
    ├── Doppler Effect Generator
    └── Reverb Environment Manager
```

### 1.4 Sound Cue Library

A total of 40+ sound cues are required for the full system:

**Engine Cues (8)**
- SC_Engine_Idle
- SC_Engine_Thrust_Loop
- SC_Engine_Spool_Up
- SC_Engine_Spool_Down
- SC_Engine_High_Power
- SC_Engine_Low_Power
- SC_Engine_Stall
- SC_Engine_Shutdown

**Thruster Cues (6)**
- SC_Thruster_Fire_Main
- SC_Thruster_Fire_Secondary
- SC_Thruster_Fire_Agile
- SC_Thruster_Fire_Heavy
- SC_Thruster_Burst_Short
- SC_Thruster_Burst_Long

**UI Cues (15)**
- SC_UI_PartSelect_Click
- SC_UI_PartEquip_Begin
- SC_UI_PartEquip_Complete
- SC_UI_PartUnequip_Begin
- SC_UI_PartUnequip_Complete
- SC_UI_LoadoutSwitch_Begin
- SC_UI_LoadoutSwitch_Complete
- SC_UI_PurchasePart_Success
- SC_UI_PurchasePart_Fail
- SC_UI_UnlockPart_Success
- SC_UI_LevelUp_Fanfare
- SC_UI_InsufficientCredits_Error
- SC_UI_Hover_Subtle
- SC_UI_Cancel
- SC_UI_Navigate

**Hull/Damage Cues (8)**
- SC_Hull_Creaking_Loop_Light
- SC_Hull_Creaking_Loop_Heavy
- SC_Hull_Breach_Warning
- SC_Collision_Impact_Light
- SC_Collision_Impact_Medium
- SC_Collision_Impact_Heavy
- SC_Hull_Damage_Pop
- SC_Hull_Integrity_Critical

**Shield Cues (8)**
- SC_Shield_Activate
- SC_Shield_Idle_Hum
- SC_Shield_Hit_Light
- SC_Shield_Hit_Medium
- SC_Shield_Hit_Heavy
- SC_Shield_Depleted
- SC_Shield_Recharge_Begin
- SC_Shield_Recharge_Loop

**Weapon Cues (10)** *(Optional for combat iteration)*
- SC_Weapon_Laser_Fire
- SC_Weapon_Laser_Charge
- SC_Weapon_Railgun_Fire
- SC_Weapon_Railgun_Charge
- SC_Weapon_Missile_Launch
- SC_Weapon_Missile_Charge
- SC_Weapon_Cooldown
- SC_Weapon_Overheat_Warning
- SC_Weapon_Overheat_Critical
- SC_Weapon_Reload

**Ambient Cues (8)**
- SC_Ambient_ComputerSystems_Loop
- SC_Ambient_LifeSupport_Loop
- SC_Ambient_Hydraulics_Loop
- SC_Ambient_Cockpit_Rumble
- SC_Ambient_LightShip_Variation
- SC_Ambient_HeavyShip_Variation
- SC_Ambient_Hum_Baseline
- SC_Ambient_Electronic_Chatter

**Environmental Cues (5)**
- SC_Environment_AtmosphericEntry
- SC_Environment_AsteroidPass
- SC_Environment_SolarFlare
- SC_Environment_MusicExploration
- SC_Environment_MusicCombat

### 1.5 Audio Component Architecture

The Unreal Engine audio system uses the following component strategy:

**UAudioComponent for Ship**

```cpp
class AShip {
private:
    UPROPERTY(EditAnywhere)
    UAudioComponent* EngineAudioComponent;

    UPROPERTY(EditAnywhere)
    TArray<UAudioComponent*> ThrusterAudioComponents; // 6+ components

    UPROPERTY(EditAnywhere)
    UAudioComponent* HullAudioComponent;

    UPROPERTY(EditAnywhere)
    UAudioComponent* ShieldAudioComponent;

    UPROPERTY(EditAnywhere)
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(EditAnywhere)
    UAudioComponent* WeaponAudioComponent;
};
```

Each audio component:
- Attaches to the ship actor at appropriate location
- Implements 3D spatialization
- Uses modulator-controlled parameters
- Routes through appropriate sound class

### 1.6 Sound Class Hierarchy

```
Master (1.0 volume)
├── Engine (0.4 mix)
│   ├── Engines_Main
│   └── Engines_Idle
├── UI (0.2 mix)
│   ├── UI_Confirmations
│   ├── UI_Warnings
│   └── UI_Notifications
├── Effects (0.2 mix)
│   ├── Impact_Collision
│   ├── Impact_Explosion
│   ├── Ship_Hull
│   └── Ship_Shield
├── Ambient (0.1 mix)
│   ├── Ship_Interior
│   ├── Environment
│   └── Thrusters
└── Music (0.1 mix)
    ├── Exploration
    └── Combat
```

### 1.7 Audio Parameter System

Dynamic parameters link audio to game state:

**Engine Parameters**
- `ThrustInput` (0.0 - 1.0): Current thrust lever position
- `ThrustPower` (0.5 - 2.0): Ship stat multiplier
- `EngineHealth` (0.0 - 1.0): Engine operational status

**Thruster Parameters**
- `ThrusterCount` (1 - 8): Number of active thrusters
- `ThrusterIntensity` (0.0 - 1.0): Individual thruster power
- `ThrusterType` (0.0 - 1.0): Agile vs Heavy morphing

**Damage Parameters**
- `HullIntegrity` (0.0 - 1.0): Current hull health
- `DamageType` (enum): Collision, energy, explosive
- `ImpactForce` (0.0 - 1.0): Normalized impact strength

**Shield Parameters**
- `ShieldStrength` (0.0 - 1.0): Current shield capacity
- `ShieldActive` (0.0 - 1.0): Shield activation state
- `AbsorbedDamage` (0.0 - 1.0): Last hit intensity

---

## 2. Engine Audio Detailed Specification

### 2.1 Engine Sound Purpose

The engine audio is the primary feedback mechanism for thrust control and provides:
- Real-time response to player throttle input
- Ship power level communication
- Immersive sense of mechanical power
- Status indicator for engine health

### 2.2 Engine Audio States

#### 2.2.1 Idle Engine State

**Trigger Condition**: Ship powered on, thrust input at 0%

**Audio Characteristics**
- Sound Cue: SC_Engine_Idle
- Base Frequency: 40-60 Hz (low rumble)
- Volume: 0.3 (subtle background presence)
- Continuous playback (loops indefinitely)
- Modulation: Subtle variation (±5% frequency wobble)

**Psychoacoustic Effect**
- Creates sense of contained power
- Suggests engine waiting for input
- Establishes baseline acoustic environment
- Provides haptic feedback through subwoofer rumble (if available)

**Parameter Mapping**
```cpp
float IdleFrequency = 50.0f; // Hz
float IdleVolume = 0.3f;
float IdleWobbleRate = 0.5f; // Hz modulation rate
```

#### 2.2.2 Thrust Applied State

**Trigger Condition**: Thrust input > 0%

**Audio Characteristics**
- Sound Cue: SC_Engine_Thrust_Loop
- Base Frequency Range: 100 Hz (0% thrust) → 500 Hz (100% thrust)
- Volume Range: 0.4 → 0.9 (increases with thrust)
- Continuous playback during thrust application
- Pitch follows thrust input in real-time

**Frequency Mapping Algorithm**
```cpp
float CalculateEngineFrequency(float ThrustInput, float ThrustPowerStat)
{
    // ThrustInput: 0.0 - 1.0 (player control)
    // ThrustPowerStat: 0.5 - 2.0 (ship customization)

    float BaseFrequency = FMath::Lerp(100.0f, 500.0f, ThrustInput);
    float ThrustModulation = ThrustPowerStat * 200.0f; // Up to ±200 Hz
    float FinalFrequency = FMath::Clamp(
        BaseFrequency + ThrustModulation,
        100.0f,
        700.0f
    );
    return FinalFrequency;
}
```

**Volume Mapping Algorithm**
```cpp
float CalculateEngineVolume(float ThrustInput, float ThrustPowerStat, float Mass)
{
    float BaseVolume = FMath::Lerp(0.4f, 0.9f, ThrustInput);
    float PowerMultiplier = FMath::GetMappedRangeValueClamped(
        FVector2D(0.5f, 2.0f),  // ThrustPower range
        FVector2D(0.8f, 1.2f),  // Volume multiplier range
        ThrustPowerStat
    );
    float MassInfluence = FMath::Clamp(Mass / 1000.0f, 0.9f, 1.3f);
    return BaseVolume * PowerMultiplier * MassInfluence;
}
```

**Real-Time Modulation**
- Frequency changes smoothly with thrust input (no stepped changes)
- Volume ramps with frequency for coherent perception
- Harmonic overtones shift with frequency for naturalistic effect
- Slight random variation (±3%) prevents mechanical sound

#### 2.2.3 Engine Spool-Up Transition

**Trigger Condition**: Thrust input changes from 0% to X% (any increase)

**Audio Characteristics**
- Sound Cue: SC_Engine_Spool_Up
- Duration: 0.5 seconds
- Initial Frequency: 50 Hz
- Final Frequency: Current ThrustInput frequency
- Envelope: Exponential rise (faster at start, smooths at end)
- Overlap: Fades in over spool-up, main engine fades in at end

**Psychoacoustic Effect**
- Communicates mechanical response time
- Prevents harsh frequency jumps
- Creates sense of inertial system coming online
- Establishes immersive engine response

**Implementation Strategy**
```cpp
void SpoolUpEngine(float TargetFrequency, float Duration = 0.5f)
{
    if (!bIsSpoolingUp)
    {
        bIsSpoolingUp = true;
        SpoolUpStartTime = GetWorld()->GetTimeSeconds();
        SpoolUpTargetFrequency = TargetFrequency;

        // Start spool-up sound
        PlaySpoolUpSound(Duration);
    }
}

void UpdateEngineFrequency(float DeltaTime)
{
    if (bIsSpoolingUp)
    {
        float ElapsedTime = GetWorld()->GetTimeSeconds() - SpoolUpStartTime;
        float Alpha = FMath::Min(ElapsedTime / SpoolUpDuration, 1.0f);

        // Exponential easing
        float EaseAlpha = FMath::Pow(Alpha, 0.3f); // Faster start
        float CurrentFrequency = FMath::Lerp(
            IdleFrequency,
            SpoolUpTargetFrequency,
            EaseAlpha
        );

        EngineAudioComponent->SetFloatParameter(FName("Frequency"), CurrentFrequency);

        if (Alpha >= 1.0f)
        {
            bIsSpoolingUp = false;
        }
    }
}
```

#### 2.2.4 Engine Spool-Down Transition

**Trigger Condition**: Thrust input decreases to 0%, or ship loses power

**Audio Characteristics**
- Sound Cue: SC_Engine_Spool_Down
- Duration: 1.0 seconds (slower than spool-up for realism)
- Initial Frequency: Current engine frequency
- Final Frequency: 40 Hz (engine idle)
- Envelope: Exponential decay
- Overlap: Main engine fades out over 0.7 seconds, spool-down continues

**Psychoacoustic Effect**
- Communicates engine coasting down
- Prevents abrupt silence
- Creates sense of momentum
- Allows player to anticipate engine stop

**Implementation Strategy**
```cpp
void SpoolDownEngine(float Duration = 1.0f)
{
    if (!bIsSpoolingDown)
    {
        bIsSpoolingDown = true;
        SpoolDownStartTime = GetWorld()->GetTimeSeconds();
        CurrentEngineFrequency = GetCurrentFrequency();

        // Start spool-down sound
        PlaySpoolDownSound(Duration);
    }
}

void UpdateEngineSpoolDown(float DeltaTime)
{
    if (bIsSpoolingDown)
    {
        float ElapsedTime = GetWorld()->GetTimeSeconds() - SpoolDownStartTime;
        float Alpha = FMath::Min(ElapsedTime / SpoolDownDuration, 1.0f);

        // Exponential decay easing
        float EaseAlpha = 1.0f - FMath::Pow(1.0f - Alpha, 0.5f);
        float CurrentFrequency = FMath::Lerp(
            CurrentEngineFrequency,
            IdleFrequency,
            EaseAlpha
        );

        EngineAudioComponent->SetFloatParameter(FName("Frequency"), CurrentFrequency);

        if (Alpha >= 1.0f)
        {
            bIsSpoolingDown = false;
            SetEngineState(EEngineState::Idle);
        }
    }
}
```

#### 2.2.5 High Power Engine Variant

**Trigger Condition**: ThrustPowerStat > 1.5 OR Mass > 1200 kg

**Audio Characteristics**
- Sound Cue: SC_Engine_High_Power
- Frequency Range: 80 Hz → 600 Hz (lower baseline than normal)
- Volume Range: 0.5 → 1.0 (louder overall)
- Tone: Deeper, more aggressive, with bass emphasis
- Harmonic Content: Dominant 2nd harmonic (lower frequency emphasis)
- Psychoacoustic Effect: Conveys brute force, military-grade power

**Psychoacoustic Design**
- Lower frequencies indicate larger, more powerful engine
- Higher volume conveys more aggressive thrust
- Bass emphasis creates felt impact through chest/haptics
- Aggressive harmonic profile suggests military application

#### 2.2.6 Low Power Engine Variant

**Trigger Condition**: ThrustPowerStat < 1.0 OR Mass < 400 kg

**Audio Characteristics**
- Sound Cue: SC_Engine_Low_Power
- Frequency Range: 150 Hz → 700 Hz (higher baseline)
- Volume Range: 0.2 → 0.6 (quieter overall)
- Tone: Higher pitched, whiny, with treble emphasis
- Harmonic Content: Dominant 3rd+ harmonics (higher frequency emphasis)
- Psychoacoustic Effect: Conveys efficiency, agility, electrical propulsion

**Psychoacoustic Design**
- Higher frequencies indicate smaller, lighter engine
- Lower volume conveys less raw power
- Treble emphasis creates sense of precision and efficiency
- Electrical-sounding profile suggests exotic propulsion

### 2.3 Engine Audio Implementation Code

```cpp
class UEngineAudioController : public UActorComponent
{
private:
    UPROPERTY()
    UAudioComponent* EngineAudioComponent;

    float CurrentEngineFrequency = 50.0f;
    float CurrentEngineVolume = 0.3f;
    float LastThrustInput = 0.0f;

    enum class EEngineState : uint8
    {
        Idle,
        Running,
        SpoolingUp,
        SpoolingDown,
        Critical,
        Shutdown
    };

    EEngineState CurrentEngineState = EEngineState::Idle;

public:
    void InitializeEngineAudio(UAudioComponent* InAudioComponent);

    void UpdateEngineAudio(
        float ThrustInput,
        float ThrustPowerStat,
        float ShipMass,
        float EngineHealth,
        float DeltaTime
    );

private:
    float CalculateEngineFrequency(float ThrustInput, float ThrustPowerStat);
    float CalculateEngineVolume(float ThrustInput, float ThrustPowerStat, float Mass);

    void UpdateEngineState(float ThrustInput, float EngineHealth);
    void ApplyEngineFrequency(float TargetFrequency, float DeltaTime);
    void ApplyEngineVolume(float TargetVolume, float DeltaTime);
};
```

### 2.4 Engine Audio Quality Standards

**Audio Fidelity**
- Minimum 48 kHz sample rate (44.1 kHz acceptable)
- 16-bit depth minimum, 24-bit preferred
- Mono source material (will be spatialized in 3D)
- -3 dB to -6 dB peak level (sufficient headroom)

**Frequency Response**
- Engine Idle: 40-100 Hz dominant energy
- Engine Run: 100-600 Hz fundamental range
- Harmonic content: Up to 2 kHz for richness
- Upper frequencies: 2-8 kHz for definition

**Audio Duration**
- Looping samples: Seamless loop points (no clicks)
- Transition samples: Full envelope from silence to peak
- Event samples: Complete sound arc with natural decay

---

## 3. Thruster Audio Specification

### 3.1 Thruster System Overview

Thrusters provide RCS (Reaction Control System) audio feedback for ship rotation and translation. Unlike main engines, thrusters:
- Fire in short bursts (0.1-0.3 seconds)
- Position audio spatially (left/right/front/back)
- Create overlapping polyphonic effects
- Vary in character based on thruster type

### 3.2 Thruster Positions

A typical 6-DOF (degree of freedom) ship has 8 thruster positions:

```
          [Front]
            |
    [Left] [Up] [Right]
            |
         [Back]

    [Down] [Roll-Left] [Roll-Right]
```

Each thruster has:
- Spatial position in 3D space (relative to cockpit)
- Audio component with positional 3D sound
- Individual volume and pitch parameters
- Firing duration and intensity

### 3.3 Thruster Fire Audio

#### 3.3.1 Basic Thruster Fire

**Audio Characteristics**
- Sound Cue: SC_Thruster_Fire_Main
- Duration: 0.15 seconds (typical thrust control input)
- Pitch: 200-400 Hz base
- Volume: 0.4 - 0.8 (based on thrust intensity)
- Attack: Immediate (sharp attack)
- Decay: 100 ms tail-off
- Sustain: Constant throughout firing

**Spatial Audio Properties**
- 3D position: Exact thruster location on ship
- Attenuation: Linear falloff over 50 meters
- HRTF: Full head-related transfer function
- Doppler: Applied if relative velocity exists

**Parameter Mapping**
```cpp
float CalculateThrusterPitch(float ThrusterIntensity, float ThrusterType)
{
    // ThrusterIntensity: 0.0 - 1.0
    // ThrusterType: 0.0 (Agile) - 1.0 (Heavy)

    float AgileFrequency = FMath::Lerp(300.0f, 500.0f, ThrusterIntensity);
    float HeavyFrequency = FMath::Lerp(150.0f, 300.0f, ThrusterIntensity);

    return FMath::Lerp(AgileFrequency, HeavyFrequency, ThrusterType);
}

float CalculateThrusterVolume(float ThrusterIntensity, float ThrusterCount)
{
    float BaseVolume = FMath::Lerp(0.2f, 0.8f, ThrusterIntensity);
    float OverlapPenalty = 1.0f - (ThrusterCount - 1) * 0.1f; // Reduce for overlap
    return BaseVolume * FMath::Max(OverlapPenalty, 0.5f);
}
```

#### 3.3.2 Thruster Variations

**Agile Thruster Variant** (lightweight, maneuvering-optimized)

**Audio Characteristics**
- Sound Cue: SC_Thruster_Fire_Agile
- Frequency Range: 350 Hz - 600 Hz (high-pitched)
- Duration: 0.1 seconds (quick, snappy response)
- Volume: 0.3 - 0.6 (lighter, more delicate)
- Tone: Electronic, almost musical quality
- Harmonic Content: Thin, sparse harmonics
- Psychoacoustic Effect: Responsive, precision-focused

**Design Rationale**
- Higher frequency indicates smaller thruster
- Shorter duration conveys quick response
- Lower volume suggests lighter propellant mass
- Electronic tone suggests advanced technology

**Heavy Thruster Variant** (bulky, high-thrust, cargo-optimized)

**Audio Characteristics**
- Sound Cue: SC_Thruster_Fire_Heavy
- Frequency Range: 150 Hz - 300 Hz (low-pitched)
- Duration: 0.25 seconds (longer, more sustained)
- Volume: 0.6 - 1.0 (powerful, commanding presence)
- Tone: Mechanical, percussive, industrial
- Harmonic Content: Rich, multiple harmonics
- Psychoacoustic Effect: Powerful, mechanical, industrial

**Design Rationale**
- Lower frequency indicates larger thruster
- Longer duration conveys sustained force
- Higher volume suggests heavier propellant mass
- Mechanical tone suggests robust, simple design

### 3.4 Thruster Burst Patterns

#### 3.4.1 Single Thruster Fire

**Usage**: Translation in single axis (forward, left, up)

```cpp
void FireThruster(FName ThrusterName, float Intensity, float Duration = 0.15f)
{
    UAudioComponent* ThrusterAudio = GetThrusterAudioComponent(ThrusterName);

    float Pitch = CalculateThrusterPitch(Intensity, GetThrusterType(ThrusterName));
    float Volume = CalculateThrusterVolume(Intensity, 1); // Single thruster

    ThrusterAudio->SetFloatParameter(FName("Pitch"), Pitch);
    ThrusterAudio->SetVolumeMultiplier(Volume);
    ThrusterAudio->Play();

    // Auto-stop after duration
    GetWorld()->GetTimerManager().SetTimer(
        ThrusterHandle,
        [ThrusterAudio]() { ThrusterAudio->Stop(); },
        Duration,
        false
    );
}
```

#### 3.4.2 Dual Opposing Thruster Fire

**Usage**: Rotation around single axis (pitch, yaw, roll)

**Audio Effect**: Two thrusters fire simultaneously from opposite sides
- Left and Right thrusters for yaw
- Top and Bottom thrusters for pitch
- Roll-Left and Roll-Right for roll rotation

```cpp
void FireOpposingThrusters(
    FName ThrusterName1,
    FName ThrusterName2,
    float Intensity,
    float Duration = 0.15f
)
{
    FireThruster(ThrusterName1, Intensity, Duration);
    FireThruster(ThrusterName2, Intensity, Duration);

    // Slight variation in pitch (±5%) for natural effect
    float PitchVariation = FMath::RandRange(-0.05f, 0.05f);
    // Apply variation to one thruster
}
```

**Spatial Audio Effect**
- One thruster sound pans left
- Other thruster sound pans right
- Creates immersive directional cue
- Communicates rotation axis to player

#### 3.4.3 Multi-Thruster Coupling

**Usage**: Complex maneuvers using 3+ thrusters

**Audio Characteristics**
- Multiple thruster sounds layer together
- Pitch variations prevent mechanical uniformity
- Volume reduction per overlapping thruster
- Complex spatial positioning creates immersive effect

```cpp
void FireMultipleThrusters(const TArray<FName>& ThrusterNames, float Intensity)
{
    for (int32 i = 0; i < ThrusterNames.Num(); ++i)
    {
        FName ThrusterName = ThrusterNames[i];

        // Each thruster gets slight pitch variation
        float PitchOffset = FMath::RandRange(-0.03f, 0.03f);

        UAudioComponent* ThrusterAudio = GetThrusterAudioComponent(ThrusterName);
        float BasePitch = CalculateThrusterPitch(Intensity, GetThrusterType(ThrusterName));
        ThrusterAudio->SetFloatParameter(FName("Pitch"), BasePitch * (1.0f + PitchOffset));

        // Volume reduces with more concurrent thrusters
        float ConcurrentCount = (float)ThrusterNames.Num();
        float VolumePenalty = FMath::Clamp(1.0f / ConcurrentCount, 0.4f, 1.0f);
        float Volume = CalculateThrusterVolume(Intensity, ThrusterNames.Num()) * VolumePenalty;
        ThrusterAudio->SetVolumeMultiplier(Volume);

        ThrusterAudio->Play();
    }
}
```

### 3.5 Thruster Audio Quality Standards

**Audio Specifications**
- Sample Rate: 48 kHz minimum
- Bit Depth: 16-bit minimum
- Duration: 0.1-0.3 seconds with sharp attack
- Frequency Response: 100-8000 Hz
- Loop Capability: Not required (one-shot sounds)

**Spatial Characteristics**
- Mono source material for 3D spatialization
- Clean, isolated sound (minimal background)
- Clear attack for immediate perception
- Quick decay (100-200 ms tail)

---

## 4. Ship Customization Menu Audio Specification

### 4.1 UI Audio Purpose

The menu audio system provides:
- Confirmation feedback for player actions
- Notification of state changes (purchases, unlocks)
- Error indication for invalid actions
- Subtle navigation cues
- Immersive menu interface feedback

### 4.2 Menu Interaction Sounds

#### 4.2.1 Part Selection Click

**Trigger Condition**: Player hovers over/selects a ship part in customization menu

**Audio Characteristics**
- Sound Cue: SC_UI_PartSelect_Click
- Duration: 0.08 seconds
- Frequency: 2000-4000 Hz (mid-high range)
- Volume: 0.4
- Attack: Immediate (no ramp)
- Decay: Natural tail (50 ms minimum)
- Type: Clean, digital click

**Implementation**
```cpp
void OnPartSelected(UShipPart* Part)
{
    UIAudioComponent->SetSound(SC_PartSelect_Click);
    UIAudioComponent->Play();
}
```

**Psychoacoustic Effect**
- High frequency creates attention without startling
- Short duration prevents cognitive load
- Clean digital tone matches menu aesthetic
- Confirms interaction without being obtrusive

#### 4.2.2 Part Equip Sound

**Trigger Condition**: Player equips a ship part (installs on ship)

**Audio Characteristics**
- Sound Sequence: Mechanical clunk + electronic confirm beep
- Total Duration: 0.5 seconds
- Cue Name: SC_UI_PartEquip_Complete
- Volume Progression: 0.3 → 0.7 → 0.5

**Audio Layers**
1. **Mechanical Clunk** (0.0-0.2s)
   - Frequency: 100-200 Hz
   - Type: Physical impact sound
   - Purpose: Conveys mechanical installation
   - Volume: 0.7

2. **Electronic Confirm Beep** (0.2-0.5s)
   - Frequency: 2000 Hz (sustained)
   - Type: Synthesized tone
   - Duration: 0.3 seconds
   - Purpose: System confirmation
   - Volume: 0.5

**Implementation**
```cpp
void OnPartEquipped(UShipPart* Part)
{
    // Play complete equip sequence
    UIAudioComponent->SetSound(SC_UI_PartEquip_Complete);
    UIAudioComponent->SetVolumeMultiplier(0.7f);
    UIAudioComponent->Play();

    // Could also play layered:
    // PlayMechanicalClunk();
    // FTimerHandle TimerHandle;
    // GetWorld()->GetTimerManager().SetTimer(
    //     TimerHandle,
    //     [this]() { PlayElectronicConfirm(); },
    //     0.2f,
    //     false
    // );
}
```

#### 4.2.3 Part Unequip Sound

**Trigger Condition**: Player removes a ship part from the ship

**Audio Characteristics**
- Sound Cue: SC_UI_PartUnequip_Complete
- Structure: Reverse of equip sound (reverse psychoacoustic effect)
- Total Duration: 0.4 seconds
- Volume Progression: 0.5 → 0.7 → 0.3

**Audio Layers**
1. **Electronic Disconnect Beep** (0.0-0.2s)
   - Frequency: 2000 Hz (sustain, then pitch bend down)
   - Type: Synthesized tone with frequency modulation
   - Pitch Bend: 2000 Hz → 1000 Hz
   - Purpose: System disengagement
   - Volume: 0.5

2. **Mechanical Release Click** (0.2-0.4s)
   - Frequency: 200-400 Hz
   - Type: Sharp, quick mechanical release
   - Purpose: Conveys physical disconnection
   - Volume: 0.7 (initially), 0.3 (decay tail)

**Psychoacoustic Effect**
- Frequency bend down communicates power-down
- Mechanical click creates sense of disconnection
- Reverse of equip maintains mental association
- Lower end volume indicates "less active"

#### 4.2.4 Loadout Switch Sound

**Trigger Condition**: Player switches between saved ship configurations

**Audio Characteristics**
- Sound Cue: SC_UI_LoadoutSwitch_Complete
- Total Duration: 0.8 seconds
- Primary Effect: Whoosh + confirmation
- Volume Profile: 0.3 → 0.6 → 0.5

**Audio Layers**
1. **Transition Whoosh** (0.0-0.3s)
   - Frequency Sweep: 800 Hz → 2000 Hz
   - Type: Pitched whoosh sound
   - Purpose: Auditory transition cue
   - Volume: 0.6 (peak at 0.2s)

2. **Configuration Confirm** (0.3-0.8s)
   - Frequency: 2500 Hz + 3000 Hz (dual tone)
   - Type: Digital confirm sequence
   - Purpose: New configuration established
   - Duration: 0.5 seconds
   - Volume: 0.5

**Implementation**
```cpp
void OnLoadoutSwitched(UShipLoadout* NewLoadout)
{
    // Play transition effect
    UIAudioComponent->SetSound(SC_UI_LoadoutSwitch_Complete);
    UIAudioComponent->Play();

    // Update ship audio with new stats
    GetWorld()->GetTimerManager().SetTimer(
        LoadoutSwitchHandle,
        [this, NewLoadout]()
        {
            ApplyLoadoutAudio(NewLoadout);
        },
        0.4f, // Apply after transition
        false
    );
}
```

#### 4.2.5 Purchase Part Success

**Trigger Condition**: Player successfully purchases a new ship part with credits

**Audio Characteristics**
- Sound Cue: SC_UI_PurchasePart_Success
- Total Duration: 0.6 seconds
- Structure: Cash register ding + success chime
- Volume Progression: 0.4 → 0.8 → 0.5

**Audio Layers**
1. **Cash Register Ding** (0.0-0.15s)
   - Frequency: 800-1200 Hz
   - Type: Metal bell strike
   - Attack: Immediate
   - Decay: Natural (resonance tail)
   - Purpose: Economic transaction sound
   - Volume: 0.8

2. **Success Chime** (0.1-0.6s)
   - Frequency Sequence: 2000 Hz, 2500 Hz, 3000 Hz (ascending)
   - Type: Three-note ascending sequence
   - Timing: 0.15s each note
   - Overlap: Slight overlap for cohesion
   - Purpose: Positive confirmation
   - Volume: 0.5 (per note)

**Psychoacoustic Effect**
- Cash sound indicates money spent
- Ascending pitch contour indicates success
- Multiple notes provide extended positive feedback
- Total duration keeps player engaged but not annoyed

#### 4.2.6 Purchase Part Failure

**Trigger Condition**: Player attempts to purchase but lacks sufficient credits

**Audio Characteristics**
- Sound Cue: SC_UI_InsufficientCredits_Error
- Total Duration: 0.5 seconds
- Structure: Error buzz with descending tone
- Volume: 0.5 (not alarming)

**Audio Layers**
1. **Insufficient Funds Error** (0.0-0.5s)
   - Frequency Sweep: 1000 Hz → 500 Hz (descending)
   - Type: Synthesized error tone
   - Attack: Immediate
   - Modulation: 100 Hz wobble (vibrato effect)
   - Purpose: Negative feedback, not severe
   - Volume: 0.5

**Psychoacoustic Effect**
- Descending pitch communicates failure
- Wobble conveys uncertainty/instability
- Lower volume prevents frustration escalation
- Distinct from success sound for clear distinction

#### 4.2.7 Unlock Part Sound

**Trigger Condition**: Player unlocks a new ship part (via leveling, achievements, etc.)

**Audio Characteristics**
- Sound Cue: SC_UI_UnlockPart_Success
- Total Duration: 2.0 seconds
- Structure: Achievement fanfare (shorter than level-up)
- Volume Profile: Varied per section
- Intensity: Medium (significant but not overwhelming)

**Audio Layers**
1. **Unlock Fanfare** (0.0-2.0s)
   - Frequency Sequence: Complex multi-note progression
   - Pattern: 3-note ascending phrase, repeated with variation
   - Note 1 (0.0-0.5s): 2000 Hz
   - Note 2 (0.3-0.8s): 2500 Hz
   - Note 3 (0.6-1.2s): 3000 Hz
   - Repeat variation (1.2-2.0s): Ascending pattern with reverb tail
   - Purpose: Achievement recognition
   - Volume: 0.6 (sustained throughout)

**Implementation**
```cpp
void OnPartUnlocked(UShipPart* UnlockedPart)
{
    // Play unlock fanfare
    UIAudioComponent->SetSound(SC_UI_UnlockPart_Success);
    UIAudioComponent->SetVolumeMultiplier(0.6f);
    UIAudioComponent->Play();

    // Display unlock notification (visual + audio combination)
    ShowUnlockNotification(UnlockedPart);
}
```

#### 4.2.8 Level Up Sound

**Trigger Condition**: Player's ship reaches a new level / progression tier

**Audio Characteristics**
- Sound Cue: SC_UI_LevelUp_Fanfare
- Total Duration: 3.0 seconds
- Structure: Epic level-up fanfare
- Volume Profile: Dynamic progression
- Intensity: High (celebrating major achievement)

**Audio Composition**
1. **Initial Ascending Arpeggio** (0.0-1.0s)
   - Notes: G4, B4, D5, G5 (ascending)
   - Tempo: Rapid (one note every 250ms)
   - Type: Orchestral note sequence
   - Volume Rise: 0.2 → 0.8 (ascending)

2. **Sustained Victory Note** (0.8-1.8s)
   - Frequency: 2500 Hz (G5 equivalent)
   - Type: Synthesized sustained tone
   - Harmonic Content: Rich (multiple harmonics)
   - Purpose: Triumphant peak
   - Volume: 0.8

3. **Descending Resolution** (1.6-3.0s)
   - Pattern: Descending chromatic scale with reverb
   - Frequency Range: 2500 Hz → 1500 Hz
   - Type: Orchestral resolution
   - Purpose: Musical conclusion
   - Volume: 0.8 → 0.3 (decays to silence)

**Psychoacoustic Effect**
- Rapid arpeggio creates excitement
- Sustained peak conveys importance
- Descending resolution provides musical closure
- Total duration emphasizes significance
- Orchestral tone matches achievement magnitude

#### 4.2.9 Hover Over Part

**Trigger Condition**: Player hovers cursor/aim over a ship part in menu

**Audio Characteristics**
- Sound Cue: SC_UI_Hover_Subtle
- Duration: 0.1 seconds
- Frequency: 1500-2000 Hz
- Volume: 0.2 (very subtle)
- Attack: Immediate but very soft
- Decay: Quick (50 ms)
- Type: Barely audible feedback

**Psychoacoustic Effect**
- Provides navigation feedback without distraction
- Low volume prevents cognitive overload
- Quick decay prevents repetition fatigue
- Distinct enough to be noticeable in quiet moments

#### 4.2.10 Menu Cancel/Back

**Trigger Condition**: Player cancels dialog or navigates back in menu

**Audio Characteristics**
- Sound Cue: SC_UI_Cancel
- Duration: 0.2 seconds
- Frequency Sweep: 1000 Hz → 600 Hz (descending)
- Volume: 0.4
- Type: Soft cancel sound
- Modulation: Slight wobble (20 Hz modulation)
- Purpose: Non-confirmatory feedback

**Psychoacoustic Effect**
- Descending pitch communicates "going back"
- Softer than equip sound (less consequential)
- Wobble suggests state of flux/transition

### 4.3 Menu Audio Integration

**Audio Component Setup**

```cpp
class UShipCustomizationMenuAudio : public UActorComponent
{
private:
    UPROPERTY(EditAnywhere)
    UAudioComponent* UIAudioComponent;

    UPROPERTY(EditAnywhere)
    class USoundCue* SC_PartSelect_Click;

    UPROPERTY(EditAnywhere)
    class USoundCue* SC_PartEquip_Complete;

    // ... other sound cues ...

public:
    void PlayPartSelectSound();
    void PlayPartEquipSound(float Delay = 0.0f);
    void PlayPartUnequipSound();
    void PlayLoadoutSwitchSound();
    void PlayPurchaseSuccessSound();
    void PlayPurchaseFailureSound();
    void PlayUnlockSound();
    void PlayLevelUpSound();
    void PlayHoverSound();
    void PlayCancelSound();
};
```

### 4.4 Audio Priority and Ducking

**Sound Mixing Strategy**

```cpp
// Engine/Ambient audio ducks when UI sounds play
void OnUIAudioPlay(float UIVolume = 0.5f)
{
    // Reduce background audio during important UI events
    float DuckAmount = 0.5f; // Reduce to 50% volume
    EngineAudioComponent->SetVolumeMultiplier(1.0f - DuckAmount);
    AmbientAudioComponent->SetVolumeMultiplier(1.0f - DuckAmount);

    // Restore after UI sound completes
    GetWorld()->GetTimerManager().SetTimer(
        DuckHandle,
        [this]()
        {
            EngineAudioComponent->SetVolumeMultiplier(1.0f);
            AmbientAudioComponent->SetVolumeMultiplier(1.0f);
        },
        UIAudioDuration,
        false
    );
}
```

---

## 5. Hull and Damage Audio Specification

### 5.1 Hull Integrity Feedback

The hull audio system provides feedback on ship damage state through:
- Creaking sounds indicating structural stress
- Warning alarms for critical damage
- Impact sounds for collisions
- Damage-related audio parameter modulation

### 5.2 Hull Creaking Sounds

#### 5.2.1 Light Creaking

**Trigger Condition**: Hull Integrity: 30-50%

**Audio Characteristics**
- Sound Cue: SC_Hull_Creaking_Loop_Light
- Playback: Continuous loop (while damaged)
- Frequency Range: 100-300 Hz (low structural creaks)
- Volume: 0.15 (subtle background presence)
- Modulation: Random timing, 0.5-2.0 second intervals
- Duration per creak: 0.3-0.6 seconds
- Type: Organic structural stress sounds

**Parameter Mapping**
```cpp
void UpdateHullCreakingAudio(float HullIntegrity)
{
    if (HullIntegrity < 0.5f && HullIntegrity >= 0.3f)
    {
        // Light creaking
        HullAudioComponent->SetSound(SC_Hull_Creaking_Light);
        HullAudioComponent->SetVolumeMultiplier(0.15f);

        if (!HullAudioComponent->IsPlaying())
        {
            HullAudioComponent->Play();
        }
    }
    else if (HullIntegrity < 0.3f)
    {
        // Heavy creaking (covered below)
        PlayHeavyCreaking(HullIntegrity);
    }
    else
    {
        // Healthy - no creaking
        if (HullAudioComponent->IsPlaying())
        {
            HullAudioComponent->Stop();
        }
    }
}
```

**Psychoacoustic Effect**
- Low frequency creaks suggest deep structural stress
- Subtle volume doesn't alarm but indicates damage
- Random timing prevents mechanical feel
- Continuous loop creates ambient warning

#### 5.2.2 Heavy Creaking

**Trigger Condition**: Hull Integrity: 10-30%

**Audio Characteristics**
- Sound Cue: SC_Hull_Creaking_Loop_Heavy
- Playback: Continuous loop (while critically damaged)
- Frequency Range: 80-250 Hz (deeper, more urgent)
- Volume Range: 0.25-0.4 (scales with damage severity)
- Modulation: More frequent, 0.3-1.0 second intervals
- Duration per creak: 0.4-0.8 seconds
- Type: Severe structural stress, alarming

**Volume Scaling Algorithm**
```cpp
float CalculateCreakingVolume(float HullIntegrity)
{
    if (HullIntegrity < 0.1f)
    {
        return 0.5f; // Maximum creaking at critical damage
    }
    else if (HullIntegrity < 0.3f)
    {
        // Linear scale from 0.25 to 0.5 between 30% and 10%
        return FMath::GetMappedRangeValueClamped(
            FVector2D(0.3f, 0.1f),  // HullIntegrity range
            FVector2D(0.25f, 0.5f), // Volume range
            HullIntegrity
        );
    }
    return 0.0f; // No creaking above 30%
}
```

**Psychoacoustic Effect**
- Deeper frequencies indicate critical structural failure
- Higher volume creates sense of urgency
- More frequent creaks communicate increasing danger
- Still organic (not harsh alarm) to maintain immersion

#### 5.2.3 Hull Integrity Critical Warning

**Trigger Condition**: Hull Integrity: < 10%

**Audio Characteristics**
- Sound Cue: SC_Hull_Integrity_Critical
- Playback: Looping alarm (until repaired or ship destroyed)
- Frequency: 2000 Hz alarm tone + 100 Hz rumble
- Volume: 0.4-0.6 (prominent but not overwhelming)
- Pattern: 0.5s on, 0.5s off (pulsing alarm)
- Modulation: Wobble effect (±100 Hz modulation on tone)
- Purpose: Urgent warning of imminent destruction

**Implementation**
```cpp
void PlayHullCriticalAlarm(float HullIntegrity)
{
    if (HullIntegrity < 0.1f && !bCriticalAlarmActive)
    {
        bCriticalAlarmActive = true;

        // Start alarm sound
        HullAlarmComponent->SetSound(SC_Hull_Integrity_Critical);
        HullAlarmComponent->SetVolumeMultiplier(0.5f);
        HullAlarmComponent->Play();

        // Start visual indicator (flashing light)
        ActivateAlarmLighting();
    }
    else if (HullIntegrity >= 0.15f && bCriticalAlarmActive)
    {
        bCriticalAlarmActive = false;
        HullAlarmComponent->Stop();
        DeactivateAlarmLighting();
    }
}
```

### 5.3 Collision Impact Sounds

#### 5.3.1 Light Collision Impact

**Trigger Condition**: Collision with velocity < 500 cm/s

**Audio Characteristics**
- Sound Cue: SC_Collision_Impact_Light
- Duration: 0.15 seconds
- Frequency: 200-400 Hz (low frequency impact)
- Volume: 0.2-0.4 (based on impact force)
- Attack: Immediate
- Decay: Quick (100 ms tail)
- Type: Metallic bump sound

**Impact Force Calculation**
```cpp
void OnCollisionHit(const FHitResult& HitResult, float ImpactVelocity)
{
    // Normalize velocity to 0-1 range
    float NormalizedImpact = FMath::Clamp(
        ImpactVelocity / 1000.0f, // 1000 cm/s = 1.0
        0.0f,
        1.0f
    );

    if (NormalizedImpact < 0.5f)
    {
        PlayCollisionLight(NormalizedImpact);
    }
    else if (NormalizedImpact < 0.8f)
    {
        PlayCollisionMedium(NormalizedImpact);
    }
    else
    {
        PlayCollisionHeavy(NormalizedImpact);
    }
}

void PlayCollisionLight(float ImpactIntensity)
{
    UAudioComponent* ImpactAudio = GetCollisionAudioComponent();
    ImpactAudio->SetSound(SC_Collision_Impact_Light);

    // Volume scales with impact
    float Volume = FMath::Lerp(0.2f, 0.4f, ImpactIntensity);
    ImpactAudio->SetVolumeMultiplier(Volume);

    ImpactAudio->Play();
}
```

#### 5.3.2 Medium Collision Impact

**Trigger Condition**: Collision with velocity 500-1000 cm/s

**Audio Characteristics**
- Sound Cue: SC_Collision_Impact_Medium
- Duration: 0.25 seconds
- Frequency: 150-350 Hz (deeper impact)
- Volume: 0.4-0.7 (moderate impact)
- Attack: Sharp attack (10 ms)
- Decay: Moderate (150 ms tail)
- Harmonic Content: Richer than light impact
- Type: Substantial metallic thud

**Psychoacoustic Effect**
- Deeper frequency indicates more substantial collision
- Longer duration conveys more force
- Richer harmonics create more detailed impact sound

#### 5.3.3 Heavy Collision Impact

**Trigger Condition**: Collision with velocity > 1000 cm/s

**Audio Characteristics**
- Sound Cue: SC_Collision_Impact_Heavy
- Duration: 0.4 seconds
- Frequency Range: 80-300 Hz (very deep, structural impact)
- Volume: 0.7-1.0 (commanding presence)
- Attack: Immediate, powerful
- Decay: Long resonance (400+ ms tail)
- Harmonic Content: Complex multi-harmonic impact
- Type: Destructive collision, major structural impact

**Mass-Based Modulation**

```cpp
void PlayCollisionHeavy(float ImpactIntensity, float ColliderMass = 0.0f)
{
    UAudioComponent* ImpactAudio = GetCollisionAudioComponent();
    ImpactAudio->SetSound(SC_Collision_Impact_Heavy);

    // Volume scales with impact AND collider mass
    float VolumeFromImpact = FMath::Lerp(0.7f, 1.0f, ImpactIntensity);
    float MassInfluence = 1.0f;

    if (ColliderMass > 0.0f)
    {
        // Heavier objects create louder impacts
        MassInfluence = FMath::Min(1.0f + (ColliderMass / 5000.0f), 1.3f);
    }

    float FinalVolume = VolumeFromImpact * MassInfluence;
    ImpactAudio->SetVolumeMultiplier(FMath::Min(FinalVolume, 1.0f));

    // Pitch shifts with mass too
    float PitchShift = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, 5000.0f),   // Mass range (kg)
        FVector2D(1.0f, 0.8f),      // Pitch multiplier
        ColliderMass
    );
    ImpactAudio->SetPitchMultiplier(PitchShift);

    ImpactAudio->Play();
}
```

### 5.4 Hull Breach Consequences

#### 5.4.1 Atmospheric Leakage

**Trigger Condition**: Hull Integrity < 20% with atmosphere intact

**Audio Characteristics**
- Sound Cue: SC_Hull_Breach_Warning
- Playback: Starts at 20% integrity, continues until repaired
- Frequency: 3000-4000 Hz alarm + 100 Hz rumble
- Volume: 0.3-0.5 (warning level)
- Modulation: Periodic (0.3s on, 0.7s off)
- Purpose: Alert player to hull breach

**Implementation**
```cpp
void UpdateHullBreachAudio(float HullIntegrity, bool bHasAtmosphere)
{
    if (HullIntegrity < 0.2f && bHasAtmosphere)
    {
        if (!bBreachAlarmActive)
        {
            bBreachAlarmActive = true;
            PlayBreachAlarm();
        }
    }
    else
    {
        if (bBreachAlarmActive)
        {
            bBreachAlarmActive = false;
            StopBreachAlarm();
        }
    }
}
```

---

## 6. Shield Audio Specification

### 6.1 Shield System Audio Overview

The shield audio system provides feedback on defensive status through:
- Activation/deactivation sounds
- Idle hum indicating active shield
- Impact sounds based on damage absorbed
- Recharge progression audio

### 6.2 Shield Activation Sound

**Trigger Condition**: Shield activated (shield strength > 0 after being off)

**Audio Characteristics**
- Sound Cue: SC_Shield_Activate
- Duration: 1.0 second
- Frequency: 800 Hz starting point, rising to 2000 Hz
- Frequency Sweep: Linear rise from 800 Hz to 2000 Hz
- Volume: 0.3 → 0.6 → 0.4 (rise then decay)
- Type: Synthesized power-up sound
- Envelope: Attack (0.1s), Sustain (0.7s), Release (0.2s)

**Implementation**
```cpp
void ActivateShield()
{
    if (!bShieldActive)
    {
        bShieldActive = true;
        ShieldAudioComponent->SetSound(SC_Shield_Activate);
        ShieldAudioComponent->SetVolumeMultiplier(0.3f);
        ShieldAudioComponent->Play();

        // After activation, start idle hum
        GetWorld()->GetTimerManager().SetTimer(
            ShieldIdleHandle,
            [this]() { StartShieldIdleHum(); },
            1.0f,
            false
        );
    }
}
```

**Psychoacoustic Effect**
- Rising frequency communicates power-up sequence
- Synthesized tone suggests energy/force field
- Conveys sense of protection activated

### 6.3 Shield Idle Hum

**Trigger Condition**: Shield is active and not taking damage

**Audio Characteristics**
- Sound Cue: SC_Shield_Idle_Hum
- Playback: Continuous loop while shield active
- Base Frequency: 1000 Hz
- Volume: Scales with shield strength (0.1-0.3)
- Modulation: Subtle wobble (±50 Hz at 1 Hz rate)
- Harmonics: Light presence of 2nd and 3rd harmonics
- Type: Sustained electronic hum

**Shield Strength Volume Mapping**
```cpp
void UpdateShieldIdleHum(float ShieldStrength)
{
    if (ShieldStrength > 0.0f)
    {
        if (!bShieldIdleActive)
        {
            bShieldIdleActive = true;
            ShieldIdleComponent->Play();
        }

        // Volume increases with shield strength
        float IdleVolume = FMath::GetMappedRangeValueClamped(
            FVector2D(0.0f, 1.0f),    // Shield strength range
            FVector2D(0.1f, 0.3f),    // Volume range
            ShieldStrength
        );
        ShieldIdleComponent->SetVolumeMultiplier(IdleVolume);
    }
    else
    {
        if (bShieldIdleActive)
        {
            bShieldIdleActive = false;
            ShieldIdleComponent->Stop();
        }
    }
}
```

**Psychoacoustic Effect**
- Constant hum indicates active protection
- Volume variation communicates shield strength
- Subtle wobble prevents mechanical perception
- Subconscious awareness of shield status

### 6.4 Shield Hit Sounds

#### 6.4.1 Light Shield Hit

**Trigger Condition**: Shield absorbs damage < 25% capacity

**Audio Characteristics**
- Sound Cue: SC_Shield_Hit_Light
- Duration: 0.2 seconds
- Frequency: 1500-3000 Hz (impact frequency)
- Type: Electric "zap" sound
- Volume: 0.3-0.5 (based on damage absorbed)
- Attack: Immediate crackle
- Decay: Quick tail (150 ms)

**Implementation**
```cpp
void OnShieldHit(float DamageAbsorbed, float MaxShieldCapacity)
{
    float DamageRatio = DamageAbsorbed / MaxShieldCapacity;

    if (DamageRatio < 0.25f)
    {
        PlayShieldHitLight(DamageRatio);
    }
    else if (DamageRatio < 0.6f)
    {
        PlayShieldHitMedium(DamageRatio);
    }
    else
    {
        PlayShieldHitHeavy(DamageRatio);
    }
}

void PlayShieldHitLight(float DamageRatio)
{
    ShieldHitComponent->SetSound(SC_Shield_Hit_Light);
    float Volume = FMath::Lerp(0.3f, 0.5f, DamageRatio);
    ShieldHitComponent->SetVolumeMultiplier(Volume);
    ShieldHitComponent->Play();
}
```

#### 6.4.2 Medium Shield Hit

**Trigger Condition**: Shield absorbs damage 25-60% capacity

**Audio Characteristics**
- Sound Cue: SC_Shield_Hit_Medium
- Duration: 0.3 seconds
- Frequency: 800-2000 Hz (broader impact range)
- Type: Stronger electric impact
- Volume: 0.5-0.8
- Attack: Rapid build-up
- Decay: Longer tail with reverb

#### 6.4.3 Heavy Shield Hit

**Trigger Condition**: Shield absorbs damage > 60% capacity

**Audio Characteristics**
- Sound Cue: SC_Shield_Hit_Heavy
- Duration: 0.4 seconds
- Frequency: 400-2000 Hz (very broad, bass emphasis)
- Type: Powerful electrical impact
- Volume: 0.8-1.0 (commanding presence)
- Attack: Sharp, impactful
- Decay: Extended resonance (400+ ms)

### 6.5 Shield Depletion Sound

**Trigger Condition**: Shield capacity reaches 0

**Audio Characteristics**
- Sound Cue: SC_Shield_Depleted
- Duration: 0.8 seconds
- Structure: Power-down + warning beep sequence
- Volume: 0.4-0.6

**Audio Layers**
1. **Shield Power-Down** (0.0-0.5s)
   - Frequency: 2000 Hz descending to 600 Hz
   - Type: Descending electronic whine
   - Purpose: Shield disengaging
   - Volume: 0.6 → 0.3

2. **Depletion Warning Beep** (0.5-0.8s)
   - Frequency: 2500 Hz
   - Type: Three short beeps
   - Duration per beep: 0.1s
   - Spacing: 0.05s between beeps
   - Purpose: Alert player of depletion
   - Volume: 0.4

### 6.6 Shield Recharge Sound

**Trigger Condition**: Shield begins recharging (external recharge source or repair)

**Audio Characteristics**
- Sound Cue: SC_Shield_Recharge_Begin
- Total Duration: 5.0+ seconds (recharge time)
- Structure: Gradual power-up + sustained hum

**Audio Layers**
1. **Recharge Begin** (0.0-0.5s)
   - Frequency Sweep: 500 Hz → 1200 Hz
   - Type: Rising synthesized tone
   - Purpose: Recharge initiation
   - Volume: 0.2 → 0.4

2. **Recharge Loop** (0.5s - completion)
   - Frequency: 1000-1500 Hz (modulating)
   - Type: Sustained hum with subtle variation
   - Modulation Rate: Increases as recharge progresses
   - Purpose: Continuous recharge feedback
   - Volume: 0.3 (constant throughout)

**Modulation Strategy**
```cpp
void UpdateShieldRecharge(float RechargeProgress)
{
    if (RechargeProgress > 0.0f && RechargeProgress < 1.0f)
    {
        if (!bRechargeActive)
        {
            bRechargeActive = true;
            StartRechargeSound();
        }

        // Modulation rate increases with recharge progress
        float ModulationRate = FMath::Lerp(0.5f, 2.0f, RechargeProgress);
        ShieldRechargeComponent->SetFloatParameter(FName("ModulationRate"), ModulationRate);

        // Volume increases slightly as recharge progresses
        float Volume = FMath::Lerp(0.2f, 0.4f, RechargeProgress);
        ShieldRechargeComponent->SetVolumeMultiplier(Volume);
    }
    else if (RechargeProgress >= 1.0f && bRechargeActive)
    {
        bRechargeActive = false;
        CompleteRechargeSound();
    }
}
```

---

## 7. Weapon Audio Specification

### 7.1 Weapon System Audio Overview

*(Note: Weapon audio is optional for the current flight-only iteration. Include this specification for future combat implementation.)*

The weapon audio system provides feedback on:
- Weapon firing sounds
- Weapon charging states
- Weapon cooldown progression
- Weapon heat management

### 7.2 Laser Weapon Sounds

#### 7.2.1 Laser Fire

**Audio Characteristics**
- Sound Cue: SC_Weapon_Laser_Fire
- Duration: 0.1 seconds
- Frequency: 2000-4000 Hz (high energy)
- Type: "Pew pew" laser beam sound
- Volume: 0.4-0.7
- Attack: Immediate
- Decay: Quick tail (50 ms)

#### 7.2.2 Laser Charge

**Audio Characteristics**
- Sound Cue: SC_Weapon_Laser_Charge
- Duration: 1-3 seconds (configurable charge time)
- Frequency Sweep: 1000 Hz → 3000 Hz
- Type: Rising energy build-up
- Volume: Gradual increase from 0.2 to 0.6
- Modulation: Wobble effect increases toward end

### 7.3 Railgun Weapon Sounds

#### 7.3.1 Railgun Fire

**Audio Characteristics**
- Sound Cue: SC_Weapon_Railgun_Fire
- Duration: 0.5 seconds
- Structure: Bass thump + electric crack overlay
- Volume: 0.7-1.0

**Audio Layers**
1. **Bass Thump** (0.0-0.3s)
   - Frequency: 50-100 Hz
   - Type: Deep bass impact
   - Purpose: Physical force sensation
   - Volume: 0.8

2. **Electric Crack** (0.1-0.5s)
   - Frequency: 3000-6000 Hz
   - Type: Harsh electrical discharge
   - Purpose: High-energy impact
   - Volume: 0.6

### 7.4 Missile Weapon Sounds

#### 7.4.1 Missile Launch

**Audio Characteristics**
- Sound Cue: SC_Weapon_Missile_Launch
- Duration: 1.0 second
- Structure: Whoosh + engine ignition
- Volume: 0.6-0.9

**Audio Layers**
1. **Launch Whoosh** (0.0-0.3s)
   - Frequency Sweep: 800 Hz → 1500 Hz
   - Type: Air displacement sound
   - Purpose: Acceleration cue
   - Volume: 0.7

2. **Engine Ignition** (0.2-1.0s)
   - Frequency: 200-400 Hz
   - Type: Rocket engine ignition
   - Purpose: Thruster engagement
   - Volume: 0.6

---

## 8. Ambient Ship Audio Specification

### 8.1 Ambient Layer Strategy

Ambient sounds create background texture and immersion through layered, subtle audio:
- Computer systems beeping and whirring
- Life support air circulation
- Hydraulic system noises
- Cockpit rumble and vibration

### 8.2 Computer Systems Audio

**Sound Cue**: SC_Ambient_ComputerSystems_Loop

**Audio Characteristics**
- Duration: 3-5 seconds looping pattern
- Frequency Range: 500-3000 Hz (processing sounds)
- Volume: 0.15-0.2 (very subtle background)
- Type: Abstract synthesized computer sounds
- Modulation: Various electronic bleeps and whirs

**Audio Elements**
- Periodic beeps (randomized timing, 1-2 second intervals)
- Soft whoosh sounds (data processing effect)
- Electronic chirps (system alerts or processing)
- Low-level digital noise (system running)

**Psychoacoustic Effect**
- Creates sense of active computer system
- Establishes technological environment
- Prevents dead silence
- Subconscious awareness of ship systems

### 8.3 Life Support Audio

**Sound Cue**: SC_Ambient_LifeSupport_Loop

**Audio Characteristics**
- Duration: 4-6 seconds looping pattern
- Frequency Range: 100-400 Hz (air circulation)
- Volume: 0.1-0.15 (subtle background)
- Type: White noise base with occasional mechanical elements
- Modulation: Subtle breathing-like rhythm

**Audio Elements**
- Base air circulation white noise (low volume, continuous)
- Periodic air pump sounds (every 2-3 seconds)
- Subtle valve operations (random timing)
- Humidity sensor calibration clicks (rare)

**Psychoacoustic Effect**
- Suggests life support system actively working
- Creates sense of breathable environment
- Establishes subtle danger (system must remain functional)
- Adds depth to cockpit ambience

### 8.4 Hydraulics Audio

**Sound Cue**: SC_Ambient_Hydraulics_Loop

**Audio Characteristics**
- Duration: 5-7 seconds looping pattern
- Frequency Range: 200-800 Hz (fluid mechanics)
- Volume: 0.08-0.12 (very subtle background)
- Type: Organic mechanical sounds
- Modulation: Occasional pressure release hisses

**Audio Elements**
- Base hydraulic flow sound (continuous, low volume)
- Periodic pump cycles (every 3-5 seconds)
- Pressure relief valve occasional opens
- Fluid movement through pipes
- Mechanical joint movement (rare)

**Psychoacoustic Effect**
- Establishes mechanical ship construction
- Suggests complex systems working
- Creates sense of controlled pressure/forces
- Adds mechanical authenticity

### 8.5 Cockpit Rumble Ambience

**Sound Cue**: SC_Ambient_Cockpit_Rumble

**Audio Characteristics**
- Duration: Continuous (no loop point)
- Frequency Range: 30-100 Hz (deep rumble)
- Volume: 0.12-0.18 (subtle but felt)
- Type: Low-frequency engine/vibration background
- Modulation: Gentle variations to avoid static perception

**Audio Elements**
- Base engine rumble (always present at low volume)
- Periodic vibration peaks (0.5-1.0 second intervals)
- Hull resonance frequencies (natural harmonics)
- Gravitational stress rumble (subtle)

**Psychoacoustic Effect**
- Conveys power and presence of engine
- Felt through headphone haptics/subwoofer
- Creates immersive cockpit environment
- Communicates ship mass and inertia

### 8.6 Ship Type Variations

#### 8.6.1 Light Ship Ambient Variant

**Sound Cue**: SC_Ambient_LightShip_Variation

**Characteristics**
- All ambient frequencies shifted up by 30% (higher pitch)
- All ambient volumes reduced by 20% (more subtle)
- Faster mechanical rhythms (quicker cycling)
- Electronic tone emphasis (less mechanical)
- Overall impression: Nimble, responsive, efficient

**Modifications to Base Loops**
- Computer systems: Higher frequency beeps, faster chirps
- Life support: Higher-pitched air circulation, quicker pump cycles
- Hydraulics: Higher-frequency flow sounds, faster pressure cycles
- Cockpit: Higher-frequency rumble, lighter felt impact

#### 8.6.2 Heavy Ship Ambient Variant

**Sound Cue**: SC_Ambient_HeavyShip_Variation

**Characteristics**
- All ambient frequencies shifted down by 20% (lower pitch)
- All ambient volumes increased by 20% (more prominent)
- Slower mechanical rhythms (deliberate cycling)
- Mechanical tone emphasis (more industrial)
- Overall impression: Powerful, substantial, ponderous

**Modifications to Base Loops**
- Computer systems: Lower-frequency beeps, slower chirps
- Life support: Lower-pitched air circulation, slower pump cycles
- Hydraulics: Lower-frequency flow sounds, slower pressure cycles
- Cockpit: Lower-frequency rumble, heavier felt impact

### 8.7 Ambient Audio Implementation

```cpp
class UAmbientShipAudioController : public UActorComponent
{
private:
    UPROPERTY(EditAnywhere)
    UAudioComponent* ComputerSystemsAudio;

    UPROPERTY(EditAnywhere)
    UAudioComponent* LifeSupportAudio;

    UPROPERTY(EditAnywhere)
    UAudioComponent* HydraulicsAudio;

    UPROPERTY(EditAnywhere)
    UAudioComponent* CockpitRumbleAudio;

    enum class EShipSize : uint8
    {
        Light,
        Medium,
        Heavy
    };

public:
    void InitializeAmbientAudio(EShipSize ShipSize);
    void SetAmbientLevel(float Volume);
    void UpdateAmbientForShipType(EShipSize ShipSize);

private:
    void ApplyLightShipVariant();
    void ApplyHeavyShipVariant();
};
```

---

## 9. Environmental Audio Specification

### 9.1 Environmental Interaction Sounds

### 9.2 Atmospheric Entry Rumble

**Trigger Condition**: Ship enters planet atmosphere (if applicable to game design)

**Audio Characteristics**
- Sound Cue: SC_Environment_AtmosphericEntry
- Duration: 5-10 seconds (atmospheric entry duration)
- Frequency Range: 100-400 Hz (air friction rumble)
- Volume: 0.4-0.8 (prominent but not overwhelming)
- Type: Roaring atmospheric friction sound
- Modulation: Volume increases then decreases as entry progresses

**Audio Layers**
1. **Atmospheric Friction** (main content)
   - Base frequency: 200-300 Hz
   - Modulation: Turbulent air effects
   - Type: Organic roaring sound

2. **Heat Shimmer** (high frequency content)
   - Frequency: 3000-5000 Hz
   - Type: Crackling/shimmering texture
   - Purpose: Heat effect

**Psychoacoustic Effect**
- Roaring suggests extreme air friction and temperature
- Duration communicates extended entry process
- Conveys sense of danger and intensity

### 9.3 Asteroid Pass Doppler

**Trigger Condition**: Asteroid passes ship with significant relative velocity

**Audio Characteristics**
- Sound Cue: SC_Environment_AsteroidPass
- Duration: 2-4 seconds (pass duration)
- Type: Doppler effect whoosh
- Frequency Profile: High frequency → Low frequency (doppler shift)

**Doppler Implementation**
```cpp
void OnAsteroidPass(FVector AsteroidPosition, FVector AsteroidVelocity, float AsteroidSize)
{
    // Calculate relative velocity
    FVector RelativeVelocity = AsteroidVelocity - GetShipVelocity();
    float RelativeSpeed = RelativeVelocity.Length();

    // Calculate doppler shift
    const float SpeedOfSound = 343.0f; // m/s
    float DopplerRatio = SpeedOfSound / (SpeedOfSound + RelativeSpeed);

    // Start whoosh sound with frequency shift
    UAudioComponent* DopplerAudio = GetEnvironmentAudioComponent();
    DopplerAudio->SetSound(SC_Environment_AsteroidPass);

    // Frequency modulation based on doppler ratio
    float BaseFrequency = 1000.0f;
    float ShiftedFrequency = BaseFrequency * DopplerRatio;
    DopplerAudio->SetFloatParameter(FName("Frequency"), ShiftedFrequency);

    // Volume based on asteroid size and distance
    float DistanceToAsteroid = FVector::Dist(GetActorLocation(), AsteroidPosition);
    float Volume = CalculateAttenuatedVolume(AsteroidSize, DistanceToAsteroid);
    DopplerAudio->SetVolumeMultiplier(Volume);

    DopplerAudio->Play();
}
```

**Psychoacoustic Effect**
- Frequency shift creates sense of motion
- Communicates object approaching and passing
- Increases immersion in space environment
- Useful navigation cue (asteroid location)

---

## 10. Audio Technical Architecture

### 10.1 Audio Manager System

The audio system is controlled through a central Audio Manager that coordinates all audio playback:

```cpp
class AAudioManager : public AActor
{
public:
    // Singleton pattern
    static AAudioManager* GetInstance();

    // Audio component management
    void RegisterAudioComponent(FName ComponentName, UAudioComponent* Component);
    void UnregisterAudioComponent(FName ComponentName);

    // Master volume control
    void SetMasterVolume(float Volume);
    void SetSoundClassVolume(ESoundClass SoundClass, float Volume);

    // Spatial audio
    void UpdateListenerPosition(FVector ListenerPosition, FVector ListenerForward);

    // Audio effects
    void PlaySoundEffect(FName EffectName, FVector Location);
    void PlayUISound(FName SoundName);

private:
    UPROPERTY()
    TMap<FName, UAudioComponent*> RegisteredComponents;

    UPROPERTY(EditAnywhere)
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere)
    TMap<ESoundClass, float> SoundClassVolumes;
};
```

### 10.2 Sound Cue System

Each sound cue encapsulates audio logic:

```cpp
// In Unreal Engine, Sound Cues are created in the Content Browser
// They define:
// - Source audio files
// - Attenuation
// - Concurrency limits
// - Parameter modulation
// - Randomization
// - Mixing

// Example C++ reference:
class AShip : public APawn
{
private:
    UPROPERTY(EditAnywhere, Category = "Audio")
    class USoundCue* SC_Engine_Idle;

    UPROPERTY(EditAnywhere, Category = "Audio")
    class USoundCue* SC_Engine_Thrust_Loop;

    // ... other sound cue references
};
```

### 10.3 Audio Component Strategy

**Main Ship Audio Component Hierarchy**

```cpp
class AShip : public APawn
{
private:
    // Primary audio component for engine
    UPROPERTY(VisibleAnywhere, Category = "Audio")
    UAudioComponent* EngineAudioComponent;

    // Thruster audio components (one per thruster location)
    UPROPERTY(VisibleAnywhere, Category = "Audio")
    TArray<UAudioComponent*> ThrusterAudioComponents;

    // Hull and damage audio
    UPROPERTY(VisibleAnywhere, Category = "Audio")
    UAudioComponent* HullAudioComponent;

    // Shield audio
    UPROPERTY(VisibleAnywhere, Category = "Audio")
    UAudioComponent* ShieldAudioComponent;

    // Ambient audio
    UPROPERTY(VisibleAnywhere, Category = "Audio")
    UAudioComponent* AmbientAudioComponent;

    // UI audio (attached to player controller, not ship)
    // Handled separately in HUD/Menu system

public:
    void InitializeAudioComponents();
    void UpdateAudioParameters(float DeltaTime);
};
```

### 10.4 Parameter Modulation System

Audio parameters update in real-time based on game state:

```cpp
class UAudioParameterController : public UActorComponent
{
public:
    // Engine parameters
    void SetEngineFrequency(float Frequency);
    void SetEngineVolume(float Volume);
    void SetEnginePower(float PowerLevel);

    // Thruster parameters
    void SetThrusterIntensity(FName ThrusterName, float Intensity);
    void SetThrusterType(EShipType ShipType);

    // Damage parameters
    void SetHullIntegrity(float Integrity);
    void SetShieldStrength(float Strength);

private:
    UPROPERTY()
    TMap<FName, UAudioComponent*> AudioComponents;

    // Parameter update rate (typically 60 Hz)
    float ParameterUpdateRate = 0.016f;
    float TimeSinceLastUpdate = 0.0f;
};
```

### 10.5 Spatial Audio Processing

**3D Audio Positioning**

```cpp
void UAudioComponent::Update3DSpatialization(
    FVector ListenerPosition,
    FVector ListenerForward,
    FVector SourcePosition
)
{
    // Calculate audio properties based on position
    FVector ToSource = SourcePosition - ListenerPosition;
    float Distance = ToSource.Length();

    // Attenuation (distance-based volume)
    float AttenuationVolume = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, 5000.0f),    // Distance range (cm)
        FVector2D(1.0f, 0.0f),       // Volume range
        Distance
    );

    // Panning (left/right positioning)
    FVector Right = FVector::CrossProduct(ListenerForward, FVector::UpVector);
    float PanValue = FVector::DotProduct(ToSource.GetSafeNormal(), Right);

    // HRTF processing is handled by Unreal's spatial audio system
    SetVolumeMultiplier(AttenuationVolume);
    SetPan(PanValue);
}
```

### 10.6 Audio Mixing and Ducking

**Sound Class Routing**

```cpp
// Master Volume Control
MasterVolume (1.0)
├── Engine (0.4)
│   ├── EngineAudioComponent
│   └── Other engine sounds
├── UI (0.2)
│   └── All UI sounds with auto-ducking
├── Effects (0.2)
│   ├── Thrusters
│   ├── Impacts
│   └── Shield hits
├── Ambient (0.1)
│   └── All ambient loops
└── Music (0.1)
    └── Dynamic music system
```

**Ducking Implementation**

```cpp
class UAudioDuckingManager : public UActorComponent
{
public:
    void DuckAudio(ESoundClass ClassToDuck, float DuckAmount, float Duration);
    void RestoreAudio(ESoundClass RestoredClass);

private:
    void UpdateDuckingLevel(float DeltaTime);

    UPROPERTY()
    TMap<ESoundClass, float> CurrentDuckLevels;

    UPROPERTY()
    TMap<ESoundClass, float> TargetDuckLevels;
};
```

---

## 11. Integration with Ship Customization

### 11.1 Audio Integration Points

Ship customization affects audio through direct stat modulation:

#### 11.1.1 Thrust Power Stat Integration

```cpp
class UShipCustomizationComponent : public UActorComponent
{
private:
    UPROPERTY(EditAnywhere, Category = "Stats")
    FShipStats CurrentStats;

public:
    void OnShipStatsChanged(const FShipStats& NewStats)
    {
        CurrentStats = NewStats;
        ApplyStatsToAudio();
    }

private:
    void ApplyStatsToAudio()
    {
        AShip* Ship = Cast<AShip>(GetOwner());
        if (!Ship) return;

        // Engine audio adjustment based on ThrustPower
        float BaseEngineFrequency = FMath::GetMappedRangeValueClamped(
            FVector2D(0.5f, 2.0f),    // ThrustPower range
            FVector2D(200.0f, 500.0f), // Base frequency range
            CurrentStats.ThrustPower
        );

        UAudioComponent* EngineAudio = Ship->GetEngineAudio();
        EngineAudio->SetFloatParameter(FName("BaseFrequency"), BaseEngineFrequency);

        // Engine volume adjustment
        float EngineVolumeMultiplier = FMath::GetMappedRangeValueClamped(
            FVector2D(0.5f, 2.0f),    // ThrustPower range
            FVector2D(0.8f, 1.2f),    // Volume multiplier range
            CurrentStats.ThrustPower
        );

        EngineAudio->SetVolumeMultiplier(EngineVolumeMultiplier);
    }
};
```

#### 11.1.2 Mass Stat Integration

```cpp
void ApplyMassToAudio(float ShipMass)
{
    // Mass affects engine frequency (heavier = lower frequency base)
    float MassNormalized = FMath::GetMappedRangeValueClamped(
        FVector2D(200.0f, 2000.0f), // Mass range (kg)
        FVector2D(-100.0f, -50.0f), // Frequency offset
        ShipMass
    );

    UAudioComponent* EngineAudio = GetEngineAudio();
    EngineAudio->SetFloatParameter(FName("MassOffset"), MassNormalized);

    // Mass affects collision impact severity
    float CollisionVolumeMultiplier = 1.0f + (ShipMass / 2000.0f);
    UAudioComponent* ImpactAudio = GetCollisionAudio();
    ImpactAudio->SetFloatParameter(FName("MassInfluence"), CollisionVolumeMultiplier);
}
```

#### 11.1.3 Agility/Maneuverability Stat Integration

```cpp
void ApplyAgilityToAudio(float AgilityRating)
{
    // Agility affects thruster sound character
    // Higher agility = higher-pitched, faster thrusters

    for (UAudioComponent* ThrusterAudio : ThrusterAudioComponents)
    {
        float ThrusterType = AgilityRating; // 0.0 = Heavy, 1.0 = Agile
        ThrusterAudio->SetFloatParameter(FName("ThrusterType"), ThrusterType);

        // Agility also affects spool-up/down times
        float SpoolUpTime = FMath::GetMappedRangeValueClamped(
            FVector2D(0.0f, 1.0f),   // Agility range
            FVector2D(0.8f, 0.3f),   // Spool time range
            AgilityRating
        );

        ThrusterAudio->SetFloatParameter(FName("SpoolUpDuration"), SpoolUpTime);
    }
}
```

#### 11.1.4 Shield Power Stat Integration

```cpp
void ApplyShieldStatsToAudio(float ShieldPowerStat)
{
    // Shield power affects idle hum frequency and base strength

    float IdleHumFrequency = FMath::GetMappedRangeValueClamped(
        FVector2D(0.5f, 2.0f),      // ShieldPower range
        FVector2D(800.0f, 1500.0f), // Frequency range
        ShieldPowerStat
    );

    UAudioComponent* ShieldAudio = GetShieldAudio();
    ShieldAudio->SetFloatParameter(FName("IdleFrequency"), IdleHumFrequency);

    // Max shield capacity affects peak impact sound volume
    float MaxShieldVolume = FMath::Clamp(ShieldPowerStat * 0.5f, 0.2f, 1.0f);
    ShieldAudio->SetFloatParameter(FName("MaxImpactVolume"), MaxShieldVolume);
}
```

### 11.2 Real-Time Audio Update Loop

```cpp
void AShip::UpdateAudioSystem(float DeltaTime)
{
    // Update engine audio
    UpdateEngineAudio(GetThrustInput(), CurrentStats.ThrustPower, CurrentStats.Mass);

    // Update thruster audio
    for (int32 i = 0; i < ActiveThrusters.Num(); ++i)
    {
        UpdateThrusterAudio(ActiveThrusters[i], i);
    }

    // Update hull audio based on damage
    UpdateHullAudio(CurrentStats.HullIntegrity);

    // Update shield audio based on shield status
    UpdateShieldAudio(CurrentStats.ShieldStrength);

    // Update ambient audio for variations
    UpdateAmbientAudio(CurrentStats);
}

void AShip::UpdateEngineAudio(float ThrustInput, float ThrustPower, float Mass)
{
    if (!EngineAudioComponent) return;

    // Calculate target frequency
    float TargetFrequency = CalculateEngineFrequency(ThrustInput, ThrustPower);

    // Smoothly interpolate to target (prevents abrupt changes)
    CurrentEngineFrequency = FMath::FInterpTo(
        CurrentEngineFrequency,
        TargetFrequency,
        GetWorld()->DeltaTimeSeconds,
        5.0f // Interpolation speed
    );

    // Apply to audio component
    EngineAudioComponent->SetFloatParameter(FName("Frequency"), CurrentEngineFrequency);

    // Update volume
    float TargetVolume = CalculateEngineVolume(ThrustInput, ThrustPower, Mass);
    CurrentEngineVolume = FMath::FInterpTo(
        CurrentEngineVolume,
        TargetVolume,
        GetWorld()->DeltaTimeSeconds,
        3.0f
    );
    EngineAudioComponent->SetVolumeMultiplier(CurrentEngineVolume);
}
```

### 11.3 Customization Menu Audio Feedback Loop

```cpp
class AShipCustomizationMenu : public AActor
{
public:
    void OnPartEquipped(UShipPart* Part)
    {
        // Play equip sound
        PlayUISound(SC_UI_PartEquip_Complete);

        // Update ship preview audio immediately
        UpdatePreviewAudio();
    }

    void OnLoadoutApplied(UShipLoadout* Loadout)
    {
        // Play loadout switch sound
        PlayUISound(SC_UI_LoadoutSwitch_Complete);

        // Apply audio characteristics of new loadout
        ApplyLoadoutAudio(Loadout);
    }

private:
    void UpdatePreviewAudio()
    {
        // Recalculate all audio parameters based on current customization
        FShipStats PreviewStats = CalculatePreviewStats();
        ApplyStatsToPreviewAudio(PreviewStats);
    }

    void ApplyLoadoutAudio(UShipLoadout* Loadout)
    {
        // Apply full audio configuration for this loadout
        ApplyMassToAudio(Loadout->Stats.Mass);
        ApplyThrustPowerToAudio(Loadout->Stats.ThrustPower);
        ApplyAgilityToAudio(Loadout->Stats.Agility);
        ApplyShieldStatsToAudio(Loadout->Stats.ShieldPower);
    }
};
```

---

## 12. VR Audio Guidelines

### 12.1 Spatial Audio in VR

#### 12.1.1 HRTF (Head-Related Transfer Function)

VR spatial audio uses HRTF to position sounds in 3D space:

**HRTF Processing**
- Mimics how human ears perceive direction
- Creates elevation and azimuth cues
- Processed by Unreal's spatial audio system
- Requires binaural audio (stereo headphones)

**Implementation in Unreal**

```cpp
// Audio component already supports 3D spatialization
UAudioComponent* ThrusterAudio = ThrusterAudioComponents[0];

// Enable spatial audio
ThrusterAudio->bUseAttenuation = true;
ThrusterAudio->bEnableStereoPanning = true;
ThrusterAudio->bEnableFocusedEcho = false; // Disable if immersion is priority

// Set 3D attenuation
FAttenuationSettings AttenuationSettings;
AttenuationSettings.Falloff = EAttenuationDistanceModel::Linear;
AttenuationSettings.MaxDistance = 5000.0f; // 50 meters
AttenuationSettings.MinDistance = 100.0f;  // 1 meter

ThrusterAudio->AttenuationSettings = AttenuationSettings;
```

#### 12.1.2 Doppler Effect

Real-time doppler shifts enhance spatial immersion:

```cpp
void CalculateDopplerShift(
    FVector ListenerPosition,
    FVector ListenerVelocity,
    FVector SourcePosition,
    FVector SourceVelocity,
    float& OutPitchShift
)
{
    const float SpeedOfSound = 34300.0f; // cm/s in air

    FVector ToSource = SourcePosition - ListenerPosition;
    float Distance = ToSource.Length();

    if (Distance < 0.1f) return; // Prevent division issues

    // Relative velocity along line to source
    FVector DirectionToSource = ToSource / Distance;
    float ListenerVelocityToward = FVector::DotProduct(
        ListenerVelocity,
        DirectionToSource
    );
    float SourceVelocityToward = FVector::DotProduct(
        SourceVelocity,
        DirectionToSource
    );

    // Doppler formula
    float DopplerRatio = (SpeedOfSound - ListenerVelocityToward) /
                        (SpeedOfSound - SourceVelocityToward);

    // Clamp to reasonable range
    DopplerRatio = FMath::Clamp(DopplerRatio, 0.5f, 1.5f);

    OutPitchShift = DopplerRatio;
}
```

### 12.2 Immersion Through Audio

#### 12.2.1 Preventing Audio Fatigue

VR audio should enhance immersion without causing listener fatigue:

**Fatigue Prevention Strategies**
- Avoid sudden loud sounds (max 0.9 volume for UI)
- Vary ambient frequencies to prevent tinnitus-like effects
- Use modulation to prevent static-sounding loops
- Limit high-frequency content in long-duration sounds
- Provide audio leveling for unpredictable events

**Implementation**

```cpp
class UAudioFatigueManager : public UActorComponent
{
private:
    // Track recent loud sounds
    float LastLoudSoundTime = 0.0f;
    float MinTimeBetweenLoud = 2.0f;

public:
    bool CanPlayLoudSound()
    {
        float TimeSinceLoud = GetWorld()->GetTimeSeconds() - LastLoudSoundTime;
        return TimeSinceLoud >= MinTimeBetweenLoud;
    }

    void PlayAudio(float Volume)
    {
        if (Volume > 0.7f && !CanPlayLoudSound())
        {
            // Reduce volume to prevent fatigue
            Volume = 0.7f;
        }
        LastLoudSoundTime = GetWorld()->GetTimeSeconds();
    }
};
```

#### 12.2.2 Discomfort Prevention

Certain audio characteristics can cause discomfort:

**Comfort Guidelines**
- Avoid 10-20 Hz frequencies (sub-bass can cause discomfort)
- Limit bass content in highly directional sounds
- Avoid sudden panning (smooth transitions preferred)
- Use smooth frequency sweeps (no harsh discontinuities)
- Prevent audio clipping (maintain -6dB headroom)

**Discomfort Monitoring**

```cpp
bool IsAudioComfortable(const FAudioSpec& Audio)
{
    // Check frequency range
    if (Audio.MinFrequency < 10.0f || Audio.MinFrequency > 20.0f)
    {
        // Sub-bass may cause discomfort
        return false;
    }

    // Check for clipping
    if (Audio.PeakVolume > 0.95f)
    {
        return false;
    }

    // Check for harsh frequency transitions
    if (Audio.MaxFrequencyDelta > 200.0f) // Hz per frame
    {
        return false;
    }

    return true;
}
```

### 12.3 Headphone Optimization

#### 12.3.1 Headphone-Specific Mixing

VR audio is delivered entirely through headphones:

**Headphone Mixing Adjustments**
- Increase clarity in 2-4 kHz range (vocal/presence peak)
- Reduce extreme frequency content (< 50 Hz, > 12 kHz)
- Optimize for binaural reproduction
- Account for headphone frequency response

**EQ Profile for VR Headphones**

```cpp
class UVRAudioEQProfile : public UActorComponent
{
public:
    // Apply EQ for optimal VR headphone experience
    void ApplyVRHeadphoneEQ(UAudioComponent* AudioComponent)
    {
        // Example EQ adjustments
        // (Would be applied through Unreal's audio effects system)

        // +3dB at 2kHz (presence/clarity)
        // -3dB at 100Hz (reduce sub-bass rumble)
        // -6dB at 16kHz (reduce harshness)

        // Implementation would use Unreal's Spectral Analyzer
        // and parametric EQ effects
    }
};
```

#### 12.3.2 Headphone Leakage Prevention

Sound bleed from headphones is undesirable:

**Leakage Prevention**
- Keep maximum peak levels below 0.9
- Avoid extreme frequency content (< 50 Hz especially)
- Use compression on dynamic sounds
- Ensure consistent loudness across audio

### 12.4 Motion Sickness Mitigation

Audio can contribute to VR motion sickness:

#### 12.4.1 Audio-Visual Synchronization

Delays between audio and visual events can cause discomfort:

```cpp
// Ensure tight audio-visual sync
class AAudioVisualSyncManager : public UActorComponent
{
public:
    // Maximum acceptable latency: 50ms (0.05s)
    static constexpr float MaxAcceptableLatency = 0.05f;

    void PlaySyncedAudioVisual(
        UAudioComponent* AudioComponent,
        const FVector& VisualLocation,
        float MaxLatency = MaxAcceptableLatency
    )
    {
        // Ensure audio and visual events occur within 50ms
        float AudioLatency = AudioComponent->GetAudioLatency();

        if (AudioLatency > MaxLatency)
        {
            // Warn developer about potential sync issues
            UE_LOG(LogAudio, Warning, TEXT("Audio-visual sync latency exceeded: %f ms"),
                   AudioLatency * 1000.0f);
        }

        AudioComponent->Play();
    }
};
```

#### 12.4.2 Vestibular System Considerations

Audio movement cues should align with visual/physical motion:

**Guidelines**
- Doppler effects should match visual motion
- Panning should correspond to visual direction
- Frequency shifts should align with visual pitch changes
- Avoid contradictory audio-visual cues

### 12.5 Spatial Audio Testing

#### 12.5.1 HRTF Quality Verification

```cpp
class UAudioSpatialQualityTest : public UActorComponent
{
public:
    // Test spatial positioning accuracy
    void TestSpatialAudio()
    {
        // Create test sounds at various positions
        // Verify perceived direction matches intended position

        // Test positions: front, back, left, right, up, down
        TArray<FVector> TestPositions = {
            FVector(0, 0, 0),       // Center (reference)
            FVector(1000, 0, 0),    // Front
            FVector(-1000, 0, 0),   // Back
            FVector(0, 1000, 0),    // Left
            FVector(0, -1000, 0),   // Right
            FVector(0, 0, 1000),    // Up
            FVector(0, 0, -1000),   // Down
        };

        for (const FVector& Position : TestPositions)
        {
            PlayTestTone(Position);
            // Verify perceived position through user feedback
        }
    }
};
```

---

## 13. Audio Asset List

### 13.1 Engine Sound Assets

**Engine Idle Sounds (2 variations)**
- SC_Engine_Idle_Base: Base idle rumble
- SC_Engine_Idle_Variant: Variant with more wobble

**Engine Thrust Sounds (3 variations)**
- SC_Engine_Thrust_Light: Quiet thrust (light ships)
- SC_Engine_Thrust_Medium: Normal thrust
- SC_Engine_Thrust_Heavy: Powerful thrust (heavy ships)

**Engine Transition Sounds (2)**
- SC_Engine_SpoolUp: Spool-up whoosh
- SC_Engine_SpoolDown: Spool-down coast

**Engine Variant Sounds (2)**
- SC_Engine_HighPower: Aggressive powerful variant
- SC_Engine_LowPower: Whiny efficient variant

**Engine Special Effects (2)**
- SC_Engine_Stall: Engine stall sound
- SC_Engine_Shutdown: Engine powering down

### 13.2 Thruster Sound Assets

**Thruster Fire Sounds (4)**
- SC_Thruster_Fire_Main: Primary thruster burst
- SC_Thruster_Fire_Agile: Light, responsive thruster
- SC_Thruster_Fire_Heavy: Heavy, powerful thruster
- SC_Thruster_Fire_RCS: Small RCS thruster

**Thruster Burst Sounds (2)**
- SC_Thruster_Burst_Short: 0.1s burst
- SC_Thruster_Burst_Long: 0.3s sustained burn

**Thruster Variants (2)**
- SC_Thruster_Dual_Opposing: Two thrusters firing
- SC_Thruster_Multi_Fire: Multiple thrusters coupling

### 13.3 UI Sound Assets

**Selection/Navigation Sounds (3)**
- SC_UI_PartSelect_Click: Selection click
- SC_UI_Hover_Subtle: Hover feedback
- SC_UI_Navigate: Menu navigation whoosh

**Equip/Unequip Sounds (2)**
- SC_UI_PartEquip_Complete: Installation sequence
- SC_UI_PartUnequip_Complete: Removal sequence

**Loadout Sounds (1)**
- SC_UI_LoadoutSwitch_Complete: Configuration switch

**Purchase Sounds (2)**
- SC_UI_PurchasePart_Success: Successful purchase
- SC_UI_PurchasePart_Fail: Purchase denied

**Achievement Sounds (2)**
- SC_UI_UnlockPart_Success: Part unlock fanfare
- SC_UI_LevelUp_Fanfare: Level up celebration

**Error/Cancel Sounds (2)**
- SC_UI_InsufficientCredits_Error: Not enough credits
- SC_UI_Cancel: Cancel/back action

### 13.4 Hull and Damage Sound Assets

**Creaking Sounds (2)**
- SC_Hull_Creaking_Loop_Light: Light structural stress
- SC_Hull_Creaking_Loop_Heavy: Severe structural damage

**Impact Sounds (3)**
- SC_Collision_Impact_Light: Minor collision
- SC_Collision_Impact_Medium: Moderate collision
- SC_Collision_Impact_Heavy: Severe collision

**Damage Alert Sounds (2)**
- SC_Hull_Breach_Warning: Hull breach alarm
- SC_Hull_Integrity_Critical: Critical damage alarm

**Additional Hull Sounds (1)**
- SC_Hull_Damage_Pop: Sharp damage event

### 13.5 Shield Sound Assets

**Shield State Sounds (2)**
- SC_Shield_Activate: Shield powering up
- SC_Shield_Depleted: Shield power-down + warning

**Shield Impact Sounds (3)**
- SC_Shield_Hit_Light: Minor shield impact
- SC_Shield_Hit_Medium: Moderate shield impact
- SC_Shield_Hit_Heavy: Severe shield impact

**Shield Maintenance Sounds (2)**
- SC_Shield_Idle_Hum: Shield active hum (looping)
- SC_Shield_Recharge_Begin: Recharge start

**Shield Recharge Sounds (1)**
- SC_Shield_Recharge_Loop: Recharge in progress (looping)

### 13.6 Weapon Sound Assets

*Note: These are optional for current iteration, included for future combat implementation.*

**Laser Weapons (2)**
- SC_Weapon_Laser_Fire: Laser beam discharge
- SC_Weapon_Laser_Charge: Laser charge build-up

**Railgun Weapons (2)**
- SC_Weapon_Railgun_Fire: Railgun discharge
- SC_Weapon_Railgun_Charge: Railgun charge build-up

**Missile Weapons (2)**
- SC_Weapon_Missile_Launch: Missile launch
- SC_Weapon_Missile_Charge: Missile guidance lock-on

**Weapon System Sounds (3)**
- SC_Weapon_Cooldown: Weapon cooling loop
- SC_Weapon_Overheat_Warning: Overheat alarm
- SC_Weapon_Overheat_Critical: Critical overheating

**Special Weapon Effects (1)**
- SC_Weapon_Reload: Magazine reload sound

### 13.7 Ambient Sound Assets

**Ship Systems (4)**
- SC_Ambient_ComputerSystems_Loop: Computer beeps/processing
- SC_Ambient_LifeSupport_Loop: Air circulation/pumps
- SC_Ambient_Hydraulics_Loop: Hydraulic fluid/pressure
- SC_Ambient_Cockpit_Rumble: Engine rumble/vibration

**Ship Type Variants (2)**
- SC_Ambient_LightShip_Variation: Light ship modifications
- SC_Ambient_HeavyShip_Variation: Heavy ship modifications

**Additional Ambient (2)**
- SC_Ambient_Hum_Baseline: General ship hum
- SC_Ambient_Electronic_Chatter: Random electronic chatter

### 13.8 Environmental Sound Assets

**Atmospheric Sounds (1)**
- SC_Environment_AtmosphericEntry: Atmospheric entry rumble

**Space Environmental (1)**
- SC_Environment_AsteroidPass: Asteroid pass doppler

**Additional Environmental (3)**
- SC_Environment_SolarFlare: Solar radiation sound
- SC_Environment_MusicExploration: Exploration music cue
- SC_Environment_MusicCombat: Combat music cue

### 13.9 Total Asset Count

**Sound Cues**: 68+ total
- Engine: 8
- Thrusters: 6
- UI: 15
- Hull/Damage: 8
- Shield: 8
- Weapons: 10
- Ambient: 8
- Environmental: 5

**Audio Files Required**: 80-100 source files (accounting for multiple takes/variations)

**Estimated Asset Size**: 500-800 MB (at 48kHz 16-bit stereo for editing, compressed to 200-300 MB in-engine)

---

## 14. Testing and Iteration

### 14.1 Audio Quality Verification

#### 14.1.1 Technical Specifications Checklist

```
Engine Audio:
- [ ] Idle frequency range: 40-60 Hz verified
- [ ] Thrust frequency range: 100-500 Hz verified
- [ ] Spool-up time: 0.5s confirmed
- [ ] Spool-down time: 1.0s confirmed
- [ ] Volume response to thrust input verified
- [ ] Frequency response to ThrustPower stat verified

Thruster Audio:
- [ ] Fire duration: 0.1-0.3s verified
- [ ] Spatial positioning accurate (L/R/F/B)
- [ ] Pitch variation ±5% working correctly
- [ ] Volume reduction for multiple concurrent thrusters working
- [ ] Thruster type differentiation audible (Agile vs Heavy)

UI Audio:
- [ ] All UI sounds < 0.8 volume (no fatigue)
- [ ] Part equip sequence timing: 0.5s
- [ ] Level-up fanfare duration: 3.0s
- [ ] All UI sounds distinct and recognizable
- [ ] Audio-visual sync within 50ms

Hull/Damage Audio:
- [ ] Creaking starts at < 50% integrity
- [ ] Creaking intensity scales with damage
- [ ] Collision impact scales with velocity
- [ ] Hull breach alarm properly prioritized
- [ ] Impact sounds spatially relevant

Shield Audio:
- [ ] Shield activation duration: 1.0s
- [ ] Idle hum volume scales with strength (0.1-0.3)
- [ ] Impact sounds vary by damage absorbed
- [ ] Recharge progression audible and distinct
- [ ] Depletion warning clear and not alarming

Ambient Audio:
- [ ] All ambient loops seamless
- [ ] Ship type variations clearly distinct
- [ ] Ambient doesn't mask important audio
- [ ] Frequency content appropriate for VR
```

#### 14.1.2 Perceptual Testing

```
Audio Perception Checklist:
- [ ] Engine sounds feel responsive to input
- [ ] Thruster positions are clearly locatable
- [ ] Shield status understood through audio
- [ ] Damage state clearly indicated
- [ ] No audio fatigue after 30-minute session
- [ ] All audio enhances immersion (not distracting)
- [ ] Audio helps navigation (spatial cues work)
- [ ] No audio-visual sync issues
- [ ] Headphone comfort verified
```

### 14.2 Iteration Process

#### 14.2.1 Audio Mixing Refinement

1. **Initial Mix**: Establish baseline levels
   - Engine: 0.4 master volume
   - Thrusters: 0.4 master volume
   - UI: 0.2 master volume
   - Ambient: 0.1 master volume
   - Music: 0.1 master volume

2. **Balance Testing**: Play through typical gameplay loop
   - Adjust individual component volumes
   - Test ducking behavior during UI sounds
   - Verify no component masks critical audio

3. **Context Testing**: Test audio in different situations
   - Near asteroids (environmental audio)
   - With shield active (shield hum present)
   - With heavy damage (creaking + alarms)
   - During intense combat (overlapping sounds)

#### 14.2.2 Parameter Tuning

```cpp
// Audio parameters can be tweaked in real-time
class FAudioParameterTuner
{
public:
    // Engine parameters
    UPROPERTY(EditAnywhere, Category = "Engine Audio")
    float IdleFrequency = 50.0f;

    UPROPERTY(EditAnywhere, Category = "Engine Audio")
    float ThrustFrequencyMin = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Engine Audio")
    float ThrustFrequencyMax = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Engine Audio")
    float SpoolUpDuration = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Engine Audio")
    float SpoolDownDuration = 1.0f;

    // Thruster parameters
    UPROPERTY(EditAnywhere, Category = "Thruster Audio")
    float AgileFrequencyMin = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Thruster Audio")
    float AgileFrequencyMax = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Thruster Audio")
    float HeavyFrequencyMin = 150.0f;

    UPROPERTY(EditAnywhere, Category = "Thruster Audio")
    float HeavyFrequencyMax = 300.0f;

    // Apply tuned parameters to active audio
    void ApplyTunedParameters(AAudioManager* AudioManager);
};
```

#### 14.2.3 Player Feedback Integration

- Gather feedback from test players
- Document preferences (too loud/quiet, too high/low frequency, etc.)
- Iterate on specific problem areas
- Re-test after adjustments
- Document final approved parameter set

### 14.3 Performance Optimization

#### 14.3.1 Audio Playback Limits

```cpp
// Prevent audio performance issues
class UAudioPerformanceManager : public UActorComponent
{
private:
    // Maximum concurrent audio channels
    static constexpr int32 MaxEngineChannels = 2;
    static constexpr int32 MaxThrusterChannels = 8;
    static constexpr int32 MaxUIChannels = 4;
    static constexpr int32 MaxAmbientChannels = 5;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

public:
    bool CanPlayNewSound(ESoundCategory Category)
    {
        int32 CurrentCount = GetActiveSoundCount(Category);
        int32 MaxAllowed = GetMaxChannelsForCategory(Category);
        return CurrentCount < MaxAllowed;
    }
};
```

#### 14.3.2 Memory Management

- Audio assets stored in optimized format (OGG/Opus)
- Streaming audio for long-duration sounds (ambient loops)
- Caching frequently-used sounds (UI, engine idle)
- Unloading unused loadout-specific audio

---

## Conclusion

This comprehensive audio system design provides:

1. **Immersive VR Experience**: Spatial audio positioning and 3D effects
2. **Game State Feedback**: Audio directly reflects ship customization and status
3. **Intuitive Navigation**: Audio cues guide player interaction and awareness
4. **Performance Efficiency**: Optimized audio playback with intelligent ducking
5. **Scalability**: Supports future additions (weapons, combat, etc.)

The modular architecture allows for:
- Independent tuning of each audio category
- Easy addition of new sounds or effects
- Clear integration with game systems
- Straightforward testing and iteration

Audio implementation should proceed in phases:
1. **Phase 1**: Engine and ambient audio (core flight experience)
2. **Phase 2**: Thruster audio (maneuvering feedback)
3. **Phase 3**: UI audio (customization menu)
4. **Phase 4**: Damage/shield audio (combat preparation)
5. **Phase 5**: Weapon audio (future combat iteration)

