// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerSystem.h"
#include "SolarPanel.generated.h"

// Forward declarations
class UStaticMeshComponent;
class UArrowComponent;

/**
 * Solar panel actor - generates power from sunlight
 * Power output depends on sun angle and time of day
 */
UCLASS()
class ALEXANDER_API ASolarPanel : public AActor, public IPowerProvider
{
	GENERATED_BODY()

public:
	ASolarPanel();

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
	UStaticMeshComponent* PanelMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MountMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* SunDirectionIndicator;

	// ============================================================================
	// SOLAR PANEL PROPERTIES
	// ============================================================================

	/** Maximum power output in watts (at optimal sun angle) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar")
	float MaxPowerOutput;

	/** Current power output in watts */
	UPROPERTY(BlueprintReadOnly, Category = "Solar")
	float CurrentPowerOutput;

	/** Efficiency rating (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar")
	float Efficiency;

	/** Panel surface area in square meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar")
	float PanelArea;

	/** Whether panel can rotate to track sun */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar")
	bool bSunTracking;

	/** Sun tracking rotation speed (degrees per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar", meta = (EditCondition = "bSunTracking"))
	float TrackingSpeed;

	/** Minimum sun angle for power generation (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar")
	float MinSunAngle;

	/** Reference to planet actor for sun position */
	UPROPERTY(BlueprintReadWrite, Category = "Solar")
	AActor* PlanetActor;

	// ============================================================================
	// POWER GENERATION
	// ============================================================================

	/**
	 * Calculate current power output based on sun position
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar")
	void UpdatePowerOutput();

	/**
	 * Get sun direction from planet's day-night cycle
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar")
	FVector GetSunDirection() const;

	/**
	 * Calculate sun angle relative to panel surface
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar")
	float CalculateSunAngle() const;

	/**
	 * Calculate efficiency multiplier based on sun angle
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar")
	float CalculateAngleEfficiency(float SunAngle) const;

	/**
	 * Rotate panel to face sun (if sun tracking enabled)
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar")
	void TrackSun(float DeltaTime);

	/**
	 * Get power output percentage (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar")
	float GetPowerOutputPercent() const;

	// ============================================================================
	// IPowerProvider Interface
	// ============================================================================

	virtual float GetAvailablePower_Implementation() const override;
	virtual float GetMaxPowerCapacity_Implementation() const override;
	virtual EPowerSourceType GetPowerSourceType_Implementation() const override;

protected:
	/**
	 * Find planet actor in world
	 */
	void FindPlanetActor();

	/**
	 * Update visual indicators
	 */
	void UpdateVisuals();
};
