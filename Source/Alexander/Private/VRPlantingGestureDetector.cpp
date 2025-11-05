// Copyright Epic Games, Inc. All Rights Reserved.

#include "VRPlantingGestureDetector.h"
#include "VRSeedItem.h"
#include "FarmPlot.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UVRPlantingGestureDetector::UVRPlantingGestureDetector()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Initialize detection parameters
	MinDownwardVelocity = 50.0f; // 50 cm/s downward
	MaxPlantingHeight = 30.0f; // 30 cm above ground
	GestureTimeWindow = 0.5f; // 0.5 second window
	MinConfidenceThreshold = 0.7f; // 70% confidence
	MaxHistorySize = 30; // 30 frames of history (~0.5s at 60fps)
	bDebugVisualization = false;

	// Initialize state
	TrackedSeedItem = nullptr;
	TrackedHandActor = nullptr;
	bIsTracking = false;
	CurrentGesture = EPlantingGesture::None;
	GestureConfidence = 0.0f;
	GestureStartTime = 0.0f;
}

void UVRPlantingGestureDetector::BeginPlay()
{
	Super::BeginPlay();

	PositionHistory.Reserve(MaxHistorySize);
	TimeHistory.Reserve(MaxHistorySize);
}

void UVRPlantingGestureDetector::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsTracking || !TrackedSeedItem || !TrackedHandActor)
	{
		return;
	}

	// Update motion history
	FVector CurrentPosition = TrackedSeedItem->GetActorLocation();
	float CurrentTime = GetWorld()->GetTimeSeconds();
	UpdateMotionHistory(CurrentPosition, CurrentTime);

	// Detect gesture
	DetectGesture();

	// Draw debug visualization
	if (bDebugVisualization)
	{
		DrawDebugInfo();
	}
}

void UVRPlantingGestureDetector::StartTracking(AVRSeedItem* SeedItem, AActor* HandActor)
{
	if (!SeedItem || !HandActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("VRPlantingGestureDetector: Invalid seed item or hand actor"));
		return;
	}

	TrackedSeedItem = SeedItem;
	TrackedHandActor = HandActor;
	bIsTracking = true;
	GestureStartTime = GetWorld()->GetTimeSeconds();

	// Clear history
	PositionHistory.Empty();
	TimeHistory.Empty();

	UE_LOG(LogTemp, Log, TEXT("VRPlantingGestureDetector: Started tracking seed item"));
}

void UVRPlantingGestureDetector::StopTracking()
{
	bIsTracking = false;
	TrackedSeedItem = nullptr;
	TrackedHandActor = nullptr;
	CurrentGesture = EPlantingGesture::None;
	GestureConfidence = 0.0f;

	// Clear history
	PositionHistory.Empty();
	TimeHistory.Empty();

	UE_LOG(LogTemp, Log, TEXT("VRPlantingGestureDetector: Stopped tracking"));
}

FGestureDetectionResult UVRPlantingGestureDetector::DetectGesture()
{
	FGestureDetectionResult Result;

	if (!bIsTracking || !TrackedSeedItem)
	{
		return Result;
	}

	FVector CurrentPosition = TrackedSeedItem->GetActorLocation();

	// Check for downward motion gesture
	float DownwardConfidence = 0.0f;
	bool bDownwardMotion = DetectDownwardMotion(DownwardConfidence);

	// Check if near ground
	float HeightAboveGround = 0.0f;
	bool bNearGround = IsNearGround(CurrentPosition, HeightAboveGround);

	// Determine gesture type and confidence
	if (bDownwardMotion && bNearGround)
	{
		// Strong planting gesture detected
		Result.bGestureDetected = true;
		Result.GestureType = EPlantingGesture::DownwardMotion;
		Result.Confidence = DownwardConfidence;
		Result.GestureLocation = CurrentPosition;

		CurrentGesture = EPlantingGesture::DownwardMotion;
		GestureConfidence = DownwardConfidence;

		UE_LOG(LogTemp, Log, TEXT("VRPlantingGestureDetector: Downward motion detected (Confidence: %.2f)"), DownwardConfidence);
	}
	else if (bNearGround)
	{
		// Release near ground gesture
		Result.bGestureDetected = true;
		Result.GestureType = EPlantingGesture::ReleaseNearGround;
		Result.Confidence = 1.0f - (HeightAboveGround / MaxPlantingHeight);
		Result.GestureLocation = CurrentPosition;

		CurrentGesture = EPlantingGesture::ReleaseNearGround;
		GestureConfidence = Result.Confidence;
	}
	else
	{
		// No gesture detected
		CurrentGesture = EPlantingGesture::None;
		GestureConfidence = 0.0f;
	}

	return Result;
}

void UVRPlantingGestureDetector::ResetGesture()
{
	CurrentGesture = EPlantingGesture::None;
	GestureConfidence = 0.0f;
	PositionHistory.Empty();
	TimeHistory.Empty();
}

void UVRPlantingGestureDetector::UpdateMotionHistory(FVector CurrentPosition, float CurrentTime)
{
	// Add to history
	PositionHistory.Add(CurrentPosition);
	TimeHistory.Add(CurrentTime);

	// Limit history size
	if (PositionHistory.Num() > MaxHistorySize)
	{
		PositionHistory.RemoveAt(0);
		TimeHistory.RemoveAt(0);
	}
}

bool UVRPlantingGestureDetector::DetectDownwardMotion(float& OutConfidence)
{
	if (PositionHistory.Num() < 2)
	{
		OutConfidence = 0.0f;
		return false;
	}

	// Calculate velocity
	FVector Velocity = CalculateAverageVelocity(GestureTimeWindow);

	// Check if velocity is downward
	float DownwardVelocity = -Velocity.Z; // Negative Z is down in Unreal

	if (DownwardVelocity >= MinDownwardVelocity)
	{
		// Calculate confidence based on velocity magnitude
		// Higher downward velocity = higher confidence
		OutConfidence = FMath::Clamp(DownwardVelocity / (MinDownwardVelocity * 2.0f), 0.0f, 1.0f);
		return OutConfidence >= MinConfidenceThreshold;
	}

	OutConfidence = 0.0f;
	return false;
}

bool UVRPlantingGestureDetector::IsNearGround(FVector Position, float& OutHeight)
{
	// Trace downward to find ground
	FHitResult HitResult;
	FVector StartLocation = Position;
	FVector EndLocation = Position - FVector(0, 0, MaxPlantingHeight * 2.0f);

	FCollisionQueryParams QueryParams;
	if (TrackedSeedItem)
	{
		QueryParams.AddIgnoredActor(TrackedSeedItem);
	}

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_WorldStatic,
		QueryParams
	);

	if (bHit)
	{
		OutHeight = Position.Z - HitResult.Location.Z;
		return OutHeight <= MaxPlantingHeight;
	}

	OutHeight = MaxPlantingHeight;
	return false;
}

FVector UVRPlantingGestureDetector::CalculateVelocity()
{
	if (PositionHistory.Num() < 2)
	{
		return FVector::ZeroVector;
	}

	int32 LastIndex = PositionHistory.Num() - 1;
	FVector PositionDelta = PositionHistory[LastIndex] - PositionHistory[LastIndex - 1];
	float TimeDelta = TimeHistory[LastIndex] - TimeHistory[LastIndex - 1];

	if (TimeDelta > 0.0f)
	{
		return PositionDelta / TimeDelta;
	}

	return FVector::ZeroVector;
}

FVector UVRPlantingGestureDetector::CalculateAverageVelocity(float TimeWindow)
{
	if (PositionHistory.Num() < 2)
	{
		return FVector::ZeroVector;
	}

	float CurrentTime = TimeHistory.Last();
	float StartTime = CurrentTime - TimeWindow;

	// Find start index within time window
	int32 StartIndex = 0;
	for (int32 i = TimeHistory.Num() - 1; i >= 0; i--)
	{
		if (TimeHistory[i] <= StartTime)
		{
			StartIndex = i;
			break;
		}
	}

	// Calculate average velocity over time window
	FVector TotalDisplacement = PositionHistory.Last() - PositionHistory[StartIndex];
	float TotalTime = TimeHistory.Last() - TimeHistory[StartIndex];

	if (TotalTime > 0.0f)
	{
		return TotalDisplacement / TotalTime;
	}

	return FVector::ZeroVector;
}

void UVRPlantingGestureDetector::DrawDebugInfo()
{
	if (!GetWorld() || !TrackedSeedItem)
	{
		return;
	}

	// Draw motion history
	for (int32 i = 1; i < PositionHistory.Num(); i++)
	{
		DrawDebugLine(
			GetWorld(),
			PositionHistory[i - 1],
			PositionHistory[i],
			FColor::Yellow,
			false,
			0.0f,
			0,
			2.0f
		);
	}

	// Draw current velocity
	FVector CurrentPosition = TrackedSeedItem->GetActorLocation();
	FVector Velocity = CalculateVelocity();
	DrawDebugDirectionalArrow(
		GetWorld(),
		CurrentPosition,
		CurrentPosition + Velocity * 0.1f,
		10.0f,
		FColor::Green,
		false,
		0.0f,
		0,
		3.0f
	);

	// Draw ground detection
	FVector GroundCheckStart = CurrentPosition;
	FVector GroundCheckEnd = CurrentPosition - FVector(0, 0, MaxPlantingHeight * 2.0f);
	DrawDebugLine(
		GetWorld(),
		GroundCheckStart,
		GroundCheckEnd,
		FColor::Blue,
		false,
		0.0f,
		0,
		1.0f
	);

	// Draw gesture confidence
	FString DebugText = FString::Printf(TEXT("Gesture: %d\nConfidence: %.2f"),
		static_cast<int32>(CurrentGesture), GestureConfidence);
	DrawDebugString(
		GetWorld(),
		CurrentPosition + FVector(0, 0, 20),
		DebugText,
		nullptr,
		FColor::White,
		0.0f
	);
}
