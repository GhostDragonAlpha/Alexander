// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AssetIntegrationTest.generated.h"

// Forward declarations
class USpaceAssetManager;
class APlanet;
class ASpaceship;
class AAsteroid;

/**
 * Asset Integration Test - Demonstrates integration of free assets with existing systems
 * Shows how assets are applied to planets, ships, and other space objects
 */
UCLASS(BlueprintType, Blueprintable)
class ALEXANDER_API UAssetIntegrationTest : public UObject
{
	GENERATED_BODY()

public:
	UAssetIntegrationTest();

protected:
	virtual void BeginDestroy() override;

public:
	// Asset integration tests
	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void RunAssetIntegrationTests();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestPlanetAssetIntegration();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestSpaceshipAssetIntegration();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestAsteroidAssetIntegration();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestAudioAssetIntegration();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestUIAssetIntegration();

	// Asset loading tests
	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestKenneyAssetLoading();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestOpenGameArtAssetLoading();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestProceduralAssetGeneration();

	// Performance tests
	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestAssetStreamingPerformance();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestMemoryUsage();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestAssetOptimization();

	// Asset quality tests
	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestAssetQuality();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestAssetCompatibility();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestAssetLicensing();

	// Integration with existing systems
	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestPlanetSystemIntegration();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestShipSystemIntegration();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestFarmingSystemIntegration();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void TestAISystemIntegration();

	// Demo functions
	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void CreateDemoScene();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void CreateDemoSpaceship();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void CreateDemoPlanet();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void CreateDemoAsteroidField();

	UFUNCTION(BlueprintCallable, Category = "Asset Integration")
	void CreateDemoSpaceStation();

protected:
	// Test results
	UPROPERTY()
	bool bPlanetIntegrationPassed;

	UPROPERTY()
	bool bSpaceshipIntegrationPassed;

	UPROPERTY()
	bool bAsteroidIntegrationPassed;

	UPROPERTY()
	bool bAudioIntegrationPassed;

	UPROPERTY()
	bool bUIIntegrationPassed;

	UPROPERTY()
	bool bKenneyLoadingPassed;

	UPROPERTY()
	bool bOpenGameArtLoadingPassed;

	UPROPERTY()
	bool bProceduralGenerationPassed;

	UPROPERTY()
	bool bStreamingPerformancePassed;

	UPROPERTY()
	bool bMemoryUsagePassed;

	UPROPERTY()
	bool bOptimizationPassed;

	UPROPERTY()
	bool bAssetQualityPassed;

	UPROPERTY()
	bool bCompatibilityPassed;

	UPROPERTY()
	bool bLicensingPassed;

	// Asset manager reference
	UPROPERTY()
	USpaceAssetManager* SpaceAssetManager;

private:
	void LogTestResult(const FString& TestName, bool bPassed);
	void InitializeAssetManager();
	void CleanupTestAssets();
	bool ValidateAsset(UObject* Asset, const FString& AssetName);
};