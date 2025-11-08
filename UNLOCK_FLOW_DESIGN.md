# Unlock Flow Design

## Overview

This document provides comprehensive documentation for the ship part unlock and purchase flow, including state machines, error handling, edge cases, and user feedback systems.

---

## Flow Diagram

### High-Level Unlock Flow

```
┌─────────────────────────────────────────────────────────────────────────┐
│                          UNLOCK FLOW DIAGRAM                            │
└─────────────────────────────────────────────────────────────────────────┘

    [User in Customization Menu]
                │
                ▼
    ┌───────────────────────┐
    │ User Clicks Part Card │
    │  (Locked Part Icon)   │
    └───────────────────────┘
                │
                ▼
    ┌────────────────────────────┐
    │ Check: Is Part Unlocked?   │
    └────────────────────────────┘
                │
        ┌───────┴───────┐
        │               │
       YES              NO
        │               │
        │               ▼
        │   ┌─────────────────────────┐
        │   │ Check: Level Req Met?   │
        │   └─────────────────────────┘
        │               │
        │       ┌───────┴───────┐
        │       │               │
        │      YES              NO
        │       │               │
        │       │               ▼
        │       │   ┌────────────────────────┐
        │       │   │ Show Level Requirement │
        │       │   │   Error in Dialog      │
        │       │   └────────────────────────┘
        │       │               │
        │       │               ▼
        │       │   ┌────────────────────────┐
        │       │   │  Disable Unlock Button │
        │       │   │  Show "Level X Required"│
        │       │   └────────────────────────┘
        │       │               │
        │       │               ▼
        │       │          [User Closes Dialog]
        │       │               │
        │       │               ▼
        │       │          [Flow Ends]
        │       │
        │       ▼
        │   ┌──────────────────────────┐
        │   │ Check: Has Enough Credits?│
        │   └──────────────────────────┘
        │               │
        │       ┌───────┴───────┐
        │       │               │
        │      YES              NO
        │       │               │
        │       │               ▼
        │       │   ┌────────────────────────────┐
        │       │   │ Show Insufficient Funds    │
        │       │   │   Warning in Dialog        │
        │       │   └────────────────────────────┘
        │       │               │
        │       │               ▼
        │       │   ┌────────────────────────────┐
        │       │   │  Disable Unlock Button     │
        │       │   │  Highlight Balance vs Cost │
        │       │   └────────────────────────────┘
        │       │               │
        │       │               ▼
        │       │          [User Closes Dialog]
        │       │               │
        │       │               ▼
        │       │          [Flow Ends]
        │       │
        │       ▼
        │   ┌─────────────────────────────┐
        │   │ Show WBP_UnlockDialog       │
        │   │  - Part preview             │
        │   │  - Cost breakdown           │
        │   │  - Balance display          │
        │   │  - Confirm/Cancel buttons   │
        │   └─────────────────────────────┘
        │               │
        │               ▼
        │   ┌──────────────────────┐
        │   │ User Makes Choice    │
        │   └──────────────────────┘
        │               │
        │       ┌───────┴───────┐
        │       │               │
        │    CANCEL          CONFIRM
        │       │               │
        │       │               ▼
        │       │   ┌──────────────────────────┐
        │       │   │ Deduct Credits           │
        │       │   │ PlayerCredits -= Cost    │
        │       │   └──────────────────────────┘
        │       │               │
        │       │               ▼
        │       │   ┌──────────────────────────┐
        │       │   │ Mark Part as Unlocked    │
        │       │   │ Component->UnlockPart()  │
        │       │   └──────────────────────────┘
        │       │               │
        │       │               ▼
        │       │   ┌──────────────────────────┐
        │       │   │ Save Progress            │
        │       │   │ SaveGame->UnlockedParts  │
        │       │   └──────────────────────────┘
        │       │               │
        │       │               ▼
        │       │   ┌──────────────────────────┐
        │       │   │ Play Success Animation   │
        │       │   │  - Button flash          │
        │       │   │  - Icon celebration      │
        │       │   │  - Particle burst        │
        │       │   └──────────────────────────┘
        │       │               │
        │       │               ▼
        │       │   ┌──────────────────────────┐
        │       │   │ Play Success Sound       │
        │       │   │ "unlock_success.wav"     │
        │       │   └──────────────────────────┘
        │       │               │
        │       │               ▼
        │       │   ┌──────────────────────────┐
        │       │   │ Trigger Haptic Feedback  │
        │       │   │ (VR: Controller pulse)   │
        │       │   └──────────────────────────┘
        │       │               │
        │       │               ▼
        │       │   ┌──────────────────────────┐
        │       │   │ Show Notification Toast  │
        │       │   │ "Part Unlocked!"         │
        │       │   └──────────────────────────┘
        │       │               │
        │       │               ▼
        │       │   ┌──────────────────────────┐
        │       │   │ Update UI State          │
        │       │   │  - Remove lock icon      │
        │       │   │  - Enable equip button   │
        │       │   │  - Update credits display│
        │       │   └──────────────────────────┘
        │       │               │
        │       │               ▼
        │       │   ┌──────────────────────────┐
        │       │   │ Close Dialog             │
        │       │   │ (After 1.5s delay)       │
        │       │   └──────────────────────────┘
        │       │               │
        │       ▼               ▼
        │   ┌──────────────────────────┐
        │   │ Close Dialog Immediately │
        │   └──────────────────────────┘
        │               │
        ▼               ▼
    ┌───────────────────────────┐
    │ Enable Part Equip Button  │
    │  (If was already unlocked)│
    └───────────────────────────┘
                │
                ▼
    ┌───────────────────────────┐
    │ User Can Now Equip Part   │
    └───────────────────────────┘
                │
                ▼
           [Flow Ends]
```

---

## State Machine

### Unlock Process States

```cpp
UENUM(BlueprintType)
enum class EUnlockState : uint8
{
    Idle,                    // No unlock in progress
    CheckingRequirements,    // Validating level and credits
    ShowingDialog,           // Dialog is displayed
    AwaitingUserInput,       // Waiting for Confirm/Cancel
    ProcessingUnlock,        // Deducting credits and unlocking
    PlayingSuccessAnimation, // Success feedback animation
    Completed,               // Unlock complete
    Failed,                  // Unlock failed (error occurred)
    Cancelled                // User cancelled unlock
};
```

### State Transitions

```
Idle
  │
  │ User clicks locked part
  ▼
CheckingRequirements
  │
  │ Level OK && Credits OK
  ▼
ShowingDialog
  │
  │ Dialog displayed
  ▼
AwaitingUserInput
  │
  ├─ User clicks Cancel ──► Cancelled ──► Idle
  │
  └─ User clicks Confirm
      ▼
  ProcessingUnlock
      │
      ├─ Error occurred ──► Failed ──► Idle
      │
      └─ Success
          ▼
      PlayingSuccessAnimation
          ▼
      Completed ──► Idle
```

### State Machine Implementation

```cpp
class FUnlockStateMachine
{
private:
    EUnlockState CurrentState;
    FShipPartDefinition PartToUnlock;
    int32 PlayerCredits;
    int32 PlayerLevel;

public:
    void Initialize(const FShipPartDefinition& Part, int32 Credits, int32 Level)
    {
        PartToUnlock = Part;
        PlayerCredits = Credits;
        PlayerLevel = Level;
        CurrentState = EUnlockState::Idle;
    }

    void StartUnlockProcess()
    {
        if (CurrentState != EUnlockState::Idle)
            return; // Already processing

        CurrentState = EUnlockState::CheckingRequirements;
        CheckRequirements();
    }

private:
    void CheckRequirements()
    {
        // Check level requirement
        if (PlayerLevel < PartToUnlock.LevelRequirement)
        {
            OnUnlockFailed(TEXT("Level requirement not met"));
            return;
        }

        // Check credit balance
        if (PlayerCredits < PartToUnlock.UnlockCost)
        {
            // Show dialog with warning, but don't fail yet
            // User can still see the requirements
        }

        // Requirements checked, show dialog
        CurrentState = EUnlockState::ShowingDialog;
        ShowUnlockDialog();
    }

    void ShowUnlockDialog()
    {
        // Create and display WBP_UnlockDialog
        UWBPUnlockDialog* Dialog = CreateWidget<UWBPUnlockDialog>(...);
        Dialog->ShowDialog(PartToUnlock, PlayerCredits, PlayerLevel);

        // Bind to dialog events
        Dialog->OnUnlockConfirmed.AddDynamic(this, &FUnlockStateMachine::OnUserConfirmed);
        Dialog->OnUnlockCancelled.AddDynamic(this, &FUnlockStateMachine::OnUserCancelled);

        CurrentState = EUnlockState::AwaitingUserInput;
    }

    void OnUserConfirmed()
    {
        CurrentState = EUnlockState::ProcessingUnlock;
        ProcessUnlock();
    }

    void OnUserCancelled()
    {
        CurrentState = EUnlockState::Cancelled;
        ResetToIdle();
    }

    void ProcessUnlock()
    {
        // Double-check credits (could have changed)
        if (PlayerCredits < PartToUnlock.UnlockCost)
        {
            OnUnlockFailed(TEXT("Insufficient credits"));
            return;
        }

        // Deduct credits
        PlayerCredits -= PartToUnlock.UnlockCost;
        SaveCredits(PlayerCredits);

        // Unlock the part
        bool bSuccess = UnlockPart(PartToUnlock.PartID);
        if (!bSuccess)
        {
            // Rollback credits
            PlayerCredits += PartToUnlock.UnlockCost;
            SaveCredits(PlayerCredits);
            OnUnlockFailed(TEXT("Failed to unlock part"));
            return;
        }

        // Save progress
        SaveUnlockedParts();

        // Play success animation
        CurrentState = EUnlockState::PlayingSuccessAnimation;
        PlaySuccessAnimation();
    }

    void PlaySuccessAnimation()
    {
        // Trigger all success feedback
        // - Dialog animation
        // - Sound effect
        // - Haptic feedback
        // - Notification toast

        // After animation completes (1.5s), transition to Completed
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            OnUnlockCompleted();
        }, 1.5f, false);
    }

    void OnUnlockCompleted()
    {
        CurrentState = EUnlockState::Completed;

        // Update UI
        UpdatePartCardUI(PartToUnlock.PartID);
        UpdateCreditsDisplay(PlayerCredits);
        ShowNotificationToast(PartToUnlock);

        // Close dialog
        CloseUnlockDialog();

        // Reset to idle
        ResetToIdle();
    }

    void OnUnlockFailed(const FString& Reason)
    {
        CurrentState = EUnlockState::Failed;

        // Log error
        UE_LOG(LogCustomization, Error, TEXT("Unlock failed: %s"), *Reason);

        // Show error message to user
        ShowErrorMessage(Reason);

        // Reset to idle
        ResetToIdle();
    }

    void ResetToIdle()
    {
        CurrentState = EUnlockState::Idle;
        PartToUnlock = FShipPartDefinition();
    }
};
```

---

## Error Handling

### Error Types

#### 1. Insufficient Credits

**Condition:**
```cpp
if (PlayerCredits < PartData.UnlockCost)
{
    // Handle insufficient credits
}
```

**UI Feedback:**
- Show warning section in dialog
- Display "INSUFFICIENT FUNDS" message in red
- Highlight balance vs cost (balance in red)
- Disable "Unlock" button
- Show "After Purchase" as negative value
- Optional: Shake animation on warning section

**User Actions:**
- Cancel and earn more credits
- View other parts they can afford

**Recovery:**
```
User must:
1. Close dialog
2. Play matches to earn credits
3. Return when they have enough
```

#### 2. Level Requirement Not Met

**Condition:**
```cpp
if (PlayerLevel < PartData.LevelRequirement)
{
    // Handle level requirement
}
```

**UI Feedback:**
- Show level requirement section
- Display "Requires Level X" with lock icon
- Disable "Unlock" button
- Show current level vs required level
- Highlight gap in yellow

**User Actions:**
- Cancel and earn XP
- View parts available at current level

**Recovery:**
```
User must:
1. Close dialog
2. Play matches to gain XP and level up
3. Return when level requirement is met
```

#### 3. Part Already Unlocked

**Condition:**
```cpp
if (CustomizationComponent->IsPartUnlocked(PartID))
{
    // Handle already unlocked
}
```

**UI Feedback:**
- Don't show unlock dialog
- Show equip button instead of unlock button
- Part card has no lock icon
- Part is selectable

**User Actions:**
- Equip the part directly
- No unlock needed

**Edge Case Handling:**
```cpp
// Race condition: Part unlocked between click and dialog show
void ShowUnlockDialog(FString PartID)
{
    // Re-check unlock status
    if (IsPartUnlocked(PartID))
    {
        // Redirect to equip flow
        EquipPart(PartID);
        return;
    }

    // Proceed with unlock dialog
    // ...
}
```

#### 4. Invalid Part ID

**Condition:**
```cpp
FShipPartDefinition* PartData = GetPartData(PartID);
if (PartData == nullptr)
{
    // Handle invalid part
}
```

**UI Feedback:**
- Log error to console
- Show generic error message
- Close dialog if already open
- Prevent further interaction with part

**User Actions:**
- Report bug (this should never happen in production)

**Developer Actions:**
- Verify data asset references
- Check for typos in part IDs
- Validate part registry on startup

#### 5. Save Game Failure

**Condition:**
```cpp
bool bSaveSuccess = SaveGameToSlot(SaveGame, SlotName);
if (!bSaveSuccess)
{
    // Handle save failure
}
```

**UI Feedback:**
- Show error message: "Failed to save progress"
- Offer retry option
- Don't mark part as unlocked in UI yet

**User Actions:**
- Retry save
- Check disk space
- Report issue if persists

**Recovery Strategy:**
```cpp
void ProcessUnlock()
{
    // Deduct credits
    int32 OldCredits = PlayerCredits;
    PlayerCredits -= Cost;

    // Unlock part
    bool bUnlocked = UnlockPart(PartID);

    // Try to save
    bool bSaved = SaveProgress();

    if (!bSaved)
    {
        // Rollback changes
        PlayerCredits = OldCredits;
        LockPart(PartID); // Re-lock the part

        // Show error
        ShowErrorDialog(TEXT("Failed to save. Please try again."));
        return;
    }

    // Success!
    ShowSuccessAnimation();
}
```

#### 6. Network Errors (Multiplayer/Cloud Saves)

**Condition:**
```cpp
if (!IsConnectedToServer())
{
    // Handle offline mode
}
```

**UI Feedback:**
- Show warning: "Offline mode - progress saved locally only"
- Allow unlock to proceed
- Queue for server sync when online

**Recovery:**
```cpp
void OnNetworkReconnected()
{
    // Sync local unlocks to server
    SyncUnlocksToServer();
}
```

---

## Edge Cases

### Edge Case 1: Rapid Clicking

**Scenario:** User clicks unlock button multiple times rapidly

**Prevention:**
```cpp
bool bUnlockInProgress = false;

void OnUnlockButtonClicked()
{
    if (bUnlockInProgress)
        return; // Ignore subsequent clicks

    bUnlockInProgress = true;

    // Process unlock...

    // Reset flag when complete
    OnUnlockComplete.AddLambda([this]()
    {
        bUnlockInProgress = false;
    });
}
```

**Alternative:** Disable button immediately on click

```cpp
void OnUnlockButtonClicked()
{
    UnlockButton->SetIsEnabled(false);

    // Process unlock...

    // Re-enable on complete/cancel
}
```

### Edge Case 2: Credits Change During Dialog Display

**Scenario:** User earns/spends credits while unlock dialog is open (multiplayer, background events)

**Prevention:**
```cpp
void WBP_UnlockDialog::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Poll for credit changes
    int32 CurrentCredits = GetPlayerCredits();
    if (CurrentCredits != CachedCredits)
    {
        // Credits changed, update dialog
        UpdateAffordability(CurrentCredits);
        CachedCredits = CurrentCredits;
    }
}
```

**UI Update:**
```cpp
void UpdateAffordability(int32 NewCredits)
{
    bCanAfford = (NewCredits >= PartCost);

    // Update balance text
    BalanceText->SetText(FText::AsNumber(NewCredits));

    // Update button state
    ConfirmButton->SetIsEnabled(bCanAfford);

    // Show/hide warning
    WarningSection->SetVisibility(bCanAfford ?
        ESlateVisibility::Collapsed :
        ESlateVisibility::Visible);

    // Update "After Purchase" display
    int32 AfterPurchaseAmount = NewCredits - PartCost;
    AfterAmount->SetText(FText::AsNumber(AfterPurchaseAmount));
    AfterAmount->SetColorAndOpacity(bCanAfford ?
        FLinearColor(0.26f, 0.63f, 0.42f) : // Green
        FLinearColor(0.90f, 0.22f, 0.21f)); // Red
}
```

### Edge Case 3: Level Up During Dialog Display

**Scenario:** User levels up while dialog is open (multiplayer XP gain)

**Handling:**
```cpp
void WBP_UnlockDialog::OnPlayerLevelChanged(int32 NewLevel)
{
    // Check if level requirement is now met
    if (NewLevel >= PartLevelRequirement && !bMeetsLevelRequirement)
    {
        // Level requirement just met!
        bMeetsLevelRequirement = true;

        // Hide level requirement warning
        LevelRequirement->SetVisibility(ESlateVisibility::Collapsed);

        // Show celebration message
        ShowMessage(TEXT("Level requirement now met!"));

        // Update button state
        UpdateConfirmButtonState();
    }
}
```

### Edge Case 4: Dialog Open When Map Changes

**Scenario:** User is in unlock dialog when map changes/game ends

**Prevention:**
```cpp
void WBP_UnlockDialog::NativeDestruct()
{
    // Clean up if destroyed mid-process
    if (UnlockState == EUnlockState::ProcessingUnlock)
    {
        // Critical: ensure unlock completes or rolls back
        CompleteUnlockProcess();
    }

    Super::NativeDestruct();
}

void ACustomizationGameMode::PreLevelChange()
{
    // Close all UI dialogs before level change
    CloseAllDialogs();
}
```

### Edge Case 5: Multiple Parts Unlocked Simultaneously

**Scenario:** User completes challenge that unlocks multiple parts at once

**Handling:**
```cpp
void UnlockMultipleParts(const TArray<FString>& PartIDs)
{
    for (const FString& PartID : PartIDs)
    {
        UnlockPart(PartID);
    }

    // Queue notifications with staggered timing
    float Delay = 0.0f;
    for (const FString& PartID : PartIDs)
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, PartID]()
        {
            ShowUnlockNotification(PartID);
        }, Delay, false);

        Delay += 0.5f; // 500ms between each notification
    }
}
```

### Edge Case 6: Part Cost Changes (Live Updates)

**Scenario:** Server pushes economy update, changing part costs

**Handling:**
```cpp
void OnPartCostsUpdated(const TMap<FString, int32>& NewCosts)
{
    // Update data assets
    UpdatePartCosts(NewCosts);

    // If dialog is open for affected part, update display
    if (ActiveDialog && NewCosts.Contains(ActiveDialog->GetPartID()))
    {
        int32 NewCost = NewCosts[ActiveDialog->GetPartID()];
        ActiveDialog->UpdateCost(NewCost);

        // Show notification
        ShowMessage(FString::Printf(
            TEXT("Price updated: %d CR"), NewCost
        ));
    }

    // Refresh part grid UI
    RefreshPartGrid();
}
```

---

## Success Feedback

### Visual Feedback

**1. Dialog Button Flash (300ms)**
```cpp
void WBP_UnlockDialog::OnConfirmClicked()
{
    // Flash button green
    UMaterialInstanceDynamic* ButtonMaterial =
        ConfirmButton->GetDynamicMaterial();

    // Animate color change
    FLinearColor StartColor(0.117f, 0.533f, 0.898f, 1.0f); // Blue
    FLinearColor EndColor(0.263f, 0.627f, 0.278f, 1.0f);   // Green

    PlayColorTransition(ButtonMaterial, StartColor, EndColor, 0.3f);
}
```

**2. Part Icon Celebration (800ms)**
```cpp
// Scale and rotate animation
FWidgetTransform StartTransform;
StartTransform.Scale = FVector2D(1.0f, 1.0f);
StartTransform.Angle = 0.0f;

FWidgetTransform EndTransform;
EndTransform.Scale = FVector2D(1.3f, 1.3f);
EndTransform.Angle = 360.0f;

PlayWidgetTransform(PartIcon, StartTransform, EndTransform, 0.8f,
    EEasingFunc::EaseOutElastic);
```

**3. Particle Burst (1.5s)**
```cpp
// Spawn particle system at part icon location
UNiagaraComponent* ParticleComponent =
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        UnlockParticleSystem,
        PartIcon->GetCachedGeometry().GetAbsolutePosition(),
        FRotator::ZeroRotator
    );

// Set particle color to rarity color
ParticleComponent->SetColorParameter(TEXT("ParticleColor"), RarityColor);
```

**4. Glow Effect (2s)**
```cpp
// Animate glow intensity
UMaterialInstanceDynamic* IconMaterial = PartIcon->GetDynamicMaterial();

PlayFloatAnimation(IconMaterial, TEXT("GlowIntensity"),
    0.0f, 2.0f, 0.5f);  // 0 to 2 over 0.5s

// Then fade out
FTimerHandle Handle;
GetWorld()->GetTimerManager().SetTimer(Handle, [IconMaterial]()
{
    PlayFloatAnimation(IconMaterial, TEXT("GlowIntensity"),
        2.0f, 0.0f, 1.5f);  // 2 to 0 over 1.5s
}, 0.5f, false);
```

### Audio Feedback

**1. Success Sound Effect**
```cpp
void PlayUnlockSuccessSound()
{
    // Play 2D sound for UI
    UGameplayStatics::PlaySound2D(
        GetWorld(),
        UnlockSuccessSound,
        1.0f,  // Volume
        1.0f   // Pitch
    );
}
```

**Sound Specifications:**
```
File: unlock_success.wav
Duration: 1.2s
Format: 16-bit PCM, 44.1kHz, Stereo
Description: Ascending chime with sparkle

Layers:
  - 0.0s: Base chime (C major chord)
  - 0.3s: High sparkle (ascending arpeggio)
  - 0.8s: Reverb tail (fade out)
```

**2. Credit Deduction Sound**
```cpp
void PlayCreditDeductionSound()
{
    UGameplayStatics::PlaySound2D(
        GetWorld(),
        CreditDeductionSound,
        0.7f,  // Quieter than success
        1.0f
    );
}
```

**Sound Specifications:**
```
File: credit_deduction.wav
Duration: 0.4s
Format: 16-bit PCM, 44.1kHz, Stereo
Description: Cash register "cha-ching" sound
```

### Haptic Feedback (VR)

**1. Success Pulse**
```cpp
void PlaySuccessHaptic()
{
    // Get VR player controller
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
        return;

    // Play haptic effect on both controllers
    PC->PlayHapticEffect(
        UnlockSuccessHaptic,
        EControllerHand::Left,
        1.0f,  // Intensity
        false  // Don't loop
    );

    PC->PlayHapticEffect(
        UnlockSuccessHaptic,
        EControllerHand::Right,
        1.0f,
        false
    );
}
```

**Haptic Pattern:**
```
Duration: 500ms
Pattern:
  - 0ms: Strong pulse (1.0 intensity, 50ms)
  - 100ms: Medium pulse (0.6 intensity, 50ms)
  - 200ms: Light pulse (0.3 intensity, 50ms)
  - 300ms: Rest
  - 400ms: Final pulse (0.5 intensity, 100ms)
```

**2. Error Haptic**
```cpp
void PlayErrorHaptic()
{
    PC->PlayHapticEffect(
        ErrorHaptic,
        EControllerHand::Right,  // Only right controller
        0.8f,
        false
    );
}
```

**Haptic Pattern:**
```
Duration: 200ms
Pattern:
  - 0ms: Double tap (0.8 intensity)
    - Pulse 1: 0-30ms
    - Gap: 30-60ms
    - Pulse 2: 60-90ms
```

### Notification Toast

**1. Show Toast**
```cpp
void ShowUnlockNotification(const FShipPartDefinition& PartData)
{
    // Create toast widget
    UWBPNotificationToast* Toast =
        CreateWidget<UWBPNotificationToast>(GetOwningPlayer(), ToastClass);

    // Configure toast
    Toast->ShowUnlockNotification(
        PartData.PartName,
        PartData.Rarity,
        PartData.Icon
    );

    // Add to viewport
    Toast->AddToViewport(100);  // High Z-order

    // Toast will auto-dismiss after 4 seconds
}
```

**2. Toast Content**
```
┌─────────────────────────────────────────┐
│  [Icon]  PART UNLOCKED!                 │
│          Plasma Cannon Mk II            │
└─────────────────────────────────────────┘
```

---

## UI State Updates

### Immediate Updates (On Unlock)

**1. Part Card State**
```cpp
void UpdatePartCardAfterUnlock(FString PartID)
{
    UWBPPartCard* PartCard = FindPartCard(PartID);
    if (!PartCard)
        return;

    // Remove lock overlay
    PartCard->LockOverlay->SetVisibility(ESlateVisibility::Collapsed);

    // Show equip button
    PartCard->EquipButton->SetVisibility(ESlateVisibility::Visible);
    PartCard->EquipButton->SetIsEnabled(true);

    // Update button text
    PartCard->ActionButton->SetText(FText::FromString("EQUIP"));

    // Play reveal animation
    PartCard->PlayUnlockRevealAnimation();
}
```

**2. Credits Display**
```cpp
void UpdateCreditsDisplay(int32 NewCredits)
{
    // Update progression display
    ProgressionDisplay->UpdateCredits(NewCredits, true);  // Animate

    // Flash red (spent credits)
    FLinearColor FlashColor(0.90f, 0.22f, 0.21f, 1.0f);  // Red
    ProgressionDisplay->FlashCreditsColor(FlashColor, 0.3f);
}
```

**3. Unlock Progress**
```cpp
void UpdateUnlockProgress()
{
    int32 TotalParts = GetTotalParts();
    int32 UnlockedParts = GetUnlockedPartCount();

    // Update progress text
    UnlockProgressText->SetText(FText::FromString(
        FString::Printf(TEXT("Unlocked: %d / %d"), UnlockedParts, TotalParts)
    ));

    // Update progress bar
    float Progress = (float)UnlockedParts / (float)TotalParts;
    UnlockProgressBar->SetPercent(Progress);
}
```

### Deferred Updates (After Animation)

**1. Save Game**
```cpp
void SaveUnlockProgress()
{
    USaveGame* SaveGame = LoadSaveGame();

    // Add to unlocked parts list
    SaveGame->UnlockedParts.Add(PartID);

    // Update credits
    SaveGame->PlayerCredits = NewCredits;

    // Save to disk
    UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, 0);
}
```

**2. Analytics Event**
```cpp
void LogUnlockAnalytics(const FShipPartDefinition& PartData)
{
    TMap<FString, FString> EventParams;
    EventParams.Add(TEXT("PartID"), PartData.PartID);
    EventParams.Add(TEXT("PartName"), PartData.PartName);
    EventParams.Add(TEXT("Rarity"), GetRarityString(PartData.Rarity));
    EventParams.Add(TEXT("Cost"), FString::FromInt(PartData.UnlockCost));
    EventParams.Add(TEXT("PlayerLevel"), FString::FromInt(PlayerLevel));

    LogAnalyticsEvent(TEXT("part_unlocked"), EventParams);
}
```

**3. Achievement Check**
```cpp
void CheckUnlockAchievements()
{
    int32 UnlockedCount = GetUnlockedPartCount();

    // Check milestones
    if (UnlockedCount == 10)
        UnlockAchievement("ACH_UNLOCK_10_PARTS");
    else if (UnlockedCount == 25)
        UnlockAchievement("ACH_UNLOCK_25_PARTS");
    else if (UnlockedCount == 50)
        UnlockAchievement("ACH_UNLOCK_50_PARTS");

    // Check rarity achievements
    if (PartData.Rarity == EItemRarity::Legendary)
        UnlockAchievement("ACH_UNLOCK_LEGENDARY");
}
```

---

## Testing Scenarios

### Functional Tests

**Test 1: Successful Unlock**
```
Given:
  - Player has 2000 credits
  - Player is level 10
  - Part costs 1500 credits, requires level 5
  - Part is locked

When:
  - Player clicks part card
  - Player clicks "Unlock" button

Then:
  - Credits reduced to 500
  - Part is unlocked
  - Success animation plays
  - Notification toast appears
  - Part card updates to show "Equip" button
  - Save game updated
```

**Test 2: Insufficient Credits**
```
Given:
  - Player has 1000 credits
  - Part costs 1500 credits
  - Part is locked

When:
  - Player clicks part card

Then:
  - Dialog shows warning: "INSUFFICIENT FUNDS"
  - Balance text is red
  - "Unlock" button is disabled
  - "After Purchase" shows negative value
  - Cancel button works normally
```

**Test 3: Level Too Low**
```
Given:
  - Player is level 5
  - Part requires level 10
  - Part is locked

When:
  - Player clicks part card

Then:
  - Dialog shows: "Requires Level 10"
  - Lock icon displayed
  - "Unlock" button is disabled
  - Current level highlighted
  - Cancel button works normally
```

**Test 4: Cancel Unlock**
```
Given:
  - Unlock dialog is open
  - Player can afford part

When:
  - Player clicks "Cancel" button

Then:
  - No credits deducted
  - Part remains locked
  - Dialog closes
  - No notification shown
  - UI returns to previous state
```

**Test 5: Already Unlocked**
```
Given:
  - Part is already unlocked

When:
  - Player clicks part card

Then:
  - No unlock dialog appears
  - Equip button is visible and enabled
  - Part can be equipped immediately
```

### Edge Case Tests

**Test 6: Rapid Clicking**
```
Given:
  - Unlock dialog is open

When:
  - Player clicks "Unlock" button 5 times rapidly

Then:
  - Only one unlock processes
  - Credits deducted only once
  - Button disabled after first click
  - No duplicate notifications
```

**Test 7: Credits Change During Dialog**
```
Given:
  - Dialog is open, player has 1000 credits
  - Part costs 1500 credits
  - Player cannot afford

When:
  - Background event grants 1000 credits
  - Player now has 2000 credits

Then:
  - Dialog updates in real-time
  - Warning disappears
  - "Unlock" button enables
  - Balance text updates
  - Player can now unlock
```

**Test 8: Dialog During Level Change**
```
Given:
  - Unlock dialog is open
  - Unlock in progress

When:
  - Map changes (level load)

Then:
  - Unlock completes before level change
  - OR unlock is rolled back safely
  - No credits lost
  - Progress saved correctly
```

### Performance Tests

**Test 9: Multiple Toast Notifications**
```
Given:
  - 5 parts unlock simultaneously

When:
  - All notifications trigger

Then:
  - Toasts stack vertically
  - No overlap
  - All visible
  - Performance remains at 60fps
  - All dismiss correctly
```

**Test 10: Animation Performance**
```
Given:
  - Success animation playing
  - Particles spawned
  - Dialog animating

When:
  - VR mode active

Then:
  - Framerate remains at 90fps minimum
  - No stuttering
  - Smooth animation
```

---

## Implementation Checklist

**Core Functionality:**
- [ ] UnlockStateMachine implemented
- [ ] WBP_UnlockDialog created and styled
- [ ] Credit deduction logic working
- [ ] Save/load unlock progress
- [ ] Part card UI updates on unlock

**Validation:**
- [ ] Level requirement check
- [ ] Credit balance check
- [ ] Already unlocked check
- [ ] Invalid part ID handling

**Feedback:**
- [ ] Success animation (dialog)
- [ ] Success sound effect
- [ ] Haptic feedback (VR)
- [ ] Notification toast
- [ ] Credits display update

**Error Handling:**
- [ ] Insufficient credits warning
- [ ] Level too low warning
- [ ] Save failure rollback
- [ ] Network error handling

**Edge Cases:**
- [ ] Rapid click prevention
- [ ] Real-time credit updates
- [ ] Multiple unlocks queuing
- [ ] Dialog cleanup on level change

**Polish:**
- [ ] All animations smooth and polished
- [ ] Sound timing perfect
- [ ] Text clearly communicates state
- [ ] Accessibility features working

**Testing:**
- [ ] All functional tests pass
- [ ] All edge case tests pass
- [ ] Performance tests pass
- [ ] VR testing complete

---

**Document Version:** 1.0
**Last Updated:** 2025-11-07
**Author:** Ship Customization System - Phase 3, Subagent 4
