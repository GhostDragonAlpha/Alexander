// Copyright Epic Games, Inc. All Rights Reserved.

#include "OrbitalMechanics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "OrbitalBody.h"
#include "GameFramework/Actor.h"

using namespace OrbitalConstants;

UOrbitalMechanics::UOrbitalMechanics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UOrbitalMechanics::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize state vectors from current actor position and velocity
    AActor* Owner = GetOwner();
    if (Owner)
    {
        CurrentStateVectors.Position = Owner->GetActorLocation() / 1000.0f; // Convert to km
        
        // Try to get velocity from physics component
        if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
        {
            CurrentStateVectors.Velocity = RootPrimitive->GetPhysicsLinearVelocity() / 1000.0f; // Convert to km/s
        }
        
        // Convert to orbital elements
        CurrentOrbitalElements = StateVectorsToElements(CurrentStateVectors, CurrentMu);
        
        // Calculate sphere of influence if we have a primary body
        if (PrimaryBody.IsValid())
        {
            if (const AOrbitalBody* OrbitalBody = Cast<AOrbitalBody>(Owner))
            {
                const AOrbitalBody* PrimaryOrbitalBody = Cast<AOrbitalBody>(PrimaryBody.Get());
                if (PrimaryOrbitalBody)
                {
                    SphereOfInfluence = CalculateSphereOfInfluence(
                        FVector::Dist(Owner->GetActorLocation(), PrimaryBody->GetActorLocation()) / 1000.0f,
                        PrimaryOrbitalBody->GetMass(),
                        OrbitalBody->GetMass()
                    );
                }
            }
        }
    }
}

void UOrbitalMechanics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Apply time warp
    const double ScaledDeltaTime = DeltaTime * TimeWarpFactor;
    SimulationTime += ScaledDeltaTime;
    
    // Update orbital state
    if (PrimaryBody.IsValid())
    {
        // In a real implementation, we would:
        // 1. Update the position based on current orbital elements
        // 2. Handle sphere of influence transitions
        // 3. Apply perturbations from other bodies
        
        // For now, just update the state vectors based on current orbit
        CurrentStateVectors = ElementsToStateVectors(CurrentOrbitalElements);
        
        // Update actor transform
        AActor* Owner = GetOwner();
        if (Owner)
        {
            // Convert from km to Unreal units (cm)
            FVector NewLocation = CurrentStateVectors.Position * 1000.0f;
            
            // Calculate orientation (point velocity vector)
            FVector Direction = CurrentStateVectors.Velocity.GetSafeNormal();
            if (!Direction.IsNearlyZero())
            {
                FRotator NewRotation = Direction.Rotation();
                Owner->SetActorLocationAndRotation(NewLocation, NewRotation);
                
                // Update physics velocity if available
                if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
                {
                    RootPrimitive->SetPhysicsLinearVelocity(CurrentStateVectors.Velocity * 1000.0f, false);
                }
            }
        }
    }
}

FOrbitalStateVectors UOrbitalMechanics::ElementsToStateVectors(const FOrbitalElements& Elements)
{
    FOrbitalStateVectors Result;
    
    // Convert angles to radians
    const float InclinationRad = FMath::DegreesToRadians(Elements.Inclination);
    const float LongitudeAscendingNodeRad = FMath::DegreesToRadians(Elements.LongitudeAscendingNode);
    const float ArgumentOfPeriapsisRad = FMath::DegreesToRadians(Elements.ArgumentOfPeriapsis);
    const float TrueAnomalyRad = FMath::DegreesToRadians(Elements.TrueAnomaly);
    
    // Calculate semi-latus rectum
    const float P = Elements.SemiMajorAxis * (1.0f - Elements.Eccentricity * Elements.Eccentricity);
    
    // Calculate position in the orbital plane
    const float R = p / (1.0f + Elements.Eccentricity * FMath::Cos(TrueAnomalyRad));
    const FVector2D PositionInPlane = FVector2D(
        r * FMath::Cos(TrueAnomalyRad),
        r * FMath::Sin(TrueAnomalyRad)
    );
    
    // Calculate velocity in the orbital plane
    const float Mu = Elements.StandardGravitationalParameter;
    const float H = FMath::Sqrt(Mu * p);
    const FVector2D VelocityInPlane = FVector2D(
        -FMath::Sin(TrueAnomalyRad) * (H / p),
        (Elements.Eccentricity + FMath::Cos(TrueAnomalyRad)) * (H / p)
    );
    
    // Create rotation matrices
    const FQuat RotZLong = FQuat(FVector::UpVector, -LongitudeAscendingNodeRad);
    const FQuat RotXIncl = FQuat(FVector::ForwardVector, -InclinationRad);
    const FQuat RotZArg = FQuat(FVector::UpVector, -ArgumentOfPeriapsisRad);
    
    // Combine rotations
    const FQuat TotalRotation = RotZLong * RotXIncl * RotZArg;
    
    // Transform position and velocity to 3D space
    FVector Position3D = TotalRotation.RotateVector(FVector(PositionInPlane.X, PositionInPlane.Y, 0.0f));
    FVector Velocity3D = TotalRotation.RotateVector(FVector(VelocityInPlane.X, VelocityInPlane.Y, 0.0f));
    
    // Set results
    Result.Position = Position3D;
    Result.Velocity = Velocity3D;
    Result.EpochTime = 0.0; // This would be set based on the current time
    
    return Result;
}

FOrbitalElements UOrbitalMechanics::StateVectorsToElements(const FOrbitalStateVectors& StateVectors, float Mu)
{
    FOrbitalElements Elements;
    
    const FVector& R = StateVectors.Position;
    const FVector& V = StateVectors.Velocity;
    
    // Calculate specific angular momentum
    const FVector H = FVector::CrossProduct(R, V);
    const float HMag = H.Size();
    
    // Calculate eccentricity vector
    const FVector EccentricityVec = (FVector::CrossProduct(V, H) / Mu) - (R / R.Size());
    Elements.Eccentricity = EccentricityVec.Size();
    
    // Calculate semi-major axis
    const float SpecificEnergy = (V.SizeSquared() / 2.0f) - (Mu / R.Size());
    Elements.SemiMajorAxis = -Mu / (2.0f * SpecificEnergy);
    
    // Calculate inclination
    Elements.Inclination = FMath::RadiansToDegrees(FMath::Acos(H.Z / HMag));
    
    // Calculate longitude of ascending node
    const FVector N = FVector::CrossProduct(FVector::UpVector, H);
    const float NMag = N.Size();
    if (NMag > SMALL_NUMBER)
    {
        Elements.LongitudeAscendingNode = FMath::RadiansToDegrees(FMath::Atan2(N.Y, N.X));
        
        // Calculate argument of periapsis
        const float CosOmega = FVector::DotProduct(N, EccentricityVec) / (NMag * Elements.Eccentricity);
        Elements.ArgumentOfPeriapsis = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(CosOmega, -1.0f, 1.0f)));
        
        // Adjust for correct quadrant
        if (EccentricityVec.Z < 0)
        {
            Elements.ArgumentOfPeriapsis = 360.0f - Elements.ArgumentOfPeriapsis;
        }
    }
    
    // Calculate true anomaly
    const float EV = FVector::DotProduct(EccentricityVec, V) * R.Size() / (Elements.Eccentricity * Mu);
    const float ER = FVector::DotProduct(EccentricityVec, R) / (Elements.Eccentricity * R.Size());
    Elements.TrueAnomaly = FMath::RadiansToDegrees(FMath::Atan2(EV, ER));
    
    // Ensure angles are in 0-360 range
    Elements.LongitudeAscendingNode = FMath::Fmod(Elements.LongitudeAscendingNode + 360.0f, 360.0f);
    Elements.ArgumentOfPeriapsis = FMath::Fmod(Elements.ArgumentOfPeriapsis + 360.0f, 360.0f);
    Elements.TrueAnomaly = FMath::Fmod(Elements.TrueAnomaly + 360.0f, 360.0f);
    
    // Set standard gravitational parameter
    Elements.StandardGravitationalParameter = Mu;
    
    return Elements;
}

float UOrbitalMechanics::CalculateOrbitalPeriod(float SemiMajorAxis, float Mu)
{
    if (SemiMajorAxis <= 0.0f || Mu <= 0.0f)
        return 0.0f;
        
    // T = 2π * sqrt(a³/μ)
    return 2.0f * PI * FMath::Sqrt(FMath::Cube(SemiMajorAxis) / Mu);
}

FVector UOrbitalMechanics::CalculateVelocityAtTrueAnomaly(const FOrbitalElements& Elements, float TrueAnomaly)
{
    // Convert true anomaly to radians
    const float Nu = FMath::DegreesToRadians(TrueAnomaly);
    
    // Calculate distance from focus to satellite
    const float R = (Elements.SemiMajorAxis * (1.0f - Elements.Eccentricity * Elements.Eccentricity)) / 
                   (1.0f + Elements.Eccentricity * FMath::Cos(Nu));
    
    // Calculate velocity magnitude (vis-viva equation)
    const float V = FMath::Sqrt(Elements.StandardGravitationalParameter * (2.0f / R - 1.0f / Elements.SemiMajorAxis));
    
    // Calculate flight path angle
    const float Gamma = FMath::Atan2(
        Elements.Eccentricity * FMath::Sin(Nu),
        1.0f + Elements.Eccentricity * FMath::Cos(Nu)
    );
    
    // Calculate velocity components in orbital plane
    const float Vr = V * FMath::Sin(gamma);
    const float Vt = V * FMath::Cos(gamma);
    
    // Convert to 3D (this is a simplified version, in practice you'd need to rotate to the correct plane)
    return FVector(Vt, Vr, 0.0f);
}

bool UOrbitalMechanics::CalculateHohmannTransfer(float R1, float R2, float Mu, float& OutDeltaV1, float& OutDeltaV2, float& OutTransferTime)
{
    if (R1 <= 0.0f || R2 <= 0.0f || Mu <= 0.0f)
        return false;
    
    // Ensure R1 < R2
    if (R1 > R2)
    {
        Swap(R1, R2);
    }
    
    // Semi-major axis of transfer orbit
    const float A_Transfer = (R1 + R2) / 2.0f;
    
    // Velocity at periapsis of transfer orbit
    const float Vp = FMath::Sqrt(Mu * (2.0f / R1 - 1.0f / A_Transfer));
    
    // Velocity at apoapsis of transfer orbit
    const float Va = FMath::Sqrt(Mu * (2.0f / R2 - 1.0f / A_Transfer));
    
    // Circular orbit velocities
    const float V1 = FMath::Sqrt(Mu / R1);
    const float V2 = FMath::Sqrt(Mu / R2);
    
    // Delta-V values
    OutDeltaV1 = Vp - V1;
    OutDeltaV2 = V2 - Va;
    
    // Transfer time (half the orbital period of transfer orbit)
    OutTransferTime = PI * FMath::Sqrt(FMath::Cube(A_Transfer) / Mu);
    
    return true;
}

float UOrbitalMechanics::CalculateSphereOfInfluence(float SemiMajorAxis, float PrimaryMass, float SecondaryMass)
{
    if (PrimaryMass <= 0.0f || SecondaryMass <= 0.0f)
        return 0.0f;
        
    // SOI radius: r = a * (m/M)^(2/5)
    return SemiMajorAxis * FMath::Pow(SecondaryMass / PrimaryMass, 0.4f);
}

bool UOrbitalMechanics::IsInSphereOfInfluence(const FVector& Position, const FVector& BodyPosition, float SoIRadius)
{
    return FVector::DistSquared(Position, BodyPosition) <= (SoIRadius * SoIRadius);
}

void UOrbitalMechanics::UpdateOrbitalElements()
{
    if (PrimaryBody.IsValid())
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            // Get current position and velocity relative to primary body
            FVector RelativePosition = (Owner->GetActorLocation() - PrimaryBody->GetActorLocation()) / 1000.0f; // km
            FVector RelativeVelocity = FVector::ZeroVector;
            
            if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
            {
                if (UPrimitiveComponent* PrimaryRoot = Cast<UPrimitiveComponent>(PrimaryBody->GetRootComponent()))
                {
                    RelativeVelocity = (RootPrimitive->GetPhysicsLinearVelocity() - 
                                       PrimaryRoot->GetPhysicsLinearVelocity()) / 1000.0f; // km/s
                }
            }
            
            // Update state vectors
            CurrentStateVectors.Position = RelativePosition;
            CurrentStateVectors.Velocity = RelativeVelocity;
            
            // Update orbital elements
            CurrentOrbitalElements = StateVectorsToElements(CurrentStateVectors, CurrentMu);
        }
    }
}

void UOrbitalMechanics::SetPrimaryBody(AActor* NewPrimaryBody)
{
    if (NewPrimaryBody && NewPrimaryBody != PrimaryBody)
    {
        PrimaryBody = NewPrimaryBody;
        
        // Update gravitational parameter if this is an OrbitalBody
        if (const AOrbitalBody* OrbitalBody = Cast<AOrbitalBody>(NewPrimaryBody))
        {
            CurrentMu = OrbitalBody->GetStandardGravitationalParameter();
        }
        
        // Update orbital elements
        UpdateOrbitalElements();
    }
}
