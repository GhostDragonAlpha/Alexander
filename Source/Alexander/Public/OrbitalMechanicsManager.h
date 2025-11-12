// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OrbitalMechanics.h"
#include "HohmannTransferCalculator.h"
#include "SphereOfInfluenceManager.h"
#include "ProceduralStarSystemGenerator.h"
#include "SpatialPartitioningOctree.h"
#include "SystemSelfTestInterface.h"
#include "OrbitalMechanicsManager.generated.h"

// Forward declarations
class AOrbitalBody;
class UHohmannTransferCalculator;
class USphereOfInfluenceManager;
class UProceduralStarSystemGenerator;
class USpatialPartitioningOctree;

// Gravity calculation mode enum (must be in global scope for UENUM)
UENUM(BlueprintType)
enum class EGravityCalculationMode : uint8
{
	// Only consider primary body's gravity (faster, less accurate)
	PrimaryOnly,
	
	// Consider multiple bodies within sphere of influence (balanced)
	PatchedConics,
	
	// Full n-body simulation (most accurate, computationally expensive)
	NBody
};

/**
 * Main manager class for the Orbital Mechanics system
 * Coordinates all orbital mechanics calculations and manages the universe
 */
UCLASS(Blueprintable)
class ALEXANDER_API UOrbitalMechanicsManager : public UWorldSubsystem, public ISystemSelfTestInterface
{
    GENERATED_BODY()

public:
    UOrbitalMechanicsManager();

    //~ Begin USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End USubsystem interface

    //~ Begin ISystemSelfTestInterface interface
    virtual bool RunSelfTest_Implementation(FSystemTestResult& OutResult) override;
    virtual FString GetSystemName_Implementation() const override;
    virtual FString GetTestDescription_Implementation() const override;
    virtual bool IsReadyForTesting_Implementation() const override;
    //~ End ISystemSelfTestInterface interface

    // Tick function
    virtual void Tick(float DeltaTime);
    virtual TStatId GetStatId() const;

    // ============================================================================
    // UNIVERSE MANAGEMENT
    // ============================================================================

    // Initialize the orbital mechanics system with a root body (typically a star)
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Universe")
    void InitializeUniverse(AOrbitalBody* RootBody);

    // Register an orbital body with the system
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Universe")
    void RegisterOrbitalBody(AOrbitalBody* Body);

    // Unregister an orbital body from the system
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Universe")
    void UnregisterOrbitalBody(AOrbitalBody* Body);

    // Get all registered orbital bodies
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Universe")
    TArray<AOrbitalBody*> GetAllOrbitalBodies() const;

    // Find orbital bodies within a radius from a position
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Universe")
    TArray<AOrbitalBody*> FindBodiesInRadius(const FVector& Position, float Radius) const;

    // ============================================================================
    // ORBITAL CALCULATIONS
    // ============================================================================

    // Calculate the next orbital position for a body using Keplerian mechanics
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Calculations")
    FVector CalculateOrbitalPosition(AOrbitalBody* Body, float DeltaTime) const;

    // Calculate gravitational force between two bodies
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Calculations")
    FVector CalculateGravitationalForce(AOrbitalBody* Body1, AOrbitalBody* Body2) const;

    // Check if a body will collide with another body
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Calculations")
    bool WillCollide(AOrbitalBody* Body1, AOrbitalBody* Body2, float TimeStep) const;

    // ============================================================================
    // TRANSFER MANEUVERS
    // ============================================================================

    // Calculate Hohmann transfer between two bodies
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Transfers")
    bool CalculateHohmannTransfer(AOrbitalBody* FromBody, AOrbitalBody* ToBody, 
                                  float& OutDeltaV1, float& OutDeltaV2, float& OutTransferTime);

    // Calculate optimal transfer window between two planets
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Transfers")
    float CalculateTransferWindow(AOrbitalBody* FromBody, AOrbitalBody* ToBody) const;

    // ============================================================================
    // SPHERE OF INFLUENCE
    // ============================================================================

    // Check if a position is within a body's sphere of influence
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    bool IsInSphereOfInfluence(const FVector& Position, AOrbitalBody* Body) const;

    // Get the body with the strongest gravitational influence at a position
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    AOrbitalBody* GetDominantBody(const FVector& Position) const;

    // Handle sphere of influence transition for a body
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    void HandleSOITransition(AOrbitalBody* Body);

    // ============================================================================
    // PROCEDURAL GENERATION
    // ============================================================================

    // Generate a complete star system procedurally
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Generation")
    AOrbitalBody* GenerateStarSystem(const FStarSystemConfig& Config);

    // Generate a planetary system around a star
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Generation")
    TArray<AOrbitalBody*> GeneratePlanetarySystem(AOrbitalBody* Star, const FPlanetarySystemConfig& Config);

    // ============================================================================
    // SPATIAL PARTITIONING
    // ============================================================================

    // Add a body to the spatial partitioning system
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Spatial")
    void AddBodyToSpatialPartition(AOrbitalBody* Body);

    // Remove a body from the spatial partitioning system
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Spatial")
    void RemoveBodyFromSpatialPartition(AOrbitalBody* Body);

    // Query bodies near a position using spatial partitioning
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Spatial")
    TArray<AOrbitalBody*> QueryNearbyBodies(const FVector& Position, float Radius) const;

    // ============================================================================
    // DEBUG AND VISUALIZATION
    // ============================================================================

    // Enable/disable debug visualization
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Debug")
    void SetDebugVisualization(bool bEnable);

    // Draw orbital paths for all bodies
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Debug")
    void DrawOrbitalPaths() const;

    // Draw sphere of influence for all bodies
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Debug")
    void DrawSpheresOfInfluence() const;

    // ============================================================================
    // PERSISTENCE
    // ============================================================================

    // Save current orbital state to persistence system
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Persistence")
    void SaveOrbitalState();

    // Load orbital state from persistence system
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Persistence")
    void LoadOrbitalState();

    // ============================================================================
    // PROPERTIES
    // ============================================================================

    // Root body of the universe (typically the central star)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orbital Mechanics")
    TWeakObjectPtr<AOrbitalBody> RootBody;

    // All registered orbital bodies
    UPROPERTY(VisibleAnywhere, Category = "Orbital Mechanics")
    TArray<TWeakObjectPtr<AOrbitalBody>> RegisteredBodies;

    // Hohmann transfer calculator
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orbital Mechanics|Components")
    UHohmannTransferCalculator* HohmannCalculator;

    // Sphere of influence manager
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orbital Mechanics|Components")
    USphereOfInfluenceManager* SOIManager;

    // Procedural star system generator
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orbital Mechanics|Components")
    UProceduralStarSystemGenerator* SystemGenerator;

    // Spatial partitioning octree
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orbital Mechanics|Components")
    USpatialPartitioningOctree* SpatialPartitioner;

    // Enable automatic SOI transitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital Mechanics|Settings")
    bool bEnableAutomaticSOITransitions;

    // Enable debug visualization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital Mechanics|Settings")
    bool bEnableDebugVisualization;

    // Maximum time step for orbital calculations (prevents instability)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital Mechanics|Settings")
    float MaxTimeStep;

    // Gravity calculation mode
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital Mechanics|Settings")
    EGravityCalculationMode GravityCalculationMode;

protected:
    // Update all orbital bodies
    void UpdateOrbitalBodies(float DeltaTime);

    // Handle sphere of influence transitions
    void UpdateSphereOfInfluenceTransitions();

    // Apply gravity to bodies
    void ApplyGravity(float DeltaTime);

    // Check for collisions between bodies
    void CheckCollisions();

    // Draw debug information
    void DrawDebugInfo() const;

    // Registered bodies map for fast lookup
    TMap<FGuid, TWeakObjectPtr<AOrbitalBody>> BodyRegistry;

    // Performance tracking
    float LastUpdateTime;
    int32 UpdateCount;
};