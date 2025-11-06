// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TestLevelGameMode.h"
#include "NavigationKioskWidget.generated.h"

// Forward declarations
class ATestLevelGameMode;
class UTextBlock;
class UVerticalBox;
class UScrollBox;
class UButton;
class UEditableText;
class UImage;
class UCanvasPanel;

/**
 * Station display mode for navigation kiosk
 */
UENUM(BlueprintType)
enum class EStationDisplayMode : uint8
{
	List UMETA(DisplayName = "List View"),
	Grid UMETA(DisplayName = "Grid View"),
	Map UMETA(DisplayName = "Map View")
};

/**
 * Station status for visual indicators
 */
UENUM(BlueprintType)
enum class EStationStatus : uint8
{
	NotStarted UMETA(DisplayName = "Not Started"),
	InProgress UMETA(DisplayName = "In Progress"),
	Completed UMETA(DisplayName = "Completed"),
	Failed UMETA(DisplayName = "Failed")
};

/**
 * UMG Widget for test station navigation and teleportation
 *
 * Features:
 * - Display all test stations with names and descriptions
 * - Teleport buttons for each station
 * - Visual indicators for station status (completed/running/pending)
 * - Search/filter functionality
 * - Return to hub button
 * - VR laser pointer interaction support
 * - Optional minimap display
 *
 * Usage:
 * 1. Create UMG Blueprint inheriting from this class
 * 2. Bind widget components using matching names (see BindWidget properties)
 * 3. Customize appearance in UMG Designer
 * 4. Place in level as 3D widget or screen overlay
 */
UCLASS()
class ALEXANDER_API UNavigationKioskWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UNavigationKioskWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

public:
	// Navigation controls
	UFUNCTION(BlueprintCallable, Category = "Navigation Kiosk")
	void TeleportToStation(const FString& StationName);

	UFUNCTION(BlueprintCallable, Category = "Navigation Kiosk")
	void TeleportToHub();

	UFUNCTION(BlueprintCallable, Category = "Navigation Kiosk")
	void RefreshStationList();

	// Display mode
	UFUNCTION(BlueprintCallable, Category = "Navigation Kiosk")
	void SetDisplayMode(EStationDisplayMode Mode);

	UFUNCTION(BlueprintPure, Category = "Navigation Kiosk")
	EStationDisplayMode GetDisplayMode() const { return CurrentDisplayMode; }

	// Search/filter
	UFUNCTION(BlueprintCallable, Category = "Navigation Kiosk")
	void SetSearchFilter(const FString& Filter);

	UFUNCTION(BlueprintCallable, Category = "Navigation Kiosk")
	void ClearSearchFilter();

	UFUNCTION(BlueprintCallable, Category = "Navigation Kiosk")
	void FilterByStatus(EStationStatus Status);

	UFUNCTION(BlueprintCallable, Category = "Navigation Kiosk")
	void ClearStatusFilter();

	// Station information
	UFUNCTION(BlueprintPure, Category = "Navigation Kiosk")
	TArray<FTestStationLocation> GetFilteredStations() const;

	UFUNCTION(BlueprintPure, Category = "Navigation Kiosk")
	EStationStatus GetStationStatus(const FString& StationName) const;

	UFUNCTION(BlueprintPure, Category = "Navigation Kiosk")
	FString GetStationDescription(const FString& StationName) const;

	UFUNCTION(BlueprintPure, Category = "Navigation Kiosk")
	int32 GetStationTestCount(const FString& StationName) const;

	UFUNCTION(BlueprintPure, Category = "Navigation Kiosk")
	float GetStationProgress(const FString& StationName) const;

	// Blueprint implementable events
	UFUNCTION(BlueprintImplementableEvent, Category = "Navigation Kiosk|Events")
	void OnStationTeleport(const FString& StationName, bool bSuccess);

	UFUNCTION(BlueprintImplementableEvent, Category = "Navigation Kiosk|Events")
	void OnHubTeleport(bool bSuccess);

	UFUNCTION(BlueprintImplementableEvent, Category = "Navigation Kiosk|Events")
	void OnStationListUpdated(int32 StationCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Navigation Kiosk|Events")
	void OnSearchFilterChanged(const FString& Filter);

protected:
	// Update functions
	void UpdateStationListDisplay();
	void UpdateStationStatusIndicators();
	void UpdateMinimap();

	// Widget creation helpers
	UWidget* CreateStationListItem(const FTestStationLocation& Station);
	UWidget* CreateStationGridItem(const FTestStationLocation& Station);

	// Filter helpers
	bool PassesSearchFilter(const FTestStationLocation& Station) const;
	bool PassesStatusFilter(const FTestStationLocation& Station) const;

	// Status determination
	EStationStatus DetermineStationStatus(const FString& StationName) const;

	// Button callbacks
	UFUNCTION()
	void OnStationButtonClicked(FString StationName);

	UFUNCTION()
	void OnHubButtonClicked();

	UFUNCTION()
	void OnListViewClicked();

	UFUNCTION()
	void OnGridViewClicked();

	UFUNCTION()
	void OnMapViewClicked();

	UFUNCTION()
	void OnSearchTextChanged(const FText& Text);

	UFUNCTION()
	void OnClearSearchClicked();

	UFUNCTION()
	void OnRefreshClicked();

protected:
	// Widget bindings - Must match names in UMG Designer
	// Header section
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TitleText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StationCountText;

	// Station list section
	UPROPERTY(meta = (BindWidget))
	UScrollBox* StationListScrollBox;

	// Search/filter section
	UPROPERTY(meta = (BindWidget))
	UEditableText* SearchTextBox;

	UPROPERTY(meta = (BindWidget))
	UButton* ClearSearchButton;

	// Display mode buttons
	UPROPERTY(meta = (BindWidget))
	UButton* ListViewButton;

	UPROPERTY(meta = (BindWidget))
	UButton* GridViewButton;

	UPROPERTY(meta = (BindWidget))
	UButton* MapViewButton;

	// Navigation buttons
	UPROPERTY(meta = (BindWidget))
	UButton* ReturnToHubButton;

	UPROPERTY(meta = (BindWidget))
	UButton* RefreshButton;

	// Minimap (optional)
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* MinimapCanvas;

	UPROPERTY(meta = (BindWidget))
	UImage* MinimapBackground;

	// Status section
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentLocationText;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Kiosk|Config")
	bool bShowMinimap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Kiosk|Config")
	bool bAutoUpdateStatus = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Kiosk|Config")
	float StatusUpdateInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Kiosk|Config")
	bool bVROptimizedLayout = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Kiosk|Config")
	float VRButtonScale = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Kiosk|Config")
	bool bShowStationProgress = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Kiosk|Config")
	bool bShowStationDescriptions = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Kiosk|Config")
	EStationDisplayMode DefaultDisplayMode = EStationDisplayMode::List;

	// Visual styling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Kiosk|Styling")
	FLinearColor NotStartedColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // Gray

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Kiosk|Styling")
	FLinearColor InProgressColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Kiosk|Styling")
	FLinearColor CompletedColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Kiosk|Styling")
	FLinearColor FailedColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Kiosk|Styling")
	FLinearColor CurrentStationColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f); // Blue

private:
	// Internal state
	UPROPERTY()
	ATestLevelGameMode* GameModeRef = nullptr;

	EStationDisplayMode CurrentDisplayMode;
	FString CurrentSearchFilter;
	bool bHasStatusFilter = false;
	EStationStatus StatusFilter;

	TArray<FTestStationLocation> CachedStations;
	FString CurrentStation;

	float TimeSinceLastStatusUpdate = 0.0f;

	// VR interaction support
	bool bIsVRMode = false;
};
