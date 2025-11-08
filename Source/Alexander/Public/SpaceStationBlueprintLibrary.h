// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SpaceStationHub.h"
#include "SpaceStationBlueprintLibrary.generated.h"

/**
 * Station module type enumeration
 */
UENUM(BlueprintType)
enum class EStationModuleType : uint8
{
	HubCore UMETA(DisplayName = "Hub Core"),
	DockingRing UMETA(DisplayName = "Docking Ring"),
	Habitation UMETA(DisplayName = "Habitation Module"),
	Industrial UMETA(DisplayName = "Industrial Section"),
	TradingPost UMETA(DisplayName = "Trading Post"),
	SocialArea UMETA(DisplayName = "Social Area"),
	MissionBoard UMETA(DisplayName = "Mission Board"),
	DockingBay UMETA(DisplayName = "Docking Bay")
};

/**
 * Station configuration structure
 */
USTRUCT(BlueprintType)
struct FStationConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	FName StationID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	FText StationName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	TArray<EStationModuleType> Modules;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	int32 MaxDockingSlots = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	int32 MaxPlayerCapacity = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	bool bHasTrading = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	bool bHasMissions = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	bool bHasVoiceChat = true;

	FStationConfiguration()
		: StationID(NAME_None)
		, StationName(FText::FromString(TEXT("Unnamed Station")))
		, MaxDockingSlots(10)
		, MaxPlayerCapacity(100)
		, bHasTrading(true)
		, bHasMissions(true)
		, bHasVoiceChat(true)
	{
	}
};

/**
 * Wrapper struct for TArray<UStaticMesh*> to work in TMap
 */
USTRUCT()
struct FStationModuleMeshArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<UStaticMesh*> Meshes;
};

/**
 * Station asset registration structure
 */
USTRUCT(BlueprintType)
struct FStationAssetRegistration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
	UStaticMesh* StationMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
	EStationModuleType ModuleType = EStationModuleType::HubCore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
	int32 Capacity = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
	FVector BoundsSize = FVector(1000.0f, 1000.0f, 500.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
	TArray<FTransform> DockingPoints;
};

/**
 * Blueprint function library for Space Station construction and asset management
 */
UCLASS()
class ALEXANDER_API USpaceStationBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Spawn a modular station from configuration
	 * @param WorldContextObject - World context
	 * @param Config - Station configuration
	 * @param SpawnTransform - Spawn location and rotation
	 * @return Spawned station hub actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Station|Construction", meta = (WorldContext = "WorldContextObject"))
	static ASpaceStationHub* SpawnStationFromConfig(
		UObject* WorldContextObject,
		const FStationConfiguration& Config,
		const FTransform& SpawnTransform
	);

	/**
	 * Register station asset with SpaceAssetManager
	 * @param WorldContextObject - World context
	 * @param Registration - Asset registration data
	 * @return True if registration succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Station|Assets", meta = (WorldContext = "WorldContextObject"))
	static bool RegisterStationAsset(
		UObject* WorldContextObject,
		const FStationAssetRegistration& Registration
	);

	/**
	 * Get all registered station modules of a specific type
	 * @param WorldContextObject - World context
	 * @param ModuleType - Type of module to retrieve
	 * @return Array of registered meshes
	 */
	UFUNCTION(BlueprintCallable, Category = "Station|Assets", meta = (WorldContext = "WorldContextObject"))
	static TArray<UStaticMesh*> GetStationModulesByType(
		UObject* WorldContextObject,
		EStationModuleType ModuleType
	);

	/**
	 * Add module to existing station
	 * @param Station - Target station hub
	 * @param ModuleMesh - Module mesh to add
	 * @param RelativeTransform - Relative transform to station
	 * @param ModuleType - Type of module being added
	 * @return True if module was added successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Station|Construction")
	static bool AddModuleToStation(
		ASpaceStationHub* Station,
		UStaticMesh* ModuleMesh,
		const FTransform& RelativeTransform,
		EStationModuleType ModuleType
	);

	/**
	 * Calculate optimal docking points for a station configuration
	 * @param Config - Station configuration
	 * @param Radius - Radius from center for docking points
	 * @return Array of docking point transforms
	 */
	UFUNCTION(BlueprintCallable, Category = "Station|Construction")
	static TArray<FTransform> CalculateDockingPoints(
		const FStationConfiguration& Config,
		float Radius = 1000.0f
	);

	/**
	 * Validate station configuration
	 * @param Config - Configuration to validate
	 * @param OutErrorMessage - Error message if validation fails
	 * @return True if configuration is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Station|Construction")
	static bool ValidateStationConfiguration(
		const FStationConfiguration& Config,
		FText& OutErrorMessage
	);

	/**
	 * Get estimated station bounds from configuration
	 * @param Config - Station configuration
	 * @return Estimated bounds size
	 */
	UFUNCTION(BlueprintPure, Category = "Station|Construction")
	static FVector GetEstimatedStationBounds(const FStationConfiguration& Config);

	/**
	 * Create default station configuration
	 * @param StationName - Name for the station
	 * @return Default configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Station|Construction")
	static FStationConfiguration CreateDefaultStationConfig(const FText& StationName);

private:
	// Internal helper to get SpaceAssetManager
	static class USpaceAssetManager* GetAssetManager(UObject* WorldContextObject);

	// Module type to asset map
	static TMap<EStationModuleType, FStationModuleMeshArray> RegisteredModules;
};
