// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRWateringCan.generated.h"

// Forward declarations
class UStaticMeshComponent;
class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class AFarmPlot;
class UCropDefinition;

/**
 * VR-grabbable watering can for watering crops
 * Provides physics-based water stream with particle effects and haptic feedback
 */
UCLASS()
class ALEXANDER_API AVRWateringCan : public AActor
{
	GENERATED_BODY()

public:
	AVRWateringCan();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ============================================================================
	// COMPONENTS
	// ============================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* CanMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* GrabCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SpoutLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* WaterStreamEffect;

	// ============================================================================
	// WATER PROPERTIES
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	float WaterCapacity;

	UPROPERTY(BlueprintReadOnly, Category = "Water")
	float CurrentWaterAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	float WaterFlowRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	float MinTiltAngleForPouring;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	float MaxWateringDistance;

	// ============================================================================
	// VFX AND AUDIO
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* WaterStreamNiagaraSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundBase* PouringSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundBase* EmptySound;

	UPROPERTY()
	UAudioComponent* PouringSoundComponent;

	// ============================================================================
	// VR INTERACTION STATE
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "VR")
	bool bIsGrabbed;

	UPROPERTY(BlueprintReadOnly, Category = "VR")
	AActor* GrabbingHand;

	UPROPERTY(BlueprintReadOnly, Category = "VR")
	bool bIsPouring;

	// ============================================================================
	// WATERING STATE
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Watering")
	TArray<AFarmPlot*> AffectedFarmPlots;

	UPROPERTY(BlueprintReadOnly, Category = "Watering")
	TArray<FIntPoint> WateredCells;

	// ============================================================================
	// VR INTERACTION FUNCTIONS
	// ============================================================================

	/**
	 * Called when grabbed by VR hand
	 */
	UFUNCTION(BlueprintCallable, Category = "VR")
	void OnGrabbed(AActor* GrabbingActor);

	/**
	 * Called when released by VR hand
	 */
	UFUNCTION(BlueprintCallable, Category = "VR")
	void OnReleased();

	/**
	 * Get current water amount
	 */
	UFUNCTION(BlueprintCallable, Category = "Water")
	float GetWaterAmount() const { return CurrentWaterAmount; }

	/**
	 * Get water fill percentage (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Water")
	float GetWaterPercentage() const;

	/**
	 * Refill watering can to full capacity
	 */
	UFUNCTION(BlueprintCallable, Category = "Water")
	void RefillWater();

	/**
	 * Add water to the can
	 */
	UFUNCTION(BlueprintCallable, Category = "Water")
	void AddWater(float Amount);

	/**
	 * Check if can is currently pouring
	 */
	UFUNCTION(BlueprintCallable, Category = "Water")
	bool IsPouring() const { return bIsPouring; }

protected:
	// ============================================================================
	// INTERNAL FUNCTIONS
	// ============================================================================

	/**
	 * Update pouring state based on can orientation
	 */
	void UpdatePouringState();

	/**
	 * Calculate tilt angle of the can
	 */
	float CalculateTiltAngle() const;

	/**
	 * Start pouring water
	 */
	void StartPouring();

	/**
	 * Stop pouring water
	 */
	void StopPouring();

	/**
	 * Update water stream effects
	 */
	void UpdateWaterStream(float DeltaTime);

	/**
	 * Perform water raycast to find what we're watering
	 */
	void PerformWateringRaycast();

	/**
	 * Apply water to farm plots
	 */
	void ApplyWaterToFarmPlots(float DeltaTime);

	/**
	 * Update haptic feedback based on pouring state
	 */
	void UpdateHapticFeedback();

	/**
	 * Play haptic pulse for watering
	 */
	void PlayWateringHaptics(float Intensity);

	/**
	 * Update visual feedback (water level, etc.)
	 */
	void UpdateVisualFeedback();

	/**
	 * Update audio based on pouring state
	 */
	void UpdateAudio();

	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	// Material instance for water level visualization
	UPROPERTY()
	UMaterialInstanceDynamic* CanMaterialInstance;

	// Last haptic feedback time
	float LastHapticTime;

	// Haptic feedback interval
	UPROPERTY(EditAnywhere, Category = "Haptics")
	float HapticFeedbackInterval;

	// Haptic intensity multiplier
	UPROPERTY(EditAnywhere, Category = "Haptics")
	float HapticIntensityMultiplier;

	// Water stream hit location
	FVector WaterHitLocation;

	// Is water hitting something
	bool bWaterHittingSurface;
};
