// Copyright Epic Games, Inc. All Rights Reserved.

#include "TerrainStreamingManager.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "TerrainTile.h"
#include "HAL/PlatformTime.h"

UTerrainStreamingManager::UTerrainStreamingManager()
	: NextRequestID(1)
	, MaxLoadTimeSamples(100)
	, CurrentTime(0.0)
{
	bIsShuttingDown = false;
}

UTerrainStreamingManager::~UTerrainStreamingManager()
{
	Shutdown();
}

void UTerrainStreamingManager::Initialize(const FStreamingConfig& InConfig)
{
	Config = InConfig;

	// Reset state
	NextRequestID = 1;
	ActiveRequests.Empty();
	TileCache.Empty();
	ResetStats();

	// Start worker threads if enabled
	if (Config.bUseBackgroundThread)
	{
		StartWorkerThreads();
	}

	UE_LOG(LogTemp, Log, TEXT("TerrainStreamingManager initialized: %d worker threads, max cache size: %d"),
		Config.NumWorkerThreads, Config.MaxCacheSize);
}

void UTerrainStreamingManager::Shutdown()
{
	// Signal shutdown
	bIsShuttingDown = true;

	// Stop worker threads
	StopWorkerThreads();

	// Clear all data
	ActiveRequests.Empty();
	TileCache.Empty();

	// Clear queues
	FTileLoadRequest DummyRequest;
	while (PendingQueue.Dequeue(DummyRequest)) {}
	while (CompletedQueue.Dequeue(DummyRequest)) {}

	UE_LOG(LogTemp, Log, TEXT("TerrainStreamingManager shutdown complete"));
}

int32 UTerrainStreamingManager::RequestTileLoad(
	FVector2D TilePosition,
	float TileSize,
	int32 LODLevel,
	int32 Resolution,
	const FTerrainGenerationConfig& GenerationConfig,
	ETileLoadPriority Priority,
	FVector2D ViewerPosition)
{
	// Check if tile is in cache
	FTerrainTileData CachedTile;
	if (GetCachedTile(TilePosition, LODLevel, CachedTile))
	{
		// Create immediate request with cached data
		int32 RequestID = NextRequestID++;

		FTileLoadRequest Request;
		Request.RequestID = RequestID;
		Request.TilePosition = TilePosition;
		Request.TileSize = TileSize;
		Request.LODLevel = LODLevel;
		Request.Resolution = Resolution;
		Request.GenerationConfig = GenerationConfig;
		Request.Priority = Priority;
		Request.DistanceFromViewer = FVector2D::Distance(TilePosition, ViewerPosition);
		Request.RequestTime = CurrentTime;
		Request.GeneratedTile = CachedTile;
		Request.bIsComplete = true;

		ActiveRequests.Add(RequestID, Request);

		Stats.CacheHits++;

		return RequestID;
	}

	// Cache miss - create new request
	Stats.CacheMisses++;

	// Check if we're at max pending requests
	if (ActiveRequests.Num() >= Config.MaxPendingRequests)
	{
		UE_LOG(LogTemp, Warning, TEXT("TerrainStreamingManager: Max pending requests reached (%d)"),
			Config.MaxPendingRequests);
		return -1;
	}

	// Create request
	int32 RequestID = NextRequestID++;

	FTileLoadRequest Request;
	Request.RequestID = RequestID;
	Request.TilePosition = TilePosition;
	Request.TileSize = TileSize;
	Request.LODLevel = LODLevel;
	Request.Resolution = Resolution;
	Request.GenerationConfig = GenerationConfig;
	Request.Priority = Priority;
	Request.DistanceFromViewer = FVector2D::Distance(TilePosition, ViewerPosition);
	Request.RequestTime = CurrentTime;
	Request.bIsComplete = false;

	// Add to active requests
	ActiveRequests.Add(RequestID, Request);

	// Add to pending queue
	if (Config.bUseBackgroundThread)
	{
		PendingQueue.Enqueue(Request);
	}
	else
	{
		// Process immediately on main thread
		ProcessTileRequest(Request);
		CompletedQueue.Enqueue(Request);
	}

	return RequestID;
}

bool UTerrainStreamingManager::IsTileReady(int32 RequestID) const
{
	const FTileLoadRequest* Request = ActiveRequests.Find(RequestID);
	return Request && Request->bIsComplete;
}

bool UTerrainStreamingManager::GetLoadedTile(int32 RequestID, FTerrainTileData& OutTileData)
{
	FTileLoadRequest* Request = ActiveRequests.Find(RequestID);
	if (!Request || !Request->bIsComplete)
	{
		return false;
	}

	OutTileData = Request->GeneratedTile;

	// Remove from active requests
	ActiveRequests.Remove(RequestID);

	return true;
}

void UTerrainStreamingManager::CancelRequest(int32 RequestID)
{
	ActiveRequests.Remove(RequestID);
}

void UTerrainStreamingManager::Update(float DeltaTime)
{
	CurrentTime += DeltaTime;

	// Reset frame stats
	Stats.LastFrameLoadTime = 0.0f;
	Stats.TilesLoadedThisFrame = 0;

	// Process completed requests within frame time budget
	ProcessCompletedRequests(Config.MaxFrameTimeMS);

	// Update stats
	Stats.PendingRequests = ActiveRequests.Num();
	Stats.CachedTiles = TileCache.Num();
}

int32 UTerrainStreamingManager::ProcessCompletedRequests(float MaxTimeMS)
{
	double StartTime = FPlatformTime::Seconds();
	int32 ProcessedCount = 0;

	// Process completed requests from queue
	FTileLoadRequest CompletedRequest;
	while (CompletedQueue.Dequeue(CompletedRequest))
	{
		// Check frame time budget
		double ElapsedMS = (FPlatformTime::Seconds() - StartTime) * 1000.0;
		if (ElapsedMS >= MaxTimeMS || ProcessedCount >= Config.MaxTilesPerFrame)
		{
			// Put request back in queue for next frame
			CompletedQueue.Enqueue(CompletedRequest);
			break;
		}

		// Update active request
		FTileLoadRequest* ActiveRequest = ActiveRequests.Find(CompletedRequest.RequestID);
		if (ActiveRequest)
		{
			ActiveRequest->GeneratedTile = CompletedRequest.GeneratedTile;
			ActiveRequest->bIsComplete = true;
			ActiveRequest->bHasError = CompletedRequest.bHasError;

			// Add to cache if successful
			if (!CompletedRequest.bHasError)
			{
				AddToCache(CompletedRequest.GeneratedTile);
			}

			// Update stats
			float LoadTime = (CurrentTime - CompletedRequest.RequestTime) * 1000.0f;
			UpdateStats(LoadTime);

			ProcessedCount++;
			Stats.CompletedRequests++;
		}
	}

	// Update frame stats
	Stats.TilesLoadedThisFrame = ProcessedCount;
	Stats.LastFrameLoadTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;

	return ProcessedCount;
}

bool UTerrainStreamingManager::IsTileCached(FVector2D TilePosition, int32 LODLevel) const
{
	FString Key = GetCacheKey(TilePosition, LODLevel);
	return TileCache.Contains(Key);
}

bool UTerrainStreamingManager::GetCachedTile(FVector2D TilePosition, int32 LODLevel, FTerrainTileData& OutTileData)
{
	FString Key = GetCacheKey(TilePosition, LODLevel);

	FTileCacheEntry* Entry = TileCache.Find(Key);
	if (!Entry)
	{
		return false;
	}

	// Update access time and count
	Entry->LastAccessTime = CurrentTime;
	Entry->AccessCount++;

	OutTileData = Entry->TileData;

	return true;
}

void UTerrainStreamingManager::AddToCache(const FTerrainTileData& TileData)
{
	// Check if cache is full
	if (TileCache.Num() >= Config.MaxCacheSize)
	{
		// Evict 10% of cache
		int32 NumToEvict = FMath::Max(1, Config.MaxCacheSize / 10);
		EvictLRUTiles(NumToEvict);
	}

	// Create cache entry
	FTileCacheEntry Entry;
	Entry.TileData = TileData;
	Entry.LastAccessTime = CurrentTime;
	Entry.AccessCount = 1;

	// Add to cache
	FString Key = GetCacheKey(TileData.WorldPosition, TileData.LODLevel);
	TileCache.Add(Key, Entry);
}

void UTerrainStreamingManager::ClearCache()
{
	TileCache.Empty();
	Stats.CachedTiles = 0;
}

void UTerrainStreamingManager::EvictLRUTiles(int32 NumToEvict)
{
	if (TileCache.Num() == 0 || NumToEvict <= 0)
	{
		return;
	}

	// Collect all entries with their keys
	TArray<TPair<FString, double>> Entries;
	Entries.Reserve(TileCache.Num());

	for (const auto& Pair : TileCache)
	{
		Entries.Add(TPair<FString, double>(Pair.Key, Pair.Value.LastAccessTime));
	}

	// Sort by last access time (oldest first)
	Entries.Sort([](const TPair<FString, double>& A, const TPair<FString, double>& B)
	{
		return A.Value < B.Value;
	});

	// Remove oldest entries
	int32 NumEvicted = FMath::Min(NumToEvict, Entries.Num());
	for (int32 I = 0; I < NumEvicted; ++I)
	{
		TileCache.Remove(Entries[I].Key);
	}

	UE_LOG(LogTemp, Verbose, TEXT("Evicted %d tiles from cache (LRU)"), NumEvicted);
}

FStreamingStats UTerrainStreamingManager::GetStreamingStats() const
{
	return Stats;
}

void UTerrainStreamingManager::ResetStats()
{
	Stats = FStreamingStats();
	LoadTimeSamples.Empty();
}

FString UTerrainStreamingManager::GetCacheKey(FVector2D Position, int32 LODLevel) const
{
	return FString::Printf(TEXT("%.1f_%.1f_%d"), Position.X, Position.Y, LODLevel);
}

void UTerrainStreamingManager::StartWorkerThreads()
{
	if (WorkerThreads.Num() > 0)
	{
		return; // Already started
	}

	bIsShuttingDown = false;

	// Create worker threads
	for (int32 I = 0; I < Config.NumWorkerThreads; ++I)
	{
		FString ThreadName = FString::Printf(TEXT("TerrainWorker_%d"), I);

		FRunnable* Runnable = new FTerrainWorkerRunnable([this]()
		{
			WorkerThreadFunction();
		});

		FRunnableThread* Thread = FRunnableThread::Create(
			Runnable,
			*ThreadName,
			0,
			TPri_BelowNormal
		);

		WorkerRunnables.Add(Runnable);
		WorkerThreads.Add(Thread);
	}

	UE_LOG(LogTemp, Log, TEXT("Started %d terrain worker threads"), Config.NumWorkerThreads);
}

void UTerrainStreamingManager::StopWorkerThreads()
{
	if (WorkerThreads.Num() == 0)
	{
		return;
	}

	// Signal shutdown
	bIsShuttingDown = true;

	// Wait for threads to finish
	for (FRunnableThread* Thread : WorkerThreads)
	{
		if (Thread)
		{
			Thread->WaitForCompletion();
			delete Thread;
		}
	}

	// Clean up runnables
	for (FRunnable* Runnable : WorkerRunnables)
	{
		if (Runnable)
		{
			delete Runnable;
		}
	}

	WorkerThreads.Empty();
	WorkerRunnables.Empty();

	UE_LOG(LogTemp, Log, TEXT("Stopped terrain worker threads"));
}

void UTerrainStreamingManager::WorkerThreadFunction()
{
	while (!bIsShuttingDown)
	{
		FTileLoadRequest Request;

		// Try to dequeue a request
		if (PendingQueue.Dequeue(Request))
		{
			// Process the request
			ProcessTileRequest(Request);

			// Add to completed queue
			CompletedQueue.Enqueue(Request);
		}
		else
		{
			// No work available, sleep briefly
			FPlatformProcess::Sleep(0.001f); // 1ms
		}
	}
}

void UTerrainStreamingManager::ProcessTileRequest(FTileLoadRequest& Request)
{
	double StartTime = FPlatformTime::Seconds();

	try
	{
		// Enhanced terrain generation with advanced features
		Request.GeneratedTile = GenerateEnhancedTerrainTile(
			Request.TilePosition,
			Request.TileSize,
			Request.Resolution,
			Request.LODLevel,
			Request.GenerationConfig
		);

		Request.bIsComplete = true;
		Request.bHasError = false;
	}
	catch (...)
	{
		UE_LOG(LogTemp, Error, TEXT("Error generating terrain tile at (%.1f, %.1f)"),
			Request.TilePosition.X, Request.TilePosition.Y);

		Request.bIsComplete = true;
		Request.bHasError = true;
	}

	double ElapsedMS = (FPlatformTime::Seconds() - StartTime) * 1000.0;

	UE_LOG(LogTemp, Verbose, TEXT("Generated enhanced tile at (%.1f, %.1f) LOD %d in %.2f ms"),
		Request.TilePosition.X, Request.TilePosition.Y, Request.LODLevel, ElapsedMS);
}

FTerrainTileData UTerrainStreamingManager::GenerateEnhancedTerrainTile(
	FVector2D TilePosition,
	float TileSize,
	int32 Resolution,
	int32 LODLevel,
	const FTerrainGenerationConfig& Config)
{
	FTerrainTileData TileData;
	TileData.WorldPosition = TilePosition;
	TileData.TileSize = TileSize;
	TileData.Resolution = Resolution;
	TileData.LODLevel = LODLevel;

	// Initialize heightmap
	TileData.HeightMap.SetNumUninitialized(Resolution * Resolution);
	TileData.NormalMap.SetNumUninitialized(Resolution * Resolution);
	TileData.BiomeMap.SetNumUninitialized(Resolution * Resolution);

	// Generate base terrain using continental-scale noise
	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			int32 Index = Y * Resolution + X;
			
			// Calculate world coordinates
			float WorldX = TilePosition.X + (X / float(Resolution)) * TileSize;
			float WorldY = TilePosition.Y + (Y / float(Resolution)) * TileSize;

			// Generate base terrain height
			float BaseHeight = UProceduralNoiseGenerator::GenerateContinentalTerrain(
				WorldX, WorldY, Config.Seed, Config.PlanetRadius);

			// Apply biome-specific features
			EBiomeType DominantBiome = GetDominantBiomeAtPosition(WorldX, WorldY, Config);
			float BiomeHeight = UProceduralNoiseGenerator::GenerateBiomeFeatures(
				WorldX, WorldY, DominantBiome, Config.Seed + 1, BaseHeight);

			// Apply erosion effects
			float ErodedHeight = ApplyErosionEffects(BiomeHeight, WorldX, WorldY, Config);

			// Store final height
			TileData.HeightMap[Index] = ErodedHeight;

			// Generate normal map
			TileData.NormalMap[Index] = CalculateTerrainNormal(
				WorldX, WorldY, ErodedHeight, Config);

			// Store biome information
			TileData.BiomeMap[Index] = static_cast<uint8>(DominantBiome);
		}
	}

	// Generate additional data layers
	// GenerateAdditionalDataLayers(TileData, Config); // Disabled - method not implemented

	// Apply post-processing effects
	// ApplyPostProcessingEffects(TileData, Config); // Disabled - method not implemented

	return TileData;
}

EBiomeType UTerrainStreamingManager::GetDominantBiomeAtPosition(
	float X, float Y, const FTerrainGenerationConfig& Config)
{
	// Use temperature and humidity to determine biome
	float Temperature = CalculateTemperatureAtPosition(X, Y, Config);
	float Humidity = CalculateHumidityAtPosition(X, Y, Config);
	float Altitude = UProceduralNoiseGenerator::GenerateContinentalTerrain(
		X, Y, Config.Seed, Config.PlanetRadius);

	// Simplified biome determination logic
	if (Altitude < 0.0f) // Below sea level
	{
		return EBiomeType::Ocean;
	}
	else if (Temperature < -10.0f)
	{
		return Humidity > 0.5f ? EBiomeType::Tundra : EBiomeType::Alpine;
	}
	else if (Temperature < 15.0f)
	{
		return Humidity > 0.6f ? EBiomeType::Forest : EBiomeType::Grassland;
	}
	else if (Temperature < 25.0f)
	{
		return Humidity > 0.4f ? EBiomeType::Savanna : EBiomeType::Desert;
	}
	else
	{
		return Humidity > 0.7f ? EBiomeType::Rainforest : EBiomeType::Desert;
	}
}

float UTerrainStreamingManager::ApplyErosionEffects(
	float BaseHeight, float X, float Y, const FTerrainGenerationConfig& Config)
{
	// Apply thermal erosion for slope smoothing
	float Slope = CalculateSlopeAtPosition(X, Y, Config);
	if (Slope > Config.TalusAngle)
	{
		BaseHeight *= 0.95f; // Reduce height on steep slopes
	}

	// Apply hydraulic erosion for river valleys
	float RiverDepth = UProceduralNoiseGenerator::GenerateRiverNetwork(
		X, Y, Config.Seed + 2, BaseHeight);
	BaseHeight -= RiverDepth;

	// Apply volcanic features if enabled
	if (Config.bGenerateVolcanoes)
	{
		BaseHeight = UProceduralNoiseGenerator::GenerateVolcanicTerrain(
			X, Y, Config.Seed + 3, BaseHeight);
	}

	// Apply crater field if this is a celestial body
	if (Config.bGenerateCraters)
	{
		BaseHeight = UProceduralNoiseGenerator::GenerateCraterField(
			X, Y, Config.Seed + 4, Config.CraterDensity, Config.MaxCraterSize);
	}

	return BaseHeight;
}

FVector4 UTerrainStreamingManager::CalculateTerrainNormal(
	float X, float Y, float Height, const FTerrainGenerationConfig& Config)
{
	// Calculate height at neighboring points
	float Offset = Config.TerrainScale * 0.1f;
	float HRight = UProceduralNoiseGenerator::GenerateContinentalTerrain(
		X + Offset, Y, Config.Seed, Config.PlanetRadius);
	float HLeft = UProceduralNoiseGenerator::GenerateContinentalTerrain(
		X - Offset, Y, Config.Seed, Config.PlanetRadius);
	float HUp = UProceduralNoiseGenerator::GenerateContinentalTerrain(
		X, Y + Offset, Config.Seed, Config.PlanetRadius);
	float HDown = UProceduralNoiseGenerator::GenerateContinentalTerrain(
		X, Y - Offset, Config.Seed, Config.PlanetRadius);

	// Calculate normal using finite differences
	FVector Normal = FVector(HLeft - HRight, HDown - HUp, 2.0f * Offset);
	Normal.Normalize();

	return FVector4(Normal.X, Normal.Y, Normal.Z, 0.0f);
}

void UTerrainStreamingManager::GenerateAdditionalDataLayers(
	FTerrainTileData& TileData, const FTerrainGenerationConfig& Config)
{
	int32 Resolution = TileData.Resolution;
	
	// Generate cave system data
	TileData.CaveMap.SetNumUninitialized(Resolution * Resolution);
	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			int32 Index = Y * Resolution + X;
			float WorldX = TileData.WorldPosition.X + (X / float(Resolution)) * TileData.TileSize;
			float WorldY = TileData.WorldPosition.Y + (Y / float(Resolution)) * TileData.TileSize;
			float WorldZ = TileData.HeightMap[Index];
			
			TileData.CaveMap[Index] = UProceduralNoiseGenerator::GenerateCaveSystem(
				WorldX, WorldY, WorldZ, Config.Seed + 5);
		}
	}

	// Generate mineral deposit data
	TileData.MineralMap.SetNumUninitialized(Resolution * Resolution);
	GenerateMineralDeposits(TileData, Config);
}

void UTerrainStreamingManager::ApplyPostProcessingEffects(
	FTerrainTileData& TileData, const FTerrainGenerationConfig& Config)
{
	// Apply smoothing filter to reduce artifacts
	if (Config.bApplySmoothing)
	{
		ApplySmoothingFilter(TileData.HeightMap, TileData.Resolution);
	}

	// Apply edge blending for seamless tiles
	if (Config.bEnableEdgeBlending)
	{
		ApplyEdgeBlending(TileData);
	}

	// Generate LOD-specific optimizations
	ApplyLODOptimizations(TileData, Config);
}

void UTerrainStreamingManager::UpdateStats(float LoadTimeMS)
{
	// Add to samples
	LoadTimeSamples.Add(LoadTimeMS);

	// Keep only recent samples
	if (LoadTimeSamples.Num() > MaxLoadTimeSamples)
	{
		LoadTimeSamples.RemoveAt(0);
	}

	// Calculate average
	float Sum = 0.0f;
	for (float Sample : LoadTimeSamples)
	{
		Sum += Sample;
	}

	Stats.AverageLoadTime = LoadTimeSamples.Num() > 0 ? Sum / LoadTimeSamples.Num() : 0.0f;
}
