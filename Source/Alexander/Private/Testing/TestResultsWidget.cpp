// Copyright Epic Games, Inc. All Rights Reserved.

#include "Testing/TestResultsWidget.h"
#include "Testing/TestLevelGameMode.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/ScrollBox.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

UTestResultsWidget::UTestResultsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoRefresh = true;
	RefreshInterval = 0.1f;
	bVROptimizedLayout = true;
	VRTextScale = 1.5f;
	DefaultExportPath = TEXT("Saved/TestResults/");
	TimeSinceLastRefresh = 0.0f;
	bTestsWereRunning = false;
}

void UTestResultsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get game mode reference
	GameModeRef = Cast<ATestLevelGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GameModeRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("TestResultsWidget: Failed to get TestLevelGameMode reference"));
	}

	// Bind button events
	if (ExportJSONButton)
	{
		ExportJSONButton->OnClicked.AddDynamic(this, &UTestResultsWidget::OnExportJSONClicked);
	}

	if (ExportCSVButton)
	{
		ExportCSVButton->OnClicked.AddDynamic(this, &UTestResultsWidget::OnExportCSVClicked);
	}

	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UTestResultsWidget::OnRefreshClicked);
	}

	if (ExpandAllButton)
	{
		ExpandAllButton->OnClicked.AddDynamic(this, &UTestResultsWidget::OnExpandAllClicked);
	}

	if (CollapseAllButton)
	{
		CollapseAllButton->OnClicked.AddDynamic(this, &UTestResultsWidget::OnCollapseAllClicked);
	}

	// Initial refresh
	RefreshResults();
}

void UTestResultsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bAutoRefresh || !GameModeRef)
	{
		return;
	}

	TimeSinceLastRefresh += InDeltaTime;

	// Refresh at specified interval, or immediately when tests complete
	bool bTestsRunning = GameModeRef->AreTestsRunning();
	bool bTestsJustCompleted = bTestsWereRunning && !bTestsRunning;

	if (TimeSinceLastRefresh >= RefreshInterval || bTestsJustCompleted)
	{
		RefreshResults();
		TimeSinceLastRefresh = 0.0f;

		if (bTestsJustCompleted)
		{
			FTestResults Results = GameModeRef->GetOverallResults();
			OnTestsCompleted(Results.AllTestsPassed());
		}
	}

	bTestsWereRunning = bTestsRunning;
}

void UTestResultsWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UTestResultsWidget::RefreshResults()
{
	if (!GameModeRef)
	{
		return;
	}

	FTestResults Results = GameModeRef->GetOverallResults();
	CachedResults = Results;

	UpdateOverallStatistics(Results);
	UpdateStationList(Results);

	// Update status text
	if (StatusText)
	{
		if (GameModeRef->AreTestsRunning())
		{
			StatusText->SetText(FText::FromString(TEXT("Tests Running...")));
			if (StatusBorder)
			{
				StatusBorder->SetBrushColor(RunningColor);
			}
		}
		else if (Results.GetTotalTests() > 0)
		{
			if (Results.AllTestsPassed())
			{
				StatusText->SetText(FText::FromString(TEXT("All Tests Passed!")));
				if (StatusBorder)
				{
					StatusBorder->SetBrushColor(PassedColor);
				}
			}
			else
			{
				StatusText->SetText(FText::FromString(TEXT("Tests Failed")));
				if (StatusBorder)
				{
					StatusBorder->SetBrushColor(FailedColor);
				}
			}
		}
		else
		{
			StatusText->SetText(FText::FromString(TEXT("No Tests Run")));
			if (StatusBorder)
			{
				StatusBorder->SetBrushColor(NotRunColor);
			}
		}
	}

	// Notify Blueprint
	OnResultsUpdated(Results);
}

void UTestResultsWidget::SetAutoRefresh(bool bEnabled)
{
	bAutoRefresh = bEnabled;
}

void UTestResultsWidget::ClearResults()
{
	if (TotalTestsText) TotalTestsText->SetText(FText::FromString(TEXT("0")));
	if (PassedTestsText) PassedTestsText->SetText(FText::FromString(TEXT("0")));
	if (FailedTestsText) FailedTestsText->SetText(FText::FromString(TEXT("0")));
	if (PassRateText) PassRateText->SetText(FText::FromString(TEXT("0.0%")));
	if (OverallProgressBar) OverallProgressBar->SetPercent(0.0f);
	if (StationListScrollBox) StationListScrollBox->ClearChildren();
	if (StatusText) StatusText->SetText(FText::FromString(TEXT("No Tests Run")));

	ExpandedStations.Empty();
}

void UTestResultsWidget::ExpandStation(const FString& StationName)
{
	ExpandedStations.Add(StationName);
	RefreshResults();
}

void UTestResultsWidget::CollapseStation(const FString& StationName)
{
	ExpandedStations.Remove(StationName);
	RefreshResults();
}

void UTestResultsWidget::ExpandAllStations()
{
	for (const FTestStationResults& Station : CachedResults.StationResults)
	{
		ExpandedStations.Add(Station.StationName);
	}
	RefreshResults();
}

void UTestResultsWidget::CollapseAllStations()
{
	ExpandedStations.Empty();
	RefreshResults();
}

void UTestResultsWidget::ExportToJSON()
{
	if (!GameModeRef)
	{
		return;
	}

	// Generate filename with timestamp
	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	FString Filename = FString::Printf(TEXT("TestResults_%s.json"), *Timestamp);
	FString FilePath = FPaths::ProjectDir() / DefaultExportPath / Filename;

	bool bSuccess = SaveResultsToFile(FilePath, true);
	OnExportCompleted(FilePath, bSuccess);
}

void UTestResultsWidget::ExportToCSV()
{
	if (!GameModeRef)
	{
		return;
	}

	// Generate filename with timestamp
	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	FString Filename = FString::Printf(TEXT("TestResults_%s.csv"), *Timestamp);
	FString FilePath = FPaths::ProjectDir() / DefaultExportPath / Filename;

	bool bSuccess = SaveResultsToFile(FilePath, false);
	OnExportCompleted(FilePath, bSuccess);
}

bool UTestResultsWidget::SaveResultsToFile(const FString& FilePath, bool bIsJSON)
{
	if (!GameModeRef)
	{
		UE_LOG(LogTemp, Error, TEXT("TestResultsWidget: Cannot export - no GameMode reference"));
		return false;
	}

	FString Content;
	if (bIsJSON)
	{
		Content = GameModeRef->GetResultsJSON();
	}
	else
	{
		Content = GameModeRef->GetResultsCSV();
	}

	// Ensure directory exists
	FString Directory = FPaths::GetPath(FilePath);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*Directory))
	{
		PlatformFile.CreateDirectoryTree(*Directory);
	}

	// Write file
	bool bSuccess = FFileHelper::SaveStringToFile(Content, *FilePath);
	if (bSuccess)
	{
		LastExportPath = FilePath;
		UE_LOG(LogTemp, Log, TEXT("TestResultsWidget: Exported results to %s"), *FilePath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TestResultsWidget: Failed to export results to %s"), *FilePath);
	}

	return bSuccess;
}

FSlateColor UTestResultsWidget::GetStatusColor(ETestStatus Status) const
{
	return FSlateColor(GetStatusLinearColor(Status));
}

FLinearColor UTestResultsWidget::GetStatusLinearColor(ETestStatus Status) const
{
	switch (Status)
	{
	case ETestStatus::Passed:
		return PassedColor;
	case ETestStatus::Failed:
		return FailedColor;
	case ETestStatus::Running:
		return RunningColor;
	case ETestStatus::Skipped:
		return SkippedColor;
	case ETestStatus::Timeout:
		return TimeoutColor;
	case ETestStatus::NotRun:
	default:
		return NotRunColor;
	}
}

FString UTestResultsWidget::GetStatusText(ETestStatus Status) const
{
	switch (Status)
	{
	case ETestStatus::Passed:
		return TEXT("PASSED");
	case ETestStatus::Failed:
		return TEXT("FAILED");
	case ETestStatus::Running:
		return TEXT("RUNNING");
	case ETestStatus::Skipped:
		return TEXT("SKIPPED");
	case ETestStatus::Timeout:
		return TEXT("TIMEOUT");
	case ETestStatus::NotRun:
	default:
		return TEXT("NOT RUN");
	}
}

void UTestResultsWidget::UpdateOverallStatistics(const FTestResults& Results)
{
	int32 TotalTests = Results.GetTotalTests();
	int32 PassedTests = Results.GetPassedTests();
	int32 FailedTests = Results.GetFailedTests();
	float PassRate = Results.GetOverallPassRate();

	if (TotalTestsText)
	{
		TotalTestsText->SetText(FText::AsNumber(TotalTests));
	}

	if (PassedTestsText)
	{
		PassedTestsText->SetText(FText::AsNumber(PassedTests));
		PassedTestsText->SetColorAndOpacity(FSlateColor(PassedColor));
	}

	if (FailedTestsText)
	{
		FailedTestsText->SetText(FText::AsNumber(FailedTests));
		FailedTestsText->SetColorAndOpacity(FSlateColor(FailedColor));
	}

	if (PassRateText)
	{
		FString PassRateString = FString::Printf(TEXT("%.1f%%"), PassRate);
		PassRateText->SetText(FText::FromString(PassRateString));

		// Color code pass rate
		FLinearColor RateColor = PassRate >= 90.0f ? PassedColor : (PassRate >= 50.0f ? RunningColor : FailedColor);
		PassRateText->SetColorAndOpacity(FSlateColor(RateColor));
	}

	if (OverallProgressBar)
	{
		float Progress = TotalTests > 0 ? PassRate / 100.0f : 0.0f;
		OverallProgressBar->SetPercent(Progress);

		// Color code progress bar
		FLinearColor BarColor = PassRate >= 90.0f ? PassedColor : (PassRate >= 50.0f ? RunningColor : FailedColor);
		OverallProgressBar->SetFillColorAndOpacity(BarColor);
	}
}

void UTestResultsWidget::UpdateStationList(const FTestResults& Results)
{
	if (!StationListScrollBox)
	{
		return;
	}

	// Clear existing widgets
	StationListScrollBox->ClearChildren();

	// Create widget for each station
	for (int32 i = 0; i < Results.StationResults.Num(); ++i)
	{
		const FTestStationResults& StationResults = Results.StationResults[i];
		UWidget* StationWidget = CreateStationWidget(StationResults, i);
		if (StationWidget)
		{
			StationListScrollBox->AddChild(StationWidget);
		}
	}
}

void UTestResultsWidget::UpdateIndividualStation(const FTestStationResults& StationResults, int32 StationIndex)
{
	// This would be called for incremental updates - for now we do full refresh
	RefreshResults();
}

UWidget* UTestResultsWidget::CreateStationWidget(const FTestStationResults& StationResults, int32 StationIndex)
{
	// This is meant to be overridden in Blueprint for custom styling
	// Base implementation would create simple text representation
	// Blueprint can bind to this data and create rich UI

	// For C++ base implementation, return nullptr - Blueprint will handle creation
	// Alternatively, could create basic TextBlock here
	return nullptr;
}

UWidget* UTestResultsWidget::CreateTestCaseWidget(const FTestCaseResult& TestCase)
{
	// This is meant to be overridden in Blueprint for custom styling
	// Base implementation would create simple text representation
	// Blueprint can bind to this data and create rich UI

	return nullptr;
}

void UTestResultsWidget::OnExportJSONClicked()
{
	ExportToJSON();
}

void UTestResultsWidget::OnExportCSVClicked()
{
	ExportToCSV();
}

void UTestResultsWidget::OnRefreshClicked()
{
	RefreshResults();
}

void UTestResultsWidget::OnExpandAllClicked()
{
	ExpandAllStations();
}

void UTestResultsWidget::OnCollapseAllClicked()
{
	CollapseAllStations();
}
