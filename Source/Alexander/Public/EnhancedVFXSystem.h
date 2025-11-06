#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
// #include "Particles/ParticleSystemComponent.h" // Removed in UE5.6 - migrated to Niagara
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"
#include "Sound/SoundBase.h"
#include "EnhancedVFXSystem.generated.h"

class UParticleSystem;
class UNiagaraSystem;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class USoundBase;
class UCurveFloat;
class UCurveVector;

// VFX Effect Types
UENUM(BlueprintType)
enum class EVFXEffectType : uint8
{
    None,
    WarpDrive,
    WeaponImpact,
    Explosion,
    EnvironmentalHazard,
    ShieldHit,
    EngineTrail,
    AtmosphericEntry,
    EnergyBurst,
    DebrisField,
    QuantumEffect,
    GravityWell,
    PlasmaDischarge,
    EMPBlast,
    HyperspaceRipple
};

// VFX Intensity Levels
UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Low,
    Medium,
    High,
    Extreme,
    Catastrophic
};

// VFX Quality Settings
USTRUCT(BlueprintType)
struct FVFXQualitySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ParticleCount = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EffectRadius = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EffectDuration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float UpdateRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableHighQualityFeatures = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableScreenSpaceEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnablePhysicsSimulation = true;
};

// VFX Effect Definition
USTRUCT(BlueprintType)
struct FVFXEffectDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXEffectType EffectType = EVFXEffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UParticleSystem* ParticleSystem = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UNiagaraSystem* NiagaraSystem = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* EffectMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundBase* EffectSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseDuration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor EffectColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAttachToSource = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFollowSource = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> EffectParameters;
};

// VFX Instance Data
USTRUCT(BlueprintType)
struct FVFXInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InstanceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UParticleSystemComponent* ParticleComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UNiagaraComponent* NiagaraComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInstanceDynamic* DynamicMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* SourceActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXIntensity IntensityLevel = EVFXIntensity::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> ParameterValues;
};

// VFX Environment Data
USTRUCT(BlueprintType)
struct FVFXEnvironmentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AtmosphericDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GravityStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WindDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WindSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInSpace = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInAtmosphere = false;
};

// VFX Warp Data
USTRUCT(BlueprintType)
struct FVFXWarpData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector StartLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector EndLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WarpSpeed = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WarpDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor WarpColor = FLinearColor::Blue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCreateRipple = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCreateTrail = true;
};

// VFX Impact Data
USTRUCT(BlueprintType)
struct FVFXImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ImpactNormal = FVector::UpVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ImpactForce = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamageAmount = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXIntensity ImpactIntensity = EVFXIntensity::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* SurfaceMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCreateSparks = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCreateDebris = true;
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVFXEffectStarted, const FName&, EffectName, EVFXEffectType, EffectType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVFXEffectEnded, const FName&, EffectName, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVFXIntensityChanged, const FName&, EffectName, float, OldIntensity, float, NewIntensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVFXQualityChanged, EVFXIntensity, NewQuality);

/**
 * Enhanced VFX System
 * Provides advanced visual effects including warp drives, weapon impacts, 
 * environmental hazards, and dynamic particle systems with quality scaling.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(VFX))
class ALEXANDER_API UEnhancedVFXSystem : public UObject
{
    GENERATED_BODY()

public:
    UEnhancedVFXSystem();

    // System Management
    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void InitializeVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void ShutdownVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void UpdateVFXSystem(float DeltaTime);

    // Effect Creation
    UFUNCTION(BlueprintCallable, Category = "VFX System")
    FName CreateVFXEffect(const FVFXEffectDefinition& EffectDefinition, AActor* SourceActor = nullptr, FVector TargetLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void CreateWarpEffect(AActor* ShipActor, const FVFXWarpData& WarpData);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void CreateWeaponImpact(const FVFXImpactData& ImpactData);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void CreateExplosion(FVector Location, float Radius, EVFXIntensity Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void CreateShieldHit(AActor* ShieldActor, FVector HitLocation, float Damage);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void CreateEngineTrail(AActor* ShipActor, float ThrustLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void CreateEnvironmentalHazard(const FString& HazardType, FVector Location, float Radius);

    // Effect Control
    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void SetEffectIntensity(const FName& EffectName, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void SetEffectColor(const FName& EffectName, FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void SetEffectParameter(const FName& EffectName, const FName& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void StopEffect(const FName& EffectName, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void PauseEffect(const FName& EffectName);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void ResumeEffect(const FName& EffectName);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void RestartEffect(const FName& EffectName);

    // Effect Queries
    UFUNCTION(BlueprintPure, Category = "VFX System")
    bool IsEffectActive(const FName& EffectName) const;

    UFUNCTION(BlueprintPure, Category = "VFX System")
    float GetEffectIntensity(const FName& EffectName) const;

    UFUNCTION(BlueprintPure, Category = "VFX System")
    FLinearColor GetEffectColor(const FName& EffectName) const;

    UFUNCTION(BlueprintPure, Category = "VFX System")
    TArray<FName> GetActiveEffects() const;

    UFUNCTION(BlueprintPure, Category = "VFX System")
    FVFXInstance GetEffectInstance(const FName& EffectName) const;

    // Quality Management
    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void SetVFXQuality(const FVFXQualitySettings& QualitySettings);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void SetQualityLevel(EVFXIntensity QualityLevel);

    UFUNCTION(BlueprintPure, Category = "VFX System")
    FVFXQualitySettings GetVFXQuality() const;

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void OptimizeForPerformance();

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void OptimizeForQuality();

    // Environment Integration
    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void SetEnvironmentData(const FVFXEnvironmentData& EnvironmentData);

    UFUNCTION(BlueprintPure, Category = "VFX System")
    FVFXEnvironmentData GetEnvironmentData() const;

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void UpdateEnvironmentEffects();

    // Advanced Effects
    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void CreateQuantumEffect(FVector Location, float Intensity, FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void CreateGravityWell(FVector Location, float Strength, float Radius);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void CreatePlasmaDischarge(FVector StartLocation, FVector EndLocation, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void CreateEMPBlast(FVector Location, float Radius, float Duration);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void CreateHyperspaceRipple(FVector Location, float WaveSpeed, float Amplitude);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void AttachEffectToActor(const FName& EffectName, AActor* TargetActor, const FName& SocketName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void DetachEffectFromActor(const FName& EffectName);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void MoveEffectToLocation(const FName& EffectName, FVector NewLocation, float Duration = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void ScaleEffect(const FName& EffectName, FVector Scale, float Duration = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void RotateEffect(const FName& EffectName, FRotator Rotation, float Duration = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void FadeEffect(const FName& EffectName, float TargetOpacity, float Duration);

    // Batch Operations
    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void StopAllEffects(bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void PauseAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void ResumeAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void SetGlobalIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void SetGlobalColor(FLinearColor Color);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "VFX System")
    FOnVFXEffectStarted OnVFXEffectStarted;

    UPROPERTY(BlueprintAssignable, Category = "VFX System")
    FOnVFXEffectEnded OnVFXEffectEnded;

    UPROPERTY(BlueprintAssignable, Category = "VFX System")
    FOnVFXIntensityChanged OnVFXIntensityChanged;

    UPROPERTY(BlueprintAssignable, Category = "VFX System")
    FOnVFXQualityChanged OnVFXQualityChanged;

protected:
    // Effect Storage
    UPROPERTY(BlueprintReadOnly, Category = "VFX System")
    TMap<FName, FVFXInstance> ActiveEffects;

    UPROPERTY(BlueprintReadOnly, Category = "VFX System")
    TMap<EVFXEffectType, FVFXEffectDefinition> EffectDefinitions;

    // Quality and Performance
    UPROPERTY(BlueprintReadOnly, Category = "VFX System")
    FVFXQualitySettings CurrentQualitySettings;

    UPROPERTY(BlueprintReadOnly, Category = "VFX System")
    FVFXEnvironmentData CurrentEnvironmentData;

    UPROPERTY(BlueprintReadOnly, Category = "VFX System")
    float GlobalIntensityMultiplier = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX System")
    FLinearColor GlobalColorTint = FLinearColor::White;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX System")
    int32 MaxActiveEffects = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX System")
    float EffectUpdateInterval = 0.016f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX System")
    bool bEnableDebugDrawing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX System")
    bool bEnablePerformanceMonitoring = true;

private:
    float LastUpdateTime;
    int32 EffectCounter;
    TMap<FName, UCurveFloat*> IntensityCurves;
    TMap<FName, UCurveVector*> ColorCurves;

    void UpdateEffectInstances(float DeltaTime);
    void CleanupExpiredEffects();
    void OptimizeEffects();
    void UpdateEffectParameters(const FName& EffectName);
    void ApplyQualitySettings(const FVFXEffectDefinition& Definition, FVFXInstance& Instance);

    FName GenerateUniqueEffectName(const FString& BaseName);
    UParticleSystemComponent* CreateParticleComponent(const FVFXEffectDefinition& Definition);
    UNiagaraComponent* CreateNiagaraComponent(const FVFXEffectDefinition& Definition);
    UMaterialInstanceDynamic* CreateDynamicMaterial(UMaterialInterface* BaseMaterial);

    void ProcessWarpEffect(FVFXInstance& Instance, float DeltaTime);
    void ProcessImpactEffect(FVFXInstance& Instance, float DeltaTime);
    void ProcessEnvironmentalEffect(FVFXInstance& Instance, float DeltaTime);

    FVector CalculateEnvironmentInfluence(FVector Location) const;
    float CalculateEnvironmentIntensity(const FVFXInstance& Instance) const;
    FLinearColor CalculateEnvironmentColor(const FVFXInstance& Instance) const;

    void DrawDebugInfo();
    void LogPerformanceMetrics();
};