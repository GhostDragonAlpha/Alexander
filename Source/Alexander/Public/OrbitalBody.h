// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OrbitalMechanics.h"
#include "OrbitalBody.generated.h"

// Forward declarations
class UOrbitalMechanics;
class UStaticMeshComponent;
class UPrimitiveComponent;

/**
 * Base class for ALL objects that exist in space and are subject to orbital mechanics
 * This includes: Stars, Planets, Moons, Asteroids, Spacecraft, Characters, Projectiles, Debris
 * Everything in space is either orbiting something, or is the root of an orbital system
 */
UENUM(BlueprintType)
enum class EOrbitalBodyType : uint8
{
    Star,
    Planet,
    Moon,
    DwarfPlanet,
    Asteroid,
    Comet,
    Spacecraft,
    Debris
};

UENUM(BlueprintType)
enum class EOrbitMode : uint8
{
    // Object is in stable circular/elliptical orbit
    Orbit UMETA(DisplayName = "Orbital"),

    // Object is in ballistic trajectory (jump, throw, etc) - may return to surface or escape
    Ballistic UMETA(DisplayName = "Ballistic"),

    // Object is using thrust/propulsion to move
    Thrust UMETA(DisplayName = "Thrust"),

    // Object is landed/attached to another body
    Landed UMETA(DisplayName = "Landed"),

    // Object is the root of the system (no orbit target)
    Root UMETA(DisplayName = "Root")
};

UCLASS(Blueprintable, ClassGroup = (Space))
class ALEXANDER_API AOrbitalBody : public AActor
{
    GENERATED_BODY()

public:
    AOrbitalBody();

protected:
    virtual void BeginPlay() override;
    virtual void PostInitializeComponents() override;
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void PostLoad() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

    // ============================================================================
    // COMPONENTS
    // ============================================================================

    // Root component for this actor
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Mesh component (optional)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    // Orbital mechanics component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UOrbitalMechanics* OrbitalMechanics;

    // Physics component for collision and movement
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPrimitiveComponent* PhysicsComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital System")
	EOrbitMode OrbitMode;

	// ============================================================================
	// PHYSICAL PROPERTIES
	// ============================================================================

	// Mass in kg (affects gravity)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float Mass;

	// Body radius in km
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float Radius;

	// Standard gravitational parameter (μ = G * Mass)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float StandardGravitationalParameter;

	// Rotation period in hours
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float RotationPeriod;

	// Axial tilt in degrees
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float AxialTilt;

	// Current velocity vector (m/s)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
	FVector Velocity;

	// Current angular velocity (degrees/s)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
	FVector AngularVelocity;

	// ============================================================================
	// ORBITAL PARAMETERS (for stable orbits)
	// ============================================================================

	// Distance from orbit target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	float OrbitRadius;

	// Angular velocity (degrees per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	float OrbitSpeed;

	// Current angle in orbit (internal)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orbit")
	float CurrentOrbitAngle;

	// Current rotation angle
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orbit")
	float CurrentRotation;

	// True anomaly (current position in orbit)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orbit")
	float TrueAnomaly;

	// Orbital period in seconds
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orbit")
	float OrbitalPeriod;

	// Time since last periapsis passage
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orbit")
	float TimeSincePeriapsis;

	// Orbit inclination (tilt from orbital plane)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	float OrbitInclination;

	// Target body to orbit around (can be Sun, Planet, Moon, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	TWeakObjectPtr<AOrbitalBody> OrbitTarget;

	// ============================================================================
	// PHYSICS SETTINGS
	// ============================================================================

	// Enable physics simulation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	bool bEnablePhysics;

	// Enable gravity calculations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	bool bEnableGravity;

	// Enable collision detection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	bool bEnableCollision;

	// ============================================================================
	// ATMOSPHERE PROPERTIES
	// ============================================================================

	// Does this body have an atmosphere?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	bool bHasAtmosphere;

	// Surface pressure in kPa
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	float SurfacePressure;

	// Atmospheric scale height in km
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	float ScaleHeight;

	// ============================================================================
	// DEBUG PROPERTIES
	// ============================================================================

	// Enable debug visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebug;

	// Debug visualization color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	FColor DebugColor;

	// ============================================================================
	// TIME CONTROL
	// ============================================================================

	// Time warp factor for simulation speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float TimeWarpFactor;

	// ============================================================================
	// GRAVITATIONAL CALCULATIONS
	// ============================================================================

	// Calculate gravitational force from OrbitTarget
	UFUNCTION(BlueprintCallable, Category = "Physics")
	FVector CalculateGravitationalForce() const;

	// Calculate escape velocity from current position
	UFUNCTION(BlueprintCallable, Category = "Physics")
	float CalculateEscapeVelocity() const;

	// Check if current velocity will result in orbit or escape
	UFUNCTION(BlueprintCallable, Category = "Physics")
	bool WillEscape() const;

	// Get the radius of the target body (in km)
	UFUNCTION(BlueprintCallable, Category = "Physics")
	float GetTargetBodyRadius() const;

	// Get the sphere of influence radius (in km)
	UFUNCTION(BlueprintCallable, Category = "Physics")
	float GetSphereOfInfluence() const;

	// Get the radius of this body (in km)
	UFUNCTION(BlueprintCallable, Category = "Physics")
	float GetBodyRadius() const { return Radius; }

	// ============================================================================
	// ORBIT CONTROL
	// ============================================================================

	// Initialize stable circular orbit at current distance
	UFUNCTION(BlueprintCallable, Category = "Orbit")
	void InitializeCircularOrbit();

	// Apply impulse (for jumps, thrust, etc.)
	UFUNCTION(BlueprintCallable, Category = "Physics")
	void ApplyImpulse(FVector Impulse);

	// Land on target body (attach and stop orbiting)
	UFUNCTION(BlueprintCallable, Category = "Orbit")
	void Land();

	// Take off from landed state (enter ballistic mode)
	UFUNCTION(BlueprintCallable, Category = "Orbit")
	void TakeOff(FVector InitialVelocity);

	// Set orbital elements for realistic elliptical orbits
	UFUNCTION(BlueprintCallable, Category = "Orbit")
	void SetOrbitalElements(float SemiMajorAxis, float Eccentricity, float Inclination, float ArgumentOfPeriapsis, float LongitudeOfAscendingNode);

	// ============================================================================
	// LANDING SYSTEM
	// ============================================================================

	// Transform relative to landed body when landed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landing")
	FTransform LandedTransform;

	// ============================================================================
	// DELEGATES
	// ============================================================================

	// Called when this body lands on another body
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLanded, AOrbitalBody*, TargetBody);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLanded OnLanded;

	// Called when this body establishes a stable orbit
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOrbitEstablished);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnOrbitEstablished OnOrbitEstablished;

	// Called when this body takes off from a landed state
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTakenOff);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTakenOff OnTakenOff;

	// Get the body's mass
	UFUNCTION(BlueprintCallable, Category = "Physics")
	float GetMass() const { return Mass; }

	// Get the standard gravitational parameter
	UFUNCTION(BlueprintCallable, Category = "Physics")
	float GetStandardGravitationalParameter() const { return StandardGravitationalParameter; }

protected:

	// Update rotation based on rotation period
	void UpdateRotation(float DeltaTime);

	// Draw debug information
	void DrawDebug() const;

	// Update orbital position based on Kepler mechanics
	void UpdateOrbitalPosition(float DeltaTime);

	// Update ballistic trajectory
	void UpdateBallisticTrajectory(float DeltaTime);

	// Constants
	static constexpr float GravitationalConstant = 6.67430e-11f; // G in m³/(kg·s²)

};
