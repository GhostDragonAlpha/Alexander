// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SphericalSpacePhysicsManager.generated.h"

// Forward declarations
class AActor;
class USceneComponent;

/**
 * Spherical coordinate representation
 */
USTRUCT(BlueprintType)
struct FSphericalCoordinate
{
    GENERATED_BODY()

    // Radius from center (in Unreal units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spherical Physics")
    float Radius;

    // Polar angle (0 to 180 degrees, 0 = north pole)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spherical Physics")
    float PolarAngle;

    // Azimuth angle (0 to 360 degrees)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spherical Physics")
    float AzimuthAngle;

    FSphericalCoordinate() : Radius(0.0f), PolarAngle(0.0f), AzimuthAngle(0.0f) {}
    FSphericalCoordinate(float InRadius, float InPolar, float InAzimuth)
        : Radius(InRadius), PolarAngle(InPolar), AzimuthAngle(InAzimuth) {}

    // Convert to Cartesian coordinates
    FVector ToCartesian() const;
    
    // Convert from Cartesian coordinates
    static FSphericalCoordinate FromCartesian(const FVector& Cartesian);
};

/**
 * Bubble of influence configuration
 */
USTRUCT(BlueprintType)
struct FBubbleInfluenceConfig
{
    GENERATED_BODY()

    // Inner radius where objects are at full scale
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble Influence")
    float InnerRadius;

    // Outer radius where objects start scaling down
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble Influence")
    float OuterRadius;

    // Maximum scale reduction factor (0.0 = invisible, 1.0 = no scaling)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble Influence", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinScaleFactor;

    // Scaling curve (0.0 = linear, 1.0 = exponential)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble Influence", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float ScalingExponent;

    FBubbleInfluenceConfig()
        : InnerRadius(10000.0f)    // 100m inner radius
        , OuterRadius(1000000.0f)  // 10km outer radius
        , MinScaleFactor(0.1f)     // Scale down to 10% at max distance
        , ScalingExponent(2.0f)    // Quadratic scaling
    {}
};

/**
 * Celestial object scaling data
 */
USTRUCT(BlueprintType)
struct FCelestialScalingData
{
    GENERATED_BODY()

    // True physical radius of the object (in km)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Scaling")
    float TrueRadiusKm;

    // Apparent radius at reference distance (in Unreal units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Scaling")
    float ApparentRadius;

    // Reference distance for apparent size (in km)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Scaling")
    float ReferenceDistanceKm;

    // Current visual scale factor
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Celestial Scaling")
    float CurrentScaleFactor;

    FCelestialScalingData()
        : TrueRadiusKm(6371.0f)    // Earth radius by default
        , ApparentRadius(1000.0f)  // 10m apparent radius
        , ReferenceDistanceKm(10000.0f) // 10,000km reference
        , CurrentScaleFactor(1.0f)
    {}
};

/**
 * Spherical space physics data for an actor
 */
USTRUCT(BlueprintType)
struct FSphericalSpaceData
{
    GENERATED_BODY()

    // Spherical coordinates relative to center
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spherical Physics")
    FSphericalCoordinate SphericalCoord;

    // Local Cartesian offset within spherical sector
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spherical Physics")
    FVector LocalOffset;

    // Current bubble influence level (0.0 = outside bubble, 1.0 = full influence)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spherical Physics")
    float InfluenceLevel;

    // Celestial scaling data (for planets, stars, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spherical Physics")
    FCelestialScalingData CelestialData;

    FSphericalSpaceData()
        : SphericalCoord(0.0f, 0.0f, 0.0f)
        , LocalOffset(FVector::ZeroVector)
        , InfluenceLevel(1.0f)
    {}
};

/**
 * Spherical Space Physics Manager
 * Manages objects in spherical coordinate space with distance-based scaling
 * and bubble-of-influence systems for seamless space navigation
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API USphericalSpacePhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    USphericalSpacePhysicsManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Actor Registration
    UFUNCTION(BlueprintCallable, Category = "Spherical Physics")
    void RegisterActor(AActor* Actor, const FCelestialScalingData& ScalingData);

    UFUNCTION(BlueprintCallable, Category = "Spherical Physics")
    void UnregisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Spherical Physics")
    bool IsActorRegistered(AActor* Actor) const;

    // Position Management - Spherical
    UFUNCTION(BlueprintCallable, Category = "Spherical Physics")
    void SetActorSphericalPosition(AActor* Actor, const FSphericalCoordinate& SphericalCoord, const FVector& LocalOffset);

    UFUNCTION(BlueprintPure, Category = "Spherical Physics")
    FSphericalCoordinate GetActorSphericalPosition(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Spherical Physics")
    void SetActorCartesianPosition(AActor* Actor, const FVector& WorldPosition);

    UFUNCTION(BlueprintPure, Category = "Spherical Physics")
    FVector GetActorCartesianPosition(AActor* Actor) const;

    // Bubble Influence Management
    UFUNCTION(BlueprintCallable, Category = "Bubble Influence")
    void SetPlayerBubbleCenter(AActor* PlayerActor);

    UFUNCTION(BlueprintPure, Category = "Bubble Influence")
    AActor* GetPlayerBubbleCenter() const { return PlayerBubbleCenter; }

    UFUNCTION(BlueprintCallable, Category = "Bubble Influence")
    void SetBubbleConfig(const FBubbleInfluenceConfig& Config) { BubbleConfig = Config; }

    UFUNCTION(BlueprintPure, Category = "Bubble Influence")
    FBubbleInfluenceConfig GetBubbleConfig() const { return BubbleConfig; }

    // Distance and Scaling Calculations
    UFUNCTION(BlueprintPure, Category = "Scaling")
    float CalculateDistanceToPlayer(AActor* Actor) const;

    UFUNCTION(BlueprintPure, Category = "Scaling")
    float CalculateInfluenceLevel(float Distance) const;

    UFUNCTION(BlueprintPure, Category = "Scaling")
    float CalculateCelestialScaleFactor(const FCelestialScalingData& Data, float DistanceKm) const;

    UFUNCTION(BlueprintCallable, Category = "Scaling")
    void UpdateActorScaling(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Scaling")
    void UpdateAllActorScaling();

    // Spherical Plane Management
    UFUNCTION(BlueprintPure, Category = "Spherical Plane")
    FVector GetPointOnSphericalPlane(float Radius, float PolarAngle, float AzimuthAngle) const;

    UFUNCTION(BlueprintPure, Category = "Spherical Plane")
    void GetSphericalPlaneBounds(float Radius, float& MinRadius, float& MaxRadius) const;

    // Navigation and Orbits
    UFUNCTION(BlueprintCallable, Category = "Orbits")
    void SetOrbitalParameters(AActor* Actor, AActor* CenterActor, float OrbitalRadiusKm, float OrbitalPeriodHours);

    UFUNCTION(BlueprintCallable, Category = "Orbits")
    void UpdateOrbitalPosition(AActor* Actor, float DeltaTime);

    // Serialization
    UFUNCTION(BlueprintCallable, Category = "Serialization")
    FSphericalSpaceData SerializeActorData(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Serialization")
    void DeserializeActorData(AActor* Actor, const FSphericalSpaceData& Data);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInfluenceLevelChanged, AActor*, Actor, float, NewInfluenceLevel);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActorEnteredBubble, AActor*, Actor, float, Distance);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorLeftBubble, AActor*, Actor);

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnInfluenceLevelChanged OnInfluenceLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnActorEnteredBubble OnActorEnteredBubble;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnActorLeftBubble OnActorLeftBubble;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FBubbleInfluenceConfig BubbleConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FVector SpaceCenter; // Center of the spherical coordinate system

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableDistanceScaling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableSmoothTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float TransitionSpeed; // Speed of scale transitions

private:
    // Actor data storage
    UPROPERTY()
    TMap<AActor*, FSphericalSpaceData> ActorDataMap;

    // Player bubble center
    UPROPERTY()
    AActor* PlayerBubbleCenter;

    // Orbital tracking
    UPROPERTY()
    TMap<AActor*, TPair<AActor*, float>> OrbitalParameters; // Actor -> (CenterActor, AngularSpeed)

    // Helper functions
    void UpdateActorInfluence(AActor* Actor);
    void UpdateCelestialScaling(AActor* Actor, FSphericalSpaceData& Data);
    void HandleBubbleTransition(AActor* Actor, float OldInfluence, float NewInfluence);
    FVector SphericalToCartesian(const FSphericalCoordinate& Spherical) const;
    FSphericalCoordinate CartesianToSpherical(const FVector& Cartesian) const;
    float CalculateAngularSpeed(float OrbitalRadiusKm, float OrbitalPeriodHours) const;
};

// Inline implementations
inline FVector FSphericalCoordinate::ToCartesian() const
{
    float RadPolar = FMath::DegreesToRadians(PolarAngle);
    float RadAzimuth = FMath::DegreesToRadians(AzimuthAngle);
    
    float SinPolar = FMath::Sin(RadPolar);
    float CosPolar = FMath::Cos(RadPolar);
    float SinAzimuth = FMath::Sin(RadAzimuth);
    float CosAzimuth = FMath::Cos(RadAzimuth);
    
    return FVector(
        Radius * SinPolar * CosAzimuth,
        Radius * SinPolar * SinAzimuth,
        Radius * CosPolar
    );
}

inline FSphericalCoordinate FSphericalCoordinate::FromCartesian(const FVector& Cartesian)
{
    FSphericalCoordinate Spherical;
    Spherical.Radius = Cartesian.Size();
    
    if (Spherical.Radius > KINDA_SMALL_NUMBER)
    {
        Spherical.PolarAngle = FMath::RadiansToDegrees(FMath::Acos(Cartesian.Z / Spherical.Radius));
        Spherical.AzimuthAngle = FMath::RadiansToDegrees(FMath::Atan2(Cartesian.Y, Cartesian.X));
        
        // Normalize angles
        if (Spherical.AzimuthAngle < 0.0f)
        {
            Spherical.AzimuthAngle += 360.0f;
        }
    }
    else
    {
        Spherical.PolarAngle = 0.0f;
        Spherical.AzimuthAngle = 0.0f;
    }
    
    return Spherical;
}