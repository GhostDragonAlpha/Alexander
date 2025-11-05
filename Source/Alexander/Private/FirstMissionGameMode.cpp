// Copyright Epic Games, Inc. All Rights Reserved.

#include "FirstMissionGameMode.h"
#include "Spaceship.h"
#include "Planet.h"
#include "TutorialSystem.h"
#include "QuestSystem.h"
#include "AudioManager.h"
#include "VFXManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AFirstMissionGameMode::AFirstMissionGameMode()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default configuration
    MoonDistance = 50000.0f; // 50km scaled distance (realistic would be 384,400 km)
    LandingAltitudeThreshold = 100.0f; // meters
    LandingVelocityThreshold = 5.0f; // m/s
    ExplorationDistanceRequired = 100.0f; // meters
    bUseRealisticDistances = false;
    TimeCompressionFactor = 1.0f;

    // Initialize tracking
    TotalDistanceTraveled = 0.0f;
    CurrentObjectiveIndex = 0;
}

void AFirstMissionGameMode::BeginPlay()
{
    Super::BeginPlay();

    InitializeMission();
}

void AFirstMissionGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (MissionState.CurrentStage != EMissionStage::NotStarted &&
        MissionState.CurrentStage != EMissionStage::MissionComplete &&
        MissionState.CurrentStage != EMissionStage::MissionFailed)
    {
        UpdateMissionProgress(DeltaTime);
    }
}

void AFirstMissionGameMode::InitializeMission()
{
    // Find or spawn systems
    TArray<AActor*> FoundActors;
    
    // Find Tutorial System
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATutorialSystem::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        TutorialSystem = Cast<ATutorialSystem>(FoundActors[0]);
    }
    else
    {
        TutorialSystem = GetWorld()->SpawnActor<ATutorialSystem>();
    }

    // Find Quest System
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AQuestSystem::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        QuestSystem = Cast<AQuestSystem>(FoundActors[0]);
    }

    // Find Audio Manager
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAudioManager::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        AudioManager = Cast<AAudioManager>(FoundActors[0]);
    }

    // Find VFX Manager
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVFXManager::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        VFXManager = Cast<AVFXManager>(FoundActors[0]);
    }

    // Setup objectives
    SetupObjectives();

    UE_LOG(LogTemp, Log, TEXT("FirstMissionGameMode: Mission initialized"));
}

void AFirstMissionGameMode::SetupObjectives()
{
    // Objective 1: Walk to ship
    FMissionObjective Obj1;
    Obj1.ObjectiveID = "obj_walk_to_ship";
    Obj1.ObjectiveTitle = FText::FromString("Walk to Your Ship");
    Obj1.ObjectiveDescription = FText::FromString("Navigate to the landing zone and approach your spacecraft.");
    Obj1.RequiredStage = EMissionStage::PlanetSurface;
    Obj1.CompletionRadius = 5.0f;
    Obj1.ExperienceReward = 50;
    RegisterObjective(Obj1);

    // Objective 2: Enter ship
    FMissionObjective Obj2;
    Obj2.ObjectiveID = "obj_enter_ship";
    Obj2.ObjectiveTitle = FText::FromString("Enter Your Ship");
    Obj2.ObjectiveDescription = FText::FromString("Board your spacecraft and familiarize yourself with the controls.");
    Obj2.RequiredStage = EMissionStage::EnteringShip;
    Obj2.ExperienceReward = 100;
    RegisterObjective(Obj2);

    // Objective 3: Launch from planet
    FMissionObjective Obj3;
    Obj3.ObjectiveID = "obj_launch";
    Obj3.ObjectiveTitle = FText::FromString("Launch to Space");
    Obj3.ObjectiveDescription = FText::FromString("Take off and reach orbit altitude.");
    Obj3.RequiredStage = EMissionStage::Launch;
    Obj3.ExperienceReward = 200;
    RegisterObjective(Obj3);

    // Objective 4: Navigate to moon
    FMissionObjective Obj4;
    Obj4.ObjectiveID = "obj_navigate_moon";
    Obj4.ObjectiveTitle = FText::FromString("Navigate to the Moon");
    Obj4.ObjectiveDescription = FText::FromString("Fly to the moon and prepare for landing.");
    Obj4.RequiredStage = EMissionStage::SpaceFlight;
    Obj4.ExperienceReward = 300;
    RegisterObjective(Obj4);

    // Objective 5: Land on moon
    FMissionObjective Obj5;
    Obj5.ObjectiveID = "obj_land_moon";
    Obj5.ObjectiveTitle = FText::FromString("Land on the Moon");
    Obj5.ObjectiveDescription = FText::FromString("Execute a safe landing on the lunar surface.");
    Obj5.RequiredStage = EMissionStage::MoonApproach;
    Obj5.ExperienceReward = 400;
    RegisterObjective(Obj5);

    // Objective 6: Explore moon
    FMissionObjective Obj6;
    Obj6.ObjectiveID = "obj_explore_moon";
    Obj6.ObjectiveTitle = FText::FromString("Explore the Moon");
    Obj6.ObjectiveDescription = FText::FromString("Exit your ship and walk on the lunar surface.");
    Obj6.RequiredStage = EMissionStage::MoonSurface;
    Obj6.ExperienceReward = 500;
    RegisterObjective(Obj6);

    // Objective 7: Return to planet
    FMissionObjective Obj7;
    Obj7.ObjectiveID = "obj_return_planet";
    Obj7.ObjectiveTitle = FText::FromString("Return to Planet");
    Obj7.ObjectiveDescription = FText::FromString("Fly back to the planet and land safely.");
    Obj7.RequiredStage = EMissionStage::ReturnFlight;
    Obj7.ExperienceReward = 600;
    RegisterObjective(Obj7);

    MissionState.TotalObjectives = Objectives.Num();
}

void AFirstMissionGameMode::StartMission()
{
    MissionState.MissionStartTime = GetWorld()->GetTimeSeconds();
    MissionState.CurrentStage = EMissionStage::Briefing;
    
    OnMissionStageChanged.Broadcast(MissionState.CurrentStage);
    
    // Transition to planet surface after brief delay
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        SetMissionStage(EMissionStage::PlanetSurface);
    }, 3.0f, false);

    UE_LOG(LogTemp, Log, TEXT("FirstMissionGameMode: Mission started"));
}

void AFirstMissionGameMode::CompleteMission()
{
    MissionState.CurrentStage = EMissionStage::MissionComplete;
    MissionState.MissionElapsedTime = GetWorld()->GetTimeSeconds() - MissionState.MissionStartTime;

    OnMissionStageChanged.Broadcast(MissionState.CurrentStage);
    OnMissionCompleted.Broadcast(MissionState);

    UE_LOG(LogTemp, Log, TEXT("FirstMissionGameMode: Mission completed in %.2f seconds"), 
           MissionState.MissionElapsedTime);
}

void AFirstMissionGameMode::FailMission(const FString& Reason)
{
    MissionState.CurrentStage = EMissionStage::MissionFailed;
    
    OnMissionStageChanged.Broadcast(MissionState.CurrentStage);
    OnMissionFailed.Broadcast(Reason);

    UE_LOG(LogTemp, Warning, TEXT("FirstMissionGameMode: Mission failed - %s"), *Reason);
}

void AFirstMissionGameMode::RestartMission()
{
    // Reset mission state
    MissionState = FMissionState();
    
    // Reset all objectives
    for (auto& ObjPair : Objectives)
    {
        ObjPair.Value.bIsComplete = false;
    }
    
    CurrentObjectiveIndex = 0;
    TotalDistanceTraveled = 0.0f;

    // Restart mission
    StartMission();

    UE_LOG(LogTemp, Log, TEXT("FirstMissionGameMode: Mission restarted"));
}

void AFirstMissionGameMode::SetMissionStage(EMissionStage NewStage)
{
    if (MissionState.CurrentStage == NewStage)
    {
        return;
    }

    EMissionStage OldStage = MissionState.CurrentStage;
    MissionState.CurrentStage = NewStage;

    OnMissionStageChanged.Broadcast(NewStage);
    
    PlayStageAudio(NewStage);
    SpawnStageVFX(NewStage);
    UpdateTutorialSteps();

    UE_LOG(LogTemp, Log, TEXT("FirstMissionGameMode: Stage changed from %d to %d"), 
           (int32)OldStage, (int32)NewStage);
}

void AFirstMissionGameMode::RegisterObjective(const FMissionObjective& Objective)
{
    Objectives.Add(Objective.ObjectiveID, Objective);
    ObjectiveOrder.Add(Objective.ObjectiveID);
}

void AFirstMissionGameMode::CompleteObjective(const FString& ObjectiveID)
{
    FMissionObjective* Objective = Objectives.Find(ObjectiveID);
    if (!Objective || Objective->bIsComplete)
    {
        return;
    }

    Objective->bIsComplete = true;
    MissionState.ObjectivesCompleted++;
    MissionState.TotalExperience += Objective->ExperienceReward;

    OnObjectiveCompleted.Broadcast(ObjectiveID, Objective->ExperienceReward);

    UE_LOG(LogTemp, Log, TEXT("FirstMissionGameMode: Objective completed - %s (+%d XP)"), 
           *ObjectiveID, Objective->ExperienceReward);
}

bool AFirstMissionGameMode::IsObjectiveComplete(const FString& ObjectiveID) const
{
    const FMissionObjective* Objective = Objectives.Find(ObjectiveID);
    return Objective && Objective->bIsComplete;
}

TArray<FMissionObjective> AFirstMissionGameMode::GetActiveObjectives() const
{
    TArray<FMissionObjective> ActiveObjectives;
    for (const auto& ObjPair : Objectives)
    {
        if (!ObjPair.Value.bIsComplete && ObjPair.Value.RequiredStage == MissionState.CurrentStage)
        {
            ActiveObjectives.Add(ObjPair.Value);
        }
    }
    return ActiveObjectives;
}

FMissionObjective AFirstMissionGameMode::GetCurrentObjective() const
{
    if (CurrentObjectiveIndex < ObjectiveOrder.Num())
    {
        const FString& ObjID = ObjectiveOrder[CurrentObjectiveIndex];
        const FMissionObjective* Obj = Objectives.Find(ObjID);
        if (Obj)
        {
            return *Obj;
        }
    }
    return FMissionObjective();
}

void AFirstMissionGameMode::SetPlayerInShip(bool bInShip)
{
    MissionState.bPlayerInShip = bInShip;

    if (bInShip && MissionState.CurrentStage == EMissionStage::EnteringShip)
    {
        CompleteObjective("obj_enter_ship");
        SetMissionStage(EMissionStage::ShipTutorial);
    }
}

void AFirstMissionGameMode::NotifyShipLaunched()
{
    MissionState.bShipLaunched = true;
    
    if (MissionState.CurrentStage == EMissionStage::Launch)
    {
        CompleteObjective("obj_launch");
        SetMissionStage(EMissionStage::SpaceFlight);
    }
}

void AFirstMissionGameMode::NotifyReachedMoon()
{
    MissionState.bReachedMoon = true;
    
    if (MissionState.CurrentStage == EMissionStage::SpaceFlight)
    {
        CompleteObjective("obj_navigate_moon");
        SetMissionStage(EMissionStage::MoonApproach);
    }
}

void AFirstMissionGameMode::NotifyLandedOnMoon()
{
    MissionState.bLandedOnMoon = true;
    
    if (MissionState.CurrentStage == EMissionStage::MoonLanding)
    {
        CompleteObjective("obj_land_moon");
        SetMissionStage(EMissionStage::MoonSurface);
    }
}

void AFirstMissionGameMode::NotifyMoonExplorationComplete()
{
    MissionState.bExploredMoon = true;
    
    if (MissionState.CurrentStage == EMissionStage::MoonExploration)
    {
        CompleteObjective("obj_explore_moon");
        SetMissionStage(EMissionStage::ReturnPrep);
    }
}

void AFirstMissionGameMode::NotifyReturnedToPlanet()
{
    MissionState.bReturnedToPlanet = true;
    
    if (MissionState.CurrentStage == EMissionStage::PlanetLanding)
    {
        CompleteObjective("obj_return_planet");
        CompleteMission();
    }
}

void AFirstMissionGameMode::RegisterPlayerShip(ASpaceship* Ship)
{
    PlayerShip = Ship;
    UE_LOG(LogTemp, Log, TEXT("FirstMissionGameMode: Player ship registered"));
}

void AFirstMissionGameMode::RegisterPlanet(APlanet* InPlanet)
{
    Planet = InPlanet;
    UE_LOG(LogTemp, Log, TEXT("FirstMissionGameMode: Planet registered"));
}

void AFirstMissionGameMode::RegisterMoon(APlanet* InMoon)
{
    Moon = InMoon;
    UE_LOG(LogTemp, Log, TEXT("FirstMissionGameMode: Moon registered"));
}

void AFirstMissionGameMode::ShowTutorialStep(const FString& StepID)
{
    if (TutorialSystem)
    {
        TutorialSystem->StartTutorialStep(StepID);
    }
}

void AFirstMissionGameMode::CompleteTutorialStep(const FString& StepID)
{
    if (TutorialSystem)
    {
        TutorialSystem->CompleteTutorialStep(StepID);
    }
}

float AFirstMissionGameMode::GetDistanceToMoon() const
{
    if (PlayerShip && Moon)
    {
        return FVector::Dist(PlayerShip->GetActorLocation(), Moon->GetActorLocation());
    }
    return -1.0f;
}

float AFirstMissionGameMode::GetDistanceToPlanet() const
{
    if (PlayerShip && Planet)
    {
        return FVector::Dist(PlayerShip->GetActorLocation(), Planet->GetActorLocation());
    }
    return -1.0f;
}

FVector AFirstMissionGameMode::GetMoonDirection() const
{
    if (PlayerShip && Moon)
    {
        return (Moon->GetActorLocation() - PlayerShip->GetActorLocation()).GetSafeNormal();
    }
    return FVector::ZeroVector;
}

FVector AFirstMissionGameMode::GetPlanetDirection() const
{
    if (PlayerShip && Planet)
    {
        return (Planet->GetActorLocation() - PlayerShip->GetActorLocation()).GetSafeNormal();
    }
    return FVector::ZeroVector;
}

void AFirstMissionGameMode::UpdateMissionProgress(float DeltaTime)
{
    MissionState.MissionElapsedTime = GetWorld()->GetTimeSeconds() - MissionState.MissionStartTime;
    
    CheckObjectiveCompletion();
    ProcessStageTransitions();
}

void AFirstMissionGameMode::CheckObjectiveCompletion()
{
    // Check distance-based objectives
    FMissionObjective CurrentObj = GetCurrentObjective();
    
    if (!CurrentObj.bIsComplete && PlayerShip)
    {
        float DistanceToTarget = FVector::Dist(PlayerShip->GetActorLocation(), CurrentObj.TargetLocation);
        if (DistanceToTarget <= CurrentObj.CompletionRadius)
        {
            CompleteObjective(CurrentObj.ObjectiveID);
        }
    }
}

void AFirstMissionGameMode::ProcessStageTransitions()
{
    // Automatic stage transitions based on state
    switch (MissionState.CurrentStage)
    {
        case EMissionStage::ShipTutorial:
            // Transition to launch after tutorial complete
            if (TutorialSystem && TutorialSystem->IsTutorialStepCompleted("ship_controls"))
            {
                SetMissionStage(EMissionStage::Launch);
            }
            break;

        case EMissionStage::MoonApproach:
            // Check if we're close enough for landing
            if (GetDistanceToMoon() < MoonDistance * 0.1f) // Within 10% of moon distance
            {
                SetMissionStage(EMissionStage::MoonLanding);
            }
            break;

        case EMissionStage::MoonSurface:
            // Check if player has exited ship
            if (!MissionState.bPlayerInShip)
            {
                SetMissionStage(EMissionStage::MoonExploration);
                MoonExplorationStartLocation = PlayerShip ? PlayerShip->GetActorLocation() : FVector::ZeroVector;
            }
            break;

        case EMissionStage::ReturnPrep:
            // Check if player has re-entered ship
            if (MissionState.bPlayerInShip)
            {
                SetMissionStage(EMissionStage::ReturnFlight);
            }
            break;

        case EMissionStage::ReturnFlight:
            // Check if we're approaching planet
            if (GetDistanceToPlanet() < MoonDistance * 0.2f)
            {
                SetMissionStage(EMissionStage::PlanetApproach);
            }
            break;

        case EMissionStage::PlanetApproach:
            // Check if we're close enough for landing
            if (GetDistanceToPlanet() < 1000.0f)
            {
                SetMissionStage(EMissionStage::PlanetLanding);
            }
            break;
    }
}

void AFirstMissionGameMode::UpdateTutorialSteps()
{
    if (!TutorialSystem)
    {
        return;
    }

    // Show stage-specific tutorials
    switch (MissionState.CurrentStage)
    {
        case EMissionStage::PlanetSurface:
            ShowTutorialStep("movement_basic");
            break;
        case EMissionStage::ShipTutorial:
            ShowTutorialStep("ship_controls");
            break;
        case EMissionStage::Launch:
            ShowTutorialStep("ship_launch");
            break;
        case EMissionStage::SpaceFlight:
            ShowTutorialStep("space_navigation");
            break;
        case EMissionStage::MoonLanding:
            ShowTutorialStep("ship_landing");
            break;
        case EMissionStage::MoonExploration:
            ShowTutorialStep("moon_exploration");
            break;
    }
}

void AFirstMissionGameMode::PlayStageAudio(EMissionStage Stage)
{
    if (!AudioManager)
    {
        return;
    }

    // Stage-specific audio cues (placeholder for actual audio implementation)
    UE_LOG(LogTemp, Log, TEXT("FirstMissionGameMode: Playing audio for stage %d"), (int32)Stage);
}

void AFirstMissionGameMode::SpawnStageVFX(EMissionStage Stage)
{
    if (!VFXManager)
    {
        return;
    }

    // Stage-specific VFX (placeholder for actual VFX implementation)
    UE_LOG(LogTemp, Log, TEXT("FirstMissionGameMode: Spawning VFX for stage %d"), (int32)Stage);
}
