// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
// #include "Engine/ContentStreaming.h" // Temporarily disabled
// #include "Engine/StaticMeshStreamingManager.h" // Not available in UE5.6
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "MemoryOptimizationManager.generated.h"

/**
 * Memory optimization strategy
 */
UENUM(BlueprintType)
enum class EMemoryOptimizationStrategy : uint8
{
    Aggressive      UMETA(DisplayName = "Aggressive - Maximum Memory Savings"),
    Balanced        UMETA(DisplayName = "Balanced - Performance/Memory Trade-off"),
    Conservative    UMETA(DisplayName = "Conservative - Prefer Performance"),
    Disabled        UMETA(DisplayName = "Disabled")
};

/**
 * Object pool for frequently spawned actors
 */
USTRUCT(BlueprintType)
struct FObjectPool
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<AActor*> AvailableObjects;

    UPROPERTY()
    TArray<AActor*> ActiveObjects;

    UPROPERTY()
    UClass* PooledClass;

    UPROPERTY()
    int32 MaxPoolSize;

    UPROPERTY()
    int32 InitialPoolSize;

    FObjectPool()
        : PooledClass(nullptr)
        , MaxPoolSize(100)
        , InitialPoolSize(10)
    {}
};

/**
 * Memory usage statistics
 */
USTRUCT(BlueprintType)
struct FMemoryStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TotalAllocatedMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedPhysicalMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedVirtualMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float PeakUsedPhysicalMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TextureMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MeshMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float AudioMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 ObjectCount;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 ActorCount;

    FMemoryStats()
        : TotalAllocatedMB(0.0f)
        , UsedPhysicalMB(0.0f)
        , UsedVirtualMB(0.0f)
        , PeakUsedPhysicalMB(0.0f)
        , TextureMemoryMB(0.0f)
        , MeshMemoryMB(0.0f)
        , AudioMemoryMB(0.0f)
        , ObjectCount(0)
        , ActorCount(0)
    {}
};

/**
 * Garbage collection configuration
 */
USTRUCT(BlueprintType)
struct FGarbageCollectionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GC")
    bool bEnableAutoGC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GC")
    float GCInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GC")
    float MemoryThresholdMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GC")
    bool bPurgeUnusedAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GC")
    float AssetUnusedTime;

    FGarbageCollectionConfig()
        : bEnableAutoGC(true)
        , GCInterval(60.0f)
        , MemoryThresholdMB(3072.0f)
        , bPurgeUnusedAssets(true)
        , AssetUnusedTime(300.0f)
    {}
};

/**
 * Asset streaming configuration
 */
USTRUCT(BlueprintType)
struct FAlexanderStreamingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bEnableTextureStreaming;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bEnableMeshLODStreaming;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingDistanceScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 TexturePoolSizeMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bEnableAsyncLoading;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 MaxAsyncLoadConcurrency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float CriticalMemoryMipBias;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float CriticalMemoryLODBias;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bEnableLevelStreaming;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float LevelStreamingMemoryLimitMB;

    FAlexanderStreamingConfig()
        : bEnableTextureStreaming(true)
        , bEnableMeshLODStreaming(true)
        , StreamingDistanceScale(1.0f)
        , TexturePoolSizeMB(2048)
        , bEnableAsyncLoading(true)
        , MaxAsyncLoadConcurrency(5)
        , CriticalMemoryMipBias(2.0f)
        , CriticalMemoryLODBias(2.0f)
        , bEnableLevelStreaming(true)
        , LevelStreamingMemoryLimitMB(512.0f)
    {}
};

/**
 * Delegate for memory warnings
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMemoryWarning, float, CurrentMemoryMB, float, ThresholdMB);

/**
 * Memory Optimization Manager Component
 * Manages memory usage and optimization strategies
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UMemoryOptimizationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UMemoryOptimizationManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory|Settings")
    EMemoryOptimizationStrategy OptimizationStrategy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory|Settings")
    FGarbageCollectionConfig GCConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory|Settings")
    FAlexanderStreamingConfig StreamingConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory|Settings")
    bool bEnableObjectPooling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory|Settings")
    bool bEnableMemoryProfiling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory|Settings")
    float MemoryCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory|Settings")
    float WarningThresholdMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory|Settings")
    float CriticalThresholdMB;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Memory|Events")
    FOnMemoryWarning OnMemoryWarning;

    UPROPERTY(BlueprintAssignable, Category = "Memory|Events")
    FOnMemoryWarning OnMemoryCritical;

    // Object Pooling
    UFUNCTION(BlueprintCallable, Category = "Memory|ObjectPool")
    void CreateObjectPool(UClass* PoolClass, int32 InitialSize, int32 MaxSize);

    UFUNCTION(BlueprintCallable, Category = "Memory|ObjectPool")
    AActor* GetPooledObject(UClass* PoolClass, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Memory|ObjectPool")
    void ReturnPooledObject(AActor* Object);

    UFUNCTION(BlueprintCallable, Category = "Memory|ObjectPool")
    void ClearObjectPool(UClass* PoolClass);

    UFUNCTION(BlueprintCallable, Category = "Memory|ObjectPool")
    void ClearAllObjectPools();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Memory|ObjectPool")
    int32 GetPoolSize(UClass* PoolClass) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Memory|ObjectPool")
    int32 GetActivePooledObjects(UClass* PoolClass) const;

    // Garbage Collection
    UFUNCTION(BlueprintCallable, Category = "Memory|GarbageCollection")
    void ForceGarbageCollection(bool bFullPurge);

    UFUNCTION(BlueprintCallable, Category = "Memory|GarbageCollection")
    void PurgeUnusedAssets();

    UFUNCTION(BlueprintCallable, Category = "Memory|GarbageCollection")
    void SetGarbageCollectionConfig(const FGarbageCollectionConfig& NewConfig);

    // Memory Statistics
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Memory|Stats")
    FMemoryStats GetCurrentMemoryStats() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Memory|Stats")
    float GetMemoryUsageMB() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Memory|Stats")
    float GetPeakMemoryUsageMB() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Memory|Stats")
    float GetAvailableMemoryMB() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Memory|Stats")
    int32 GetTotalObjectCount() const;

    // Asset Management
    UFUNCTION(BlueprintCallable, Category = "Memory|Assets")
    void UnloadUnusedAssets();

    UFUNCTION(BlueprintCallable, Category = "Memory|Assets")
    void FlushStreamingManagers();

    UFUNCTION(BlueprintCallable, Category = "Memory|Assets")
    void SetStreamingConfig(const FAlexanderStreamingConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Memory|Assets")
    void PreloadAssets(const TArray<FSoftObjectPath>& AssetPaths);

    UFUNCTION(BlueprintCallable, Category = "Memory|Assets")
    void UnloadAssets(const TArray<FSoftObjectPath>& AssetPaths);

    // Optimization
    UFUNCTION(BlueprintCallable, Category = "Memory|Optimization")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Memory|Optimization")
    void SetOptimizationStrategy(EMemoryOptimizationStrategy NewStrategy);

    UFUNCTION(BlueprintCallable, Category = "Memory|Optimization")
    TArray<FString> GetMemoryOptimizationSuggestions() const;

    UFUNCTION(BlueprintCallable, Category = "Memory|Optimization")
    bool IsMemoryHealthy() const;

    // Leak Detection
    UFUNCTION(BlueprintCallable, Category = "Memory|Debugging")
    void StartLeakDetection();

    UFUNCTION(BlueprintCallable, Category = "Memory|Debugging")
    void StopLeakDetection();

    UFUNCTION(BlueprintCallable, Category = "Memory|Debugging")
    TArray<FString> GetPotentialMemoryLeaks() const;

    UFUNCTION(BlueprintCallable, Category = "Memory|Debugging")
    FString GenerateMemoryReport() const;

    // Streaming Integration
    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void InitializeStreamingIntegration();

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void ShutdownStreamingIntegration();

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void OnTextureMemoryLimit(float CurrentMemoryMB, float ThresholdMB);

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void OnMeshMemoryLimit(float CurrentMemoryMB, float ThresholdMB);

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void OnAssetLoadPriorityChanged(const FSoftObjectPath& AssetPath, int32 NewPriority);

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void OnLevelStreamingMemoryImpact(float MemoryImpactMB);

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void UpdateStreamingSettings();

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void ForceLowerTextureMips();

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void ForceLowerMeshLODs();

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void CancelLowPriorityAssetLoads();

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void UnloadDistantLevels();

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void OptimizeMaterialInstances();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Memory|Streaming")
    FMemoryStats GetStreamingMemoryStats() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Memory|Streaming")
    int32 GetActiveAsyncLoads() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Memory|Streaming")
    int32 GetStreamingLevelsCount() const;

private:
    // Object pools
    UPROPERTY()
    TMap<UClass*, FObjectPool> ObjectPools;

    // Internal state
    float TimeSinceLastGC;
    float TimeSinceLastMemoryCheck;
    float TimeSinceLastLeakCheck;
    bool bLeakDetectionActive;
    
    // Memory tracking
    TMap<UObject*, double> TrackedObjects;
    TArray<FMemoryStats> MemoryHistory;
    FMemoryStats PeakMemoryStats;
    
    // Leak detection
    TMap<FString, int32> ObjectCountByClass;
    TMap<FString, int32> PreviousObjectCountByClass;
    
    // Internal functions
    void UpdateMemoryStats();
    void CheckMemoryThresholds();
    void PerformAutoGarbageCollection();
    void UpdateLeakDetection();
    void ApplyOptimizationStrategy();
    void OptimizeTextures();
    void OptimizeMeshes();
    void OptimizeActors();
    FMemoryStats CaptureMemoryStats() const;
    void LogMemoryStats(const FMemoryStats& Stats) const;
    
    // Pool management
    void InitializePool(UClass* PoolClass, int32 InitialSize);
    AActor* CreatePooledActor(UClass* PoolClass);
    void ResetPooledActor(AActor* Actor);
    
    // Streaming integration
    void RegisterTextureStreamingCallbacks();
    void RegisterMeshStreamingCallbacks();
    void RegisterAssetManagerCallbacks();
    void RegisterLevelStreamingCallbacks();
    void UnregisterAllStreamingCallbacks();
    void UpdateTextureStreamingSettings();
    void UpdateMeshStreamingSettings();
    void UpdateAsyncLoadingSettings();
    void UpdateLevelStreamingSettings();
    void TrackMaterialInstanceCreation(UMaterialInstanceDynamic* MaterialInstance);
    void OptimizeMaterialInstancePool();
    
    // Streaming state
    UPROPERTY()
    TMap<FString, int32> MaterialInstanceCountByMaterial;
    
    UPROPERTY()
    TArray<UMaterialInstanceDynamic*> TrackedMaterialInstances;
    
    UPROPERTY()
    TArray<FSoftObjectPath> ActiveAsyncLoads;
    
    UPROPERTY()
    TArray<FName> StreamingLevelNames;
    
    // Callback handles
    FDelegateHandle TextureStreamingHandle;
    FDelegateHandle MeshStreamingHandle;
    FDelegateHandle AssetManagerHandle;
    FDelegateHandle LevelStreamingHandle;
    
    // Streaming metrics
    float LastTextureMemoryUsage;
    float LastMeshMemoryUsage;
    int32 LastActiveAsyncLoadCount;
    int32 LastStreamingLevelCount;
};
