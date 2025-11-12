// Copyright Epic Games, Inc. All Rights Reserved.

#include "RefiningSystem.h"
#include "BaseBuildingManager.h"
#include "BaseModule.h"
#include "ResourceGatheringSystem.h"
#include "InventoryManager.h"
#include "Engine/World.h"

URefiningSystem::URefiningSystem()
{
	TotalRefiningOperations = 0;
	TotalEnergyConsumed = 0.0f;
	RefiningSkillLevel = 0.0f;
}

void URefiningSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("RefiningSystem initialized"));

	// Get references to other systems
	ResourceGatheringSystem = GetWorld()->GetSubsystem<UResourceGatheringSystem>();
	// InventoryManager will be set later when available

	// Initialize default recipes
	InitializeDefaultRecipes();

	// Reset statistics
	TotalRefiningOperations = 0;
	TotalResourcesProcessed.Empty();
	TotalEnergyConsumed = 0.0f;
}

void URefiningSystem::Deinitialize()
{
	// Clear all data
	Recipes.Empty();
	RefineryModules.Empty();
	ActiveOperations.Empty();

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("RefiningSystem deinitialized"));
}

void URefiningSystem::Tick(float DeltaTime)
{
	// Update active refining operations
	for (int32 i = ActiveOperations.Num() - 1; i >= 0; i--)
	{
		FActiveRefiningOperation& Operation = ActiveOperations[i];

		if (!Operation.bIsActive || !Operation.RefineryModule.IsValid())
		{
			continue;
		}

		float CurrentTime = GetWorld()->GetTimeSeconds();
		float ElapsedTime = CurrentTime - Operation.StartTime;

		// Update progress
		float ModuleSpeed = 1.0f;
		const FRefineryModuleInfo* ModuleInfo = RefineryModules.Find(Operation.RefineryModule.Get());
		if (ModuleInfo)
		{
			ModuleSpeed = ModuleInfo->ProcessingSpeedMultiplier;
		}

		float TotalDuration = CalculateProcessingTime(Operation.Recipe, Operation.RefineryModule.Get());
		Operation.Progress = FMath::Clamp(ElapsedTime / TotalDuration, 0.0f, 1.0f);

		// Check if operation is complete
		if (Operation.Progress >= 1.0f)
		{
			// Complete refining operation
			FRefiningResult Result = CompleteRefiningOperation(Operation);

			// Fire completion event
			OnRefiningCompleted.Broadcast(Operation.Recipe, Result);

			// Remove from active operations
			ActiveOperations.RemoveAt(i);
		}
	}
}

void URefiningSystem::RegisterRecipe(const FRefiningRecipe& Recipe)
{
	if (Recipe.RecipeID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot register recipe with empty ID"));
		return;
	}

	Recipes.Add(Recipe.RecipeID, Recipe);
	UE_LOG(LogTemp, Log, TEXT("Registered refining recipe: %s (%s)"), *Recipe.RecipeID.ToString(), *Recipe.DisplayName);
}

FRefiningRecipe URefiningSystem::GetRecipe(FName RecipeID) const
{
	const FRefiningRecipe* Recipe = Recipes.Find(RecipeID);
	return Recipe ? *Recipe : FRefiningRecipe();
}

TArray<FRefiningRecipe> URefiningSystem::GetAllRecipes() const
{
	TArray<FRefiningRecipe> AllRecipes;
	Recipes.GenerateValueArray(AllRecipes);
	return AllRecipes;
}

TArray<FRefiningRecipe> URefiningSystem::GetRecipesByModuleType(ERefineryModuleType ModuleType) const
{
	TArray<FRefiningRecipe> ModuleRecipes;
	for (const auto& Pair : Recipes)
	{
		if (Pair.Value.RequiredModuleType == ModuleType)
		{
			ModuleRecipes.Add(Pair.Value);
		}
	}
	return ModuleRecipes;
}

TArray<FRefiningRecipe> URefiningSystem::GetUnlockedRecipes() const
{
	TArray<FRefiningRecipe> UnlockedRecipes;
	for (const auto& Pair : Recipes)
	{
		if (Pair.Value.bIsUnlocked)
		{
			UnlockedRecipes.Add(Pair.Value);
		}
	}
	return UnlockedRecipes;
}

bool URefiningSystem::UnlockRecipe(FName RecipeID)
{
	FRefiningRecipe* Recipe = Recipes.Find(RecipeID);
	if (!Recipe)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot unlock recipe: Recipe not found"));
		return false;
	}

	if (Recipe->bIsUnlocked)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot unlock recipe: Already unlocked"));
		return false;
	}

	Recipe->bIsUnlocked = true;

	// Fire unlock event
	OnRecipeUnlocked.Broadcast(*Recipe);

	UE_LOG(LogTemp, Log, TEXT("Unlocked refining recipe: %s"), *RecipeID.ToString());

	return true;
}

bool URefiningSystem::StartRefining(FName RecipeID, ABaseModule* Module)
{
	if (!Module)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start refining: Invalid module"));
		return false;
	}

	if (!ResourceGatheringSystem.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start refining: Resource gathering system not available"));
		return false;
	}

	// Get recipe
	FRefiningRecipe Recipe = GetRecipe(RecipeID);
	if (!Recipe.bIsUnlocked)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start refining: Recipe not unlocked"));
		return false;
	}

	// Check if module can process this recipe
	if (!CanProcessRecipe(Recipe, Module))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start refining: Module cannot process this recipe"));
		return false;
	}

	// Check if module is already refining
	if (IsModuleRefining(Module))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start refining: Module already refining"));
		return false;
	}

	// Check if we have required resources
	if (!HasRequiredResources(Recipe))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start refining: Insufficient resources"));
		return false;
	}

	// Consume input resources
	ConsumeResources(Recipe);

	// Create active operation
	FActiveRefiningOperation Operation;
	Operation.Recipe = Recipe;
	Operation.RefineryModule = Module;
	Operation.StartTime = GetWorld()->GetTimeSeconds();
	Operation.bIsActive = true;
	Operation.Progress = 0.0f;

	ActiveOperations.Add(Operation);

	// Fire started event
	OnRefiningStarted.Broadcast(Recipe, Module);

	UE_LOG(LogTemp, Log, TEXT("Started refining operation: %s in module %s"), 
		   *RecipeID.ToString(), *Module->GetName());

	return true;
}

void URefiningSystem::CancelRefining(ABaseModule* Module)
{
	if (!Module)
	{
		return;
	}

	// Find and remove operation for this module
	for (int32 i = ActiveOperations.Num() - 1; i >= 0; i--)
	{
		if (ActiveOperations[i].RefineryModule.Get() == Module)
		{
			// Return consumed resources (partial refund)
			// In a full implementation, this would return a portion of the input resources
			
			ActiveOperations.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("Cancelled refining operation for module: %s"), *Module->GetName());
			break;
		}
	}
}

bool URefiningSystem::IsModuleRefining(ABaseModule* Module) const
{
	if (!Module)
	{
		return false;
	}

	for (const FActiveRefiningOperation& Operation : ActiveOperations)
	{
		if (Operation.RefineryModule.Get() == Module && Operation.bIsActive)
		{
			return true;
		}
	}

	return false;
}

float URefiningSystem::GetRefiningProgress(ABaseModule* Module) const
{
	if (!Module)
	{
		return 0.0f;
	}

	for (const FActiveRefiningOperation& Operation : ActiveOperations)
	{
		if (Operation.RefineryModule.Get() == Module)
		{
			return Operation.Progress;
		}
	}

	return 0.0f;
}

void URefiningSystem::RegisterRefineryModule(ABaseModule* Module, ERefineryModuleType ModuleType, int32 Tier)
{
	if (!Module)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot register refinery module: Invalid module"));
		return;
	}

	FRefineryModuleInfo ModuleInfo;
	ModuleInfo.Module = Module;
	ModuleInfo.ModuleType = ModuleType;
	ModuleInfo.ModuleTier = Tier;
	ModuleInfo.bIsOperational = true;
	ModuleInfo.ProcessingSpeedMultiplier = 1.0f + (Tier - 1) * 0.2f; // 20% faster per tier
	ModuleInfo.EnergyEfficiencyMultiplier = 1.0f - (Tier - 1) * 0.1f; // 10% more efficient per tier

	RefineryModules.Add(Module, ModuleInfo);

	// Fire module added event
	OnRefineryModuleAdded.Broadcast(Module, ModuleType);

	UE_LOG(LogTemp, Log, TEXT("Registered refinery module: %s (Type: %s, Tier: %d)"), 
		   *Module->GetName(), *UEnum::GetDisplayValueAsText(ModuleType).ToString(), Tier);
}

void URefiningSystem::UnregisterRefineryModule(ABaseModule* Module)
{
	if (!Module)
	{
		return;
	}

	// Cancel any active operation for this module
	CancelRefining(Module);

	// Remove from registry
	RefineryModules.Remove(Module);

	// Fire module removed event
	OnRefineryModuleRemoved.Broadcast(Module);

	UE_LOG(LogTemp, Log, TEXT("Unregistered refinery module: %s"), *Module->GetName());
}

FRefineryModuleInfo URefiningSystem::GetRefineryModuleInfo(ABaseModule* Module) const
{
	const FRefineryModuleInfo* ModuleInfo = RefineryModules.Find(Module);
	return ModuleInfo ? *ModuleInfo : FRefineryModuleInfo();
}

TArray<FRefineryModuleInfo> URefiningSystem::GetAllRefineryModules() const
{
	TArray<FRefineryModuleInfo> AllModules;
	RefineryModules.GenerateValueArray(AllModules);
	return AllModules;
}

TArray<FRefineryModuleInfo> URefiningSystem::GetModulesByType(ERefineryModuleType ModuleType) const
{
	TArray<FRefineryModuleInfo> ModulesByType;
	for (const auto& Pair : RefineryModules)
	{
		if (Pair.Value.ModuleType == ModuleType)
		{
			ModulesByType.Add(Pair.Value);
		}
	}
	return ModulesByType;
}

void URefiningSystem::SetModuleOperational(ABaseModule* Module, bool bIsOperational)
{
	if (!Module)
	{
		return;
	}

	FRefineryModuleInfo* ModuleInfo = RefineryModules.Find(Module);
	if (ModuleInfo)
	{
		ModuleInfo->bIsOperational = bIsOperational;

		// Cancel operation if module becomes non-operational
		if (!bIsOperational)
		{
			CancelRefining(Module);
		}

		UE_LOG(LogTemp, Log, TEXT("Refinery module %s operational status: %s"), 
			   *Module->GetName(), bIsOperational ? TEXT("true") : TEXT("false"));
	}
}

FRefiningResult URefiningSystem::ProcessResources(const FRefiningRecipe& Recipe, ABaseModule* Module)
{
	FRefiningResult Result;
	Result.bSuccess = false;

	if (!Module)
	{
		Result.FailureReason = "Invalid module";
		return Result;
	}

	// Check if we can process this recipe
	if (!CanProcessRecipe(Recipe, Module))
	{
		Result.FailureReason = "Cannot process recipe with this module";
		return Result;
	}

	// Check resources
	if (!HasRequiredResources(Recipe))
	{
		Result.FailureReason = "Insufficient resources";
		return Result;
	}

	// Consume resources
	ConsumeResources(Recipe);

	// Calculate processing parameters
	float ProcessingTime = CalculateProcessingTime(Recipe, Module);
	float EnergyRequired = CalculateEnergyRequired(Recipe, Module);

	// Calculate success chance
	float SuccessChance = CalculateSuccessChance(Recipe, Module);

	if (FMath::FRand() > SuccessChance)
	{
		Result.bSuccess = false;
		Result.FailureReason = "Processing failed";
		Result.ProcessingTime = ProcessingTime;
		Result.EnergyConsumed = EnergyRequired * 0.5f; // Half energy on failure
		Result.ExperienceGained = 1.0f; // Small XP on failure

		// Update statistics
		TotalRefiningOperations++;
		TotalEnergyConsumed += Result.EnergyConsumed;

		return Result;
	}

	// Success - produce outputs
	ProduceOutputs(Recipe, Result);

	Result.bSuccess = true;
	Result.ProcessingTime = ProcessingTime;
	Result.EnergyConsumed = EnergyRequired;
	Result.ExperienceGained = Recipe.OutputResources.Num() * 5.0f; // XP based on number of outputs

	// Update statistics
	TotalRefiningOperations++;
	TotalEnergyConsumed += EnergyRequired;

	for (const FResourceQuantity& Resource : Recipe.InputResources)
	{
		int32& ProcessedAmount = TotalResourcesProcessed.FindOrAdd(Resource.ResourceID);
		ProcessedAmount += Resource.Quantity;
	}

	UE_LOG(LogTemp, Log, TEXT("Completed refining operation: %s (Success: %s)"), 
		   *Recipe.RecipeID.ToString(), Result.bSuccess ? TEXT("true") : TEXT("false"));

	return Result;
}

bool URefiningSystem::CanProcessRecipe(const FRefiningRecipe& Recipe, ABaseModule* Module) const
{
	if (!Module)
	{
		return false;
	}

	// Get module info
	FRefineryModuleInfo ModuleInfo = GetRefineryModuleInfo(Module);
	if (!ModuleInfo.bIsOperational)
	{
		return false;
	}

	// Check module type
	if (Recipe.RequiredModuleType != ModuleInfo.ModuleType)
	{
		return false;
	}

	// Check module tier
	if (Recipe.MinimumModuleTier > ModuleInfo.ModuleTier)
	{
		return false;
	}

	return true;
}

float URefiningSystem::CalculateProcessingTime(const FRefiningRecipe& Recipe, ABaseModule* Module) const
{
	float BaseTime = Recipe.ProcessingTime;

	// Apply module speed multiplier
	FRefineryModuleInfo ModuleInfo = GetRefineryModuleInfo(Module);
	float ModuleSpeed = ModuleInfo.ProcessingSpeedMultiplier;

	// Apply skill bonus
	float SkillMultiplier = 1.0f + (RefiningSkillLevel / 100.0f) * 0.3f;

	return BaseTime / (ModuleSpeed * SkillMultiplier);
}

float URefiningSystem::CalculateEnergyRequired(const FRefiningRecipe& Recipe, ABaseModule* Module) const
{
	float BaseEnergy = Recipe.EnergyRequired;

	// Apply module efficiency multiplier
	FRefineryModuleInfo ModuleInfo = GetRefineryModuleInfo(Module);
	float Efficiency = ModuleInfo.EnergyEfficiencyMultiplier;

	return BaseEnergy * Efficiency;
}

void URefiningSystem::InitializeDefaultRecipes()
{
	// Initialize all recipe categories
	InitializeMetallicRecipes();
	InitializeCrystallineRecipes();
	InitializeOrganicRecipes();
	InitializeRadioactiveRecipes();
	InitializeAdvancedRecipes();

	UE_LOG(LogTemp, Log, TEXT("Initialized default refining recipes"));
}

void URefiningSystem::LoadRecipesFromDataAssets()
{
	// Placeholder for loading from data assets
	UE_LOG(LogTemp, Log, TEXT("LoadRecipesFromDataAssets not yet implemented"));
}

void URefiningSystem::ResetStatistics()
{
	TotalRefiningOperations = 0;
	TotalResourcesProcessed.Empty();
	TotalEnergyConsumed = 0.0f;
	UE_LOG(LogTemp, Log, TEXT("Refining statistics reset"));
}

FRefiningResult URefiningSystem::CompleteRefiningOperation(FActiveRefiningOperation& Operation)
{
	FRefiningResult Result;
	Result.bSuccess = false;

	if (!Operation.RefineryModule.IsValid())
	{
		Result.FailureReason = "Invalid module";
		return Result;
	}

	// Process resources
	Result = ProcessResources(Operation.Recipe, Operation.RefineryModule.Get());

	return Result;
}

bool URefiningSystem::HasRequiredResources(const FRefiningRecipe& Recipe) const
{
	if (!ResourceGatheringSystem.IsValid())
	{
		return false;
	}

	// In a full implementation, this would check the inventory system
	// For now, assume we have the resources
	return true;
}

void URefiningSystem::ConsumeResources(const FRefiningRecipe& Recipe)
{
	// In a full implementation, this would remove resources from inventory
	// For now, this is a placeholder
	UE_LOG(LogTemp, Log, TEXT("Consumed resources for recipe: %s"), *Recipe.RecipeID.ToString());
}

void URefiningSystem::ProduceOutputs(const FRefiningRecipe& Recipe, FRefiningResult& Result)
{
	// Add output resources to result
	Result.OutputResources = Recipe.OutputResources;

	// Add byproducts
	Result.Byproducts = Recipe.Byproducts;

	// In a full implementation, this would add resources to inventory
	UE_LOG(LogTemp, Log, TEXT("Produced outputs for recipe: %s"), *Recipe.RecipeID.ToString());
}

float URefiningSystem::CalculateSuccessChance(const FRefiningRecipe& Recipe, ABaseModule* Module) const
{
	float BaseChance = Recipe.SuccessRate;

	// Apply module tier bonus
	FRefineryModuleInfo ModuleInfo = GetRefineryModuleInfo(Module);
	float TierBonus = (ModuleInfo.ModuleTier - Recipe.MinimumModuleTier) * 0.05f; // 5% bonus per tier above minimum

	// Apply skill bonus
	float SkillBonus = (RefiningSkillLevel / 100.0f) * 0.2f; // Up to 20% bonus from skill

	return FMath::Clamp(BaseChance + TierBonus + SkillBonus, 0.0f, 1.0f);
}

void URefiningSystem::InitializeMetallicRecipes()
{
	// Iron Ore -> Iron Ingots
	FRefiningRecipe IronIngots;
	IronIngots.RecipeID = FName("IronIngots");
	IronIngots.DisplayName = "Smelt Iron Ingots";
	IronIngots.RequiredModuleType = ERefineryModuleType::Smelter;
	IronIngots.MinimumModuleTier = 1;
	IronIngots.ProcessingTime = 5.0f;
	IronIngots.EnergyRequired = 10.0f;
	IronIngots.SuccessRate = 1.0f;
	IronIngots.bIsUnlocked = true;
	IronIngots.Description = "Process iron ore into usable iron ingots.";

	// Input: Iron Ore
	FResourceQuantity IronInput;
	IronInput.ResourceID = FName("Iron");
	IronInput.Quantity = 2;
	IronIngots.InputResources.Add(IronInput);

	// Output: Iron Ingots
	FResourceQuantity IronOutput;
	IronOutput.ResourceID = FName("IronIngots");
	IronOutput.Quantity = 1;
	IronIngots.OutputResources.Add(IronOutput);

	// Byproduct: Slag
	FResourceQuantity SlagByproduct;
	SlagByproduct.ResourceID = FName("Slag");
	SlagByproduct.Quantity = 1;
	IronIngots.Byproducts.Add(SlagByproduct);

	RegisterRecipe(IronIngots);

	// Copper Ore -> Copper Ingots
	FRefiningRecipe CopperIngots;
	CopperIngots.RecipeID = FName("CopperIngots");
	CopperIngots.DisplayName = "Smelt Copper Ingots";
	CopperIngots.RequiredModuleType = ERefineryModuleType::Smelter;
	CopperIngots.MinimumModuleTier = 1;
	CopperIngots.ProcessingTime = 6.0f;
	CopperIngots.EnergyRequired = 12.0f;
	CopperIngots.SuccessRate = 1.0f;
	CopperIngots.bIsUnlocked = true;
	CopperIngots.Description = "Process copper ore into usable copper ingots.";

	FResourceQuantity CopperInput;
	CopperInput.ResourceID = FName("Copper");
	CopperInput.Quantity = 2;
	CopperIngots.InputResources.Add(CopperInput);

	FResourceQuantity CopperOutput;
	CopperOutput.ResourceID = FName("CopperIngots");
	CopperOutput.Quantity = 1;
	CopperIngots.OutputResources.Add(CopperOutput);

	CopperIngots.Byproducts.Add(SlagByproduct);

	RegisterRecipe(CopperIngots);

	// Steel Production (Iron + Coal)
	FRefiningRecipe SteelIngots;
	SteelIngots.RecipeID = FName("SteelIngots");
	SteelIngots.DisplayName = "Produce Steel";
	SteelIngots.RequiredModuleType = ERefineryModuleType::Smelter;
	SteelIngots.MinimumModuleTier = 2;
	SteelIngots.ProcessingTime = 10.0f;
	SteelIngots.EnergyRequired = 25.0f;
	SteelIngots.SuccessRate = 0.95f;
	SteelIngots.bIsUnlocked = false; // Must be unlocked
	SteelIngots.Description = "Combine iron and carbon to produce high-quality steel.";

	FResourceQuantity SteelIronInput;
	SteelIronInput.ResourceID = FName("IronIngots");
	SteelIronInput.Quantity = 2;
	SteelIngots.InputResources.Add(SteelIronInput);

	FResourceQuantity CoalInput;
	CoalInput.ResourceID = FName("Coal");
	CoalInput.Quantity = 1;
	SteelIngots.InputResources.Add(CoalInput);

	FResourceQuantity SteelOutput;
	SteelOutput.ResourceID = FName("SteelIngots");
	SteelOutput.Quantity = 1;
	SteelIngots.OutputResources.Add(SteelOutput);

	RegisterRecipe(SteelIngots);
}

void URefiningSystem::InitializeCrystallineRecipes()
{
	// Silicon Purification
	FRefiningRecipe PureSilicon;
	PureSilicon.RecipeID = FName("PureSilicon");
	PureSilicon.DisplayName = "Purify Silicon";
	PureSilicon.RequiredModuleType = ERefineryModuleType::Crystallizer;
	PureSilicon.MinimumModuleTier = 2;
	PureSilicon.ProcessingTime = 15.0f;
	PureSilicon.EnergyRequired = 20.0f;
	PureSilicon.SuccessRate = 0.9f;
	PureSilicon.bIsUnlocked = true;
	PureSilicon.Description = "Purify silicon crystals for electronics-grade silicon.";

	FResourceQuantity SiliconInput;
	SiliconInput.ResourceID = FName("Silicon");
	SiliconInput.Quantity = 3;
	PureSilicon.InputResources.Add(SiliconInput);

	FResourceQuantity PureSiliconOutput;
	PureSiliconOutput.ResourceID = FName("PureSilicon");
	PureSiliconOutput.Quantity = 1;
	PureSilicon.OutputResources.Add(PureSiliconOutput);

	RegisterRecipe(PureSilicon);

	// Quartz Purification
	FRefiningRecipe PureQuartz;
	PureQuartz.RecipeID = FName("PureQuartz");
	PureQuartz.DisplayName = "Purify Quartz";
	PureQuartz.RequiredModuleType = ERefineryModuleType::Crystallizer;
	PureQuartz.MinimumModuleTier = 2;
	PureQuartz.ProcessingTime = 12.0f;
	PureQuartz.EnergyRequired = 18.0f;
	PureQuartz.SuccessRate = 0.92f;
	PureQuartz.bIsUnlocked = true;
	PureQuartz.Description = "Purify quartz crystals for optical applications.";

	FResourceQuantity QuartzInput;
	QuartzInput.ResourceID = FName("Quartz");
	QuartzInput.Quantity = 2;
	PureQuartz.InputResources.Add(QuartzInput);

	FResourceQuantity PureQuartzOutput;
	PureQuartzOutput.ResourceID = FName("PureQuartz");
	PureQuartzOutput.Quantity = 1;
	PureQuartz.OutputResources.Add(PureQuartzOutput);

	RegisterRecipe(PureQuartz);
}

void URefiningSystem::InitializeOrganicRecipes()
{
	// Biomass Processing
	FRefiningRecipe ProcessedBiomass;
	ProcessedBiomass.RecipeID = FName("ProcessedBiomass");
	ProcessedBiomass.DisplayName = "Process Biomass";
	ProcessedBiomass.RequiredModuleType = ERefineryModuleType::ChemicalPlant;
	ProcessedBiomass.MinimumModuleTier = 1;
	ProcessedBiomass.ProcessingTime = 8.0f;
	ProcessedBiomass.EnergyRequired = 8.0f;
	ProcessedBiomass.SuccessRate = 1.0f;
	ProcessedBiomass.bIsUnlocked = true;
	ProcessedBiomass.Description = "Process raw biomass into usable fuel.";

	FResourceQuantity BiomassInput;
	BiomassInput.ResourceID = FName("Biomass");
	BiomassInput.Quantity = 5;
	ProcessedBiomass.InputResources.Add(BiomassInput);

	FResourceQuantity FuelOutput;
	FuelOutput.ResourceID = FName("Biofuel");
	FuelOutput.Quantity = 2;
	ProcessedBiomass.OutputResources.Add(FuelOutput);

	RegisterRecipe(ProcessedBiomass);

	// Oil Refining
	FRefiningRecipe RefinedOil;
	RefinedOil.RecipeID = FName("RefinedOil");
	RefinedOil.DisplayName = "Refine Oil";
	RefinedOil.RequiredModuleType = ERefineryModuleType::ChemicalPlant;
	RefinedOil.MinimumModuleTier = 2;
	RefinedOil.ProcessingTime = 20.0f;
	RefinedOil.EnergyRequired = 30.0f;
	RefinedOil.SuccessRate = 0.95f;
	RefinedOil.bIsUnlocked = false;
	RefinedOil.Description = "Refine crude oil into various petroleum products.";

	FResourceQuantity OilInput;
	OilInput.ResourceID = FName("Oil");
	OilInput.Quantity = 3;
	RefinedOil.InputResources.Add(OilInput);

	FResourceQuantity FuelOutput2;
	FuelOutput2.ResourceID = FName("RefinedFuel");
	FuelOutput2.Quantity = 2;
	RefinedOil.OutputResources.Add(FuelOutput2);

	FResourceQuantity PlasticOutput;
	PlasticOutput.ResourceID = FName("Plastic");
	PlasticOutput.Quantity = 1;
	RefinedOil.OutputResources.Add(PlasticOutput);

	RegisterRecipe(RefinedOil);
}

void URefiningSystem::InitializeRadioactiveRecipes()
{
	// Uranium Enrichment
	FRefiningRecipe EnrichedUranium;
	EnrichedUranium.RecipeID = FName("EnrichedUranium");
	EnrichedUranium.DisplayName = "Enrich Uranium";
	EnrichedUranium.RequiredModuleType = ERefineryModuleType::EnrichmentFacility;
	EnrichedUranium.MinimumModuleTier = 3;
	EnrichedUranium.ProcessingTime = 60.0f;
	EnrichedUranium.EnergyRequired = 100.0f;
	EnrichedUranium.SuccessRate = 0.85f;
	EnrichedUranium.bIsUnlocked = false;
	EnrichedUranium.Description = "Enrich uranium ore for nuclear applications.";

	FResourceQuantity UraniumInput;
	UraniumInput.ResourceID = FName("Uranium");
	UraniumInput.Quantity = 5;
	EnrichedUranium.InputResources.Add(UraniumInput);

	FResourceQuantity EnrichedUraniumOutput;
	EnrichedUraniumOutput.ResourceID = FName("EnrichedUranium");
	EnrichedUraniumOutput.Quantity = 1;
	EnrichedUranium.OutputResources.Add(EnrichedUraniumOutput);

	RegisterRecipe(EnrichedUranium);

	// Thorium Processing
	FRefiningRecipe ProcessedThorium;
	ProcessedThorium.RecipeID = FName("ProcessedThorium");
	ProcessedThorium.DisplayName = "Process Thorium";
	ProcessedThorium.RequiredModuleType = ERefineryModuleType::EnrichmentFacility;
	ProcessedThorium.MinimumModuleTier = 2;
	ProcessedThorium.ProcessingTime = 45.0f;
	ProcessedThorium.EnergyRequired = 80.0f;
	ProcessedThorium.SuccessRate = 0.9f;
	ProcessedThorium.bIsUnlocked = false;
	ProcessedThorium.Description = "Process thorium ore for advanced nuclear applications.";

	FResourceQuantity ThoriumInput;
	ThoriumInput.ResourceID = FName("Thorium");
	ThoriumInput.Quantity = 4;
	ProcessedThorium.InputResources.Add(ThoriumInput);

	FResourceQuantity ProcessedThoriumOutput;
	ProcessedThoriumOutput.ResourceID = FName("ProcessedThorium");
	ProcessedThoriumOutput.Quantity = 1;
	ProcessedThorium.OutputResources.Add(ProcessedThoriumOutput);

	RegisterRecipe(ProcessedThorium);
}

void URefiningSystem::InitializeAdvancedRecipes()
{
	// Electronics Manufacturing
	FRefiningRecipe Electronics;
	Electronics.RecipeID = FName("Electronics");
	Electronics.DisplayName = "Manufacture Electronics";
	Electronics.RequiredModuleType = ERefineryModuleType::ChemicalPlant;
	Electronics.MinimumModuleTier = 3;
	Electronics.ProcessingTime = 30.0f;
	Electronics.EnergyRequired = 40.0f;
	Electronics.SuccessRate = 0.88f;
	Electronics.bIsUnlocked = false;
	Electronics.Description = "Manufacture electronic components from refined materials.";

	FResourceQuantity CopperInput;
	CopperInput.ResourceID = FName("CopperIngots");
	CopperInput.Quantity = 2;
	Electronics.InputResources.Add(CopperInput);

	FResourceQuantity SiliconInput;
	SiliconInput.ResourceID = FName("PureSilicon");
	SiliconInput.Quantity = 1;
	Electronics.InputResources.Add(SiliconInput);

	FResourceQuantity PlasticInput;
	PlasticInput.ResourceID = FName("Plastic");
	PlasticInput.Quantity = 1;
	Electronics.InputResources.Add(PlasticInput);

	FResourceQuantity ElectronicsOutput;
	ElectronicsOutput.ResourceID = FName("Electronics");
	ElectronicsOutput.Quantity = 1;
	Electronics.OutputResources.Add(ElectronicsOutput);

	RegisterRecipe(Electronics);

	// Advanced Alloy Production
	FRefiningRecipe AdvancedAlloy;
	AdvancedAlloy.RecipeID = FName("AdvancedAlloy");
	AdvancedAlloy.DisplayName = "Produce Advanced Alloy";
	AdvancedAlloy.RequiredModuleType = ERefineryModuleType::Smelter;
	AdvancedAlloy.MinimumModuleTier = 4;
	AdvancedAlloy.ProcessingTime = 40.0f;
	AdvancedAlloy.EnergyRequired = 60.0f;
	AdvancedAlloy.SuccessRate = 0.92f;
	AdvancedAlloy.bIsUnlocked = false;
	AdvancedAlloy.Description = "Produce advanced alloy combining multiple metals.";

	FResourceQuantity SteelInput;
	SteelInput.ResourceID = FName("SteelIngots");
	SteelInput.Quantity = 2;
	AdvancedAlloy.InputResources.Add(SteelInput);

	FResourceQuantity TitaniumInput;
	TitaniumInput.ResourceID = FName("TitaniumIngots");
	TitaniumInput.Quantity = 1;
	AdvancedAlloy.InputResources.Add(TitaniumInput);

	FResourceQuantity AluminumInput;
	AluminumInput.ResourceID = FName("AluminumIngots");
	AluminumInput.Quantity = 1;
	AdvancedAlloy.InputResources.Add(AluminumInput);

	FResourceQuantity AdvancedAlloyOutput;
	AdvancedAlloyOutput.ResourceID = FName("AdvancedAlloy");
	AdvancedAlloyOutput.Quantity = 1;
	AdvancedAlloy.OutputResources.Add(AdvancedAlloyOutput);

	RegisterRecipe(AdvancedAlloy);
}