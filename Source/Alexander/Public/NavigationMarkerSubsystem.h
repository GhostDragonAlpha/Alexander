// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NavigationMarkerComponent.h"
#include "NavigationMarkerSubsystem.generated.h"

/**
 * Marker filter settings for controlling which markers are visible
 */
USTRUCT(BlueprintType)
struct FMarkerFilterSettings
{
	GENERATED_BODY()

	/** Show landing pad markers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bShowLandingPads = true;

	/** Show points of interest */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bShowPointsOfInterest = true;

	/** Show waypoints */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bShowWaypoints = true;

	/** Show hazards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bShowHazards = true;

	/** Show resources */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bShowResources = true;

	/** Show settlements */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bShowSettlements = true;

	/** Show custom markers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bShowCustom = true;

	/** Maximum number of visible markers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	int32 MaxVisibleMarkers = 50;

	/** Prioritize closer markers when limiting count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bPrioritizeByDistance = true;

	/**
	 * Check if a marker type should be shown
	 */
	bool ShouldShowMarkerType(EMarkerType Type) const
	{
		switch (Type)
		{
		case EMarkerType::LandingPad:
			return bShowLandingPads;
		case EMarkerType::PointOfInterest:
			return bShowPointsOfInterest;
		case EMarkerType::Waypoint:
			return bShowWaypoints;
		case EMarkerType::Hazard:
			return bShowHazards;
		case EMarkerType::Resource:
			return bShowResources;
		case EMarkerType::Settlement:
			return bShowSettlements;
		case EMarkerType::Custom:
			return bShowCustom;
		default:
			return true;
		}
	}
};

/**
 * World subsystem for managing navigation markers
 * Handles marker registration, filtering, and batch updates
 */
UCLASS()
class ALEXANDER_API UNavigationMarkerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// ========== Subsystem Lifecycle ==========

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// ========== Marker Registration ==========

	/**
	 * Register a navigation marker with the subsystem
	 * @param Marker - The marker component to register
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Markers")
	void RegisterMarker(UNavigationMarkerComponent* Marker);

	/**
	 * Unregister a navigation marker from the subsystem
	 * @param Marker - The marker component to unregister
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Markers")
	void UnregisterMarker(UNavigationMarkerComponent* Marker);

	/**
	 * Get all registered markers
	 * @return Array of all registered markers
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Markers")
	TArray<UNavigationMarkerComponent*> GetAllMarkers() const;

	/**
	 * Get markers of a specific type
	 * @param Type - The marker type to filter by
	 * @return Array of markers of the specified type
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Markers")
	TArray<UNavigationMarkerComponent*> GetMarkersByType(EMarkerType Type) const;

	/**
	 * Get markers within a distance range
	 * @param Location - Center location for search
	 * @param MaxDistance - Maximum distance from location
	 * @return Array of markers within range
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Markers")
	TArray<UNavigationMarkerComponent*> GetMarkersInRange(const FVector& Location, float MaxDistance) const;

	/**
	 * Find the nearest marker to a location
	 * @param Location - Location to search from
	 * @param Type - Optional type filter (use Custom for no filter)
	 * @return Nearest marker, or nullptr if none found
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Markers")
	UNavigationMarkerComponent* FindNearestMarker(const FVector& Location, EMarkerType Type = EMarkerType::Custom) const;

	// ========== Marker Filtering ==========

	/**
	 * Set the marker filter settings
	 * @param NewSettings - New filter settings to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Markers")
	void SetFilterSettings(const FMarkerFilterSettings& NewSettings);

	/**
	 * Get the current marker filter settings
	 * @return Current filter settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Markers")
	FMarkerFilterSettings GetFilterSettings() const { return FilterSettings; }

	/**
	 * Enable or disable a specific marker type
	 * @param Type - The marker type to toggle
	 * @param bEnabled - Whether to enable or disable
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Markers")
	void SetMarkerTypeEnabled(EMarkerType Type, bool bEnabled);

	/**
	 * Apply current filter settings to all markers
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Markers")
	void ApplyFilters();

	// ========== Batch Operations ==========

	/**
	 * Update all registered markers
	 * Called automatically each frame
	 */
	void UpdateAllMarkers();

	/**
	 * Enable or disable all markers
	 * @param bEnabled - Whether to enable or disable all markers
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Markers")
	void SetAllMarkersEnabled(bool bEnabled);

	/**
	 * Set the color for all markers of a specific type
	 * @param Type - The marker type to update
	 * @param Color - New color for the markers
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Markers")
	void SetMarkerTypeColor(EMarkerType Type, const FLinearColor& Color);

	// ========== Statistics ==========

	/**
	 * Get the number of registered markers
	 * @return Total number of registered markers
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Markers")
	int32 GetMarkerCount() const { return RegisteredMarkers.Num(); }

	/**
	 * Get the number of currently visible markers
	 * @return Number of visible markers
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Markers")
	int32 GetVisibleMarkerCount() const;

	/**
	 * Get marker count by type
	 * @param Type - The marker type to count
	 * @return Number of markers of the specified type
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Markers")
	int32 GetMarkerCountByType(EMarkerType Type) const;

protected:
	// ========== Internal State ==========

	/** All registered markers */
	UPROPERTY()
	TArray<UNavigationMarkerComponent*> RegisteredMarkers;

	/** Current filter settings */
	UPROPERTY()
	FMarkerFilterSettings FilterSettings;

	/** Whether automatic updates are enabled */
	UPROPERTY()
	bool bAutoUpdateEnabled;

	// ========== Internal Methods ==========

	/** Sort markers by distance for prioritization */
	void SortMarkersByDistance(TArray<UNavigationMarkerComponent*>& Markers, const FVector& Location) const;

	/** Apply marker count limit based on filter settings */
	void ApplyMarkerLimit();
};
