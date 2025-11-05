// Copyright Epic Games, Inc. All Rights Reserved.

#include "VRSeedItem.h"
#include "FarmPlot.h"
#include "CropDefinition.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "MotionControllerComponent.h"          // For UMotionControllerComponent
#include "Math/UnrealMathUtility.h"
#include "Math/Quat.h"                // For FQuat operations
#include "Math/Vector.h"
#include "Math/Transform.h"
#include "PhysicsEngine/BodyInstance.h"

AVRSeedItem::AVRSeedItem()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(RootComp);

	// Create seed mesh
	SeedMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SeedMesh"));
	SeedMesh->SetupAttachment(RootComp);
	SeedMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SeedMesh->SetCollisionResponseToAllChannels(ECR_Block);
	SeedMesh->SetSimulatePhysics(true);
	SeedMesh->SetMassOverrideInKg(NAME_None, 0.01f); // Very light

	// Create grab collision sphere
	GrabCollision = CreateDefaultSubobject<USphereComponent>(TEXT("GrabCollision"));
	GrabCollision->SetupAttachment(SeedMesh);
	GrabCollision->SetSphereRadius(10.0f);
	GrabCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GrabCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	GrabCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Initialize properties
	SeedCount = 1;
	SeedColor = FLinearColor::Green;
	bIsGrabbed = false;
	GrabbingHand = nullptr;
	bIsOverPlantableArea = false;
	TargetFarmPlot = nullptr;
	TargetGridPosition = FIntPoint(-1, -1);
	PlantingDetectionRadius = 50.0f;
	PlantingHeightThreshold = 20.0f;
}

void AVRSeedItem::BeginPlay()
{
	Super::BeginPlay();

	// Create dynamic material instance for visual feedback
	if (SeedMesh && SeedMesh->GetMaterial(0))
	{
		SeedMaterialInstance = SeedMesh->CreateDynamicMaterialInstance(0);
		if (SeedMaterialInstance)
		{
			SeedMaterialInstance->SetVectorParameterValue(TEXT("SeedColor"), SeedColor);
		}
	}
}

void AVRSeedItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update planting target if grabbed
	if (bIsGrabbed)
	{
		UpdatePlantingTarget();
	}
}

void AVRSeedItem::OnGrabbed(AActor* GrabbingActor)
{
	bIsGrabbed = true;
	GrabbingHand = GrabbingActor;

	// Disable physics while grabbed
	if (SeedMesh)
	{
		SeedMesh->SetSimulatePhysics(false);
	}

	UE_LOG(LogTemp, Log, TEXT("VRSeedItem: Grabbed by %s"), *GrabbingActor->GetName());
}

void AVRSeedItem::OnReleased()
{
	bIsGrabbed = false;

	// Try to plant if over plantable area
	if (bIsOverPlantableArea && TargetFarmPlot)
	{
		if (TryPlantSeed())
		{
			UE_LOG(LogTemp, Log, TEXT("VRSeedItem: Successfully planted seed"));
			return; // Seed was planted, actor will be destroyed or hidden
		}
	}

	// Re-enable physics if not planted
	if (SeedMesh)
	{
		SeedMesh->SetSimulatePhysics(true);
	}

	GrabbingHand = nullptr;
	bIsOverPlantableArea = false;
	TargetFarmPlot = nullptr;
	TargetGridPosition = FIntPoint(-1, -1);

	UE_LOG(LogTemp, Log, TEXT("VRSeedItem: Released"));
}

bool AVRSeedItem::CanPlantAtCurrentLocation() const
{
	if (!bIsOverPlantableArea || !TargetFarmPlot || !CropType)
	{
		return false;
	}

	if (TargetGridPosition.X < 0 || TargetGridPosition.Y < 0)
	{
		return false;
	}

	if (SeedCount <= 0)
	{
		return false;
	}

	return true;
}

bool AVRSeedItem::TryPlantSeed()
{
	if (!CanPlantAtCurrentLocation())
	{
		UE_LOG(LogTemp, Warning, TEXT("VRSeedItem: Cannot plant at current location"));
		return false;
	}

	// Attempt to plant crop
	bool bPlanted = TargetFarmPlot->PlantCrop(TargetGridPosition, CropType);

	if (bPlanted)
	{
		// Play haptic feedback
		PlayPlantingHaptics();

		// Consume seed
		ConsumeSeed();

		// Destroy or hide seed if no more seeds
		if (SeedCount <= 0)
		{
			Destroy();
		}

		UE_LOG(LogTemp, Log, TEXT("VRSeedItem: Planted %s at grid position (%d, %d)"),
			*CropType->CropName, TargetGridPosition.X, TargetGridPosition.Y);

		return true;
	}

	return false;
}

void AVRSeedItem::UpdatePlantingTarget()
{
	FVector CurrentLocation = GetActorLocation();

	// Find farm plot at current location
	AFarmPlot* FarmPlot = FindFarmPlotAtLocation(CurrentLocation);

	if (FarmPlot)
	{
		// Calculate grid position
		FIntPoint GridPos = CalculateGridPosition(FarmPlot, CurrentLocation);

		// Check if position is valid
		if (GridPos.X >= 0 && GridPos.Y >= 0 &&
			GridPos.X < FarmPlot->GridResolution && GridPos.Y < FarmPlot->GridResolution)
		{
			bIsOverPlantableArea = true;
			TargetFarmPlot = FarmPlot;
			TargetGridPosition = GridPos;

			// Update visual feedback
			UpdatePlantingVisuals();

			return;
		}
	}

	// Not over plantable area
	bIsOverPlantableArea = false;
	TargetFarmPlot = nullptr;
	TargetGridPosition = FIntPoint(-1, -1);
	UpdatePlantingVisuals();
}

void AVRSeedItem::SetCropType(UCropDefinition* NewCropType)
{
	CropType = NewCropType;

	// Update seed color based on crop
	if (CropType && SeedMaterialInstance)
	{
		// Use a default color scheme based on crop name
		// In a real implementation, this would come from the crop definition
		SeedMaterialInstance->SetVectorParameterValue(TEXT("SeedColor"), SeedColor);
	}
}

bool AVRSeedItem::ConsumeSeed()
{
	if (SeedCount > 0)
	{
		SeedCount--;
		return true;
	}
	return false;
}

AFarmPlot* AVRSeedItem::FindFarmPlotAtLocation(FVector Location)
{
	// Sphere trace to find farm plots
	TArray<FHitResult> HitResults;
	FVector StartLocation = Location + FVector(0, 0, PlantingHeightThreshold);
	FVector EndLocation = Location - FVector(0, 0, PlantingHeightThreshold * 2);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_WorldStatic,
		FCollisionShape::MakeSphere(PlantingDetectionRadius),
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

FIntPoint AVRSeedItem::CalculateGridPosition(AFarmPlot* FarmPlot, FVector WorldLocation)
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

void AVRSeedItem::UpdatePlantingVisuals()
{
	if (!SeedMaterialInstance)
	{
		return;
	}

	// Change seed color based on planting state
	if (bIsOverPlantableArea)
	{
		// Green tint when over plantable area
		SeedMaterialInstance->SetVectorParameterValue(TEXT("PlantingFeedback"), FLinearColor::Green);
		SeedMaterialInstance->SetScalarParameterValue(TEXT("FeedbackIntensity"), 0.5f);
	}
	else
	{
		// No tint when not over plantable area
		SeedMaterialInstance->SetScalarParameterValue(TEXT("FeedbackIntensity"), 0.0f);
	}
}

void AVRSeedItem::PlayPlantingHaptics()
{
	if (!GrabbingHand)
	{
		return;
	}

	// Play haptic feedback on the grabbing hand
	// This would integrate with Unreal's haptic feedback system
	// For now, we'll use a simple haptic effect

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		// Play a short, medium-intensity haptic pulse
		// Intensity: 0.6, Duration: 0.2 seconds
		PC->PlayHapticEffect(
			nullptr, // Haptic effect asset (would be set in Blueprint)
			EControllerHand::Right, // Would be determined by which hand grabbed
			0.6f, // Intensity
			false // Loop
		);

		UE_LOG(LogTemp, Log, TEXT("VRSeedItem: Playing planting haptic feedback"));
	}
}
