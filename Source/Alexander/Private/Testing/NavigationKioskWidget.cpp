// Copyright Epic Games, Inc. All Rights Reserved.

#include "Testing/NavigationKioskWidget.h"
#include "Testing/TestLevelGameMode.h"
#include "Testing/TestStation.h"
#include "TestScenarioManager.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
// #include "HeadMountedDisplayFunctionLibrary.h" // UE5.6: Moved or deprecated

UNavigationKioskWidget::UNavigationKioskWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShowMinimap = true;
	bAutoUpdateStatus = true;
	StatusUpdateInterval = 0.5f;
	bVROptimizedLayout = true;
	VRButtonScale = 1.5f;
	bShowStationProgress = true;
	bShowStationDescriptions = true;
	DefaultDisplayMode = EStationDisplayMode::List;
	CurrentDisplayMode = EStationDisplayMode::List;
	bHasStatusFilter = false;
	bIsVRMode = false;
	TimeSinceLastStatusUpdate = 0.0f;
}

void UNavigationKioskWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get game mode reference
	GameModeRef = Cast<ATestLevelGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GameModeRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("NavigationKioskWidget: Failed to get TestLevelGameMode reference"));
	}

	// Check if VR mode
	// TODO: IsHeadMountedDisplayEnabled() is deprecated in UE5.6 - using stub (false) for tests
	bIsVRMode = false;

	// Bind button events
	if (ReturnToHubButton)
	{
		ReturnToHubButton->OnClicked.AddDynamic(this, &UNavigationKioskWidget::OnHubButtonClicked);
	}

	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UNavigationKioskWidget::OnRefreshClicked);
	}

	if (ListViewButton)
	{
		ListViewButton->OnClicked.AddDynamic(this, &UNavigationKioskWidget::OnListViewClicked);
	}

	if (GridViewButton)
	{
		GridViewButton->OnClicked.AddDynamic(this, &UNavigationKioskWidget::OnGridViewClicked);
	}

	if (MapViewButton)
	{
		MapViewButton->OnClicked.AddDynamic(this, &UNavigationKioskWidget::OnMapViewClicked);
	}

	if (ClearSearchButton)
	{
		ClearSearchButton->OnClicked.AddDynamic(this, &UNavigationKioskWidget::OnClearSearchClicked);
	}

	if (SearchTextBox)
	{
		SearchTextBox->OnTextChanged.AddDynamic(this, &UNavigationKioskWidget::OnSearchTextChanged);
	}

	// Set initial display mode
	SetDisplayMode(DefaultDisplayMode);

	// Initial refresh
	RefreshStationList();

	// Hide minimap if disabled
	if (MinimapCanvas && !bShowMinimap)
	{
		MinimapCanvas->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UNavigationKioskWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bAutoUpdateStatus)
	{
		return;
	}

	TimeSinceLastStatusUpdate += InDeltaTime;
	if (TimeSinceLastStatusUpdate >= StatusUpdateInterval)
	{
		UpdateStationStatusIndicators();
		TimeSinceLastStatusUpdate = 0.0f;
	}
}

void UNavigationKioskWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UNavigationKioskWidget::TeleportToStation(const FString& StationName)
{
	if (!GameModeRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("NavigationKiosk: Cannot teleport - no GameMode reference"));
		OnStationTeleport(StationName, false);
		return;
	}

	bool bSuccess = GameModeRef->TeleportPlayerToStation(StationName);

	if (bSuccess)
	{
		CurrentStation = StationName;
		if (CurrentLocationText)
		{
			CurrentLocationText->SetText(FText::FromString(FString::Printf(TEXT("Current: %s"), *StationName)));
		}
		UE_LOG(LogTemp, Log, TEXT("NavigationKiosk: Teleported to station '%s'"), *StationName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NavigationKiosk: Failed to teleport to station '%s'"), *StationName);
	}

	OnStationTeleport(StationName, bSuccess);
}

void UNavigationKioskWidget::TeleportToHub()
{
	if (!GameModeRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("NavigationKiosk: Cannot teleport - no GameMode reference"));
		OnHubTeleport(false);
		return;
	}

	bool bSuccess = GameModeRef->TeleportPlayerToHub();

	if (bSuccess)
	{
		CurrentStation = TEXT("");
		if (CurrentLocationText)
		{
			CurrentLocationText->SetText(FText::FromString(TEXT("Current: Hub")));
		}
		UE_LOG(LogTemp, Log, TEXT("NavigationKiosk: Teleported to hub"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NavigationKiosk: Failed to teleport to hub"));
	}

	OnHubTeleport(bSuccess);
}

void UNavigationKioskWidget::RefreshStationList()
{
	if (!GameModeRef)
	{
		return;
	}

	CachedStations = GameModeRef->GetAllStations();
	UpdateStationListDisplay();
	UpdateStationStatusIndicators();

	if (StationCountText)
	{
		int32 FilteredCount = GetFilteredStations().Num();
		int32 TotalCount = CachedStations.Num();
		FString CountText = FString::Printf(TEXT("%d / %d Stations"), FilteredCount, TotalCount);
		StationCountText->SetText(FText::FromString(CountText));
	}

	OnStationListUpdated(CachedStations.Num());
}

void UNavigationKioskWidget::SetDisplayMode(EStationDisplayMode Mode)
{
	CurrentDisplayMode = Mode;
	UpdateStationListDisplay();
}

void UNavigationKioskWidget::SetSearchFilter(const FString& Filter)
{
	CurrentSearchFilter = Filter;
	UpdateStationListDisplay();
	OnSearchFilterChanged(Filter);
}

void UNavigationKioskWidget::ClearSearchFilter()
{
	CurrentSearchFilter = TEXT("");
	if (SearchTextBox)
	{
		SearchTextBox->SetText(FText::GetEmpty());
	}
	UpdateStationListDisplay();
	OnSearchFilterChanged(TEXT(""));
}

void UNavigationKioskWidget::FilterByStatus(EStationStatus Status)
{
	bHasStatusFilter = true;
	StatusFilter = Status;
	UpdateStationListDisplay();
}

void UNavigationKioskWidget::ClearStatusFilter()
{
	bHasStatusFilter = false;
	UpdateStationListDisplay();
}

TArray<FTestStationLocation> UNavigationKioskWidget::GetFilteredStations() const
{
	TArray<FTestStationLocation> FilteredStations;

	for (const FTestStationLocation& Station : CachedStations)
	{
		if (PassesSearchFilter(Station) && PassesStatusFilter(Station))
		{
			FilteredStations.Add(Station);
		}
	}

	return FilteredStations;
}

EStationStatus UNavigationKioskWidget::GetStationStatus(const FString& StationName) const
{
	return DetermineStationStatus(StationName);
}

FString UNavigationKioskWidget::GetStationDescription(const FString& StationName) const
{
	for (const FTestStationLocation& Station : CachedStations)
	{
		if (Station.StationName.Equals(StationName))
		{
			return Station.Description;
		}
	}
	return TEXT("");
}

int32 UNavigationKioskWidget::GetStationTestCount(const FString& StationName) const
{
	if (!GameModeRef)
	{
		return 0;
	}

	FTestResults Results = GameModeRef->GetOverallResults();
	for (const FTestStationResults& StationResults : Results.StationResults)
	{
		if (StationResults.StationName.Equals(StationName))
		{
			return StationResults.GetTotalTests();
		}
	}

	return 0;
}

float UNavigationKioskWidget::GetStationProgress(const FString& StationName) const
{
	if (!GameModeRef)
	{
		return 0.0f;
	}

	FTestResults Results = GameModeRef->GetOverallResults();
	for (const FTestStationResults& StationResults : Results.StationResults)
	{
		if (StationResults.StationName.Equals(StationName))
		{
			return StationResults.GetPassRate();
		}
	}

	return 0.0f;
}

void UNavigationKioskWidget::UpdateStationListDisplay()
{
	if (!StationListScrollBox)
	{
		return;
	}

	// Clear existing widgets
	StationListScrollBox->ClearChildren();

	TArray<FTestStationLocation> FilteredStations = GetFilteredStations();

	// Create widgets based on display mode
	for (const FTestStationLocation& Station : FilteredStations)
	{
		UWidget* StationWidget = nullptr;

		switch (CurrentDisplayMode)
		{
		case EStationDisplayMode::List:
			StationWidget = CreateStationListItem(Station);
			break;
		case EStationDisplayMode::Grid:
			StationWidget = CreateStationGridItem(Station);
			break;
		case EStationDisplayMode::Map:
			// Map view handled separately in minimap
			StationWidget = CreateStationListItem(Station);
			break;
		}

		if (StationWidget)
		{
			StationListScrollBox->AddChild(StationWidget);
		}
	}
}

void UNavigationKioskWidget::UpdateStationStatusIndicators()
{
	// Status indicators are updated within the widget creation
	// This would trigger a refresh if widgets have dynamic status displays
	// For now, we rely on periodic full refreshes
}

void UNavigationKioskWidget::UpdateMinimap()
{
	if (!bShowMinimap || !MinimapCanvas)
	{
		return;
	}

	// Minimap update logic would go here
	// This would plot station locations on the minimap canvas
	// For base implementation, this is left to Blueprint
}

UWidget* UNavigationKioskWidget::CreateStationListItem(const FTestStationLocation& Station)
{
	// Base implementation returns nullptr - Blueprint will create custom widgets
	// Blueprint can bind to GetFilteredStations() and create rich UI elements
	return nullptr;
}

UWidget* UNavigationKioskWidget::CreateStationGridItem(const FTestStationLocation& Station)
{
	// Base implementation returns nullptr - Blueprint will create custom widgets
	return nullptr;
}

bool UNavigationKioskWidget::PassesSearchFilter(const FTestStationLocation& Station) const
{
	if (CurrentSearchFilter.IsEmpty())
	{
		return true;
	}

	// Case-insensitive search in station name and description
	FString SearchLower = CurrentSearchFilter.ToLower();
	return Station.StationName.ToLower().Contains(SearchLower) ||
		   Station.Description.ToLower().Contains(SearchLower);
}

bool UNavigationKioskWidget::PassesStatusFilter(const FTestStationLocation& Station) const
{
	if (!bHasStatusFilter)
	{
		return true;
	}

	EStationStatus Status = DetermineStationStatus(Station.StationName);
	return Status == StatusFilter;
}

EStationStatus UNavigationKioskWidget::DetermineStationStatus(const FString& StationName) const
{
	if (!GameModeRef)
	{
		return EStationStatus::NotStarted;
	}

	FTestResults Results = GameModeRef->GetOverallResults();

	for (const FTestStationResults& StationResults : Results.StationResults)
	{
		if (StationResults.StationName.Equals(StationName))
		{
			// Check if any tests are running
			for (const FTestCaseResult& TestCase : StationResults.TestCases)
			{
				if (TestCase.Status == EAlexanderTestStatus::Running)
				{
					return EStationStatus::InProgress;
				}
			}

			// Check if all tests passed
			if (StationResults.AllTestsPassed() && StationResults.GetTotalTests() > 0)
			{
				return EStationStatus::Completed;
			}

			// Check if any tests failed
			if (StationResults.GetFailedTests() > 0)
			{
				return EStationStatus::Failed;
			}

			// Tests registered but not run
			if (StationResults.GetTotalTests() > 0)
			{
				return EStationStatus::NotStarted;
			}
		}
	}

	return EStationStatus::NotStarted;
}

void UNavigationKioskWidget::OnStationButtonClicked(FString StationName)
{
	TeleportToStation(StationName);
}

void UNavigationKioskWidget::OnHubButtonClicked()
{
	TeleportToHub();
}

void UNavigationKioskWidget::OnListViewClicked()
{
	SetDisplayMode(EStationDisplayMode::List);
}

void UNavigationKioskWidget::OnGridViewClicked()
{
	SetDisplayMode(EStationDisplayMode::Grid);
}

void UNavigationKioskWidget::OnMapViewClicked()
{
	SetDisplayMode(EStationDisplayMode::Map);
}

void UNavigationKioskWidget::OnSearchTextChanged(const FText& Text)
{
	SetSearchFilter(Text.ToString());
}

void UNavigationKioskWidget::OnClearSearchClicked()
{
	ClearSearchFilter();
}

void UNavigationKioskWidget::OnRefreshClicked()
{
	RefreshStationList();
}
