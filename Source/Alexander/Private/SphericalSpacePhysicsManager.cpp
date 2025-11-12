// Copyright Epic Games, Inc. All Rights Reserved.

#include "SphericalSpacePhysicsManager.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

USphericalSpacePhysicsManager::USphericalSpacePhysicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    SpaceCenter = FVector::ZeroVector;
    bEnableDistanceScaling = true;
    bEnableSmoothTransitions = true;
    TransitionSpeed = 2.0f;
    PlayerBubbleCenter = nullptr;
}

void USphericalSpacePhysicsManager::BeginPlay()
{
    Super::BeginPlay();

    // Set default bubble config if not already set
    if (BubbleConfig.InnerRadius < KINDA_SMALL_NUMBER)
    {
        BubbleConfig = FBubbleInfluenceConfig();
    }

    UE_LOG(LogTemp, Log, TEXT("SphericalSpacePhysicsManager initialized with bubble inner radius: %.1f, outer radius: %.1f"),
        BubbleConfig.InnerRadius, BubbleConfig.OuterRadius);
}

void USphericalSpacePhysicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update all registered actors
    for (const auto& Pair : ActorDataMap)
    {
        AActor* Actor = Pair.Key;
        if (Actor && IsValid(Actor))
        {
            UpdateActorInfluence(Actor);
            UpdateOrbitalPosition(Actor, DeltaTime);
        }
    }

    // Update all actor scaling
    if (bEnableDistanceScaling)
    {
        UpdateAllActorScaling();
    }
}

void USphericalSpacePhysicsManager::RegisterActor(AActor* Actor, const FCelestialScalingData& ScalingData)
{
    if (!Actor || !IsValid(Actor))
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to register invalid actor"));
        return;
    }

    FSphericalSpaceData Data;
    Data.CelestialData = ScalingData;
    
    // Convert current position to spherical coordinates
    FVector WorldPos = Actor->GetActorLocation();
    Data.SphericalCoord = CartesianToSpherical(WorldPos - SpaceCenter);
    Data.LocalOffset = FVector::ZeroVector;

    ActorDataMap.Add(Actor, Data);
    UpdateActorInfluence(Actor);

    UE_LOG(LogTemp, Log, TEXT("Registered actor %s at spherical coordinates: Radius=%.1f, Polar=%.1f, Azimuth=%.1f"),
        *Actor->GetName(), Data.SphericalCoord.Radius, Data.SphericalCoord.PolarAngle, Data.SphericalCoord.AzimuthAngle);
}

void USphericalSpacePhysicsManager::UnregisterActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    ActorDataMap.Remove(Actor);
    OrbitalParameters.Remove(Actor);

    UE_LOG(LogTemp, Log, TEXT("Unregistered actor %s"), *Actor->GetName());
}

bool USphericalSpacePhysicsManager::IsActorRegistered(AActor* Actor) const
{
    return Actor && ActorDataMap.Contains(Actor);
}

void USphericalSpacePhysicsManager::SetActorSphericalPosition(AActor* Actor, const FSphericalCoordinate& SphericalCoord, const FVector& LocalOffset)
{
    if (!Actor || !ActorDataMap.Contains(Actor))
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to set spherical position for unregistered actor"));
        return;
    }

    FSphericalSpaceData& Data = ActorDataMap[Actor];
    Data.SphericalCoord = SphericalCoord;
    Data.LocalOffset = LocalOffset;

    // Update actor's world position
    FVector WorldPosition = SphericalToCartesian(SphericalCoord) + SpaceCenter + LocalOffset;
    Actor->SetActorLocation(WorldPosition);

    UpdateActorInfluence(Actor);
}

FSphericalCoordinate USphericalSpacePhysicsManager::GetActorSphericalPosition(AActor* Actor) const
{
    if (!Actor || !ActorDataMap.Contains(Actor))
    {
        return FSphericalCoordinate();
    }

    return ActorDataMap[Actor].SphericalCoord;
}

void USphericalSpacePhysicsManager::SetActorCartesianPosition(AActor* Actor, const FVector& WorldPosition)
{
    if (!Actor || !ActorDataMap.Contains(Actor))
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to set cartesian position for unregistered actor"));
        return;
    }

    FSphericalSpaceData& Data = ActorDataMap[Actor];
    Data.SphericalCoord = CartesianToSpherical(WorldPosition - SpaceCenter);
    Data.LocalOffset = FVector::ZeroVector;

    Actor->SetActorLocation(WorldPosition);
    UpdateActorInfluence(Actor);
}

FVector USphericalSpacePhysicsManager::GetActorCartesianPosition(AActor* Actor) const
{
    if (!Actor)
    {
        return FVector::ZeroVector;
    }

    return Actor->GetActorLocation();
}

void USphericalSpacePhysicsManager::SetPlayerBubbleCenter(AActor* PlayerActor)
{
    if (!PlayerActor || !IsValid(PlayerActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to set invalid player bubble center"));
        return;
    }

    PlayerBubbleCenter = PlayerActor;
    UE_LOG(LogTemp, Log, TEXT("Player bubble center set to actor %s"), *PlayerActor->GetName());
}

float USphericalSpacePhysicsManager::CalculateDistanceToPlayer(AActor* Actor) const
{
    if (!Actor || !PlayerBubbleCenter)
    {
        return -1.0f;
    }

    FVector ActorPos = Actor->GetActorLocation();
    FVector PlayerPos = PlayerBubbleCenter->GetActorLocation();

    return FVector::Dist(ActorPos, PlayerPos);
}

float USphericalSpacePhysicsManager::CalculateInfluenceLevel(float Distance) const
{
    if (Distance < 0.0f)
    {
        return 0.0f;
    }

    if (Distance <= BubbleConfig.InnerRadius)
    {
        return 1.0f; // Full influence
    }

    if (Distance >= BubbleConfig.OuterRadius)
    {
        return 0.0f; // No influence
    }

    // Calculate influence based on distance using scaling exponent
    float NormalizedDistance = (Distance - BubbleConfig.InnerRadius) / (BubbleConfig.OuterRadius - BubbleConfig.InnerRadius);
    float Influence = 1.0f - FMath::Pow(NormalizedDistance, BubbleConfig.ScalingExponent);
    
    return FMath::Clamp(Influence, BubbleConfig.MinScaleFactor, 1.0f);
}

float USphericalSpacePhysicsManager::CalculateCelestialScaleFactor(const FCelestialScalingData& Data, float DistanceKm) const
{
    if (DistanceKm < KINDA_SMALL_NUMBER)
    {
        return Data.CurrentScaleFactor;
    }

    // Calculate apparent size based on true size and distance
    float ApparentSize = (Data.TrueRadiusKm / DistanceKm) * Data.ReferenceDistanceKm;
    float ScaleFactor = ApparentSize / Data.ApparentRadius;

    // Clamp to reasonable bounds
    return FMath::Clamp(ScaleFactor, 0.001f, 1000.0f);
}

void USphericalSpacePhysicsManager::UpdateActorScaling(AActor* Actor)
{
    if (!Actor || !ActorDataMap.Contains(Actor))
    {
        return;
    }

    FSphericalSpaceData& Data = ActorDataMap[Actor];
    float OldInfluence = Data.InfluenceLevel;

    // Calculate distance to player
    float Distance = CalculateDistanceToPlayer(Actor);
    Data.InfluenceLevel = CalculateInfluenceLevel(Distance);

    // Update celestial scaling if applicable
    if (Data.CelestialData.TrueRadiusKm > 0.0f)
    {
        float DistanceKm = Distance / 100000.0f; // Convert to km (assuming 1UU = 1cm)
        Data.CelestialData.CurrentScaleFactor = CalculateCelestialScaleFactor(Data.CelestialData, DistanceKm);
    }

    // Apply scaling to actor
    if (bEnableDistanceScaling)
    {
        float TargetScale = Data.InfluenceLevel;
        if (Data.CelestialData.CurrentScaleFactor > 0.0f)
        {
            TargetScale *= Data.CelestialData.CurrentScaleFactor;
        }

        FVector CurrentScale = Actor->GetActorScale3D();
        FVector TargetScaleVector(TargetScale, TargetScale, TargetScale);

        if (bEnableSmoothTransitions)
        {
            // Smooth transition
            FVector NewScale = FMath::VInterpConstantTo(CurrentScale, TargetScaleVector, GetWorld()->DeltaTimeSeconds, TransitionSpeed);
            Actor->SetActorScale3D(NewScale);
        }
        else
        {
            // Instant transition
            Actor->SetActorScale3D(TargetScaleVector);
        }
    }

    // Handle bubble transition events
    if (OldInfluence <= 0.0f && Data.InfluenceLevel > 0.0f)
    {
        OnActorEnteredBubble.Broadcast(Actor, Distance);
    }
    else if (OldInfluence > 0.0f && Data.InfluenceLevel <= 0.0f)
    {
        OnActorLeftBubble.Broadcast(Actor);
    }

    // Broadcast influence change
    if (!FMath::IsNearlyEqual(OldInfluence, Data.InfluenceLevel, 0.01f))
    {
        OnInfluenceLevelChanged.Broadcast(Actor, Data.InfluenceLevel);
    }
}

void USphericalSpacePhysicsManager::UpdateAllActorScaling()
{
    for (const auto& Pair : ActorDataMap)
    {
        AActor* Actor = Pair.Key;
        if (Actor && IsValid(Actor))
        {
            UpdateActorScaling(Actor);
        }
    }
}

void USphericalSpacePhysicsManager::SetOrbitalParameters(AActor* Actor, AActor* CenterActor, float OrbitalRadiusKm, float OrbitalPeriodHours)
{
    if (!Actor || !CenterActor || !ActorDataMap.Contains(Actor))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid parameters for orbital setup"));
        return;
    }

    float AngularSpeed = CalculateAngularSpeed(OrbitalRadiusKm, OrbitalPeriodHours);
    OrbitalParameters.Add(Actor, FOrbitalParameterData{CenterActor, AngularSpeed});

    UE_LOG(LogTemp, Log, TEXT("Set orbital parameters for %s around %s: Radius=%.1fkm, Period=%.1fhours"),
        *Actor->GetName(), *CenterActor->GetName(), OrbitalRadiusKm, OrbitalPeriodHours);
}

void USphericalSpacePhysicsManager::UpdateOrbitalPosition(AActor* Actor, float DeltaTime)
{
    if (!Actor || !OrbitalParameters.Contains(Actor))
    {
        return;
    }

    const auto& OrbitalData = OrbitalParameters[Actor];
    AActor* CenterActor = OrbitalData.CenterActor;
    float AngularSpeed = OrbitalData.AngularSpeed;

    if (!CenterActor || !IsValid(CenterActor))
    {
        return;
    }

    // Update spherical coordinates based on orbital motion
    if (ActorDataMap.Contains(Actor))
    {
        FSphericalSpaceData& Data = ActorDataMap[Actor];
        
        // Update azimuth angle for orbital motion
        Data.SphericalCoord.AzimuthAngle += AngularSpeed * DeltaTime;
        
        // Normalize angle
        while (Data.SphericalCoord.AzimuthAngle >= 360.0f)
        {
            Data.SphericalCoord.AzimuthAngle -= 360.0f;
        }
        while (Data.SphericalCoord.AzimuthAngle < 0.0f)
        {
            Data.SphericalCoord.AzimuthAngle += 360.0f;
        }

        // Update actor position
        FVector WorldPosition = SphericalToCartesian(Data.SphericalCoord) + SpaceCenter + Data.LocalOffset;
        Actor->SetActorLocation(WorldPosition);
    }
}

FVector USphericalSpacePhysicsManager::GetPointOnSphericalPlane(float Radius, float PolarAngle, float AzimuthAngle) const
{
    FSphericalCoordinate Spherical(Radius, PolarAngle, AzimuthAngle);
    return SphericalToCartesian(Spherical) + SpaceCenter;
}

void USphericalSpacePhysicsManager::GetSphericalPlaneBounds(float Radius, float& MinRadius, float& MaxRadius) const
{
    // For a spherical plane, bounds are based on the sphere's radius
    MinRadius = Radius * 0.9f;  // 10% variation allowed
    MaxRadius = Radius * 1.1f;
}

FSphericalSpaceData USphericalSpacePhysicsManager::SerializeActorData(AActor* Actor) const
{
    if (!Actor || !ActorDataMap.Contains(Actor))
    {
        return FSphericalSpaceData();
    }

    return ActorDataMap[Actor];
}

void USphericalSpacePhysicsManager::DeserializeActorData(AActor* Actor, const FSphericalSpaceData& Data)
{
    if (!Actor)
    {
        return;
    }

    ActorDataMap.Add(Actor, Data);
    
    // Restore position
    FVector WorldPosition = SphericalToCartesian(Data.SphericalCoord) + SpaceCenter + Data.LocalOffset;
    Actor->SetActorLocation(WorldPosition);
    
    // Restore scale
    float TargetScale = Data.InfluenceLevel;
    if (Data.CelestialData.CurrentScaleFactor > 0.0f)
    {
        TargetScale *= Data.CelestialData.CurrentScaleFactor;
    }
    Actor->SetActorScale3D(FVector(TargetScale, TargetScale, TargetScale));

    UE_LOG(LogTemp, Log, TEXT("Deserialized actor %s data"), *Actor->GetName());
}

void USphericalSpacePhysicsManager::UpdateActorInfluence(AActor* Actor)
{
    UpdateActorScaling(Actor);
}

FVector USphericalSpacePhysicsManager::SphericalToCartesian(const FSphericalCoordinate& Spherical) const
{
    return Spherical.ToCartesian();
}

FSphericalCoordinate USphericalSpacePhysicsManager::CartesianToSpherical(const FVector& Cartesian) const
{
    return FSphericalCoordinate::FromCartesian(Cartesian);
}

float USphericalSpacePhysicsManager::CalculateAngularSpeed(float OrbitalRadiusKm, float OrbitalPeriodHours) const
{
    if (OrbitalPeriodHours <= 0.0f)
    {
        return 0.0f;
    }

    // Angular speed in degrees per second
    // 360 degrees / (orbital period in hours * 3600 seconds per hour)
    return 360.0f / (OrbitalPeriodHours * 3600.0f);
}

void USphericalSpacePhysicsManager::UpdateCelestialScaling(AActor* Actor, FSphericalSpaceData& Data)
{
    if (!PlayerBubbleCenter)
    {
        return;
    }

    float Distance = CalculateDistanceToPlayer(Actor);
    float DistanceKm = Distance / 100000.0f; // Convert to km

    Data.CelestialData.CurrentScaleFactor = CalculateCelestialScaleFactor(Data.CelestialData, DistanceKm);
}