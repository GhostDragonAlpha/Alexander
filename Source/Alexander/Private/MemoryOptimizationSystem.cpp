// Copyright Epic Games, Inc. All Rights Reserved.

#include "MemoryOptimizationSystem.h"
#include "Engine/Texture.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformMemory.h"

void UMemoryOptimizationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Set default memory budget
    MemoryBudget.MaxTotalMemoryMB = 8192; // 8GB
    MemoryBudget.MaxTextureMemoryMB = 4096; // 4GB
    MemoryBudget.MaxMeshMemoryMB = 2048; // 2GB
    MemoryBudget.MaxTileCacheMB = 1024; // 1GB
    MemoryBudget.MaxActiveTiles = 500;

    // Set default mesh LOD config
    MeshLODConfig.LOD0Distance = 500.0f;
    MeshLODConfig.LOD1Distance = 2000.0f;
    MeshLODConfig.LOD2Distance = 10000.0f;
    MeshLODConfig.LOD3Distance = 50000.0f;
    MeshLODConfig.MaxLODLevel = 5;

    UE_LOG(LogTemp, Log, TEXT("MemoryOptimizationSystem initialized with 8GB budget"));
}

void UMemoryOptimizationSystem::Deinitialize()
{
    // Clear all caches
    ClearTileCache();
    TextureStreamingPriorities.Empty();
    StreamedTextures.Empty();
    RegisteredMeshComponents.Empty();

    Super::Deinitialize();
}

void UMemoryOptimizationSystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    // Start auto optimization timer
    if (bAutoOptimizationEnabled)
    {
        FTimerHandle OptimizationTimer;
        InWorld.GetTimerManager().SetTimer(
            OptimizationTimer,
            [this]()
            {
                PerformMemoryOptimization();
            },
            OptimizationInterval,
            true
        );
    }
}

void UMemoryOptimizationSystem::SetMemoryBudget(const FMemoryBudgetConfig& Budget)
{
    MemoryBudget = Budget;
    UE_LOG(LogTemp, Log, TEXT("Memory budget updated: Total=%dMB, Texture=%dMB, Mesh=%dMB, TileCache=%dMB"),
        Budget.MaxTotalMemoryMB, Budget.MaxTextureMemoryMB, Budget.MaxMeshMemoryMB, Budget.MaxTileCacheMB);

    // Immediately check if we need to free memory
    CheckMemoryBudget();
}

void UMemoryOptimizationSystem::SetMeshLODConfig(const FMeshLODConfig& Config)
{
    MeshLODConfig = Config;
    UE_LOG(LogTemp, Log, TEXT("Mesh LOD config updated: LOD0=%fm, LOD1=%fm, LOD2=%fm, LOD3=%fm"),
        Config.LOD0Distance, Config.LOD1Distance, Config.LOD2Distance, Config.LOD3Distance);
}

void UMemoryOptimizationSystem::UpdateTextureStreaming(const FVector& ViewerLocation, const FVector& ViewerDirection)
{
    UpdateTextureStreamingInternal(ViewerLocation, ViewerDirection);
}

void UMemoryOptimizationSystem::SetTextureStreamingPriority(UTexture* Texture, ETextureStreamingPriority Priority)
{
    if (!Texture)
    {
        return;
    }

    TextureStreamingPriorities.Add(Texture, Priority);

    // Force stream critical textures
    if (Priority == ETextureStreamingPriority::Critical)
    {
        ForceStreamTexture(Texture);
    }
}

void UMemoryOptimizationSystem::ForceStreamTexture(UTexture* Texture)
{
    if (!Texture)
    {
        return;
    }

    // Mark as streamed
    StreamedTextures.Add(Texture);

    // Force full resolution
    Texture->bForceMiplevelsToBeResident = true;
    Texture->WaitForStreaming();
}

void UMemoryOptimizationSystem::UnloadDistantTextures(float DistanceThreshold)
{
    TArray<UTexture*> TexturesToUnload;

    for (auto& Pair : TextureStreamingPriorities)
    {
        if (Pair.Value == ETextureStreamingPriority::VeryLow || Pair.Value == ETextureStreamingPriority::Low)
        {
            TexturesToUnload.Add(Pair.Key);
        }
    }

    for (UTexture* Texture : TexturesToUnload)
    {
        if (Texture)
        {
            Texture->bForceMiplevelsToBeResident = false;
            StreamedTextures.Remove(Texture);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Unloaded %d distant textures"), TexturesToUnload.Num());
}

void UMemoryOptimizationSystem::UpdateMeshLODs(const FVector& ViewerLocation)
{
    UpdateMeshLODsInternal(ViewerLocation);
}

int32 UMemoryOptimizationSystem::CalculateLODLevel(const FVector& MeshLocation, const FVector& ViewerLocation) const
{
    float Distance = FVector::Dist(MeshLocation, ViewerLocation);

    if (Distance < MeshLODConfig.LOD0Distance)
    {
        return 0;
    }
    else if (Distance < MeshLODConfig.LOD1Distance)
    {
        return 1;
    }
    else if (Distance < MeshLODConfig.LOD2Distance)
    {
        return 2;
    }
    else if (Distance < MeshLODConfig.LOD3Distance)
    {
        return 3;
    }
    else
    {
        return FMath::Min(4, MeshLODConfig.MaxLODLevel);
    }
}

void UMemoryOptimizationSystem::RegisterMeshComponent(UStaticMeshComponent* MeshComponent)
{
    if (MeshComponent && !RegisteredMeshComponents.Contains(MeshComponent))
    {
        RegisteredMeshComponents.Add(MeshComponent);
    }
}

void UMemoryOptimizationSystem::UnregisterMeshComponent(UStaticMeshComponent* MeshComponent)
{
    RegisteredMeshComponents.Remove(MeshComponent);
}

void UMemoryOptimizationSystem::OptimizeTileCache()
{
    OptimizeTileCacheInternal();
}

void UMemoryOptimizationSystem::SetMaxTileCacheSize(int32 MaxSizeMB)
{
    MemoryBudget.MaxTileCacheMB = MaxSizeMB;
    
    // Evict tiles if we're over the new limit
    while (CurrentTileCacheSizeBytes > MaxSizeMB * 1024 * 1024)
    {
        int32 LRUTile = FindLeastRecentlyUsedTile();
        if (LRUTile >= 0)
        {
            EvictTileCacheEntry(LRUTile);
        }
        else
        {
            break;
        }
    }
}

void UMemoryOptimizationSystem::ClearTileCache()
{
    TileCache.Empty();
    CurrentTileCacheSizeBytes = 0;
    TileCacheHits = 0;
    TileCacheMisses = 0;
    UE_LOG(LogTemp, Log, TEXT("Tile cache cleared"));
}

void UMemoryOptimizationSystem::EvictLeastRecentlyUsedTiles(int32 Count)
{
    for (int32 i = 0; i < Count; ++i)
    {
        int32 LRUTile = FindLeastRecentlyUsedTile();
        if (LRUTile >= 0)
        {
            EvictTileCacheEntry(LRUTile);
        }
        else
        {
            break;
        }
    }
}

FOverallMemoryStats UMemoryOptimizationSystem::GetMemoryStats() const
{
    FOverallMemoryStats Stats;
    Stats.TextureMemoryMB = CalculateTextureMemoryUsage();
    Stats.MeshMemoryMB = CalculateMeshMemoryUsage();
    Stats.TileCacheMemoryMB = CalculateTileCacheMemoryUsage();
    Stats.OtherMemoryMB = 0.0f; // Could be calculated from platform memory stats
    Stats.TotalMemoryMB = Stats.TextureMemoryMB + Stats.MeshMemoryMB + Stats.TileCacheMemoryMB + Stats.OtherMemoryMB;
    Stats.bWithinBudget = Stats.TotalMemoryMB <= MemoryBudget.MaxTotalMemoryMB;

    return Stats;
}

FTextureStreamingStats UMemoryOptimizationSystem::GetTextureStreamingStats() const
{
    FTextureStreamingStats Stats;
    Stats.StreamedTextures = StreamedTextures.Num();
    Stats.ResidentTextures = TextureStreamingPriorities.Num();
    Stats.TextureMemoryMB = CalculateTextureMemoryUsage();
    Stats.StreamingBandwidthMBps = TextureStreamingBandwidth;

    return Stats;
}

FMeshLODStats UMemoryOptimizationSystem::GetMeshLODStats() const
{
    FMeshLODStats Stats;
    Stats.TotalMeshes = RegisteredMeshComponents.Num();
    Stats.MeshesByLOD = MeshCountByLOD;
    Stats.MeshMemoryMB = CalculateMeshMemoryUsage();
    Stats.VisibleTriangles = TotalVisibleTriangles;

    return Stats;
}

FTileCacheStats UMemoryOptimizationSystem::GetTileCacheStats() const
{
    FTileCacheStats Stats;
    Stats.ActiveTiles = TileCache.Num();
    Stats.CachedTiles = TileCache.Num();
    Stats.TileCacheMemoryMB = CalculateTileCacheMemoryUsage();
    Stats.CacheHits = TileCacheHits;
    Stats.CacheMisses = TileCacheMisses;
    
    int32 TotalAccesses = TileCacheHits + TileCacheMisses;
    Stats.HitRate = TotalAccesses > 0 ? (float)TileCacheHits / TotalAccesses : 0.0f;

    return Stats;
}

bool UMemoryOptimizationSystem::IsWithinMemoryBudget() const
{
    FOverallMemoryStats Stats = GetMemoryStats();
    return Stats.bWithinBudget;
}

void UMemoryOptimizationSystem::LogMemoryStats() const
{
    FOverallMemoryStats Stats = GetMemoryStats();
    
    UE_LOG(LogTemp, Log, TEXT("=== Memory Optimization Stats ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Memory: %.2f MB / %d MB"), Stats.TotalMemoryMB, MemoryBudget.MaxTotalMemoryMB);
    UE_LOG(LogTemp, Log, TEXT("Texture Memory: %.2f MB / %d MB"), Stats.TextureMemoryMB, MemoryBudget.MaxTextureMemoryMB);
    UE_LOG(LogTemp, Log, TEXT("Mesh Memory: %.2f MB / %d MB"), Stats.MeshMemoryMB, MemoryBudget.MaxMeshMemoryMB);
    UE_LOG(LogTemp, Log, TEXT("Tile Cache: %.2f MB / %d MB"), Stats.TileCacheMemoryMB, MemoryBudget.MaxTileCacheMB);
    UE_LOG(LogTemp, Log, TEXT("Within Budget: %s"), Stats.bWithinBudget ? TEXT("YES") : TEXT("NO"));
    
    FTextureStreamingStats TexStats = GetTextureStreamingStats();
    UE_LOG(LogTemp, Log, TEXT("Streamed Textures: %d / %d"), TexStats.StreamedTextures, TexStats.ResidentTextures);
    
    FMeshLODStats MeshStats = GetMeshLODStats();
    UE_LOG(LogTemp, Log, TEXT("Registered Meshes: %d"), MeshStats.TotalMeshes);
    UE_LOG(LogTemp, Log, TEXT("Visible Triangles: %d"), MeshStats.VisibleTriangles);
    
    FTileCacheStats TileStats = GetTileCacheStats();
    UE_LOG(LogTemp, Log, TEXT("Cached Tiles: %d"), TileStats.CachedTiles);
    UE_LOG(LogTemp, Log, TEXT("Cache Hit Rate: %.2f%%"), TileStats.HitRate * 100.0f);
}

void UMemoryOptimizationSystem::EnableAutoOptimization(bool bEnable)
{
    bAutoOptimizationEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Auto optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UMemoryOptimizationSystem::PerformMemoryOptimization()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Skip if called too frequently
    if (CurrentTime - LastOptimizationTime < OptimizationInterval)
    {
        return;
    }
    
    LastOptimizationTime = CurrentTime;

    // Check memory budget
    CheckMemoryBudget();

    // Optimize tile cache
    OptimizeTileCacheInternal();
}

void UMemoryOptimizationSystem::UpdateTextureStreamingInternal(const FVector& ViewerLocation, const FVector& ViewerDirection)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float DeltaTime = CurrentTime - LastTextureStreamingUpdate;
    LastTextureStreamingUpdate = CurrentTime;

    int32 TexturesStreamed = 0;
    float BytesStreamed = 0.0f;

    // Update priorities based on viewer location
    for (auto& Pair : TextureStreamingPriorities)
    {
        UTexture* Texture = Pair.Key;
        ETextureStreamingPriority& Priority = Pair.Value;

        if (!Texture)
        {
            continue;
        }

        // Stream high priority textures
        if (Priority == ETextureStreamingPriority::Critical || Priority == ETextureStreamingPriority::High)
        {
            if (!StreamedTextures.Contains(Texture))
            {
                ForceStreamTexture(Texture);
                TexturesStreamed++;
                // Estimate texture size (rough approximation)
                BytesStreamed += 4.0f * 1024.0f * 1024.0f; // 4MB average
            }
        }
        // Unload low priority textures
        else if (Priority == ETextureStreamingPriority::VeryLow)
        {
            if (StreamedTextures.Contains(Texture))
            {
                Texture->bForceMiplevelsToBeResident = false;
                StreamedTextures.Remove(Texture);
            }
        }
    }

    // Calculate streaming bandwidth
    if (DeltaTime > 0.0f)
    {
        TextureStreamingBandwidth = (BytesStreamed / (1024.0f * 1024.0f)) / DeltaTime;
    }
}

void UMemoryOptimizationSystem::UpdateMeshLODsInternal(const FVector& ViewerLocation)
{
    MeshCountByLOD.Empty();
    TotalVisibleTriangles = 0;

    for (UStaticMeshComponent* MeshComponent : RegisteredMeshComponents)
    {
        if (!MeshComponent || !MeshComponent->GetStaticMesh())
        {
            continue;
        }

        FVector MeshLocation = MeshComponent->GetComponentLocation();
        int32 LODLevel = CalculateLODLevel(MeshLocation, ViewerLocation);

        // Set forced LOD level
        MeshComponent->SetForcedLodModel(LODLevel + 1); // +1 because 0 means auto

        // Track statistics
        int32& Count = MeshCountByLOD.FindOrAdd(LODLevel);
        Count++;

        // Estimate triangle count (rough approximation)
        UStaticMesh* Mesh = MeshComponent->GetStaticMesh();
        if (Mesh && Mesh->GetRenderData())
        {
            int32 NumLODs = Mesh->GetRenderData()->LODResources.Num();
            if (LODLevel < NumLODs)
            {
                TotalVisibleTriangles += Mesh->GetRenderData()->LODResources[LODLevel].GetNumTriangles();
            }
        }
    }
}

void UMemoryOptimizationSystem::OptimizeTileCacheInternal()
{
    // Check if we're over the cache size limit
    int32 MaxCacheSizeBytes = MemoryBudget.MaxTileCacheMB * 1024 * 1024;

    while (CurrentTileCacheSizeBytes > MaxCacheSizeBytes && TileCache.Num() > 0)
    {
        int32 LRUTile = FindLeastRecentlyUsedTile();
        if (LRUTile >= 0)
        {
            EvictTileCacheEntry(LRUTile);
        }
        else
        {
            break;
        }
    }

    // Also check if we have too many tiles
    while (TileCache.Num() > MemoryBudget.MaxActiveTiles)
    {
        int32 LRUTile = FindLeastRecentlyUsedTile();
        if (LRUTile >= 0)
        {
            EvictTileCacheEntry(LRUTile);
        }
        else
        {
            break;
        }
    }
}

void UMemoryOptimizationSystem::CheckMemoryBudget()
{
    FOverallMemoryStats Stats = GetMemoryStats();

    if (!Stats.bWithinBudget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Memory budget exceeded: %.2f MB / %d MB"), 
            Stats.TotalMemoryMB, MemoryBudget.MaxTotalMemoryMB);

        // Free memory by unloading distant textures
        if (Stats.TextureMemoryMB > MemoryBudget.MaxTextureMemoryMB)
        {
            UnloadDistantTextures(10000.0f);
        }

        // Free memory by evicting tile cache
        if (Stats.TileCacheMemoryMB > MemoryBudget.MaxTileCacheMB)
        {
            int32 TilesToEvict = FMath::CeilToInt((Stats.TileCacheMemoryMB - MemoryBudget.MaxTileCacheMB) / 2.0f);
            EvictLeastRecentlyUsedTiles(TilesToEvict);
        }
    }
}

float UMemoryOptimizationSystem::CalculateTextureMemoryUsage() const
{
    // Rough estimate based on streamed textures
    // In a real implementation, you'd query actual texture memory usage
    return StreamedTextures.Num() * 4.0f; // 4MB average per texture
}

float UMemoryOptimizationSystem::CalculateMeshMemoryUsage() const
{
    // Rough estimate based on registered meshes
    // In a real implementation, you'd query actual mesh memory usage
    return RegisteredMeshComponents.Num() * 2.0f; // 2MB average per mesh
}

float UMemoryOptimizationSystem::CalculateTileCacheMemoryUsage() const
{
    return CurrentTileCacheSizeBytes / (1024.0f * 1024.0f);
}

void UMemoryOptimizationSystem::EvictTileCacheEntry(int32 TileID)
{
    if (FTileCacheEntry* Entry = TileCache.Find(TileID))
    {
        CurrentTileCacheSizeBytes -= Entry->SizeBytes;
        TileCache.Remove(TileID);
        UE_LOG(LogTemp, Verbose, TEXT("Evicted tile %d from cache"), TileID);
    }
}

int32 UMemoryOptimizationSystem::FindLeastRecentlyUsedTile() const
{
    int32 LRUTileID = -1;
    float OldestAccessTime = FLT_MAX;

    for (const auto& Pair : TileCache)
    {
        if (Pair.Value.LastAccessTime < OldestAccessTime)
        {
            OldestAccessTime = Pair.Value.LastAccessTime;
            LRUTileID = Pair.Key;
        }
    }

    return LRUTileID;
}
