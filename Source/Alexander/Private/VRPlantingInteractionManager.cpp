// Copyright Epic Games, Inc. All Rights Reserved.

#include "VRPlantingInteractionManager.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Math/Quat.h"                // For FQuat operations
#include "VRSeedItem.h"
#include "VRPlantingGestureDetector.h"
#include "FarmPlot.h"
#include "CropDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "MotionControllerComponent.h"          // For UMotionControllerComponent
#include "Components/InputComponent.h"          // For UInputComponent

UVRPlantingInteractionManager::UVRPlantingInteractionManager()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Initialize configuration
	bEnableGestureDetection = true;
	bAutoPlantOnRelease = true;
	PlantingRange = 200.0f; // 2 meters

	// Initialize state
	CurrentSeedItem = nullptr;
	CurrentHandActor = nullptr;
	GestureDetector = nullptr;
	PlantingPreviewActor = nullptr;
}

void UVRPlantingInteractionManager::BeginPlay()
{
	Super::BeginPlay();

	// Initialize gesture detector if enabled
	if (bEnableGestureDetection)
	{
		InitializeGestureDetector();
	}

	// Create planting preview
	CreatePlantingPreview();
}

void UVRPlantingInteractionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update planting preview if holding seed
	if (CurrentSeedItem && CurrentHandActor)
	{
		FVector SeedLocation = CurrentSeedItem->GetActorLocation();
		UpdatePlantingPreview(SeedLocation);
	}
}

AVRSeedItem* UVRPlantingInteractionManager::SpawnSeedItem(UCropDefinition* CropType, FVector Location, int32 SeedCount)
{
	if (!CropType)
	{
		UE_LOG(LogTemp, Warning, TEXT("VRPlantingInteractionManager: Cannot spawn seed without crop type"));
		return nullptr;
	}

	if (!SeedItemClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("VRPlantingInteractionManager: SeedItemClass not set"));
		return nullptr;
	}

	// Spawn seed item
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AVRSeedItem* SeedItem = GetWorld()->SpawnActor<AVRSeedItem>(SeedItemClass, Location, FRotator::ZeroRotator, SpawnParams);

	if (SeedItem)
	{
		SeedItem->SetCropType(CropType);
		SeedItem->SeedCount = SeedCount;

		UE_LOG(LogTemp, Log, TEXT("VRPlantingInteractionManager: Spawned seed item for %s"), *CropType->CropName);
	}

	return SeedItem;
}

void UVRPlantingInteractionManager::OnSeedGrabbed(AVRSeedItem* SeedItem, AActor* HandActor)
{
	if (!SeedItem || !HandActor)
	{
		return;
	}

	CurrentSeedItem = SeedItem;
	CurrentHandActor = HandActor;

	// Notify seed item
	SeedItem->OnGrabbed(HandActor);

	// Start gesture tracking
	if (bEnableGestureDetection && GestureDetector)
	{
		GestureDetector->StartTracking(SeedItem, HandActor);
	}

	// Play grab haptics
	PlayGrabHaptics(HandActor);

	// Show planting preview
	ShowPlantingPreview(SeedItem->GetActorLocation(), false);

	UE_LOG(LogTemp, Log, TEXT("VRPlantingInteractionManager: Seed grabbed"));
}

void UVRPlantingInteractionManager::OnSeedReleased(AVRSeedItem* SeedItem)
{
	if (!SeedItem)
	{
		return;
	}

	// Check if we should auto-plant
	if (bAutoPlantOnRelease)
	{
		FVector SeedLocation = SeedItem->GetActorLocation();
		PlantSeedAtLocation(SeedItem, SeedLocation);
	}

	// Stop gesture tracking
	if (GestureDetector)
	{
		GestureDetector->StopTracking();
	}

	// Notify seed item
	SeedItem->OnReleased();

	// Hide planting preview
	HidePlantingPreview();

	// Clear current seed
	CurrentSeedItem = nullptr;
	CurrentHandActor = nullptr;

	UE_LOG(LogTemp, Log, TEXT("VRPlantingInteractionManager: Seed released"));
}

bool UVRPlantingInteractionManager::PlantSeedAtLocation(AVRSeedItem* SeedItem, FVector Location)
{
	if (!SeedItem)
	{
		return false;
	}

	// Find farm plot and grid position
	AFarmPlot* FarmPlot = nullptr;
	FIntPoint GridPosition;

	if (!IsValidPlantingLocation(Location, FarmPlot, GridPosition))
	{
		// Play invalid placement haptics
		if (CurrentHandActor)
		{
			PlayInvalidPlacementHaptics(CurrentHandActor);
		}

		UE_LOG(LogTemp, Warning, TEXT("VRPlantingInteractionManager: Invalid planting location"));
		return false;
	}

	// Plant seed in grid
	return PlantSeedInGrid(SeedItem, FarmPlot, GridPosition);
}

bool UVRPlantingInteractionManager::PlantSeedInGrid(AVRSeedItem* SeedItem, AFarmPlot* FarmPlot, FIntPoint GridPosition)
{
	if (!SeedItem || !FarmPlot)
	{
		return false;
	}

	UCropDefinition* CropType = SeedItem->GetCropType();
	if (!CropType)
	{
		UE_LOG(LogTemp, Warning, TEXT("VRPlantingInteractionManager: Seed has no crop type"));
		return false;
	}

	// Attempt to plant
	bool bPlanted = FarmPlot->PlantCrop(GridPosition, CropType);

	if (bPlanted)
	{
		// Play planting haptics
		if (CurrentHandActor)
		{
			PlayPlantingHaptics(CurrentHandActor);
		}

		// Consume seed
		SeedItem->ConsumeSeed();

		// Destroy seed if no more seeds
		if (SeedItem->GetSeedCount() <= 0)
		{
			SeedItem->Destroy();
		}

		UE_LOG(LogTemp, Log, TEXT("VRPlantingInteractionManager: Successfully planted %s at (%d, %d)"),
			*CropType->CropName, GridPosition.X, GridPosition.Y);

		return true;
	}

	return false;
}

bool UVRPlantingInteractionManager::IsValidPlantingLocation(FVector Location, AFarmPlot*& OutFarmPlot, FIntPoint& OutGridPosition)
{
	// Find farm plot at location
	AFarmPlot* FarmPlot = FindFarmPlotAtLocation(Location);

	if (!FarmPlot)
	{
		return false;
	}

	// Calculate grid position
	FIntPoint GridPosition = CalculateGridPosition(FarmPlot, Location);

	// Validate grid position
	if (GridPosition.X < 0 || GridPosition.Y < 0 ||
		GridPosition.X >= FarmPlot->GridResolution || GridPosition.Y >= FarmPlot->GridResolution)
	{
		return false;
	}

	OutFarmPlot = FarmPlot;
	OutGridPosition = GridPosition;
	return true;
}

void UVRPlantingInteractionManager::PlayPlantingHaptics(AActor* HandActor)
{
	PlayHapticEffect(HandActor, HapticConfig.PlantingIntensity, 0.3f);
}

void UVRPlantingInteractionManager::PlayGrabHaptics(AActor* HandActor)
{
	PlayHapticEffect(HandActor, HapticConfig.GrabIntensity, 0.1f);
}

void UVRPlantingInteractionManager::PlayInvalidPlacementHaptics(AActor* HandActor)
{
	PlayHapticEffect(HandActor, HapticConfig.InvalidPlacementIntensity, 0.15f);
}

EControllerHand UVRPlantingInteractionManager::GetControllerHand(AActor* HandActor)
{
	// This would be determined by the VR pawn implementation
	// For now, default to right hand
	// In a real implementation, check the hand actor's properties
	return EControllerHand::Right;
}

void UVRPlantingInteractionManager::ShowPlantingPreview(FVector Location, bool bIsValid)
{
	if (!PlantingPreviewActor)
	{
		return;
	}

	PlantingPreviewActor->SetActorHiddenInGame(false);
	PlantingPreviewActor->SetActorLocation(Location);

	// Update preview color based on validity
	// This would be implemented in Blueprint or with a material parameter
}

void UVRPlantingInteractionManager::HidePlantingPreview()
{
	if (PlantingPreviewActor)
	{
		PlantingPreviewActor->SetActorHiddenInGame(true);
	}
}

void UVRPlantingInteractionManager::UpdatePlantingPreview(FVector Location)
{
	if (!PlantingPreviewActor || !CurrentSeedItem)
	{
		return;
	}

	// Check if location is valid
	AFarmPlot* FarmPlot = nullptr;
	FIntPoint GridPosition;
	bool bIsValid = IsValidPlantingLocation(Location, FarmPlot, GridPosition);

	// Update preview
	ShowPlantingPreview(Location, bIsValid);
}

void UVRPlantingInteractionManager::InitializeGestureDetector()
{
	// Create gesture detector component
	GestureDetector = NewObject<UVRPlantingGestureDetector>(this, TEXT("GestureDetector"));

	if (GestureDetector)
	{
		GestureDetector->RegisterComponent();
		UE_LOG(LogTemp, Log, TEXT("VRPlantingInteractionManager: Gesture detector initialized"));
	}
}

void UVRPlantingInteractionManager::CreatePlantingPreview()
{
	// Create a simple preview actor
	// In a real implementation, this would be a Blueprint actor with visual effects
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PlantingPreviewActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (PlantingPreviewActor)
	{
		PlantingPreviewActor->SetActorHiddenInGame(true);
		UE_LOG(LogTemp, Log, TEXT("VRPlantingInteractionManager: Planting preview created"));
	}
}

AFarmPlot* UVRPlantingInteractionManager::FindFarmPlotAtLocation(FVector Location)
{
	// Sphere trace to find farm plots
	TArray<FHitResult> HitResults;
	FVector StartLocation = Location + FVector(0, 0, 50.0f);
	FVector EndLocation = Location - FVector(0, 0, 100.0f);

	FCollisionQueryParams QueryParams;
	if (CurrentSeedItem)
	{
		QueryParams.AddIgnoredActor(CurrentSeedItem);
	}

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_WorldStatic,
		FCollisionShape::MakeSphere(PlantingRange),
		QueryParams
	);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			// Check if hit actor is a farm plot
			AFarmPlot* FarmPlot = Cast<AFarmPlot>(Hit.GetActor());
			if (FarmPlot)
			{
				return FarmPlot;
			}

			// Check if hit component belongs to a farm plot
			if (Hit.GetComponent())
			{
				FarmPlot = Cast<AFarmPlot>(Hit.GetComponent()->GetOwner());
				if (FarmPlot)
				{
					return FarmPlot;
				}
			}
		}
	}

	return nullptr;
}

FIntPoint UVRPlantingInteractionManager::CalculateGridPosition(AFarmPlot* FarmPlot, FVector WorldLocation)
{
	if (!FarmPlot)
	{
		return FIntPoint(-1, -1);
	}

	// Get farm plot location and rotation
	FVector PlotLocation = FarmPlot->GetActorLocation();
	FRotator PlotRotation = FarmPlot->GetActorRotation();

	// Transform world location to local space
	FVector LocalLocation = PlotRotation.UnrotateVector(WorldLocation - PlotLocation);

	// Calculate grid position
	FVector2D PlotSize = FarmPlot->PlotSize;
	int32 GridResolution = FarmPlot->GridResolution;

	// Normalize to 0-1 range
	float NormalizedX = (LocalLocation.X + PlotSize.X * 0.5f) / PlotSize.X;
	float NormalizedY = (LocalLocation.Y + PlotSize.Y * 0.5f) / PlotSize.Y;

	// Convert to grid coordinates
	int32 GridX = FMath::FloorToInt(NormalizedX * GridResolution);
	int32 GridY = FMath::FloorToInt(NormalizedY * GridResolution);

	// Clamp to valid range
	GridX = FMath::Clamp(GridX, 0, GridResolution - 1);
	GridY = FMath::Clamp(GridY, 0, GridResolution - 1);

	return FIntPoint(GridX, GridY);
}

void UVRPlantingInteractionManager::PlayHapticEffect(AActor* HandActor, float Intensity, float Duration)
{
	if (!HandActor)
	{
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	// Determine which hand
	EControllerHand Hand = GetControllerHand(HandActor);

	// Play haptic effect
	if (HapticConfig.PlantingHapticEffect)
	{
		PC->PlayHapticEffect(HapticConfig.PlantingHapticEffect, Hand, Intensity, false);
	}
	else
	{
		// Fallback to simple haptic feedback - simplified for UE 5.6 compatibility
		UE_LOG(LogTemp, Warning, TEXT("VRPlantingInteractionManager: No haptic effect configured, skipping feedback"));
	}

	UE_LOG(LogTemp, Log, TEXT("VRPlantingInteractionManager: Playing haptic feedback (Intensity: %.2f, Duration: %.2f)"), Intensity, Duration);
}
