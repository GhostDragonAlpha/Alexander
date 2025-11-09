# Alexander Integration Patterns - Quick Reference

> **Key patterns and best practices for integrating autonomous baseline content**

---

## Core Integration Patterns

### Pattern 1: Component-Based Architecture

**Systems are accessed through components:**

```cpp
// Get system component from world
UMissionBoardComponent* MissionBoard = GetWorld()->GetSubsystem<UMissionBoardComponent>();
UVFXManager* VFXManager = FindComponentByClass<UVFXManager>();
UPerformanceProfilerSubsystem* Profiler = GetWorld()->GetSubsystem<UPerformanceProfilerSubsystem>();
```

**Always check for null before use:**

```cpp
if (!MissionBoard)
{
    UE_LOG(LogTemp, Error, TEXT("Mission Board not available"));
    return;
}
```

---

### Pattern 2: Data-Driven Configuration

**Use data assets instead of hardcoding:**

```cpp
// Load from data asset
UMissionTemplateDataAssetClass* TemplateAsset = LoadObject<UMissionTemplateDataAssetClass>(
    nullptr,
    TEXT("/Game/Data/Missions/DA_MissionTemplates.DA_MissionTemplates")
);

// Query templates
TArray<FMissionTemplateData> Templates = TemplateAsset->GetTemplatesForContext(Context);
```

**Benefits:**
- Designers can modify without code changes
- Easy to add new content
- Better organization

---

### Pattern 3: Event-Driven Updates

**Subscribe to system events instead of polling:**

```cpp
// Bind to mission events
MissionBoard->OnMissionAccepted.AddDynamic(this, &UMyWidget::OnMissionAccepted);
MissionBoard->OnMissionCompleted.AddDynamic(this, &UMyWidget::OnMissionCompleted);
MissionBoard->OnMissionObjectiveCompleted.AddDynamic(this, &UMyWidget::OnObjectiveCompleted);

// React to events
void UMyWidget::OnMissionAccepted(APlayerState* Player, const FMissionData& Mission, const FName& BoardName)
{
    RefreshMissionList();
}
```

**Benefits:**
- Better performance (no constant checking)
- Cleaner code
- Immediate response

---

### Pattern 4: Caching References

**Cache component references to avoid repeated lookups:**

```cpp
// In BeginPlay or NativeConstruct
void UMyWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Cache once
    CachedMissionBoard = GetWorld()->GetSubsystem<UMissionBoardComponent>();
    CachedPlayerState = GetOwningPlayer()->GetPlayerState<APlayerState>();
}

// Use cached reference in Tick
void UMyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    if (CachedMissionBoard)
    {
        // Fast access, no lookup
        UpdateUI();
    }
}
```

**Performance Impact:** 10-100x faster than repeated GetWorld() calls.

---

### Pattern 5: Profiling Integration

**Wrap expensive operations in profiling scopes:**

```cpp
void AMySystem::ExpensiveOperation()
{
    UPerformanceProfilerSubsystem* Profiler = GetWorld()->GetSubsystem<UPerformanceProfilerSubsystem>();

    // Automatic timing
    PROFILE_SCOPE(Profiler, TEXT("MySystem_ExpensiveOp"));

    // Do expensive work here
    ProcessComplexData();
}
```

**Query results:**

```cpp
FProfileData Data = Profiler->GetSystemProfile(TEXT("MySystem_ExpensiveOp"));
UE_LOG(LogTemp, Log, TEXT("Average time: %.2f ms"), Data.AverageTimeMs);
```

---

### Pattern 6: VFX Instance Management

**Use instance IDs to control effects:**

```cpp
// Spawn and store ID
FString EffectID = VFXManager->SpawnVFX(System, Location, Rotation, Category, Lifetime);

// Control later
VFXManager->SetVFXParameter(EffectID, TEXT("Intensity"), 0.75f);
VFXManager->StopVFX(EffectID);
```

**Use templates for common effects:**

```cpp
// Register once
FVFXTemplate Template;
Template.TemplateName = TEXT("Explosion");
Template.NiagaraSystem = ExplosionSystem;
VFXManager->RegisterVFXTemplate(Template);

// Spawn many times
FString ID = VFXManager->SpawnVFXFromTemplate(TEXT("Explosion"), Location, Rotation);
```

---

### Pattern 7: Progressive LOD

**Reduce detail with distance:**

```cpp
void UpdateLOD()
{
    float Distance = GetDistanceToPlayer();

    if (Distance < 5000.0f)
    {
        // High detail
        SetActorTickInterval(0.0f);
        bEnableDetailedSimulation = true;
    }
    else if (Distance < 25000.0f)
    {
        // Medium detail
        SetActorTickInterval(0.5f);
        bEnableDetailedSimulation = false;
    }
    else
    {
        // Low detail
        SetActorTickInterval(5.0f);
        bEnableDetailedSimulation = false;
    }
}
```

**Performance Impact:** Up to 10x improvement for distant objects.

---

### Pattern 8: Widget State Binding

**Bind UI to game state automatically:**

```cpp
// Cache ship reference
void UShipHUDWidget::SetShipReference(ASpaceship* Ship)
{
    CachedShip = Ship;
}

// Update from cache (no lookup overhead)
void UShipHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    if (CachedShip)
    {
        SpeedText->SetText(FText::FromString(
            FString::Printf(TEXT("%.0f km/h"), CachedShip->GetVelocity().Size() * 3.6f)
        ));

        HealthBar->SetPercent(CachedShip->HullIntegrity / 100.0f);
    }
}
```

---

### Pattern 9: Station Service Access

**Modular service architecture:**

```cpp
void AccessStationService(ASpaceStationHub* Station, EHubZoneType ServiceType)
{
    switch (ServiceType)
    {
        case EHubZoneType::TradingZone:
            Station->RefreshTradingPost();
            ShowTradingUI(Station->GetAvailableItems());
            break;

        case EHubZoneType::MissionZone:
            Station->RefreshMissionBoard();
            ShowMissionBoardUI(Station->GetAvailableMissions(Player));
            break;

        // ... other services
    }
}
```

**Extensible:** Easy to add new service types.

---

### Pattern 10: VR-Friendly UI

**Adapt UI for VR automatically:**

```cpp
void ApplyVROptimizations()
{
    bool bIsVR = GEngine->XRSystem.IsValid();

    if (bIsVR)
    {
        // Lower resolution
        WidgetComponent->SetDrawSize(FVector2D(1024, 768));

        // Larger fonts
        TitleText->SetFont(GetFont().Size * 1.5f);

        // Simpler animations
        bEnableComplexAnimations = false;

        // Higher contrast
        SetColorAndOpacity(HighContrastColors);
    }
}
```

---

## Common Anti-Patterns to Avoid

### Anti-Pattern 1: Repeated Component Lookups

**BAD:**
```cpp
void Tick(float DeltaTime)
{
    // Expensive lookup every frame!
    UMissionBoardComponent* Board = GetWorld()->GetSubsystem<UMissionBoardComponent>();
    Board->UpdateMissions();
}
```

**GOOD:**
```cpp
void BeginPlay()
{
    // Cache once
    CachedBoard = GetWorld()->GetSubsystem<UMissionBoardComponent>();
}

void Tick(float DeltaTime)
{
    // Fast cached access
    if (CachedBoard)
    {
        CachedBoard->UpdateMissions();
    }
}
```

---

### Anti-Pattern 2: Missing Null Checks

**BAD:**
```cpp
void DoSomething()
{
    UMissionBoardComponent* Board = GetWorld()->GetSubsystem<UMissionBoardComponent>();
    Board->AcceptMission(Player, MissionID); // CRASH if Board is null!
}
```

**GOOD:**
```cpp
void DoSomething()
{
    UMissionBoardComponent* Board = GetWorld()->GetSubsystem<UMissionBoardComponent>();

    if (!Board)
    {
        UE_LOG(LogTemp, Error, TEXT("Mission board not available"));
        return;
    }

    Board->AcceptMission(Player, MissionID);
}
```

---

### Anti-Pattern 3: Ignoring Performance

**BAD:**
```cpp
void Tick(float DeltaTime)
{
    // Updates every frame (60 FPS = 60 updates/sec)
    UpdateComplexUI();
    UpdateMissionList();
    RefreshAllData();
}
```

**GOOD:**
```cpp
void BeginPlay()
{
    // Update UI at 10 FPS (sufficient for most UI)
    GetWorld()->GetTimerManager().SetTimer(
        UIUpdateTimer,
        this,
        &UMyWidget::UpdateUI,
        0.1f,
        true
    );
}

void UpdateUI()
{
    UpdateComplexUI();
    UpdateMissionList();
    RefreshAllData();
}
```

---

### Anti-Pattern 4: Hardcoded Values

**BAD:**
```cpp
void SpawnMission()
{
    FMissionData Mission;
    Mission.Title = TEXT("Kill 10 Pirates");
    Mission.Reward.Credits = 5000;
    // ... hardcoded mission data
}
```

**GOOD:**
```cpp
void SpawnMission()
{
    // Load from data asset
    UMissionTemplateDataAssetClass* Templates = LoadObject<UMissionTemplateDataAssetClass>(...);
    FMissionTemplateData Template = Templates->FindTemplateByID(TEXT("KillPirates"));

    // Generate from template
    FMissionData Mission = GenerateMissionFromTemplate(Template);
}
```

---

### Anti-Pattern 5: No VFX Pooling

**BAD:**
```cpp
void FireWeapon()
{
    // Spawns new effect every shot (expensive!)
    UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        MuzzleFlashSystem,
        MuzzleLocation
    );
}
```

**GOOD:**
```cpp
void BeginPlay()
{
    // Preload pool once
    VFXManager->PreloadVFXPool(TEXT("MuzzleFlash"), 20);
}

void FireWeapon()
{
    // Use pooled instance (fast!)
    FString ID = VFXManager->SpawnVFXFromTemplate(TEXT("MuzzleFlash"), MuzzleLocation, Rotation);
}
```

---

## Performance Guidelines

### Frame Budget Targets

| System | Target Time | Max Time | Notes |
|--------|-------------|----------|-------|
| Mission Update | 0.1 ms | 0.5 ms | Update once per second |
| VFX Spawning | 0.05 ms | 0.2 ms | Use pooling |
| UI Update | 0.5 ms | 2.0 ms | Update at 10-30 Hz |
| Station Simulation | 0.2 ms | 1.0 ms | Use LOD for distant stations |
| Profiling Overhead | 0.01 ms | 0.05 ms | Negligible when optimized |

### Memory Budget Targets

| System | Typical | Maximum | Notes |
|--------|---------|---------|-------|
| Mission Data | 5 MB | 20 MB | Limit active missions |
| VFX Instances | 10 MB | 50 MB | Pool and cull |
| UI Widgets | 20 MB | 100 MB | Unload when hidden |
| Station Assets | 50 MB | 200 MB | Stream distant stations |
| Profiling Data | 1 MB | 5 MB | Export and clear periodically |

---

## Integration Checklist

### When Adding a New System:

- [ ] Create component class (inherit from UActorComponent or UWorldSubsystem)
- [ ] Add profiling scopes to expensive functions
- [ ] Implement BeginPlay/EndPlay cleanup
- [ ] Add null checks for all external references
- [ ] Cache frequently accessed components
- [ ] Use timers instead of Tick when possible
- [ ] Add Blueprint-callable functions (UFUNCTION)
- [ ] Document public API in header comments
- [ ] Add LOD/culling for distant objects
- [ ] Test in VR if applicable

### When Adding a New VFX:

- [ ] Create Niagara system in Content Browser
- [ ] Register as VFX template
- [ ] Set appropriate category
- [ ] Configure pooling (pool size = expected simultaneous spawns)
- [ ] Test performance with 50+ simultaneous instances
- [ ] Add quality scaling parameters
- [ ] Document required parameters

### When Adding a New UI Widget:

- [ ] Create UMG widget blueprint
- [ ] Add C++ widget class for logic
- [ ] Use BindWidget for UI elements
- [ ] Cache component references in NativeConstruct
- [ ] Update at appropriate rate (not every frame)
- [ ] Test in VR (larger fonts, simpler layout)
- [ ] Handle show/hide with proper input mode switching
- [ ] Add keyboard/gamepad navigation

### When Adding a New Mission Type:

- [ ] Add enum value to EMissionType
- [ ] Create mission template data asset
- [ ] Define objectives with clear completion criteria
- [ ] Set appropriate rewards
- [ ] Add to mission board generation
- [ ] Test mission flow (accept → progress → complete)
- [ ] Add UI representation
- [ ] Document for designers

---

## Quick Troubleshooting

### "Component not found" errors

**Solution:** Ensure component is added in BeginPlay or exists in world:

```cpp
if (!MyComponent)
{
    MyComponent = FindComponentByClass<UMyComponent>();

    if (!MyComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Component still not found - check Blueprint setup"));
        return;
    }
}
```

### VFX not appearing

**Solution:** Check category limits and quality settings:

```cpp
// Increase limits
VFXManager->SetCategoryMaxVFX(EVFXCategory::Weapon, 50);

// Check if instance is active
bool bActive = VFXManager->IsVFXActive(InstanceID);
UE_LOG(LogTemp, Log, TEXT("VFX Active: %s"), bActive ? TEXT("Yes") : TEXT("No"));
```

### Mission not accepting

**Solution:** Check prerequisites and player level:

```cpp
if (!MissionBoard->CanPlayerAcceptMission(Player, MissionID))
{
    FMissionData* Mission = MissionBoard->GetMission(MissionID);

    UE_LOG(LogTemp, Warning, TEXT("Cannot accept mission:"));
    UE_LOG(LogTemp, Warning, TEXT("  Required Level: %d"), Mission->LevelRequirement);
    UE_LOG(LogTemp, Warning, TEXT("  Status: %d"), (int32)Mission->Status);
    UE_LOG(LogTemp, Warning, TEXT("  Prerequisites: %d"), Mission->PrerequisiteMissions.Num());
}
```

### UI not updating

**Solution:** Verify widget tick is enabled and reference is valid:

```cpp
void NativeConstruct()
{
    Super::NativeConstruct();

    // Enable tick
    SetIsFocusable(true);

    // Verify references
    if (!SpeedText)
    {
        UE_LOG(LogTemp, Error, TEXT("SpeedText not bound - check widget names"));
    }
}
```

### Performance issues

**Solution:** Use profiler to identify bottlenecks:

```cpp
UPerformanceProfilerSubsystem* Profiler = GetWorld()->GetSubsystem<UPerformanceProfilerSubsystem>();
Profiler->LogTopBottlenecks(10);
Profiler->ExportToJSON(TEXT("C:/Temp/profile.json"));
```

---

## Key Takeaways

1. **Cache component references** - Don't lookup every frame
2. **Always check for null** - Prevent crashes
3. **Use events over polling** - Better performance
4. **Profile your code** - Measure before optimizing
5. **Pool VFX instances** - Avoid spawn overhead
6. **Use data assets** - Make content designer-friendly
7. **Implement LOD** - Scale with distance
8. **Test in VR** - Different UI requirements
9. **Document your API** - Help future developers
10. **Follow naming conventions** - Consistency matters

---

For complete code examples, see:
- c:\Users\allen\Desktop\Alexander\Alexander\INTEGRATION_CODE_EXAMPLES.md
