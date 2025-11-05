// Copyright Epic Games, Inc. All Rights Reserved.

#include "VRCropInspectionComponent.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "FarmPlot.h"
#include "CropDefinition.h"
#include "CropHealthSystem.h"
#include "CropGrowthCalculator.h"
#include "VRCropInfoWidget.h"
#include "Blueprint/UserWidget.h"      // For CreateWidget, UUserWidget
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UVRCropInspectionComponent::UVRCropInspectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Default configuration
	InspectionRange = 200.0f; // 2 meters
	LookAtAngleTolerance = 15.0f; // 15 degrees
	LookAtDuration = 0.5f; // 0.5 seconds
	bShowDebugVisualization = false;

	// State initialization
	CurrentFarmPlot = nullptr;
	CurrentCropPosition = FIntPoint(-1, -1);
	bIsLookingAtCrop = false;
	CurrentLookAtTime = 0.0f;
	bIsInfoVisible = false;
	CropInfoWidget = nullptr;
}

void UVRCropInspectionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Create crop info widget if class is specified
	if (CropInfoWidgetClass)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PC)
		{
			CropInfoWidget = CreateWidget<UVRCropInfoWidget>(PC, CropInfoWidgetClass);
			if (CropInfoWidget)
			{
				CropInfoWidget->AddToViewport();
				CropInfoWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("VRCropInspectionComponent: Initialized (Range: %.1fm, Angle: %.1f°)"),
		InspectionRange / 100.0f, LookAtAngleTolerance);
}

void UVRCropInspectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateInspection(DeltaTime);
}

// ============================================================================
// INSPECTION FUNCTIONS
// ============================================================================

void UVRCropInspectionComponent::UpdateInspection(float DeltaTime)
{
	AFarmPlot* DetectedFarmPlot = nullptr;
	FIntPoint DetectedPosition(-1, -1);

	// Check if looking at a crop
	bool bLookingAtCrop = DetectLookAtCrop(DetectedFarmPlot, DetectedPosition);

	if (bLookingAtCrop)
	{
		// Check if this is the same crop we were looking at
		bool bSameCrop = (DetectedFarmPlot == CurrentFarmPlot) && (DetectedPosition == CurrentCropPosition);

		if (bSameCrop)
		{
			// Continue looking at same crop
			CurrentLookAtTime += DeltaTime;

			// Show info after looking for required duration
			if (CurrentLookAtTime >= LookAtDuration && !bIsInfoVisible)
			{
				// Get inspection data
				CurrentInspectionData = GetCropInspectionData(CurrentFarmPlot, CurrentCropPosition);
				ShowCropInfo(CurrentInspectionData);
			}
			else if (bIsInfoVisible)
			{
				// Update info continuously while looking
				CurrentInspectionData = GetCropInspectionData(CurrentFarmPlot, CurrentCropPosition);
				UpdateCropInfo(CurrentInspectionData);
			}
		}
		else
		{
			// Looking at different crop - reset
			if (bIsInfoVisible)
			{
				HideCropInfo();
			}

			CurrentFarmPlot = DetectedFarmPlot;
			CurrentCropPosition = DetectedPosition;
			CurrentLookAtTime = 0.0f;
			bIsLookingAtCrop = true;
		}
	}
	else
	{
		// Not looking at any crop
		if (bIsLookingAtCrop || bIsInfoVisible)
		{
			// Hide info
			HideCropInfo();
			CurrentFarmPlot = nullptr;
			CurrentCropPosition = FIntPoint(-1, -1);
			CurrentLookAtTime = 0.0f;
			bIsLookingAtCrop = false;
		}
	}

	// Debug visualization
	if (bShowDebugVisualization)
	{
		FVector CameraLocation, CameraForward;
		GetCameraInfo(CameraLocation, CameraForward);
		DrawDebugVisualization(CameraLocation, CameraForward);
	}
}

bool UVRCropInspectionComponent::DetectLookAtCrop(AFarmPlot*& OutFarmPlot, FIntPoint& OutGridPosition)
{
	// Get camera info
	FVector CameraLocation, CameraForward;
	GetCameraInfo(CameraLocation, CameraForward);

	// Find nearby farm plots
	TArray<AFarmPlot*> NearbyPlots = FindNearbyFarmPlots(CameraLocation, InspectionRange);

	// Check each farm plot for crops in look direction
	for (AFarmPlot* FarmPlot : NearbyPlots)
	{
		if (!FarmPlot)
		{
			continue;
		}

		// Check each crop cell
		int32 GridRes = FarmPlot->GridResolution;
		for (int32 Y = 0; Y < GridRes; ++Y)
		{
			for (int32 X = 0; X < GridRes; ++X)
			{
				FIntPoint GridPos(X, Y);

				// Check if cell has a crop
				int32 CellIndex = Y * GridRes + X;
				if (CellIndex >= FarmPlot->CropGrid.Num())
				{
					continue;
				}

				const FCropCell& Cell = FarmPlot->CropGrid[CellIndex];
				if (!Cell.CropType || Cell.GrowthProgress <= 0.0f)
				{
					continue;
				}

				// Get crop world location
				FVector CropLocation = GetCropCellWorldLocation(FarmPlot, GridPos);

				// Check distance
				float Distance = FVector::Dist(CameraLocation, CropLocation);
				if (Distance > InspectionRange)
				{
					continue;
				}

				// Check if looking at this crop
				if (IsLookingAt(CameraLocation, CameraForward, CropLocation, LookAtAngleTolerance))
				{
					OutFarmPlot = FarmPlot;
					OutGridPosition = GridPos;
					return true;
				}
			}
		}
	}

	return false;
}

FCropInspectionData UVRCropInspectionComponent::GetCropInspectionData(AFarmPlot* FarmPlot, FIntPoint GridPosition)
{
	FCropInspectionData Data;

	if (!FarmPlot || !FarmPlot->IsValidGridPosition(GridPosition))
	{
		return Data;
	}

	// Get crop cell
	int32 CellIndex = GridPosition.Y * FarmPlot->GridResolution + GridPosition.X;
	if (CellIndex >= FarmPlot->CropGrid.Num())
	{
		return Data;
	}

	const FCropCell& Cell = FarmPlot->CropGrid[CellIndex];
	if (!Cell.CropType)
	{
		return Data;
	}

	// Basic crop info
	Data.CropName = Cell.CropType->CropName;
	Data.GrowthProgress = Cell.GrowthProgress;
	Data.Health = Cell.Health;
	Data.WorldLocation = GetCropCellWorldLocation(FarmPlot, GridPosition);

	// Growth stage info
	Data.GrowthStageName = FarmPlot->GetCropGrowthStageName(GridPosition);

	// Health status
	ECropHealthStatus HealthStatus = UCropHealthSystem::GetHealthStatus(Cell.Health);
	Data.HealthStatus = UCropHealthSystem::GetHealthStatusDescription(HealthStatus);
	Data.HealthStatusColor = UCropHealthSystem::GetHealthStatusColor(HealthStatus);

	// Get detailed health information
	FFarmEnvironment Environment = FarmPlot->GetEnvironmentalConditions();
	FEnvironmentalFactors EnvFactors;
	EnvFactors.Temperature = Environment.Temperature;
	EnvFactors.Humidity = Environment.Humidity;
	EnvFactors.Sunlight = Environment.Sunlight;
	EnvFactors.SoilQuality = FarmPlot->SoilQuality;
	EnvFactors.WaterLevel = FarmPlot->WaterLevel;
	EnvFactors.Fertility = FarmPlot->Fertility;

	FHealthCalculationResult HealthResult = UCropHealthSystem::CalculateCropHealth(
		Cell.CropType,
		EnvFactors,
		Cell.Health,
		FarmPlot->WaterLevel,
		FarmPlot->Fertility,
		Cell.TimeSinceLastWater,
		0.0f // No delta time for snapshot
	);

	Data.PrimaryIssue = HealthResult.PrimaryIssue;
	Data.bNeedsWater = HealthResult.bNeedsWater;
	Data.bNeedsFertilizer = HealthResult.bNeedsFertilizer;

	// Time to harvest
	Data.TimeToHarvest = FarmPlot->GetTimeToHarvest(GridPosition);

	return Data;
}

void UVRCropInspectionComponent::ShowCropInfo(const FCropInspectionData& InspectionData)
{
	if (!CropInfoWidget)
	{
		return;
	}

	CropInfoWidget->UpdateCropInfo(InspectionData);
	CropInfoWidget->SetVisibility(ESlateVisibility::Visible);
	bIsInfoVisible = true;

	UE_LOG(LogTemp, Verbose, TEXT("VRCropInspection: Showing info for %s (Health: %.1f%%, Growth: %.1f%%)"),
		*InspectionData.CropName, InspectionData.Health * 100.0f, InspectionData.GrowthProgress * 100.0f);
}

void UVRCropInspectionComponent::HideCropInfo()
{
	if (!CropInfoWidget)
	{
		return;
	}

	CropInfoWidget->SetVisibility(ESlateVisibility::Hidden);
	bIsInfoVisible = false;
}

void UVRCropInspectionComponent::UpdateCropInfo(const FCropInspectionData& InspectionData)
{
	if (!CropInfoWidget || !bIsInfoVisible)
	{
		return;
	}

	CropInfoWidget->UpdateCropInfo(InspectionData);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void UVRCropInspectionComponent::GetCameraInfo(FVector& OutLocation, FVector& OutForward) const
{
	// Get player pawn
	APawn* PlayerPawn = Cast<APawn>(GetOwner());
	if (!PlayerPawn)
	{
		PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	if (PlayerPawn)
	{
		// Try to get camera component
		UCameraComponent* Camera = PlayerPawn->FindComponentByClass<UCameraComponent>();
		if (Camera)
		{
			OutLocation = Camera->GetComponentLocation();
			OutForward = Camera->GetForwardVector();
			return;
		}

		// Fallback to pawn location and rotation
		OutLocation = PlayerPawn->GetActorLocation();
		OutForward = PlayerPawn->GetActorForwardVector();

		// Adjust for VR eye height
		OutLocation.Z += 160.0f; // Approximate eye height in cm
	}
	else
	{
		// Fallback to component location
		OutLocation = GetOwner()->GetActorLocation();
		OutForward = GetOwner()->GetActorForwardVector();
	}
}

TArray<AFarmPlot*> UVRCropInspectionComponent::FindNearbyFarmPlots(const FVector& Location, float Range) const
{
	TArray<AFarmPlot*> NearbyPlots;

	// Find all farm plots in world
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFarmPlot::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		AFarmPlot* FarmPlot = Cast<AFarmPlot>(Actor);
		if (FarmPlot)
		{
			// Check if farm plot is within range
			float Distance = FVector::Dist(Location, FarmPlot->GetActorLocation());
			if (Distance <= Range + FarmPlot->PlotSize.GetMax())
			{
				NearbyPlots.Add(FarmPlot);
			}
		}
	}

	return NearbyPlots;
}

FVector UVRCropInspectionComponent::GetCropCellWorldLocation(AFarmPlot* FarmPlot, FIntPoint GridPosition) const
{
	if (!FarmPlot)
	{
		return FVector::ZeroVector;
	}

	// Calculate cell size
	float CellSizeX = FarmPlot->PlotSize.X / FarmPlot->GridResolution;
	float CellSizeY = FarmPlot->PlotSize.Y / FarmPlot->GridResolution;

	// Calculate local offset from plot center
	FVector CellOffset(
		(GridPosition.X - FarmPlot->GridResolution / 2.0f) * CellSizeX + (CellSizeX / 2.0f),
		(GridPosition.Y - FarmPlot->GridResolution / 2.0f) * CellSizeY + (CellSizeY / 2.0f),
		50.0f // Height above ground (approximate crop center)
	);

	// Transform to world space
	return FarmPlot->GetActorLocation() + FarmPlot->GetActorRotation().RotateVector(CellOffset);
}

bool UVRCropInspectionComponent::IsLookingAt(const FVector& CameraLocation, const FVector& CameraForward, const FVector& TargetLocation, float AngleTolerance) const
{
	// Calculate direction to target
	FVector ToTarget = (TargetLocation - CameraLocation).GetSafeNormal();

	// Calculate angle between camera forward and target direction
	float DotProduct = FVector::DotProduct(CameraForward, ToTarget);
	float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

	return AngleDegrees <= AngleTolerance;
}

FString UVRCropInspectionComponent::FormatTimeDuration(float Seconds) const
{
	if (Seconds < 60.0f)
	{
		return FString::Printf(TEXT("%.0fs"), Seconds);
	}
	else if (Seconds < 3600.0f)
	{
		int32 Minutes = FMath::FloorToInt(Seconds / 60.0f);
		int32 Secs = FMath::FloorToInt(Seconds) % 60;
		return FString::Printf(TEXT("%dm %ds"), Minutes, Secs);
	}
	else
	{
		int32 Hours = FMath::FloorToInt(Seconds / 3600.0f);
		int32 Minutes = FMath::FloorToInt((Seconds - Hours * 3600.0f) / 60.0f);
		return FString::Printf(TEXT("%dh %dm"), Hours, Minutes);
	}
}

void UVRCropInspectionComponent::DrawDebugVisualization(const FVector& CameraLocation, const FVector& CameraForward)
{
	if (!GetWorld())
	{
		return;
	}

	// Draw inspection range sphere
	DrawDebugSphere(GetWorld(), CameraLocation, InspectionRange, 16, FColor::Cyan, false, 0.0f, 0, 2.0f);

	// Draw look direction
	FVector LookEnd = CameraLocation + (CameraForward * InspectionRange);
	DrawDebugLine(GetWorld(), CameraLocation, LookEnd, FColor::Green, false, 0.0f, 0, 3.0f);

	// Draw look cone
	float ConeAngle = FMath::DegreesToRadians(LookAtAngleTolerance);
	DrawDebugCone(GetWorld(), CameraLocation, CameraForward, InspectionRange, ConeAngle, ConeAngle, 16, FColor::Yellow, false, 0.0f, 0, 1.0f);

	// Draw currently inspected crop
	if (bIsLookingAtCrop && CurrentFarmPlot)
	{
		FVector CropLocation = GetCropCellWorldLocation(CurrentFarmPlot, CurrentCropPosition);
		DrawDebugSphere(GetWorld(), CropLocation, 20.0f, 8, FColor::Red, false, 0.0f, 0, 3.0f);
		DrawDebugLine(GetWorld(), CameraLocation, CropLocation, FColor::Red, false, 0.0f, 0, 2.0f);
	}
}
