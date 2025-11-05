// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "NavigationMarkerComponent.generated.h"

/**
 * Marker visibility types for different flight phases
 */
UENUM(BlueprintType)
enum class EMarkerVisibility : uint8
{
	Orbital UMETA(DisplayName = "Orbital (>100km)"),
	Atmospheric UMETA(DisplayName = "Atmospheric (10-100km)"),
	Surface UMETA(DisplayName = "Surface (<10km)"),
	Always UMETA(DisplayName = "Always Visible")
};

/**
 * Marker types for filtering
 */
UENUM(BlueprintType)
enum class EMarkerType : uint8
{
	LandingPad UMETA(DisplayName = "Landing Pad"),
	PointOfInterest UMETA(DisplayName = "Point of Interest"),
	Waypoint UMETA(DisplayName = "Waypoint"),
	Hazard UMETA(DisplayName = "Hazard"),
	Resource UMETA(DisplayName = "Resource"),
	Settlement UMETA(DisplayName = "Settlement"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Navigation marker component that provides visible markers for navigation
 * Supports orbital and atmospheric visibility with distance-based scaling
 */
UCLASS(ClassGroup=(Navigation), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UNavigationMarkerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UNavigationMarkerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ========== Configuration ==========

	/** Display name for this marker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Identity")
	FString MarkerName;

	/** Type of marker for filtering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Identity")
	EMarkerType MarkerType;

	/** Visibility mode for this marker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Visibility")
	EMarkerVisibility VisibilityMode;

	/** Whether this marker is currently enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Visibility")
	bool bIsEnabled;

	/** Color of the marker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Appearance")
	FLinearColor MarkerColor;

	/** Icon texture for the marker (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Appearance")
	UTexture2D* MarkerIcon;

	// ========== Distance-Based Scaling ==========

	/** Minimum distance for marker visibility (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Distance Scaling")
	float MinVisibilityDistance;

	/** Maximum distance for marker visibility (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Distance Scaling")
	float MaxVisibilityDistance;

	/** Base size of the marker at reference distance (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Distance Scaling")
	float BaseMarkerSize;

	/** Distance at which marker is at base size (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Distance Scaling")
	float ReferenceDistance;

	/** Minimum scale factor for marker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Distance Scaling")
	float MinScaleFactor;

	/** Maximum scale factor for marker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Distance Scaling")
	float MaxScaleFactor;

	// ========== Orbital Visibility ==========

	/** Altitude threshold for orbital visibility (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Orbital")
	float OrbitalAltitudeThreshold;

	/** Scale multiplier for orbital view */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Orbital")
	float OrbitalScaleMultiplier;

	// ========== Atmospheric Visibility ==========

	/** Altitude range for atmospheric visibility (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Atmospheric")
	FVector2D AtmosphericAltitudeRange;

	/** Scale multiplier for atmospheric view */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Atmospheric")
	float AtmosphericScaleMultiplier;

	/** Fade distance for atmospheric markers (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Atmospheric")
	float AtmosphericFadeDistance;

	// ========== Debug ==========

	/** Show debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Debug")
	bool bShowDebugVisualization;

	// ========== Public Methods ==========

	/**
	 * Update marker visibility and scale based on viewer location
	 * @param ViewerLocation - Location of the viewer (player camera)
	 * @param ViewerAltitude - Altitude of the viewer above planet surface
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Marker")
	void UpdateMarker(const FVector& ViewerLocation, float ViewerAltitude);

	/**
	 * Check if marker should be visible based on viewer altitude
	 * @param ViewerAltitude - Altitude of the viewer above planet surface
	 * @return True if marker should be visible
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Marker")
	bool ShouldBeVisible(float ViewerAltitude) const;

	/**
	 * Calculate marker scale based on distance and altitude
	 * @param Distance - Distance from viewer to marker
	 * @param ViewerAltitude - Altitude of the viewer above planet surface
	 * @return Scale factor for the marker
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Marker")
	float CalculateMarkerScale(float Distance, float ViewerAltitude) const;

	/**
	 * Calculate marker opacity based on distance and altitude
	 * @param Distance - Distance from viewer to marker
	 * @param ViewerAltitude - Altitude of the viewer above planet surface
	 * @return Opacity value (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Marker")
	float CalculateMarkerOpacity(float Distance, float ViewerAltitude) const;

	/**
	 * Get the current distance to viewer
	 * @return Distance in cm
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Marker")
	float GetDistanceToViewer() const;

	/**
	 * Enable or disable the marker
	 * @param bEnabled - Whether to enable the marker
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Marker")
	void SetMarkerEnabled(bool bEnabled);

	/**
	 * Set the marker color
	 * @param NewColor - New color for the marker
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Marker")
	void SetMarkerColor(const FLinearColor& NewColor);

	// ========== Events ==========

	/** Called when marker becomes visible */
// Called when marker becomes visible
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMarkerBecameVisible);
	UPROPERTY(BlueprintAssignable, Category = "Navigation Marker|Events")
	FOnMarkerBecameVisible OnMarkerBecameVisible;

	/** Called when marker becomes hidden */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMarkerBecameHidden);
	UPROPERTY(BlueprintAssignable, Category = "Navigation Marker|Events")
	FOnMarkerBecameHidden OnMarkerBecameHidden;

protected:
	// ========== Internal State ==========

	/** Cached viewer location */
	FVector CachedViewerLocation;

	/** Cached viewer altitude */
	float CachedViewerAltitude;

	/** Current distance to viewer */
	float CurrentDistance;

	/** Current scale factor */
	float CurrentScale;

	/** Current opacity */
	float CurrentOpacity;

	/** Whether marker is currently visible */
	bool bIsCurrentlyVisible;

	/** Previous visibility state for event triggering */
	bool bWasVisibleLastFrame;

	// ========== Internal Methods ==========

	/** Update marker visual representation */
	void UpdateMarkerVisuals();

	/** Get the viewer location and altitude */
	bool GetViewerInfo(FVector& OutLocation, float& OutAltitude) const;

	/** Calculate altitude above planet surface */
	float CalculateAltitudeAboveSurface(const FVector& Location) const;
};
