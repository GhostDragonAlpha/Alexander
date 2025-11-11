// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ResourceGatheringSystem.h"
#include "RefiningSystem.generated.h"

// Forward declarations
class ABaseModule;
class UInventoryManager;

// Refinery module types
UENUM(BlueprintType)
enum class ERefineryModuleType : uint8
{
	Smelter UMETA(DisplayName = "Smelter"),
	ChemicalPlant UMETA(DisplayName = "Chemical Plant"),
	EnrichmentFacility UMETA(DisplayName = "Enrichment Facility"),
	Crystallizer UMETA(DisplayName = "Crystallizer"),
	Extractor UMETA(DisplayName = "Extractor"),
	Purifier UMETA(DisplayName = "Purifier")
};

// Refining recipe
USTRUCT(BlueprintType)
struct FRefiningRecipe
{
	GENERATED_BODY()

	// Recipe ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FName RecipeID;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FString DisplayName;

	// Input resources required
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TArray<FResourceQuantity> InputResources;

	// Output resources produced
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TArray<FResourceQuantity> OutputResources;

	// Processing time in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	float ProcessingTime;

	// Energy required per operation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	float EnergyRequired;

	// Required refinery module type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	ERefineryModuleType RequiredModuleType;

	// Minimum module tier required
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 MinimumModuleTier;

	// Byproducts produced (waste, slag, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TArray<FResourceQuantity> Byproducts;

	// Success rate (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	float SuccessRate;

	// Is recipe unlocked?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	bool bIsUnlocked;

	// Recipe description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FString Description;

	FRefiningRecipe()
	{
		ProcessingTime = 10.0f;
		EnergyRequired = 10.0f;
		RequiredModuleType = ERefineryModuleType::Smelter;
		MinimumModuleTier = 1;
		SuccessRate = 1.0f;
		bIsUnlocked = false;
	}
};

// Active refining operation
USTRUCT(BlueprintType)
struct FActiveRefiningOperation
{
	GENERATED_BODY()

	// Operation ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operation")
	FGuid OperationID;

	// Recipe being processed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operation")
	FRefiningRecipe Recipe;

	// Module performing the operation
	UPROPERTY()
	TWeakObjectPtr<ABaseModule> RefineryModule;

	// Start time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operation")
	float StartTime;

	// Is operation active?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operation")
	bool bIsActive;

	// Progress (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operation")
	float Progress;

	FActiveRefiningOperation()
	{
		OperationID = FGuid::NewGuid();
		StartTime = 0.0f;
		bIsActive = false;
		Progress = 0.0f;
	}
};

// Refining operation result
USTRUCT(BlueprintType)
struct FRefiningResult
{
	GENERATED_BODY()

	// Was operation successful?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bSuccess;

	// Resources produced
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FResourceQuantity> OutputResources;

	// Byproducts produced
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FResourceQuantity> Byproducts;

	// Energy consumed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	float EnergyConsumed;

	// Time taken
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	float ProcessingTime;

	// Experience gained
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	float ExperienceGained;

	// Failure reason
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FString FailureReason;

	FRefiningResult()
	{
		bSuccess = false;
		EnergyConsumed = 0.0f;
		ProcessingTime = 0.0f;
		ExperienceGained = 0.0f;
	}
};

// Refinery module information
USTRUCT(BlueprintType)
struct FRefineryModuleInfo
{
	GENERATED_BODY()

	// Module reference
	UPROPERTY()
	TWeakObjectPtr<ABaseModule> Module;

	// Module type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	ERefineryModuleType ModuleType;

	// Module tier (1-5)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	int32 ModuleTier;

	// Is module operational?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	bool bIsOperational;

	// Current operation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	FActiveRefiningOperation CurrentOperation;

	// Processing speed multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	float ProcessingSpeedMultiplier;

	// Energy efficiency multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	float EnergyEfficiencyMultiplier;

	FRefineryModuleInfo()
	{
		ModuleType = ERefineryModuleType::Smelter;
		ModuleTier = 1;
		bIsOperational = false;
		ProcessingSpeedMultiplier = 1.0f;
		EnergyEfficiencyMultiplier = 1.0f;
	}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRefiningRecipeUnlocked, const FRefiningRecipe&, Recipe);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRefiningStarted, const FRefiningRecipe&, Recipe, ABaseModule*, Module);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRefiningCompleted, const FRefiningRecipe&, Recipe, const FRefiningResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRefineryModuleAdded, ABaseModule*, Module, ERefineryModuleType, ModuleType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRefineryModuleRemoved, ABaseModule*, Module);

/**
 * Refining System - Handles resource processing and refining operations
 */
UCLASS()
class ALEXANDER_API URefiningSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	URefiningSystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tick function
	virtual void Tick(float DeltaTime);
	virtual bool IsTickable() const { return true; }

	// ============================================================================
	// REFINING RECIPES
	// ============================================================================

	// Register a refining recipe
	UFUNCTION(BlueprintCallable, Category = "Refining")
	void RegisterRecipe(const FRefiningRecipe& Recipe);

	// Get recipe by ID
	UFUNCTION(BlueprintCallable, Category = "Refining")
	FRefiningRecipe GetRecipe(FName RecipeID) const;

	// Get all recipes
	UFUNCTION(BlueprintCallable, Category = "Refining")
	TArray<FRefiningRecipe> GetAllRecipes() const;

	// Get recipes by module type
	UFUNCTION(BlueprintCallable, Category = "Refining")
	TArray<FRefiningRecipe> GetRecipesByModuleType(ERefineryModuleType ModuleType) const;

	// Get unlocked recipes
	UFUNCTION(BlueprintCallable, Category = "Refining")
	TArray<FRefiningRecipe> GetUnlockedRecipes() const;

	// Unlock recipe
	UFUNCTION(BlueprintCallable, Category = "Refining")
	bool UnlockRecipe(FName RecipeID);

	// ============================================================================
	// REFINING OPERATIONS
	// ============================================================================

	// Start refining operation
	UFUNCTION(BlueprintCallable, Category = "Refining")
	bool StartRefining(FName RecipeID, ABaseModule* Module);

	// Cancel refining operation
	UFUNCTION(BlueprintCallable, Category = "Refining")
	void CancelRefining(ABaseModule* Module);

	// Is module currently refining?
	UFUNCTION(BlueprintCallable, Category = "Refining")
	bool IsModuleRefining(ABaseModule* Module) const;

	// Get refining progress (0-1)
	UFUNCTION(BlueprintCallable, Category = "Refining")
	float GetRefiningProgress(ABaseModule* Module) const;

	// ============================================================================
	// REFINERY MODULE MANAGEMENT
	// ============================================================================

	// Register refinery module
	UFUNCTION(BlueprintCallable, Category = "Refinery Modules")
	void RegisterRefineryModule(ABaseModule* Module, ERefineryModuleType ModuleType, int32 Tier);

	// Unregister refinery module
	UFUNCTION(BlueprintCallable, Category = "Refinery Modules")
	void UnregisterRefineryModule(ABaseModule* Module);

	// Get refinery module info
	UFUNCTION(BlueprintCallable, Category = "Refinery Modules")
	FRefineryModuleInfo GetRefineryModuleInfo(ABaseModule* Module) const;

	// Get all refinery modules
	UFUNCTION(BlueprintCallable, Category = "Refinery Modules")
	TArray<FRefineryModuleInfo> GetAllRefineryModules() const;

	// Get modules by type
	UFUNCTION(BlueprintCallable, Category = "Refinery Modules")
	TArray<FRefineryModuleInfo> GetModulesByType(ERefineryModuleType ModuleType) const;

	// Update module operational status
	UFUNCTION(BlueprintCallable, Category = "Refinery Modules")
	void SetModuleOperational(ABaseModule* Module, bool bIsOperational);

	// ============================================================================
	// RESOURCE PROCESSING
	// ============================================================================

	// Process resources using a recipe
	UFUNCTION(BlueprintCallable, Category = "Processing")
	FRefiningResult ProcessResources(const FRefiningRecipe& Recipe, ABaseModule* Module);

	// Can process recipe? (checks resources, module type, tier)
	UFUNCTION(BlueprintCallable, Category = "Processing")
	bool CanProcessRecipe(const FRefiningRecipe& Recipe, ABaseModule* Module) const;

	// Calculate processing time
	UFUNCTION(BlueprintCallable, Category = "Processing")
	float CalculateProcessingTime(const FRefiningRecipe& Recipe, ABaseModule* Module) const;

	// Calculate energy required
	UFUNCTION(BlueprintCallable, Category = "Processing")
	float CalculateEnergyRequired(const FRefiningRecipe& Recipe, ABaseModule* Module) const;

	// ============================================================================
	// DEFAULT RECIPES
	// ============================================================================

	// Initialize default refining recipes
	UFUNCTION(BlueprintCallable, Category = "Recipes")
	void InitializeDefaultRecipes();

	// Load recipes from data assets
	UFUNCTION(BlueprintCallable, Category = "Recipes")
	void LoadRecipesFromDataAssets();

	// ============================================================================
	// STATISTICS
	// ============================================================================

	// Get total refining operations
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	int32 GetTotalRefiningOperations() const { return TotalRefiningOperations; }

	// Get total resources processed
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	TMap<FName, int32> GetTotalResourcesProcessed() const { return TotalResourcesProcessed; }

	// Get total energy consumed
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	float GetTotalEnergyConsumed() const { return TotalEnergyConsumed; }

	// Reset statistics
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void ResetStatistics();

	// ============================================================================
	// EVENTS
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Refining Events")
	FOnRefiningRecipeUnlocked OnRecipeUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Refining Events")
	FOnRefiningStarted OnRefiningStarted;

	UPROPERTY(BlueprintAssignable, Category = "Refining Events")
	FOnRefiningCompleted OnRefiningCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Refining Events")
	FOnRefineryModuleAdded OnRefineryModuleAdded;

	UPROPERTY(BlueprintAssignable, Category = "Refining Events")
	FOnRefineryModuleRemoved OnRefineryModuleRemoved;

protected:
	// Resource gathering system reference
	UPROPERTY()
	TWeakObjectPtr<UResourceGatheringSystem> ResourceGatheringSystem;

	// Inventory manager reference
	UPROPERTY()
	TWeakObjectPtr<UInventoryManager> InventoryManager;

	// Refining recipes database
	UPROPERTY()
	TMap<FName, FRefiningRecipe> Recipes;

	// Refinery modules
	UPROPERTY()
	TMap<ABaseModule*, FRefineryModuleInfo> RefineryModules;

	// Active refining operations
	UPROPERTY()
	TArray<FActiveRefiningOperation> ActiveOperations;

	// Statistics
	UPROPERTY()
	int32 TotalRefiningOperations;

	UPROPERTY()
	TMap<FName, int32> TotalResourcesProcessed;

	UPROPERTY()
	float TotalEnergyConsumed;

	// Refining skill level (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Refining")
	float RefiningSkillLevel;

private:
	// Helper functions
	void InitializeMetallicRecipes();
	void InitializeCrystallineRecipes();
	void InitializeOrganicRecipes();
	void InitializeRadioactiveRecipes();
	void InitializeAdvancedRecipes();
	FRefiningResult CompleteRefiningOperation(FActiveRefiningOperation& Operation);
	bool HasRequiredResources(const FRefiningRecipe& Recipe) const;
	void ConsumeResources(const FRefiningRecipe& Recipe);
	void ProduceOutputs(const FRefiningRecipe& Recipe, FRefiningResult& Result);
	float CalculateSuccessChance(const FRefiningRecipe& Recipe, ABaseModule* Module) const;
};