// Copyright Epic Games, Inc. All Rights Reserved.

#include "LandingGuidanceSystem.h"
#include "Spaceship.h"
#include "Planet.h"
#include "FirstMissionGameMode.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

ALandingGuidanceSystem::ALandingGuidanceSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
    RootComponent = RootComp;

    // Create Niagara components
    LandingBeam = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LandingBeam"));
    LandingBeam->SetupAttachment(RootComp);
    LandingBeam->SetAutoActivate(false);

    LandingZoneIndicator = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LandingZoneIndicator"));
    LandingZoneIndicator->SetupAttachment(RootComp);
    LandingZoneIndicator->SetAutoActivate(false);

    // Configuration
    MaxGuidanceAltitude = 5000.0f;
    SafeLandingSpeed = 5.0f;
    MaxApproachAngle = 45.0f;
    TerrainScanRadius = 50.0f;
    MaxLandingSlope = 15.0f;
    UpdateFrequency = 10.0f;
    LandingAssistStrength = 0.5f;
    bAutoLandEnabled = false;

    // State
    bIsActive = false;
    bShowVisualGuidance = true;
    bShowHUDGuidance = true;
    TargetShip = nullptr;
    TargetPlanet = nullptr;

    // VFX Colors
    SafeColor = FLinearColor::Green;
    WarningColor = FLinearColor::Yellow;
    DangerColor = FLinearColor::Red;

    // Tracking
    LastUpdateTime = 0.0f;
    UpdateInterval = 1.0f / UpdateFrequency;
    PreviousStatus = ELandingStatus::TooFarFromSurface;
    bWasLandingApproved = false;
    bAutoLandActive = false;
    AutoLandTarget = FVector::ZeroVector;
}

void ALandingGuidanceSystem::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("LandingGuidanceSystem: Initialized"));
}

void ALandingGuidanceSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsActive || !TargetShip || !TargetPlanet)
    {
        return;
    }

    // Throttle updates
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime < UpdateInterval)
    {
        return;
    }
    LastUpdateTime = CurrentTime;

    // Update guidance
    UpdateGuidance(DeltaTime);

    // Apply landing assist if enabled
    if (LandingAssistStrength > 0.0f)
    {
        ApplyLandingAssist(DeltaTime);
    }

    // Execute auto-land if enabled
    if (bAutoLandEnabled && bAutoLandActive)
    {
        ExecuteAutoLand(DeltaTime);
    }
}

void ALandingGuidanceSystem::ActivateGuidance(ASpaceship* Ship, APlanet* Planet)
{
    if (!Ship || !Planet)
    {
        UE_LOG(LogTemp, Warning, TEXT("LandingGuidanceSystem: Invalid ship or planet"));
        return;
    }

    TargetShip = Ship;
    TargetPlanet = Planet;
    bIsActive = true;

    // Activate VFX
    if (bShowVisualGuidance)
    {
        if (LandingBeam)
        {
            LandingBeam->Activate(true);
        }
        if (LandingZoneIndicator)
        {
            LandingZoneIndicator->Activate(true);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("LandingGuidanceSystem: Activated for ship %s"), *Ship->GetName());
}

void ALandingGuidanceSystem::DeactivateGuidance()
{
    bIsActive = false;
    bAutoLandActive = false;

    // Deactivate VFX
    if (LandingBeam)
    {
        LandingBeam->Deactivate();
    }
    if (LandingZoneIndicator)
    {
        LandingZoneIndicator->Deactivate();
    }

    UE_LOG(LogTemp, Log, TEXT("LandingGuidanceSystem: Deactivated"));
}

void ALandingGuidanceSystem::SetTargetShip(ASpaceship* Ship)
{
    TargetShip = Ship;
}

void ALandingGuidanceSystem::SetTargetPlanet(APlanet* Planet)
{
    TargetPlanet = Planet;
}

void ALandingGuidanceSystem::SetVisualGuidance(bool bEnabled)
{
    bShowVisualGuidance = bEnabled;

    if (bEnabled && bIsActive)
    {
        if (LandingBeam)
        {
            LandingBeam->Activate(true);
        }
        if (LandingZoneIndicator)
        {
            LandingZoneIndicator->Activate(true);
        }
    }
    else
    {
        if (LandingBeam)
        {
            LandingBeam->Deactivate();
        }
        if (LandingZoneIndicator)
        {
            LandingZoneIndicator->Deactivate();
        }
    }
}

void ALandingGuidanceSystem::SetAutoLand(bool bEnabled)
{
    bAutoLandEnabled = bEnabled;
    
    if (bEnabled && IsSafeToLand())
    {
        bAutoLandActive = true;
        AutoLandTarget = CalculateLandingPoint();
        UE_LOG(LogTemp, Log, TEXT("LandingGuidanceSystem: Auto-land engaged to %s"), *AutoLandTarget.ToString());
    }
    else
    {
        bAutoLandActive = false;
    }
}

void ALandingGuidanceSystem::UpdateGuidance(float DeltaTime)
{
    if (!TargetShip || !TargetPlanet)
    {
        return;
    }

    // Calculate guidance data
    CalculateGuidanceData();

    // Update landing status
    UpdateLandingStatus();

    // Update visual guidance
    if (bShowVisualGuidance)
    {
        UpdateVisualGuidance();
    }
}

void ALandingGuidanceSystem::ApplyLandingAssist(float DeltaTime)
{
    if (!TargetShip || LandingAssistStrength <= 0.0f)
    {
        return;
    }

    // Only assist when close to landing
    if (GuidanceData.AltitudeAboveSurface > MaxGuidanceAltitude)
    {
        return;
    }

    // Calculate correction forces
    FVector ShipVelocity = TargetShip->GetVelocity();
    FVector DesiredVelocity = FVector::ZeroVector;

    // Vertical speed correction
    float DesiredVerticalSpeed = FMath::Clamp(
        -GuidanceData.AltitudeAboveSurface / 100.0f,
        -SafeLandingSpeed,
        0.0f
    );
    DesiredVelocity.Z = DesiredVerticalSpeed * 100.0f; // Convert to cm/s

    // Apply correction with assist strength
    FVector VelocityCorrection = (DesiredVelocity - ShipVelocity) * LandingAssistStrength * DeltaTime;
    
    // This would be applied through the ship's physics system
    // TargetShip->AddForce(VelocityCorrection * TargetShip->GetMass());
}

void ALandingGuidanceSystem::ExecuteAutoLand(float DeltaTime)
{
    if (!TargetShip || !bAutoLandActive)
    {
        return;
    }

    // Calculate approach vector
    FVector ApproachVector = CalculateApproachVector();
    FVector TargetPosition = AutoLandTarget + (ApproachVector * GuidanceData.AltitudeAboveSurface);

    // Move ship towards target
    FVector CurrentPosition = TargetShip->GetActorLocation();
    FVector ToTarget = TargetPosition - CurrentPosition;
    float Distance = ToTarget.Size();

    if (Distance < 100.0f && GuidanceData.AltitudeAboveSurface < 50.0f)
    {
        // Landing complete
        bAutoLandActive = false;
        
        // Notify mission system
        if (AFirstMissionGameMode* MissionMode = Cast<AFirstMissionGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
        {
            MissionMode->NotifyLandedOnMoon();
        }
        
        UE_LOG(LogTemp, Log, TEXT("LandingGuidanceSystem: Auto-land complete"));
    }
}

FVector ALandingGuidanceSystem::CalculateLandingPoint() const
{
    if (!TargetShip || !TargetPlanet)
    {
        return FVector::ZeroVector;
    }

    // Raycast from ship to surface
    FVector Start = TargetShip->GetActorLocation();
    FVector HitPoint;
    FVector Normal;

    if (RaycastToSurface(Start, HitPoint, Normal))
    {
        return HitPoint;
    }

    return FVector::ZeroVector;
}

FVector ALandingGuidanceSystem::CalculateApproachVector() const
{
    if (!TargetShip || !TargetPlanet)
    {
        return FVector::UpVector;
    }

    // Use surface normal as approach vector
    return GuidanceData.SurfaceNormal;
}

void ALandingGuidanceSystem::CalculateGuidanceData()
{
    if (!TargetShip || !TargetPlanet)
    {
        return;
    }

    // Get ship data
    FVector ShipLocation = TargetShip->GetActorLocation();
    FVector ShipVelocity = TargetShip->GetVelocity();
    
    // Raycast to surface
    FVector HitPoint;
    FVector SurfaceNormal;
    if (RaycastToSurface(ShipLocation, HitPoint, SurfaceNormal))
    {
        GuidanceData.LandingPoint = HitPoint;
        GuidanceData.SurfaceNormal = SurfaceNormal;
        GuidanceData.AltitudeAboveSurface = FVector::Dist(ShipLocation, HitPoint);
    }
    else
    {
        GuidanceData.AltitudeAboveSurface = 99999.0f;
    }

    // Calculate speeds (convert from cm/s to m/s)
    FVector LocalVelocity = TargetShip->GetActorRotation().UnrotateVector(ShipVelocity);
    GuidanceData.VerticalSpeed = -LocalVelocity.Z / 100.0f;
    GuidanceData.HorizontalSpeed = FVector(LocalVelocity.X, LocalVelocity.Y, 0.0f).Size() / 100.0f;

    // Calculate approach angle
    GuidanceData.ApproachAngle = CalculateApproachAngle();
}

void ALandingGuidanceSystem::UpdateLandingStatus()
{
    ELandingStatus NewStatus = ELandingStatus::ClearToLand;
    FString StatusMsg = "Clear to Land";
    bool bSafe = true;

    // Check altitude
    if (GuidanceData.AltitudeAboveSurface > MaxGuidanceAltitude)
    {
        NewStatus = ELandingStatus::TooFarFromSurface;
        StatusMsg = "Too far from surface";
        bSafe = false;
    }
    // Check speed
    else if (FMath::Abs(GuidanceData.VerticalSpeed) > SafeLandingSpeed * 2.0f)
    {
        NewStatus = ELandingStatus::SpeedTooHigh;
        StatusMsg = FString::Printf(TEXT("Speed too high: %.1f m/s"), FMath::Abs(GuidanceData.VerticalSpeed));
        bSafe = false;
    }
    // Check approach angle
    else if (GuidanceData.ApproachAngle > MaxApproachAngle)
    {
        NewStatus = ELandingStatus::ApproachTooSteep;
        StatusMsg = FString::Printf(TEXT("Approach too steep: %.1f°"), GuidanceData.ApproachAngle);
        bSafe = false;
    }
    // Check terrain
    else if (!CheckTerrainSafety(GuidanceData.LandingPoint, TerrainScanRadius))
    {
        NewStatus = ELandingStatus::TerrainUnsafe;
        StatusMsg = "Terrain unsafe";
        bSafe = false;
    }

    GuidanceData.Status = NewStatus;
    GuidanceData.StatusMessage = StatusMsg;
    GuidanceData.bSafeToLand = bSafe;

    // Broadcast events
    if (NewStatus != PreviousStatus)
    {
        SetLandingStatus(NewStatus);
        PreviousStatus = NewStatus;
    }

    if (bSafe && !bWasLandingApproved)
    {
        OnLandingApproved.Broadcast();
        bWasLandingApproved = true;
    }
    else if (!bSafe && bWasLandingApproved)
    {
        BroadcastWarning(StatusMsg);
        bWasLandingApproved = false;
    }
}

void ALandingGuidanceSystem::UpdateVisualGuidance()
{
    if (!LandingBeam || !LandingZoneIndicator)
    {
        return;
    }

    // Update landing beam position
    if (TargetShip)
    {
        LandingBeam->SetWorldLocation(TargetShip->GetActorLocation());
    }

    // Update landing zone indicator
    if (GuidanceData.AltitudeAboveSurface < MaxGuidanceAltitude)
    {
        LandingZoneIndicator->SetWorldLocation(GuidanceData.LandingPoint);
        LandingZoneIndicator->SetWorldRotation(GuidanceData.SurfaceNormal.Rotation());

        // Set color based on status
        FLinearColor StatusColor = SafeColor;
        if (GuidanceData.Status == ELandingStatus::SpeedTooHigh || 
            GuidanceData.Status == ELandingStatus::ApproachTooSteep)
        {
            StatusColor = WarningColor;
        }
        else if (GuidanceData.Status == ELandingStatus::TerrainUnsafe)
        {
            StatusColor = DangerColor;
        }

        // Apply color to VFX
        LandingZoneIndicator->SetColorParameter(FName("IndicatorColor"), StatusColor);
    }
}

bool ALandingGuidanceSystem::CheckTerrainSafety(const FVector& Location, float Radius)
{
    if (!TargetPlanet)
    {
        return false;
    }

    // Perform multiple raycasts around the landing point
    int32 NumSamples = 8;
    float AngleStep = 360.0f / NumSamples;
    
    for (int32 i = 0; i < NumSamples; i++)
    {
        float Angle = AngleStep * i;
        FVector SampleOffset = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
            0.0f
        );
        
        FVector SampleLocation = Location + SampleOffset;
        FVector HitPoint;
        FVector Normal;
        
        if (RaycastToSurface(SampleLocation + FVector(0, 0, 1000), HitPoint, Normal))
        {
            // Check slope
            float Slope = FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector));
            Slope = FMath::RadiansToDegrees(Slope);
            
            if (Slope > MaxLandingSlope)
            {
                return false;
            }
        }
    }

    return true;
}

bool ALandingGuidanceSystem::RaycastToSurface(const FVector& Start, FVector& OutHitPoint, FVector& OutNormal)
{
    if (!TargetPlanet)
    {
        return false;
    }

    FVector Direction = (TargetPlanet->GetActorLocation() - Start).GetSafeNormal();
    FVector End = Start + (Direction * 100000.0f);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(TargetShip);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
    {
        OutHitPoint = HitResult.Location;
        OutNormal = HitResult.Normal;
        return true;
    }

    return false;
}

float ALandingGuidanceSystem::CalculateApproachAngle() const
{
    if (!TargetShip)
    {
        return 0.0f;
    }

    FVector ShipForward = TargetShip->GetActorForwardVector();
    FVector ToSurface = -GuidanceData.SurfaceNormal;
    
    float DotProduct = FVector::DotProduct(ShipForward, ToSurface);
    float Angle = FMath::Acos(DotProduct);
    return FMath::RadiansToDegrees(Angle);
}

void ALandingGuidanceSystem::SetLandingStatus(ELandingStatus NewStatus)
{
    OnLandingStatusChanged.Broadcast(NewStatus);
    UE_LOG(LogTemp, Log, TEXT("LandingGuidanceSystem: Status changed to %d"), (int32)NewStatus);
}

void ALandingGuidanceSystem::BroadcastWarning(const FString& Message)
{
    OnLandingWarning.Broadcast(Message);
    UE_LOG(LogTemp, Warning, TEXT("LandingGuidanceSystem: %s"), *Message);
}
