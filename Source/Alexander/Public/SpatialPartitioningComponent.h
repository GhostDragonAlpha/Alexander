// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpatialPartitioningComponent.generated.h"

<<<<<<< Updated upstream
// Simple octree node for spatial partitioning
=======
// Simple octree node for spatial partitioning
>>>>>>> Stashed changes
struct FSimpleOctreeNode
{
    FVector Center;
    float HalfSize;
    TArray<FString> SystemIDs;
    TArray<TSharedPtr<FSimpleOctreeNode>> Children;
    bool bIsLeaf;

    FSimpleOctreeNode(const FVector& InCenter, float InHalfSize)
        : Center(InCenter)
        , HalfSize(InHalfSize)
        , bIsLeaf(true)
    {}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API USpatialPartitioningComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USpatialPartitioningComponent();

    // Initialize octree with bounds and depth
    UFUNCTION(BlueprintCallable, Category = "SpatialPartitioning")
    void Initialize(const FVector& WorldCenter, float WorldSize, int32 MaxDepth);

    // Insert system into octree
    UFUNCTION(BlueprintCallable, Category = "SpatialPartitioning")
    void InsertSystem(const FString& SystemID, const FVector& Position);

    // Remove system from octree
    UFUNCTION(BlueprintCallable, Category = "SpatialPartitioning")
    void RemoveSystem(const FString& SystemID);

    // Find systems within radius
    UFUNCTION(BlueprintCallable, Category = "SpatialPartitioning")
    TArray<FString> FindInRadius(const FVector& Position, float Radius) const;

    // Find nearest systems
    UFUNCTION(BlueprintCallable, Category = "SpatialPartitioning")
    TArray<FString> FindNearest(const FVector& Position, int32 Count) const;

    // Find systems in box
    UFUNCTION(BlueprintCallable, Category = "SpatialPartitioning")
    TArray<FString> FindInBox(const FVector& Center, const FVector& Extent) const;

    // Clear all systems
    UFUNCTION(BlueprintCallable, Category = "SpatialPartitioning")
    void Clear();

    // Get statistics
    UFUNCTION(BlueprintCallable, Category = "SpatialPartitioning")
    int32 GetNodeCount() const;

    UFUNCTION(BlueprintCallable, Category = "SpatialPartitioning")
    int32 GetSystemCount() const;

protected:
    virtual void BeginPlay() override;

private:
    TSharedPtr<FSimpleOctreeNode> RootNode;
    int32 MaxTreeDepth;
    float WorldBoundsSize;
    FVector WorldBoundsCenter;

    // Helper functions
    void SubdivideNode(TSharedPtr<FSimpleOctreeNode> Node);
    void InsertIntoNode(TSharedPtr<FSimpleOctreeNode> Node, const FString& SystemID, const FVector& Position);
    void FindInRadiusRecursive(TSharedPtr<FSimpleOctreeNode> Node, const FVector& Position, float Radius, TArray<FString>& Results) const;
    void FindNearestRecursive(TSharedPtr<FSimpleOctreeNode> Node, const FVector& Position, int32 Count, TArray<TPair<float, FString>>& Candidates) const;
    bool IsPointInNode(TSharedPtr<FSimpleOctreeNode> Node, const FVector& Position) const;
    bool DoesSphereIntersectNode(TSharedPtr<FSimpleOctreeNode> Node, const FVector& Position, float Radius) const;
    int32 GetNodeDepth(TSharedPtr<FSimpleOctreeNode> Node) const;
};