# Audio Polish System - Quick Start Guide

## Overview

The Audio Polish System provides environmental audio, weather sound effects, farming tool sounds, and atmospheric entry audio to enhance immersion.

## Biome Environmental Audio

### Configuring Biome Audio

```cpp
// Get audio system
UAudioPolishSystem* AudioSystem = GetWorld()->GetSubsystem<UAudioPolishSystem>();

// Load audio assets
USoundBase* ForestAmbient = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Biomes/Forest_Ambient"));
USoundBase* ForestWind = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Biomes/Forest_Wind"));
USoundBase* BirdChirp = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Animals/Bird_Chirp"));
USoundBase* OwlHoot = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Animals/Owl_Hoot"));

// Configure biome audio
FBiomeAudioConfig ForestAudio;
ForestAudio.AmbientSound = ForestAmbient;
ForestAudio.WindSound = ForestWind;
ForestAudio.AnimalSounds.Add(BirdChirp);
ForestAudio.AnimalSounds.Add(OwlHoot);
ForestAudio.AmbientVolume = 0.5f;
ForestAudio.WindVolume = 0.3f;
ForestAudio.AnimalSoundFrequency = 10.0f; // Every 10 seconds

AudioSystem->SetBiomeAudio("Forest", ForestAudio);
```

### Transitioning Between Biomes

```cpp
// Smooth transition to new biome audio
AudioSystem->TransitionToBiomeAudio("Desert", 3.0f); // 3 second fade
```

### Playing Animal Sounds

```cpp
// Manually trigger animal sound
AudioSystem->PlayAnimalSound("Forest");

// Animal sounds also play automatically based on frequency
```

### Example Biome Configurations

**Forest Biome:**

```cpp
FBiomeAudioConfig ForestAudio;
ForestAudio.AmbientSound = Forest_Ambient; // Birds, rustling leaves
ForestAudio.WindSound = Forest_Wind;       // Gentle breeze through trees
ForestAudio.AnimalSounds = {
    Bird_Chirp,
    Owl_Hoot,
    Deer_Call,
    Cricket_Chirp
};
ForestAudio.AmbientVolume = 0.5f;
ForestAudio.WindVolume = 0.3f;
ForestAudio.AnimalSoundFrequency = 8.0f;
```

**Desert Biome:**

```cpp
FBiomeAudioConfig DesertAudio;
DesertAudio.AmbientSound = Desert_Ambient; // Subtle wind, distant sounds
DesertAudio.WindSound = Desert_Wind;       // Hot, dry wind
DesertAudio.AnimalSounds = {
    Coyote_Howl,
    Snake_Rattle,
    Vulture_Call
};
DesertAudio.AmbientVolume = 0.4f;
DesertAudio.WindVolume = 0.5f;
DesertAudio.AnimalSoundFrequency = 15.0f; // Less frequent
```

**Ocean Biome:**

```cpp
FBiomeAudioConfig OceanAudio;
OceanAudio.AmbientSound = Ocean_Ambient; // Waves, seagulls
OceanAudio.WindSound = Ocean_Wind;       // Sea breeze
OceanAudio.AnimalSounds = {
    Seagull_Call,
    Whale_Song,
    Dolphin_Click
};
OceanAudio.AmbientVolume = 0.6f;
OceanAudio.WindVolume = 0.4f;
OceanAudio.AnimalSoundFrequency = 12.0f;
```

**Tundra Biome:**

```cpp
FBiomeAudioConfig TundraAudio;
TundraAudio.AmbientSound = Tundra_Ambient; // Howling wind, ice cracking
TundraAudio.WindSound = Tundra_Wind;       // Harsh, cold wind
TundraAudio.AnimalSounds = {
    Wolf_Howl,
    Arctic_Fox_Call,
    Penguin_Call
};
TundraAudio.AmbientVolume = 0.3f;
TundraAudio.WindVolume = 0.7f; // Prominent wind
TundraAudio.AnimalSoundFrequency = 20.0f; // Sparse wildlife
```

## Weather Sound Effects

### Configuring Weather Audio

```cpp
// Load weather sounds
USoundBase* RainSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Weather/Rain"));
USoundBase* ThunderSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Weather/Thunder"));
USoundBase* WindSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Weather/Wind"));
USoundBase* SnowSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Weather/Snow"));
USoundBase* SandstormSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Weather/Sandstorm"));

// Configure weather audio
FWeatherAudioConfig WeatherAudio;
WeatherAudio.RainSound = RainSound;
WeatherAudio.ThunderSound = ThunderSound;
WeatherAudio.WindSound = WindSound;
WeatherAudio.SnowSound = SnowSound;
WeatherAudio.SandstormSound = SandstormSound;
WeatherAudio.BaseVolume = 0.6f;

AudioSystem->SetWeatherAudio(WeatherAudio);
```

### Playing Weather Sounds

```cpp
// Play rain sound
AudioSystem->PlayWeatherSound("Rain", 0.8f); // 80% intensity

// Play light rain
AudioSystem->PlayWeatherSound("LightRain", 0.4f); // 40% intensity

// Play snow
AudioSystem->PlayWeatherSound("Snow", 1.0f);

// Play sandstorm
AudioSystem->PlayWeatherSound("Sandstorm", 1.0f);

// Stop weather sound
AudioSystem->StopWeatherSound(2.0f); // 2 second fade out
```

### Thunder Effects

```cpp
// Play thunder at specific location
FVector LightningLocation = FVector(1000, 500, 2000);
AudioSystem->PlayThunderSound(LightningLocation);

// Thunder will be 3D positioned based on distance from player
```

### Weather Intensity

Weather intensity affects volume:

- 0.0 = Silent
- 0.3 = Light weather
- 0.6 = Moderate weather
- 1.0 = Heavy weather

## Farming Tool Sounds

### Configuring Farming Tool Audio

```cpp
// Load farming tool sounds
USoundBase* PlantingSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Farming/Planting"));
USoundBase* WateringSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Farming/Watering"));
USoundBase* HarvestingSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Farming/Harvesting"));
USoundBase* HoeingSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Farming/Hoeing"));
USoundBase* FertilizingSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Farming/Fertilizing"));

// Configure farming tool audio
FFarmingToolAudioConfig ToolAudio;
ToolAudio.PlantingSound = PlantingSound;
ToolAudio.WateringSound = WateringSound;
ToolAudio.HarvestingSound = HarvestingSound;
ToolAudio.HoeingSound = HoeingSound;
ToolAudio.FertilizingSound = FertilizingSound;
ToolAudio.ToolVolume = 0.7f;

AudioSystem->SetFarmingToolAudio(ToolAudio);
```

### Playing Tool Sounds

```cpp
// Planting
FVector PlantLocation = GetPlantingLocation();
AudioSystem->PlayPlantingSound(PlantLocation);

// Watering (looping)
FVector WateringLocation = GetWateringLocation();
AudioSystem->PlayWateringSound(WateringLocation, true); // Loop while watering

// Stop watering when done
AudioSystem->StopWateringSound();

// Harvesting
FVector HarvestLocation = GetHarvestLocation();
AudioSystem->PlayHarvestingSound(HarvestLocation);

// Hoeing
FVector HoeLocation = GetHoeLocation();
AudioSystem->PlayHoeingSound(HoeLocation);

// Fertilizing
FVector FertilizeLocation = GetFertilizeLocation();
AudioSystem->PlayFertilizingSound(FertilizeLocation);
```

### Integration with VR Interactions

```cpp
// In VR planting gesture detector
void UVRPlantingGestureDetector::OnPlantingGestureDetected(FVector Location)
{
    // Play planting sound
    UAudioPolishSystem* AudioSystem = GetWorld()->GetSubsystem<UAudioPolishSystem>();
    AudioSystem->PlayPlantingSound(Location);

    // Plant the seed
    PlantSeed(Location);
}

// In VR watering can
void UVRWateringCan::OnWateringStarted()
{
    UAudioPolishSystem* AudioSystem = GetWorld()->GetSubsystem<UAudioPolishSystem>();
    AudioSystem->PlayWateringSound(GetActorLocation(), true);
}

void UVRWateringCan::OnWateringStopped()
{
    UAudioPolishSystem* AudioSystem = GetWorld()->GetSubsystem<UAudioPolishSystem>();
    AudioSystem->StopWateringSound();
}
```

## Atmospheric Entry Audio

### Playing Atmospheric Entry Sound

```cpp
// Start atmospheric entry audio
AudioSystem->PlayAtmosphericEntrySound(0.5f); // Initial intensity
```

### Updating During Entry

```cpp
// Update audio based on altitude and speed
void UpdateAtmosphericEntry(float DeltaTime)
{
    float CurrentAltitude = GetAltitude();
    float CurrentSpeed = GetSpeed();

    AudioSystem->UpdateAtmosphericEntryAudio(CurrentAltitude, CurrentSpeed);
}
```

### Stopping Atmospheric Entry Sound

```cpp
// Stop when entry complete
AudioSystem->StopAtmosphericEntrySound(3.0f); // 3 second fade out
```

### Example Entry Sequence

```cpp
void PlayAtmosphericEntrySequence()
{
    UAudioPolishSystem* AudioSystem = GetWorld()->GetSubsystem<UAudioPolishSystem>();

    // Start entry sound
    AudioSystem->PlayAtmosphericEntrySound(0.3f);

    // Update continuously during entry
    GetWorld()->GetTimerManager().SetTimer(
        EntryUpdateTimer,
        [this, AudioSystem]()
        {
            float Altitude = GetAltitude();
            float Speed = GetSpeed();

            AudioSystem->UpdateAtmosphericEntryAudio(Altitude, Speed);

            // Stop when altitude is low enough
            if (Altitude < 1000.0f)
            {
                AudioSystem->StopAtmosphericEntrySound(2.0f);
                GetWorld()->GetTimerManager().ClearTimer(EntryUpdateTimer);
            }
        },
        0.1f,
        true
    );
}
```

## Volume Controls

### Master Volume

```cpp
// Set master volume (affects all audio)
AudioSystem->SetMasterVolume(0.8f); // 80%
```

### Ambient Volume

```cpp
// Set ambient volume (biome and weather sounds)
AudioSystem->SetAmbientVolume(0.6f); // 60%
```

### Effects Volume

```cpp
// Set effects volume (tools, atmospheric entry)
AudioSystem->SetEffectsVolume(0.7f); // 70%
```

### Volume Hierarchy

```
Final Volume = MasterVolume × CategoryVolume × SoundVolume

Example:
- Master: 0.8
- Ambient: 0.6
- Biome Ambient: 0.5
= 0.8 × 0.6 × 0.5 = 0.24 (24% of max volume)
```

## 3D Audio Positioning

### Updating Listener Position

```cpp
// Update audio listener (usually camera position)
FVector CameraLocation = GetCameraLocation();
FRotator CameraRotation = GetCameraRotation();

AudioSystem->UpdateListenerPosition(CameraLocation, CameraRotation);
```

### 3D Sound Attenuation

All positional sounds (tools, thunder, animals) use 3D attenuation:

- Closer sounds are louder
- Distant sounds are quieter
- Sounds behind player are quieter than in front

## Complete Example: Biome Transition with Audio

```cpp
void TransitionToBiome(FName NewBiome)
{
    UAudioPolishSystem* AudioSystem = GetWorld()->GetSubsystem<UAudioPolishSystem>();

    // Transition biome audio
    AudioSystem->TransitionToBiomeAudio(NewBiome, 3.0f);

    // Update weather audio if needed
    if (NewBiome == "Desert")
    {
        AudioSystem->StopWeatherSound(2.0f); // Deserts are dry
    }
    else if (NewBiome == "Rainforest")
    {
        AudioSystem->PlayWeatherSound("LightRain", 0.4f); // Frequent rain
    }
}
```

## Performance Considerations

### Audio Budget

- **Ambient sounds**: 2-3 looping sounds per biome
- **Weather sounds**: 1 looping sound
- **Tool sounds**: Short, one-shot sounds
- **Animal sounds**: Occasional, 3D positioned

### Optimization Tips

1. **Use sound cues** - Combine multiple sounds efficiently
2. **Limit concurrent sounds** - Max 20-30 active sounds
3. **Use attenuation** - Sounds fade with distance
4. **Compress audio** - Use appropriate compression
5. **Stream large files** - Don't load everything into memory

## Troubleshooting

### No Sound Playing

- Check volume settings (master, ambient, effects)
- Verify sound assets are loaded
- Check audio component is valid
- Ensure listener position is set

### Sound Too Quiet

- Increase relevant volume (master/ambient/effects)
- Check sound's base volume setting
- Verify 3D attenuation settings
- Check distance from listener

### Sound Stuttering

- Reduce number of concurrent sounds
- Use sound compression
- Check CPU usage
- Enable audio streaming for large files

### Biome Audio Not Transitioning

- Verify biome audio is configured
- Check transition duration
- Ensure biome name matches exactly
- Check for audio component conflicts

---

**Version:** 1.0  
**Last Updated:** 2025-11-04
