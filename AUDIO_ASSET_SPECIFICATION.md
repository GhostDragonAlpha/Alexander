# Audio Asset Specification and Production Guide

Complete specification for all audio assets required for the ship audio feedback system.

---

## 1. Engine Sound Assets

### 1.1 SC_Engine_Idle_Base

**Purpose**: Base idle rumble when engine is on but ship has no thrust input

**Technical Specifications**
- Duration: Infinite loop (seamless 4-8 second loop point)
- Sample Rate: 48 kHz
- Bit Depth: 16-bit (24-bit preferred)
- Format: WAV master, OGG Vorbis in-engine
- Frequency Response: 40-100 Hz dominant
- Peak Level: -6 dB (6 dB headroom)

**Audio Characteristics**
- Frequency Content: Deep rumble, 40-60 Hz fundamental
- Tone: Heavy, mechanical, reliable
- Dynamics: Subtle variation (±5% wobble)
- Modulation: Slow sine wave wobble at 0.5 Hz
- Harmonic Series: Weak overtones (avoid being metallic)

**Production Notes**
- Use subwoofer/bass speaker for capture
- Filter frequencies above 200 Hz to maintain purity
- Add very subtle noise floor for realism
- Create seamless loop point (crossfade first/last 50ms if needed)
- Avoid any clicks or pops at loop point

**Variations/Derivatives**
- Variant with more wobble: SC_Engine_Idle_Variant
- Light ship version: 20% higher in frequency
- Heavy ship version: 20% lower in frequency

---

### 1.2 SC_Engine_Thrust_Medium

**Purpose**: Engine sound during normal thrust application (primary loop sound)

**Technical Specifications**
- Duration: Infinite loop (4-8 second seamless loop)
- Sample Rate: 48 kHz
- Bit Depth: 24-bit recommended
- Format: WAV master, OGG Vorbis in-engine
- Frequency Response: 100-600 Hz dominant
- Peak Level: -6 dB

**Audio Characteristics**
- Base Frequency: Morphs from 100 Hz to 500 Hz based on thrust input
- Tone: Mechanical, powerful, responsive
- Dynamics: Fully dynamic (changes per frame)
- Modulation: Real-time frequency modulation from thrust parameter
- Harmonic Content: Rich 2nd and 3rd harmonics

**Production Strategy**
- Record real turbine/jet engine source material
- Layer multiple frequency-controlled synthesizers
- Create morphing envelope from low to high thrust
- Ensure smooth frequency transitions
- Test with pitch modulation from 100-500 Hz

**Sound Source Options**
- Turbine engine recording (filtered and processed)
- Synthesized engine tone (more controllable)
- Hybrid: Real recording + synth overlay

**Variations**
- SC_Engine_Thrust_Light: Quieter, higher-pitched variant
- SC_Engine_Thrust_Heavy: Deeper, more aggressive variant

---

### 1.3 SC_Engine_SpoolUp

**Purpose**: Transition sound when applying thrust from idle

**Technical Specifications**
- Duration: Exactly 0.5 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Format: WAV master, OGG Vorbis in-engine
- Frequency Content: 50 Hz → 500 Hz sweep
- Peak Level: -3 dB (immediate peak at ~0.2s)

**Audio Characteristics**
- Envelope: Attack (0-0.1s) → Peak (0.1-0.3s) → Tail (0.3-0.5s)
- Frequency Sweep: Exponential rise from 50 Hz to 500 Hz
- Type: Whoosh-like mechanical acceleration sound
- Harmonics: Build up as frequency increases
- End State: Smooth transition to main engine loop

**Production Notes**
- Create frequency sweep using pitch modulation
- Attack should be immediate (engine responds quickly)
- Peak at 0.2s for emphasis
- Tail should not abruptly cut off (natural decay)
- Test concatenation: spool-up → engine thrust loop (should be seamless)

**Quality Standards**
- No clicks or discontinuities
- Frequency sweep must be smooth (no jumps)
- Tail must integrate well with engine thrust loop

---

### 1.4 SC_Engine_SpoolDown

**Purpose**: Engine coast-down sound when reducing thrust

**Technical Specifications**
- Duration: Exactly 1.0 second
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Format: WAV master, OGG Vorbis in-engine
- Frequency Content: 500 Hz → 50 Hz sweep (inverse of spool-up)
- Peak Level: -6 dB (sustained throughout)

**Audio Characteristics**
- Envelope: Start at peak → Gradual decay over 1.0s
- Frequency Sweep: Exponential decay from 500 Hz to 50 Hz
- Type: Mechanical deceleration with resonance
- Harmonics: Gradually diminish as frequency decreases
- End State: Tail fades to silence or engine idle

**Production Notes**
- Create reverse of spool-up sound (not identical, with slight decay)
- More gradual than spool-up (engine coasting down)
- Include resonance effects (engine rings as it slows)
- Smooth transition to idle (if engine stays on)
- Smooth transition to silence (if engine powers down)

**Integration Notes**
- Must work both before idle and after complete shutdown
- Should sound like physical momentum (not abrupt)

---

### 1.5 SC_Engine_HighPower

**Purpose**: Aggressive engine sound for high ThrustPower stat ships

**Technical Specifications**
- Duration: Infinite loop (looping source)
- Sample Rate: 48 kHz
- Bit Depth: 24-bit
- Format: WAV master, OGG Vorbis in-engine
- Frequency Response: 80-600 Hz (deeper baseline than normal)
- Peak Level: -3 dB (more aggressive volume)

**Audio Characteristics**
- Tone: Deep, aggressive, military-grade
- Fundamental: Octave lower than normal engine sound
- Harmonics: Emphasis on 2nd harmonic (bass emphasis)
- Dynamics: More pronounced response to input
- Quality: Heavy, industrial, powerful

**Production Strategy**
- Use larger/heavier turbine sound as source
- Add sub-bass layer (80-100 Hz)
- Emphasize lower harmonics
- Reduce high-frequency content above 3 kHz
- Create more dramatic frequency response

**Psychoacoustic Goals**
- Convey power and force
- Suggest heavier machinery
- Create sense of military capability
- Feel more dangerous/impressive

---

### 1.6 SC_Engine_LowPower

**Purpose**: Efficient engine sound for low ThrustPower stat ships

**Technical Specifications**
- Duration: Infinite loop (looping source)
- Sample Rate: 48 kHz
- Bit Depth: 24-bit
- Format: WAV master, OGG Vorbis in-engine
- Frequency Response: 150-700 Hz (higher baseline)
- Peak Level: -9 dB (quieter, more refined)

**Audio Characteristics**
- Tone: High-pitched, whiny, electronic
- Fundamental: Octave higher than normal engine sound
- Harmonics: Emphasis on 3rd+ harmonics (treble emphasis)
- Dynamics: More responsive, quick acceleration
- Quality: Light, efficient, electrical-sounding

**Production Strategy**
- Use smaller/lighter turbine or electrical motor sound
- Add high-frequency content (3-6 kHz)
- Reduce bass content below 150 Hz
- Create faster transient response
- Electronic synth overlay optional

**Psychoacoustic Goals**
- Convey efficiency and precision
- Suggest lighter machinery/construction
- Create sense of agility
- Feel more refined/technological

---

### 1.7 SC_Engine_Stall

**Purpose**: Engine stall/flameout sound (rare, critical condition)

**Technical Specifications**
- Duration: 0.8 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Format: WAV master, OGG Vorbis in-engine
- Frequency Content: Chaotic, 100-2000 Hz
- Peak Level: -3 dB

**Audio Characteristics**
- Envelope: Normal engine → Sudden drop → Silence
- Type: Mechanical failure, catastrophic
- Dynamics: Sudden change (indicates emergency)
- Harmonics: Chaotic, dissonant
- Duration: Quick (player should recognize immediately)

**Production Notes**
- Record or synthesize turbine spooling down abruptly
- Add mechanical failure sound (pop, crack)
- End with sudden silence (loss of power)
- Should be alarming but not painful to hear

**Use Cases**
- Engine overheat/failure
- Critical damage state
- Emergency shutdown

---

### 1.8 SC_Engine_Shutdown

**Purpose**: Controlled engine shutdown sequence

**Technical Specifications**
- Duration: 1.5 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Format: WAV master, OGG Vorbis in-engine
- Frequency Content: Gradual decay 500 Hz → 0 Hz
- Peak Level: -6 dB

**Audio Characteristics**
- Envelope: Peak → Decay → Silence
- Type: Controlled mechanical shutdown
- Dynamics: Gradual, organized
- Harmonics: Gradually diminish
- End: Complete silence

**Production Notes**
- Should sound like controlled, intentional process
- Slower than stall (not emergency)
- Clean ending (not abrupt)
- Engine components power down sequentially

**Use Cases**
- Landing procedures
- Docking procedures
- Parking mode

---

## 2. Thruster Sound Assets

### 2.1 SC_Thruster_Fire_Main

**Purpose**: Primary RCS thruster burst sound

**Technical Specifications**
- Duration: 0.15 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Format: WAV master, OGG Vorbis in-engine
- Frequency Response: 200-400 Hz dominant
- Peak Level: -3 dB

**Audio Characteristics**
- Attack: Immediate (sharp onset)
- Peak: Sustained at 0.15s
- Decay: Quick tail (50ms)
- Type: Clean burst sound
- Harmonics: Moderate harmonic content

**Production Strategy**
- Synthesize sharp transient
- Use band-limited noise burst
- Create 200-400 Hz tone burst
- Quick attack, quick release
- Test at various pitches (±5% variation)

**Positional Playback**
- Will be spatialized based on thruster location
- Front/back thrusters: Stereo center or slightly directional
- Left/right thrusters: Full stereo panning
- Up/down thrusters: Vertical cues via HRTF

---

### 2.2 SC_Thruster_Fire_Agile

**Purpose**: Light, responsive thruster sound (agile ships)

**Technical Specifications**
- Duration: 0.1 seconds (shorter, snappier)
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Format: WAV master, OGG Vorbis in-engine
- Frequency Response: 350-600 Hz (higher pitched)
- Peak Level: -6 dB (quieter, more delicate)

**Audio Characteristics**
- Attack: Very immediate
- Quality: Electronic, refined
- Tone: High-pitched, responsive
- Type: Quick burst
- Character: Precise, efficient

**Production Notes**
- Use higher frequency source material
- Create very sharp attack transient
- Minimal tail (quick cutoff)
- Electronic synth-like tone
- Test at multiple pitches

**Design Rationale**
- Shorter duration conveys responsiveness
- Higher frequency indicates lighter system
- Lower volume suggests delicate operation

---

### 2.3 SC_Thruster_Fire_Heavy

**Purpose**: Heavy, powerful thruster sound (cargo/heavy ships)

**Technical Specifications**
- Duration: 0.25 seconds (longer, sustained)
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Format: WAV master, OGG Vorbis in-engine
- Frequency Response: 150-300 Hz (lower pitched)
- Peak Level: -3 dB (more powerful)

**Audio Characteristics**
- Attack: Immediate but powerful
- Quality: Mechanical, industrial
- Tone: Low-pitched, commanding
- Type: Substantial burst with resonance
- Character: Powerful, heavy, relentless

**Production Notes**
- Use lower frequency source material
- Create rounded attack transient
- Extended tail with resonance (200ms)
- Mechanical, industrial tone
- Deep frequency content

**Design Rationale**
- Longer duration conveys sustained force
- Lower frequency indicates larger system
- Higher volume indicates power

---

### 2.4 SC_Thruster_Fire_RCS

**Purpose**: Small RCS (Reaction Control System) thruster

**Technical Specifications**
- Duration: 0.08 seconds (very short)
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Format: WAV master, OGG Vorbis in-engine
- Frequency Response: 400-800 Hz
- Peak Level: -9 dB (very subtle)

**Audio Characteristics**
- Attack: Sharp
- Quality: Thin, minimal
- Tone: High frequency click/puff
- Type: Quick punctuation
- Character: Minimal, precise

**Use Cases**
- Fine attitude adjustments
- Trim control
- Small correction burns

---

### 2.5 SC_Thruster_Burst_Short

**Purpose**: Short burst variant (manual playback control)

**Technical Specifications**
- Duration: 0.1 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Format: WAV master, OGG Vorbis in-engine
- Frequency Response: 200-400 Hz
- Peak Level: -6 dB

**Production Notes**
- Template for triggering shorter thruster bursts
- Can be used when precise burst duration is known
- Cuts off naturally (no tail requirement)

---

### 2.6 SC_Thruster_Burst_Long

**Purpose**: Extended thruster burn (sustained thrust)

**Technical Specifications**
- Duration: 0.3 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Format: WAV master, OGG Vorbis in-engine
- Frequency Response: 200-400 Hz
- Peak Level: -3 dB

**Production Notes**
- For sustained burns
- Slightly higher volume than short burst
- Natural decay at end (no abrupt cutoff)

---

## 3. UI Sound Assets

### 3.1 SC_UI_PartSelect_Click

**Purpose**: Feedback when hovering over or selecting a ship part

**Technical Specifications**
- Duration: 0.08 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Frequency Response: 2000-4000 Hz
- Peak Level: -6 dB
- Type: Digital click

**Audio Characteristics**
- Attack: Immediate
- Quality: Clean, digital
- Tone: High-pitched click
- Decay: Quick (50ms tail)
- Character: Non-intrusive but clear

**Production Notes**
- Create using synthesizer (click oscillator)
- Use band-limited noise for digital quality
- Very short duration (no lingering)
- Should not startle (not loud)

---

### 3.2 SC_UI_PartEquip_Complete

**Purpose**: Confirmation sequence when equipping a part (0.5s total)

**Technical Specifications**
- Duration: 0.5 seconds (composite sound)
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -6 dB
- Structure: Two-layer composite

**Layer 1: Mechanical Clunk** (0.0-0.2s)
- Frequency: 100-200 Hz
- Type: Physical metal-on-metal impact
- Volume: 0.7

**Layer 2: Electronic Confirm** (0.2-0.5s)
- Frequency: 2000 Hz sustained
- Type: Digital beep/tone
- Duration: 0.3s
- Volume: 0.5

**Production Strategy**
- Record metal impact for clunk (filter to 100-200 Hz)
- Create synthesized beep for confirmation
- Crossfade between layers around 0.15s
- Test as composite sound file

---

### 3.3 SC_UI_PartUnequip_Complete

**Purpose**: Confirmation sequence when unequipping a part (reverse effect)

**Technical Specifications**
- Duration: 0.4 seconds (shorter than equip)
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -6 dB
- Structure: Two-layer composite (reversed order)

**Layer 1: Electronic Disconnect** (0.0-0.2s)
- Frequency: 2000 Hz with pitch bend downward (2000→1000 Hz)
- Type: Digital disconnect with frequency sweep
- Volume: 0.5

**Layer 2: Mechanical Release** (0.2-0.4s)
- Frequency: 200-400 Hz
- Type: Sharp release click
- Volume: 0.7→0.3 (decays)

**Production Notes**
- Reverse psychoacoustic effect of equip sound
- Pitch bend down communicates disengagement
- Mechanical release sound is sharper than clunk
- Overall shorter than equip (less consequential)

---

### 3.4 SC_UI_LoadoutSwitch_Complete

**Purpose**: Confirmation sequence when switching ship loadout (0.8s total)

**Technical Specifications**
- Duration: 0.8 seconds (composite)
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -6 dB

**Layer 1: Transition Whoosh** (0.0-0.3s)
- Type: Frequency sweep whoosh
- Frequency: 800 Hz → 2000 Hz (ascending)
- Volume: 0.6 (peaks at 0.2s)

**Layer 2: Confirmation Chord** (0.3-0.8s)
- Type: Two-note ascending sequence
- Note 1: 2500 Hz (0.3-0.5s)
- Note 2: 3000 Hz (0.5-0.8s)
- Overlap: Slight overlap for continuity
- Volume: 0.5 per note

**Production Strategy**
- Create whoosh using frequency sweep
- Synthesize two-note ascending chord
- Fade between whoosh and chord
- Smooth, musical quality

---

### 3.5 SC_UI_PurchasePart_Success

**Purpose**: Positive feedback when purchase succeeds (0.6s total)

**Technical Specifications**
- Duration: 0.6 seconds (composite)
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -6 dB

**Layer 1: Cash Register Ding** (0.0-0.15s)
- Frequency: 800-1200 Hz metal bell
- Type: Recorded bell strike (processed)
- Attack: Immediate
- Decay: Natural resonance (ring)
- Volume: 0.8

**Layer 2: Success Chime** (0.1-0.6s, overlaps with ding)
- Type: Three-note ascending sequence
- Note 1: 2000 Hz (0.1-0.25s)
- Note 2: 2500 Hz (0.3-0.45s)
- Note 3: 3000 Hz (0.5-0.65s, may extend past layer end)
- Duration per note: 0.15s with 0.05s gap
- Volume: 0.5 per note

**Production Notes**
- Record metal bell or synthesize bell tone
- Create ascending three-note sequence
- Overlap creates musical effect
- Conveys both transaction (ding) and success (chimes)

---

### 3.6 SC_UI_PurchasePart_Fail

**Purpose**: Negative feedback when purchase fails (insufficient credits)

**Technical Specifications**
- Duration: 0.5 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -6 dB
- Type: Single composite sound

**Audio Characteristics**
- Frequency Sweep: 1000 Hz → 500 Hz (descending)
- Modulation: ±100 Hz wobble at 3 Hz rate
- Envelope: Immediate onset, quick decay
- Quality: Synthesized error tone
- Volume: 0.5 (not alarming)

**Production Notes**
- Create descending pitch sweep (conveys failure)
- Add vibrato/wobble for "uncertain" effect
- Keep volume moderate (not frustrating)
- Distinct from success sound

**Psychoacoustic Goals**
- Communicate failure without being harsh
- Suggest "try again" rather than permanent block
- Not as severe as critical alarms

---

### 3.7 SC_UI_UnlockPart_Success

**Purpose**: Celebration sound for unlocking new ship part (2.0s)

**Technical Specifications**
- Duration: 2.0 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -6 dB
- Type: Fanfare sequence

**Audio Structure**
- Opening: Three-note ascending (0.0-0.5s)
  - C, E, G notes (ascending interval pattern)
  - Frequency: ~260 Hz, ~330 Hz, ~400 Hz
- Middle: Sustained high note (0.5-1.2s)
  - Frequency: 2000 Hz
  - Type: Sustained tone
  - Volume: 0.7
- Closing: Descending resolution (1.2-2.0s)
  - Chromatic descending scale
  - Frequency: 2000 Hz → 1000 Hz
  - Volume: Decays from 0.7 to 0.2

**Production Strategy**
- Create musical fanfare (not just beeps)
- Use orchestral or synthesized notes
- Ascending pattern conveys achievement
- Sustained peak for emphasis
- Descending resolution provides closure
- Total duration conveys significance

---

### 3.8 SC_UI_LevelUp_Fanfare

**Purpose**: Epic celebration sound for ship level-up achievement (3.0s)

**Technical Specifications**
- Duration: 3.0 seconds
- Sample Rate: 48 kHz
- Bit Depth: 24-bit (for richness)
- Peak Level: -6 dB
- Type: Extended fanfare

**Audio Composition**

**Section 1: Ascending Arpeggio** (0.0-1.0s)
- Notes: G4, B4, D5, G5 (ascending)
- Type: Rapid arpeggio
- Tempo: One note per 250ms
- Volume: 0.2 → 0.8 (ascending)
- Quality: Orchestral or bright synthesized

**Section 2: Triumphant Peak** (0.8-1.8s)
- Note: G5 (2500 Hz equivalent)
- Type: Sustained note
- Harmonics: Rich multi-harmonic content
- Volume: 0.8 (sustained)
- Duration: 1.0 second

**Section 3: Descending Resolution** (1.6-3.0s)
- Pattern: Chromatic descending scale
- Frequency: 2500 Hz → 1500 Hz
- Type: Orchestral resolution
- Volume: 0.8 → 0.3 (decays)
- Reverb: Slight reverb tail for "grandeur"

**Production Strategy**
- Create orchestral-quality fanfare
- Use real instrument samples if possible
- Rapid ascending passage for excitement
- Sustained peak for emotional impact
- Descending resolution for musical closure
- Emphasize richness and complexity

**Sound Design Goals**
- Convey major achievement
- Create memorable moment
- Epic, celebratory tone
- Musical satisfaction

---

### 3.9 SC_UI_Hover_Subtle

**Purpose**: Subtle navigation feedback when hovering over menu items

**Technical Specifications**
- Duration: 0.1 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Frequency Response: 1500-2000 Hz
- Peak Level: -12 dB (very subtle)
- Type: Barely audible feedback

**Audio Characteristics**
- Attack: Immediate but very soft
- Quality: Thin, minimal
- Tone: High-frequency puff
- Decay: Quick (50ms)
- Character: Almost subliminal

**Production Notes**
- Create using high-frequency noise burst
- Very low amplitude (not distracting)
- Noticeable only in quiet moments
- Should not interrupt concentration

**Use Cases**
- Hovering over menu options
- Scrolling through lists
- Navigating interface

---

### 3.10 SC_UI_Cancel

**Purpose**: Feedback for canceling action or navigating back

**Technical Specifications**
- Duration: 0.2 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Frequency Response: 1000-600 Hz
- Peak Level: -6 dB
- Type: Descending tone

**Audio Characteristics**
- Frequency Sweep: 1000 Hz → 600 Hz (descending)
- Modulation: Subtle wobble (±50 Hz)
- Envelope: Peak at start, quick decay
- Quality: Soft, non-confirmatory
- Character: Neutral, non-alarming

**Production Notes**
- Create pitch downward sweep
- Indicate "going back" through descending tone
- Less assertive than equip sounds
- Softer than error sounds
- Wobble suggests "transition" state

---

### 3.11 SC_UI_Navigate

**Purpose**: Transition sound when navigating through menu sections

**Technical Specifications**
- Duration: 0.15 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -6 dB

**Audio Characteristics**
- Type: Quick whoosh or swipe sound
- Frequency Sweep: 600 Hz → 1500 Hz (ascending)
- Volume: 0.4
- Attack: Immediate
- Character: Smooth transition

**Production Notes**
- Create ascending whoosh sound
- Convey forward motion through UI
- Faster than loadout switch (less significant)
- Clean, quick transition

---

## 4. Hull and Damage Sound Assets

### 4.1 SC_Hull_Creaking_Loop_Light

**Purpose**: Light structural stress indication (30-50% integrity)

**Technical Specifications**
- Duration: Looping pattern (4-6 seconds seamless loop)
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -12 dB (subtle)
- Frequency Response: 100-300 Hz
- Type: Organic creaking sounds

**Audio Characteristics**
- Content: 3-5 distinct creak sounds in sequence
- Timing: Random intervals (0.5-2.0s apart)
- Duration per creak: 0.3-0.6s
- Type: Wooden/metallic creaks
- Volume: 0.15 average (subtle background)

**Production Strategy**
- Record real structural creaks (wood, metal)
- Filter to 100-300 Hz range
- Create variations (different pitch/tone)
- Arrange into looping pattern
- Ensure seamless loop points
- Add slight variation per loop (randomize timing)

**Design Intent**
- Communicate structural stress without alarm
- Psychoacoustic: "Ship is damaged but holding"
- Encourage player to seek repairs
- Constant but non-distracting presence

---

### 4.2 SC_Hull_Creaking_Loop_Heavy

**Purpose**: Severe structural stress indication (10-30% integrity)

**Technical Specifications**
- Duration: Looping pattern (3-5 seconds seamless loop)
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -6 dB (more prominent)
- Frequency Response: 80-250 Hz (deeper)
- Type: Organic severe creaking

**Audio Characteristics**
- Content: 4-6 distinct severe creaks
- Timing: More frequent (0.3-1.0s apart)
- Duration per creak: 0.4-0.8s (longer)
- Type: Deep, concerning creaks
- Volume: 0.25-0.4 (more prominent than light)

**Production Strategy**
- Use deeper, more substantial creak sounds
- More aggressive pitch drops
- Slightly louder volume
- More frequent creaking pattern
- Emphasize urgency without horror

**Design Intent**
- Communicate critical structural damage
- Psychoacoustic: "Ship is failing, urgent repairs needed"
- Create sense of danger
- Encourage immediate player action

---

### 4.3 SC_Hull_Breach_Warning

**Purpose**: Hull breach alarm (< 20% integrity with atmosphere)

**Technical Specifications**
- Duration: Looping alarm (0.8s repeating pattern)
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -3 dB (prominent)
- Type: Alarm sequence

**Audio Layers**
1. **Alarm Tone** (0.0-0.4s)
   - Frequency: 3000 Hz
   - Duration: 0.4s
   - Volume: 0.6
   - Type: Sharp alert tone

2. **Rumble Layer** (0.0-0.4s, concurrent)
   - Frequency: 100 Hz
   - Type: Deep warning rumble
   - Volume: 0.4
   - Creates sense of structural failure

3. **Silent Gap** (0.4-0.8s)
   - Purpose: Temporal pattern recognition
   - Allows player to distinguish from continuous alarm

**Pattern**: 0.4s on, 0.4s off (repeating)

**Production Notes**
- Create sharp alarm tone at 3000 Hz
- Layer with 100 Hz rumble for physical sensation
- Distinct on/off pattern for urgency
- Not harsh enough to cause pain
- Clear alert without horror tone

---

### 4.4 SC_Hull_Integrity_Critical

**Purpose**: Imminent destruction alarm (< 10% integrity)

**Technical Specifications**
- Duration: Continuous loop until destroyed
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -3 dB
- Type: Critical alarm

**Audio Characteristics**
- Frequency: 4000 Hz primary + 100 Hz rumble
- Pattern: 0.3s on, 0.3s off (faster than breach warning)
- Volume: 0.5 (sustained alarm level)
- Modulation: ±200 Hz wobble on main tone (creates urgency)
- Type: Urgent, demanding attention

**Production Strategy**
- Higher frequency than breach alarm
- Faster pulsing pattern
- Include frequency modulation for "intensity"
- Deep rumble creates felt sensation
- More aggressive than breach alarm

**Design Intent**
- Communicate imminent catastrophic failure
- Demand immediate player action (jump to escape pod, etc.)
- Create urgency without causing panic
- Clear distinction from breach alarm

---

### 4.5 SC_Collision_Impact_Light

**Purpose**: Minor collision feedback (velocity < 500 cm/s)

**Technical Specifications**
- Duration: 0.15 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -9 dB
- Frequency Range: 200-400 Hz
- Type: Metallic impact

**Audio Characteristics**
- Attack: Immediate, sharp
- Tone: Metallic impact sound
- Decay: Quick tail (100ms)
- Harmonics: Moderate metal ring
- Volume: 0.2-0.4 (based on collision intensity)

**Production Strategy**
- Record metal-on-metal impact
- Filter to 200-400 Hz range
- Create sharp attack
- Allow natural decay/ring
- Vary amplitude for different intensities

**Sound Quality**
- Clean, clear impact
- No excessive resonance
- Recognizable as metallic contact
- Not alarming (minor event)

---

### 4.6 SC_Collision_Impact_Medium

**Purpose**: Moderate collision feedback (velocity 500-1000 cm/s)

**Technical Specifications**
- Duration: 0.25 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -6 dB
- Frequency Range: 150-350 Hz
- Type: Substantial metallic impact

**Audio Characteristics**
- Attack: Sharp but powerful (10ms rise)
- Tone: Deeper metallic impact
- Decay: Longer resonance (150ms tail)
- Harmonics: Richer metal ring
- Volume: 0.4-0.7 (moderate impact)

**Production Strategy**
- Use heavier metal impact
- Deeper frequency content
- More pronounced resonance
- Slightly longer duration
- More dramatic than light impact

**Psychoacoustic Effect**
- Communicate moderate collision severity
- Convey some structural impact
- Suggest potential damage

---

### 4.7 SC_Collision_Impact_Heavy

**Purpose**: Severe collision feedback (velocity > 1000 cm/s)

**Technical Specifications**
- Duration: 0.4 seconds
- Sample Rate: 48 kHz
- Bit Depth: 24-bit (for richness)
- Peak Level: -3 dB
- Frequency Range: 80-300 Hz
- Type: Destructive collision impact

**Audio Characteristics**
- Attack: Immediate, commanding
- Tone: Deep, substantial impact
- Decay: Extended resonance (400+ms tail)
- Harmonics: Complex multi-harmonic impact
- Volume: 0.7-1.0 (very prominent)
- Sub-bass: 80-100 Hz for felt impact

**Production Strategy**
- Use large object collision impact
- Deep fundamental frequency
- Rich harmonic structure
- Extended resonance
- Create sense of destructive force
- Include sub-bass for haptic feedback

**Sound Composition**
- 0.0-0.1s: Impact transient (sharp)
- 0.1-0.4s: Resonance tail (metallic ring)
- May include: Crack/pop noise overlay

**Psychoacoustic Effect**
- Communicate severe collision
- Create sense of danger
- Suggest significant structural damage
- Felt through haptics/subwoofer

---

### 4.8 SC_Hull_Damage_Pop

**Purpose**: Sharp damage event indicator (explosion, cracking)

**Technical Specifications**
- Duration: 0.08 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -6 dB
- Frequency Response: 100-4000 Hz
- Type: Sharp transient pop

**Audio Characteristics**
- Attack: Immediate, crisp
- Type: Sharp pop/crack sound
- Decay: Quick (50ms)
- Harmonics: Broad spectrum (transient)
- Volume: 0.3-0.6 (noticeable but not alarming)

**Production Strategy**
- Create short sharp transient
- Broad frequency spectrum
- Quick onset and decay
- Crisp, clean sound
- Can be layered with other damage sounds

---

## 5. Shield Sound Assets

### 5.1 SC_Shield_Activate

**Purpose**: Confirmation sound when shield powers up

**Technical Specifications**
- Duration: 1.0 seconds (power-up sequence)
- Sample Rate: 48 kHz
- Bit Depth: 24-bit
- Peak Level: -6 dB
- Type: Power-up synthesis

**Audio Composition**
- **Rise Phase** (0.0-0.5s)
  - Frequency Sweep: 800 Hz → 2000 Hz
  - Type: Ascending synthesized tone
  - Volume: 0.3 → 0.6

- **Peak Hold** (0.5-0.8s)
  - Frequency: 2000 Hz sustained
  - Type: Confirmation tone
  - Volume: 0.6

- **Settle Phase** (0.8-1.0s)
  - Frequency: 2000 Hz with slight decay
  - Volume: 0.6 → 0.4

**Production Strategy**
- Create smooth frequency sweep
- Rising frequency = power increasing
- Peak represents shield active
- Settle phase represents stabilization
- Smooth transitions between phases

**Psychoacoustic Effect**
- Communicate protection/defensive system activated
- Rising frequency conveys power build-up
- Sustained peak conveys readiness
- Settle represents stable operation

---

### 5.2 SC_Shield_Idle_Hum

**Purpose**: Continuous background hum indicating active shield

**Technical Specifications**
- Duration: Infinite loop (looping source)
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -12 dB (subtle)
- Base Frequency: 1000 Hz
- Type: Sustained electronic hum

**Audio Characteristics**
- Frequency: 1000 Hz fundamental
- Harmonics: 2nd and 3rd harmonics present
- Modulation: ±50 Hz wobble at 1 Hz rate
- Volume: 0.1-0.3 (scales with shield strength)
- Tone: Electronic, clean, non-intrusive

**Production Strategy**
- Create sustained 1000 Hz tone (synthesized)
- Add slight harmonic content (2nd and 3rd)
- Implement wobble/vibrato effect
- Create seamless loop (no discontinuities)
- Keep volume subtle (background presence)

**Design Intent**
- Constant reminder of active shield
- Subconscious awareness of protection
- Changes with shield strength (volume modulation)
- Non-distracting background element

---

### 5.3 SC_Shield_Hit_Light

**Purpose**: Audio feedback for minor shield impact

**Technical Specifications**
- Duration: 0.2 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -9 dB
- Frequency Range: 1500-3000 Hz
- Type: Electric "zap" sound

**Audio Characteristics**
- Attack: Immediate crackle
- Type: Electric discharge effect
- Tone: High-frequency electrical
- Decay: Quick tail (150ms)
- Harmonics: Broad, crackling quality
- Volume: 0.3-0.5 (based on damage)

**Production Strategy**
- Create electrical discharge sound (synthesized)
- High-frequency crackling noise
- Sharp attack, quick decay
- Conveys energy absorption
- Test at various volumes (0.3-0.5 range)

**Psychoacoustic Effect**
- Communicate shield absorbing damage
- "Zap" sound suggests electrical force field
- Quick decay suggests minor impact
- Non-threatening tone

---

### 5.4 SC_Shield_Hit_Medium

**Purpose**: Audio feedback for moderate shield impact

**Technical Specifications**
- Duration: 0.3 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -6 dB
- Frequency Range: 800-2000 Hz
- Type: Strong electrical impact

**Audio Characteristics**
- Attack: Rapid build-up (20ms)
- Type: Stronger electrical impact
- Tone: Broader frequency range than light
- Decay: Longer tail with reverb (200ms)
- Harmonics: Complex electrical noise
- Volume: 0.5-0.8 (stronger impact)

**Production Strategy**
- Create stronger electrical discharge
- Broader frequency spectrum
- More prominent attack
- Longer decay phase
- Conveys more significant energy transfer

---

### 5.5 SC_Shield_Hit_Heavy

**Purpose**: Audio feedback for severe shield impact

**Technical Specifications**
- Duration: 0.4 seconds
- Sample Rate: 48 kHz
- Bit Depth: 24-bit
- Peak Level: -3 dB
- Frequency Range: 400-2000 Hz
- Type: Severe electrical impact

**Audio Characteristics**
- Attack: Immediate, commanding
- Type: Powerful electrical discharge
- Bass Layer: 400-600 Hz deep impact
- Mid Layer: 1000-2000 Hz electrical
- Decay: Extended with reverb (400+ms)
- Volume: 0.8-1.0 (very prominent)

**Production Strategy**
- Create multi-layered electrical impact
- Include bass component for felt impact
- Prominent mid-frequency content
- Extended resonance
- Convey danger/severity

**Sound Composition**
- Deep bass impact (0.0-0.2s, 400-600 Hz)
- Electrical crackle overlay (0.0-0.4s, 1000-2000 Hz)
- Extended tail (0.2-0.4s reverb)

---

### 5.6 SC_Shield_Depleted

**Purpose**: Indication that shield has lost all capacity

**Technical Specifications**
- Duration: 0.8 seconds
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -6 dB
- Type: Power-down + warning sequence

**Audio Layers**
1. **Shield Power-Down** (0.0-0.5s)
   - Frequency: 2000 Hz descending to 600 Hz
   - Type: Descending electronic whine
   - Volume: 0.6 → 0.3

2. **Depletion Warning Beep** (0.5-0.8s)
   - Frequency: 2500 Hz
   - Type: Three short beeps (0.1s each)
   - Spacing: 0.05s between beeps
   - Volume: 0.4

**Production Strategy**
- Create descending frequency sweep (2000→600 Hz)
- Convey "power loss" through descent
- Add warning beep sequence
- Combine to create emergency indication

**Psychoacoustic Effect**
- Descending pitch = power-down
- Warning beeps indicate vulnerability
- Urgent but not panic-inducing
- Clear indication of defensive loss

---

### 5.7 SC_Shield_Recharge_Begin

**Purpose**: Indication that shield begins recharging

**Technical Specifications**
- Duration: 0.5 seconds (start sequence)
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -6 dB
- Type: Power-up start

**Audio Characteristics**
- Frequency Sweep: 500 Hz → 1200 Hz (ascending)
- Type: Rising synthesized tone
- Harmonics: Light harmonic content
- Volume: 0.2 → 0.4
- Duration: 0.5s ramp-up

**Production Strategy**
- Create ascending frequency sweep
- Smooth, gradual rise (0.5s)
- Conveys "recharge initiating"
- Transitions smoothly to recharge loop

**Integration**
- Plays once when recharge begins
- Transitions to SC_Shield_Recharge_Loop
- Continuous during recharge process

---

### 5.8 SC_Shield_Recharge_Loop

**Purpose**: Continuous feedback during shield recharge process

**Technical Specifications**
- Duration: Looping (entire recharge duration, 5+ seconds)
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -6 dB
- Type: Looping recharge tone

**Audio Characteristics**
- Base Frequency: 1000-1500 Hz (morphing)
- Type: Sustained hum with subtle modulation
- Modulation: Increases as recharge progresses
- Volume: 0.3 (constant throughout)
- Loop Duration: 4-6 seconds seamless loop

**Production Strategy**
- Create sustained tone morphing 1000→1500 Hz over loop
- Add progressive modulation effect
- Ensure seamless loop point
- Volume remains constant
- Conveys "active recharging in progress"

**Dynamic Modulation**
- Modulation rate parameter scales with recharge progress
- Early recharge: 0.5 Hz modulation
- Late recharge: 2.0 Hz modulation
- Creates perception of accelerating process

**Design Intent**
- Communicate "recharge in progress"
- Gradual acceleration suggests time passing
- Non-intrusive but clearly audible
- Satisfying audio feedback for waiting

---

## 6. Ambient Sound Assets

### 6.1 SC_Ambient_ComputerSystems_Loop

**Purpose**: Background computer system beeps and processing sounds

**Technical Specifications**
- Duration: 3-5 seconds seamless loop
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -12 dB (subtle background)
- Frequency Range: 500-3000 Hz
- Type: Composite looping pattern

**Audio Elements**
1. **Periodic Beeps** (randomized timing)
   - Frequency: 1000-2000 Hz
   - Interval: 1-2 second spacing
   - Duration: 0.1s per beep
   - Purpose: System processing sounds

2. **Soft Whoosh Sounds** (randomized)
   - Frequency: 800-1500 Hz sweep
   - Interval: 2-3 second spacing
   - Duration: 0.3s per whoosh
   - Purpose: Data transfer effect

3. **Electronic Chirps** (randomized)
   - Frequency: 2000-3000 Hz
   - Interval: 3-5 second spacing
   - Duration: 0.15s per chirp
   - Purpose: System alerts

4. **Low-Level Digital Noise** (continuous)
   - Frequency: 500-2000 Hz
   - Type: Subtle background hum/noise
   - Volume: Very subtle (0.05)
   - Purpose: "System running" indicator

**Production Strategy**
- Layer multiple synthesized elements
- Randomize element timing (no repeating pattern)
- Create complex composite loop
- Ensure seamless loop point
- Keep overall volume subtle

**Psychoacoustic Effect**
- Establishes "active computer system" presence
- Prevents dead silence
- Conveys technological environment
- Non-intrusive background texture

---

### 6.2 SC_Ambient_LifeSupport_Loop

**Purpose**: Air circulation and life support system sounds

**Technical Specifications**
- Duration: 4-6 seconds seamless loop
- Sample Rate: 48 kHz
- Bit Depth: 16-bit
- Peak Level: -12 dB (very subtle)
- Frequency Range: 100-400 Hz
- Type: Organic circulation pattern

**Audio Elements**
1. **Air Circulation Base** (continuous)
   - Frequency: 100-200 Hz
   - Type: White noise base
   - Volume: Very subtle (0.05)
   - Purpose: Breathing effect

2. **Periodic Air Pump** (every 2-3 seconds)
   - Frequency: 150-250 Hz
   - Type: Mechanical pump sound
   - Duration: 0.5s
   - Volume: 0.1
   - Purpose: Pressurization cycles

3. **Subtle Valve Operations** (randomized)
   - Frequency: 300-400 Hz click
   - Interval: Random, 2-5 seconds
   - Duration: 0.2s
   - Volume: 0.05
   - Purpose: Air flow control

4. **Humidity Sensor Calibration** (rare, 10-15s intervals)
   - Frequency: High-pitched click (2000+ Hz)
   - Duration: 0.1s
   - Volume: 0.03
   - Purpose: System calibration

**Production Strategy**
- Create white noise base for air circulation
- Record or synthesize pump cycles
- Add subtle valve clicks
- Randomize all timing elements
- Keep overall volume very subtle

**Design Intent**
- Establish "life support active" presence
- Subtle indication of maintained atmosphere
- Non-threatening environmental presence
- Communicates system functionality

---

## 7. Total Asset Count Summary

### Audio Cues Required: 68+

**By Category**
- Engine: 8 cues
- Thrusters: 6 cues
- UI: 15 cues
- Hull/Damage: 8 cues
- Shield: 8 cues
- Weapons: 10 cues (optional)
- Ambient: 8 cues
- Environmental: 5 cues

### Source Audio Files: 80-100

Account for:
- Multiple takes per sound
- Variations (light/heavy/agile)
- Processing versions
- Edit masters
- Compression variations

### Estimated Asset Storage

**Uncompressed (master files)**
- 48 kHz, 24-bit stereo: ~400-500 MB
- 48 kHz, 16-bit stereo: ~250-350 MB

**In-Engine (compressed)**
- OGG Vorbis quality: ~150-200 MB
- MP3 fallback: ~120-150 MB

---

## Asset Production Workflow

### Phase 1: Core Sounds (Weeks 1-2)
1. Engine idle and thrust loops
2. Thruster fire variations
3. Spool-up/down transitions
4. Basic UI feedback sounds

### Phase 2: Secondary Systems (Weeks 3-4)
1. Shield activation and hit sounds
2. Collision impacts
3. Hull creaking and alarms
4. UI menu sounds (equip, unlock, etc.)

### Phase 3: Ambient and Polish (Weeks 5-6)
1. Ambient ship system loops
2. Environmental sounds (atmospheric entry, etc.)
3. Weapon sounds (if combat planned)
4. Sound testing and iteration

### Quality Assurance
- Verify all sounds meet technical specifications
- Test in-game audio integration
- Gather player feedback
- Iterate on sounds needing improvement
- Final mastering and optimization

