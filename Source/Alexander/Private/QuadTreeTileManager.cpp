// Copyright Epic Games, Inc. All Rights Reserved.

#include "QuadTreeTileManager.h"
#include "TerrainRenderingOptimizer.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "ProceduralMeshComponent.h"

UQuadTreeTileManager::UQuadTreeTileManager()
{
	LastViewerPosition = FVector2D::ZeroVector;
}

void UQuadTreeTileManager::Initialize(FVector2D RootPosition, float RootSize, const FQuadTreeLODConfig& Config)
{
	// Store configuration
	LODConfig = Config;

	// Create root node
	RootNode = CreateNode(
		FIntPoint(0, 0),
		LODConfig.MaxLODLevel, // Root starts at lowest detail
		RootPosition,
		RootSize,
		nullptr
	);

	// Initialize active nodes cache
	RebuildActiveNodesCache();

	// Initialize rendering optimizer if owner is set
	if (OwnerActor && !RenderingOptimizer)
	{
		RenderingOptimizer = NewObject<UTerrainRenderingOptimizer>(this);
		RenderingOptimizer->Initialize(OwnerActor);
		UE_LOG(LogTemp, Log, TEXT("QuadTreeTileManager: Initialized rendering optimizer"));
	}

	UE_LOG(LogTemp, Log, TEXT("QuadTreeTileManager initialized: Root at (%.1f, %.1f), Size: %.1f"),
		RootPosition.X, RootPosition.Y, RootSize);
}

void UQuadTreeTileManager::Shutdown()
{
	// Shutdown rendering optimizer
	if (RenderingOptimizer)
	{
		RenderingOptimizer->Shutdown();
		RenderingOptimizer = nullptr;
	}

	// Clear all nodes
	RootNode.Reset();
	ActiveNodes.Empty();

	UE_LOG(LogTemp, Log, TEXT("QuadTreeTileManager shutdown complete"));
}

void UQuadTreeTileManager::UpdateVisibleTiles(FVector ViewerPosition, float DeltaTime)
{
	if (!RootNode.IsValid())
	{
		return;
	}

	// Convert 3D viewer position to 2D (ignore Z for terrain queries)
	FVector2D ViewerPosition2D(ViewerPosition.X, ViewerPosition.Y);

	// Update quadtree hierarchy
	UpdateNodeRecursive(RootNode, ViewerPosition2D);

	// Update visibility culling
	UpdateVisibilityCulling(ViewerPosition2D);

	// Rebuild active nodes cache if needed
	RebuildActiveNodesCache();

	// Update rendering optimizer with frustum culling
	if (RenderingOptimizer && OwnerActor)
	{
		// Get camera info from player controller
		APlayerController* PC = OwnerActor->GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			FVector CameraLocation;
			FRotator CameraRotation;
			PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

			// Update frustum culling
			RenderingOptimizer->UpdateFrustumCulling(CameraLocation, CameraRotation, 90.0f);
		}
	}

	// Store last viewer position
	LastViewerPosition = ViewerPosition2D;
}

void UQuadTreeTileManager::ForceUpdateAllTiles()
{
	if (!RootNode.IsValid())
	{
		return;
	}

	// Rebuild entire cache
	RebuildActiveNodesCache();

	UE_LOG(LogTemp, Log, TEXT("Force updated all tiles: %d active tiles"), ActiveNodes.Num());
}

int32 UQuadTreeTileManager::CalculateLODLevel(FVector2D TileCenter, FVector2D ViewerPosition) const
{
	float Distance = FVector2D::Distance(TileCenter, ViewerPosition);

	// Find appropriate LOD level based on distance
	for (int32 LOD = 0; LOD < LODConfig.LODDistances.Num(); ++LOD)
	{
		if (Distance < LODConfig.LODDistances[LOD])
		{
			return LOD;
		}
	}

	// Return maximum LOD if beyond all thresholds
	return LODConfig.MaxLODLevel;
}

bool UQuadTreeTileManager::ShouldSubdivide(const TSharedPtr<FQuadTreeNode>& Node, const FVector2D& ViewerPosition) const
{
	if (!Node.IsValid() || Node->LODLevel <= 0)
	{
		// Already at highest detail
		return false;
	}

	// Calculate distance to viewer
	float Distance = Node->GetDistanceToPoint(ViewerPosition);

	// Get target LOD for this distance
	int32 TargetLOD = CalculateLODLevel(Node->WorldPosition, ViewerPosition);

	// Subdivide if target LOD is higher detail than current
	return TargetLOD < Node->LODLevel;
}

bool UQuadTreeTileManager::ShouldMerge(const TSharedPtr<FQuadTreeNode>& Node, const FVector2D& ViewerPosition) const
{
	if (!Node.IsValid() || !Node->bIsSubdivided)
	{
		return false;
	}

	// Calculate distance to viewer
	float Distance = Node->GetDistanceToPoint(ViewerPosition);

	// Get target LOD for this distance
	int32 TargetLOD = CalculateLODLevel(Node->WorldPosition, ViewerPosition);

	// Merge if target LOD is lower detail than current children
	// Children are at LODLevel - 1, so merge if target >= current level
	return TargetLOD >= Node->LODLevel;
}

void UQuadTreeTileManager::SubdivideTile(TSharedPtr<FQuadTreeNode> ParentNode)
{
	if (!ParentNode.IsValid() || ParentNode->bIsSubdivided || ParentNode->LODLevel <= 0)
	{
		return;
	}

	// Calculate child properties
	int32 ChildLOD = ParentNode->LODLevel - 1;
	float ChildSize = ParentNode->NodeSize * 0.5f;
	float QuarterSize = ChildSize * 0.5f;

	// Create 4 children
	ParentNode->Children.SetNum(4);

	// Child 0: Bottom-left
	ParentNode->Children[0] = CreateNode(
		FIntPoint(ParentNode->GridPosition.X * 2, ParentNode->GridPosition.Y * 2),
		ChildLOD,
		ParentNode->WorldPosition + FVector2D(-QuarterSize, -QuarterSize),
		ChildSize,
		ParentNode
	);

	// Child 1: Bottom-right
	ParentNode->Children[1] = CreateNode(
		FIntPoint(ParentNode->GridPosition.X * 2 + 1, ParentNode->GridPosition.Y * 2),
		ChildLOD,
		ParentNode->WorldPosition + FVector2D(QuarterSize, -QuarterSize),
		ChildSize,
		ParentNode
	);

	// Child 2: Top-left
	ParentNode->Children[2] = CreateNode(
		FIntPoint(ParentNode->GridPosition.X * 2, ParentNode->GridPosition.Y * 2 + 1),
		ChildLOD,
		ParentNode->WorldPosition + FVector2D(-QuarterSize, QuarterSize),
		ChildSize,
		ParentNode
	);

	// Child 3: Top-right
	ParentNode->Children[3] = CreateNode(
		FIntPoint(ParentNode->GridPosition.X * 2 + 1, ParentNode->GridPosition.Y * 2 + 1),
		ChildLOD,
		ParentNode->WorldPosition + FVector2D(QuarterSize, QuarterSize),
		ChildSize,
		ParentNode
	);

	ParentNode->bIsSubdivided = true;
}

void UQuadTreeTileManager::MergeTiles(TSharedPtr<FQuadTreeNode> ParentNode)
{
	if (!ParentNode.IsValid() || !ParentNode->bIsSubdivided)
	{
		return;
	}

	// Clean up children
	for (TSharedPtr<FQuadTreeNode>& Child : ParentNode->Children)
	{
		if (Child.IsValid())
		{
			// Recursively merge children if they are subdivided
			if (Child->bIsSubdivided)
			{
				MergeTiles(Child);
			}

			// Destroy mesh component if exists
			if (Child->MeshComponent && Child->MeshComponent->IsValidLowLevel())
			{
				Child->MeshComponent->DestroyComponent();
				Child->MeshComponent = nullptr;
			}

			// Clear tile data
			Child->TileData.Clear();
		}
	}

	// Clear children array
	ParentNode->Children.Empty();
	ParentNode->bIsSubdivided = false;
}

void UQuadTreeTileManager::UpdateVisibilityCulling(const FVector2D& ViewerPosition)
{
	for (TSharedPtr<FQuadTreeNode>& Node : ActiveNodes)
	{
		if (Node.IsValid())
		{
			Node->bIsVisible = IsTileVisible(Node, ViewerPosition);
		}
	}
}

bool UQuadTreeTileManager::IsTileVisible(const TSharedPtr<FQuadTreeNode>& Node, const FVector2D& ViewerPosition) const
{
	if (!Node.IsValid())
	{
		return false;
	}

	// Simple distance-based culling
	float Distance = Node->GetDistanceToPoint(ViewerPosition);

	// Tile is visible if within view distance
	// Add node size to account for tile extent
	return Distance < (LODConfig.ViewDistance + Node->NodeSize);
}

TArray<FTerrainTileData> UQuadTreeTileManager::GetActiveTiles() const
{
	TArray<FTerrainTileData> Tiles;
	Tiles.Reserve(ActiveNodes.Num());

	for (const TSharedPtr<FQuadTreeNode>& Node : ActiveNodes)
	{
		if (Node.IsValid() && Node->bIsVisible)
		{
			Tiles.Add(Node->TileData);
		}
	}

	return Tiles;
}

int32 UQuadTreeTileManager::GetActiveTileCount() const
{
	return ActiveNodes.Num();
}

TSharedPtr<FQuadTreeNode> UQuadTreeTileManager::GetTileAtPosition(FVector2D WorldPosition) const
{
	if (!RootNode.IsValid())
	{
		return nullptr;
	}

	// Start from root and traverse down
	TSharedPtr<FQuadTreeNode> CurrentNode = RootNode;

	while (CurrentNode.IsValid())
	{
		// Check if position is within this node's bounds
		FBox2D Bounds = CurrentNode->GetBounds();
		if (!Bounds.IsInside(WorldPosition))
		{
			return nullptr;
		}

		// If not subdivided, this is the leaf node
		if (!CurrentNode->bIsSubdivided)
		{
			return CurrentNode;
		}

		// Find which child contains the position
		bool bFound = false;
		for (const TSharedPtr<FQuadTreeNode>& Child : CurrentNode->Children)
		{
			if (Child.IsValid())
			{
				FBox2D ChildBounds = Child->GetBounds();
				if (ChildBounds.IsInside(WorldPosition))
				{
					CurrentNode = Child;
					bFound = true;
					break;
				}
			}
		}

		if (!bFound)
		{
			break;
		}
	}

	return CurrentNode;
}

void UQuadTreeTileManager::DrawDebugQuadTree(UWorld* World, float Duration) const
{
	if (!World || !RootNode.IsValid())
	{
		return;
	}

	DrawDebugNodeRecursive(RootNode, World, Duration);
}

void UQuadTreeTileManager::GetDebugStats(int32& OutTotalNodes, int32& OutActiveNodes, int32& OutSubdividedNodes) const
{
	OutTotalNodes = 0;
	OutActiveNodes = 0;
	OutSubdividedNodes = 0;

	if (RootNode.IsValid())
	{
		CountNodesRecursive(RootNode, OutTotalNodes, OutActiveNodes, OutSubdividedNodes);
	}
}

TSharedPtr<FQuadTreeNode> UQuadTreeTileManager::CreateNode(
	FIntPoint GridPos,
	int32 LODLevel,
	FVector2D WorldPos,
	float Size,
	TSharedPtr<FQuadTreeNode> Parent)
{
	TSharedPtr<FQuadTreeNode> Node = MakeShared<FQuadTreeNode>();

	Node->GridPosition = GridPos;
	Node->LODLevel = LODLevel;
	Node->WorldPosition = WorldPos;
	Node->NodeSize = Size;
	Node->Parent = Parent;
	Node->bIsSubdivided = false;
	Node->bIsVisible = false;
	Node->bIsLoaded = false;

	// Initialize tile data
	Node->TileData.WorldPosition = WorldPos;
	Node->TileData.TileSize = Size;
	Node->TileData.LODLevel = LODLevel;
	Node->TileData.Resolution = LODConfig.TileResolution;

	return Node;
}

void UQuadTreeTileManager::UpdateNodeRecursive(TSharedPtr<FQuadTreeNode> Node, const FVector2D& ViewerPosition)
{
	if (!Node.IsValid())
	{
		return;
	}

	// Check if this node should be subdivided
	if (ShouldSubdivide(Node, ViewerPosition))
	{
		if (!Node->bIsSubdivided)
		{
			SubdivideTile(Node);
		}

		// Update children recursively
		for (TSharedPtr<FQuadTreeNode>& Child : Node->Children)
		{
			UpdateNodeRecursive(Child, ViewerPosition);
		}
	}
	// Check if children should be merged
	else if (ShouldMerge(Node, ViewerPosition))
	{
		MergeTiles(Node);
	}
	// Update children if already subdivided
	else if (Node->bIsSubdivided)
	{
		for (TSharedPtr<FQuadTreeNode>& Child : Node->Children)
		{
			UpdateNodeRecursive(Child, ViewerPosition);
		}
	}
}

void UQuadTreeTileManager::RebuildActiveNodesCache()
{
	ActiveNodes.Empty();

	if (RootNode.IsValid())
	{
		CollectActiveNodesRecursive(RootNode, ActiveNodes);
	}
}

void UQuadTreeTileManager::CollectActiveNodesRecursive(
	const TSharedPtr<FQuadTreeNode>& Node,
	TArray<TSharedPtr<FQuadTreeNode>>& OutNodes) const
{
	if (!Node.IsValid())
	{
		return;
	}

	// If node is subdivided, collect children
	if (Node->bIsSubdivided)
	{
		for (const TSharedPtr<FQuadTreeNode>& Child : Node->Children)
		{
			CollectActiveNodesRecursive(Child, OutNodes);
		}
	}
	else
	{
		// This is a leaf node (active tile)
		OutNodes.Add(Node);
	}
}

void UQuadTreeTileManager::DrawDebugNodeRecursive(
	const TSharedPtr<FQuadTreeNode>& Node,
	UWorld* World,
	float Duration) const
{
	if (!Node.IsValid() || !World)
	{
		return;
	}

	// Get node bounds
	FBox2D Bounds = Node->GetBounds();

	// Choose color based on LOD level
	FColor Color;
	switch (Node->LODLevel)
	{
	case 0: Color = FColor::Red; break;
	case 1: Color = FColor::Orange; break;
	case 2: Color = FColor::Yellow; break;
	case 3: Color = FColor::Green; break;
	case 4: Color = FColor::Cyan; break;
	case 5: Color = FColor::Blue; break;
	default: Color = FColor::Magenta; break;
	}

	// Make invisible tiles darker
	if (!Node->bIsVisible)
	{
		Color = Color.WithAlpha(64);
	}

	// Draw box at ground level (Z=0)
	FVector Min(Bounds.Min.X, Bounds.Min.Y, 0.0f);
	FVector Max(Bounds.Max.X, Bounds.Max.Y, 100.0f);

	DrawDebugBox(World, (Min + Max) * 0.5f, (Max - Min) * 0.5f, Color, false, Duration, 0, 2.0f);

	// Draw LOD level text
	FVector Center(Node->WorldPosition.X, Node->WorldPosition.Y, 200.0f);
	DrawDebugString(World, Center, FString::Printf(TEXT("LOD %d"), Node->LODLevel), nullptr, Color, Duration, true);

	// Recursively draw children
	if (Node->bIsSubdivided)
	{
		for (const TSharedPtr<FQuadTreeNode>& Child : Node->Children)
		{
			DrawDebugNodeRecursive(Child, World, Duration);
		}
	}
}

void UQuadTreeTileManager::CountNodesRecursive(
	const TSharedPtr<FQuadTreeNode>& Node,
	int32& TotalNodes,
	int32& ActiveNodes,
	int32& SubdividedNodes) const
{
	if (!Node.IsValid())
	{
		return;
	}

	TotalNodes++;

	if (Node->bIsSubdivided)
	{
		SubdividedNodes++;

		for (const TSharedPtr<FQuadTreeNode>& Child : Node->Children)
		{
			CountNodesRecursive(Child, TotalNodes, ActiveNodes, SubdividedNodes);
		}
	}
	else
	{
		ActiveNodes++;
	}
}
