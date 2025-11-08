// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestLevelCentralHub.generated.h"

// Forward declarations
class UNavigationKioskWidget;
class UTestResultsWidget;
class UWidgetComponent;
class APlayerStart;
class USpotLightComponent;
class UPointLightComponent;
class UAudioComponent;
class USoundBase;

/**
 * Central Hub Actor for the Comprehensive Test Level
 *
 * Contains:
 * - Navigation kiosk for teleporting between test stations
 * - Results display showing test progress and outcomes
 * - Settings panel for test configuration
 * - Spawn point at world origin
 * - Visual elements and signage
 *
 * Usage:
 * 1. Place one instance in the test level at world origin (0,0,0)
 * 2. Configure widget classes and visual elements
 * 3. Test stations will register themselves with the hub
 */
UCLASS()
class ALEXANDER_API ATestLevelCentralHub : public AActor
{
	GENERATED_BODY()

public:
	ATestLevelCentralHub();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	// Hub initialization
	UFUNCTION(BlueprintCallable, Category = "Test Hub")
	void InitializeHub();

	UFUNCTION(BlueprintCallable, Category = "Test Hub")
	void RegisterWithGameMode();

	// Widget management
	UFUNCTION(BlueprintCallable, Category = "Test Hub")
	void ShowNavigationKiosk();

	UFUNCTION(BlueprintCallable, Category = "Test Hub")
	void ShowResultsDisplay();

	UFUNCTION(BlueprintCallable, Category = "Test Hub")
	void ShowSettingsPanel();

	UFUNCTION(BlueprintCallable, Category = "Test Hub")
	void HideAllWidgets();

	// Visual feedback
	UFUNCTION(BlueprintCallable, Category = "Test Hub")
	void UpdateHubStatus();

	UFUNCTION(BlueprintCallable, Category = "Test Hub")
	void PlayWelcomeSequence();

	UFUNCTION(BlueprintCallable, Category = "Test Hub")
	void HighlightActiveStation(const FString& StationName);

	// Teleportation
	UFUNCTION(BlueprintCallable, Category = "Test Hub")
	bool TeleportPlayerToStation(const FString& StationName);

	UFUNCTION(BlueprintCallable, Category = "Test Hub")
	void ReturnPlayerToHub();

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Hub|Config")
	TSubclassOf<UNavigationKioskWidget> NavigationKioskClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Hub|Config")
	TSubclassOf<UTestResultsWidget> ResultsWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Hub|Config")
	TSubclassOf<UUserWidget> SettingsWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Hub|Config")
	bool bAutoShowNavigationOnSpawn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Hub|Config")
	float HubUpdateInterval = 1.0f;

	// Visual components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Hub|Visual")
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Hub|Visual")
	UStaticMeshComponent* KioskMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Hub|Visual")
	UStaticMeshComponent* SignageMesh;

	// Widget components (3D widgets in world)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Hub|Widgets")
	UWidgetComponent* NavigationKioskComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Hub|Widgets")
	UWidgetComponent* ResultsDisplayComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Hub|Widgets")
	UWidgetComponent* SettingsPanelComponent;

	// Lighting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Hub|Lighting")
	USpotLightComponent* KioskLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Hub|Lighting")
	UPointLightComponent* AmbientLight;

	// Audio
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Hub|Audio")
	UAudioComponent* AmbientAudio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Hub|Audio")
	USoundBase* WelcomeSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Hub|Audio")
	USoundBase* TeleportSound;

protected:
	// Internal functions
	void CreateWidgets();
	void SetupLighting();
	void SetupAudio();
	void UpdateWidgetVisibility();
	void HandlePlayerSpawn();

	// Event handlers
	UFUNCTION()
	void OnTestsStarted(const FString& TestName);

	UFUNCTION()
	void OnTestsCompleted(const FTestResults& Results);

	UFUNCTION()
	void OnStationChanged(const FString& StationName);

private:
	// References
	UPROPERTY()
	class ATestLevelGameMode* GameModeRef = nullptr;

	UPROPERTY()
	APlayerStart* HubSpawnPoint = nullptr;

	// Widget instances
	UPROPERTY()
	UNavigationKioskWidget* NavigationKioskWidget = nullptr;

	UPROPERTY()
	UTestResultsWidget* ResultsWidget = nullptr;

	UPROPERTY()
	UUserWidget* SettingsWidget = nullptr;

	// State
	bool bHubInitialized = false;
	float TimeSinceLastUpdate = 0.0f;
	FString CurrentActiveStation;
};
