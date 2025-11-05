// Copyright Epic Games, Inc. All Rights Reserved.

#include "CockpitComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SceneComponent.h"
#include "MotionControllerComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Sound/SoundBase.h"

// ============================================================================
// CONSTRUCTOR
// ============================================================================

UCockpitComponent::UCockpitComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.016f; // ~60 Hz for smooth VR

	// Create default subobjects
	CockpitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CockpitMesh"));
	CameraAttachmentPoint = CreateDefaultSubobject<USceneComponent>(TEXT("CameraAttachPoint"));
	
	if (CockpitMesh && CameraAttachmentPoint)
	{
		CameraAttachmentPoint->SetupAttachment(CockpitMesh);
	}
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UCockpitComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeCockpit();

	if (bShowInteractionDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("CockpitComponent: Initialized with %d controls and %d displays"),
			CockpitControls.Num(), CockpitDisplays.Num());
	}
}

void UCockpitComponent::TickComponent(float DeltaTime, ELevelTick TickType, 
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update displays every frame
	UpdateDisplays();

	// Draw debug info if enabled
	if (bShowInteractionDebug)
	{
		DrawDebugInfo();
	}

	// Check for VR controller interactions
	if (LeftController && RightController)
	{
		// Check left controller
		FVector LeftLocation = LeftController->GetComponentLocation();
		FVector LeftForward = LeftController->GetForwardVector();
		
		// Simple raycast for interaction
		FHitResult Hit;
		FVector EndLocation = LeftLocation + (LeftForward * InteractionDistance);
		
		if (GetWorld()->LineTraceSingleByChannel(Hit, LeftLocation, EndLocation, ECC_Visibility))
		{
			HandleControlInteraction(LeftController, Hit.Location, Hit.Normal);
		}

		// Check right controller
		FVector RightLocation = RightController->GetComponentLocation();
		FVector RightForward = RightController->GetForwardVector();
		EndLocation = RightLocation + (RightForward * InteractionDistance);
		
		if (GetWorld()->LineTraceSingleByChannel(Hit, RightLocation, EndLocation, ECC_Visibility))
		{
			HandleControlInteraction(RightController, Hit.Location, Hit.Normal);
		}
	}
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void UCockpitComponent::InitializeCockpit()
{
	if (bIsInitialized)
	{
		return;
	}

	// Create all control meshes
	CreateControls();

	// Create all display widgets
	CreateDisplays();

	// Setup VR interaction
	SetupVRInteraction();

	bIsInitialized = true;
	bSystemsOnline = true;
	PowerLevel = 100.0f;
}

void UCockpitComponent::CreateControls()
{
	if (!CockpitMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("CockpitComponent: Cannot create controls without CockpitMesh"));
		return;
	}

	// Clear existing controls
	for (auto& Pair : ControlComponents)
	{
		if (Pair.Value)
		{
			Pair.Value->DestroyComponent();
		}
	}
	ControlComponents.Empty();

	// Create mesh component for each control
	for (FCockpitControl& Control : CockpitControls)
	{
		if (Control.ControlMesh)
		{
			UStaticMeshComponent* ControlMesh = CreateControlMesh(Control);
			if (ControlMesh)
			{
				ControlComponents.Add(Control.ControlName, ControlMesh);
			}
		}
	}

	if (bShowInteractionDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("Created %d cockpit controls"), ControlComponents.Num());
	}
}

void UCockpitComponent::CreateDisplays()
{
	if (!CockpitMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("CockpitComponent: Cannot create displays without CockpitMesh"));
		return;
	}

	// Clear existing displays
	for (auto& Pair : DisplayComponents)
	{
		if (Pair.Value)
		{
			Pair.Value->DestroyComponent();
		}
	}
	DisplayComponents.Empty();

	// Create widget component for each display
	for (FCockpitDisplay& Display : CockpitDisplays)
	{
		if (Display.WidgetClass)
		{
			UWidgetComponent* DisplayWidget = CreateDisplayWidget(Display);
			if (DisplayWidget)
			{
				DisplayComponents.Add(Display.DisplayName, DisplayWidget);
			}
		}
	}

	if (bShowInteractionDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("Created %d cockpit displays"), DisplayComponents.Num());
	}
}

void UCockpitComponent::SetupVRInteraction()
{
	// VR controllers are typically managed by the pawn/character
	// This function can be used to configure interaction settings
	
	// Find motion controllers in owner
	if (AActor* Owner = GetOwner())
	{
		TArray<UActorComponent*> Controllers;
		Owner->GetComponents(UMotionControllerComponent::StaticClass(), Controllers);

		for (UActorComponent* Component : Controllers)
		{
			UMotionControllerComponent* MotionController = Cast<UMotionControllerComponent>(Component);
			if (MotionController)
			{
				// Determine which hand
				if (MotionController->MotionSource == FName("Left"))
				{
					LeftController = MotionController;
				}
				else if (MotionController->MotionSource == FName("Right"))
				{
					RightController = MotionController;
				}
			}
		}
	}

	if (bShowInteractionDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("VR Interaction setup: Left=%s, Right=%s"),
			LeftController ? TEXT("Found") : TEXT("Not Found"),
			RightController ? TEXT("Found") : TEXT("Not Found"));
	}
}

// ============================================================================
// CONTROL INTERACTION
// ============================================================================

bool UCockpitComponent::HandleControlInteraction(UMotionControllerComponent* Controller, 
	const FVector& HitLocation, const FVector& HitNormal)
{
	if (!Controller || !bSystemsOnline)
	{
		return false;
	}

	// Check interaction cooldown
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastInteractionTime < InteractionCooldown)
	{
		return false;
	}

	// Find control at hit location
	FCockpitControl* Control = GetControlAtLocation(HitLocation);
	if (!Control)
	{
		// Clear highlighted control if no longer hovering
		if (HoveredControl)
		{
			UpdateControlHighlight(HoveredControl, false);
			HoveredControl = nullptr;
		}
		return false;
	}

	// Update hover state
	if (HoveredControl != Control)
	{
		if (HoveredControl)
		{
			UpdateControlHighlight(HoveredControl, false);
		}
		UpdateControlHighlight(Control, true);
		HoveredControl = Control;
	}

	// Check if trigger/grip is pressed (simplified - would need actual input binding)
	// For now, assume interaction happens on hover for certain time
	InteractWithControl(*Control, Controller);
	LastInteractionTime = CurrentTime;

	return true;
}

FCockpitControl* UCockpitComponent::GetControlAtLocation(const FVector& WorldLocation)
{
	float ClosestDistance = InteractionDistance;
	FCockpitControl* ClosestControl = nullptr;

	for (FCockpitControl& Control : CockpitControls)
	{
		if (!Control.bIsInteractive)
		{
			continue;
		}

		// Get control world location
		UStaticMeshComponent* ControlMesh = ControlComponents.FindRef(Control.ControlName);
		if (ControlMesh)
		{
			FVector ControlLocation = ControlMesh->GetComponentLocation();
			float Distance = FVector::Dist(WorldLocation, ControlLocation);

			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestControl = &Control;
			}
		}
	}

	return ClosestControl;
}

void UCockpitComponent::InteractWithControl(FCockpitControl& Control, 
	UMotionControllerComponent* Controller)
{
	if (!Control.bIsInteractive)
	{
		return;
	}

	// Handle different control types
	switch (Control.ControlType)
	{
	case ECockpitInteractionType::Button:
		// Toggle or press button
		Control.CurrentValue = Control.CurrentValue > 0.5f ? 0.0f : 1.0f;
		break;

	case ECockpitInteractionType::Switch:
		// Toggle switch
		Control.CurrentValue = Control.CurrentValue > 0.5f ? 0.0f : 1.0f;
		break;

	case ECockpitInteractionType::Slider:
	case ECockpitInteractionType::Throttle:
		// Would need more sophisticated input for continuous adjustment
		// For now, increment by 0.1
		Control.CurrentValue += 0.1f;
		if (Control.CurrentValue > Control.ValueRange.Y)
		{
			Control.CurrentValue = Control.ValueRange.X;
		}
		break;

	case ECockpitInteractionType::Stick:
		// Complex 2D input - would need proper tracking
		break;

	case ECockpitInteractionType::Display:
		// Display interaction handled separately
		break;
	}

	// Update visual state
	UpdateControlVisual(Control);

	// Play feedback
	PlayInteractionFeedback(Control, Controller);

	// Call interaction function if defined
	if (Control.OnInteractFunction != NAME_None)
	{
		// Would trigger Blueprint event or C++ function
		if (bShowInteractionDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("Control '%s' interacted, calling function '%s'"),
				*Control.ControlName.ToString(), *Control.OnInteractFunction.ToString());
		}
	}
}

void UCockpitComponent::UpdateControlVisual(FCockpitControl& Control)
{
	UStaticMeshComponent* ControlMesh = ControlComponents.FindRef(Control.ControlName);
	if (!ControlMesh)
	{
		return;
	}

	// Update material parameters based on control value
	UMaterialInstanceDynamic* DynamicMaterial = ControlMesh->CreateDynamicMaterialInstance(0);
	if (DynamicMaterial)
	{
		DynamicMaterial->SetScalarParameterValue(FName("ControlValue"), Control.CurrentValue);
		DynamicMaterial->SetScalarParameterValue(FName("PowerLevel"), PowerLevel / 100.0f);
	}

	// Update position/rotation for certain control types
	switch (Control.ControlType)
	{
	case ECockpitInteractionType::Switch:
	{
		FRotator CurrentRotation = ControlMesh->GetRelativeRotation();
		CurrentRotation.Pitch = Control.CurrentValue > 0.5f ? 30.0f : -30.0f;
		ControlMesh->SetRelativeRotation(CurrentRotation);
		break;
	}

	case ECockpitInteractionType::Slider:
	case ECockpitInteractionType::Throttle:
	{
		FVector CurrentLocation = Control.RelativePosition;
		float Range = Control.ValueRange.Y - Control.ValueRange.X;
		float Offset = (Control.CurrentValue - Control.ValueRange.X) / Range * 10.0f; // 10cm range
		CurrentLocation.Z += Offset;
		ControlMesh->SetRelativeLocation(CurrentLocation);
		break;
	}

	default:
		break;
	}
}

void UCockpitComponent::PlayInteractionFeedback(const FCockpitControl& Control, 
	UMotionControllerComponent* Controller)
{
	// Play sound effect
	if (Control.InteractionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Control.InteractionSound, 
			Controller->GetComponentLocation());
	}

	// Play haptic feedback
	if (Controller)
	{
		// Would need proper haptic feedback implementation
		// Controller->PlayHapticEffect(...);
	}
}

// ============================================================================
// DISPLAY MANAGEMENT
// ============================================================================

void UCockpitComponent::UpdateDisplays()
{
	// Update all display widgets with current data
	for (auto& Pair : DisplayComponents)
	{
		UWidgetComponent* DisplayWidget = Pair.Value;
		if (DisplayWidget && DisplayWidget->GetUserWidgetObject())
		{
			// Widget would be updated through Blueprint or C++ interface
			// For now, just ensure it's visible if systems are online
			DisplayWidget->SetVisibility(bSystemsOnline);
		}
	}
}

FCockpitDisplay* UCockpitComponent::GetDisplay(const FName& DisplayName)
{
	for (FCockpitDisplay& Display : CockpitDisplays)
	{
		if (Display.DisplayName == DisplayName)
		{
			return &Display;
		}
	}
	return nullptr;
}

void UCockpitComponent::UpdateDisplay(const FName& DisplayName, UUserWidget* NewWidget)
{
	UWidgetComponent* DisplayWidget = DisplayComponents.FindRef(DisplayName);
	if (DisplayWidget && NewWidget)
	{
		DisplayWidget->SetWidget(NewWidget);
	}
}

// ============================================================================
// SYSTEM INTEGRATION
// ============================================================================

void UCockpitComponent::SetPowerLevel(float NewPowerLevel)
{
	PowerLevel = FMath::Clamp(NewPowerLevel, 0.0f, 100.0f);

	// Update all controls with new power level
	for (FCockpitControl& Control : CockpitControls)
	{
		UpdateControlVisual(Control);
	}

	// Disable systems if power too low
	if (PowerLevel < 10.0f && bSystemsOnline)
	{
		bSystemsOnline = false;
		UE_LOG(LogTemp, Warning, TEXT("Cockpit systems offline - insufficient power"));
	}
	else if (PowerLevel >= 10.0f && !bSystemsOnline)
	{
		bSystemsOnline = true;
		UE_LOG(LogTemp, Log, TEXT("Cockpit systems online"));
	}
}

void UCockpitComponent::ToggleSystems()
{
	bSystemsOnline = !bSystemsOnline;

	// Update all displays
	UpdateDisplays();

	if (bShowInteractionDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("Cockpit systems: %s"), 
			bSystemsOnline ? TEXT("ONLINE") : TEXT("OFFLINE"));
	}
}

void UCockpitComponent::ApplyDamageEffects(float DamageAmount)
{
	// Reduce power
	SetPowerLevel(PowerLevel - DamageAmount);

	// Random control failures at low power
	if (PowerLevel < 50.0f && FMath::FRand() < 0.3f)
	{
		// Randomly disable a control
		if (CockpitControls.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, CockpitControls.Num() - 1);
			CockpitControls[RandomIndex].bIsInteractive = false;

			if (bShowInteractionDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("Control '%s' damaged and non-functional"),
					*CockpitControls[RandomIndex].ControlName.ToString());
			}
		}
	}
}

FString UCockpitComponent::GetCockpitStatus() const
{
	return FString::Printf(TEXT("Systems: %s | Power: %.1f%% | Controls: %d/%d operational"),
		bSystemsOnline ? TEXT("ONLINE") : TEXT("OFFLINE"),
		PowerLevel,
		CockpitControls.Num(),
		CockpitControls.Num());
}

// ============================================================================
// DEBUG & UTILITIES
// ============================================================================

void UCockpitComponent::DrawDebugInfo()
{
	if (!GetWorld())
	{
		return;
	}

	// Draw interaction spheres around controls
	if (bHighlightInteractiveControls)
	{
		for (const FCockpitControl& Control : CockpitControls)
		{
			if (Control.bIsInteractive)
			{
				UStaticMeshComponent* ControlMesh = ControlComponents.FindRef(Control.ControlName);
				if (ControlMesh)
				{
					FVector Location = ControlMesh->GetComponentLocation();
					FColor Color = (HoveredControl == &Control) ? FColor::Green : FColor::Yellow;
					DrawDebugSphere(GetWorld(), Location, 5.0f, 8, Color, false, -1.0f, 0, 1.0f);
				}
			}
		}
	}

	// Draw controller rays
	if (LeftController)
	{
		FVector Start = LeftController->GetComponentLocation();
		FVector End = Start + (LeftController->GetForwardVector() * InteractionDistance);
		DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, -1.0f, 0, 1.0f);
	}

	if (RightController)
	{
		FVector Start = RightController->GetComponentLocation();
		FVector End = Start + (RightController->GetForwardVector() * InteractionDistance);
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, -1.0f, 0, 1.0f);
	}
}

// ============================================================================
// PRIVATE HELPER FUNCTIONS
// ============================================================================

UStaticMeshComponent* UCockpitComponent::CreateControlMesh(const FCockpitControl& Control)
{
	if (!CockpitMesh)
	{
		return nullptr;
	}

	UStaticMeshComponent* ControlMesh = NewObject<UStaticMeshComponent>(this, 
		*FString::Printf(TEXT("Control_%s"), *Control.ControlName.ToString()));

	if (ControlMesh)
	{
		ControlMesh->SetupAttachment(CockpitMesh);
		ControlMesh->SetStaticMesh(Control.ControlMesh);
		ControlMesh->SetRelativeLocation(Control.RelativePosition);
		ControlMesh->SetRelativeRotation(Control.RelativeRotation);
		ControlMesh->SetRelativeScale3D(Control.Scale);

		if (Control.ControlMaterial)
		{
			ControlMesh->SetMaterial(0, Control.ControlMaterial);
		}

		ControlMesh->RegisterComponent();
	}

	return ControlMesh;
}

UWidgetComponent* UCockpitComponent::CreateDisplayWidget(const FCockpitDisplay& Display)
{
	if (!CockpitMesh)
	{
		return nullptr;
	}

	UWidgetComponent* DisplayWidget = NewObject<UWidgetComponent>(this,
		*FString::Printf(TEXT("Display_%s"), *Display.DisplayName.ToString()));

	if (DisplayWidget)
	{
		DisplayWidget->SetupAttachment(CockpitMesh);
		DisplayWidget->SetRelativeLocation(Display.RelativePosition);
		DisplayWidget->SetDrawSize(Display.DisplaySize);
		DisplayWidget->SetWidgetClass(Display.WidgetClass);
		DisplayWidget->SetWidgetSpace(EWidgetSpace::World);
		
		// Make it interactive if specified
		if (Display.bIsInteractive)
		{
			DisplayWidget->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}

		DisplayWidget->RegisterComponent();
	}

	return DisplayWidget;
}

void UCockpitComponent::UpdateControlHighlight(FCockpitControl* Control, bool bHighlight)
{
	if (!Control)
	{
		return;
	}

	UStaticMeshComponent* ControlMesh = ControlComponents.FindRef(Control->ControlName);
	if (ControlMesh)
	{
		UMaterialInstanceDynamic* DynamicMaterial = ControlMesh->CreateDynamicMaterialInstance(0);
		if (DynamicMaterial)
		{
			DynamicMaterial->SetScalarParameterValue(FName("Highlight"), bHighlight ? 1.0f : 0.0f);
		}
	}
}

bool UCockpitComponent::IsWithinInteractionDistance(const FVector& ControllerLocation, 
	const FVector& ControlLocation)
{
	return FVector::Dist(ControllerLocation, ControlLocation) <= InteractionDistance;
}
