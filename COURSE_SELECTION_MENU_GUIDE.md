# Course Selection Menu Implementation Guide

## Overview

Complete specification for implementing a course selection menu system with UMG widgets and Blueprint logic. Allows players to browse, select, and launch flight courses with unlock progression.

## System Architecture

```
BP_CourseSelectionController (Actor)
    |
    +-- WBP_CourseSelectionMenu (User Widget)
            |
            +-- WBP_CourseCard (User Widget) [Array]
            +-- WBP_CourseDetails (User Widget)
            +-- UI Buttons (Start, Back)
```

## Component Breakdown

### 1. Main Menu Widget (WBP_CourseSelectionMenu)

**Widget Type**: User Widget (UMG)
**Canvas Size**: 1920x1080 (scales automatically)

#### Widget Hierarchy

```
Canvas Panel
|
+-- Overlay (BackgroundOverlay)
|   +-- Image (BG_Image) - Full screen, space background
|   +-- BackgroundBlur (BG_Blur) - Blur strength: 10
|
+-- Vertical Box (MainContainer)
    +-- Horizontal Box (TitleSection)
    |   +-- Text Block: "SELECT FLIGHT COURSE"
    |   |   Font: Roboto Bold 48pt
    |   |   Color: White
    |   |   Shadow: (2, 2) offset, 0.5 opacity
    |   +-- Spacer (fills space)
    |   +-- Button (BackButton) - "BACK"
    |
    +-- Scroll Box (CourseListSection)
    |   Size: Fill, Height 400
    |   Orientation: Horizontal
    |   Scroll Bar Visibility: Collapsed
    |   [Course cards added dynamically]
    |
    +-- Border (DetailsSection)
    |   Padding: 20
    |   Background: (0.05, 0.05, 0.05, 0.9)
    |   +-- WBP_CourseDetails widget
    |
    +-- Horizontal Box (ButtonSection)
        Alignment: Center
        Padding: 30
        +-- Button (StartButton)
            Size: 300x80
            Text: "START COURSE"
            Font: Roboto Bold 28pt
            Color: Green when enabled, Gray when disabled
            Initial State: Disabled
```

#### Widget Variables (Blueprint)

```cpp
// Widget references
UPROPERTY(meta = (BindWidget))
UScrollBox* CourseListScrollBox;

UPROPERTY(meta = (BindWidget))
UWBPCourseDetails* DetailsPanel;

UPROPERTY(meta = (BindWidget))
UButton* StartButton;

UPROPERTY(meta = (BindWidget))
UButton* BackButton;

// Controller reference
UPROPERTY(BlueprintReadWrite)
ABPCourseSelectionController* Controller;
```

#### Widget Events

```
Event Construct:
  - Bind button click events
  - Set initial state (Start button disabled)

OnStartButtonClicked:
  - Call Controller->StartSelectedCourse()

OnBackButtonClicked:
  - Call Controller->ReturnToMainMenu()
```

---

### 2. Course Card Widget (WBP_CourseCard)

**Widget Type**: User Widget (UMG)
**Size**: 500x300

#### Widget Hierarchy

```
Button (SelectButton) - Full size, transparent
|
+-- Border (CardBackground)
    Brush Color: (0.05, 0.05, 0.05, 0.95)
    Padding: 20
    |
    +-- Vertical Box (CardContent)
        +-- Text Block (CourseTitle)
        |   Font: Roboto Bold 32pt
        |   Color: White
        |   Text: [Bound to CourseName]
        |
        +-- Text Block (DifficultyBadge)
        |   Font: Roboto Bold 20pt
        |   Color: [Bound to DifficultyColor]
        |   Text: [Bound to DifficultyText]
        |
        +-- Text Block (CourseDescription)
        |   Font: Roboto Regular 16pt
        |   Color: (0.7, 0.7, 0.7, 1)
        |   Auto Wrap: True
        |   Text: [Bound to Description]
        |
        +-- Spacer (fills space)
        |
        +-- Horizontal Box (StatsRow)
        |   +-- Image (DistanceIcon) + Text Block (DistanceText)
        |   +-- Image (TimeIcon) + Text Block (ParTimeText)
        |   +-- Image (RewardIcon) + Text Block (RewardText)
        |
        +-- Text Block (BestTimeDisplay)
        |   Font: Roboto Bold 18pt
        |   Color: Gold (1, 0.8, 0, 1)
        |   Text: [Bound to BestTimeText]
        |   Visibility: [Bound to HasBestTime]
        |
        +-- Overlay (LockOverlay)
            Visibility: [Bound to IsLocked]
            +-- Image (DarkOverlay)
            |   Color: (0, 0, 0, 0.7)
            |   Fill screen
            +-- Image (LockIcon)
            |   Size: 64x64, Center aligned
            +-- Text Block (UnlockRequirement)
                Font: Roboto Bold 16pt
                Center aligned
                Text: [Bound to UnlockText]
```

#### Widget Variables

```cpp
// Course data
UPROPERTY(BlueprintReadWrite)
UFlightCourseData* CourseData;

UPROPERTY(BlueprintReadWrite)
int32 CourseIndex;

UPROPERTY(BlueprintReadWrite)
bool bIsLocked;

UPROPERTY(BlueprintReadWrite)
float BestTime;

// Cached text/colors
FString CourseName;
FString Description;
FString DifficultyText;
FLinearColor DifficultyColor;
FString DistanceText;
FString ParTimeText;
FString RewardText;
FString BestTimeText;
FString UnlockText;
```

#### Widget Functions

```cpp
// Set course data and update display
void SetCourseData(UFlightCourseData* InCourseData)
{
    CourseData = InCourseData;

    // Cache display strings
    CourseName = CourseData->CourseName;
    Description = CourseData->Description;

    // Difficulty
    switch (CourseData->Difficulty)
    {
        case EDifficultyLevel::Easy:
            DifficultyText = "EASY";
            DifficultyColor = FLinearColor(0, 1, 0, 1);
            break;
        case EDifficultyLevel::Medium:
            DifficultyText = "MEDIUM";
            DifficultyColor = FLinearColor(1, 0.8, 0, 1);
            break;
        case EDifficultyLevel::Hard:
            DifficultyText = "HARD";
            DifficultyColor = FLinearColor(1, 0, 0, 1);
            break;
    }

    // Stats
    DistanceText = FString::Printf(TEXT("%.0f units"), CourseData->TotalDistance);
    ParTimeText = FString::Printf(TEXT("Par: %.0fs"), CourseData->ParTime);
    RewardText = FString::Printf(TEXT("+%d XP"), CourseData->CompletionXP);

    // Force refresh bindings
    SynchronizeProperties();
}

void SetLockedState(bool bLocked)
{
    bIsLocked = bLocked;

    if (bIsLocked)
    {
        // Set unlock requirement text based on difficulty
        if (CourseData->Difficulty == EDifficultyLevel::Medium)
            UnlockText = "Complete any Easy course to unlock";
        else if (CourseData->Difficulty == EDifficultyLevel::Hard)
            UnlockText = "Complete any Medium course to unlock";
    }

    SynchronizeProperties();
}

void SetBestTime(float InBestTime)
{
    BestTime = InBestTime;

    if (BestTime > 0)
    {
        BestTimeText = FString::Printf(TEXT("Best: %.2fs"), BestTime);
    }

    SynchronizeProperties();
}
```

#### Widget Events

```
OnSelectButtonClicked:
  - If IsLocked: Play error sound, return
  - Fire OnCardSelected event with CourseIndex
  - Play selection animation (highlight border)

OnSelectButtonHovered:
  - Play hover animation (scale 1.05)
  - Play hover sound

OnSelectButtonUnhovered:
  - Play unhover animation (scale 1.0)
```

#### Property Bindings

```
CourseTitleText: Get -> Return CourseName
DifficultyBadgeText: Get -> Return DifficultyText
DifficultyBadgeColor: Get -> Return DifficultyColor
DescriptionText: Get -> Return Description
DistanceText: Get -> Return DistanceText
ParTimeText: Get -> Return ParTimeText
RewardText: Get -> Return RewardText
BestTimeText: Get -> Return BestTimeText
BestTimeVisibility: Get -> Return (BestTime > 0) ? Visible : Collapsed
LockOverlayVisibility: Get -> Return bIsLocked ? Visible : Collapsed
UnlockRequirementText: Get -> Return UnlockText
```

---

### 3. Course Details Panel (WBP_CourseDetails)

**Widget Type**: User Widget (UMG)
**Size**: Fills available space

#### Widget Hierarchy

```
Horizontal Box
|
+-- Image (PreviewImage)
|   Size: 600x400
|   Stretch: Uniform to Fill
|   Brush: [Bound to CoursePreviewImage]
|
+-- Vertical Box (DetailsList)
    Padding: 30
    |
    +-- Text Block (CourseName)
    |   Font: Roboto Bold 36pt
    |   Color: White
    |
    +-- Text Block (DifficultyLabel)
    |   Font: Roboto Bold 24pt
    |   Color: [Bound to DifficultyColor]
    |
    +-- Rich Text Block (DescriptionText)
    |   Font: Roboto Regular 18pt
    |   Color: (0.8, 0.8, 0.8, 1)
    |   Auto Wrap: True
    |
    +-- Vertical Box (ObjectivesSection)
    |   +-- Text Block: "OBJECTIVES"
    |   +-- Text Block: "- Complete all waypoints"
    |   +-- Text Block: "- Pass through all rings"
    |   +-- Text Block: [Difficulty-specific objective]
    |
    +-- Uniform Grid Panel (StatisticsGrid) - 2 columns
    |   +-- Stat blocks for:
    |       - Total Distance
    |       - Par Time
    |       - Recommended Time
    |       - Waypoints Count
    |       - Rings Count
    |       - Max Velocity
    |       - Completion XP
    |       - Completion Credits
    |
    +-- Vertical Box (LeaderboardPreview)
        +-- Text Block: "TOP TIMES"
        +-- List of top 5 times (if available)
```

#### Widget Functions

```cpp
void UpdateCourseDetails(UFlightCourseData* CourseData)
{
    // Update all text blocks with course data
    // Load preview image
    // Update statistics grid
    // Load leaderboard data (top 5 times)
}
```

---

### 4. Controller Blueprint (BP_CourseSelectionController)

**Blueprint Type**: Actor
**Components**: None (pure logic controller)

#### Variables

Already specified in menu_controller_spec above.

#### Functions

See menu_controller_spec for all function implementations.

#### Key Logic Flow

1. **Initialization** (BeginPlay):
   - Load all course data assets from `/Game/FlightCourses/`
   - Get PlayerProgressionComponent reference
   - Create and display WBP_CourseSelectionMenu
   - Populate course list with WBP_CourseCard instances
   - Set input mode to UI

2. **Course Selection**:
   - Player clicks course card
   - Check if unlocked
   - If locked: Show error, display unlock requirement
   - If unlocked: Update details panel, highlight card, enable start button

3. **Course Launch**:
   - Player clicks Start button
   - Save selected course to Game Instance
   - Play fade out animation
   - Load course level (async)
   - When loaded: Remove menu, set input to game mode

4. **Back Navigation**:
   - Player clicks Back button
   - Fade out menu
   - Destroy controller
   - Return to previous menu/level

---

## Data Asset Setup

### Course Data Asset (DA_FlightCourse)

Create 3 data assets inheriting from `UFlightCourseData`:

1. **DA_BeginnerCanyon**
   - Course Name: "Beginner's Canyon"
   - Difficulty: Easy
   - Description: "Wide open canyon run perfect for learning flight controls"
   - Par Time: 60
   - Recommended Time: 75
   - Completion XP: 150
   - Completion Credits: 50
   - Time Bonus Multiplier: 1.0
   - First Completion Bonus: 100
   - Course Level: `/Game/Levels/L_BeginnerCanyon`
   - Physics Config: `DA_PhysicsConfig_Easy`
   - Preview Image: `T_Preview_BeginnerCanyon`

2. **DA_IndustrialNexus**
   - Course Name: "Industrial Nexus"
   - Difficulty: Medium
   - [Similar fields with Medium values]

3. **DA_AsteroidGauntlet**
   - Course Name: "Asteroid Gauntlet"
   - Difficulty: Hard
   - [Similar fields with Hard values]

---

## Integration with Existing Systems

### PlayerProgressionComponent

Required new functions:

```cpp
// Check if player has completed any course of given difficulty
bool HasCompletedAnyDifficulty(EDifficultyLevel Difficulty);

// Get best time for specific course
float GetCourseBestTime(FString CourseName);

// Save course completion
void SaveCourseCompletion(FString CourseName, float CompletionTime, int32 XPEarned, int32 CreditsEarned);
```

### Game Instance

Required new variables:

```cpp
// Currently active course
UPROPERTY(BlueprintReadWrite)
UFlightCourseData* ActiveCourseData;

// Set active course before loading level
void SetActiveCourse(UFlightCourseData* CourseData);
```

### Course Manager (in course levels)

```cpp
// On level load, get active course from Game Instance
void BeginPlay()
{
    UAlexanderGameInstance* GI = Cast<UAlexanderGameInstance>(GetGameInstance());
    if (GI && GI->ActiveCourseData)
    {
        // Load physics config
        ApplyPhysicsConfig(GI->ActiveCourseData->PhysicsConfig);

        // Setup course parameters
        ParTime = GI->ActiveCourseData->ParTime;
        CompletionXP = GI->ActiveCourseData->CompletionXP;
        // etc.
    }
}
```

---

## UI/UX Features

### Animations

1. **Menu Fade In** (Event Construct):
   - Duration: 0.3s
   - Opacity: 0 -> 1
   - Position: Y +50 -> 0

2. **Menu Fade Out**:
   - Duration: 0.3s
   - Opacity: 1 -> 0

3. **Card Hover**:
   - Duration: 0.2s
   - Scale: 1.0 -> 1.05
   - Border brightness increase

4. **Card Select**:
   - Border color change to accent blue
   - Glow effect

5. **Button Press**:
   - Duration: 0.1s
   - Scale: 1.0 -> 0.95 -> 1.0

### Sound Effects

Required sound cues:
- `UI_CourseSelect_Cue` - Positive selection sound
- `UI_Locked_Cue` - Negative/error sound
- `UI_CourseStart_Cue` - Dramatic start sound
- `UI_Back_Cue` - Soft back sound
- `UI_Hover_Cue` - Subtle hover sound

---

## Implementation Steps

### Step 1: Create Data Assets (30 min)
1. Create `UFlightCourseData` C++ class (or Blueprint struct)
2. Create 3 data assets: DA_BeginnerCanyon, DA_IndustrialNexus, DA_AsteroidGauntlet
3. Fill in all properties (names, times, rewards, etc.)
4. Create placeholder preview images

### Step 2: Create Course Card Widget (45 min)
1. Create WBP_CourseCard widget
2. Build hierarchy as specified
3. Add variables and property bindings
4. Implement SetCourseData, SetLockedState, SetBestTime functions
5. Add hover/click animations
6. Test with dummy data

### Step 3: Create Details Panel Widget (30 min)
1. Create WBP_CourseDetails widget
2. Build hierarchy
3. Implement UpdateCourseDetails function
4. Test with dummy data

### Step 4: Create Main Menu Widget (45 min)
1. Create WBP_CourseSelectionMenu widget
2. Build canvas hierarchy
3. Add widget binding variables
4. Wire up button events
5. Add fade in/out animations

### Step 5: Create Controller Blueprint (1-2 hours)
1. Create BP_CourseSelectionController actor
2. Add variables for course data, selection, etc.
3. Implement InitializeMenu function
4. Implement LoadCourseData function
5. Implement PopulateCourseList function
6. Implement OnCourseSelected function
7. Implement UpdateDetailsPanel function
8. Implement StartSelectedCourse function
9. Implement course unlock logic
10. Test all functionality

### Step 6: Integrate with Progression System (1 hour)
1. Add HasCompletedAnyDifficulty to PlayerProgressionComponent
2. Add GetCourseBestTime to PlayerProgressionComponent
3. Add SaveCourseCompletion to PlayerProgressionComponent
4. Update save/load logic

### Step 7: Integrate with Game Instance (30 min)
1. Add ActiveCourseData variable to Game Instance
2. Add SetActiveCourse function
3. Test course data persistence between levels

### Step 8: Update Course Managers (1 hour)
1. Update BP_CourseManager to read from Game Instance
2. Apply physics configs
3. Test course parameters apply correctly

### Step 9: Polish and Testing (1-2 hours)
1. Add all animations
2. Add all sound effects
3. Create preview images for courses
4. Test unlock progression
5. Test course launching
6. Test leaderboard display
7. Fix any bugs

**Total Estimated Time: 7-10 hours**

---

## Testing Checklist

### Functional Testing
- [ ] Menu opens and displays correctly
- [ ] All 3 courses appear in list
- [ ] Easy course is unlocked by default
- [ ] Medium/Hard courses show as locked initially
- [ ] Clicking locked course shows error and requirement
- [ ] Clicking unlocked course updates details panel
- [ ] Start button enables only when course selected
- [ ] Start button launches correct course
- [ ] Course level loads with correct parameters
- [ ] Back button returns to previous menu
- [ ] Best times display for completed courses
- [ ] Unlock progression works (Easy -> Medium -> Hard)

### UI Testing
- [ ] All text is readable and properly sized
- [ ] Difficulty colors are correct (Green/Yellow/Red)
- [ ] Course cards scale properly on different resolutions
- [ ] Scrolling works smoothly in course list
- [ ] Hover effects work on all interactive elements
- [ ] Animations play smoothly
- [ ] No UI elements overlap incorrectly

### Performance Testing
- [ ] Menu opens without stuttering
- [ ] Course data loads quickly (<1 second)
- [ ] Level loading shows progress indicator
- [ ] No memory leaks when opening/closing menu multiple times

---

## Future Enhancements

1. **Course Filtering**:
   - Filter by difficulty
   - Filter by completion status
   - Sort by name, difficulty, best time

2. **Search Functionality**:
   - Search courses by name

3. **Course Preview**:
   - 3D preview of course layout
   - Flythrough camera animation

4. **Leaderboard Integration**:
   - Full leaderboard view
   - Friend leaderboards
   - Global leaderboards

5. **Course Ratings**:
   - Star rating system
   - Player reviews
   - Difficulty votes

6. **Course Customization**:
   - Custom physics settings
   - Custom ship selection
   - Challenge modifiers (no assists, time attack, etc.)

---

## Conclusion

Complete specification for course selection menu system with:
- 4 UMG widgets (Main menu, Course card, Details panel, and bindings)
- 1 Controller Blueprint with full logic
- Integration with progression system
- Course unlock progression
- Polish animations and sounds

**Status**: Design Complete - Ready for Implementation (CE03)
