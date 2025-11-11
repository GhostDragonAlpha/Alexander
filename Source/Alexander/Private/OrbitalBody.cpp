// Copyright Epic Games, Inc. All Rights Reserved.

#include "OrbitalBody.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodySetup.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"  // For replication macros, bReplicates, bReplicateMovement
#include "SystemValidation.h"  // For validation framework

// Conversion factors
static const double KM_TO_CM = 100000.0;  // 1 km = 100,000 cm
static const double CM_TO_KM = 1.0 / KM_TO_CM;
static const double GRAVITATIONAL_CONSTANT = 6.67430e-11;  // m³/(kg·s²)

void AOrbitalBody::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOrbitalBody, ThrustPower);
}

AOrbitalBody::AOrbitalBody()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
    bReplicates = true;
    bNetLoadOnClient = true;
    bNetUseOwnerRelevancy = true;
    bRelevantForNetworkReplays = true;

    // Set up root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Create static mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
    MeshComponent->SetSimulatePhysics(false);
    MeshComponent->SetGenerateOverlapEvents(true);
    MeshComponent->SetNotifyRigidBodyCollision(true);
    MeshComponent->SetEnableGravity(false);
    MeshComponent->SetLinearDamping(0.0f);
    MeshComponent->SetAngularDamping(0.0f);

    // Create orbital mechanics component
    OrbitalMechanics = CreateDefaultSubobject<UOrbitalMechanics>(TEXT("OrbitalMechanics"));
    
    // Initialize physics component (will be set in PostInitializeComponents)
    PhysicsComponent = nullptr;

    // Initialize default values
    Mass = 5.972e24; // Earth mass by default (kg)
    Radius = 6371.0; // Earth radius (km)
    StandardGravitationalParameter = 3.986004418e5; // Earth's μ (km³/s²)
    RotationPeriod = 23.9345; // Earth's sidereal day (hours)
    AxialTilt = 23.44; // Earth's axial tilt (degrees)
    CurrentRotation = 0.0;
    
    // Initialize orbital parameters
    OrbitMode = EOrbitMode::Root;
    TrueAnomaly = 0.0;
    OrbitalPeriod = 0.0;
    TimeSincePeriapsis = 0.0;
    
    // Initialize physics
    Velocity = FVector::ZeroVector;
    AngularVelocity = FVector(0.0f, 0.0f, 0.0f);
    bEnablePhysics = true;
    bEnableGravity = true;
    bEnableCollision = true;
    
    // Initialize atmosphere
    bHasAtmosphere = false;
    SurfacePressure = 101.325; // Earth sea level pressure (kPa)
    ScaleHeight = 8.5; // Earth's scale height (km)
    
    // Initialize debugging
    bDrawDebug = false;
    
    // Initialize time control
    TimeWarpFactor = 1.0f; // Real-time by default
    DebugColor = FColor::White;
    
    // Enable replication
    SetReplicateMovement(true);
    SetReplicates(true);
}

void AOrbitalBody::BeginPlay()
{
    Super::BeginPlay();

    // Cache physics component
    PhysicsComponent = Cast<UPrimitiveComponent>(GetRootComponent());
    if (!PhysicsComponent && MeshComponent)
    {
        PhysicsComponent = MeshComponent;
    }

    // Initialize physics properties
    if (PhysicsComponent)
    {
        PhysicsComponent->SetSimulatePhysics(bEnablePhysics);
        PhysicsComponent->SetEnableGravity(false); // We handle gravity manually
        PhysicsComponent->SetNotifyRigidBodyCollision(bEnableCollision);
        
        // Set mass if simulating physics
        if (bEnablePhysics && Mass > 0.0)
        {
            PhysicsComponent->SetMassOverrideInKg(NAME_None, Mass);
        }
    }

    // Initialize orbital mechanics
    if (OrbitalMechanics)
    {
        OrbitalMechanics->SetGravitationalParameter(StandardGravitationalParameter);
        
        // If we have an orbit target, set it up
        if (OrbitTarget.IsValid() && OrbitMode != EOrbitMode::Root)
        {
            OrbitalMechanics->SetPrimaryBody(OrbitTarget.Get());
            
            // If in orbit mode, initialize the orbit
            if (OrbitMode == EOrbitMode::Orbit)
            {
                InitializeCircularOrbit();
            }
        }
    }
    
    // Initial debug draw
    if (bDrawDebug)
    {
        DrawDebug();
    }
}

void AOrbitalBody::Tick(float DeltaTime)
{
    // Validate DeltaTime to prevent division by zero and invalid physics calculations
    VALIDATE_OR_EXECUTE(USystemValidation::ValidatePositive(DeltaTime, TEXT("DeltaTime"), TEXT("OrbitalBody::Tick")), { return; });

    Super::Tick(DeltaTime);

    // Skip if we're not the authority
    if (!HasAuthority() && !IsNetMode(NM_Standalone))
    {
        return;
    }

    // Update rotation
    UpdateRotation(DeltaTime);

    // Update position based on current mode
    switch (OrbitMode)
    {
    case EOrbitMode::Orbit:
        UpdateOrbitalPosition(DeltaTime);
        break;

    case EOrbitMode::Ballistic:
    case EOrbitMode::Thrust:
        UpdateBallisticTrajectory(DeltaTime);
        break;

    case EOrbitMode::Landed:
        // Landed: follow the target's position and rotation
        if (OrbitTarget.IsValid() && LandedTransform.IsValid())
        {
            FTransform TargetTransform = OrbitTarget->GetTransform() * LandedTransform;
            SetActorTransform(TargetTransform, false, nullptr, ETeleportType::TeleportPhysics);
        }
        break;

    case EOrbitMode::Root:
        // Root bodies don't move (they define the coordinate system)
        break;
    }

    // Update physics if enabled
    if (bEnablePhysics && PhysicsComponent)
    {
        // Apply gravity if enabled
        if (bEnableGravity && OrbitTarget.IsValid() && OrbitMode != EOrbitMode::Root)
        {
            FVector GravityForce = CalculateGravitationalForce();
            PhysicsComponent->AddForce(GravityForce, NAME_None, true);
        }

        // Update velocity from physics
        Velocity = PhysicsComponent->GetPhysicsLinearVelocity() * CM_TO_KM; // Convert to km/s
    }

    // Update orbital elements if we have a target
    if (OrbitTarget.IsValid() && OrbitalMechanics)
    {
        OrbitalMechanics->UpdateOrbitalElements();
    }

    // Debug drawing
    if (bDrawDebug)
    {
        DrawDebug();
    }
}

void AOrbitalBody::UpdateOrbitalPosition(float DeltaTime)
{
    // Validate inputs and critical pointers
    VALIDATE_OR_EXECUTE(USystemValidation::ValidatePositive(DeltaTime, TEXT("DeltaTime"), TEXT("OrbitalBody::UpdateOrbitalPosition")), { return; });
    VALIDATE_OR_EXECUTE(USystemValidation::ValidateNotNull(OrbitTarget.Get(), TEXT("OrbitTarget"), TEXT("OrbitalBody::UpdateOrbitalPosition")), { return; });
    VALIDATE_OR_EXECUTE(USystemValidation::ValidateNotNull(OrbitalMechanics, TEXT("OrbitalMechanics"), TEXT("OrbitalBody::UpdateOrbitalPosition")), { return; });

    // Get current orbital elements
    const FOrbitalElements& Elements = OrbitalMechanics->GetCurrentOrbitalElements();
    
    // Validate orbital elements before calculations
    VALIDATE_OR_EXECUTE(USystemValidation::ValidatePositive(Elements.SemiMajorAxis, TEXT("SemiMajorAxis"), TEXT("OrbitalBody::UpdateOrbitalPosition")), { return; });
    VALIDATE_OR_EXECUTE(USystemValidation::ValidateRange(Elements.Eccentricity, 0.0f, 0.999f, TEXT("Eccentricity"), TEXT("OrbitalBody::UpdateOrbitalPosition")), { return; });
    VALIDATE_OR_EXECUTE(USystemValidation::ValidatePositive(Elements.StandardGravitationalParameter, TEXT("StandardGravitationalParameter"), TEXT("OrbitalBody::UpdateOrbitalPosition")), { return; });

    // Update time since periapsis
    TimeSincePeriapsis += DeltaTime * TimeWarpFactor;
    
    // Calculate mean anomaly
    double CurrentOrbitalPeriod = UOrbitalMechanics::CalculateOrbitalPeriod(Elements.SemiMajorAxis, Elements.StandardGravitationalParameter);
    double MeanAnomaly = (2.0 * PI * TimeSincePeriapsis) / CurrentOrbitalPeriod;
    MeanAnomaly = FMath::Fmod(MeanAnomaly, 2.0 * PI);
    
    // Solve Kepler's equation for eccentric anomaly (using Newton's method)
    double EccentricAnomaly = MeanAnomaly; // Initial guess
    double Eccentricity = Elements.Eccentricity;
    
    for (int32 i = 0; i < 10; ++i) // Limit iterations for stability
    {
        double DeltaE = (MeanAnomaly - (EccentricAnomaly - Eccentricity * FMath::Sin(EccentricAnomaly))) /
                       (1.0 - Eccentricity * FMath::Cos(EccentricAnomaly));
        
        EccentricAnomaly += DeltaE;
        
        if (FMath::Abs(DeltaE) < 1e-6)
        {
            break;
        }
    }
    
    // Calculate true anomaly
    double CosE = FMath::Cos(EccentricAnomaly);
    double SinE = FMath::Sin(EccentricAnomaly);
    double TrueAnomalyRad = FMath::Atan2(
        FMath::Sqrt(1.0 - Eccentricity * Eccentricity) * SinE,
        CosE - Eccentricity
    );
    
    // Update true anomaly
    TrueAnomaly = FMath::RadiansToDegrees(TrueAnomalyRad);
    
    // Calculate distance from focus
    double Distance = Elements.SemiMajorAxis * (1.0 - Eccentricity * CosE);
    
    // Calculate position in orbital plane
    double X = Distance * FMath::Cos(TrueAnomalyRad);
    double Y = Distance * FMath::Sin(TrueAnomalyRad);
    
    // Convert to 3D space (apply orbital elements)
    double InclinationRad = FMath::DegreesToRadians(Elements.Inclination);
    double LongitudeAscendingNodeRad = FMath::DegreesToRadians(Elements.LongitudeAscendingNode);
    double ArgumentOfPeriapsisRad = FMath::DegreesToRadians(Elements.ArgumentOfPeriapsis);
    
    // Apply rotations (3-1-3 Euler angles)
    double CosLAN = FMath::Cos(LongitudeAscendingNodeRad);
    double SinLAN = FMath::Sin(LongitudeAscendingNodeRad);
    double CosI = FMath::Cos(InclinationRad);
    double SinI = FMath::Sin(InclinationRad);
    double CosAOP = FMath::Cos(ArgumentOfPeriapsisRad);
    double SinAOP = FMath::Sin(ArgumentOfPeriapsisRad);
    
    // Rotation matrix (3-1-3)
    double R11 = CosLAN * CosAOP - SinLAN * SinAOP * CosI;
    double R12 = -CosLAN * SinAOP - SinLAN * CosAOP * CosI;
    double R13 = SinLAN * SinI;
    
    double R21 = SinLAN * CosAOP + CosLAN * SinAOP * CosI;
    double R22 = -SinLAN * SinAOP + CosLAN * CosAOP * CosI;
    double R23 = -CosLAN * SinI;
    
    double R31 = SinAOP * SinI;
    double R32 = CosAOP * SinI;
    double R33 = CosI;
    
    // Apply rotation to position
    FVector Position3D(
        R11 * X + R12 * Y,
        R21 * X + R22 * Y,
        R31 * X + R32 * Y
    );
    
    // Calculate velocity (optional, for more accurate physics)
    double Mu = OrbitalMechanics->GetGravitationalParameter();
    double P = Elements.SemiMajorAxis * (1.0 - Eccentricity * Eccentricity);
    double H = FMath::Sqrt(Mu * P);
    
    double Vr = (Mu / H) * Eccentricity * FMath::Sin(TrueAnomalyRad);
    double Vt = (Mu / H) * (1.0 + Eccentricity * FMath::Cos(TrueAnomalyRad));
    
    FVector Velocity3D(
        R11 * (Vr * FMath::Cos(TrueAnomalyRad) - Vt * FMath::Sin(TrueAnomalyRad)) +
        R12 * (Vr * FMath::Sin(TrueAnomalyRad) + Vt * FMath::Cos(TrueAnomalyRad)),
        
        R21 * (Vr * FMath::Cos(TrueAnomalyRad) - Vt * FMath::Sin(TrueAnomalyRad)) +
        R22 * (Vr * FMath::Sin(TrueAnomalyRad) + Vt * FMath::Cos(TrueAnomalyRad)),
        
        R31 * (Vr * FMath::Cos(TrueAnomalyRad) - Vt * FMath::Sin(TrueAnomalyRad)) +
        R32 * (Vr * FMath::Sin(TrueAnomalyRad) + Vt * FMath::Cos(TrueAnomalyRad))
    );
    
    // Update position and velocity
    FVector NewPosition = OrbitTarget->GetActorLocation() + (Position3D * KM_TO_CM);
    Velocity = Velocity3D; // km/s
    
    // Update actor transform
    if (bEnablePhysics && PhysicsComponent)
    {
        // Update physics state
        PhysicsComponent->SetPhysicsLinearVelocity(Velocity * 1000.0f, false); // Convert to cm/s
        
        // Calculate orientation (point along velocity vector)
        if (!Velocity.IsNearlyZero())
        {
            FRotator NewRotation = Velocity.Rotation();
            PhysicsComponent->SetWorldRotation(NewRotation);
        }
        
        // Teleport physics body to new position
        FHitResult Hit;
        PhysicsComponent->SetWorldLocation(NewPosition, false, &Hit, ETeleportType::TeleportPhysics);
    }
    else
    {
        // Just update transform directly
        SetActorLocation(NewPosition);
    }
}

void AOrbitalBody::UpdateBallisticTrajectory(float DeltaTime)
{
    if (!OrbitTarget.IsValid())
    {
        // No gravity source, just move with current velocity
        FVector NewLocation = GetActorLocation() + (Velocity * DeltaTime);
        
        if (bEnablePhysics && PhysicsComponent)
        {
            PhysicsComponent->SetPhysicsLinearVelocity(Velocity, false);
            PhysicsComponent->SetWorldLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
        }
        else
        {
            SetActorLocation(NewLocation);
        }
        return;
    }
    
    // Get the primary body's position and mass
    FVector PrimaryPosition = OrbitTarget->GetActorLocation();
    double PrimaryMu = OrbitTarget->GetStandardGravitationalParameter();
    
    // Calculate relative position and distance
    FVector RelativePosition = GetActorLocation() - PrimaryPosition;
    double Distance = RelativePosition.Size() * CM_TO_KM; // Convert to km
    
    if (Distance < KINDA_SMALL_NUMBER)
    {
        // Avoid division by zero
        return;
    }
    
    // Calculate gravitational acceleration (a = μ/r²)
    double GravitationalAccel = PrimaryMu / (Distance * Distance); // km/s²
    
    // Calculate acceleration vector (towards primary body)
    FVector GravitationalDirection = -RelativePosition.GetSafeNormal();
    FVector Acceleration = GravitationalDirection * (GravitationalAccel * 1000.0f); // Convert to m/s²
    
    // Update velocity (v = v0 + a*t)
    Velocity += Acceleration * DeltaTime; // km/s
    
    // Update position (x = x0 + v*t)
    FVector NewPosition = GetActorLocation() + (Velocity * DeltaTime);
    
    // Check for collision with primary body
    double PrimaryRadius = OrbitTarget->GetBodyRadius() * 100.0f; // km to m
    if (Distance * 1000.0f <= PrimaryRadius * 1.1f) // 10% margin
    {
        // We've collided with the primary body
        if (OrbitMode == EOrbitMode::Ballistic)
        {
            // Land on the surface
            Land();
            return;
        }
        else if (OrbitMode == EOrbitMode::Thrust)
        {
            // Bounce off the surface (simplified)
            Velocity = -Velocity * 0.5f; // 50% energy loss
            NewPosition = PrimaryPosition + GravitationalDirection * (PrimaryRadius * 1.1f * 100.0f); // Just above surface
        }
    }
    
    // Update actor transform
    if (bEnablePhysics && PhysicsComponent)
    {
        // Update physics state
        PhysicsComponent->SetPhysicsLinearVelocity(Velocity * 1000.0f, false); // Convert to cm/s
        
        // Calculate orientation (point along velocity vector if moving)
        if (!Velocity.IsNearlyZero())
        {
            FRotator NewRotation = Velocity.Rotation();
            PhysicsComponent->SetWorldRotation(NewRotation);
        }
        
        // Teleport physics body to new position
        PhysicsComponent->SetWorldLocation(NewPosition, false, nullptr, ETeleportType::TeleportPhysics);
    }
    else
    {
        // Just update transform directly
        SetActorLocation(NewPosition);
    }
    
    // Update orbital elements if we have an orbital mechanics component
    if (OrbitalMechanics && OrbitTarget.IsValid())
    {
        // Convert to orbital elements
        FOrbitalStateVectors StateVectors;
        StateVectors.Position = (GetActorLocation() - OrbitTarget->GetActorLocation()) * CM_TO_KM; // Convert to km
        StateVectors.Velocity = Velocity; // Already in km/s
        StateVectors.EpochTime = GetWorld()->GetTimeSeconds();
        
        // Update orbital elements
        OrbitalMechanics->SetStateVectors(StateVectors);
        OrbitalMechanics->UpdateOrbitalElements();
    }

    // Calculate gravitational acceleration
    FVector GravityAccel = CalculateGravitationalForce() / Mass;

    // Apply velocity Verlet integration for better orbital stability
    FVector CurrentLocation = GetActorLocation();
    FVector NewLocation = CurrentLocation + (Velocity * DeltaTime) + (0.5f * GravityAccel * DeltaTime * DeltaTime);
    
    // Calculate new gravity at new position
    FVector OldLocation = GetActorLocation();
    SetActorLocation(NewLocation);
    FVector NewGravityAccel = CalculateGravitationalForce() / Mass;
    
    // Update velocity using average acceleration
    Velocity += 0.5f * (GravityAccel + NewGravityAccel) * DeltaTime;

    // Check for collision with target body (landing detection)
    float DistanceToTarget = FVector::Dist(NewLocation, OrbitTarget->GetActorLocation());
    float TargetRadius = GetTargetBodyRadius();

    // If we're within the target's radius, we've landed
    if (DistanceToTarget <= TargetRadius)
    {
        Land();
        return;
    }

    // Check if we've escaped the system's influence
    if (WillEscape() && DistanceToTarget > GetSphereOfInfluence())
    {
        // We've escaped - clear orbit target
        OrbitTarget = nullptr;
        OrbitMode = EOrbitMode::Root;
        UE_LOG(LogTemp, Log, TEXT("%s has escaped the gravitational system"), *GetName());
    }
}

FVector AOrbitalBody::CalculateGravitationalForce() const
{
    if (!OrbitTarget.IsValid())
    {
        return FVector::ZeroVector;
    }

    // Get direction to target (from this to target)
    FVector Direction = OrbitTarget->GetActorLocation() - GetActorLocation();
    double Distance = Direction.Size() * 0.01; // Convert cm to m

    // Check for zero distance
    if (Distance < SMALL_NUMBER)
    {
        return FVector::ZeroVector;
    }

    // Normalize direction
    Direction.Normalize();

    // Calculate gravitational force magnitude (F = G * (m1 * m2) / r²)
    // Using double precision for better accuracy with large numbers
    double ForceMagnitude = GRAVITATIONAL_CONSTANT * (Mass * OrbitTarget->GetMass()) / (Distance * Distance);
    
    // Convert to kN for Unreal physics (1 N = 1 kg·m/s²)
    return Direction * (ForceMagnitude * 0.001f); // Convert N to kN
}

float AOrbitalBody::CalculateEscapeVelocity() const
{
    if (!OrbitTarget.IsValid())
    {
        return 0.0f;
    }

    // Get distance to primary body in meters
    double Distance = (GetActorLocation() - OrbitTarget->GetActorLocation()).Size() * 0.01; // cm to m
    
    // Escape velocity: v = sqrt(2GM/r)
    // Using double precision for better accuracy with large numbers
    double EscapeVelocity = FMath::Sqrt(2.0 * GRAVITATIONAL_CONSTANT * OrbitTarget->GetMass() / Distance);
    
    // Convert to km/s for consistency
    return EscapeVelocity * 0.001f; // m/s to km/s
}

bool AOrbitalBody::WillEscape() const
{
    if (!OrbitTarget.IsValid())
    {
        return true; // No orbit target, not bound to anything
    }

    float CurrentSpeed = Velocity.Size();
    float EscapeVel = CalculateEscapeVelocity();

    return CurrentSpeed >= EscapeVel;
}

void AOrbitalBody::InitializeCircularOrbit()
{
    if (!OrbitTarget.IsValid())
    {
        return;
    }

    // Calculate distance from target
    FVector ToTarget = OrbitTarget->GetActorLocation() - GetActorLocation();
    OrbitRadius = ToTarget.Size();

    // Calculate angle
    CurrentOrbitAngle = FMath::RadiansToDegrees(FMath::Atan2(ToTarget.Y, ToTarget.X));

    // Calculate required velocity for circular orbit: v = sqrt(G * M / r)
    float OrbitalVelocity = FMath::Sqrt(GravitationalConstant * OrbitTarget->Mass / OrbitRadius);

    // Convert to angular velocity (degrees per second)
    // omega = v / r, then convert radians to degrees
    OrbitSpeed = FMath::RadiansToDegrees(OrbitalVelocity / OrbitRadius);

    // Set mode to orbit
    OrbitMode = EOrbitMode::Orbit;
}

void AOrbitalBody::ApplyImpulse(FVector Impulse)
{
    // Convert impulse from N·s to kg·km/s (1 N·s = 1 kg·m/s = 0.001 kg·km/s)
    FVector DeltaV = Impulse * (1.0f / Mass) * 0.001f; // Convert to km/s
    
    // Update velocity
    Velocity += DeltaV;
    
    // Update physics if enabled
    if (bEnablePhysics && PhysicsComponent)
    {
        PhysicsComponent->AddImpulse(Impulse, NAME_None, true);
    }
    
    // If we were in orbit mode, switch to ballistic mode
    if (OrbitMode == EOrbitMode::Orbit)
    {
        OrbitMode = EOrbitMode::Ballistic;
    }
}

void AOrbitalBody::Land()
{
    if (!OrbitTarget.IsValid())
    {
        return;
    }

    // Save the current transform relative to the target
    FTransform RelativeTransform = GetTransform().GetRelativeTransform(OrbitTarget->GetTransform());
    LandedTransform = RelativeTransform;
    
    // Stop all movement
    Velocity = FVector::ZeroVector;
    
    if (PhysicsComponent)
    {
        PhysicsComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
        PhysicsComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        PhysicsComponent->SetSimulatePhysics(false);
    }
    
    // Set orbit mode
    OrbitMode = EOrbitMode::Landed;
    
    // Notify blueprint
    OnLanded.Broadcast(OrbitTarget.Get());
}

void AOrbitalBody::TakeOff(FVector InitialVelocity)
{
    if (OrbitMode != EOrbitMode::Landed || !OrbitTarget.IsValid())
    {
        return;
    }

    // Switch to ballistic mode
    OrbitMode = EOrbitMode::Ballistic;
    
    // Convert velocity to km/s
    Velocity = InitialVelocity;
    
    // Enable physics if needed
    if (PhysicsComponent)
    {
        PhysicsComponent->SetSimulatePhysics(true);
        PhysicsComponent->SetPhysicsLinearVelocity(Velocity * 1000.0f, false); // Convert to cm/s
    }
    
    // Clear landed transform
    LandedTransform = FTransform::Identity;
    
    // Notify blueprint
    OnTakenOff.Broadcast();
}

float AOrbitalBody::GetTargetBodyRadius() const
{
    if (!OrbitTarget.IsValid())
    {
        return 0.0f;
    }
    
    // Get the target's radius in km
    return OrbitTarget->GetBodyRadius();
}

float AOrbitalBody::GetSphereOfInfluence() const
{
    if (!OrbitTarget.IsValid())
    {
        return 0.0f;
    }
    
    // Calculate sphere of influence using the standard formula:
    // SOI = a * (m/M)^(2/5)
    // Where:
    // - a is the semi-major axis of the orbit (km)
    // - m is the mass of this body (kg)
    // - M is the mass of the primary body (kg)
    
    // Get distance to primary in km
    double Distance = (GetActorLocation() - OrbitTarget->GetActorLocation()).Size() * CM_TO_KM;
    
    // Calculate mass ratio (ensure we don't divide by zero)
    double PrimaryMass = OrbitTarget->GetMass();
    if (PrimaryMass < SMALL_NUMBER)
    {
        return 0.0f;
    }
    
    double MassRatio = Mass / PrimaryMass;
    
    // Calculate SOI radius in km
    double SOI = Distance * FMath::Pow(MassRatio, 0.4);
    
    return static_cast<float>(SOI);
}

void AOrbitalBody::SetOrbitalElements(float InSemiMajorAxis, float InEccentricity, float InInclination, float InArgumentOfPeriapsis, float InLongitudeOfAscendingNode)
{
    if (!OrbitTarget.IsValid() || !OrbitalMechanics)
    {
        return;
    }
    
    // Validate inputs
    float SemiMajorAxis = FMath::Max(KINDA_SMALL_NUMBER, InSemiMajorAxis);
    float Eccentricity = FMath::Clamp(InEccentricity, 0.0f, 0.999f); // Cap at 0.999 to avoid division by zero
    float Inclination = FMath::Fmod(InInclination, 360.0f);
    float ArgumentOfPeriapsis = FMath::Fmod(InArgumentOfPeriapsis, 360.0f);
    float LongitudeOfAscendingNode = FMath::Fmod(InLongitudeOfAscendingNode, 360.0f);
    
    // Update orbital elements
    FOrbitalElements Elements;
    Elements.SemiMajorAxis = SemiMajorAxis;
    Elements.Eccentricity = Eccentricity;
    Elements.Inclination = Inclination;
    Elements.ArgumentOfPeriapsis = ArgumentOfPeriapsis;
    Elements.LongitudeAscendingNode = LongitudeOfAscendingNode;
    Elements.TrueAnomaly = 0.0f; // Start at periapsis
    Elements.StandardGravitationalParameter = OrbitTarget->GetStandardGravitationalParameter();
    
    // Update orbital mechanics
    OrbitalMechanics->SetOrbitalElements(Elements);
    
    // Calculate initial state vectors
    FOrbitalStateVectors StateVectors = UOrbitalMechanics::ElementsToStateVectors(Elements);
    
    // Update position and velocity
    FVector NewPosition = OrbitTarget->GetActorLocation() + (StateVectors.Position * KM_TO_CM);
    Velocity = StateVectors.Velocity; // km/s
    
    // Update actor transform
    if (bEnablePhysics && PhysicsComponent)
    {
        PhysicsComponent->SetPhysicsLinearVelocity(Velocity * 1000.0f, false); // Convert to cm/s
        
        // Calculate orientation (point along velocity vector)
        if (!Velocity.IsNearlyZero())
        {
            FRotator NewRotation = Velocity.Rotation();
            PhysicsComponent->SetWorldRotation(NewRotation);
        }
        
        // Teleport physics body to new position
        PhysicsComponent->SetWorldLocation(NewPosition, false, nullptr, ETeleportType::TeleportPhysics);
    }
    else
    {
        SetActorLocation(NewPosition);
    }
    
    // Update orbital period
    OrbitalPeriod = UOrbitalMechanics::CalculateOrbitalPeriod(SemiMajorAxis, Elements.StandardGravitationalParameter);
    TimeSincePeriapsis = 0.0;
    
    // Set orbit mode
    OrbitMode = EOrbitMode::Orbit;
    
    // Notify blueprint
    OnOrbitEstablished.Broadcast();
    
    // Calculate 2D position in orbital plane
    float InclinationRad = FMath::DegreesToRadians(Inclination);
    FVector Position2D = FVector(SemiMajorAxis * 1000.0f, 0.0f, 0.0f); // Convert km to cm
    Position2D.Z = Position2D.Y * FMath::Sin(InclinationRad);
    Position2D.Y = Position2D.Y * FMath::Cos(InclinationRad);
    
    // Set world position
    SetActorLocation(OrbitTarget->GetActorLocation() + Position2D);
    
    // Calculate orbital velocity for this position
    float Distance = Position2D.Size() * CM_TO_KM; // Convert back to km
    float OrbitalSpeed = FMath::Sqrt(GravitationalConstant * OrbitTarget->Mass * (2.0f / Distance - 1.0f / SemiMajorAxis));
    
    // Velocity is perpendicular to radius vector
    FVector VelocityDirection = FVector(-Position2D.Y, Position2D.X, 0.0f).GetSafeNormal();
    Velocity = VelocityDirection * OrbitalSpeed;

	// Set to ballistic mode to use physics simulation
	OrbitMode = EOrbitMode::Ballistic;
}

void AOrbitalBody::UpdateRotation(float DeltaTime)
{
	// Update rotation based on rotation period (in hours)
	if (!FMath::IsNearlyZero(RotationPeriod))
	{
		// Convert DeltaTime from seconds to hours and calculate rotation increment
		float DeltaHours = DeltaTime / 3600.0f;
		float RotationIncrement = (DeltaHours / RotationPeriod) * 360.0f; // Degrees per frame

		CurrentRotation += RotationIncrement;
		CurrentRotation = FMath::Fmod(CurrentRotation, 360.0f); // Keep in 0-360 range

		// Apply to actor rotation
		FRotator NewRotation = GetActorRotation();
		NewRotation.Yaw = CurrentRotation;
		SetActorRotation(NewRotation);
	}
}

void AOrbitalBody::DrawDebug() const
{
	// Debug visualization using DrawDebugHelpers
	if (GetWorld())
	{
		FVector Location = GetActorLocation();

		// Draw body as sphere
		DrawDebugSphere(GetWorld(), Location, Radius, 12, FColor::Green, false, -1.0f, 0, 2.0f);

		// Draw orbit path if we have a target
		if (OrbitTarget.IsValid() && OrbitalMechanics)
		{
			// TODO: Draw orbital path visualization
		}
	}
}

void AOrbitalBody::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	// Initialize physics component after all components are created
	if (!PhysicsComponent)
	{
		PhysicsComponent = FindComponentByClass<UPrimitiveComponent>();
	}
}

void AOrbitalBody::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	// Construction-time initialization
	if (MeshComponent && !MeshComponent->GetStaticMesh())
	{
		// Will be set up in editor or at runtime
	}
}

void AOrbitalBody::PostLoad()
{
	Super::PostLoad();
	
	// Post-load initialization
	if (OrbitalMechanics)
	{
		// OrbitalMechanics initialization if needed
	}
}

void AOrbitalBody::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, 
	bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	
	// Handle collisions if needed
	if (bEnableCollision && Other)
	{
		// Collision handling
	}
}
