// Copyright Epic Games, Inc. All Rights Reserved.

#include "VFXManager.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"         // For APlayerCameraManager
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UVFXManager::UVFXManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz for VFX management

    // Default configuration
    CurrentQuality = EVFXQuality::High;
    MaxActiveVFX = 100;
    bEnablePooling = true;
    bEnableDistanceCulling = true;
    CullingDistance = 10000.0f;
    bEnablePrioritySystem = true;

    TotalSpawnedCount = 0;
    TotalLifetimeSum = 0.0f;

    // Set default category limits
    CategoryMaxCounts.Add(EVFXCategory::Weapon, 20);
    CategoryMaxCounts.Add(EVFXCategory::Explosion, 15);
    CategoryMaxCounts.Add(EVFXCategory::Impact, 30);
    CategoryMaxCounts.Add(EVFXCategory::Weather, 10);
    CategoryMaxCounts.Add(EVFXCategory::Environment, 20);
}

void UVFXManager::BeginPlay()
{
    Super::BeginPlay();

    // Register templates from configuration
    for (const FVFXTemplate& Template : VFXTemplates)
    {
        RegisterVFXTemplate(Template);
    }
}

void UVFXManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Cleanup expired VFX
    CleanupExpiredVFX(DeltaTime);

    // Enforce category limits
    if (bEnablePrioritySystem)
    {
        EnforceCategoryLimits();
    }

    // Apply quality settings
    EnforceQualitySettings();
}

// ===== VFX Spawning =====

FString UVFXManager::SpawnVFX(UNiagaraSystem* System, FVector Location, FRotator Rotation, EVFXCategory Category, float Lifetime)
{
    if (!System)
    {
        return FString();
    }

    // Check if we've hit the max VFX limit
    if (ActiveVFX.Num() >= MaxActiveVFX)
    {
        return FString();
    }

    // Try to get pooled component
    FString InstanceID = GenerateInstanceID();
    UNiagaraComponent* Component = nullptr;

    if (bEnablePooling)
    {
        Component = GetPooledVFX(System->GetName());
    }

    // Create new component if no pooled one available
    if (!Component)
    {
        Component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            System,
            Location,
            Rotation,
            FVector(1.0f),
            true,
            true,
            ENCPoolMethod::None
        );
    }
    else
    {
        Component->SetWorldLocation(Location);
        Component->SetWorldRotation(Rotation);
        Component->Activate(true);
    }

    if (Component)
    {
        // Apply quality settings
        ApplyQualityToVFX(Component);

        // Create instance data
        FVFXManagerInstance Instance;
        Instance.InstanceID = InstanceID;
        Instance.NiagaraComponent = Component;
        Instance.Category = Category;
        Instance.Location = Location;
        Instance.SpawnTime = GetWorld()->GetTimeSeconds();
        Instance.Lifetime = Lifetime;
        Instance.bIsPooled = bEnablePooling;
        Instance.bIsActive = true;

        ActiveVFX.Add(InstanceID, Instance);

        // Update statistics
        TotalSpawnedCount++;
        TotalLifetimeSum += Lifetime;

        // Broadcast event
        OnVFXSpawned.Broadcast(InstanceID, Category);

        return InstanceID;
    }

    return FString();
}

FString UVFXManager::SpawnVFXFromTemplate(const FString& TemplateName, FVector Location, FRotator Rotation)
{
    FVFXTemplate Template;
    if (GetVFXTemplate(TemplateName, Template))
    {
        FString InstanceID = SpawnVFX(Template.NiagaraSystem, Location, Rotation, Template.Category, Template.DefaultLifetime);

        if (!InstanceID.IsEmpty() && ActiveVFX.Contains(InstanceID))
        {
            // Apply template default parameters
            for (const auto& ParamPair : Template.DefaultParameters)
            {
                SetVFXParameter(InstanceID, ParamPair.Key, ParamPair.Value);
            }
        }

        return InstanceID;
    }

    return FString();
}

FString UVFXManager::SpawnVFXAttached(UNiagaraSystem* System, USceneComponent* AttachTo, FName SocketName, EVFXCategory Category, float Lifetime)
{
    if (!System || !AttachTo)
    {
        return FString();
    }

    FString InstanceID = GenerateInstanceID();

    UNiagaraComponent* Component = UNiagaraFunctionLibrary::SpawnSystemAttached(
        System,
        AttachTo,
        SocketName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        true
    );

    if (Component)
    {
        ApplyQualityToVFX(Component);

        FVFXManagerInstance Instance;
        Instance.InstanceID = InstanceID;
        Instance.NiagaraComponent = Component;
        Instance.Category = Category;
        Instance.Location = Component->GetComponentLocation();
        Instance.SpawnTime = GetWorld()->GetTimeSeconds();
        Instance.Lifetime = Lifetime;
        Instance.bIsActive = true;

        ActiveVFX.Add(InstanceID, Instance);
        TotalSpawnedCount++;

        OnVFXSpawned.Broadcast(InstanceID, Category);

        return InstanceID;
    }

    return FString();
}

FString UVFXManager::SpawnExplosionVFX(FVector Location, float Scale)
{
    if (ExplosionVFXSystem)
    {
        FString InstanceID = SpawnVFX(ExplosionVFXSystem, Location, FRotator::ZeroRotator, EVFXCategory::Explosion, 3.0f);

        if (!InstanceID.IsEmpty())
        {
            SetVFXParameter(InstanceID, TEXT("Scale"), Scale);
        }

        return InstanceID;
    }

    return FString();
}

FString UVFXManager::SpawnImpactVFX(FVector Location, FVector Normal, float Intensity)
{
    if (ImpactVFXSystem)
    {
        FRotator Rotation = Normal.Rotation();
        FString InstanceID = SpawnVFX(ImpactVFXSystem, Location, Rotation, EVFXCategory::Impact, 1.0f);

        if (!InstanceID.IsEmpty())
        {
            SetVFXParameter(InstanceID, TEXT("Intensity"), Intensity);
        }

        return InstanceID;
    }

    return FString();
}

FString UVFXManager::SpawnWeaponTrailVFX(FVector Start, FVector End, float Thickness)
{
    if (WeaponTrailVFXSystem)
    {
        FVector Direction = (End - Start).GetSafeNormal();
        FRotator Rotation = Direction.Rotation();
        float Distance = FVector::Dist(Start, End);

        FString InstanceID = SpawnVFX(WeaponTrailVFXSystem, Start, Rotation, EVFXCategory::Weapon, 0.5f);

        if (!InstanceID.IsEmpty())
        {
            SetVFXVectorParameter(InstanceID, TEXT("StartLocation"), Start);
            SetVFXVectorParameter(InstanceID, TEXT("EndLocation"), End);
            SetVFXParameter(InstanceID, TEXT("Thickness"), Thickness);
            SetVFXParameter(InstanceID, TEXT("Distance"), Distance);
        }

        return InstanceID;
    }

    return FString();
}

// ===== VFX Control =====

void UVFXManager::StopVFX(const FString& InstanceID, bool bImmediate)
{
    if (ActiveVFX.Contains(InstanceID))
    {
        FVFXManagerInstance& Instance = ActiveVFX[InstanceID];

        if (Instance.NiagaraComponent)
        {
            if (bImmediate)
            {
                Instance.NiagaraComponent->DestroyComponent();
            }
            else
            {
                Instance.NiagaraComponent->Deactivate();
            }
        }

        if (bImmediate || !Instance.bIsPooled)
        {
            ActiveVFX.Remove(InstanceID);
            OnVFXCompleted.Broadcast(InstanceID, Instance.Category);
        }
        else
        {
            Instance.bIsActive = false;
        }
    }
}

void UVFXManager::PauseVFX(const FString& InstanceID)
{
    if (ActiveVFX.Contains(InstanceID))
    {
        FVFXManagerInstance& Instance = ActiveVFX[InstanceID];

        if (Instance.NiagaraComponent)
        {
            Instance.NiagaraComponent->SetPaused(true);
        }
    }
}

void UVFXManager::ResumeVFX(const FString& InstanceID)
{
    if (ActiveVFX.Contains(InstanceID))
    {
        FVFXManagerInstance& Instance = ActiveVFX[InstanceID];

        if (Instance.NiagaraComponent)
        {
            Instance.NiagaraComponent->SetPaused(false);
        }
    }
}

void UVFXManager::SetVFXParameter(const FString& InstanceID, const FString& ParameterName, float Value)
{
    if (ActiveVFX.Contains(InstanceID))
    {
        FVFXManagerInstance& Instance = ActiveVFX[InstanceID];

        if (Instance.NiagaraComponent)
        {
            Instance.NiagaraComponent->SetFloatParameter(FName(*ParameterName), Value);
        }
    }
}

void UVFXManager::SetVFXVectorParameter(const FString& InstanceID, const FString& ParameterName, FVector Value)
{
    if (ActiveVFX.Contains(InstanceID))
    {
        FVFXManagerInstance& Instance = ActiveVFX[InstanceID];

        if (Instance.NiagaraComponent)
        {
            Instance.NiagaraComponent->SetVectorParameter(FName(*ParameterName), Value);
        }
    }
}

void UVFXManager::SetVFXColorParameter(const FString& InstanceID, const FString& ParameterName, FLinearColor Value)
{
    if (ActiveVFX.Contains(InstanceID))
    {
        FVFXManagerInstance& Instance = ActiveVFX[InstanceID];

        if (Instance.NiagaraComponent)
        {
            Instance.NiagaraComponent->SetColorParameter(FName(*ParameterName), Value);
        }
    }
}

// ===== VFX Queries =====

bool UVFXManager::IsVFXActive(const FString& InstanceID) const
{
    if (ActiveVFX.Contains(InstanceID))
    {
        return ActiveVFX[InstanceID].bIsActive;
    }
    return false;
}

int32 UVFXManager::GetActiveVFXCount(EVFXCategory Category) const
{
    if (Category == EVFXCategory::Special)
    {
        return ActiveVFX.Num();
    }

    int32 Count = 0;
    for (const auto& Pair : ActiveVFX)
    {
        if (Pair.Value.Category == Category && Pair.Value.bIsActive)
        {
            Count++;
        }
    }
    return Count;
}

TArray<FString> UVFXManager::GetActiveVFXInstances(EVFXCategory Category) const
{
    TArray<FString> Instances;

    for (const auto& Pair : ActiveVFX)
    {
        if (Pair.Value.bIsActive && (Category == EVFXCategory::Special || Pair.Value.Category == Category))
        {
            Instances.Add(Pair.Key);
        }
    }

    return Instances;
}

FVFXStats UVFXManager::GetVFXStatistics() const
{
    FVFXStats Stats;
    Stats.ActiveInstances = ActiveVFX.Num();
    Stats.TotalSpawned = TotalSpawnedCount;

    int32 PooledCount = 0;
    for (const auto& PoolPair : VFXPools)
    {
        PooledCount += PoolPair.Value.Num();
    }
    Stats.PooledInstances = PooledCount;

    if (TotalSpawnedCount > 0)
    {
        Stats.AverageLifetime = TotalLifetimeSum / TotalSpawnedCount;
    }

    // Rough memory estimate (1MB per active effect)
    Stats.MemoryUsageMB = ActiveVFX.Num() * 1.0f;

    return Stats;
}

// ===== Quality Management =====

void UVFXManager::SetVFXQuality(EVFXQuality Quality)
{
    CurrentQuality = Quality;
    EnforceQualitySettings();
}

void UVFXManager::SetMaxActiveVFX(int32 MaxCount)
{
    MaxActiveVFX = FMath::Max(1, MaxCount);
}

void UVFXManager::SetCategoryMaxVFX(EVFXCategory Category, int32 MaxCount)
{
    CategoryMaxCounts.Add(Category, FMath::Max(1, MaxCount));
}

// ===== Pooling System =====

void UVFXManager::PreloadVFXPool(const FString& TemplateName, int32 PoolSize)
{
    FVFXTemplate Template;
    if (!GetVFXTemplate(TemplateName, Template) || !Template.NiagaraSystem)
    {
        return;
    }

    TArray<UNiagaraComponent*>& Pool = VFXPools.FindOrAdd(TemplateName);

    for (int32 i = 0; i < PoolSize; ++i)
    {
        UNiagaraComponent* Component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            Template.NiagaraSystem,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            FVector(1.0f),
            false,
            true,
            ENCPoolMethod::None
        );

        if (Component)
        {
            Component->Deactivate();
            Pool.Add(Component);
        }
    }
}

void UVFXManager::ClearVFXPool(const FString& TemplateName)
{
    if (VFXPools.Contains(TemplateName))
    {
        TArray<UNiagaraComponent*>& Pool = VFXPools[TemplateName];

        for (UNiagaraComponent* Component : Pool)
        {
            if (Component)
            {
                Component->DestroyComponent();
            }
        }

        VFXPools.Remove(TemplateName);
    }
}

void UVFXManager::ClearAllPools()
{
    for (auto& PoolPair : VFXPools)
    {
        for (UNiagaraComponent* Component : PoolPair.Value)
        {
            if (Component)
            {
                Component->DestroyComponent();
            }
        }
    }

    VFXPools.Empty();
}

// ===== Batch Operations =====

void UVFXManager::StopAllVFX(EVFXCategory Category, bool bImmediate)
{
    TArray<FString> InstancesToStop;

    for (const auto& Pair : ActiveVFX)
    {
        if (Category == EVFXCategory::Special || Pair.Value.Category == Category)
        {
            InstancesToStop.Add(Pair.Key);
        }
    }

    for (const FString& InstanceID : InstancesToStop)
    {
        StopVFX(InstanceID, bImmediate);
    }
}

void UVFXManager::StopVFXInRadius(FVector Location, float Radius, bool bImmediate)
{
    TArray<FString> InstancesToStop;
    float RadiusSquared = Radius * Radius;

    for (const auto& Pair : ActiveVFX)
    {
        float DistanceSquared = FVector::DistSquared(Pair.Value.Location, Location);
        if (DistanceSquared <= RadiusSquared)
        {
            InstancesToStop.Add(Pair.Key);
        }
    }

    for (const FString& InstanceID : InstancesToStop)
    {
        StopVFX(InstanceID, bImmediate);
    }
}

void UVFXManager::SetVFXQualityForCategory(EVFXCategory Category, float QualityMultiplier)
{
    for (auto& Pair : ActiveVFX)
    {
        if (Pair.Value.Category == Category && Pair.Value.NiagaraComponent)
        {
            // Apply quality multiplier to various parameters
            Pair.Value.NiagaraComponent->SetFloatParameter(TEXT("QualityMultiplier"), QualityMultiplier);
        }
    }
}

// ===== Template Management =====

void UVFXManager::RegisterVFXTemplate(const FVFXTemplate& Template)
{
    TemplateRegistry.Add(Template.TemplateName, Template);

    // Preload pool if specified
    if (bEnablePooling && Template.bEnablePooling && Template.PoolSize > 0)
    {
        PreloadVFXPool(Template.TemplateName, Template.PoolSize);
    }
}

bool UVFXManager::GetVFXTemplate(const FString& TemplateName, FVFXTemplate& OutTemplate) const
{
    if (TemplateRegistry.Contains(TemplateName))
    {
        OutTemplate = TemplateRegistry[TemplateName];
        return true;
    }
    return false;
}

// ===== Helper Functions =====

FString UVFXManager::GenerateInstanceID()
{
    static int32 Counter = 0;
    return FString::Printf(TEXT("VFX_%d_%lld"), ++Counter, FDateTime::Now().GetTicks());
}

UNiagaraComponent* UVFXManager::GetPooledVFX(const FString& TemplateName)
{
    if (!VFXPools.Contains(TemplateName))
    {
        return nullptr;
    }

    TArray<UNiagaraComponent*>& Pool = VFXPools[TemplateName];

    for (int32 i = Pool.Num() - 1; i >= 0; --i)
    {
        if (Pool[i] && !Pool[i]->IsActive())
        {
            UNiagaraComponent* Component = Pool[i];
            Pool.RemoveAt(i);
            return Component;
        }
    }

    return nullptr;
}

void UVFXManager::ReturnToPool(const FString& InstanceID)
{
    if (!ActiveVFX.Contains(InstanceID))
    {
        return;
    }

    FVFXManagerInstance& Instance = ActiveVFX[InstanceID];

    if (Instance.bIsPooled && Instance.NiagaraComponent)
    {
        Instance.NiagaraComponent->Deactivate();

        // Find template name for this system
        for (const auto& TemplatePair : TemplateRegistry)
        {
            if (TemplatePair.Value.NiagaraSystem == Instance.NiagaraComponent->GetAsset())
            {
                TArray<UNiagaraComponent*>& Pool = VFXPools.FindOrAdd(TemplatePair.Key);
                Pool.Add(Instance.NiagaraComponent);
                break;
            }
        }
    }

    ActiveVFX.Remove(InstanceID);
}

void UVFXManager::CleanupExpiredVFX(float DeltaTime)
{
    TArray<FString> ExpiredInstances;
    float CurrentTime = GetWorld()->GetTimeSeconds();

    for (auto& Pair : ActiveVFX)
    {
        FVFXManagerInstance& Instance = Pair.Value;

        if (Instance.Lifetime > 0.0f)
        {
            float ElapsedTime = CurrentTime - Instance.SpawnTime;

            if (ElapsedTime >= Instance.Lifetime)
            {
                ExpiredInstances.Add(Pair.Key);
            }
        }

        // Check for distance culling
        if (bEnableDistanceCulling && ShouldCullVFX(Instance))
        {
            ExpiredInstances.Add(Pair.Key);
        }
    }

    for (const FString& InstanceID : ExpiredInstances)
    {
        StopVFX(InstanceID, false);
    }
}

void UVFXManager::EnforceCategoryLimits()
{
    for (const auto& LimitPair : CategoryMaxCounts)
    {
        EVFXCategory Category = LimitPair.Key;
        int32 MaxCount = LimitPair.Value;
        int32 CurrentCount = GetActiveVFXCount(Category);

        if (CurrentCount > MaxCount)
        {
            // Find lowest priority instances to remove
            TArray<TPair<FString, int32>> CategoryInstances;

            for (const auto& Pair : ActiveVFX)
            {
                if (Pair.Value.Category == Category && Pair.Value.bIsActive)
                {
                    CategoryInstances.Add(TPair<FString, int32>(Pair.Key, Pair.Value.Priority));
                }
            }

            // Sort by priority (lowest first)
            CategoryInstances.Sort([](const TPair<FString, int32>& A, const TPair<FString, int32>& B)
            {
                return A.Value < B.Value;
            });

            // Remove excess instances
            int32 ToRemove = CurrentCount - MaxCount;
            for (int32 i = 0; i < ToRemove && i < CategoryInstances.Num(); ++i)
            {
                StopVFX(CategoryInstances[i].Key, true);
            }
        }
    }
}

void UVFXManager::EnforceQualitySettings()
{
    float QualityMultiplier = GetQualityMultiplier();

    for (auto& Pair : ActiveVFX)
    {
        if (Pair.Value.NiagaraComponent)
        {
            ApplyQualityToVFX(Pair.Value.NiagaraComponent);
        }
    }
}

bool UVFXManager::ShouldCullVFX(const FVFXManagerInstance& Instance) const
{
    if (!bEnableDistanceCulling || !GetWorld())
    {
        return false;
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->PlayerCameraManager)
    {
        return false;
    }

    FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
    float Distance = FVector::Dist(CameraLocation, Instance.Location);

    return Distance > CullingDistance;
}

float UVFXManager::GetQualityMultiplier() const
{
    switch (CurrentQuality)
    {
    case EVFXQuality::Low: return 0.5f;
    case EVFXQuality::Medium: return 0.75f;
    case EVFXQuality::High: return 1.0f;
    case EVFXQuality::Ultra: return 1.25f;
    case EVFXQuality::Cinematic: return 1.5f;
    default: return 1.0f;
    }
}

void UVFXManager::ApplyQualityToVFX(UNiagaraComponent* Component)
{
    if (!Component)
    {
        return;
    }

    float QualityMultiplier = GetQualityMultiplier();

    // Apply quality parameters
    Component->SetFloatParameter(TEXT("QualityMultiplier"), QualityMultiplier);
    Component->SetFloatParameter(TEXT("ParticleDensity"), QualityMultiplier);
    Component->SetFloatParameter(TEXT("DetailLevel"), QualityMultiplier);
}
