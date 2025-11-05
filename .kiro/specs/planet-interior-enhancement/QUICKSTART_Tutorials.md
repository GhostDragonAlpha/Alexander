# Tutorial System - Quick Start Guide

## Overview

The tutorial system provides in-game guidance for players learning farming, landing, VR interactions, and biome exploration.

## Available Tutorials

### 1. Farming Tutorial (`FarmingBasics`)

Teaches players how to establish and manage farms:

- Finding suitable farm locations
- Creating farm plots
- Planting seeds with VR gestures
- Watering crops
- Monitoring growth
- Harvesting
- Soil management

**Duration:** ~8 steps, 5-10 minutes

### 2. Landing Tutorial (`LandingGuidance`)

Guides players through atmospheric entry and landing:

- Approaching planets
- Atmospheric entry effects
- Finding landing zones
- Using landing guidance system
- Final approach and alignment
- Touchdown procedures

**Duration:** ~7 steps, 5-8 minutes

### 3. VR Interaction Tutorial (`VRInteractions`)

Introduces VR motion controller interactions:

- Movement controls
- Grabbing objects
- Using tools
- Planting gestures
- Harvesting
- Inventory access
- Comfort settings

**Duration:** ~8 steps, 5-10 minutes

### 4. Biome Exploration Tutorial (`BiomeExploration`)

Teaches biome navigation and survival:

- Identifying biomes
- Environmental hazards
- Resource gathering
- Weather effects
- Biome transitions
- Farming suitability
- Navigation tips

**Duration:** ~8 steps, 5-8 minutes

## Starting Tutorials

### From C++

```cpp
// Get tutorial system
UTutorialSystem* TutorialSystem = GetWorld()->GetSubsystem<UTutorialSystem>();

// Start a tutorial
TutorialSystem->StartTutorial("FarmingBasics");
TutorialSystem->StartTutorial("LandingGuidance");
TutorialSystem->StartTutorial("VRInteractions");
TutorialSystem->StartTutorial("BiomeExploration");
```

### From Blueprint

```
Get Tutorial System → Start Tutorial (Tutorial Name: "FarmingBasics")
```

### Automatic Triggers

Tutorials can be triggered automatically based on game events:

```cpp
// Trigger farming tutorial when player first lands
void APlayerCharacter::OnLanded()
{
	UTutorialSystem* TutorialSystem = GetWorld()->GetSubsystem<UTutorialSystem>();
	if (!TutorialSystem->HasCompletedTutorial("FarmingBasics"))
	{
		TutorialSystem->StartTutorial("FarmingBasics");
	}
}

// Trigger landing tutorial when approaching first planet
void ASpaceship::OnApproachingPlanet()
{
	UTutorialSystem* TutorialSystem = GetWorld()->GetSubsystem<UTutorialSystem>();
	if (!TutorialSystem->HasCompletedTutorial("LandingGuidance"))
	{
		TutorialSystem->StartTutorial("LandingGuidance");
	}
}
```

## Tutorial Completion

### Manual Completion

Some tutorial steps require player action:

```cpp
// Mark step as completed when player performs action
void AFarmPlot::OnCropPlanted()
{
	UTutorialSystem* TutorialSystem = GetWorld()->GetSubsystem<UTutorialSystem>();
	if (TutorialSystem->IsTutorialActive())
	{
		// Check if current step requires this action
		FTutorialStep CurrentStep = TutorialSystem->GetCurrentStep();
		if (CurrentStep.CompletionTag == "CropPlanted")
		{
			TutorialSystem->CompleteCurrentStep();
		}
	}
}
```

### Auto-Completion

Steps without `bRequiresCompletion` advance automatically after their duration.

### Skipping

Players can skip tutorials if `bCanSkip` is true:

```cpp
// Skip current tutorial
TutorialSystem->SkipCurrentTutorial();
```

## Tutorial UI

### Creating Tutorial Widget

1. Create a UMG widget blueprint based on `UTutorialWidget`
2. Add text blocks for:
   - Title
   - Description
   - Hint
   - Step counter (e.g., "Step 3/8")
3. Add buttons:
   - Skip button (calls `OnSkipButtonClicked`)
   - Next button (calls `OnNextButtonClicked`)
4. Add progress bar bound to `StepProgress`

### Example Widget Layout

```
┌─────────────────────────────────────┐
│  Tutorial: Farming Basics           │
│  Step 3/8                           │
├─────────────────────────────────────┤
│                                     │
│  Creating a Farm Plot               │
│                                     │
│  Open your build menu and select    │
│  'Farm Plot'. Place it on suitable  │
│  terrain. The plot will show green  │
│  if the location is valid.          │
│                                     │
│  Hint: Place a farm plot            │
│                                     │
│  [████████░░░░░░░░] 37%            │
│                                     │
│  [Skip]              [Next]         │
└─────────────────────────────────────┘
```

## Checking Tutorial Status

```cpp
// Check if player has completed a tutorial
bool bCompleted = TutorialSystem->HasCompletedTutorial("FarmingBasics");

// Check if any tutorial is currently active
bool bActive = TutorialSystem->IsTutorialActive();

// Get current step information
FTutorialStep CurrentStep = TutorialSystem->GetCurrentStep();
int32 StepIndex = TutorialSystem->GetCurrentStepIndex();
int32 TotalSteps = TutorialSystem->GetTotalSteps();
```

## Creating Custom Tutorials

### Define Tutorial Sequence

```cpp
FTutorialSequence CustomTutorial;
CustomTutorial.SequenceName = "MyCustomTutorial";
CustomTutorial.SequenceTitle = FText::FromString("Custom Tutorial");
CustomTutorial.bCanSkip = true;
CustomTutorial.bShowOnce = true;

// Add steps
FTutorialStep Step1;
Step1.Title = FText::FromString("Step Title");
Step1.Description = FText::FromString("Step description text");
Step1.HintText = FText::FromString("Hint for player");
Step1.Duration = 5.0f;
Step1.bRequiresCompletion = false;
CustomTutorial.Steps.Add(Step1);

// Register tutorial
TutorialSystem->RegisterTutorial(CustomTutorial);
```

## Best Practices

### Tutorial Design

1. **Keep steps short** - 5-10 seconds for auto-advance steps
2. **Clear objectives** - Tell players exactly what to do
3. **Progressive complexity** - Start simple, build up
4. **Allow skipping** - Don't force tutorials on experienced players
5. **Show once** - Don't repeat tutorials unless requested

### Step Requirements

1. **Auto-advance for information** - Use for explanatory steps
2. **Require completion for actions** - Use for interactive steps
3. **Provide clear hints** - Tell players what action completes the step
4. **Use completion tags** - Match tags to game events

### UI Considerations

1. **Non-intrusive** - Don't block gameplay
2. **VR-friendly** - Large text, comfortable viewing distance
3. **Pausable** - Allow players to pause and resume
4. **Accessible** - Support different languages and accessibility needs

## Troubleshooting

### Tutorial Not Starting

- Check tutorial name is correct
- Verify tutorial system is initialized
- Check if tutorial was already completed (if `bShowOnce` is true)

### Step Not Advancing

- Verify completion tag matches game event
- Check if `bRequiresCompletion` is set correctly
- Ensure `CompleteCurrentStep()` is being called

### UI Not Showing

- Check widget visibility
- Verify tutorial system reference is valid
- Check if `UpdateTutorialDisplay()` is being called

## Console Commands

```cpp
// Start tutorial
StartTutorial FarmingBasics

// Skip current tutorial
SkipTutorial

// Complete current step
CompleteTutorialStep

// Reset tutorial progress
ResetTutorial FarmingBasics

// List all tutorials
ListTutorials

// Show tutorial debug info
ShowDebug Tutorial
```

---

**Version:** 1.0  
**Last Updated:** 2025-11-04
