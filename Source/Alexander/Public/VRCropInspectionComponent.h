// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRCropInspectionComponent.generated.h"

// Forward declarations
class AFarmPlot;
class UCropDefinition;
class UUserWidget;
class UVRCropInfoWidget;
struct FCropCell;
struct FHealthCalculationResult;

/**
 * Crop inspection data for UI display
 */
USTRUCT(BlueprintType)
struct FCropInspectionData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inspection")
	FString CropName;

	UPROPERTY(BlueprintReadOnly, Category = "Inspection")
	FString GrowthStageName;

	UPROPERTY(BlueprintReadOnly, Category = "Inspection")
	float GrowthProgress; // 0-1

	UPROPERTY(BlueprintReadOnly, Category = "Inspection")
	float Health; // 0-1

	UPROPERTY(BlueprintReadOnly, Category = "Inspection")
	FString HealthStatus; // "Excellent", "Good", etc.

	UPROPERTY(BlueprintReadOnly, Category = "Inspection")
	FLinearColor HealthStatusColor;

	UPROPERTY(BlueprintReadOnly, Category = "Inspection")
	FString PrimaryIssue;

	UPROPERTY(BlueprintReadOnly, Category = "Inspection")
	bool bNeedsWater;

	UPROPERTY(BlueprintReadOnly, Category = "Inspection")
	bool bNeedsFertilizer;

	UPROPERTY(BlueprintReadOnly, Category = "Inspection")
	float TimeToHarvest; // Seconds

	UPROPERTY(BlueprintReadOnly, Category = "Inspection")
	FVector WorldLocation;

	FCropInspectionData()
		: GrowthProgress(0.0f)
		, Health(1.0f)
		, HealthStatusColor(FLinearColor::Green)
		, bNeedsWater(false)
		, bNeedsFertilizer(false)
		, TimeToHarvest(0.0f)
		, WorldLocation(FVector::ZeroVector)
	{
	}
};

/**
 * VR Crop Inspection Component
 * Detects when player looks at crops and displays information
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UVRCropInspectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRCropInspectionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Maximum distance to detect crops (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection|Config")
	float InspectionRange;

	/** Angle tolerance for look-at detection (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection|Config")
	float LookAtAngleTolerance;

	/** How long to look at crop before showing info (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection|Config")
	float LookAtDuration;

	/** UI widget class for crop info display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection|UI")
	TSubclassOf<UUserWidget> CropInfoWidgetClass;

	/** Whether to show debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection|Debug")
	bool bShowDebugVisualization;

	// ============================================================================
	// STATE
	// ============================================================================

	/** Currently inspected farm plot */
	UPROPERTY(BlueprintReadOnly, Category = "Inspection|State")
	AFarmPlot* CurrentFarmPlot;

	/** Currently inspected crop grid position */
	UPROPERTY(BlueprintReadOnly, Category = "Inspection|State")
	FIntPoint CurrentCropPosition;

	/** Current inspection data */
	UPROPERTY(BlueprintReadOnly, Category = "Inspection|State")
	FCropInspectionData CurrentInspectionData;

	/** Whether currently looking at a crop */
	UPROPERTY(BlueprintReadOnly, Category = "Inspection|State")
	bool bIsLookingAtCrop;

	/** Time spent looking at current crop */
	UPROPERTY(BlueprintReadOnly, Category = "Inspection|State")
	float CurrentLookAtTime;

	/** Whether info UI is currently visible */
	UPROPERTY(BlueprintReadOnly, Category = "Inspection|State")
	bool bIsInfoVisible;

	// ============================================================================
	// UI WIDGET
	// ============================================================================

	/** Crop info widget instance */
	UPROPERTY()
	UVRCropInfoWidget* CropInfoWidget;

	// ============================================================================
	// INSPECTION FUNCTIONS
	// ============================================================================

	/**
	 * Perform look-at detection and update inspection state
	 */
	UFUNCTION(BlueprintCallable, Category = "Inspection")
	void UpdateInspection(float DeltaTime);

	/**
	 * Check if player is looking at a crop
	 * @param OutFarmPlot The farm plot containing the crop
	 * @param OutGridPosition The grid position of the crop
	 * @return True if looking at a crop
	 */
	UFUNCTION(BlueprintCallable, Category = "Inspection")
	bool DetectLookAtCrop(AFarmPlot*& OutFarmPlot, FIntPoint& OutGridPosition);

	/**
	 * Get inspection data for a specific crop
	 */
	UFUNCTION(BlueprintCallable, Category = "Inspection")
	FCropInspectionData GetCropInspectionData(AFarmPlot* FarmPlot, FIntPoint GridPosition);

	/**
	 * Show crop info UI
	 */
	UFUNCTION(BlueprintCallable, Category = "Inspection")
	void ShowCropInfo(const FCropInspectionData& InspectionData);

	/**
	 * Hide crop info UI
	 */
	UFUNCTION(BlueprintCallable, Category = "Inspection")
	void HideCropInfo();

	/**
	 * Update crop info UI with new data
	 */
	UFUNCTION(BlueprintCallable, Category = "Inspection")
	void UpdateCropInfo(const FCropInspectionData& InspectionData);

	// ============================================================================
	// HELPER FUNCTIONS
	// ============================================================================

	/**
	 * Get camera location and forward vector for look-at detection
	 */
	void GetCameraInfo(FVector& OutLocation, FVector& OutForward) const;

	/**
	 * Find farm plots in range
	 */
	TArray<AFarmPlot*> FindNearbyFarmPlots(const FVector& Location, float Range) const;

	/**
	 * Get world location of crop cell
	 */
	FVector GetCropCellWorldLocation(AFarmPlot* FarmPlot, FIntPoint GridPosition) const;

	/**
	 * Check if looking at specific location
	 */
	bool IsLookingAt(const FVector& CameraLocation, const FVector& CameraForward, const FVector& TargetLocation, float AngleTolerance) const;

	/**
	 * Format time duration for display
	 */
	FString FormatTimeDuration(float Seconds) const;

	/**
	 * Draw debug visualization
	 */
	void DrawDebugVisualization(const FVector& CameraLocation, const FVector& CameraForward);
};
