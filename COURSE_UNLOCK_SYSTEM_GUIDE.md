# Course Unlocking System Implementation Guide

## Overview

Complete implementation specification for progressive course unlocking system with difficulty tiers, secret courses, achievements, and reward unlocks.

## System Design

### Unlock Progression Tree

```
Tutorial (Always Available)
    |
    +--> Easy Courses (Unlock: Complete Tutorial)
            |
            +--> Medium Courses (Unlock: Complete any Easy)
                    |
                    +--> Hard Courses (Unlock: Complete any Medium)
                            |
                            +--> Expert Courses (Unlock: Complete any Hard) [Future]
```

### Special Unlocks

In addition to the main progression tree, players can unlock:
- **Secret Courses**: Hidden courses for achieving all gold times in a difficulty tier
- **Titles**: Cosmetic player titles for achievements
- **Ship Skins**: Visual customization rewards
- **Achievements**: Major milestones with multi-part rewards

---

## Data Structures

### FPlayerCourseProgress

Stores progress for a single course.

```cpp
USTRUCT(BlueprintType)
struct FPlayerCourseProgress
{
    GENERATED_BODY()

    // Course identifier
    UPROPERTY(SaveGame, BlueprintReadWrite)
    FString CourseName;

    // Completion status
    UPROPERTY(SaveGame, BlueprintReadWrite)
    bool bCompleted = false;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    float BestTime = 0.0f;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    int32 TimesCompleted = 0;

    // Rewards earned
    UPROPERTY(SaveGame, BlueprintReadWrite)
    int32 TotalXPEarned = 0;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    int32 TotalCreditsEarned = 0;

    // Time medals
    UPROPERTY(SaveGame, BlueprintReadWrite)
    bool bGoldTime = false;  // <= 90% of par time

    UPROPERTY(SaveGame, BlueprintReadWrite)
    bool bSilverTime = false;  // <= Par time

    UPROPERTY(SaveGame, BlueprintReadWrite)
    bool bBronzeTime = false;  // <= 110% of par time

    // Timestamps
    UPROPERTY(SaveGame, BlueprintReadWrite)
    FDateTime FirstCompletionDate;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    FDateTime LastCompletionDate;
};
```

### FPlayerGlobalProgress

Overall player progression.

```cpp
USTRUCT(BlueprintType)
struct FPlayerGlobalProgress
{
    GENERATED_BODY()

    // Tutorial status
    UPROPERTY(SaveGame, BlueprintReadWrite)
    bool bTutorialCompleted = false;

    // Global stats
    UPROPERTY(SaveGame, BlueprintReadWrite)
    int32 TotalXP = 0;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    int32 TotalCredits = 0;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    int32 PlayerLevel = 1;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    int32 TotalCoursesCompleted = 0;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    float TotalPlayTime = 0.0f;  // seconds

    // Unlocked content
    UPROPERTY(SaveGame, BlueprintReadWrite)
    TArray<EDifficultyLevel> UnlockedDifficulties;  // Default: [Tutorial]

    UPROPERTY(SaveGame, BlueprintReadWrite)
    TMap<FString, FPlayerCourseProgress> CourseProgress;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    TArray<FString> UnlockedTitles;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    TArray<FString> UnlockedSkins;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    TArray<FString> AchievementsUnlocked;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    TArray<FString> SecretCoursesUnlocked;
};
```

### FUnlockNotification

Notification data for UI display.

```cpp
USTRUCT(BlueprintType)
struct FUnlockNotification
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Type;  // "DifficultyTier", "SecretCourse", "Achievement", "Title", "Skin"

    UPROPERTY(BlueprintReadWrite)
    FString Title;  // "Medium Courses Unlocked!"

    UPROPERTY(BlueprintReadWrite)
    FString Description;  // "You can now access all Medium difficulty courses"

    UPROPERTY(BlueprintReadWrite)
    UTexture2D* Icon;  // Display icon

    UPROPERTY(BlueprintReadWrite)
    float DisplayDuration = 5.0f;

    UPROPERTY(BlueprintReadWrite)
    USoundBase* Sound;  // Notification sound
};
```

---

## UUnlockSystem (Game Instance Subsystem)

Central system for managing unlocks.

### Setup

Create C++ class:
```cpp
UCLASS()
class ALEXANDER_API UUnlockSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core unlock logic
    UFUNCTION(BlueprintCallable, Category = "Unlocks")
    bool IsCourseUnlocked(FString CourseName);

    UFUNCTION(BlueprintCallable, Category = "Unlocks")
    FString GetUnlockRequirementText(FString CourseName);

    UFUNCTION(BlueprintCallable, Category = "Unlocks")
    bool HasCompletedAnyDifficulty(EDifficultyLevel Difficulty);

    // Course completion handler
    UFUNCTION(BlueprintCallable, Category = "Progression")
    void OnCourseCompleted(FString CourseName, float CompletionTime, int32 XPEarned, int32 CreditsEarned);

    // Progression queries
    UFUNCTION(BlueprintCallable, Category = "Progression")
    FPlayerCourseProgress GetCourseProgress(FString CourseName);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    FPlayerGlobalProgress GetGlobalProgress();

    // Save/Load
    UFUNCTION(BlueprintCallable, Category = "Save")
    void SavePlayerProgress();

    UFUNCTION(BlueprintCallable, Category = "Save")
    bool LoadPlayerProgress();

protected:
    // Internal progression data
    UPROPERTY()
    FPlayerGlobalProgress PlayerProgress;

    // Pending notifications to display
    UPROPERTY()
    TArray<FUnlockNotification> PendingUnlockNotifications;

    // Broadcast unlock events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnlockNotification, FUnlockNotification, Notification);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnUnlockNotification OnUnlockNotification;

    // Helper functions (see unlock_logic_spec for implementations)
    void CheckDifficultyUnlocks(FString CompletedCourse);
    void CheckSpecialUnlocks();
    bool HasAllGoldTimes(EDifficultyLevel Difficulty);
    int32 CountCoursesWithParTime();
};
```

### Key Function Implementations

See `unlock_logic_spec` in JSON export for full C++ implementations of:
- `IsCourseUnlocked()`
- `HasCompletedAnyDifficulty()`
- `OnCourseCompleted()`
- `CheckDifficultyUnlocks()`
- `CheckSpecialUnlocks()`
- `GetUnlockRequirementText()`

---

## Unlock Tiers

### Tier 1: Tutorial (Always Available)

**Courses**:
- Flight Training Academy

**Unlock Requirement**: None - always available

**Unlocks on Completion**:
- All Easy difficulty courses
- Sets `bTutorialCompleted = true`
- Adds `EDifficultyLevel::Easy` to `UnlockedDifficulties`

---

### Tier 2: Easy

**Courses**:
- Beginner's Canyon
- Morning Run (future)
- Peaceful Valley (future)

**Unlock Requirement**: Complete Tutorial

**Unlocks on Completion** (any Easy course):
- All Medium difficulty courses
- Adds `EDifficultyLevel::Medium` to `UnlockedDifficulties`

---

### Tier 3: Medium

**Courses**:
- Industrial Nexus
- Debris Field Navigation (future)
- Station Weave (future)

**Unlock Requirement**: Complete any Easy course

**Unlocks on Completion** (any Medium course):
- All Hard difficulty courses
- Adds `EDifficultyLevel::Hard` to `UnlockedDifficulties`

---

### Tier 4: Hard

**Courses**:
- Asteroid Gauntlet
- The Crucible (future)
- Expert Challenge (future)

**Unlock Requirement**: Complete any Medium course

**Unlocks on Completion** (any Hard course):
- Expert tier (future expansion)

---

## Special Unlocks

### Secret Courses

Hidden courses unlocked by achieving all gold times in a difficulty tier.

#### Secret Easy Course: "Hidden Canyon"

**Unlock Requirement**: Achieve gold time (90% of par) on ALL Easy courses

**Check Logic**:
```cpp
bool HasAllEasyGoldTimes()
{
    TArray<FString> EasyCourses = {"Beginner's Canyon", "Morning Run", "Peaceful Valley"};

    for (FString Course : EasyCourses)
    {
        FPlayerCourseProgress* Progress = PlayerProgress.CourseProgress.Find(Course);
        if (!Progress || !Progress->bGoldTime)
            return false;
    }

    return true;
}
```

**Reward**: Unlock "Hidden Canyon" secret course

---

#### Secret Medium Course: "The Labyrinth"

**Unlock Requirement**: Achieve gold time on ALL Medium courses

**Reward**: Unlock "The Labyrinth" secret course

---

#### Secret Hard Course: "Death Run"

**Unlock Requirement**: Achieve gold time on ALL Hard courses

**Reward**: Unlock "Death Run" secret course (ultimate challenge)

---

### Achievements

#### Achievement: "Academy Graduate"

**Unlock Requirement**: Complete tutorial without any collisions

**Rewards**:
- Title: "Academy Graduate"
- Ship Skin: "Training Stripes"

**Implementation**:
- Track collision count during tutorial
- If collision count == 0 on completion, award achievement

---

#### Achievement: "Speed Demon"

**Unlock Requirement**: Beat par time on 10 or more courses (any difficulty)

**Rewards**:
- Title: "Speed Demon"
- Ship Skin: "Racing Flames"

**Implementation**:
```cpp
int32 CountCoursesWithParTime()
{
    int32 Count = 0;
    for (auto& Entry : PlayerProgress.CourseProgress)
    {
        if (Entry.Value.bSilverTime)  // Silver = par time or better
            Count++;
    }
    return Count;
}
```

---

#### Achievement: "Master Pilot"

**Unlock Requirement**: Complete ALL courses (including secret courses)

**Rewards**:
- Title: "Master Pilot"
- Ship Skin: "Chrome Finish"

**Implementation**:
```cpp
if (PlayerProgress.TotalCoursesCompleted >= GetTotalCoursesCount())
{
    AwardAchievement("Master Pilot");
}
```

---

## Time Medals

Each course has 3 time medal tiers:

- **Gold**: Complete in <= 90% of par time
- **Silver**: Complete in <= par time (100%)
- **Bronze**: Complete in <= 110% of par time

**Example** (Par time = 60 seconds):
- Gold: <= 54 seconds
- Silver: <= 60 seconds
- Bronze: <= 66 seconds

**Implementation**:
```cpp
void AwardTimeMedals(FString CourseName, float CompletionTime)
{
    UFlightCourseData* CourseData = GetCourseData(CourseName);
    FPlayerCourseProgress& Progress = PlayerProgress.CourseProgress.FindOrAdd(CourseName);

    if (CourseData->ParTime > 0)
    {
        float GoldTime = CourseData->ParTime * 0.9f;
        float SilverTime = CourseData->ParTime;
        float BronzeTime = CourseData->ParTime * 1.1f;

        if (CompletionTime <= GoldTime)
        {
            Progress.bGoldTime = true;
            ShowMedalNotification("Gold", CourseName);
        }
        else if (CompletionTime <= SilverTime)
        {
            Progress.bSilverTime = true;
            ShowMedalNotification("Silver", CourseName);
        }
        else if (CompletionTime <= BronzeTime)
        {
            Progress.bBronzeTime = true;
            ShowMedalNotification("Bronze", CourseName);
        }
    }
}
```

---

## UI Integration

### Course Selection Menu

Update `WBP_CourseCard` widget:

1. **Check Unlock Status**:
```cpp
void UWBPCourseCard::UpdateLockState()
{
    UUnlockSystem* UnlockSystem = GetGameInstance()->GetSubsystem<UUnlockSystem>();
    bIsLocked = !UnlockSystem->IsCourseUnlocked(CourseName);

    if (bIsLocked)
    {
        // Show lock overlay
        LockOverlay->SetVisibility(ESlateVisibility::Visible);

        // Get unlock requirement text
        FString Requirement = UnlockSystem->GetUnlockRequirementText(CourseName);
        UnlockRequirementText->SetText(FText::FromString(Requirement));

        // Disable interaction
        SelectButton->SetIsEnabled(false);
    }
    else
    {
        LockOverlay->SetVisibility(ESlateVisibility::Collapsed);
        SelectButton->SetIsEnabled(true);
    }
}
```

2. **Display Time Medals**:
```cpp
void UWBPCourseCard::DisplayMedals()
{
    UUnlockSystem* UnlockSystem = GetGameInstance()->GetSubsystem<UUnlockSystem>();
    FPlayerCourseProgress Progress = UnlockSystem->GetCourseProgress(CourseName);

    // Show medal icons
    GoldMedalIcon->SetVisibility(Progress.bGoldTime ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    SilverMedalIcon->SetVisibility(Progress.bSilverTime ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    BronzeMedalIcon->SetVisibility(Progress.bBronzeTime ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
```

---

### Unlock Notification Widget (WBP_UnlockNotification)

**Purpose**: Display popup notifications for unlocks

**Layout**:
```
+------------------------------------+
| [Icon]  Medium Courses Unlocked!  |
|         You can now access all    |
|         Medium difficulty courses |
+------------------------------------+
```

**Hierarchy**:
- Canvas Panel
  - Border (Background with glow)
    - Horizontal Box
      - Image (Icon) - 64x64
      - Vertical Box
        - Text Block (Title) - Gold color, 24pt
        - Text Block (Description) - White, 16pt

**Animation**:
1. **Spawn**: Slide in from right + fade in (0.3s)
2. **Display**: Gentle pulse glow (5 seconds)
3. **Despawn**: Slide out right + fade out (0.3s)

**Blueprint Implementation**:
```cpp
void UWBPUnlockNotification::ShowNotification(FUnlockNotification Notification)
{
    // Set content
    TitleText->SetText(FText::FromString(Notification.Title));
    DescriptionText->SetText(FText::FromString(Notification.Description));
    IconImage->SetBrushFromTexture(Notification.Icon);

    // Play enter animation
    PlayAnimation(EnterAnimation);

    // Play sound
    if (Notification.Sound)
        PlaySound(Notification.Sound);

    // Auto-despawn after duration
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this,
        &UWBPUnlockNotification::DespawnNotification,
        Notification.DisplayDuration, false);
}

void UWBPUnlockNotification::DespawnNotification()
{
    PlayAnimation(ExitAnimation);

    // Remove from viewport after animation
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        RemoveFromParent();
    }, 0.3f, false);
}
```

---

### Notification Manager

**Purpose**: Queue and display multiple unlock notifications

```cpp
UCLASS()
class ANotificationManager : public AActor
{
    GENERATED_BODY()

protected:
    UPROPERTY()
    TArray<FUnlockNotification> NotificationQueue;

    UPROPERTY()
    UWBPUnlockNotification* CurrentNotification = nullptr;

public:
    void EnqueueNotification(FUnlockNotification Notification)
    {
        NotificationQueue.Add(Notification);
        ProcessQueue();
    }

    void ProcessQueue()
    {
        // Don't show new notification if one is already displayed
        if (CurrentNotification && CurrentNotification->IsInViewport())
            return;

        if (NotificationQueue.Num() == 0)
            return;

        // Show next notification
        FUnlockNotification Next = NotificationQueue[0];
        NotificationQueue.RemoveAt(0);

        CurrentNotification = CreateWidget<UWBPUnlockNotification>(
            GetWorld(), NotificationWidgetClass);
        CurrentNotification->AddToViewport();
        CurrentNotification->ShowNotification(Next);

        // Check queue again after notification duration
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this,
            &ANotificationManager::ProcessQueue,
            Next.DisplayDuration + 0.5f, false);
    }
};
```

Bind to UnlockSystem events:
```cpp
void ANotificationManager::BeginPlay()
{
    Super::BeginPlay();

    UUnlockSystem* UnlockSystem = GetGameInstance()->GetSubsystem<UUnlockSystem>();
    UnlockSystem->OnUnlockNotification.AddDynamic(this, &ANotificationManager::EnqueueNotification);
}
```

---

## Save System

### Save Game Class

```cpp
UCLASS()
class USaveGamePlayerProgress : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(SaveGame)
    FPlayerGlobalProgress ProgressData;

    UPROPERTY(SaveGame)
    FString SaveDate;

    UPROPERTY(SaveGame)
    int32 SaveVersion = 1;
};
```

### Save Function

```cpp
void UUnlockSystem::SavePlayerProgress()
{
    USaveGamePlayerProgress* SaveGame = Cast<USaveGamePlayerProgress>(
        UGameplayStatics::CreateSaveGameObject(USaveGamePlayerProgress::StaticClass()));

    if (SaveGame)
    {
        // Copy current progress
        SaveGame->ProgressData = PlayerProgress;
        SaveGame->SaveDate = FDateTime::Now().ToString();

        // Save to slot
        UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("PlayerProgress"), 0);

        // Create backup
        UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("PlayerProgress_Backup"), 0);
    }
}
```

### Load Function

```cpp
bool UUnlockSystem::LoadPlayerProgress()
{
    USaveGamePlayerProgress* SaveGame = Cast<USaveGamePlayerProgress>(
        UGameplayStatics::LoadGameFromSlot(TEXT("PlayerProgress"), 0));

    if (SaveGame)
    {
        PlayerProgress = SaveGame->ProgressData;
        return true;
    }
    else
    {
        // Initialize default progress
        PlayerProgress = FPlayerGlobalProgress();
        PlayerProgress.UnlockedDifficulties.Add(EDifficultyLevel::Tutorial);
        return false;
    }
}
```

---

## Testing Checklist

### Core Unlock Logic
- [ ] Tutorial always unlocked
- [ ] Easy courses locked until tutorial complete
- [ ] Medium courses locked until any Easy complete
- [ ] Hard courses locked until any Medium complete
- [ ] Unlock requirement text displays correctly

### Course Completion
- [ ] Completing course awards XP and Credits
- [ ] Best time updates correctly
- [ ] Time medals awarded correctly (Gold/Silver/Bronze)
- [ ] First completion bonus awarded
- [ ] Times completed counter increments

### Difficulty Tier Unlocks
- [ ] Tutorial completion unlocks Easy tier
- [ ] First Easy completion unlocks Medium tier
- [ ] First Medium completion unlocks Hard tier
- [ ] Unlock notifications display correctly
- [ ] Course selection menu updates after unlock

### Secret Courses
- [ ] All Easy gold times unlocks Hidden Canyon
- [ ] All Medium gold times unlocks The Labyrinth
- [ ] All Hard gold times unlocks Death Run
- [ ] Secret course notifications display
- [ ] Secret courses appear in menu when unlocked

### Achievements
- [ ] Perfect tutorial awards Academy Graduate
- [ ] 10 par time beats awards Speed Demon
- [ ] All courses complete awards Master Pilot
- [ ] Achievement notifications display
- [ ] Titles and skins unlock correctly

### Save/Load
- [ ] Progress saves on course completion
- [ ] Progress loads on game start
- [ ] All unlock states persist
- [ ] Backup save created
- [ ] Corrupted save recoverable from backup

### UI
- [ ] Locked courses show lock overlay
- [ ] Unlock requirements display correctly
- [ ] Medal icons display on completed courses
- [ ] Notifications queue properly (multiple unlocks)
- [ ] Notifications animate smoothly

---

## Future Enhancements

1. **Seasonal Content**: Time-limited courses and challenges
2. **Daily Challenges**: Rotating objectives with bonus rewards
3. **Prestige System**: Reset progress for special rewards
4. **Course Creator**: Player-created courses with unlock requirements
5. **Multiplayer Unlocks**: Competitive mode courses
6. **Weekly Leaderboards**: Special rewards for top performers

---

## Conclusion

Complete course unlocking system with:
- **4 difficulty tiers** with progressive unlocking
- **3 secret courses** for master-level play
- **3 achievements** with title and skin rewards
- **Time medals** (Gold/Silver/Bronze) for all courses
- **Notification system** for unlock feedback
- **Save system** with backup support

Estimated implementation time: **8-12 hours**

**Status**: Design Complete - Ready for Implementation
