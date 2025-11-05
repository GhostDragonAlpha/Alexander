// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LandingPad.generated.h"

// Forward declarations
class UStaticMeshComponent;
class UBoxComponent;
class UArrowComponent;
class UNavigationMarkerComponent;

/**
 * Enum for landing pad status
 */
UENUM(BlueprintType)
enum class ELandingPadStatus : uint8
{
	Available UMETA(DisplayName = "Available"),
	Reserved UMETA(DisplayName = "Reserved"),
	Occupied UMETA(DisplayName = "Occupied"),
	Disabled UMETA(DisplayName = "Disabled")
};

/**
 * Landing pad actor that provides designated landing zones on planetary surfaces
 * Implements requirements 9.1 and 9.4 from the planet interior enhancement spec
 */
UCLASS(Blueprintable)
class ALEXANDER_API ALandingPad : public AActor
{
	GENERATED_BODY()
	
public:	
	ALandingPad();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// ============================================================================
	// COMPONENTS
	// ============================================================================

	/** Main landing pad mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PadMesh;

	/** Collision volume for detecting landing ships */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* LandingZone;

	/** Approach vector indicator (shows recommended approach direction) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* ApproachVector;

	/** Navigation marker for orbital and atmospheric visibility */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNavigationMarkerComponent* NavigationMarker;

	// ============================================================================
	// PAD CONFIGURATION
	// ============================================================================

	/** Size of the landing pad in meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing Pad")
	FVector2D PadSize = FVector2D(20.0f, 20.0f);

	/** Height of the pad above terrain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing Pad")
	float PadHeight = 0.5f;

	/** Maximum slope angle allowed for pad placement (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing Pad")
	float MaxSlopeAngle = 5.0f;

	/** Pad identification number */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing Pad")
	int32 PadID = 0;

	/** Pad name for UI display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing Pad")
	FString PadName = TEXT("Landing Pad");

	// ============================================================================
	// RESERVATION SYSTEM
	// ============================================================================

	/** Current status of the landing pad */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reservation")
	ELandingPadStatus PadStatus = ELandingPadStatus::Available;

	/** Actor that has reserved or is occupying this pad */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reservation")
	AActor* ReservedBy = nullptr;

	/** Time when reservation expires (in seconds) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reservation")
	float ReservationExpireTime = 0.0f;

	/** Default reservation duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reservation")
	float DefaultReservationDuration = 300.0f; // 5 minutes

	/**
	 * Reserve this landing pad for an actor
	 * @param Actor The actor reserving the pad
	 * @param Duration How long to reserve the pad (seconds)
	 * @return True if reservation was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Reservation")
	bool ReservePad(AActor* Actor, float Duration = 0.0f);

	/**
	 * Release the reservation on this pad
	 * @param Actor The actor releasing the pad (must match ReservedBy)
	 * @return True if release was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Reservation")
	bool ReleasePad(AActor* Actor);

	/**
	 * Check if this pad is available for landing
	 * @return True if pad is available
	 */
	UFUNCTION(BlueprintPure, Category = "Reservation")
	bool IsAvailable() const;

	/**
	 * Force release the pad (admin/debug function)
	 */
	UFUNCTION(BlueprintCallable, Category = "Reservation")
	void ForceRelease();

	// ============================================================================
	// APPROACH GUIDANCE
	// ============================================================================

	/** Recommended approach altitude above pad (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Approach")
	float ApproachAltitude = 100.0f;

	/** Recommended approach distance from pad (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Approach")
	float ApproachDistance = 200.0f;

	/** Approach angle relative to pad forward vector (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Approach")
	float ApproachAngle = 0.0f;

	/**
	 * Get the recommended approach position for landing
	 * @return World space position for approach
	 */
	UFUNCTION(BlueprintPure, Category = "Approach")
	FVector GetApproachPosition() const;

	/**
	 * Get the approach vector direction
	 * @return Normalized direction vector for approach
	 */
	UFUNCTION(BlueprintPure, Category = "Approach")
	FVector GetApproachDirection() const;

	/**
	 * Check if an actor is within acceptable approach parameters
	 * @param Actor The actor to check
	 * @param OutDistance Distance from ideal approach position
	 * @param OutAlignment Alignment with approach vector (0-1, 1 = perfect)
	 * @return True if within acceptable parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "Approach")
	bool IsInApproachCorridor(AActor* Actor, float& OutDistance, float& OutAlignment) const;

	// ============================================================================
	// TERRAIN INTEGRATION
	// ============================================================================

	/** Whether to flatten terrain under the pad */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	bool bFlattenTerrain = true;

	/** Radius around pad to flatten (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float FlattenRadius = 25.0f;

	/** Blend distance for terrain flattening (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float FlattenBlendDistance = 5.0f;

	/**
	 * Flatten terrain under the landing pad
	 * This modifies the terrain heightmap to create a flat surface
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void FlattenTerrainUnderPad();

	/**
	 * Check if terrain is suitable for pad placement
	 * @param OutAverageSlope The average slope angle at this location
	 * @return True if terrain is suitable
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	bool IsTerrainSuitable(float& OutAverageSlope) const;

	// ============================================================================
	// VISUAL MARKERS
	// ============================================================================

	/** Whether to show approach markers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	bool bShowApproachMarkers = true;

	/** Whether to show status lights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	bool bShowStatusLights = true;

	/** Color for available status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor AvailableColor = FLinearColor::Green;

	/** Color for reserved status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor ReservedColor = FLinearColor::Yellow;

	/** Color for occupied status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor OccupiedColor = FLinearColor::Red;

	/** Color for disabled status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor DisabledColor = FLinearColor::Gray;

	/**
	 * Update visual indicators based on current status
	 */
	UFUNCTION(BlueprintCallable, Category = "Visuals")
	void UpdateVisuals();

	/**
	 * Get the current status color
	 */
	UFUNCTION(BlueprintPure, Category = "Visuals")
	FLinearColor GetStatusColor() const;

	// ============================================================================
	// EVENTS
	// ============================================================================

	/** Called when a ship enters the landing zone */
	UFUNCTION()
	void OnLandingZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Called when a ship leaves the landing zone */
	UFUNCTION()
	void OnLandingZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Blueprint event for when pad is reserved */
	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnPadReserved(AActor* ReservingActor);

	/** Blueprint event for when pad is released */
	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnPadReleased(AActor* ReleasingActor);

	/** Blueprint event for when ship lands */
	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnShipLanded(AActor* Ship);

	/** Blueprint event for when ship takes off */
	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnShipDeparted(AActor* Ship);

private:
	/** Update reservation timer */
	void UpdateReservation(float DeltaTime);

	/** Check if actor is a valid ship */
	bool IsValidShip(AActor* Actor) const;
};
