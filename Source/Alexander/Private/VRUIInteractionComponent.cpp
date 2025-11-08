// Copyright Epic Games, Inc. All Rights Reserved.

#include "VRUIInteractionComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "MotionControllerComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

UVRUIInteractionComponent::UVRUIInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// Set reasonable tick interval for VR (target 90 FPS)
	PrimaryComponentTick.TickInterval = 0.0f; // Tick every frame

	// Initialize default haptic patterns
	HoverHapticPattern.Intensity = 0.3f;
	HoverHapticPattern.Duration = 0.1f;
	HoverHapticPattern.bLoop = false;

	PressHapticPattern.Intensity = 0.7f;
	PressHapticPattern.Duration = 0.15f;
	PressHapticPattern.bLoop = false;

	ReleaseHapticPattern.Intensity = 0.5f;
	ReleaseHapticPattern.Duration = 0.1f;
	ReleaseHapticPattern.bLoop = false;
}

void UVRUIInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize components
	InitializeWidgetInteraction();
	InitializeLaserPointer();
	FindMotionController();

	// Start with pointer visible
	ShowPointer();
}

void UVRUIInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Performance optimization - skip frames if configured
	if (!ShouldPerformRaycastThisFrame())
	{
		FrameCounter++;
		return;
	}

	FrameCounter = 0;

	// Perform raycast to detect widgets
	if (PerformRaycast())
	{
		UpdateHoverState();
		UpdateLaserPointerVisual();
	}
	else
	{
		// No widget hit - clear hover state
		if (CurrentHoveredWidget != nullptr)
		{
			OnHoverExit(CurrentHoveredWidget);
			CurrentHoveredWidget = nullptr;
		}
		UpdateLaserPointerVisual();
	}
}

// ============================================================================
// RAYCAST & HIT DETECTION
// ============================================================================

bool UVRUIInteractionComponent::PerformRaycast()
{
	if (!WidgetInteraction || !MotionController)
	{
		return false;
	}

	// Get raycast start and end positions
	FVector StartLocation = MotionController->GetComponentLocation();
	FVector ForwardVector = MotionController->GetForwardVector();
	FVector EndLocation = StartLocation + (ForwardVector * MaxInteractionDistance);

	// Perform widget interaction raycast (UE 5.6 API)
	const FHitResult& HitResult = WidgetInteraction->GetLastHitResult();
	bool bHit = HitResult.bBlockingHit;

	// Update pointer visualization
	FVector EndPoint = bHit ? FVector(HitResult.ImpactPoint) : EndLocation;
	UpdatePointerLocation(StartLocation, EndPoint);

	// Draw debug if enabled
	if (bShowDebugRays)
	{
		DrawDebugVisualization(StartLocation, EndPoint, bHit);
	}

	// Update interaction result
	LastInteractionResult.bSuccess = bHit;
	if (bHit)
	{
		LastInteractionResult.HitWidget = Cast<UWidgetComponent>(HitResult.GetComponent());
		LastInteractionResult.HitLocation = HitResult.ImpactPoint;
		LastInteractionResult.Distance = HitResult.Distance;

		// Update current hovered widget
		PreviousHoveredWidget = CurrentHoveredWidget;
		CurrentHoveredWidget = LastInteractionResult.HitWidget;

		// Track raycast time
		LastRaycastTime = GetWorld()->GetTimeSeconds();
	}
	else
	{
		LastInteractionResult.HitWidget = nullptr;
		LastInteractionResult.HitLocation = FVector::ZeroVector;
		LastInteractionResult.Distance = 0.0f;
	}

	return bHit;
}

// ============================================================================
// BUTTON PRESS & INTERACTION
// ============================================================================

void UVRUIInteractionComponent::HandleButtonPress()
{
	if (!bIsButtonPressed && WidgetInteraction && CurrentHoveredWidget)
	{
		bIsButtonPressed = true;
		LastInteractionResult.HoverState = EVRWidgetHoverState::Pressed;

		// Simulate press through widget interaction
		WidgetInteraction->PressPointerKey(EKeys::LeftMouseButton);

		// Play haptic feedback
		if (bEnableHapticFeedback)
		{
			PlayPressHaptics();
		}

		// Update pointer visual
		UpdateLaserPointerVisual();
	}
}

void UVRUIInteractionComponent::HandleButtonRelease()
{
	if (bIsButtonPressed && WidgetInteraction)
	{
		bIsButtonPressed = false;
		LastInteractionResult.HoverState = CurrentHoveredWidget ? EVRWidgetHoverState::Hovering : EVRWidgetHoverState::None;

		// Simulate release through widget interaction
		WidgetInteraction->ReleasePointerKey(EKeys::LeftMouseButton);

		// Play haptic feedback
		if (bEnableHapticFeedback)
		{
			PlayReleaseHaptics();
		}

		// Update pointer visual
		UpdateLaserPointerVisual();
	}
}

void UVRUIInteractionComponent::SimulateClick()
{
	if (WidgetInteraction && CurrentHoveredWidget)
	{
		HandleButtonPress();

		// Schedule release after a short delay
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			HandleButtonRelease();
		}, 0.1f, false);
	}
}

// ============================================================================
// POINTER VISUALIZATION
// ============================================================================

void UVRUIInteractionComponent::UpdatePointerLocation(const FVector& StartLocation, const FVector& EndLocation)
{
	if (LaserPointerComponent)
	{
		// Update laser pointer transform
		FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
		float Distance = FVector::Dist(StartLocation, EndLocation);

		LaserPointerComponent->SetWorldLocation(StartLocation);
		LaserPointerComponent->SetWorldRotation(Direction.Rotation());

		// Update Niagara parameters
		LaserPointerComponent->SetVectorParameter(FName("BeamEnd"), EndLocation);
		LaserPointerComponent->SetFloatParameter(FName("BeamLength"), Distance);
		LaserPointerComponent->SetFloatParameter(FName("BeamWidth"), PointerThickness);
	}
}

void UVRUIInteractionComponent::ShowPointer()
{
	bPointerVisible = true;
	if (LaserPointerComponent)
	{
		LaserPointerComponent->SetVisibility(true);
		LaserPointerComponent->Activate();
	}
}

void UVRUIInteractionComponent::HidePointer()
{
	bPointerVisible = false;
	if (LaserPointerComponent)
	{
		LaserPointerComponent->SetVisibility(false);
		LaserPointerComponent->Deactivate();
	}
}

void UVRUIInteractionComponent::SetPointerColor(FLinearColor NewColor)
{
	if (LaserPointerComponent)
	{
		LaserPointerComponent->SetColorParameter(FName("BeamColor"), NewColor);
	}
}

// ============================================================================
// HAPTIC FEEDBACK
// ============================================================================

void UVRUIInteractionComponent::TriggerHapticFeedback(const FVRHapticPattern& Pattern)
{
	if (!bEnableHapticFeedback || !MotionController)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return;
	}

	EControllerHand Hand = GetControllerHand();

	// Play haptic effect
	if (Pattern.HapticEffect)
	{
		PC->PlayHapticEffect(Pattern.HapticEffect, Hand, Pattern.Intensity, Pattern.bLoop);
	}
	else
	{
		// Use simple haptic feedback if no effect specified
		PlayHapticOnController(Pattern.Intensity, Pattern.Duration);
	}
}

void UVRUIInteractionComponent::PlayHoverHaptics()
{
	TriggerHapticFeedback(HoverHapticPattern);
}

void UVRUIInteractionComponent::PlayPressHaptics()
{
	TriggerHapticFeedback(PressHapticPattern);
}

void UVRUIInteractionComponent::PlayReleaseHaptics()
{
	TriggerHapticFeedback(ReleaseHapticPattern);
}

void UVRUIInteractionComponent::StopHapticFeedback()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		EControllerHand Hand = GetControllerHand();
		PC->StopHapticEffect(Hand);
	}
}

// ============================================================================
// HAND TRACKING
// ============================================================================

void UVRUIInteractionComponent::SetInteractionHand(EVRInteractionHand NewHand)
{
	InteractionHand = NewHand;

	// Re-find motion controller for new hand
	FindMotionController();
}

void UVRUIInteractionComponent::SetMotionController(UMotionControllerComponent* NewController)
{
	MotionController = NewController;

	// Re-initialize widget interaction with new controller
	if (WidgetInteraction && MotionController)
	{
		// Attach widget interaction to motion controller
		WidgetInteraction->AttachToComponent(MotionController, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void UVRUIInteractionComponent::InitializeWidgetInteraction()
{
	// Create widget interaction component if it doesn't exist
	if (!WidgetInteraction)
	{
		WidgetInteraction = NewObject<UWidgetInteractionComponent>(this, UWidgetInteractionComponent::StaticClass());
		if (WidgetInteraction)
		{
			WidgetInteraction->RegisterComponent();
			WidgetInteraction->InteractionDistance = MaxInteractionDistance;
			WidgetInteraction->bShowDebug = bShowDebugRays;
			WidgetInteraction->TraceChannel = TraceChannel;

			// Enable virtual user index for widget interaction
			WidgetInteraction->VirtualUserIndex = 0;
		}
	}
}

void UVRUIInteractionComponent::InitializeLaserPointer()
{
	// Create Niagara laser pointer component
	if (LaserPointerEffect && !LaserPointerComponent && MotionController)
	{
		LaserPointerComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			LaserPointerEffect,
			MotionController,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			false
		);

		if (LaserPointerComponent)
		{
			// Set initial parameters
			LaserPointerComponent->SetColorParameter(FName("BeamColor"), PointerColorIdle);
			LaserPointerComponent->SetFloatParameter(FName("BeamWidth"), PointerThickness);
		}
	}
}

void UVRUIInteractionComponent::FindMotionController()
{
	if (MotionController)
	{
		return; // Already have a controller
	}

	// Try to find motion controller on owner
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Search for motion controller component
	TArray<UMotionControllerComponent*> MotionControllers;
	Owner->GetComponents<UMotionControllerComponent>(MotionControllers);

	// Find controller matching our hand
	for (UMotionControllerComponent* Controller : MotionControllers)
	{
		if (InteractionHand == EVRInteractionHand::Left && Controller->MotionSource == FName("Left"))
		{
			SetMotionController(Controller);
			break;
		}
		else if (InteractionHand == EVRInteractionHand::Right && Controller->MotionSource == FName("Right"))
		{
			SetMotionController(Controller);
			break;
		}
	}
}

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

void UVRUIInteractionComponent::UpdateHoverState()
{
	// Check if hover state changed
	if (CurrentHoveredWidget != PreviousHoveredWidget)
	{
		// Exit previous hover
		if (PreviousHoveredWidget)
		{
			OnHoverExit(PreviousHoveredWidget);
		}

		// Enter new hover
		if (CurrentHoveredWidget)
		{
			OnHoverEnter(CurrentHoveredWidget);
		}
	}

	// Update hover state enum
	if (CurrentHoveredWidget)
	{
		if (bIsButtonPressed)
		{
			LastInteractionResult.HoverState = EVRWidgetHoverState::Pressed;
		}
		else
		{
			LastInteractionResult.HoverState = EVRWidgetHoverState::Hovering;
		}
	}
	else
	{
		LastInteractionResult.HoverState = EVRWidgetHoverState::None;
	}
}

void UVRUIInteractionComponent::OnHoverEnter(UWidgetComponent* Widget)
{
	if (!Widget)
	{
		return;
	}

	// Play hover haptics
	if (bEnableHapticFeedback)
	{
		PlayHoverHaptics();
	}

	// Update pointer color
	UpdateLaserPointerVisual();
}

void UVRUIInteractionComponent::OnHoverExit(UWidgetComponent* Widget)
{
	if (!Widget)
	{
		return;
	}

	// Reset any pressed state
	if (bIsButtonPressed)
	{
		HandleButtonRelease();
	}

	// Update pointer color
	UpdateLaserPointerVisual();
}

EControllerHand UVRUIInteractionComponent::GetControllerHand() const
{
	switch (InteractionHand)
	{
		case EVRInteractionHand::Left:
			return EControllerHand::Left;
		case EVRInteractionHand::Right:
			return EControllerHand::Right;
		default:
			return EControllerHand::Right;
	}
}

void UVRUIInteractionComponent::PlayHapticOnController(float Intensity, float Duration)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		EControllerHand Hand = GetControllerHand();
		// UE 5.6 uses PlayHapticEffect for controller-specific haptics
		// Create a simple haptic curve for dynamic feedback
		PC->PlayDynamicForceFeedback(Intensity, Duration, true, true, true, true, EDynamicForceFeedbackAction::Start);

		// Additional hand-specific haptic (if HapticEffect is assigned via FVRHapticPattern)
		if (HoverHapticPattern.HapticEffect)
		{
			PC->PlayHapticEffect(HoverHapticPattern.HapticEffect, Hand, Intensity);
		}
	}
}

void UVRUIInteractionComponent::UpdateLaserPointerVisual()
{
	if (!LaserPointerComponent)
	{
		return;
	}

	// Set color based on state
	FLinearColor TargetColor = PointerColorIdle;

	if (bIsButtonPressed)
	{
		TargetColor = PointerColorPressed;
	}
	else if (CurrentHoveredWidget)
	{
		TargetColor = PointerColorHover;
	}

	SetPointerColor(TargetColor);
}

bool UVRUIInteractionComponent::ShouldPerformRaycastThisFrame() const
{
	// Always raycast if no frame skipping
	if (RaycastSkipFrames == 0)
	{
		return true;
	}

	// Check if we should skip this frame
	return (FrameCounter % (RaycastSkipFrames + 1)) == 0;
}

void UVRUIInteractionComponent::DrawDebugVisualization(const FVector& Start, const FVector& End, bool bHit)
{
	if (!bShowDebugRays || !GetWorld())
	{
		return;
	}

	FColor LineColor = bHit ? FColor::Green : FColor::Red;
	DrawDebugLine(GetWorld(), Start, End, LineColor, false, -1.0f, 0, 1.0f);

	if (bHit)
	{
		DrawDebugSphere(GetWorld(), End, 2.0f, 8, FColor::Yellow, false, -1.0f);
	}
}
