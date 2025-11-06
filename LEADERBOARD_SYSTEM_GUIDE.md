# Time Trial Leaderboard System Implementation Guide

## Overview

Complete specification for local time trial leaderboard system with personal best tracking, rankings, medals, and detailed results screen.

## System Architecture

```
ULocalLeaderboardManager (Game Instance Subsystem)
    |
    +-- Per-Course Leaderboards (Top 10 times each)
    +-- Statistics Tracking
    +-- Personal Best Tracking
    +-- Time Medal System
    |
    +-- UI Components:
        +-- WBP_LeaderboardDisplay (Leaderboard viewer)
        +-- WBP_LeaderboardEntry (Entry row)
        +-- WBP_CourseResults (Post-completion screen)
```

---

## Data Structures

### FLeaderboardEntry

Single leaderboard entry.

```cpp
USTRUCT(BlueprintType)
struct FLeaderboardEntry
{
    GENERATED_BODY()

    UPROPERTY(SaveGame, BlueprintReadWrite)
    FString PlayerName;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    float CompletionTime;  // Seconds

    UPROPERTY(SaveGame, BlueprintReadWrite)
    int32 Rank;  // 1 = best

    UPROPERTY(SaveGame, BlueprintReadWrite)
    EMedalType Medal;  // Gold/Silver/Bronze/None

    UPROPERTY(SaveGame, BlueprintReadWrite)
    FString ShipName;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    EFlightAssistMode AssistMode;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    FDateTime DateAchieved;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    bool bIsPersonalBest = false;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    FString ReplayDataPath;  // Optional: path to replay file
};
```

### EMedalType

```cpp
UENUM(BlueprintType)
enum class EMedalType : uint8
{
    None UMETA(DisplayName = "None"),
    Bronze UMETA(DisplayName = "Bronze"),
    Silver UMETA(DisplayName = "Silver"),
    Gold UMETA(DisplayName = "Gold")
};
```

### FCourseStatistics

Detailed statistics per course.

```cpp
USTRUCT(BlueprintType)
struct FCourseStatistics
{
    GENERATED_BODY()

    UPROPERTY(SaveGame, BlueprintReadWrite)
    int32 TotalAttempts = 0;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    int32 TotalCompletions = 0;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    float AverageTime = 0.0f;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    float FastestTime = 0.0f;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    float SlowestTime = 0.0f;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    int32 TotalCollisions = 0;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    int32 PerfectRuns = 0;  // 0 collisions

    UPROPERTY(SaveGame, BlueprintReadWrite)
    float TotalPlayTime = 0.0f;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    FDateTime FirstAttemptDate;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    FDateTime LastAttemptDate;
};
```

---

## ULocalLeaderboardManager

Game Instance Subsystem managing leaderboards.

### Class Declaration

```cpp
UCLASS()
class ALEXANDER_API ULocalLeaderboardManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Submit new time
    UFUNCTION(BlueprintCallable, Category = "Leaderboard")
    void SubmitTime(FString CourseName, float CompletionTime, FString ShipName,
                    EFlightAssistMode AssistMode, int32& OutRank, bool& bNewPersonalBest);

    // Query leaderboards
    UFUNCTION(BlueprintCallable, Category = "Leaderboard")
    TArray<FLeaderboardEntry> GetLeaderboard(FString CourseName);

    UFUNCTION(BlueprintCallable, Category = "Leaderboard")
    float GetPersonalBest(FString CourseName);

    UFUNCTION(BlueprintCallable, Category = "Leaderboard")
    int32 GetRankForTime(FString CourseName, float Time);

    // Statistics
    UFUNCTION(BlueprintCallable, Category = "Statistics")
    FCourseStatistics GetCourseStatistics(FString CourseName);

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    void RecordAttempt(FString CourseName, bool bCompleted, float Time, int32 Collisions);

    // Save/Load
    UFUNCTION(BlueprintCallable, Category = "Save")
    void SaveLeaderboards();

    UFUNCTION(BlueprintCallable, Category = "Save")
    bool LoadLeaderboards();

protected:
    // Leaderboards data
    UPROPERTY(SaveGame)
    TMap<FString, TArray<FLeaderboardEntry>> CourseLeaderboards;

    UPROPERTY(SaveGame)
    TMap<FString, FCourseStatistics> CourseStatistics;

    UPROPERTY(EditDefaultsOnly)
    int32 MaxEntriesPerCourse = 10;

    // Helper functions
    EMedalType CalculateMedal(FString CourseName, float CompletionTime);
    FString GetPlayerName();
};
```

### Key Function Implementations

#### SubmitTime()

See `local_leaderboard_spec` in JSON export for full implementation. Summary:
1. Create new FLeaderboardEntry
2. Calculate medal type
3. Check if personal best
4. Add to leaderboard array
5. Sort by time (ascending)
6. Keep only top 10
7. Update ranks
8. Save leaderboards
9. Return rank and PB status

#### GetLeaderboard()

```cpp
TArray<FLeaderboardEntry> ULocalLeaderboardManager::GetLeaderboard(FString CourseName)
{
    if (CourseLeaderboards.Contains(CourseName))
    {
        return CourseLeaderboards[CourseName];
    }
    return TArray<FLeaderboardEntry>();
}
```

#### GetPersonalBest()

```cpp
float ULocalLeaderboardManager::GetPersonalBest(FString CourseName)
{
    TArray<FLeaderboardEntry> Leaderboard = GetLeaderboard(CourseName);

    for (const FLeaderboardEntry& Entry : Leaderboard)
    {
        if (Entry.bIsPersonalBest)
        {
            return Entry.CompletionTime;
        }
    }

    return 0.0f;  // No PB recorded
}
```

#### CalculateMedal()

```cpp
EMedalType ULocalLeaderboardManager::CalculateMedal(FString CourseName, float CompletionTime)
{
    UFlightCourseData* CourseData = GetCourseData(CourseName);
    if (!CourseData || CourseData->ParTime <= 0)
        return EMedalType::None;

    float GoldTime = CourseData->ParTime * 0.9f;
    float SilverTime = CourseData->ParTime;
    float BronzeTime = CourseData->ParTime * 1.1f;

    if (CompletionTime <= GoldTime)
        return EMedalType::Gold;
    else if (CompletionTime <= SilverTime)
        return EMedalType::Silver;
    else if (CompletionTime <= BronzeTime)
        return EMedalType::Bronze;
    else
        return EMedalType::None;
}
```

#### RecordAttempt()

```cpp
void ULocalLeaderboardManager::RecordAttempt(FString CourseName, bool bCompleted, float Time, int32 Collisions)
{
    FCourseStatistics& Stats = CourseStatistics.FindOrAdd(CourseName);

    Stats.TotalAttempts++;
    Stats.LastAttemptDate = FDateTime::Now();

    if (Stats.TotalAttempts == 1)
    {
        Stats.FirstAttemptDate = FDateTime::Now();
    }

    if (bCompleted)
    {
        Stats.TotalCompletions++;

        // Update average time
        Stats.AverageTime = ((Stats.AverageTime * (Stats.TotalCompletions - 1)) + Time) / Stats.TotalCompletions;

        // Update fastest/slowest
        if (Stats.FastestTime == 0 || Time < Stats.FastestTime)
            Stats.FastestTime = Time;
        if (Time > Stats.SlowestTime)
            Stats.SlowestTime = Time;

        // Track perfect runs
        if (Collisions == 0)
            Stats.PerfectRuns++;
    }

    Stats.TotalCollisions += Collisions;
    Stats.TotalPlayTime += Time;

    SaveLeaderboards();
}
```

---

## UI Components

### WBP_LeaderboardDisplay

Main leaderboard viewer widget.

**Size**: 800x600

**Hierarchy**:
```
Canvas Panel
|
+-- Vertical Box (MainContainer)
    +-- Border (HeaderSection)
    |   +-- Text Block (CourseTitle) - "Beginner's Canyon Leaderboard"
    |   +-- Text Block (ParTimeDisplay) - "Par Time: 60.0s"
    |
    +-- Scroll Box (LeaderboardScroll)
    |   +-- Vertical Box (LeaderboardList) [Dynamic - populated in code]
    |       +-- WBP_LeaderboardEntry [Multiple instances]
    |
    +-- Border (PersonalBestSection)
    |   Background: Highlighted
    |   +-- Horizontal Box
    |       +-- Text Block (PBLabel) - "Your Best:"
    |       +-- Text Block (PBTime) - "58.3s"
    |       +-- Image (PBMedal) - Gold medal icon
    |
    +-- Horizontal Box (ButtonSection)
        +-- Button (CloseButton) - "Close"
```

**Blueprint Functions**:

```cpp
void UWBPLeaderboardDisplay::PopulateLeaderboard(FString CourseName)
{
    ULocalLeaderboardManager* LeaderboardManager =
        GetGameInstance()->GetSubsystem<ULocalLeaderboardManager>();

    // Get leaderboard data
    TArray<FLeaderboardEntry> Entries = LeaderboardManager->GetLeaderboard(CourseName);

    // Clear existing entries
    LeaderboardList->ClearChildren();

    // Create entry widget for each
    for (const FLeaderboardEntry& Entry : Entries)
    {
        UWBPLeaderboardEntry* EntryWidget = CreateWidget<UWBPLeaderboardEntry>(
            GetWorld(), LeaderboardEntryClass);

        EntryWidget->SetEntryData(Entry);
        LeaderboardList->AddChild(EntryWidget);
    }

    // Update personal best display
    float PB = LeaderboardManager->GetPersonalBest(CourseName);
    if (PB > 0)
    {
        PBTime->SetText(FText::FromString(FString::Printf(TEXT("%.2fs"), PB)));

        EMedalType Medal = LeaderboardManager->CalculateMedal(CourseName, PB);
        PBMedal->SetBrushFromTexture(GetMedalTexture(Medal));
    }
    else
    {
        PBTime->SetText(FText::FromString(TEXT("No time recorded")));
        PBMedal->SetVisibility(ESlateVisibility::Collapsed);
    }
}
```

---

### WBP_LeaderboardEntry

Single leaderboard entry row.

**Hierarchy**:
```
Border (EntryBackground)
    Brush Color: Varies (highlight for personal best)
    |
    +-- Horizontal Box
        +-- Text Block (RankText) - Width 60 - "#1"
        +-- Image (MedalIcon) - Width 40 - Gold medal
        +-- Text Block (TimeText) - Width 120 - "58.34s"
        +-- Text Block (PlayerNameText) - Width 200 - "Player"
        +-- Text Block (ShipNameText) - Width 150 - "Viper MK2"
        +-- Text Block (DateText) - Width 150 - "2025-11-06"
```

**Blueprint Functions**:

```cpp
void UWBPLeaderboardEntry::SetEntryData(FLeaderboardEntry EntryData)
{
    // Set rank
    RankText->SetText(FText::AsNumber(EntryData.Rank));

    // Set time with formatting
    FString TimeStr = FString::Printf(TEXT("%.2fs"), EntryData.CompletionTime);
    TimeText->SetText(FText::FromString(TimeStr));

    // Set player name
    PlayerNameText->SetText(FText::FromString(EntryData.PlayerName));

    // Set ship name
    ShipNameText->SetText(FText::FromString(EntryData.ShipName));

    // Set date
    FString DateStr = EntryData.DateAchieved.ToString(TEXT("%Y-%m-%d"));
    DateText->SetText(FText::FromString(DateStr));

    // Show medal icon
    if (EntryData.Medal != EMedalType::None)
    {
        MedalIcon->SetBrushFromTexture(GetMedalTexture(EntryData.Medal));
        MedalIcon->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        MedalIcon->SetVisibility(ESlateVisibility::Collapsed);
    }

    // Highlight personal best
    if (EntryData.bIsPersonalBest)
    {
        EntryBackground->SetBrushColor(FLinearColor(0.2, 0.4, 0.8, 1.0));  // Blue highlight
    }
    else
    {
        EntryBackground->SetBrushColor(FLinearColor(0.1, 0.1, 0.1, 1.0));  // Default dark
    }

    // Color code by rank
    if (EntryData.Rank == 1)
        RankText->SetColorAndOpacity(FLinearColor(1, 0.84, 0, 1));  // Gold
    else if (EntryData.Rank == 2)
        RankText->SetColorAndOpacity(FLinearColor(0.75, 0.75, 0.75, 1));  // Silver
    else if (EntryData.Rank == 3)
        RankText->SetColorAndOpacity(FLinearColor(0.8, 0.5, 0.2, 1));  // Bronze
    else
        RankText->SetColorAndOpacity(FLinearColor(1, 1, 1, 1));  // White
}
```

---

### WBP_CourseResults

Post-completion results screen.

**Displayed**: After completing a course

**Hierarchy**:
```
Canvas Panel
|
+-- Overlay (FullScreenOverlay)
    +-- Image (DarkBackground) - Semi-transparent black
    +-- Border (ResultsPanel) - Center screen, 900x700
        |
        +-- Vertical Box (MainContent)
            +-- Text Block (CompletionTitle) - "COURSE COMPLETE!"
            +-- Text Block (CourseName) - "Beginner's Canyon"
            |
            +-- Border (TimeSection) - Highlighted
            |   +-- Vertical Box
            |       +-- Text Block (TimeLabel) - "Time"
            |       +-- Text Block (CompletionTime) - "58.34s" (Large font)
            |       +-- Text Block (ParComparison) - "1.66s under par!"
            |       +-- Text Block (PBComparison) - "New Personal Best!" or "-0.23s"
            |       +-- Image (MedalDisplay) - Gold/Silver/Bronze medal
            |
            +-- Border (LeaderboardSection)
            |   +-- Vertical Box
            |       +-- Text Block (RankTitle) - "Your Rank"
            |       +-- Text Block (RankDisplay) - "Rank 3 of 10"
            |       +-- Text Block (Top3Title) - "Top 3 Times:"
            |       +-- Horizontal Box (Top3Display)
            |           +-- 3 columns showing top 3 times
            |
            +-- Border (RewardsSection)
            |   +-- Vertical Box
            |       +-- Text Block (RewardsTitle) - "Rewards"
            |       +-- Text Block (XPEarned) - "+300 XP"
            |       +-- Text Block (CreditsEarned) - "+100 Credits"
            |       +-- Text Block (TimeBonus) - "+50 XP (Time Bonus)"
            |       +-- Text Block (UnlockMessage) - "Medium Courses Unlocked!"
            |
            +-- Horizontal Box (ActionButtons)
                +-- Button (RetryButton) - "Retry Course"
                +-- Button (NextCourseButton) - "Next Course"
                +-- Button (MenuButton) - "Return to Menu"
```

**Animations**:

1. **Entrance** (Event Construct):
   - Fade in background (0.3s)
   - Scale results panel from 0.9 to 1.0 (0.5s)
   - Fade in panel (0.5s)

2. **Time Reveal** (Sequenced):
   - Count up from 0 to completion time (1.0s)
   - Play sound at end

3. **Medal Reveal** (After time):
   - Medal flies in from top with bounce (0.8s)
   - Play medal sound

4. **Rank Reveal** (After medal):
   - Rank text pops in with scale (0.5s)

5. **Rewards Reveal** (After rank):
   - Each reward line appears sequentially (0.3s each)

**Blueprint Implementation**:

```cpp
void UWBPCourseResults::ShowResults(FString CourseName, float Time, int32 Rank, bool bNewPB,
                                     int32 XP, int32 Credits, TArray<FString> Unlocks)
{
    // Play entrance animation
    PlayAnimation(EntranceAnimation);

    // Start time count-up after delay
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Time]()
    {
        StartTimeCountUp(Time);
    }, 0.8f, false);

    // Reveal medal after time count-up
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, CourseName, Time]()
    {
        RevealMedal(CourseName, Time);
    }, 2.0f, false);

    // Reveal rank after medal
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Rank]()
    {
        RevealRank(Rank);
    }, 3.0f, false);

    // Reveal rewards after rank
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, XP, Credits, Unlocks]()
    {
        RevealRewards(XP, Credits, Unlocks);
    }, 3.8f, false);
}

void UWBPCourseResults::StartTimeCountUp(float TargetTime)
{
    // Animate time counting up
    float CurrentTime = 0.0f;
    float CountDuration = 1.0f;
    float ElapsedTime = 0.0f;

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, TargetTime, &CurrentTime, &ElapsedTime, CountDuration]()
    {
        ElapsedTime += 0.016f;  // ~60 FPS
        float Alpha = FMath::Clamp(ElapsedTime / CountDuration, 0.0f, 1.0f);
        CurrentTime = FMath::Lerp(0.0f, TargetTime, Alpha);

        FString TimeStr = FString::Printf(TEXT("%.2fs"), CurrentTime);
        CompletionTimeText->SetText(FText::FromString(TimeStr));

        if (Alpha >= 1.0f)
        {
            // Count-up complete, play sound
            PlaySound(TimeRevealSound);
        }
    }, 0.016f, true);
}

void UWBPCourseResults::RevealMedal(FString CourseName, float Time)
{
    ULocalLeaderboardManager* LBManager = GetGameInstance()->GetSubsystem<ULocalLeaderboardManager>();
    EMedalType Medal = LBManager->CalculateMedal(CourseName, Time);

    if (Medal != EMedalType::None)
    {
        MedalDisplay->SetBrushFromTexture(GetMedalTexture(Medal));
        PlayAnimation(MedalFlyInAnimation);
        PlaySound(GetMedalSound(Medal));
    }
}

void UWBPCourseResults::RevealRank(int32 Rank)
{
    if (Rank > 0)
    {
        FString RankStr = FString::Printf(TEXT("Rank %d of 10"), Rank);
        RankDisplayText->SetText(FText::FromString(RankStr));
        PlayAnimation(RankPopInAnimation);
    }
    else
    {
        RankDisplayText->SetText(FText::FromString(TEXT("Not in Top 10")));
    }
}

void UWBPCourseResults::RevealRewards(int32 XP, int32 Credits, TArray<FString> Unlocks)
{
    // Show rewards sequentially
    float Delay = 0.0f;

    // XP
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, XP]()
    {
        XPEarnedText->SetText(FText::FromString(FString::Printf(TEXT("+%d XP"), XP)));
        PlayAnimation(XPRevealAnimation);
    }, Delay, false);
    Delay += 0.3f;

    // Credits
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Credits]()
    {
        CreditsEarnedText->SetText(FText::FromString(FString::Printf(TEXT("+%d Credits"), Credits)));
        PlayAnimation(CreditsRevealAnimation);
    }, Delay, false);
    Delay += 0.3f;

    // Unlocks
    for (const FString& Unlock : Unlocks)
    {
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Unlock]()
        {
            UnlockMessageText->SetText(FText::FromString(Unlock));
            PlayAnimation(UnlockRevealAnimation);
            PlaySound(UnlockSound);
        }, Delay, false);
        Delay += 0.5f;
    }
}
```

**Button Handlers**:

```cpp
void UWBPCourseResults::OnRetryButtonClicked()
{
    // Restart current course
    UGameplayStatics::OpenLevel(this, FName(*CurrentCourseLevelName));
}

void UWBPCourseResults::OnNextCourseButtonClicked()
{
    // Load next course in sequence (if available)
    // Or return to course selection
}

void UWBPCourseResults::OnMenuButtonClicked()
{
    // Return to course selection menu
    UGameplayStatics::OpenLevel(this, TEXT("MainMenu"));
}
```

---

## Integration with Course Manager

### BP_CourseManager (in course levels)

Add leaderboard submission on course completion:

```cpp
void ABPCourseManager::OnCourseCompleted(float CompletionTime)
{
    ULocalLeaderboardManager* LBManager = GetGameInstance()->GetSubsystem<ULocalLeaderboardManager>();

    // Get course info
    FString CourseName = GetCurrentCourseName();
    FString ShipName = "Default Ship";  // Get from player
    EFlightAssistMode AssistMode = GetPlayerAssistMode();

    // Submit time to leaderboard
    int32 Rank = 0;
    bool bNewPB = false;
    LBManager->SubmitTime(CourseName, CompletionTime, ShipName, AssistMode, Rank, bNewPB);

    // Record attempt statistics
    int32 Collisions = GetCollisionCount();
    LBManager->RecordAttempt(CourseName, true, CompletionTime, Collisions);

    // Calculate rewards
    int32 XPEarned = CalculateXPReward(CompletionTime);
    int32 CreditsEarned = CalculateCreditsReward(CompletionTime);

    // Award progression rewards
    UUnlockSystem* UnlockSystem = GetGameInstance()->GetSubsystem<UUnlockSystem>();
    UnlockSystem->OnCourseCompleted(CourseName, CompletionTime, XPEarned, CreditsEarned);

    // Show results screen
    ShowResultsScreen(CourseName, CompletionTime, Rank, bNewPB, XPEarned, CreditsEarned);
}
```

---

## Save System Integration

### Save Game Class Extension

Add leaderboards to save game:

```cpp
UCLASS()
class USaveGamePlayerProgress : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(SaveGame)
    FPlayerGlobalProgress ProgressData;

    UPROPERTY(SaveGame)
    TMap<FString, TArray<FLeaderboardEntry>> Leaderboards;

    UPROPERTY(SaveGame)
    TMap<FString, FCourseStatistics> Statistics;

    UPROPERTY(SaveGame)
    FString SaveDate;

    UPROPERTY(SaveGame)
    int32 SaveVersion = 1;
};
```

Save/Load functions updated to include leaderboards.

---

## Future Enhancements (Friend/Global Leaderboards)

### Friend Leaderboards

- **Requirement**: Online service integration (Steam, Epic, Xbox Live)
- **Features**:
  - Compare times with friends list
  - Download friend ghost replays
  - Send challenge invitations
  - Friend-only rankings

### Global Leaderboards

- **Requirement**: Dedicated server or online service
- **Features**:
  - Worldwide rankings
  - Regional leaderboards
  - Leaderboard seasons (weekly/monthly)
  - Anti-cheat verification
  - Replay validation

---

## Testing Checklist

### Core Functionality
- [ ] SubmitTime() correctly adds entries to leaderboard
- [ ] Leaderboard sorts by time (ascending)
- [ ] Keeps only top 10 entries per course
- [ ] Ranks update correctly after new entries
- [ ] Personal best detection works
- [ ] Medal calculation correct (Gold/Silver/Bronze)

### UI
- [ ] WBP_LeaderboardDisplay populates correctly
- [ ] Entry rows display all data (rank, time, name, etc.)
- [ ] Personal best highlighted in leaderboard
- [ ] Medal icons display correctly
- [ ] Results screen animations play smoothly
- [ ] Time count-up animation works
- [ ] Medal reveal animation works
- [ ] Rewards reveal sequentially

### Statistics
- [ ] Attempt count increments
- [ ] Completion count tracks successfully finished courses
- [ ] Average time calculates correctly
- [ ] Fastest/slowest times update
- [ ] Perfect runs count (0 collisions)
- [ ] Total play time accumulates

### Save/Load
- [ ] Leaderboards save to file
- [ ] Leaderboards load on game start
- [ ] Statistics persist across sessions
- [ ] Backup save created

### Integration
- [ ] Course completion triggers leaderboard submission
- [ ] Unlock system integrates with leaderboard
- [ ] Results screen shows correct rank and rewards
- [ ] Button actions work (Retry, Next, Menu)

---

## Conclusion

Complete local leaderboard system with:
- **Top 10 tracking** per course
- **Personal best** highlighting
- **Time medals** (Gold/Silver/Bronze)
- **Rankings** with rank display
- **Statistics tracking** (attempts, completions, average time, etc.)
- **Results screen** with animations
- **Save system** integration

Estimated implementation time: **8-10 hours**

**Status**: Design Complete - Ready for Implementation

Future-ready for friend and global leaderboards with online service integration.
