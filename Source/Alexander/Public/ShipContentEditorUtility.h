// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ShipCustomizationComponent.h"
#include "ShipPhysicsConfig.h"
#include "ShipContentEditorUtility.generated.h"

/**
 * Content Import Result
 */
USTRUCT(BlueprintType)
struct FContentImportResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 ItemsImported = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 ItemsFailed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FString> ErrorMessages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FString> WarningMessages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FString> ImportedItems;
};

/**
 * Content Validation Result
 */
USTRUCT(BlueprintType)
struct FContentValidationResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	bool bIsValid = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	int32 TotalItems = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	int32 ValidItems = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	int32 InvalidItems = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	TArray<FString> ValidationErrors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	TArray<FString> ValidationWarnings;
};

/**
 * Ship Content Editor Utility
 *
 * Provides tools for designers to easily create, import, and manage ship customization content.
 * Can be used as a Blueprint base class or directly in C++ automation scripts.
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API UShipContentEditorUtility : public UObject
{
	GENERATED_BODY()

public:
	UShipContentEditorUtility();

	// ============================================================================
	// CONTENT IMPORT FUNCTIONS
	// ============================================================================

	/**
	 * Import ship parts from JSON file
	 *
	 * JSON Format:
	 * {
	 *   "parts": [
	 *     {
	 *       "PartID": "engine_basic_01",
	 *       "DisplayName": "Basic Engine",
	 *       "Description": "Standard propulsion system",
	 *       "Category": "Engine",
	 *       "Rarity": "Common",
	 *       "LevelRequirement": 1,
	 *       "UnlockCost": 0,
	 *       "UnlockedByDefault": true,
	 *       "StatModifiers": {
	 *         "ThrustPower": 1.0,
	 *         "MaxVelocity": 1.0,
	 *         ...
	 *       },
	 *       "PartMeshPath": "/Game/Ships/Meshes/...",
	 *       "IconPath": "/Game/Ships/Icons/...",
	 *       "ParentPartID": "",
	 *       "CanBeUpgraded": true,
	 *       "UpgradeLevel": 0
	 *     }
	 *   ]
	 * }
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Import")
	FContentImportResult ImportPartsFromJSON(const FString& JSONFilePath, UDataTable* TargetDataTable);

	/**
	 * Import ship skins from JSON file
	 *
	 * JSON Format:
	 * {
	 *   "skins": [
	 *     {
	 *       "SkinID": "skin_default",
	 *       "DisplayName": "Default",
	 *       "Description": "Standard ship appearance",
	 *       "Rarity": "Common",
	 *       "SkinMaterialPath": "/Game/Ships/Materials/...",
	 *       "IconPath": "/Game/Ships/Icons/...",
	 *       "UnlockCost": 0,
	 *       "UnlockedByDefault": true
	 *     }
	 *   ]
	 * }
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Import")
	FContentImportResult ImportSkinsFromJSON(const FString& JSONFilePath, UDataTable* TargetDataTable);

	/**
	 * Create a new physics configuration asset from JSON data
	 *
	 * JSON Format:
	 * {
	 *   "ConfigName": "Balanced",
	 *   "ThrustForce": 50000.0,
	 *   "RotationTorque": 20000.0,
	 *   "MaxSpeed": 1000.0,
	 *   "Mass": 10000.0,
	 *   "Drag": 0.1,
	 *   "AngularDrag": 0.05
	 * }
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Import")
	bool CreatePhysicsConfigAsset(const FString& JSONFilePath, const FString& AssetPath);

	// ============================================================================
	// BATCH OPERATIONS
	// ============================================================================

	/**
	 * Import multiple JSON files in batch
	 * Automatically detects file type (parts, skins, configs) and routes to correct import function
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Import")
	TArray<FContentImportResult> BatchImportContent(const TArray<FString>& JSONFilePaths);

	/**
	 * Create all default data tables if they don't exist
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Management")
	bool CreateDefaultDataTables();

	// ============================================================================
	// CONTENT VALIDATION
	// ============================================================================

	/**
	 * Validate all ship parts in data table
	 * Checks for:
	 * - Missing required fields
	 * - Invalid stat values
	 * - Broken asset references
	 * - Duplicate IDs
	 * - Invalid upgrade chains
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Validation")
	FContentValidationResult ValidateAllParts(UDataTable* PartsDataTable);

	/**
	 * Validate all ship skins in data table
	 * Checks for:
	 * - Missing required fields
	 * - Broken material references
	 * - Duplicate IDs
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Validation")
	FContentValidationResult ValidateAllSkins(UDataTable* SkinsDataTable);

	/**
	 * Validate all content (parts + skins + configs)
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Validation")
	FContentValidationResult ValidateAllContent();

	/**
	 * Validate individual ship part
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Validation")
	bool ValidateShipPart(const FShipPartData& PartData, TArray<FString>& OutErrors);

	/**
	 * Validate individual ship skin
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Validation")
	bool ValidateSkin(const FShipSkinData& SkinData, TArray<FString>& OutErrors);

	// ============================================================================
	// CONTENT EXPORT
	// ============================================================================

	/**
	 * Export all parts from data table to JSON
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Export")
	bool ExportPartsToJSON(UDataTable* PartsDataTable, const FString& OutputFilePath);

	/**
	 * Export all skins from data table to JSON
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Export")
	bool ExportSkinsToJSON(UDataTable* SkinsDataTable, const FString& OutputFilePath);

	/**
	 * Export all content to JSON files (parts, skins, configs)
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Export")
	bool ExportContentToJSON(const FString& OutputDirectory);

	// ============================================================================
	// CONTENT CREATION HELPERS
	// ============================================================================

	/**
	 * Create a new ship part with sensible defaults
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Creation")
	FShipPartData CreateNewPart(
		FName PartID,
		const FText& DisplayName,
		EShipPartCategory Category,
		EShipPartRarity Rarity = EShipPartRarity::Common
	);

	/**
	 * Create a new ship skin with sensible defaults
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Creation")
	FShipSkinData CreateNewSkin(
		FName SkinID,
		const FText& DisplayName,
		EShipPartRarity Rarity = EShipPartRarity::Common
	);

	/**
	 * Create balanced stats for a given rarity level
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Creation")
	FShipStats CreateBalancedStats(EShipPartRarity Rarity, EShipPartCategory Category);

	/**
	 * Add part to data table
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Creation")
	bool AddPartToDataTable(const FShipPartData& PartData, UDataTable* TargetDataTable);

	/**
	 * Add skin to data table
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Creation")
	bool AddSkinToDataTable(const FShipSkinData& SkinData, UDataTable* TargetDataTable);

	// ============================================================================
	// CONTENT ORGANIZATION
	// ============================================================================

	/**
	 * Create proper folder structure for ship customization content
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Organization")
	bool CreateContentFolders();

	/**
	 * Move assets to correct folders based on type
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Organization")
	bool OrganizeContentAssets();

	/**
	 * Generate asset naming conventions report
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Organization")
	FString GenerateNamingConventionsReport();

	// ============================================================================
	// DEBUGGING & REPORTING
	// ============================================================================

	/**
	 * Generate content statistics report
	 */
	UFUNCTION(BlueprintCallable, Category = "Reporting")
	FString GenerateContentReport(UDataTable* PartsDataTable, UDataTable* SkinsDataTable);

	/**
	 * Get all parts by category and rarity
	 */
	UFUNCTION(BlueprintCallable, Category = "Querying")
	TArray<FShipPartData> GetPartsByRarity(UDataTable* PartsDataTable, EShipPartRarity Rarity);

	/**
	 * Get all parts by category
	 */
	UFUNCTION(BlueprintCallable, Category = "Querying")
	TArray<FShipPartData> GetPartsByCategory(UDataTable* PartsDataTable, EShipPartCategory Category);

	/**
	 * Check for duplicate part IDs
	 */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	TArray<FName> FindDuplicatePartIDs(UDataTable* PartsDataTable);

	/**
	 * Check for broken asset references
	 */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	TArray<FString> FindBrokenAssetReferences(UDataTable* PartsDataTable);

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Default data table paths
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FString DefaultPartsDataTablePath = "/Game/ShipCustomization/DataTables/DT_ShipParts";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FString DefaultSkinsDataTablePath = "/Game/ShipCustomization/DataTables/DT_ShipSkins";

	// Content folder paths
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FString ContentRootFolder = "/Game/ShipCustomization";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FString DataTablesFolder = "/Game/ShipCustomization/DataTables";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FString DataAssetsFolder = "/Game/ShipCustomization/DataAssets";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FString MaterialsFolder = "/Game/ShipCustomization/Materials";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FString MeshesFolder = "/Game/ShipCustomization/Meshes";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FString IconsFolder = "/Game/ShipCustomization/Icons";

	// Stat balance configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	TMap<EShipPartRarity, float> RarityStatMultipliers = {
		{EShipPartRarity::Common, 1.0f},
		{EShipPartRarity::Uncommon, 1.25f},
		{EShipPartRarity::Rare, 1.5f},
		{EShipPartRarity::Epic, 1.75f},
		{EShipPartRarity::Legendary, 2.0f}
	};

protected:
	// ============================================================================
	// INTERNAL HELPERS
	// ============================================================================

	// Parse JSON string to FShipPartData
	bool ParsePartFromJSON(const TSharedPtr<FJsonObject>& JsonObject, FShipPartData& OutPartData);

	// Parse JSON string to FShipSkinData
	bool ParseSkinFromJSON(const TSharedPtr<FJsonObject>& JsonObject, FShipSkinData& OutSkinData);

	// Convert FShipPartData to JSON
	TSharedPtr<FJsonObject> PartToJSON(const FShipPartData& PartData);

	// Convert FShipSkinData to JSON
	TSharedPtr<FJsonObject> SkinToJSON(const FShipSkinData& SkinData);

	// Load JSON file to string
	bool LoadJSONFile(const FString& FilePath, FString& OutJSONString);

	// Save JSON string to file
	bool SaveJSONFile(const FString& FilePath, const FString& JSONString);

	// Get enum value from string
	template<typename TEnum>
	bool GetEnumValueFromString(const FString& EnumString, TEnum& OutEnumValue);

	// Get string from enum value
	template<typename TEnum>
	FString GetStringFromEnumValue(TEnum EnumValue);

	// Validate asset path exists
	bool ValidateAssetPath(const FString& AssetPath);

	// Create data table asset
	UDataTable* CreateDataTableAsset(const FString& AssetPath, UScriptStruct* RowStruct);

	// Log import result
	void LogImportResult(const FContentImportResult& Result);

	// Log validation result
	void LogValidationResult(const FContentValidationResult& Result);
};
