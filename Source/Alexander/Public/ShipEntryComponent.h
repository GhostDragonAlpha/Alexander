// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "ShipEntryComponent.generated.h"

// Forward declarations
class ASpaceship;
class APawn;
class APlayerController;
class UInputComponent;

/**
 * Entry Point Type
 */
UENUM(BlueprintType)
enum class EEntryPointType : uint8
{
    Cockpit UMETA(DisplayName = "Cockpit Entry"),
    Hatch UMETA(DisplayName = "Hatch Entry"),
    Airlock UMETA(DisplayName = "Airlock Entry"),
    Cargo UMETA(DisplayName = "Cargo Bay"),
    Emergency UMETA(DisplayName = "Emergency Exit")
};

/**
 * Entry State
 */
UENUM(BlueprintType)
enum class EShipEntryState : uint8
{
    None UMETA(DisplayName = "None"),
    ApproachingShip UMETA(DisplayName = "Approaching Ship"),
    NearEntryPoint UMETA(DisplayName = "Near Entry Point"),
    Entering UMETA(DisplayName = "Entering"),
    InsideShip UMETA(DisplayName = "Inside Ship"),
    Exiting UMETA(DisplayName = "Exiting"),
    ExitedShip UMETA(DisplayName = "Exited Ship")
};

/**
 * Entry Point Definition
 */
USTRUCT(BlueprintType)
struct FShipEntryPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entry Point")
    FName EntryPointID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entry Point")
    EEntryPointType EntryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entry Point")
    FVector LocalLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entry Point")
    FRotator LocalRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entry Point")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entry Point")
    bool bRequiresSpacesuit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entry Point")
    bool bIsLocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entry Point")
    FText InteractionPrompt;

    FShipEntryPoint()
        : EntryType(EEntryPointType::Cockpit)
        , LocalLocation(FVector::ZeroVector)
        , LocalRotation(FRotator::ZeroRotator)
        , InteractionRadius(200.0f)
        , bRequiresSpacesuit(false)
        , bIsLocked(false)
        , InteractionPrompt(FText::FromString("Press E to Enter Ship"))
    {
    }
};

/**
 * Event Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerEnteredShip, APawn*, Player, ASpaceship*, Ship);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerExitedShip, APawn*, Player, ASpaceship*, Ship);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntryStateChanged, EShipEntryState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionAvailable, const FShipEntryPoint&, EntryPoint, float, Distance);

/**
 * Ship Entry Component
 * Handles player entry/exit from spacecraft with smooth transitions
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UShipEntryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UShipEntryComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Entry/Exit Management
    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    void EnterShip(APawn* Player, const FName& EntryPointID);

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    void ExitShip(APawn* Player, const FName& ExitPointID);

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    bool CanEnterShip(APawn* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    bool CanExitShip(APawn* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    bool IsPlayerInShip() const;

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    APawn* GetCurrentPlayer() const { return CurrentPlayer; }

    // Entry Point Management
    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    void RegisterEntryPoint(const FShipEntryPoint& EntryPoint);

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    void RemoveEntryPoint(const FName& EntryPointID);

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    FShipEntryPoint GetEntryPoint(const FName& EntryPointID) const;

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    TArray<FShipEntryPoint> GetAllEntryPoints() const;

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    FShipEntryPoint GetNearestEntryPoint(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    float GetDistanceToNearestEntry(const FVector& Location) const;

    // State Management
    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    EShipEntryState GetEntryState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    void SetEntryState(EShipEntryState NewState);

    // Interaction
    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    void EnableInteraction(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    bool IsInteractionEnabled() const { return bInteractionEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    void ShowInteractionPrompt(const FShipEntryPoint& EntryPoint);

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    void HideInteractionPrompt();

    // VR Support
    UFUNCTION(BlueprintCallable, Category = "Ship Entry|VR")
    void EnableVRInteraction(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Ship Entry|VR")
    bool IsVRInteractionEnabled() const { return bVRInteractionEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Ship Entry|VR")
    void HandleVRGrab(class UMotionControllerComponent* Controller);

    UFUNCTION(BlueprintCallable, Category = "Ship Entry|VR")
    void HandleVRRelease(class UMotionControllerComponent* Controller);

    // Transition Control
    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    void SetTransitionDuration(float Duration);

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    float GetTransitionDuration() const { return TransitionDuration; }

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    bool IsTransitioning() const;

    UFUNCTION(BlueprintCallable, Category = "Ship Entry")
    float GetTransitionProgress() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Ship Entry|Events")
    FOnPlayerEnteredShip OnPlayerEnteredShip;

    UPROPERTY(BlueprintAssignable, Category = "Ship Entry|Events")
    FOnPlayerExitedShip OnPlayerExitedShip;

    UPROPERTY(BlueprintAssignable, Category = "Ship Entry|Events")
    FOnEntryStateChanged OnEntryStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Ship Entry|Events")
    FOnInteractionAvailable OnInteractionAvailable;

protected:
    // Internal Functions
    void UpdateProximityDetection(float DeltaTime);
    void UpdateTransition(float DeltaTime);
    void ProcessPlayerInput();
    void HandleEntryTransition(APawn* Player, const FShipEntryPoint& EntryPoint);
    void HandleExitTransition(APawn* Player, const FShipEntryPoint& ExitPoint);
    void TransferPlayerControl(APawn* FromPawn, APawn* ToPawn);
    void SetupCockpitView(APawn* Player);
    void SetupExteriorView(APawn* Player);
    bool ValidateEntry(APawn* Player, const FShipEntryPoint& EntryPoint) const;
    bool ValidateExit(APawn* Player, const FShipEntryPoint& ExitPoint) const;

    // Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entry Points")
    TMap<FName, FShipEntryPoint> EntryPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EShipEntryState CurrentState;

    UPROPERTY()
    APawn* CurrentPlayer;

    UPROPERTY()
    APawn* OriginalPlayerPawn;

    UPROPERTY()
    ASpaceship* OwningShip;

    UPROPERTY()
    APlayerController* PlayerController;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Entry|Config")
    float ProximityCheckRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Entry|Config")
    float InteractionCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Entry|Config")
    float TransitionDuration;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Entry|Config")
    bool bUseSmoothTransition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Entry|Config")
    bool bInteractionEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Entry|Config")
    bool bVRInteractionEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Entry|Config")
    bool bRequireInputToEnter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Entry|Config")
    bool bAutoExitOnLanding;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Entry|Config")
    FName DefaultEntryPointID;

    // Transition State
    bool bIsTransitioning;
    float TransitionStartTime;
    float TransitionProgress;
    FVector TransitionStartLocation;
    FVector TransitionTargetLocation;
    FRotator TransitionStartRotation;
    FRotator TransitionTargetRotation;

    // Proximity Tracking
    float LastProximityCheckTime;
    FName NearestEntryPointID;
    float DistanceToNearestEntry;
    bool bInInteractionRange;

    // Input Handling
    bool bInputSetup;
    FName EntryInputAction;
    FName ExitInputAction;
};
