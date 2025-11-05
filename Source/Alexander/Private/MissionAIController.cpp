// Copyright Epic Games, Inc. All Rights Reserved.

#include "MissionAIController.h"
#include "SurfacePlayerCharacter.h"
#include "Spaceship.h"
#include "LandingGuidanceSystem.h"
#include "ShipEntryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

AMissionAIController::AMissionAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Default configuration
    bCaptureScreenshots = true;
    bDebugMode = true;
    SpeedMultiplier = 1.0f;
    StageDelayTime = 2.0f;
    
    // State initialization
    CurrentMissionStage = EMissionStage::NotStarted;
    CurrentAction = EMissionAIAction::Idle;
    bIsAutomating = false;
    bIsPaused = false;
    ActionTimer = 0.0f;
    MissionStartTime = 0.0f;
    
    // Reference initialization
    MissionGameMode = nullptr;
    SurfaceCharacter = nullptr;
    Spaceship = nullptr;
    LandingGuidance = nullptr;
    
    // State tracking
    bFoundSpaceship = false;
    bInSpaceship = false;
    bHasTakenOff = false;
    bReachedMoon = false;
    ExplorationDistance = 0.0f;
    LandingPhase = 0;
    
    SpaceshipLocation = FVector::ZeroVector;
    MoonLocation = FVector::ZeroVector;
    PlanetLocation = FVector::ZeroVector;
    FlightTarget = FVector::ZeroVector;
    LastExplorationPosition = FVector::ZeroVector;
}

void AMissionAIController::BeginPlay()
{
    Super::BeginPlay();
    
    // Find mission game mode
    MissionGameMode = Cast<AFirstMissionGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (MissionGameMode)
    {
        // Bind to mission stage changes
        MissionGameMode->OnMissionStageChanged.AddDynamic(this, &AMissionAIController::OnMissionStageChanged);
        LogDebug("Mission AI Controller initialized - bound to game mode");
    }
    else
    {
        LogDebug("WARNING: Could not find FirstMissionGameMode!");
    }
}

void AMissionAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bIsAutomating || bIsPaused)
        return;
    
    ActionTimer += DeltaTime;
    
    // Execute current action
    ExecuteCurrentAction(DeltaTime);
    
    // Debug visualization
    if (bDebugMode && GetPawn())
    {
        DrawDebugSphere(GetWorld(), GetPawn()->GetActorLocation(), 100.0f, 12, FColor::Green, false, -1.0f, 0, 2.0f);
    }
}

void AMissionAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    // Check if possessing character or spaceship
    SurfaceCharacter = Cast<ASurfacePlayerCharacter>(InPawn);
    if (SurfaceCharacter)
    {
        LogDebug("AI Controller possessed Surface Character");
        bInSpaceship = false;
    }
    
    Spaceship = Cast<ASpaceship>(InPawn);
    if (Spaceship)
    {
        LogDebug("AI Controller possessed Spaceship");
        bInSpaceship = true;
        
        // Find landing guidance system
        LandingGuidance = Cast<ALandingGuidanceSystem>(
            UGameplayStatics::GetActorOfClass(GetWorld(), ALandingGuidanceSystem::StaticClass())
        );
    }
}

void AMissionAIController::OnUnPossess()
{
    Super::OnUnPossess();
    
    LogDebug("AI Controller unpossessed pawn");
    SurfaceCharacter = nullptr;
    Spaceship = nullptr;
}

// ==================== Main Control ====================

void AMissionAIController::StartAutomatedMission()
{
    if (bIsAutomating)
    {
        LogDebug("Mission automation already running");
        return;
    }
    
    LogDebug("Starting automated mission playthrough");
    
    bIsAutomating = true;
    bIsPaused = false;
    MissionStartTime = GetWorld()->GetTimeSeconds();
    ActionTimer = 0.0f;
    
    // Reset state
    bFoundSpaceship = false;
    bInSpaceship = false;
    bHasTakenOff = false;
    bReachedMoon = false;
    ExplorationDistance = 0.0f;
    LandingPhase = 0;
    
    // Find celestial bodies
    FindCelestialBodies();
    
    // Start mission if game mode exists
    if (MissionGameMode)
    {
        MissionGameMode->StartMission();
    }
    
    // Transition to first action
    TransitionToNextAction();
    
    OnMissionMilestone.Broadcast("Mission Automation Started");
}

void AMissionAIController::StopAutomatedMission()
{
    if (!bIsAutomating)
        return;
    
    LogDebug("Stopping automated mission playthrough");
    
    bIsAutomating = false;
    CurrentAction = EMissionAIAction::Idle;
    
    OnMissionMilestone.Broadcast("Mission Automation Stopped");
}

void AMissionAIController::SetAutomationPaused(bool bPaused)
{
    bIsPaused = bPaused;
    LogDebug(bPaused ? "Mission automation paused" : "Mission automation resumed");
}

// ==================== Action Management ====================

void AMissionAIController::OnMissionStageChanged(EMissionStage NewStage)
{
    CurrentMissionStage = NewStage;
    LogDebug(FString::Printf(TEXT("Mission stage changed to: %d"), (int32)NewStage));
    
    // Capture screenshot if enabled
    if (bCaptureScreenshots)
    {
        FString StageName = UEnum::GetValueAsString(NewStage);
        CaptureStageScreenshot(StageName);
    }
    
    // Transition to appropriate action
    TransitionToNextAction();
}

void AMissionAIController::ExecuteCurrentAction(float DeltaTime)
{
    switch (CurrentAction)
    {
        case EMissionAIAction::WalkToSpaceship:
            Action_WalkToSpaceship(DeltaTime);
            break;
            
        case EMissionAIAction::EnterSpaceship:
            Action_EnterSpaceship(DeltaTime);
            break;
            
        case EMissionAIAction::WaitForTutorial:
            // Just wait for tutorial to complete
            if (ActionTimer >= 5.0f)
            {
                CompleteAction(true);
            }
            break;
            
        case EMissionAIAction::TakeOff:
            Action_TakeOff(DeltaTime);
            break;
            
        case EMissionAIAction::FlyToMoon:
            Action_FlyToMoon(DeltaTime);
            break;
            
        case EMissionAIAction::LandOnMoon:
            Action_LandOnMoon(DeltaTime);
            break;
            
        case EMissionAIAction::ExploreMoon:
            Action_ExploreMoon(DeltaTime);
            break;
            
        case EMissionAIAction::ReturnToShip:
            Action_ReturnToShip(DeltaTime);
            break;
            
        case EMissionAIAction::FlyToPlanet:
            Action_FlyToPlanet(DeltaTime);
            break;
            
        case EMissionAIAction::LandOnPlanet:
            Action_LandOnPlanet(DeltaTime);
            break;
            
        case EMissionAIAction::ExitShip:
            Action_ExitShip(DeltaTime);
            break;
            
        case EMissionAIAction::MissionComplete:
            // Mission complete - stop automation
            if (ActionTimer >= StageDelayTime)
            {
                StopAutomatedMission();
            }
            break;
            
        default:
            break;
    }
}

void AMissionAIController::CompleteAction(bool bSuccess)
{
    LogDebug(FString::Printf(TEXT("Action completed: %s (Success: %s)"), 
        *UEnum::GetValueAsString(CurrentAction), 
        bSuccess ? TEXT("Yes") : TEXT("No")));
    
    OnAIActionCompleted.Broadcast(CurrentAction, bSuccess);
    
    ActionTimer = 0.0f;
    
    // Wait before transitioning
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        [this]() { TransitionToNextAction(); },
        StageDelayTime,
        false
    );
}

void AMissionAIController::TransitionToNextAction()
{
    EMissionAIAction NewAction = EMissionAIAction::Idle;
    
    switch (CurrentMissionStage)
    {
        case EMissionStage::NotStarted:
        case EMissionStage::Briefing:
            NewAction = EMissionAIAction::Idle;
            break;
            
        case EMissionStage::PlanetSurface:
            NewAction = EMissionAIAction::WalkToSpaceship;
            break;
            
        case EMissionStage::EnteringShip:
            NewAction = EMissionAIAction::EnterSpaceship;
            break;
            
        case EMissionStage::ShipTutorial:
            NewAction = EMissionAIAction::WaitForTutorial;
            break;
            
        case EMissionStage::Launch:
            NewAction = EMissionAIAction::TakeOff;
            break;
            
        case EMissionStage::SpaceFlight:
        case EMissionStage::MoonApproach:
            NewAction = EMissionAIAction::FlyToMoon;
            break;
            
        case EMissionStage::MoonLanding:
            NewAction = EMissionAIAction::LandOnMoon;
            break;
            
        case EMissionStage::MoonSurface:
        case EMissionStage::MoonExploration:
            NewAction = EMissionAIAction::ExploreMoon;
            break;
            
        case EMissionStage::ReturnPrep:
            NewAction = EMissionAIAction::ReturnToShip;
            break;
            
        case EMissionStage::ReturnFlight:
        case EMissionStage::PlanetApproach:
            NewAction = EMissionAIAction::FlyToPlanet;
            break;
            
        case EMissionStage::PlanetLanding:
            NewAction = EMissionAIAction::LandOnPlanet;
            break;
            
        case EMissionStage::MissionComplete:
            NewAction = EMissionAIAction::ExitShip;
            break;
            
        default:
            NewAction = EMissionAIAction::Idle;
            break;
    }
    
    if (NewAction != CurrentAction)
    {
        CurrentAction = NewAction;
        ActionTimer = 0.0f;
        LogDebug(FString::Printf(TEXT("Transitioned to action: %s"), *UEnum::GetValueAsString(CurrentAction)));
    }
}

// ==================== Ground Movement ====================

void AMissionAIController::Action_WalkToSpaceship(float DeltaTime)
{
    if (!SurfaceCharacter)
    {
        LogDebug("No surface character - cannot walk");
        CompleteAction(false);
        return;
    }
    
    // Find spaceship if not found yet
    if (!bFoundSpaceship)
    {
        FindSpaceship();
        if (!Spaceship)
        {
            LogDebug("Spaceship not found in level");
            CompleteAction(false);
            return;
        }
    }
    
    // Calculate direction to spaceship
    FVector CurrentLocation = SurfaceCharacter->GetActorLocation();
    FVector Direction = (SpaceshipLocation - CurrentLocation).GetSafeNormal();
    float Distance = FVector::Dist(CurrentLocation, SpaceshipLocation);
    
    // Check if reached spaceship
    if (Distance < 300.0f)
    {
        LogDebug("Reached spaceship");
        CompleteAction(true);
        return;
    }
    
    // Move towards spaceship
    FVector2D MoveInput(Direction.X, Direction.Y);
    MoveInput.Normalize();
    MoveInput *= SpeedMultiplier;
    
    ApplyCharacterInput(MoveInput, FVector2D::ZeroVector);
    
    // Debug visualization
    if (bDebugMode)
    {
        DrawDebugLine(GetWorld(), CurrentLocation, SpaceshipLocation, FColor::Blue, false, -1.0f, 0, 2.0f);
    }
}

void AMissionAIController::Action_EnterSpaceship(float DeltaTime)
{
    if (!SurfaceCharacter || !Spaceship)
    {
        CompleteAction(false);
        return;
    }
    
    // Find ship entry component
    UShipEntryComponent* EntryComponent = Spaceship->FindComponentByClass<UShipEntryComponent>();
    if (EntryComponent && EntryComponent->CanEnterShip(SurfaceCharacter))
    {
        // Simulate interaction button press
        SimulateButtonPress("Interact");
        
        // Wait a moment for entry to complete
        if (ActionTimer >= 2.0f)
        {
            LogDebug("Entered spaceship");
            bInSpaceship = true;
            CompleteAction(true);
        }
    }
    else
    {
        LogDebug("Cannot enter spaceship yet");
        if (ActionTimer >= 5.0f)
        {
            CompleteAction(false);
        }
    }
}

void AMissionAIController::Action_ExploreMoon(float DeltaTime)
{
    if (!SurfaceCharacter)
    {
        CompleteAction(false);
        return;
    }
    
    // Initialize exploration
    if (ExplorationDistance < 1.0f)
    {
        LastExplorationPosition = SurfaceCharacter->GetActorLocation();
    }
    
    // Walk in a pattern (circle or figure-8)
    float ExplorationAngle = ActionTimer * 30.0f; // Degrees per second
    FVector ExplorationDirection(
        FMath::Cos(FMath::DegreesToRadians(ExplorationAngle)),
        FMath::Sin(FMath::DegreesToRadians(ExplorationAngle)),
        0.0f
    );
    
    FVector2D MoveInput(ExplorationDirection.X, ExplorationDirection.Y);
    MoveInput *= SpeedMultiplier;
    
    ApplyCharacterInput(MoveInput, FVector2D::ZeroVector);
    
    // Track distance
    FVector CurrentLocation = SurfaceCharacter->GetActorLocation();
    ExplorationDistance += FVector::Dist(LastExplorationPosition, CurrentLocation);
    LastExplorationPosition = CurrentLocation;
    
    // Check if explored enough (500 meters)
    if (ExplorationDistance >= 50000.0f || ActionTimer >= 30.0f)
    {
        LogDebug(FString::Printf(TEXT("Moon exploration complete - traveled %.0f cm"), ExplorationDistance));
        
        if (MissionGameMode)
        {
            MissionGameMode->NotifyMoonExplorationComplete();
        }
        
        CompleteAction(true);
    }
}

void AMissionAIController::Action_ReturnToShip(float DeltaTime)
{
    if (!SurfaceCharacter || !Spaceship)
    {
        CompleteAction(false);
        return;
    }
    
    // Navigate back to spaceship
    FVector CurrentLocation = SurfaceCharacter->GetActorLocation();
    FVector ShipLocation = Spaceship->GetActorLocation();
    FVector Direction = (ShipLocation - CurrentLocation).GetSafeNormal();
    float Distance = FVector::Dist(CurrentLocation, ShipLocation);
    
    // Check if reached ship
    if (Distance < 300.0f)
    {
        LogDebug("Returned to spaceship");
        
        // Enter ship
        UShipEntryComponent* EntryComponent = Spaceship->FindComponentByClass<UShipEntryComponent>();
        if (EntryComponent)
        {
            SimulateButtonPress("Interact");
        }
        
        CompleteAction(true);
        return;
    }
    
    // Move towards ship
    FVector2D MoveInput(Direction.X, Direction.Y);
    MoveInput.Normalize();
    MoveInput *= SpeedMultiplier;
    
    ApplyCharacterInput(MoveInput, FVector2D::ZeroVector);
}

void AMissionAIController::Action_ExitShip(float DeltaTime)
{
    if (!Spaceship)
    {
        CompleteAction(false);
        return;
    }
    
    // Find ship entry component
    UShipEntryComponent* EntryComponent = Spaceship->FindComponentByClass<UShipEntryComponent>();
    if (EntryComponent && EntryComponent->CanExitShip(GetPawn()))
    {
        // Simulate exit button press
        SimulateButtonPress("Interact");
        
        // Wait for exit to complete
        if (ActionTimer >= 2.0f)
        {
            LogDebug("Exited spaceship - mission complete!");
            bInSpaceship = false;
            CompleteAction(true);
            
            OnMissionMilestone.Broadcast("Mission Complete!");
        }
    }
    else
    {
        if (ActionTimer >= 5.0f)
        {
            CompleteAction(false);
        }
    }
}

// ==================== Flight Control ====================

void AMissionAIController::Action_TakeOff(float DeltaTime)
{
    if (!Spaceship)
    {
        CompleteAction(false);
        return;
    }
    
    // Apply throttle to take off
    float TargetAltitude = 1000.0f; // 10 meters
    FVector CurrentLocation = Spaceship->GetActorLocation();
    
    if (CurrentLocation.Z >= TargetAltitude || ActionTimer >= 10.0f)
    {
        LogDebug("Take off complete");
        bHasTakenOff = true;
        
        if (MissionGameMode)
        {
            MissionGameMode->NotifyShipLaunched();
        }
        
        CompleteAction(true);
        return;
    }
    
    // Apply upward throttle
    ApplySpaceshipInput(0.7f * SpeedMultiplier, 0.0f, 0.0f, 0.0f);
}

void AMissionAIController::Action_FlyToMoon(float DeltaTime)
{
    if (!Spaceship)
    {
        CompleteAction(false);
        return;
    }
    
    if (MoonLocation.IsZero())
    {
        FindCelestialBodies();
    }
    
    // Calculate direction to moon
    FVector CurrentLocation = Spaceship->GetActorLocation();
    FVector Direction = (MoonLocation - CurrentLocation).GetSafeNormal();
    float Distance = FVector::Dist(CurrentLocation, MoonLocation);
    
    LogDebug(FString::Printf(TEXT("Distance to moon: %.0f meters"), Distance / 100.0f));
    
    // Check if reached moon proximity (5km)
    if (Distance < 500000.0f || ActionTimer >= 60.0f)
    {
        LogDebug("Reached moon");
        bReachedMoon = true;
        
        if (MissionGameMode)
        {
            MissionGameMode->NotifyReachedMoon();
        }
        
        CompleteAction(true);
        return;
    }
    
    // Calculate pitch and yaw to face moon
    FRotator TargetRotation = Direction.Rotation();
    FRotator CurrentRotation = Spaceship->GetActorRotation();
    FRotator DeltaRotation = (TargetRotation - CurrentRotation).GetNormalized();
    
    float Pitch = FMath::Clamp(DeltaRotation.Pitch / 90.0f, -1.0f, 1.0f);
    float Yaw = FMath::Clamp(DeltaRotation.Yaw / 90.0f, -1.0f, 1.0f);
    
    // Apply input - full throttle, adjust orientation
    ApplySpaceshipInput(
        0.9f * SpeedMultiplier,
        Pitch * SpeedMultiplier,
        Yaw * SpeedMultiplier,
        0.0f
    );
}

void AMissionAIController::Action_LandOnMoon(float DeltaTime)
{
    if (!Spaceship)
    {
        CompleteAction(false);
        return;
    }
    
    FVector CurrentLocation = Spaceship->GetActorLocation();
    FVector CurrentVelocity = Spaceship->GetVelocity();
    
    // Landing phases:
    // 0: Approach - descend to 100m
    // 1: Final approach - slow descent to surface
    // 2: Touchdown
    
    if (LandingPhase == 0)
    {
        // Descend to 100m above surface
        if (CurrentLocation.Z <= MoonLocation.Z + 10000.0f)
        {
            LandingPhase = 1;
            LogDebug("Landing phase 1: Final approach");
        }
        else
        {
            // Reduce throttle, pitch down
            ApplySpaceshipInput(0.3f, -0.5f * SpeedMultiplier, 0.0f, 0.0f);
        }
    }
    else if (LandingPhase == 1)
    {
        // Slow final descent
        if (CurrentLocation.Z <= MoonLocation.Z + 500.0f)
        {
            LandingPhase = 2;
            LogDebug("Landing phase 2: Touchdown");
        }
        else
        {
            // Very gentle descent
            float VerticalVelocity = CurrentVelocity.Z;
            float ThrottleAdjustment = FMath::Clamp(-VerticalVelocity / 1000.0f, -0.5f, 0.5f);
            ApplySpaceshipInput(0.1f + ThrottleAdjustment, 0.0f, 0.0f, 0.0f);
        }
    }
    else if (LandingPhase == 2)
    {
        // Check if landed (velocity near zero, close to surface)
        if (CurrentVelocity.Size() < 100.0f || ActionTimer >= 30.0f)
        {
            LogDebug("Landed on moon!");
            
            if (MissionGameMode)
            {
                MissionGameMode->NotifyLandedOnMoon();
            }
            
            LandingPhase = 0; // Reset for planet landing
            CompleteAction(true);
        }
        else
        {
            // Cut throttle for final touchdown
            ApplySpaceshipInput(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }
}

void AMissionAIController::Action_FlyToPlanet(float DeltaTime)
{
    if (!Spaceship)
    {
        CompleteAction(false);
        return;
    }
    
    if (PlanetLocation.IsZero())
    {
        FindCelestialBodies();
    }
    
    // Calculate direction to planet
    FVector CurrentLocation = Spaceship->GetActorLocation();
    FVector Direction = (PlanetLocation - CurrentLocation).GetSafeNormal();
    float Distance = FVector::Dist(CurrentLocation, PlanetLocation);
    
    LogDebug(FString::Printf(TEXT("Distance to planet: %.0f meters"), Distance / 100.0f));
    
    // Check if reached planet proximity (5km)
    if (Distance < 500000.0f || ActionTimer >= 60.0f)
    {
        LogDebug("Reached planet");
        CompleteAction(true);
        return;
    }
    
    // Calculate pitch and yaw to face planet
    FRotator TargetRotation = Direction.Rotation();
    FRotator CurrentRotation = Spaceship->GetActorRotation();
    FRotator DeltaRotation = (TargetRotation - CurrentRotation).GetNormalized();
    
    float Pitch = FMath::Clamp(DeltaRotation.Pitch / 90.0f, -1.0f, 1.0f);
    float Yaw = FMath::Clamp(DeltaRotation.Yaw / 90.0f, -1.0f, 1.0f);
    
    // Apply input
    ApplySpaceshipInput(
        0.9f * SpeedMultiplier,
        Pitch * SpeedMultiplier,
        Yaw * SpeedMultiplier,
        0.0f
    );
}

void AMissionAIController::Action_LandOnPlanet(float DeltaTime)
{
    if (!Spaceship)
    {
        CompleteAction(false);
        return;
    }
    
    FVector CurrentLocation = Spaceship->GetActorLocation();
    FVector CurrentVelocity = Spaceship->GetVelocity();
    
    // Similar landing logic to moon landing
    if (LandingPhase == 0)
    {
        if (CurrentLocation.Z <= PlanetLocation.Z + 10000.0f)
        {
            LandingPhase = 1;
            LogDebug("Planet landing phase 1");
        }
        else
        {
            ApplySpaceshipInput(0.3f, -0.5f * SpeedMultiplier, 0.0f, 0.0f);
        }
    }
    else if (LandingPhase == 1)
    {
        if (CurrentLocation.Z <= PlanetLocation.Z + 500.0f)
        {
            LandingPhase = 2;
            LogDebug("Planet landing phase 2");
        }
        else
        {
            float VerticalVelocity = CurrentVelocity.Z;
            float ThrottleAdjustment = FMath::Clamp(-VerticalVelocity / 1000.0f, -0.5f, 0.5f);
            ApplySpaceshipInput(0.1f + ThrottleAdjustment, 0.0f, 0.0f, 0.0f);
        }
    }
    else if (LandingPhase == 2)
    {
        if (CurrentVelocity.Size() < 100.0f || ActionTimer >= 30.0f)
        {
            LogDebug("Landed on planet!");
            
            if (MissionGameMode)
            {
                MissionGameMode->NotifyReturnedToPlanet();
            }
            
            LandingPhase = 0;
            CompleteAction(true);
        }
        else
        {
            ApplySpaceshipInput(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }
}

// ==================== Helper Functions ====================

void AMissionAIController::ApplyCharacterInput(FVector2D MoveInput, FVector2D LookInput)
{
    if (!SurfaceCharacter)
        return;
    
    // Apply movement via character functions
    SurfaceCharacter->MoveForward(MoveInput.X);
    SurfaceCharacter->MoveRight(MoveInput.Y);
    
    if (!LookInput.IsZero())
    {
        SurfaceCharacter->Turn(LookInput.X);
        SurfaceCharacter->LookUp(LookInput.Y);
    }
}

void AMissionAIController::ApplySpaceshipInput(float Throttle, float Pitch, float Yaw, float Roll)
{
    if (!Spaceship)
        return;
    
    // Apply spaceship control inputs
    // Note: Actual implementation depends on Spaceship class interface
    // This is a placeholder - adjust based on your Spaceship implementation
    
    LogDebug(FString::Printf(TEXT("Spaceship Input - Throttle: %.2f, Pitch: %.2f, Yaw: %.2f, Roll: %.2f"),
        Throttle, Pitch, Yaw, Roll));
}

void AMissionAIController::SimulateButtonPress(FName ActionName)
{
    // Simulate button press for interaction
    LogDebug(FString::Printf(TEXT("Simulating button press: %s"), *ActionName.ToString()));
    
    // This would normally trigger input events
    // Implementation depends on your input system
}

void AMissionAIController::FindSpaceship()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpaceship::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        Spaceship = Cast<ASpaceship>(FoundActors[0]);
        if (Spaceship)
        {
            SpaceshipLocation = Spaceship->GetActorLocation();
            bFoundSpaceship = true;
            LogDebug("Found spaceship in level");
        }
    }
}

void AMissionAIController::FindCelestialBodies()
{
    if (MissionGameMode)
    {
        APlanet* Planet = MissionGameMode->GetPlanet();
        APlanet* Moon = MissionGameMode->GetMoon();
        
        if (Planet)
        {
            PlanetLocation = Planet->GetActorLocation();
            LogDebug("Found planet location");
        }
        
        if (Moon)
        {
            MoonLocation = Moon->GetActorLocation();
            LogDebug("Found moon location");
        }
    }
}

void AMissionAIController::CaptureStageScreenshot(const FString& StageName)
{
    if (!bCaptureScreenshots)
        return;
    
    FString FileName = FString::Printf(TEXT("Mission_%s_%d"), *StageName, FMath::RandRange(1000, 9999));
    
    LogDebug(FString::Printf(TEXT("Capturing screenshot: %s"), *FileName));
    
    // Request screenshot capture
    // Note: Actual implementation depends on your screenshot system
    FScreenshotRequest::RequestScreenshot(FileName, false, false);
}

void AMissionAIController::LogDebug(const FString& Message)
{
    if (!bDebugMode)
        return;
    
    UE_LOG(LogTemp, Log, TEXT("[MissionAI] %s"), *Message);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("[MissionAI] %s"), *Message));
    }
}
