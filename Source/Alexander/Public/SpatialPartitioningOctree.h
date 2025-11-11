// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SpatialPartitioningOctree.generated.h"

// Forward declarations
class AOrbitalBody;

/**
 * Octree node for spatial partitioning
 */
USTRUCT()
struct FOctreeNode
{
    GENERATED_BODY()

    // Bounding box for this node
    UPROPERTY()
    FBox BoundingBox;

    // Orbital bodies in this node
    UPROPERTY()
    TArray<TWeakObjectPtr<AOrbitalBody>> Bodies;

    // Children nodes (8 for octree)
    UPROPERTY()
    TArray<FOctreeNode> Children;

    // Parent node (null for root)
    UPROPERTY()
    FOctreeNode* Parent;

    // Node depth (0 for root)
    UPROPERTY()
    int32 Depth;

    // Maximum bodies before splitting
    static constexpr int32 MAX_BODIES_PER_NODE = 8;

    // Maximum depth to prevent infinite recursion
    static constexpr int32 MAX_DEPTH = 6;

    FOctreeNode()
        : Parent(nullptr)
        , Depth(0)
    {
    }

    // Check if node is leaf (no children)
    bool IsLeaf() const
    {
        return Children.Num() == 0;
    }

    // Check if node needs to be split
    bool NeedsSplit() const
    {
        return Bodies.Num() > MAX_BODIES_PER_NODE && Depth < MAX_DEPTH;
    }
};

/**
 * Query result from spatial query
 */
USTRUCT(BlueprintType)
struct FSpatialQueryResult
{
    GENERATED_BODY()

    // Found orbital bodies
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spatial Query")
    TArray<TWeakObjectPtr<AOrbitalBody>> FoundBodies;

    // Query position
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spatial Query")
    FVector QueryPosition;

    // Query radius
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spatial Query")
    float QueryRadius;

    // Number of nodes visited during query
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spatial Query")
    int32 NodesVisited;

    // Query time in milliseconds
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spatial Query")
    float QueryTime;

    FSpatialQueryResult()
        : QueryPosition(FVector::ZeroVector)
        , QueryRadius(0.0f)
        , NodesVisited(0)
        , QueryTime(0.0f)
    {
    }
};

/**
 * Spatial partitioning octree for efficient orbital body management
 * Provides fast spatial queries and LOD management for thousands of bodies
 */
UCLASS(Blueprintable)
class ALEXANDER_API USpatialPartitioningOctree : public UObject
{
    GENERATED_BODY()

public:
    USpatialPartitioningOctree();

    // Initialize octree with a bounding box
    UFUNCTION(BlueprintCallable, Category = "Spatial Partitioning")
    void Initialize(const FBox& WorldBounds);

    // Add an orbital body to the octree
    UFUNCTION(BlueprintCallable, Category = "Spatial Partitioning")
    void AddBody(AOrbitalBody* Body);

    // Remove an orbital body from the octree
    UFUNCTION(BlueprintCallable, Category = "Spatial Partitioning")
    void RemoveBody(AOrbitalBody* Body);

    // Update body position in octree
    UFUNCTION(BlueprintCallable, Category = "Spatial Partitioning")
    void UpdateBodyPosition(AOrbitalBody* Body, const FVector& NewPosition);

    // Query bodies within a sphere
    UFUNCTION(BlueprintCallable, Category = "Spatial Partitioning")
    FSpatialQueryResult QueryBodiesInRadius(const FVector& Position, float Radius) const;

    // Query bodies within a box
    UFUNCTION(BlueprintCallable, Category = "Spatial Partitioning")
    FSpatialQueryResult QueryBodiesInBox(const FBox& QueryBox) const;

    // Get all bodies in octree
    UFUNCTION(BlueprintCallable, Category = "Spatial Partitioning")
    TArray<AOrbitalBody*> GetAllBodies() const;

    // Get body count
    UFUNCTION(BlueprintCallable, Category = "Spatial Partitioning")
    int32 GetBodyCount() const { return BodyCount; }

    // Get node count
    UFUNCTION(BlueprintCallable, Category = "Spatial Partitioning")
    int32 GetNodeCount() const { return NodeCount; }

    // Clear all bodies from octree
    UFUNCTION(BlueprintCallable, Category = "Spatial Partitioning")
    void Clear();

    // Rebuild octree from scratch
    UFUNCTION(BlueprintCallable, Category = "Spatial Partitioning")
    void Rebuild();

    // Get statistics
    UFUNCTION(BlueprintCallable, Category = "Spatial Partitioning")
    void GetStatistics(int32& OutBodyCount, int32& OutNodeCount, int32& OutMaxDepth) const;

    // Enable/disable debug visualization
    UFUNCTION(BlueprintCallable, Category = "Spatial Partitioning|Debug")
    void SetDebugVisualization(bool bEnable) { bEnableDebugVisualization = bEnable; }

    // Draw debug visualization
    UFUNCTION(BlueprintCallable, Category = "Spatial Partitioning|Debug")
    void DrawDebugOctree() const;

protected:
    // Root node of octree
    UPROPERTY()
    FOctreeNode RootNode;

    // World bounds
    UPROPERTY()
    FBox WorldBounds;

    // Body count
    UPROPERTY()
    int32 BodyCount;

    // Node count
    UPROPERTY()
    int32 NodeCount;

    // Maximum depth reached
    UPROPERTY()
    int32 MaxDepth;

    // Enable debug visualization
    UPROPERTY()
    bool bEnableDebugVisualization;

    // Add body to node
    void AddBodyToNode(FOctreeNode& Node, AOrbitalBody* Body);

    // Remove body from node
    void RemoveBodyFromNode(FOctreeNode& Node, AOrbitalBody* Body);

    // Split node into 8 children
    void SplitNode(FOctreeNode& Node);

    // Find appropriate node for body
    FOctreeNode* FindNodeForBody(FOctreeNode& Node, const FVector& Position);

    // Query nodes recursively
    void QueryNode(const FOctreeNode& Node, const FVector& Position, float Radius, TArray<AOrbitalBody*>& FoundBodies, int32& NodesVisited) const;

    // Query box recursively
    void QueryBoxNode(const FOctreeNode& Node, const FBox& QueryBox, TArray<AOrbitalBody*>& FoundBodies, int32& NodesVisited) const;

    // Get all bodies from node recursively
    void GetAllBodiesFromNode(const FOctreeNode& Node, TArray<AOrbitalBody*>& Bodies) const;

    // Count nodes recursively
    void CountNodes(const FOctreeNode& Node, int32& Count, int32& Depth) const;

    // Draw debug for node recursively
    void DrawDebugNode(const FOctreeNode& Node) const;

    // Clear node recursively
    void ClearNode(FOctreeNode& Node);
};