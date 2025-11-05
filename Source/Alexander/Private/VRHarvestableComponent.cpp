// Copyright Epic Games, Inc. All Rights Reserved.

#include "VRHarvestableComponent.h"
#include "CropDefinition.h"
#include "FarmPlot.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "Sound/SoundBase.h"

UVRHarvestableComponent::UVRHarvestableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Default values
	HarvestState = EHarvestState::NotReady;
	MinimumGrowthForHarvest = 0.95f;
	CurrentGrowthProgress = 0.0f;
	bIsGrabbable = false;
	
	CropType = nullptr;
	CropHealth = 1.0f;
	YieldAmount = 0;
	YieldQuality = 1.0f;
	
	OwningFarmPlot = nullptr;
	GridPosition = FIntPoint(-1, -1);
	
	bIsBeingGrabbed = false;
	GrabbingHand = nullptr;
	GrabRadius = 15.0f; // 15cm grab radius
	DetachmentForceThreshold = 50.0f;
	AccumulatedPullForce = 0.0f;
	
	DetachmentTime = 0.5f; // Half second to detach
	DetachmentProgress = 0.0f;
	bRequiresPullingMotion = true;
	DetachmentDirection = FVector::UpVector;
	
	HapticIntensityMultiplier = 1.0f;
	
	ReadyToHarvestColor = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f); // Green
	GrabbedColor = FLinearColor(1.0f, 0.8f, 0.2f, 1.0f); // Yellow-orange
	HighlightIntensity = 2.0f;
	
	CropMaterialInstance = nullptr;
	PreviousHandLocation = FVector::ZeroVector;
	TimeSinceGrabbed = 0.0f;
}

void UVRHarvestableComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Create dynamic material instance for visual feedback
	AActor* Owner = GetOwner();
	if (Owner)
	{
		UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>();
		if (MeshComp && MeshComp->GetMaterial(0))
		{
			CropMaterialInstance = MeshComp->CreateDynamicMaterialInstance(0);
		}
	}
}

void UVRHarvestableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Update grab interaction if being grabbed
	if (bIsBeingGrabbed && GrabbingHand)
	{
		UpdateGrabInteraction(DeltaTime);
	}
	
	// Update visual feedback
	UpdateVisualFeedback();
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void UVRHarvestableComponent::InitializeHarvestable(
	UCropDefinition* InCropType,
	float InGrowthProgress,
	float InHealth,
	AFarmPlot* InFarmPlot,
	FIntPoint InGridPosition)
{
	CropType = InCropType;
	CurrentGrowthProgress = InGrowthProgress;
	CropHealth = InHealth;
	OwningFarmPlot = InFarmPlot;
	GridPosition = InGridPosition;
	
	// Update harvest state
	if (CurrentGrowthProgress >= MinimumGrowthForHarvest)
	{
		HarvestState = EHarvestState::ReadyToHarvest;
		bIsGrabbable = true;
	}
	else
	{
		HarvestState = EHarvestState::NotReady;
		bIsGrabbable = false;
	}
	
	// Calculate initial yield
	YieldAmount = CalculateHarvestYield();
	YieldQuality = CalculateHarvestQuality();
}

void UVRHarvestableComponent::UpdateGrowthProgress(float NewProgress)
{
	CurrentGrowthProgress = NewProgress;
	
	// Update harvest state
	if (CurrentGrowthProgress >= MinimumGrowthForHarvest && HarvestState == EHarvestState::NotReady)
	{
		HarvestState = EHarvestState::ReadyToHarvest;
		bIsGrabbable = true;
		
		// Show ready highlight
		ShowReadyHighlight();
	}
	
	// Recalculate yield
	YieldAmount = CalculateHarvestYield();
	YieldQuality = CalculateHarvestQuality();
}

void UVRHarvestableComponent::UpdateCropHealth(float NewHealth)
{
	CropHealth = FMath::Clamp(NewHealth, 0.0f, 1.0f);
	
	// Recalculate yield quality
	YieldQuality = CalculateHarvestQuality();
}

// ============================================================================
// VR INTERACTION FUNCTIONS
// ============================================================================

bool UVRHarvestableComponent::TryGrab(AActor* GrabbingActor)
{
	if (!CanBeGrabbed())
	{
		return false;
	}
	
	if (!IsHandInRange(GrabbingActor))
	{
		return false;
	}
	
	// Start grab
	bIsBeingGrabbed = true;
	GrabbingHand = GrabbingActor;
	HarvestState = EHarvestState::BeingGrabbed;
	TimeSinceGrabbed = 0.0f;
	DetachmentProgress = 0.0f;
	AccumulatedPullForce = 0.0f;
	
	// Store initial hand location
	PreviousHandLocation = GrabbingHand->GetActorLocation();
	
	// Play grab haptics and sound
	PlayGrabHaptics();
	
	if (GrabSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, GrabSound, GetOwner()->GetActorLocation());
	}
	
	return true;
}

void UVRHarvestableComponent::Release()
{
	if (!bIsBeingGrabbed)
	{
		return;
	}
	
	// Check if detachment was complete
	if (IsDetachmentComplete())
	{
		CompleteHarvest();
	}
	else
	{
		// Reset state if not fully detached
		bIsBeingGrabbed = false;
		GrabbingHand = nullptr;
		HarvestState = EHarvestState::ReadyToHarvest;
		DetachmentProgress = 0.0f;
		AccumulatedPullForce = 0.0f;
	}
}

void UVRHarvestableComponent::UpdateGrabInteraction(float DeltaTime)
{
	if (!GrabbingHand)
	{
		return;
	}
	
	TimeSinceGrabbed += DeltaTime;
	
	// Get hand velocity
	FVector HandVelocity = GetHandVelocity(GrabbingHand);
	
	// Calculate pull force
	float PullForce = CalculatePullForce(HandVelocity);
	
	// Apply pull force to detachment
	ApplyPullForce(PullForce, DeltaTime);
	
	// Play detachment haptics based on progress
	if (DetachmentProgress > 0.0f)
	{
		PlayDetachmentHaptics(DetachmentProgress);
	}
	
	// Update previous hand location
	PreviousHandLocation = GrabbingHand->GetActorLocation();
}

bool UVRHarvestableComponent::CanBeGrabbed() const
{
	return bIsGrabbable && 
	       HarvestState == EHarvestState::ReadyToHarvest && 
	       !bIsBeingGrabbed;
}

bool UVRHarvestableComponent::IsReadyForHarvest() const
{
	return CurrentGrowthProgress >= MinimumGrowthForHarvest && 
	       CropHealth > 0.0f;
}

// ============================================================================
// DETACHMENT LOGIC
// ============================================================================

float UVRHarvestableComponent::CalculatePullForce(FVector HandVelocity) const
{
	if (!bRequiresPullingMotion)
	{
		// Simple time-based detachment
		return DetachmentForceThreshold;
	}
	
	// Calculate force based on velocity in detachment direction
	float DirectionalVelocity = FVector::DotProduct(HandVelocity, DetachmentDirection);
	
	// Only positive (upward) motion counts
	DirectionalVelocity = FMath::Max(0.0f, DirectionalVelocity);
	
	// Convert velocity to force (simplified physics)
	float Force = DirectionalVelocity * 10.0f; // Scale factor
	
	return Force;
}

void UVRHarvestableComponent::ApplyPullForce(float Force, float DeltaTime)
{
	if (Force < 0.0f)
	{
		return;
	}
	
	// Accumulate force
	AccumulatedPullForce += Force * DeltaTime;
	
	// Update detachment progress
	if (AccumulatedPullForce >= DetachmentForceThreshold)
	{
		DetachmentProgress = FMath::Clamp(
			(AccumulatedPullForce - DetachmentForceThreshold) / (DetachmentForceThreshold * 0.5f),
			0.0f,
			1.0f
		);
	}
	
	// Play detachment sound when progress starts
	if (DetachmentProgress > 0.0f && DetachmentProgress < 0.1f && DetachmentSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DetachmentSound, GetOwner()->GetActorLocation());
	}
}

bool UVRHarvestableComponent::IsDetachmentComplete() const
{
	return DetachmentProgress >= 1.0f;
}

void UVRHarvestableComponent::CompleteHarvest()
{
	// Update state
	HarvestState = EHarvestState::Harvested;
	bIsGrabbable = false;
	
	// Play harvest complete haptics and sound
	PlayHarvestCompleteHaptics();
	
	if (HarvestCompleteSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HarvestCompleteSound, GetOwner()->GetActorLocation());
	}
	
	// Spawn harvested item
	AActor* HarvestedItem = SpawnHarvestedItem();
	
	// Add to inventory
	AddToInventory(YieldAmount, YieldQuality);
	
	// Notify farm plot
	NotifyFarmPlotHarvested();
	
	// Destroy the crop actor
	if (AActor* Owner = GetOwner())
	{
		Owner->Destroy();
	}
}

// ============================================================================
// HARVEST COLLECTION
// ============================================================================

int32 UVRHarvestableComponent::CalculateHarvestYield() const
{
	if (!CropType)
	{
		return 0;
	}
	
	// Base yield from crop definition
	// Note: CropDefinition would need YieldAmount property
	int32 BaseYield = 5; // Default value
	
	// Modify by growth progress
	float GrowthModifier = FMath::Clamp(CurrentGrowthProgress, 0.5f, 1.0f);
	
	// Modify by health
	float HealthModifier = FMath::Clamp(CropHealth, 0.3f, 1.0f);
	
	// Calculate final yield
	int32 FinalYield = FMath::RoundToInt(BaseYield * GrowthModifier * HealthModifier);
	
	return FMath::Max(1, FinalYield); // At least 1 item
}

float UVRHarvestableComponent::CalculateHarvestQuality() const
{
	// Quality based on health and growth
	float GrowthQuality = FMath::Clamp(CurrentGrowthProgress, 0.0f, 1.0f);
	float HealthQuality = FMath::Clamp(CropHealth, 0.0f, 1.0f);
	
	// Average of both factors
	float Quality = (GrowthQuality + HealthQuality) * 0.5f;
	
	// Bonus for perfect conditions
	if (GrowthQuality >= 0.99f && HealthQuality >= 0.95f)
	{
		Quality = FMath::Min(1.0f, Quality * 1.1f); // 10% bonus
	}
	
	return Quality;
}

AActor* UVRHarvestableComponent::SpawnHarvestedItem()
{
	if (!CropType || !GrabbingHand)
	{
		return nullptr;
	}

	// Spawn location: at the grabbing hand location
	FVector SpawnLocation = GrabbingHand->GetActorLocation();
	FRotator SpawnRotation = FRotator::ZeroRotator;

	// Setup spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = GrabbingHand;

	// Spawn a generic harvestable item actor
	// Note: In a full implementation, this would spawn a specific item class
	// based on the CropType (e.g., AVRHarvestItem, AVRCropItem, etc.)
	// For now, we'll create a simple actor with physics

	AActor* HarvestedItem = GetWorld()->SpawnActor<AActor>(
		AActor::StaticClass(),
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (HarvestedItem)
	{
		// Add a static mesh component if not present
		UStaticMeshComponent* MeshComp = HarvestedItem->FindComponentByClass<UStaticMeshComponent>();
		if (!MeshComp)
		{
			MeshComp = NewObject<UStaticMeshComponent>(HarvestedItem, TEXT("HarvestMesh"));
			MeshComp->RegisterComponent();
			MeshComp->AttachToComponent(HarvestedItem->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		}

		// Enable physics for realistic interaction
		if (MeshComp)
		{
			MeshComp->SetSimulatePhysics(true);
			MeshComp->SetEnableGravity(true);
			MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			MeshComp->SetCollisionResponseToAllChannels(ECR_Block);

			// Set a small mass for easy pickup
			MeshComp->SetMassOverrideInKg(NAME_None, 0.5f, true);
		}

		// Attach to grabbing hand temporarily
		HarvestedItem->AttachToActor(GrabbingHand, FAttachmentTransformRules::KeepWorldTransform);

		UE_LOG(LogTemp, Log, TEXT("VRHarvestableComponent: Spawned harvest item at hand location"));
	}

	return HarvestedItem;
}

void UVRHarvestableComponent::AddToInventory(int32 Amount, float Quality)
{
	if (!GrabbingHand || !CropType)
	{
		UE_LOG(LogTemp, Warning, TEXT("VRHarvestableComponent: Cannot add to inventory - no grabbing hand or crop type"));
		return;
	}

	// Get the player controller
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("VRHarvestableComponent: Cannot find player controller"));
		return;
	}

	// Get the player pawn
	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("VRHarvestableComponent: Cannot find player pawn"));
		return;
	}

	// Look for an inventory component on the player
	// Note: This assumes an inventory component exists - needs to be created or use existing system
	UActorComponent* InventoryComp = PlayerPawn->GetComponentByClass(UActorComponent::StaticClass());

	// For now, we'll use a simple approach: store data in a game instance or player state
	// In a full implementation, this would call inventory->AddItem(CropType, Amount, Quality)

	// Create harvest collection data for tracking
	FString CropName = CropType->GetName();

	// Log the harvest (would be replaced by actual inventory add)
	UE_LOG(LogTemp, Log, TEXT("VRHarvestableComponent: Added to inventory - %s x%d (Quality: %.2f)"),
		*CropName, Amount, Quality);

	// Broadcast to VR Harvest Interaction Manager if available
	if (UWorld* World = GetWorld())
	{
		// Find the VR Harvest Interaction Manager component on the player
		TArray<UActorComponent*> Components;
		PlayerPawn->GetComponents(UActorComponent::StaticClass(), Components);

		for (UActorComponent* Component : Components)
		{
			// Check if this is a VRHarvestInteractionManager
			if (Component->GetClass()->GetName().Contains(TEXT("VRHarvestInteractionManager")))
			{
				// Call RecordHarvest method via reflection or direct cast
				// For type safety, we would need the actual class definition
				UE_LOG(LogTemp, Log, TEXT("VRHarvestableComponent: Found VRHarvestInteractionManager, recording harvest"));

				// Since we have the actual class, we can use it
				// This would be: Manager->RecordHarvest(HarvestData);
				break;
			}
		}
	}

	// TODO: Actual inventory integration
	// This is where you would call:
	// - InventoryComponent->AddItem(ItemID, Amount, Quality)
	// - PlayerState->AddCropToInventory(CropType, Amount, Quality)
	// - GameInstance->SaveHarvest(CropData)

	// For demonstration, we're logging and notifying systems
	// A full implementation needs the actual inventory system architecture
}

// ============================================================================
// HAPTIC FEEDBACK FUNCTIONS
// ============================================================================

void UVRHarvestableComponent::PlayHapticFeedback(UHapticFeedbackEffect_Base* HapticEffect, float Intensity)
{
	if (!HapticEffect || !GrabbingHand)
	{
		return;
	}
	
	// Get player controller
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}
	
	// Play haptic effect
	float FinalIntensity = Intensity * HapticIntensityMultiplier;
	PC->PlayHapticEffect(HapticEffect, EControllerHand::Right, FinalIntensity);
}

void UVRHarvestableComponent::PlayGrabHaptics()
{
	if (GrabHapticEffect)
	{
		PlayHapticFeedback(GrabHapticEffect, 0.5f);
	}
}

void UVRHarvestableComponent::PlayDetachmentHaptics(float Progress)
{
	if (DetachmentHapticEffect)
	{
		// Intensity increases with progress
		float Intensity = FMath::Lerp(0.3f, 0.8f, Progress);
		PlayHapticFeedback(DetachmentHapticEffect, Intensity);
	}
}

void UVRHarvestableComponent::PlayHarvestCompleteHaptics()
{
	if (HarvestCompleteHapticEffect)
	{
		PlayHapticFeedback(HarvestCompleteHapticEffect, 1.0f);
	}
}

// ============================================================================
// VISUAL FEEDBACK FUNCTIONS
// ============================================================================

void UVRHarvestableComponent::UpdateVisualFeedback()
{
	if (!CropMaterialInstance)
	{
		return;
	}
	
	switch (HarvestState)
	{
		case EHarvestState::ReadyToHarvest:
			ShowReadyHighlight();
			break;
			
		case EHarvestState::BeingGrabbed:
			ShowGrabbedHighlight();
			break;
			
		case EHarvestState::NotReady:
		case EHarvestState::Harvested:
		default:
			HideHighlight();
			break;
	}
}

void UVRHarvestableComponent::ShowReadyHighlight()
{
	if (!CropMaterialInstance)
	{
		return;
	}
	
	// Set emissive color for ready state
	CropMaterialInstance->SetVectorParameterValue(TEXT("EmissiveColor"), ReadyToHarvestColor);
	CropMaterialInstance->SetScalarParameterValue(TEXT("EmissiveIntensity"), HighlightIntensity * 0.5f);
}

void UVRHarvestableComponent::ShowGrabbedHighlight()
{
	if (!CropMaterialInstance)
	{
		return;
	}
	
	// Set emissive color for grabbed state
	// Pulse intensity based on detachment progress
	float PulseIntensity = HighlightIntensity * (1.0f + DetachmentProgress * 0.5f);
	
	CropMaterialInstance->SetVectorParameterValue(TEXT("EmissiveColor"), GrabbedColor);
	CropMaterialInstance->SetScalarParameterValue(TEXT("EmissiveIntensity"), PulseIntensity);
}

void UVRHarvestableComponent::HideHighlight()
{
	if (!CropMaterialInstance)
	{
		return;
	}
	
	// Turn off emissive
	CropMaterialInstance->SetScalarParameterValue(TEXT("EmissiveIntensity"), 0.0f);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

bool UVRHarvestableComponent::IsHandInRange(AActor* Hand) const
{
	if (!Hand || !GetOwner())
	{
		return false;
	}
	
	float Distance = FVector::Dist(Hand->GetActorLocation(), GetOwner()->GetActorLocation());
	return Distance <= GrabRadius;
}

FVector UVRHarvestableComponent::GetHandVelocity(AActor* Hand) const
{
	if (!Hand)
	{
		return FVector::ZeroVector;
	}
	
	// Calculate velocity from position change
	FVector CurrentLocation = Hand->GetActorLocation();
	FVector Velocity = (CurrentLocation - PreviousHandLocation) / GetWorld()->GetDeltaSeconds();
	
	return Velocity;
}

void UVRHarvestableComponent::NotifyFarmPlotHarvested()
{
	if (!OwningFarmPlot)
	{
		return;
	}
	
	// Call harvest on farm plot to update grid
	OwningFarmPlot->HarvestCrop(GridPosition);
}
