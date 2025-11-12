// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HohmannTransferCalculator.generated.h"

// Forward declarations
class AOrbitalBody;

/**
 * Configuration for Hohmann transfer calculations
 */
USTRUCT(BlueprintType)
struct FHohmannTransferConfig
{
    GENERATED_BODY()

    // Starting orbital radius (km)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer")
    float StartRadius;

    // Target orbital radius (km)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer")
    float TargetRadius;

    // Standard gravitational parameter of central body (km³/s²)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer")
    float GravitationalParameter;

    // Desired transfer time (seconds) - optional, 0 = optimal
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer")
    float DesiredTransferTime;

    // Allow bi-elliptic transfers for large radius differences
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer")
    bool bAllowBiEllipticTransfers;

    FHohmannTransferConfig()
        : StartRadius(0.0f)
        , TargetRadius(0.0f)
        , GravitationalParameter(3.986004418e5f) // Earth's mu
        , DesiredTransferTime(0.0f)
        , bAllowBiEllipticTransfers(false)
    {
    }
};

/**
 * Result of Hohmann transfer calculation
 */
USTRUCT(BlueprintType)
struct FHohmannTransferResult
{
    GENERATED_BODY()

    // First burn delta-V (km/s)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Result")
    float DeltaV1;

    // Second burn delta-V (km/s)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Result")
    float DeltaV2;

    // Total delta-V required (km/s)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Result")
    float TotalDeltaV;

    // Transfer orbit semi-major axis (km)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Result")
    float TransferSemiMajorAxis;

    // Transfer time (seconds)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Result")
    float TransferTime;

    // Transfer orbit eccentricity
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Result")
    float TransferEccentricity;

    // True anomaly at departure (degrees)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Result")
    float DepartureTrueAnomaly;

    // True anomaly at arrival (degrees)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Result")
    float ArrivalTrueAnomaly;

    // Whether this is a bi-elliptic transfer
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Result")
    bool bIsBiElliptic;

    // Additional delta-V for bi-elliptic transfer (km/s)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Result")
    float DeltaV3;

    FHohmannTransferResult()
        : DeltaV1(0.0f)
        , DeltaV2(0.0f)
        , TotalDeltaV(0.0f)
        , TransferSemiMajorAxis(0.0f)
        , TransferTime(0.0f)
        , TransferEccentricity(0.0f)
        , DepartureTrueAnomaly(0.0f)
        , ArrivalTrueAnomaly(180.0f)
        , bIsBiElliptic(false)
        , DeltaV3(0.0f)
    {
    }
};

/**
 * Advanced Hohmann transfer calculator with support for:
 * - Standard Hohmann transfers
 * - Bi-elliptic transfers for large radius ratios
 * - Plane change maneuvers
 * - Phasing orbits
 * - Multi-burn sequences
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API UHohmannTransferCalculator : public UObject
{
    GENERATED_BODY()

public:
    UHohmannTransferCalculator();

    // Calculate standard Hohmann transfer between two circular orbits
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Transfers")
    bool CalculateTransfer(float R1, float R2, float Mu, float& OutDeltaV1, float& OutDeltaV2, float& OutTransferTime);

    // Calculate detailed Hohmann transfer with full result information
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Transfers")
    FHohmannTransferResult CalculateDetailedTransfer(const FHohmannTransferConfig& Config);

    // Calculate bi-elliptic transfer for large radius ratios (>11.94)
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Transfers")
    FHohmannTransferResult CalculateBiEllipticTransfer(const FHohmannTransferConfig& Config);

    // Calculate optimal transfer (chooses between Hohmann and bi-elliptic)
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Transfers")
    FHohmannTransferResult CalculateOptimalTransfer(const FHohmannTransferConfig& Config);

    // Calculate plane change maneuver delta-V
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Transfers")
    float CalculatePlaneChangeDeltaV(float Velocity, float InclinationChange);

    // Calculate combined plane change and circularization delta-V
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Transfers")
    float CalculateCombinedManeuverDeltaV(float V1, float V2, float InclinationChange);

    // Calculate phasing orbit for rendezvous
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Transfers")
    bool CalculatePhasingOrbit(float TargetRadius, float Mu, float PhaseAngle, float& OutDeltaV, float& OutPhasingTime);

    // Calculate transfer window for interplanetary transfers
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Transfers")
    float CalculateTransferWindow(float OrbitRadius1, float OrbitRadius2, float Mu, float CurrentPhaseAngle);

    // Check if bi-elliptic transfer is more efficient than Hohmann
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Transfers")
    bool ShouldUseBiEllipticTransfer(float RadiusRatio) const;

    // Get the critical radius ratio where bi-elliptic becomes more efficient
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Transfers")
    float GetBiEllipticThreshold() const { return 11.94f; }

protected:
    // Calculate standard Hohmann transfer details
    void CalculateStandardHohmannDetails(FHohmannTransferResult& Result, const FHohmannTransferConfig& Config);

    // Calculate bi-elliptic transfer details
    void CalculateBiEllipticDetails(FHohmannTransferResult& Result, const FHohmannTransferConfig& Config);

    // Validate transfer configuration
    bool ValidateConfig(const FHohmannTransferConfig& Config) const;

    // Performance tracking
    int32 CalculationCount;
    float TotalCalculationTime;
};