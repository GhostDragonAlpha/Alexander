// Copyright Epic Games, Inc. All Rights Reserved.

#include "MemoryOptimizationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformMemory.h"
#include "Misc/CoreDelegates.h"
#include "UObject/UObjectIterator.h"
#include "EngineUtils.h"
#include "Engine/Texture.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundWave.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Engine/ContentStreaming.h"
#include "Engine/StaticMeshStreamingManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"

UMemoryOptimizationManager::UMemoryOptimizationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    
    // Default settings
    OptimizationStrategy = EMemoryOptimizationStrategy::Balanced;
    bEnableObjectPooling = true;
    bEnableMemoryProfiling = true;
    MemoryCheckInterval = 5.0f;
    WarningThresholdMB = 3072.0f;   // 3 GB
    CriticalThresholdMB = 4096.0f;  // 4 GB
    
    // Internal state
    TimeSinceLastGC = 0.0f;
    TimeSinceLastMemoryCheck = 0.0f;
    TimeSinceLastLeakCheck = 0.0f;
    bLeakDetectionActive = false;
}

void UMemoryOptimizationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Capture initial memory state
    UpdateMemoryStats();
    
    // Apply optimization strategy
    ApplyOptimizationStrategy();
    
    // Initialize streaming integration
    InitializeStreamingIntegration();
    
    UE_LOG(LogTemp, Log, TEXT("Memory Optimization Manager: Initialized with %s strategy"),
        *UEnum::GetValueAsString(OptimizationStrategy));
}

void UMemoryOptimizationManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Shutdown streaming integration
    ShutdownStreamingIntegration();
    
    // Clean up object pools
    ClearAllObjectPools();
    
    // Generate final memory report
    FString FinalReport = GenerateMemoryReport();
    UE_LOG(LogTemp, Log, TEXT("Memory Optimization Manager: Final Report\n%s"), *FinalReport);
    
    Super::EndPlay(EndPlayReason);
}

void UMemoryOptimizationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OptimizationStrategy == EMemoryOptimizationStrategy::Disabled)
    {
        return;
    }
    
    // Update memory statistics periodically
    TimeSinceLastMemoryCheck += DeltaTime;
    if (TimeSinceLastMemoryCheck >= MemoryCheckInterval)
    {
        UpdateMemoryStats();
        CheckMemoryThresholds();
        TimeSinceLastMemoryCheck = 0.0f;
    }
    
    // Perform auto garbage collection
    TimeSinceLastGC += DeltaTime;
    if (GCConfig.bEnableAutoGC && TimeSinceLastGC >= GCConfig.GCInterval)
    {
        PerformAutoGarbageCollection();
        TimeSinceLastGC = 0.0f;
    }
    
    // Update leak detection
    if (bLeakDetectionActive)
    {
        TimeSinceLastLeakCheck += DeltaTime;
        if (TimeSinceLastLeakCheck >= 30.0f) // Check every 30 seconds
        {
            UpdateLeakDetection();
            TimeSinceLastLeakCheck = 0.0f;
        }
    }
}

void UMemoryOptimizationManager::CreateObjectPool(UClass* PoolClass, int32 InitialSize, int32 MaxSize)
{
    if (!PoolClass || !bEnableObjectPooling)
    {
        return;
    }
    
    if (ObjectPools.Contains(PoolClass))
    {
        UE_LOG(LogTemp, Warning, TEXT("Memory Manager: Pool for class %s already exists"), *PoolClass->GetName());
        return;
    }
    
    FObjectPool NewPool;
    NewPool.PooledClass = PoolClass;
    NewPool.InitialPoolSize = InitialSize;
    NewPool.MaxPoolSize = MaxSize;
    
    ObjectPools.Add(PoolClass, NewPool);
    InitializePool(PoolClass, InitialSize);
    
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Created object pool for %s (Initial: %d, Max: %d)"),
        *PoolClass->GetName(), InitialSize, MaxSize);
}

AActor* UMemoryOptimizationManager::GetPooledObject(UClass* PoolClass, FVector Location, FRotator Rotation)
{
    if (!PoolClass || !bEnableObjectPooling)
    {
        return nullptr;
    }
    
    FObjectPool* Pool = ObjectPools.Find(PoolClass);
    if (!Pool)
    {
        // Auto-create pool if it doesn't exist
        CreateObjectPool(PoolClass, 10, 100);
        Pool = ObjectPools.Find(PoolClass);
    }
    
    AActor* PooledActor = nullptr;
    
    if (Pool->AvailableObjects.Num() > 0)
    {
        // Reuse existing object
        PooledActor = Pool->AvailableObjects.Pop();
        ResetPooledActor(PooledActor);
    }
    else if (Pool->ActiveObjects.Num() < Pool->MaxPoolSize)
    {
        // Create new object if under max size
        PooledActor = CreatePooledActor(PoolClass);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Memory Manager: Pool for %s reached max size"), *PoolClass->GetName());
        return nullptr;
    }
    
    if (PooledActor)
    {
        PooledActor->SetActorLocationAndRotation(Location, Rotation);
        PooledActor->SetActorHiddenInGame(false);
        PooledActor->SetActorEnableCollision(true);
        Pool->ActiveObjects.Add(PooledActor);
    }
    
    return PooledActor;
}

void UMemoryOptimizationManager::ReturnPooledObject(AActor* Object)
{
    if (!Object || !bEnableObjectPooling)
    {
        return;
    }
    
    UClass* ObjectClass = Object->GetClass();
    FObjectPool* Pool = ObjectPools.Find(ObjectClass);
    
    if (Pool && Pool->ActiveObjects.Contains(Object))
    {
        Pool->ActiveObjects.Remove(Object);
        Pool->AvailableObjects.Add(Object);
        
        // Hide and disable the object
        Object->SetActorHiddenInGame(true);
        Object->SetActorEnableCollision(false);
        Object->SetActorLocation(FVector(0, 0, -10000)); // Move far away
    }
}

void UMemoryOptimizationManager::ClearObjectPool(UClass* PoolClass)
{
    FObjectPool* Pool = ObjectPools.Find(PoolClass);
    if (!Pool)
    {
        return;
    }
    
    // Destroy all objects in the pool
    for (AActor* Actor : Pool->AvailableObjects)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }
    
    for (AActor* Actor : Pool->ActiveObjects)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }
    
    ObjectPools.Remove(PoolClass);
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Cleared object pool for %s"), *PoolClass->GetName());
}

void UMemoryOptimizationManager::ClearAllObjectPools()
{
    TArray<UClass*> PoolClasses;
    ObjectPools.GetKeys(PoolClasses);
    
    for (UClass* PoolClass : PoolClasses)
    {
        ClearObjectPool(PoolClass);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Cleared all object pools"));
}

int32 UMemoryOptimizationManager::GetPoolSize(UClass* PoolClass) const
{
    const FObjectPool* Pool = ObjectPools.Find(PoolClass);
    return Pool ? (Pool->AvailableObjects.Num() + Pool->ActiveObjects.Num()) : 0;
}

int32 UMemoryOptimizationManager::GetActivePooledObjects(UClass* PoolClass) const
{
    const FObjectPool* Pool = ObjectPools.Find(PoolClass);
    return Pool ? Pool->ActiveObjects.Num() : 0;
}

void UMemoryOptimizationManager::ForceGarbageCollection(bool bFullPurge)
{
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Forcing garbage collection (Full Purge: %s)"),
        bFullPurge ? TEXT("Yes") : TEXT("No"));
    
    if (bFullPurge)
    {
        CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS, true);
    }
    else
    {
        CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS, false);
    }
    
    TimeSinceLastGC = 0.0f;
}

void UMemoryOptimizationManager::PurgeUnusedAssets()
{
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Purging unused assets"));
    
    // This would integrate with the asset manager
    if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
    {
        // Unload unused assets
        AssetManager->GetStreamableManager().RequestAsyncLoad(TArray<FSoftObjectPath>());
    }
    
    ForceGarbageCollection(true);
}

void UMemoryOptimizationManager::SetGarbageCollectionConfig(const FGarbageCollectionConfig& NewConfig)
{
    GCConfig = NewConfig;
    TimeSinceLastGC = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Updated GC config (Interval: %.1fs, Threshold: %.0fMB)"),
        GCConfig.GCInterval, GCConfig.MemoryThresholdMB);
}

FMemoryStats UMemoryOptimizationManager::GetCurrentMemoryStats() const
{
    return CaptureMemoryStats();
}

float UMemoryOptimizationManager::GetMemoryUsageMB() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

float UMemoryOptimizationManager::GetPeakMemoryUsageMB() const
{
    return PeakMemoryStats.PeakUsedPhysicalMB;
}

float UMemoryOptimizationManager::GetAvailableMemoryMB() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.AvailablePhysical / (1024.0f * 1024.0f);
}

int32 UMemoryOptimizationManager::GetTotalObjectCount() const
{
    return GUObjectArray.GetObjectArrayNum();
}

void UMemoryOptimizationManager::UnloadUnusedAssets()
{
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Unloading unused assets"));
    PurgeUnusedAssets();
}

void UMemoryOptimizationManager::FlushStreamingManagers()
{
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Flushing streaming managers"));
    
    if (UWorld* World = GetWorld())
    {
        World->FlushLevelStreaming();
    }
}

void UMemoryOptimizationManager::SetStreamingConfig(const FAlexanderStreamingConfig& NewConfig)
{
    StreamingConfig = NewConfig;
    
    // Apply streaming settings
    // This would integrate with UE5's streaming systems
    
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Updated streaming config"));
}

void UMemoryOptimizationManager::PreloadAssets(const TArray<FSoftObjectPath>& AssetPaths)
{
    if (AssetPaths.Num() == 0)
    {
        return;
    }
    
    if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
    {
        AssetManager->GetStreamableManager().RequestAsyncLoad(AssetPaths);
        UE_LOG(LogTemp, Log, TEXT("Memory Manager: Preloading %d assets"), AssetPaths.Num());
    }
}

void UMemoryOptimizationManager::UnloadAssets(const TArray<FSoftObjectPath>& AssetPaths)
{
    if (AssetPaths.Num() == 0)
    {
        return;
    }
    
    // Mark assets for unloading
    for (const FSoftObjectPath& Path : AssetPaths)
    {
        if (UObject* Asset = Path.ResolveObject())
        {
            Asset->ConditionalBeginDestroy();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Unloading %d assets"), AssetPaths.Num());
}

void UMemoryOptimizationManager::OptimizeMemoryUsage()
{
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Running memory optimization"));
    
    switch (OptimizationStrategy)
    {
        case EMemoryOptimizationStrategy::Aggressive:
            ForceGarbageCollection(true);
            PurgeUnusedAssets();
            OptimizeTextures();
            OptimizeMeshes();
            OptimizeActors();
            break;
            
        case EMemoryOptimizationStrategy::Balanced:
            ForceGarbageCollection(false);
            OptimizeTextures();
            break;
            
        case EMemoryOptimizationStrategy::Conservative:
            // Minimal optimization
            break;
            
        default:
            break;
    }
}

void UMemoryOptimizationManager::SetOptimizationStrategy(EMemoryOptimizationStrategy NewStrategy)
{
    OptimizationStrategy = NewStrategy;
    ApplyOptimizationStrategy();
    
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Changed optimization strategy to %s"),
        *UEnum::GetValueAsString(OptimizationStrategy));
}

TArray<FString> UMemoryOptimizationManager::GetMemoryOptimizationSuggestions() const
{
    TArray<FString> Suggestions;
    FMemoryStats CurrentStats = CaptureMemoryStats();
    
    if (CurrentStats.UsedPhysicalMB > WarningThresholdMB)
    {
        Suggestions.Add(TEXT("Consider reducing texture quality or resolution"));
        Suggestions.Add(TEXT("Enable aggressive LOD settings for distant meshes"));
        Suggestions.Add(TEXT("Implement object pooling for frequently spawned actors"));
    }
    
    if (CurrentStats.TextureMemoryMB > 1024.0f)
    {
        Suggestions.Add(TEXT("Texture memory is high - consider streaming or lower mip levels"));
    }
    
    if (CurrentStats.ActorCount > 5000)
    {
        Suggestions.Add(TEXT("High actor count - consider using instanced static meshes"));
    }
    
    if (ObjectPools.Num() == 0 && bEnableObjectPooling)
    {
        Suggestions.Add(TEXT("No object pools configured - consider pooling projectiles and effects"));
    }
    
    return Suggestions;
}

bool UMemoryOptimizationManager::IsMemoryHealthy() const
{
    float CurrentMemory = GetMemoryUsageMB();
    return CurrentMemory < WarningThresholdMB;
}

void UMemoryOptimizationManager::StartLeakDetection()
{
    bLeakDetectionActive = true;
    ObjectCountByClass.Empty();
    PreviousObjectCountByClass.Empty();
    TimeSinceLastLeakCheck = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Started memory leak detection"));
}

void UMemoryOptimizationManager::StopLeakDetection()
{
    bLeakDetectionActive = false;
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Stopped memory leak detection"));
}

TArray<FString> UMemoryOptimizationManager::GetPotentialMemoryLeaks() const
{
    TArray<FString> Leaks;
    
    for (const auto& Pair : ObjectCountByClass)
    {
        const FString& ClassName = Pair.Key;
        int32 CurrentCount = Pair.Value;
        
        if (const int32* PrevCount = PreviousObjectCountByClass.Find(ClassName))
        {
            int32 Difference = CurrentCount - *PrevCount;
            if (Difference > 100) // Significant increase
            {
                Leaks.Add(FString::Printf(TEXT("%s: +%d objects (Total: %d)"),
                    *ClassName, Difference, CurrentCount));
            }
        }
    }
    
    return Leaks;
}

FString UMemoryOptimizationManager::GenerateMemoryReport() const
{
    FString Report = TEXT("====================================\n");
    Report += TEXT("   MEMORY OPTIMIZATION REPORT\n");
    Report += TEXT("====================================\n\n");
    
    FMemoryStats CurrentStats = CaptureMemoryStats();
    
    Report += TEXT("--- Current Memory Usage ---\n");
    Report += FString::Printf(TEXT("Physical Memory: %.1fMB / %.1fMB Peak\n"),
        CurrentStats.UsedPhysicalMB, CurrentStats.PeakUsedPhysicalMB);
    Report += FString::Printf(TEXT("Virtual Memory: %.1fMB\n"), CurrentStats.UsedVirtualMB);
    Report += FString::Printf(TEXT("Texture Memory: %.1fMB\n"), CurrentStats.TextureMemoryMB);
    Report += FString::Printf(TEXT("Mesh Memory: %.1fMB\n"), CurrentStats.MeshMemoryMB);
    Report += FString::Printf(TEXT("Audio Memory: %.1fMB\n"), CurrentStats.AudioMemoryMB);
    Report += FString::Printf(TEXT("Total Objects: %d\n"), CurrentStats.ObjectCount);
    Report += FString::Printf(TEXT("Active Actors: %d\n\n"), CurrentStats.ActorCount);
    
    Report += TEXT("--- Object Pools ---\n");
    if (ObjectPools.Num() > 0)
    {
        for (const auto& Pair : ObjectPools)
        {
            const FObjectPool& Pool = Pair.Value;
            Report += FString::Printf(TEXT("%s: %d active, %d available (Max: %d)\n"),
                *Pool.PooledClass->GetName(),
                Pool.ActiveObjects.Num(),
                Pool.AvailableObjects.Num(),
                Pool.MaxPoolSize);
        }
    }
    else
    {
        Report += TEXT("No object pools configured\n");
    }
    
    Report += TEXT("\n--- Optimization Suggestions ---\n");
    TArray<FString> Suggestions = GetMemoryOptimizationSuggestions();
    if (Suggestions.Num() > 0)
    {
        for (const FString& Suggestion : Suggestions)
        {
            Report += TEXT("- ") + Suggestion + TEXT("\n");
        }
    }
    else
    {
        Report += TEXT("Memory usage is healthy\n");
    }
    
    Report += TEXT("\n====================================\n");
    return Report;
}

void UMemoryOptimizationManager::UpdateMemoryStats()
{
    if (!bEnableMemoryProfiling)
    {
        return;
    }
    
    FMemoryStats NewStats = CaptureMemoryStats();
    MemoryHistory.Add(NewStats);
    
    // Update peak stats
    if (NewStats.UsedPhysicalMB > PeakMemoryStats.PeakUsedPhysicalMB)
    {
        PeakMemoryStats = NewStats;
    }
    
    // Trim history to last 100 entries
    if (MemoryHistory.Num() > 100)
    {
        MemoryHistory.RemoveAt(0, MemoryHistory.Num() - 100);
    }
}

void UMemoryOptimizationManager::CheckMemoryThresholds()
{
    float CurrentMemory = GetMemoryUsageMB();
    
    if (CurrentMemory > CriticalThresholdMB)
    {
        UE_LOG(LogTemp, Error, TEXT("Memory Manager: CRITICAL - Memory usage %.1fMB exceeds critical threshold %.1fMB"),
            CurrentMemory, CriticalThresholdMB);
        OnMemoryCritical.Broadcast(CurrentMemory, CriticalThresholdMB);
        
        // Perform emergency optimization
        OptimizeMemoryUsage();
    }
    else if (CurrentMemory > WarningThresholdMB)
    {
        UE_LOG(LogTemp, Warning, TEXT("Memory Manager: WARNING - Memory usage %.1fMB exceeds warning threshold %.1fMB"),
            CurrentMemory, WarningThresholdMB);
        OnMemoryWarning.Broadcast(CurrentMemory, WarningThresholdMB);
    }
}

void UMemoryOptimizationManager::PerformAutoGarbageCollection()
{
    float CurrentMemory = GetMemoryUsageMB();
    
    if (CurrentMemory > GCConfig.MemoryThresholdMB)
    {
        UE_LOG(LogTemp, Log, TEXT("Memory Manager: Auto GC triggered (Memory: %.1fMB > Threshold: %.1fMB)"),
            CurrentMemory, GCConfig.MemoryThresholdMB);
        
        ForceGarbageCollection(GCConfig.bPurgeUnusedAssets);
    }
}

void UMemoryOptimizationManager::UpdateLeakDetection()
{
    PreviousObjectCountByClass = ObjectCountByClass;
    ObjectCountByClass.Empty();
    
    // Count objects by class
    for (TObjectIterator<UObject> It; It; ++It)
    {
        UObject* Object = *It;
        if (Object && Object->IsValidLowLevel())
        {
            FString ClassName = Object->GetClass()->GetName();
            int32& Count = ObjectCountByClass.FindOrAdd(ClassName, 0);
            Count++;
        }
    }
    
    // Check for potential leaks
    TArray<FString> Leaks = GetPotentialMemoryLeaks();
    if (Leaks.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Memory Manager: Detected %d potential memory leaks"), Leaks.Num());
        for (const FString& Leak : Leaks)
        {
            UE_LOG(LogTemp, Warning, TEXT("  %s"), *Leak);
        }
    }
}

void UMemoryOptimizationManager::ApplyOptimizationStrategy()
{
    switch (OptimizationStrategy)
    {
        case EMemoryOptimizationStrategy::Aggressive:
            GCConfig.GCInterval = 30.0f;
            GCConfig.MemoryThresholdMB = 2048.0f;
            StreamingConfig.StreamingDistanceScale = 0.7f;
            break;
            
        case EMemoryOptimizationStrategy::Balanced:
            GCConfig.GCInterval = 60.0f;
            GCConfig.MemoryThresholdMB = 3072.0f;
            StreamingConfig.StreamingDistanceScale = 1.0f;
            break;
            
        case EMemoryOptimizationStrategy::Conservative:
            GCConfig.GCInterval = 120.0f;
            GCConfig.MemoryThresholdMB = 4096.0f;
            StreamingConfig.StreamingDistanceScale = 1.3f;
            break;
            
        default:
            break;
    }
    
    // Update streaming settings based on new strategy
    UpdateStreamingSettings();
}

void UMemoryOptimizationManager::OptimizeTextures()
{
    // Texture optimization would be implemented here
    // Could include forcing lower mip levels, compressing textures, etc.
    UE_LOG(LogTemp, Verbose, TEXT("Memory Manager: Optimizing textures"));
}

void UMemoryOptimizationManager::OptimizeMeshes()
{
    // Mesh optimization would be implemented here
    // Could include forcing LODs, reducing poly counts, etc.
    UE_LOG(LogTemp, Verbose, TEXT("Memory Manager: Optimizing meshes"));
}

void UMemoryOptimizationManager::OptimizeActors()
{
    // Actor optimization would be implemented here
    // Could include removing distant actors, disabling ticking, etc.
    UE_LOG(LogTemp, Verbose, TEXT("Memory Manager: Optimizing actors"));
}

FMemoryStats UMemoryOptimizationManager::CaptureMemoryStats() const
{
    FMemoryStats Stats;
    
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Stats.UsedPhysicalMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    Stats.UsedVirtualMB = MemStats.UsedVirtual / (1024.0f * 1024.0f);
    Stats.TotalAllocatedMB = MemStats.TotalPhysical / (1024.0f * 1024.0f);
    Stats.PeakUsedPhysicalMB = MemStats.PeakUsedPhysical / (1024.0f * 1024.0f);
    
    // Count objects and actors
    Stats.ObjectCount = GUObjectArray.GetObjectArrayNum();
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            Stats.ActorCount++;
        }
    }
    
    return Stats;
}

void UMemoryOptimizationManager::LogMemoryStats(const FMemoryStats& Stats) const
{
    UE_LOG(LogTemp, Display, TEXT("=== Memory Stats ==="));
    UE_LOG(LogTemp, Display, TEXT("Physical: %.1fMB | Virtual: %.1fMB | Objects: %d | Actors: %d"),
        Stats.UsedPhysicalMB, Stats.UsedVirtualMB, Stats.ObjectCount, Stats.ActorCount);
}

void UMemoryOptimizationManager::InitializePool(UClass* PoolClass, int32 InitialSize)
{
    FObjectPool* Pool = ObjectPools.Find(PoolClass);
    if (!Pool)
    {
        return;
    }
    
    for (int32 i = 0; i < InitialSize; ++i)
    {
        AActor* NewActor = CreatePooledActor(PoolClass);
        if (NewActor)
        {
            Pool->AvailableObjects.Add(NewActor);
        }
    }
}

AActor* UMemoryOptimizationManager::CreatePooledActor(UClass* PoolClass)
{
    UWorld* World = GetWorld();
    if (!World || !PoolClass)
    {
        return nullptr;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AActor* NewActor = World->SpawnActor<AActor>(PoolClass, FVector(0, 0, -10000), FRotator::ZeroRotator, SpawnParams);
    if (NewActor)
    {
        NewActor->SetActorHiddenInGame(true);
        NewActor->SetActorEnableCollision(false);
    }
    
    return NewActor;
}

void UMemoryOptimizationManager::ResetPooledActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    // Reset actor to default state
    Actor->SetActorRotation(FRotator::ZeroRotator);
    Actor->SetActorScale3D(FVector(1, 1, 1));
}

void UMemoryOptimizationManager::InitializeStreamingIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Initializing streaming integration"));
    
    // Register callbacks with streaming managers
    RegisterTextureStreamingCallbacks();
    RegisterMeshStreamingCallbacks();
    RegisterAssetManagerCallbacks();
    RegisterLevelStreamingCallbacks();
    
    // Initialize streaming metrics
    LastTextureMemoryUsage = 0.0f;
    LastMeshMemoryUsage = 0.0f;
    LastActiveAsyncLoadCount = 0;
    LastStreamingLevelCount = 0;
    
    // Apply initial streaming settings
    UpdateStreamingSettings();
}

void UMemoryOptimizationManager::ShutdownStreamingIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Shutting down streaming integration"));
    
    // Unregister all streaming callbacks
    UnregisterAllStreamingCallbacks();
    
    // Clear streaming state
    ActiveAsyncLoads.Empty();
    StreamingLevelNames.Empty();
    TrackedMaterialInstances.Empty();
    MaterialInstanceCountByMaterial.Empty();
}

void UMemoryOptimizationManager::RegisterTextureStreamingCallbacks()
{
    if (GEngine && GEngine->TextureStreamingManager)
    {
        // Register texture memory limit callback
        TextureStreamingHandle = GEngine->TextureStreamingManager->SetMemoryLimitCallback(
            FMemoryLimitCallback::CreateUObject(this, &UMemoryOptimizationManager::OnTextureMemoryLimit)
        );
        
        UE_LOG(LogTemp, Log, TEXT("Memory Manager: Registered texture streaming callbacks"));
    }
}

void UMemoryOptimizationManager::RegisterMeshStreamingCallbacks()
{
    if (GEngine && GEngine->StaticMeshStreamingManager)
    {
        // Register mesh memory limit callback
        MeshStreamingHandle = GEngine->StaticMeshStreamingManager->SetMemoryLimitCallback(
            FMemoryLimitCallback::CreateUObject(this, &UMemoryOptimizationManager::OnMeshMemoryLimit)
        );
        
        UE_LOG(LogTemp, Log, TEXT("Memory Manager: Registered mesh streaming callbacks"));
    }
}

void UMemoryOptimizationManager::RegisterAssetManagerCallbacks()
{
    if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
    {
        // Register asset load priority changed callback
        AssetManagerHandle = AssetManager->OnAssetLoadPriorityChanged.AddUObject(
            this, &UMemoryOptimizationManager::OnAssetLoadPriorityChanged
        );
        
        UE_LOG(LogTemp, Log, TEXT("Memory Manager: Registered asset manager callbacks"));
    }
}

void UMemoryOptimizationManager::RegisterLevelStreamingCallbacks()
{
    if (UWorld* World = GetWorld())
    {
        // Register level streaming callbacks
        LevelStreamingHandle = World->OnLevelAddedToWorld.AddUObject(
            this, &UMemoryOptimizationManager::OnLevelStreamingMemoryImpact
        );
        
        UE_LOG(LogTemp, Log, TEXT("Memory Manager: Registered level streaming callbacks"));
    }
}

void UMemoryOptimizationManager::UnregisterAllStreamingCallbacks()
{
    if (GEngine && GEngine->TextureStreamingManager && TextureStreamingHandle.IsValid())
    {
        GEngine->TextureStreamingManager->RemoveMemoryLimitCallback(TextureStreamingHandle);
    }
    
    if (GEngine && GEngine->StaticMeshStreamingManager && MeshStreamingHandle.IsValid())
    {
        GEngine->StaticMeshStreamingManager->RemoveMemoryLimitCallback(MeshStreamingHandle);
    }
    
    if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
    {
        AssetManager->OnAssetLoadPriorityChanged.Remove(AssetManagerHandle);
    }
    
    if (UWorld* World = GetWorld())
    {
        World->OnLevelAddedToWorld.Remove(LevelStreamingHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Unregistered all streaming callbacks"));
}

void UMemoryOptimizationManager::OnTextureMemoryLimit(float CurrentMemoryMB, float ThresholdMB)
{
    UE_LOG(LogTemp, Warning, TEXT("Memory Manager: Texture memory limit reached (%.1fMB / %.1fMB)"),
        CurrentMemoryMB, ThresholdMB);
    
    // Force lower mip levels when texture memory is critical
    if (CurrentMemoryMB > ThresholdMB * 0.9f)
    {
        ForceLowerTextureMips();
    }
    
    // Update streaming settings based on memory pressure
    UpdateTextureStreamingSettings();
}

void UMemoryOptimizationManager::OnMeshMemoryLimit(float CurrentMemoryMB, float ThresholdMB)
{
    UE_LOG(LogTemp, Warning, TEXT("Memory Manager: Mesh memory limit reached (%.1fMB / %.1fMB)"),
        CurrentMemoryMB, ThresholdMB);
    
    // Force lower LODs when mesh memory is critical
    if (CurrentMemoryMB > ThresholdMB * 0.9f)
    {
        ForceLowerMeshLODs();
    }
    
    // Update streaming settings based on memory pressure
    UpdateMeshStreamingSettings();
}

void UMemoryOptimizationManager::OnAssetLoadPriorityChanged(const FSoftObjectPath& AssetPath, int32 NewPriority)
{
    UE_LOG(LogTemp, Verbose, TEXT("Memory Manager: Asset load priority changed for %s (Priority: %d)"),
        *AssetPath.ToString(), NewPriority);
    
    // Track active async loads
    if (NewPriority > 0)
    {
        if (!ActiveAsyncLoads.Contains(AssetPath))
        {
            ActiveAsyncLoads.Add(AssetPath);
        }
    }
    else
    {
        ActiveAsyncLoads.Remove(AssetPath);
    }
}

void UMemoryOptimizationManager::OnLevelStreamingMemoryImpact(float MemoryImpactMB)
{
    UE_LOG(LogTemp, Verbose, TEXT("Memory Manager: Level streaming memory impact: %.1fMB"),
        MemoryImpactMB);
    
    // Check if level streaming exceeds memory limits
    if (MemoryImpactMB > StreamingConfig.LevelStreamingMemoryLimitMB)
    {
        UE_LOG(LogTemp, Warning, TEXT("Memory Manager: Level streaming memory impact exceeds limit"));
        UnloadDistantLevels();
    }
}

void UMemoryOptimizationManager::UpdateStreamingSettings()
{
    UpdateTextureStreamingSettings();
    UpdateMeshStreamingSettings();
    UpdateAsyncLoadingSettings();
    UpdateLevelStreamingSettings();
}

void UMemoryOptimizationManager::UpdateTextureStreamingSettings()
{
    if (GEngine && GEngine->TextureStreamingManager)
    {
        // Adjust texture streaming pool size based on optimization strategy
        int32 NewPoolSize = StreamingConfig.TexturePoolSizeMB;
        
        switch (OptimizationStrategy)
        {
            case EMemoryOptimizationStrategy::Aggressive:
                NewPoolSize = FMath::FloorToInt(NewPoolSize * 0.7f);
                break;
            case EMemoryOptimizationStrategy::Balanced:
                NewPoolSize = FMath::FloorToInt(NewPoolSize * 0.85f);
                break;
            default:
                break;
        }
        
        GEngine->TextureStreamingManager->SetPoolSize(NewPoolSize);
        UE_LOG(LogTemp, Log, TEXT("Memory Manager: Updated texture pool size to %dMB"), NewPoolSize);
    }
}

void UMemoryOptimizationManager::UpdateMeshStreamingSettings()
{
    if (GEngine && GEngine->StaticMeshStreamingManager)
    {
        // Adjust mesh streaming distance scale based on optimization strategy
        float NewDistanceScale = StreamingConfig.StreamingDistanceScale;
        
        switch (OptimizationStrategy)
        {
            case EMemoryOptimizationStrategy::Aggressive:
                NewDistanceScale *= 0.7f;
                break;
            case EMemoryOptimizationStrategy::Balanced:
                NewDistanceScale *= 0.85f;
                break;
            default:
                break;
        }
        
        GEngine->StaticMeshStreamingManager->SetDistanceScale(NewDistanceScale);
        UE_LOG(LogTemp, Log, TEXT("Memory Manager: Updated mesh streaming distance scale to %.2f"), NewDistanceScale);
    }
}

void UMemoryOptimizationManager::UpdateAsyncLoadingSettings()
{
    if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
    {
        // Adjust async loading concurrency based on optimization strategy
        int32 NewConcurrency = StreamingConfig.MaxAsyncLoadConcurrency;
        
        switch (OptimizationStrategy)
        {
            case EMemoryOptimizationStrategy::Aggressive:
                NewConcurrency = FMath::Max(1, FMath::FloorToInt(NewConcurrency * 0.5f));
                break;
            case EMemoryOptimizationStrategy::Balanced:
                NewConcurrency = FMath::Max(2, FMath::FloorToInt(NewConcurrency * 0.75f));
                break;
            default:
                break;
        }
        
        AssetManager->GetStreamableManager().SetConcurrencyLimit(NewConcurrency);
        UE_LOG(LogTemp, Log, TEXT("Memory Manager: Updated async loading concurrency to %d"), NewConcurrency);
    }
}

void UMemoryOptimizationManager::UpdateLevelStreamingSettings()
{
    if (UWorld* World = GetWorld())
    {
        // Update level streaming memory limit
        World->SetLevelStreamingMemoryLimit(StreamingConfig.LevelStreamingMemoryLimitMB);
        UE_LOG(LogTemp, Log, TEXT("Memory Manager: Updated level streaming memory limit to %.0fMB"),
            StreamingConfig.LevelStreamingMemoryLimitMB);
    }
}

void UMemoryOptimizationManager::ForceLowerTextureMips()
{
    UE_LOG(LogTemp, Warning, TEXT("Memory Manager: Forcing lower texture mip levels"));
    
    if (GEngine && GEngine->TextureStreamingManager)
    {
        // Force lower mip levels for all streaming textures
        GEngine->TextureStreamingManager->SetGlobalMipBias(StreamingConfig.CriticalMemoryMipBias);
    }
}

void UMemoryOptimizationManager::ForceLowerMeshLODs()
{
    UE_LOG(LogTemp, Warning, TEXT("Memory Manager: Forcing lower mesh LODs"));
    
    if (GEngine && GEngine->StaticMeshStreamingManager)
    {
        // Force lower LODs for all streaming meshes
        GEngine->StaticMeshStreamingManager->SetGlobalLODBias(StreamingConfig.CriticalMemoryLODBias);
    }
}

void UMemoryOptimizationManager::CancelLowPriorityAssetLoads()
{
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Canceling low priority asset loads"));
    
    if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
    {
        // Cancel loads with priority below threshold
        TArray<FSoftObjectPath> LoadsToCancel;
        for (const FSoftObjectPath& AssetPath : ActiveAsyncLoads)
        {
            // This would need actual priority checking logic
            LoadsToCancel.Add(AssetPath);
        }
        
        for (const FSoftObjectPath& AssetPath : LoadsToCancel)
        {
            AssetManager->GetStreamableManager().CancelAsyncLoad(AssetPath);
            ActiveAsyncLoads.Remove(AssetPath);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Memory Manager: Canceled %d low priority asset loads"), LoadsToCancel.Num());
    }
}

void UMemoryOptimizationManager::UnloadDistantLevels()
{
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Unloading distant levels"));
    
    if (UWorld* World = GetWorld())
    {
        // Unload levels that are far from player
        for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
        {
            if (StreamingLevel && StreamingLevel->IsLevelLoaded())
            {
                // Check distance from player (simplified logic)
                bool bShouldUnload = false;
                
                // This would need actual distance checking logic
                if (bShouldUnload)
                {
                    StreamingLevel->SetShouldBeLoaded(false);
                    StreamingLevel->SetShouldBeVisible(false);
                }
            }
        }
        
        World->FlushLevelStreaming();
    }
}

void UMemoryOptimizationManager::OptimizeMaterialInstances()
{
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Optimizing material instances"));
    
    // Remove duplicate material instances
    TMap<UMaterialInterface*, int32> MaterialUsageCount;
    
    for (UMaterialInstanceDynamic* MaterialInstance : TrackedMaterialInstances)
    {
        if (MaterialInstance && MaterialInstance->IsValidLowLevel())
        {
            UMaterialInterface* ParentMaterial = MaterialInstance->Parent;
            if (ParentMaterial)
            {
                int32& Count = MaterialUsageCount.FindOrAdd(ParentMaterial, 0);
                Count++;
            }
        }
    }
    
    // Clean up unused material instances
    for (int32 i = TrackedMaterialInstances.Num() - 1; i >= 0; --i)
    {
        UMaterialInstanceDynamic* MaterialInstance = TrackedMaterialInstances[i];
        if (MaterialInstance && !MaterialInstance->IsUsedByAnyPrimitive())
        {
            MaterialInstance->ConditionalBeginDestroy();
            TrackedMaterialInstances.RemoveAt(i);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Memory Manager: Optimized %d material instances"), TrackedMaterialInstances.Num());
}

FMemoryStats UMemoryOptimizationManager::GetStreamingMemoryStats() const
{
    FMemoryStats Stats = CaptureMemoryStats();
    
    // Add streaming-specific metrics
    Stats.TextureMemoryMB = LastTextureMemoryUsage;
    Stats.MeshMemoryMB = LastMeshMemoryUsage;
    
    return Stats;
}

int32 UMemoryOptimizationManager::GetActiveAsyncLoads() const
{
    return ActiveAsyncLoads.Num();
}

int32 UMemoryOptimizationManager::GetStreamingLevelsCount() const
{
    return StreamingLevelNames.Num();
}
