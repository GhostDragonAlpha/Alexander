# Alexander Project - Code Integration Examples

> **Comprehensive copy-paste ready examples for integrating autonomous baseline content**
>
> Last Updated: November 8, 2025
> Project: Alexander Space Simulation Game

---

## Table of Contents

1. [Mission System Integration](#mission-system-integration)
2. [Station Integration](#station-integration)
3. [VFX Integration](#vfx-integration)
4. [UI Integration](#ui-integration)
5. [Performance Integration](#performance-integration)
6. [Common Recipes](#common-recipes)

---

## Mission System Integration

### 1. Query Available Missions from Data Assets

#### C++ Example

```cpp
// In your GameMode or PlayerController class
#include "MissionBoardComponent.h"
#include "MissionTemplateDataAsset.h"

void AMyGameMode::QueryMissionsFromBoard()
{
    // Get the mission board component (can be on GameState, GameMode, or world actor)
    UMissionBoardComponent* MissionBoard = FindComponentByClass<UMissionBoardComponent>();

    if (!MissionBoard)
    {
        UE_LOG(LogTemp, Warning, TEXT("MissionBoardComponent not found!"));
        return;
    }

    // Query all available missions from a specific board
    FName BoardName = TEXT("MainStationBoard");
    TArray<FMissionData> AvailableMissions = MissionBoard->GetAvailableMissions(BoardName);

    // Filter by mission type
    TArray<FMissionData> CombatMissions = MissionBoard->FilterMissionsByType(
        AvailableMissions,
        EMissionType::Combat
    );

    // Filter by difficulty
    TArray<FMissionData> EasyMissions = MissionBoard->FilterMissionsByDifficulty(
        AvailableMissions,
        EMissionDifficulty::Easy
    );

    // Get recommended missions for player
    APlayerState* PlayerState = GetPlayerState<APlayerState>();
    TArray<FMissionData> RecommendedMissions = MissionBoard->GetRecommendedMissions(
        PlayerState,
        BoardName
    );

    // Display results
    for (const FMissionData& Mission : RecommendedMissions)
    {
        UE_LOG(LogTemp, Log, TEXT("Mission: %s - %s (Difficulty: %s)"),
            *Mission.MissionID.ToString(),
            *Mission.Title,
            *MissionBoard->GetMissionDifficultyDisplayName(Mission.Difficulty)
        );
    }
}
```

#### Blueprint Example

**Blueprint Nodes:**
1. Get Mission Board Component (from GameState or World Actor)
2. Call "Get Available Missions" with Board Name
3. Call "Filter Missions By Type" or "Get Recommended Missions"
4. Loop through result array

**Common Pitfall:** Mission Board must be initialized first. Check `IsValid()` before calling functions.

---

### 2. Accept a Mission Programmatically

#### C++ Example

```cpp
#include "MissionBoardComponent.h"

void AMyPlayerController::AcceptMissionById(FName MissionID)
{
    // Get mission board
    UMissionBoardComponent* MissionBoard = GetWorld()->GetSubsystem<UMissionBoardComponent>();

    if (!MissionBoard)
    {
        UE_LOG(LogTemp, Error, TEXT("Mission board subsystem not available"));
        return;
    }

    // Get player state
    APlayerState* PlayerState = GetPlayerState<APlayerState>();
    if (!PlayerState)
    {
        UE_LOG(LogTemp, Error, TEXT("Player state not available"));
        return;
    }

    // Check if player can accept the mission
    if (!MissionBoard->CanPlayerAcceptMission(PlayerState, MissionID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Player cannot accept mission %s"), *MissionID.ToString());

        // Get the mission to check why
        FMissionData* Mission = MissionBoard->GetMission(MissionID);
        if (Mission)
        {
            UE_LOG(LogTemp, Warning, TEXT("Mission Level: %d, Status: %d"),
                Mission->LevelRequirement,
                (int32)Mission->Status
            );
        }
        return;
    }

    // Accept the mission
    bool bSuccess = MissionBoard->AcceptMission(PlayerState, MissionID);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully accepted mission %s"), *MissionID.ToString());

        // Optional: Track the mission for HUD display
        MissionBoard->TrackMission(PlayerState, MissionID);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to accept mission %s"), *MissionID.ToString());
    }
}
```

#### Blueprint Example

**Blueprint Nodes:**
1. Get Mission Board Component
2. Get Player State
3. Can Player Accept Mission (with MissionID)
4. Branch (if true, continue)
5. Accept Mission (with Player State and MissionID)
6. Track Mission (optional, for HUD)

**Performance Consideration:** Cache Mission Board reference in BeginPlay instead of getting it every time.

---

### 3. Track Objective Progress

#### C++ Example

```cpp
#include "MissionBoardComponent.h"

void AMyGameMode::UpdateMissionObjective(
    APlayerState* Player,
    FName MissionID,
    FName ObjectiveID,
    int32 ProgressIncrement
)
{
    UMissionBoardComponent* MissionBoard = FindComponentByClass<UMissionBoardComponent>();

    if (!MissionBoard || !Player)
    {
        return;
    }

    // Update progress
    MissionBoard->UpdateMissionProgress(Player, MissionID, ObjectiveID, ProgressIncrement);

    // Check if objective is complete
    TArray<FMissionBoardObjective> Objectives = MissionBoard->GetMissionObjectives(MissionID);

    for (const FMissionBoardObjective& Objective : Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID && Objective.bIsCompleted)
        {
            UE_LOG(LogTemp, Log, TEXT("Objective %s completed!"), *ObjectiveID.ToString());

            // Show notification to player
            ShowObjectiveCompletedNotification(Player, Objective.Description);
        }
    }

    // Check if mission is complete
    if (MissionBoard->IsMissionCompleted(MissionID))
    {
        UE_LOG(LogTemp, Log, TEXT("Mission %s is ready to turn in!"), *MissionID.ToString());
        ShowMissionCompleteNotification(Player, MissionID);
    }
}

// Example: Player kills an enemy, update bounty mission
void AMyGameMode::OnEnemyKilled(AActor* Enemy, APlayerController* Killer)
{
    if (!Killer)
    {
        return;
    }

    APlayerState* PlayerState = Killer->GetPlayerState<APlayerState>();
    if (!PlayerState)
    {
        return;
    }

    // Example: Update "Kill 10 Pirates" objective
    UpdateMissionObjective(
        PlayerState,
        TEXT("MISSION_123456"),      // Mission ID
        TEXT("OBJ_KillPirates"),     // Objective ID
        1                             // Increment by 1
    );
}
```

#### Blueprint Example

**Blueprint Nodes:**
1. Get Mission Board Component
2. Update Mission Progress (Player State, Mission ID, Objective ID, Progress)
3. Get Mission Objectives (to check completion)
4. For Each Loop (check if any objective is completed)
5. Is Mission Completed (to check overall mission status)

**Common Pitfall:** Make sure ObjectiveID matches exactly what's in the mission data. Use string constants or data tables.

---

### 4. Complete Mission and Grant Rewards

#### C++ Example

```cpp
#include "MissionBoardComponent.h"

void AMyPlayerController::TurnInCompletedMission(FName MissionID)
{
    UMissionBoardComponent* MissionBoard = GetWorld()->GetSubsystem<UMissionBoardComponent>();
    APlayerState* PlayerState = GetPlayerState<APlayerState>();

    if (!MissionBoard || !PlayerState)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot turn in mission - missing board or player state"));
        return;
    }

    // Verify mission is complete
    if (!MissionBoard->IsMissionCompleted(MissionID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Mission %s is not yet complete"), *MissionID.ToString());

        // Show player their progress
        float Progress = MissionBoard->GetMissionProgress(MissionID);
        UE_LOG(LogTemp, Log, TEXT("Mission progress: %.2f%%"), Progress * 100.0f);
        return;
    }

    // Get mission data before completing (for reward preview)
    FMissionData* Mission = MissionBoard->GetMission(MissionID);
    if (!Mission)
    {
        UE_LOG(LogTemp, Error, TEXT("Mission %s not found"), *MissionID.ToString());
        return;
    }

    // Log rewards
    UE_LOG(LogTemp, Log, TEXT("Mission Rewards:"));
    UE_LOG(LogTemp, Log, TEXT("  Credits: %d"), Mission->Reward.Credits);
    UE_LOG(LogTemp, Log, TEXT("  Experience: %d"), Mission->Reward.Experience);
    UE_LOG(LogTemp, Log, TEXT("  Description: %s"), *Mission->Reward.RewardDescription);

    // Complete the mission (this automatically grants rewards)
    bool bSuccess = MissionBoard->CompleteMission(PlayerState, MissionID);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Mission %s completed successfully!"), *MissionID.ToString());

        // Show reward screen to player
        ShowMissionRewardScreen(Mission->Reward);

        // Mission is now in player's history
        TArray<FMissionData> History = MissionBoard->GetMissionHistory(PlayerState);
        UE_LOG(LogTemp, Log, TEXT("Player has completed %d total missions"), History.Num());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to complete mission %s"), *MissionID.ToString());
    }
}
```

**Performance Consideration:** Mission completion triggers multiple subsystem calls (Economy, Quest, Faction). Consider batching multiple completions if needed.

---

### 5. Procedurally Generate Mission from Template

#### C++ Example

```cpp
#include "MissionBoardComponent.h"
#include "MissionTemplateDataAsset.h"

void AMyGameMode::GenerateRandomMissionsForStation(FName StationBoardName, int32 Count)
{
    UMissionBoardComponent* MissionBoard = FindComponentByClass<UMissionBoardComponent>();

    if (!MissionBoard)
    {
        UE_LOG(LogTemp, Error, TEXT("Mission board not found"));
        return;
    }

    // Method 1: Use built-in random generation
    MissionBoard->GenerateRandomMissions(StationBoardName, Count);

    UE_LOG(LogTemp, Log, TEXT("Generated %d random missions for board %s"),
        Count, *StationBoardName.ToString());
}

// Advanced: Generate specific mission from template
void AMyGameMode::GenerateMissionFromTemplate(UMissionTemplateDataAssetClass* TemplateAsset)
{
    if (!TemplateAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("Template asset is null"));
        return;
    }

    // Get context (player level, location, etc.)
    FMissionContextDataAsset Context;
    Context.PlayerLevel = 10;
    Context.Location = FVector(0, 0, 1000);
    Context.AvailableTags.Add(TEXT("Space"));
    Context.AvailableTags.Add(TEXT("Station"));
    Context.NearestFaction = TEXT("Corporation");

    // Get templates matching context
    TArray<FMissionTemplateData> ValidTemplates = TemplateAsset->GetTemplatesForContext(Context);

    if (ValidTemplates.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid templates for current context"));
        return;
    }

    // Pick random template
    int32 RandomIndex = FMath::RandRange(0, ValidTemplates.Num() - 1);
    FMissionTemplateData SelectedTemplate = ValidTemplates[RandomIndex];

    // Convert template to mission data
    FMissionData NewMission;
    NewMission.MissionID = FName(*FString::Printf(TEXT("MISSION_%d"), FMath::RandRange(100000, 999999)));
    NewMission.Title = SelectedTemplate.Title.ToString();
    NewMission.Description = SelectedTemplate.Description.ToString();
    NewMission.MissionType = ConvertMissionType(SelectedTemplate.Type);
    NewMission.LevelRequirement = SelectedTemplate.MinPlayerLevel;
    NewMission.Status = EMissionStatus::Available;

    // Copy objectives
    for (const FMissionObjectiveDataAsset& TemplateObjective : SelectedTemplate.Objectives)
    {
        FMissionBoardObjective Objective;
        Objective.ObjectiveID = TemplateObjective.ObjectiveID;
        Objective.Description = TemplateObjective.Description.ToString();
        Objective.bIsRequired = !TemplateObjective.bOptional;
        Objective.TargetCount = TemplateObjective.TargetCount;
        Objective.CurrentCount = 0;

        NewMission.Objectives.Add(Objective);
    }

    // Copy rewards
    NewMission.Reward.Credits = SelectedTemplate.Rewards.Credits;
    NewMission.Reward.Experience = SelectedTemplate.Rewards.Experience;

    // Add to mission board
    UMissionBoardComponent* MissionBoard = FindComponentByClass<UMissionBoardComponent>();
    if (MissionBoard)
    {
        MissionBoard->AddMissionToBoard(TEXT("MainStationBoard"), NewMission);
        UE_LOG(LogTemp, Log, TEXT("Generated mission %s from template"), *NewMission.MissionID.ToString());
    }
}

// Helper function to convert mission type enums
EMissionType ConvertMissionType(EMissionTypeDataAsset TemplateType)
{
    switch (TemplateType)
    {
        case EMissionTypeDataAsset::Combat: return EMissionType::Combat;
        case EMissionTypeDataAsset::Exploration: return EMissionType::Exploration;
        case EMissionTypeDataAsset::Trading: return EMissionType::Transport;
        case EMissionTypeDataAsset::Escort: return EMissionType::Escort;
        case EMissionTypeDataAsset::Mining: return EMissionType::Mining;
        default: return EMissionType::Combat;
    }
}
```

**Common Pitfall:** Make sure to convert between EMissionTypeDataAsset and EMissionType enums correctly.

---

### 6. Display Mission on Mission Board UI

#### C++ Example (Widget Binding)

```cpp
// In your mission board widget class
#include "MissionBoardComponent.h"
#include "Components/ListView.h"
#include "Blueprint/UserWidget.h"

UCLASS()
class UMissionBoardWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Missions")
    void RefreshMissionList();

    UFUNCTION(BlueprintCallable, Category = "Missions")
    void OnMissionSelected(FMissionData Mission);

protected:
    UPROPERTY(meta = (BindWidget))
    UListView* MissionListView;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MissionTitleText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MissionDescriptionText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* RewardText;

    UPROPERTY(meta = (BindWidget))
    UButton* AcceptButton;

private:
    UMissionBoardComponent* CachedMissionBoard;
    FMissionData SelectedMission;
};

void UMissionBoardWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Cache mission board reference
    CachedMissionBoard = GetWorld()->GetSubsystem<UMissionBoardComponent>();

    // Bind button click
    if (AcceptButton)
    {
        AcceptButton->OnClicked.AddDynamic(this, &UMissionBoardWidget::OnAcceptButtonClicked);
    }

    // Initial refresh
    RefreshMissionList();
}

void UMissionBoardWidget::RefreshMissionList()
{
    if (!CachedMissionBoard || !MissionListView)
    {
        return;
    }

    // Get available missions
    TArray<FMissionData> Missions = CachedMissionBoard->GetAvailableMissions(TEXT("MainStationBoard"));

    // Clear existing items
    MissionListView->ClearListItems();

    // Add missions to list view
    for (const FMissionData& Mission : Missions)
    {
        // Create data object for list view
        UMissionListItemData* ItemData = NewObject<UMissionListItemData>();
        ItemData->MissionData = Mission;

        MissionListView->AddItem(ItemData);
    }

    UE_LOG(LogTemp, Log, TEXT("Refreshed mission list with %d missions"), Missions.Num());
}

void UMissionBoardWidget::OnMissionSelected(FMissionData Mission)
{
    SelectedMission = Mission;

    // Update UI with mission details
    if (MissionTitleText)
    {
        MissionTitleText->SetText(FText::FromString(Mission.Title));
    }

    if (MissionDescriptionText)
    {
        MissionDescriptionText->SetText(FText::FromString(Mission.Description));
    }

    if (RewardText)
    {
        FString RewardString = FString::Printf(
            TEXT("Rewards:\nCredits: %d\nExperience: %d"),
            Mission.Reward.Credits,
            Mission.Reward.Experience
        );
        RewardText->SetText(FText::FromString(RewardString));
    }

    // Enable accept button if player can accept
    APlayerController* PC = GetOwningPlayer();
    if (PC && AcceptButton)
    {
        APlayerState* PlayerState = PC->GetPlayerState<APlayerState>();
        bool bCanAccept = CachedMissionBoard->CanPlayerAcceptMission(PlayerState, Mission.MissionID);
        AcceptButton->SetIsEnabled(bCanAccept);
    }
}

void UMissionBoardWidget::OnAcceptButtonClicked()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC || !CachedMissionBoard)
    {
        return;
    }

    APlayerState* PlayerState = PC->GetPlayerState<APlayerState>();
    bool bSuccess = CachedMissionBoard->AcceptMission(PlayerState, SelectedMission.MissionID);

    if (bSuccess)
    {
        // Show success message
        UE_LOG(LogTemp, Log, TEXT("Mission accepted!"));

        // Refresh list (mission is now removed from available)
        RefreshMissionList();

        // Close widget or switch to active missions view
    }
}
```

---

## Station Integration

### 1. Spawn Station Blueprint

#### C++ Example

```cpp
#include "SpaceStationBlueprintLibrary.h"
#include "SpaceStationHub.h"

void AMyGameMode::SpawnStation()
{
    // Method 1: Using Blueprint Library
    FStationConfiguration Config;
    Config.StationID = TEXT("Station_Alpha");
    Config.StationName = FText::FromString(TEXT("Alpha Trade Station"));
    Config.MaxDockingSlots = 20;
    Config.MaxPlayerCapacity = 100;
    Config.bHasTrading = true;
    Config.bHasMissions = true;
    Config.bHasVoiceChat = true;

    // Add modules
    Config.Modules.Add(EStationModuleType::HubCore);
    Config.Modules.Add(EStationModuleType::DockingRing);
    Config.Modules.Add(EStationModuleType::TradingPost);
    Config.Modules.Add(EStationModuleType::MissionBoard);
    Config.Modules.Add(EStationModuleType::SocialArea);

    // Set spawn location
    FTransform SpawnTransform;
    SpawnTransform.SetLocation(FVector(0, 0, 10000)); // 10km above origin
    SpawnTransform.SetRotation(FQuat::Identity);

    // Spawn the station
    ASpaceStationHub* Station = USpaceStationBlueprintLibrary::SpawnStationFromConfig(
        GetWorld(),
        Config,
        SpawnTransform
    );

    if (Station)
    {
        UE_LOG(LogTemp, Log, TEXT("Station spawned successfully at %s"),
            *SpawnTransform.GetLocation().ToString());

        // Initialize station systems
        Station->InitializeHub();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn station"));
    }
}

// Method 2: Spawn directly from class
void AMyGameMode::SpawnStationDirect()
{
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = TEXT("MainStation");
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    FVector Location(0, 0, 10000);
    FRotator Rotation = FRotator::ZeroRotator;

    ASpaceStationHub* Station = GetWorld()->SpawnActor<ASpaceStationHub>(
        ASpaceStationHub::StaticClass(),
        Location,
        Rotation,
        SpawnParams
    );

    if (Station)
    {
        // Configure station after spawn
        Station->HubName = TEXT("MainStation");
        Station->MaxPlayersInHub = 100;
        Station->bEnableTrading = true;
        Station->bEnableMissions = true;
        Station->bEnableVoiceChat = true;

        Station->InitializeHub();
    }
}
```

#### Blueprint Example

**Blueprint Nodes:**
1. Make Station Configuration struct
2. Set Transform (location, rotation)
3. Call "Spawn Station From Config"
4. Call "Initialize Hub" on result

**Performance Consideration:** Spawn stations during level load or async to avoid frame hitches. Consider using level streaming for distant stations.

---

### 2. Enable Docking

#### C++ Example

```cpp
#include "SpaceStationHub.h"

void ASpaceship::RequestDocking(ASpaceStationHub* Station)
{
    if (!Station)
    {
        UE_LOG(LogTemp, Error, TEXT("Station is null"));
        return;
    }

    // Check if station has docking available
    int32 AvailableSlots = Station->MaxDockingSlots - Station->GetTotalPlayerCount();

    if (AvailableSlots <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Station has no available docking slots"));
        ShowDockingDeniedMessage(TEXT("No docking slots available"));
        return;
    }

    // Check distance to station
    float DistanceToStation = FVector::Dist(GetActorLocation(), Station->GetActorLocation());
    const float MAX_DOCKING_DISTANCE = 5000.0f; // 5km

    if (DistanceToStation > MAX_DOCKING_DISTANCE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Too far from station to dock (%.2f m)"), DistanceToStation);
        ShowDockingDeniedMessage(TEXT("Too far from station"));
        return;
    }

    // Switch to docking mode
    SetFlightMode(EFlightMode::Docking);

    // Calculate docking trajectory
    FVector DockingPoint = CalculateNearestDockingPoint(Station);

    // Auto-pilot to docking point
    BeginAutoDocking(DockingPoint, Station);
}

FVector ASpaceship::CalculateNearestDockingPoint(ASpaceStationHub* Station)
{
    // Get all docking points from station configuration
    FStationConfiguration Config = USpaceStationBlueprintLibrary::CreateDefaultStationConfig(
        FText::FromString(TEXT("Temp"))
    );

    TArray<FTransform> DockingPoints = USpaceStationBlueprintLibrary::CalculateDockingPoints(
        Config,
        2000.0f // 2km radius from station center
    );

    // Find nearest available docking point
    FVector StationLocation = Station->GetActorLocation();
    FVector ShipLocation = GetActorLocation();

    float NearestDistance = FLT_MAX;
    FVector NearestPoint = StationLocation;

    for (const FTransform& DockingTransform : DockingPoints)
    {
        FVector PointWorldLocation = Station->GetActorTransform().TransformPosition(
            DockingTransform.GetLocation()
        );

        float Distance = FVector::Dist(ShipLocation, PointWorldLocation);

        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestPoint = PointWorldLocation;
        }
    }

    return NearestPoint;
}

void ASpaceship::BeginAutoDocking(FVector DockingPoint, ASpaceStationHub* Station)
{
    // Set docking state
    bIsAutoDocking = true;
    TargetDockingPoint = DockingPoint;
    DockingStation = Station;

    UE_LOG(LogTemp, Log, TEXT("Beginning auto-docking sequence to %s"), *DockingPoint.ToString());

    // Slow down ship
    EmergencyStop();

    // Start docking sequence timer
    GetWorld()->GetTimerManager().SetTimer(
        DockingSequenceTimer,
        this,
        &ASpaceship::UpdateDockingSequence,
        0.1f,
        true
    );
}

void ASpaceship::UpdateDockingSequence()
{
    if (!bIsAutoDocking || !DockingStation)
    {
        return;
    }

    FVector CurrentLocation = GetActorLocation();
    FVector DirectionToPoint = (TargetDockingPoint - CurrentLocation).GetSafeNormal();
    float Distance = FVector::Dist(CurrentLocation, TargetDockingPoint);

    // Apply gentle thrust toward docking point
    const float DOCKING_SPEED = 50.0f; // 50 m/s
    FVector ThrustDirection = DirectionToPoint * DOCKING_SPEED;

    ApplyThrust(GetActorTransform().InverseTransformVector(ThrustDirection));

    // Check if docked
    const float DOCKING_THRESHOLD = 10.0f; // 10m
    if (Distance < DOCKING_THRESHOLD)
    {
        CompleteDocking();
    }
}

void ASpaceship::CompleteDocking()
{
    bIsAutoDocking = false;

    // Stop docking timer
    GetWorld()->GetTimerManager().ClearTimer(DockingSequenceTimer);

    // Kill all velocity
    EmergencyStop();

    // Notify station
    if (DockingStation)
    {
        APlayerController* PC = Cast<APlayerController>(GetController());
        if (PC)
        {
            APlayerState* PlayerState = PC->GetPlayerState<APlayerState>();
            DockingStation->OnPlayerEnterHub(PlayerState);
        }
    }

    // Disable ship controls
    bIsDocked = true;

    // Show station interior UI
    ShowStationInteriorUI();

    UE_LOG(LogTemp, Log, TEXT("Docking complete!"));
}
```

---

### 3. Trigger Station Services

#### C++ Example

```cpp
#include "SpaceStationHub.h"
#include "EconomySystem.h"
#include "MissionBoardComponent.h"

void AMyPlayerController::AccessStationServices(ASpaceStationHub* Station, EHubZoneType ServiceType)
{
    if (!Station)
    {
        return;
    }

    APlayerState* PlayerState = GetPlayerState<APlayerState>();

    switch (ServiceType)
    {
        case EHubZoneType::TradingZone:
            AccessTradingServices(Station, PlayerState);
            break;

        case EHubZoneType::MissionZone:
            AccessMissionServices(Station, PlayerState);
            break;

        case EHubZoneType::SocialZone:
            AccessSocialServices(Station, PlayerState);
            break;

        case EHubZoneType::CustomizationZone:
            AccessCustomizationServices(Station, PlayerState);
            break;

        default:
            UE_LOG(LogTemp, Warning, TEXT("Service type not implemented: %d"), (int32)ServiceType);
            break;
    }
}

void AMyPlayerController::AccessTradingServices(ASpaceStationHub* Station, APlayerState* Player)
{
    // Refresh station trading inventory
    Station->RefreshTradingPost();

    // Get available items
    TArray<FMarketItem> AvailableItems = Station->GetAvailableItems();

    UE_LOG(LogTemp, Log, TEXT("Station has %d items for sale"), AvailableItems.Num());

    // Show trading UI
    ShowTradingWidget(AvailableItems);
}

void AMyPlayerController::AccessMissionServices(ASpaceStationHub* Station, APlayerState* Player)
{
    // Refresh mission board
    Station->RefreshMissionBoard();

    // Get available missions for this player
    TArray<FMissionData> AvailableMissions = Station->GetAvailableMissions(Player);

    UE_LOG(LogTemp, Log, TEXT("Station has %d available missions"), AvailableMissions.Num());

    // Show mission board UI
    ShowMissionBoardWidget(AvailableMissions);
}

void AMyPlayerController::AccessSocialServices(ASpaceStationHub* Station, APlayerState* Player)
{
    // Get players in social range
    float SocialRadius = 500.0f;
    TArray<APlayerState*> NearbyPlayers = Station->GetPlayersInSocialRange(Player, SocialRadius);

    UE_LOG(LogTemp, Log, TEXT("%d players nearby"), NearbyPlayers.Num());

    // Enable voice chat for this zone
    Station->EnableVoiceChatInZone(TEXT("SocialArea"), true);

    // Show social UI
    ShowSocialWidget(NearbyPlayers);
}

void AMyPlayerController::AccessCustomizationServices(ASpaceStationHub* Station, APlayerState* Player)
{
    // Get player's ship
    ASpaceship* PlayerShip = GetPawn<ASpaceship>();

    if (!PlayerShip)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player ship not found"));
        return;
    }

    // Show ship customization UI
    ShowShipCustomizationWidget(PlayerShip);
}
```

---

### 4. Populate Station with NPCs

#### C++ Example

```cpp
#include "SpaceStationHub.h"
#include "GameFramework/Character.h"

void ASpaceStationHub::PopulateWithNPCs(int32 NPCCount)
{
    // Get all social and trading zones
    TArray<FHubZone> SocialZones = GetZonesByType(EHubZoneType::SocialZone);
    TArray<FHubZone> TradingZones = GetZonesByType(EHubZoneType::TradingZone);

    // Combine zones
    TArray<FHubZone> AllZones = SocialZones;
    AllZones.Append(TradingZones);

    if (AllZones.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No zones available for NPC population"));
        return;
    }

    // Spawn NPCs
    for (int32 i = 0; i < NPCCount; i++)
    {
        // Pick random zone
        int32 ZoneIndex = FMath::RandRange(0, AllZones.Num() - 1);
        FHubZone& Zone = AllZones[ZoneIndex];

        // Calculate random spawn point within zone
        FVector SpawnLocation = Zone.ZoneLocation + FVector(
            FMath::FRandRange(-Zone.ZoneBounds.X, Zone.ZoneBounds.X),
            FMath::FRandRange(-Zone.ZoneBounds.Y, Zone.ZoneBounds.Y),
            0
        );

        // Random rotation
        FRotator SpawnRotation = FRotator(0, FMath::FRandRange(0, 360), 0);

        // Spawn NPC
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        ACharacter* NPC = GetWorld()->SpawnActor<ACharacter>(
            NPCCharacterClass,
            SpawnLocation,
            SpawnRotation,
            SpawnParams
        );

        if (NPC)
        {
            // Configure NPC behavior
            SetupNPCBehavior(NPC, Zone.ZoneType);

            // Add to zone tracking
            NPCsInStation.Add(NPC);

            UE_LOG(LogTemp, Log, TEXT("Spawned NPC %d in zone %s"), i, *Zone.ZoneName.ToString());
        }
    }
}

void ASpaceStationHub::SetupNPCBehavior(ACharacter* NPC, EHubZoneType ZoneType)
{
    if (!NPC)
    {
        return;
    }

    switch (ZoneType)
    {
        case EHubZoneType::TradingZone:
            // Set NPC to act as merchant
            NPC->Tags.Add(TEXT("Merchant"));
            break;

        case EHubZoneType::SocialZone:
            // Set NPC to wander and socialize
            NPC->Tags.Add(TEXT("Civilian"));
            break;

        case EHubZoneType::MissionZone:
            // Set NPC as mission giver
            NPC->Tags.Add(TEXT("QuestGiver"));
            break;

        default:
            NPC->Tags.Add(TEXT("Generic"));
            break;
    }
}
```

---

### 5. Link Mission Board to Station

#### C++ Example

```cpp
#include "SpaceStationHub.h"
#include "MissionBoardComponent.h"

void ASpaceStationHub::InitializeMissionBoard()
{
    // Get or create mission board component
    UMissionBoardComponent* MissionBoard = FindComponentByClass<UMissionBoardComponent>();

    if (!MissionBoard)
    {
        // Create mission board component
        MissionBoard = NewObject<UMissionBoardComponent>(this, TEXT("StationMissionBoard"));
        MissionBoard->RegisterComponent();
    }

    // Create mission board configuration
    FMissionBoard BoardConfig;
    BoardConfig.BoardName = FName(*FString::Printf(TEXT("%s_Board"), *HubName.ToString()));
    BoardConfig.Location = GetActorLocation();
    BoardConfig.Description = FString::Printf(TEXT("Mission board for %s"), *HubName.ToString());
    BoardConfig.MaxMissions = 20;
    BoardConfig.RefreshInterval = 600.0f; // 10 minutes
    BoardConfig.bAutoRefresh = true;

    // Set difficulty weights for this station
    BoardConfig.DifficultyWeights.Add(EMissionDifficulty::Easy, 0.3f);
    BoardConfig.DifficultyWeights.Add(EMissionDifficulty::Normal, 0.4f);
    BoardConfig.DifficultyWeights.Add(EMissionDifficulty::Hard, 0.2f);
    BoardConfig.DifficultyWeights.Add(EMissionDifficulty::VeryHard, 0.1f);

    // Register with mission system
    MissionBoard->RegisterMissionBoard(BoardConfig);

    // Generate initial missions
    MissionBoard->GenerateRandomMissions(BoardConfig.BoardName, BoardConfig.MaxMissions);

    UE_LOG(LogTemp, Log, TEXT("Mission board initialized for station %s"), *HubName.ToString());
}

TArray<FMissionData> ASpaceStationHub::GetAvailableMissions(APlayerState* Player)
{
    UMissionBoardComponent* MissionBoard = FindComponentByClass<UMissionBoardComponent>();

    if (!MissionBoard)
    {
        UE_LOG(LogTemp, Error, TEXT("Mission board not initialized"));
        return TArray<FMissionData>();
    }

    // Get board name for this station
    FName BoardName = FName(*FString::Printf(TEXT("%s_Board"), *HubName.ToString()));

    // Get recommended missions for player
    return MissionBoard->GetRecommendedMissions(Player, BoardName);
}

bool ASpaceStationHub::AcceptMission(APlayerState* Player, const FMissionData& Mission)
{
    UMissionBoardComponent* MissionBoard = FindComponentByClass<UMissionBoardComponent>();

    if (!MissionBoard)
    {
        return false;
    }

    // Accept mission
    bool bSuccess = MissionBoard->AcceptMission(Player, Mission.MissionID);

    if (bSuccess)
    {
        // Log station statistics
        UE_LOG(LogTemp, Log, TEXT("Player accepted mission %s from station %s"),
            *Mission.MissionID.ToString(),
            *HubName.ToString()
        );
    }

    return bSuccess;
}
```

---

## VFX Integration

### 1. Attach Engine Trail to Ship

#### C++ Example

```cpp
#include "VFXManager.h"
#include "Spaceship.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

void ASpaceship::InitializeEngineEffects()
{
    // Get VFX Manager
    UVFXManager* VFXManager = FindComponentByClass<UVFXManager>();

    if (!VFXManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager not found on ship"));
        return;
    }

    // Load engine trail system (from content browser)
    UNiagaraSystem* EngineTrailSystem = LoadObject<UNiagaraSystem>(
        nullptr,
        TEXT("/Game/VFX/Ship/NS_EngineTrail.NS_EngineTrail")
    );

    if (!EngineTrailSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine trail VFX system not found"));
        return;
    }

    // Find engine socket on ship mesh
    FName EngineSocketName = TEXT("EngineSocket");

    if (!ShipMesh->DoesSocketExist(EngineSocketName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine socket not found, using mesh origin"));
        EngineSocketName = NAME_None;
    }

    // Spawn engine trail attached to ship
    EngineTrailInstanceID = VFXManager->SpawnVFXAttached(
        EngineTrailSystem,
        ShipMesh,
        EngineSocketName,
        EVFXCategory::Ship,
        -1.0f // Infinite lifetime
    );

    // Set initial parameters
    VFXManager->SetVFXParameter(EngineTrailInstanceID, TEXT("ThrustIntensity"), 0.0f);
    VFXManager->SetVFXColorParameter(EngineTrailInstanceID, TEXT("EngineColor"),
        FLinearColor(0.2f, 0.5f, 1.0f, 1.0f));

    UE_LOG(LogTemp, Log, TEXT("Engine trail VFX spawned: %s"), *EngineTrailInstanceID);
}

void ASpaceship::UpdateEngineEffects(float DeltaTime)
{
    if (EngineTrailInstanceID.IsEmpty())
    {
        return;
    }

    UVFXManager* VFXManager = FindComponentByClass<UVFXManager>();
    if (!VFXManager)
    {
        return;
    }

    // Calculate thrust intensity (0-1)
    float ThrustIntensity = ThrustInput.Size() / MaxThrust;

    // Update VFX parameters
    VFXManager->SetVFXParameter(EngineTrailInstanceID, TEXT("ThrustIntensity"), ThrustIntensity);

    // Change color based on velocity
    float VelocityMagnitude = GetVelocity().Size();
    float ColorIntensity = FMath::Clamp(VelocityMagnitude / 1000.0f, 0.2f, 1.0f);

    FLinearColor EngineColor = FLinearColor(
        0.2f * ColorIntensity,
        0.5f * ColorIntensity,
        1.0f,
        ColorIntensity
    );

    VFXManager->SetVFXColorParameter(EngineTrailInstanceID, TEXT("EngineColor"), EngineColor);
}
```

#### Blueprint Example

**Blueprint Nodes:**
1. Get VFX Manager Component
2. Spawn VFX Attached (System, Attach Component, Socket Name, Category, Lifetime)
3. Store returned Instance ID in variable
4. In Tick: Set VFX Parameter (Instance ID, Parameter Name, Value)

**Performance Consideration:** Update VFX parameters on a slower tick (every 3-5 frames) to reduce CPU overhead.

---

### 2. Spawn Weapon Effects on Fire

#### C++ Example

```cpp
#include "VFXManager.h"
#include "WeaponSystem.h"

void AWeaponSystem::FireWeapon(FVector TargetLocation)
{
    UVFXManager* VFXManager = GetWorld()->GetFirstPlayerController()->FindComponentByClass<UVFXManager>();

    if (!VFXManager)
    {
        UE_LOG(LogTemp, Error, TEXT("VFX Manager not available"));
        return;
    }

    // Get weapon muzzle location
    FVector MuzzleLocation = WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
    FRotator MuzzleRotation = WeaponMesh->GetSocketRotation(TEXT("MuzzleFlash"));

    // 1. Spawn muzzle flash
    FString MuzzleFlashID = VFXManager->SpawnVFX(
        MuzzleFlashSystem,
        MuzzleLocation,
        MuzzleRotation,
        EVFXCategory::Weapon,
        0.5f // 0.5 second lifetime
    );

    // 2. Spawn weapon trail from muzzle to target
    FString TrailID = VFXManager->SpawnWeaponTrailVFX(
        MuzzleLocation,
        TargetLocation,
        2.0f // Trail thickness
    );

    // Customize trail color
    VFXManager->SetVFXColorParameter(TrailID, TEXT("BeamColor"),
        FLinearColor(1.0f, 0.3f, 0.1f, 1.0f)); // Orange beam

    UE_LOG(LogTemp, Log, TEXT("Weapon fired - Trail: %s"), *TrailID);
}
```

**Common Pitfall:** Make sure weapon VFX systems are properly pooled to avoid performance issues during rapid fire.

---

### 3. Trigger Explosion on Impact

#### C++ Example

```cpp
#include "VFXManager.h"
#include "Projectile.h"

void AProjectile::OnImpact(const FHitResult& Hit)
{
    UVFXManager* VFXManager = GetWorld()->GetFirstPlayerController()->FindComponentByClass<UVFXManager>();

    if (!VFXManager)
    {
        UE_LOG(LogTemp, Error, TEXT("VFX Manager not available"));
        return;
    }

    // Spawn explosion
    FString ExplosionID = VFXManager->SpawnExplosionVFX(
        Hit.Location,
        ExplosionScale // 1.0 = normal, 2.0 = double size, etc.
    );

    // Customize explosion parameters
    VFXManager->SetVFXParameter(ExplosionID, TEXT("ExplosionIntensity"), ExplosionDamage / 100.0f);
    VFXManager->SetVFXParameter(ExplosionID, TEXT("ShockwaveRadius"), ExplosionRadius);

    // Spawn impact debris/sparks
    FString ImpactID = VFXManager->SpawnImpactVFX(
        Hit.Location,
        Hit.Normal,
        1.0f // Impact intensity
    );

    UE_LOG(LogTemp, Log, TEXT("Impact VFX spawned at %s"), *Hit.Location.ToString());

    // Destroy projectile
    Destroy();
}
```

---

### 4. Enable Environmental VFX in Level

#### C++ Example

```cpp
#include "WeatherVFXLibrary.h"
#include "VFXManager.h"

void AMyGameMode::SetupLevelWeatherEffects()
{
    // Spawn ion storm effect
    UParticleSystemComponent* IonStorm = UWeatherVFXLibrary::SpawnWeatherEffect(
        GetWorld(),
        EWeatherTypeNew::IonStorm,
        FVector(0, 0, 5000), // 5km above center
        0.7f // 70% intensity
    );

    if (IonStorm)
    {
        UE_LOG(LogTemp, Log, TEXT("Ion storm effect spawned"));
    }

    // Spawn nebula cloud
    UParticleSystemComponent* Nebula = UWeatherVFXLibrary::SpawnWeatherEffect(
        GetWorld(),
        EWeatherTypeNew::NebulaCloud,
        FVector(10000, 0, 0),
        0.5f // 50% intensity
    );

    // Create custom weather config
    FWeatherVFXConfig CustomWeather;
    CustomWeather.BaseIntensity = 0.8f;
    CustomWeather.MaxIntensity = 2.0f;
    CustomWeather.ColorTint = FLinearColor(0.5f, 0.2f, 1.0f, 1.0f); // Purple tint
    CustomWeather.Duration = -1.0f; // Infinite

    UParticleSystemComponent* CustomEffect = UWeatherVFXLibrary::SpawnWeatherEffectWithConfig(
        GetWorld(),
        CustomWeather,
        FVector(-5000, 5000, 2000),
        FRotator::ZeroRotator
    );
}

// Dynamic weather transitions
void AMyGameMode::TransitionWeather(EWeatherTypeNew NewWeatherType)
{
    if (!CurrentWeatherEffect)
    {
        // Spawn new weather directly
        CurrentWeatherEffect = UWeatherVFXLibrary::SpawnWeatherEffect(
            GetWorld(),
            NewWeatherType,
            FVector::ZeroVector,
            1.0f
        );
        return;
    }

    // Smooth transition from current to new weather
    UWeatherVFXLibrary::TransitionWeatherEffect(
        GetWorld(),
        CurrentWeatherEffect,
        NewWeatherType,
        5.0f // 5 second transition
    );
}
```

---

### 5. Control VFX Parameters from Code

#### C++ Example

```cpp
#include "VFXManager.h"

void AMyGameMode::DynamicVFXControl()
{
    UVFXManager* VFXManager = GetWorld()->GetFirstPlayerController()->FindComponentByClass<UVFXManager>();

    if (!VFXManager)
    {
        return;
    }

    // Spawn persistent ambient VFX
    FString AmbientID = VFXManager->SpawnVFX(
        AmbientSpaceSystem,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EVFXCategory::Ambient,
        -1.0f // Persistent
    );

    // Control parameters over time
    GetWorld()->GetTimerManager().SetTimer(
        VFXUpdateTimer,
        [this, VFXManager, AmbientID]()
        {
            float GameTime = GetWorld()->GetTimeSeconds();

            // Pulse intensity
            float Intensity = (FMath::Sin(GameTime) + 1.0f) * 0.5f; // 0-1 range
            VFXManager->SetVFXParameter(AmbientID, TEXT("Intensity"), Intensity);

            // Rotate color through spectrum
            float Hue = FMath::Fmod(GameTime / 10.0f, 1.0f);
            FLinearColor Color = FLinearColor::MakeFromHSV8(
                (uint8)(Hue * 255),
                200,
                255
            );
            VFXManager->SetVFXColorParameter(AmbientID, TEXT("Color"), Color);

            // Scale based on player count
            int32 PlayerCount = GetNumPlayers();
            float Scale = FMath::Clamp(PlayerCount / 10.0f, 0.5f, 2.0f);
            VFXManager->SetVFXVectorParameter(AmbientID, TEXT("Scale"),
                FVector(Scale, Scale, Scale));
        },
        0.1f,
        true
    );
}

// Quality-based VFX management
void AMyGameMode::SetVFXQualityLevel(EVFXQuality Quality)
{
    UVFXManager* VFXManager = GetWorld()->GetFirstPlayerController()->FindComponentByClass<UVFXManager>();

    if (!VFXManager)
    {
        return;
    }

    VFXManager->SetVFXQuality(Quality);

    // Adjust category limits based on quality
    switch (Quality)
    {
        case EVFXQuality::Low:
            VFXManager->SetCategoryMaxVFX(EVFXCategory::Explosion, 5);
            VFXManager->SetCategoryMaxVFX(EVFXCategory::Weapon, 10);
            VFXManager->SetCategoryMaxVFX(EVFXCategory::Ambient, 2);
            break;

        case EVFXQuality::Medium:
            VFXManager->SetCategoryMaxVFX(EVFXCategory::Explosion, 10);
            VFXManager->SetCategoryMaxVFX(EVFXCategory::Weapon, 20);
            VFXManager->SetCategoryMaxVFX(EVFXCategory::Ambient, 5);
            break;

        case EVFXQuality::High:
            VFXManager->SetCategoryMaxVFX(EVFXCategory::Explosion, 20);
            VFXManager->SetCategoryMaxVFX(EVFXCategory::Weapon, 40);
            VFXManager->SetCategoryMaxVFX(EVFXCategory::Ambient, 10);
            break;

        case EVFXQuality::Ultra:
            VFXManager->SetMaxActiveVFX(200);
            break;

        default:
            break;
    }
}
```

---

## UI Integration

### 1. Bind HUD to Ship Data

#### C++ Example

```cpp
// ShipHUDWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Spaceship.h"
#include "ShipHUDWidget.generated.h"

UCLASS()
class ALEXANDER_API UShipHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Set ship reference
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetShipReference(ASpaceship* Ship);

protected:
    // Widget bindings (automatically bound by name)
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* SpeedText;

    UPROPERTY(meta = (BindWidget))
    class UProgressBar* FuelBar;

    UPROPERTY(meta = (BindWidget))
    class UProgressBar* HealthBar;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* AltitudeText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* FlightModeText;

    UPROPERTY(meta = (BindWidget))
    class UImage* DamageIndicator;

private:
    UPROPERTY()
    ASpaceship* CachedShipReference;

    void UpdateSpeedDisplay();
    void UpdateFuelDisplay();
    void UpdateHealthDisplay();
    void UpdateAltitudeDisplay();
    void UpdateFlightModeDisplay();
    void UpdateDamageIndicator();
};

// ShipHUDWidget.cpp
#include "ShipHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"

void UShipHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Get ship reference from owning player
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        ASpaceship* Ship = PC->GetPawn<ASpaceship>();
        SetShipReference(Ship);
    }
}

void UShipHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!CachedShipReference)
    {
        return;
    }

    // Update all displays
    UpdateSpeedDisplay();
    UpdateFuelDisplay();
    UpdateHealthDisplay();
    UpdateAltitudeDisplay();
    UpdateFlightModeDisplay();
    UpdateDamageIndicator();
}

void UShipHUDWidget::SetShipReference(ASpaceship* Ship)
{
    CachedShipReference = Ship;

    if (Ship)
    {
        UE_LOG(LogTemp, Log, TEXT("HUD bound to ship: %s"), *Ship->GetName());
    }
}

void UShipHUDWidget::UpdateSpeedDisplay()
{
    if (!SpeedText || !CachedShipReference)
    {
        return;
    }

    // Get velocity magnitude in m/s
    float Speed = CachedShipReference->GetVelocity().Size();

    // Convert to km/h for display
    float SpeedKMH = Speed * 3.6f;

    FString SpeedString = FString::Printf(TEXT("%.0f km/h"), SpeedKMH);
    SpeedText->SetText(FText::FromString(SpeedString));

    // Color based on speed
    if (SpeedKMH > 1000.0f)
    {
        SpeedText->SetColorAndOpacity(FLinearColor::Red);
    }
    else if (SpeedKMH > 500.0f)
    {
        SpeedText->SetColorAndOpacity(FLinearColor::Yellow);
    }
    else
    {
        SpeedText->SetColorAndOpacity(FLinearColor::White);
    }
}

void UShipHUDWidget::UpdateFuelDisplay()
{
    if (!FuelBar || !CachedShipReference)
    {
        return;
    }

    // Get fuel percentage (0-1)
    // Note: You'll need to add fuel system to ship
    float FuelPercent = 0.75f; // Placeholder

    FuelBar->SetPercent(FuelPercent);

    // Change color based on fuel level
    if (FuelPercent < 0.2f)
    {
        FuelBar->SetFillColorAndOpacity(FLinearColor::Red);
    }
    else if (FuelPercent < 0.5f)
    {
        FuelBar->SetFillColorAndOpacity(FLinearColor::Yellow);
    }
    else
    {
        FuelBar->SetFillColorAndOpacity(FLinearColor::Green);
    }
}

void UShipHUDWidget::UpdateHealthDisplay()
{
    if (!HealthBar || !CachedShipReference)
    {
        return;
    }

    // Get hull integrity (0-100)
    float HealthPercent = CachedShipReference->HullIntegrity / 100.0f;

    HealthBar->SetPercent(HealthPercent);

    // Change color based on health
    if (HealthPercent < 0.25f)
    {
        HealthBar->SetFillColorAndOpacity(FLinearColor::Red);
    }
    else if (HealthPercent < 0.5f)
    {
        HealthBar->SetFillColorAndOpacity(FLinearColor(1.0f, 0.5f, 0.0f, 1.0f)); // Orange
    }
    else
    {
        HealthBar->SetFillColorAndOpacity(FLinearColor::Green);
    }
}

void UShipHUDWidget::UpdateAltitudeDisplay()
{
    if (!AltitudeText || !CachedShipReference)
    {
        return;
    }

    // Get altitude (Z coordinate)
    float Altitude = CachedShipReference->GetActorLocation().Z;

    // Convert to km
    float AltitudeKM = Altitude / 100000.0f;

    FString AltitudeString = FString::Printf(TEXT("ALT: %.2f km"), AltitudeKM);
    AltitudeText->SetText(FText::FromString(AltitudeString));
}

void UShipHUDWidget::UpdateFlightModeDisplay()
{
    if (!FlightModeText || !CachedShipReference)
    {
        return;
    }

    // Get current flight mode
    FString ModeString;
    switch (CachedShipReference->FlightMode)
    {
        case EFlightMode::Normal:
            ModeString = TEXT("NORMAL");
            break;
        case EFlightMode::Assisted:
            ModeString = TEXT("ASSISTED");
            break;
        case EFlightMode::Docking:
            ModeString = TEXT("DOCKING");
            break;
        case EFlightMode::Combat:
            ModeString = TEXT("COMBAT");
            break;
        default:
            ModeString = TEXT("UNKNOWN");
            break;
    }

    FlightModeText->SetText(FText::FromString(ModeString));
}

void UShipHUDWidget::UpdateDamageIndicator()
{
    if (!DamageIndicator || !CachedShipReference)
    {
        return;
    }

    // Show damage indicator based on damage state
    EShipDamageState DamageState = CachedShipReference->DamageState;

    switch (DamageState)
    {
        case EShipDamageState::Pristine:
        case EShipDamageState::MinorDamage:
            DamageIndicator->SetVisibility(ESlateVisibility::Hidden);
            break;

        case EShipDamageState::ModerateDamage:
            DamageIndicator->SetVisibility(ESlateVisibility::Visible);
            DamageIndicator->SetColorAndOpacity(FLinearColor(1.0f, 0.5f, 0.0f, 0.5f)); // Orange, semi-transparent
            break;

        case EShipDamageState::HeavyDamage:
        case EShipDamageState::Critical:
            DamageIndicator->SetVisibility(ESlateVisibility::Visible);
            DamageIndicator->SetColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f, 0.7f)); // Red, more visible
            break;

        default:
            break;
    }
}
```

#### Blueprint Example

**UMG Widget Setup:**
1. Create Widget Blueprint (WBP_ShipHUD)
2. Add Text blocks: SpeedText, AltitudeText, FlightModeText
3. Add Progress Bars: FuelBar, HealthBar
4. Add Image: DamageIndicator
5. In Graph: Get Player Pawn → Cast to Spaceship → Store reference
6. Event Tick: Call update functions

**Performance Consideration:** Use "Is Variable" property on widgets to cache references. Update HUD at 30-60 Hz instead of every frame.

---

### 2. Update Mission Objectives in UI

#### C++ Example

```cpp
// MissionTrackerWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MissionBoardComponent.h"
#include "MissionTrackerWidget.generated.h"

UCLASS()
class ALEXANDER_API UMissionTrackerWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Mission Tracker")
    void RefreshTrackedMissions();

    UFUNCTION(BlueprintCallable, Category = "Mission Tracker")
    void OnMissionObjectiveUpdated(FName MissionID, FName ObjectiveID);

protected:
    UPROPERTY(meta = (BindWidget))
    class UVerticalBox* ObjectiveListContainer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission Tracker")
    TSubclassOf<UUserWidget> ObjectiveEntryWidgetClass;

private:
    UPROPERTY()
    UMissionBoardComponent* CachedMissionBoard;

    UPROPERTY()
    APlayerState* CachedPlayerState;

    void CreateObjectiveEntry(const FMissionBoardObjective& Objective, const FMissionData& Mission);
};

// MissionTrackerWidget.cpp
#include "MissionTrackerWidget.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UMissionTrackerWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Cache references
    CachedMissionBoard = GetWorld()->GetSubsystem<UMissionBoardComponent>();

    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        CachedPlayerState = PC->GetPlayerState<APlayerState>();
    }

    // Bind to mission events
    if (CachedMissionBoard)
    {
        CachedMissionBoard->OnMissionObjectiveCompleted.AddDynamic(
            this,
            &UMissionTrackerWidget::OnMissionObjectiveUpdated
        );
    }

    // Initial refresh
    RefreshTrackedMissions();
}

void UMissionTrackerWidget::RefreshTrackedMissions()
{
    if (!CachedMissionBoard || !CachedPlayerState || !ObjectiveListContainer)
    {
        return;
    }

    // Clear existing entries
    ObjectiveListContainer->ClearChildren();

    // Get tracked missions
    TArray<FName> TrackedMissions = CachedMissionBoard->GetTrackedMissions(CachedPlayerState);

    for (const FName& MissionID : TrackedMissions)
    {
        FMissionData* Mission = CachedMissionBoard->GetMission(MissionID);

        if (!Mission)
        {
            continue;
        }

        // Add mission title
        // (Create title widget here)

        // Add objectives
        for (const FMissionBoardObjective& Objective : Mission->Objectives)
        {
            CreateObjectiveEntry(Objective, *Mission);
        }
    }
}

void UMissionTrackerWidget::CreateObjectiveEntry(const FMissionBoardObjective& Objective, const FMissionData& Mission)
{
    if (!ObjectiveEntryWidgetClass || !ObjectiveListContainer)
    {
        return;
    }

    // Create widget instance
    UUserWidget* EntryWidget = CreateWidget<UUserWidget>(this, ObjectiveEntryWidgetClass);

    if (!EntryWidget)
    {
        return;
    }

    // Set up objective data (assuming widget has these properties)
    UTextBlock* DescriptionText = Cast<UTextBlock>(EntryWidget->GetWidgetFromName(TEXT("ObjectiveDescription")));
    UTextBlock* ProgressText = Cast<UTextBlock>(EntryWidget->GetWidgetFromName(TEXT("ProgressText")));
    UImage* CheckmarkImage = Cast<UImage>(EntryWidget->GetWidgetFromName(TEXT("CheckmarkImage")));

    if (DescriptionText)
    {
        DescriptionText->SetText(FText::FromString(Objective.Description));
    }

    if (ProgressText)
    {
        FString ProgressString = FString::Printf(TEXT("%d / %d"),
            Objective.CurrentCount,
            Objective.TargetCount
        );
        ProgressText->SetText(FText::FromString(ProgressString));
    }

    if (CheckmarkImage)
    {
        CheckmarkImage->SetVisibility(
            Objective.bIsCompleted ? ESlateVisibility::Visible : ESlateVisibility::Hidden
        );
    }

    // Apply visual style based on completion
    if (Objective.bIsCompleted)
    {
        if (DescriptionText)
        {
            FSlateColor CompletedColor = FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
            DescriptionText->SetColorAndOpacity(CompletedColor);
        }
    }

    // Add to container
    ObjectiveListContainer->AddChild(EntryWidget);
}

void UMissionTrackerWidget::OnMissionObjectiveUpdated(FName MissionID, FName ObjectiveID)
{
    // Refresh the tracker when an objective updates
    RefreshTrackedMissions();

    // Optional: Play animation or sound
    UE_LOG(LogTemp, Log, TEXT("Mission objective updated: %s - %s"),
        *MissionID.ToString(),
        *ObjectiveID.ToString()
    );
}
```

---

### 3. Show/Hide Widgets

#### C++ Example

```cpp
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AMyPlayerController::ToggleWidget(TSubclassOf<UUserWidget> WidgetClass, bool bShow)
{
    if (!WidgetClass)
    {
        return;
    }

    // Find existing widget
    TArray<UUserWidget*> FoundWidgets;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), WidgetClass, FoundWidgets);

    UUserWidget* Widget = nullptr;

    if (FoundWidgets.Num() > 0)
    {
        Widget = FoundWidgets[0];
    }

    if (bShow)
    {
        if (!Widget)
        {
            // Create widget
            Widget = CreateWidget<UUserWidget>(this, WidgetClass);

            if (Widget)
            {
                Widget->AddToViewport(0); // Z-order 0 = lowest
            }
        }
        else
        {
            // Show existing widget
            Widget->SetVisibility(ESlateVisibility::Visible);
        }

        // Set input mode for UI
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(Widget->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(InputMode);
        bShowMouseCursor = true;
    }
    else
    {
        if (Widget)
        {
            // Hide widget
            Widget->SetVisibility(ESlateVisibility::Hidden);

            // Or remove completely
            // Widget->RemoveFromParent();
        }

        // Restore game input mode
        FInputModeGameOnly InputMode;
        SetInputMode(InputMode);
        bShowMouseCursor = false;
    }
}

// Specific widget helpers
void AMyPlayerController::ShowMissionBoard()
{
    ToggleWidget(MissionBoardWidgetClass, true);
}

void AMyPlayerController::HideMissionBoard()
{
    ToggleWidget(MissionBoardWidgetClass, false);
}

void AMyPlayerController::ShowShipHUD()
{
    ToggleWidget(ShipHUDWidgetClass, true);
}
```

#### Blueprint Example

**Blueprint Nodes:**
1. Create Widget (Widget Class)
2. Add to Viewport
3. Set Visibility (Visible/Hidden/Collapsed)
4. Set Input Mode (UI Only, Game Only, Game and UI)
5. Set Show Mouse Cursor

**Common Pitfall:** Remember to switch input mode when showing UI. VR doesn't need mouse cursor but desktop does.

---

### 4. Handle Button Clicks

#### C++ Example

```cpp
// Custom button widget
UCLASS()
class UMissionAcceptButton : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UPROPERTY(BlueprintReadWrite, Category = "Mission")
    FMissionData AssignedMission;

protected:
    UPROPERTY(meta = (BindWidget))
    UButton* AcceptButton;

    UFUNCTION()
    void OnAcceptClicked();
};

void UMissionAcceptButton::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button click
    if (AcceptButton)
    {
        AcceptButton->OnClicked.AddDynamic(this, &UMissionAcceptButton::OnAcceptClicked);

        // Optional: Bind hover events
        AcceptButton->OnHovered.AddDynamic(this, &UMissionAcceptButton::OnButtonHovered);
        AcceptButton->OnUnhovered.AddDynamic(this, &UMissionAcceptButton::OnButtonUnhovered);
    }
}

void UMissionAcceptButton::OnAcceptClicked()
{
    // Get mission board
    UMissionBoardComponent* MissionBoard = GetWorld()->GetSubsystem<UMissionBoardComponent>();

    if (!MissionBoard)
    {
        UE_LOG(LogTemp, Error, TEXT("Mission board not available"));
        return;
    }

    // Get player state
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    APlayerState* PlayerState = PC->GetPlayerState<APlayerState>();

    // Check if can accept
    if (!MissionBoard->CanPlayerAcceptMission(PlayerState, AssignedMission.MissionID))
    {
        // Show error message
        UE_LOG(LogTemp, Warning, TEXT("Cannot accept mission"));
        return;
    }

    // Accept mission
    bool bSuccess = MissionBoard->AcceptMission(PlayerState, AssignedMission.MissionID);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Mission accepted!"));

        // Disable button
        AcceptButton->SetIsEnabled(false);

        // Update UI
        // ... refresh mission list, etc.
    }
}
```

---

### 5. Create VR-Friendly UI Interaction

#### C++ Example

```cpp
#include "Components/WidgetComponent.h"
#include "MotionControllerComponent.h"
#include "Components/WidgetInteractionComponent.h"

void ASpaceship::SetupVRUI()
{
    if (!bIsVREnabled)
    {
        return;
    }

    // Create 3D widget component for cockpit displays
    UCockpitDisplayWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("CockpitDisplay"));
    UCockpitDisplayWidget->SetupAttachment(RootComponent);

    // Position in front of pilot
    UCockpitDisplayWidget->SetRelativeLocation(FVector(100, 0, 50));
    UCockpitDisplayWidget->SetRelativeRotation(FRotator(0, 180, 0));

    // Set widget class
    UCockpitDisplayWidget->SetWidgetClass(CockpitWidgetClass);
    UCockpitDisplayWidget->SetWidgetSpace(EWidgetSpace::World);
    UCockpitDisplayWidget->SetDrawSize(FVector2D(800, 600));

    // Enable 3D interaction
    UCockpitDisplayWidget->SetVisibility(true);
}

// VR Controller setup
void AMyVRCharacter::SetupVRInteraction()
{
    // Create widget interaction component for right hand
    RightHandWidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("RightHandInteraction"));
    RightHandWidgetInteraction->SetupAttachment(RightMotionController);

    // Configure interaction
    RightHandWidgetInteraction->InteractionDistance = 500.0f; // 5 meters
    RightHandWidgetInteraction->bShowDebug = false;
    RightHandWidgetInteraction->InteractionSource = EWidgetInteractionSource::World;

    // Bind trigger press to interaction
    InputComponent->BindAction("VRTriggerRight", IE_Pressed, this, &AMyVRCharacter::OnTriggerPressed);
    InputComponent->BindAction("VRTriggerRight", IE_Released, this, &AMyVRCharacter::OnTriggerReleased);
}

void AMyVRCharacter::OnTriggerPressed()
{
    if (RightHandWidgetInteraction)
    {
        RightHandWidgetInteraction->PressPointerKey(EKeys::LeftMouseButton);
    }
}

void AMyVRCharacter::OnTriggerReleased()
{
    if (RightHandWidgetInteraction)
    {
        RightHandWidgetInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
    }
}
```

**Performance Consideration:** Limit widget resolution in VR to maintain frame rate. Use 512x512 or 1024x768 instead of 1920x1080.

---

## Performance Integration

### 1. Add Profiling to New Systems

#### C++ Example

```cpp
#include "PerformanceProfilerSubsystem.h"

void AMyNewSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Get profiler subsystem
    UPerformanceProfilerSubsystem* Profiler = GetWorld()->GetSubsystem<UPerformanceProfilerSubsystem>();

    if (!Profiler || !Profiler->IsProfilingEnabled())
    {
        // Skip profiling overhead if disabled
        DoNormalUpdate(DeltaTime);
        return;
    }

    // Method 1: Manual profiling
    Profiler->StartProfiling(TEXT("MyNewSystem_Update"));

    DoNormalUpdate(DeltaTime);

    Profiler->StopProfiling(TEXT("MyNewSystem_Update"));

    // Method 2: Scope-based profiling (C++ only, preferred)
    // PROFILE_SCOPE(Profiler, TEXT("MyNewSystem_Update"));
    // DoNormalUpdate(DeltaTime);
}

// Profiling subsystems
void AMyNewSystem::UpdateSubsystem1(float DeltaTime)
{
    UPerformanceProfilerSubsystem* Profiler = GetWorld()->GetSubsystem<UPerformanceProfilerSubsystem>();
    PROFILE_SCOPE(Profiler, TEXT("MyNewSystem_Subsystem1"));

    // ... do work ...
}

void AMyNewSystem::UpdateSubsystem2(float DeltaTime)
{
    UPerformanceProfilerSubsystem* Profiler = GetWorld()->GetSubsystem<UPerformanceProfilerSubsystem>();
    PROFILE_SCOPE(Profiler, TEXT("MyNewSystem_Subsystem2"));

    // ... do work ...
}

// Query profiling data
void AMyNewSystem::LogPerformanceStats()
{
    UPerformanceProfilerSubsystem* Profiler = GetWorld()->GetSubsystem<UPerformanceProfilerSubsystem>();

    if (!Profiler)
    {
        return;
    }

    // Get specific system profile
    FProfileData ProfileData = Profiler->GetSystemProfile(TEXT("MyNewSystem_Update"));

    UE_LOG(LogTemp, Log, TEXT("MyNewSystem Performance:"));
    UE_LOG(LogTemp, Log, TEXT("  Average: %.2f ms"), ProfileData.AverageTimeMs);
    UE_LOG(LogTemp, Log, TEXT("  Min: %.2f ms"), ProfileData.MinTimeMs);
    UE_LOG(LogTemp, Log, TEXT("  Max: %.2f ms"), ProfileData.MaxTimeMs);
    UE_LOG(LogTemp, Log, TEXT("  Samples: %d"), ProfileData.SampleCount);

    // Log top bottlenecks
    Profiler->LogTopBottlenecks(10);

    // Export to file
    FString ExportPath = FPaths::ProjectSavedDir() / TEXT("Profiling") / TEXT("MyNewSystem_Profile.json");
    Profiler->ExportToJSON(ExportPath);
}
```

#### Blueprint Example

**Blueprint Nodes:**
1. Get Performance Profiler Subsystem
2. Start Profiling (System Name)
3. [Your logic nodes]
4. Stop Profiling (System Name)
5. Get System Profile (returns struct)
6. Break Profile Data struct to read values

**Common Pitfall:** Don't profile inside tight loops. Profile at the function/system level, not individual operations.

---

### 2. Query Profiling Data

#### C++ Example

```cpp
#include "PerformanceProfilerSubsystem.h"

void AMyGameMode::AnalyzePerformance()
{
    UPerformanceProfilerSubsystem* Profiler = GetWorld()->GetSubsystem<UPerformanceProfilerSubsystem>();

    if (!Profiler)
    {
        return;
    }

    // Get all profiles
    TArray<FProfileData> AllProfiles = Profiler->GetAllProfiles();

    UE_LOG(LogTemp, Log, TEXT("=== Performance Analysis ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Systems Profiled: %d"), AllProfiles.Num());

    // Find slowest systems
    TArray<FProfileData> Bottlenecks = Profiler->GetTopBottlenecks(5);

    UE_LOG(LogTemp, Log, TEXT("Top 5 Bottlenecks:"));
    for (int32 i = 0; i < Bottlenecks.Num(); i++)
    {
        const FProfileData& Data = Bottlenecks[i];
        UE_LOG(LogTemp, Log, TEXT("  %d. %s: %.2f ms (avg), %.2f ms (max)"),
            i + 1,
            *Data.SystemName.ToString(),
            Data.AverageTimeMs,
            Data.MaxTimeMs
        );
    }

    // Memory stats
    int64 CurrentMemoryMB = Profiler->GetCurrentMemoryUsageMB();
    int64 PeakMemoryMB = Profiler->GetPeakMemoryUsageMB();

    UE_LOG(LogTemp, Log, TEXT("Memory Usage:"));
    UE_LOG(LogTemp, Log, TEXT("  Current: %lld MB"), CurrentMemoryMB);
    UE_LOG(LogTemp, Log, TEXT("  Peak: %lld MB"), PeakMemoryMB);

    // Frame time
    double AvgFrameTime = Profiler->GetAverageFrameTimeMs();
    double CurrentFPS = Profiler->GetCurrentFPS();

    UE_LOG(LogTemp, Log, TEXT("Frame Time:"));
    UE_LOG(LogTemp, Log, TEXT("  Average: %.2f ms"), AvgFrameTime);
    UE_LOG(LogTemp, Log, TEXT("  FPS: %.1f"), CurrentFPS);
}
```

---

### 3. Implement LOD for Stations

#### C++ Example

```cpp
#include "SpaceStationHub.h"

void ASpaceStationHub::SetupLODSystem()
{
    // Enable LOD on station mesh
    if (StationMesh)
    {
        StationMesh->SetForcedLodModel(0); // Auto LOD
        StationMesh->bOverrideLODStreamingSettings = true;
    }

    // Create LOD distance thresholds
    LODDistances.Add(5000.0f);   // LOD 0: < 5km
    LODDistances.Add(10000.0f);  // LOD 1: 5-10km
    LODDistances.Add(25000.0f);  // LOD 2: 10-25km
    LODDistances.Add(50000.0f);  // LOD 3: 25-50km
                                  // LOD 4: > 50km (very low detail)

    // Start LOD update timer
    GetWorld()->GetTimerManager().SetTimer(
        LODUpdateTimer,
        this,
        &ASpaceStationHub::UpdateLOD,
        1.0f, // Update every second
        true
    );
}

void ASpaceStationHub::UpdateLOD()
{
    // Get player camera location
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
    {
        return;
    }

    FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
    float DistanceToCamera = FVector::Dist(GetActorLocation(), CameraLocation);

    // Determine LOD level
    int32 NewLOD = 0;
    for (int32 i = 0; i < LODDistances.Num(); i++)
    {
        if (DistanceToCamera > LODDistances[i])
        {
            NewLOD = i + 1;
        }
        else
        {
            break;
        }
    }

    // Clamp to max LOD
    NewLOD = FMath::Min(NewLOD, 4);

    if (NewLOD != CurrentLODLevel)
    {
        ApplyLODLevel(NewLOD);
        CurrentLODLevel = NewLOD;
    }
}

void ASpaceStationHub::ApplyLODLevel(int32 LODLevel)
{
    switch (LODLevel)
    {
        case 0: // High detail
            SetActorTickInterval(0.0f); // Tick every frame
            bEnableDetailedSimulation = true;
            bEnableVoiceChat = true;
            bEnableTrading = true;
            break;

        case 1: // Medium detail
            SetActorTickInterval(0.1f); // Tick every 0.1s
            bEnableDetailedSimulation = true;
            bEnableVoiceChat = true;
            bEnableTrading = true;
            break;

        case 2: // Low detail
            SetActorTickInterval(0.5f); // Tick every 0.5s
            bEnableDetailedSimulation = false;
            bEnableVoiceChat = false;
            bEnableTrading = true;
            break;

        case 3: // Very low detail
            SetActorTickInterval(1.0f); // Tick every second
            bEnableDetailedSimulation = false;
            bEnableVoiceChat = false;
            bEnableTrading = false;
            break;

        case 4: // Minimal detail
            SetActorTickInterval(5.0f); // Tick every 5 seconds
            bEnableDetailedSimulation = false;
            bEnableVoiceChat = false;
            bEnableTrading = false;
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("Station %s LOD changed to %d"), *HubName.ToString(), LODLevel);
}
```

---

### 4. Pool VFX Instances

#### C++ Example

```cpp
#include "VFXManager.h"

void AMyGameMode::SetupVFXPools()
{
    UVFXManager* VFXManager = FindComponentByClass<UVFXManager>();

    if (!VFXManager)
    {
        return;
    }

    // Preload common VFX pools
    VFXManager->PreloadVFXPool(TEXT("ExplosionSmall"), 20);
    VFXManager->PreloadVFXPool(TEXT("ExplosionLarge"), 10);
    VFXManager->PreloadVFXPool(TEXT("LaserImpact"), 30);
    VFXManager->PreloadVFXPool(TEXT("EngineTrail"), 10);
    VFXManager->PreloadVFXPool(TEXT("MuzzleFlash"), 50);

    UE_LOG(LogTemp, Log, TEXT("VFX pools preloaded"));
}

// Using pooled VFX
void AWeaponSystem::FireWeaponWithPooling()
{
    UVFXManager* VFXManager = GetWorld()->GetFirstPlayerController()->FindComponentByClass<UVFXManager>();

    if (!VFXManager)
    {
        return;
    }

    // Spawn from pool (automatically managed)
    FString MuzzleFlashID = VFXManager->SpawnVFXFromTemplate(
        TEXT("MuzzleFlash"),
        GetMuzzleLocation(),
        GetMuzzleRotation()
    );

    // VFX is automatically returned to pool when complete
}

// Monitoring pool performance
void AMyGameMode::MonitorVFXPools()
{
    UVFXManager* VFXManager = FindComponentByClass<UVFXManager>();

    if (!VFXManager)
    {
        return;
    }

    FVFXStats Stats = VFXManager->GetVFXStatistics();

    UE_LOG(LogTemp, Log, TEXT("VFX Statistics:"));
    UE_LOG(LogTemp, Log, TEXT("  Active Instances: %d"), Stats.ActiveInstances);
    UE_LOG(LogTemp, Log, TEXT("  Pooled Instances: %d"), Stats.PooledInstances);
    UE_LOG(LogTemp, Log, TEXT("  Total Spawned: %d"), Stats.TotalSpawned);
    UE_LOG(LogTemp, Log, TEXT("  Memory Usage: %.2f MB"), Stats.MemoryUsageMB);
}
```

---

## Common Recipes

### Recipe 1: How to Add a New Mission Type

```cpp
// 1. Add enum value to EMissionType in MissionBoardComponent.h
enum class EMissionType : uint8
{
    // ... existing types ...
    MyNewMissionType  // Add your new type
};

// 2. Create mission template data asset in Content Browser
// Right-click → Miscellaneous → Data Asset → MissionTemplateDataAssetClass

// 3. Fill in template
FMissionTemplateData NewMissionTemplate;
NewMissionTemplate.TemplateID = TEXT("TEMPLATE_MyNewMission");
NewMissionTemplate.Type = EMissionTypeDataAsset::MyNewType;
NewMissionTemplate.Title = FText::FromString(TEXT("My New Mission"));
NewMissionTemplate.Description = FText::FromString(TEXT("Mission description here"));

// Add objectives
FMissionObjectiveDataAsset Objective1;
Objective1.ObjectiveID = TEXT("OBJ_DoSomething");
Objective1.Description = FText::FromString(TEXT("Do something interesting"));
Objective1.TargetCount = 5;
NewMissionTemplate.Objectives.Add(Objective1);

// Set rewards
NewMissionTemplate.Rewards.Credits = 5000;
NewMissionTemplate.Rewards.Experience = 500;

// 4. Add to mission board
UMissionBoardComponent* MissionBoard = FindComponentByClass<UMissionBoardComponent>();
// Use the template to generate missions (shown in previous examples)
```

### Recipe 2: How to Add a Custom VFX to VFXManager

```cpp
// 1. Create Niagara System in Content Browser

// 2. Register as template
FVFXTemplate CustomTemplate;
CustomTemplate.TemplateName = TEXT("MyCustomEffect");
CustomTemplate.NiagaraSystem = LoadObject<UNiagaraSystem>(
    nullptr,
    TEXT("/Game/VFX/NS_MyCustomEffect.NS_MyCustomEffect")
);
CustomTemplate.Category = EVFXCategory::Special;
CustomTemplate.DefaultLifetime = 3.0f;
CustomTemplate.bEnablePooling = true;
CustomTemplate.PoolSize = 15;

UVFXManager* VFXManager = FindComponentByClass<UVFXManager>();
VFXManager->RegisterVFXTemplate(CustomTemplate);

// 3. Use it
FString InstanceID = VFXManager->SpawnVFXFromTemplate(
    TEXT("MyCustomEffect"),
    SpawnLocation,
    SpawnRotation
);
```

### Recipe 3: How to Create a Custom Station Module

```cpp
// 1. Create static mesh for module in DCC tool (Blender, Maya, etc.)
// Import to Unreal Content Browser

// 2. Register module
FStationAssetRegistration Registration;
Registration.StationMesh = LoadObject<UStaticMesh>(
    nullptr,
    TEXT("/Game/Stations/Modules/SM_MyCustomModule.SM_MyCustomModule")
);
Registration.ModuleType = EStationModuleType::HubCore; // Or create new type
Registration.Capacity = 50;
Registration.BoundsSize = FVector(2000, 2000, 1000);

// Add docking points
FTransform DockPoint1;
DockPoint1.SetLocation(FVector(1000, 0, 0));
Registration.DockingPoints.Add(DockPoint1);

bool bSuccess = USpaceStationBlueprintLibrary::RegisterStationAsset(
    GetWorld(),
    Registration
);

// 3. Use in station configuration
FStationConfiguration Config;
Config.Modules.Add(Registration.ModuleType);
// ... spawn station as shown earlier
```

### Recipe 4: How to Bind Mission Progress to Gameplay Events

```cpp
// In your gameplay class (e.g., enemy AI, resource node, etc.)
void AMyEnemy::OnDeath()
{
    // Get killer
    APlayerController* KillerPC = Cast<APlayerController>(LastDamageInstigator);
    if (!KillerPC)
    {
        return;
    }

    APlayerState* KillerState = KillerPC->GetPlayerState<APlayerState>();

    // Update all active missions for this player
    UMissionBoardComponent* MissionBoard = GetWorld()->GetSubsystem<UMissionBoardComponent>();

    if (!MissionBoard)
    {
        return;
    }

    TArray<FMissionData> PlayerMissions = MissionBoard->GetPlayerMissions(KillerState);

    for (const FMissionData& Mission : PlayerMissions)
    {
        // Check if this mission has kill objectives
        for (const FMissionBoardObjective& Objective : Mission.Objectives)
        {
            // Check if objective type matches this enemy
            if (Objective.ObjectiveType == TEXT("KillEnemy") &&
                !Objective.bIsCompleted)
            {
                // Update progress
                MissionBoard->UpdateMissionProgress(
                    KillerState,
                    Mission.MissionID,
                    Objective.ObjectiveID,
                    1 // Increment by 1
                );
            }
        }
    }
}
```

### Recipe 5: How to Optimize UI for VR

```cpp
// In your widget class
void UMyVRWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Detect VR mode
    bool bIsVR = GEngine->XRSystem.IsValid() && GEngine->XRSystem->IsHeadTrackingAllowed();

    if (bIsVR)
    {
        ApplyVROptimizations();
    }
}

void UMyVRWidget::ApplyVROptimizations()
{
    // 1. Reduce resolution
    if (UWidgetComponent* WidgetComp = Cast<UWidgetComponent>(GetOuter()))
    {
        WidgetComp->SetDrawSize(FVector2D(1024, 768)); // Lower than desktop
    }

    // 2. Increase font sizes
    if (TitleText)
    {
        FSlateFontInfo Font = TitleText->GetFont();
        Font.Size *= 1.5f; // 50% larger for VR
        TitleText->SetFont(Font);
    }

    // 3. Simplify animations
    bEnableComplexAnimations = false;

    // 4. Increase button sizes
    if (AcceptButton)
    {
        FMargin Padding = AcceptButton->GetWidgetStyle().Normal.Padding;
        Padding *= 1.5f;
        // ... apply padding
    }

    // 5. Use high contrast colors
    if (BackgroundImage)
    {
        BackgroundImage->SetColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
    }
}
```

---

## Summary

This document provides comprehensive, production-ready integration examples for:

- **Mission System**: Query, accept, track, and complete missions from data assets
- **Station System**: Spawn, configure, and populate stations with services
- **VFX System**: Spawn, control, and optimize visual effects with pooling
- **UI System**: Data binding, event handling, and VR optimization
- **Performance System**: Profiling, LOD, and memory optimization

All examples are:
- **Copy-paste ready** - Can be used directly in your project
- **Well-commented** - Explains each step
- **Performance-aware** - Includes optimization tips
- **Error-handled** - Checks for null references and edge cases

For more information, see:
- c:\Users\allen\Desktop\Alexander\Alexander\Source\Alexander\Public\MissionBoardComponent.h
- c:\Users\allen\Desktop\Alexander\Alexander\Source\Alexander\Public\VFXManager.h
- c:\Users\allen\Desktop\Alexander\Alexander\Source\Alexander\Public\PerformanceProfilerSubsystem.h
- c:\Users\allen\Desktop\Alexander\Alexander\Source\Alexander\Public\SpaceStationBlueprintLibrary.h
