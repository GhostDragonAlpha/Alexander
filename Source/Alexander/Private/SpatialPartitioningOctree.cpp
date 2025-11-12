// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpatialPartitioningOctree.h"
#include "OrbitalBody.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"

USpatialPartitioningOctree::USpatialPartitioningOctree()
{
    BodyCount = 0;
    NodeCount = 0;
    MaxDepth = 0;
    bEnableDebugVisualization = false;
}

void USpatialPartitioningOctree::Initialize(const FBox& Bounds)
{
    WorldBounds = Bounds;
    RootNode = FOctreeNode();
    RootNode.BoundingBox = Bounds;
    RootNode.Depth = 0;
    RootNode.Parent = nullptr;
    
    BodyCount = 0;
    NodeCount = 1; // Root node
    MaxDepth = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Spatial Partitioning Octree initialized with bounds: %s"), *Bounds.ToString());
}

void USpatialPartitioningOctree::AddBody(AOrbitalBody* Body)
{
    if (!Body)
    {
        return;
    }

    // Remove body if already exists
    RemoveBody(Body);

    // Add to root node
    AddBodyToNode(RootNode, Body);
    BodyCount++;
}

void USpatialPartitioningOctree::RemoveBody(AOrbitalBody* Body)
{
    if (!Body)
    {
        return;
    }

    // Find and remove from current node
    RemoveBodyFromNode(RootNode, Body);
}

void USpatialPartitioningOctree::UpdateBodyPosition(AOrbitalBody* Body, const FVector& NewPosition)
{
    if (!Body)
    {
        return;
    }

    // Remove from current location
    RemoveBody(Body);
    
    // Update body position
    Body->SetActorLocation(NewPosition, false, nullptr, ETeleportType::TeleportPhysics);
    
    // Add back at new position
    AddBody(Body);
}

FAlexanderSpatialQueryResult USpatialPartitioningOctree::QueryBodiesInRadius(const FVector& Position, float Radius) const
{
    FAlexanderSpatialQueryResult Result;
    Result.QueryPosition = Position;
    Result.QueryRadius = Radius;
    
    float StartTime = FPlatformTime::Seconds();
    
    // Query the octree
    QueryNode(RootNode, Position, Radius, Result.FoundBodies, Result.NodesVisited);
    
    Result.QueryTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f; // Convert to milliseconds
    
    return Result;
}

FAlexanderSpatialQueryResult USpatialPartitioningOctree::QueryBodiesInBox(const FBox& QueryBox) const
{
    FAlexanderSpatialQueryResult Result;
    Result.QueryPosition = QueryBox.GetCenter();
    Result.QueryRadius = QueryBox.GetExtent().Size();
    
    float StartTime = FPlatformTime::Seconds();
    
    // Query the octree
    QueryBoxNode(RootNode, QueryBox, Result.FoundBodies, Result.NodesVisited);
    
    Result.QueryTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f; // Convert to milliseconds
    
    return Result;
}

TArray<AOrbitalBody*> USpatialPartitioningOctree::GetAllBodies() const
{
    TArray<AOrbitalBody*> Bodies;
    GetAllBodiesFromNode(RootNode, Bodies);
    return Bodies;
}

void USpatialPartitioningOctree::Clear()
{
    ClearNode(RootNode);
    BodyCount = 0;
    NodeCount = 1; // Root node only
    MaxDepth = 0;
}

void USpatialPartitioningOctree::Rebuild()
{
    // Get all bodies
    TArray<AOrbitalBody*> AllBodies = GetAllBodies();
    
    // Clear current tree
    Clear();
    
    // Re-add all bodies
    for (AOrbitalBody* Body : AllBodies)
    {
        AddBody(Body);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Octree rebuilt with %d bodies"), AllBodies.Num());
}

void USpatialPartitioningOctree::GetStatistics(int32& OutBodyCount, int32& OutNodeCount, int32& OutMaxDepth) const
{
    OutBodyCount = BodyCount;
    OutNodeCount = NodeCount;
    OutMaxDepth = MaxDepth;
}

void USpatialPartitioningOctree::DrawDebugOctree() const
{
    if (!GetWorld() || !bEnableDebugVisualization)
    {
        return;
    }
    
    DrawDebugNode(RootNode);
}

void USpatialPartitioningOctree::AddBodyToNode(FOctreeNode& Node, AOrbitalBody* Body)
{
    if (!Body)
    {
        return;
    }

    FVector BodyPosition = Body->GetActorLocation();

    // Check if body is in node's bounding box
    if (!Node.BoundingBox.IsInsideOrOn(BodyPosition))
    {
        return;
    }

    // If node is leaf, add body here
    if (Node.IsLeaf())
    {
        Node.Bodies.Add(Body);

        // Split node if it has too many bodies
        if (Node.NeedsSplit())
        {
            SplitNode(Node);
        }
    }
    else
    {
        // Add to appropriate child
        for (FOctreeNode& Child : Node.Children)
        {
            if (Child.BoundingBox.IsInsideOrOn(BodyPosition))
            {
                AddBodyToNode(Child, Body);
                break;
            }
        }
    }
}

void USpatialPartitioningOctree::RemoveBodyFromNode(FOctreeNode& Node, AOrbitalBody* Body)
{
    if (!Body)
    {
        return;
    }

    // Remove from this node's body list
    Node.Bodies.Remove(Body);

    // Recursively remove from children
    for (FOctreeNode& Child : Node.Children)
    {
        RemoveBodyFromNode(Child, Body);
    }
}

void USpatialPartitioningOctree::SplitNode(FOctreeNode& Node)
{
    if (!Node.IsLeaf() || Node.Depth >= MAX_DEPTH)
    {
        return;
    }

    FVector Center = Node.BoundingBox.GetCenter();
    FVector Extent = Node.BoundingBox.GetExtent();

    // Create 8 children
    Node.Children.Reserve(8);
    
    for (int32 i = 0; i < 8; i++)
    {
        FOctreeNode Child;
        Child.Depth = Node.Depth + 1;
        Child.Parent = &Node;
        
        // Calculate child bounds
        FVector ChildMin(
            (i & 1) ? Center.X : Node.BoundingBox.Min.X,
            (i & 2) ? Center.Y : Node.BoundingBox.Min.Y,
            (i & 4) ? Center.Z : Node.BoundingBox.Min.Z
        );
        
        FVector ChildMax(
            (i & 1) ? Node.BoundingBox.Max.X : Center.X,
            (i & 2) ? Node.BoundingBox.Max.Y : Center.Y,
            (i & 4) ? Node.BoundingBox.Max.Z : Center.Z
        );
        
        Child.BoundingBox = FBox(ChildMin, ChildMax);
        Node.Children.Add(Child);
        
        NodeCount++;
        MaxDepth = FMath::Max(MaxDepth, Child.Depth);
    }

    // Redistribute bodies to children
    TArray<TWeakObjectPtr<AOrbitalBody>> BodiesToMove = Node.Bodies;
    Node.Bodies.Empty();

    for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : BodiesToMove)
    {
        if (AOrbitalBody* Body = BodyPtr.Get())
        {
            AddBodyToNode(Node, Body);
        }
    }
}

FOctreeNode* USpatialPartitioningOctree::FindNodeForBody(FOctreeNode& Node, const FVector& Position)
{
    if (!Node.BoundingBox.IsInsideOrOn(Position))
    {
        return nullptr;
    }

    // If leaf node, return it
    if (Node.IsLeaf())
    {
        return &Node;
    }

    // Search children
    for (FOctreeNode& Child : Node.Children)
    {
        FOctreeNode* FoundNode = FindNodeForBody(Child, Position);
        if (FoundNode)
        {
            return FoundNode;
        }
    }

    return nullptr;
}

void USpatialPartitioningOctree::QueryNode(const FOctreeNode& Node, const FVector& Position, float Radius, TArray<AOrbitalBody*>& FoundBodies, int32& NodesVisited) const
{
    NodesVisited++;

    // Check if query sphere intersects node bounds
    float DistSq = Node.BoundingBox.ComputeSquaredDistanceToPoint(Position);
    float RadiusSq = Radius * Radius;

    if (DistSq > RadiusSq)
    {
        return; // No intersection
    }

    // Check bodies in this node
    for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : Node.Bodies)
    {
        if (AOrbitalBody* Body = BodyPtr.Get())
        {
            float BodyDistSq = FVector::DistSquared(Position, Body->GetActorLocation());
            if (BodyDistSq <= RadiusSq)
            {
                FoundBodies.Add(Body);
            }
        }
    }

    // Recursively query children
    for (const FOctreeNode& Child : Node.Children)
    {
        QueryNode(Child, Position, Radius, FoundBodies, NodesVisited);
    }
}

void USpatialPartitioningOctree::QueryBoxNode(const FOctreeNode& Node, const FBox& QueryBox, TArray<AOrbitalBody*>& FoundBodies, int32& NodesVisited) const
{
    NodesVisited++;

    // Check if query box intersects node bounds
    if (!Node.BoundingBox.Intersect(QueryBox))
    {
        return; // No intersection
    }

    // Check bodies in this node
    for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : Node.Bodies)
    {
        if (AOrbitalBody* Body = BodyPtr.Get())
        {
            if (QueryBox.IsInsideOrOn(Body->GetActorLocation()))
            {
                FoundBodies.Add(Body);
            }
        }
    }

    // Recursively query children
    for (const FOctreeNode& Child : Node.Children)
    {
        QueryBoxNode(Child, QueryBox, FoundBodies, NodesVisited);
    }
}

void USpatialPartitioningOctree::GetAllBodiesFromNode(const FOctreeNode& Node, TArray<AOrbitalBody*>& Bodies) const
{
    // Add bodies from this node
    for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : Node.Bodies)
    {
        if (AOrbitalBody* Body = BodyPtr.Get())
        {
            Bodies.Add(Body);
        }
    }

    // Recursively add from children
    for (const FOctreeNode& Child : Node.Children)
    {
        GetAllBodiesFromNode(Child, Bodies);
    }
}

void USpatialPartitioningOctree::CountNodes(const FOctreeNode& Node, int32& Count, int32& Depth) const
{
    Count++;
    Depth = FMath::Max(Depth, Node.Depth);

    for (const FOctreeNode& Child : Node.Children)
    {
        CountNodes(Child, Count, Depth);
    }
}

void USpatialPartitioningOctree::DrawDebugNode(const FOctreeNode& Node) const
{
    if (!GetWorld() || !bEnableDebugVisualization)
    {
        return;
    }

    // Draw node bounding box
    DrawDebugBox(GetWorld(), Node.BoundingBox.GetCenter(), Node.BoundingBox.GetExtent(), FColor::Blue, false, -1.0f, 0, 1.0f);

    // Draw bodies in node
    for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : Node.Bodies)
    {
        if (const AOrbitalBody* Body = BodyPtr.Get())
        {
            DrawDebugSphere(GetWorld(), Body->GetActorLocation(), 100.0f, 16, FColor::Green, false, -1.0f, 0, 1.0f);
        }
    }

    // Recursively draw children
    for (const FOctreeNode& Child : Node.Children)
    {
        DrawDebugNode(Child);
    }
}

void USpatialPartitioningOctree::ClearNode(FOctreeNode& Node)
{
    Node.Bodies.Empty();
    Node.Children.Empty();
}