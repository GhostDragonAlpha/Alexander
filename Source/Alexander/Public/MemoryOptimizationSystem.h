// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MemoryOptimizationSystem.generated.h"

/**
 * Texture streaming priority levels
 */
UENUM(BlueprintType)
enum class ETextureStreamingPriority : uint8
{
    Critical = 0,    // Player immediate vicinity
    High = 1,        // Visible and close
    Medium = 2,      // Visible but distant
    Low = 3,         // Just outside view
    VeryLow = 4      // Far from player
};

/**
 * Mesh LOD configuration
 */
USTRUCT(BlueprintType)
struct FMeshLODConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0Distance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1Distance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2Distance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD3Distance = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxLODLevel = 5;
};

/**
 * Memory budget configuration
 */
USTRUCT(BlueprintType)
struct FMemoryBudgetConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxTotalMemoryMB = 8192; // 8GB

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxTextureMemoryMB = 4096; // 4GB

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMeshMemoryMB = 2048; // 2GB

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxTileCacheMB = 1024; // 1GB

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxActiveTiles = 500;
};

/**
 * Texture streaming statistics
 */
USTRUCT(BlueprintType)
struct FTextureStreamingStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 StreamedTextures = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 ResidentTextures = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float TextureMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float StreamingBandwidthMBps = 0.0f;
};

/**
 * Mesh LOD statistics
 */
USTRUCT(BlueprintType)
struct FMeshLODStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 TotalMeshes = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    TMap<int32, int32> MeshesByLOD;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float MeshMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 VisibleTriangles = 0;
};

/**
 * Tile cache statistics
 */
USTRUCT(BlueprintType)
struct FTileCacheStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 ActiveTiles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 CachedTiles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float TileCacheMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 CacheHits = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 CacheMisses = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float HitRate = 0.0f;
};

/**
 * Overall memory statistics
 */
USTRUCT(BlueprintType)
struct FOverallMemoryStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float TotalMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float TextureMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float MeshMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float TileCacheMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float OtherMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    bool bWithinBudget = true;
};

/**
 * Memory Optimization System
 * Manages texture streaming, mesh LOD, and tile cache to keep memory under 8GB
 * Requirement 8.4: Memory optimization with texture streaming, mesh LOD, and tile cache
 */
UCLASS()
class ALEXANDER_API UMemoryOptimizationSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void SetMemoryBudget(const FMemoryBudgetConfig& Budget);

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void SetMeshLODConfig(const FMeshLODConfig& Config);

    // Texture streaming
    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void UpdateTextureStreaming(const FVector& ViewerLocation, const FVector& ViewerDirection);

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void SetTextureStreamingPriority(UTexture* Texture, ETextureStreamingPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void ForceStreamTexture(UTexture* Texture);

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void UnloadDistantTextures(float DistanceThreshold);

    // Mesh LOD
    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void UpdateMeshLODs(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    int32 CalculateLODLevel(const FVector& MeshLocation, const FVector& ViewerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void RegisterMeshComponent(UStaticMeshComponent* MeshComponent);

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void UnregisterMeshComponent(UStaticMeshComponent* MeshComponent);

    // Tile cache
    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void OptimizeTileCache();

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void SetMaxTileCacheSize(int32 MaxSizeMB);

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void ClearTileCache();

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void EvictLeastRecentlyUsedTiles(int32 Count);

    // Memory monitoring
    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    FOverallMemoryStats GetMemoryStats() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    FTextureStreamingStats GetTextureStreamingStats() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    FMeshLODStats GetMeshLODStats() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    FTileCacheStats GetTileCacheStats() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    bool IsWithinMemoryBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void LogMemoryStats() const;

    // Automatic optimization
    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void EnableAutoOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void PerformMemoryOptimization();

private:
    // Configuration
    UPROPERTY()
    FMemoryBudgetConfig MemoryBudget;

    UPROPERTY()
    FMeshLODConfig MeshLODConfig;

    // Texture streaming
    UPROPERTY()
    TMap<UTexture*, ETextureStreamingPriority> TextureStreamingPriorities;

    UPROPERTY()
    TSet<UTexture*> StreamedTextures;

    float TextureStreamingBandwidth = 0.0f;
    float LastTextureStreamingUpdate = 0.0f;

    // Mesh LOD
    UPROPERTY()
    TArray<UStaticMeshComponent*> RegisteredMeshComponents;

    TMap<int32, int32> MeshCountByLOD;
    int32 TotalVisibleTriangles = 0;

    // Tile cache
    struct FTileCacheEntry
    {
        int32 TileID;
        float LastAccessTime;
        int32 SizeBytes;
        TArray<uint8> Data;
    };

    TMap<int32, FTileCacheEntry> TileCache;
    int32 TileCacheHits = 0;
    int32 TileCacheMisses = 0;
    int32 CurrentTileCacheSizeBytes = 0;

    // Auto optimization
    bool bAutoOptimizationEnabled = true;
    float LastOptimizationTime = 0.0f;
    float OptimizationInterval = 1.0f; // seconds

    // Helper functions
    void UpdateTextureStreamingInternal(const FVector& ViewerLocation, const FVector& ViewerDirection);
    void UpdateMeshLODsInternal(const FVector& ViewerLocation);
    void OptimizeTileCacheInternal();
    void CheckMemoryBudget();
    float CalculateTextureMemoryUsage() const;
    float CalculateMeshMemoryUsage() const;
    float CalculateTileCacheMemoryUsage() const;
    void EvictTileCacheEntry(int32 TileID);
    int32 FindLeastRecentlyUsedTile() const;
};
