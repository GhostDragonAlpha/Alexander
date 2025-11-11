// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VrBubbleComponent.generated.h"

// Forward declarations
class AOrbitalBody;

/**
 * Configuration for VR bubble projection system
 */
USTRUCT(BlueprintType)
struct FBubbleProjectionConfig
{
    GENERATED_BODY()

    // Base bubble radius in centimeters (default: 100m = 10000cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
    float BaseBubbleRadius;

    // Maximum real distance that can be projected (km)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
    float MaxProjectedDistance;

    // Inverse square scaling factor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
    float InverseSquareFactor;

    // Enable dynamic bubble sizing based on ship dimensions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
    bool bEnableDynamicSizing;

    // Dynamic size multiplier (bubble = max_ship_dimension * Multiplier)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble", meta = (EditCondition = "bEnableDynamicSizing"))
    float DynamicSizeMultiplier;

    // Minimum bubble radius when using dynamic sizing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble", meta = (EditCondition = "bEnableDynamicSizing"))
    float MinDynamicRadius;

    // Maximum bubble radius when using dynamic sizing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble", meta = (EditCondition = "bEnableDynamicSizing"))
    float MaxDynamicRadius;

    FBubbleProjectionConfig()
        : BaseBubbleRadius(10000.0f) // 100m in cm
        , MaxProjectedDistance(1000000.0f) // 1 million km
        , InverseSquareFactor(1.0f)
        , bEnableDynamicSizing(true)
        , DynamicSizeMultiplier(2.0f)
        , MinDynamicRadius(5000.0f) // 50m minimum
        , MaxDynamicRadius(20000.0f) // 200m maximum
    {
    }
};

/**
 * Result of bubble projection calculation
 */
USTRUCT(BlueprintType)
struct FBubbleProjectionResult
{
    GENERATED_BODY()

    // Projected position within bubble (cm)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projection")
    FVector ProjectedPosition;

    // Projected scale factor
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projection")
    float ScaleFactor;

    // Original real-world position (km)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projection")
    FVector RealPosition;

    // Distance from bubble center (cm)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projection")
    float DistanceFromCenter;

    // Whether the object is inside the bubble
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projection")
    bool bIsInsideBubble;

    FBubbleProjectionResult()
        : ScaleFactor(1.0f)
        , DistanceFromCenter(0.0f)
        , bIsInsideBubble(false)
    {
    }
};

/**
 * VR Bubble Component for astronomical scale projection
 * Projects astronomical distances into a localized bubble for optimal VR comfort
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UVrBubbleComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVrBubbleComponent();

    //~ Begin UActorComponent interface
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    //~ End UActorComponent interface

    // ============================================================================
    // BUBBLE PROJECTION
    // ============================================================================

    // Project a real-world position into bubble space
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Projection")
    FBubbleProjectionResult ProjectPosition(const FVector& RealWorldPositionKm) const;

    // Project a distance into bubble space
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Projection")
    float ProjectDistance(float RealDistanceKm) const;

    // Convert bubble position back to real-world position
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Projection")
    FVector UnprojectPosition(const FVector& BubblePositionCm) const;

    // Calculate scale factor for a given distance
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Projection")
    float CalculateScaleFactor(float RealDistanceKm) const;

    // Check if a real-world position is within projection range
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Projection")
    bool IsWithinProjectionRange(float RealDistanceKm) const;

    // ============================================================================
    // BUBBLE MANAGEMENT
    // ============================================================================

    // Update bubble center position
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Management")
    void UpdateBubbleCenter(const FVector& NewCenter);

    // Recalculate bubble radius based on current ship dimensions
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Management")
    void RecalculateBubbleRadius();

    // Get current bubble radius
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Management")
    float GetBubbleRadius() const { return CurrentBubbleRadius; }

    // Set bubble configuration
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Management")
    void SetBubbleConfig(const FBubbleProjectionConfig& NewConfig);

    // Get current bubble configuration
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Management")
    FBubbleProjectionConfig GetBubbleConfig() const { return BubbleConfig; }

    // ============================================================================
    // OBJECT MANAGEMENT
    // ============================================================================

    // Register an orbital body with the bubble system
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Objects")
    void RegisterOrbitalBody(AOrbitalBody* Body);

    // Unregister an orbital body from the bubble system
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Objects")
    void UnregisterOrbitalBody(AOrbitalBody* Body);

    // Update projection for all registered bodies
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Objects")
    void UpdateAllProjections();

    // Get all registered bodies
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Objects")
    TArray<AOrbitalBody*> GetRegisteredBodies() const;

    // ============================================================================
    // PHYSICS INTEGRATION
    // ============================================================================

    // Calculate gravitational force using projected distances
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Physics")
    FVector CalculateProjectedGravitationalForce(AOrbitalBody* Attractor, AOrbitalBody* Satellite) const;

    // Calculate orbital velocity using projected distances
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Physics")
    float CalculateProjectedOrbitalVelocity(float CentralMassKg, float ProjectedDistanceCm) const;

    // Update physics for a body using bubble projection
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Physics")
    void UpdateBodyPhysics(AOrbitalBody* Body, float DeltaTime);

    // ============================================================================
    // DEBUG AND VISUALIZATION
    // ============================================================================

    // Enable/disable debug visualization
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Debug")
    void SetDebugVisualization(bool bEnable) { bEnableDebugVisualization = bEnable; }

    // Draw debug sphere representing the bubble
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Debug")
    void DrawDebugBubble() const;

    // Get debug information about the bubble
    UFUNCTION(BlueprintCallable, Category = "VR Bubble|Debug")
    FString GetDebugInfo() const;

    // ============================================================================
    // PROPERTIES
    // ============================================================================

    // Bubble configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Bubble")
    FBubbleProjectionConfig BubbleConfig;

    // Current bubble center position (world space)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Bubble")
    FVector BubbleCenter;

    // Current bubble radius (dynamic)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Bubble")
    float CurrentBubbleRadius;

    // Registered orbital bodies
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Bubble")
    TArray<TWeakObjectPtr<AOrbitalBody>> RegisteredBodies;

    // Enable debug visualization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Bubble|Debug")
    bool bEnableDebugVisualization;

    // Show bubble boundary
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Bubble|Debug")
    bool bShowBubbleBoundary;

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Bubble|Performance")
    float LastProjectionTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Bubble|Performance")
    int32 ProjectionCount;

protected:
    // Calculate dynamic bubble radius based on ship dimensions
    float CalculateDynamicBubbleRadius() const;

    // Apply inverse square projection
    FVector ApplyInverseSquareProjection(const FVector& Direction, float RealDistanceKm) const;

    // Update individual body projection
    void UpdateBodyProjection(AOrbitalBody* Body);

    // Performance tracking
    void UpdatePerformanceStats(float ProjectionTime);

    // Debug drawing
    void DrawDebugInfo() const;
};