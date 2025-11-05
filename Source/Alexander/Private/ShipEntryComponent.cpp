// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShipEntryComponent.h"
#include "Spaceship.h"
#include "FirstMissionGameMode.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "MotionControllerComponent.h"
#include "DrawDebugHelpers.h"

UShipEntryComponent::UShipEntryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Default configuration
    ProximityCheckRadius = 1000.0f;
    InteractionCheckInterval = 0.1f;
    TransitionDuration = 2.0f;
    bUseSmoothTransition = true;
    bInteractionEnabled = true;
    bVRInteractionEnabled = true;
    bRequireInputToEnter = true;
    bAutoExitOnLanding = false;
    DefaultEntryPointID = FName("Cockpit_Main");

    // Initialize state
    CurrentState = EShipEntryState::None;
    CurrentPlayer = nullptr;
    OriginalPlayerPawn = nullptr;
    OwningShip = nullptr;
    PlayerController = nullptr;

    // Transition state
    bIsTransitioning = false;
    TransitionStartTime = 0.0f;
    TransitionProgress = 0.0f;
    TransitionStartLocation = FVector::ZeroVector;
    TransitionTargetLocation = FVector::ZeroVector;
    TransitionStartRotation = FRotator::ZeroRotator;
    TransitionTargetRotation = FRotator::ZeroRotator;

    // Proximity tracking
    LastProximityCheckTime = 0.0f;
    NearestEntryPointID = NAME_None;
    DistanceToNearestEntry = MAX_flt;
    bInInteractionRange = false;

    // Input handling
    bInputSetup = false;
    EntryInputAction = FName("EnterShip");
    ExitInputAction = FName("ExitShip");
}

void UShipEntryComponent::BeginPlay()
{
    Super::BeginPlay();

    // Get owning ship
    OwningShip = Cast<ASpaceship>(GetOwner());
    if (!OwningShip)
    {
        UE_LOG(LogTemp, Error, TEXT("ShipEntryComponent: Owner is not a Spaceship!"));
        return;
    }

    // Setup default entry points if none exist
    if (EntryPoints.Num() == 0)
    {
        FShipEntryPoint DefaultEntry;
        DefaultEntry.EntryPointID = DefaultEntryPointID;
        DefaultEntry.EntryType = EEntryPointType::Cockpit;
        DefaultEntry.LocalLocation = FVector(200.0f, 0.0f, 100.0f);
        DefaultEntry.LocalRotation = FRotator::ZeroRotator;
        DefaultEntry.InteractionRadius = 200.0f;
        DefaultEntry.bRequiresSpacesuit = false;
        DefaultEntry.bIsLocked = false;
        DefaultEntry.InteractionPrompt = FText::FromString("Press E to Enter Ship");
        RegisterEntryPoint(DefaultEntry);
    }

    UE_LOG(LogTemp, Log, TEXT("ShipEntryComponent: Initialized with %d entry points"), EntryPoints.Num());
}

void UShipEntryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwningShip)
    {
        return;
    }

    // Update proximity detection
    if (bInteractionEnabled && CurrentState != EShipEntryState::InsideShip)
    {
        UpdateProximityDetection(DeltaTime);
    }

    // Update transitions
    if (bIsTransitioning)
    {
        UpdateTransition(DeltaTime);
    }

    // Process player input
    if (CurrentPlayer && bInputSetup)
    {
        ProcessPlayerInput();
    }
}

void UShipEntryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up any active transitions
    if (bIsTransitioning && CurrentPlayer && PlayerController)
    {
        bIsTransitioning = false;
        CurrentPlayer->SetActorEnableCollision(true);
    }

    Super::EndPlay(EndPlayReason);
}

void UShipEntryComponent::EnterShip(APawn* Player, const FName& EntryPointID)
{
    if (!Player || !OwningShip)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShipEntryComponent: Invalid player or ship"));
        return;
    }

    // Get entry point
    FShipEntryPoint* EntryPoint = EntryPoints.Find(EntryPointID);
    if (!EntryPoint)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShipEntryComponent: Entry point %s not found"), *EntryPointID.ToString());
        return;
    }

    // Validate entry
    if (!ValidateEntry(Player, *EntryPoint))
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ShipEntryComponent: Player entering ship via %s"), *EntryPointID.ToString());

    // Store original pawn
    OriginalPlayerPawn = Player;
    CurrentPlayer = Player;
    PlayerController = Cast<APlayerController>(Player->GetController());

    // Start transition
    SetEntryState(EShipEntryState::Entering);
    HandleEntryTransition(Player, *EntryPoint);

    // Notify mission system
    if (AFirstMissionGameMode* MissionMode = Cast<AFirstMissionGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        MissionMode->SetPlayerInShip(true);
    }

    // Broadcast event
    OnPlayerEnteredShip.Broadcast(Player, OwningShip);
}

void UShipEntryComponent::ExitShip(APawn* Player, const FName& ExitPointID)
{
    if (!Player || !OwningShip)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShipEntryComponent: Invalid player or ship"));
        return;
    }

    // Get exit point
    FShipEntryPoint* ExitPoint = EntryPoints.Find(ExitPointID);
    if (!ExitPoint)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShipEntryComponent: Exit point %s not found"), *ExitPointID.ToString());
        return;
    }

    // Validate exit
    if (!ValidateExit(Player, *ExitPoint))
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ShipEntryComponent: Player exiting ship via %s"), *ExitPointID.ToString());

    // Start transition
    SetEntryState(EShipEntryState::Exiting);
    HandleExitTransition(Player, *ExitPoint);

    // Notify mission system
    if (AFirstMissionGameMode* MissionMode = Cast<AFirstMissionGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        MissionMode->SetPlayerInShip(false);
    }

    // Broadcast event
    OnPlayerExitedShip.Broadcast(Player, OwningShip);
}

bool UShipEntryComponent::CanEnterShip(APawn* Player) const
{
    if (!Player || !OwningShip || !bInteractionEnabled)
    {
        return false;
    }

    // Check if already in ship
    if (CurrentState == EShipEntryState::InsideShip || CurrentState == EShipEntryState::Entering)
    {
        return false;
    }

    // Check if in range of any entry point
    return bInInteractionRange;
}

bool UShipEntryComponent::CanExitShip(APawn* Player) const
{
    if (!Player || !OwningShip || !bInteractionEnabled)
    {
        return false;
    }

    // Can only exit if inside ship
    if (CurrentState != EShipEntryState::InsideShip)
    {
        return false;
    }

    // Check if ship is landed or moving slowly
    if (OwningShip->GetVelocity().Size() > 100.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShipEntryComponent: Cannot exit while ship is moving"));
        return false;
    }

    return true;
}

bool UShipEntryComponent::IsPlayerInShip() const
{
    return CurrentState == EShipEntryState::InsideShip && CurrentPlayer != nullptr;
}

void UShipEntryComponent::RegisterEntryPoint(const FShipEntryPoint& EntryPoint)
{
    EntryPoints.Add(EntryPoint.EntryPointID, EntryPoint);
    UE_LOG(LogTemp, Log, TEXT("ShipEntryComponent: Registered entry point %s"), *EntryPoint.EntryPointID.ToString());
}

void UShipEntryComponent::RemoveEntryPoint(const FName& EntryPointID)
{
    EntryPoints.Remove(EntryPointID);
}

FShipEntryPoint UShipEntryComponent::GetEntryPoint(const FName& EntryPointID) const
{
    const FShipEntryPoint* Found = EntryPoints.Find(EntryPointID);
    return Found ? *Found : FShipEntryPoint();
}

TArray<FShipEntryPoint> UShipEntryComponent::GetAllEntryPoints() const
{
    TArray<FShipEntryPoint> Result;
    EntryPoints.GenerateValueArray(Result);
    return Result;
}

FShipEntryPoint UShipEntryComponent::GetNearestEntryPoint(const FVector& Location) const
{
    if (!OwningShip || EntryPoints.Num() == 0)
    {
        return FShipEntryPoint();
    }

    float MinDistance = MAX_flt;
    FShipEntryPoint NearestPoint;

    for (const auto& Pair : EntryPoints)
    {
        const FShipEntryPoint& EntryPoint = Pair.Value;
        FVector WorldLocation = OwningShip->GetActorTransform().TransformPosition(EntryPoint.LocalLocation);
        float Distance = FVector::Dist(Location, WorldLocation);

        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestPoint = EntryPoint;
        }
    }

    return NearestPoint;
}

float UShipEntryComponent::GetDistanceToNearestEntry(const FVector& Location) const
{
    if (!OwningShip || EntryPoints.Num() == 0)
    {
        return MAX_flt;
    }

    float MinDistance = MAX_flt;

    for (const auto& Pair : EntryPoints)
    {
        const FShipEntryPoint& EntryPoint = Pair.Value;
        FVector WorldLocation = OwningShip->GetActorTransform().TransformPosition(EntryPoint.LocalLocation);
        float Distance = FVector::Dist(Location, WorldLocation);

        if (Distance < MinDistance)
        {
            MinDistance = Distance;
        }
    }

    return MinDistance;
}

void UShipEntryComponent::SetEntryState(EShipEntryState NewState)
{
    if (CurrentState != NewState)
    {
        EShipEntryState OldState = CurrentState;
        CurrentState = NewState;
        OnEntryStateChanged.Broadcast(NewState);
        
        UE_LOG(LogTemp, Log, TEXT("ShipEntryComponent: State changed from %d to %d"), (int32)OldState, (int32)NewState);
    }
}

void UShipEntryComponent::EnableInteraction(bool bEnabled)
{
    bInteractionEnabled = bEnabled;
}

void UShipEntryComponent::ShowInteractionPrompt(const FShipEntryPoint& EntryPoint)
{
    // Broadcast interaction available event
    float Distance = GetDistanceToNearestEntry(CurrentPlayer ? CurrentPlayer->GetActorLocation() : FVector::ZeroVector);
    OnInteractionAvailable.Broadcast(EntryPoint, Distance);
}

void UShipEntryComponent::HideInteractionPrompt()
{
    // Implementation handled by UI
}

void UShipEntryComponent::EnableVRInteraction(bool bEnabled)
{
    bVRInteractionEnabled = bEnabled;
}

void UShipEntryComponent::HandleVRGrab(UMotionControllerComponent* Controller)
{
    if (!bVRInteractionEnabled || !Controller)
    {
        return;
    }

    // Check if grabbing near an entry point
    FVector GrabLocation = Controller->GetComponentLocation();
    FShipEntryPoint NearestPoint = GetNearestEntryPoint(GrabLocation);
    float Distance = GetDistanceToNearestEntry(GrabLocation);

    if (Distance < NearestPoint.InteractionRadius)
    {
        if (CurrentPlayer && CanEnterShip(CurrentPlayer))
        {
            EnterShip(CurrentPlayer, NearestPoint.EntryPointID);
        }
    }
}

void UShipEntryComponent::HandleVRRelease(UMotionControllerComponent* Controller)
{
    // VR release handling
}

void UShipEntryComponent::SetTransitionDuration(float Duration)
{
    TransitionDuration = FMath::Max(0.1f, Duration);
}

bool UShipEntryComponent::IsTransitioning() const
{
    return bIsTransitioning;
}

float UShipEntryComponent::GetTransitionProgress() const
{
    return TransitionProgress;
}

void UShipEntryComponent::UpdateProximityDetection(float DeltaTime)
{
    if (!OwningShip)
    {
        return;
    }

    // Throttle proximity checks
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastProximityCheckTime < InteractionCheckInterval)
    {
        return;
    }
    LastProximityCheckTime = CurrentTime;

    // Get player pawn
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn())
    {
        return;
    }

    APawn* PlayerPawn = PC->GetPawn();
    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Check distance to ship
    float DistanceToShip = FVector::Dist(PlayerLocation, OwningShip->GetActorLocation());
    if (DistanceToShip > ProximityCheckRadius)
    {
        if (CurrentState == EShipEntryState::NearEntryPoint || CurrentState == EShipEntryState::ApproachingShip)
        {
            SetEntryState(EShipEntryState::None);
        }
        bInInteractionRange = false;
        return;
    }

    // Update state based on proximity
    if (DistanceToShip < ProximityCheckRadius && CurrentState == EShipEntryState::None)
    {
        SetEntryState(EShipEntryState::ApproachingShip);
        CurrentPlayer = PlayerPawn;
    }

    // Find nearest entry point
    FShipEntryPoint NearestPoint = GetNearestEntryPoint(PlayerLocation);
    float DistanceToEntry = GetDistanceToNearestEntry(PlayerLocation);

    // Check if within interaction range
    if (DistanceToEntry < NearestPoint.InteractionRadius)
    {
        if (!bInInteractionRange)
        {
            bInInteractionRange = true;
            NearestEntryPointID = NearestPoint.EntryPointID;
            DistanceToNearestEntry = DistanceToEntry;
            SetEntryState(EShipEntryState::NearEntryPoint);
            ShowInteractionPrompt(NearestPoint);
        }
    }
    else
    {
        if (bInInteractionRange)
        {
            bInInteractionRange = false;
            NearestEntryPointID = NAME_None;
            SetEntryState(EShipEntryState::ApproachingShip);
            HideInteractionPrompt();
        }
    }
}

void UShipEntryComponent::UpdateTransition(float DeltaTime)
{
    if (!bIsTransitioning || !CurrentPlayer)
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    float ElapsedTime = CurrentTime - TransitionStartTime;
    TransitionProgress = FMath::Clamp(ElapsedTime / TransitionDuration, 0.0f, 1.0f);

    // Smooth transition curve
    float Alpha = bUseSmoothTransition ? FMath::SmoothStep(0.0f, 1.0f, TransitionProgress) : TransitionProgress;

    // Interpolate position and rotation
    FVector NewLocation = FMath::Lerp(TransitionStartLocation, TransitionTargetLocation, Alpha);
    FRotator NewRotation = FMath::Lerp(TransitionStartRotation, TransitionTargetRotation, Alpha);

    CurrentPlayer->SetActorLocation(NewLocation);
    CurrentPlayer->SetActorRotation(NewRotation);

    // Complete transition
    if (TransitionProgress >= 1.0f)
    {
        bIsTransitioning = false;
        CurrentPlayer->SetActorEnableCollision(true);

        if (CurrentState == EShipEntryState::Entering)
        {
            SetEntryState(EShipEntryState::InsideShip);
            TransferPlayerControl(OriginalPlayerPawn, OwningShip);
        }
        else if (CurrentState == EShipEntryState::Exiting)
        {
            SetEntryState(EShipEntryState::ExitedShip);
            TransferPlayerControl(OwningShip, OriginalPlayerPawn);
        }
    }
}

void UShipEntryComponent::ProcessPlayerInput()
{
    // Input processing handled by PlayerController
}

void UShipEntryComponent::HandleEntryTransition(APawn* Player, const FShipEntryPoint& EntryPoint)
{
    if (!Player || !OwningShip)
    {
        return;
    }

    // Setup transition
    bIsTransitioning = true;
    TransitionStartTime = GetWorld()->GetTimeSeconds();
    TransitionProgress = 0.0f;
    TransitionStartLocation = Player->GetActorLocation();
    TransitionStartRotation = Player->GetActorRotation();

    // Calculate target location (cockpit position)
    FVector CockpitLocation = OwningShip->GetActorTransform().TransformPosition(EntryPoint.LocalLocation);
    FRotator CockpitRotation = OwningShip->GetActorRotation() + EntryPoint.LocalRotation;

    TransitionTargetLocation = CockpitLocation;
    TransitionTargetRotation = CockpitRotation;

    // Disable collision during transition
    Player->SetActorEnableCollision(false);

    UE_LOG(LogTemp, Log, TEXT("ShipEntryComponent: Starting entry transition from %s to %s"), 
        *TransitionStartLocation.ToString(), *TransitionTargetLocation.ToString());
}

void UShipEntryComponent::HandleExitTransition(APawn* Player, const FShipEntryPoint& ExitPoint)
{
    if (!Player || !OwningShip)
    {
        return;
    }

    // Setup transition
    bIsTransitioning = true;
    TransitionStartTime = GetWorld()->GetTimeSeconds();
    TransitionProgress = 0.0f;
    TransitionStartLocation = Player->GetActorLocation();
    TransitionStartRotation = Player->GetActorRotation();

    // Calculate target location (exit point position)
    FVector ExitLocation = OwningShip->GetActorTransform().TransformPosition(ExitPoint.LocalLocation);
    FRotator ExitRotation = OwningShip->GetActorRotation() + ExitPoint.LocalRotation;

    // Offset exit location to be outside ship
    ExitLocation += OwningShip->GetActorForwardVector() * 300.0f;

    TransitionTargetLocation = ExitLocation;
    TransitionTargetRotation = ExitRotation;

    // Disable collision during transition
    Player->SetActorEnableCollision(false);

    UE_LOG(LogTemp, Log, TEXT("ShipEntryComponent: Starting exit transition from %s to %s"), 
        *TransitionStartLocation.ToString(), *TransitionTargetLocation.ToString());
}

void UShipEntryComponent::TransferPlayerControl(APawn* FromPawn, APawn* ToPawn)
{
    if (!PlayerController || !ToPawn)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ShipEntryComponent: Transferring control from %s to %s"), 
        FromPawn ? *FromPawn->GetName() : TEXT("None"),
        *ToPawn->GetName());

    // Possess new pawn
    PlayerController->Possess(ToPawn);

    // Setup camera view
    if (CurrentState == EShipEntryState::InsideShip)
    {
        SetupCockpitView(ToPawn);
    }
    else
    {
        SetupExteriorView(ToPawn);
    }
}

void UShipEntryComponent::SetupCockpitView(APawn* Player)
{
    if (!Player || !OwningShip)
    {
        return;
    }

    // Get VR camera if available
    UCameraComponent* Camera = OwningShip->FindComponentByClass<UCameraComponent>();
    if (Camera)
    {
        Camera->SetActive(true);
    }
}

void UShipEntryComponent::SetupExteriorView(APawn* Player)
{
    if (!Player)
    {
        return;
    }

    // Setup third person camera
    UCameraComponent* Camera = Player->FindComponentByClass<UCameraComponent>();
    if (Camera)
    {
        Camera->SetActive(true);
    }
}

bool UShipEntryComponent::ValidateEntry(APawn* Player, const FShipEntryPoint& EntryPoint) const
{
    if (!Player || !OwningShip)
    {
        return false;
    }

    // Check if entry point is locked
    if (EntryPoint.bIsLocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShipEntryComponent: Entry point is locked"));
        return false;
    }

    // Check if player is in range
    FVector PlayerLocation = Player->GetActorLocation();
    FVector EntryLocation = OwningShip->GetActorTransform().TransformPosition(EntryPoint.LocalLocation);
    float Distance = FVector::Dist(PlayerLocation, EntryLocation);

    if (Distance > EntryPoint.InteractionRadius)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShipEntryComponent: Player too far from entry point (%.2f > %.2f)"), 
            Distance, EntryPoint.InteractionRadius);
        return false;
    }

    // Check if spacesuit is required
    if (EntryPoint.bRequiresSpacesuit)
    {
        // Check player equipment for spacesuit
        // In a real implementation, this would check the player's inventory or equipment component

        // Method 1: Check for equipment component on player
        bool bHasSpacesuit = false;

        // Try to find equipment component or inventory component
        ACharacter* PlayerCharacter = Cast<ACharacter>(Player);
        if (PlayerCharacter)
        {
            // Check if player has spacesuit equipped
            // This would integrate with an inventory/equipment system
            // For demonstration, we'll check for a component by class name

            TArray<UActorComponent*> Components;
            PlayerCharacter->GetComponents(Components);

            for (UActorComponent* Component : Components)
            {
                // Check for equipment indicators
                if (Component->GetName().Contains(TEXT("Spacesuit")) ||
                    Component->GetName().Contains(TEXT("EVA")))
                {
                    bHasSpacesuit = true;
                    break;
                }
            }
        }

        if (!bHasSpacesuit)
        {
            UE_LOG(LogTemp, Warning, TEXT("ShipEntryComponent: Player needs spacesuit to enter via %s"),
                *EntryPoint.EntryPointID.ToString());

            // In a real implementation, display UI message to player
            // OnEntryRequirementFailed.Broadcast(Player, "Spacesuit Required");

            return false;
        }

        UE_LOG(LogTemp, Log, TEXT("ShipEntryComponent: Player has required spacesuit"));
    }

    return true;
}

bool UShipEntryComponent::ValidateExit(APawn* Player, const FShipEntryPoint& ExitPoint) const
{
    if (!Player || !OwningShip)
    {
        return false;
    }

    // Check if exit point is locked
    if (ExitPoint.bIsLocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShipEntryComponent: Exit point is locked"));
        return false;
    }

    // Check ship velocity
    float Velocity = OwningShip->GetVelocity().Size();
    if (Velocity > 100.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShipEntryComponent: Ship moving too fast to exit (%.2f)"), Velocity);
        return false;
    }

    return true;
}
