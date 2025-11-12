// Copyright Epic Games, Inc. All Rights Reserved.

#include "HohmannTransferCalculator.h"
#include "OrbitalBody.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"

UHohmannTransferCalculator::UHohmannTransferCalculator()
{
    CalculationCount = 0;
    TotalCalculationTime = 0.0f;
}

bool UHohmannTransferCalculator::CalculateTransfer(float R1, float R2, float Mu, float& OutDeltaV1, float& OutDeltaV2, float& OutTransferTime)
{
    float StartTime = FPlatformTime::Seconds();
    
    if (R1 <= 0.0f || R2 <= 0.0f || Mu <= 0.0f)
    {
        return false;
    }

    // Ensure R1 < R2 for standard Hohmann transfer
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
    
    CalculationCount++;
    TotalCalculationTime += (FPlatformTime::Seconds() - StartTime);
    
    return true;
}

FHohmannTransferResult UHohmannTransferCalculator::CalculateDetailedTransfer(const FHohmannTransferConfig& Config)
{
    float StartTime = FPlatformTime::Seconds();
    
    FHohmannTransferResult Result;
    
    if (!ValidateConfig(Config))
    {
        return Result;
    }
    
    // Check if bi-elliptic might be more efficient
    float RadiusRatio = Config.TargetRadius / Config.StartRadius;
    if (Config.bAllowBiEllipticTransfers && ShouldUseBiEllipticTransfer(RadiusRatio))
    {
        return CalculateBiEllipticTransfer(Config);
    }
    
    CalculateStandardHohmannDetails(Result, Config);
    
    CalculationCount++;
    TotalCalculationTime += (FPlatformTime::Seconds() - StartTime);
    
    return Result;
}

FHohmannTransferResult UHohmannTransferCalculator::CalculateBiEllipticTransfer(const FHohmannTransferConfig& Config)
{
    FHohmannTransferResult Result;
    Result.bIsBiElliptic = true;
    
    if (!ValidateConfig(Config))
    {
        return Result;
    }
    
    CalculateBiEllipticDetails(Result, Config);
    
    return Result;
}

FHohmannTransferResult UHohmannTransferCalculator::CalculateOptimalTransfer(const FHohmannTransferConfig& Config)
{
    float StartTime = FPlatformTime::Seconds();
    
    FHohmannTransferResult Result;
    
    if (!ValidateConfig(Config))
    {
        return Result;
    }
    
    // Calculate both Hohmann and bi-elliptic transfers
    FHohmannTransferResult HohmannResult;
    CalculateStandardHohmannDetails(HohmannResult, Config);
    
    FHohmannTransferResult BiEllipticResult;
    if (Config.bAllowBiEllipticTransfers)
    {
        CalculateBiEllipticDetails(BiEllipticResult, Config);
    }
    
    // Choose the transfer with lower total delta-V
    if (Config.bAllowBiEllipticTransfers && BiEllipticResult.TotalDeltaV < HohmannResult.TotalDeltaV)
    {
        Result = BiEllipticResult;
    }
    else
    {
        Result = HohmannResult;
    }
    
    CalculationCount++;
    TotalCalculationTime += (FPlatformTime::Seconds() - StartTime);
    
    return Result;
}

float UHohmannTransferCalculator::CalculatePlaneChangeDeltaV(float Velocity, float InclinationChange)
{
    if (Velocity <= 0.0f)
    {
        return 0.0f;
    }
    
    // Delta-V for plane change: Δv = 2 × v × sin(Δi/2)
    float InclinationRad = FMath::DegreesToRadians(InclinationChange);
    return 2.0f * Velocity * FMath::Sin(InclinationRad / 2.0f);
}

float UHohmannTransferCalculator::CalculateCombinedManeuverDeltaV(float V1, float V2, float InclinationChange)
{
    if (V1 <= 0.0f || V2 <= 0.0f)
    {
        return 0.0f;
    }
    
    // Law of cosines for combined maneuvers
    float InclinationRad = FMath::DegreesToRadians(InclinationChange);
    float CosineTerm = 2.0f * V1 * V2 * FMath::Cos(InclinationRad);
    return FMath::Sqrt(V1 * V1 + V2 * V2 - CosineTerm);
}

bool UHohmannTransferCalculator::CalculatePhasingOrbit(float TargetRadius, float Mu, float PhaseAngle, float& OutDeltaV, float& OutPhasingTime)
{
    if (TargetRadius <= 0.0f || Mu <= 0.0f)
    {
        return false;
    }
    
    // Circular orbit velocity at target radius
    float Vc = FMath::Sqrt(Mu / TargetRadius);
    
    // Calculate phasing orbit semi-major axis based on phase angle
    // We want to catch up or slow down to match the target
    float PhaseAngleRad = FMath::DegreesToRadians(PhaseAngle);
    float PhasingPeriodFactor = 1.0f / (1.0f + PhaseAngleRad / (2.0f * PI));
    
    // Semi-major axis of phasing orbit
    float A_Phasing = TargetRadius * FMath::Pow(PhasingPeriodFactor, 2.0f / 3.0f);
    
    // Velocity at target radius in phasing orbit
    float V_Phasing = FMath::Sqrt(Mu * (2.0f / TargetRadius - 1.0f / A_Phasing));
    
    // Delta-V required
    OutDeltaV = FMath::Abs(V_Phasing - Vc);
    
    // Phasing orbit period
    OutPhasingTime = 2.0f * PI * FMath::Sqrt(FMath::Cube(A_Phasing) / Mu);
    
    return true;
}

float UHohmannTransferCalculator::CalculateTransferWindow(float OrbitRadius1, float OrbitRadius2, float Mu, float CurrentPhaseAngle)
{
    if (OrbitRadius1 <= 0.0f || OrbitRadius2 <= 0.0f || Mu <= 0.0f)
    {
        return -1.0f;
    }
    
    // Orbital periods
    float Period1 = 2.0f * PI * FMath::Sqrt(FMath::Cube(OrbitRadius1) / Mu);
    float Period2 = 2.0f * PI * FMath::Sqrt(FMath::Cube(OrbitRadius2) / Mu);
    
    // Angular velocities
    float Omega1 = 2.0f * PI / Period1;
    float Omega2 = 2.0f * PI / Period2;
    
    // Relative angular velocity
    float RelativeOmega = FMath::Abs(Omega1 - Omega2);
    
    // Transfer window occurs when phase angle equals transfer angle
    // For Hohmann transfer, departure angle is 0 degrees (from inner orbit)
    float TargetPhaseAngle = 0.0f;
    
    // Calculate time to next transfer window
    float PhaseDifference = FMath::Fmod(TargetPhaseAngle - CurrentPhaseAngle + 360.0f, 360.0f);
    float TimeToWindow = PhaseDifference / FMath::RadiansToDegrees(RelativeOmega);
    
    return TimeToWindow;
}

bool UHohmannTransferCalculator::ShouldUseBiEllipticTransfer(float RadiusRatio) const
{
    // Bi-elliptic transfers are more efficient when radius ratio > 11.94
    return RadiusRatio > GetBiEllipticThreshold();
}

void UHohmannTransferCalculator::CalculateStandardHohmannDetails(FHohmannTransferResult& Result, const FHohmannTransferConfig& Config)
{
    // Ensure start radius is smaller
    float R1 = FMath::Min(Config.StartRadius, Config.TargetRadius);
    float R2 = FMath::Max(Config.StartRadius, Config.TargetRadius);
    
    // Semi-major axis of transfer orbit
    Result.TransferSemiMajorAxis = (R1 + R2) / 2.0f;
    
    // Transfer orbit eccentricity
    Result.TransferEccentricity = (R2 - R1) / (R2 + R1);
    
    // Velocities
    float V1 = FMath::Sqrt(Config.GravitationalParameter / R1);
    float V2 = FMath::Sqrt(Config.GravitationalParameter / R2);
    float Vp = FMath::Sqrt(Config.GravitationalParameter * (2.0f / R1 - 1.0f / Result.TransferSemiMajorAxis));
    float Va = FMath::Sqrt(Config.GravitationalParameter * (2.0f / R2 - 1.0f / Result.TransferSemiMajorAxis));
    
    // Delta-V values
    Result.DeltaV1 = Vp - V1;
    Result.DeltaV2 = V2 - Va;
    Result.TotalDeltaV = FMath::Abs(Result.DeltaV1) + FMath::Abs(Result.DeltaV2);
    
    // Transfer time
    Result.TransferTime = PI * FMath::Sqrt(FMath::Cube(Result.TransferSemiMajorAxis) / Config.GravitationalParameter);
    
    // True anomalies
    Result.DepartureTrueAnomaly = 0.0f;
    Result.ArrivalTrueAnomaly = 180.0f;
    
    // If we reversed R1 and R2, adjust delta-V signs
    if (Config.StartRadius > Config.TargetRadius)
    {
        Result.DeltaV1 = -Result.DeltaV1;
        Result.DeltaV2 = -Result.DeltaV2;
    }
}

void UHohmannTransferCalculator::CalculateBiEllipticDetails(FHohmannTransferResult& Result, const FHohmannTransferConfig& Config)
{
    // For bi-elliptic transfer, we need an intermediate radius > R2
    float R1 = FMath::Min(Config.StartRadius, Config.TargetRadius);
    float R2 = FMath::Max(Config.StartRadius, Config.TargetRadius);
    
    // Choose intermediate radius (typically 2-3 times R2)
    float R3 = FMath::Max(R2 * 2.5f, R2 + 1000000.0f); // At least 2.5x R2 or R2 + 1000km
    
    // First transfer orbit (R1 to R3)
    float A1 = (R1 + R3) / 2.0f;
    float V1 = FMath::Sqrt(Config.GravitationalParameter / R1);
    float Vp1 = FMath::Sqrt(Config.GravitationalParameter * (2.0f / R1 - 1.0f / A1));
    
    // Second transfer orbit (R3 to R2)
    float A2 = (R2 + R3) / 2.0f;
    float V2 = FMath::Sqrt(Config.GravitationalParameter / R2);
    float Va2 = FMath::Sqrt(Config.GravitationalParameter * (2.0f / R2 - 1.0f / A2));
    float Vp2 = FMath::Sqrt(Config.GravitationalParameter * (2.0f / R3 - 1.0f / A2));
    float Va1 = FMath::Sqrt(Config.GravitationalParameter * (2.0f / R3 - 1.0f / A1));
    
    // Delta-V values
    Result.DeltaV1 = Vp1 - V1;
    Result.DeltaV2 = Va1 - Vp2;
    Result.DeltaV3 = Va2 - V2;
    Result.TotalDeltaV = FMath::Abs(Result.DeltaV1) + FMath::Abs(Result.DeltaV2) + FMath::Abs(Result.DeltaV3);
    
    // Transfer times
    float TransferTime1 = PI * FMath::Sqrt(FMath::Cube(A1) / Config.GravitationalParameter);
    float TransferTime2 = PI * FMath::Sqrt(FMath::Cube(A2) / Config.GravitationalParameter);
    Result.TransferTime = TransferTime1 + TransferTime2;
    
    // True anomalies
    Result.DepartureTrueAnomaly = 0.0f;
    Result.ArrivalTrueAnomaly = 180.0f;
    
    // If we reversed R1 and R2, adjust delta-V signs
    if (Config.StartRadius > Config.TargetRadius)
    {
        Result.DeltaV1 = -Result.DeltaV1;
        Result.DeltaV2 = -Result.DeltaV2;
        Result.DeltaV3 = -Result.DeltaV3;
    }
}

bool UHohmannTransferCalculator::ValidateConfig(const FHohmannTransferConfig& Config) const
{
    if (Config.StartRadius <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid start radius: %.2f"), Config.StartRadius);
        return false;
    }
    
    if (Config.TargetRadius <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid target radius: %.2f"), Config.TargetRadius);
        return false;
    }
    
    if (Config.GravitationalParameter <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid gravitational parameter: %.2f"), Config.GravitationalParameter);
        return false;
    }
    
    if (Config.StartRadius == Config.TargetRadius)
    {
        UE_LOG(LogTemp, Warning, TEXT("Start and target radii are identical: %.2f"), Config.StartRadius);
        return false;
    }
    
    return true;
}