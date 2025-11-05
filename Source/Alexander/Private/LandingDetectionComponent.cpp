// Copyright Epic Games, Inc. All Rights Reserved.

#include "LandingDetectionComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "LandingPad.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Quat.h"                // For FQuat operations
#include "Math/Vector.h"
#include "PhysicsEngine/BodyInstance.h"

ULandingDetectionComponent::ULandingDetectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	// Set default landable surface types
	LandableSurfaceTypes.Add(ESurfaceType::Rock);
	LandableSurfaceTypes.Add(ESurfaceType::Dirt);
	LandableSurfaceTypes.Add(ESurfaceType::Sand);
	LandableSurfaceTypes.Add(ESurfaceType::Grass);
	LandableSurfaceTypes.Add(ESurfaceType::Snow);
	LandableSurfaceTypes.Add(ESurfaceType::Metal);
}

void ULandingDetectionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize landing confirmation
	LandingConfirmation.bIsLanded = false;
	LandingConfirmation.LandingLocation = FVector::ZeroVector;
	LandingConfirmation.LandingTime = 0.0f;
	LandingConfirmation.LandingVelocity = 0.0f;
	LandingConfirmation.bOnLandingPad = false;
	LandingConfirmation.LandingPad = nullptr;

	TimeSinceGroundContact = 0.0f;
	bWasOnGroundLastFrame = false;

	UE_LOG(LogTemp, Log, TEXT("LandingDetectionComponent initialized for %s"), *GetOwner()->GetName());
}

void ULandingDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update landing state
	UpdateLandingState(DeltaTime);

	// Debug visualization
	if (bShowDebugVisualization && GetOwner())
	{
		FVector Location = GetOwner()->GetActorLocation();
		FLandingZoneValidation Validation = ValidateLandingZone(Location);
		DrawDebugInfo(Location, Validation);
	}
}

// ============================================================================
// SLOPE VALIDATION
// ============================================================================

bool ULandingDetectionComponent::ValidateSlopeAngle(const FVector& Location, float& OutSlopeAngle) const
{
	OutSlopeAngle = CalculateSlopeAngle(Location);
	return OutSlopeAngle <= MaxSlopeAngle;
}

float ULandingDetectionComponent::CalculateSlopeAngle(const FVector& Location) const
{
	// Sample terrain at multiple points in a circle
	TArray<FVector> SamplePoints;
	TArray<float> Heights;

	for (int32 i = 0; i < SlopeSampleCount; i++)
	{
		float Angle = (2.0f * PI * i) / SlopeSampleCount;
		FVector Offset(FMath::Cos(Angle) * SlopeSampleRadius, FMath::Sin(Angle) * SlopeSampleRadius, 0.0f);
		FVector SampleLocation = Location + Offset;

		// Trace down to find terrain height
		FHitResult HitResult;
		if (TraceToGround(SampleLocation, HitResult))
		{
			SamplePoints.Add(HitResult.Location);
			Heights.Add(HitResult.Location.Z);
		}
	}

	// Need at least 3 points to calculate slope
	if (Heights.Num() < 3)
	{
		return 0.0f;
	}

	// Find min and max heights
	float MinHeight = Heights[0];
	float MaxHeight = Heights[0];
	for (float Height : Heights)
	{
		MinHeight = FMath::Min(MinHeight, Height);
		MaxHeight = FMath::Max(MaxHeight, Height);
	}

	// Calculate slope angle from height difference
	float HeightDifference = MaxHeight - MinHeight;
	float SlopeRadians = FMath::Atan2(HeightDifference, SlopeSampleRadius);
	float SlopeDegrees = FMath::RadiansToDegrees(SlopeRadians);

	return SlopeDegrees;
}

bool ULandingDetectionComponent::GetSurfaceNormal(const FVector& Location, FVector& OutNormal) const
{
	FHitResult HitResult;
	if (TraceToGround(Location, HitResult))
	{
		OutNormal = HitResult.Normal;
		return true;
	}

	OutNormal = FVector::UpVector;
	return false;
}

// ============================================================================
// SURFACE TYPE CHECKING
// ============================================================================

ESurfaceType ULandingDetectionComponent::DetectSurfaceType(const FVector& Location) const
{
	FHitResult HitResult;
	if (!TraceToGround(Location, HitResult))
	{
		return ESurfaceType::Unknown;
	}

	// Check if hit a landing pad
	if (HitResult.GetActor() && HitResult.GetActor()->IsA(ALandingPad::StaticClass()))
	{
		return ESurfaceType::Metal;
	}

	// Get physical material
	UPhysicalMaterial* PhysMaterial = HitResult.PhysMaterial.Get();
	if (PhysMaterial)
	{
		return GetSurfaceTypeFromPhysMaterial(PhysMaterial);
	}

	// Default to rock if no physical material
	return ESurfaceType::Rock;
}

bool ULandingDetectionComponent::IsSurfaceTypeLandable(ESurfaceType SurfaceType) const
{
	// Water, lava, and ice are not safe for landing
	if (SurfaceType == ESurfaceType::Water || 
		SurfaceType == ESurfaceType::Lava || 
		SurfaceType == ESurfaceType::Ice)
	{
		return false;
	}

	// Check if in landable surface types set
	return LandableSurfaceTypes.Contains(SurfaceType);
}

ESurfaceType ULandingDetectionComponent::GetSurfaceTypeFromPhysMaterial(UPhysicalMaterial* PhysMaterial) const
{
	if (!PhysMaterial)
	{
		return ESurfaceType::Unknown;
	}

	// Map physical material surface type to our enum
	// This is a simplified mapping - in a full implementation, you'd use
	// physical material properties or custom data
	FString MaterialName = PhysMaterial->GetName().ToLower();

	if (MaterialName.Contains(TEXT("rock")) || MaterialName.Contains(TEXT("stone")))
	{
		return ESurfaceType::Rock;
	}
	else if (MaterialName.Contains(TEXT("dirt")) || MaterialName.Contains(TEXT("soil")))
	{
		return ESurfaceType::Dirt;
	}
	else if (MaterialName.Contains(TEXT("sand")) || MaterialName.Contains(TEXT("desert")))
	{
		return ESurfaceType::Sand;
	}
	else if (MaterialName.Contains(TEXT("grass")) || MaterialName.Contains(TEXT("vegetation")))
	{
		return ESurfaceType::Grass;
	}
	else if (MaterialName.Contains(TEXT("snow")))
	{
		return ESurfaceType::Snow;
	}
	else if (MaterialName.Contains(TEXT("ice")))
	{
		return ESurfaceType::Ice;
	}
	else if (MaterialName.Contains(TEXT("water")) || MaterialName.Contains(TEXT("ocean")))
	{
		return ESurfaceType::Water;
	}
	else if (MaterialName.Contains(TEXT("lava")) || MaterialName.Contains(TEXT("magma")))
	{
		return ESurfaceType::Lava;
	}
	else if (MaterialName.Contains(TEXT("metal")) || MaterialName.Contains(TEXT("pad")))
	{
		return ESurfaceType::Metal;
	}

	// Default to rock
	return ESurfaceType::Rock;
}

// ============================================================================
// SAFE LANDING ZONE DETECTION
// ============================================================================

FLandingZoneValidation ULandingDetectionComponent::ValidateLandingZone(const FVector& Location) const
{
	FLandingZoneValidation Validation;
	Validation.bIsSafe = false;
	Validation.SlopeAngle = 0.0f;
	Validation.SurfaceType = ESurfaceType::Unknown;
	Validation.ClearanceDistance = 0.0f;
	Validation.bIsFlatEnough = false;
	Validation.bIsLandableSurface = false;
	Validation.bHasClearance = false;
	Validation.FailureReason = TEXT("");

	// 1. Validate slope angle
	float SlopeAngle = 0.0f;
	Validation.bIsFlatEnough = ValidateSlopeAngle(Location, SlopeAngle);
	Validation.SlopeAngle = SlopeAngle;

	if (!Validation.bIsFlatEnough)
	{
		Validation.FailureReason = FString::Printf(TEXT("Slope too steep: %.1f° (max %.1f°)"), 
			SlopeAngle, MaxSlopeAngle);
		return Validation;
	}

	// 2. Check surface type
	ESurfaceType SurfaceType = DetectSurfaceType(Location);
	Validation.SurfaceType = SurfaceType;
	Validation.bIsLandableSurface = IsSurfaceTypeLandable(SurfaceType);

	if (!Validation.bIsLandableSurface)
	{
		Validation.FailureReason = FString::Printf(TEXT("Unsafe surface type: %s"), 
			*UEnum::GetValueAsString(SurfaceType));
		return Validation;
	}

	// 3. Check clearance
	float ClearanceDistance = 0.0f;
	Validation.bHasClearance = CheckClearance(Location, MinClearanceRadius, ClearanceDistance);
	Validation.ClearanceDistance = ClearanceDistance;

	if (!Validation.bHasClearance)
	{
		Validation.FailureReason = FString::Printf(TEXT("Insufficient clearance: %.1fm (min %.1fm)"), 
			ClearanceDistance / 100.0f, MinClearanceRadius / 100.0f);
		return Validation;
	}

	// All checks passed
	Validation.bIsSafe = true;
	Validation.FailureReason = TEXT("Landing zone is safe");

	return Validation;
}

bool ULandingDetectionComponent::CheckClearance(const FVector& Location, float Radius, float& OutClearanceDistance) const
{
	// Check for obstacles in a cylinder around the landing zone
	const int32 NumRadialSamples = 12;
	const int32 NumHeightSamples = 3;
	
	float MinObstacleDistance = Radius * 2.0f; // Start with large value

	for (int32 h = 0; h < NumHeightSamples; h++)
	{
		float Height = (ClearanceCheckHeight * h) / FMath::Max(1, NumHeightSamples - 1);
		
		for (int32 i = 0; i < NumRadialSamples; i++)
		{
			float Angle = (2.0f * PI * i) / NumRadialSamples;
			FVector Offset(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, Height);
			FVector CheckLocation = Location + Offset;

			// Sphere trace to check for obstacles
			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(GetOwner());

			bool bHit = GetWorld()->SweepSingleByChannel(
				HitResult,
				Location + FVector(0, 0, Height),
				CheckLocation,
				FQuat::Identity,
				ECC_WorldStatic,
				FCollisionShape::MakeSphere(50.0f), // 0.5m sphere
				QueryParams
			);

			if (bHit)
			{
				float Distance = FVector::Dist(Location, HitResult.Location);
				MinObstacleDistance = FMath::Min(MinObstacleDistance, Distance);
			}
		}
	}

	OutClearanceDistance = MinObstacleDistance;
	return MinObstacleDistance >= Radius;
}

bool ULandingDetectionComponent::FindNearestSafeLandingZone(const FVector& StartLocation, float SearchRadius, FVector& OutLandingLocation) const
{
	// Search in a spiral pattern for a safe landing zone
	const int32 NumRings = 5;
	const int32 PointsPerRing = 8;
	
	for (int32 Ring = 0; Ring < NumRings; Ring++)
	{
		float RingRadius = (SearchRadius * (Ring + 1)) / NumRings;
		
		for (int32 i = 0; i < PointsPerRing; i++)
		{
			float Angle = (2.0f * PI * i) / PointsPerRing;
			FVector Offset(FMath::Cos(Angle) * RingRadius, FMath::Sin(Angle) * RingRadius, 0.0f);
			FVector TestLocation = StartLocation + Offset;

			// Validate this location
			FLandingZoneValidation Validation = ValidateLandingZone(TestLocation);
			if (Validation.bIsSafe)
			{
				OutLandingLocation = TestLocation;
				return true;
			}
		}
	}

	// No safe zone found
	OutLandingLocation = StartLocation;
	return false;
}

// ============================================================================
// LANDING CONFIRMATION
// ============================================================================

bool ULandingDetectionComponent::ConfirmLanding(const FVector& Velocity)
{
	if (!GetOwner())
	{
		return false;
	}

	FVector Location = GetOwner()->GetActorLocation();
	
	// Validate landing zone
	FLandingZoneValidation Validation = ValidateLandingZone(Location);
	if (!Validation.bIsSafe)
	{
		UE_LOG(LogTemp, Warning, TEXT("Landing failed: %s"), *Validation.FailureReason);
		OnUnsafeLandingZone.Broadcast(Validation.FailureReason);
		return false;
	}

	// Check landing velocity
	float LandingSpeed = Velocity.Size();
	if (LandingSpeed > MaxLandingVelocity)
	{
		FString Reason = FString::Printf(TEXT("Landing velocity too high: %.1f m/s (max %.1f m/s)"),
			LandingSpeed / 100.0f, MaxLandingVelocity / 100.0f);
		UE_LOG(LogTemp, Warning, TEXT("Landing failed: %s"), *Reason);
		OnUnsafeLandingZone.Broadcast(Reason);
		return false;
	}

	// Check if on a landing pad
	FHitResult HitResult;
	bool bOnPad = false;
	AActor* PadActor = nullptr;
	
	if (TraceToGround(Location, HitResult))
	{
		if (HitResult.GetActor() && HitResult.GetActor()->IsA(ALandingPad::StaticClass()))
		{
			bOnPad = true;
			PadActor = HitResult.GetActor();
		}
	}

	// Confirm landing
	LandingConfirmation.bIsLanded = true;
	LandingConfirmation.LandingLocation = Location;
	LandingConfirmation.LandingTime = GetWorld()->GetTimeSeconds();
	LandingConfirmation.LandingVelocity = LandingSpeed;
	LandingConfirmation.bOnLandingPad = bOnPad;
	LandingConfirmation.LandingPad = PadActor;

	UE_LOG(LogTemp, Log, TEXT("Landing confirmed at %s (velocity: %.1f m/s, on pad: %s)"),
		*Location.ToString(), LandingSpeed / 100.0f, bOnPad ? TEXT("Yes") : TEXT("No"));

	// Broadcast event
	OnLandingConfirmed.Broadcast(LandingConfirmation);

	return true;
}

void ULandingDetectionComponent::CancelLanding()
{
	if (!LandingConfirmation.bIsLanded)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Takeoff detected from %s"), *LandingConfirmation.LandingLocation.ToString());

	// Reset landing confirmation
	LandingConfirmation.bIsLanded = false;
	LandingConfirmation.LandingLocation = FVector::ZeroVector;
	LandingConfirmation.LandingTime = 0.0f;
	LandingConfirmation.LandingVelocity = 0.0f;
	LandingConfirmation.bOnLandingPad = false;
	LandingConfirmation.LandingPad = nullptr;

	TimeSinceGroundContact = 0.0f;

	// Broadcast event
	OnTakeoff.Broadcast();
}

bool ULandingDetectionComponent::IsInLandingApproach(float& OutDistanceToGround, float& OutApproachAngle) const
{
	if (!GetOwner())
	{
		OutDistanceToGround = 0.0f;
		OutApproachAngle = 0.0f;
		return false;
	}

	// Check distance to ground
	float GroundDistance = 0.0f;
	if (!IsOnGround(GroundDistance))
	{
		OutDistanceToGround = GroundDistance;
		OutApproachAngle = 0.0f;
		return false;
	}

	OutDistanceToGround = GroundDistance;

	// Calculate approach angle
	FVector Velocity = GetOwner()->GetVelocity();
	if (Velocity.SizeSquared() < 1.0f)
	{
		OutApproachAngle = 0.0f;
		return GroundDistance < 5000.0f; // Within 50m
	}

	FVector VelocityDirection = Velocity.GetSafeNormal();
	FVector DownVector = -FVector::UpVector;
	
	float DotProduct = FVector::DotProduct(VelocityDirection, DownVector);
	OutApproachAngle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

	// In landing approach if within 50m and descending
	return (GroundDistance < 5000.0f) && (Velocity.Z < 0.0f);
}

// ============================================================================
// INTERNAL METHODS
// ============================================================================

bool ULandingDetectionComponent::TraceToGround(const FVector& StartLocation, FHitResult& OutHitResult) const
{
	FVector TraceStart = StartLocation + FVector(0.0f, 0.0f, 1000.0f);
	FVector TraceEnd = StartLocation - FVector(0.0f, 0.0f, 10000.0f);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	return GetWorld()->LineTraceSingleByChannel(
		OutHitResult,
		TraceStart,
		TraceEnd,
		ECC_WorldStatic,
		QueryParams
	);
}

bool ULandingDetectionComponent::IsOnGround(float& OutGroundDistance) const
{
	if (!GetOwner())
	{
		OutGroundDistance = 0.0f;
		return false;
	}

	FVector Location = GetOwner()->GetActorLocation();
	FHitResult HitResult;

	if (TraceToGround(Location, HitResult))
	{
		OutGroundDistance = FVector::Dist(Location, HitResult.Location);
		return OutGroundDistance < 200.0f; // Within 2m of ground
	}

	OutGroundDistance = 10000.0f; // Far from ground
	return false;
}

void ULandingDetectionComponent::UpdateLandingState(float DeltaTime)
{
	if (!GetOwner())
	{
		return;
	}

	float GroundDistance = 0.0f;
	bool bIsOnGroundNow = IsOnGround(GroundDistance);

	// Check for landing
	if (bIsOnGroundNow && !bWasOnGroundLastFrame)
	{
		// Just touched down
		TimeSinceGroundContact = 0.0f;
	}
	else if (bIsOnGroundNow && bWasOnGroundLastFrame)
	{
		// Still on ground
		TimeSinceGroundContact += DeltaTime;

		// Auto-confirm landing after minimum ground contact time
		if (!LandingConfirmation.bIsLanded && TimeSinceGroundContact >= MinGroundContactTime)
		{
			FVector Velocity = GetOwner()->GetVelocity();
			ConfirmLanding(Velocity);
		}
	}
	else if (!bIsOnGroundNow && bWasOnGroundLastFrame)
	{
		// Just left ground
		if (LandingConfirmation.bIsLanded)
		{
			CancelLanding();
		}
		TimeSinceGroundContact = 0.0f;
	}

	bWasOnGroundLastFrame = bIsOnGroundNow;
}

void ULandingDetectionComponent::DrawDebugInfo(const FVector& Location, const FLandingZoneValidation& Validation) const
{
	if (!GetWorld())
	{
		return;
	}

	// Draw landing zone circle
	FColor ZoneColor = Validation.bIsSafe ? FColor::Green : FColor::Red;
	DrawDebugCircle(GetWorld(), Location, MinClearanceRadius, 32, ZoneColor, false, -1.0f, 0, 5.0f, FVector(1, 0, 0), FVector(0, 1, 0));

	// Draw slope sample points
	for (int32 i = 0; i < SlopeSampleCount; i++)
	{
		float Angle = (2.0f * PI * i) / SlopeSampleCount;
		FVector Offset(FMath::Cos(Angle) * SlopeSampleRadius, FMath::Sin(Angle) * SlopeSampleRadius, 0.0f);
		FVector SampleLocation = Location + Offset;

		FHitResult HitResult;
		if (TraceToGround(SampleLocation, HitResult))
		{
			DrawDebugSphere(GetWorld(), HitResult.Location, 25.0f, 8, FColor::Yellow, false, -1.0f);
		}
	}

	// Draw status text
	FString StatusText = FString::Printf(TEXT("Landing Zone: %s\nSlope: %.1f°\nSurface: %s\nClearance: %.1fm"),
		Validation.bIsSafe ? TEXT("SAFE") : TEXT("UNSAFE"),
		Validation.SlopeAngle,
		*UEnum::GetValueAsString(Validation.SurfaceType),
		Validation.ClearanceDistance / 100.0f);

	DrawDebugString(GetWorld(), Location + FVector(0, 0, 200.0f), StatusText, nullptr, ZoneColor, 0.0f, true);

	// Draw landing confirmation if landed
	if (LandingConfirmation.bIsLanded)
	{
		DrawDebugSphere(GetWorld(), LandingConfirmation.LandingLocation, 100.0f, 16, FColor::Cyan, false, -1.0f);
		DrawDebugString(GetWorld(), LandingConfirmation.LandingLocation + FVector(0, 0, 300.0f), 
			TEXT("LANDED"), nullptr, FColor::Cyan, 0.0f, true);
	}
}
