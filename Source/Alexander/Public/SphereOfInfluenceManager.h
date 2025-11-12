// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SphereOfInfluenceManager.generated.h"

// Forward declarations
class AOrbitalBody;
class UOrbitalMechanics;

/**
 * Sphere of influence transition event
 */
USTRUCT(BlueprintType)
struct FSOITransitionEvent
{
    GENERATED_BODY()

    // Body that transitioned
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SOI")
    TWeakObjectPtr<AOrbitalBody> Body;

    // Previous primary body
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SOI")
    TWeakObjectPtr<AOrbitalBody> PreviousPrimary;

    // New primary body
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SOI")
    TWeakObjectPtr<AOrbitalBody> NewPrimary;

    // Transition position
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SOI")
    FVector TransitionPosition;

    // Transition time
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SOI")
    float TransitionTime;

    FSOITransitionEvent()
        : TransitionTime(0.0f)
    {
    }
};

/**
 * Hierarchical sphere of influence node
 */
USTRUCT()
struct FSOINode
{
    GENERATED_BODY()

    // Body at this node
    UPROPERTY()
    TWeakObjectPtr<AOrbitalBody> Body;

    // Children bodies (moons, satellites, etc.)
    UPROPERTY()
    TArray<TWeakObjectPtr<AOrbitalBody>> Children;

    // Parent body (null for root)
    UPROPERTY()
    TWeakObjectPtr<AOrbitalBody> Parent;

    // Sphere of influence radius (km)
    UPROPERTY()
    float SphereOfInfluenceRadius;

    // Level in hierarchy (0 = root, 1 = planet, 2 = moon, etc.)
    UPROPERTY()
    int32 HierarchyLevel;

    FSOINode()
        : SphereOfInfluenceRadius(0.0f)
        , HierarchyLevel(0)
    {
    }
};

/**
 * Sphere of influence manager for handling hierarchical gravitational systems
 * Manages transitions between different spheres of influence using patched conics
 */
UCLASS(Blueprintable)
class ALEXANDER_API USphereOfInfluenceManager : public UObject
{
    GENERATED_BODY()

public:
    USphereOfInfluenceManager();

    // Initialize the SOI manager with a root body
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    void Initialize(AOrbitalBody* RootBody);

    // Update all sphere of influence calculations
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    void UpdateSOI(float DeltaTime);

    // Check if a body needs to transition to a different SOI
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    bool CheckSOITransition(AOrbitalBody* Body) const;

    // Handle sphere of influence transition for a body
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    void HandleSOITransition(AOrbitalBody* Body);

    // Get the dominant gravitational body at a position
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    AOrbitalBody* GetDominantBody(const FVector& Position) const;

    // Get all bodies that affect a position (within their SOI)
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    TArray<AOrbitalBody*> GetInfluentialBodies(const FVector& Position) const;

    // Calculate sphere of influence radius for a body
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    float CalculateSphereOfInfluence(AOrbitalBody* Body) const;

    // Check if a position is within a body's sphere of influence
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    bool IsInSphereOfInfluence(const FVector& Position, AOrbitalBody* Body) const;

    // Get the SOI hierarchy root
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    AOrbitalBody* GetRootBody() const { return RootBody; }

    // Get the SOI hierarchy as a tree structure
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    FSOINode GetSOIHierarchy() const;

    // Get all children of a body in the hierarchy
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    TArray<AOrbitalBody*> GetChildren(AOrbitalBody* Body) const;

    // Get the parent of a body in the hierarchy
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    AOrbitalBody* GetParent(AOrbitalBody* Body) const;

    // Get the hierarchy level of a body
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    int32 GetHierarchyLevel(AOrbitalBody* Body) const;

    // Register a body with the SOI system
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    void RegisterBody(AOrbitalBody* Body);

    // Unregister a body from the SOI system
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    void UnregisterBody(AOrbitalBody* Body);

    // Update the SOI hierarchy after body changes
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    void UpdateHierarchy();

    // Enable/disable automatic SOI transitions
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    void SetAutomaticTransitions(bool bEnable) { bAutomaticTransitions = bEnable; }

    // Get automatic transitions state
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|SOI")
    bool GetAutomaticTransitions() const { return bAutomaticTransitions; }

    // Delegate for SOI transition events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSOITransition, const FSOITransitionEvent&, TransitionEvent);
    UPROPERTY(BlueprintAssignable, Category = "Orbital Mechanics|SOI")
    FOnSOITransition OnSOITransition;

protected:
    // Build the SOI hierarchy tree
    void BuildHierarchy();

    // Find the appropriate parent for a body in the hierarchy
    AOrbitalBody* FindAppropriateParent(AOrbitalBody* Body) const;

    // Update sphere of influence radii for all bodies
    void UpdateAllSphereOfInfluences();

    // Handle the actual SOI transition
    void PerformSOITransition(AOrbitalBody* Body, AOrbitalBody* NewPrimary);

    // Calculate gravitational parameter for a body
    float CalculateGravitationalParameter(AOrbitalBody* Body) const;

    // Get all registered bodies
    TArray<AOrbitalBody*> GetAllRegisteredBodies() const;

    // Root body of the hierarchy
    UPROPERTY()
    TWeakObjectPtr<AOrbitalBody> RootBody;

    // All registered bodies
    UPROPERTY()
    TArray<TWeakObjectPtr<AOrbitalBody>> RegisteredBodies;

    // SOI hierarchy tree
    UPROPERTY()
    FSOINode HierarchyRoot;

    // Map of body to its SOI node for fast lookup
    TMap<FGuid, FSOINode*> BodyNodeMap;

    // Enable automatic SOI transitions
    UPROPERTY()
    bool bAutomaticTransitions;

    // Transition threshold (how close to SOI boundary before transition)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital Mechanics|SOI")
    float TransitionThreshold;

    // Minimum time between SOI transitions (prevents rapid transitions)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital Mechanics|SOI")
    float MinTransitionTime;

    // Performance tracking
    int32 TransitionCount;
    float TotalTransitionTime;
};