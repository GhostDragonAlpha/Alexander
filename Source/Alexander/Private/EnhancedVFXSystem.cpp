#include "EnhancedVFXSystem.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"

UEnhancedVFXSystem::UEnhancedVFXSystem()
{
    LastUpdateTime = 0.0f;
    EffectCounter = 0;
    MaxActiveEffects = 100;
    EffectUpdateInterval = 0.016f;
    bEnableDebugDrawing = false;
    bEnablePerformanceMonitoring = true;
    GlobalIntensityMultiplier = 1.0f;
    GlobalColorTint = FLinearColor::White;
}

void UEnhancedVFXSystem::InitializeVFXSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Enhanced VFX System Initialized"));
    
    // Initialize default effect definitions
    InitializeDefaultEffects();
    
    // Set default quality settings
    SetQualityLevel(EVFXIntensity::Medium);
    
    // Set default environment data
    FVFXEnvironmentData DefaultEnvironment;
    DefaultEnvironment.AtmosphericDensity = 1.0f;
    DefaultEnvironment.GravityStrength = 1.0f;
    DefaultEnvironment.Temperature = 20.0f;
    DefaultEnvironment.Humidity = 0.5f;
    DefaultEnvironment.bInSpace = false;
    DefaultEnvironment.bInAtmosphere = true;
    SetEnvironmentData(DefaultEnvironment);
}

void UEnhancedVFXSystem::ShutdownVFXSystem()
{
    // Stop all active effects
    StopAllEffects(true);
    
    // Clear all data
    ActiveEffects.Empty();
    EffectDefinitions.Empty();
    IntensityCurves.Empty();
    ColorCurves.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Enhanced VFX System Shutdown"));
}

void UEnhancedVFXSystem::UpdateVFXSystem(float DeltaTime)
{
    if (!GetWorld()) return;

    // Update effect instances
    UpdateEffectInstances(DeltaTime);
    
    // Clean up expired effects
    CleanupExpiredEffects();
    
    // Optimize effects if needed
    if (bEnablePerformanceMonitoring)
    {
        OptimizeEffects();
    }
    
    // Draw debug information
    if (bEnableDebugDrawing)
    {
        DrawDebugInfo();
    }
    
    // Log performance metrics
    if (bEnablePerformanceMonitoring && GetWorld()->GetTimeSeconds() - LastUpdateTime > 1.0f)
    {
        LogPerformanceMetrics();
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void UEnhancedVFXSystem::UpdateEffectInstances(float DeltaTime)
{
    for (auto& EffectPair : ActiveEffects)
    {
        const FName& EffectName = EffectPair.Key;
        FVFXInstance& Instance = EffectPair.Value;
        
        if (!Instance.bIsActive) continue;
        
        // Update effect time
        float ElapsedTime = GetWorld()->GetTimeSeconds() - Instance.StartTime;
        
        // Check if effect should end
        if (ElapsedTime >= Instance.Duration)
        {
            StopEffect(EffectName);
            continue;
        }
        
        // Update effect parameters
        UpdateEffectParameters(EffectName);
        
        // Process specific effect types
        if (EffectDefinitions.Contains(Instance.InstanceName))
        {
            const FVFXEffectDefinition& Definition = EffectDefinitions[Instance.InstanceName];
            
            switch (Definition.EffectType)
            {
                case EVFXEffectType::WarpDrive:
                    ProcessWarpEffect(Instance, DeltaTime);
                    break;
                case EVFXEffectType::WeaponImpact:
                    ProcessImpactEffect(Instance, DeltaTime);
                    break;
                case EVFXEffectType::EnvironmentalHazard:
                    ProcessEnvironmentalEffect(Instance, DeltaTime);
                    break;
                default:
                    break;
            }
        }
        
        // Apply environment influence
        float EnvironmentIntensity = CalculateEnvironmentIntensity(Instance);
        FLinearColor EnvironmentColor = CalculateEnvironmentColor(Instance);
        
        Instance.CurrentIntensity = Instance.CurrentIntensity * EnvironmentIntensity * GlobalIntensityMultiplier;
        
        // Update particle system if exists
        if (Instance.ParticleComponent)
        {
            Instance.ParticleComponent->SetFloatParameter("Intensity", Instance.CurrentIntensity);
            Instance.ParticleComponent->SetVectorParameter("Color", EnvironmentColor.ToFVector() * GlobalColorTint.ToFVector());
        }
        
        // Update niagara system if exists
        if (Instance.NiagaraComponent)
        {
            Instance.NiagaraComponent->SetFloatParameter("Intensity", Instance.CurrentIntensity);
            Instance.NiagaraComponent->SetVectorParameter("Color", EnvironmentColor.ToFVector() * GlobalColorTint.ToFVector());
        }
    }
}

void UEnhancedVFXSystem::CleanupExpiredEffects()
{
    TArray<FName> EffectsToRemove;
    
    for (const auto& EffectPair : ActiveEffects)
    {
        const FName& EffectName = EffectPair.Key;
        const FVFXInstance& Instance = EffectPair.Value;
        
        if (!Instance.bIsActive)
        {
            EffectsToRemove.Add(EffectName);
        }
    }
    
    for (const FName& EffectName : EffectsToRemove)
    {
        FVFXInstance& Instance = ActiveEffects[EffectName];
        
        // Clean up components
        if (Instance.ParticleComponent)
        {
            Instance.ParticleComponent->DestroyComponent();
        }
        
        if (Instance.NiagaraComponent)
        {
            Instance.NiagaraComponent->DestroyComponent();
        }
        
        if (Instance.DynamicMaterial)
        {
            Instance.DynamicMaterial = nullptr;
        }
        
        ActiveEffects.Remove(EffectName);
        
        // Broadcast effect ended
        OnVFXEffectEnded.Broadcast(EffectName, GetWorld()->GetTimeSeconds() - Instance.StartTime);
    }
}

void UEnhancedVFXSystem::OptimizeEffects()
{
    // Check if we have too many active effects
    if (ActiveEffects.Num() > MaxActiveEffects)
    {
        // Sort effects by importance and remove least important ones
        TArray<TPair<float, FName>> EffectPriorities;
        
        for (const auto& EffectPair : ActiveEffects)
        {
            const FName& EffectName = EffectPair.Key;
            const FVFXInstance& Instance = EffectPair.Value;
            
            float Priority = Instance.CurrentIntensity;
            
            // Higher priority for effects attached to players
            if (Instance.SourceActor && Instance.SourceActor->IsA(APlayerController::StaticClass()))
            {
                Priority *= 2.0f;
            }
            
            EffectPriorities.Add(TPair<float, FName>(Priority, EffectName));
        }
        
        // Sort by priority (ascending)
        EffectPriorities.Sort([](const TPair<float, FName>& A, const TPair<float, FName>& B)
        {
            return A.Key < B.Key;
        });
        
        // Remove lowest priority effects
        int32 EffectsToRemove = ActiveEffects.Num() - MaxActiveEffects;
        for (int32 i = 0; i < EffectsToRemove; i++)
        {
            StopEffect(EffectPriorities[i].Value, true);
        }
    }
}

FName UEnhancedVFXSystem::CreateVFXEffect(const FVFXEffectDefinition& EffectDefinition, AActor* SourceActor, FVector TargetLocation)
{
    if (!GetWorld()) return NAME_None;

    // Generate unique effect name
    FName EffectName = GenerateUniqueEffectName(EffectDefinition.EffectType == EVFXEffectType::None ? 
        TEXT("Effect") : UEnum::GetValueAsString(EffectDefinition.EffectType));
    
    // Create effect instance
    FVFXInstance Instance;
    Instance.InstanceName = EffectName;
    Instance.SourceActor = SourceActor;
    Instance.TargetLocation = TargetLocation;
    Instance.StartTime = GetWorld()->GetTimeSeconds();
    Instance.Duration = EffectDefinition.BaseDuration;
    Instance.CurrentIntensity = EffectDefinition.BaseIntensity;
    Instance.IntensityLevel = EVFXIntensity::Medium;
    Instance.bIsActive = true;
    
    // Apply quality settings
    ApplyQualitySettings(EffectDefinition, Instance);
    
    // Create particle system component
    if (EffectDefinition.ParticleSystem)
    {
        Instance.ParticleComponent = CreateParticleComponent(EffectDefinition);
        if (Instance.ParticleComponent)
        {
            Instance.ParticleComponent->ActivateSystem();
        }
    }
    
    // Create niagara component
    if (EffectDefinition.NiagaraSystem)
    {
        Instance.NiagaraComponent = CreateNiagaraComponent(EffectDefinition);
        if (Instance.NiagaraComponent)
        {
            Instance.NiagaraComponent->Activate();
        }
    }
    
    // Create dynamic material
    if (EffectDefinition.EffectMaterial)
    {
        Instance.DynamicMaterial = CreateDynamicMaterial(EffectDefinition.EffectMaterial);
    }
    
    // Attach to source actor if specified
    if (EffectDefinition.bAttachToSource && SourceActor)
    {
        AttachEffectToActor(EffectName, SourceActor);
    }
    
    // Play effect sound
    if (EffectDefinition.EffectSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), EffectDefinition.EffectSound, 
            TargetLocation.IsZero() ? SourceActor->GetActorLocation() : TargetLocation);
    }
    
    // Store effect
    ActiveEffects.Add(EffectName, Instance);
    EffectDefinitions.Add(EffectName, EffectDefinition);
    
    // Broadcast effect started
    OnVFXEffectStarted.Broadcast(EffectName, EffectDefinition.EffectType);
    
    return EffectName;
}

void UEnhancedVFXSystem::CreateWarpEffect(AActor* ShipActor, const FVFXWarpData& WarpData)
{
    if (!ShipActor) return;

    // Create warp effect definition
    FVFXEffectDefinition WarpDefinition;
    WarpDefinition.EffectType = EVFXEffectType::WarpDrive;
    WarpDefinition.BaseDuration = WarpData.WarpDuration;
    WarpDefinition.BaseRadius = 500.0f;
    WarpDefinition.BaseIntensity = 1.0f;
    WarpDefinition.EffectColor = WarpData.WarpColor;
    WarpDefinition.bAttachToSource = true;
    WarpDefinition.bFollowSource = true;
    
    // Create main warp effect
    FName WarpEffectName = CreateVFXEffect(WarpDefinition, ShipActor);
    
    if (WarpEffectName != NAME_None && ActiveEffects.Contains(WarpEffectName))
    {
        FVFXInstance& Instance = ActiveEffects[WarpEffectName];
        
        // Store warp-specific data
        Instance.ParameterValues.Add("WarpSpeed", WarpData.WarpSpeed);
        Instance.ParameterValues.Add("StartLocation", WarpData.StartLocation.X);
        Instance.ParameterValues.Add("EndLocation", WarpData.EndLocation.X);
        Instance.ParameterValues.Add("CreateRipple", WarpData.bCreateRipple ? 1.0f : 0.0f);
        Instance.ParameterValues.Add("CreateTrail", WarpData.bCreateTrail ? 1.0f : 0.0f);
        
        // Create ripple effect if requested
        if (WarpData.bCreateRipple)
        {
            FVFXEffectDefinition RippleDefinition;
            RippleDefinition.EffectType = EVFXEffectType::HyperspaceRipple;
            RippleDefinition.BaseDuration = 3.0f;
            RippleDefinition.BaseRadius = 1000.0f;
            RippleDefinition.BaseIntensity = 0.7f;
            RippleDefinition.EffectColor = WarpData.WarpColor;
            
            CreateVFXEffect(RippleDefinition, nullptr, WarpData.StartLocation);
        }
        
        // Create trail effect if requested
        if (WarpData.bCreateTrail)
        {
            FVFXEffectDefinition TrailDefinition;
            TrailDefinition.EffectType = EVFXEffectType::EngineTrail;
            TrailDefinition.BaseDuration = WarpData.WarpDuration + 1.0f;
            TrailDefinition.BaseRadius = 200.0f;
            TrailDefinition.BaseIntensity = 0.8f;
            TrailDefinition.EffectColor = WarpData.WarpColor;
            TrailDefinition.bAttachToSource = true;
            TrailDefinition.bFollowSource = true;
            
            CreateVFXEffect(TrailDefinition, ShipActor);
        }
    }
}

void UEnhancedVFXSystem::CreateWeaponImpact(const FVFXImpactData& ImpactData)
{
    // Create impact effect definition
    FVFXEffectDefinition ImpactDefinition;
    ImpactDefinition.EffectType = EVFXEffectType::WeaponImpact;
    ImpactDefinition.BaseDuration = 2.0f;
    ImpactDefinition.BaseRadius = FMath::Clamp(ImpactData.ImpactForce / 100.0f, 50.0f, 500.0f);
    ImpactDefinition.BaseIntensity = FMath::Clamp(ImpactData.DamageAmount / 100.0f, 0.1f, 2.0f);
    
    // Set intensity level based on impact force
    if (ImpactData.ImpactForce < 500.0f)
        ImpactDefinition.BaseIntensity = 0.3f;
    else if (ImpactData.ImpactForce < 1500.0f)
        ImpactDefinition.BaseIntensity = 0.6f;
    else if (ImpactData.ImpactForce < 3000.0f)
        ImpactDefinition.BaseIntensity = 1.0f;
    else
        ImpactDefinition.BaseIntensity = 1.5f;
    
    // Create main impact effect
    FName ImpactEffectName = CreateVFXEffect(ImpactDefinition, nullptr, ImpactData.ImpactLocation);
    
    if (ImpactEffectName != NAME_None && ActiveEffects.Contains(ImpactEffectName))
    {
        FVFXInstance& Instance = ActiveEffects[ImpactEffectName];
        
        // Store impact-specific data
        Instance.ParameterValues.Add("ImpactForce", ImpactData.ImpactForce);
        Instance.ParameterValues.Add("DamageAmount", ImpactData.DamageAmount);
        Instance.ParameterValues.Add("NormalX", ImpactData.ImpactNormal.X);
        Instance.ParameterValues.Add("NormalY", ImpactData.ImpactNormal.Y);
        Instance.ParameterValues.Add("NormalZ", ImpactData.ImpactNormal.Z);
        Instance.ParameterValues.Add("CreateSparks", ImpactData.bCreateSparks ? 1.0f : 0.0f);
        Instance.ParameterValues.Add("CreateDebris", ImpactData.bCreateDebris ? 1.0f : 0.0f);
        
        // Create sparks if requested
        if (ImpactData.bCreateSparks)
        {
            FVFXEffectDefinition SparksDefinition;
            SparksDefinition.EffectType = EVFXEffectType::EnergyBurst;
            SparksDefinition.BaseDuration = 1.5f;
            SparksDefinition.BaseRadius = ImpactDefinition.BaseRadius * 0.5f;
            SparksDefinition.BaseIntensity = 0.8f;
            SparksDefinition.EffectColor = FLinearColor::Yellow;
            
            CreateVFXEffect(SparksDefinition, nullptr, ImpactData.ImpactLocation);
        }
        
        // Create debris if requested
        if (ImpactData.bCreateDebris)
        {
            FVFXEffectDefinition DebrisDefinition;
            DebrisDefinition.EffectType = EVFXEffectType::DebrisField;
            DebrisDefinition.BaseDuration = 3.0f;
            DebrisDefinition.BaseRadius = ImpactDefinition.BaseRadius * 1.5f;
            DebrisDefinition.BaseIntensity = 0.6f;
            
            CreateVFXEffect(DebrisDefinition, nullptr, ImpactData.ImpactLocation);
        }
    }
}

void UEnhancedVFXSystem::CreateExplosion(FVector Location, float Radius, EVFXIntensity Intensity)
{
    // Create explosion effect definition
    FVFXEffectDefinition ExplosionDefinition;
    ExplosionDefinition.EffectType = EVFXEffectType::Explosion;
    ExplosionDefinition.BaseDuration = 3.0f;
    ExplosionDefinition.BaseRadius = Radius;
    
    // Set intensity based on level
    switch (Intensity)
    {
        case EVFXIntensity::Low:
            ExplosionDefinition.BaseIntensity = 0.3f;
            ExplosionDefinition.EffectColor = FLinearColor(0.8f, 0.4f, 0.1f);
            break;
        case EVFXIntensity::Medium:
            ExplosionDefinition.BaseIntensity = 0.6f;
            ExplosionDefinition.EffectColor = FLinearColor(1.0f, 0.6f, 0.2f);
            break;
        case EVFXIntensity::High:
            ExplosionDefinition.BaseIntensity = 1.0f;
            ExplosionDefinition.EffectColor = FLinearColor(1.0f, 0.8f, 0.3f);
            break;
        case EVFXIntensity::Extreme:
            ExplosionDefinition.BaseIntensity = 1.5f;
            ExplosionDefinition.EffectColor = FLinearColor(1.0f, 1.0f, 0.5f);
            break;
        case EVFXIntensity::Catastrophic:
            ExplosionDefinition.BaseIntensity = 2.0f;
            ExplosionDefinition.EffectColor = FLinearColor(1.0f, 1.0f, 0.8f);
            break;
    }
    
    // Create main explosion
    CreateVFXEffect(ExplosionDefinition, nullptr, Location);
    
    // Create shockwave for larger explosions
    if (Intensity >= EVFXIntensity::High)
    {
        FVFXEffectDefinition ShockwaveDefinition;
        ShockwaveDefinition.EffectType = EVFXEffectType::HyperspaceRipple;
        ShockwaveDefinition.BaseDuration = 2.0f;
        ShockwaveDefinition.BaseRadius = Radius * 2.0f;
        ShockwaveDefinition.BaseIntensity = 0.5f;
        ShockwaveDefinition.EffectColor = FLinearColor::White;
        
        CreateVFXEffect(ShockwaveDefinition, nullptr, Location);
    }
}

void UEnhancedVFXSystem::CreateShieldHit(AActor* ShieldActor, FVector HitLocation, float Damage)
{
    if (!ShieldActor) return;

    // Create shield hit effect definition
    FVFXEffectDefinition ShieldDefinition;
    ShieldDefinition.EffectType = EVFXEffectType::ShieldHit;
    ShieldDefinition.BaseDuration = 1.0f;
    ShieldDefinition.BaseRadius = FMath::Clamp(Damage / 10.0f, 20.0f, 100.0f);
    ShieldDefinition.BaseIntensity = FMath::Clamp(Damage / 50.0f, 0.2f, 1.0f);
    ShieldDefinition.EffectColor = FLinearColor(0.2f, 0.6f, 1.0f);
    ShieldDefinition.bAttachToSource = true;
    
    // Create shield hit effect
    CreateVFXEffect(ShieldDefinition, ShieldActor, HitLocation);
}

void UEnhancedVFXSystem::CreateEngineTrail(AActor* ShipActor, float ThrustLevel)
{
    if (!ShipActor) return;

    // Create engine trail effect definition
    FVFXEffectDefinition TrailDefinition;
    TrailDefinition.EffectType = EVFXEffectType::EngineTrail;
    TrailDefinition.BaseDuration = 0.5f; // Continuous effect
    TrailDefinition.BaseRadius = 50.0f * ThrustLevel;
    TrailDefinition.BaseIntensity = ThrustLevel;
    TrailDefinition.EffectColor = FLinearColor(0.3f, 0.7f, 1.0f);
    TrailDefinition.bAttachToSource = true;
    TrailDefinition.bFollowSource = true;
    
    // Create engine trail
    CreateVFXEffect(TrailDefinition, ShipActor);
}

void UEnhancedVFXSystem::CreateEnvironmentalHazard(const FString& HazardType, FVector Location, float Radius)
{
    EVFXEffectType EffectType = EVFXEffectType::EnvironmentalHazard;
    FLinearColor HazardColor = FLinearColor::Green;
    float Duration = 10.0f;
    float Intensity = 0.7f;
    
    // Configure based on hazard type
    if (HazardType == "Radiation")
    {
        HazardColor = FLinearColor::Yellow;
        Duration = 15.0f;
        Intensity = 0.5f;
    }
    else if (HazardType == "Acid")
    {
        HazardColor = FLinearColor::Green;
        Duration = 8.0f;
        Intensity = 0.8f;
    }
    else if (HazardType == "Fire")
    {
        HazardColor = FLinearColor::Red;
        Duration = 5.0f;
        Intensity = 1.0f;
    }
    else if (HazardType == "Electric")
    {
        HazardColor = FLinearColor::Blue;
        Duration = 3.0f;
        Intensity = 1.2f;
    }
    
    // Create environmental hazard effect
    FVFXEffectDefinition HazardDefinition;
    HazardDefinition.EffectType = EffectType;
    HazardDefinition.BaseDuration = Duration;
    HazardDefinition.BaseRadius = Radius;
    HazardDefinition.BaseIntensity = Intensity;
    HazardDefinition.EffectColor = HazardColor;
    
    CreateVFXEffect(HazardDefinition, nullptr, Location);
}

void UEnhancedVFXSystem::SetEffectIntensity(const FName& EffectName, float Intensity)
{
    if (!ActiveEffects.Contains(EffectName)) return;

    FVFXInstance& Instance = ActiveEffects[EffectName];
    float OldIntensity = Instance.CurrentIntensity;
    Instance.CurrentIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
    
    // Update particle system
    if (Instance.ParticleComponent)
    {
        Instance.ParticleComponent->SetFloatParameter("Intensity", Instance.CurrentIntensity);
    }
    
    // Update niagara system
    if (Instance.NiagaraComponent)
    {
        Instance.NiagaraComponent->SetFloatParameter("Intensity", Instance.CurrentIntensity);
    }
    
    // Broadcast intensity change
    OnVFXIntensityChanged.Broadcast(EffectName, OldIntensity, Instance.CurrentIntensity);
}

void UEnhancedVFXSystem::SetEffectColor(const FName& EffectName, FLinearColor Color)
{
    if (!ActiveEffects.Contains(EffectName)) return;

    FVFXInstance& Instance = ActiveEffects[EffectName];
    
    // Update particle system
    if (Instance.ParticleComponent)
    {
        Instance.ParticleComponent->SetVectorParameter("Color", Color.ToFVector());
    }
    
    // Update niagara system
    if (Instance.NiagaraComponent)
    {
        Instance.NiagaraComponent->SetVectorParameter("Color", Color.ToFVector());
    }
    
    // Update dynamic material
    if (Instance.DynamicMaterial)
    {
        Instance.DynamicMaterial->SetVectorParameterValue("Color", Color);
    }
}

void UEnhancedVFXSystem::SetEffectParameter(const FName& EffectName, const FName& ParameterName, float Value)
{
    if (!ActiveEffects.Contains(EffectName)) return;

    FVFXInstance& Instance = ActiveEffects[EffectName];
    Instance.ParameterValues.Add(ParameterName, Value);
    
    // Update particle system
    if (Instance.ParticleComponent)
    {
        Instance.ParticleComponent->SetFloatParameter(ParameterName, Value);
    }
    
    // Update niagara system
    if (Instance.NiagaraComponent)
    {
        Instance.NiagaraComponent->SetFloatParameter(ParameterName, Value);
    }
}

void UEnhancedVFXSystem::StopEffect(const FName& EffectName, bool bImmediate)
{
    if (!ActiveEffects.Contains(EffectName)) return;

    FVFXInstance& Instance = ActiveEffects[EffectName];
    
    if (bImmediate)
    {
        Instance.bIsActive = false;
    }
    else
    {
        // Fade out effect
        FadeEffect(EffectName, 0.0f, 0.5f);
        
        // Schedule stop after fade
        FTimerHandle TimerHandle;
        FTimerDelegate TimerDelegate;
        TimerDelegate.BindUObject(this, &UEnhancedVFXSystem::StopEffect, EffectName, true);
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.5f, false);
    }
}

void UEnhancedVFXSystem::PauseEffect(const FName& EffectName)
{
    if (!ActiveEffects.Contains(EffectName)) return;

    FVFXInstance& Instance = ActiveEffects[EffectName];
    
    if (Instance.ParticleComponent)
    {
        Instance.ParticleComponent->DeactivateSystem();
    }
    
    if (Instance.NiagaraComponent)
    {
        Instance.NiagaraComponent->Deactivate();
    }
}

void UEnhancedVFXSystem::ResumeEffect(const FName& EffectName)
{
    if (!ActiveEffects.Contains(EffectName)) return;

    FVFXInstance& Instance = ActiveEffects[EffectName];
    
    if (Instance.ParticleComponent)
    {
        Instance.ParticleComponent->ActivateSystem();
    }
    
    if (Instance.NiagaraComponent)
    {
        Instance.NiagaraComponent->Activate();
    }
}

void UEnhancedVFXSystem::RestartEffect(const FName& EffectName)
{
    if (!ActiveEffects.Contains(EffectName)) return;

    FVFXInstance& Instance = ActiveEffects[EffectName];
    Instance.StartTime = GetWorld()->GetTimeSeconds();
    
    if (Instance.ParticleComponent)
    {
        Instance.ParticleComponent->ResetParticles();
        Instance.ParticleComponent->ActivateSystem();
    }
    
    if (Instance.NiagaraComponent)
    {
        Instance.NiagaraComponent->ResetParameters();
        Instance.NiagaraComponent->Activate();
    }
}

bool UEnhancedVFXSystem::IsEffectActive(const FName& EffectName) const
{
    if (!ActiveEffects.Contains(EffectName)) return false;
    return ActiveEffects[EffectName].bIsActive;
}

float UEnhancedVFXSystem::GetEffectIntensity(const FName& EffectName) const
{
    if (!ActiveEffects.Contains(EffectName)) return 0.0f;
    return ActiveEffects[EffectName].CurrentIntensity;
}

FLinearColor UEnhancedVFXSystem::GetEffectColor(const FName& EffectName) const
{
    if (!ActiveEffects.Contains(EffectName)) return FLinearColor::White;
    
    const FVFXInstance& Instance = ActiveEffects[EffectName];
    if (EffectDefinitions.Contains(Instance.InstanceName))
    {
        return EffectDefinitions[Instance.InstanceName].EffectColor;
    }
    
    return FLinearColor::White;
}

TArray<FName> UEnhancedVFXSystem::GetActiveEffects() const
{
    TArray<FName> ActiveEffectNames;
    
    for (const auto& EffectPair : ActiveEffects)
    {
        if (EffectPair.Value.bIsActive)
        {
            ActiveEffectNames.Add(EffectPair.Key);
        }
    }
    
    return ActiveEffectNames;
}

FVFXInstance UEnhancedVFXSystem::GetEffectInstance(const FName& EffectName) const
{
    if (!ActiveEffects.Contains(EffectName)) 
    {
        FVFXInstance EmptyInstance;
        return EmptyInstance;
    }
    return ActiveEffects[EffectName];
}

void UEnhancedVFXSystem::SetVFXQuality(const FVFXQualitySettings& QualitySettings)
{
    CurrentQualitySettings = QualitySettings;
    
    // Apply quality settings to all active effects
    for (auto& EffectPair : ActiveEffects)
    {
        FVFXInstance& Instance = EffectPair.Value;
        
        if (EffectDefinitions.Contains(Instance.InstanceName))
        {
            const FVFXEffectDefinition& Definition = EffectDefinitions[Instance.InstanceName];
            ApplyQualitySettings(Definition, Instance);
        }
    }
}

void UEnhancedVFXSystem::SetQualityLevel(EVFXIntensity QualityLevel)
{
    FVFXQualitySettings QualitySettings;
    
    switch (QualityLevel)
    {
        case EVFXIntensity::Low:
            QualitySettings.ParticleCount = 0.3f;
            QualitySettings.EffectRadius = 0.7f;
            QualitySettings.EffectDuration = 0.8f;
            QualitySettings.UpdateRate = 0.5f;
            QualitySettings.bEnableHighQualityFeatures = false;
            QualitySettings.bEnableScreenSpaceEffects = false;
            QualitySettings.bEnablePhysicsSimulation = false;
            break;
            
        case EVFXIntensity::Medium:
            QualitySettings.ParticleCount = 0.6f;
            QualitySettings.EffectRadius = 0.9f;
            QualitySettings.EffectDuration = 0.9f;
            QualitySettings.UpdateRate = 0.7f;
            QualitySettings.bEnableHighQualityFeatures = true;
            QualitySettings.bEnableScreenSpaceEffects = false;
            QualitySettings.bEnablePhysicsSimulation = true;
            break;
            
        case EVFXIntensity::High:
            QualitySettings.ParticleCount = 0.8f;
            QualitySettings.EffectRadius = 1.0f;
            QualitySettings.EffectDuration = 1.0f;
            QualitySettings.UpdateRate = 0.9f;
            QualitySettings.bEnableHighQualityFeatures = true;
            QualitySettings.bEnableScreenSpaceEffects = true;
            QualitySettings.bEnablePhysicsSimulation = true;
            break;
            
        case EVFXIntensity::Extreme:
        case EVFXIntensity::Catastrophic:
            QualitySettings.ParticleCount = 1.0f;
            QualitySettings.EffectRadius = 1.2f;
            QualitySettings.EffectDuration = 1.1f;
            QualitySettings.UpdateRate = 1.0f;
            QualitySettings.bEnableHighQualityFeatures = true;
            QualitySettings.bEnableScreenSpaceEffects = true;
            QualitySettings.bEnablePhysicsSimulation = true;
            break;
    }
    
    SetVFXQuality(QualitySettings);
    OnVFXQualityChanged.Broadcast(QualityLevel);
}

FVFXQualitySettings UEnhancedVFXSystem::GetVFXQuality() const
{
    return CurrentQualitySettings;
}

void UEnhancedVFXSystem::OptimizeForPerformance()
{
    SetQualityLevel(EVFXIntensity::Low);
    MaxActiveEffects = 50;
}

void UEnhancedVFXSystem::OptimizeForQuality()
{
    SetQualityLevel(EVFXIntensity::Extreme);
    MaxActiveEffects = 150;
}

void UEnhancedVFXSystem::SetEnvironmentData(const FVFXEnvironmentData& EnvironmentData)
{
    CurrentEnvironmentData = EnvironmentData;
}

FVFXEnvironmentData UEnhancedVFXSystem::GetEnvironmentData() const
{
    return CurrentEnvironmentData;
}

void UEnhancedVFXSystem::UpdateEnvironmentEffects()
{
    // Update all active effects with new environment data
    for (auto& EffectPair : ActiveEffects)
    {
        const FName& EffectName = EffectPair.Key;
        UpdateEffectParameters(EffectName);
    }
}

void UEnhancedVFXSystem::CreateQuantumEffect(FVector Location, float Intensity, FLinearColor Color)
{
    FVFXEffectDefinition QuantumDefinition;
    QuantumDefinition.EffectType = EVFXEffectType::QuantumEffect;
    QuantumDefinition.BaseDuration = 2.0f;
    QuantumDefinition.BaseRadius = 100.0f * Intensity;
    QuantumDefinition.BaseIntensity = Intensity;
    QuantumDefinition.EffectColor = Color;
    
    CreateVFXEffect(QuantumDefinition, nullptr, Location);
}

void UEnhancedVFXSystem::CreateGravityWell(FVector Location, float Strength, float Radius)
{
    FVFXEffectDefinition GravityDefinition;
    GravityDefinition.EffectType = EVFXEffectType::GravityWell;
    GravityDefinition.BaseDuration = 10.0f;
    GravityDefinition.BaseRadius = Radius;
    GravityDefinition.BaseIntensity = Strength;
    GravityDefinition.EffectColor = FLinearColor::Purple;
    
    CreateVFXEffect(GravityDefinition, nullptr, Location);
}

void UEnhancedVFXSystem::CreatePlasmaDischarge(FVector StartLocation, FVector EndLocation, float Intensity)
{
    FVFXEffectDefinition PlasmaDefinition;
    PlasmaDefinition.EffectType = EVFXEffectType::PlasmaDischarge;
    PlasmaDefinition.BaseDuration = 1.0f;
    PlasmaDefinition.BaseRadius = 50.0f * Intensity;
    PlasmaDefinition.BaseIntensity = Intensity;
    PlasmaDefinition.EffectColor = FLinearColor::Cyan;
    
    CreateVFXEffect(PlasmaDefinition, nullptr, StartLocation);
}

void UEnhancedVFXSystem::CreateEMPBlast(FVector Location, float Radius, float Duration)
{
    FVFXEffectDefinition EMPDefinition;
    EMPDefinition.EffectType = EVFXEffectType::EMPBlast;
    EMPDefinition.BaseDuration = Duration;
    EMPDefinition.BaseRadius = Radius;
    EMPDefinition.BaseIntensity = 1.0f;
    EMPDefinition.EffectColor = FLinearColor::White;
    
    CreateVFXEffect(EMPDefinition, nullptr, Location);
}

void UEnhancedVFXSystem::CreateHyperspaceRipple(FVector Location, float WaveSpeed, float Amplitude)
{
    FVFXEffectDefinition RippleDefinition;
    RippleDefinition.EffectType = EVFXEffectType::HyperspaceRipple;
    RippleDefinition.BaseDuration = 3.0f;
    RippleDefinition.BaseRadius = Amplitude * 1000.0f;
    RippleDefinition.BaseIntensity = WaveSpeed / 100.0f;
    RippleDefinition.EffectColor = FLinearColor::Blue;
    
    CreateVFXEffect(RippleDefinition, nullptr, Location);
}

void UEnhancedVFXSystem::AttachEffectToActor(const FName& EffectName, AActor* TargetActor, const FName& SocketName)
{
    if (!ActiveEffects.Contains(EffectName) || !TargetActor) return;

    FVFXInstance& Instance = ActiveEffects[EffectName];
    Instance.SourceActor = TargetActor;
    
    // Attach particle component
    if (Instance.ParticleComponent)
    {
        USceneComponent* AttachComponent = TargetActor->GetRootComponent();
        Instance.ParticleComponent->AttachToComponent(AttachComponent, FAttachmentTransformRules::KeepRelativeTransform, SocketName);
    }
    
    // Attach niagara component
    if (Instance.NiagaraComponent)
    {
        USceneComponent* AttachComponent = TargetActor->GetRootComponent();
        Instance.NiagaraComponent->AttachToComponent(AttachComponent, FAttachmentTransformRules::KeepRelativeTransform, SocketName);
    }
}

void UEnhancedVFXSystem::DetachEffectFromActor(const FName& EffectName)
{
    if (!ActiveEffects.Contains(EffectName)) return;

    FVFXInstance& Instance = ActiveEffects[EffectName];
    
    // Detach particle component
    if (Instance.ParticleComponent)
    {
        Instance.ParticleComponent->DetachFromComponent();
    }
    
    // Detach niagara component
    if (Instance.NiagaraComponent)
    {
        Instance.NiagaraComponent->DetachFromComponent();
    }
    
    Instance.SourceActor = nullptr;
}

void UEnhancedVFXSystem::MoveEffectToLocation(const FName& EffectName, FVector NewLocation, float Duration)
{
    if (!ActiveEffects.Contains(EffectName)) return;

    FVFXInstance& Instance = ActiveEffects[EffectName];
    
    if (Duration <= 0.0f)
    {
        // Immediate move
        Instance.TargetLocation = NewLocation;
        
        if (Instance.ParticleComponent)
        {
            Instance.ParticleComponent->SetWorldLocation(NewLocation);
        }
        
        if (Instance.NiagaraComponent)
        {
            Instance.NiagaraComponent->SetWorldLocation(NewLocation);
        }
    }
    else
    {
        // Smooth move over duration
        FVector StartLocation = Instance.TargetLocation;
        
        // Create timeline for smooth movement
        // This would need a timeline component in a full implementation
        // For now, just set the target location
        Instance.TargetLocation = NewLocation;
    }
}

void UEnhancedVFXSystem::ScaleEffect(const FName& EffectName, FVector Scale, float Duration)
{
    if (!ActiveEffects.Contains(EffectName)) return;

    FVFXInstance& Instance = ActiveEffects[EffectName];
    
    if (Instance.ParticleComponent)
    {
        Instance.ParticleComponent->SetRelativeScale3D(Scale);
    }
    
    if (Instance.NiagaraComponent)
    {
        Instance.NiagaraComponent->SetRelativeScale3D(Scale);
    }
}

void UEnhancedVFXSystem::RotateEffect(const FName& EffectName, FRotator Rotation, float Duration)
{
    if (!ActiveEffects.Contains(EffectName)) return;

    FVFXInstance& Instance = ActiveEffects[EffectName];
    
    if (Instance.ParticleComponent)
    {
        Instance.ParticleComponent->SetWorldRotation(Rotation);
    }
    
    if (Instance.NiagaraComponent)
    {
        Instance.NiagaraComponent->SetWorldRotation(Rotation);
    }
}

void UEnhancedVFXSystem::FadeEffect(const FName& EffectName, float TargetOpacity, float Duration)
{
    if (!ActiveEffects.Contains(EffectName)) return;

    FVFXInstance& Instance = ActiveEffects[EffectName];
    
    // Calculate fade rate
    float CurrentOpacity = Instance.CurrentIntensity;
    float FadeRate = (TargetOpacity - CurrentOpacity) / Duration;
    
    // Store fade parameters
    Instance.ParameterValues.Add("FadeTarget", TargetOpacity);
    Instance.ParameterValues.Add("FadeRate", FadeRate);
}

void UEnhancedVFXSystem::StopAllEffects(bool bImmediate)
{
    TArray<FName> EffectNames;
    ActiveEffects.GetKeys(EffectNames);
    
    for (const FName& EffectName : EffectNames)
    {
        StopEffect(EffectName, bImmediate);
    }
}

void UEnhancedVFXSystem::PauseAllEffects()
{
    for (auto& EffectPair : ActiveEffects)
    {
        PauseEffect(EffectPair.Key);
    }
}

void UEnhancedVFXSystem::ResumeAllEffects()
{
    for (auto& EffectPair : ActiveEffects)
    {
        ResumeEffect(EffectPair.Key);
    }
}

void UEnhancedVFXSystem::SetGlobalIntensity(float Intensity)
{
    GlobalIntensityMultiplier = FMath::Clamp(Intensity, 0.0f, 2.0f);
}

void UEnhancedVFXSystem::SetGlobalColor(FLinearColor Color)
{
    GlobalColorTint = Color;
}

FName UEnhancedVFXSystem::GenerateUniqueEffectName(const FString& BaseName)
{
    EffectCounter++;
    return FName(*FString::Printf(TEXT("%s_%d"), *BaseName, EffectCounter));
}

UParticleSystemComponent* UEnhancedVFXSystem::CreateParticleComponent(const FVFXEffectDefinition& Definition)
{
    if (!Definition.ParticleSystem || !GetWorld()) return nullptr;

    UParticleSystemComponent* ParticleComponent = NewObject<UParticleSystemComponent>(GetWorld());
    if (ParticleComponent)
    {
        ParticleComponent->SetTemplate(Definition.ParticleSystem);
        ParticleComponent->RegisterComponent();
        
        if (!Definition.bAttachToSource)
        {
            ParticleComponent->SetWorldLocation(Definition.bAttachToSource && Definition.bFollowSource ? 
                FVector::ZeroVector : Definition.bAttachToSource ? FVector::ZeroVector : FVector::ZeroVector);
        }
    }
    
    return ParticleComponent;
}

UNiagaraComponent* UEnhancedVFXSystem::CreateNiagaraComponent(const FVFXEffectDefinition& Definition)
{
    if (!Definition.NiagaraSystem || !GetWorld()) return nullptr;

    UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), 
        Definition.NiagaraSystem, 
        FVector::ZeroVector, 
        FRotator::ZeroRotator, 
        FVector::OneVector, 
        true, 
        true
    );
    
    return NiagaraComponent;
}

UMaterialInstanceDynamic* UEnhancedVFXSystem::CreateDynamicMaterial(UMaterialInterface* BaseMaterial)
{
    if (!BaseMaterial) return nullptr;

    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
    return DynamicMaterial;
}

void UEnhancedVFXSystem::ApplyQualitySettings(const FVFXEffectDefinition& Definition, FVFXInstance& Instance)
{
    // Apply quality scaling
    Instance.Duration = Definition.BaseDuration * CurrentQualitySettings.EffectDuration;
    Instance.CurrentIntensity = Definition.BaseIntensity * CurrentQualitySettings.ParticleCount;
    
    // Scale particle system parameters
    if (Instance.ParticleComponent)
    {
        Instance.ParticleComponent->SetFloatParameter("ParticleCount", CurrentQualitySettings.ParticleCount);
        Instance.ParticleComponent->SetFloatParameter("EffectRadius", Definition.BaseRadius * CurrentQualitySettings.EffectRadius);
    }
    
    // Scale niagara system parameters
    if (Instance.NiagaraComponent)
    {
        Instance.NiagaraComponent->SetFloatParameter("ParticleCount", CurrentQualitySettings.ParticleCount);
        Instance.NiagaraComponent->SetFloatParameter("EffectRadius", Definition.BaseRadius * CurrentQualitySettings.EffectRadius);
    }
}

void UEnhancedVFXSystem::UpdateEffectParameters(const FName& EffectName)
{
    if (!ActiveEffects.Contains(EffectName)) return;

    FVFXInstance& Instance = ActiveEffects[EffectName];
    
    // Update fade effect
    if (Instance.ParameterValues.Contains("FadeRate"))
    {
        float FadeRate = Instance.ParameterValues["FadeRate"];
        float FadeTarget = Instance.ParameterValues.Contains("FadeTarget") ? Instance.ParameterValues["FadeTarget"] : 0.0f;
        
        Instance.CurrentIntensity += FadeRate * GetWorld()->GetDeltaSeconds();
        Instance.CurrentIntensity = FMath::Clamp(Instance.CurrentIntensity, 0.0f, 2.0f);
        
        // Check if fade is complete
        if (FMath::Abs(Instance.CurrentIntensity - FadeTarget) < 0.01f)
        {
            Instance.CurrentIntensity = FadeTarget;
            Instance.ParameterValues.Remove("FadeRate");
            Instance.ParameterValues.Remove("FadeTarget");
            
            if (FadeTarget <= 0.0f)
            {
                StopEffect(EffectName, true);
            }
        }
    }
}

void UEnhancedVFXSystem::ProcessWarpEffect(FVFXInstance& Instance, float DeltaTime)
{
    // Process warp-specific logic
    if (Instance.ParameterValues.Contains("WarpSpeed"))
    {
        float WarpSpeed = Instance.ParameterValues["WarpSpeed"];
        
        // Update warp visual effects based on speed
        float SpeedIntensity = FMath::Clamp(WarpSpeed / 1000.0f, 0.0f, 2.0f);
        SetEffectIntensity(Instance.InstanceName, SpeedIntensity);
    }
}

void UEnhancedVFXSystem::ProcessImpactEffect(FVFXInstance& Instance, float DeltaTime)
{
    // Process impact-specific logic
    if (Instance.ParameterValues.Contains("ImpactForce"))
    {
        float ImpactForce = Instance.ParameterValues["ImpactForce"];
        
        // Update impact effects based on force
        float ForceIntensity = FMath::Clamp(ImpactForce / 1000.0f, 0.0f, 2.0f);
        SetEffectIntensity(Instance.InstanceName, ForceIntensity);
    }
}

void UEnhancedVFXSystem::ProcessEnvironmentalEffect(FVFXInstance& Instance, float DeltaTime)
{
    // Process environmental hazard logic
    float EnvironmentInfluence = CalculateEnvironmentIntensity(Instance);
    SetEffectIntensity(Instance.InstanceName, EnvironmentInfluence);
}

FVector UEnhancedVFXSystem::CalculateEnvironmentInfluence(FVector Location) const
{
    FVector Influence = FVector::ZeroVector;
    
    // Apply wind influence
    if (CurrentEnvironmentData.WindSpeed > 0.0f)
    {
        Influence += CurrentEnvironmentData.WindDirection * CurrentEnvironmentData.WindSpeed;
    }
    
    // Apply gravity influence
    if (CurrentEnvironmentData.GravityStrength != 1.0f)
    {
        Influence.Z -= (CurrentEnvironmentData.GravityStrength - 1.0f) * 100.0f;
    }
    
    return Influence;
}

float UEnhancedVFXSystem::CalculateEnvironmentIntensity(const FVFXInstance& Instance) const
{
    float Intensity = 1.0f;
    
    // Atmospheric density affects intensity
    if (CurrentEnvironmentData.bInAtmosphere)
    {
        Intensity *= CurrentEnvironmentData.AtmosphericDensity;
    }
    
    // Space effects are reduced
    if (CurrentEnvironmentData.bInSpace)
    {
        Intensity *= 0.7f;
    }
    
    // Temperature affects certain effects
    if (CurrentEnvironmentData.Temperature > 100.0f) // Hot environment
    {
        Intensity *= 1.2f;
    }
    else if (CurrentEnvironmentData.Temperature < 0.0f) // Cold environment
    {
        Intensity *= 0.8f;
    }
    
    return Intensity;
}

FLinearColor UEnhancedVFXSystem::CalculateEnvironmentColor(const FVFXInstance& Instance) const
{
    FLinearColor BaseColor = FLinearColor::White;
    
    if (EffectDefinitions.Contains(Instance.InstanceName))
    {
        BaseColor = EffectDefinitions[Instance.InstanceName].EffectColor;
    }
    
    // Modify color based on environment
    if (CurrentEnvironmentData.bInSpace)
    {
        // Slightly blue tint in space
        BaseColor = FLinearColor::LerpUsingHSV(BaseColor, FLinearColor(0.5f, 0.7f, 1.0f), 0.1f);
    }
    
    if (CurrentEnvironmentData.Temperature > 100.0f)
    {
        // Redder tint in hot environments
        BaseColor = FLinearColor::LerpUsingHSV(BaseColor, FLinearColor::Red, 0.2f);
    }
    
    return BaseColor;
}

void UEnhancedVFXSystem::DrawDebugInfo()
{
    if (!GetWorld() || !bEnableDebugDrawing) return;

    // Draw debug information for active effects
    for (const auto& EffectPair : ActiveEffects)
    {
        const FName& EffectName = EffectPair.Key;
        const FVFXInstance& Instance = EffectPair.Value;
        
        if (!Instance.bIsActive) continue;
        
        FVector Location = Instance.SourceActor ? 
            Instance.SourceActor->GetActorLocation() : Instance.TargetLocation;
        
        // Draw effect radius
        DrawDebugSphere(GetWorld(), Location, 50.0f, 16, FColor::Green, false, 0.0f);
        
        // Draw effect name
        DrawDebugString(GetWorld(), Location + FVector(0, 0, 60), EffectName.ToString(), nullptr, FColor::White, 0.0f);
        
        // Draw intensity indicator
        FString IntensityText = FString::Printf(TEXT("Intensity: %.2f"), Instance.CurrentIntensity);
        DrawDebugString(GetWorld(), Location + FVector(0, 0, 80), IntensityText, nullptr, FColor::Yellow, 0.0f);
    }
}

void UEnhancedVFXSystem::LogPerformanceMetrics()
{
    if (!bEnablePerformanceMonitoring) return;

    int32 ActiveEffectCount = 0;
    int32 ParticleSystemCount = 0;
    int32 NiagaraSystemCount = 0;
    
    for (const auto& EffectPair : ActiveEffects)
    {
        const FVFXInstance& Instance = EffectPair.Value;
        
        if (Instance.bIsActive)
        {
            ActiveEffectCount++;
            
            if (Instance.ParticleComponent) ParticleSystemCount++;
            if (Instance.NiagaraComponent) NiagaraSystemCount++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX Performance Metrics - Active Effects: %d, Particle Systems: %d, Niagara Systems: %d, Quality Level: %.2f"),
        ActiveEffectCount, ParticleSystemCount, NiagaraSystemCount, CurrentQualitySettings.ParticleCount);
}

void UEnhancedVFXSystem::InitializeDefaultEffects()
{
    // This would be populated with default effect definitions
    // For now, it's a placeholder for where default effects would be set up
    UE_LOG(LogTemp, Log, TEXT("Default VFX effects initialized"));
}