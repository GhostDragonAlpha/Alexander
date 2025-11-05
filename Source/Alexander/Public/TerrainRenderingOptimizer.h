// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "TerrainRenderingOptimizer.generated.h"

// Forward declarations
class UStaticMesh;
class UMaterialInterface;

/**
 * Nanite configuration for terrain meshes
 */
USTRUCT(BlueprintType)
struct FNaniteTerrainConfig
{
	GENERATED_BODY()

	// Enable Nanite for terrain tiles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nanite")
	bool bEnableNanite = true;

	// Fallback mesh for non-Nanite platforms
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nanite")
	UStaticMesh* FallbackMesh = nullptr;

	// Position precision (affects Nanite quality)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nanite")
	float PositionPrecision = 0.01f; // cm

	// Enable Nanite displacement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nanite")
	bool bEnableDisplacement = false;

	// Preserve area (better quality but more memory)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nanite")
	bool bPreserveArea = true;
};

/**
 * GPU instancing configuration for vegetation
 */
USTRUCT(BlueprintType)
struct FGPUInstancingConfig
{
	GENERATED_BODY()

	// Enable GPU instancing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instancing")
	bool bEnableInstancing = true;

	// Use hierarchical instancing (better culling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instancing")
	bool bUseHierarchicalInstancing = true;

	// Maximum instances per component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instancing")
	int32 MaxInstancesPerComponent = 10000;

	// Cull distance for instances (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instancing")
	float CullDistance = 1000000.0f; // 10km

	// Enable distance-based LOD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instancing")
	bool bEnableLOD = true;

	// LOD distances (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instancing")
	TArray<float> LODDistances = {50000.0f, 100000.0f, 200000.0f}; // 500m, 1km, 2km
};

/**
 * Frustum culling configuration
 */
USTRUCT(BlueprintType)
struct FFrustumCullingConfig
{
	GENERATED_BODY()

	// Enable frustum culling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
	bool bEnableFrustumCulling = true;

	// Enable occlusion culling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
	bool bEnableOcclusionCulling = true;

	// Culling margin (expand frustum by this amount)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
	float CullingMargin = 1000.0f; // cm

	// Update frequency (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
	float UpdateFrequency = 0.1f; // 10 times per second

	// Enable distance-based culling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
	bool bEnableDistanceCulling = true;

	// Maximum render distance (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
	float MaxRenderDistance = 5000000.0f; // 50km
};

/**
 * Draw call batching configuration
 */
USTRUCT(BlueprintType)
struct FDrawCallBatchingConfig
{
	GENERATED_BODY()

	// Enable draw call batching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Batching")
	bool bEnableBatching = true;

	// Batch by material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Batching")
	bool bBatchByMaterial = true;

	// Batch by LOD level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Batching")
	bool bBatchByLOD = true;

	// Maximum batch size (triangles)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Batching")
	int32 MaxBatchTriangles = 1000000;

	// Enable static mesh merging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Batching")
	bool bEnableMeshMerging = false;
};

/**
 * Instance batch for GPU instancing
 */
USTRUCT()
struct FInstanceBatch
{
	GENERATED_BODY()

	UPROPERTY()
	UStaticMesh* Mesh = nullptr;

	UPROPERTY()
	UMaterialInterface* Material = nullptr;

	UPROPERTY()
	TArray<FTransform> Instances;

	UPROPERTY()
	UHierarchicalInstancedStaticMeshComponent* InstanceComponent = nullptr;

	UPROPERTY()
	int32 LODLevel = 0;

	void Clear()
	{
		Instances.Empty();
		InstanceComponent = nullptr;
	}
};

/**
 * Terrain rendering optimizer
 * Implements Nanite, GPU instancing, frustum culling, and draw call batching
 */
UCLASS(BlueprintType)
class ALEXANDER_API UTerrainRenderingOptimizer : public UObject
{
	GENERATED_BODY()

public:
	UTerrainRenderingOptimizer();

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize the rendering optimizer
	 * @param InOwnerActor Actor that owns the terrain components
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization")
	void Initialize(AActor* InOwnerActor);

	/**
	 * Shutdown and cleanup
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization")
	void Shutdown();

	// ============================================================================
	// NANITE TERRAIN
	// ============================================================================

	/**
	 * Enable Nanite for a static mesh component
	 * @param MeshComponent Component to enable Nanite on
	 * @return True if Nanite was enabled successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization|Nanite")
	bool EnableNaniteForMesh(UStaticMeshComponent* MeshComponent);

	/**
	 * Check if Nanite is supported on current platform
	 * @return True if Nanite is supported
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization|Nanite")
	bool IsNaniteSupported() const;

	/**
	 * Convert terrain mesh to Nanite-enabled mesh
	 * @param SourceMesh Original mesh
	 * @return Nanite-enabled mesh or nullptr if conversion failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization|Nanite")
	UStaticMesh* ConvertToNaniteMesh(UStaticMesh* SourceMesh);

	// ============================================================================
	// GPU INSTANCING
	// ============================================================================

	/**
	 * Create instanced component for vegetation/detail objects
	 * @param Mesh Mesh to instance
	 * @param Material Material to apply
	 * @param LODLevel LOD level for this batch
	 * @return Created instance component
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization|Instancing")
	UHierarchicalInstancedStaticMeshComponent* CreateInstancedComponent(
		UStaticMesh* Mesh,
		UMaterialInterface* Material,
		int32 LODLevel
	);

	/**
	 * Add instances to batch
	 * @param Mesh Mesh to instance
	 * @param Material Material to apply
	 * @param Transforms Instance transforms
	 * @param LODLevel LOD level
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization|Instancing")
	void AddInstances(
		UStaticMesh* Mesh,
		UMaterialInterface* Material,
		const TArray<FTransform>& Transforms,
		int32 LODLevel = 0
	);

	/**
	 * Update all instance components
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization|Instancing")
	void UpdateInstanceComponents();

	/**
	 * Clear all instances
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization|Instancing")
	void ClearAllInstances();

	/**
	 * Get total instance count
	 * @return Total number of instances
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization|Instancing")
	int32 GetTotalInstanceCount() const;

	// ============================================================================
	// FRUSTUM CULLING
	// ============================================================================

	/**
	 * Update frustum culling for all components
	 * @param ViewLocation Camera location
	 * @param ViewRotation Camera rotation
	 * @param FOV Field of view in degrees
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization|Culling")
	void UpdateFrustumCulling(FVector ViewLocation, FRotator ViewRotation, float FOV);

	/**
	 * Check if bounds are in frustum
	 * @param Bounds Bounds to check
	 * @param ViewLocation Camera location
	 * @param ViewRotation Camera rotation
	 * @param FOV Field of view
	 * @return True if bounds are visible
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization|Culling")
	bool IsInFrustum(const FBox& Bounds, FVector ViewLocation, FRotator ViewRotation, float FOV) const;

	/**
	 * Enable/disable component based on culling
	 * @param Component Component to update
	 * @param bVisible Whether component should be visible
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization|Culling")
	void SetComponentCulled(UPrimitiveComponent* Component, bool bVisible);

	// ============================================================================
	// DRAW CALL BATCHING
	// ============================================================================

	/**
	 * Optimize draw calls by batching similar meshes
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization|Batching")
	void OptimizeDrawCalls();

	/**
	 * Get current draw call count estimate
	 * @return Estimated draw calls
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization|Batching")
	int32 GetEstimatedDrawCalls() const;

	/**
	 * Merge static meshes for batching
	 * @param Meshes Meshes to merge
	 * @param Material Material to apply
	 * @return Merged mesh
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization|Batching")
	UStaticMesh* MergeStaticMeshes(const TArray<UStaticMesh*>& Meshes, UMaterialInterface* Material);

	// ============================================================================
	// PERFORMANCE MONITORING
	// ============================================================================

	/**
	 * Get performance statistics
	 * @param OutDrawCalls Estimated draw calls
	 * @param OutTriangles Total triangles rendered
	 * @param OutInstances Total instances
	 * @param OutCulledObjects Number of culled objects
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization|Stats")
	void GetPerformanceStats(
		int32& OutDrawCalls,
		int32& OutTriangles,
		int32& OutInstances,
		int32& OutCulledObjects
	) const;

	/**
	 * Log performance statistics to console
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering Optimization|Stats")
	void LogPerformanceStats() const;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FNaniteTerrainConfig NaniteConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FGPUInstancingConfig InstancingConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FFrustumCullingConfig CullingConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FDrawCallBatchingConfig BatchingConfig;

protected:
	// Owner actor
	UPROPERTY()
	AActor* OwnerActor = nullptr;

	// Instance batches
	UPROPERTY()
	TArray<FInstanceBatch> InstanceBatches;

	// Tracked components for culling
	UPROPERTY()
	TArray<UPrimitiveComponent*> TrackedComponents;

	// Culling timer
	float CullingTimer = 0.0f;

	// Performance stats
	int32 CachedDrawCalls = 0;
	int32 CachedTriangles = 0;
	int32 CachedCulledObjects = 0;

	// ============================================================================
	// INTERNAL METHODS
	// ============================================================================

	/**
	 * Find or create instance batch
	 */
	FInstanceBatch* FindOrCreateBatch(UStaticMesh* Mesh, UMaterialInterface* Material, int32 LODLevel);

	/**
	 * Calculate frustum planes
	 */
	void CalculateFrustumPlanes(
		FVector ViewLocation,
		FRotator ViewRotation,
		float FOV,
		TArray<FPlane>& OutPlanes
	) const;

	/**
	 * Test box against frustum planes
	 */
	bool TestBoxAgainstFrustum(const FBox& Box, const TArray<FPlane>& Planes) const;

	/**
	 * Update performance statistics
	 */
	void UpdatePerformanceStats();
};
