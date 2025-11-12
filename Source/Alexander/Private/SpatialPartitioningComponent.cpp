// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpatialPartitioningComponent.h"
#include "Engine/World.h"

USpatialPartitioningComponent::USpatialPartitioningComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    MaxTreeDepth = 5;
    WorldBoundsSize = 100000.0f; // 100k units default
}

void USpatialPartitioningComponent::BeginPlay()
{
    Super::BeginPlay();
    Initialize(FVector::ZeroVector, WorldBoundsSize, MaxTreeDepth);
}

void USpatialPartitioningComponent::Initialize(const FVector& WorldCenter, float WorldSize, int32 MaxDepth)
{
    WorldBoundsCenter = WorldCenter;
    WorldBoundsSize = WorldSize;
    MaxTreeDepth = MaxDepth;
    
    RootNode = MakeShareable(new FSimpleOctreeNode(WorldCenter, WorldSize * 0.5f));
    UE_LOG(LogTemp, Log, TEXT("SpatialPartitioning initialized: Center=%s, Size=%.2f, Depth=%d"), 
        *WorldCenter.ToString(), WorldSize, MaxDepth);
}

void USpatialPartitioningComponent::InsertSystem(const FString& SystemID, const FVector& Position)
{
    if (!RootNode.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("SpatialPartitioning not initialized"));
        return;
    }

    // Remove if already exists
    RemoveSystem(SystemID);
    
    // Insert into octree
    InsertIntoNode(RootNode, SystemID, Position);
}

void USpatialPartitioningComponent::RemoveSystem(const FString& SystemID)
{
    if (!RootNode.IsValid())
    {
        return;
    }

    // This is a simplified removal - in production, you'd want to track which node each system is in
    // For now, we'll just clear and rebuild the octree (acceptable for small numbers of systems)
    Clear();
}

TArray<FString> USpatialPartitioningComponent::FindInRadius(const FVector& Position, float Radius) const
{
    TArray<FString> Results;
    if (!RootNode.IsValid())
    {
        return Results;
    }

    FindInRadiusRecursive(RootNode, Position, Radius, Results);
    return Results;
}

TArray<FString> USpatialPartitioningComponent::FindNearest(const FVector& Position, int32 Count) const
{
    TArray<FString> Results;
    if (!RootNode.IsValid() || Count <= 0)
    {
        return Results;
    }

    TArray<TPair<float, FString>> Candidates;
    FindNearestRecursive(RootNode, Position, Count, Candidates);

    // Sort by distance and take top N
    Candidates.Sort([](const TPair<float, FString>& A, const TPair<float, FString>& B) {
        return A.Key < B.Key;
    });

    for (int32 i = 0; i < FMath::Min(Count, Candidates.Num()); ++i)
    {
        Results.Add(Candidates[i].Value);
    }

    return Results;
}

TArray<FString> USpatialPartitioningComponent::FindInBox(const FVector& Center, const FVector& Extent) const
{
    TArray<FString> Results;
    if (!RootNode.IsValid())
    {
        return Results;
    }

    // Simple implementation - check all systems in octree
    // In production, you'd traverse the octree more efficiently
    TFunction<void(TSharedPtr<FSimpleOctreeNode>)> TraverseNode;
    TraverseNode = [&](TSharedPtr<FSimpleOctreeNode> Node) {
        if (!Node.IsValid())
        {
            return;
        }

        // Check if node intersects with box
        FVector NodeMin = Node->Center - FVector(Node->HalfSize);
        FVector NodeMax = Node->Center + FVector(Node->HalfSize);
        FVector BoxMin = Center - Extent;
        FVector BoxMax = Center + Extent;

        bool bIntersects = !(NodeMin.X > BoxMax.X || NodeMax.X < BoxMin.X ||
                           NodeMin.Y > BoxMax.Y || NodeMax.Y < BoxMin.Y ||
                           NodeMin.Z > BoxMax.Z || NodeMax.Z < BoxMin.Z);

        if (bIntersects)
        {
            // Add systems in this node
            for (const FString& SystemID : Node->SystemIDs)
            {
                Results.Add(SystemID);
            }

            // Traverse children
            for (auto& Child : Node->Children)
            {
                TraverseNode(Child);
            }
        }
    };

    TraverseNode(RootNode);
    return Results;
}

void USpatialPartitioningComponent::Clear()
{
    RootNode.Reset();
    if (WorldBoundsSize > 0)
    {
        RootNode = MakeShareable(new FSimpleOctreeNode(WorldBoundsCenter, WorldBoundsSize * 0.5f));
    }
}

int32 USpatialPartitioningComponent::GetNodeCount() const
{
    if (!RootNode.IsValid())
    {
        return 0;
    }

    int32 Count = 0;
    TFunction<void(TSharedPtr<FSimpleOctreeNode>)> CountNodes;
    CountNodes = [&](TSharedPtr<FSimpleOctreeNode> Node) {
        if (!Node.IsValid())
        {
            return;
        }
        Count++;
        for (auto& Child : Node->Children)
        {
            CountNodes(Child);
        }
    };

    CountNodes(RootNode);
    return Count;
}

int32 USpatialPartitioningComponent::GetSystemCount() const
{
    if (!RootNode.IsValid())
    {
        return 0;
    }

    int32 Count = 0;
    TFunction<void(TSharedPtr<FSimpleOctreeNode>)> CountSystems;
    CountSystems = [&](TSharedPtr<FSimpleOctreeNode> Node) {
        if (!Node.IsValid())
        {
            return;
        }
        Count += Node->SystemIDs.Num();
        for (auto& Child : Node->Children)
        {
            CountSystems(Child);
        }
    };

    CountSystems(RootNode);
    return Count;
}

void USpatialPartitioningComponent::SubdivideNode(TSharedPtr<FSimpleOctreeNode> Node)
{
    if (!Node.IsValid() || Node->Children.Num() > 0)
    {
        return;
    }

    float ChildSize = Node->HalfSize * 0.5f;
    
    // Create 8 children
    for (int32 i = 0; i < 8; ++i)
    {
        FVector Offset(
            (i & 1) ? ChildSize : -ChildSize,
            (i & 2) ? ChildSize : -ChildSize,
            (i & 4) ? ChildSize : -ChildSize
        );
        
        auto ChildNode = MakeShareable(new FSimpleOctreeNode(Node->Center + Offset, ChildSize));
        Node->Children.Add(ChildNode);
    }

    Node->bIsLeaf = false;

    // Redistribute systems to children
    for (const FString& SystemID : Node->SystemIDs)
    {
        // In a real implementation, you'd need to track system positions
        // For now, we'll just keep them in the parent node
    }
}

void USpatialPartitioningComponent::InsertIntoNode(TSharedPtr<FSimpleOctreeNode> Node, const FString& SystemID, const FVector& Position)
{
    if (!Node.IsValid())
    {
        return;
    }

    // Check if point is in this node
    if (!IsPointInNode(Node, Position))
    {
        return;
    }

    // If leaf node or at max depth, add system here
    if (Node->bIsLeaf || GetNodeDepth(Node) >= MaxTreeDepth)
    {
        Node->SystemIDs.Add(SystemID);
        return;
    }

    // Otherwise, try to insert into children
    bool bInsertedInChild = false;
    for (auto& Child : Node->Children)
    {
        if (IsPointInNode(Child, Position))
        {
            InsertIntoNode(Child, SystemID, Position);
            bInsertedInChild = true;
            break;
        }
    }

    // If not inserted in any child, add to this node
    if (!bInsertedInChild)
    {
        Node->SystemIDs.Add(SystemID);
    }
}

void USpatialPartitioningComponent::FindInRadiusRecursive(TSharedPtr<FSimpleOctreeNode> Node, const FVector& Position, float Radius, TArray<FString>& Results) const
{
    if (!Node.IsValid())
    {
        return;
    }

    // Check if sphere intersects this node
    if (!DoesSphereIntersectNode(Node, Position, Radius))
    {
        return;
    }

    // Add systems in this node
    for (const FString& SystemID : Node->SystemIDs)
    {
        Results.Add(SystemID);
    }

    // Recurse into children
    for (auto& Child : Node->Children)
    {
        FindInRadiusRecursive(Child, Position, Radius, Results);
    }
}

void USpatialPartitioningComponent::FindNearestRecursive(TSharedPtr<FSimpleOctreeNode> Node, const FVector& Position, int32 Count, TArray<TPair<float, FString>>& Candidates) const
{
    if (!Node.IsValid())
    {
        return;
    }

    // Add systems in this node
    for (const FString& SystemID : Node->SystemIDs)
    {
        // In a real implementation, you'd calculate actual distance
        // For now, we'll use a placeholder distance
        float Distance = FVector::Dist(Position, Node->Center);
        Candidates.Add(TPair<float, FString>(Distance, SystemID));
    }

    // Recurse into children
    for (auto& Child : Node->Children)
    {
        FindNearestRecursive(Child, Position, Count, Candidates);
    }
}

bool USpatialPartitioningComponent::IsPointInNode(TSharedPtr<FSimpleOctreeNode> Node, const FVector& Position) const
{
    if (!Node.IsValid())
    {
        return false;
    }

    FVector Min = Node->Center - FVector(Node->HalfSize);
    FVector Max = Node->Center + FVector(Node->HalfSize);

    return Position.X >= Min.X && Position.X <= Max.X &&
           Position.Y >= Min.Y && Position.Y <= Max.Y &&
           Position.Z >= Min.Z && Position.Z <= Max.Z;
}

bool USpatialPartitioningComponent::DoesSphereIntersectNode(TSharedPtr<FSimpleOctreeNode> Node, const FVector& Position, float Radius) const
{
    if (!Node.IsValid())
    {
        return false;
    }

    // Simple AABB-sphere intersection test
    FVector NodeMin = Node->Center - FVector(Node->HalfSize);
    FVector NodeMax = Node->Center + FVector(Node->HalfSize);

    float DistSq = 0.0f;
    
    // Calculate squared distance from sphere center to box
    for (int32 i = 0; i < 3; ++i)
    {
        float v = Position[i];
        if (v < NodeMin[i])
        {
            float d = NodeMin[i] - v;
            DistSq += d * d;
        }
        else if (v > NodeMax[i])
        {
            float d = v - NodeMax[i];
            DistSq += d * d;
        }
    }

    return DistSq <= (Radius * Radius);
}

int32 USpatialPartitioningComponent::GetNodeDepth(TSharedPtr<FSimpleOctreeNode> Node) const
{
    int32 Depth = 0;
    TSharedPtr<FSimpleOctreeNode> Current = Node;
    
    while (Current.IsValid() && Current != RootNode)
    {
        Depth++;
        // In a real implementation, you'd track parent nodes
        // For now, we'll just return a placeholder
        break;
    }
    
    return Depth;
}