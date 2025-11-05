// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TerrainTile.h"
#include "QuadTreeTileManager.generated.h"

/**
 * Quadtree node for hierarchical terrain tile management
 */
USTRUCT()
struct FQuadTreeNode
{
	GENERATED_BODY()

	// Node identification
	UPROPERTY()
	FIntPoint GridPosition; // Position in grid space

	UPROPERTY()
	int32 LODLevel = 0; // 0 = highest detail

	UPROPERTY()
	FVector2D WorldPosition; // Center position in world space

	UPROPERTY()
	float NodeSize = 1000.0f; // Size in meters

	// Tile data
	UPROPERTY()
	FTerrainTileData TileData;

	// Hierarchy
	TSharedPtr<FQuadTreeNode> Parent;
	TArray<TSharedPtr<FQuadTreeNode>> Children; // 4 children when subdivided

	// State
	UPROPERTY()
	bool bIsSubdivided = false;

	UPROPERTY()
	bool bIsVisible = false;

	UPROPERTY()
	bool bIsLoaded = false;

	// Mesh component for rendering
	UPROPERTY()
	class UProceduralMeshComponent* MeshComponent = nullptr;

	// Constructor
	FQuadTreeNode()
		: GridPosition(0, 0)
		, LODLevel(0)
		, WorldPosition(FVector2D::ZeroVector)
		, NodeSize(1000.0f)
		, bIsSubdivided(false)
		, bIsVisible(false)
		, bIsLoaded(false)
		, MeshComponent(nullptr)
	{
	}

	// Get bounds of this node
	FBox2D GetBounds() const
	{
		float HalfSize = NodeSize * 0.5f;
		return FBox2D(
			WorldPosition - FVector2D(HalfSize, HalfSize),
			WorldPosition + FVector2D(HalfSize, HalfSize)
		);
	}

	// Get distance to a point
	float GetDistanceToPoint(const FVector2D& Point) const
	{
		return FVector2D::Distance(WorldPosition, Point);
	}
};

/**
 * LOD configuration for quadtree
 */
USTRUCT(BlueprintType)
struct FQuadTreeLODConfig
{
	GENERATED_BODY()

	// Maximum LOD level (0 = highest detail)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	int32 MaxLODLevel = 6;

	// LOD distance thresholds (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	TArray<float> LODDistances;

	// Base tile size at LOD 0 (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	float BaseTileSize = 500.0f;

	// Tile resolution (vertices per side)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	int32 TileResolution = 64;

	// View distance for culling (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	float ViewDistance = 50000.0f;

	// Constructor with default LOD distances
	FQuadTreeLODConfig()
	{
		MaxLODLevel = 6;
		BaseTileSize = 500.0f;
		TileResolution = 64;
		ViewDistance = 50000.0f;

		// Default LOD distances based on design document
		LODDistances = {
			500.0f,    // LOD 0: 0-500m
			2000.0f,   // LOD 1: 500m-2km
			10000.0f,  // LOD 2: 2km-10km
			50000.0f,  // LOD 3: 10km-50km
			200000.0f, // LOD 4: 50km-200km
			500000.0f  // LOD 5: 200km+
		};
	}
};

/**
 * Quadtree-based terrain tile manager
 * Manages hierarchical LOD and streaming for planetary terrain
 */
UCLASS(BlueprintType)
class ALEXANDER_API UQuadTreeTileManager : public UObject
{
	GENERATED_BODY()

public:
	UQuadTreeTileManager();

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize the quadtree with root tile
	 * @param RootPosition Center position of root tile
	 * @param RootSize Size of root tile (covers entire terrain area)
	 * @param Config LOD configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	void Initialize(FVector2D RootPosition, float RootSize, const FQuadTreeLODConfig& Config);

	/**
	 * Shutdown and cleanup all tiles
	 */
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	void Shutdown();

	// ============================================================================
	// UPDATE
	// ============================================================================

	/**
	 * Update visible tiles based on viewer position
	 * @param ViewerPosition Current viewer position in world space
	 * @param DeltaTime Time since last update
	 */
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	void UpdateVisibleTiles(FVector ViewerPosition, float DeltaTime);

	/**
	 * Force update of all tiles (expensive, use sparingly)
	 */
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	void ForceUpdateAllTiles();

	// ============================================================================
	// LOD CALCULATION
	// ============================================================================

	/**
	 * Calculate appropriate LOD level for a tile based on distance
	 * @param TileCenter Center position of tile
	 * @param ViewerPosition Viewer position
	 * @return LOD level (0 = highest detail)
	 */
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	int32 CalculateLODLevel(FVector2D TileCenter, FVector2D ViewerPosition) const;

	/**
	 * Check if tile should be subdivided based on distance
	 * @param Node Node to check
	 * @param ViewerPosition Viewer position
	 * @return True if node should be subdivided
	 */
	bool ShouldSubdivide(const TSharedPtr<FQuadTreeNode>& Node, const FVector2D& ViewerPosition) const;

	/**
	 * Check if children should be merged back to parent
	 * @param Node Parent node to check
	 * @param ViewerPosition Viewer position
	 * @return True if children should be merged
	 */
	bool ShouldMerge(const TSharedPtr<FQuadTreeNode>& Node, const FVector2D& ViewerPosition) const;

	// ============================================================================
	// TILE MANAGEMENT
	// ============================================================================

	/**
	 * Subdivide a tile into 4 children
	 * @param ParentNode Parent node to subdivide
	 */
	void SubdivideTile(TSharedPtr<FQuadTreeNode> ParentNode);

	/**
	 * Merge 4 child tiles back to parent
	 * @param ParentNode Parent node whose children will be merged
	 */
	void MergeTiles(TSharedPtr<FQuadTreeNode> ParentNode);

	/**
	 * Update visibility culling for all tiles
	 * @param ViewerPosition Viewer position
	 */
	void UpdateVisibilityCulling(const FVector2D& ViewerPosition);

	/**
	 * Check if tile is within view distance
	 * @param Node Node to check
	 * @param ViewerPosition Viewer position
	 * @return True if tile is visible
	 */
	bool IsTileVisible(const TSharedPtr<FQuadTreeNode>& Node, const FVector2D& ViewerPosition) const;

	// ============================================================================
	// QUERIES
	// ============================================================================

	/**
	 * Get all active (leaf) tiles
	 * @return Array of active tile nodes
	 */
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	TArray<FTerrainTileData> GetActiveTiles() const;

	/**
	 * Get number of active tiles
	 * @return Count of active tiles
	 */
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	int32 GetActiveTileCount() const;

	/**
	 * Get tile at world position
	 * @param WorldPosition Position to query
	 * @return Tile node at position, or nullptr if not found
	 */
	TSharedPtr<FQuadTreeNode> GetTileAtPosition(FVector2D WorldPosition) const;

	// ============================================================================
	// DEBUG
	// ============================================================================

	/**
	 * Draw debug visualization of quadtree
	 * @param World World context
	 * @param Duration How long to display
	 */
	UFUNCTION(BlueprintCallable, Category = "QuadTree|Debug")
	void DrawDebugQuadTree(UWorld* World, float Duration = 0.0f) const;

	/**
	 * Get debug statistics
	 * @param OutTotalNodes Total number of nodes
	 * @param OutActiveNodes Number of active (leaf) nodes
	 * @param OutSubdividedNodes Number of subdivided nodes
	 */
	UFUNCTION(BlueprintCallable, Category = "QuadTree|Debug")
	void GetDebugStats(int32& OutTotalNodes, int32& OutActiveNodes, int32& OutSubdividedNodes) const;

	// ============================================================================
	// PROPERTIES
	// ============================================================================

	// LOD configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FQuadTreeLODConfig LODConfig;

	// Terrain generation configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FTerrainGenerationConfig TerrainConfig;

	// Rendering optimizer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	class UTerrainRenderingOptimizer* RenderingOptimizer = nullptr;

	// Owner actor (for mesh component creation)
	UPROPERTY()
	AActor* OwnerActor = nullptr;

private:
	// Root node of quadtree
	TSharedPtr<FQuadTreeNode> RootNode;

	// Cache of all active (leaf) nodes for quick access
	TArray<TSharedPtr<FQuadTreeNode>> ActiveNodes;

	// Last viewer position (for optimization)
	FVector2D LastViewerPosition;

	// Helper: Create a new node
	TSharedPtr<FQuadTreeNode> CreateNode(
		FIntPoint GridPos,
		int32 LODLevel,
		FVector2D WorldPos,
		float Size,
		TSharedPtr<FQuadTreeNode> Parent
	);

	// Helper: Recursively update node and children
	void UpdateNodeRecursive(TSharedPtr<FQuadTreeNode> Node, const FVector2D& ViewerPosition);

	// Helper: Rebuild active nodes cache
	void RebuildActiveNodesCache();

	// Helper: Recursively collect active nodes
	void CollectActiveNodesRecursive(const TSharedPtr<FQuadTreeNode>& Node, TArray<TSharedPtr<FQuadTreeNode>>& OutNodes) const;

	// Helper: Recursively draw debug for node
	void DrawDebugNodeRecursive(const TSharedPtr<FQuadTreeNode>& Node, UWorld* World, float Duration) const;

	// Helper: Recursively count nodes
	void CountNodesRecursive(const TSharedPtr<FQuadTreeNode>& Node, int32& TotalNodes, int32& ActiveNodes, int32& SubdividedNodes) const;
};
