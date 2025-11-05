// Copyright Epic Games, Inc. All Rights Reserved.

#include "NavigationMarkerComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "Math/Transform.h"

UNavigationMarkerComponent::UNavigationMarkerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	// Default configuration
	MarkerName = TEXT("Navigation Marker");
	MarkerType = EMarkerType::Waypoint;
	VisibilityMode = EMarkerVisibility::Always;
	bIsEnabled = true;
	MarkerColor = FLinearColor::Green;
	MarkerIcon = nullptr;

	// Distance-based scaling defaults
	MinVisibilityDistance = 100.0f * 100.0f; // 100m
	MaxVisibilityDistance = 100000.0f * 100.0f; // 100km
	BaseMarkerSize = 100.0f; // 1m
	ReferenceDistance = 10000.0f * 100.0f; // 10km
	MinScaleFactor = 0.5f;
	MaxScaleFactor = 5.0f;

	// Orbital visibility defaults
	OrbitalAltitudeThreshold = 100000.0f * 100.0f; // 100km
	OrbitalScaleMultiplier = 3.0f;

	// Atmospheric visibility defaults
	AtmosphericAltitudeRange = FVector2D(10000.0f * 100.0f, 100000.0f * 100.0f); // 10-100km
	AtmosphericScaleMultiplier = 1.5f;
	AtmosphericFadeDistance = 5000.0f * 100.0f; // 5km fade range

	// Debug
	bShowDebugVisualization = false;

	// Internal state
	CachedViewerLocation = FVector::ZeroVector;
	CachedViewerAltitude = 0.0f;
	CurrentDistance = 0.0f;
	CurrentScale = 1.0f;
	CurrentOpacity = 1.0f;
	bIsCurrentlyVisible = false;
	bWasVisibleLastFrame = false;
}

void UNavigationMarkerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UNavigationMarkerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsEnabled)
	{
		return;
	}

	// Get viewer information
	FVector ViewerLocation;
	float ViewerAltitude;
	if (GetViewerInfo(ViewerLocation, ViewerAltitude))
	{
		UpdateMarker(ViewerLocation, ViewerAltitude);
	}

	// Debug visualization
	if (bShowDebugVisualization && bIsCurrentlyVisible)
	{
		FVector MarkerLocation = GetComponentLocation();
		DrawDebugSphere(GetWorld(), MarkerLocation, BaseMarkerSize * CurrentScale, 16, MarkerColor.ToFColor(true), false, -1.0f, 0, 2.0f);
		
		FString DebugText = FString::Printf(TEXT("%s\nDist: %.0fm\nAlt: %.0fm\nScale: %.2f\nOpacity: %.2f"),
			*MarkerName,
			CurrentDistance / 100.0f,
			CachedViewerAltitude / 100.0f,
			CurrentScale,
			CurrentOpacity);
		
		DrawDebugString(GetWorld(), MarkerLocation + FVector(0, 0, 100), DebugText, nullptr, MarkerColor.ToFColor(true), 0.0f, true);
	}
}

void UNavigationMarkerComponent::UpdateMarker(const FVector& ViewerLocation, float ViewerAltitude)
{
	CachedViewerLocation = ViewerLocation;
	CachedViewerAltitude = ViewerAltitude;

	// Calculate distance
	FVector MarkerLocation = GetComponentLocation();
	CurrentDistance = FVector::Dist(ViewerLocation, MarkerLocation);

	// Check visibility
	bWasVisibleLastFrame = bIsCurrentlyVisible;
	bIsCurrentlyVisible = ShouldBeVisible(ViewerAltitude);

	if (bIsCurrentlyVisible)
	{
		// Calculate scale and opacity
		CurrentScale = CalculateMarkerScale(CurrentDistance, ViewerAltitude);
		CurrentOpacity = CalculateMarkerOpacity(CurrentDistance, ViewerAltitude);

		// Update visuals
		UpdateMarkerVisuals();
	}

	// Trigger visibility events
	if (bIsCurrentlyVisible && !bWasVisibleLastFrame)
	{
		OnMarkerBecameVisible.Broadcast();
	}
	else if (!bIsCurrentlyVisible && bWasVisibleLastFrame)
	{
		OnMarkerBecameHidden.Broadcast();
	}
}

bool UNavigationMarkerComponent::ShouldBeVisible(float ViewerAltitude) const
{
	if (!bIsEnabled)
	{
		return false;
	}

	// Check distance range
	if (CurrentDistance < MinVisibilityDistance || CurrentDistance > MaxVisibilityDistance)
	{
		return false;
	}

	// Check visibility mode based on altitude
	switch (VisibilityMode)
	{
	case EMarkerVisibility::Orbital:
		// Only visible above orbital threshold
		return ViewerAltitude >= OrbitalAltitudeThreshold;

	case EMarkerVisibility::Atmospheric:
		// Only visible in atmospheric range
		return ViewerAltitude >= AtmosphericAltitudeRange.X && ViewerAltitude <= AtmosphericAltitudeRange.Y;

	case EMarkerVisibility::Surface:
		// Only visible below atmospheric range
		return ViewerAltitude < AtmosphericAltitudeRange.X;

	case EMarkerVisibility::Always:
	default:
		// Always visible within distance range
		return true;
	}
}

float UNavigationMarkerComponent::CalculateMarkerScale(float Distance, float ViewerAltitude) const
{
	// Base scale from distance
	float DistanceScale = 1.0f;
	if (Distance > ReferenceDistance)
	{
		// Scale up for distant markers
		DistanceScale = Distance / ReferenceDistance;
	}
	else
	{
		// Scale down for close markers
		DistanceScale = FMath::Max(0.5f, Distance / ReferenceDistance);
	}

	// Apply altitude-based multiplier
	float AltitudeMultiplier = 1.0f;
	if (ViewerAltitude >= OrbitalAltitudeThreshold)
	{
		// Orbital view - larger markers
		AltitudeMultiplier = OrbitalScaleMultiplier;
	}
	else if (ViewerAltitude >= AtmosphericAltitudeRange.X && ViewerAltitude <= AtmosphericAltitudeRange.Y)
	{
		// Atmospheric view - medium markers
		AltitudeMultiplier = AtmosphericScaleMultiplier;
	}
	// Surface view uses 1.0 multiplier

	// Combine scales
	float FinalScale = DistanceScale * AltitudeMultiplier;

	// Clamp to min/max
	return FMath::Clamp(FinalScale, MinScaleFactor, MaxScaleFactor);
}

float UNavigationMarkerComponent::CalculateMarkerOpacity(float Distance, float ViewerAltitude) const
{
	float Opacity = 1.0f;

	// Fade out at maximum distance
	float FadeStartDistance = MaxVisibilityDistance - AtmosphericFadeDistance;
	if (Distance > FadeStartDistance)
	{
		float FadeProgress = (Distance - FadeStartDistance) / AtmosphericFadeDistance;
		Opacity *= 1.0f - FadeProgress;
	}

	// Fade in at minimum distance
	float FadeInDistance = MinVisibilityDistance + (AtmosphericFadeDistance * 0.2f);
	if (Distance < FadeInDistance)
	{
		float FadeProgress = Distance / FadeInDistance;
		Opacity *= FadeProgress;
	}

	// Additional fade for atmospheric markers at altitude boundaries
	if (VisibilityMode == EMarkerVisibility::Atmospheric)
	{
		// Fade at lower boundary
		float LowerBoundaryFade = 2000.0f * 100.0f; // 2km fade zone
		if (ViewerAltitude < AtmosphericAltitudeRange.X + LowerBoundaryFade)
		{
			float FadeProgress = (ViewerAltitude - AtmosphericAltitudeRange.X) / LowerBoundaryFade;
			Opacity *= FMath::Clamp(FadeProgress, 0.0f, 1.0f);
		}

		// Fade at upper boundary
		float UpperBoundaryFade = 10000.0f * 100.0f; // 10km fade zone
		if (ViewerAltitude > AtmosphericAltitudeRange.Y - UpperBoundaryFade)
		{
			float FadeProgress = (AtmosphericAltitudeRange.Y - ViewerAltitude) / UpperBoundaryFade;
			Opacity *= FMath::Clamp(FadeProgress, 0.0f, 1.0f);
		}
	}

	return FMath::Clamp(Opacity, 0.0f, 1.0f);
}

float UNavigationMarkerComponent::GetDistanceToViewer() const
{
	return CurrentDistance;
}

void UNavigationMarkerComponent::SetMarkerEnabled(bool bEnabled)
{
	bIsEnabled = bEnabled;
	
	if (!bEnabled)
	{
		bIsCurrentlyVisible = false;
		UpdateMarkerVisuals();
	}
}

void UNavigationMarkerComponent::SetMarkerColor(const FLinearColor& NewColor)
{
	MarkerColor = NewColor;
	UpdateMarkerVisuals();
}

void UNavigationMarkerComponent::UpdateMarkerVisuals()
{
	// This method can be overridden in Blueprint or extended in C++
	// to update actual visual components (widgets, meshes, etc.)
	
	// Base implementation does nothing - visuals are handled by child components
	// or Blueprint implementations
}

bool UNavigationMarkerComponent::GetViewerInfo(FVector& OutLocation, float& OutAltitude) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Get player controller
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		return false;
	}

	// Get camera location
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	OutLocation = CameraLocation;
	OutAltitude = CalculateAltitudeAboveSurface(CameraLocation);

	return true;
}

float UNavigationMarkerComponent::CalculateAltitudeAboveSurface(const FVector& Location) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0f;
	}

	// Perform line trace downward to find ground
	FVector TraceStart = Location;
	FVector TraceEnd = Location - FVector(0, 0, 1000000.0f); // Trace down 10km

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		// Calculate altitude above hit point
		return Location.Z - HitResult.Location.Z;
	}

	// If no ground found, use absolute Z as altitude
	// This works for space where there's no ground below
	return Location.Z;
}
