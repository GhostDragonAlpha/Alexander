// Copyright Epic Games, Inc. All Rights Reserved.

#include "VrBubbleComponent.h"
#include "OrbitalBody.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Math/UnrealMathUtility.h"

UVrBubbleComponent::UVrBubbleComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize default values
    BubbleCenter = FVector::ZeroVector;
    CurrentBubbleRadius = BubbleConfig.BaseBubbleRadius;
    bEnableDebugVisualization = false;
    bShowBubbleBoundary = true;
    LastProjectionTime = 0.0f;
    ProjectionCount = 0;
}

void UVrBubbleComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize bubble center to owner location
    if (AActor* Owner = GetOwner())
    {
        BubbleCenter = Owner->GetActorLocation();
        RecalculateBubbleRadius();
    }

    UE_LOG(LogTemp, Log, TEXT("VR Bubble Component initialized with radius: %.2f cm"), CurrentBubbleRadius);
}

void UVrBubbleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update bubble center to follow owner
    if (AActor* Owner = GetOwner())
    {
        BubbleCenter = Owner->GetActorLocation();
    }

    // Update projections for all registered bodies
    UpdateAllProjections();

    // Draw debug visualization if enabled
    if (bEnableDebugVisualization)
    {
        DrawDebugInfo();
    }
}

FBubbleProjectionResult UVrBubbleComponent::ProjectPosition(const FVector& RealWorldPositionKm) const
{
    FBubbleProjectionResult Result;
    Result.RealPosition = RealWorldPositionKm;
    
    // Convert real position from km to cm for calculation
    FVector RealPositionCm = RealWorldPositionKm * 100000.0f; // 1 km = 100,000 cm
    
    // Calculate direction and distance from bubble center
    FVector Direction = RealPositionCm - BubbleCenter;
    float RealDistanceCm = Direction.Size();
    float RealDistanceKm = RealDistanceCm / 100000.0f;
    
    Result.DistanceFromCenter = RealDistanceCm;
    Result.bIsInsideBubble = RealDistanceCm <= CurrentBubbleRadius;
    
    if (Result.bIsInsideBubble)
    {
        // If already inside bubble, no projection needed
        Result.ProjectedPosition = RealPositionCm;
        Result.ScaleFactor = 1.0f;
    }
    else
    {
        // Apply inverse square projection
        Result.ProjectedPosition = BubbleCenter + ApplyInverseSquareProjection(Direction.GetSafeNormal(), RealDistanceKm);
        Result.ScaleFactor = CalculateScaleFactor(RealDistanceKm);
    }
    
    return Result;
}

float UVrBubbleComponent::ProjectDistance(float RealDistanceKm) const
{
    if (RealDistanceKm <= 0.0f)
    {
        return 0.0f;
    }
    
    // Apply inverse square projection formula
    // ProjectedDistance = BubbleRadius × (1 - (1 / (1 + RealDistance² × Factor)))
    float BubbleRadiusKm = CurrentBubbleRadius / 100000.0f; // Convert to km
    float ProjectedDistanceKm = BubbleRadiusKm * (1.0f - (1.0f / (1.0f + RealDistanceKm * RealDistanceKm * BubbleConfig.InverseSquareFactor)));
    
    // Convert back to cm
    return ProjectedDistanceKm * 100000.0f;
}

FVector UVrBubbleComponent::UnprojectPosition(const FVector& BubblePositionCm) const
{
    // Calculate direction from bubble center
    FVector Direction = BubblePositionCm - BubbleCenter;
    float ProjectedDistanceCm = Direction.Size();
    
    if (ProjectedDistanceCm <= CurrentBubbleRadius)
    {
        // If inside bubble, assume it's at real scale
        return BubblePositionCm / 100000.0f; // Convert to km
    }
    
    // For projected positions, we need to solve the inverse square equation
    // This is an approximation as the projection is not perfectly reversible
    float ProjectedDistanceKm = ProjectedDistanceCm / 100000.0f;
    float BubbleRadiusKm = CurrentBubbleRadius / 100000.0f;
    
    // Solve for RealDistanceKm: Projected = BubbleRadius × (1 - (1 / (1 + RealDistance²)))
    // Rearranged: RealDistance² = (1 / (1 - Projected/BubbleRadius)) - 1
    float Ratio = ProjectedDistanceKm / BubbleRadiusKm;
    if (Ratio >= 1.0f)
    {
        return FVector::ZeroVector; // Invalid projection
    }
    
    float RealDistanceKm = FMath::Sqrt((1.0f / (1.0f - Ratio)) - 1.0f);
    FVector RealPositionCm = BubbleCenter + Direction.GetSafeNormal() * (RealDistanceKm * 100000.0f);
    
    return RealPositionCm / 100000.0f; // Convert to km
}

float UVrBubbleComponent::CalculateScaleFactor(float RealDistanceKm) const
{
    if (RealDistanceKm <= 0.0f)
    {
        return 1.0f;
    }
    
    float ProjectedDistanceCm = ProjectDistance(RealDistanceKm);
    float RealDistanceCm = RealDistanceKm * 100000.0f;
    
    return ProjectedDistanceCm / RealDistanceCm;
}

bool UVrBubbleComponent::IsWithinProjectionRange(float RealDistanceKm) const
{
    return RealDistanceKm <= BubbleConfig.MaxProjectedDistance;
}

void UVrBubbleComponent::UpdateBubbleCenter(const FVector& NewCenter)
{
    BubbleCenter = NewCenter;
}

void UVrBubbleComponent::RecalculateBubbleRadius()
{
    if (!BubbleConfig.bEnableDynamicSizing)
    {
        CurrentBubbleRadius = BubbleConfig.BaseBubbleRadius;
        return;
    }
    
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        CurrentBubbleRadius = BubbleConfig.BaseBubbleRadius;
        return;
    }
    
    // Calculate based on owner bounds
    float MaxDimension = 0.0f;
    
    // Check all components for their bounds
    TArray<UActorComponent*> Components = Owner->GetComponents().Array();
    for (UActorComponent* Component : Components)
    {
        if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
        {
            FBoxSphereBounds Bounds = Primitive->CalcBounds(Primitive->GetComponentTransform());
            float ComponentMaxDim = FMath::Max3(Bounds.BoxExtent.X, Bounds.BoxExtent.Y, Bounds.BoxExtent.Z) * 2.0f;
            MaxDimension = FMath::Max(MaxDimension, ComponentMaxDim);
        }
    }
    
    // If no primitive components found, use default
    if (MaxDimension <= 0.0f)
    {
        MaxDimension = 500.0f; // 5m default
    }
    
    // Calculate dynamic radius
    float DynamicRadius = MaxDimension * BubbleConfig.DynamicSizeMultiplier;
    CurrentBubbleRadius = FMath::Clamp(DynamicRadius, BubbleConfig.MinDynamicRadius, BubbleConfig.MaxDynamicRadius);
    
    UE_LOG(LogTemp, Verbose, TEXT("VR Bubble radius recalculated: %.2f cm (based on max dimension: %.2f cm)"), 
           CurrentBubbleRadius, MaxDimension);
}

void UVrBubbleComponent::SetBubbleConfig(const FBubbleProjectionConfig& NewConfig)
{
    BubbleConfig = NewConfig;
    RecalculateBubbleRadius();
}

void UVrBubbleComponent::RegisterOrbitalBody(AOrbitalBody* Body)
{
    if (Body && !RegisteredBodies.Contains(Body))
    {
        RegisteredBodies.Add(Body);
        UE_LOG(LogTemp, Verbose, TEXT("Registered orbital body %s with VR Bubble"), *Body->GetName());
    }
}

void UVrBubbleComponent::UnregisterOrbitalBody(AOrbitalBody* Body)
{
    if (Body)
    {
        RegisteredBodies.Remove(Body);
        UE_LOG(LogTemp, Verbose, TEXT("Unregistered orbital body %s from VR Bubble"), *Body->GetName());
    }
}

void UVrBubbleComponent::UpdateAllProjections()
{
    float StartTime = FPlatformTime::Seconds();
    
    for (TWeakObjectPtr<AOrbitalBody> BodyPtr : RegisteredBodies)
    {
        if (AOrbitalBody* Body = BodyPtr.Get())
        {
            UpdateBodyProjection(Body);
        }
    }
    
    float EndTime = FPlatformTime::Seconds();
    UpdatePerformanceStats(EndTime - StartTime);
}

void UVrBubbleComponent::UpdateBodyProjection(AOrbitalBody* Body)
{
    if (!Body || !GetWorld())
    {
        return;
    }
    
    // Get the body's real position (assuming it's stored in km)
    FVector RealPositionKm = Body->GetActorLocation() / 100000.0f; // Convert cm to km
    
    // Project the position
    FBubbleProjectionResult Projection = ProjectPosition(RealPositionKm);
    
    // Update the body's actor location to the projected position
    Body->SetActorLocation(Projection.ProjectedPosition, false, nullptr, ETeleportType::TeleportPhysics);
    
    // Store the real position for physics calculations
    // This would be stored in a custom property on the orbital body
}

FVector UVrBubbleComponent::CalculateProjectedGravitationalForce(AOrbitalBody* Attractor, AOrbitalBody* Satellite) const
{
    if (!Attractor || !Satellite)
    {
        return FVector::ZeroVector;
    }
    
    // Get real positions in km
    FVector AttractorPosKm = Attractor->GetActorLocation() / 100000.0f;
    FVector SatellitePosKm = Satellite->GetActorLocation() / 100000.0f;
    
    // Calculate real distance
    FVector Direction = AttractorPosKm - SatellitePosKm;
    float RealDistanceKm = Direction.Size();
    
    if (RealDistanceKm < KINDA_SMALL_NUMBER)
    {
        return FVector::ZeroVector;
    }
    
    // Normalize direction
    Direction.Normalize();
    
    // Calculate gravitational force using real distances (not projected)
    const float GravitationalConstant = 6.67430e-11f; // m³/(kg·s²)
    float ForceMagnitude = GravitationalConstant * (Attractor->Mass * Satellite->Mass) / (RealDistanceKm * 1000.0f * RealDistanceKm * 1000.0f);
    
    // Convert to Unreal force units and apply direction
    return Direction * (ForceMagnitude * 0.001f); // Convert N to kN
}

float UVrBubbleComponent::CalculateProjectedOrbitalVelocity(float CentralMassKg, float ProjectedDistanceCm) const
{
    if (ProjectedDistanceCm <= 0.0f)
    {
        return 0.0f;
    }
    
    // Convert projected distance back to real distance for accurate physics
    float ProjectedDistanceKm = ProjectedDistanceCm / 100000.0f;
    float RealDistanceKm = UnprojectPosition(FVector(ProjectedDistanceCm, 0.0f, 0.0f)).X;
    
    if (RealDistanceKm <= 0.0f)
    {
        return 0.0f;
    }
    
    // Calculate orbital velocity using real distance
    const float GravitationalConstant = 6.67430e-11f;
    float RealDistanceM = RealDistanceKm * 1000.0f;
    float VelocityMS = FMath::Sqrt(GravitationalConstant * CentralMassKg / RealDistanceM);
    
    // Convert to km/s
    return VelocityMS * 0.001f;
}

void UVrBubbleComponent::UpdateBodyPhysics(AOrbitalBody* Body, float DeltaTime)
{
    if (!Body || !Body->OrbitTarget.IsValid())
    {
        return;
    }
    
    // Calculate gravitational force using real distances
    FVector GravitationalForce = CalculateProjectedGravitationalForce(Body->OrbitTarget.Get(), Body);
    
    // Apply force to body
    if (UPrimitiveComponent* PhysicsComponent = Cast<UPrimitiveComponent>(Body->GetRootComponent()))
    {
        PhysicsComponent->AddForce(GravitationalForce, NAME_None, true);
    }
}

TArray<AOrbitalBody*> UVrBubbleComponent::GetRegisteredBodies() const
{
    TArray<AOrbitalBody*> Result;
    for (TWeakObjectPtr<AOrbitalBody> BodyPtr : RegisteredBodies)
    {
        if (AOrbitalBody* Body = BodyPtr.Get())
        {
            Result.Add(Body);
        }
    }
    return Result;
}

float UVrBubbleComponent::CalculateDynamicBubbleRadius() const
{
    AActor* Owner = GetOwner();
    if (!Owner || !BubbleConfig.bEnableDynamicSizing)
    {
        return BubbleConfig.BaseBubbleRadius;
    }
    
    float MaxDimension = 0.0f;
    
    // Check all components for their bounds
    TArray<UActorComponent*> Components = Owner->GetComponents().Array();
    for (UActorComponent* Component : Components)
    {
        if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
        {
            FBoxSphereBounds Bounds = Primitive->CalcBounds(Primitive->GetComponentTransform());
            float ComponentMaxDim = FMath::Max3(Bounds.BoxExtent.X, Bounds.BoxExtent.Y, Bounds.BoxExtent.Z) * 2.0f;
            MaxDimension = FMath::Max(MaxDimension, ComponentMaxDim);
        }
    }
    
    // If no primitive components found, use default
    if (MaxDimension <= 0.0f)
    {
        MaxDimension = 500.0f; // 5m default
    }
    
    // Calculate dynamic radius
    float DynamicRadius = MaxDimension * BubbleConfig.DynamicSizeMultiplier;
    return FMath::Clamp(DynamicRadius, BubbleConfig.MinDynamicRadius, BubbleConfig.MaxDynamicRadius);
}

FVector UVrBubbleComponent::ApplyInverseSquareProjection(const FVector& Direction, float RealDistanceKm) const
{
    float ProjectedDistanceCm = ProjectDistance(RealDistanceKm);
    return Direction * ProjectedDistanceCm;
}

void UVrBubbleComponent::UpdatePerformanceStats(float ProjectionTime)
{
    LastProjectionTime = ProjectionTime;
    ProjectionCount++;
}

void UVrBubbleComponent::DrawDebugInfo() const
{
    if (!GetWorld())
    {
        return;
    }
    
    // Draw bubble boundary
    if (bShowBubbleBoundary)
    {
        DrawDebugSphere(GetWorld(), BubbleCenter, CurrentBubbleRadius, 32, FColor::Cyan, false, -1.0f, 0, 2.0f);
    }
    
    // Draw center point
    DrawDebugPoint(GetWorld(), BubbleCenter, 10.0f, FColor::Red, false, -1.0f, 0);
    
    // Draw registered bodies
    for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : RegisteredBodies)
    {
        if (const AOrbitalBody* Body = BodyPtr.Get())
        {
            FVector BodyLocation = Body->GetActorLocation();
            DrawDebugLine(GetWorld(), BubbleCenter, BodyLocation, FColor::Green, false, -1.0f, 0, 1.0f);
            DrawDebugSphere(GetWorld(), BodyLocation, 100.0f, 16, FColor::Yellow, false, -1.0f, 0, 1.0f);
        }
    }
}

FString UVrBubbleComponent::GetDebugInfo() const
{
    return FString::Printf(
        TEXT("VR Bubble Debug Info:\n")
        TEXT("  Center: %s\n")
        TEXT("  Radius: %.2f cm (%.2f m)\n")
        TEXT("  Registered Bodies: %d\n")
        TEXT("  Last Projection Time: %.4f ms\n")
        TEXT("  Total Projections: %d"),
        *BubbleCenter.ToString(),
        CurrentBubbleRadius,
        CurrentBubbleRadius / 100.0f,
        RegisteredBodies.Num(),
        LastProjectionTime * 1000.0f,
        ProjectionCount
    );
}