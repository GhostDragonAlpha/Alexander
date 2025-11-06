// Copyright Epic Games, Inc. All Rights Reserved.

#include "FarmStatusWidget.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "FarmPlot.h"
#include "CropDefinition.h"
#include "FarmAlertSystem.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Engine/World.h"

UFarmStatusWidget::UFarmStatusWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, FarmPlot(nullptr)
	, UpdateInterval(0.5f)
	, bShowNutrientDetails(true)
	, bShowCropGrid(true)
	, GridCellSize(10.0f)
	, GridCellSpacing(2.0f)
	, HealthyCropColor(FLinearColor::Green)
	, NeedsWaterColor(FLinearColor::Yellow)
	, UnhealthyCropColor(FLinearColor::Red)
	, EmptyCellColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
	, MatureCropColor(FLinearColor(0.0f, 1.0f, 0.5f, 1.0f))
	, bShowCropMonitoring(true)
	, MaxCropTypesDisplayed(10)
	, bShowAlerts(true)
	, MaxAlertsDisplayed(5)
	, bShowHarvestTracking(true)
	, MaxHarvestStatsDisplayed(5)
	, MaxRecentHarvestsDisplayed(10)
	, AlertSystem(nullptr)
	, TimeSinceLastUpdate(0.0f)
	, CropTypeFilter(nullptr)
{
}

void UFarmStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get alert system
	AlertSystem = GetAlertSystem();

	// Initialize grid visualization if enabled
	if (bShowCropGrid && CropGridPanel)
	{
		CreateGridVisualization();
	}

	// Initial update
	UpdateStatus();
}

void UFarmStatusWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update at specified interval
	TimeSinceLastUpdate += InDeltaTime;
	if (TimeSinceLastUpdate >= UpdateInterval)
	{
		UpdateStatus();
		TimeSinceLastUpdate = 0.0f;
	}
}

void UFarmStatusWidget::SetFarmPlot(AFarmPlot* InFarmPlot)
{
	FarmPlot = InFarmPlot;

	// Clear and recreate grid visualization for new farm
	if (bShowCropGrid && CropGridPanel)
	{
		ClearGridVisualization();
		CreateGridVisualization();
	}

	// Update immediately
	UpdateStatus();
}

void UFarmStatusWidget::UpdateStatus()
{
	if (!FarmPlot)
	{
		return;
	}

	UpdateSoilQualityDisplay();
	UpdateWaterLevelDisplay();
	UpdateFertilityDisplay();

	if (bShowNutrientDetails)
	{
		UpdateNutrientDisplay();
	}

	if (bShowCropGrid)
	{
		UpdateCropGridVisualization();
	}

	UpdateStatistics();

	if (bShowCropMonitoring)
	{
		UpdateCropMonitoring();
	}

	if (bShowAlerts)
	{
		UpdateAlertDisplay();
	}

	if (bShowHarvestTracking)
	{
		UpdateHarvestTrackingDisplay();
	}
}

void UFarmStatusWidget::UpdateSoilQualityDisplay()
{
	if (!FarmPlot)
	{
		return;
	}

	float SoilQuality = FarmPlot->CalculateSoilQuality();

	if (SoilQualityText)
	{
		SoilQualityText->SetText(FormatPercentage(SoilQuality));
		SoilQualityText->SetColorAndOpacity(FSlateColor(GetQualityColor(SoilQuality)));
	}

	if (SoilQualityBar)
	{
		SoilQualityBar->SetPercent(SoilQuality);
		SoilQualityBar->SetFillColorAndOpacity(GetQualityColor(SoilQuality));
	}
}

void UFarmStatusWidget::UpdateWaterLevelDisplay()
{
	if (!FarmPlot)
	{
		return;
	}

	float WaterLevel = FarmPlot->WaterLevel;

	if (WaterLevelText)
	{
		WaterLevelText->SetText(FormatPercentage(WaterLevel));
		WaterLevelText->SetColorAndOpacity(FSlateColor(GetQualityColor(WaterLevel)));
	}

	if (WaterLevelBar)
	{
		WaterLevelBar->SetPercent(WaterLevel);
		WaterLevelBar->SetFillColorAndOpacity(FLinearColor(0.2f, 0.5f, 1.0f, 1.0f));
	}
}

void UFarmStatusWidget::UpdateFertilityDisplay()
{
	if (!FarmPlot)
	{
		return;
	}

	float Fertility = FarmPlot->Fertility;

	if (FertilityText)
	{
		FertilityText->SetText(FormatPercentage(Fertility));
		FertilityText->SetColorAndOpacity(FSlateColor(GetQualityColor(Fertility)));
	}

	if (FertilityBar)
	{
		FertilityBar->SetPercent(Fertility);
		FertilityBar->SetFillColorAndOpacity(GetQualityColor(Fertility));
	}
}

void UFarmStatusWidget::UpdateNutrientDisplay()
{
	if (!FarmPlot)
	{
		return;
	}

	// Nitrogen
	if (NitrogenText)
	{
		NitrogenText->SetText(FormatPercentage(FarmPlot->NitrogenLevel));
		NitrogenText->SetColorAndOpacity(FSlateColor(GetQualityColor(FarmPlot->NitrogenLevel)));
	}
	if (NitrogenBar)
	{
		NitrogenBar->SetPercent(FarmPlot->NitrogenLevel);
		NitrogenBar->SetFillColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.2f, 1.0f));
	}

	// Phosphorus
	if (PhosphorusText)
	{
		PhosphorusText->SetText(FormatPercentage(FarmPlot->PhosphorusLevel));
		PhosphorusText->SetColorAndOpacity(FSlateColor(GetQualityColor(FarmPlot->PhosphorusLevel)));
	}
	if (PhosphorusBar)
	{
		PhosphorusBar->SetPercent(FarmPlot->PhosphorusLevel);
		PhosphorusBar->SetFillColorAndOpacity(FLinearColor(1.0f, 0.5f, 0.2f, 1.0f));
	}

	// Potassium
	if (PotassiumText)
	{
		PotassiumText->SetText(FormatPercentage(FarmPlot->PotassiumLevel));
		PotassiumText->SetColorAndOpacity(FSlateColor(GetQualityColor(FarmPlot->PotassiumLevel)));
	}
	if (PotassiumBar)
	{
		PotassiumBar->SetPercent(FarmPlot->PotassiumLevel);
		PotassiumBar->SetFillColorAndOpacity(FLinearColor(0.8f, 0.2f, 0.8f, 1.0f));
	}
}

void UFarmStatusWidget::UpdateCropGridVisualization()
{
	if (!FarmPlot || !CropGridPanel)
	{
		return;
	}

	// Update each grid cell image
	int32 GridResolution = FarmPlot->GridResolution;
	for (int32 Y = 0; Y < GridResolution; ++Y)
	{
		for (int32 X = 0; X < GridResolution; ++X)
		{
			FIntPoint GridPos(X, Y);
			int32 CellIndex = Y * GridResolution + X;

			if (CellIndex < GridCellImages.Num() && GridCellImages[CellIndex])
			{
				FFarmGridCellStatus CellStatus = GetGridCellStatus(GridPos);
				UpdateGridCellImage(CellIndex, CellStatus);
			}
		}
	}
}

void UFarmStatusWidget::UpdateStatistics()
{
	if (!FarmPlot)
	{
		return;
	}

	FFarmPlotStatistics Stats = CalculateStatistics();

	if (PlantedCellsText)
	{
		PlantedCellsText->SetText(FText::Format(
			FText::FromString(TEXT("{0} / {1}")),
			FText::AsNumber(Stats.PlantedCells),
			FText::AsNumber(Stats.TotalCells)
		));
	}

	if (MatureCropsText)
	{
		MatureCropsText->SetText(FormatCount(Stats.MatureCrops));
	}

	if (CropsNeedingWaterText)
	{
		CropsNeedingWaterText->SetText(FormatCount(Stats.CropsNeedingWater));
		
		// Highlight if crops need water
		if (Stats.CropsNeedingWater > 0)
		{
			CropsNeedingWaterText->SetColorAndOpacity(FSlateColor(NeedsWaterColor));
		}
		else
		{
			CropsNeedingWaterText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
	}

	if (AverageHealthText)
	{
		AverageHealthText->SetText(FormatPercentage(Stats.AverageHealth));
		AverageHealthText->SetColorAndOpacity(FSlateColor(GetQualityColor(Stats.AverageHealth)));
	}
}

FFarmGridCellStatus UFarmStatusWidget::GetGridCellStatus(FIntPoint GridPosition) const
{
	FFarmGridCellStatus Status;

	if (!FarmPlot)
	{
		return Status;
	}

	int32 CellIndex = GridPosition.Y * FarmPlot->GridResolution + GridPosition.X;
	if (CellIndex < 0 || CellIndex >= FarmPlot->CropGrid.Num())
	{
		return Status;
	}

	const FCropCell& Cell = FarmPlot->CropGrid[CellIndex];

	Status.bHasCrop = (Cell.CropType != nullptr);
	Status.GrowthProgress = Cell.GrowthProgress;
	Status.Health = Cell.Health;
	Status.bNeedsWater = Cell.bNeedsWater;

	if (Cell.CropType)
	{
		Status.CropName = Cell.CropType->CropName;
	}

	Status.StatusColor = GetGridCellColor(Status);

	return Status;
}

FLinearColor UFarmStatusWidget::GetGridCellColor(const FFarmGridCellStatus& CellStatus) const
{
	if (!CellStatus.bHasCrop)
	{
		return EmptyCellColor;
	}

	// Mature crop
	if (CellStatus.GrowthProgress >= 1.0f)
	{
		return MatureCropColor;
	}

	// Needs water
	if (CellStatus.bNeedsWater)
	{
		return NeedsWaterColor;
	}

	// Unhealthy
	if (CellStatus.Health < 0.5f)
	{
		return UnhealthyCropColor;
	}

	// Healthy growing crop - blend from yellow to green based on growth
	return FLinearColor::LerpUsingHSV(
		FLinearColor(0.8f, 0.8f, 0.2f, 1.0f), // Yellow for young
		HealthyCropColor,                       // Green for mature
		CellStatus.GrowthProgress
	);
}

FFarmPlotStatistics UFarmStatusWidget::CalculateStatistics() const
{
	FFarmPlotStatistics Stats;

	if (!FarmPlot)
	{
		return Stats;
	}

	Stats.TotalCells = FarmPlot->CropGrid.Num();

	float TotalHealth = 0.0f;
	float TotalGrowth = 0.0f;

	for (const FCropCell& Cell : FarmPlot->CropGrid)
	{
		if (Cell.CropType)
		{
			Stats.PlantedCells++;
			TotalHealth += Cell.Health;
			TotalGrowth += Cell.GrowthProgress;

			if (Cell.GrowthProgress >= 1.0f)
			{
				Stats.MatureCrops++;
			}

			if (Cell.bNeedsWater)
			{
				Stats.CropsNeedingWater++;
			}
		}
	}

	if (Stats.PlantedCells > 0)
	{
		Stats.AverageHealth = TotalHealth / Stats.PlantedCells;
		Stats.AverageGrowthProgress = TotalGrowth / Stats.PlantedCells;
	}

	return Stats;
}

FText UFarmStatusWidget::FormatPercentage(float Value) const
{
	int32 Percentage = FMath::RoundToInt(Value * 100.0f);
	return FText::Format(FText::FromString(TEXT("{0}%")), FText::AsNumber(Percentage));
}

FText UFarmStatusWidget::FormatCount(int32 Value) const
{
	return FText::AsNumber(Value);
}

FLinearColor UFarmStatusWidget::GetQualityColor(float Quality) const
{
	// Red for low quality, yellow for medium, green for high
	if (Quality < 0.33f)
	{
		return FLinearColor::Red;
	}
	else if (Quality < 0.66f)
	{
		return FLinearColor::Yellow;
	}
	else
	{
		return FLinearColor::Green;
	}
}

void UFarmStatusWidget::CreateGridVisualization()
{
	if (!FarmPlot || !CropGridPanel)
	{
		return;
	}

	ClearGridVisualization();

	int32 GridResolution = FarmPlot->GridResolution;
	GridCellImages.Reserve(GridResolution * GridResolution);

	for (int32 Y = 0; Y < GridResolution; ++Y)
	{
		for (int32 X = 0; X < GridResolution; ++X)
		{
			// Create image widget for this cell
			UImage* CellImage = NewObject<UImage>(this);
			if (CellImage)
			{
				// Add to canvas panel
				UCanvasPanelSlot* PanelSlot = CropGridPanel->AddChildToCanvas(CellImage);
				if (PanelSlot)
				{
					// Position the cell
					float PosX = X * (GridCellSize + GridCellSpacing);
					float PosY = Y * (GridCellSize + GridCellSpacing);
					PanelSlot->SetPosition(FVector2D(PosX, PosY));
					PanelSlot->SetSize(FVector2D(GridCellSize, GridCellSize));
				}

				// Set initial color
				CellImage->SetColorAndOpacity(EmptyCellColor);

				GridCellImages.Add(CellImage);
			}
		}
	}
}

void UFarmStatusWidget::UpdateGridCellImage(int32 CellIndex, const FFarmGridCellStatus& CellStatus)
{
	if (CellIndex < 0 || CellIndex >= GridCellImages.Num())
	{
		return;
	}

	UImage* CellImage = GridCellImages[CellIndex];
	if (CellImage)
	{
		CellImage->SetColorAndOpacity(CellStatus.StatusColor);
	}
}

void UFarmStatusWidget::ClearGridVisualization()
{
	if (CropGridPanel)
	{
		CropGridPanel->ClearChildren();
	}

	GridCellImages.Empty();
}

// ============================================================================
// CROP MONITORING IMPLEMENTATION (Task 17.2)
// ============================================================================

void UFarmStatusWidget::UpdateCropMonitoring()
{
	if (!FarmPlot || !CropMonitoringGrid)
	{
		return;
	}

	// Update filter display
	if (FilteredCropTypeText)
	{
		if (CropTypeFilter)
		{
			FilteredCropTypeText->SetText(FText::Format(
				FText::FromString(TEXT("Filtered: {0}")),
				FText::FromString(CropTypeFilter->CropName)
			));
			FilteredCropTypeText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			FilteredCropTypeText->SetText(FText::FromString(TEXT("All Crops")));
			FilteredCropTypeText->SetVisibility(ESlateVisibility::Visible);
		}
	}

	// Get monitoring data
	TArray<FCropMonitoringData> MonitoringData;
	if (CropTypeFilter)
	{
		// Show only filtered crop type
		FCropMonitoringData Data = GetCropMonitoringData(CropTypeFilter);
		if (Data.TotalCount > 0)
		{
			MonitoringData.Add(Data);
		}
	}
	else
	{
		// Show all crop types
		MonitoringData = GetAllCropMonitoringData();
	}

	// Clear existing monitoring display
	CropMonitoringGrid->ClearChildren();

	// Add header row
	int32 Row = 0;
	AddMonitoringHeaderRow(Row++);

	// Add data rows (limited by MaxCropTypesDisplayed)
	int32 DisplayCount = FMath::Min(MonitoringData.Num(), MaxCropTypesDisplayed);
	for (int32 i = 0; i < DisplayCount; ++i)
	{
		AddMonitoringDataRow(MonitoringData[i], Row++);
	}
}

void UFarmStatusWidget::AddMonitoringHeaderRow(int32 Row)
{
	if (!CropMonitoringGrid)
	{
		return;
	}

	// Create header labels
	TArray<FString> Headers = {
		TEXT("Crop"),
		TEXT("Count"),
		TEXT("Growth"),
		TEXT("Health"),
		TEXT("Harvest Time"),
		TEXT("Needs Water")
	};

	for (int32 Col = 0; Col < Headers.Num(); ++Col)
	{
		UTextBlock* HeaderText = NewObject<UTextBlock>(this);
		if (HeaderText)
		{
			HeaderText->SetText(FText::FromString(Headers[Col]));
			HeaderText->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f)));
			
			UGridSlot* GridSlot = CropMonitoringGrid->AddChildToGrid(HeaderText, Row, Col);
			if (GridSlot)
			{
				GridSlot->SetPadding(FMargin(5.0f, 2.0f));
			}
		}
	}
}

void UFarmStatusWidget::AddMonitoringDataRow(const FCropMonitoringData& Data, int32 Row)
{
	if (!CropMonitoringGrid)
	{
		return;
	}

	int32 Col = 0;

	// Crop Name
	{
		UTextBlock* NameText = NewObject<UTextBlock>(this);
		if (NameText)
		{
			NameText->SetText(FText::FromString(Data.CropName));
			UGridSlot* GridSlot = CropMonitoringGrid->AddChildToGrid(NameText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}

	// Count (Mature / Total)
	{
		UTextBlock* CountText = NewObject<UTextBlock>(this);
		if (CountText)
		{
			CountText->SetText(FText::Format(
				FText::FromString(TEXT("{0} / {1}")),
				FText::AsNumber(Data.MatureCount),
				FText::AsNumber(Data.TotalCount)
			));
			UGridSlot* GridSlot = CropMonitoringGrid->AddChildToGrid(CountText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}

	// Growth Progress
	{
		UTextBlock* GrowthText = NewObject<UTextBlock>(this);
		if (GrowthText)
		{
			GrowthText->SetText(FormatPercentage(Data.AverageGrowthProgress));
			GrowthText->SetColorAndOpacity(FSlateColor(GetQualityColor(Data.AverageGrowthProgress)));
			UGridSlot* GridSlot = CropMonitoringGrid->AddChildToGrid(GrowthText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}

	// Health
	{
		UTextBlock* HealthText = NewObject<UTextBlock>(this);
		if (HealthText)
		{
			HealthText->SetText(FormatPercentage(Data.AverageHealth));
			HealthText->SetColorAndOpacity(FSlateColor(GetQualityColor(Data.AverageHealth)));
			UGridSlot* GridSlot = CropMonitoringGrid->AddChildToGrid(HealthText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}

	// Estimated Harvest Time
	{
		UTextBlock* TimeText = NewObject<UTextBlock>(this);
		if (TimeText)
		{
			TimeText->SetText(FormatTimespan(Data.EstimatedTimeToHarvest));
			UGridSlot* GridSlot = CropMonitoringGrid->AddChildToGrid(TimeText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}

	// Needs Water Count
	{
		UTextBlock* WaterText = NewObject<UTextBlock>(this);
		if (WaterText)
		{
			WaterText->SetText(FText::AsNumber(Data.CropsNeedingWater));
			if (Data.CropsNeedingWater > 0)
			{
				WaterText->SetColorAndOpacity(FSlateColor(NeedsWaterColor));
			}
			UGridSlot* GridSlot = CropMonitoringGrid->AddChildToGrid(WaterText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}
}

TArray<UCropDefinition*> UFarmStatusWidget::GetPlantedCropTypes() const
{
	TArray<UCropDefinition*> CropTypes;

	if (!FarmPlot)
	{
		return CropTypes;
	}

	// Collect unique crop types
	TSet<UCropDefinition*> UniqueTypes;
	for (const FCropCell& Cell : FarmPlot->CropGrid)
	{
		if (Cell.CropType)
		{
			UniqueTypes.Add(Cell.CropType);
		}
	}

	CropTypes = UniqueTypes.Array();
	return CropTypes;
}

FCropMonitoringData UFarmStatusWidget::GetCropMonitoringData(UCropDefinition* CropType) const
{
	FCropMonitoringData Data;

	if (!FarmPlot || !CropType)
	{
		return Data;
	}

	Data.CropType = CropType;
	Data.CropName = CropType->CropName;

	float TotalGrowth = 0.0f;
	float TotalHealth = 0.0f;
	float MinTimeToHarvest = FLT_MAX;

	// Analyze all cells with this crop type
	for (int32 i = 0; i < FarmPlot->CropGrid.Num(); ++i)
	{
		const FCropCell& Cell = FarmPlot->CropGrid[i];
		
		if (Cell.CropType == CropType)
		{
			Data.TotalCount++;
			TotalGrowth += Cell.GrowthProgress;
			TotalHealth += Cell.Health;

			if (Cell.GrowthProgress >= 1.0f)
			{
				Data.MatureCount++;
			}

			if (Cell.bNeedsWater)
			{
				Data.CropsNeedingWater++;
			}

			if (Cell.Health < 0.5f)
			{
				Data.UnhealthyCrops++;
			}

			// Calculate time to harvest for this cell
			int32 GridX = i % FarmPlot->GridResolution;
			int32 GridY = i / FarmPlot->GridResolution;
			FTimespan TimeToHarvest = GetEstimatedHarvestTime(FIntPoint(GridX, GridY));
			
			if (TimeToHarvest.GetTotalSeconds() > 0 && TimeToHarvest.GetTotalSeconds() < MinTimeToHarvest)
			{
				MinTimeToHarvest = TimeToHarvest.GetTotalSeconds();
			}
		}
	}

	// Calculate averages
	if (Data.TotalCount > 0)
	{
		Data.AverageGrowthProgress = TotalGrowth / Data.TotalCount;
		Data.AverageHealth = TotalHealth / Data.TotalCount;
	}

	// Set estimated harvest time (time until first crop is ready)
	if (MinTimeToHarvest < FLT_MAX)
	{
		Data.EstimatedTimeToHarvest = FTimespan::FromSeconds(MinTimeToHarvest);
	}
	else
	{
		Data.EstimatedTimeToHarvest = FTimespan::Zero();
	}

	return Data;
}

TArray<FCropMonitoringData> UFarmStatusWidget::GetAllCropMonitoringData() const
{
	TArray<FCropMonitoringData> AllData;

	TArray<UCropDefinition*> CropTypes = GetPlantedCropTypes();
	
	for (UCropDefinition* CropType : CropTypes)
	{
		FCropMonitoringData Data = GetCropMonitoringData(CropType);
		if (Data.TotalCount > 0)
		{
			AllData.Add(Data);
		}
	}

	// Sort by crop name for consistent display
	AllData.Sort([](const FCropMonitoringData& A, const FCropMonitoringData& B)
	{
		return A.CropName < B.CropName;
	});

	return AllData;
}

void UFarmStatusWidget::SetCropTypeFilter(UCropDefinition* CropType)
{
	CropTypeFilter = CropType;
	UpdateCropMonitoring();
}

void UFarmStatusWidget::ClearCropTypeFilter()
{
	CropTypeFilter = nullptr;
	UpdateCropMonitoring();
}

FTimespan UFarmStatusWidget::GetEstimatedHarvestTime(FIntPoint GridPosition) const
{
	if (!FarmPlot)
	{
		return FTimespan::Zero();
	}

	int32 CellIndex = GridPosition.Y * FarmPlot->GridResolution + GridPosition.X;
	if (CellIndex < 0 || CellIndex >= FarmPlot->CropGrid.Num())
	{
		return FTimespan::Zero();
	}

	const FCropCell& Cell = FarmPlot->CropGrid[CellIndex];
	
	if (!Cell.CropType || Cell.GrowthProgress >= 1.0f)
	{
		return FTimespan::Zero();
	}

	// Calculate remaining growth time
	float RemainingGrowth = 1.0f - Cell.GrowthProgress;
	float TotalGrowthTime = Cell.CropType->GrowthDuration;
	
	// Get current growth rate from farm plot
	float TimeToHarvest = FarmPlot->GetTimeToHarvest(GridPosition);
	
	if (TimeToHarvest > 0.0f)
	{
		return FTimespan::FromSeconds(TimeToHarvest);
	}

	// Fallback calculation if farm plot doesn't provide time
	float EstimatedTime = RemainingGrowth * TotalGrowthTime;
	return FTimespan::FromSeconds(EstimatedTime);
}

FText UFarmStatusWidget::FormatTimespan(FTimespan Time) const
{
	if (Time.GetTotalSeconds() <= 0)
	{
		return FText::FromString(TEXT("Ready"));
	}

	int32 Hours = Time.GetHours();
	int32 Minutes = Time.GetMinutes();
	int32 Seconds = Time.GetSeconds();

	if (Hours > 0)
	{
		return FText::Format(
			FText::FromString(TEXT("{0}h {1}m")),
			FText::AsNumber(Hours),
			FText::AsNumber(Minutes)
		);
	}
	else if (Minutes > 0)
	{
		return FText::Format(
			FText::FromString(TEXT("{0}m {1}s")),
			FText::AsNumber(Minutes),
			FText::AsNumber(Seconds)
		);
	}
	else
	{
		return FText::Format(
			FText::FromString(TEXT("{0}s")),
			FText::AsNumber(Seconds)
		);
	}
}

// ============================================================================
// ALERT SYSTEM IMPLEMENTATION (Task 17.3)
// ============================================================================

UFarmAlertSystem* UFarmStatusWidget::GetAlertSystem()
{
	if (!AlertSystem)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			AlertSystem = World->GetSubsystem<UFarmAlertSystem>();
		}
	}

	return AlertSystem;
}

void UFarmStatusWidget::UpdateAlertDisplay()
{
	if (!AlertListGrid)
	{
		return;
	}

	// Get alerts for this farm
	TArray<FFarmAlert> Alerts = GetFarmAlerts();

	// Update alert count
	if (AlertCountText)
	{
		int32 UnacknowledgedCount = 0;
		for (const FFarmAlert& Alert : Alerts)
		{
			if (!Alert.bAcknowledged)
			{
				UnacknowledgedCount++;
			}
		}

		if (UnacknowledgedCount > 0)
		{
			AlertCountText->SetText(FText::Format(
				FText::FromString(TEXT("Alerts: {0}")),
				FText::AsNumber(UnacknowledgedCount)
			));
			AlertCountText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.5f, 0.0f)));
		}
		else
		{
			AlertCountText->SetText(FText::FromString(TEXT("No Alerts")));
			AlertCountText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
	}

	// Clear existing alert display
	AlertListGrid->ClearChildren();

	// Sort alerts by priority (Critical > High > Medium > Low)
	Alerts.Sort([](const FFarmAlert& A, const FFarmAlert& B)
	{
		if (A.Priority != B.Priority)
		{
			return (int32)A.Priority > (int32)B.Priority;
		}
		return A.Timestamp > B.Timestamp; // Newer first
	});

	// Display alerts (limited by MaxAlertsDisplayed)
	int32 DisplayCount = FMath::Min(Alerts.Num(), MaxAlertsDisplayed);
	for (int32 i = 0; i < DisplayCount; ++i)
	{
		AddAlertRow(Alerts[i], i);
	}

	// Show panel if there are alerts
	if (AlertPanel)
	{
		AlertPanel->SetVisibility(Alerts.Num() > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

TArray<FFarmAlert> UFarmStatusWidget::GetFarmAlerts() const
{
	TArray<FFarmAlert> Result;

	if (!FarmPlot || !AlertSystem)
	{
		return Result;
	}

	// Get all alerts for this farm plot
	Result = AlertSystem->GetAlertsForActor(FarmPlot);

	return Result;
}

void UFarmStatusWidget::AcknowledgeAlert(int32 AlertID)
{
	if (AlertSystem)
	{
		AlertSystem->AcknowledgeAlert(AlertID);
		UpdateAlertDisplay();
	}
}

void UFarmStatusWidget::DismissAlert(int32 AlertID)
{
	if (AlertSystem)
	{
		AlertSystem->DismissAlert(AlertID);
		UpdateAlertDisplay();
	}
}

void UFarmStatusWidget::AddAlertRow(const FFarmAlert& Alert, int32 Row)
{
	if (!AlertListGrid)
	{
		return;
	}

	int32 Col = 0;

	// Priority indicator (colored box)
	{
		UImage* PriorityImage = NewObject<UImage>(this);
		if (PriorityImage)
		{
			FLinearColor PriorityColor = FLinearColor::White; // Initialize to default
			switch (Alert.Priority)
			{
			case EAlertPriority::Critical:
				PriorityColor = FLinearColor(1.0f, 0.0f, 0.0f);
				break;
			case EAlertPriority::High:
				PriorityColor = FLinearColor(1.0f, 0.5f, 0.0f);
				break;
			case EAlertPriority::Medium:
				PriorityColor = FLinearColor(1.0f, 1.0f, 0.0f);
				break;
			case EAlertPriority::Low:
				PriorityColor = FLinearColor(0.5f, 0.5f, 1.0f);
				break;
			default:
				// Unknown priority - use white as fallback
				PriorityColor = FLinearColor::White;
				break;
			}

			PriorityImage->SetColorAndOpacity(PriorityColor);
			UGridSlot* GridSlot = AlertListGrid->AddChildToGrid(PriorityImage, Row, Col++);
			if (GridSlot)
			{
				GridSlot->SetPadding(FMargin(5.0f, 2.0f));
			}
		}
	}

	// Alert message
	{
		UTextBlock* MessageText = NewObject<UTextBlock>(this);
		if (MessageText)
		{
			MessageText->SetText(FText::FromString(Alert.Message));
			
			// Dim acknowledged alerts
			if (Alert.bAcknowledged)
			{
				MessageText->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)));
			}

			UGridSlot* GridSlot = AlertListGrid->AddChildToGrid(MessageText, Row, Col++);
			if (GridSlot)
			{
				GridSlot->SetPadding(FMargin(5.0f, 2.0f));
			}
		}
	}

	// Time since alert
	{
		UTextBlock* TimeText = NewObject<UTextBlock>(this);
		if (TimeText)
		{
			FTimespan TimeSince = FDateTime::Now() - Alert.Timestamp;
			int32 MinutesAgo = FMath::FloorToInt(TimeSince.GetTotalMinutes());
			
			FString TimeString;
			if (MinutesAgo < 1)
			{
				TimeString = TEXT("Just now");
			}
			else if (MinutesAgo < 60)
			{
				TimeString = FString::Printf(TEXT("%dm ago"), MinutesAgo);
			}
			else
			{
				int32 HoursAgo = MinutesAgo / 60;
				TimeString = FString::Printf(TEXT("%dh ago"), HoursAgo);
			}

			TimeText->SetText(FText::FromString(TimeString));
			TimeText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)));

			UGridSlot* GridSlot = AlertListGrid->AddChildToGrid(TimeText, Row, Col++);
			if (GridSlot)
			{
				GridSlot->SetPadding(FMargin(5.0f, 2.0f));
			}
		}
	}
}

// ============================================================================
// HARVEST TRACKING IMPLEMENTATION (Task 17.4)
// ============================================================================

void UFarmStatusWidget::UpdateHarvestTrackingDisplay()
{
	if (!FarmPlot)
	{
		return;
	}

	// Update total economic value
	float TotalValue = GetTotalFarmValue();
	if (TotalEconomicValueText)
	{
		TotalEconomicValueText->SetText(FormatCurrency(TotalValue));
		
		// Color based on value
		FLinearColor ValueColor = FLinearColor::White;
		if (TotalValue > 10000.0f)
		{
			ValueColor = FLinearColor(0.0f, 1.0f, 0.5f); // Green for high value
		}
		else if (TotalValue > 1000.0f)
		{
			ValueColor = FLinearColor(1.0f, 1.0f, 0.0f); // Yellow for medium value
		}
		
		TotalEconomicValueText->SetColorAndOpacity(FSlateColor(ValueColor));
	}

	// Update total harvests count
	TArray<FHarvestRecord> AllHarvests = FarmPlot->GetHarvestHistory();
	if (TotalHarvestsText)
	{
		TotalHarvestsText->SetText(FText::Format(
			FText::FromString(TEXT("Total Harvests: {0}")),
			FText::AsNumber(AllHarvests.Num())
		));
	}

	// Update harvest statistics grid
	if (HarvestStatisticsGrid)
	{
		HarvestStatisticsGrid->ClearChildren();

		TArray<FHarvestStatistics> Stats = GetHarvestStatisticsForDisplay();

		// Add header row
		int32 Row = 0;
		AddHarvestStatsHeaderRow(Row++);

		// Add data rows
		int32 DisplayCount = FMath::Min(Stats.Num(), MaxHarvestStatsDisplayed);
		for (int32 i = 0; i < DisplayCount; ++i)
		{
			AddHarvestStatsDataRow(Stats[i], Row++);
		}
	}

	// Update recent harvests grid
	if (RecentHarvestsGrid)
	{
		RecentHarvestsGrid->ClearChildren();

		TArray<FHarvestRecord> RecentHarvests = GetRecentHarvests(MaxRecentHarvestsDisplayed);

		// Add header row
		int32 Row = 0;
		AddRecentHarvestsHeaderRow(Row++);

		// Add data rows
		for (int32 i = 0; i < RecentHarvests.Num(); ++i)
		{
			AddRecentHarvestDataRow(RecentHarvests[i], Row++);
		}
	}

	// Show/hide panel based on whether there's data
	if (HarvestTrackingPanel)
	{
		HarvestTrackingPanel->SetVisibility(
			AllHarvests.Num() > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
		);
	}
}

TArray<FHarvestStatistics> UFarmStatusWidget::GetHarvestStatisticsForDisplay() const
{
	if (!FarmPlot)
	{
		return TArray<FHarvestStatistics>();
	}

	return FarmPlot->GetAllHarvestStatistics();
}

TArray<FHarvestRecord> UFarmStatusWidget::GetRecentHarvests(int32 Count) const
{
	TArray<FHarvestRecord> RecentHarvests;

	if (!FarmPlot)
	{
		return RecentHarvests;
	}

	TArray<FHarvestRecord> AllHarvests = FarmPlot->GetHarvestHistory();

	// Sort by harvest time (newest first)
	AllHarvests.Sort([](const FHarvestRecord& A, const FHarvestRecord& B)
	{
		return A.HarvestTime > B.HarvestTime;
	});

	// Take the most recent N harvests
	int32 NumToTake = FMath::Min(Count, AllHarvests.Num());
	for (int32 i = 0; i < NumToTake; ++i)
	{
		RecentHarvests.Add(AllHarvests[i]);
	}

	return RecentHarvests;
}

float UFarmStatusWidget::GetTotalFarmValue() const
{
	if (!FarmPlot)
	{
		return 0.0f;
	}

	return FarmPlot->GetTotalEconomicValue();
}

FText UFarmStatusWidget::FormatCurrency(float Value) const
{
	// Format as currency with thousands separators
	if (Value >= 1000000.0f)
	{
		return FText::Format(
			FText::FromString(TEXT("${0}M")),
			FText::AsNumber(FMath::RoundToInt(Value / 1000000.0f))
		);
	}
	else if (Value >= 1000.0f)
	{
		return FText::Format(
			FText::FromString(TEXT("${0}K")),
			FText::AsNumber(FMath::RoundToInt(Value / 1000.0f))
		);
	}
	else
	{
		return FText::Format(
			FText::FromString(TEXT("${0}")),
			FText::AsNumber(FMath::RoundToInt(Value))
		);
	}
}

void UFarmStatusWidget::AddHarvestStatsHeaderRow(int32 Row)
{
	if (!HarvestStatisticsGrid)
	{
		return;
	}

	TArray<FString> Headers = {
		TEXT("Crop"),
		TEXT("Harvests"),
		TEXT("Avg Yield"),
		TEXT("Avg Quality"),
		TEXT("Total Value"),
		TEXT("Best Yield")
	};

	for (int32 Col = 0; Col < Headers.Num(); ++Col)
	{
		UTextBlock* HeaderText = NewObject<UTextBlock>(this);
		if (HeaderText)
		{
			HeaderText->SetText(FText::FromString(Headers[Col]));
			HeaderText->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f)));
			
			UGridSlot* GridSlot = HarvestStatisticsGrid->AddChildToGrid(HeaderText, Row, Col);
			if (GridSlot)
			{
				GridSlot->SetPadding(FMargin(5.0f, 2.0f));
			}
		}
	}
}

void UFarmStatusWidget::AddHarvestStatsDataRow(const FHarvestStatistics& Stats, int32 Row)
{
	if (!HarvestStatisticsGrid)
	{
		return;
	}

	int32 Col = 0;

	// Crop Name
	{
		UTextBlock* NameText = NewObject<UTextBlock>(this);
		if (NameText)
		{
			NameText->SetText(FText::FromString(Stats.CropName));
			UGridSlot* GridSlot = HarvestStatisticsGrid->AddChildToGrid(NameText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}

	// Total Harvests
	{
		UTextBlock* HarvestsText = NewObject<UTextBlock>(this);
		if (HarvestsText)
		{
			HarvestsText->SetText(FText::AsNumber(Stats.TotalHarvests));
			UGridSlot* GridSlot = HarvestStatisticsGrid->AddChildToGrid(HarvestsText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}

	// Average Yield
	{
		UTextBlock* YieldText = NewObject<UTextBlock>(this);
		if (YieldText)
		{
			YieldText->SetText(FText::Format(
				FText::FromString(TEXT("{0}")),
				FText::AsNumber(FMath::RoundToInt(Stats.AverageYield))
			));
			UGridSlot* GridSlot = HarvestStatisticsGrid->AddChildToGrid(YieldText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}

	// Average Quality
	{
		UTextBlock* QualityText = NewObject<UTextBlock>(this);
		if (QualityText)
		{
			QualityText->SetText(FormatPercentage(Stats.AverageQuality));
			QualityText->SetColorAndOpacity(FSlateColor(GetQualityColor(Stats.AverageQuality)));
			UGridSlot* GridSlot = HarvestStatisticsGrid->AddChildToGrid(QualityText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}

	// Total Economic Value
	{
		UTextBlock* ValueText = NewObject<UTextBlock>(this);
		if (ValueText)
		{
			ValueText->SetText(FormatCurrency(Stats.TotalEconomicValue));
			ValueText->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.5f)));
			UGridSlot* GridSlot = HarvestStatisticsGrid->AddChildToGrid(ValueText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}

	// Best Yield
	{
		UTextBlock* BestText = NewObject<UTextBlock>(this);
		if (BestText)
		{
			BestText->SetText(FText::AsNumber(Stats.BestYield));
			BestText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.8f, 0.0f)));
			UGridSlot* GridSlot = HarvestStatisticsGrid->AddChildToGrid(BestText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}
}

void UFarmStatusWidget::AddRecentHarvestsHeaderRow(int32 Row)
{
	if (!RecentHarvestsGrid)
	{
		return;
	}

	TArray<FString> Headers = {
		TEXT("Time"),
		TEXT("Crop"),
		TEXT("Yield"),
		TEXT("Quality"),
		TEXT("Value")
	};

	for (int32 Col = 0; Col < Headers.Num(); ++Col)
	{
		UTextBlock* HeaderText = NewObject<UTextBlock>(this);
		if (HeaderText)
		{
			HeaderText->SetText(FText::FromString(Headers[Col]));
			HeaderText->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f)));
			
			UGridSlot* GridSlot = RecentHarvestsGrid->AddChildToGrid(HeaderText, Row, Col);
			if (GridSlot)
			{
				GridSlot->SetPadding(FMargin(5.0f, 2.0f));
			}
		}
	}
}

void UFarmStatusWidget::AddRecentHarvestDataRow(const FHarvestRecord& Record, int32 Row)
{
	if (!RecentHarvestsGrid)
	{
		return;
	}

	int32 Col = 0;

	// Time ago
	{
		UTextBlock* TimeText = NewObject<UTextBlock>(this);
		if (TimeText)
		{
			FTimespan TimeSince = FDateTime::Now() - Record.HarvestTime;
			int32 MinutesAgo = FMath::FloorToInt(TimeSince.GetTotalMinutes());
			
			FString TimeString;
			if (MinutesAgo < 1)
			{
				TimeString = TEXT("Just now");
			}
			else if (MinutesAgo < 60)
			{
				TimeString = FString::Printf(TEXT("%dm ago"), MinutesAgo);
			}
			else
			{
				int32 HoursAgo = MinutesAgo / 60;
				if (HoursAgo < 24)
				{
					TimeString = FString::Printf(TEXT("%dh ago"), HoursAgo);
				}
				else
				{
					int32 DaysAgo = HoursAgo / 24;
					TimeString = FString::Printf(TEXT("%dd ago"), DaysAgo);
				}
			}

			TimeText->SetText(FText::FromString(TimeString));
			TimeText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)));
			UGridSlot* GridSlot = RecentHarvestsGrid->AddChildToGrid(TimeText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}

	// Crop Name
	{
		UTextBlock* CropText = NewObject<UTextBlock>(this);
		if (CropText)
		{
			CropText->SetText(FText::FromString(Record.CropName));
			UGridSlot* GridSlot = RecentHarvestsGrid->AddChildToGrid(CropText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}

	// Yield
	{
		UTextBlock* YieldText = NewObject<UTextBlock>(this);
		if (YieldText)
		{
			YieldText->SetText(FText::AsNumber(Record.YieldAmount));
			UGridSlot* GridSlot = RecentHarvestsGrid->AddChildToGrid(YieldText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}

	// Quality
	{
		UTextBlock* QualityText = NewObject<UTextBlock>(this);
		if (QualityText)
		{
			QualityText->SetText(FormatPercentage(Record.Quality));
			QualityText->SetColorAndOpacity(FSlateColor(GetQualityColor(Record.Quality)));
			UGridSlot* GridSlot = RecentHarvestsGrid->AddChildToGrid(QualityText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}

	// Economic Value
	{
		UTextBlock* ValueText = NewObject<UTextBlock>(this);
		if (ValueText)
		{
			ValueText->SetText(FormatCurrency(Record.EconomicValue));
			ValueText->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.5f)));
			UGridSlot* GridSlot = RecentHarvestsGrid->AddChildToGrid(ValueText, Row, Col++);
			if (GridSlot) GridSlot->SetPadding(FMargin(5.0f, 2.0f));
		}
	}
}
