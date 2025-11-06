// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TestDataStructures.h"
#include "TestResultsWidget.generated.h"

// Forward declarations
class ATestLevelGameMode;
class UTextBlock;
class UVerticalBox;
class UScrollBox;
class UProgressBar;
class UButton;
class UBorder;

/**
 * UMG Widget for displaying comprehensive test results
 *
 * Features:
 * - Real-time test results display with color-coded status
 * - Overall pass/fail statistics and pass rate
 * - Expandable station list showing individual test cases
 * - VR-friendly layout with large text and clear colors
 * - Auto-refresh during test execution
 * - Export functionality (JSON/CSV)
 *
 * Usage:
 * 1. Create UMG Blueprint inheriting from this class
 * 2. Bind widget components using matching names (see BindWidget properties)
 * 3. Customize appearance in UMG Designer
 * 4. Place in level or show via Blueprint
 */
UCLASS()
class ALEXANDER_API UTestResultsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTestResultsWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

public:
	// Manual update controls
	UFUNCTION(BlueprintCallable, Category = "Test Results")
	void RefreshResults();

	UFUNCTION(BlueprintCallable, Category = "Test Results")
	void SetAutoRefresh(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Test Results")
	void ClearResults();

	// Station expansion/collapse
	UFUNCTION(BlueprintCallable, Category = "Test Results")
	void ExpandStation(const FString& StationName);

	UFUNCTION(BlueprintCallable, Category = "Test Results")
	void CollapseStation(const FString& StationName);

	UFUNCTION(BlueprintCallable, Category = "Test Results")
	void ExpandAllStations();

	UFUNCTION(BlueprintCallable, Category = "Test Results")
	void CollapseAllStations();

	// Export functionality
	UFUNCTION(BlueprintCallable, Category = "Test Results|Export")
	void ExportToJSON();

	UFUNCTION(BlueprintCallable, Category = "Test Results|Export")
	void ExportToCSV();

	UFUNCTION(BlueprintCallable, Category = "Test Results|Export")
	bool SaveResultsToFile(const FString& FilePath, bool bIsJSON = true);

	// Color coding helpers
	UFUNCTION(BlueprintPure, Category = "Test Results|Styling")
	FSlateColor GetStatusColor(ETestStatus Status) const;

	UFUNCTION(BlueprintPure, Category = "Test Results|Styling")
	FLinearColor GetStatusLinearColor(ETestStatus Status) const;

	UFUNCTION(BlueprintPure, Category = "Test Results|Styling")
	FString GetStatusText(ETestStatus Status) const;

	// Blueprint implementable events
	UFUNCTION(BlueprintImplementableEvent, Category = "Test Results|Events")
	void OnResultsUpdated(const FTestResults& Results);

	UFUNCTION(BlueprintImplementableEvent, Category = "Test Results|Events")
	void OnTestsCompleted(bool bAllPassed);

	UFUNCTION(BlueprintImplementableEvent, Category = "Test Results|Events")
	void OnExportCompleted(const FString& FilePath, bool bSuccess);

protected:
	// Update functions
	void UpdateOverallStatistics(const FTestResults& Results);
	void UpdateStationList(const FTestResults& Results);
	void UpdateIndividualStation(const FTestStationResults& StationResults, int32 StationIndex);

	// Widget creation helpers
	UWidget* CreateStationWidget(const FTestStationResults& StationResults, int32 StationIndex);
	UWidget* CreateTestCaseWidget(const FTestCaseResult& TestCase);

	// Button callbacks
	UFUNCTION()
	void OnExportJSONClicked();

	UFUNCTION()
	void OnExportCSVClicked();

	UFUNCTION()
	void OnRefreshClicked();

	UFUNCTION()
	void OnExpandAllClicked();

	UFUNCTION()
	void OnCollapseAllClicked();

protected:
	// Widget bindings - Must match names in UMG Designer
	// Overall statistics section
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalTestsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PassedTestsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* FailedTestsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PassRateText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* OverallProgressBar;

	// Station list section
	UPROPERTY(meta = (BindWidget))
	UScrollBox* StationListScrollBox;

	// Control buttons
	UPROPERTY(meta = (BindWidget))
	UButton* ExportJSONButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ExportCSVButton;

	UPROPERTY(meta = (BindWidget))
	UButton* RefreshButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ExpandAllButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CollapseAllButton;

	// Status indicator
	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatusText;

	UPROPERTY(meta = (BindWidget))
	UBorder* StatusBorder;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Results|Config")
	bool bAutoRefresh = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Results|Config")
	float RefreshInterval = 0.1f; // Refresh every 0.1 seconds when tests running

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Results|Config")
	bool bVROptimizedLayout = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Results|Config")
	float VRTextScale = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Results|Config")
	FString DefaultExportPath = TEXT("Saved/TestResults/");

	// Color scheme for status indication
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Results|Styling")
	FLinearColor PassedColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Results|Styling")
	FLinearColor FailedColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Results|Styling")
	FLinearColor RunningColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Results|Styling")
	FLinearColor NotRunColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // Gray

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Results|Styling")
	FLinearColor SkippedColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f); // Blue

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Results|Styling")
	FLinearColor TimeoutColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange

private:
	// Internal state
	UPROPERTY()
	ATestLevelGameMode* GameModeRef = nullptr;

	FTestResults CachedResults;
	float TimeSinceLastRefresh = 0.0f;
	bool bTestsWereRunning = false;

	// Expanded stations tracking
	TSet<FString> ExpandedStations;

	// Last export path
	FString LastExportPath;
};
