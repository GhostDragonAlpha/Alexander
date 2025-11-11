// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpatialPartitioningComponent.generated.h"

// Octree node for spatial partitioning
struct FOctreeNode
{
    FVector Center;
    float HalfSize;
    TArray<FString> SystemIDs;
    TArray<TSharedPtr<FOctreeNode>> Children;
    bool bIsLeaf;

    FOctreeNode(const FVector& InCenter, float InHalfSize)
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
    TSharedPtr<FOctreeNode> RootNode;
    int32 MaxTreeDepth;
    float WorldBoundsSize;
    FVector WorldBoundsCenter;

    // Helper functions
    void SubdivideNode(TSharedPtr<FOctreeNode> Node);
    void InsertIntoNode(TSharedPtr<FOctreeNode> Node, const FString& SystemID, const FVector& Position);
    void FindInRadiusRecursive(TSharedPtr<FOctreeNode> Node, const FVector& Position, float Radius, TArray<FString>& Results) const;
    void FindNearestRecursive(TSharedPtr<FOctreeNode> Node, const FVector& Position, int32 Count, TArray<TPair<float, FString>>& Candidates) const;
    bool IsPointInNode(TSharedPtr<FOctreeNode> Node, const FVector& Position) const;
    bool DoesSphereIntersectNode(TSharedPtr<FOctreeNode> Node, const FVector& Position, float Radius) const;
    int32 GetNodeDepth(TSharedPtr<FOctreeNode> Node) const;
};