// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OrbitalMechanics.generated.h"

// Forward declarations
class AActor;
class AOrbitalBody;

/**
 * Represents orbital elements for Keplerian orbits
 */
USTRUCT(BlueprintType)
struct FOrbitalElements
{
    GENERATED_BODY()

    // Semi-major axis (km)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital Elements")
    float SemiMajorAxis = 0.0f;

    // Eccentricity (0=circular, 0-1=elliptical, 1=parabolic, >1=hyperbolic)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital Elements")
    float Eccentricity = 0.0f;

    // Inclination (degrees)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital Elements")
    float Inclination = 0.0f;

    // Longitude of ascending node (degrees)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital Elements")
    float LongitudeAscendingNode = 0.0f;

    // Argument of periapsis (degrees)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital Elements")
    float ArgumentOfPeriapsis = 0.0f;

    // True anomaly (degrees)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital Elements")
    float TrueAnomaly = 0.0f;

    // Standard gravitational parameter (km³/s²)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital Elements")
    float StandardGravitationalParameter = 0.0f;
};

/**
 * Orbital state vectors (position and velocity)
 */
USTRUCT(BlueprintType)
struct FOrbitalStateVectors
{
    GENERATED_BODY()

    // Position in world space (km)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital State")
    FVector Position = FVector::ZeroVector;

    // Velocity in world space (km/s)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital State")
    FVector Velocity = FVector::ZeroVector;

    // Time at which these state vectors are valid (seconds since epoch)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital State")
    double EpochTime = 0.0;
};

/**
 * Orbital mechanics component for handling realistic space flight
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UOrbitalMechanics : public UActorComponent
{
    GENERATED_BODY()

public:    
    UOrbitalMechanics();

    //~ Begin UActorComponent interface
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    //~ End UActorComponent interface

    // Convert orbital elements to state vectors
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Conversions")
    static FOrbitalStateVectors ElementsToStateVectors(const FOrbitalElements& Elements);

    // Convert state vectors to orbital elements
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Conversions")
    static FOrbitalElements StateVectorsToElements(const FOrbitalStateVectors& StateVectors, float Mu);

    // Calculate orbital period in seconds
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Calculations")
    static float CalculateOrbitalPeriod(float SemiMajorAxis, float Mu);

    // Calculate orbital velocity at a given true anomaly
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Calculations")
    static FVector CalculateVelocityAtTrueAnomaly(const FOrbitalElements& Elements, float TrueAnomaly);

    // Calculate Hohmann transfer orbit between two circular orbits
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Maneuvers")
    static bool CalculateHohmannTransfer(float R1, float R2, float Mu, float& OutDeltaV1, float& OutDeltaV2, float& OutTransferTime);

    // Calculate sphere of influence radius
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Calculations")
    static float CalculateSphereOfInfluence(float SemiMajorAxis, float PrimaryMass, float SecondaryMass);

    // Check if a position is within a body's sphere of influence
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Checks")
    static bool IsInSphereOfInfluence(const FVector& Position, const FVector& BodyPosition, float SoIRadius);

    // Update orbital elements based on current state
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|State")
    void UpdateOrbitalElements();

    // Get current orbital elements
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|State")
    const FOrbitalElements& GetCurrentOrbitalElements() const { return CurrentOrbitalElements; }

    // Get current state vectors
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|State")
    const FOrbitalStateVectors& GetCurrentStateVectors() const { return CurrentStateVectors; }

    // Set the gravitational parameter for the current primary body
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Configuration")
    void SetGravitationalParameter(float Mu) { CurrentMu = Mu; }

    // Set the current primary body
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Configuration")
    void SetPrimaryBody(AActor* PrimaryBody);

protected:
    // Current orbital elements
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orbital State")
    FOrbitalElements CurrentOrbitalElements;

    // Current state vectors
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orbital State")
    FOrbitalStateVectors CurrentStateVectors;

    // Current primary body (optional, for reference)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orbital State")
    TWeakObjectPtr<AActor> PrimaryBody;

    // Standard gravitational parameter (km³/s²) of the primary body
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital Configuration")
    float CurrentMu = 3.986004418e5f; // Default: Earth (km³/s²)

    // Time warp factor (1.0 = realtime)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeWarpFactor = 1.0f;

    // Current simulation time (seconds since start)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time")
    double SimulationTime = 0.0;

    // Whether to use patched conics approximation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital Configuration")
    bool bUsePatchedConics = true;

    // Sphere of influence radius (km)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orbital State")
    float SphereOfInfluence = 0.0f;
};

// Constants
namespace OrbitalConstants
{
    // Gravitational constant (km³/kg/s²)
    static constexpr float G = 6.67430e-20f;
    
    // Astronomical unit in kilometers
    static constexpr float AU = 149597870.7f;
    
    // Standard gravitational parameters (km³/s²)
    namespace Mu
    {
        static constexpr float Sun = 1.32712440018e11f;
        static constexpr float Earth = 3.986004418e5f;
        static constexpr float Moon = 4.9048695e3f;
        static constexpr float Mars = 4.282837e4f;
        static constexpr float Jupiter = 1.26686534e8f;
    }
}
