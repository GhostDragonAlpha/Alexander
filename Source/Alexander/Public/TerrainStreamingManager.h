// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TerrainTile.h"
#include "HAL/Runnable.h"
#include "Containers/Queue.h"
#include "TerrainStreamingManager.generated.h"

// Custom runnable class for lambda execution
class FTerrainWorkerRunnable : public FRunnable
{
public:
    FTerrainWorkerRunnable(TFunction<void()> InFunction)
        : Function(InFunction)
    {}

    virtual uint32 Run() override
    {
        Function();
        return 0;
    }

private:
    TFunction<void()> Function;
};

/**
 * Priority levels for tile loading
 */
UENUM(BlueprintType)
enum class ETileLoadPriority : uint8
{
	Low = 0,
	Normal = 1,
	High = 2,
	Critical = 3
};

/**
 * Tile loading request
 */
USTRUCT()
struct FTileLoadRequest
{
	GENERATED_BODY()

	// Tile identification
	UPROPERTY()
	FVector2D TilePosition;

	UPROPERTY()
	float TileSize = 1000.0f;

	UPROPERTY()
	int32 LODLevel = 0;

	UPROPERTY()
	int32 Resolution = 64;

	// Generation config
	UPROPERTY()
	FTerrainGenerationConfig GenerationConfig;

	// Priority
	UPROPERTY()
	ETileLoadPriority Priority = ETileLoadPriority::Normal;

	// Distance from viewer (for sorting)
	UPROPERTY()
	float DistanceFromViewer = 0.0f;

	// Request ID for tracking
	UPROPERTY()
	int32 RequestID = 0;

	// Timestamp
	double RequestTime = 0.0;

	// Result (filled by background thread)
	FTerrainTileData GeneratedTile;

	// Status
	bool bIsComplete = false;
	bool bHasError = false;

	// Constructor
	FTileLoadRequest()
		: TilePosition(FVector2D::ZeroVector)
		, TileSize(1000.0f)
		, LODLevel(0)
		, Resolution(64)
		, Priority(ETileLoadPriority::Normal)
		, DistanceFromViewer(0.0f)
		, RequestID(0)
		, RequestTime(0.0)
		, bIsComplete(false)
		, bHasError(false)
	{
	}

	// Comparison for priority queue (higher priority first, then closer distance)
	bool operator<(const FTileLoadRequest& Other) const
	{
		if (Priority != Other.Priority)
		{
			return Priority > Other.Priority; // Higher priority first
		}
		return DistanceFromViewer < Other.DistanceFromViewer; // Closer first
	}
};

/**
 * Tile cache entry with LRU tracking
 */
USTRUCT()
struct FTileCacheEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FTerrainTileData TileData;

	UPROPERTY()
	double LastAccessTime = 0.0;

	UPROPERTY()
	int32 AccessCount = 0;

	// Cache key (position + LOD)
	FString GetCacheKey() const
	{
		return FString::Printf(TEXT("%.1f_%.1f_%d"),
			TileData.WorldPosition.X,
			TileData.WorldPosition.Y,
			TileData.LODLevel);
	}
};

/**
 * Streaming statistics
 */
USTRUCT(BlueprintType)
struct FStreamingStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 PendingRequests = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 CompletedRequests = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 CachedTiles = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 CacheHits = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 CacheMisses = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float AverageLoadTime = 0.0f; // milliseconds

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float LastFrameLoadTime = 0.0f; // milliseconds

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 TilesLoadedThisFrame = 0;
};

/**
 * Streaming configuration
 */
USTRUCT(BlueprintType)
struct FStreamingConfig
{
	GENERATED_BODY()

	// Maximum time per frame for loading operations (milliseconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float MaxFrameTimeMS = 5.0f;

	// Maximum number of tiles to load per frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 MaxTilesPerFrame = 4;

	// Maximum cache size (number of tiles)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cache")
	int32 MaxCacheSize = 500;

	// Enable background thread for generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threading")
	bool bUseBackgroundThread = true;

	// Number of worker threads
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threading")
	int32 NumWorkerThreads = 2;

	// Maximum pending requests in queue
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threading")
	int32 MaxPendingRequests = 100;
};

/**
 * Asynchronous terrain streaming manager
 * Handles background tile generation, loading queue, and tile cache
 */
UCLASS(BlueprintType)
class ALEXANDER_API UTerrainStreamingManager : public UObject
{
	GENERATED_BODY()

public:
	UTerrainStreamingManager();
	virtual ~UTerrainStreamingManager();

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize streaming system
	 * @param Config Streaming configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void Initialize(const FStreamingConfig& Config);

	/**
	 * Shutdown streaming system and cleanup
	 */
	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void Shutdown();

	// ============================================================================
	// TILE LOADING
	// ============================================================================

	/**
	 * Request tile to be loaded
	 * @param TilePosition World position of tile
	 * @param TileSize Size of tile in meters
	 * @param LODLevel Level of detail
	 * @param Resolution Tile resolution
	 * @param GenerationConfig Terrain generation config
	 * @param Priority Loading priority
	 * @param ViewerPosition Viewer position for distance calculation
	 * @return Request ID for tracking
	 */
	UFUNCTION(BlueprintCallable, Category = "Streaming")
	int32 RequestTileLoad(
		FVector2D TilePosition,
		float TileSize,
		int32 LODLevel,
		int32 Resolution,
		const FTerrainGenerationConfig& GenerationConfig,
		ETileLoadPriority Priority,
		FVector2D ViewerPosition
	);

	/**
	 * Check if tile is ready
	 * @param RequestID Request ID from RequestTileLoad
	 * @return True if tile is ready
	 */
	UFUNCTION(BlueprintCallable, Category = "Streaming")
	bool IsTileReady(int32 RequestID) const;

	/**
	 * Get loaded tile data
	 * @param RequestID Request ID from RequestTileLoad
	 * @param OutTileData Output tile data
	 * @return True if tile was retrieved successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Streaming")
	bool GetLoadedTile(int32 RequestID, FTerrainTileData& OutTileData);

	/**
	 * Cancel pending tile request
	 * @param RequestID Request ID to cancel
	 */
	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void CancelRequest(int32 RequestID);

	// ============================================================================
	// UPDATE
	// ============================================================================

	/**
	 * Update streaming system (call every frame)
	 * @param DeltaTime Time since last update
	 */
	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void Update(float DeltaTime);

	/**
	 * Process completed tile requests within frame time budget
	 * @param MaxTimeMS Maximum time to spend this frame
	 * @return Number of tiles processed
	 */
	int32 ProcessCompletedRequests(float MaxTimeMS);

	// ============================================================================
	// CACHE MANAGEMENT
	// ============================================================================

	/**
	 * Check if tile is in cache
	 * @param TilePosition Tile position
	 * @param LODLevel LOD level
	 * @return True if tile is cached
	 */
	UFUNCTION(BlueprintCallable, Category = "Streaming|Cache")
	bool IsTileCached(FVector2D TilePosition, int32 LODLevel) const;

	/**
	 * Get tile from cache
	 * @param TilePosition Tile position
	 * @param LODLevel LOD level
	 * @param OutTileData Output tile data
	 * @return True if tile was found in cache
	 */
	UFUNCTION(BlueprintCallable, Category = "Streaming|Cache")
	bool GetCachedTile(FVector2D TilePosition, int32 LODLevel, FTerrainTileData& OutTileData);

	/**
	 * Add tile to cache
	 * @param TileData Tile data to cache
	 */
	void AddToCache(const FTerrainTileData& TileData);

	/**
	 * Clear tile cache
	 */
	UFUNCTION(BlueprintCallable, Category = "Streaming|Cache")
	void ClearCache();

	/**
	 * Evict least recently used tiles from cache
	 * @param NumToEvict Number of tiles to evict
	 */
	void EvictLRUTiles(int32 NumToEvict);

	// ============================================================================
	// STATISTICS
	// ============================================================================

	/**
	 * Get streaming statistics
	 * @return Current streaming stats
	 */
	UFUNCTION(BlueprintCallable, Category = "Streaming|Stats")
	FStreamingStats GetStreamingStats() const;

	/**
	 * Reset statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Streaming|Stats")
	void ResetStats();

	// ============================================================================
	// PROPERTIES
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FStreamingConfig Config;

private:
	// Request management
	TQueue<FTileLoadRequest, EQueueMode::Mpsc> PendingQueue; // Multi-producer, single-consumer
	TQueue<FTileLoadRequest, EQueueMode::Spsc> CompletedQueue; // Single-producer, single-consumer
	TMap<int32, FTileLoadRequest> ActiveRequests;
	int32 NextRequestID;

	// Tile cache (LRU)
	TMap<FString, FTileCacheEntry> TileCache;

	// Statistics
	FStreamingStats Stats;
	TArray<float> LoadTimeSamples;
	int32 MaxLoadTimeSamples;

	// Threading
	TArray<FRunnableThread*> WorkerThreads;
	TArray<FRunnable*> WorkerRunnables;
	FCriticalSection QueueLock;
	FThreadSafeBool bIsShuttingDown;

	// Timing
	double CurrentTime;

	// Helper: Generate cache key
	FString GetCacheKey(FVector2D Position, int32 LODLevel) const;

	// Helper: Start worker threads
	void StartWorkerThreads();

	// Helper: Stop worker threads
	void StopWorkerThreads();

	// Helper: Worker thread function
	void WorkerThreadFunction();

	// Helper: Process single tile request
	void ProcessTileRequest(FTileLoadRequest& Request);

	// Helper: Update statistics
	void UpdateStats(float LoadTimeMS);
};
