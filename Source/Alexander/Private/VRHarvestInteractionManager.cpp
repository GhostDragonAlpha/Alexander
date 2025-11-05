// Copyright Epic Games, Inc. All Rights Reserved.

#include "VRHarvestInteractionManager.h"
#include "VRHarvestableComponent.h"
#include "FarmPlot.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UVRHarvestInteractionManager::UVRHarvestInteractionManager()
{
	PrimaryComponentTick.bCanEverTick = true;

	LeftHand = nullptr;
	RightHand = nullptr;
	HandDetectionRadius = 30.0f; // 30cm detection radius
	
	LeftHandTarget = nullptr;
	RightHandTarget = nullptr;
	bLeftHandGrabbing = false;
	bRightHandGrabbing = false;
	
	MaxHistorySize = 100;
	TotalHarvestedToday = 0;
	AverageQualityToday = 0.0f;
	
	FarmPlotDetectionRadius = 1000.0f; // 10 meters
	
	LastDayCheck = FDateTime::Now();
}

void UVRHarvestInteractionManager::BeginPlay()
{
	Super::BeginPlay();
	
	// Find nearby farm plots
	UpdateNearbyFarmPlots();
}

void UVRHarvestInteractionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Check for new day
	if (IsNewDay())
	{
		UpdateDailyStatistics();
	}
	
	// Update hand targets
	UpdateHandTargets();
	
	// Update grab interactions
	UpdateGrabInteractions(DeltaTime);
	
	// Update proximity indicators
	UpdateProximityIndicators();
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void UVRHarvestInteractionManager::InitializeHands(AActor* InLeftHand, AActor* InRightHand)
{
	LeftHand = InLeftHand;
	RightHand = InRightHand;
}

void UVRHarvestInteractionManager::RegisterFarmPlot(AFarmPlot* FarmPlot)
{
	if (FarmPlot && !NearbyFarmPlots.Contains(FarmPlot))
	{
		NearbyFarmPlots.Add(FarmPlot);
	}
}

void UVRHarvestInteractionManager::UnregisterFarmPlot(AFarmPlot* FarmPlot)
{
	NearbyFarmPlots.Remove(FarmPlot);
}

// ============================================================================
// INTERACTION DETECTION
// ============================================================================

void UVRHarvestInteractionManager::UpdateHandTargets()
{
	// Update left hand target
	if (LeftHand && !bLeftHandGrabbing)
	{
		LeftHandTarget = FindHarvestableNearHand(LeftHand);
	}
	
	// Update right hand target
	if (RightHand && !bRightHandGrabbing)
	{
		RightHandTarget = FindHarvestableNearHand(RightHand);
	}
}

UVRHarvestableComponent* UVRHarvestInteractionManager::FindHarvestableNearHand(AActor* Hand)
{
	if (!Hand)
	{
		return nullptr;
	}
	
	FVector HandLocation = Hand->GetActorLocation();
	
	// Find all harvestables in range
	TArray<UVRHarvestableComponent*> Harvestables = FindHarvestablesInRange(HandLocation, HandDetectionRadius);
	
	// Get closest harvestable
	return GetClosestHarvestable(HandLocation, Harvestables);
}

bool UVRHarvestInteractionManager::IsHandGrabbing(AActor* Hand) const
{
	if (!Hand)
	{
		return false;
	}

	// Get the player controller
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return false;
	}

	// Determine which hand we're checking
	bool bIsLeftHand = (Hand == LeftHand);
	EControllerHand HandType = bIsLeftHand ? EControllerHand::Left : EControllerHand::Right;

	// Check for grip button press
	// The grip button is typically used for grabbing in VR

	// Method 1: Check via MotionController component - disabled for compatibility
	// UMotionControllerComponent* MotionController = Hand->FindComponentByClass<UMotionControllerComponent>();
	// if (MotionController)
	// {
	//     // The motion controller doesn't directly expose button state,
	//     // so we need to check through the player controller input
	// }

	// Method 2: Check input directly through player controller
	// For OpenXR and most VR systems, the grip axis/button is standard

	// Check grip axis value - disabled for compatibility
	float GripValue = 0.0f; // Default to not pressed

	// VR input checking disabled for UE 5.6 compatibility
	/*
	if (bIsLeftHand)
	{
		// Check left grip axis - this is the standard OpenXR/SteamVR binding
		GripValue = PC->GetInputAnalogKeyValue(FKey("MotionController_Left_Grip_Axis"));

		// Also check digital grip button as fallback
		if (GripValue < 0.5f)
		{
			bool bGripPressed = PC->IsInputKeyDown(FKey("MotionController_Left_Grip1"));
			GripValue = bGripPressed ? 1.0f : 0.0f;
		}
	}
	else
	{
		// Check right grip axis
		GripValue = PC->GetInputAnalogKeyValue(FKey("MotionController_Right_Grip_Axis"));

		// Also check digital grip button as fallback
		if (GripValue < 0.5f)
		{
			bool bGripPressed = PC->IsInputKeyDown(FKey("MotionController_Right_Grip1"));
			GripValue = bGripPressed ? 1.0f : 0.0f;
		}
	}
	*/

	// Consider grip pressed if value is above threshold (0.5 = 50% squeezed)
	const float GripThreshold = 0.5f;
	bool bIsGripping = GripValue >= GripThreshold;

	// Method 3: Advanced gesture detection using hand tracking data
	// This would use OpenXR hand tracking to detect pinch/grab gestures
	// For now, we rely on grip button, but this can be extended

	// TODO: Implement advanced gesture recognition:
	// - Detect pinch gesture (thumb + index finger)
	// - Detect fist gesture (all fingers closed)
	// - Detect pointing gesture
	// - Calculate gesture confidence scores

	return bIsGripping;
}

// ============================================================================
// GRAB INTERACTION
// ============================================================================

bool UVRHarvestInteractionManager::TryGrabLeft()
{
	if (!LeftHandTarget || bLeftHandGrabbing)
	{
		return false;
	}
	
	if (LeftHandTarget->TryGrab(LeftHand))
	{
		bLeftHandGrabbing = true;
		return true;
	}
	
	return false;
}

bool UVRHarvestInteractionManager::TryGrabRight()
{
	if (!RightHandTarget || bRightHandGrabbing)
	{
		return false;
	}
	
	if (RightHandTarget->TryGrab(RightHand))
	{
		bRightHandGrabbing = true;
		return true;
	}
	
	return false;
}

void UVRHarvestInteractionManager::ReleaseLeft()
{
	if (LeftHandTarget && bLeftHandGrabbing)
	{
		LeftHandTarget->Release();
		bLeftHandGrabbing = false;
		LeftHandTarget = nullptr;
	}
}

void UVRHarvestInteractionManager::ReleaseRight()
{
	if (RightHandTarget && bRightHandGrabbing)
	{
		RightHandTarget->Release();
		bRightHandGrabbing = false;
		RightHandTarget = nullptr;
	}
}

void UVRHarvestInteractionManager::UpdateGrabInteractions(float DeltaTime)
{
	// Check for grab input
	// TODO: Replace with actual VR input
	// For now, this is a placeholder for the interaction system
	
	// Left hand
	if (IsHandGrabbing(LeftHand))
	{
		if (!bLeftHandGrabbing)
		{
			TryGrabLeft();
		}
	}
	else
	{
		if (bLeftHandGrabbing)
		{
			ReleaseLeft();
		}
	}
	
	// Right hand
	if (IsHandGrabbing(RightHand))
	{
		if (!bRightHandGrabbing)
		{
			TryGrabRight();
		}
	}
	else
	{
		if (bRightHandGrabbing)
		{
			ReleaseRight();
		}
	}
}

// ============================================================================
// HARVEST COLLECTION TRACKING
// ============================================================================

void UVRHarvestInteractionManager::RecordHarvest(const FHarvestCollection& Harvest)
{
	// Add to history
	HarvestHistory.Add(Harvest);
	
	// Trim history if too large
	if (HarvestHistory.Num() > MaxHistorySize)
	{
		HarvestHistory.RemoveAt(0);
	}
	
	// Update daily stats
	TotalHarvestedToday += Harvest.Quantity;
	
	// Recalculate average quality
	float TotalQuality = 0.0f;
	int32 Count = 0;
	
	for (const FHarvestCollection& Entry : HarvestHistory)
	{
		TotalQuality += Entry.Quality * Entry.Quantity;
		Count += Entry.Quantity;
	}
	
	if (Count > 0)
	{
		AverageQualityToday = TotalQuality / Count;
	}
	
	// Show feedback
	ShowHarvestFeedback(Harvest);
}

int32 UVRHarvestInteractionManager::GetTotalHarvested(const FString& CropName) const
{
	int32 Total = 0;
	
	for (const FHarvestCollection& Entry : HarvestHistory)
	{
		if (Entry.CropName == CropName)
		{
			Total += Entry.Quantity;
		}
	}
	
	return Total;
}

float UVRHarvestInteractionManager::GetAverageQuality(const FString& CropName) const
{
	float TotalQuality = 0.0f;
	int32 Count = 0;
	
	for (const FHarvestCollection& Entry : HarvestHistory)
	{
		if (Entry.CropName == CropName)
		{
			TotalQuality += Entry.Quality * Entry.Quantity;
			Count += Entry.Quantity;
		}
	}
	
	if (Count > 0)
	{
		return TotalQuality / Count;
	}
	
	return 0.0f;
}

void UVRHarvestInteractionManager::ClearHarvestHistory()
{
	HarvestHistory.Empty();
	TotalHarvestedToday = 0;
	AverageQualityToday = 0.0f;
}

void UVRHarvestInteractionManager::GetHarvestStatistics(int32& OutTotalHarvested, float& OutAverageQuality) const
{
	OutTotalHarvested = TotalHarvestedToday;
	OutAverageQuality = AverageQualityToday;
}

// ============================================================================
// FARM PLOT QUERIES
// ============================================================================

void UVRHarvestInteractionManager::UpdateNearbyFarmPlots()
{
	if (!GetOwner())
	{
		return;
	}
	
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	
	// Find all farm plots in world
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFarmPlot::StaticClass(), FoundActors);
	
	// Clear current list
	NearbyFarmPlots.Empty();
	
	// Add plots within detection radius
	for (AActor* Actor : FoundActors)
	{
		AFarmPlot* FarmPlot = Cast<AFarmPlot>(Actor);
		if (FarmPlot)
		{
			float Distance = FVector::Dist(OwnerLocation, FarmPlot->GetActorLocation());
			if (Distance <= FarmPlotDetectionRadius)
			{
				NearbyFarmPlots.Add(FarmPlot);
			}
		}
	}
}

TArray<UVRHarvestableComponent*> UVRHarvestInteractionManager::GetAllHarvestableCrops() const
{
	TArray<UVRHarvestableComponent*> AllHarvestables;
	
	// Get harvestables from all nearby farm plots
	for (AFarmPlot* FarmPlot : NearbyFarmPlots)
	{
		if (!FarmPlot)
		{
			continue;
		}
		
		// Get all harvestable components in the farm plot
		TArray<UVRHarvestableComponent*> PlotHarvestables;
		FarmPlot->GetComponents<UVRHarvestableComponent>(PlotHarvestables);
		
		AllHarvestables.Append(PlotHarvestables);
	}
	
	return AllHarvestables;
}

int32 UVRHarvestInteractionManager::GetReadyCropCount() const
{
	int32 Count = 0;
	
	TArray<UVRHarvestableComponent*> AllHarvestables = GetAllHarvestableCrops();
	
	for (UVRHarvestableComponent* Harvestable : AllHarvestables)
	{
		if (Harvestable && Harvestable->IsReadyForHarvest())
		{
			Count++;
		}
	}
	
	return Count;
}

// ============================================================================
// VISUAL FEEDBACK
// ============================================================================

void UVRHarvestInteractionManager::ShowHarvestFeedback(const FHarvestCollection& Harvest)
{
	// TODO: Show UI feedback for harvest
	// This would display a notification showing what was harvested
	
	UE_LOG(LogTemp, Log, TEXT("Harvested: %s x%d (Quality: %.2f)"), 
		*Harvest.CropName, Harvest.Quantity, Harvest.Quality);
}

void UVRHarvestInteractionManager::UpdateProximityIndicators()
{
	// TODO: Update visual indicators showing nearby harvestable crops
	// This could highlight crops when hands are near
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

TArray<UVRHarvestableComponent*> UVRHarvestInteractionManager::FindHarvestablesInRange(FVector Location, float Radius) const
{
	TArray<UVRHarvestableComponent*> Result;
	
	// Get all harvestables from nearby farm plots
	TArray<UVRHarvestableComponent*> AllHarvestables = GetAllHarvestableCrops();
	
	// Filter by distance
	for (UVRHarvestableComponent* Harvestable : AllHarvestables)
	{
		if (!Harvestable || !Harvestable->GetOwner())
		{
			continue;
		}
		
		// Check if harvestable
		if (!Harvestable->CanBeGrabbed())
		{
			continue;
		}
		
		// Check distance
		float Distance = FVector::Dist(Location, Harvestable->GetOwner()->GetActorLocation());
		if (Distance <= Radius)
		{
			Result.Add(Harvestable);
		}
	}
	
	return Result;
}

UVRHarvestableComponent* UVRHarvestInteractionManager::GetClosestHarvestable(FVector Location, const TArray<UVRHarvestableComponent*>& Harvestables) const
{
	if (Harvestables.Num() == 0)
	{
		return nullptr;
	}
	
	UVRHarvestableComponent* Closest = nullptr;
	float ClosestDistance = FLT_MAX;
	
	for (UVRHarvestableComponent* Harvestable : Harvestables)
	{
		if (!Harvestable || !Harvestable->GetOwner())
		{
			continue;
		}
		
		float Distance = FVector::Dist(Location, Harvestable->GetOwner()->GetActorLocation());
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			Closest = Harvestable;
		}
	}
	
	return Closest;
}

void UVRHarvestInteractionManager::UpdateDailyStatistics()
{
	// Reset daily stats
	TotalHarvestedToday = 0;
	AverageQualityToday = 0.0f;
	
	// Clear old history (keep only today's harvests)
	FDateTime Now = FDateTime::Now();
	HarvestHistory.RemoveAll([Now](const FHarvestCollection& Entry)
	{
		return (Now - Entry.HarvestTime).GetDays() >= 1;
	});
	
	// Recalculate stats from remaining history
	float TotalQuality = 0.0f;
	int32 Count = 0;
	
	for (const FHarvestCollection& Entry : HarvestHistory)
	{
		TotalHarvestedToday += Entry.Quantity;
		TotalQuality += Entry.Quality * Entry.Quantity;
		Count += Entry.Quantity;
	}
	
	if (Count > 0)
	{
		AverageQualityToday = TotalQuality / Count;
	}
}

bool UVRHarvestInteractionManager::IsNewDay()
{
	FDateTime Now = FDateTime::Now();
	
	if (Now.GetDay() != LastDayCheck.GetDay() || 
	    Now.GetMonth() != LastDayCheck.GetMonth() || 
	    Now.GetYear() != LastDayCheck.GetYear())
	{
		LastDayCheck = Now;
		return true;
	}
	
	return false;
}
