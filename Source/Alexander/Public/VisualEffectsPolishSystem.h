// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "VisualEffectsPolishSystem.generated.h"

/**
 * Quality settings for visual effects
 */
UENUM(BlueprintType)
enum class EVisualEffectsQuality : uint8
{
	Low,
	Medium,
	High,
	Ultra,
	Cinematic
};

/**
 * Ambient detail configuration
 */
USTRUCT(BlueprintType)
struct FAmbientDetailConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient")
	bool bEnableDustParticles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient")
	bool bEnableInsects = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient")
	bool bEnableLeaves = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient")
	bool bEnablePollen = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient")
	float ParticleDensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient")
	float SpawnRadius = 50.0f;
};

/**
 * Material transition settings
 */
USTRUCT(BlueprintType)
struct FMaterialTransitionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float TransitionDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	bool bUseSmoothing = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float BlendSharpness = 2.0f;
};

/**
 * System for polishing visual effects
 */
UCLASS()
class ALEXANDER_API UVisualEffectsPolishSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Quality settings
	UFUNCTION(BlueprintCallable, Category = "Visual Effects")
	void SetEffectsQuality(EVisualEffectsQuality Quality);

	UFUNCTION(BlueprintPure, Category = "Visual Effects")
	EVisualEffectsQuality GetEffectsQuality() const { return CurrentQuality; }

	// Particle effects enhancement
	UFUNCTION(BlueprintCallable, Category = "Visual Effects")
	void EnhanceParticleEffect(UNiagaraComponent* ParticleComponent);

	UFUNCTION(BlueprintCallable, Category = "Visual Effects")
	UNiagaraComponent* SpawnEnhancedParticleEffect(
		UNiagaraSystem* System,
		FVector Location,
		FRotator Rotation,
		FVector Scale = FVector(1.0f)
	);

	// Material transitions
	UFUNCTION(BlueprintCallable, Category = "Visual Effects")
	void TransitionMaterial(
		UMeshComponent* MeshComponent,
		UMaterialInterface* TargetMaterial,
		float Duration = 1.0f
	);

	UFUNCTION(BlueprintCallable, Category = "Visual Effects")
	void TransitionMaterialParameter(
		UMaterialInstanceDynamic* Material,
		FName ParameterName,
		float TargetValue,
		float Duration = 1.0f
	);

	// Ambient details
	UFUNCTION(BlueprintCallable, Category = "Visual Effects")
	void SpawnAmbientDetails(FVector Location, float Radius);

	UFUNCTION(BlueprintCallable, Category = "Visual Effects")
	void SetAmbientDetailConfig(const FAmbientDetailConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Visual Effects")
	FAmbientDetailConfig GetAmbientDetailConfig() const { return AmbientConfig; }

	// Cinematic camera effects
	UFUNCTION(BlueprintCallable, Category = "Visual Effects")
	void EnableCinematicMode(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Visual Effects")
	void SetDepthOfFieldSettings(float FocalDistance, float Aperture);

	UFUNCTION(BlueprintCallable, Category = "Visual Effects")
	void SetMotionBlurAmount(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Visual Effects")
	void SetVignetteIntensity(float Intensity);

	// Post-process effects
	UFUNCTION(BlueprintCallable, Category = "Visual Effects")
	void SetBloomIntensity(float Intensity);

	UFUNCTION(BlueprintCallable, Category = "Visual Effects")
	void SetColorGrading(FLinearColor Tint, float Saturation, float Contrast);

protected:
	void UpdateAmbientDetails(float DeltaTime);
	void ApplyQualitySettings();
	void CleanupAmbientDetails();

private:
	UPROPERTY()
	EVisualEffectsQuality CurrentQuality = EVisualEffectsQuality::High;

	UPROPERTY()
	FAmbientDetailConfig AmbientConfig;

	UPROPERTY()
	TArray<UNiagaraComponent*> AmbientParticles;

	UPROPERTY()
	bool bCinematicModeEnabled = false;

	// Material transition tracking
	struct FMaterialTransition
	{
		UMeshComponent* MeshComponent;
		UMaterialInterface* TargetMaterial;
		float ElapsedTime;
		float Duration;
	};

	TArray<FMaterialTransition> ActiveTransitions;

	FTimerHandle UpdateTimerHandle;
};
