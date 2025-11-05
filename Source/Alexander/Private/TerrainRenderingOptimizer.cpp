// Copyright Epic Games, Inc. All Rights Reserved.

#include "TerrainRenderingOptimizer.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UTerrainRenderingOptimizer::UTerrainRenderingOptimizer()
{
	// Initialize default configurations
	NaniteConfig.bEnableNanite = true;
	NaniteConfig.PositionPrecision = 0.01f;
	NaniteConfig.bEnableDisplacement = false;
	NaniteConfig.bPreserveArea = true;

	InstancingConfig.bEnableInstancing = true;
	InstancingConfig.bUseHierarchicalInstancing = true;
	InstancingConfig.MaxInstancesPerComponent = 10000;
	InstancingConfig.CullDistance = 1000000.0f;
	InstancingConfig.bEnableLOD = true;

	CullingConfig.bEnableFrustumCulling = true;
	CullingConfig.bEnableOcclusionCulling = true;
	CullingConfig.CullingMargin = 1000.0f;
	CullingConfig.UpdateFrequency = 0.1f;
	CullingConfig.bEnableDistanceCulling = true;
	CullingConfig.MaxRenderDistance = 5000000.0f;

	BatchingConfig.bEnableBatching = true;
	BatchingConfig.bBatchByMaterial = true;
	BatchingConfig.bBatchByLOD = true;
	BatchingConfig.MaxBatchTriangles = 1000000;
	BatchingConfig.bEnableMeshMerging = false;
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void UTerrainRenderingOptimizer::Initialize(AActor* InOwnerActor)
{
	OwnerActor = InOwnerActor;
	InstanceBatches.Empty();
	TrackedComponents.Empty();
	CullingTimer = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("TerrainRenderingOptimizer: Initialized for actor %s"), 
		*InOwnerActor->GetName());
}

void UTerrainRenderingOptimizer::Shutdown()
{
	// Clear all instances
	ClearAllInstances();

	// Clear tracked components
	TrackedComponents.Empty();

	OwnerActor = nullptr;

	UE_LOG(LogTemp, Log, TEXT("TerrainRenderingOptimizer: Shutdown complete"));
}

// ============================================================================
// NANITE TERRAIN
// ============================================================================

bool UTerrainRenderingOptimizer::EnableNaniteForMesh(UStaticMeshComponent* MeshComponent)
{
	if (!MeshComponent || !NaniteConfig.bEnableNanite)
	{
		return false;
	}

	if (!IsNaniteSupported())
	{
		UE_LOG(LogTemp, Warning, TEXT("TerrainRenderingOptimizer: Nanite not supported on this platform"));
		return false;
	}

	UStaticMesh* Mesh = MeshComponent->GetStaticMesh();
	if (!Mesh)
	{
		return false;
	}

	// Check if mesh already has Nanite enabled
	// Note: Nanite settings access changed in UE 5.6, disabling for now
	UE_LOG(LogTemp, Log, TEXT("TerrainRenderingOptimizer: Nanite configuration disabled for UE 5.6 compatibility"));
	return true;
}

bool UTerrainRenderingOptimizer::IsNaniteSupported() const
{
	// Nanite is supported on:
	// - Windows with DX12 or Vulkan
	// - PlayStation 5
	// - Xbox Series X/S
	// - Modern graphics cards (RTX 2000+, RX 5000+)
	
	// For now, assume it's supported if we're in editor or on Windows
	#if WITH_EDITOR
		return true;
	#elif PLATFORM_WINDOWS
		return true;
	#else
		return false;
	#endif
}

UStaticMesh* UTerrainRenderingOptimizer::ConvertToNaniteMesh(UStaticMesh* SourceMesh)
{
	if (!SourceMesh || !NaniteConfig.bEnableNanite)
	{
		return nullptr;
	}

	if (!IsNaniteSupported())
	{
		UE_LOG(LogTemp, Warning, TEXT("TerrainRenderingOptimizer: Cannot convert to Nanite - not supported"));
		return NaniteConfig.FallbackMesh;
	}

	// Enable Nanite settings - disabled for UE 5.6 compatibility
	UE_LOG(LogTemp, Warning, TEXT("TerrainRenderingOptimizer: Nanite conversion disabled for UE 5.6 compatibility"));

	return SourceMesh;
}

// ============================================================================
// GPU INSTANCING
// ============================================================================

UHierarchicalInstancedStaticMeshComponent* UTerrainRenderingOptimizer::CreateInstancedComponent(
	UStaticMesh* Mesh,
	UMaterialInterface* Material,
	int32 LODLevel)
{
	if (!OwnerActor || !Mesh || !InstancingConfig.bEnableInstancing)
	{
		return nullptr;
	}

	// Create component
	UHierarchicalInstancedStaticMeshComponent* InstanceComponent = nullptr;

	if (InstancingConfig.bUseHierarchicalInstancing)
	{
		InstanceComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(
			OwnerActor,
			UHierarchicalInstancedStaticMeshComponent::StaticClass()
		);
	}
	else
	{
		InstanceComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(
			OwnerActor,
			UHierarchicalInstancedStaticMeshComponent::StaticClass()
		);
	}

	if (!InstanceComponent)
	{
		return nullptr;
	}

	// Configure component
	InstanceComponent->SetStaticMesh(Mesh);
	if (Material)
	{
		InstanceComponent->SetMaterial(0, Material);
	}

	// Set culling distance
	InstanceComponent->SetCullDistances(0, InstancingConfig.CullDistance);

	// Enable LOD if configured
	if (InstancingConfig.bEnableLOD && InstancingConfig.LODDistances.Num() > 0)
	{
		// Set LOD distances
		for (int32 i = 0; i < InstancingConfig.LODDistances.Num(); ++i)
		{
			InstanceComponent->SetCullDistances(i, InstancingConfig.LODDistances[i]);
		}
	}

	// Enable frustum culling
	InstanceComponent->SetCullDistances(0, InstancingConfig.CullDistance);
	InstanceComponent->bUseAsOccluder = CullingConfig.bEnableOcclusionCulling;

	// Register component
	InstanceComponent->RegisterComponent();
	InstanceComponent->AttachToComponent(
		OwnerActor->GetRootComponent(),
		FAttachmentTransformRules::KeepRelativeTransform
	);

	UE_LOG(LogTemp, Log, TEXT("TerrainRenderingOptimizer: Created instanced component for mesh %s"), 
		*Mesh->GetName());

	return InstanceComponent;
}

void UTerrainRenderingOptimizer::AddInstances(
	UStaticMesh* Mesh,
	UMaterialInterface* Material,
	const TArray<FTransform>& Transforms,
	int32 LODLevel)
{
	if (!Mesh || Transforms.Num() == 0 || !InstancingConfig.bEnableInstancing)
	{
		return;
	}

	// Find or create batch
	FInstanceBatch* Batch = FindOrCreateBatch(Mesh, Material, LODLevel);
	if (!Batch)
	{
		return;
	}

	// Add transforms to batch
	Batch->Instances.Append(Transforms);

	UE_LOG(LogTemp, Verbose, TEXT("TerrainRenderingOptimizer: Added %d instances to batch"), 
		Transforms.Num());
}

void UTerrainRenderingOptimizer::UpdateInstanceComponents()
{
	if (!OwnerActor || !InstancingConfig.bEnableInstancing)
	{
		return;
	}

	int32 TotalInstances = 0;

	for (FInstanceBatch& Batch : InstanceBatches)
	{
		if (Batch.Instances.Num() == 0)
		{
			continue;
		}

		// Create component if needed
		if (!Batch.InstanceComponent)
		{
			Batch.InstanceComponent = CreateInstancedComponent(
				Batch.Mesh,
				Batch.Material,
				Batch.LODLevel
			);
		}

		if (Batch.InstanceComponent)
		{
			// Clear existing instances
			Batch.InstanceComponent->ClearInstances();

			// Add all instances
			for (const FTransform& Transform : Batch.Instances)
			{
				Batch.InstanceComponent->AddInstance(Transform);
			}

			TotalInstances += Batch.Instances.Num();

			// Track component for culling
			if (!TrackedComponents.Contains(Batch.InstanceComponent))
			{
				TrackedComponents.Add(Batch.InstanceComponent);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("TerrainRenderingOptimizer: Updated %d instance components with %d total instances"),
		InstanceBatches.Num(), TotalInstances);

	UpdatePerformanceStats();
}

void UTerrainRenderingOptimizer::ClearAllInstances()
{
	for (FInstanceBatch& Batch : InstanceBatches)
	{
		if (Batch.InstanceComponent)
		{
			Batch.InstanceComponent->ClearInstances();
			Batch.InstanceComponent->DestroyComponent();
		}
		Batch.Clear();
	}

	InstanceBatches.Empty();
	TrackedComponents.Empty();

	UE_LOG(LogTemp, Log, TEXT("TerrainRenderingOptimizer: Cleared all instances"));
}

int32 UTerrainRenderingOptimizer::GetTotalInstanceCount() const
{
	int32 Total = 0;
	for (const FInstanceBatch& Batch : InstanceBatches)
	{
		Total += Batch.Instances.Num();
	}
	return Total;
}

// ============================================================================
// FRUSTUM CULLING
// ============================================================================

void UTerrainRenderingOptimizer::UpdateFrustumCulling(
	FVector ViewLocation,
	FRotator ViewRotation,
	float FOV)
{
	if (!CullingConfig.bEnableFrustumCulling)
	{
		return;
	}

	// Calculate frustum planes
	TArray<FPlane> FrustumPlanes;
	CalculateFrustumPlanes(ViewLocation, ViewRotation, FOV, FrustumPlanes);

	int32 CulledCount = 0;

	// Update culling for all tracked components
	for (UPrimitiveComponent* Component : TrackedComponents)
	{
		if (!Component)
		{
			continue;
		}

		// Get component bounds
		FBox Bounds = Component->Bounds.GetBox();

		// Check frustum
		bool bInFrustum = TestBoxAgainstFrustum(Bounds, FrustumPlanes);

		// Check distance if enabled
		if (bInFrustum && CullingConfig.bEnableDistanceCulling)
		{
			float Distance = FVector::Dist(ViewLocation, Component->GetComponentLocation());
			if (Distance > CullingConfig.MaxRenderDistance)
			{
				bInFrustum = false;
			}
		}

		// Update visibility
		SetComponentCulled(Component, bInFrustum);

		if (!bInFrustum)
		{
			CulledCount++;
		}
	}

	CachedCulledObjects = CulledCount;

	UE_LOG(LogTemp, Verbose, TEXT("TerrainRenderingOptimizer: Culled %d/%d objects"),
		CulledCount, TrackedComponents.Num());
}

bool UTerrainRenderingOptimizer::IsInFrustum(
	const FBox& Bounds,
	FVector ViewLocation,
	FRotator ViewRotation,
	float FOV) const
{
	TArray<FPlane> FrustumPlanes;
	CalculateFrustumPlanes(ViewLocation, ViewRotation, FOV, FrustumPlanes);
	return TestBoxAgainstFrustum(Bounds, FrustumPlanes);
}

void UTerrainRenderingOptimizer::SetComponentCulled(UPrimitiveComponent* Component, bool bVisible)
{
	if (!Component)
	{
		return;
	}

	Component->SetVisibility(bVisible, false);
}

// ============================================================================
// DRAW CALL BATCHING
// ============================================================================

void UTerrainRenderingOptimizer::OptimizeDrawCalls()
{
	if (!BatchingConfig.bEnableBatching)
	{
		return;
	}

	// Group batches by material if enabled
	if (BatchingConfig.bBatchByMaterial)
	{
		// Sort batches by material
		InstanceBatches.Sort([](const FInstanceBatch& A, const FInstanceBatch& B)
		{
			return A.Material < B.Material;
		});
	}

	// Group batches by LOD if enabled
	if (BatchingConfig.bBatchByLOD)
	{
		// Sort batches by LOD level
		InstanceBatches.StableSort([](const FInstanceBatch& A, const FInstanceBatch& B)
		{
			return A.LODLevel < B.LODLevel;
		});
	}

	UpdatePerformanceStats();

	UE_LOG(LogTemp, Log, TEXT("TerrainRenderingOptimizer: Optimized draw calls - estimated %d draw calls"),
		CachedDrawCalls);
}

int32 UTerrainRenderingOptimizer::GetEstimatedDrawCalls() const
{
	return CachedDrawCalls;
}

UStaticMesh* UTerrainRenderingOptimizer::MergeStaticMeshes(
	const TArray<UStaticMesh*>& Meshes,
	UMaterialInterface* Material)
{
	// Mesh merging is complex and typically done in editor
	// For runtime, we rely on instancing instead
	UE_LOG(LogTemp, Warning, TEXT("TerrainRenderingOptimizer: Runtime mesh merging not implemented - use instancing instead"));
	return nullptr;
}

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

void UTerrainRenderingOptimizer::GetPerformanceStats(
	int32& OutDrawCalls,
	int32& OutTriangles,
	int32& OutInstances,
	int32& OutCulledObjects) const
{
	OutDrawCalls = CachedDrawCalls;
	OutTriangles = CachedTriangles;
	OutInstances = GetTotalInstanceCount();
	OutCulledObjects = CachedCulledObjects;
}

void UTerrainRenderingOptimizer::LogPerformanceStats() const
{
	int32 DrawCalls, Triangles, Instances, Culled;
	GetPerformanceStats(DrawCalls, Triangles, Instances, Culled);

	UE_LOG(LogTemp, Log, TEXT("=== Terrain Rendering Performance Stats ==="));
	UE_LOG(LogTemp, Log, TEXT("  Draw Calls: %d"), DrawCalls);
	UE_LOG(LogTemp, Log, TEXT("  Triangles: %d"), Triangles);
	UE_LOG(LogTemp, Log, TEXT("  Instances: %d"), Instances);
	UE_LOG(LogTemp, Log, TEXT("  Culled Objects: %d"), Culled);
	UE_LOG(LogTemp, Log, TEXT("  Instance Batches: %d"), InstanceBatches.Num());
	UE_LOG(LogTemp, Log, TEXT("  Tracked Components: %d"), TrackedComponents.Num());
	UE_LOG(LogTemp, Log, TEXT("=========================================="));
}

// ============================================================================
// INTERNAL METHODS
// ============================================================================

FInstanceBatch* UTerrainRenderingOptimizer::FindOrCreateBatch(
	UStaticMesh* Mesh,
	UMaterialInterface* Material,
	int32 LODLevel)
{
	// Find existing batch
	for (FInstanceBatch& Batch : InstanceBatches)
	{
		if (Batch.Mesh == Mesh && Batch.Material == Material && Batch.LODLevel == LODLevel)
		{
			return &Batch;
		}
	}

	// Create new batch
	FInstanceBatch NewBatch;
	NewBatch.Mesh = Mesh;
	NewBatch.Material = Material;
	NewBatch.LODLevel = LODLevel;

	InstanceBatches.Add(NewBatch);
	return &InstanceBatches.Last();
}

void UTerrainRenderingOptimizer::CalculateFrustumPlanes(
	FVector ViewLocation,
	FRotator ViewRotation,
	float FOV,
	TArray<FPlane>& OutPlanes) const
{
	OutPlanes.Empty();

	// Calculate view direction and right/up vectors
	FVector ViewDir = ViewRotation.Vector();
	FVector RightDir = FRotationMatrix(ViewRotation).GetScaledAxis(EAxis::Y);
	FVector UpDir = FRotationMatrix(ViewRotation).GetScaledAxis(EAxis::Z);

	// Calculate half angles
	float HalfFOVRadians = FMath::DegreesToRadians(FOV * 0.5f);
	float HalfFOVTan = FMath::Tan(HalfFOVRadians);

	// Aspect ratio (assume 16:9 for now)
	float AspectRatio = 16.0f / 9.0f;

	// Calculate frustum corners
	FVector FarCenter = ViewLocation + ViewDir * CullingConfig.MaxRenderDistance;
	FVector FarUp = UpDir * (CullingConfig.MaxRenderDistance * HalfFOVTan);
	FVector FarRight = RightDir * (CullingConfig.MaxRenderDistance * HalfFOVTan * AspectRatio);

	// Create frustum planes (pointing inward)
	// Near plane
	OutPlanes.Add(FPlane(ViewLocation, ViewDir));

	// Far plane
	OutPlanes.Add(FPlane(FarCenter, -ViewDir));

	// Left plane
	FVector LeftNormal = FVector::CrossProduct(UpDir, ViewDir - RightDir * HalfFOVTan * AspectRatio);
	LeftNormal.Normalize();
	OutPlanes.Add(FPlane(ViewLocation, LeftNormal));

	// Right plane
	FVector RightNormal = FVector::CrossProduct(ViewDir + RightDir * HalfFOVTan * AspectRatio, UpDir);
	RightNormal.Normalize();
	OutPlanes.Add(FPlane(ViewLocation, RightNormal));

	// Top plane
	FVector TopNormal = FVector::CrossProduct(RightDir, ViewDir + UpDir * HalfFOVTan);
	TopNormal.Normalize();
	OutPlanes.Add(FPlane(ViewLocation, TopNormal));

	// Bottom plane
	FVector BottomNormal = FVector::CrossProduct(ViewDir - UpDir * HalfFOVTan, RightDir);
	BottomNormal.Normalize();
	OutPlanes.Add(FPlane(ViewLocation, BottomNormal));
}

bool UTerrainRenderingOptimizer::TestBoxAgainstFrustum(
	const FBox& Box,
	const TArray<FPlane>& Planes) const
{
	// Expand box by culling margin
	FBox ExpandedBox = Box.ExpandBy(CullingConfig.CullingMargin);

	// Test box against each plane
	for (const FPlane& Plane : Planes)
	{
		// Get the positive vertex (furthest in plane normal direction)
		FVector PositiveVertex;
		PositiveVertex.X = (Plane.X >= 0.0f) ? ExpandedBox.Max.X : ExpandedBox.Min.X;
		PositiveVertex.Y = (Plane.Y >= 0.0f) ? ExpandedBox.Max.Y : ExpandedBox.Min.Y;
		PositiveVertex.Z = (Plane.Z >= 0.0f) ? ExpandedBox.Max.Z : ExpandedBox.Min.Z;

		// If positive vertex is outside plane, box is outside frustum
		if (Plane.PlaneDot(PositiveVertex) < 0.0f)
		{
			return false;
		}
	}

	return true;
}

void UTerrainRenderingOptimizer::UpdatePerformanceStats()
{
	// Estimate draw calls (one per batch if instancing is enabled)
	CachedDrawCalls = InstanceBatches.Num();

	// Estimate triangles
	CachedTriangles = 0;
	for (const FInstanceBatch& Batch : InstanceBatches)
	{
		if (Batch.Mesh && Batch.Instances.Num() > 0)
		{
			// Get triangle count from mesh (LOD 0)
			if (Batch.Mesh->GetRenderData() && Batch.Mesh->GetRenderData()->LODResources.Num() > 0)
			{
				int32 MeshTriangles = Batch.Mesh->GetRenderData()->LODResources[0].GetNumTriangles();
				CachedTriangles += MeshTriangles * Batch.Instances.Num();
			}
		}
	}
}
