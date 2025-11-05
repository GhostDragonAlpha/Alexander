// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FarmStatusWidget.generated.h"

// Forward declarations
class AFarmPlot;
class UTextBlock;
class UProgressBar;
class UImage;
class UCanvasPanel;
class UGridPanel;
class UCropDefinition;
class UFarmAlertSystem;
struct FFarmAlert;

/**
 * Grid cell status for visualization
 */
USTRUCT(BlueprintType)
struct FFarmGridCellStatus
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	bool bHasCrop;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	FString CropName;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	float GrowthProgress;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	float Health;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	bool bNeedsWater;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	FLinearColor StatusColor;

	FFarmGridCellStatus()
		: bHasCrop(false)
		, GrowthProgress(0.0f)
		, Health(1.0f)
		, bNeedsWater(false)
		, StatusColor(FLinearColor::White)
	{
	}
};

/**
 * Farm plot statistics
 */
USTRUCT(BlueprintType)
struct FFarmPlotStatistics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 TotalCells;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 PlantedCells;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 MatureCrops;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 CropsNeedingWater;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float AverageHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float AverageGrowthProgress;

	FFarmPlotStatistics()
		: TotalCells(0)
		, PlantedCells(0)
		, MatureCrops(0)
		, CropsNeedingWater(0)
		, AverageHealth(0.0f)
		, AverageGrowthProgress(0.0f)
	{
	}
};

/**
 * Crop monitoring data for a specific crop type
 */
USTRUCT(BlueprintType)
struct FCropMonitoringData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
	UCropDefinition* CropType;

	UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
	FString CropName;

	UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
	int32 TotalCount;

	UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
	int32 MatureCount;

	UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
	float AverageGrowthProgress;

	UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
	float AverageHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
	FTimespan EstimatedTimeToHarvest;

	UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
	int32 CropsNeedingWater;

	UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
	int32 UnhealthyCrops;

	FCropMonitoringData()
		: CropType(nullptr)
		, TotalCount(0)
		, MatureCount(0)
		, AverageGrowthProgress(0.0f)
		, AverageHealth(1.0f)
		, CropsNeedingWater(0)
		, UnhealthyCrops(0)
	{
	}
};

/**
 * Widget for displaying farm plot status information
 * Shows soil quality, water level, fertility, and crop grid visualization
 */
UCLASS()
class ALEXANDER_API UFarmStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFarmStatusWidget(const FObjectInitializer& ObjectInitializer);

	// ============================================================================
	// WIDGET LIFECYCLE
	// ============================================================================

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ============================================================================
	// FARM PLOT BINDING
	// ============================================================================

	/**
	 * Set the farm plot to display status for
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status")
	void SetFarmPlot(AFarmPlot* InFarmPlot);

	/**
	 * Get the currently bound farm plot
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status")
	AFarmPlot* GetFarmPlot() const { return FarmPlot; }

	// ============================================================================
	// UPDATE FUNCTIONS
	// ============================================================================

	/**
	 * Update all status displays
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status")
	void UpdateStatus();

	/**
	 * Update soil quality display
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status")
	void UpdateSoilQualityDisplay();

	/**
	 * Update water level display
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status")
	void UpdateWaterLevelDisplay();

	/**
	 * Update fertility display
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status")
	void UpdateFertilityDisplay();

	/**
	 * Update nutrient levels display (NPK)
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status")
	void UpdateNutrientDisplay();

	/**
	 * Update crop grid visualization
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status")
	void UpdateCropGridVisualization();

	/**
	 * Update farm statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status")
	void UpdateStatistics();

	/**
	 * Update crop monitoring display
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status")
	void UpdateCropMonitoring();

	// ============================================================================
	// ALERT SYSTEM (Task 17.3)
	// ============================================================================

	/**
	 * Update alert display
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status|Alerts")
	void UpdateAlertDisplay();

	/**
	 * Get active alerts for this farm
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status|Alerts")
	TArray<FFarmAlert> GetFarmAlerts() const;

	/**
	 * Acknowledge alert
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status|Alerts")
	void AcknowledgeAlert(int32 AlertID);

	/**
	 * Dismiss alert
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status|Alerts")
	void DismissAlert(int32 AlertID);

	// ============================================================================
	// CROP MONITORING (Task 17.2)
	// ============================================================================

	/**
	 * Get list of all unique crop types currently planted
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status|Monitoring")
	TArray<UCropDefinition*> GetPlantedCropTypes() const;

	/**
	 * Get crop monitoring data for a specific crop type
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status|Monitoring")
	FCropMonitoringData GetCropMonitoringData(UCropDefinition* CropType) const;

	/**
	 * Get all crop monitoring data
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status|Monitoring")
	TArray<FCropMonitoringData> GetAllCropMonitoringData() const;

	/**
	 * Set crop type filter (nullptr for all crops)
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status|Monitoring")
	void SetCropTypeFilter(UCropDefinition* CropType);

	/**
	 * Clear crop type filter
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status|Monitoring")
	void ClearCropTypeFilter();

	/**
	 * Get estimated harvest time for a crop cell
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status|Monitoring")
	FTimespan GetEstimatedHarvestTime(FIntPoint GridPosition) const;

	/**
	 * Format timespan for display
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status|Monitoring")
	FText FormatTimespan(FTimespan Time) const;

	// ============================================================================
	// HARVEST TRACKING (Task 17.4)
	// ============================================================================

	/**
	 * Update harvest tracking display
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status|Harvest")
	void UpdateHarvestTrackingDisplay();

	/**
	 * Get harvest statistics for display
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status|Harvest")
	TArray<FHarvestStatistics> GetHarvestStatisticsForDisplay() const;

	/**
	 * Get recent harvest records
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status|Harvest")
	TArray<FHarvestRecord> GetRecentHarvests(int32 Count = 10) const;

	/**
	 * Get total economic value
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status|Harvest")
	float GetTotalFarmValue() const;

	/**
	 * Format currency value for display
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status|Harvest")
	FText FormatCurrency(float Value) const;

	// ============================================================================
	// GRID VISUALIZATION
	// ============================================================================

	/**
	 * Get status for a specific grid cell
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status")
	FFarmGridCellStatus GetGridCellStatus(FIntPoint GridPosition) const;

	/**
	 * Get color for grid cell based on status
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status")
	FLinearColor GetGridCellColor(const FFarmGridCellStatus& CellStatus) const;

	/**
	 * Calculate farm plot statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm Status")
	FFarmPlotStatistics CalculateStatistics() const;

	// ============================================================================
	// WIDGET COMPONENTS (Bind in Blueprint)
	// ============================================================================

	// Soil Quality
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* SoilQualityText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* SoilQualityBar;

	// Water Level
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* WaterLevelText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* WaterLevelBar;

	// Fertility
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* FertilityText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* FertilityBar;

	// Nutrients (NPK)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* NitrogenText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* NitrogenBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* PhosphorusText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* PhosphorusBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* PotassiumText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* PotassiumBar;

	// Statistics
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* PlantedCellsText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* MatureCropsText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* CropsNeedingWaterText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* AverageHealthText;

	// Crop Grid Visualization
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UCanvasPanel* CropGridPanel;

	// Crop Monitoring (Task 17.2)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* CropMonitoringTitleText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UGridPanel* CropMonitoringGrid;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* FilteredCropTypeText;

	// Alert Display (Task 17.3)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UCanvasPanel* AlertPanel;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* AlertCountText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UGridPanel* AlertListGrid;

	// Harvest Tracking Display (Task 17.4)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UCanvasPanel* HarvestTrackingPanel;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* TotalEconomicValueText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* TotalHarvestsText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UGridPanel* HarvestStatisticsGrid;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UGridPanel* RecentHarvestsGrid;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Update interval in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	float UpdateInterval;

	// Show detailed nutrient information
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	bool bShowNutrientDetails;

	// Show crop grid visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	bool bShowCropGrid;

	// Grid cell size in pixels
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	float GridCellSize;

	// Grid cell spacing in pixels
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	float GridCellSpacing;

	// Color for healthy crops
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	FLinearColor HealthyCropColor;

	// Color for crops needing water
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	FLinearColor NeedsWaterColor;

	// Color for unhealthy crops
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	FLinearColor UnhealthyCropColor;

	// Color for empty cells
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	FLinearColor EmptyCellColor;

	// Color for mature crops
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	FLinearColor MatureCropColor;

	// Show crop monitoring section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	bool bShowCropMonitoring;

	// Maximum number of crop types to display in monitoring
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	int32 MaxCropTypesDisplayed;

	// Show alert panel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	bool bShowAlerts;

	// Maximum number of alerts to display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	int32 MaxAlertsDisplayed;

	// Show harvest tracking section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	bool bShowHarvestTracking;

	// Maximum number of harvest statistics to display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	int32 MaxHarvestStatsDisplayed;

	// Maximum number of recent harvests to display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
	int32 MaxRecentHarvestsDisplayed;

protected:
	// Farm plot being monitored
	UPROPERTY()
	AFarmPlot* FarmPlot;

	// Current crop type filter (nullptr = show all)
	UPROPERTY()
	UCropDefinition* CropTypeFilter;

	// Alert system reference
	UPROPERTY()
	UFarmAlertSystem* AlertSystem;

	// Time since last update
	float TimeSinceLastUpdate;

	// Grid cell images for visualization
	UPROPERTY()
	TArray<UImage*> GridCellImages;

	// ============================================================================
	// HELPER FUNCTIONS
	// ============================================================================

	/**
	 * Format percentage value for display
	 */
	FText FormatPercentage(float Value) const;

	/**
	 * Format count value for display
	 */
	FText FormatCount(int32 Value) const;

	/**
	 * Get quality color based on value (0-1)
	 */
	FLinearColor GetQualityColor(float Quality) const;

	/**
	 * Create grid cell visualization
	 */
	void CreateGridVisualization();

	/**
	 * Update individual grid cell image
	 */
	void UpdateGridCellImage(int32 CellIndex, const FFarmGridCellStatus& CellStatus);

	/**
	 * Clear grid visualization
	 */
	void ClearGridVisualization();

	/**
	 * Add monitoring header row to grid
	 */
	void AddMonitoringHeaderRow(int32 Row);

	/**
	 * Add monitoring data row to grid
	 */
	void AddMonitoringDataRow(const FCropMonitoringData& Data, int32 Row);

	/**
	 * Add alert row to grid
	 */
	void AddAlertRow(const FFarmAlert& Alert, int32 Row);

	/**
	 * Get alert system
	 */
	UFarmAlertSystem* GetAlertSystem();

	/**
	 * Add harvest statistics header row to grid
	 */
	void AddHarvestStatsHeaderRow(int32 Row);

	/**
	 * Add harvest statistics data row to grid
	 */
	void AddHarvestStatsDataRow(const FHarvestStatistics& Stats, int32 Row);

	/**
	 * Add recent harvests header row to grid
	 */
	void AddRecentHarvestsHeaderRow(int32 Row);

	/**
	 * Add recent harvest data row to grid
	 */
	void AddRecentHarvestDataRow(const FHarvestRecord& Record, int32 Row);
};
