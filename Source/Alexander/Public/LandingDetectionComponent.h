// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LandingDetectionComponent.generated.h"

// Forward declarations
class UTransitionController;

/**
 * Surface type enumeration for landing validation
 */
UENUM(BlueprintType)
enum class ESurfaceType : uint8
{
	Unknown UMETA(DisplayName = "Unknown"),
	Rock UMETA(DisplayName = "Rock"),
	Dirt UMETA(DisplayName = "Dirt"),
	Sand UMETA(DisplayName = "Sand"),
	Grass UMETA(DisplayName = "Grass"),
	Snow UMETA(DisplayName = "Snow"),
	Ice UMETA(DisplayName = "Ice"),
	Water UMETA(DisplayName = "Water"),
	Lava UMETA(DisplayName = "Lava"),
	Metal UMETA(DisplayName = "Metal (Landing Pad)")
};

/**
 * Landing zone validation result
 */
USTRUCT(BlueprintType)
struct FLandingZoneValidation
{
	GENERATED_BODY()

	/** Whether the landing zone is safe */
	UPROPERTY(BlueprintReadOnly, Category = "Landing")
	bool bIsSafe = false;

	/** Slope angle at landing location (degrees) */
	UPROPERTY(BlueprintReadOnly, Category = "Landing")
	float SlopeAngle = 0.0f;

	/** Surface type at landing location */
	UPROPERTY(BlueprintReadOnly, Category = "Landing")
	ESurfaceType SurfaceType = ESurfaceType::Unknown;

	/** Distance to nearest obstacle (cm) */
	UPROPERTY(BlueprintReadOnly, Category = "Landing")
	float ClearanceDistance = 0.0f;

	/** Whether surface is flat enough */
	UPROPERTY(BlueprintReadOnly, Category = "Landing")
	bool bIsFlatEnough = false;

	/** Whether surface type is landable */
	UPROPERTY(BlueprintReadOnly, Category = "Landing")
	bool bIsLandableSurface = false;

	/** Whether there's enough clearance */
	UPROPERTY(BlueprintReadOnly, Category = "Landing")
	bool bHasClearance = false;

	/** Reason for failure (if not safe) */
	UPROPERTY(BlueprintReadOnly, Category = "Landing")
	FString FailureReason;
};

/**
 * Landing confirmation data
 */
USTRUCT(BlueprintType)
struct FLandingConfirmation
{
	GENERATED_BODY()

	/** Whether landing is confirmed */
	UPROPERTY(BlueprintReadOnly, Category = "Landing")
	bool bIsLanded = false;

	/** Landing location */
	UPROPERTY(BlueprintReadOnly, Category = "Landing")
	FVector LandingLocation = FVector::ZeroVector;

	/** Landing time */
	UPROPERTY(BlueprintReadOnly, Category = "Landing")
	float LandingTime = 0.0f;

	/** Landing velocity (cm/s) */
	UPROPERTY(BlueprintReadOnly, Category = "Landing")
	float LandingVelocity = 0.0f;

	/** Whether landing was on a designated pad */
	UPROPERTY(BlueprintReadOnly, Category = "Landing")
	bool bOnLandingPad = false;

	/** Landing pad actor (if applicable) */
	UPROPERTY(BlueprintReadOnly, Category = "Landing")
	AActor* LandingPad = nullptr;
};

/**
 * Component that handles landing detection and validation
 * Implements requirement 9.3 from the planet interior enhancement spec
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API ULandingDetectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULandingDetectionComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Maximum slope angle for safe landing (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing Detection")
	float MaxSlopeAngle = 15.0f;

	/** Minimum clearance radius required around landing zone (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing Detection")
	float MinClearanceRadius = 1000.0f; // 10 meters

	/** Height above ground to check for clearance (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing Detection")
	float ClearanceCheckHeight = 500.0f; // 5 meters

	/** Maximum landing velocity (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing Detection")
	float MaxLandingVelocity = 500.0f; // 5 m/s

	/** Number of sample points for slope validation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing Detection")
	int32 SlopeSampleCount = 8;

	/** Radius for slope sampling (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing Detection")
	float SlopeSampleRadius = 500.0f; // 5 meters

	/** Whether to enable debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing Detection")
	bool bShowDebugVisualization = false;

	/** Surface types that are safe for landing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing Detection")
	TSet<ESurfaceType> LandableSurfaceTypes = {
		ESurfaceType::Rock,
		ESurfaceType::Dirt,
		ESurfaceType::Sand,
		ESurfaceType::Grass,
		ESurfaceType::Snow,
		ESurfaceType::Metal
	};

	// ============================================================================
	// SLOPE VALIDATION
	// ============================================================================

	/**
	 * Validate slope angle at a location
	 * @param Location World location to check
	 * @param OutSlopeAngle Calculated slope angle in degrees
	 * @return True if slope is within acceptable range
	 */
	UFUNCTION(BlueprintCallable, Category = "Landing Detection")
	bool ValidateSlopeAngle(const FVector& Location, float& OutSlopeAngle) const;

	/**
	 * Calculate slope angle at a location using multiple sample points
	 * @param Location Center location for sampling
	 * @return Slope angle in degrees
	 */
	UFUNCTION(BlueprintPure, Category = "Landing Detection")
	float CalculateSlopeAngle(const FVector& Location) const;

	/**
	 * Get surface normal at a location
	 * @param Location World location to check
	 * @param OutNormal Surface normal vector
	 * @return True if surface was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Landing Detection")
	bool GetSurfaceNormal(const FVector& Location, FVector& OutNormal) const;

	// ============================================================================
	// SURFACE TYPE CHECKING
	// ============================================================================

	/**
	 * Detect surface type at a location
	 * @param Location World location to check
	 * @return Detected surface type
	 */
	UFUNCTION(BlueprintCallable, Category = "Landing Detection")
	ESurfaceType DetectSurfaceType(const FVector& Location) const;

	/**
	 * Check if surface type is safe for landing
	 * @param SurfaceType Surface type to check
	 * @return True if surface is landable
	 */
	UFUNCTION(BlueprintPure, Category = "Landing Detection")
	bool IsSurfaceTypeLandable(ESurfaceType SurfaceType) const;

	/**
	 * Get surface type from physical material
	 * @param PhysMaterial Physical material to check
	 * @return Corresponding surface type
	 */
	UFUNCTION(BlueprintPure, Category = "Landing Detection")
	ESurfaceType GetSurfaceTypeFromPhysMaterial(UPhysicalMaterial* PhysMaterial) const;

	// ============================================================================
	// SAFE LANDING ZONE DETECTION
	// ============================================================================

	/**
	 * Validate if a location is a safe landing zone
	 * @param Location World location to check
	 * @return Validation result with detailed information
	 */
	UFUNCTION(BlueprintCallable, Category = "Landing Detection")
	FLandingZoneValidation ValidateLandingZone(const FVector& Location) const;

	/**
	 * Check for obstacles around landing zone
	 * @param Location Center location to check
	 * @param Radius Radius to check for obstacles
	 * @param OutClearanceDistance Distance to nearest obstacle
	 * @return True if area is clear
	 */
	UFUNCTION(BlueprintCallable, Category = "Landing Detection")
	bool CheckClearance(const FVector& Location, float Radius, float& OutClearanceDistance) const;

	/**
	 * Find nearest safe landing zone from a location
	 * @param StartLocation Location to search from
	 * @param SearchRadius Maximum search radius
	 * @param OutLandingLocation Found safe landing location
	 * @return True if safe zone was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Landing Detection")
	bool FindNearestSafeLandingZone(const FVector& StartLocation, float SearchRadius, FVector& OutLandingLocation) const;

	// ============================================================================
	// LANDING CONFIRMATION
	// ============================================================================

	/**
	 * Check if actor is currently landed
	 * @return True if landed
	 */
	UFUNCTION(BlueprintPure, Category = "Landing Detection")
	bool IsLanded() const { return LandingConfirmation.bIsLanded; }

	/**
	 * Get current landing confirmation data
	 * @return Landing confirmation struct
	 */
	UFUNCTION(BlueprintPure, Category = "Landing Detection")
	FLandingConfirmation GetLandingConfirmation() const { return LandingConfirmation; }

	/**
	 * Attempt to confirm landing at current location
	 * @param Velocity Current velocity of the actor
	 * @return True if landing was confirmed
	 */
	UFUNCTION(BlueprintCallable, Category = "Landing Detection")
	bool ConfirmLanding(const FVector& Velocity);

	/**
	 * Cancel current landing (takeoff)
	 */
	UFUNCTION(BlueprintCallable, Category = "Landing Detection")
	void CancelLanding();

	/**
	 * Check if actor is in landing approach
	 * @param OutDistanceToGround Distance to ground in cm
	 * @param OutApproachAngle Angle of approach in degrees
	 * @return True if in landing approach
	 */
	UFUNCTION(BlueprintCallable, Category = "Landing Detection")
	bool IsInLandingApproach(float& OutDistanceToGround, float& OutApproachAngle) const;

	// ============================================================================
	// EVENTS
	// ============================================================================

	/** Called when landing is confirmed */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLandingConfirmed, const FLandingConfirmation&, Confirmation);
	UPROPERTY(BlueprintAssignable, Category = "Landing Detection")
	FOnLandingConfirmed OnLandingConfirmed;

	/** Called when takeoff is detected */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTakeoff);
	UPROPERTY(BlueprintAssignable, Category = "Landing Detection")
	FOnTakeoff OnTakeoff;

	/** Called when entering unsafe landing zone */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnsafeLandingZone, const FString&, Reason);
	UPROPERTY(BlueprintAssignable, Category = "Landing Detection")
	FOnUnsafeLandingZone OnUnsafeLandingZone;

protected:
	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	/** Current landing confirmation data */
	UPROPERTY()
	FLandingConfirmation LandingConfirmation;

	/** Time since last ground contact */
	float TimeSinceGroundContact = 0.0f;

	/** Minimum time on ground to confirm landing */
	float MinGroundContactTime = 0.5f;

	/** Whether actor was on ground last frame */
	bool bWasOnGroundLastFrame = false;

	// ============================================================================
	// INTERNAL METHODS
	// ============================================================================

	/**
	 * Perform line trace to ground
	 * @param StartLocation Start location for trace
	 * @param OutHitResult Hit result
	 * @return True if ground was hit
	 */
	bool TraceToGround(const FVector& StartLocation, FHitResult& OutHitResult) const;

	/**
	 * Check if actor is on ground
	 * @param OutGroundDistance Distance to ground
	 * @return True if on ground
	 */
	bool IsOnGround(float& OutGroundDistance) const;

	/**
	 * Update landing state
	 */
	void UpdateLandingState(float DeltaTime);

	/**
	 * Draw debug visualization
	 */
	void DrawDebugInfo(const FVector& Location, const FLandingZoneValidation& Validation) const;
};
