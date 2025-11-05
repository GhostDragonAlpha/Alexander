// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/ParticleSystemComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystem.h"
#include "VFXManager.generated.h"

/**
 * VFX quality levels for performance scaling
 */
UENUM(BlueprintType)
enum class EVFXQuality : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Ultra       UMETA(DisplayName = "Ultra"),
    Cinematic   UMETA(DisplayName = "Cinematic")
};

/**
 * VFX category types
 */
UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Weapon          UMETA(DisplayName = "Weapon"),
    Explosion       UMETA(DisplayName = "Explosion"),
    Environment     UMETA(DisplayName = "Environment"),
    Weather         UMETA(DisplayName = "Weather"),
    Ship            UMETA(DisplayName = "Ship"),
    UI              UMETA(DisplayName = "UI"),
    Impact          UMETA(DisplayName = "Impact"),
    Ambient         UMETA(DisplayName = "Ambient"),
    Special         UMETA(DisplayName = "Special"),
    Persistent      UMETA(DisplayName = "Persistent")
};

/**
 * VFX instance data
 */
USTRUCT(BlueprintType)
struct FVFXManagerInstance
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "VFX")
    FString InstanceID;

    UPROPERTY(BlueprintReadWrite, Category = "VFX")
    UNiagaraComponent* NiagaraComponent = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "VFX")
    EVFXCategory Category = EVFXCategory::Special;

    UPROPERTY(BlueprintReadWrite, Category = "VFX")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "VFX")
    float SpawnTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "VFX")
    float Lifetime = 5.0f;

    UPROPERTY(BlueprintReadWrite, Category = "VFX")
    bool bIsPooled = false;

    UPROPERTY(BlueprintReadWrite, Category = "VFX")
    bool bIsActive = true;

    UPROPERTY(BlueprintReadWrite, Category = "VFX")
    int32 Priority = 50;
};

/**
 * VFX template configuration
 */
USTRUCT(BlueprintType)
struct FVFXTemplate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FString TemplateName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UNiagaraSystem* NiagaraSystem = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFXCategory Category = EVFXCategory::Special;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DefaultLifetime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    int32 DefaultPriority = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bEnablePooling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    int32 PoolSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TMap<FString, float> DefaultParameters;
};

/**
 * VFX performance statistics
 */
USTRUCT(BlueprintType)
struct FVFXStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 ActiveInstances = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 PooledInstances = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 TotalSpawned = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float AverageLifetime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float MemoryUsageMB = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVFXSpawned, const FString&, InstanceID, EVFXCategory, Category);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVFXCompleted, const FString&, InstanceID, EVFXCategory, Category);

/**
 * VFX Manager Component
 * Centralized management of visual effects with pooling, LOD, and performance optimization
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UVFXManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ===== VFX Spawning =====

    UFUNCTION(BlueprintCallable, Category = "VFX")
    FString SpawnVFX(UNiagaraSystem* System, FVector Location, FRotator Rotation, 
                     EVFXCategory Category = EVFXCategory::Special, float Lifetime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    FString SpawnVFXFromTemplate(const FString& TemplateName, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    FString SpawnVFXAttached(UNiagaraSystem* System, USceneComponent* AttachTo, FName SocketName,
                            EVFXCategory Category = EVFXCategory::Special, float Lifetime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    FString SpawnExplosionVFX(FVector Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    FString SpawnImpactVFX(FVector Location, FVector Normal, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    FString SpawnWeaponTrailVFX(FVector Start, FVector End, float Thickness = 1.0f);

    // ===== VFX Control =====

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(const FString& InstanceID, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PauseVFX(const FString& InstanceID);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ResumeVFX(const FString& InstanceID);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXParameter(const FString& InstanceID, const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXVectorParameter(const FString& InstanceID, const FString& ParameterName, FVector Value);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXColorParameter(const FString& InstanceID, const FString& ParameterName, FLinearColor Value);

    // ===== VFX Queries =====

    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool IsVFXActive(const FString& InstanceID) const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveVFXCount(EVFXCategory Category = EVFXCategory::Special) const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    TArray<FString> GetActiveVFXInstances(EVFXCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    FVFXStats GetVFXStatistics() const;

    // ===== Quality Management =====

    UFUNCTION(BlueprintCallable, Category = "VFX Quality")
    void SetVFXQuality(EVFXQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "VFX Quality")
    EVFXQuality GetVFXQuality() const { return CurrentQuality; }

    UFUNCTION(BlueprintCallable, Category = "VFX Quality")
    void SetMaxActiveVFX(int32 MaxCount);

    UFUNCTION(BlueprintCallable, Category = "VFX Quality")
    void SetCategoryMaxVFX(EVFXCategory Category, int32 MaxCount);

    // ===== Pooling System =====

    UFUNCTION(BlueprintCallable, Category = "VFX Pooling")
    void PreloadVFXPool(const FString& TemplateName, int32 PoolSize);

    UFUNCTION(BlueprintCallable, Category = "VFX Pooling")
    void ClearVFXPool(const FString& TemplateName);

    UFUNCTION(BlueprintCallable, Category = "VFX Pooling")
    void ClearAllPools();

    // ===== Batch Operations =====

    UFUNCTION(BlueprintCallable, Category = "VFX Batch")
    void StopAllVFX(EVFXCategory Category = EVFXCategory::Special, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "VFX Batch")
    void StopVFXInRadius(FVector Location, float Radius, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "VFX Batch")
    void SetVFXQualityForCategory(EVFXCategory Category, float QualityMultiplier);

    // ===== Template Management =====

    UFUNCTION(BlueprintCallable, Category = "VFX Templates")
    void RegisterVFXTemplate(const FVFXTemplate& Template);

    UFUNCTION(BlueprintCallable, Category = "VFX Templates")
    bool GetVFXTemplate(const FString& TemplateName, FVFXTemplate& OutTemplate) const;

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "VFX Events")
    FOnVFXSpawned OnVFXSpawned;

    UPROPERTY(BlueprintAssignable, Category = "VFX Events")
    FOnVFXCompleted OnVFXCompleted;

    // ===== Configuration =====

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFXQuality CurrentQuality = EVFXQuality::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    int32 MaxActiveVFX = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bEnablePooling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float CullingDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bEnablePrioritySystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TMap<EVFXCategory, int32> CategoryMaxCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TArray<FVFXTemplate> VFXTemplates;

    // Explosion VFX
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UNiagaraSystem* ExplosionVFXSystem;

    // Impact VFX
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UNiagaraSystem* ImpactVFXSystem;

    // Weapon trail VFX
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UNiagaraSystem* WeaponTrailVFXSystem;

private:
    // Active VFX tracking
    UPROPERTY()
    TMap<FString, FVFXManagerInstance> ActiveVFX;

    // VFX pools
// VFX pools (C++ only due to TArray in TMap limitation)
    TMap<FString, TArray<UNiagaraComponent*>> VFXPools;

    // Template storage
    UPROPERTY()
    TMap<FString, FVFXTemplate> TemplateRegistry;

    // Statistics
    int32 TotalSpawnedCount;
    float TotalLifetimeSum;

    // Helper functions
    FString GenerateInstanceID();
    UNiagaraComponent* GetPooledVFX(const FString& TemplateName);
    void ReturnToPool(const FString& InstanceID);
    void CleanupExpiredVFX(float DeltaTime);
    void EnforceCategoryLimits();
    void EnforceQualitySettings();
    bool ShouldCullVFX(const FVFXManagerInstance& Instance) const;
    float GetQualityMultiplier() const;
    void ApplyQualityToVFX(UNiagaraComponent* Component);
};
