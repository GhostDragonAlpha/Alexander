// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRSeedItem.generated.h"

// Forward declarations
class UCropDefinition;
class UStaticMeshComponent;
class USphereComponent;

/**
 * VR-grabbable seed item for planting crops
 * Represents a physical seed that players can pick up and plant
 */
UCLASS()
class ALEXANDER_API AVRSeedItem : public AActor
{
	GENERATED_BODY()

public:
	AVRSeedItem();

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
	UStaticMeshComponent* SeedMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* GrabCollision;

	// ============================================================================
	// SEED PROPERTIES
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seed")
	UCropDefinition* CropType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seed")
	int32 SeedCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seed")
	FLinearColor SeedColor;

	// ============================================================================
	// VR INTERACTION STATE
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "VR")
	bool bIsGrabbed;

	UPROPERTY(BlueprintReadOnly, Category = "VR")
	AActor* GrabbingHand;

	// ============================================================================
	// PLANTING STATE
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Planting")
	bool bIsOverPlantableArea;

	UPROPERTY(BlueprintReadOnly, Category = "Planting")
	class AFarmPlot* TargetFarmPlot;

	UPROPERTY(BlueprintReadOnly, Category = "Planting")
	FIntPoint TargetGridPosition;

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
	 * Check if seed can be planted at current location
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting")
	bool CanPlantAtCurrentLocation() const;

	/**
	 * Attempt to plant seed at current location
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting")
	bool TryPlantSeed();

	/**
	 * Update planting target based on current position
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting")
	void UpdatePlantingTarget();

	/**
	 * Get crop definition for this seed
	 */
	UFUNCTION(BlueprintCallable, Category = "Seed")
	UCropDefinition* GetCropType() const { return CropType; }

	/**
	 * Set crop definition for this seed
	 */
	UFUNCTION(BlueprintCallable, Category = "Seed")
	void SetCropType(UCropDefinition* NewCropType);

	/**
	 * Get remaining seed count
	 */
	UFUNCTION(BlueprintCallable, Category = "Seed")
	int32 GetSeedCount() const { return SeedCount; }

	/**
	 * Consume one seed (returns false if no seeds left)
	 */
	UFUNCTION(BlueprintCallable, Category = "Seed")
	bool ConsumeSeed();

protected:
	// Find farm plot at current location
	AFarmPlot* FindFarmPlotAtLocation(FVector Location);

	// Calculate grid position from world location
	FIntPoint CalculateGridPosition(AFarmPlot* FarmPlot, FVector WorldLocation);

	// Update visual feedback for planting
	void UpdatePlantingVisuals();

	// Spawn haptic feedback for planting
	void PlayPlantingHaptics();

	// Visual feedback material
	UPROPERTY()
	UMaterialInstanceDynamic* SeedMaterialInstance;

	// Planting detection radius
	UPROPERTY(EditAnywhere, Category = "Planting")
	float PlantingDetectionRadius;

	// Height above ground for planting detection
	UPROPERTY(EditAnywhere, Category = "Planting")
	float PlantingHeightThreshold;
};
