// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataAsset.h"
#include "SystemSelfTestInterface.h"
#include "ResourceGatheringSystem.generated.h"

// Forward declarations
class AAsteroid;
class APlanet;
class UInventoryManager;
class URefiningSystem;
class UCraftingSystem;

// Resource rarity levels
UENUM(BlueprintType)
enum class EResourceRarity : uint8
{
	Common UMETA(DisplayName = "Common"),
	Uncommon UMETA(DisplayName = "Uncommon"),
	Rare UMETA(DisplayName = "Rare"),
	Exotic UMETA(DisplayName = "Exotic"),
	Legendary UMETA(DisplayName = "Legendary")
};

// Resource categories
UENUM(BlueprintType)
enum class EResourceCategory : uint8
{
	Metallic UMETA(DisplayName = "Metallic"),
	Crystalline UMETA(DisplayName = "Crystalline"),
	Organic UMETA(DisplayName = "Organic"),
	Radioactive UMETA(DisplayName = "Radioactive"),
	Gaseous UMETA(DisplayName = "Gaseous"),
	Exotic UMETA(DisplayName = "Exotic"),
	Component UMETA(DisplayName = "Component"),
	Consumable UMETA(DisplayName = "Consumable")
};

// Resource quality levels
UENUM(BlueprintType)
enum class EResourceQuality : uint8
{
	Damaged UMETA(DisplayName = "Damaged"),
	Normal UMETA(DisplayName = "Normal"),
	Pristine UMETA(DisplayName = "Pristine"),
	Legendary UMETA(DisplayName = "Legendary")
};

// Mining difficulty levels
UENUM(BlueprintType)
enum class EMiningDifficulty : uint8
{
	Easy UMETA(DisplayName = "Easy"),
	Medium UMETA(DisplayName = "Medium"),
	Hard UMETA(DisplayName = "Hard"),
	Extreme UMETA(DisplayName = "Extreme")
};

// Deposit types for planetary mining
UENUM(BlueprintType)
enum class EDepositType : uint8
{
	Surface UMETA(DisplayName = "Surface"),
	Underground UMETA(DisplayName = "Underground"),
	Deep UMETA(DisplayName = "Deep"),
	Fracking UMETA(DisplayName = "Fracking")
};

// Resource definition
USTRUCT(BlueprintType)
struct FResourceDefinition
{
	GENERATED_BODY()

	// Resource ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FName ResourceID;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FString DisplayName;

	// Resource category
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EResourceCategory Category;

	// Resource rarity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EResourceRarity Rarity;

	// Base value (credits per unit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float BaseValue;

	// Weight per unit (kg)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float WeightPerUnit;

	// Volume per unit (cubic meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float VolumePerUnit;

	// Icon texture
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|UI")
	UTexture2D* Icon;

	// Description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FString Description;

	FResourceDefinition()
	{
		BaseValue = 1.0f;
		WeightPerUnit = 1.0f;
		VolumePerUnit = 0.001f;
		Category = EResourceCategory::Metallic;
		Rarity = EResourceRarity::Common;
	}
};

// Resource quantity structure
USTRUCT(BlueprintType)
struct FResourceQuantity
{
	GENERATED_BODY()

	// Resource ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FName ResourceID;

	// Quantity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 Quantity;

	// Quality level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EResourceQuality Quality;

	// Constructor
	FResourceQuantity()
	{
		Quantity = 0;
		Quality = EResourceQuality::Normal;
	}

	FResourceQuantity(FName InResourceID, int32 InQuantity, EResourceQuality InQuality = EResourceQuality::Normal)
		: ResourceID(InResourceID), Quantity(InQuantity), Quality(InQuality) {}
};

// Resource deposit data
USTRUCT(BlueprintType)
struct FResourceDeposit
{
	GENERATED_BODY()

	// Resource ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deposit")
	FName ResourceID;

	// Current quantity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deposit")
	int32 CurrentQuantity;

	// Maximum quantity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deposit")
	int32 MaxQuantity;

	// Mining difficulty
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deposit")
	EMiningDifficulty Difficulty;

	// Deposit depth (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deposit")
	float Depth;

	// Deposit type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deposit")
	EDepositType DepositType;

	// Is deposit depleted?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deposit")
	bool bIsDepleted;

	// World location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deposit")
	FVector Location;

	// Quality multiplier (affects extracted resource quality)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deposit")
	float QualityMultiplier;

	FResourceDeposit()
	{
		CurrentQuantity = 0;
		MaxQuantity = 100;
		Difficulty = EMiningDifficulty::Easy;
		Depth = 0.0f;
		DepositType = EDepositType::Surface;
		bIsDepleted = false;
		QualityMultiplier = 1.0f;
	}
};

// Mining result
USTRUCT(BlueprintType)
struct FMiningResult
{
	GENERATED_BODY()

	// Resources extracted
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	TArray<FResourceQuantity> ResourcesExtracted;

	// Was mining successful?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	bool bSuccess;

	// Failure reason (if not successful)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	FString FailureReason;

	// Experience gained
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	float ExperienceGained;

	FMiningResult()
	{
		bSuccess = false;
		ExperienceGained = 0.0f;
	}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResourceMined, const FResourceQuantity&, Resource);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDepositDepleted, const FResourceDeposit&, Deposit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResourceDiscovered, const FResourceDeposit&, Deposit);

/**
 * Resource Gathering System - Core system for managing resource collection
 */
UCLASS()
class ALEXANDER_API UResourceGatheringSystem : public UWorldSubsystem, public ISystemSelfTestInterface
{
	GENERATED_BODY()

public:
	UResourceGatheringSystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tick function
	virtual void Tick(float DeltaTime);
	virtual bool IsTickable() const { return true; }

	//~ Begin ISystemSelfTestInterface interface
	virtual bool RunSelfTest_Implementation(FSystemTestResult& OutResult) override;
	virtual FString GetSystemName_Implementation() const override;
	virtual FString GetTestDescription_Implementation() const override;
	virtual bool IsReadyForTesting_Implementation() const override;
	//~ End ISystemSelfTestInterface interface

	// ============================================================================
	// RESOURCE DEFINITIONS
	// ============================================================================

	// Register a resource definition
	UFUNCTION(BlueprintCallable, Category = "Resources")
	void RegisterResource(const FResourceDefinition& ResourceDef);

	// Get resource definition by ID
	UFUNCTION(BlueprintCallable, Category = "Resources")
	FResourceDefinition GetResourceDefinition(FName ResourceID) const;

	// Get all resource definitions
	UFUNCTION(BlueprintCallable, Category = "Resources")
	TArray<FResourceDefinition> GetAllResourceDefinitions() const;

	// Get resources by category
	UFUNCTION(BlueprintCallable, Category = "Resources")
	TArray<FResourceDefinition> GetResourcesByCategory(EResourceCategory Category) const;

	// Get resources by rarity
	UFUNCTION(BlueprintCallable, Category = "Resources")
	TArray<FResourceDefinition> GetResourcesByRarity(EResourceRarity Rarity) const;

	// ============================================================================
	// ASTEROID RESOURCES
	// ============================================================================

	// Generate resources for an asteroid
	UFUNCTION(BlueprintCallable, Category = "Asteroid Resources")
	void GenerateAsteroidResources(AAsteroid* Asteroid);

	// Get resources from an asteroid
	UFUNCTION(BlueprintCallable, Category = "Asteroid Resources")
	TArray<FResourceDeposit> GetAsteroidResources(AAsteroid* Asteroid) const;

	// Scan asteroid for resources
	UFUNCTION(BlueprintCallable, Category = "Asteroid Resources")
	bool ScanAsteroid(AAsteroid* Asteroid, float ScanStrength, TArray<FResourceDeposit>& OutDeposits);

	// ============================================================================
	// PLANETARY RESOURCES
	// ============================================================================

	// Generate planetary resources for a planet
	UFUNCTION(BlueprintCallable, Category = "Planetary Resources")
	void GeneratePlanetaryResources(APlanet* Planet);

	// Get resources at a planetary location
	UFUNCTION(BlueprintCallable, Category = "Planetary Resources")
	TArray<FResourceDeposit> GetPlanetaryResources(APlanet* Planet, FVector Location) const;

	// Scan planetary location for resources
	UFUNCTION(BlueprintCallable, Category = "Planetary Resources")
	bool ScanPlanetaryLocation(APlanet* Planet, FVector Location, float ScanStrength, TArray<FResourceDeposit>& OutDeposits);

	// ============================================================================
	// MINING OPERATIONS
	// ============================================================================

	// Mine resources from a deposit
	UFUNCTION(BlueprintCallable, Category = "Mining")
	FMiningResult MineDeposit(FResourceDeposit& Deposit, float MiningPower, float MiningEfficiency);

	// Mine asteroid resources
	UFUNCTION(BlueprintCallable, Category = "Mining")
	FMiningResult MineAsteroid(AAsteroid* Asteroid, float MiningPower, float MiningEfficiency);

	// Mine planetary resources
	UFUNCTION(BlueprintCallable, Category = "Mining")
	FMiningResult MinePlanetaryDeposit(APlanet* Planet, FVector Location, float MiningPower, float MiningEfficiency);

	// ============================================================================
	// RESOURCE DATABASE
	// ============================================================================

	// Initialize default resources
	UFUNCTION(BlueprintCallable, Category = "Resources")
	void InitializeDefaultResources();

	// Load resources from data assets
	UFUNCTION(BlueprintCallable, Category = "Resources")
	void LoadResourcesFromDataAssets();

	// ============================================================================
	// STATISTICS
	// ============================================================================

	// Get total resources mined
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	TMap<FName, int32> GetTotalResourcesMined() const { return TotalResourcesMined; }

	// Get mining statistics
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	int32 GetTotalMiningOperations() const { return TotalMiningOperations; }

	// Reset statistics
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void ResetStatistics();

	// ============================================================================
	// EVENTS
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Resource Events")
	FOnResourceMined OnResourceMined;

	UPROPERTY(BlueprintAssignable, Category = "Resource Events")
	FOnDepositDepleted OnDepositDepleted;

	UPROPERTY(BlueprintAssignable, Category = "Resource Events")
	FOnResourceDiscovered OnResourceDiscovered;

protected:
	// Resource definitions database
	UPROPERTY()
	TMap<FName, FResourceDefinition> ResourceDefinitions;

	// Asteroid resource deposits
	UPROPERTY()
	TMap<AAsteroid*, TArray<FResourceDeposit>> AsteroidDeposits;

	// Planetary resource deposits
	UPROPERTY()
	TMap<APlanet*, TMap<FVector, TArray<FResourceDeposit>>> PlanetaryDeposits;

	// Statistics tracking
	UPROPERTY()
	TMap<FName, int32> TotalResourcesMined;

	UPROPERTY()
	int32 TotalMiningOperations;

	// Mining skill multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	float MiningSkillMultiplier;

	// Default resource generation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float CommonResourceChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float UncommonResourceChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float RareResourceChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float ExoticResourceChance;

private:
	// Helper functions
	FResourceDefinition GetRandomResourceForCategory(EResourceCategory Category) const;
	EResourceQuality CalculateResourceQuality(float QualityMultiplier, EMiningDifficulty Difficulty) const;
	void InitializeMetallicResources();
	void InitializeCrystallineResources();
	void InitializeOrganicResources();
	void InitializeRadioactiveResources();
	void InitializeExoticResources();
};