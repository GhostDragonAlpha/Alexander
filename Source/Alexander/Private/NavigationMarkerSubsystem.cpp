// Copyright Epic Games, Inc. All Rights Reserved.

#include "NavigationMarkerSubsystem.h"
#include "Engine/World.h"

void UNavigationMarkerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RegisteredMarkers.Empty();
	bAutoUpdateEnabled = true;

	// Initialize default filter settings
	FilterSettings = FMarkerFilterSettings();
}

void UNavigationMarkerSubsystem::Deinitialize()
{
	RegisteredMarkers.Empty();
	Super::Deinitialize();
}

void UNavigationMarkerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
}

void UNavigationMarkerSubsystem::RegisterMarker(UNavigationMarkerComponent* Marker)
{
	if (!Marker)
	{
		return;
	}

	if (!RegisteredMarkers.Contains(Marker))
	{
		RegisteredMarkers.Add(Marker);
		
		// Apply current filter settings to new marker
		bool bShouldBeEnabled = FilterSettings.ShouldShowMarkerType(Marker->MarkerType);
		if (!bShouldBeEnabled)
		{
			Marker->SetMarkerEnabled(false);
		}
	}
}

void UNavigationMarkerSubsystem::UnregisterMarker(UNavigationMarkerComponent* Marker)
{
	if (Marker)
	{
		RegisteredMarkers.Remove(Marker);
	}
}

TArray<UNavigationMarkerComponent*> UNavigationMarkerSubsystem::GetAllMarkers() const
{
	// Remove any null entries
	TArray<UNavigationMarkerComponent*> ValidMarkers;
	for (UNavigationMarkerComponent* Marker : RegisteredMarkers)
	{
		if (Marker && IsValid(Marker))
		{
			ValidMarkers.Add(Marker);
		}
	}
	return ValidMarkers;
}

TArray<UNavigationMarkerComponent*> UNavigationMarkerSubsystem::GetMarkersByType(EMarkerType Type) const
{
	TArray<UNavigationMarkerComponent*> FilteredMarkers;
	
	for (UNavigationMarkerComponent* Marker : RegisteredMarkers)
	{
		if (Marker && IsValid(Marker) && Marker->MarkerType == Type)
		{
			FilteredMarkers.Add(Marker);
		}
	}
	
	return FilteredMarkers;
}

TArray<UNavigationMarkerComponent*> UNavigationMarkerSubsystem::GetMarkersInRange(const FVector& Location, float MaxDistance) const
{
	TArray<UNavigationMarkerComponent*> MarkersInRange;
	float MaxDistanceSquared = MaxDistance * MaxDistance;
	
	for (UNavigationMarkerComponent* Marker : RegisteredMarkers)
	{
		if (Marker && IsValid(Marker))
		{
			float DistanceSquared = FVector::DistSquared(Location, Marker->GetComponentLocation());
			if (DistanceSquared <= MaxDistanceSquared)
			{
				MarkersInRange.Add(Marker);
			}
		}
	}
	
	return MarkersInRange;
}

UNavigationMarkerComponent* UNavigationMarkerSubsystem::FindNearestMarker(const FVector& Location, EMarkerType Type) const
{
	UNavigationMarkerComponent* NearestMarker = nullptr;
	float NearestDistanceSquared = FLT_MAX;
	
	for (UNavigationMarkerComponent* Marker : RegisteredMarkers)
	{
		if (!Marker || !IsValid(Marker))
		{
			continue;
		}

		// Apply type filter if not Custom
		if (Type != EMarkerType::Custom && Marker->MarkerType != Type)
		{
			continue;
		}

		float DistanceSquared = FVector::DistSquared(Location, Marker->GetComponentLocation());
		if (DistanceSquared < NearestDistanceSquared)
		{
			NearestDistanceSquared = DistanceSquared;
			NearestMarker = Marker;
		}
	}
	
	return NearestMarker;
}

void UNavigationMarkerSubsystem::SetFilterSettings(const FMarkerFilterSettings& NewSettings)
{
	FilterSettings = NewSettings;
	ApplyFilters();
}

void UNavigationMarkerSubsystem::SetMarkerTypeEnabled(EMarkerType Type, bool bEnabled)
{
	switch (Type)
	{
	case EMarkerType::LandingPad:
		FilterSettings.bShowLandingPads = bEnabled;
		break;
	case EMarkerType::PointOfInterest:
		FilterSettings.bShowPointsOfInterest = bEnabled;
		break;
	case EMarkerType::Waypoint:
		FilterSettings.bShowWaypoints = bEnabled;
		break;
	case EMarkerType::Hazard:
		FilterSettings.bShowHazards = bEnabled;
		break;
	case EMarkerType::Resource:
		FilterSettings.bShowResources = bEnabled;
		break;
	case EMarkerType::Settlement:
		FilterSettings.bShowSettlements = bEnabled;
		break;
	case EMarkerType::Custom:
		FilterSettings.bShowCustom = bEnabled;
		break;
	}

	ApplyFilters();
}

void UNavigationMarkerSubsystem::ApplyFilters()
{
	// Apply type filters
	for (UNavigationMarkerComponent* Marker : RegisteredMarkers)
	{
		if (Marker && IsValid(Marker))
		{
			bool bShouldBeEnabled = FilterSettings.ShouldShowMarkerType(Marker->MarkerType);
			Marker->SetMarkerEnabled(bShouldBeEnabled);
		}
	}

	// Apply marker count limit if needed
	if (FilterSettings.MaxVisibleMarkers > 0)
	{
		ApplyMarkerLimit();
	}
}

void UNavigationMarkerSubsystem::UpdateAllMarkers()
{
	if (!bAutoUpdateEnabled)
	{
		return;
	}

	// Markers update themselves in their Tick
	// This method can be used for additional batch processing if needed
}

void UNavigationMarkerSubsystem::SetAllMarkersEnabled(bool bEnabled)
{
	for (UNavigationMarkerComponent* Marker : RegisteredMarkers)
	{
		if (Marker && IsValid(Marker))
		{
			Marker->SetMarkerEnabled(bEnabled);
		}
	}
}

void UNavigationMarkerSubsystem::SetMarkerTypeColor(EMarkerType Type, const FLinearColor& Color)
{
	for (UNavigationMarkerComponent* Marker : RegisteredMarkers)
	{
		if (Marker && IsValid(Marker) && Marker->MarkerType == Type)
		{
			Marker->SetMarkerColor(Color);
		}
	}
}

int32 UNavigationMarkerSubsystem::GetVisibleMarkerCount() const
{
	int32 Count = 0;
	
	for (UNavigationMarkerComponent* Marker : RegisteredMarkers)
	{
		if (Marker && IsValid(Marker) && Marker->bIsEnabled)
		{
			Count++;
		}
	}
	
	return Count;
}

int32 UNavigationMarkerSubsystem::GetMarkerCountByType(EMarkerType Type) const
{
	int32 Count = 0;
	
	for (UNavigationMarkerComponent* Marker : RegisteredMarkers)
	{
		if (Marker && IsValid(Marker) && Marker->MarkerType == Type)
		{
			Count++;
		}
	}
	
	return Count;
}

void UNavigationMarkerSubsystem::SortMarkersByDistance(TArray<UNavigationMarkerComponent*>& Markers, const FVector& Location) const
{
	Markers.Sort([Location](const UNavigationMarkerComponent& A, const UNavigationMarkerComponent& B)
	{
		float DistA = FVector::DistSquared(Location, A.GetComponentLocation());
		float DistB = FVector::DistSquared(Location, B.GetComponentLocation());
		return DistA < DistB;
	});
}

void UNavigationMarkerSubsystem::ApplyMarkerLimit()
{
	if (FilterSettings.MaxVisibleMarkers <= 0)
	{
		return;
	}

	// Get all enabled markers
	TArray<UNavigationMarkerComponent*> EnabledMarkers;
	for (UNavigationMarkerComponent* Marker : RegisteredMarkers)
	{
		if (Marker && IsValid(Marker) && Marker->bIsEnabled)
		{
			EnabledMarkers.Add(Marker);
		}
	}

	// If under limit, nothing to do
	if (EnabledMarkers.Num() <= FilterSettings.MaxVisibleMarkers)
	{
		return;
	}

	// Sort by distance if prioritization is enabled
	if (FilterSettings.bPrioritizeByDistance)
	{
		// Get player location
		UWorld* World = GetWorld();
		if (World)
		{
			APlayerController* PC = World->GetFirstPlayerController();
			if (PC && PC->GetPawn())
			{
				FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
				SortMarkersByDistance(EnabledMarkers, PlayerLocation);
			}
		}
	}

	// Disable markers beyond the limit
	for (int32 i = FilterSettings.MaxVisibleMarkers; i < EnabledMarkers.Num(); i++)
	{
		if (EnabledMarkers[i])
		{
			EnabledMarkers[i]->SetMarkerEnabled(false);
		}
	}
}
