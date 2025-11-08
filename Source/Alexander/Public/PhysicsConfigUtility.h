// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShipPhysicsConfig.h"
#include "PhysicsConfigUtility.generated.h"

/**
 * Physics Configuration Validation Result
 */
USTRUCT(BlueprintType)
struct FPhysicsConfigValidation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	bool bIsValid = true;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FString> Warnings;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FString> Errors;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	float ThrustToWeightRatio = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	float MaxAcceleration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	float TimeToMaxVelocity = 0.0f;
};

/**
 * Physics Configuration Utility Actor
 *
 * Provides editor and runtime utilities for working with UShipPhysicsConfig data assets.
 * - Create physics configs from JSON data
 * - Validate physics parameters
 * - Calculate derived metrics
 * - Batch import/export configs
 *
 * Usage:
 *   1. Place actor in level for editor access
 *   2. Use Blueprint functions to manage configs
 *   3. Call LoadPhysicsConfigsFromJSON to import presets
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API APhysicsConfigUtility : public AActor
{
	GENERATED_BODY()

public:
	APhysicsConfigUtility();

protected:
	virtual void BeginPlay() override;

public:
	// ============================================================================
	// VALIDATION FUNCTIONS
	// ============================================================================

	/**
	 * Validates a physics configuration and returns detailed analysis
	 * @param Config The physics configuration to validate
	 * @return Validation result with warnings, errors, and metrics
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Config|Validation")
	static FPhysicsConfigValidation ValidatePhysicsConfig(UShipPhysicsConfig* Config);

	/**
	 * Quick validation check - returns true if config has no errors
	 * @param Config The physics configuration to check
	 * @return True if valid, false if has errors
	 */
	UFUNCTION(BlueprintPure, Category = "Physics Config|Validation")
	static bool IsPhysicsConfigValid(UShipPhysicsConfig* Config);

	/**
	 * Calculates thrust-to-weight ratio for a config
	 * Good range: 15-40 for fighters, 5-10 for heavy ships
	 * @param Config The physics configuration
	 * @return Thrust to weight ratio
	 */
	UFUNCTION(BlueprintPure, Category = "Physics Config|Metrics")
	static float CalculateThrustToWeightRatio(UShipPhysicsConfig* Config);

	/**
	 * Calculates maximum acceleration in m/s^2
	 * @param Config The physics configuration
	 * @return Max acceleration
	 */
	UFUNCTION(BlueprintPure, Category = "Physics Config|Metrics")
	static float CalculateMaxAcceleration(UShipPhysicsConfig* Config);

	/**
	 * Estimates time to reach maximum velocity in seconds
	 * @param Config The physics configuration
	 * @return Time to max velocity
	 */
	UFUNCTION(BlueprintPure, Category = "Physics Config|Metrics")
	static float CalculateTimeToMaxVelocity(UShipPhysicsConfig* Config);

	// ============================================================================
	// JSON IMPORT/EXPORT
	// ============================================================================

	/**
	 * Loads physics configurations from JSON file
	 * Creates Data Asset files in the specified output path
	 * @param JSONFilePath Path to PhysicsConfigs.json file
	 * @param OutputPath Content browser path for created assets (e.g., "/Game/Data/PhysicsConfigs/")
	 * @return Number of configs successfully created
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Config|Import")
	static int32 LoadPhysicsConfigsFromJSON(const FString& JSONFilePath, const FString& OutputPath);

	/**
	 * Creates a single physics config data asset
	 * @param AssetName Name for the data asset (e.g., "DA_LightFighter")
	 * @param ConfigName Display name (e.g., "Light Fighter")
	 * @param Description Config description
	 * @param OutputPath Content browser path
	 * @return The created config asset
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Config|Import")
	static UShipPhysicsConfig* CreatePhysicsConfigAsset(
		const FString& AssetName,
		const FString& ConfigName,
		const FString& Description,
		const FString& OutputPath
	);

	/**
	 * Exports physics configuration to JSON format
	 * @param Config The config to export
	 * @return JSON string representation
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Config|Export")
	static FString ExportPhysicsConfigToJSON(UShipPhysicsConfig* Config);

	/**
	 * Exports all physics configs in a directory to JSON file
	 * @param ConfigDirectory Content browser path to search
	 * @param OutputFilePath File path to save JSON
	 * @return Number of configs exported
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Config|Export")
	static int32 ExportAllPhysicsConfigsToJSON(const FString& ConfigDirectory, const FString& OutputFilePath);

	// ============================================================================
	// EDITOR UTILITIES
	// ============================================================================

	/**
	 * Lists all physics config assets in the project
	 * @return Array of found config assets
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Config|Utility")
	static TArray<UShipPhysicsConfig*> GetAllPhysicsConfigs();

	/**
	 * Finds a physics config by name
	 * @param ConfigName Name to search for
	 * @return First matching config, or nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Config|Utility")
	static UShipPhysicsConfig* FindPhysicsConfigByName(const FString& ConfigName);

	/**
	 * Compares two physics configs and returns differences
	 * @param ConfigA First config
	 * @param ConfigB Second config
	 * @return Human-readable comparison
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Config|Utility")
	static FString ComparePhysicsConfigs(UShipPhysicsConfig* ConfigA, UShipPhysicsConfig* ConfigB);

	/**
	 * Generates a performance report for all configs
	 * @return Formatted report string
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Config|Utility")
	static FString GeneratePerformanceReport();

	// ============================================================================
	// TESTING UTILITIES
	// ============================================================================

	/**
	 * Applies a physics config to a test actor for validation
	 * @param TestActor Actor to apply config to (must have physics component)
	 * @param Config Config to test
	 * @return True if successfully applied
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Config|Testing")
	static bool ApplyConfigToTestActor(AActor* TestActor, UShipPhysicsConfig* Config);

	/**
	 * Prints detailed config information to log
	 * @param Config Config to print
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Config|Debug")
	static void PrintPhysicsConfigDetails(UShipPhysicsConfig* Config);

private:
	// Internal validation helpers
	static void ValidateMass(UShipPhysicsConfig* Config, FPhysicsConfigValidation& OutValidation);
	static void ValidateThrust(UShipPhysicsConfig* Config, FPhysicsConfigValidation& OutValidation);
	static void ValidateVelocity(UShipPhysicsConfig* Config, FPhysicsConfigValidation& OutValidation);
	static void ValidateAngularThrust(UShipPhysicsConfig* Config, FPhysicsConfigValidation& OutValidation);
	static void ValidateMultipliers(UShipPhysicsConfig* Config, FPhysicsConfigValidation& OutValidation);
	static void ValidateBalance(UShipPhysicsConfig* Config, FPhysicsConfigValidation& OutValidation);
};
