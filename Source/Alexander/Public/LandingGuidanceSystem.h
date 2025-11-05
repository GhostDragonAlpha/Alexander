// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LandingGuidanceSystem.generated.h"

// Forward declarations
class ASpaceship;
class APlanet;
class UNiagaraSystem;
class UNiagaraComponent;

/**
 * Landing Status
 */
UENUM(BlueprintType)
enum class ELandingStatus : uint8
{
    ClearToLand UMETA(DisplayName = "Clear To Land"),
    ApproachTooSteep UMETA(DisplayName = "Approach Too Steep"),
    SpeedTooHigh UMETA(DisplayName = "Speed Too High"),
    TerrainUnsafe UMETA(DisplayName = "Terrain Unsafe"),
    RotationUnstable UMETA(DisplayName = "Rotation Unstable"),
    TooFarFromSurface UMETA(DisplayName = "Too Far From Surface"),
    NoLandingZone UMETA(DisplayName = "No Landing Zone")
};

/**
 * Landing Guidance Data
 */
USTRUCT(BlueprintType)
struct FLandingGuidanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing")
    ELandingStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing")
    float AltitudeAboveSurface;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing")
    float VerticalSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing")
    float HorizontalSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing")
    float ApproachAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing")
    FVector LandingPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing")
    FVector SurfaceNormal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing")
    bool bSafeToLand;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing")
    FString StatusMessage;

    FLandingGuidanceData()
        : Status(ELandingStatus::TooFarFromSurface)
        , AltitudeAboveSurface(0.0f)
        , VerticalSpeed(0.0f)
        , HorizontalSpeed(0.0f)
        , ApproachAngle(0.0f)
        , LandingPoint(FVector::ZeroVector)
        , SurfaceNormal(FVector::UpVector)
        , bSafeToLand(false)
    {
    }
};

/**
 * Event Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLandingStatusChanged, ELandingStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLandingApproved);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLandingWarning, const FString&, WarningMessage);

/**
 * Landing Guidance System
 * Provides visual and data feedback for safe spacecraft landing
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API ALandingGuidanceSystem : public AActor
{
    GENERATED_BODY()

public:
    ALandingGuidanceSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ============================================================================
    // COMPONENTS
    // ============================================================================

    // Root scene component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootComp;

    // Landing beam VFX
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* LandingBeam;

    // Landing zone indicator VFX
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* LandingZoneIndicator;

    // ============================================================================
    // SYSTEM REFERENCES
    // ============================================================================

    // Target spaceship
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guidance")
    ASpaceship* TargetShip;

    // Target planet/moon
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guidance")
    APlanet* TargetPlanet;

    // ============================================================================
    // GUIDANCE DATA
    // ============================================================================

    // Current guidance data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guidance")
    FLandingGuidanceData GuidanceData;

    // Is guidance system active?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guidance")
    bool bIsActive;

    // Show visual guidance?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guidance")
    bool bShowVisualGuidance;

    // Show HUD guidance?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guidance")
    bool bShowHUDGuidance;

    // ============================================================================
    // CONFIGURATION
    // ============================================================================

    // Maximum altitude for guidance (meters)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float MaxGuidanceAltitude;

    // Safe landing speed (m/s)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float SafeLandingSpeed;

    // Maximum approach angle (degrees)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float MaxApproachAngle;

    // Terrain scan radius (meters)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float TerrainScanRadius;

    // Maximum slope for landing (degrees)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float MaxLandingSlope;

    // Update frequency (Hz)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float UpdateFrequency;

    // Landing assist strength (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float LandingAssistStrength;

    // Auto-land enabled?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    bool bAutoLandEnabled;

    // ============================================================================
    // SYSTEM CONTROL
    // ============================================================================

    // Activate guidance system
    UFUNCTION(BlueprintCallable, Category = "Guidance")
    void ActivateGuidance(ASpaceship* Ship, APlanet* Planet);

    // Deactivate guidance system
    UFUNCTION(BlueprintCallable, Category = "Guidance")
    void DeactivateGuidance();

    // Set target ship
    UFUNCTION(BlueprintCallable, Category = "Guidance")
    void SetTargetShip(ASpaceship* Ship);

    // Set target planet
    UFUNCTION(BlueprintCallable, Category = "Guidance")
    void SetTargetPlanet(APlanet* Planet);

    // Toggle visual guidance
    UFUNCTION(BlueprintCallable, Category = "Guidance")
    void SetVisualGuidance(bool bEnabled);

    // Toggle auto-land
    UFUNCTION(BlueprintCallable, Category = "Guidance")
    void SetAutoLand(bool bEnabled);

    // ============================================================================
    // GUIDANCE FUNCTIONS
    // ============================================================================

    // Update guidance data
    UFUNCTION(BlueprintCallable, Category = "Guidance")
    void UpdateGuidance(float DeltaTime);

    // Get current landing status
    UFUNCTION(BlueprintCallable, Category = "Guidance")
    ELandingStatus GetLandingStatus() const { return GuidanceData.Status; }

    // Get guidance data
    UFUNCTION(BlueprintCallable, Category = "Guidance")
    FLandingGuidanceData GetGuidanceData() const { return GuidanceData; }

    // Is safe to land?
    UFUNCTION(BlueprintCallable, Category = "Guidance")
    bool IsSafeToLand() const { return GuidanceData.bSafeToLand; }

    // Get altitude above surface
    UFUNCTION(BlueprintCallable, Category = "Guidance")
    float GetAltitude() const { return GuidanceData.AltitudeAboveSurface; }

    // Get vertical speed
    UFUNCTION(BlueprintCallable, Category = "Guidance")
    float GetVerticalSpeed() const { return GuidanceData.VerticalSpeed; }

    // Get status message
    UFUNCTION(BlueprintCallable, Category = "Guidance")
    FString GetStatusMessage() const { return GuidanceData.StatusMessage; }

    // ============================================================================
    // LANDING ASSIST
    // ============================================================================

    // Apply landing assist forces
    UFUNCTION(BlueprintCallable, Category = "Landing Assist")
    void ApplyLandingAssist(float DeltaTime);

    // Execute auto-land
    UFUNCTION(BlueprintCallable, Category = "Landing Assist")
    void ExecuteAutoLand(float DeltaTime);

    // Calculate landing point
    UFUNCTION(BlueprintCallable, Category = "Landing Assist")
    FVector CalculateLandingPoint() const;

    // Calculate approach vector
    UFUNCTION(BlueprintCallable, Category = "Landing Assist")
    FVector CalculateApproachVector() const;

    // ============================================================================
    // EVENTS
    // ============================================================================

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnLandingStatusChanged OnLandingStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnLandingApproved OnLandingApproved;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnLandingWarning OnLandingWarning;

protected:
    // Internal Functions
    void CalculateGuidanceData();
    void UpdateLandingStatus();
    void UpdateVisualGuidance();
    bool CheckTerrainSafety(const FVector& Location, float Radius);
    bool RaycastToSurface(const FVector& Start, FVector& OutHitPoint, FVector& OutNormal);
    float CalculateApproachAngle() const;
    void SetLandingStatus(ELandingStatus NewStatus);
    void BroadcastWarning(const FString& Message);

    // VFX
    UPROPERTY(EditAnywhere, Category = "VFX")
    UNiagaraSystem* LandingBeamVFX;

    UPROPERTY(EditAnywhere, Category = "VFX")
    UNiagaraSystem* LandingZoneVFX;

    UPROPERTY(EditAnywhere, Category = "VFX")
    FLinearColor SafeColor;

    UPROPERTY(EditAnywhere, Category = "VFX")
    FLinearColor WarningColor;

    UPROPERTY(EditAnywhere, Category = "VFX")
    FLinearColor DangerColor;

private:
    // Update tracking
    float LastUpdateTime;
    float UpdateInterval;

    // State tracking
    ELandingStatus PreviousStatus;
    bool bWasLandingApproved;

    // Auto-land state
    bool bAutoLandActive;
    FVector AutoLandTarget;
};
