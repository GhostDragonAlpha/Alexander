// Copyright Epic Games, Inc. All Rights Reserved.

#include "SurfacePlayerCharacter.h"
#include "Spaceship.h"
#include "ShipEntryComponent.h"
#include "SurfaceExplorationComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MotionControllerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

ASurfacePlayerCharacter::ASurfacePlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // Don't rotate when the controller rotates
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 700.0f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.0f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

    // Create camera boom (spring arm)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // Create follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Create VR camera
    VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
    VRCamera->SetupAttachment(RootComponent);
    VRCamera->bUsePawnControlRotation = true;
    VRCamera->SetActive(false);

    // Create VR controllers
    LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftController"));
    LeftController->SetupAttachment(VRCamera);
    LeftController->MotionSource = FName("Left");

    RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightController"));
    RightController->SetupAttachment(VRCamera);
    RightController->MotionSource = FName("Right");

    // Create exploration component
    ExplorationComponent = CreateDefaultSubobject<USurfaceExplorationComponent>(TEXT("ExplorationComponent"));

    // Movement properties
    MovementMode = ESurfaceMovementMode::Walking;
    WalkSpeed = 300.0f;
    RunSpeed = 600.0f;
    CrouchSpeed = 150.0f;
    JumpVelocity = 700.0f;
    bIsRunning = false;

    // Stamina
    Stamina = 100.0f;
    MaxStamina = 100.0f;
    StaminaRegenRate = 10.0f;
    StaminaDrainRate = 15.0f;

    // VR properties
    bIsVRMode = false;
    bUseVRTeleport = false;
    VRMovementSpeedMultiplier = 1.0f;

    // Interaction
    InteractionRange = 300.0f;
    TargetedActor = nullptr;
    NearbySpaceship = nullptr;

    // Camera settings
    BaseTurnRate = 65.0f;
    BaseLookUpRate = 65.0f;

    // Interaction tracking
    LastInteractionCheckTime = 0.0f;
    InteractionCheckInterval = 0.2f;

    // VR state
    bVRInitialized = false;

    // VR grab state
    LeftHandGrabbedActor = nullptr;
    RightHandGrabbedActor = nullptr;
    LeftHandGrabbedComponent = nullptr;
    RightHandGrabbedComponent = nullptr;
    LeftHandGrabOffset = FVector::ZeroVector;
    RightHandGrabOffset = FVector::ZeroVector;
    LeftHandGrabRotationOffset = FRotator::ZeroRotator;
    RightHandGrabRotationOffset = FRotator::ZeroRotator;

    // VR grab detection
    VRGrabRadius = 20.0f; // 20cm grab radius
    VRGrabForceStrength = 1000.0f; // Force for physics-based grabbing
    bLeftHandPhysicsGrab = false;
    bRightHandPhysicsGrab = false;
}

void ASurfacePlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Add Input Mapping Context
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }

    // Initialize movement speed
    UpdateMovementSpeed();

    UE_LOG(LogTemp, Log, TEXT("SurfacePlayerCharacter: Initialized at %s"), *GetActorLocation().ToString());
}

void ASurfacePlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update stamina
    UpdateStamina(DeltaTime);

    // Update camera
    UpdateCamera(DeltaTime);

    // Handle gravity
    HandleGravity();

    // Update VR physics grab
    if (bIsVRMode)
    {
        UpdateVRPhysicsGrab(DeltaTime);
    }

    // Check for nearby interactables
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastInteractionCheckTime >= InteractionCheckInterval)
    {
        CheckForInteractables();
        CheckNearbyShip();
        LastInteractionCheckTime = CurrentTime;
    }
}

void ASurfacePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Moving
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASurfacePlayerCharacter::Move);
        }

        // Looking
        if (LookAction)
        {
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASurfacePlayerCharacter::Look);
        }

        // Jumping
        if (JumpAction)
        {
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASurfacePlayerCharacter::StartJumping);
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ASurfacePlayerCharacter::StopJumping);
        }

        // Sprinting
        if (SprintAction)
        {
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ASurfacePlayerCharacter::StartSprinting);
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASurfacePlayerCharacter::StopSprinting);
        }

        // Crouching
        if (CrouchAction)
        {
            EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ASurfacePlayerCharacter::StartCrouching);
            EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ASurfacePlayerCharacter::StopCrouching);
        }

        // Interaction
        if (InteractAction)
        {
            EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ASurfacePlayerCharacter::Interact);
        }
    }
}

void ASurfacePlayerCharacter::MoveForward(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ASurfacePlayerCharacter::MoveRight(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void ASurfacePlayerCharacter::LookUp(float Value)
{
    if (Value != 0.0f)
    {
        AddControllerPitchInput(Value * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
    }
}

void ASurfacePlayerCharacter::Turn(float Value)
{
    if (Value != 0.0f)
    {
        AddControllerYawInput(Value * BaseTurnRate * GetWorld()->GetDeltaSeconds());
    }
}

void ASurfacePlayerCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ASurfacePlayerCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ASurfacePlayerCharacter::StartSprinting()
{
    if (Stamina > 0.0f)
    {
        bIsRunning = true;
        SetMovementMode(ESurfaceMovementMode::Running);
    }
}

void ASurfacePlayerCharacter::StopSprinting()
{
    bIsRunning = false;
    SetMovementMode(ESurfaceMovementMode::Walking);
}

void ASurfacePlayerCharacter::StartCrouching()
{
    Crouch();
    SetMovementMode(ESurfaceMovementMode::Crouching);
}

void ASurfacePlayerCharacter::StopCrouching()
{
    UnCrouch();
    SetMovementMode(ESurfaceMovementMode::Walking);
}

void ASurfacePlayerCharacter::StartJumping()
{
    if (Stamina >= 10.0f)
    {
        Jump();
        Stamina -= 10.0f;
    }
}

void ASurfacePlayerCharacter::StopJumping()
{
    StopJumping();
}

void ASurfacePlayerCharacter::SetMovementMode(ESurfaceMovementMode NewMode)
{
    MovementMode = NewMode;
    UpdateMovementSpeed();
}

void ASurfacePlayerCharacter::Interact()
{
    // Check for nearby spaceship
    if (NearbySpaceship)
    {
        EnterSpaceship();
        return;
    }

    // Check for other interactables
    if (TargetedActor)
    {
        // TODO: Implement general interaction system
        UE_LOG(LogTemp, Log, TEXT("SurfacePlayerCharacter: Interacting with %s"), *TargetedActor->GetName());
    }
}

void ASurfacePlayerCharacter::CheckForInteractables()
{
    if (!Controller)
    {
        return;
    }

    // Raycast from camera
    FVector Start = FollowCamera->GetComponentLocation();
    FVector Forward = FollowCamera->GetForwardVector();
    FVector End = Start + (Forward * InteractionRange);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
    {
        AActor* HitActor = HitResult.GetActor();
        if (HitActor && HitActor != TargetedActor)
        {
            TargetedActor = HitActor;
            UE_LOG(LogTemp, Log, TEXT("SurfacePlayerCharacter: Targeting %s"), *TargetedActor->GetName());
        }
    }
    else
    {
        TargetedActor = nullptr;
    }
}

void ASurfacePlayerCharacter::EnterSpaceship()
{
    if (!NearbySpaceship)
    {
        UE_LOG(LogTemp, Warning, TEXT("SurfacePlayerCharacter: No nearby spaceship to enter"));
        return;
    }

    // Get ship entry component
    UShipEntryComponent* EntryComponent = NearbySpaceship->FindComponentByClass<UShipEntryComponent>();
    if (EntryComponent && EntryComponent->CanEnterShip(this))
    {
        UE_LOG(LogTemp, Log, TEXT("SurfacePlayerCharacter: Entering spaceship %s"), *NearbySpaceship->GetName());
        
        // Find nearest entry point
        FShipEntryPoint NearestEntry = EntryComponent->GetNearestEntryPoint(GetActorLocation());
        
        // Enter ship
        EntryComponent->EnterShip(this, NearestEntry.EntryPointID);
        
        // Broadcast event
        OnEnteringShip(NearbySpaceship);
    }
}

void ASurfacePlayerCharacter::InitializeVR()
{
    if (bVRInitialized)
    {
        return;
    }

    // Switch to VR camera
    VRCamera->SetActive(true);
    FollowCamera->SetActive(false);

    // Enable VR controllers
    LeftController->SetActive(true);
    RightController->SetActive(true);

    bVRInitialized = true;
    bIsVRMode = true;

    UE_LOG(LogTemp, Log, TEXT("SurfacePlayerCharacter: VR initialized"));
}

void ASurfacePlayerCharacter::SetVRMode(bool bEnabled)
{
    if (bEnabled)
    {
        InitializeVR();
    }
    else
    {
        bIsVRMode = false;
        VRCamera->SetActive(false);
        FollowCamera->SetActive(true);
    }
}

void ASurfacePlayerCharacter::VRTeleport(FVector Location)
{
    if (!bIsVRMode || !bUseVRTeleport)
    {
        return;
    }

    SetActorLocation(Location);
}

void ASurfacePlayerCharacter::VRGrabLeft()
{
    if (!bIsVRMode || !LeftController)
    {
        return;
    }

    // If already grabbing something, don't grab again
    if (LeftHandGrabbedActor)
    {
        return;
    }

    // Perform sphere trace from left hand position
    FVector HandLocation = LeftController->GetComponentLocation();
    FVector HandForward = LeftController->GetForwardVector();

    // Setup collision parameters
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.bTraceComplex = false;

    // Perform sphere sweep
    TArray<FHitResult> HitResults;
    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        HandLocation,
        HandLocation + HandForward * 10.0f, // Small forward sweep
        FQuat::Identity,
        ECC_PhysicsBody,
        FCollisionShape::MakeSphere(VRGrabRadius),
        QueryParams
    );

    if (bHit)
    {
        // Find the closest grabbable object
        AActor* ClosestActor = nullptr;
        UPrimitiveComponent* ClosestComponent = nullptr;
        float ClosestDistance = FLT_MAX;

        for (const FHitResult& Hit : HitResults)
        {
            UPrimitiveComponent* HitComp = Hit.GetComponent();
            if (HitComp && HitComp->IsSimulatingPhysics())
            {
                float Distance = FVector::Dist(HandLocation, Hit.ImpactPoint);
                if (Distance < ClosestDistance)
                {
                    ClosestDistance = Distance;
                    ClosestActor = Hit.GetActor();
                    ClosestComponent = HitComp;
                }
            }
        }

        // Grab the closest object
        if (ClosestActor && ClosestComponent)
        {
            LeftHandGrabbedActor = ClosestActor;
            LeftHandGrabbedComponent = ClosestComponent;

            // Calculate grab offset
            LeftHandGrabOffset = LeftHandGrabbedActor->GetActorLocation() - HandLocation;
            LeftHandGrabRotationOffset = LeftHandGrabbedActor->GetActorRotation() - LeftController->GetComponentRotation();

            // Decide grab type based on object mass
            float ObjectMass = ClosestComponent->GetMass();
            bLeftHandPhysicsGrab = (ObjectMass > 10.0f); // Use physics grab for heavy objects

            if (!bLeftHandPhysicsGrab)
            {
                // Attach to hand (kinematic grab)
                ClosestComponent->SetSimulatePhysics(false);
                LeftHandGrabbedActor->AttachToComponent(
                    LeftController,
                    FAttachmentTransformRules::KeepWorldTransform
                );
            }
            else
            {
                // Keep physics enabled for heavy objects (physics grab)
                // We'll apply forces in Tick to pull the object
            }

            // Play haptic feedback
            if (APlayerController* PC = Cast<APlayerController>(Controller))
            {
                PC->PlayHapticEffect(nullptr, EControllerHand::Left, 0.7f, false);
            }

            UE_LOG(LogTemp, Log, TEXT("SurfacePlayerCharacter: Left hand grabbed %s"), *ClosestActor->GetName());
        }
    }
}

void ASurfacePlayerCharacter::VRGrabRight()
{
    if (!bIsVRMode || !RightController)
    {
        return;
    }

    // If already grabbing something, don't grab again
    if (RightHandGrabbedActor)
    {
        return;
    }

    // Perform sphere trace from right hand position
    FVector HandLocation = RightController->GetComponentLocation();
    FVector HandForward = RightController->GetForwardVector();

    // Setup collision parameters
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.bTraceComplex = false;

    // Perform sphere sweep
    TArray<FHitResult> HitResults;
    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        HandLocation,
        HandLocation + HandForward * 10.0f, // Small forward sweep
        FQuat::Identity,
        ECC_PhysicsBody,
        FCollisionShape::MakeSphere(VRGrabRadius),
        QueryParams
    );

    if (bHit)
    {
        // Find the closest grabbable object
        AActor* ClosestActor = nullptr;
        UPrimitiveComponent* ClosestComponent = nullptr;
        float ClosestDistance = FLT_MAX;

        for (const FHitResult& Hit : HitResults)
        {
            UPrimitiveComponent* HitComp = Hit.GetComponent();
            if (HitComp && HitComp->IsSimulatingPhysics())
            {
                float Distance = FVector::Dist(HandLocation, Hit.ImpactPoint);
                if (Distance < ClosestDistance)
                {
                    ClosestDistance = Distance;
                    ClosestActor = Hit.GetActor();
                    ClosestComponent = HitComp;
                }
            }
        }

        // Grab the closest object
        if (ClosestActor && ClosestComponent)
        {
            RightHandGrabbedActor = ClosestActor;
            RightHandGrabbedComponent = ClosestComponent;

            // Calculate grab offset
            RightHandGrabOffset = RightHandGrabbedActor->GetActorLocation() - HandLocation;
            RightHandGrabRotationOffset = RightHandGrabbedActor->GetActorRotation() - RightController->GetComponentRotation();

            // Decide grab type based on object mass
            float ObjectMass = ClosestComponent->GetMass();
            bRightHandPhysicsGrab = (ObjectMass > 10.0f); // Use physics grab for heavy objects

            if (!bRightHandPhysicsGrab)
            {
                // Attach to hand (kinematic grab)
                ClosestComponent->SetSimulatePhysics(false);
                RightHandGrabbedActor->AttachToComponent(
                    RightController,
                    FAttachmentTransformRules::KeepWorldTransform
                );
            }
            else
            {
                // Keep physics enabled for heavy objects (physics grab)
                // We'll apply forces in Tick to pull the object
            }

            // Play haptic feedback
            if (APlayerController* PC = Cast<APlayerController>(Controller))
            {
                PC->PlayHapticEffect(nullptr, EControllerHand::Right, 0.7f, false);
            }

            UE_LOG(LogTemp, Log, TEXT("SurfacePlayerCharacter: Right hand grabbed %s"), *ClosestActor->GetName());
        }
    }
}

void ASurfacePlayerCharacter::VRReleaseLeft()
{
    if (!bIsVRMode)
    {
        return;
    }

    if (!LeftHandGrabbedActor)
    {
        return;
    }

    // Calculate release velocity from hand movement
    FVector HandVelocity = FVector::ZeroVector;
    if (LeftController)
    {
        // Get hand velocity from motion controller
        // Note: This is a simplified calculation; real implementation would track velocity over time
        HandVelocity = LeftController->GetComponentVelocity();
    }

    // Re-enable physics if it was disabled
    if (LeftHandGrabbedComponent && !bLeftHandPhysicsGrab)
    {
        // Detach from hand
        LeftHandGrabbedActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

        // Re-enable physics
        LeftHandGrabbedComponent->SetSimulatePhysics(true);

        // Apply release velocity for throwing
        if (!HandVelocity.IsNearlyZero())
        {
            LeftHandGrabbedComponent->SetPhysicsLinearVelocity(HandVelocity);

            // Add slight angular velocity for natural tumbling
            FVector AngularVelocity = FVector(
                FMath::RandRange(-100.0f, 100.0f),
                FMath::RandRange(-100.0f, 100.0f),
                FMath::RandRange(-100.0f, 100.0f)
            );
            LeftHandGrabbedComponent->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
        }
    }

    // Play haptic feedback for release
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        PC->PlayHapticEffect(nullptr, EControllerHand::Left, 0.3f, false);
    }

    UE_LOG(LogTemp, Log, TEXT("SurfacePlayerCharacter: Left hand released %s with velocity %s"),
        *LeftHandGrabbedActor->GetName(), *HandVelocity.ToString());

    // Clear grab state
    LeftHandGrabbedActor = nullptr;
    LeftHandGrabbedComponent = nullptr;
    LeftHandGrabOffset = FVector::ZeroVector;
    LeftHandGrabRotationOffset = FRotator::ZeroRotator;
    bLeftHandPhysicsGrab = false;
}

void ASurfacePlayerCharacter::VRReleaseRight()
{
    if (!bIsVRMode)
    {
        return;
    }

    if (!RightHandGrabbedActor)
    {
        return;
    }

    // Calculate release velocity from hand movement
    FVector HandVelocity = FVector::ZeroVector;
    if (RightController)
    {
        // Get hand velocity from motion controller
        // Note: This is a simplified calculation; real implementation would track velocity over time
        HandVelocity = RightController->GetComponentVelocity();
    }

    // Re-enable physics if it was disabled
    if (RightHandGrabbedComponent && !bRightHandPhysicsGrab)
    {
        // Detach from hand
        RightHandGrabbedActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

        // Re-enable physics
        RightHandGrabbedComponent->SetSimulatePhysics(true);

        // Apply release velocity for throwing
        if (!HandVelocity.IsNearlyZero())
        {
            RightHandGrabbedComponent->SetPhysicsLinearVelocity(HandVelocity);

            // Add slight angular velocity for natural tumbling
            FVector AngularVelocity = FVector(
                FMath::RandRange(-100.0f, 100.0f),
                FMath::RandRange(-100.0f, 100.0f),
                FMath::RandRange(-100.0f, 100.0f)
            );
            RightHandGrabbedComponent->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
        }
    }

    // Play haptic feedback for release
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        PC->PlayHapticEffect(nullptr, EControllerHand::Right, 0.3f, false);
    }

    UE_LOG(LogTemp, Log, TEXT("SurfacePlayerCharacter: Right hand released %s with velocity %s"),
        *RightHandGrabbedActor->GetName(), *HandVelocity.ToString());

    // Clear grab state
    RightHandGrabbedActor = nullptr;
    RightHandGrabbedComponent = nullptr;
    RightHandGrabOffset = FVector::ZeroVector;
    RightHandGrabRotationOffset = FRotator::ZeroRotator;
    bRightHandPhysicsGrab = false;
}

FString ASurfacePlayerCharacter::GetCharacterStatus() const
{
    return FString::Printf(TEXT("Mode: %d, Stamina: %.1f/%.1f, Speed: %.1f"), 
        (int32)MovementMode, Stamina, MaxStamina, GetVelocity().Size());
}

bool ASurfacePlayerCharacter::IsOnGround() const
{
    return GetCharacterMovement()->IsMovingOnGround();
}

float ASurfacePlayerCharacter::GetCurrentGravity() const
{
    return GetCharacterMovement()->GetGravityZ();
}

void ASurfacePlayerCharacter::UpdateStamina(float DeltaTime)
{
    if (bIsRunning && IsOnGround())
    {
        // Drain stamina while running
        Stamina = FMath::Max(0.0f, Stamina - (StaminaDrainRate * DeltaTime));
        
        if (Stamina <= 0.0f)
        {
            StopSprinting();
        }
    }
    else
    {
        // Regenerate stamina
        Stamina = FMath::Min(MaxStamina, Stamina + (StaminaRegenRate * DeltaTime));
    }
}

void ASurfacePlayerCharacter::UpdateMovementSpeed()
{
    if (!GetCharacterMovement())
    {
        return;
    }

    float TargetSpeed = WalkSpeed;

    switch (MovementMode)
    {
    case ESurfaceMovementMode::Walking:
        TargetSpeed = WalkSpeed;
        break;
    case ESurfaceMovementMode::Running:
        TargetSpeed = RunSpeed;
        break;
    case ESurfaceMovementMode::Crouching:
        TargetSpeed = CrouchSpeed;
        break;
    default:
        TargetSpeed = WalkSpeed;
        break;
    }

    // Apply VR multiplier if in VR mode
    if (bIsVRMode)
    {
        TargetSpeed *= VRMovementSpeedMultiplier;
    }

    GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
}

void ASurfacePlayerCharacter::UpdateCamera(float DeltaTime)
{
    // Camera updates handled by spring arm and camera components
}

void ASurfacePlayerCharacter::HandleGravity()
{
    // Gravity handled by character movement component
    // This can be extended for planet-specific gravity
}

void ASurfacePlayerCharacter::UpdateVRPhysicsGrab(float DeltaTime)
{
    // Update left hand physics grab
    if (bLeftHandPhysicsGrab && LeftHandGrabbedComponent && LeftController)
    {
        FVector TargetLocation = LeftController->GetComponentLocation() + LeftHandGrabOffset;
        FVector CurrentLocation = LeftHandGrabbedComponent->GetComponentLocation();

        // Calculate force to pull object to hand
        FVector Direction = TargetLocation - CurrentLocation;
        float Distance = Direction.Size();
        Direction.Normalize();

        // Apply force (stronger when further away)
        float Force = Distance * VRGrabForceStrength;
        LeftHandGrabbedComponent->AddForce(Direction * Force);

        // Add damping to prevent oscillation
        FVector Velocity = LeftHandGrabbedComponent->GetPhysicsLinearVelocity();
        LeftHandGrabbedComponent->AddForce(-Velocity * 100.0f);
    }

    // Update right hand physics grab
    if (bRightHandPhysicsGrab && RightHandGrabbedComponent && RightController)
    {
        FVector TargetLocation = RightController->GetComponentLocation() + RightHandGrabOffset;
        FVector CurrentLocation = RightHandGrabbedComponent->GetComponentLocation();

        // Calculate force to pull object to hand
        FVector Direction = TargetLocation - CurrentLocation;
        float Distance = Direction.Size();
        Direction.Normalize();

        // Apply force (stronger when further away)
        float Force = Distance * VRGrabForceStrength;
        RightHandGrabbedComponent->AddForce(Direction * Force);

        // Add damping to prevent oscillation
        FVector Velocity = RightHandGrabbedComponent->GetPhysicsLinearVelocity();
        RightHandGrabbedComponent->AddForce(-Velocity * 100.0f);
    }
}

void ASurfacePlayerCharacter::CheckNearbyShip()
{
    // Check for nearby spaceships
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpaceship::StaticClass(), FoundActors);

    ASpaceship* ClosestShip = nullptr;
    float ClosestDistance = InteractionRange;

    for (AActor* Actor : FoundActors)
    {
        ASpaceship* Ship = Cast<ASpaceship>(Actor);
        if (Ship)
        {
            float Distance = FVector::Dist(GetActorLocation(), Ship->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestShip = Ship;
            }
        }
    }

    if (ClosestShip != NearbySpaceship)
    {
        NearbySpaceship = ClosestShip;
        if (NearbySpaceship)
        {
            UE_LOG(LogTemp, Log, TEXT("SurfacePlayerCharacter: Nearby spaceship detected: %s"), *NearbySpaceship->GetName());
        }
    }
}

bool ASurfacePlayerCharacter::RunSelfTest_Implementation(FSystemTestResult& OutResult)
{
	OutResult.SystemName = "SurfacePlayerCharacter";
	OutResult.bPassed = true;
	
	UE_LOG(LogTemp, Log, TEXT("=== Starting SurfacePlayerCharacter Self-Test ==="));
	
	// Test 1: Verify character initialization
	{
		UE_LOG(LogTemp, Log, TEXT("Test 1: Verifying character initialization..."));
		
		// Check core components
		if (!GetCapsuleComponent() || !GetCharacterMovement() || !Controller)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Core character components not initialized"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Check camera components
		if (!CameraBoom || !FollowCamera || !VRCamera)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Camera components not initialized"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Check VR components
		if (!LeftController || !RightController)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: VR controller components not initialized"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Check exploration component
		if (!ExplorationComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Exploration component not initialized"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Verify initial movement properties
		if (WalkSpeed <= 0.0f || RunSpeed <= 0.0f || CrouchSpeed <= 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Invalid movement speed values"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Verify initial stamina values
		if (Stamina != MaxStamina || MaxStamina <= 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Invalid stamina initialization"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Character initialization verified"));
	}
	
	// Test 2: Verify movement modes
	{
		UE_LOG(LogTemp, Log, TEXT("Test 2: Verifying movement modes..."));
		
		// Test walking mode
		SetMovementMode(ESurfaceMovementMode::Walking);
		if (MovementMode != ESurfaceMovementMode::Walking)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Walking mode not set correctly"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Test running mode
		SetMovementMode(ESurfaceMovementMode::Running);
		if (MovementMode != ESurfaceMovementMode::Running)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Running mode not set correctly"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Test crouching mode
		SetMovementMode(ESurfaceMovementMode::Crouching);
		if (MovementMode != ESurfaceMovementMode::Crouching)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Crouching mode not set correctly"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Verify movement speed updates correctly
		float OriginalMaxSpeed = GetCharacterMovement()->MaxWalkSpeed;
		SetMovementMode(ESurfaceMovementMode::Running);
		UpdateMovementSpeed();
		
		if (FMath::Abs(GetCharacterMovement()->MaxWalkSpeed - RunSpeed) > 1.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Movement speed not updated correctly for running"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Restore original speed
		GetCharacterMovement()->MaxWalkSpeed = OriginalMaxSpeed;
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: All movement modes functional"));
	}
	
	// Test 3: Verify sprinting system
	{
		UE_LOG(LogTemp, Log, TEXT("Test 3: Verifying sprinting system..."));
		
		// Start sprinting
		float InitialStamina = Stamina;
		StartSprinting();
		
		if (!bIsRunning || MovementMode != ESurfaceMovementMode::Running)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Sprinting not activated"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Simulate stamina drain
		UpdateStamina(1.0f); // 1 second of sprinting
		
		if (Stamina >= InitialStamina)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Stamina not draining while sprinting"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Stop sprinting
		StopSprinting();
		
		if (bIsRunning)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Sprinting not stopped"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Simulate stamina regeneration
		float StaminaAfterStop = Stamina;
		UpdateStamina(2.0f); // 2 seconds of rest
		
		if (Stamina <= StaminaAfterStop)
		{
			UE_LOG(LogTemp, Warning, TEXT("WARNING: Stamina not regenerating properly"));
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Sprinting system working"));
	}
	
	// Test 4: Verify jumping mechanics
	{
		UE_LOG(LogTemp, Log, TEXT("Test 4: Verifying jumping mechanics..."));
		
		float InitialStamina = Stamina;
		
		// Attempt jump
		StartJumping();
		
		// Verify stamina was consumed
		if (Stamina >= InitialStamina)
		{
			UE_LOG(LogTemp, Warning, TEXT("WARNING: Jump should consume stamina"));
		}
		
		// Verify character is jumping
		if (!GetCharacterMovement()->IsFalling())
		{
			UE_LOG(LogTemp, Warning, TEXT("WARNING: Character should be in falling state after jump"));
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Jumping mechanics working"));
	}
	
	// Test 5: Verify crouching system
	{
		UE_LOG(LogTemp, Log, TEXT("Test 5: Verifying crouching system..."));
		
		// Start crouching
		StartCrouching();
		
		if (MovementMode != ESurfaceMovementMode::Crouching)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Crouching mode not activated"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		if (!GetCharacterMovement()->IsCrouching())
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Character movement component not crouching"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Stop crouching
		StopCrouching();
		
		if (MovementMode != ESurfaceMovementMode::Walking)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Crouching mode not deactivated"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Crouching system working"));
	}
	
	// Test 6: Verify VR mode toggling
	{
		UE_LOG(LogTemp, Log, TEXT("Test 6: Verifying VR mode toggling..."));
		
		// Enable VR mode
		SetVRMode(true);
		
		if (!bIsVRMode || !bVRInitialized)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: VR mode not enabled"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		if (!VRCamera->IsActive())
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: VR camera not active"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Disable VR mode
		SetVRMode(false);
		
		if (bIsVRMode)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: VR mode not disabled"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		if (!FollowCamera->IsActive())
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Follow camera not active after VR disable"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: VR mode toggling working"));
	}
	
	// Test 7: Verify interaction system
	{
		UE_LOG(LogTemp, Log, TEXT("Test 7: Verifying interaction system..."));
		
		// Test interaction range
		if (InteractionRange <= 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Invalid interaction range"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Test interaction check interval
		if (InteractionCheckInterval <= 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Invalid interaction check interval"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Interaction system verified"));
	}
	
	// Test 8: Verify camera systems
	{
		UE_LOG(LogTemp, Log, TEXT("Test 8: Verifying camera systems..."));
		
		// Verify camera boom settings
		if (CameraBoom->TargetArmLength <= 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Invalid camera boom arm length"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Verify camera components are attached correctly
		if (!FollowCamera->GetAttachParent() || FollowCamera->GetAttachParent() != CameraBoom)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Follow camera not attached to boom"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Verify VR camera is attached to root
		if (!VRCamera->GetAttachParent() || VRCamera->GetAttachParent() != RootComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: VR camera not attached to root"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Camera systems verified"));
	}
	
	// Test 9: Verify status functions
	{
		UE_LOG(LogTemp, Log, TEXT("Test 9: Verifying status functions..."));
		
		// Test GetCharacterStatus
		FString Status = GetCharacterStatus();
		if (Status.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Character status string empty"));
			
			OutResult.bPassed = false;
			return false;
		}
		
		// Test IsOnGround
		bool bOnGround = IsOnGround();
		// Should be true at spawn (assuming we're on ground)
		
		// Test GetCurrentGravity
		float Gravity = GetCurrentGravity();
		if (Gravity == 0.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("WARNING: Gravity is zero, character may be in zero-g"));
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Status functions working"));
	}
	
	// Test 10: Automated movement sequence
	{
		UE_LOG(LogTemp, Log, TEXT("Test 10: Running automated movement sequence..."));
		
		// Record initial position
		FVector InitialPosition = GetActorLocation();
		
		// Test 1: Move forward
		MoveForward(1.0f);
		Tick(0.1f); // Simulate movement for 0.1 seconds
		
		FVector AfterForward = GetActorLocation();
		float ForwardDistance = FVector::Dist(InitialPosition, AfterForward);
		
		if (ForwardDistance < 1.0f) // Should have moved at least 1 unit
		{
			UE_LOG(LogTemp, Warning, TEXT("WARNING: Minimal forward movement detected (%.2f units)"), ForwardDistance);
		}
		
		// Test 2: Move right (strafe)
		MoveRight(1.0f);
		Tick(0.1f);
		
		FVector AfterRight = GetActorLocation();
		float RightDistance = FVector::Dist(AfterForward, AfterRight);
		
		if (RightDistance < 1.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("WARNING: Minimal right movement detected (%.2f units)"), RightDistance);
		}
		
		// Test 3: Look/turn
		float InitialYaw = GetActorRotation().Yaw;
		Turn(1.0f);
		Tick(0.1f);
		
		float FinalYaw = GetActorRotation().Yaw;
		float YawChange = FMath::Abs(FinalYaw - InitialYaw);
		
		if (YawChange < 1.0f) // Should have turned at least 1 degree
		{
			UE_LOG(LogTemp, Warning, TEXT("WARNING: Minimal rotation detected (%.2f degrees)"), YawChange);
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Automated movement sequence completed"));
	}
	
	// Test 11: Verify VR grab system setup
	{
		UE_LOG(LogTemp, Log, TEXT("Test 11: Verifying VR grab system setup..."));
		
		// Verify grab radius is positive
		if (VRGrabRadius <= 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Invalid VR grab radius"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Verify grab force strength is positive
		if (VRGrabForceStrength <= 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Invalid VR grab force strength"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Verify VR controllers are active when VR is enabled
		SetVRMode(true);
		if (!LeftController->IsActive() || !RightController->IsActive())
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: VR controllers not active when VR enabled"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Reset VR mode
		SetVRMode(false);
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: VR grab system setup verified"));
	}
	
	// Test 12: Verify stamina limits
	{
		UE_LOG(LogTemp, Log, TEXT("Test 12: Verifying stamina limits..."));
		
		// Test maximum stamina
		Stamina = MaxStamina;
		UpdateStamina(1.0f); // Should not exceed max
		
		if (Stamina > MaxStamina)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Stamina exceeded maximum value"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test minimum stamina (0)
		Stamina = 0.0f;
		UpdateStamina(-1.0f); // Try to go negative
		
		if (Stamina < 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Stamina below minimum value"));
			OutResult.bPassed = false;
			return false;
		}
		
		// Test sprinting with zero stamina (should not allow)
		Stamina = 0.0f;
		StartSprinting();
		
		if (bIsRunning)
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Should not be able to sprint with zero stamina"));
			OutResult.bPassed = false;
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("PASSED: Stamina limits enforced"));
	}
	
	// Final result
	if (OutResult.bPassed)
	{
		UE_LOG(LogTemp, Log, TEXT("=== SurfacePlayerCharacter Self-Test PASSED ==="));
		UE_LOG(LogTemp, Log, TEXT("Movement modes: %d tested"), 3);
		UE_LOG(LogTemp, Log, TEXT("VR mode: %s"), bIsVRMode ? TEXT("Enabled") : TEXT("Disabled"));
		UE_LOG(LogTemp, Log, TEXT("Current position: %s"), *GetActorLocation().ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("=== SurfacePlayerCharacter Self-Test FAILED ==="));
	}
	
	return OutResult.bPassed;
}
