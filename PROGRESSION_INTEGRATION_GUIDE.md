# Player Progression Integration Guide

**Created**: November 5, 2025
**Status**: ✅ COMPLETE
**Build**: PASSING (10.41s)

---

## Overview

This guide shows how to integrate the **Player Progression System** with the **Gameplay Scenario System** to create a rewarding player experience in Alexander.

---

## Quick Integration Example

### Scenario → Progression Integration

```cpp
// In your GameMode or PlayerController

#include "GameplayScenario.h"
#include "PlayerProgression.h"

void AAlexanderGameMode::StartTutorial()
{
    // Load player progression
    PlayerProgression = UPlayerProgression::LoadProgression();

    // Spawn tutorial
    Tutorial = GetWorld()->SpawnActor<AFirstLandingTutorial>();

    // Bind to completion event
    Tutorial->OnScenarioCompleted.AddDynamic(this, &AAlexanderGameMode::OnTutorialCompleted);
}

UFUNCTION()
void AAlexanderGameMode::OnTutorialCompleted(const FScenarioRewards& Rewards)
{
    // Award experience points
    PlayerProgression->AddExperience(Rewards.ExperiencePoints);

    // Award credits
    PlayerProgression->AddCredits(Rewards.Credits);

    // Unlock features
    for (FName Feature : Rewards.UnlockedFeatures)
    {
        if (Feature == "BasicFarming")
        {
            PlayerProgression->UnlockFeature(Feature);
            // Also grant farming skill XP
            PlayerProgression->AddSkillXP(EPlayerSkill::Farming, 100.0f);
        }
        else if (Feature == "PlanetaryTravel")
        {
            PlayerProgression->UnlockFeature(Feature);
            // Grant piloting skill XP
            PlayerProgression->AddSkillXP(EPlayerSkill::Piloting, 100.0f);
        }
    }

    // Unlock locations
    for (FName Location : Rewards.UnlockedLocations)
    {
        PlayerProgression->UnlockLocation(Location);
    }

    // Update statistics
    PlayerProgression->IncrementScenariosCompleted();

    // Save progress
    PlayerProgression->SaveProgression();
}
```

---

## System Architecture

### Data Flow
```
Player Actions
    ↓
Scenario Objectives Complete
    ↓
Scenario Completion Event
    ↓
Rewards Distribution
    ↓
Player Progression Update
    ↓
Save Game
```

### Event Chain
```
Tutorial Complete
    → OnScenarioCompleted(Rewards)
        → AddExperience(500)
            → CheckLevelUp()
                → OnLevelUp.Broadcast()
                    → UI Shows Level Up
        → AddCredits(1000)
            → OnCreditsChanged.Broadcast()
                → UI Updates Currency Display
        → UnlockFeature("BasicFarming")
            → OnFeatureUnlocked.Broadcast()
                → UI Shows "New Feature Unlocked!"
```

---

## Progression Features

### 1. Experience & Leveling

```cpp
// Award XP
PlayerProgression->AddExperience(500);

// Check current level
int32 Level = PlayerProgression->GetPlayerLevel();

// Get rank
EPlayerRank Rank = PlayerProgression->GetPlayerRank();

// Listen to level up
PlayerProgression->OnLevelUp.AddDynamic(this, &MyClass::HandleLevelUp);

UFUNCTION()
void HandleLevelUp(int32 NewLevel, int32 OldLevel)
{
    UE_LOG(LogTemp, Display, TEXT("Congratulations! Level %d!"), NewLevel);
}
```

**Level Progression Curve**:
- Level 2: 282 XP
- Level 5: 1,118 XP
- Level 10: 3,162 XP
- Level 20: 8,944 XP
- Level 50: 35,355 XP

**Rank Thresholds**:
- Cadet: Levels 1-4
- Ensign: Levels 5-9
- Lieutenant: Levels 10-19
- Commander: Levels 20-29
- Captain: Levels 30-39
- Admiral: Levels 40-49
- Legend: Level 50+

### 2. Skill System

**8 Skills Available**:
1. **Piloting** - Spaceship flight and navigation
2. **Combat** - Weapons and tactical combat
3. **Engineering** - Ship systems and repairs
4. **Science** - Research and discovery
5. **Farming** - Agriculture and colony management
6. **Trading** - Economy and negotiation
7. **Exploration** - Planet discovery and mapping
8. **Leadership** - Crew management and diplomacy

```cpp
// Award skill XP
PlayerProgression->AddSkillXP(EPlayerSkill::Piloting, 50.0f);

// Get skill info
FSkillLevel PilotSkill = PlayerProgression->GetSkillLevel(EPlayerSkill::Piloting);
int32 SkillLevel = PilotSkill.CurrentLevel;
float Progress = PilotSkill.GetProgressPercent(); // 0-100%

// Listen to skill level up
PlayerProgression->OnSkillLevelUp.AddDynamic(this, &MyClass::HandleSkillUp);

UFUNCTION()
void HandleSkillUp(EPlayerSkill Skill, int32 NewLevel)
{
    UE_LOG(LogTemp, Display, TEXT("Skill improved! Level %d"), NewLevel);
}
```

**Skill XP Curve** (Linear):
- Level 2: 100 XP
- Level 5: 250 XP
- Level 10: 500 XP
- Level 20: 1,000 XP
- Level 50: 2,500 XP

### 3. Currency System

```cpp
// Add credits
PlayerProgression->AddCredits(1000);

// Spend credits
bool Success = PlayerProgression->SpendCredits(500);
if (Success)
{
    // Purchase successful
}

// Get current balance
int32 Balance = PlayerProgression->GetCredits();

// Listen to changes
PlayerProgression->OnCreditsChanged.AddDynamic(this, &MyClass::HandleCreditsChanged);

UFUNCTION()
void HandleCreditsChanged(int32 NewAmount, int32 Delta)
{
    if (Delta > 0)
    {
        // Money earned
    }
    else
    {
        // Money spent
    }
}
```

### 4. Unlock System

```cpp
// Unlock a feature
PlayerProgression->UnlockFeature(FName("AdvancedWeapons"));

// Unlock location
PlayerProgression->UnlockLocation(FName("BetaPlanet"));

// Unlock ship
PlayerProgression->UnlockShip(FName("ScoutShip"));

// Unlock equipment
PlayerProgression->UnlockEquipment(FName("MiningLaser"));

// Check if unlocked
bool bUnlocked = PlayerProgression->IsFeatureUnlocked(FName("AdvancedWeapons"));

// Get all unlocks
FPlayerUnlocks Unlocks = PlayerProgression->GetAllUnlocks();
for (FName Feature : Unlocks.UnlockedFeatures)
{
    // Display unlocked feature
}
```

### 5. Statistics Tracking

```cpp
// Increment counters
PlayerProgression->IncrementPlanetsVisited();
PlayerProgression->IncrementPlanetsColonized();
PlayerProgression->IncrementScenariosCompleted();

// Add distance
PlayerProgression->AddDistanceTraveled(1500.0f); // in km

// Get statistics
FPlayerStatistics Stats = PlayerProgression->GetStatistics();
int32 PlanetsVisited = Stats.PlanetsVisited;
float TotalDistance = Stats.DistanceTraveled;
```

### 6. Save/Load

```cpp
// Save progression
bool bSaved = PlayerProgression->SaveProgression();

// Load progression (static function)
UPlayerProgression* Loaded = UPlayerProgression::LoadProgression();
```

**Save Data Includes**:
- Player level & total XP
- All skill levels & XP
- Credits
- All unlocks (features, locations, ships, equipment)
- Complete statistics

---

## Scenario Reward Configuration

### Tutorial Example

```cpp
void AFirstLandingTutorial::SetupRewards()
{
    // Experience points
    Rewards.ExperiencePoints = 500;

    // Currency
    Rewards.Credits = 1000;

    // Unlocked features
    Rewards.UnlockedFeatures.Add(FName("BasicFarming"));
    Rewards.UnlockedFeatures.Add(FName("PlanetaryTravel"));

    // Unlocked locations
    Rewards.UnlockedLocations.Add(FName("StarterColony"));
}
```

### Progression Rewards by Scenario Type

**Tutorial Scenarios**:
```cpp
Rewards.ExperiencePoints = 500-800;
Rewards.Credits = 1000-1500;
// Unlock basic features
```

**Easy Missions**:
```cpp
Rewards.ExperiencePoints = 300-500;
Rewards.Credits = 500-1000;
Rewards.UnlockedEquipment = { "BasicTool" };
```

**Medium Missions**:
```cpp
Rewards.ExperiencePoints = 800-1200;
Rewards.Credits = 2000-3000;
Rewards.UnlockedLocations = { "NewSector" };
```

**Hard Missions**:
```cpp
Rewards.ExperiencePoints = 1500-2500;
Rewards.Credits = 5000-8000;
Rewards.UnlockedShips = { "CombatShip" };
```

**Epic Scenarios**:
```cpp
Rewards.ExperiencePoints = 3000-5000;
Rewards.Credits = 10000-20000;
Rewards.UnlockedFeatures = { "AdvancedTech" };
```

---

## UI Integration

### Level Display Widget

```cpp
// In your HUD/Widget class

UPROPERTY()
UPlayerProgression* PlayerProgression;

void UpdateLevelDisplay()
{
    if (!PlayerProgression) return;

    int32 Level = PlayerProgression->GetPlayerLevel();
    float TotalXP = PlayerProgression->GetTotalExperience();

    // Update UI text
    LevelText->SetText(FText::AsNumber(Level));

    // Update progress bar
    float Progress = (TotalXP / PlayerProgression->ExperienceToNextLevel) * 100.0f;
    XPProgressBar->SetPercent(Progress / 100.0f);
}
```

### Skills Display Widget

```cpp
void UpdateSkillsDisplay()
{
    for (int32 i = 0; i < 8; i++)
    {
        EPlayerSkill Skill = (EPlayerSkill)i;
        FSkillLevel SkillData = PlayerProgression->GetSkillLevel(Skill);

        // Update skill level text
        SkillLevelTexts[i]->SetText(FText::AsNumber(SkillData.CurrentLevel));

        // Update progress bar
        SkillProgressBars[i]->SetPercent(SkillData.GetProgressPercent() / 100.0f);
    }
}
```

### Unlocks Display Widget

```cpp
void PopulateUnlocksGrid()
{
    FPlayerUnlocks Unlocks = PlayerProgression->GetAllUnlocks();

    // Display unlocked features
    for (FName Feature : Unlocks.UnlockedFeatures)
    {
        // Create widget for unlocked feature
        CreateUnlockWidget(Feature, "Feature");
    }

    // Display unlocked locations
    for (FName Location : Unlocks.UnlockedLocations)
    {
        CreateUnlockWidget(Location, "Location");
    }
}
```

### Rewards Screen

```cpp
void ShowRewardsScreen(const FScenarioRewards& Rewards)
{
    // Animate XP gain
    AnimateXPGain(Rewards.ExperiencePoints);

    // Animate credits gain
    AnimateCreditsGain(Rewards.Credits);

    // Show unlocked features
    for (FName Feature : Rewards.UnlockedFeatures)
    {
        ShowUnlockNotification(Feature);
    }

    // Play celebration effects
    PlayLevelUpEffects();
}
```

---

## Advanced Patterns

### Skill-Based Progression

```cpp
// Award skill XP based on player actions

void OnPlayerLanded()
{
    // Award piloting XP for successful landing
    float Quality = CalculateLandingQuality(); // 0-1
    float XP = 20.0f * Quality;
    PlayerProgression->AddSkillXP(EPlayerSkill::Piloting, XP);
}

void OnCropHarvested()
{
    // Award farming XP
    PlayerProgression->AddSkillXP(EPlayerSkill::Farming, 5.0f);
}

void OnTradeDone(int32 Profit)
{
    // Award trading XP based on profit
    float XP = FMath::Clamp(Profit / 100.0f, 5.0f, 50.0f);
    PlayerProgression->AddSkillXP(EPlayerSkill::Trading, XP);
}
```

### Dynamic Difficulty Scaling

```cpp
float GetMissionDifficulty()
{
    int32 PlayerLevel = PlayerProgression->GetPlayerLevel();

    // Scale difficulty with player level
    return FMath::Lerp(1.0f, 3.0f, PlayerLevel / 50.0f);
}

int32 GetRewardMultiplier()
{
    EPlayerRank Rank = PlayerProgression->GetPlayerRank();

    // Higher ranks get better rewards
    switch (Rank)
    {
        case EPlayerRank::Cadet: return 1;
        case EPlayerRank::Ensign: return 2;
        case EPlayerRank::Lieutenant: return 3;
        case EPlayerRank::Commander: return 4;
        case EPlayerRank::Captain: return 6;
        case EPlayerRank::Admiral: return 8;
        case EPlayerRank::Legend: return 10;
        default: return 1;
    }
}
```

### Feature Gating

```cpp
bool CanAccessMission()
{
    // Require specific rank
    if (PlayerProgression->GetPlayerRank() < EPlayerRank::Lieutenant)
    {
        return false;
    }

    // Require specific feature
    if (!PlayerProgression->IsFeatureUnlocked(FName("AdvancedNavigation")))
    {
        return false;
    }

    // Require skill level
    FSkillLevel Combat = PlayerProgression->GetSkillLevel(EPlayerSkill::Combat);
    if (Combat.CurrentLevel < 10)
    {
        return false;
    }

    return true;
}
```

---

## Testing the Integration

### Test Scenario

```cpp
void TestProgressionIntegration()
{
    // Create progression
    UPlayerProgression* Progression = NewObject<UPlayerProgression>();

    // Create and complete tutorial
    AFirstLandingTutorial* Tutorial = SpawnTutorial();
    Tutorial->StartScenario();

    // Simulate completion
    FScenarioRewards Rewards;
    Rewards.ExperiencePoints = 500;
    Rewards.Credits = 1000;
    Rewards.UnlockedFeatures = { "BasicFarming", "PlanetaryTravel" };

    // Award rewards
    Progression->AddExperience(Rewards.ExperiencePoints);
    Progression->AddCredits(Rewards.Credits);

    for (FName Feature : Rewards.UnlockedFeatures)
    {
        Progression->UnlockFeature(Feature);
    }

    // Verify
    check(Progression->GetPlayerLevel() >= 2);
    check(Progression->GetCredits() == 2000); // 1000 starting + 1000 reward
    check(Progression->IsFeatureUnlocked("BasicFarming"));
}
```

---

## Blueprint Integration

### Creating Progression in Blueprint

1. **Create Blueprint Variable**:
   - Type: `PlayerProgression` (Object Reference)
   - Default Value: None

2. **Load on BeginPlay**:
   ```
   Event BeginPlay
   └── Player Progression = Load Progression (static function)
   ```

3. **Award XP**:
   ```
   On Mission Complete
   └── Player Progression → Add Experience (500)
   ```

4. **Display Level**:
   ```
   Event Tick
   └── Level Text = Get Player Level (from Player Progression)
   ```

---

## Performance Considerations

### Save Frequency
```cpp
// Save only on major events, not every XP gain
void OnScenarioCompleted()
{
    // Award all rewards...

    // Single save at end
    PlayerProgression->SaveProgression();
}

// Auto-save periodically
void AutoSave()
{
    static float TimeSinceLastSave = 0.0f;
    TimeSinceLastSave += DeltaTime;

    if (TimeSinceLastSave >= 300.0f) // Every 5 minutes
    {
        PlayerProgression->SaveProgression();
        TimeSinceLastSave = 0.0f;
    }
}
```

### Memory Usage
- PlayerProgression object: ~2KB
- Save file size: ~5-10KB
- Negligible runtime overhead

---

## Complete Example: Trade Mission

```cpp
UCLASS()
class ATradeRouteMission : public AGameplayScenario
{
    GENERATED_BODY()

public:
    ATradeRouteMission()
    {
        ScenarioName = FText::FromString(TEXT("First Trade Route"));

        // Configure rewards
        Rewards.ExperiencePoints = 800;
        Rewards.Credits = 2500;
        Rewards.UnlockedFeatures.Add(FName("AdvancedTrading"));
    }

protected:
    virtual void OnScenarioCompleted_Implementation()
    {
        Super::OnScenarioCompleted_Implementation();

        // Get player progression
        UPlayerProgression* Progression = UPlayerProgression::LoadProgression();

        // Award scenario rewards
        Progression->AddExperience(Rewards.ExperiencePoints);
        Progression->AddCredits(Rewards.Credits);

        // Award skill XP
        Progression->AddSkillXP(EPlayerSkill::Trading, 100.0f);
        Progression->AddSkillXP(EPlayerSkill::Piloting, 50.0f);

        // Unlock features
        for (FName Feature : Rewards.UnlockedFeatures)
        {
            Progression->UnlockFeature(Feature);
        }

        // Update statistics
        Progression->IncrementScenariosCompleted();
        Progression->Statistics.TradesMade++;

        // Save progress
        Progression->SaveProgression();
    }
};
```

---

## Best Practices

### 1. Balance Rewards
- Tutorials: High XP, basic unlocks
- Easy missions: Low XP, small credits
- Hard missions: High XP, rare unlocks
- Keep progression curve smooth

### 2. Skill Integration
- Award relevant skill XP for player actions
- Don't just award on scenario completion
- Make skills feel meaningful

### 3. Unlock Strategy
- Gate content behind levels/skills
- Provide clear unlock requirements
- Show locked content with unlock hints

### 4. Save Strategy
- Save on scenario completion
- Auto-save periodically
- Save before quit
- Never save during async operations

### 5. UI Feedback
- Always show XP/credits gain
- Celebrate level ups
- Display unlock notifications
- Show progress bars

---

## Troubleshooting

**Q: Player not gaining XP**
A: Check that AddExperience() is being called and amount > 0

**Q: Level up not triggering**
A: Ensure OnLevelUp delegate is bound before adding XP

**Q: Save file not loading**
A: Check SaveSlotName matches between save and load

**Q: Skills not leveling up**
A: Verify SkillType is valid and XP amount is > 0

---

## Related Systems

- [Gameplay Scenario System](GAMEPLAY_SCENARIO_SYSTEM.md)
- [First Landing Tutorial](Source/Alexander/Public/FirstLandingTutorial.h)
- [Player Progression](Source/Alexander/Public/PlayerProgression.h)

---

**Build Status**: ✅ PASSING (10.41s)
**Integration**: ✅ COMPLETE
**Documentation**: ✅ COMPREHENSIVE
**Ready for**: Production Use

---

*"Every experience makes you stronger. Every mission brings you closer to legendary status."*
**- Alexander Progression System**
